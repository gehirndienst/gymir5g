import torch
from typing import Callable, List, Union

from stable_baselines3 import PPO, A2C, DDPG, TD3, SAC, DQN
from sb3_contrib import QRDQN, RecurrentPPO

from stable_baselines3.common.base_class import BaseAlgorithm
from stable_baselines3.common.save_util import recursive_getattr, recursive_setattr

SB3_MODEL_CLASSES = {
    'ppo': PPO,
    'ppo_lstm': RecurrentPPO,
    'a2c': A2C,
    'ddpg': DDPG,
    'td3': TD3,
    'sac': SAC,
    'qrdqn': QRDQN,
    'dqn': DQN,
}

ACTIVATION_FUNCTIONS = {
    'tanh': torch.nn.Tanh,
    'relu': torch.nn.ReLU,
    'leaky_relu': torch.nn.LeakyReLU,
    'elu': torch.nn.ELU,
    'gelu': torch.nn.GELU,
    'glu': torch.nn.GLU,
}


def make_linear_schedule(initial_value: Union[float, str]) -> Callable[[float], float]:
    """
    Makes a linear schedule function starting with initial value

    :param initial_value: Initial value
    :return: schedule function
    """
    if isinstance(initial_value, str):
        initial_value = float(initial_value)

    def _schedule(progress: float) -> float:
        return progress * initial_value

    return _schedule


def aggregate_models(models: List[BaseAlgorithm], weights: List[float] = None):
    """
    Aggregates several SB3 models

    :param models: List of SB3 Models
    :param models: List of aggregation weights (nullable)
    :return: aggregated model
    """

    if len(models) == 0:
        raise ValueError("aggregate_models: no models were given")

    # check if all models have the same architecture and parameters
    if not all([type(model) == type(models[0]) for model in models]):
        raise ValueError("aggregate_models: all models must have the same architecture and parameters")

    aggregated_model = models[0].__class__()

    # if weights are not provided, use equal weights for all models
    if weights is None:
        weights = [1.0] * len(models)
    else:
        # check if the number of weights matches the number of models
        if len(weights) != len(models):
            raise ValueError("Number of weights must match the number of models.")

    # normalize weights to ensure they sum up to 1.0
    normalized_weights = [w / sum(weights) for w in weights]

    # Get the model parameters and perform weighted aggregation
    for attr_name in models[0].__dict__:
        if attr_name in [
            "policy",
            "device",
            "_device",
            "policy",
            "replay_buffer",
            "rollout_buffer",
            "_vec_normalize_env",
            "_episode_storage_logger",
            "_custom_logger",
        ]:
            continue

        # recursively aggregate
        attr_values = [recursive_getattr(model, attr_name) for model in models]
        aggregated_values = sum([w * value for w, value in zip(normalized_weights, attr_values)])
        recursive_setattr(aggregated_model, attr_name, aggregated_values)

    return aggregated_model
