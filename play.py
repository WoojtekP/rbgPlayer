import sys
import socket
import os
import subprocess
import shutil
import queue
import time
import argparse
import signal
import json
import time
import re
from threading import Thread
from argparse import RawTextHelpFormatter
from itertools import chain


script_dir = sys.path[0]
os.chdir(script_dir)

compiler_dir = os.path.realpath(script_dir + "/../rbg2cpp/bin/rbg2cpp")
split_strategies_dir = os.path.realpath(script_dir + "/split_strategies") + "/"
game_name = "game"
build_dir = "build/"

available_players = set()
with open("Makefile", "r") as makefile:
    reg = re.compile("(\$\(eval \$\(call PLAYER_KIND_RULES\,[A-Z_]+\,)([a-z_]+)")
    for line in makefile.readlines():
        match = reg.search(line)
        if match:
            available_players.add(match.group(2))

split_strategies = {
    "ModPlus"      : "splitStrategy_Plus.sh",
    "ModPlusShift" : "splitStrategy_PlusShift.sh",
    "ModShift"     : "splitStrategy_Shift.sh"
}

def gen_directory(player_id):
    return build_dir+"gen_"+str(player_id)

def gen_inc_directory(player_id):
    return gen_directory(player_id)+"/inc"

def gen_src_directory(player_id):
    return gen_directory(player_id)+"/src"

def game_path(player_id):
    return gen_directory(player_id)+"/"+game_name+".rbg"

class BufferedSocket:
    def __init__(self, s):
        self.current_buffer = bytearray()
        self.working_socket = s
    def extend_buffer(self):
        len_before = len(self.current_buffer)
        chunk = self.working_socket.recv(2048)
        self.current_buffer += chunk
        return len_before, len_before+len(chunk)
    def cut_on_index(self, index):
        to_return = self.current_buffer[:index]
        self.current_buffer = self.current_buffer[index+1:]
        return to_return
    def read_until(self, byte):
        for i, b in enumerate(self.current_buffer):
            if b == byte:
                return self.cut_on_index(i)
        while True:
            search_begin, search_end = self.extend_buffer()
            if search_begin == search_end:
                return self.cut_on_index(search_end)
            else:
                for i in range(search_begin, search_end):
                    if self.current_buffer[i] == byte:
                        return self.cut_on_index(i)
    def receive_message(self):
        return self.read_until(0)
    def send_message(self, msg):
        self.working_socket.send(msg+b'\0')
    def shutdown(self):
        self.working_socket.shutdown(socket.SHUT_RDWR)

class Cd:
    def __init__(self, new_path):
        self.new_path = os.path.expanduser(new_path)
    def __enter__(self):
        self.saved_path = os.getcwd()
        os.chdir(self.new_path)
    def __exit__(self, etype, value, traceback):
        os.chdir(self.saved_path)

class ConfigData:
    def __init__(self, program_args):
        with open(program_args.player_config_file) as config_file:
            config = json.load(config_file)
            constants = { x : dict() for x in ["bool", "double", "int", "uint"] }
            for k, v in chain(config["algorithm"]["parameters"].items(),
                            *(heuristic["parameters"].items() for heuristic in config["heuristics"])):
                if isinstance(v, bool):
                    constants["bool"][k.upper()] = v.__str__().lower()
                elif isinstance(v, float):
                    constants["double"][k.upper()] = v.__str__()
                elif v >= 0:
                    constants["uint"][k.upper()] = v.__str__()
                else:
                    constants["int"][k.upper()] = v.__str__()
            constants["uint"]["BUFFER_TIME"] = config["general"]["buffer_time"].__str__()
            constants["bool"]["SIMULATE_DURING_OPP_TURN"] = config["general"]["simulate_during_opp_turn"].__str__().lower()
            if "GREEDY_CHOICE" in constants["bool"] and constants["bool"]["GREEDY_CHOICE"] == "true":
                constants["uint"]["FINAL_ROLLUP_FACTOR"] = "0"
            for heuristic in config["heuristics"]:
                if heuristic["name"].upper() in ["RAVEMIX", "RAVECONTEXT"]:
                    constants["uint"]["REF"] = "0"
            self.constants = constants
            self.tree_strategy = config["algorithm"]["tree_strategy"].lower()
            self.simulation_strategy = config["algorithm"]["simulation_strategy"].lower()
            self.reasoning_overhead = config["general"]["reasoning_overhead"]
            self.heuristics = [heuristic["name"].upper() for heuristic in config["heuristics"]]
            self.debug = int(program_args.debug)
            self.release = int(program_args.release)
            self.stats = int(program_args.stats)
            if "split_strategy" in config["algorithm"] and config["algorithm"]["split_strategy"] != "":
                self.split_strategy = config["algorithm"]["split_strategy"]
                if self.split_strategy not in split_strategies.keys():
                    print("split strategy:", self.split_strategy, "is not available")
                    exit(1)
            else:
                self.split_strategy = None
            self.getters = list()
            if self.simulation_strategy == "orthodox" or self.tree_strategy == "orthodox":
                self.getters.append("m")
            if self.tree_strategy == "rollup":
                self.getters.append("s")
            elif self.simulation_strategy == "semisplit" or self.tree_strategy == "semisplit":
                if self.split_strategy is None or "Mod" in self.split_strategy:
                    self.getters.append("a")
                else:
                    self.getters.append("s")
            self.player_strategy = self.get_player_strategy(config);
    def get_player_strategy(self, config):
        player_kind = config["algorithm"]["tree_strategy"].lower() + "_" + config["algorithm"]["simulation_strategy"].lower()
        heuristics = set(heuristic["name"].lower() for heuristic in config["heuristics"])
        if len(heuristics) == 0:
            return player_kind
        if player_kind == "semisplit_semisplit" and 'a' in self.getters and "mast" in heuristics:
            heuristics.remove("mast")
            heuristics.add("mastsplit")
        if player_kind == "orthodox_orthodox" and "mastcontext" in heuristics:
            heuristics.remove("mastcontext")
            heuristics.add("mast")
        if player_kind == "orthodox_orthodox" and "ravecontext" in heuristics:
            heuristics.remove("ravecontext")
            heuristics.add("tgrave")
        return player_kind + "_" + "_".join(sorted(heuristics))

class PlayerConfig:
    def __init__(self, program_args, config_data, player_name, player_port):
        self.config_data =  config_data
        self.address_to_connect = "127.0.0.1"
        self.port_to_connect = player_port
        self.player_name = player_name
        self.simulations_per_move = program_args.simulations_per_move
        self.simulations_limit = program_args.simulations_per_move > 0
        self.states_per_move = program_args.states_per_move
        self.states_limit = program_args.states_per_move > 0
        self.debug = program_args.debug
        if self.simulations_limit and self.states_limit:
            print("at most one type of limit is allowed", sys.stderr)
            exit(1)
    def runnable_list(self):
        return (["valgrind"] if self.debug else []) + [build_dir + "bin_"+str(self.port_to_connect)+"/"+self.config_data.player_strategy]
    def print_config_file(self, name):
        with open(gen_inc_directory(self.port_to_connect)+"/"+name,"w") as config_file:
            config_file.write("#ifndef CONFIG\n")
            config_file.write("#define CONFIG\n")
            config_file.write("\n")
            config_file.write("#define {}_TREE\n".format(self.config_data.tree_strategy.upper()))
            config_file.write("#define {}_SIMULATOR\n".format(self.config_data.simulation_strategy.upper()))
            for getter in ['m', 's', 'a']:
                if getter in self.config_data.getters:
                    config_file.write("#define GETTER_{}\n".format(getter.upper()))
            config_file.write("#define REASONING_OVERHEAD {}\n".format(self.config_data.reasoning_overhead))
            config_file.write("\n")
            config_file.write("#include \"types.hpp\"\n")
            config_file.write("#include <string>\n")
            config_file.write("\n")
            config_file.write("const std::string ADDRESS = \"{}\";\n".format(self.address_to_connect))
            config_file.write("constexpr uint PORT = {};\n".format(str(self.port_to_connect)))
            config_file.write("const std::string NAME = \"{}\";\n".format(self.player_name))
            config_file.write("constexpr bool SIMULATIONS_LIMIT = {};\n".format(str(self.simulations_limit).lower()))
            config_file.write("constexpr bool STATES_LIMIT = {};\n".format(str(self.states_limit).lower()))
            config_file.write("constexpr uint SIMULATIONS_PER_MOVE = {};\n".format(str(self.simulations_per_move)))
            config_file.write("constexpr uint STATES_PER_MOVE = {};\n".format(str(self.states_per_move)))
            config_file.write("\n")
            for t, variables in self.config_data.constants.items():
                for name, val in variables.items():
                    config_file.write("constexpr {} {} = {};\n".format(t, name, val))
            config_file.write("\n")
            if 's' in self.config_data.getters:
                config_file.write("typedef reasoner::move semimove;\n\n")
            elif 'a' in self.config_data.getters:
                config_file.write("typedef reasoner::action_representation semimove;\n\n")
            config_file.write("#endif\n")

def get_game_section(game, section):
    game_sections = game.split("#")
    for s in game_sections:
        section_fragments = s.split("=")
        if section_fragments[0].strip() == section:
            return section_fragments[1]

def get_comma_separated_item(text, number):
    split_text = text.split(",")
    return split_text[number].strip()

def get_player_name_from_players_item(item):
    split_item = item.split("(",1)
    return split_item[0].strip()

def extract_player_name(game, player_number):
    players_section = get_game_section(game, "players")
    player_item = get_comma_separated_item(players_section, player_number-1)
    return get_player_name_from_players_item(player_item)

def receive_preparation_seconds(server_socket):
    return float(server_socket.receive_message())

def write_game_to_file(server_socket, player_id):
    game = str(server_socket.receive_message(), "utf-8")
    os.makedirs(gen_directory(player_id))
    os.makedirs(gen_inc_directory(player_id))
    os.makedirs(gen_src_directory(player_id))
    with open(game_path(player_id), 'w') as out:
        out.write(game + "\n")
    return game

def receive_player_name(server_socket, game):
    player_number = int(str(server_socket.receive_message(), "utf-8"))
    return extract_player_name(game, player_number)

def apply_split_strategy(split_strategy, player_id):
    if split_strategy is not None:
        print("Using split strategy:", split_strategy)
        splitter_run_list = [split_strategies_dir + split_strategies[split_strategy], os.path.realpath(game_path(player_id))]
        result = subprocess.run(splitter_run_list)
        if result.returncode != 0:
            print("Error: split strategy wasn\'t applied properly")
            exit(1);

def compile_player(config_data, player_id):
    assert(config_data.player_strategy in available_players)
    with Cd(gen_directory(player_id)):
        compiler_run_list = [compiler_dir, "-getters", ''.join(config_data.getters), "-o", "reasoner", "../../"+game_path(player_id)]
        subprocess.run(compiler_run_list)
    shutil.move(gen_directory(player_id)+"/reasoner.cpp", gen_src_directory(player_id)+"/reasoner.cpp")
    shutil.move(gen_directory(player_id)+"/reasoner.hpp", gen_inc_directory(player_id)+"/reasoner.hpp")
    heuristics = config_data.heuristics
    run_list = [
        "make",
        config_data.player_strategy,
        "PLAYER_ID="+str(player_id),
        "DEBUG="+str(config_data.debug),
        "RELEASE="+str(config_data.release),
        "STATS="+str(config_data.stats),
        "MAST="+str(int("MAST" in heuristics or "MASTSPLIT" in heuristics) + 2 * int("MASTCONTEXT" in heuristics or "MASTMIX" in heuristics)),
        "RAVE="+str(int("TGRAVE" in heuristics) + 2 * int("RAVECONTEXT" in heuristics) + 3 * int("RAVEMIX" in heuristics))]
    subprocess.run(run_list)

def connect_to_server(server_address, server_port):
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.connect((server_address, server_port))
    server_socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
    server_socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_QUICKACK, 1)
    return server_socket

def wait_for_player_connection(player_address, return_value_queue):
    accept_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    accept_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    accept_socket.bind((player_address, 0))
    return_value_queue.put(accept_socket.getsockname()[1])
    accept_socket.listen(1)
    player_socket, _ = accept_socket.accept()
    player_socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
    player_socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_QUICKACK, 1)
    return_value_queue.put(player_socket)

def start_player(player_config):
    return subprocess.Popen(player_config.runnable_list())

def start_and_connect_player(player_address, player_config, return_value_queue, player_connection_wait):
    player_process = start_player(player_config)
    player_connection_wait.join()
    return BufferedSocket(return_value_queue.get()), player_process

def start_player_listener(player_address):
    return_value_queue = queue.Queue()
    player_connection_wait = Thread(target = wait_for_player_connection, args = (player_address, return_value_queue))
    player_connection_wait.start()
    return return_value_queue.get(), return_value_queue, player_connection_wait

def forward_and_log(source_socket, target_socket, log_begin, log_end, role):
    while True:
        data = source_socket.receive_message()
        if len(data) == 0:
            print("Connection to",role,"lost! Exitting...")
            target_socket.shutdown()
            quit()
        human_readable = data
        print(log_begin, human_readable, log_end)
        if data != b'reset':
            target_socket.send_message(data)
        else:
            print("Silently dropping reset event...")

def cleanup_process(player_process):
    print("Killing player process...")
    player_process.terminate()


if not os.path.exists(build_dir):
    os.makedirs(build_dir)
parser = argparse.ArgumentParser(description='Setup and start rbg player.', formatter_class=RawTextHelpFormatter)
parser.add_argument('server_address', metavar='server-address', type=str, help='ip address of game manager')
parser.add_argument('server_port', metavar='server-port', type=int, help='port number of game manager')
parser.add_argument('player_config_file', metavar='player-config', type=str, help='name of file with player configuration')
parser.add_argument('--simulations-limit', dest='simulations_per_move', type=int, default=-1, help='simulations limit for player\'s turn')
parser.add_argument('--states-limit', dest='states_per_move', type=int, default=-1, help='states limit for player\'s turn')
parser.add_argument('--debug', action='store_true', default=False, help='run using valgrind')
parser.add_argument('--release', action='store_true', default=False, help='run without any debug for maximal efficiency')
parser.add_argument('--stats', action='store_true', default=False, help='print statistics for legal moves')
program_args = parser.parse_args()

server_socket = BufferedSocket(connect_to_server(program_args.server_address, program_args.server_port))
print("Successfully connected to server!")

player_port, listener_queue, listener_thread = start_player_listener("localhost")

preparation_seconds = receive_preparation_seconds(server_socket)
print("Server claims to give",preparation_seconds,"for preparation")

start_time = time.time()

game = write_game_to_file(server_socket, player_port)
print("Received game rules")

player_name = receive_player_name(server_socket, game)
print("Received player name:",player_name)

config_data = ConfigData(program_args)
player_config = PlayerConfig(program_args, config_data, player_name, player_port)
player_config.print_config_file("config.hpp")

apply_split_strategy(config_data.split_strategy, player_port)
print("Game rules written to:", game_path(player_port))

compile_player(config_data, player_port)
print("Player compiled!")

player_socket, player_process = start_and_connect_player("localhost", player_config, listener_queue, listener_thread)
print("Player started on port",player_port)

end_time = time.time()
print("Player preparations took",end_time-start_time)
server_socket.send_message(b'ready')

server_to_client = Thread(target = forward_and_log, args = (server_socket, player_socket, "Server says-->","<--","server"))
client_to_server = Thread(target = forward_and_log, args = (player_socket, server_socket, "Client says-->","<--","client"))
client_to_server.daemon = True
server_to_client.daemon = True
client_to_server.start()
server_to_client.start()
signal.signal(signal.SIGTERM, lambda _1,_2: cleanup_process(player_process))
client_to_server.join()
server_to_client.join()
player_process.terminate()
