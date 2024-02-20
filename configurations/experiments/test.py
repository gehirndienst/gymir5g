import os
from gymirdrl import ROOT_DIR
from gymirdrl.core.args import GymirArgs
from gymirdrl.core.drl_manager import GymirDrlManager
from gymirdrl.utils.parse_utils import parse_drl_args_to_gymir_args

SIMULATIONS_FOLDER = os.path.join(os.path.dirname(ROOT_DIR), "simulations")
CONFIGURATIONS_FOLDER = os.path.join(os.path.dirname(ROOT_DIR), "configurations")

print(f"configurations folder: {CONFIGURATIONS_FOLDER} \nsim folder: {SIMULATIONS_FOLDER}")

# PARAMS
SIMULATION_FILE = "baseDrlBg/omnetpp.ini"
SIMULATION_PORT = 5555

SCENARIO = "HardRTCDirect"
STREAMS_CFG = "baseDrl/streams_sim_same.json"
RUNS = 10
FROM_RUN = 1
TIME = 1024
STATE_UPDATE_PERIOD = 1.0

# DRL PARAMS
MODE = "train"
DET = False
ENV_CFG = {
    "state_vars": [
        "rxGoodput",
        "rxFecGoodput",
        "lossRate",
        "fractionLossRate",
        "fractionRetransmissionRate",
        "fractionRepairRate",
        "fractionNackRate",
        "fractionPlayRate",
        "fractionAvPlayoutDelay",
        "fractionStallingRate",
        "fractionAvDelay",
        "fractionAvJitter",
        "fractionAvQueuingTime",
    ],
    "state_constants": {
        "MAX_RATE": 10.0,
        "MAX_DELAY": 1000.0,
        "MAX_JITTER": 625.0,
        "MAX_NACKS": 2,
        "FEEDBACK_PERIOD": 0.1,
        "REPORT_PERIOD": 1.0,
    },
    "reward_function": "qoe_new",
    "action_space": "Discrete(5)",
    "is_scale": False,
}

MODEL_FILE = "model_20230926-154945/last_full.zip"

# run experiment
args_dict = {
    'sim_path': os.path.join(SIMULATIONS_FOLDER, SIMULATION_FILE),
    'sim_port': SIMULATION_PORT,
    'scenario': SCENARIO,
    'runs': RUNS,
    'from_run': FROM_RUN,
    'update_period': STATE_UPDATE_PERIOD,
    'time_limit': TIME,
    'streams_cfg': os.path.join(CONFIGURATIONS_FOLDER, STREAMS_CFG),
    'mode': "train",
    'env_cfg': os.path.join(CONFIGURATIONS_FOLDER, ENV_CFG) if isinstance(ENV_CFG, str) else ENV_CFG,
    'model_file': os.path.join(os.path.join(ROOT_DIR, "models"), MODEL_FILE),
    'deterministic': DET,
    'callbacks': ['save_model', 'save_step', 'print_step'],
    'is_save_drl_log': True,
    'is_save_sim_log': True,
}

args = parse_drl_args_to_gymir_args(args_dict)

# go
manager = GymirDrlManager(args)
manager.learn() if MODE == "train" else manager.eval()
