import math
import numpy as np
from typing import Dict, Any, Tuple
from gymirdrl.core.observation import StateGenerator


def get_reward_function_class(reward_function_name: str = "best"):
    reward_function = REWARD_FUNCTIONS_DICT[reward_function_name]
    if not reward_function:
        raise Exception(f"Reward name {reward_function_name} is not registered")
    else:
        return reward_function


def arcs(
    state_gen: StateGenerator,
    last_action: int | np.ndarray[int | float],
) -> Tuple[float, Dict[str, float]]:

    # 1. rate: take logarithmic idea from fang, rate of r / r_max from 0 to 1
    reward_rate = np.log((np.exp(1) - 1) * (state_gen.rxGoodput / state_gen.MAX_RATE) + 1)

    # 2. rtt: modify idea from Acm21 Challenge QoE, namely take running min, max and average
    reward_rtt = (
        (state_gen.aux_vars["maxRtt"] - state_gen.fractionRtt)
        / (state_gen.aux_vars["maxRtt"] - state_gen.aux_vars["minRtt"])
        if state_gen.aux_vars["maxRtt"] - state_gen.aux_vars["minRtt"] > 0
        else 0.0
    )

    # 3. plr, here just a classic intuition
    reward_plr = 1 - state_gen.fractionLossRate

    # 4. jitter: Wahab et al: Direct propagation of network QoS distribution... (2020), make reward jitter 0...1
    reward_jitter = (
        -0.2 * math.sqrt(StateGenerator.clip(state_gen.interarrivalJitter, 0.0, state_gen.MAX_JITTER / 16.0)) + 1
    )

    # 5. smooth: take rate of change, ideally hold it within 10% max, because in that case no reencoding happens
    prev_goodput = state_gen.last_obs.transmission.rxGoodput if state_gen.is_last_obs else 0.0
    rate_of_change = abs(state_gen.rxGoodput - prev_goodput) / state_gen.MAX_RATE
    reward_smooth = 1 if rate_of_change <= 0.1 else 1 - rate_of_change

    # coefficients
    a = 0.3
    b = 0.2
    c = 0.3
    d = 0.15
    e = 0.05
    reward = a * reward_rate + b * reward_rtt + c * reward_plr + d * reward_jitter + e * reward_smooth
    return reward, dict(
        zip(
            ["rew", "rate", "rtt", "plr", "jit", "smt"],
            [reward, a * reward_rate, b * reward_rtt, c * reward_plr, d * reward_jitter, e * reward_smooth],
        )
    )

REWARD_FUNCTIONS_DICT = {
    "arcs": arcs,
}
