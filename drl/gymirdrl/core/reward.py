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


# 1. the idea is to try to learn only from global values
def globRewardv1(sim_observation, sim_constants=None, action=None):
    # three sub-parts
    reward = 0
    reward_gpt = 0
    reward_rtt = 0
    reward_plr = 0

    ### 1. GOODPUT REWARD
    rx_goodput_global = sim_observation["rxGoodputGlobAv"]
    chunk_variance = sim_observation["chunkVar"]

    # compensate I/P frame sizes difference
    if 0.5 <= chunk_variance < 0.75:
        key_frame_comprensation = 0.99
    elif chunk_variance >= 0.75:
        key_frame_comprensation = 0.98
    else:
        key_frame_comprensation = 1.0

    reward_gpt = 1 * rx_goodput_global
    reward_gpt *= key_frame_comprensation

    # 2. RTT REWARD
    jitter_period = sim_observation["rttJitterPerAv"]
    rtt_global = sim_observation["rttGlobAv"]

    if rtt_global > 0:
        reward_rtt = 0.05 * (1 / rtt_global)
    else:
        reward_rtt = 0

    # also look if jitter is too big then punish extra for big jumps
    if jitter_period >= 0.25:
        reward_rtt *= 0.95

    # 3. PLR REWARD
    plr_global = sim_observation["plrGlob"]

    # same as with rtt, for  0--0.5
    reward_plr = 1 - plr_global

    # Final reward: [-1, 1]
    reward = reward_gpt + reward_rtt + reward_plr

    # special case: if stream is turned off (isRunning = -1), penalize extra by -0.5
    if sim_observation["isRunning"] < 0:
        reward_gpt -= 0.5
        reward -= 0.5

    # return result
    return reward, dict(zip(["rew", "gpt", "rtt", "plr"], [reward, reward_gpt, reward_rtt, reward_plr]))


# 2. a combination of periodic and global
def globPerRewardv1(sim_observation, sim_constants=None, action=None):
    # three sub-parts
    reward = 0
    reward_gpt = 0
    reward_rtt = 0
    reward_plr = 0

    ### 1. GOODPUT REWARD
    tx_goodput_period = sim_observation["txGoodputPerAv"]
    tx_goodput_global = sim_observation["txGoodputGlobAv"]
    rx_goodput_period = sim_observation["rxGoodputPerAv"]
    rx_goodput_global = sim_observation["rxGoodputGlobAv"]
    tx_goodput_period_to_global = min(tx_goodput_period / tx_goodput_global, 1) if tx_goodput_global > 0 else 0
    rx_goodput_period_to_global = min(rx_goodput_period / rx_goodput_global, 1) if rx_goodput_global > 0 else 0
    chunk_variance = sim_observation["chunkVar"]

    # compensate I/P frame sizes difference
    if 0.5 <= chunk_variance < 0.75:
        tx_goodput_period_to_global = tx_goodput_period_to_global * 0.875
    elif chunk_variance >= 0.75:
        tx_goodput_period_to_global = tx_goodput_period_to_global * 0.75
    else:
        tx_goodput_period_to_global = tx_goodput_period_to_global

    # relation-based discrete function:
    # - how good do we try to send as max as possible?
    # - how the result is influenced?
    reward_gpt = 0.5 * tx_goodput_period_to_global + 0.5 * rx_goodput_period_to_global

    # 2. RTT REWARD
    rtt_period = sim_observation["rttPerAv"]
    jitter_period = sim_observation["rttJitterPerAv"]

    # here is a simple constant rule-based reward based on the "expert knowledge": good 5G rtt is known from reality
    # < 100 ms is considered ok, 100-500 bad but ok, > 500-1000 0 reward, > 1000 punishment
    # so far we don't take into account any relations (like for gpt), because a strategy is based to min this at any cost
    if 0 < rtt_period <= 0.1:
        reward_rtt = max(1 - rtt_period * 10, 0.5)  # 0.5 .. 1
    elif 0.1 < rtt_period <= 0.5:
        reward_rtt = 0.6 - rtt_period  # 0.1 .. 0.5
    elif 0.5 < rtt_period <= 1.0 or rtt_period == 0:
        reward_rtt = 0
    else:
        reward_rtt = -1

    # also look if jitter is too big then punish for inconsistency
    if jitter_period >= 0.2 and reward_rtt > 0:
        reward_rtt = reward_rtt * 0.875

    # 3. PLR REWARD
    plr_period = sim_observation["plrPer"]
    rx_packets = sim_observation["rxPacketsPer"]
    net_instabil_rate = sim_observation["netInstabilRate"]

    # here is just a linear function
    # FIXME: maybe change to a quadratic?
    reward_plr = 1 - 2 * plr_period  # -1 .. 1

    # soften punishment because of net instability
    if net_instabil_rate > 0.35 and plr_period > 0.5:
        reward_plr = reward_plr * (1 - net_instabil_rate)

    # if there are too few packets then refuse to estimate
    if rx_packets <= 5:
        reward_plr = 0

    # FINAL WEIGHTS: Final is reward in [-1, 1]
    reward_gpt = reward_gpt * 0.2
    reward_rtt = reward_rtt * 0.4
    reward_plr = reward_plr * 0.4
    reward = reward_gpt + reward_rtt + reward_plr

    # one special case: if stream is turned off (isRunning = -1), give -0.1
    if sim_observation["isRunning"] < 0:
        reward_gpt = 0
        reward_rtt = 0
        reward_plr = 0
        reward = -0.1

    return reward, dict(zip(["rew", "gpt", "rtt", "plr"], [reward, reward_gpt, reward_rtt, reward_plr]))


def globPerRewardv1a(sim_observation, sim_constants=None, action=None):
    # three sub-parts
    reward = 0
    reward_gpt = 0
    reward_rtt = 0
    reward_plr = 0

    ### 1. GOODPUT REWARD
    tx_goodput_period = sim_observation["txGoodputPerAv"]
    tx_goodput_global = sim_observation["txGoodputGlobAv"]
    rx_goodput_period = sim_observation["rxGoodputPerAv"]
    rx_goodput_global = sim_observation["rxGoodputGlobAv"]
    tx_goodput_period_to_global = min(tx_goodput_period / tx_goodput_global, 1) if tx_goodput_global > 0 else 0
    rx_goodput_period_to_global = min(rx_goodput_period / rx_goodput_global, 1) if rx_goodput_global > 0 else 0
    chunk_variance = sim_observation["chunkVar"]

    # compensate I/P frame sizes difference
    if 0.5 <= chunk_variance < 0.75:
        tx_goodput_period_to_global = tx_goodput_period_to_global * 0.875
    elif chunk_variance >= 0.75:
        tx_goodput_period_to_global = tx_goodput_period_to_global * 0.75
    else:
        tx_goodput_period_to_global = tx_goodput_period_to_global

    # relation-based discrete function:
    # - how good do we try to send as max as possible?
    # - how the result is influenced?
    reward_gpt = 0.5 * tx_goodput_period_to_global + 0.5 * rx_goodput_period_to_global

    # 2. RTT REWARD
    rtt_period = sim_observation["rttPerAv"]
    jitter_period = sim_observation["rttJitterPerAv"]

    # here is a simple constant rule-based reward based on the "expert knowledge": good 5G rtt is known from reality
    # < 100 ms is considered ok, 100-500 bad but ok, > 500-1000 0 reward, > 1000 punishment
    # so far we don't take into account any relations (like for gpt), because a strategy is based to min this at any cost
    if 0 < rtt_period <= 0.1:
        reward_rtt = max(1 - rtt_period * 10, 0.5)  # 0.5 .. 1
    elif 0.1 < rtt_period <= 0.5:
        reward_rtt = 0.6 - rtt_period  # 0.1 .. 0.5
    elif 0.5 < rtt_period <= 1.0 or rtt_period == 0:
        reward_rtt = 0
    else:
        reward_rtt = -1

    # also look if jitter is too big then punish for inconsistency
    if jitter_period >= 0.2 and reward_rtt > 0:
        reward_rtt = reward_rtt * 0.875

    # 3. PLR REWARD
    plr_period = sim_observation["plrPer"]
    rx_packets = sim_observation["rxPacketsPer"]
    net_instabil_rate = sim_observation["netInstabilRate"]

    # here is just a linear function
    # FIXME: maybe change to a quadratic?
    reward_plr = 1 - 2 * plr_period  # -1 .. 1

    # soften punishment because of net instability
    if net_instabil_rate > 0.35 and plr_period > 0.5:
        reward_plr = reward_plr * (1 - net_instabil_rate)

    # if there are too few packets then refuse to estimate
    if rx_packets <= 5:
        reward_plr = 0

    # FINAL WEIGHTS: Final is reward in [-1, 1]
    # the only change here is to change weights a bit
    reward_gpt = reward_gpt * 0.3
    reward_rtt = reward_rtt * 0.35
    reward_plr = reward_plr * 0.35
    reward = reward_gpt + reward_rtt + reward_plr

    # one special case: if stream is turned off (isRunning = -1), give -0.1
    if sim_observation["isRunning"] < 0:
        reward_gpt = 0
        reward_rtt = 0
        reward_plr = 0
        reward = -0.1

    return reward, dict(zip(["rew", "gpt", "rtt", "plr"], [reward, reward_gpt, reward_rtt, reward_plr]))


def globPerRewardv1b(sim_observation, sim_constants=None, action=None):
    # three sub-parts
    reward = 0
    reward_gpt = 0
    reward_rtt = 0
    reward_plr = 0

    ### 1. GOODPUT REWARD
    tx_goodput_period = sim_observation["txGoodputPerAv"]
    tx_goodput_global = sim_observation["txGoodputGlobAv"]
    rx_goodput_period = sim_observation["rxGoodputPerAv"]
    rx_goodput_global = sim_observation["rxGoodputGlobAv"]
    tx_goodput_period_to_global = min(tx_goodput_period / tx_goodput_global, 1) if tx_goodput_global > 0 else 0
    rx_goodput_period_to_global = min(rx_goodput_period / rx_goodput_global, 1) if rx_goodput_global > 0 else 0
    chunk_variance = sim_observation["chunkVar"]

    # compensate I/P frame sizes difference
    if 0.5 <= chunk_variance < 0.75:
        tx_goodput_period_to_global = tx_goodput_period_to_global * 0.875
    elif chunk_variance >= 0.75:
        tx_goodput_period_to_global = tx_goodput_period_to_global * 0.75
    else:
        tx_goodput_period_to_global = tx_goodput_period_to_global

    # relation-based discrete function:
    # - how good do we try to send as max as possible?
    # - how the result is influenced?
    reward_gpt = 0.5 * tx_goodput_period_to_global + 0.5 * rx_goodput_period_to_global

    # 2. RTT REWARD
    rtt_period = sim_observation["rttPerAv"]
    jitter_period = sim_observation["rttJitterPerAv"]

    # here is a simple constant rule-based reward based on the "expert knowledge": good 5G rtt is known from reality
    # < 100 ms is considered ok, 100-500 bad but ok, > 500-1000 0 reward, > 1000 punishment
    # so far we don't take into account any relations (like for gpt), because a strategy is based to min this at any cost
    if 0 < rtt_period <= 0.1:
        reward_rtt = max(1 - rtt_period * 10, 0.5)  # 0.5 .. 1
    elif 0.1 < rtt_period <= 0.5:
        reward_rtt = 0.6 - rtt_period  # 0.1 .. 0.5
    elif 0.5 < rtt_period <= 1.0 or rtt_period == 0:
        reward_rtt = 0
    else:
        reward_rtt = -1

    # also look if jitter is too big then punish for inconsistency
    if jitter_period >= 0.2 and reward_rtt > 0:
        reward_rtt = reward_rtt * 0.875

    # 3. PLR REWARD
    plr_period = sim_observation["plrPer"]
    rx_packets = sim_observation["rxPacketsPer"]
    net_instabil_rate = sim_observation["netInstabilRate"]

    # here is just a linear function
    # FIXME: maybe change to a quadratic?
    reward_plr = 1 - 2 * plr_period  # -1 .. 1

    # soften punishment because of net instability
    if net_instabil_rate > 0.35 and plr_period > 0.5:
        reward_plr = reward_plr * (1 - net_instabil_rate)

    # if there are too few packets then refuse to estimate
    if rx_packets <= 5:
        reward_plr = 0

    # FINAL WEIGHTS: Final is reward in [-1, 1]
    # the only change here is to change weights a bit
    reward_gpt = reward_gpt * 0.25
    reward_rtt = reward_rtt * 0.4
    reward_plr = reward_plr * 0.35
    reward = reward_gpt + reward_rtt + reward_plr

    # one special case: if stream is turned off (isRunning = -1), give -0.1
    if sim_observation["isRunning"] < 0:
        reward_gpt = 0
        reward_rtt = 0
        reward_plr = 0
        reward = -0.1

    return reward, dict(zip(["rew", "gpt", "rtt", "plr"], [reward, reward_gpt, reward_rtt, reward_plr]))


# more pressure on plr -- improving v1a
def globPerRewardv1c(sim_observation, sim_constants=None, action=None):
    # three sub-parts
    reward = 0
    reward_gpt = 0
    reward_rtt = 0
    reward_plr = 0

    ### 1. GOODPUT REWARD
    tx_goodput_period = sim_observation["txGoodputPerAv"]
    tx_goodput_global = sim_observation["txGoodputGlobAv"]
    rx_goodput_period = sim_observation["rxGoodputPerAv"]
    rx_goodput_global = sim_observation["rxGoodputGlobAv"]
    tx_goodput_period_to_global = min(tx_goodput_period / tx_goodput_global, 1) if tx_goodput_global > 0 else 0
    rx_goodput_period_to_global = min(rx_goodput_period / rx_goodput_global, 1) if rx_goodput_global > 0 else 0
    chunk_variance = sim_observation["chunkVar"]

    # compensate I/P frame sizes difference
    if 0.5 <= chunk_variance < 0.75:
        tx_goodput_period_to_global = tx_goodput_period_to_global * 0.875
    elif chunk_variance >= 0.75:
        tx_goodput_period_to_global = tx_goodput_period_to_global * 0.75
    else:
        tx_goodput_period_to_global = tx_goodput_period_to_global

    # relation-based discrete function:
    # - how good do we try to send as max as possible?
    # - how the result is influenced?
    reward_gpt = 0.5 * tx_goodput_period_to_global + 0.5 * rx_goodput_period_to_global

    # 2. RTT REWARD
    rtt_period = sim_observation["rttPerAv"]
    jitter_period = sim_observation["rttJitterPerAv"]

    # here is a simple constant rule-based reward based on the "expert knowledge": good 5G rtt is known from reality
    # < 100 ms is considered ok, 100-500 bad but ok, > 500-1000 0 reward, > 1000 punishment
    # so far we don't take into account any relations (like for gpt), because a strategy is based to min this at any cost
    if 0 < rtt_period <= 0.1:
        reward_rtt = max(1 - rtt_period * 10, 0.5)  # 0.5 .. 1
    elif 0.1 < rtt_period <= 0.5:
        reward_rtt = 0.6 - rtt_period  # 0.1 .. 0.5
    elif 0.5 < rtt_period <= 1.0 or rtt_period == 0:
        reward_rtt = 0
    else:
        reward_rtt = -1

    # also look if jitter is too big then punish for inconsistency
    if jitter_period >= 0.2 and reward_rtt > 0:
        reward_rtt = reward_rtt * 0.875

    # 3. PLR REWARD
    plr_period = sim_observation["plrPer"]
    rx_packets = sim_observation["rxPacketsPer"]
    net_instabil_rate = sim_observation["netInstabilRate"]

    # here is just a linear function
    # FIXME: maybe change to a quadratic?
    reward_plr = 1 - 2 * plr_period  # -1 .. 1

    # if there are too few packets then refuse to estimate
    if rx_packets <= 2:
        reward_plr = 0

    # FINAL WEIGHTS: Final is reward in [-1, 1]
    # the only change here is to change weights a bit
    reward_gpt = reward_gpt * 0.3
    reward_rtt = reward_rtt * 0.3
    reward_plr = reward_plr * 0.4
    reward = reward_gpt + reward_rtt + reward_plr

    # one special case: if stream is turned off (isRunning = -1), give -0.1
    if sim_observation["isRunning"] < 0:
        reward_gpt = 0
        reward_rtt = 0
        reward_plr = 0
        reward = -0.1

    return reward, dict(zip(["rew", "gpt", "rtt", "plr"], [reward, reward_gpt, reward_rtt, reward_plr]))


# 2. a combination of periodic and global
def globPerRewardv2(sim_observation, sim_constants=None, action=None):
    # three sub-parts
    reward = 0
    reward_gpt = 0
    reward_rtt = 0
    reward_plr = 0

    ### 1. GOODPUT REWARD
    rx_goodput_period = sim_observation["rxGoodputPerAv"]
    rx_goodput_global = sim_observation["rxGoodputGlobAv"]
    rx_goodput_period_to_global = min(rx_goodput_period / rx_goodput_global, 1) if rx_goodput_global > 0 else 0
    rx_goodput_max = sim_constants["maxGoodput"]
    rx_goodput_global_to_max = min(rx_goodput_global / rx_goodput_max, 1) if rx_goodput_global > 0 else 0
    chunk_variance = sim_observation["chunkVar"]

    # compensate I/P frame sizes difference
    if 0.5 <= chunk_variance < 0.75:
        rx_goodput_period_to_global *= 0.95
    elif chunk_variance >= 0.75:
        rx_goodput_period_to_global *= 0.9
    else:
        rx_goodput_period_to_global *= 1

    # relation-based discrete function:
    reward_gpt = 0.5 * rx_goodput_global_to_max + 0.5 * rx_goodput_period_to_global

    # 2. RTT REWARD
    rtt_period = sim_observation["rttPerAv"]
    jitter_period = sim_observation["rttJitterPerAv"]

    # here is a simple constant rule-based reward based on the "expert knowledge": good 5G rtt is known from reality
    # < 100 ms is considered ok, 100-500 bad but ok, > 500-1000 0 reward, > 1000 punishment
    # so far we don't take into account any relations (like for gpt), because a strategy is based to min this at any cost
    if 0 < rtt_period <= 0.1:
        reward_rtt = max(1 - rtt_period * 10, 0.5)  # 0.5 .. 1
    elif 0.1 < rtt_period <= 0.5:
        reward_rtt = 0.6 - rtt_period  # 0.1 .. 0.5
    elif 0.5 < rtt_period <= 1.0 or rtt_period == 0:
        reward_rtt = 0
    else:
        reward_rtt = -1

    # also look if jitter is too big then punish for inconsistency
    if jitter_period >= 0.2 and reward_rtt > 0:
        reward_rtt = reward_rtt * 0.875

    # 3. PLR REWARD
    plr_period = sim_observation["plrPer"]
    rx_packets = sim_observation["rxPacketsPer"]
    net_instabil_rate = sim_observation["netInstabilRate"]

    # here is just a linear function
    # FIXME: maybe change to a quadratic?
    reward_plr = 1 - 2 * plr_period  # -1 .. 1

    # if there are too few packets then refuse to estimate
    if rx_packets <= 2:
        reward_plr = 0

    # FINAL WEIGHTS: Final is reward in [-1, 1]
    reward_gpt = reward_gpt * 0.33
    reward_rtt = reward_rtt * 0.33
    reward_plr = reward_plr * 0.33
    reward = reward_gpt + reward_rtt + reward_plr

    # one special case: if stream is turned off (isRunning = -1), give -0.1
    if sim_observation["isRunning"] < 0:
        reward_gpt = -0.1
        reward_rtt = 0
        reward_plr = 0
        reward = -0.1

    return reward, dict(zip(["rew", "gpt", "rtt", "plr"], [reward, reward_gpt, reward_rtt, reward_plr]))


def globPerRewardv2a(sim_observation, sim_constants=None, action=None):
    # three sub-parts
    reward = 0
    reward_gpt = 0
    reward_rtt = 0
    reward_plr = 0

    ### 1. GOODPUT REWARD
    rx_goodput_period = sim_observation["rxGoodputPerAv"]
    rx_goodput_global = sim_observation["rxGoodputGlobAv"]
    rx_goodput_period_to_global = min(rx_goodput_period / rx_goodput_global, 1) if rx_goodput_global > 0 else 0
    rx_goodput_max = sim_constants["maxGoodput"]
    rx_goodput_global_to_max = min(rx_goodput_global / rx_goodput_max, 1) if rx_goodput_global > 0 else 0
    chunk_variance = sim_observation["chunkVar"]

    # compensate I/P frame sizes difference
    if 0.5 <= chunk_variance < 0.75:
        rx_goodput_period_to_global *= 0.95
    elif chunk_variance >= 0.75:
        rx_goodput_period_to_global *= 0.9
    else:
        rx_goodput_period_to_global *= 1

    # relation-based discrete function:
    reward_gpt = 0.5 * rx_goodput_global_to_max + 0.5 * rx_goodput_period_to_global

    # 2. RTT REWARD
    rtt_period = sim_observation["rttPerAv"]
    jitter_period = sim_observation["rttJitterPerAv"]

    # here is a simple constant rule-based reward based on the "expert knowledge": good 5G rtt is known from reality
    # < 100 ms is considered ok, 100-500 bad but ok, > 500-1000 0 reward, > 1000 punishment
    # so far we don't take into account any relations (like for gpt), because a strategy is based to min this at any cost
    if 0 < rtt_period <= 0.1:
        reward_rtt = max(1 - rtt_period * 10, 0.5)  # 0.5 .. 1
    elif 0.1 < rtt_period <= 0.5:
        reward_rtt = 0.6 - rtt_period  # 0.1 .. 0.5
    elif 0.5 < rtt_period <= 1.0 or rtt_period == 0:
        reward_rtt = 0
    else:
        reward_rtt = -1

    # also look if jitter is too big then punish for inconsistency
    if jitter_period >= 0.2 and reward_rtt > 0:
        reward_rtt = reward_rtt * 0.875

    # 3. PLR REWARD
    plr_period = sim_observation["plrPer"]
    rx_packets = sim_observation["rxPacketsPer"]
    net_instabil_rate = sim_observation["netInstabilRate"]

    # here is just a linear function
    # FIXME: maybe change to a quadratic?
    reward_plr = 1 - 2 * plr_period  # -1 .. 1

    # if there are too few packets then refuse to estimate
    if rx_packets <= 2:
        reward_plr = 0

    # FINAL WEIGHTS: Final is reward in [-1, 1]
    reward_gpt = reward_gpt * 0.35
    reward_rtt = reward_rtt * 0.35
    reward_plr = reward_plr * 0.3
    reward = reward_gpt + reward_rtt + reward_plr

    # one special case: if stream is turned off (isRunning = -1), give -0.1
    if sim_observation["isRunning"] < 0:
        reward_gpt = -0.1
        reward_rtt = 0
        reward_plr = 0
        reward = -0.1

    return reward, dict(zip(["rew", "gpt", "rtt", "plr"], [reward, reward_gpt, reward_rtt, reward_plr]))


def globPerRewardv2b(sim_observation, sim_constants=None, action=None):
    # three sub-parts
    reward = 0
    reward_gpt = 0
    reward_rtt = 0
    reward_plr = 0

    ### 1. GOODPUT REWARD
    rx_goodput_period = sim_observation["rxGoodputPerAv"]
    rx_goodput_global = sim_observation["rxGoodputGlobAv"]
    rx_goodput_period_to_global = min(rx_goodput_period / rx_goodput_global, 1) if rx_goodput_global > 0 else 0
    rx_goodput_max = sim_constants["maxGoodput"]
    rx_goodput_global_to_max = min(rx_goodput_global / rx_goodput_max, 1) if rx_goodput_global > 0 else 0
    chunk_variance = sim_observation["chunkVar"]

    # compensate I/P frame sizes difference
    if 0.5 <= chunk_variance < 0.75:
        rx_goodput_period_to_global *= 0.95
    elif chunk_variance >= 0.75:
        rx_goodput_period_to_global *= 0.9
    else:
        rx_goodput_period_to_global *= 1

    # relation-based discrete function:
    reward_gpt = 0.5 * rx_goodput_global_to_max + 0.5 * rx_goodput_period_to_global

    # 2. RTT REWARD
    rtt_period = sim_observation["rttPerAv"]
    jitter_period = sim_observation["rttJitterPerAv"]

    # here is a simple constant rule-based reward based on the "expert knowledge": good 5G rtt is known from reality
    # < 100 ms is considered ok, 100-500 bad but ok, > 500-1000 0 reward, > 1000 punishment
    # so far we don't take into account any relations (like for gpt), because a strategy is based to min this at any cost
    if 0 < rtt_period <= 0.1:
        reward_rtt = max(1 - rtt_period * 10, 0.5)  # 0.5 .. 1
    elif 0.1 < rtt_period <= 0.5:
        reward_rtt = 0.6 - rtt_period  # 0.1 .. 0.5
    elif 0.5 < rtt_period <= 1.0 or rtt_period == 0:
        reward_rtt = 0
    else:
        reward_rtt = -1

    # also look if jitter is too big then punish for inconsistency
    if jitter_period >= 0.2 and reward_rtt > 0:
        reward_rtt = reward_rtt * 0.875

    # 3. PLR REWARD
    plr_period = sim_observation["plrPer"]
    rx_packets = sim_observation["rxPacketsPer"]
    net_instabil_rate = sim_observation["netInstabilRate"]

    # here is just a linear function
    # FIXME: maybe change to a quadratic?
    reward_plr = 1 - 2 * plr_period  # -1 .. 1

    # if there are too few packets then refuse to estimate
    if rx_packets <= 2:
        reward_plr = 0

    # FINAL WEIGHTS: Final is reward in [-1, 1]
    reward_gpt = reward_gpt * 0.4
    reward_rtt = reward_rtt * 0.3
    reward_plr = reward_plr * 0.3
    reward = reward_gpt + reward_rtt + reward_plr

    # one special case: if stream is turned off (isRunning = -1), give -0.1
    if sim_observation["isRunning"] < 0:
        reward_gpt = -0.1
        reward_rtt = 0
        reward_plr = 0
        reward = -0.1

    return reward, dict(zip(["rew", "gpt", "rtt", "plr"], [reward, reward_gpt, reward_rtt, reward_plr]))


def globPerRewardv2c(sim_observation, sim_constants=None, action=None):
    # three sub-parts
    reward = 0
    reward_gpt = 0
    reward_rtt = 0
    reward_plr = 0

    ### 1. GOODPUT REWARD
    rx_goodput_period = sim_observation["rxGoodputPerAv"]
    rx_goodput_global = sim_observation["rxGoodputGlobAv"]
    rx_goodput_max = sim_constants["maxGoodput"]
    rx_goodput_period_to_max = min(rx_goodput_period / rx_goodput_max, 1)
    rx_goodput_global_to_max = min(rx_goodput_global / rx_goodput_max, 1)
    chunk_variance = sim_observation["chunkVar"]

    # compensate I/P frame sizes difference
    if 0.5 <= chunk_variance < 0.75:
        rx_goodput_period_to_max *= 0.95
    elif chunk_variance >= 0.75:
        rx_goodput_period_to_max *= 0.9
    else:
        rx_goodput_period_to_max *= 1

    # relation-based discrete function:
    # difference with v2b: change per/glob to per/max to praise tries to max tpt
    reward_gpt = 0.5 * rx_goodput_global_to_max + 0.5 * rx_goodput_period_to_max

    # 2. RTT REWARD
    rtt_period = sim_observation["rttPerAv"]
    jitter_period = sim_observation["rttJitterPerAv"]

    # here is a simple constant rule-based reward based on the "expert knowledge": good 5G rtt is known from reality
    # < 100 ms is considered ok, 100-500 bad but ok, > 500-1000 0 reward, > 1000 punishment
    # so far we don't take into account any relations (like for gpt), because a strategy is based to min this at any cost
    if 0 < rtt_period <= 0.1:
        reward_rtt = max(1 - rtt_period * 10, 0.5)  # 0.5 .. 1
    elif 0.1 < rtt_period <= 0.5:
        reward_rtt = 0.6 - rtt_period  # 0.1 .. 0.5
    elif 0.5 < rtt_period <= 1.0 or rtt_period == 0:
        reward_rtt = 0
    else:
        reward_rtt = -1

    # also look if jitter is too big then punish for inconsistency
    if jitter_period >= 0.2 and reward_rtt > 0:
        reward_rtt = reward_rtt * 0.875

    # 3. PLR REWARD
    plr_period = sim_observation["plrPer"]
    rx_packets = sim_observation["rxPacketsPer"]
    net_instabil_rate = sim_observation["netInstabilRate"]

    # here is just a linear function
    # FIXME: maybe change to a quadratic?
    reward_plr = 1 - 2 * plr_period  # -1 .. 1

    # if there are too few packets then refuse to estimate
    if rx_packets <= 2:
        reward_plr = 0

    # FINAL WEIGHTS: Final is reward in [-1, 1]
    reward_gpt = reward_gpt * 0.4
    reward_rtt = reward_rtt * 0.3
    reward_plr = reward_plr * 0.3
    reward = reward_gpt + reward_rtt + reward_plr

    # one special case: if stream is turned off (isRunning = -1), give -0.1
    if sim_observation["isRunning"] < 0:
        reward_gpt = -0.1
        reward_rtt = 0
        reward_plr = 0
        reward = -0.1

    return reward, dict(zip(["rew", "gpt", "rtt", "plr"], [reward, reward_gpt, reward_rtt, reward_plr]))


def globPerRewardv2d(sim_observation, sim_constants=None, action=None):
    # three sub-parts
    reward = 0
    reward_gpt = 0
    reward_rtt = 0
    reward_plr = 0

    ### 1. GOODPUT REWARD
    rx_goodput_period = sim_observation["rxGoodputPerAv"]
    rx_goodput_global = sim_observation["rxGoodputGlobAv"]
    rx_goodput_max = sim_constants["maxGoodput"]
    rx_goodput_period_to_max = min(rx_goodput_period / rx_goodput_max, 1)
    rx_goodput_global_to_max = min(rx_goodput_global / rx_goodput_max, 1)
    rx_goodput_period_to_global = min(rx_goodput_period / rx_goodput_global, 1) if rx_goodput_global > 0 else 0
    chunk_variance = sim_observation["chunkVar"]

    # compensate I/P frame sizes difference
    if 0.5 <= chunk_variance < 0.75:
        rx_goodput_period_to_max *= 0.95
    elif chunk_variance >= 0.75:
        rx_goodput_period_to_max *= 0.9
    else:
        rx_goodput_period_to_max *= 1

    # relation-based discrete function:
    # difference with v2b and v2c: add from both of them with equal 0.33 weights
    reward_gpt = 0.33 * rx_goodput_global_to_max + 0.33 * rx_goodput_period_to_global + 0.33 * rx_goodput_period_to_max

    # 2. RTT REWARD
    rtt_period = sim_observation["rttPerAv"]
    jitter_period = sim_observation["rttJitterPerAv"]

    # here is a simple constant rule-based reward based on the "expert knowledge": good 5G rtt is known from reality
    # < 100 ms is considered ok, 100-500 bad but ok, > 500-1000 0 reward, > 1000 punishment
    # so far we don't take into account any relations (like for gpt), because a strategy is based to min this at any cost
    if 0 < rtt_period <= 0.1:
        reward_rtt = max(1 - rtt_period * 10, 0.5)  # 0.5 .. 1
    elif 0.1 < rtt_period <= 0.5:
        reward_rtt = 0.6 - rtt_period  # 0.1 .. 0.5
    elif 0.5 < rtt_period <= 1.0 or rtt_period == 0:
        reward_rtt = 0
    else:
        reward_rtt = -1

    # also look if jitter is too big then punish for inconsistency
    if jitter_period >= 0.2 and reward_rtt > 0:
        reward_rtt = reward_rtt * 0.875

    # 3. PLR REWARD
    plr_period = sim_observation["plrPer"]
    rx_packets = sim_observation["rxPacketsPer"]
    net_instabil_rate = sim_observation["netInstabilRate"]

    # here is just a linear function
    reward_plr = 1 - 2 * plr_period  # -1 .. 1

    # if there are too few packets then refuse to estimate
    if rx_packets <= 2:
        reward_plr = 0

    # FINAL WEIGHTS: Final is reward in [-1, 1]
    reward_gpt = reward_gpt * 0.4
    reward_rtt = reward_rtt * 0.3
    reward_plr = reward_plr * 0.3
    reward = reward_gpt + reward_rtt + reward_plr

    # one special case: if stream is turned off (isRunning = -1), give -0.1
    if sim_observation["isRunning"] < 0:
        reward_gpt = -0.1
        reward_rtt = 0
        reward_plr = 0
        reward = -0.1

    return reward, dict(zip(["rew", "gpt", "rtt", "plr"], [reward, reward_gpt, reward_rtt, reward_plr]))


def globPerRewardv2e(sim_observation, sim_constants=None, action=None):
    # three sub-parts
    reward = 0
    reward_gpt = 0
    reward_rtt = 0
    reward_plr = 0

    ### 1. GOODPUT REWARD
    rx_goodput_period = sim_observation["rxGoodputPerAv"]
    rx_goodput_global = sim_observation["rxGoodputGlobAv"]
    rx_goodput_max = sim_constants["maxGoodput"]
    rx_goodput_period_to_max = min(rx_goodput_period / rx_goodput_max, 1)
    rx_goodput_global_to_max = min(rx_goodput_global / rx_goodput_max, 1)
    rx_goodput_period_to_global = min(rx_goodput_period / rx_goodput_global, 1) if rx_goodput_global > 0 else 0
    chunk_variance = sim_observation["chunkVar"]

    # compensate I/P frame sizes difference
    if 0.5 <= chunk_variance < 0.75:
        rx_goodput_period_to_max *= 0.95
    elif chunk_variance >= 0.75:
        rx_goodput_period_to_max *= 0.9
    else:
        rx_goodput_period_to_max *= 1

    # relation-based discrete function:
    # difference with v2d: instead of 0.33 give more to the two previous
    reward_gpt = 0.25 * rx_goodput_global_to_max + 0.25 * rx_goodput_period_to_global + 0.5 * rx_goodput_period_to_max

    # 2. RTT REWARD
    rtt_period = sim_observation["rttPerAv"]
    jitter_period = sim_observation["rttJitterPerAv"]

    # here is a simple constant rule-based reward based on the "expert knowledge": good 5G rtt is known from reality
    # < 100 ms is considered ok, 100-500 bad but ok, > 500-1000 0 reward, > 1000 punishment
    # so far we don't take into account any relations (like for gpt), because a strategy is based to min this at any cost
    if 0 < rtt_period <= 0.1:
        reward_rtt = max(1 - rtt_period * 10, 0.5)  # 0.5 .. 1
    elif 0.1 < rtt_period <= 0.5:
        reward_rtt = 0.6 - rtt_period  # 0.1 .. 0.5
    elif 0.5 < rtt_period <= 1.0:
        reward_rtt = 0
    else:
        reward_rtt = -1

    # also look if jitter is too big then punish for inconsistency
    if jitter_period >= 0.2 and reward_rtt > 0:
        reward_rtt = reward_rtt * 0.875

    # 3. PLR REWARD
    plr_period = sim_observation["plrPer"]

    # here is just a linear function
    reward_plr = 1 - 2 * plr_period  # -1 .. 1

    # FINAL WEIGHTS: Final is reward in [-1, 1]
    reward_gpt = reward_gpt * 0.33
    reward_rtt = reward_rtt * 0.33
    reward_plr = reward_plr * 0.33
    reward = reward_gpt + reward_rtt + reward_plr

    # one special case: if stream is turned off (isRunning = -1), give -0.1
    if sim_observation["isRunning"] < 0:
        reward_gpt = -0.5
        reward_rtt = 0
        reward_plr = 0
        reward = -0.5

    # another special case: if rx packets = 0 but stream is not turned off, try to motivate to select the minimal gpt
    rx_packets = sim_observation["rxPacketsPer"]
    if rx_packets == 0:
        tx_goodput_period = sim_observation["txGoodputPerAv"]
        reward_gpt = 1 - tx_goodput_period / rx_goodput_max
        reward_rtt = -1
        reward_plr = -1
        reward = reward_gpt - 2

    return reward, dict(zip(["rew", "gpt", "rtt", "plr"], [reward, reward_gpt, reward_rtt, reward_plr]))


def globPerRewardv2f(sim_observation, sim_constants=None, action=None):
    # three sub-parts
    reward = 0
    reward_gpt = 0
    reward_rtt = 0
    reward_plr = 0

    ### 1. GOODPUT REWARD
    rx_goodput_period = sim_observation["rxGoodputPerAv"]
    rx_goodput_global = sim_observation["rxGoodputGlobAv"]
    rx_goodput_max = sim_constants["maxGoodput"]
    rx_goodput_period_to_max = min(rx_goodput_period / rx_goodput_max, 1)
    rx_goodput_global_to_max = min(rx_goodput_global / rx_goodput_max, 1)
    rx_goodput_period_to_global = min(rx_goodput_period / rx_goodput_global, 1) if rx_goodput_global > 0 else 0
    chunk_variance = sim_observation["chunkVar"]

    # compensate I/P frame sizes difference
    if 0.5 <= chunk_variance < 0.75:
        rx_goodput_period_to_max *= 0.95
    elif chunk_variance >= 0.75:
        rx_goodput_period_to_max *= 0.9
    else:
        rx_goodput_period_to_max *= 1

    # relation-based discrete function:
    reward_gpt = 0.33 * rx_goodput_global_to_max + 0.33 * rx_goodput_period_to_global + 0.33 * rx_goodput_period_to_max

    # difference with v2d: take quality change
    qual_change_rate = sim_observation["qualChangeRate"]
    reward_gpt *= 1 - (qual_change_rate / 10)  # 1... 0.9

    # 2. RTT REWARD
    rtt_period = sim_observation["rttPerAv"]
    jitter_period = sim_observation["rttJitterPerAv"]

    # here is a simple constant rule-based reward based on the "expert knowledge": good 5G rtt is known from reality
    # < 100 ms is considered ok, 100-500 bad but ok, > 500-1000 0 reward, > 1000 punishment
    # so far we don't take into account any relations (like for gpt), because a strategy is based to min this at any cost
    if 0 < rtt_period <= 0.1:
        reward_rtt = max(1 - rtt_period * 10, 0.5)  # 0.5 .. 1
    elif 0.1 < rtt_period <= 0.5:
        reward_rtt = 0.6 - rtt_period  # 0.1 .. 0.5
    elif 0.5 < rtt_period <= 1.0 or rtt_period == 0:
        reward_rtt = 0
    else:
        reward_rtt = -1

    # also look if jitter is too big then punish for inconsistency
    # difference with v2d:
    if jitter_period >= 0.1 and reward_rtt > 0:
        reward_rtt = reward_rtt * 0.9

    # 3. PLR REWARD
    plr_period = sim_observation["plrPer"]
    rx_packets = sim_observation["rxPacketsPer"]
    net_instabil_rate = sim_observation["netInstabilRate"]

    # here is just a linear function
    reward_plr = 1 - 2 * plr_period  # -1 .. 1

    # difference with v2d
    if rx_packets == 0:
        reward_plr = -1

    # FINAL WEIGHTS: Final is reward in [-1, 1]
    # difference with v2d
    reward_gpt = reward_gpt * 0.33
    reward_rtt = reward_rtt * 0.33
    reward_plr = reward_plr * 0.33
    reward = reward_gpt + reward_rtt + reward_plr

    # one special case: if stream is turned off (isRunning = -1), give -0.1
    if sim_observation["isRunning"] < 0:
        reward_gpt = -0.1
        reward_rtt = 0
        reward_plr = 0
        reward = -0.1

    return reward, dict(zip(["rew", "gpt", "rtt", "plr"], [reward, reward_gpt, reward_rtt, reward_plr]))


def globPerRewardv2g(sim_observation, sim_constants=None, action=None):
    # three sub-parts
    reward = 0
    reward_gpt = 0
    reward_rtt = 0
    reward_plr = 0

    ### 1. GOODPUT REWARD
    rx_goodput_period = sim_observation["rxGoodputPerAv"]
    rx_goodput_global = sim_observation["rxGoodputGlobAv"]
    rx_goodput_max = sim_constants["maxGoodput"]
    rx_goodput_period_to_max = min(rx_goodput_period / rx_goodput_max, 1)
    rx_goodput_global_to_max = min(rx_goodput_global / rx_goodput_max, 1)
    rx_goodput_period_to_global = min(rx_goodput_period / rx_goodput_global, 1) if rx_goodput_global > 0 else 0
    chunk_variance = sim_observation["chunkVar"]

    # compensate I/P frame sizes difference
    if 0.5 <= chunk_variance < 0.75:
        rx_goodput_period_to_max *= 0.95
    elif chunk_variance >= 0.75:
        rx_goodput_period_to_max *= 0.9
    else:
        rx_goodput_period_to_max *= 1

    # relation-based discrete function:
    reward_gpt = 0.33 * rx_goodput_global_to_max + 0.33 * rx_goodput_period_to_global + 0.33 * rx_goodput_period_to_max

    qual_change_rate = sim_observation["qualChangeRate"]
    reward_gpt *= 1 - (qual_change_rate / 10)  # 1... 0.9

    # 2. RTT REWARD
    rtt_period = sim_observation["rttPerAv"]
    jitter_period = sim_observation["rttJitterPerAv"]

    if 0 < rtt_period <= 0.1:
        reward_rtt = max(1 - rtt_period * 10, 0.5)  # 0.5 .. 1
    elif 0.1 < rtt_period <= 0.5:
        reward_rtt = 0.6 - rtt_period  # 0.1 .. 0.5
    elif 0.5 < rtt_period <= 1.0 or rtt_period == 0:
        reward_rtt = 0
    else:
        reward_rtt = -1

    # also look if jitter is too big then punish for inconsistency
    if jitter_period >= 0.1 and reward_rtt > 0:
        reward_rtt = reward_rtt * 0.9

    # 3. PLR REWARD
    plr_period = sim_observation["plrPer"]
    rx_packets = sim_observation["rxPacketsPer"]
    net_instabil_rate = sim_observation["netInstabilRate"]

    # here is just a linear function
    reward_plr = 1 - 2 * plr_period  # -1 .. 1

    # difference with v2d
    if rx_packets == 0:
        reward_plr = -1

    # FINAL WEIGHTS: Final is reward in [-1, 1]
    # difference with v2f -- more on gpt
    reward_gpt = reward_gpt * 0.4
    reward_rtt = reward_rtt * 0.3
    reward_plr = reward_plr * 0.3
    reward = reward_gpt + reward_rtt + reward_plr

    # one special case: if stream is turned off (isRunning = -1), give -0.1
    if sim_observation["isRunning"] < 0:
        reward_gpt = -0.2
        reward_rtt = 0
        reward_plr = 0
        reward = -0.2

    return reward, dict(zip(["rew", "gpt", "rtt", "plr"], [reward, reward_gpt, reward_rtt, reward_plr]))


def globPerRewardv2h(sim_observation, sim_constants=None, action=None):
    # three sub-parts
    reward = 0
    reward_gpt = 0
    reward_rtt = 0
    reward_plr = 0

    ### 1. GOODPUT REWARD
    rx_goodput_period = sim_observation["rxGoodputPerAv"]
    rx_goodput_global = sim_observation["rxGoodputGlobAv"]
    rx_goodput_max = sim_constants["maxGoodput"]
    rx_goodput_period_to_max = min(rx_goodput_period / rx_goodput_max, 1)
    rx_goodput_global_to_max = min(rx_goodput_global / rx_goodput_max, 1)
    rx_goodput_period_to_global = min(rx_goodput_period / rx_goodput_global, 1) if rx_goodput_global > 0 else 0
    chunk_variance = sim_observation["chunkVar"]

    # compensate I/P frame sizes difference
    if 0.5 <= chunk_variance < 0.75:
        rx_goodput_period_to_max *= 0.95
    elif chunk_variance >= 0.75:
        rx_goodput_period_to_max *= 0.9
    else:
        rx_goodput_period_to_max *= 1

    # relation-based discrete function:
    reward_gpt = 0.33 * rx_goodput_global_to_max + 0.33 * rx_goodput_period_to_global + 0.33 * rx_goodput_period_to_max

    qual_change_rate = sim_observation["qualChangeRate"]
    reward_gpt *= 1 - (qual_change_rate / 10)  # 1... 0.9

    # 2. RTT REWARD
    rtt_period = sim_observation["rttPerAv"]
    jitter_period = sim_observation["rttJitterPerAv"]

    if 0 < rtt_period <= 0.1:
        reward_rtt = max(1 - rtt_period * 10, 0.5)  # 0.5 .. 1
    elif 0.1 < rtt_period <= 0.5:
        reward_rtt = 0.6 - rtt_period  # 0.1 .. 0.5
    elif 0.5 < rtt_period <= 1.0 or rtt_period == 0:
        reward_rtt = 0
    else:
        reward_rtt = -1

    # also look if jitter is too big then punish for inconsistency
    if jitter_period >= 0.1 and reward_rtt > 0:
        reward_rtt = reward_rtt * 0.9

    # 3. PLR REWARD
    plr_period = sim_observation["plrPer"]
    rx_packets = sim_observation["rxPacketsPer"]
    net_instabil_rate = sim_observation["netInstabilRate"]

    # here is just a linear function
    # difference with v2g: sqrt of plr to increase the pressure
    reward_plr = 1 - 2 * math.sqrt(plr_period)  # -1 .. 1

    # difference with v2d
    if rx_packets == 0:
        reward_plr = -1

    # FINAL WEIGHTS: Final is reward in [-1, 1]
    reward_gpt = reward_gpt * 0.4
    reward_rtt = reward_rtt * 0.3
    reward_plr = reward_plr * 0.3
    reward = reward_gpt + reward_rtt + reward_plr

    # one special case: if stream is turned off (isRunning = -1), give -0.1
    if sim_observation["isRunning"] < 0:
        reward_gpt = -0.2
        reward_rtt = 0
        reward_plr = 0
        reward = -0.2

    return reward, dict(zip(["rew", "gpt", "rtt", "plr"], [reward, reward_gpt, reward_rtt, reward_plr]))


def globPerRewardv2i(sim_observation, sim_constants=None, action=None):
    # three sub-parts
    reward = 0
    reward_gpt = 0
    reward_rtt = 0
    reward_plr = 0

    ### 1. GOODPUT REWARD
    rx_goodput_period = sim_observation["rxGoodputPerAv"]
    rx_goodput_global = sim_observation["rxGoodputGlobAv"]
    rx_goodput_max = sim_constants["maxGoodput"]
    rx_goodput_period_to_max = min(rx_goodput_period / rx_goodput_max, 1)
    rx_goodput_global_to_max = min(rx_goodput_global / rx_goodput_max, 1)
    rx_goodput_period_to_global = min(rx_goodput_period / rx_goodput_global, 1) if rx_goodput_global > 0 else 0
    chunk_variance = sim_observation["chunkVar"]

    # compensate I/P frame sizes difference
    if 0.5 <= chunk_variance < 0.75:
        rx_goodput_period_to_max *= 0.95
    elif chunk_variance >= 0.75:
        rx_goodput_period_to_max *= 0.9
    else:
        rx_goodput_period_to_max *= 1

    # relation-based discrete function:
    reward_gpt = 0.33 * rx_goodput_global_to_max + 0.33 * rx_goodput_period_to_global + 0.33 * rx_goodput_period_to_max

    qual_change_rate = sim_observation["qualChangeRate"]
    reward_gpt *= 1 - (qual_change_rate / 10)  # 1... 0.9

    # 2. RTT REWARD
    rtt_period = sim_observation["rttPerAv"]
    jitter_period = sim_observation["rttJitterPerAv"]

    if 0 < rtt_period <= 0.1:
        # difference with v2h
        reward_rtt = 1 - (5 * rtt_period)  # 0.5 .. 1
    elif 0.1 < rtt_period <= 0.5:
        reward_rtt = 0.6 - rtt_period  # 0.1 .. 0.5
    elif 0.5 < rtt_period <= 1.0 or rtt_period == 0:
        reward_rtt = 0
    else:
        reward_rtt = -1

    # also look if jitter is too big then punish for inconsistency
    if jitter_period >= 0.1 and reward_rtt > 0:
        reward_rtt = reward_rtt * 0.9

    # 3. PLR REWARD
    plr_period = sim_observation["plrPer"]
    rx_packets = sim_observation["rxPacketsPer"]
    net_instabil_rate = sim_observation["netInstabilRate"]

    # here is just a linear function
    reward_plr = 1 - 2 * plr_period  # -1 .. 1

    # difference with v2d
    if rx_packets == 0:
        reward_plr = -1

    # FINAL WEIGHTS: Final is reward in [-1, 1]
    reward_gpt = reward_gpt * 0.4
    reward_rtt = reward_rtt * 0.3
    reward_plr = reward_plr * 0.3
    reward = reward_gpt + reward_rtt + reward_plr

    # one special case: if stream is turned off (isRunning = -1), give -0.1
    if sim_observation["isRunning"] < 0:
        reward_gpt = -0.2
        reward_rtt = 0
        reward_plr = 0
        reward = -0.2

    return reward, dict(zip(["rew", "gpt", "rtt", "plr"], [reward, reward_gpt, reward_rtt, reward_plr]))


def globPerRewardv2j(sim_observation, sim_constants=None, action=None):
    # three sub-parts
    reward = 0
    reward_gpt = 0
    reward_rtt = 0
    reward_plr = 0

    ### 1. GOODPUT REWARD
    rx_goodput_period = sim_observation["rxGoodputPerAv"]
    rx_goodput_global = sim_observation["rxGoodputGlobAv"]
    rx_goodput_max = sim_constants["maxGoodput"]
    rx_goodput_period_to_max = min(rx_goodput_period / rx_goodput_max, 1)
    rx_goodput_global_to_max = min(rx_goodput_global / rx_goodput_max, 1)
    rx_goodput_period_to_global = min(rx_goodput_period / rx_goodput_global, 1) if rx_goodput_global > 0 else 0
    chunk_variance = sim_observation["chunkVar"]

    # compensate I/P frame sizes difference
    if 0.5 <= chunk_variance < 0.75:
        rx_goodput_period_to_max *= 0.95
    elif chunk_variance >= 0.75:
        rx_goodput_period_to_max *= 0.9
    else:
        rx_goodput_period_to_max *= 1

    # relation-based discrete function:
    reward_gpt = 0.33 * rx_goodput_global_to_max + 0.33 * rx_goodput_period_to_global + 0.33 * rx_goodput_period_to_max

    qual_change_rate = sim_observation["qualChangeRate"]
    reward_gpt *= 1 - (qual_change_rate / 10)  # 1... 0.9

    # 2. RTT REWARD
    # difference_ take delay here! it is limited in 500 ms also
    rtt_period = sim_observation["delayPerAv"]
    jitter_period = sim_observation["delayJitterPerAv"]

    reward_rtt = max(1 - (4 * rtt_period), -1)  # -1 .. 1

    # also look if jitter is too big then punish for inconsistency
    if jitter_period >= 0.1 and reward_rtt > 0:
        reward_rtt = reward_rtt * 0.9

    # 3. PLR REWARD
    plr_period = sim_observation["plrPer"]
    rx_packets = sim_observation["rxPacketsPer"]
    net_instabil_rate = sim_observation["netInstabilRate"]

    # here is just a linear function
    reward_plr = 1 - 2 * plr_period  # -1 .. 1

    # difference with v2d
    if rx_packets == 0:
        reward_plr = -1

    # FINAL WEIGHTS: Final is reward in [-1, 1]
    reward_gpt = reward_gpt * 0.4
    reward_rtt = reward_rtt * 0.3
    reward_plr = reward_plr * 0.3
    reward = reward_gpt + reward_rtt + reward_plr

    # one special case: if stream is turned off (isRunning = -1), give -0.1
    if sim_observation["isRunning"] < 0:
        reward_gpt = -0.2
        reward_rtt = 0
        reward_plr = 0
        reward = -0.2

    return reward, dict(zip(["rew", "gpt", "rtt", "plr"], [reward, reward_gpt, reward_rtt, reward_plr]))


# try to maximize a bandwidth
def bandwidthRewardv1(sim_observation, sim_constants=None, action=None):
    # three sub-parts
    reward = 0
    reward_bandwidth = 0
    reward_plr = 0

    ### REWARD: bandwidth!
    rx_goodput_period = sim_observation["rxGoodputPerAv"]
    rx_goodput_max = sim_constants["maxGoodput"]
    chunk_variance = sim_observation["chunkVar"]

    rtt_period = sim_observation["rttPerAv"]

    plr_period = sim_observation["plrPer"]

    # compensate I/P frame sizes difference
    if 0.5 <= chunk_variance < 0.75:
        rx_goodput_period *= 0.95
    elif chunk_variance >= 0.75:
        rx_goodput_period *= 0.9
    else:
        rx_goodput_period *= 1

    # relation-based discrete function:
    reward_bandwidth = min(min(rx_goodput_period / rtt_period, rx_goodput_max), 1) if rtt_period > 0 else 0
    reward_plr = -0.5 * plr_period

    # FINAL WEIGHTS: Final is reward in [-1, 1]
    reward = reward_bandwidth + reward_plr

    # one special case: if stream is turned off (isRunning = -1), give -0.1
    if sim_observation["isRunning"] < 0:
        reward_bandwidth = -0.2
        reward_plr = 0
        reward = -0.2

    return reward, dict(zip(["rew", "band", "plr"], [reward, reward_bandwidth, reward_plr]))


# try simple linear approach
def globPerRewardv3(sim_observation, sim_constants=None, action=None):
    # three sub-parts
    reward = 0
    reward_gpt = 0
    reward_rtt = 0
    reward_plr = 0

    ### 1. GOODPUT REWARD
    rx_goodput_global = sim_observation["rxGoodputGlobAv"]
    chunk_variance = sim_observation["chunkVar"]

    # compensate I/P frame sizes difference
    if 0.5 <= chunk_variance:
        rx_goodput_global *= 1 - (0.1 * chunk_variance)

    reward_gpt = rx_goodput_global

    # 2. RTT REWARD
    rtt_period = sim_observation["rttPerAv"]
    jitter_period = sim_observation["rttJitterPerAv"]

    reward_rtt = -rtt_period

    if rtt_period == 0 or rtt_period > 1:
        reward_rtt = -1.2

    # also look if jitter is too big then punish for inconsistency
    if jitter_period >= 0.2 and reward_rtt > 0:
        reward_rtt *= 1.05

    # 3. PLR REWARD
    plr_period = sim_observation["plrPer"]
    rx_packets = sim_observation["rxPacketsPer"]

    reward_plr = 0.5 * plr_period

    # if there are too few packets then refuse to estimate
    if rx_packets <= 2:
        reward_plr = -1

    # FINAL WEIGHTS: Final is reward in [-1, 1]
    reward = reward_gpt + reward_rtt + reward_plr

    # one special case: if stream is turned off (isRunning = -1), give -0.1
    if sim_observation["isRunning"] < 0:
        reward_gpt = 0
        reward_rtt = 0
        reward_plr = 0
        reward = -1

    return reward, dict(zip(["rew", "gpt", "rtt", "plr"], [reward, reward_gpt, reward_rtt, reward_plr]))


# effective throughput
def effGptReward(sim_observation, sim_constants=None, action=None):
    # three sub-parts
    reward = 0
    reward_gpt = 0
    reward_rtt = 0
    reward_plr = 0

    ### 1. GOODPUT REWARD
    rx_goodput_global = sim_observation["rxGoodputGlobAv"]
    chunk_variance = sim_observation["chunkVar"]

    # compensate I/P frame sizes difference
    if 0.5 <= chunk_variance < 0.75:
        key_frame_comprensation = 0.99
    elif chunk_variance >= 0.75:
        key_frame_comprensation = 0.98
    else:
        key_frame_comprensation = 1.0

    reward_gpt = 1 * rx_goodput_global
    reward_gpt *= key_frame_comprensation

    # 2. RTT REWARD
    rtt_global = sim_observation["rttGlobAv"]
    if rtt_global > 0:
        reward_rtt = 1 - min(rtt_global, 1)
    else:
        reward_rtt = 0

    # 3. PLR REWARD
    plr_global = sim_observation["plrGlob"]

    # same as with rtt, for  0--0.5
    reward_plr = 1 - plr_global

    # Final reward: [-1, 1]
    reward = reward_gpt * reward_rtt * reward_plr

    # special case: if stream is turned off (isRunning = -1), penalize extra by -0.5
    if sim_observation["isRunning"] < 0:
        reward_gpt = 0
        reward = 0

    # return result
    return reward, dict(zip(["rew", "gpt", "rtt", "plr"], [reward, reward_gpt, reward_rtt, reward_plr]))


# using previous gpt/rtt/plr
def prevRewardv1(sim_observation, sim_constants=None, action=None):
    gpt_current, gpt_previous = sim_observation["rxGoodputPerAv"], sim_observation["rxGoodputPrevPerAv"]
    rtt_current, rtt_previous = sim_observation["rttPerAv"], sim_observation["rttPrevPerAv"]
    loss_rate_current, loss_rate_previous = sim_observation["plrPer"], sim_observation["plrPrevPer"]
    gpt_avg, rtt_avg, loss_rate_avg = (
        sim_observation["rxGoodputGlobAv"],
        sim_observation["rttGlobAv"],
        sim_observation["plrGlob"],
    )

    alpha = 0.5  # weight for maximizing bitrate
    beta = 0.3  # weight for minimizing round-trip time
    gamma = 0.2  # weight for minimizing packet loss rate

    # calculate the improvements
    gpt_improvement = gpt_current - gpt_previous
    rtt_improvement = rtt_previous - rtt_current
    loss_rate_improvement = loss_rate_previous - loss_rate_current

    # Calculate the rewards for each objective
    reward_gpt = alpha * gpt_improvement / gpt_avg if gpt_avg > 0 else 0
    reward_rtt = beta * rtt_improvement / rtt_avg if rtt_avg > 0 else 0
    reward_plr = gamma * loss_rate_improvement / loss_rate_avg if loss_rate_avg > 0 else 0
    reward = reward_gpt + reward_rtt + reward_plr

    # one special case: if stream is turned off (isRunning = -1), give -0.1
    if sim_observation["isRunning"] < 0:
        reward_gpt = -0.1
        reward_rtt = 0
        reward_plr = 0
        reward = -0.1

    return reward, dict(zip(["rew", "gpt", "rtt", "plr"], [reward, reward_gpt, reward_rtt, reward_plr]))


def prevRewardv2(sim_observation, sim_constants=None, action=None):
    gpt_current, gpt_previous = sim_observation["rxGoodputPerAv"], sim_observation["rxGoodputPrevPerAv"]
    rtt_current, rtt_previous = sim_observation["rttPerAv"], sim_observation["rttPrevPerAv"]
    loss_rate_current, loss_rate_previous = sim_observation["plrPer"], sim_observation["plrPrevPer"]
    gpt_avg, rtt_avg, loss_rate_avg = (
        sim_observation["rxGoodputGlobAv"],
        sim_observation["rttGlobAv"],
        sim_observation["plrGlob"],
    )

    alpha = 0.4  # weight for maximizing bitrate
    beta = 0.3  # weight for minimizing round-trip time
    gamma = 0.3  # weight for minimizing packet loss rate

    # calculate the improvements
    gpt_improvement = (gpt_current - gpt_previous) / 10
    rtt_improvement = rtt_previous - rtt_current
    loss_rate_improvement = loss_rate_previous - loss_rate_current

    # Calculate the rewards for each objective
    reward_gpt = alpha * gpt_improvement / gpt_avg if gpt_avg > 0 else 0
    reward_rtt = beta * rtt_improvement / rtt_avg if rtt_avg > 0 else 0
    reward_plr = gamma * loss_rate_improvement / loss_rate_avg if loss_rate_avg > 0 else 0
    reward = reward_gpt + reward_rtt + reward_plr

    # one special case: if stream is turned off (isRunning = -1), give -0.1
    if sim_observation["isRunning"] < 0:
        reward_gpt = -0.1
        reward_rtt = 0
        reward_plr = 0
        reward = -0.1

    return reward, dict(zip(["rew", "gpt", "rtt", "plr"], [reward, reward_gpt, reward_rtt, reward_plr]))


def qoeRewardv1(sim_observation, sim_constants=None, action=None):
    # three sub-parts
    reward = 0
    reward_qoe_delay_plr = 0
    reward_bitrate = 0

    # 1. QoE delay + plr, use QoE from paper Mushtaq et. al: QoE in 5G Cloud Networks using Multimedia Services (2016)
    delay = sim_observation["delayPerAv"]
    plr = sim_observation["plrPer"]
    reward_qoe_delay_plr = math.exp(1.576 - (4.188e-4 * delay * 1000) - (5.766e-2 * plr * 100))

    # 2. Bitrate
    rx_goodput_period = sim_observation["rxGoodputPerAv"]
    rx_goodput_max = sim_constants["maxGoodput"]
    bitrate_ratio = min(rx_goodput_period / rx_goodput_max, 1)
    reward_bitrate = 5**bitrate_ratio if bitrate_ratio > 0 else 0

    # not running
    if sim_observation["isRunning"] < 0:
        reward_qoe_delay_plr = 0
        reward_bitrate = 0
        reward = -0.1

    reward = reward_qoe_delay_plr + reward_bitrate
    return reward, dict(zip(["rew", "qoe", "bit"], [reward, reward_qoe_delay_plr, reward_bitrate]))


def qoeRewardv2(sim_observation, sim_constants=None, action=None):
    # three sub-parts
    reward = 0
    reward_qoe_delay_plr = 0
    reward_bitrate = 0
    reward_jitter = 0

    # 1. QoE delay + plr, use QoE from paper Mushtaq et. al: QoE in 5G Cloud Networks using Multimedia Services (2016)
    delay = sim_observation["delayPerAv"]
    plr = sim_observation["plrPer"]
    reward_qoe_delay_plr = math.exp(1.576 - (4.188e-4 * delay * 1000) - (5.766e-2 * plr * 100))

    # 2. Bitrate
    rx_goodput_period = sim_observation["rxGoodputPerAv"]
    rx_goodput_max = sim_constants["maxGoodput"]
    bitrate_ratio = min(rx_goodput_period / rx_goodput_max, 1)
    reward_bitrate = 5**bitrate_ratio if bitrate_ratio > 0 else 0

    # 3. add jitter -1..1, Wahab et al: Direct propagation of network QoS distribution... (2020)
    delay_jitter = sim_observation["delayJitterPerAv"] * 1000
    delay_jitter = delay_jitter if delay_jitter <= 625 else 625  # make reward jitter -1...1
    reward_jitter = -0.08 * math.sqrt(delay_jitter) + 1

    # not running
    if sim_observation["isRunning"] < 0:
        reward_qoe_delay_plr = 0
        reward_bitrate = 0
        reward_jitter = 0
        reward = -0.1

    reward = reward_qoe_delay_plr + reward_bitrate + reward_jitter
    return reward, dict(
        zip(["rew", "qoe", "bit", "jit"], [reward, reward_qoe_delay_plr, reward_bitrate, reward_jitter])
    )


def qoeRewardv3(sim_observation, sim_constants=None, action=None):
    # three sub-parts
    reward = 0
    reward_qoe_delay_plr = 0
    reward_bitrate = 0
    reward_jitter = 0

    # 1. QoE delay + plr, use QoE from paper Mushtaq et. al: QoE in 5G Cloud Networks using Multimedia Services (2016)
    delay = sim_observation["delayPerAv"]
    plr = sim_observation["plrPer"]
    ## registered plr is good but rxPackets / txPackets (real known plr) is small (meaning acks got stuck), increase plr
    txPackets = max(sim_observation["txPacketsPer"], 1)
    rxPackets = sim_observation["rxPacketsPer"]
    if 2 * plr < (txPackets - rxPackets) / txPackets:
        plr = rxPackets / txPackets
        delay *= 10

    reward_qoe_delay_plr = math.exp(1.576 - (4.188e-4 * delay * 1000) - (5.766e-2 * plr * 100))

    # 2. Bitrate
    rx_goodput_period = sim_observation["rxGoodputPerAv"]
    rx_goodput_max = sim_constants["maxGoodput"]
    bitrate_ratio = min(rx_goodput_period / rx_goodput_max, 1)
    reward_bitrate = 5**bitrate_ratio if bitrate_ratio > 0 else 0

    # 3. add jitter -1..1, Wahab et al: Direct propagation of network QoS distribution... (2020)
    delay_jitter = sim_observation["delayJitterPerAv"] * 1000
    delay_jitter = delay_jitter if delay_jitter <= 625 else 625  # make reward jitter -1...1
    reward_jitter = -0.08 * math.sqrt(delay_jitter) + 1

    # not running
    if sim_observation["isRunning"] < 0:
        reward_qoe_delay_plr = 0
        reward_bitrate = 0
        reward_jitter = 0
        reward = -0.1

    reward = reward_qoe_delay_plr + reward_bitrate + reward_jitter
    return reward, dict(
        zip(["rew", "qoe", "bit", "jit"], [reward, reward_qoe_delay_plr, reward_bitrate, reward_jitter])
    )


def qoeRewardv4(sim_observation, sim_constants=None, action=None):
    # three sub-parts
    reward = 0
    reward_qoe_delay_plr = 0
    reward_bitrate = 0
    reward_jitter = 0

    # 1. QoE delay + plr, use QoE from paper Mushtaq et. al: QoE in 5G Cloud Networks using Multimedia Services (2016)
    delay = sim_observation["delayPerAv"]
    plr = sim_observation["plrPer"]
    ## registered plr is good but rxPackets / txPackets (real known plr) is small (meaning acks got stuck), increase plr
    txPackets = max(sim_observation["txPacketsPer"], 1)
    rxPackets = sim_observation["rxPacketsPer"]
    if 2 * plr < (txPackets - rxPackets) / txPackets:
        plr = rxPackets / txPackets
        delay *= 10

    reward_qoe_delay_plr = math.exp(1.576 - (4.188e-4 * delay * 1000) - (5.766e-2 * plr * 100))
    # normalize
    reward_qoe_delay_plr /= 5

    # 2. Bitrate
    rx_goodput_period = sim_observation["rxGoodputPerAv"]
    rx_goodput_max = sim_constants["maxGoodput"]
    bitrate_ratio = min(rx_goodput_period / rx_goodput_max, 1)
    reward_bitrate = 5**bitrate_ratio if bitrate_ratio > 0 else 0
    # normalize
    reward_bitrate /= 5

    # 3. add jitter -1..1, Wahab et al: Direct propagation of network QoS distribution... (2020)
    delay_jitter = sim_observation["delayJitterPerAv"] * 1000
    delay_jitter = delay_jitter if delay_jitter <= 625 else 625
    reward_jitter = -0.04 * math.sqrt(delay_jitter) + 1  # fixed to be -1 .. 0 + 1

    # not running
    if sim_observation["isRunning"] < 0:
        reward_qoe_delay_plr = 0
        reward_bitrate = 0
        reward_jitter = 0
        reward = 0

    # average weights by default
    reward = 0.33 * reward_qoe_delay_plr + 0.33 * reward_bitrate + 0.33 * reward_jitter
    return reward, dict(
        zip(["rew", "qoe", "bit", "jit"], [reward, reward_qoe_delay_plr, reward_bitrate, reward_jitter])
    )


def qoeRewardv5(sim_observation, sim_constants=None, action=None):
    # three sub-parts
    reward = 0
    reward_qoe_delay_plr = 0
    reward_bitrate = 0
    reward_jitter = 0

    # 1. QoE delay + plr, use QoE from paper Mushtaq et. al: QoE in 5G Cloud Networks using Multimedia Services (2016)
    delay = sim_observation["delayPerAv"]
    plr = sim_observation["plrPer"]
    ## registered plr is good but rxPackets / txPackets (real known plr) is small (meaning acks got stuck), increase plr
    txPackets = max(sim_observation["txPacketsPer"], 1)
    rxPackets = sim_observation["rxPacketsPer"]
    if 2 * plr < (txPackets - rxPackets) / txPackets:
        plr = rxPackets / txPackets
        delay *= 10

    reward_qoe_delay_plr = math.exp(1.576 - (4.188e-4 * delay * 1000) - (5.766e-2 * plr * 100))

    # 2. Bitrate NEW!!! my own approximation of plots in Uhrina et al: Impact of H.264/AVC and H.265/HEVC Compression Standards..(2014)
    # we try to model SINR-Bitrate plots via log-scaling because we do not have bitrate for simualted data, 0...5
    rx_goodput_period = sim_observation["rxGoodputPerAv"]
    rx_goodput_max = 20  # fixed so far!
    bitrate = min(rx_goodput_period, rx_goodput_max)
    reward_bitrate = 1.01 * math.log(6 * bitrate + 1)

    # 3. add jitter -1..1, Wahab et al: Direct propagation of network QoS distribution... (2020)
    delay_jitter = sim_observation["delayJitterPerAv"] * 1000
    delay_jitter = min(delay_jitter, 625)  # make reward jitter 0...1, take 625 as the max val
    reward_jitter = -0.04 * math.sqrt(delay_jitter) + 1

    # not running
    if sim_observation["isRunning"] < 0:
        reward_qoe_delay_plr = 0
        reward_bitrate = 0
        reward_jitter = 0
        reward = -0.1

    reward = reward_qoe_delay_plr + reward_bitrate + reward_jitter
    return reward, dict(
        zip(["rew", "qoe", "bit", "jit"], [reward, reward_qoe_delay_plr, reward_bitrate, reward_jitter])
    )


# only exemplarly usage and to copy-paste the boilerplate scheme
def dummy(
    state_gen: StateGenerator,
    last_action: int | np.ndarray[int | float],
) -> Tuple[float, Dict[str, float]]:
    return state_gen.state["rxGoodput"], dict(
        zip(["rew", "qoe", "bit", "jit"], [state_gen.state["rxGoodput"], 1, 1, 1])
    )


def qoe(
    state_gen: StateGenerator,
    last_action: int | np.ndarray[int | float],
) -> Tuple[float, Dict[str, float]]:
    # three sub-parts
    reward = 0
    reward_qoe_delay_plr = 0
    reward_bitrate = 0
    reward_jitter = 0

    # 1. QoE delay + plr, use QoE from paper Mushtaq et. al: QoE in 5G Cloud Networks using Multimedia Services (2016)
    reward_qoe_delay_plr = math.exp(
        1.576 - (4.188e-4 * state_gen.fractionRtt) - (5.766e-2 * state_gen.fractionLossRate * 100)
    )

    # 2. Bitrate NEW!!! my own approximation of plots in Uhrina et al: Impact of H.264/AVC and H.265/HEVC Compression Standards..(2014)
    # we try to model SINR-Bitrate plots via log-scaling because we do not have bitrate for simualted data, 0...5
    reward_bitrate = 1.01 * math.log(6 * state_gen.rxGoodput + 1)

    # 3. add jitter -1..1, Wahab et al: Direct propagation of network QoS distribution... (2020)
    # make reward jitter 0...1, take 625 as the max val
    reward_jitter = -0.04 * math.sqrt(state_gen.fractionAvInterarrivalJitter) + 1

    reward = reward_qoe_delay_plr + reward_bitrate + reward_jitter
    return reward, dict(
        zip(["rew", "qoe", "bit", "jit"], [reward, reward_qoe_delay_plr, reward_bitrate, reward_jitter])
    )


def qoeAcm(
    state_gen: StateGenerator,
    last_action: int | np.ndarray[int | float],
) -> Tuple[float, Dict[str, float]]:
    # taken from ACM MMSys’21 Grand Challenge

    reward_rate = 100 * min(state_gen.rxGoodput, state_gen.MAX_RATE) / state_gen.MAX_RATE

    # clipping over running min and max
    rtts = state_gen.aux_vars["rtts"]
    clipped = [StateGenerator.clip(r, state_gen.aux_vars["minRtt"], state_gen.aux_vars["maxRtt"]) for r in rtts]
    rtt95 = np.percentile(clipped, 95)
    reward_rtt = 100 * (
        (state_gen.aux_vars["maxRtt"] - rtt95) / (state_gen.aux_vars["maxRtt"] - state_gen.aux_vars["minRtt"])
    )

    reward_plr = 100 * (1 - state_gen.fractionLossRate)

    reward = 0.2 * reward_rate + 0.2 * reward_rtt + 0.3 * reward_plr
    return reward, dict(zip(["rew", "rate", "rtt", "plr"], [reward, reward_rate, reward_rtt, reward_plr]))


def qoeAcmv2(
    state_gen: StateGenerator,
    last_action: int | np.ndarray[int | float],
) -> Tuple[float, Dict[str, float]]:
    # change coeffs from 0.2, 0.2, 0.3 to 0.25, 0.25, 0.3 + two additional 0.1: change delay95 to av delay, add reward for jitter and for playout delay
    # also normalize reward in 0..1

    # TODO: try in the next iteration to replace it with a log rate, e.g. ln ((e - 1) * rxGoodput / MAX_RATE + 1)
    reward_rate = min(state_gen.rxGoodput, state_gen.MAX_RATE) / state_gen.MAX_RATE

    # clipping over running min and max
    rtts = state_gen.aux_vars["rtts"]
    clipped = [StateGenerator.clip(r, state_gen.aux_vars["minRtt"], state_gen.aux_vars["maxRtt"]) for r in rtts]
    clipped_av = sum(clipped) / len(clipped) if len(clipped) > 0 else 0.0
    reward_rtt = (
        (state_gen.aux_vars["maxRtt"] - clipped_av) / (state_gen.aux_vars["maxRtt"] - state_gen.aux_vars["minRtt"])
        if state_gen.aux_vars["maxRtt"] - state_gen.aux_vars["minRtt"] > 0
        else 0.0
    )

    reward_plr = 1 - state_gen.fractionLossRate

    # add reward for jitter: Wahab et al: Direct propagation of network QoS distribution... (2020), make reward jitter 0...1, take 400 as the max val
    reward_jitter = -0.05 * math.sqrt(state_gen.interarrivalJitter) + 1

    # add reward for playout delay
    reward_playout_delay = 1 - (state_gen.fractionAvPlayoutDelay / state_gen.MAX_PLAYOUT_DELAY)

    reward = (
        0.25 * reward_rate + 0.25 * reward_rtt + 0.3 * reward_plr + 0.1 * reward_jitter + 0.1 * reward_playout_delay
    )
    return reward, dict(
        zip(
            ["rew", "rate", "rtt", "plr", "jit", "ply"],
            [
                reward,
                0.25 * reward_rate,
                0.25 * reward_rtt,
                0.3 * reward_plr,
                0.1 * reward_jitter,
                0.1 * reward_playout_delay,
            ],
        )
    )


def fang(
    state_gen: StateGenerator,
    last_action: int | np.ndarray[int | float],
) -> Tuple[float, Dict[str, float]]:
    # Fang et al: Reinforcement learning for bandwidth estimation and congestion control in real-time communications (2019)

    # original: 0.6 ln(4R + 1) − D − 10L

    reward_rate = 0.6 * np.log(0.5 * state_gen.rxGoodput + 1)  # 0.5 instead of 4 because rate should be in Mb/s
    reward_rtt = -(state_gen.fractionRtt / 1000)  # seconds
    reward_plr = -10 * state_gen.fractionLossRate

    reward = reward_rate + reward_rtt + reward_plr
    return reward, dict(zip(["rew", "rate", "rtt", "plr"], [reward, reward_rate, reward_rtt, reward_plr]))


def fang10(
    state_gen: StateGenerator,
    last_action: int | np.ndarray[int | float],
) -> Tuple[float, Dict[str, float]]:
    # modified fang version: 0.6 ln(4R + 1) − D − 10L, new 0.6 ln(4R + 1) − 10D − 10L

    reward_rate = 0.6 * np.log(4 * state_gen.rxGoodput + 1)
    reward_rtt = -10 * (state_gen.fractionRtt / 1000)
    reward_plr = -10 * state_gen.fractionLossRate

    reward = reward_rate + reward_rtt + reward_plr
    return reward, dict(zip(["rew", "rate", "rtt", "plr"], [reward, reward_rate, reward_rtt, reward_plr]))


def hrcc(
    state_gen: StateGenerator,
    last_action: int | np.ndarray[int | float],
) -> Tuple[float, Dict[str, float]]:
    # Wang et al: A Hybrid Receiver-side Congestion Control Scheme for Web Real-time Communication (2021)

    # Bandwidth utilization is replaced with gpt / gpt_max

    # original: 10U - 2.5 * P_delay - 10L

    reward_rate = 10 * min(state_gen.rxGoodput, state_gen.MAX_RATE) / state_gen.MAX_RATE

    p_delay = 0.0
    if state_gen.fractionAvInterarrivalDelay < 30.0:
        p_delay = state_gen.fractionRtt / 200.0
    elif state_gen.fractionAvInterarrivalDelay >= 30.0 and state_gen.fractionAvInterarrivalDelay < 80.0:
        p_delay = 0.15 + (state_gen.fractionAvInterarrivalDelay / 100.0)
    else:
        p_delay = 0.65 + (state_gen.fractionAvInterarrivalDelay / 50.0)
    reward_delay = -2.5 * p_delay

    reward_plr = -10 * state_gen.fractionLossRate

    reward = reward_rate + reward_delay + reward_plr
    return reward, dict(zip(["rew", "rate", "delay", "plr"], [reward, reward_rate, reward_delay, reward_plr]))


def bob(
    state_gen: StateGenerator,
    last_action: int | np.ndarray[int | float],
) -> Tuple[float, Dict[str, float]]:
    # Bentaleb et al: BoB: Bandwidth Prediction for Real-Time Communications Using Heuristic and Reinforcement Learning (2022)

    # note, we assume here that our min bandwidth is equal to 0.4 as our lowest target bitrate is 400 kbit/s

    # original: Lin-to-Log(R) - min(D/1000, 1) - L

    min_rate = 0.4
    reward_rate = (np.log(state_gen.rxGoodput) - np.log(min_rate)) / (np.log(state_gen.MAX_RATE) - np.log(min_rate))

    reward_delay = -min(state_gen.fractionAvInterarrivalDelay / 1000, 1)
    reward_plr = -state_gen.fractionLossRate

    reward = reward_rate + reward_delay + reward_plr
    return reward, dict(zip(["rew", "rate", "delay", "plr"], [reward, reward_rate, reward_delay, reward_plr]))


def onrl(
    state_gen: StateGenerator,
    last_action: int | np.ndarray[int | float],
) -> Tuple[float, Dict[str, float]]:
    # Zhang et al: OnRL: Improving Mobile Video Telephony via Online Reinforcement Learning (2020)

    # original: 50 * R - 50 * L - 10 * D - 30 * (R - R_prev)

    reward_rate = 50 * state_gen.rxGoodput
    reward_plr = -50 * state_gen.fractionLossRate
    reward_delay = -10 * state_gen.fractionAvInterarrivalDelay

    prev_goodput = state_gen.last_obs.transmission.rxGoodput if state_gen.is_last_obs else 0.0
    reward_smooth = -30 * (state_gen.rxGoodput - prev_goodput)

    reward = reward_rate + reward_delay + reward_plr + reward_smooth
    return reward, dict(
        zip(["rew", "rate", "delay", "plr", "smooth"], [reward, reward_rate, reward_delay, reward_plr, reward_smooth])
    )


def combine_v1(
    state_gen: StateGenerator,
    last_action: int | np.ndarray[int | float],
) -> Tuple[float, Dict[str, float]]:
    # Combine ideas from previous DRL approaches

    # 1. rate: take logarithmic idea from fang, rate of r / r_max from 0 to 1
    reward_rate = np.log((np.exp(1) - 1) * (state_gen.rxGoodput / state_gen.MAX_RATE) + 1)

    # 2. rtt: take idea from acm QoE, namely take running min, max and average
    reward_rtt = (
        (state_gen.aux_vars["maxRtt"] - state_gen.fractionRtt)
        / (state_gen.aux_vars["maxRtt"] - state_gen.aux_vars["minRtt"])
        if state_gen.aux_vars["maxRtt"] - state_gen.aux_vars["minRtt"] > 0
        else 0.0
    )

    # 3. plr, here just a classic intuition
    reward_plr = 1 - state_gen.fractionLossRate

    # 4. jitter: Wahab et al: Direct propagation of network QoS distribution... (2020), make reward jitter 0...1, take 400 as the max val
    # but this interarrival jitter is divided by 16, so we take 400 / 16 = 25 as the max val
    reward_jitter = (
        -0.2 * math.sqrt(StateGenerator.clip(state_gen.interarrivalJitter, 0.0, state_gen.MAX_JITTER / 16.0)) + 1
    )

    # 5. playout delay
    reward_playout_delay = 1 - (state_gen.fractionAvPlayoutDelay / state_gen.MAX_PLAYOUT_DELAY)

    # coefficients
    a = 0.25
    b = 0.25
    c = 0.3
    d = 0.1
    e = 0.1
    reward = a * reward_rate + b * reward_rtt + c * reward_plr + d * reward_jitter + e * reward_playout_delay
    return reward, dict(
        zip(
            ["rew", "rate", "rtt", "plr", "jit", "ply"],
            [reward, a * reward_rate, b * reward_rtt, c * reward_plr, d * reward_jitter, e * reward_playout_delay],
        )
    )


def combine_v2(
    state_gen: StateGenerator,
    last_action: int | np.ndarray[int | float],
) -> Tuple[float, Dict[str, float]]:
    # Smooth variant: instead of playout delay we take previous rate

    # 1. rate: take logarithmic idea from fang, rate of r / r_max from 0 to 1
    reward_rate = np.log((np.exp(1) - 1) * (state_gen.rxGoodput / state_gen.MAX_RATE) + 1)

    # 2. rtt: take idea from acm QoE, namely take running min, max and average
    reward_rtt = (
        (state_gen.aux_vars["maxRtt"] - state_gen.fractionRtt)
        / (state_gen.aux_vars["maxRtt"] - state_gen.aux_vars["minRtt"])
        if state_gen.aux_vars["maxRtt"] - state_gen.aux_vars["minRtt"] > 0
        else 0.0
    )

    # 3. plr, here just a classic intuition
    reward_plr = 1 - state_gen.fractionLossRate

    # 4. jitter: Wahab et al: Direct propagation of network QoS distribution... (2020), make reward jitter 0...1, take 400 as the max val
    # but this interarrival jitter is divided by 16, so we take 400 / 16 = 25 as the max val
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
    d = 0.12
    e = 0.08
    reward = a * reward_rate + b * reward_rtt + c * reward_plr + d * reward_jitter + e * reward_smooth
    return reward, dict(
        zip(
            ["rew", "rate", "rtt", "plr", "jit", "smt"],
            [reward, a * reward_rate, b * reward_rtt, c * reward_plr, d * reward_jitter, e * reward_smooth],
        )
    )


def combine_v3(
    state_gen: StateGenerator,
    last_action: int | np.ndarray[int | float],
) -> Tuple[float, Dict[str, float]]:
    # Smooth variant: instead of playout delay we take previous rate

    # 1. rate: take logarithmic idea from Fang et. al, rate of r / r_max from 0 to 1
    reward_rate = np.log((np.exp(1) - 1) * (state_gen.rxGoodput / state_gen.MAX_RATE) + 1)

    # 2. rtt: take idea from acm QoE, namely take running min, max and average
    reward_rtt = (
        (state_gen.aux_vars["maxRtt"] - state_gen.fractionRtt)
        / (state_gen.aux_vars["maxRtt"] - state_gen.aux_vars["minRtt"])
        if state_gen.aux_vars["maxRtt"] - state_gen.aux_vars["minRtt"] > 0
        else 0.0
    )

    # 3. plr, here just a classic intuition
    reward_plr = 1 - state_gen.fractionLossRate

    # 4. jitter: Wahab et al: Direct propagation of network QoS distribution... (2020)
    # scaled rel max val is 10 (RTP ts)
    reward_jitter = -(math.sqrt(StateGenerator.clip(state_gen.interarrivalJitter, 0.0, 10.0)) / math.sqrt(10)) + 1

    # 5. smooth: take rate of change, ideally hold it within 10% max, because in that case no reencoding happens
    prev_goodput = state_gen.last_obs.transmission.rxGoodput if state_gen.is_last_obs else 0.0
    rate_of_change = abs(state_gen.rxGoodput - prev_goodput) / state_gen.MAX_RATE
    reward_smooth = 1 if rate_of_change <= 0.1 else 1 - rate_of_change

    # coefficients
    a = 0.3
    b = 0.25
    c = 0.3
    d = 0.1
    e = 0.05
    reward = a * reward_rate + b * reward_rtt + c * reward_plr + d * reward_jitter + e * reward_smooth
    return reward, dict(
        zip(
            ["rew", "rate", "rtt", "plr", "jit", "smt"],
            [reward, a * reward_rate, b * reward_rtt, c * reward_plr, d * reward_jitter, e * reward_smooth],
        )
    )


def ahoy(
    state_gen: StateGenerator,
    last_action: int | np.ndarray[int | float],
) -> Tuple[float, Dict[str, float]]:
    # assumed that is scale true!!!

    # 1. rate: take logarithmic idea from fang, rate of r / r_max from 0 to 1
    reward_rate = np.log((np.exp(1) - 1) * (state_gen.rxGoodput / state_gen.MAX_RATE) + 1)

    # 2. rtt: take idea from acm QoE, namely take running min, max and average
    reward_rtt = (
        (state_gen.aux_vars["maxRtt"] - state_gen.fractionRtt)
        / (state_gen.aux_vars["maxRtt"] - state_gen.aux_vars["minRtt"])
        if state_gen.aux_vars["maxRtt"] - state_gen.aux_vars["minRtt"] > 0
        else 0.0
    )

    # 3. plr, here just a classic intuition
    reward_plr = 1 - state_gen.fractionLossRate

    # 4. jitter: Wahab et al: Direct propagation of network QoS distribution... (2020), make reward jitter 0...1, take 400 as the max val
    # but this interarrival jitter is divided by 16, so we take 400 / 16 = 25 as the max val
    reward_jitter = (
        -0.2 * math.sqrt(StateGenerator.clip(state_gen.interarrivalRttJitter, 0.0, state_gen.MAX_JITTER / 16.0)) + 1
    )

    # 5. smooth: take rate of change, ideally hold it within 10% max, because in that case no reencoding happens
    prev_goodput = state_gen.last_obs.transmission.rxGoodput if state_gen.is_last_obs else 0.0
    rate_of_change = abs(state_gen.rxGoodput - prev_goodput) / state_gen.MAX_RATE
    reward_smooth = 1 if rate_of_change <= 0.1 else 1 - rate_of_change

    # NEW: penalties for nack and pli rates
    reward_nack = max(-0.25, -(0.1 * state_gen.fractionNackRate))
    reward_pli = max(-0.25, -(25 * state_gen.fractionPliRate))

    # coefficients
    a = 0.3
    b = 0.2
    c = 0.3
    d = 0.1
    e = 0.1
    reward = (
        a * reward_rate
        + b * reward_rtt
        + c * reward_plr
        + d * reward_jitter
        + e * reward_smooth
        + reward_nack
        + reward_pli
    )
    return reward, dict(
        zip(
            ["rew", "rate", "rtt", "plr", "jit", "smt", "nack", "pli"],
            [
                reward,
                a * reward_rate,
                b * reward_rtt,
                c * reward_plr,
                d * reward_jitter,
                e * reward_smooth,
                reward_nack,
                reward_pli,
            ],
        )
    )


REWARD_FUNCTIONS_DICT = {
    "best": globPerRewardv2g,
    "glob_v1": globRewardv1,
    "glob_per_v1": globPerRewardv1,
    "glob_per_v1a": globPerRewardv1a,
    "glob_per_v1b": globPerRewardv1b,
    "glob_per_v1c": globPerRewardv1c,
    "glob_per_v2": globPerRewardv2,
    "glob_per_v2a": globPerRewardv2a,
    "glob_per_v2b": globPerRewardv2b,
    "glob_per_v2c": globPerRewardv2c,
    "glob_per_v2d": globPerRewardv2d,
    "glob_per_v2e": globPerRewardv2e,
    "glob_per_v2f": globPerRewardv2f,
    "glob_per_v2g": globPerRewardv2g,
    "glob_per_v2h": globPerRewardv2h,
    "glob_per_v2i": globPerRewardv2i,
    "glob_per_v2j": globPerRewardv2j,
    "glob_per_v3": globPerRewardv3,
    "prev_v1": prevRewardv1,
    "prev_v2": prevRewardv2,
    "eff_v1": effGptReward,
    "bandwidth_v1": bandwidthRewardv1,
    "qoe_v1": qoeRewardv1,
    "qoe_v2": qoeRewardv2,
    "qoe_v3": qoeRewardv3,
    "qoe_v4": qoeRewardv4,
    ############################## NEW DESIGNS AFTER BREAKING ENV/OBS/STATE CHANGES ##############################
    "dummy": dummy,
    "qoe": qoe,
    "qoe_acm": qoeAcm,
    "qoe_acm_v2": qoeAcmv2,
    "fang": fang,
    "fang10": fang10,
    "hrcc": hrcc,
    "bob": bob,
    "onrl": onrl,
    "combine_v1": combine_v1,
    "combine_v2": combine_v2,
    "combine_v3": combine_v3,
    "ahoy": ahoy,
}
