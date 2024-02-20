import gymnasium
import math
import numpy as np
from gymnasium.spaces import Box, Space
from dataclasses import dataclass, field
from enum import Enum, auto
from typing import Any, Dict, List, Optional, Self, Tuple

"""

To add a new state variable one needs:
    1) (optional) implement additional variable(s) by adapting the observation source and its saving in one of cpp *State classes 
    2) (optional) add a new kv pair to ::toJson method of the class from 1) to serialize it
    3) (optional) add new variable(s) to Observation sub-dataclasses depending on the cpp class from 1) to parse it 
    4) add a new instance variable to StateGenerator class and its name to VARS class list to process it, (optional) add new constant(s) to CONSTANTS class list if needed
    5) implement its computation based on the observation by adding a case in StateGenerator.compute_variable() method
    6) (optional) set its observation space by adding an if-case in StateGenerator.generate_observation_space() method if it requires an individual space shape/type
    6) define its usage in the Gymnasium environment (namely in the reward function, see reward.py)
    
"""


class SpecialRequest(Enum):
    """
    Enum representing special request types
    """

    INITIAL = auto()
    OFF = auto()
    TERMINATED = auto()
    TRUNCATED = auto()
    NO = auto()


@dataclass
class Transmission:
    """
    Dataclass representing transmission statistics. Look at DataTransmissionState.h
    """

    numTxPackets: int = 0
    numTxBytes: int = 0
    numRxPackets: int = 0
    rxGoodput: float = 0.0
    rxFecGoodput: float = 0.0
    numOutOfOrderPackets: int = 0
    numLostPackets: int = 0
    lossRate: float = 0.0
    fractionLossRate: float = 0.0
    numPlayedPackets: int = 0
    numRepairedPackets: int = 0
    numRetransmittedPackets: int = 0
    numRepairedAndRetransmittedPackets: int = 0
    numAckPackets: int = 0
    numNackPackets: int = 0
    numPliPackets: int = 0
    numFecPackets: int = 0
    numFecBytes: int = 0
    interarrivalJitter: float = 0.0
    fractionAvTransmissionDelay: float = 0.0
    fractionAvRetransmissionDelay: float = 0.0
    fractionAvPlayoutDelay: float = 0.0
    fractionStallingRate: float = 0.0
    fractionRtt: float = 0.0
    rttJitter: float = 0.0
    deltas: List[Dict[str, int | float]] = field(default_factory=lambda: [])
    numReceiverReportPackets: int = 0
    numTransportFeedbacks: int = 0
    lastReceiverReportId: int = 0
    lastTransportFeedbackId: int = 0
    timeCurrentReceiverReport: float = 0.0
    timeCurrentTransportFeedback: float = 0.0


@dataclass
class Stream:
    """
    Dataclass representing stream statistics. Look at StreamState.h
    """

    streamName: str = "stream"
    elemsSent: int = 0
    elems: List[int] = field(default_factory=lambda: [])
    qualities: List[int] = field(default_factory=lambda: [])
    encodingTimes: List[float] = field(default_factory=lambda: [])
    queueingTimes: List[float] = field(default_factory=lambda: [])


@dataclass
class Observation:
    """
    Dataclass representing an observation with transmission and stream information.
    """

    transmission: Optional[Transmission] = None
    stream: Optional[Stream] = None
    time: float = 0.0
    special_request: SpecialRequest = SpecialRequest.NO

    @classmethod
    def from_recv(cls, recv_payload) -> Self:
        """
        Create an Observation object from a received payload.

        :param recv_payload: (dict-like) received payload
        :returns: an Observation object
        """
        special_requests: List[SpecialRequest] = []
        for special_key in SpecialRequest:
            if special_key.name.lower() in recv_payload and recv_payload[special_key.name.lower()]:
                # applied to observation in such a form: {..., "initial": true, ...}, could be more than 1 SR
                special_requests.append(special_key)
        if SpecialRequest.INITIAL in special_requests:
            return cls(special_request=SpecialRequest.INITIAL)
        else:
            return cls(
                transmission=Transmission(**recv_payload["transmission"]),
                stream=Stream(**recv_payload["stream"]),
                time=recv_payload["time"],
                special_request=Observation.select_main_special_request(special_requests),
            )

    def get_val(self, key: str) -> Optional[Any]:
        """
        Get a value from the Observation or its sub-objects.

        :param key: (str) the name of the attribute to retrieve.
        :returns: the attribute value or None if not found.
        """
        for obj in [self, self.transmission, self.stream]:
            if obj is not None:
                if key in vars(obj):
                    return getattr(obj, key)
        return None

    @staticmethod
    def select_main_special_request(special_requests: List[SpecialRequest]) -> SpecialRequest:
        """
        Select the most important special request if multiple ones arrived. The hierarchy is the following:
        TERMINATED (ret) > TRUNCATED (ret) > OFF > ...

        :param special_requests: nullable list of incoming special requests
        :returns: the main special request
        """
        if len(special_requests) == 0:
            return SpecialRequest.NO
        elif len(special_requests) == 1:
            return special_requests[-1]
        else:
            # hierachy: if TERMINATED or TRUNCATED in a list return immediately, else most likely wrong settings
            if SpecialRequest.TERMINATED in special_requests:
                return SpecialRequest.TERMINATED
            elif SpecialRequest.TRUNCATED in special_requests:
                return SpecialRequest.TRUNCATED
            else:
                raise Exception(
                    "Observation.select_special_request: a weird and disallowed combination of special"
                    f" requests has come: {special_requests}"
                )

    @property
    def is_ok(self) -> bool:
        """
        Check if the Observation is valid.
        :returns: True if valid, False otherwise.
        """
        return self.transmission is not None and self.stream is not None


class StateGenerator:
    """
    Class for generating DRL state. Initialized with a list of valid vars and constants (optional)

    NOTE: new vars should follow some naming convention as in existing vars: camel case is for cpp-incoming values,
    underscores are for python-defined ones.
    """

    MAX_RATE: float = 50.0  # Mbit/s
    MAX_DELAY: float = 1000.0  # ms
    MAX_JITTER: float = 625.0  # ms or RTP timestamp units
    MAX_PLAYOUT_DELAY: float = 250.0  # ms
    MAX_NACKS: int = 2  # num
    FEEDBACK_PERIOD: float = 0.1  # sec
    REPORT_PERIOD: float = 1.0  # sec

    CONSTANTS = [
        "MAX_RATE",
        "MAX_DELAY",
        "MAX_JITTER",
        "MAX_PLAYOUT_DELAY",
        "MAX_NACKS",
        "FEEDBACK_PERIOD",
        "REPORT_PERIOD",
    ]

    VARS = [
        "rxGoodput",
        "txGoodput",
        "rxFecGoodput",
        "txFecGoodput",
        "lossRate",
        "fractionLossRate",
        "retransmissionRate",
        "fractionRetransmissionRate",
        "fractionAvRetransmissionDelay",
        "repairRate",
        "fractionRepairRate",
        "nackRate",
        "fractionNackRate",
        "nackSuccessRate",
        "fractionNackSuccessRate",
        "fractionPliRate",
        "playRate",
        "fractionPlayRate",
        "fractionAvPlayoutDelay",
        "fractionStallingRate",
        "deltasSequence",
        "fractionAvTransmissionDelay",
        "fractionRtt",
        "fractionQueueingRtt",
        "gradientRtt",
        "rttIqr",
        "rttMean",
        "rttStd",
        "fractionAvInterarrivalDelay",
        "interarrivalJitter",
        "interarrivalRttJitter",
        "fractionAvInterarrivalJitter",
    ]

    # dims for CNN/LSTM layers
    DELTA_SEQUENCE_MAX_LENGTH = 100
    DELTA_FEATURES_DIM = 2

    def __init__(
        self,
        state_vars: List[str],
        state_constants: Optional[Dict[str, int | float]] = None,
        is_scale: bool = False,
        is_fill_nulls_with_running_limits: bool = True,
    ) -> None:
        """
        :param state_vars: list of state variables to process
        :param state_constants: optional dict of state constants to rewrite
        :param is_scale: whether to minimax scale the state variables to the static limits if available
        :param is_fill_nulls_with_running_limits: whether to fill nulls with running limits
            (e.g., when no delays are delivered, the average delay can be set to the max delay over the current episode), currently only for delays
        :returns: None
        """
        # memento the initial state
        self.reset()

        self.state_vars: List[str] = self._get_valid_state_vars(state_vars)
        self.is_scale: bool = is_scale
        self.is_fill_nulls_with_running_limits: bool = is_fill_nulls_with_running_limits

        # rewrite constants if needed
        if state_constants is not None:
            for str_var in self.CONSTANTS:
                if str_var in state_constants or str_var.lower() in state_constants:
                    setattr(self, str_var.upper(), state_constants[str_var])

    def reset(self) -> None:
        """
        Memento pattern to reset StreamGenerator by setting vars to their defaults (called during env.reset())
        """
        ############## VARS ############################
        # goodput
        self.rxGoodput: float = 0.0
        self.txGoodput: float = 0.0
        self.rxFecGoodput: float = 0.0
        self.txFecGoodput: float = 0.0

        # loss
        self.lossRate: float = 0.0
        self.fractionLossRate: float = 0.0

        # retransmission
        self.retransmissionRate: float = 0.0
        self.fractionRetransmissionRate: float = 0.0
        self.fractionAvRetransmissionDelay: float = 0.0

        # repair
        self.repairRate: float = 0.0
        self.fractionRepairRate: float = 0.0

        # nack
        self.nackRate: float = 0.0
        self.fractionNackRate: float = 0.0
        self.nackSuccessRate: float = 0.0
        self.fractionNackSuccessRate: float = 0.0

        # pli
        self.fractionPliRate: float = 0.0

        # play
        self.playRate: float = 0.0
        self.fractionPlayRate: float = 0.0
        self.fractionAvPlayoutDelay: float = 0.0
        self.fractionStallingRate: float = 0.0

        # network delays / jitters
        self.deltasSequence: List[List[float]] = [[0.0, 0.0]]  # shape (_, 2)
        self.fractionAvTransmissionDelay: float = 0.0
        self.fractionRtt: float = 0.0
        self.fractionQueueingRtt: float = 0.0
        self.gradientRtt: float = 0.0
        self.rttIqr: float = 0.0
        self.rttMean: float = 0.0
        self.rttStd: float = 0.0
        self.interarrivalRttJitter: float = 0.0
        self.fractionAvInterarrivalDelay: float = 0.0
        self.interarrivalJitter: float = 0.0
        self.fractionAvInterarrivalJitter: float = 0.0
        ################################################

        # public params
        self.state: Dict[str, Any] = {}
        self.scaled_state: Dict[str, Any] = {}
        self.observation_space: gymnasium.spaces.Dict = None
        self.observations: List[Observation] = []

        # iter params
        self.last_obs: Observation = None
        self.last_time: float = 0.0
        self.time_passed: float = 0.0
        # feedback/report receive rate (computed always and in is_missed_too_much() call)
        self.feedback_rate: float = 0.0
        self.report_rate: float = 0.0

        # flags
        self.is_last_obs: bool = False
        self.is_nonzero_time: float = False
        self.is_nonzero_rx_packets = False
        self.is_missed_too_much: bool = False

        # auxilary vars/arrays
        self.indices: List[int] = []
        self.deltas: List[float] = []
        self.aux_vars: Dict[Any] = {
            "numRxPackets": 0,
            "minRtt": 1.0,
            "maxRtt": 0.0,
            "rtts": [],
            "minInterarrivalDelay": 0.0,
            "maxInterarrivalDelay": 0.0,
        }
        self.is_rtt_counted: bool = False

    def make_state(self, observation: Observation) -> Tuple[Dict[str, Any], SpecialRequest]:
        """
        Make a new DRL state from an observation.

        :param observation: observation to process.
        :returns: a tuple containing the generated state and a special request from observation source if was any.
        In case is_scale=True, the normalized state is returned instead
        """
        if not observation.is_ok:
            return {}, observation.special_request

        self.last_obs = self.observations[-1] if self.observations else None
        self.is_last_obs = self.last_obs is not None
        self.last_time = self.last_obs.time if self.is_last_obs else 0.0
        self.time_passed = observation.time - self.last_time
        self.is_nonzero_time = self.time_passed > 0
        self.is_nonzero_rx_packets = observation.transmission.numRxPackets > 0
        self.is_missed_too_much = self._is_missed_too_much(observation)
        if self.is_need_to_parse_feedbacks:
            self._parse_feedbacks(observation.transmission.deltas)
        self.observations.append(observation)
        self.is_rtt_counted = False

        for var_name in self.state_vars:
            self.state[var_name], self.scaled_state[var_name] = self.compute_variable(var_name, observation)

        if self.is_scale:
            return self.scaled_state, observation.special_request
        else:
            return self.state, observation.special_request

    def compute_variable(self, state_var: str, observation: Observation) -> Tuple[Any, Any]:
        """
        Compute a state variable based on the given observation

        :param state_var: The name of the state variable to compute.
        :param observation: The current observation.
        :returns: a tuple of a computed value of the state variable and its scaled version.
        """
        if not hasattr(self, state_var):
            raise ValueError(
                f"StateGenerator.compute_variable: Variable '{state_var}' does not exist in StateGenerator class"
            )

        # TODO: get rid of the overhead by computing both state and norm state, keep both only during the testing phase
        match state_var:
            # goodput
            case "rxGoodput":
                self.rxGoodput = StateGenerator.clip(observation.transmission.rxGoodput, 0.0, self.MAX_RATE)
                return self.rxGoodput, StateGenerator.scale(self.rxGoodput, 0.0, self.MAX_RATE)
            case "txGoodput":
                last_numTxBytes = self.last_obs.transmission.numTxBytes if self.is_last_obs else 0.0
                self.txGoodput = StateGenerator.clip(
                    (
                        (observation.transmission.numTxBytes - last_numTxBytes) * 8e-6 / self.time_passed
                        if self.is_nonzero_time
                        else 0.0
                    ),
                    0.0,
                    self.MAX_RATE,
                )
                return self.txGoodput, StateGenerator.scale(self.txGoodput, 0.0, self.MAX_RATE)
            case "rxFecGoodput":
                self.rxFecGoodput = StateGenerator.clip(observation.transmission.rxFecGoodput, 0.0, self.MAX_RATE)
                return self.rxFecGoodput, StateGenerator.scale(self.rxFecGoodput, 0.0, self.MAX_RATE)
            case "txFecGoodput":
                last_numFecBytes = self.last_obs.transmission.numFecBytes if self.is_last_obs else 0.0
                self.txFecGoodput = StateGenerator.clip(
                    (
                        (observation.transmission.numFecBytes - last_numFecBytes) * 8e-6 / self.time_passed
                        if self.is_nonzero_time
                        else 0.0
                    ),
                    0.0,
                    self.MAX_RATE,
                )
                return self.txFecGoodput, StateGenerator.scale(self.txFecGoodput, 0.0, self.MAX_RATE)

            # loss
            case "lossRate":
                self.lossRate = StateGenerator.clip(observation.transmission.lossRate, 0.0, 1.0)
                return self.lossRate, self.lossRate
            case "fractionLossRate":
                self.fractionLossRate = StateGenerator.clip(observation.transmission.fractionLossRate, 0.0, 1.0)
                return self.fractionLossRate, self.fractionLossRate

            # retransmission
            case "retransmissionRate":
                self.retransmissionRate = StateGenerator.clip(
                    (
                        observation.transmission.numRetransmittedPackets / observation.transmission.numRxPackets
                        if self.is_nonzero_rx_packets
                        else 0.0
                    ),
                    0.0,
                    1.0,
                )
                return self.retransmissionRate, self.retransmissionRate
            case "fractionRetransmissionRate":
                last_retransmitted = self.last_obs.transmission.numRetransmittedPackets if self.is_last_obs else 0.0
                last_received = self.last_obs.transmission.numRxPackets if self.is_last_obs else 0.0
                self.fractionRetransmissionRate = StateGenerator.clip(
                    (
                        (observation.transmission.numRetransmittedPackets - last_retransmitted)
                        / (observation.transmission.numRxPackets - last_received)
                        if observation.transmission.numRxPackets - last_received > 0
                        else 0.0
                    ),
                    0.0,
                    1.0,
                )
                return self.fractionRetransmissionRate, self.fractionRetransmissionRate
            case "fractionAvRetransmissionDelay":
                self.fractionAvRetransmissionDelay = StateGenerator.clip(
                    observation.transmission.fractionAvRetransmissionDelay, 0.0, self.MAX_DELAY
                )
                return self.fractionAvRetransmissionDelay, StateGenerator.scale(
                    self.fractionAvRetransmissionDelay, 0.0, self.MAX_DELAY
                )

            # repair
            case "repairRate":
                self.repairRate = StateGenerator.clip(
                    (
                        observation.transmission.numRepairedPackets
                        / (observation.transmission.numRxPackets + observation.transmission.numRepairedPackets)
                        if self.is_nonzero_rx_packets
                        else 0.0
                    ),
                    0.0,
                    1.0,
                )
                return self.repairRate, self.repairRate
            case "fractionRepairRate":
                last_received = self.last_obs.transmission.numRxPackets if self.is_last_obs else 0.0
                last_repaired = self.last_obs.transmission.numRepairedPackets if self.is_last_obs else 0.0
                self.fractionRepairRate = StateGenerator.clip(
                    (
                        (observation.transmission.numRepairedPackets - last_repaired)
                        / (
                            observation.transmission.numRxPackets
                            - last_received
                            + observation.transmission.numRepairedPackets
                            - last_repaired
                        )
                        if observation.transmission.numRxPackets
                        - last_received
                        + observation.transmission.numRepairedPackets
                        - last_repaired
                        > 0
                        else 0.0
                    ),
                    0.0,
                    1.0,
                )
                return self.fractionRepairRate, self.fractionRepairRate

            # nack
            case "nackRate":
                self.nackRate = StateGenerator.clip(
                    (
                        observation.transmission.numNackPackets
                        / (self.MAX_NACKS * observation.transmission.numRxPackets)
                        if self.is_nonzero_rx_packets
                        else 0.0
                    ),
                    0.0,
                    1.0,
                )
                return self.nackRate, self.nackRate
            case "fractionNackRate":
                last_nack = self.last_obs.transmission.numNackPackets if self.is_last_obs else 0.0
                last_sent = self.last_obs.transmission.numTxPackets if self.is_last_obs else 0.0
                self.fractionNackRate = StateGenerator.clip(
                    (
                        (observation.transmission.numNackPackets - last_nack)
                        / (observation.transmission.numTxPackets - last_sent)
                        if observation.transmission.numTxPackets - last_sent > 0
                        else 0.0
                    ),
                    0.0,
                    1.0,
                )
                return self.fractionNackRate, self.fractionNackRate
            case "nackSuccessRate":
                self.nackSuccessRate = StateGenerator.clip(
                    (
                        observation.transmission.numRetransmittedPackets / observation.transmission.numNackPackets
                        if observation.transmission.numNackPackets > 0
                        else 0.0
                    ),
                    0.0,
                    1.0,
                )
                return self.nackSuccessRate, self.nackSuccessRate
            case "fractionNackSuccessRate":
                last_nack = self.last_obs.transmission.numNackPackets if self.is_last_obs else 0.0
                last_retransmitted = self.last_obs.transmission.numRetransmittedPackets if self.is_last_obs else 0.0
                self.fractionNackSuccessRate = StateGenerator.clip(
                    (
                        (observation.transmission.numRetransmittedPackets - last_retransmitted)
                        / (observation.transmission.numNackPackets - last_nack)
                        if observation.transmission.numNackPackets - last_nack > 0
                        else 0.0
                    ),
                    0.0,
                    1.0,
                )
                return self.fractionNackSuccessRate, self.fractionNackSuccessRate
            # pli
            case "fractionPliRate":
                last_pli = self.last_obs.transmission.numPliPackets if self.is_last_obs else 0.0
                last_sent = self.last_obs.transmission.numTxPackets if self.is_last_obs else 0.0
                self.fractionPliRate = StateGenerator.clip(
                    (
                        (observation.transmission.numPliPackets - last_pli)
                        / (observation.transmission.numTxPackets - last_sent)
                        if observation.transmission.numTxPackets - last_sent > 0
                        else 0.0
                    ),
                    0.0,
                    1.0,
                )
                return self.fractionPliRate, self.fractionPliRate
            # play
            case "playRate":
                self.playRate = StateGenerator.clip(
                    (
                        observation.transmission.numPlayedPackets
                        / (observation.transmission.numRxPackets + observation.transmission.numRepairedPackets)
                        if self.is_nonzero_rx_packets
                        else 0.0
                    ),
                    0.0,
                    1.0,
                )
                return self.playRate, self.playRate
            case "fractionPlayRate":
                last_received = self.last_obs.transmission.numRxPackets if self.is_last_obs else 0.0
                last_repaired = self.last_obs.transmission.numRepairedPackets if self.is_last_obs else 0.0
                last_played = self.last_obs.transmission.numPlayedPackets if self.is_last_obs else 0.0
                self.fractionPlayRate = StateGenerator.clip(
                    (
                        (observation.transmission.numPlayedPackets - last_played)
                        / (
                            observation.transmission.numRxPackets
                            - last_received
                            + observation.transmission.numRepairedPackets
                            - last_repaired
                        )
                        if observation.transmission.numRxPackets
                        - last_received
                        + observation.transmission.numRepairedPackets
                        - last_repaired
                        > 0
                        else 0.0
                    ),
                    0.0,
                    1.0,
                )
                return self.fractionPlayRate, self.fractionPlayRate
            case "fractionAvPlayoutDelay":
                self.fractionAvPlayoutDelay = StateGenerator.clip(
                    observation.transmission.fractionAvPlayoutDelay, 0.0, self.MAX_PLAYOUT_DELAY
                )
                return self.fractionAvPlayoutDelay, StateGenerator.scale(
                    self.fractionAvPlayoutDelay, 0.0, self.MAX_PLAYOUT_DELAY
                )
            case "fractionStallingRate":
                self.fractionStallingRate = StateGenerator.clip(observation.transmission.fractionStallingRate, 0.0, 1.0)
                return self.fractionStallingRate, self.fractionStallingRate

            # delay
            case "deltasSequence":
                self.deltasSequence = [[]]
                normDeltasSequence = [[]]
                for i in range(len(self.deltas)):
                    jitter = (
                        StateGenerator.clip(abs(self.deltas[i] - self.deltas[i - 1]), 0.0, self.MAX_JITTER)
                        if i > 0
                        else 0.0
                    )
                    self.deltasSequence.append([self.deltas[i], jitter])
                    if self.is_scale:
                        normDeltasSequence.append(
                            [
                                StateGenerator.scale(self.deltas[i], 0.0, self.MAX_DELAY),
                                StateGenerator.scale(jitter, 0.0, self.MAX_JITTER),
                            ]
                        )
                return self.deltasSequence, normDeltasSequence
            case "fractionAvTransmissionDelay":
                self.fractionAvTransmissionDelay = StateGenerator.clip(
                    observation.transmission.fractionAvTransmissionDelay, 0.0, self.MAX_DELAY
                )
                return self.fractionAvTransmissionDelay, StateGenerator.scale(
                    self.fractionAvTransmissionDelay, 0.0, self.MAX_DELAY
                )
            case "fractionRtt":
                self.fractionRtt = StateGenerator.clip(observation.transmission.fractionRtt, 0.0, self.MAX_DELAY)
                if self.fractionRtt > 0:
                    self._count_rtt(self.fractionRtt)
                else:
                    if self.is_fill_nulls_with_running_limits:
                        self.fractionRtt = self.aux_vars["maxRtt"]
                return self.fractionRtt, StateGenerator.scale(self.fractionRtt, 0.0, self.MAX_DELAY)
            case "fractionQueueingRtt":
                self.fractionRtt = StateGenerator.clip(observation.transmission.fractionRtt, 0.0, self.MAX_DELAY)
                self.fractionQueueingRtt = self.fractionRtt - self.aux_vars["minRtt"]
                return self.fractionQueueingRtt, StateGenerator.scale(self.fractionQueueingRtt, 0.0, self.MAX_DELAY)
            case "rttIqr":
                self.fractionRtt = StateGenerator.clip(observation.transmission.fractionRtt, 0.0, self.MAX_DELAY)
                self._count_rtt(self.fractionRtt)
                if len(self.aux_vars["rtts"]) > 0:
                    self.rttIqr = np.subtract(*np.percentile(self.aux_vars["rtts"], [75, 25]))
                return self.rttIqr, StateGenerator.scale(self.rttIqr, 0.0, self.MAX_DELAY)
            case "rttMean":
                self.fractionRtt = StateGenerator.clip(observation.transmission.fractionRtt, 0.0, self.MAX_DELAY)
                self._count_rtt(self.fractionRtt)
                if len(self.aux_vars["rtts"]) > 0:
                    self.rttMean = np.mean(self.aux_vars["rtts"])
                return self.rttMean, StateGenerator.scale(self.rttMean, 0.0, self.MAX_DELAY)
            case "rttStd":
                self.fractionRtt = StateGenerator.clip(observation.transmission.fractionRtt, 0.0, self.MAX_DELAY)
                self._count_rtt(self.fractionRtt)
                if len(self.aux_vars["rtts"]) > 0:
                    self.rttStd = np.std(self.aux_vars["rtts"])
                return self.rttStd, StateGenerator.scale(self.rttStd, 0.0, self.MAX_DELAY)
            case "interarrivalRttJitter":
                self.interarrivalRttJitter = StateGenerator.clip(
                    observation.transmission.rttJitter, 0.0, self.MAX_JITTER
                )
                return self.interarrivalRttJitter, StateGenerator.scale(
                    self.interarrivalRttJitter, 0.0, self.MAX_JITTER
                )
            case "gradientRtt":
                # TODO: remove this var? Otherwise can violate the rtt counting logic
                self.fractionRtt = StateGenerator.clip(observation.transmission.fractionRtt, 0.0, self.MAX_DELAY)
                if self.fractionRtt > 0:
                    self.aux_vars["minRtt"] = min(self.aux_vars["minRtt"], self.fractionRtt)
                    self.aux_vars["maxRtt"] = max(self.aux_vars["maxRtt"], self.fractionRtt)
                    self.aux_vars["rtts"].append(self.fractionRtt)
                else:
                    if self.is_fill_nulls_with_running_limits:
                        self.fractionRtt = self.aux_vars["maxRtt"]
                last_rtt = self.last_obs.transmission.fractionRtt if self.is_last_obs else 0.0
                self.gradientRtt = self.fractionRtt - last_rtt if last_rtt > 0.0 else 0.0
                return self.gradientRtt, StateGenerator.scale(self.gradientRtt, 0.0, self.MAX_DELAY)
            case "fractionAvInterarrivalDelay":
                self.fractionAvInterarrivalDelay = sum(self.deltas) / len(self.deltas) if len(self.deltas) > 0 else 0.0
                if self.fractionAvInterarrivalDelay > 0:
                    self.aux_vars["minInterarrivalDelay"] = min(
                        self.aux_vars["minInterarrivalDelay"], self.fractionAvInterarrivalDelay
                    )
                    self.aux_vars["maxInterarrivalDelay"] = max(
                        self.aux_vars["maxInterarrivalDelay"], self.fractionAvInterarrivalDelay
                    )
                else:
                    if self.is_fill_nulls_with_running_limits:
                        self.fractionAvInterarrivalDelay = self.aux_vars["maxInterarrivalDelay"]
                return self.fractionAvInterarrivalDelay, StateGenerator.scale(
                    self.fractionAvInterarrivalDelay, 0.0, self.MAX_DELAY
                )

            case "fractionAvInterarrivalJitter":
                num_deltas = len(self.deltas)
                self.fractionAvInterarrivalJitter = (
                    sum(
                        StateGenerator.clip(abs(self.deltas[i] - self.deltas[i - 1]), 0.0, self.MAX_JITTER)
                        for i in range(1, num_deltas)
                    )
                    / (num_deltas - 1)
                    if num_deltas > 1
                    else 0.0
                )
                if self.fractionAvInterarrivalJitter == 0 and self.is_fill_nulls_with_running_limits:
                    self.fractionAvInterarrivalJitter = self.aux_vars["maxInterarrivalDelay"]
                return self.fractionAvInterarrivalJitter, StateGenerator.scale(
                    self.fractionAvInterarrivalJitter, 0.0, self.MAX_JITTER
                )
            case "interarrivalJitter":
                self.interarrivalJitter = StateGenerator.clip(
                    observation.transmission.interarrivalJitter, 0.0, self.MAX_JITTER
                )
                return self.interarrivalJitter, StateGenerator.scale(self.interarrivalJitter, 0.0, self.MAX_JITTER)

            # default
            case _:
                raise ValueError(f"StateGenerator.compute_variable: No suitable computation for var '{state_var}'")

    def generate_observation_space(self) -> gymnasium.spaces.Dict:
        """
        Generate a Gymnasium space for state variables

        :returns: gymnasium.spaces.Dict that can be assigned directly to env.observation_space
        """
        spaces: Dict[str, Space] = {}
        for state_var in self.state_vars:
            if not self.is_scale:
                if "Goodput" in state_var:
                    # goodput-like
                    spaces[state_var] = Box(low=0.0, high=self.MAX_RATE, shape=(1,), dtype=np.float32)
                    continue
                elif "Rate" in state_var:
                    # rate-like
                    spaces[state_var] = Box(low=0.0, high=1.0, shape=(1,), dtype=np.float32)
                    continue
                elif "Delay" in state_var or "Rtt" in state_var:
                    # delay-like box
                    if state_var == "avPlayoutDelay":
                        spaces[state_var] = Box(low=0.0, high=self.MAX_PLAYOUT_DELAY, shape=(1,), dtype=np.float32)
                    else:
                        spaces[state_var] = Box(low=0.0, high=self.MAX_DELAY, shape=(1,), dtype=np.float32)
                    continue
                elif "Jitter" in state_var:
                    # jitter-like box
                    spaces[state_var] = Box(low=0.0, high=self.MAX_JITTER, shape=(1,), dtype=np.float32)
                    continue
                elif state_var == "deltasSequence":
                    # for all sequence-like vars you need an individual if case
                    spaces[state_var] = Box(
                        low=0.0,
                        high=[self.MAX_DELAY, self.MAX_JITTER, self.MAX_DELAY],
                        shape=(self.DELTA_SEQUENCE_MAX_LENGTH, self.DELTA_FEATURES_DIM),
                        dtype=np.float32,
                    )
            else:
                if "deltasSequence" in state_var:
                    spaces[state_var] = Box(
                        low=0.0,
                        high=1.0,
                        shape=(self.DELTA_SEQUENCE_MAX_LENGTH, self.DELTA_FEATURES_DIM),
                        dtype=np.float32,
                    )
                else:
                    spaces[state_var] = Box(low=0.0, high=1.0, shape=(1,), dtype=np.float32)

        self.observation_space = gymnasium.spaces.Dict(spaces)
        return self.observation_space

    def _get_valid_state_vars(self, state_vars: List[str]) -> List[str]:
        """
        Validate a list of given state variables

        :param state_vars: The list of state variables.
        :returns: List of filtered state variables which exist in the class (are instance variables)
        """
        valid_state_vars = []
        for state_var in state_vars:
            if hasattr(self, state_var):
                valid_state_vars.append(state_var)
        # do we have sequential inputs?
        self.is_need_to_parse_feedbacks: bool = any(
            var in valid_state_vars
            for var in ["deltasSequence", "fractionAvInterarrivalDelay", "fractionAvInterarrivalJitter"]
        )
        return valid_state_vars

    def _is_missed_too_much(self, curr_obs: Observation) -> bool:
        """
        Check if too many reports or feedbacks are missed.

        :param curr_obs: The current observation.
        :returns: True if too many reports or feedbacks are missed, False otherwise.
        """
        num_reports = (
            curr_obs.transmission.numReceiverReportPackets - self.last_obs.transmission.numReceiverReportPackets
            if self.is_last_obs
            else curr_obs.transmission.numReceiverReportPackets
        )
        num_reports_should_be = math.floor(self.time_passed / self.REPORT_PERIOD)
        self.report_rate = num_reports / num_reports_should_be if num_reports_should_be > 0.0 else 0.0
        num_feedbacks = (
            curr_obs.transmission.numTransportFeedbacks - self.last_obs.transmission.numTransportFeedbacks
            if self.is_last_obs
            else curr_obs.transmission.numTransportFeedbacks
        )
        num_feedbacks_should_be = math.floor(self.time_passed / self.FEEDBACK_PERIOD)
        self.feedback_rate = num_feedbacks / num_feedbacks_should_be if num_feedbacks_should_be > 0.0 else 0.0
        # 1. less than 50% of RRs are received -- too few info
        if num_reports == 0 or self.report_rate < 0.5:
            return True
        # 2. less than 33% of feedbacks are received -- too few info
        if num_feedbacks == 0 or self.feedback_rate < 0.33:
            return True
        # 3. rx rate is 0
        if curr_obs.transmission.rxGoodput == 0.0:
            return True
        return False

    def _parse_feedbacks(self, recv_deltas: List[Dict[str, int | float]]) -> None:
        """
        Parse feedback information from received TransportFeedback delta chunks.

        :param recv_deltas: List of received interarrival deltas in ms.
        """
        self.indices = []
        self.deltas = []
        for delta_dict in recv_deltas:
            self.indices.append(delta_dict["tfnum"])
            self.deltas.append(StateGenerator.clip(delta_dict["delay"], 0.0, self.MAX_DELAY))

    def _count_rtt(self, rtt: float) -> None:
        if not self.is_rtt_counted:
            self.is_rtt_counted = True
            self.aux_vars["rtts"].append(rtt)
            self.aux_vars["minRtt"] = min(self.aux_vars["minRtt"], rtt)
            self.aux_vars["maxRtt"] = max(self.aux_vars["maxRtt"], rtt)

    @staticmethod
    def clip(val: int | float, min_: int | float, max_: int | float) -> int | float:
        return min_ if val < min_ else max_ if val > max_ else val

    @staticmethod
    def scale(val: int | float, min_: int | float, max_: int | float) -> int | float:
        return (val - min_) / (max_ - min_) if min_ < max_ else 0.0
