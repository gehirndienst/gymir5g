import argparse
import os
from dataclasses import dataclass
from typing import Any, Dict, List, Optional, Self

from gymirdrl import ROOT_DIR

"""
    provides args that are used to run gymirdrl as a console application,
    internally they are used to create an instance of GymirDrlManager.
"""

CFG_DIR = os.path.join(os.path.abspath(os.path.join(ROOT_DIR, "..")), "configurations")


def get_argparser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser()

    ########################################################################################################################################################################################################################################
    # fmt: off
    
    # sim params
    parser.add_argument('-s', '--sim-path', dest='sim_path', type=str, help="an ABSOLUTE path to the folder with a simulation and a bash run script for her. \n\
        IMPORTANT: a simulation should be ALWAYS in the following path: '{$OMNET_WORKSPACE_DIR}/{$PROJECT_DIR}/simulations/sim_folder/sim_file.ini', otherwise your .NED files won't be loaded properly")
    
    parser.add_argument('-host', '--sim-host', dest='sim_host', type=str, default='127.0.0.1', help="the host of a zmq-context for an interprocess communication with the simulation")
    
    parser.add_argument('-port', '--sim-port', dest='sim_port', type=int, default=5555, help="the port of a zmq-context for an interprocess communication with the simulation")
    
    parser.add_argument('-r', '--runs', dest='runs', type=int, default=1, help="number of simulations runs (drl episodes) to perform (max 1000). \
        NOTE: each time a new sim run use a DIFFERENT seed. To run N sim replicas set N times the same run using --same-run <run_num> argument")
    
    parser.add_argument('-fr', '--from-run', dest='from_run', type=int, default=0, help="from which run to perform the simulation (default: 0)")
    
    parser.add_argument('-sr', '--same-run', dest='same_run', type=int, default=-1, help="whether to run the same simulation run (default: -1, if it >= 0, --from-run is ignored)")
    
    parser.add_argument('-scen', '--scenario', dest='scenario', type=str, default='Same', help="simulation scenario, check Config in omnetpp.ini file")    
    
    parser.add_argument('-sc', '--streams-cfg', dest='streams_cfg',  type=str, default=f"{CFG_DIR}/baseDrl/streams.json", help="A json file with streams configuratin for a simulation, \
        look at the example in 'configurations/baseDrl/streams*.json'")
    
    parser.add_argument('-t', '--time-limit', dest='time_limit', type=int, default=640, help="simulation time limit in seconds, check omnetpp.ini sim-time-limit")
    
    parser.add_argument('-u', '--update-period', dest='update_period', type=float, default=1.0, help="sim state update period for simulation in seconds, check omnetpp.ini file **.stateUpdatePeriod")
    
    parser.add_argument('-ad', '--adaptive-algorithm', dest='adaptive_algorithm', type=str, default="drl_base", help="adaptive algorithm, look <common/AdaptiveAlgorithm.h")

    parser.add_argument('-uv', '--use-veins', dest='is_use_veins', default=False, action='store_true', help="whether veins mobility is used and the SUMO daemon has to be run (default: False)")
    
    parser.add_argument('-vi', '--view', dest='is_view', default=False, action='store_true', help="whether to view the content of received video and lidar streams (default: False)")
    
    # drl params
    parser.add_argument('-m', '--mode', dest='mode', type=str, choices=['train', 'eval'], default='train', help="select 'train' or 'eval' mode, for the second one you have to supply a model file with -mf")

    parser.add_argument('-d', '--deterministic', dest='deterministic', action='store_true', help="set deterministic=True for evaluation")
    
    parser.add_argument('-ec', '--env-cfg', dest='env_cfg',  type=str, default=f"{CFG_DIR}/baseDrl/env.json", help="A json file with OpenAI Gym environment configuration, \
        look at the example in 'configurations/baseDrl/env*.json'")

    parser.add_argument('-envs', '--num-envs', dest='num_envs',  type=int, default=1, help="A number of environments used (default: 1). NOTE: some callbacks support only single env training")
    
    parser.add_argument('-mn', '--model-name', dest='model_name', type=str, default="ppo", help="SB3 DRL model name, by default is 'ppo' \
        look at the full dictionary of available names in gymirdrl/model_config.py")
    
    parser.add_argument('-hc', '--hyperparams-cfg', dest='hyperparams_cfg', type=str, default=None, help="SB3 DRL model hyperparameters json file, if None, then default hyperparameters are used. \
        NOTE: there are some templates for hyperparams, look in gymirdrl/templates folder")
    
    parser.add_argument('-mf', '--model-file', dest='model_file', type=str, default=None, help="a zip/tar file with the saved model for evaluation or additional training")
    
    # common params
    parser.add_argument('-cb', '--callbacks', dest='callbacks', nargs='+', default=None, help="list of callbacks , currently you can list:\
        'save_model', 'save_step', 'print_step', 'stop_no_impr'")
    parser.add_argument('-evc', '--eval-cfg', dest='eval_cfg',  type=str, default=f"{CFG_DIR}/evalCallback/default.json", help="A json file with parameters to make an evaluation environment for EvalCallback, \
        look into the examples in 'configurations/evalCallback/*'")
    parser.add_argument('--save-drl-log', dest='is_save_drl_log', action='store_true', help="save DRL model logs (tb, metrics, etc)")
    parser.add_argument('--save-sim-log', dest='is_save_sim_log', action='store_true', help="save stdout/err from the OMNeT++ simulation to the log file in the default log folder")
    parser.add_argument('-v', '--verbose', dest='verbose', type=int, default=1, choices=[0, 1, 2], help="verbose: {0, 1, 2}")

    # fmt: on
    ########################################################################################################################################################################################################################################
    return parser


@dataclass
class GymirArgs:
    """A data class to hold arguments for the Gymir simulation and deep reinforcement learning.

    :param sim_path: The path to the simulation
    :param sim_host: The host address for the simulation
    :param sim_port: The port number for the simulation
    :param runs: The total number of runs for the simulation and episodes for DRL
    :param from_run: The starting run number
    :param same_run: The only run numebr which will be run repeatedly
    :param scenario: The scenario (config) for the simulation
    :param time_limit: The time limit for each run in seconds
    :param update_period: The frequency of a new state generarion in seconds
    :param adaptive_algorithm: The adaptive algorithm str name
    :param streams_cfg: Streams configuration: either a path to a json gile or a dictionary
    :param is_use_veins: Whether to use veins/sumo mobility for the simulation
    :param is_view: Whether to view received streams in real time (video and lidar only)
    :param str mode: Either 'train' or 'eval'
    :param deterministic: Whether the DRL model should be deterministic
    :param env_cfg: Gymnasium Env configuration: either a path to a json gile or a dictionary
    :param num_envs: The number of parallel environments
    :param model_name: The name of the DRL model
    :param hyperparams_cfg: Hyperparameters configuration: either a path to a json gile or a dictionary. Nullable
    :param model_file: The file containing the DRL model
    :param callbacks: Optional list of callbacks for SB3 model given as string aliases. Nullable
    :param eval_cfg: Evaluation configuration for eval callback: either a path to a json gile or a dictionary
    :param is_save_drl_log: Whether to save sb3 logs (logger, tensorboard)
    :param is_save_sim_log: Whether to save OMNeT++ logs (stream logs, stdout)
    :param verbose: The verbosity level: 0, 1, 2
    """

    # sim
    sim_path: str
    sim_host: str
    sim_port: int
    runs: int
    from_run: int
    same_run: int
    scenario: str
    time_limit: int
    update_period: float
    adaptive_algorithm: str
    streams_cfg: str | Dict[str, Any]
    is_use_veins: bool
    is_view: bool
    # drl
    mode: str
    deterministic: bool
    env_cfg: str | Dict[str, Any]
    num_envs: int
    model_name: str
    hyperparams_cfg: str | Dict[str, Any] | None
    model_file: str
    # common
    callbacks: Optional[List[str]]
    eval_cfg: str | Dict[str, Any]
    is_save_drl_log: bool
    is_save_sim_log: bool
    verbose: int

    @classmethod
    def from_namespace(cls, args: argparse.Namespace) -> Self:
        args_dict = {k: v for k, v in vars(args).items() if k != 'help'}
        return cls(**args_dict)
