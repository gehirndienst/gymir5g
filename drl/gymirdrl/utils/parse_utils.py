import argparse
import copy
import json
from typing import Any, Dict, Tuple

from gymirdrl.core.args import get_argparser, GymirArgs


def parse_drl_args_to_namespace(args_dict: Dict[str, Any]) -> argparse.Namespace:
    """
    Parse arguments to a correct Namespace to pass as an input to create an instance of GymirArgs class
    P.S. to get a dict with default values call 'vars(get_argparser().parse_args(args=[]))'

    :param args_dict: arguments as a dictionary to parse them to a correct namespace
    :return: a namespace with all arguments
    """
    parser = get_argparser()
    parsed_args = parser.parse_args(args=[])
    for key, value in args_dict.items():
        action = next((action for action in parser._actions if action.dest == key), None)
        if action:
            if isinstance(action.choices, list) and value not in action.choices:
                raise ValueError(
                    f"invalid value for argument '{key}'. Allowed choices are:"
                    f" {', '.join(str(choice) for choice in action.choices)}"
                )
            setattr(parsed_args, key, value)
        else:
            raise ValueError(f"parse_drl_args: invalid argument: {key}")

    # fill missing arguments with their default values
    defaults = {action.dest: action.default for action in parser._actions}
    for key, d_value in defaults.items():
        if not hasattr(parsed_args, key):
            setattr(parsed_args, key, d_value)

    return parsed_args


def parse_drl_args_to_gymir_args(args_dict: Dict[str, Any]) -> GymirArgs:
    """
    Parse arguments to a correct Namespace to pass as an input to create an instance of GymirArgs class.
    Therefore unfilled values will be correctly filled with default values and then unfolded to a GymirArgs instance

    :param args_dict: arguments as a dictionary to parse them to a correct namespace
    :return: a namespace with all arguments
    """
    return GymirArgs.from_namespace(parse_drl_args_to_namespace(args_dict))


def parse_eval_args(
    eval_cfg: str | Dict[str, Any],
    sim_args: GymirArgs,
) -> Tuple[GymirArgs, Dict]:
    """
    Parse configuration for evaluation callback to a dictionary. If incorrect keys are found, it raises an exception

    :param eval_cfg: either a path to a json file or a dict with arguments for EvalCallback instantiation
    :param sim_args: default sim runner args (they stay const)
    :return: a tuple with sim and callback args to create an instance of a gymir version of EvalCallback
    """
    eval_sim_args = copy.deepcopy(sim_args)

    eval_callback_args = {
        "min_evals": -1,
        "eval_freq": -1,
        "max_no_improvement_evals": -1,
        "deterministic": False,
    }

    if isinstance(eval_cfg, str):
        with open(eval_cfg, "r") as f:
            eval_dict = json.load(f)
    else:
        eval_dict = eval_cfg

    for k, v in eval_dict.items():
        match k:
            case "sim_path":
                eval_sim_args.sim_path = v
            case "scenario":
                eval_sim_args.scenario = v
            case "streams_cfg":
                eval_sim_args.streams_cfg = v
            case "from_run":
                eval_sim_args.from_run = v
            case "episodes":
                eval_sim_args.runs = v
            case "steps":
                eval_sim_args.time_limit = eval_sim_args.update_period * v
            case "warmup_steps":
                eval_callback_args["min_evals"] = v
            case "eval_freq":
                eval_callback_args["eval_freq"] = v
            case "patience":
                eval_callback_args["max_no_improvement_evals"] = v
            case "deterministic":
                eval_callback_args["deterministic"] = v
            case _:
                raise Exception(f"Unknown key {k} with value {v} was found during parsing eval cb args")
    return eval_sim_args, eval_callback_args
