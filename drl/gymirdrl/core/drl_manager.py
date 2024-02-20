import csv
import gymnasium
import math
import multiprocessing
import os
import sys
import time
import torch
import warnings
from typing import Any, Callable, Dict, List, Optional, Union

from stable_baselines3.common.callbacks import BaseCallback, StopTrainingOnNoModelImprovement
from stable_baselines3.common.evaluation import evaluate_policy
from stable_baselines3.common.logger import configure
from stable_baselines3.common.vec_env import SubprocVecEnv

from gymirdrl import ROOT_DIR
from gymirdrl.core.args import GymirArgs
from gymirdrl.core.callbacks import (
    GymirAsyncEvalCallback,
    GymirBreakCallback,
    GymirCheckpointCallback,
    GymirPrintStepCallback,
    GymirSaveStepCallback,
    GymirEvaluatingCallback,
)
from gymirdrl.core.env import GymirEnv
from gymirdrl.core.model_config import GymirDrlModelConfigurator
from gymirdrl.core.sim_runner import GymirSimRunner
from gymirdrl.utils.parse_utils import parse_eval_args


class GymirDrlManager:
    """
    A manager for preprocessing, running and overall controlling of DRL training/evaluation process, namely it:
        1) instantiates a single or vectorized Gym environment and corresponding simulation subprocesses,
        2) configures SB3 DRL model according to the given hyperparameters and configuration settings,
        3) attaches loggers and callbacks and controls the output directories,
        4) performs a training or evaluation process for the DRL model using SB3 backend.

    :param args: Simulation and DRL model params, look into ``args.py``.
    :param callbacks: Callbacks given explicitly as instances or as str aliases.
    :param save_log_path: Path to save all log files, default is ``$(PROJ_DIR)/logs``.
    :param save_model_path: Path to save DRL model, default is ``$(PROJ_DIR)/models``
    :param is_reduce_nenvs_by_cpus: Whether to reduce the number of envs according to available CPUs
    """

    MAX_EPISODES = 1000
    MAX_ENVS = 16

    def __init__(
        self,
        args: GymirArgs,
        callbacks: Union[None, List[BaseCallback], List[str]] = None,
        save_log_path: Optional[str] = None,
        save_model_path: Optional[str] = None,
        is_reduce_nenvs_by_cpus: bool = False,
    ):
        self.args = args
        self.busy_ports = []

        self.log_path, self.model_path = self._set_save_paths(save_log_path, save_model_path)
        self.num_envs = self._set_num_envs(args.num_envs, is_reduce_nenvs_by_cpus)
        self.callbacks = self._set_callbacks(callbacks if callbacks is not None else args.callbacks)

        self._setup()

    def _setup(self) -> None:
        # set episodes and total (max) timesteps for the process
        self.episodes = self.args.runs if self.args.runs > 0 else GymirDrlManager.MAX_EPISODES
        self.episode_length = math.ceil(float(self.args.time_limit) / self.args.update_period)
        self.total_timesteps = self.episodes * self.episode_length * self.num_envs
        print(f"Number of environments: {self.num_envs}")
        print(f"Episodes: {self.episodes}, episode length: {self.episode_length}, timesteps: {self.total_timesteps}")

        # check for CUDA and set up the device
        if torch.cuda.is_available():
            self.device = "cuda"
            curr_dev = torch.cuda.current_device()
            gpu = torch.cuda.get_device_properties(curr_dev)
            print(
                f"Cuda is ON: found {torch.cuda.device_count()} GPUs available. Using the following GPU"
                f" {curr_dev} {gpu.name} with {gpu.total_memory / 1e9}Gb of total memory"
            )
        else:
            self.device = "cpu"
            print("Cuda is OFF: using cpu only\n")

        # initialize environment with corresponding sim runner(s): either a single or a vectorized one
        if self.num_envs == 1:
            # single environment
            self.env = GymirEnv(
                mdp_config=self.args.env_cfg, runner=self._make_sim_runner(), max_episodes=self.episodes
            )
        else:
            # vectorized environment
            sim_runners = [self._make_sim_runner(port=self.args.sim_port + n) for n in range(self.num_envs)]
            self.env = SubprocVecEnv([self._make_env(sim_runner) for sim_runner in sim_runners])

        # initialize SB3 DRL model
        if isinstance(self.args.hyperparams_cfg, Dict):
            model_cfg = GymirDrlModelConfigurator(
                model_name=self.args.model_name,
                tensorboard_log=self.log_path if self.args.is_save_drl_log else None,
                device=self.device,
                verbose=self.args.verbose,
                **self.args.hyperparams_cfg,
            )
        else:
            model_cfg = GymirDrlModelConfigurator(
                model_name=self.args.model_name,
                model_hyperparams_file=self.args.hyperparams_cfg,
                n_steps=self.episode_length,
                tensorboard_log=self.log_path if self.args.is_save_drl_log else None,
                device=self.device,
                verbose=self.args.verbose,
            )

        # a path to a saved model file without extension if exists
        try:
            self.model_file = os.path.splitext(self.args.model_file)[0] if self.args.model_file is not None else None
        except OSError:
            raise Exception(f"There is no valid DRL model on this path : {self.model_file}")

        # make model
        if self.model_file is None:
            # make a fresh one
            self.model = model_cfg.make_model(self.env)
            self.is_reset_timesteps = True
            print(f"Successfully created a new {self.args.model_name} model with the given hyperparameters!\n")
        else:
            # load the model from the given file
            self.model = model_cfg.get_model_class().load(self.model_file, env=self.env, device=self.device)
            self.is_reset_timesteps = False
            print(f"Successfully loaded {self.args.model_name} model from the given file {self.model_file}!\n")

        # verbose
        self.model.verbose = self.args.verbose
        loggers = []
        if self.args.verbose > 1:
            loggers.append("stdout")

        # save additional logs
        if self.args.is_save_drl_log or self.args.is_save_sim_log:
            os.makedirs(self.log_path, exist_ok=True)
            if self.args.is_save_drl_log and self.args.mode == 'train':
                loggers += ["log", "csv", "tensorboard"]
                # save run params to a file in a log folder
                self._save_run_params()

        # set up the final logger
        self.logger = configure(self.log_path if self.args.is_save_drl_log else None, loggers)
        self.model.set_logger(self.logger)

        # set deterministic flag for evaluation
        self.deterministic = self.args.deterministic if self.args.mode == 'eval' else True

        # finish all setup steps...

    def learn(self) -> None:
        """train the model"""

        print(f"Training {self.args.model_name} model for {self.total_timesteps} steps...\n")
        self.model.learn(
            total_timesteps=self.total_timesteps,
            reset_num_timesteps=self.is_reset_timesteps,
            callback=self.callbacks,
        )

    def eval(self, eval_callback: GymirEvaluatingCallback | str | None = "default") -> None:
        """evaluate the model"""

        if eval_callback is not None:
            if isinstance(eval_callback, str) and eval_callback == "default":
                callback_step = GymirEvaluatingCallback(
                    self.env,
                    save_path=self.log_path,
                    model_name=self.args.model_name + "_eval" + ("_det" if self.deterministic else "_ndet"),
                    verbose=2 if self.args.is_save_drl_log else min(1, self.args.verbose),
                )
            elif isinstance(eval_callback, GymirEvaluatingCallback):
                callback_step = eval_callback
            else:
                raise Exception(f"Unknown eval callback {eval_callback}")
        else:
            callback_step = None

        print(
            f"Evaluating {self.args.model_name} model with det={self.deterministic} for"
            f" {self.total_timesteps} steps...\n"
        )
        episode_rewards, episode_lengths = evaluate_policy(
            self.model,
            self.env,
            n_eval_episodes=self.episodes,
            deterministic=self.deterministic,
            callback=lambda locals, globals: callback_step.on_step() if callback_step is not None else None,
            return_episode_rewards=True,
        )

        if self.args.is_save_drl_log:
            eval_path = os.path.join(
                self.log_path, f'{self.args.model_name}_eval_output_{time.strftime("%Y%m%d-%H%M%S")}.csv'
            )
            with open(eval_path, "w", newline="") as f:
                writer = csv.DictWriter(f, fieldnames=["episode_rewards", "episode_lengths"])
                writer.writeheader()
                writer.writerows(
                    [{"episode_rewards": i, "episode_lengths": j} for i, j in zip(episode_rewards, episode_lengths)]
                )

    def _set_save_paths(self, save_log_path: Optional[str], save_model_path: Optional[str]) -> None:
        timestamp = time.strftime("%Y%m%d-%H%M%S-%f")[:-3]
        # log
        if save_log_path is None:
            log_path = os.path.join(ROOT_DIR, f"logs/log_{timestamp}")
        else:
            log_path = save_log_path
        # model
        if save_model_path is None:
            model_path = os.path.join(ROOT_DIR, f"models/model_{timestamp}")
        else:
            model_path = save_model_path
        return log_path, model_path

    def _set_num_envs(self, num_envs: int, is_reduce_nenvs_by_cpus: bool) -> int:
        nenvs = 1 if num_envs <= 1 else min(num_envs, GymirDrlManager.MAX_ENVS)
        if nenvs > 1 and is_reduce_nenvs_by_cpus:
            expected_cpus = nenvs * 2  # for the current env fork and for the sim subproc
            num_cpus = multiprocessing.cpu_count()
            if num_cpus < expected_cpus:
                warnings.warn(
                    f"Warning: insufficient CPUs, expected at least {expected_cpus}, but found {num_cpus}."
                    f" Reduce the amount of envs from {nenvs} to {num_cpus // 2}"
                )
                return num_cpus // 2
            else:
                return nenvs
        else:
            return nenvs

    def _set_callbacks(self, callbacks: Union[None, List[BaseCallback], List[str]]) -> List[BaseCallback]:
        cbs: List[BaseCallback] = []
        if callbacks is not None:
            if isinstance(callbacks, list) and all(isinstance(cb, str) for cb in callbacks):
                for callback_name in callbacks:
                    match callback_name:
                        case "save_model":
                            os.makedirs(self.model_path, exist_ok=True)
                            cbs.append(GymirCheckpointCallback(save_path=self.model_path, verbose=self.args.verbose))
                        case "save_step":
                            os.makedirs(self.log_path, exist_ok=True)
                            cbs.append(
                                GymirSaveStepCallback(
                                    save_path=self.log_path, model_name=self.args.model_name, verbose=self.args.verbose
                                )
                            )
                        case "print_step":
                            cbs.append(GymirPrintStepCallback(verbose=self.args.verbose))
                        case "stop_no_impr":
                            os.makedirs(self.model_path, exist_ok=True)
                            os.makedirs(self.log_path, exist_ok=True)

                            # parse eval callback args
                            eval_sim_args, eval_cb_args = parse_eval_args(self.args.eval_cfg, self.args)

                            # create StopTrainingOnNoModelImprovement with its async EvalCallback parent
                            # it evaluates certain amount of steps/episodes only on runs/scenarios all selected in eval_cb_cfg
                            stop_cb = StopTrainingOnNoModelImprovement(
                                max_no_improvement_evals=eval_cb_args["max_no_improvement_evals"],
                                min_evals=eval_cb_args["min_evals"],
                                verbose=self.args.verbose,
                            )
                            eval_cb = GymirAsyncEvalCallback(
                                eval_sim_args,
                                n_eval_episodes=eval_sim_args.runs,
                                best_model_save_path=self.model_path,
                                log_path=self.log_path,
                                eval_freq=eval_cb_args["eval_freq"],
                                deterministic=eval_cb_args["deterministic"],
                                render=False,
                                callback_after_eval=stop_cb,
                            )
                            cbs.append(eval_cb)
                        case _:
                            raise Exception(f"GymirDrlManager: unknown callback {callback_name}")
            else:
                cbs = callbacks

            # some callbacks cant be used with multiple envs
            if self.num_envs > 1:
                for cb in cbs:
                    if isinstance(cb, GymirSaveStepCallback) or isinstance(cb, GymirPrintStepCallback):
                        cbs.remove(cb)

        # always append this callback (regulates finishing)
        cbs.append(GymirBreakCallback(verbose=self.args.verbose))

        return cbs

    def _make_sim_runner(self, port: int = None) -> GymirSimRunner:
        # creates GymirSimRunner object, optionally with a new port for multiple instances spawning
        sim_runner = GymirSimRunner(
            sim_path=self.args.sim_path,
            num_runs=self.args.runs,
            from_run=self.args.from_run,
            same_run=self.args.same_run,
            scenario=self.args.scenario,
            time_limit=self.args.time_limit,
            streams_config_file=self.args.streams_cfg,
            state_update_period=self.args.update_period,
            sim_host=self.args.sim_host,
            sim_port=self.args.sim_port if port is None else port,
            adaptive_algorithm=self.args.adaptive_algorithm,
            stream_log_dir=self._get_sim_log_prefix(port) if self.args.is_save_sim_log else "null",
            is_use_veins=self.args.is_use_veins,
            is_view=self.args.is_view,
            std_output=self._get_sim_log_prefix(port) + "omnetpp" if self.args.is_save_sim_log else None,
            busy_ports=self.busy_ports,
        )

        # save occupied port to exclude it for other runners in case of multiple envs
        if self.num_envs > 1:
            self.busy_ports.append(sim_runner.sim_port)

        return sim_runner

    def _make_env(self, sim_runner: GymirSimRunner) -> Callable[[], gymnasium.Env]:
        def _env_spawner() -> gymnasium.Env:
            env = GymirEnv(mdp_config=self.args.env_cfg, runner=sim_runner, max_episodes=self.episodes)
            return env

        return _env_spawner

    def _get_sim_log_prefix(self, port: int = None) -> str:
        if port is None:
            return os.path.abspath(self.log_path) + "/"
        else:
            return os.path.abspath(self.log_path) + "/" + str(port) + "_"

    def _save_run_params(self) -> None:
        print("Saving run params to a text file...")
        os.makedirs(self.log_path, exist_ok=True)
        with open(os.path.join(self.log_path, "run_params.txt"), "w+") as file:
            file.write(f"sim-path: {self.args.sim_path}\n")
            file.write(f"sim-address-first: {self.args.sim_host}:{self.args.sim_port}\n")
            file.write(f"runs: {self.args.runs}\n")
            file.write(f"from-run: {self.args.from_run}\n")
            file.write(f"same-run: {self.args.same_run}\n")
            file.write(f"scenario: {self.args.scenario}\n")
            file.write(f"streams-cfg: {self.args.streams_cfg}\n")
            file.write(f"time-limit: {self.args.time_limit}\n")
            file.write(f"update-period: {self.args.update_period}\n")
            file.write(f"adaptive-algorithm: {self.args.adaptive_algorithm}\n")
            file.write(f"mode: {self.args.mode}\n")
            file.write(f"env-cfg: {self.args.env_cfg}\n")
            file.write(f"num-envs: {self.args.num_envs}\n")
            file.write(f"model-name: {self.args.model_name}\n")
            file.write(f"model-hyperparams: {self.args.hyperparams_cfg}\n")
            file.write(f"model-file: {self.args.model_file}\n")
            file.write(f"is-use-veins: {self.args.is_use_veins}\n")
            file.write(f"model-dir: {os.path.abspath(self.model_path)}\n")
            file.write(f"log-dir: {os.path.abspath(self.log_path)}\n")
            file.write(f"is-view: {self.args.is_view}\n")
            file.write("-----------------------------------------------------")
            train_cmd = "python " + " ".join(sys.argv)
            file.write(f"\nTraining sim-cmd: \n\t{train_cmd}")
            train_cmd_splitted = train_cmd.split(" ")
            if '-m' in train_cmd_splitted:
                train_cmd_splitted[train_cmd_splitted.index("-m") + 1] = "eval"
            else:
                train_cmd_splitted.append('-m')
                train_cmd_splitted.append('eval')
            if '-mf' in train_cmd_splitted:
                mf_index = train_cmd_splitted.index("-mf")
                train_cmd_splitted.pop(mf_index)
                train_cmd_splitted.pop(mf_index + 1)
            train_cmd_splitted = [arg for arg in train_cmd_splitted if not arg.startswith("--save")]
            eval_cmd = " ".join(train_cmd_splitted)
            file.write(
                "\nEvaluation sim-cmd: (add model file with -mf <filepath> and optionally enable eval reward"
                f" collection with --save-model-log): \n\t{eval_cmd}\n"
            )
            file.write("-----------------------------------------------------")
