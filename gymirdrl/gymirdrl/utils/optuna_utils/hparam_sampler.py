from abc import ABC, abstractmethod
from typing import Any, Dict, Self

import optuna

from gymirdrl.utils.drl_utils import ACTIVATION_FUNCTIONS, make_linear_schedule


class GymirHyperParamSampler(ABC):
    """
    Abstract base class for hyperparameter samplers
    """

    @abstractmethod
    def sample_hyperparams(self, trial: optuna.Trial) -> Dict[str, Any]:
        pass

    @classmethod
    def create_sampler(cls, model_name: str) -> Self:
        """
        Creates a hyperparameter sampler for the provided DRL model

        :param model_name: The name of the DRL model for which to create the sampler
        :return: An instance of a hyperparameter sampler
        :rtype: HyperParamSampler
        :raises ValueError: If the provided model name is not recognized
        """
        if model_name.lower() == "ppo":
            return PPOSampler()

        # ... other algos
        raise ValueError(f"Unknown algorithm: {model_name}")


class PPOSampler(GymirHyperParamSampler):
    def sample_hyperparams(self, trial: optuna.Trial) -> Dict[str, Any]:
        batch_size = trial.suggest_categorical("batch_size", [8, 16, 32, 64, 128, 256, 512, 1024])
        n_steps = trial.suggest_categorical("n_steps", [8, 16, 32, 64, 128, 256, 512, 1024, 2048])
        gamma = trial.suggest_categorical("gamma", [0.5, 0.8, 0.9, 0.95, 0.98, 0.99, 0.995, 0.999, 0.9999])
        learning_rate = trial.suggest_float("learning_rate", 1e-5, 1, log=True)
        lr_schedule = trial.suggest_categorical('lr_schedule', ['linear', 'constant'])
        ent_coef = trial.suggest_float("ent_coef", 0.00000001, 0.1, log=True)
        clip_range = trial.suggest_categorical("clip_range", [0.1, 0.2, 0.3, 0.4])
        n_epochs = trial.suggest_categorical("n_epochs", [1, 5, 10, 20])
        gae_lambda = trial.suggest_categorical("gae_lambda", [0.8, 0.9, 0.92, 0.95, 0.98, 0.99, 1.0])
        max_grad_norm = trial.suggest_categorical("max_grad_norm", [0.3, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 2, 5])
        vf_coef = trial.suggest_float("vf_coef", 0, 1)

        ## [CONTINOUS ACTIONS] gSDE could be enabled
        # log_std_init = trial.suggest_uniform("log_std_init", -4, 1)
        # sde_sample_freq = trial.suggest_categorical("sde_sample_freq", [-1, 8, 16, 32, 64, 128, 256])
        ##

        ortho_init = trial.suggest_categorical('ortho_init', [False, True])
        activation_fn = trial.suggest_categorical('activation_fn', ['tanh', 'relu', 'elu', 'leaky_relu'])

        # TODO: multiple envs?
        if batch_size > n_steps:
            batch_size = n_steps

        if lr_schedule == "linear":
            learning_rate = make_linear_schedule(learning_rate)

        net_arch_width = trial.suggest_categorical("net_arch_width", [8, 16, 32, 64, 128, 256, 512])
        net_arch_depth = trial.suggest_int("net_arch_depth", 1, 3)
        net_arch = dict(pi=[net_arch_width] * net_arch_depth, vf=[net_arch_width] * net_arch_depth)

        activation_fn = ACTIVATION_FUNCTIONS[activation_fn]

        return {
            "n_steps": n_steps,
            "batch_size": batch_size,
            "gamma": gamma,
            "learning_rate": learning_rate,
            "ent_coef": ent_coef,
            "clip_range": clip_range,
            "n_epochs": n_epochs,
            "gae_lambda": gae_lambda,
            "max_grad_norm": max_grad_norm,
            "vf_coef": vf_coef,
            # "sde_sample_freq": sde_sample_freq,
            "policy_kwargs": dict(
                # log_std_init=log_std_init,
                net_arch=net_arch,
                activation_fn=activation_fn,
                ortho_init=ortho_init,
            ),
        }
