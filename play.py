import sys
import socket
import os
import subprocess
import shutil
from threading import Thread
import queue
import time
import argparse
from argparse import RawTextHelpFormatter
import signal
import json
from itertools import chain

def gen_directory(player_id):
    return "gen_"+str(player_id)
def gen_inc_directory(player_id):
    return gen_directory(player_id)+"/inc"
def gen_src_directory(player_id):
    return gen_directory(player_id)+"/src"
game_name = "game"
def game_path(player_id):
    return gen_directory(player_id)+"/"+game_name+".rbg"
available_players = set(["random", "mcts_orthodox", "mcts_orthodox_semisplit", "mast", "mast_semisplit", "simple_best_select"])
semisplit_players = set(["mcts_orthodox_semisplit"])

def player_kind_to_make_target(player_kind):
    if player_kind == "semisplitNodalMcts":
        return "semisplitMcts"
    else:
        return player_kind

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

class PlayerConfig:
    def __init__(self, program_args, player_kind, constants, player_name, player_port):
        self.player_kind = player_kind
        self.config_constants = constants
        self.address_to_connect = "127.0.0.1"
        self.port_to_connect = player_port
        self.player_name = player_name
        self.miliseconds_per_move = program_args.miliseconds_per_move
        self.simulations_limit = program_args.simulations_limit
        self.debug_mode = program_args.debug
    def runnable_list(self):
        return (["valgrind"] if self.debug_mode else []) + ["bin_"+str(self.port_to_connect)+"/"+player_kind_to_make_target(self.player_kind)]
    def print_config_file(self, name):
        with open(gen_inc_directory(self.port_to_connect)+"/"+name,"w") as config_file:
            config_file.write("#ifndef CONFIG\n")
            config_file.write("#define CONFIG\n")
            config_file.write("\n")
            config_file.write("#include \"types.hpp\"\n")
            config_file.write("#include <string>\n")
            config_file.write("\n")
            config_file.write("const std::string ADDRESS = \"{}\";\n".format(self.address_to_connect))
            config_file.write("constexpr uint PORT = {};\n".format(str(self.port_to_connect)))
            config_file.write("const std::string NAME = \"{}\";\n".format(self.player_name))
            config_file.write("constexpr uint MILISECONDS_PER_MOVE = {};\n".format(str(self.miliseconds_per_move)))
            config_file.write("constexpr uint SIMULATIONS_PER_MOVE = {};\n".format(str(self.simulations_limit)))
            config_file.write("\n")
            for t, variables in self.config_constants.items():
                for name, val in variables.items():
                    config_file.write("constexpr {} {} = {};\n".format(t, name, val))
            config_file.write("\n")
            config_file.write("#endif\n")

def parse_config_file(file_name):
    with open(file_name) as config_file:
        config = json.load(config_file)
        player_kind = config["heuristic_configuration"]["name"].lower()
        if config["heuristic_configuration"]["name"] == "ORTHODOX":
            player_kind = config["player_configuration"]["name"].lower() + "_orthodox"
        if config["heuristic_configuration"]["semisplit"]:
            player_kind += "_semisplit"
        constants = { x : dict() for x in ["bool", "double", "int", "uint"] }
        for k, v in chain(config["game_configuration"].items(), \
                          config["player_configuration"]["parameters"].items(), \
                          config["heuristic_configuration"]["parameters"].items()):
            if isinstance(v, bool):
                constants["bool"][k.upper()] = v.__str__().lower()
            elif isinstance(v, float):
                constants["double"][k.upper()] = v.__str__()
            elif v > 0:
                constants["uint"][k.upper()] = v.__str__()
            else:
                constants["int"][k.upper()] = v.__str__()
        return player_kind, constants

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

def write_game_to_file(server_socket, player_id):
    subprocess.run(["make", "distclean"]) # to avoid problems with mobile directories dependencies
    game = str(server_socket.receive_message(), "utf-8")
    time.sleep(1.) # not to mess with other players' make clean invocations
    os.makedirs(gen_directory(player_id))
    os.makedirs(gen_inc_directory(player_id))
    os.makedirs(gen_src_directory(player_id))
    with open(game_path(player_id), 'w') as out:
        out.write(game + "\n")
    return game

def receive_player_name(server_socket, game):
    player_number = int(str(server_socket.receive_message(), "utf-8"))
    return extract_player_name(game, player_number)

def compile_player(num_of_threads, player_kind, player_id, debug_mode):
    with Cd(gen_directory(player_id)):
        if player_kind in semisplit_players:
            subprocess.run(["../rbg2cpp/bin/rbg2cpp", "-fsemi-split", "-o", "reasoner", "../"+game_path(player_id)]) # assume description is correct
        else:
            subprocess.run(["../rbg2cpp/bin/rbg2cpp", "-o", "reasoner", "../"+game_path(player_id)]) # assume description is correct
    shutil.move(gen_directory(player_id)+"/reasoner.cpp", gen_src_directory(player_id)+"/reasoner.cpp")
    shutil.move(gen_directory(player_id)+"/reasoner.hpp", gen_inc_directory(player_id)+"/reasoner.hpp")
    print("   subprocess.run: make", "-j"+str(num_of_threads), player_kind_to_make_target(player_kind), "PLAYER_ID="+str(player_id), "DEBUG="+str(debug_mode))
    subprocess.run(["make", "-j"+str(num_of_threads), player_kind_to_make_target(player_kind), "PLAYER_ID="+str(player_id), "DEBUG="+str(debug_mode)]) # again, assume everything is ok

def connect_to_server(server_address, server_port):
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.connect((server_address, server_port))
    return server_socket

def wait_for_player_connection(player_address, return_value_queue):
    accept_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    accept_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    accept_socket.bind((player_address, 0))
    return_value_queue.put(accept_socket.getsockname()[1])
    accept_socket.listen(1)
    player_socket, _ = accept_socket.accept()
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

parser = argparse.ArgumentParser(description='Setup and start rbg player.', formatter_class=RawTextHelpFormatter)
parser.add_argument('server_address', metavar='server-address', type=str, help='ip address of game manager')
parser.add_argument('server_port', metavar='server-port', type=int, help='port number of game manager')
parser.add_argument('player_config', metavar='player-config', type=str, help='path to file with player configuration')
parser.add_argument('--miliseconds-per-move', dest='miliseconds_per_move', type=int, default=2000, help='time limit for player\'s turn in miliseconds (default: 2000)')
parser.add_argument('--simulations-limit', dest='simulations_limit', type=int, default=1000000, help='simulations limit for player\'s turn (default: 1000000)')
parser.add_argument('--debug', action='store_true', default=False, help='run using valgrind')
program_args = parser.parse_args()

server_socket = BufferedSocket(connect_to_server(program_args.server_address, program_args.server_port))
print("Successfully connected to server!")

player_port, listener_queue, listener_thread = start_player_listener("localhost")

game = write_game_to_file(server_socket, player_port)
print("Game rules written to:",game_path(player_port))

player_name = receive_player_name(server_socket, game)
print("Received player name:",player_name)

player_kind, constants = parse_config_file(program_args.player_config)
print("Player kind:", player_kind)

player_config = PlayerConfig(program_args, player_kind, constants, player_name, player_port)
player_config.print_config_file("config.hpp")

compile_player(2, player_kind, player_port, int(program_args.debug))
print("Player compiled!")
time.sleep(1.) # to give other players time to end compilation

player_socket, player_process = start_and_connect_player("localhost", player_config, listener_queue, listener_thread)
print("Player started on port",player_port)

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
