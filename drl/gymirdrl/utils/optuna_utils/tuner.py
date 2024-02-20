import numpy as np
import os
import pickle as pkl
import time
import traceback
from typing import Optional

import optuna
from optuna.pruners import MedianPruner
from optuna.samplers import TPESampler

from gymirdrl.core.args import GymirArgs
from gymirdrl.core.callbacks import GymirAsyncTrialEvalCallback
from gymirdrl.core.drl_manager import GymirDrlManager
from gymirdrl.core.sim_runner import GymirSimRunner
from gymirdrl.utils.parse_utils import parse_eval_args
from gymirdrl.utils.optuna_utils import GymirHyperParamSampler


class GymirTuner:
    def __init__(
        self,
        args: GymirArgs,
        direction: str = "maximize",
        aggregation_type: str = "average",
        sampler: Optional[optuna.samplers.BaseSampler] = None,
        pruner: Optional[optuna.pruners.BasePruner] = None,
        study_name: str = "",
        save_path: str = None,
    ):
        self.args = args
        self.direction = direction
        self.aggregation_type = aggregation_type
        if self.aggregation_type == "average":
            self.aggregation_fn = np.average
        elif self.aggregation_type == "median":
            self.aggregation_fn = np.median
        elif self.aggregation_type == "max":
            self.aggregation_fn = np.max
        elif self.aggregation_type == "min":
            self.aggregation_fn = np.min
        else:
            raise ValueError(f"Unknown aggregation type {self.aggregation_type}")
        self.pruner = pruner if pruner is not None else MedianPruner()
        self.sampler = sampler if sampler is not None else TPESampler()
        self.study_name = study_name if study_name.strip() else f"study_{int(time.time())}"
        self.save_path = save_path
        if self.save_path is not None:
            os.makedirs(self.save_path)

        self.trial_ports = []

    def tune(self, n_trials: int, n_jobs: int = -1) -> None:
        # set a unique port for every trial to utilize num_jobs > 1
        if n_jobs == -1 or n_jobs > 1:
            port = self.args.sim_port
            while len(self.trial_ports) < n_trials:
                if GymirSimRunner.check_if_port_is_free(port):
                    self.trial_ports.append(port)
                port += 1
        else:
            self.trial_ports = [self.args.sim_port] * n_trials

        # complete objective
        def _objective(trial: optuna.Trial):
            # define eval callback
            eval_sim_args, eval_cb_args = parse_eval_args(self.args.eval_cfg, self.args)
            eval_callback = GymirAsyncTrialEvalCallback(
                eval_sim_args,
                trial,
                n_eval_episodes=eval_sim_args.runs,
                eval_freq=eval_cb_args["eval_freq"],
                deterministic=eval_cb_args["deterministic"],
            )

            # sample hyperparams
            hyperparams_dict = GymirHyperParamSampler.create_sampler(self.args.model_name).sample_hyperparams(trial)
            self.args.hyperparams_cfg = hyperparams_dict

            # define drl manager
            self.args.sim_port = self.trial_ports[trial.number]
            drl_manager = GymirDrlManager(args=self.args, callbacks=[eval_callback])

            # learn
            try:
                drl_manager.learn()
            except (AssertionError, ValueError):
                drl_manager.env.close()
                eval_callback.eval_env.close()
                print(traceback.print_exc())
                raise optuna.exceptions.TrialPruned()

            # prune
            if eval_callback.is_pruned:
                raise optuna.exceptions.TrialPruned()

            return eval_callback.last_mean_reward

        # define study
        study = optuna.create_study(
            study_name=self.study_name,
            direction=self.direction,
            load_if_exists=True,
            pruner=self.pruner,
            sampler=self.sampler,
        )

        # main loop
        try:
            study.optimize(_objective, n_trials=n_trials, n_jobs=n_jobs)
        except KeyboardInterrupt:
            pass

        # finish
        if self.save_path:
            with open(f"{self.save_path}/{self.study_name}.pkl", "wb+") as f:
                pkl.dump(study, f)

        # print
        if self.args.verbose >= 1:
            print(
                f"Number of finished trials: {len(study.trials)}\n"
                "Best trial:\n"
                f"  Value: {study.best_trial.value}\n"
                "  Params:\n"
                f"    {chr(10).join(f'{key}: {value}' for key, value in study.best_trial.params.items())}\n"
                "  User attrs:\n"
                f"    {chr(10).join(f'{key}: {value}' for key, value in study.best_trial.user_attrs.items())}"
            )

        return study.best_trial
