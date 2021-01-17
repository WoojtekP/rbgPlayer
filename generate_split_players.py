"""
Usage:
* read from argv:
    python3 generate_split_players.py [...agents...]

* read from stdin
    python3 generate_split_players.py --stdin < file.txt

Use short names: 'semisplitNodal_semisplit_mastsplit' instead of 'mctsM_semisplitNodal_semisplit_mastsplit.json'
"""
import json
import sys
import os


def get_agents_from_stdin():
    for name in sys.stdin:
        yield name.strip()

def get_agents_from_argv():
    for name in sys.argv[1:]:
        yield name

get_agents = get_agents_from_stdin if len(sys.argv) == 2 and sys.argv[1] == "--stdin" else get_agents_from_argv

os.chdir(os.path.realpath(sys.path[0]))

for agent in get_agents():
    M_agent = "agents/mctsM_" + agent + ".json"
    MP_agent = "agents/mctsMP_" + agent + ".json"
    MPS_agent = "agents/mctsMPS_" + agent + ".json"
    MS_agent = "agents/mctsMS_" + agent + ".json"
    if not os.path.isfile(M_agent):
        print("error: ", agent, "does not exist - skipped")
        continue
    with open(M_agent) as m_config_file:
        agent_config = json.load(m_config_file)
        agent_config["algorithm"]["split_strategy"] = "ModPlus"
        with open(MP_agent, "w") as mp_config_file:
            json.dump(agent_config, mp_config_file, indent=4)
            print(agent, "MP  created successfully")
        agent_config["algorithm"]["split_strategy"] = "ModPlusShift"
        with open(MPS_agent, "w") as mps_config_file:
            json.dump(agent_config, mps_config_file, indent=4)
            print(agent, "MPS created successfully")
        agent_config["algorithm"]["split_strategy"] = "ModShift"
        with open(MS_agent, "w") as ms_config_file:
            json.dump(agent_config, ms_config_file, indent=4)
            print(agent, "MS created successfully")
