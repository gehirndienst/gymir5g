import json
import numpy as np
import time
import zmq
from collections import OrderedDict
from typing import Any, List

from gymnasium import Env
from gymnasium.spaces import Box, Discrete, MultiDiscrete, Dict, Space

from gymirdrl.core.observation import Observation, StateGenerator, SpecialRequest
from gymirdrl.core.reward import get_reward_function_class
from gymirdrl.core.sim_runner import GymirSimRunner


class GymirEnv(Env):
    def __init__(self, mdp_config: str | dict[str, Any], runner: GymirSimRunner, max_episodes: int = 0):
        super(GymirEnv, self).__init__()

        '''
        mdp_config is either a str with a json file or a python dictionary with a following struct:
        mdp_config = {
            "state_vars": List[str], look StateGenerator.VARS
            "state_constants: Optional[List[str]], look StateGenerator.CONSTANTS
            "reward_function": str, default "best",
            "action_space": str, default "Discrete(5)",
            "is_scale": bool, default False (min-max scale),
            "is_fill_nulls_with_running_limits": bool, default True,fill nulls with running min/max values, delays-only
        }
        '''
        if isinstance(mdp_config, str):
            with open(mdp_config, "r") as mdp_config_json_file:
                self.mdp_config = json.load(mdp_config_json_file)
        else:
            self.mdp_config = mdp_config

        self.state_generator = StateGenerator(
            self.mdp_config["state_vars"],
            self.mdp_config.get("state_constants", None),
            self.mdp_config.get("is_scale", False),
            self.mdp_config.get("is_fill_nulls_with_running_limits", True),
        )
        self.state: OrderedDict = {}

        if "reward_function" in self.mdp_config:
            self.reward_function = get_reward_function_class(self.mdp_config["reward_function"])
        else:
            self.reward_function = get_reward_function_class()
        self.reward = 0.0
        self.reward_parts = {}

        self.observation_space: Dict = self.state_generator.generate_observation_space()
        self.action_space: Space = eval(self.mdp_config["action_space"])  # FIXME: apply a better solution

        self.steps = 0
        self.episodes = 0
        self.max_episodes = max_episodes
        self.last_action = None

        self.runner = runner
        self.ipc_address = "tcp://" + self.runner.sim_address
        self.context = None
        self.socket = None

        self.is_paired_with_runner = False
        self.is_finished = False

    def step(self, action):
        # send action to the simulation
        self.last_action = action
        self.socket.send_json({"actions": self._cast_actions(action)})

        if self.is_finished:
            # in case you somehow got here, return a dummy truncated state
            return self._get_initial_space_sample(), 0.0, False, True, {}

        # receive and parse a new observation via IPC
        recv_payload = self.socket.recv_json()
        observation = Observation.from_recv(recv_payload)

        # make a python-dict state from the parsed observation
        state_dict, special_request = self.state_generator.make_state(observation)
        if special_request == SpecialRequest.INITIAL:
            # TODO: remake it to raise a special Exception in utils which could be caught upstream to reinitialize runner with another port
            raise Exception(f"cannot parse an observation in step method because it has INITIAL request")

        terminated = self.is_terminated() or special_request == SpecialRequest.TERMINATED
        truncated = special_request == SpecialRequest.TRUNCATED

        self.state = self._get_space_sample(state_dict)
        self.reward = self._get_reward()
        self.steps += 1

        return self.state, self.reward, terminated, truncated, {}

    def reset(self, seed=None, options={}):
        super().reset(seed=seed)

        self.episodes += 1

        # last episode reached: return a dummy state as an output, training will be interrupted by a callback
        if self.max_episodes > 0 and self.episodes > self.max_episodes:
            print(f"GymirEnv on {self.ipc_address} reached its maximal episode {self.max_episodes}, closing..")
            self.is_finished = True
            self.socket.send_json({"finish": True})
            self._stop()
            # vec enc has to save the last dummy obs so that it should have all "normal" keys
            return self._get_initial_space_sample(), {}

        if not self.is_paired_with_runner:
            # start the simulation on the first episode
            self._start()
        else:
            # send reset flag to a simulation and wait for it to start again for the next episode
            print(f"GymirEnv on {self.ipc_address} has started episode {self.episodes}, resetting..")
            self.socket.send_json({"reset": True})
            time.sleep(self.runner.max_timeout)

        # receive initial state from the simulation
        try:
            recv_payload = self.socket.recv_json()
            initial_observation = Observation.from_recv(recv_payload)
        except zmq.ZMQError:
            raise Exception(f"ZMQError, probably the address {self.ipc_address} is already occupied")

        if initial_observation.special_request == SpecialRequest.INITIAL:
            self.is_paired_with_runner = True
        else:
            raise Exception("no 'initial' request has been received, aborting..")

        # initialize MDP
        self.state_generator.reset()
        self.last_action = None
        self.reward_parts = {}
        self.reward = 0
        self.steps = 0
        self.state = self._get_initial_space_sample()

        return self.state, {}

    def is_terminated(self):
        # there are no terminal states yet
        return False

    def get_step_info(self) -> dict[str, Any]:
        # get all env variables after step execution in a pretty-printing way
        general_dict = {"step": self.steps, "episode": self.episodes, "action": self.last_action}
        state_dict = {
            f"state/{k}": (
                v[0] if (isinstance(v, List) and len(v) == 1) or (isinstance(v, np.ndarray) and v.size == 1) else v
            )
            for k, v in self.state.items()
        }
        if not self.reward_parts:
            # call with initial state just to fill a dict
            self._get_reward()
        rewards_dict = {f"reward/{k}": v for k, v in self.reward_parts.items()}
        return general_dict | state_dict | rewards_dict

    def _get_reward(self) -> float:
        self.reward, self.reward_parts = self.reward_function(self.state_generator, self.last_action)
        return self.reward

    def _get_space_sample(self, state_dict: dict) -> OrderedDict:
        tuples = []
        for key, space in self.observation_space.items():
            if isinstance(space, Box):
                if isinstance(state_dict[key], List) or isinstance(state_dict[key], np.ndarray):
                    value = np.array(state_dict[key], dtype=space.dtype).reshape(space.shape)
                else:
                    value = np.array([state_dict[key]], dtype=space.dtype).reshape(space.shape)
            elif isinstance(space, MultiDiscrete):
                if isinstance(state_dict[key], List) or isinstance(state_dict[key], np.ndarray):
                    value = np.array(state_dict[key], dtype=space.dtype).reshape(space.nvec.shape)
                else:
                    value = np.array([state_dict[key]], dtype=space.dtype).reshape(space.nvec.shape)
            else:
                value = state_dict[key]
            tuples.append((key, value))
        return OrderedDict(tuples)

    def _get_initial_space_sample(self) -> OrderedDict:
        tuples = []
        for key, space in self.observation_space.items():
            if isinstance(space, Box):
                value = np.zeros(space.shape, dtype=space.dtype)
            elif isinstance(space, MultiDiscrete):
                value = np.zeros(space.nvec.shape, dtype=space.dtype)
            elif isinstance(space, Discrete):
                value = 0
            tuples.append((key, value))
        return OrderedDict(tuples)

    def _cast_actions(self, numpy_action):
        # cast an action to a python list to have a generic solution for all possible action spaces and also to use standard json encoder
        if isinstance(numpy_action, np.integer):
            return [int(numpy_action)]
        elif isinstance(numpy_action, np.floating):
            return [float(numpy_action)]
        elif isinstance(numpy_action, np.ndarray):
            return numpy_action.tolist()
        else:
            raise Exception("neither np.int/float not np.array supplied as an action -- other types are forbidden")

    def _start(self):
        # start the runner, all waiting and checking routine is done in the start method
        self.runner.start()
        if not self.runner.is_running():
            raise Exception("GymirEnv._start: GymirSimRunner is not running!")

        # bind to zmq
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.REP)
        self.socket.bind(self.ipc_address)
        print(f"GymirEnv has succesfully initialized a runner and is listening to: {self.ipc_address}")

    def _stop(self, is_immediate=False):
        # stop the runner on the last episode or by interruption from a callback (checked in close() method)
        if not is_immediate:
            # give 5 seconds to correctly finish herself (in case it is not forcefully interrupted)
            time.sleep(3.0)

        if self.runner.is_running():
            self.runner.stop()

        try:
            self.socket.close()
            self.context.term()
        except zmq.ZMQError:
            raise Exception(f"GymirEnv on {self.ipc_address} can't properly finish its ZMQ socket and context")

        # check either after a wait above or after a wait inside sim_runner.stop() method.
        # A sim is either finished by herself or terminated by the sim_runner. If it is still active, throw an exception
        if not self.runner.is_running():
            self.is_paired_with_runner = False
        else:
            raise Exception("GymirEnv._stop: A simulation process is stopped neither by himself nor by sim runner!")

    def close(self):
        if self.is_paired_with_runner:
            self._stop()
            self.is_finished = True
            print(f"GymirEnv on {self.ipc_address} is successfully closed during an interruption event..")
        else:
            print(f"GymirEnv on {self.ipc_address} is successfully closed")
