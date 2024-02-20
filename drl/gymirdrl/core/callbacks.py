import copy
import csv
import datetime
import glob
import gymnasium
import multiprocessing
import numpy as np
import optuna
import os
import time
import threading
import warnings

from stable_baselines3.common.callbacks import BaseCallback, CheckpointCallback, EvalCallback
from stable_baselines3.common.evaluation import evaluate_policy
from stable_baselines3.common.monitor import Monitor
from stable_baselines3.common.vec_env import VecEnv, sync_envs_normalization

from gymirdrl.core.args import GymirArgs
from gymirdrl.core.env import GymirEnv
from gymirdrl.core.sim_runner import GymirSimRunner


class GymirCheckpointCallback(CheckpointCallback):
    """
    Save model with the given frequency and always at the end of training

    :param save_freq: Save checkpoints every ``save_freq`` call of the callback.
    :param save_path: Path to the folder where the model will be saved.
    :param name_prefix: Common prefix to the saved models
    :param save_replay_buffer: Save the model replay buffer
    :param save_vecnormalize: Save the ``VecNormalize`` statistics
    :param verbose: Verbosity level (0 -- 2)
    """

    def __init__(
        self,
        save_freq: int = 5000,
        save_path: str = "./models",
        name_prefix: str = "drl_model",
        save_replay_buffer: bool = True,
        save_vecnormalize: bool = True,
        verbose: int = 0,
    ):
        super().__init__(save_freq, save_path, name_prefix, save_replay_buffer, save_vecnormalize, verbose)
        self.save_freq = save_freq
        self.save_path = save_path
        self.name_prefix = name_prefix
        self.save_replay_buffer = save_replay_buffer
        self.save_vecnormalize = save_vecnormalize

    def _on_training_end(self):
        include = [
            "policy",
            "replay_buffer",
            "rollout_buffer",
            "_vec_normalize_env",
            "_episode_storage_logger",
            "_custom_logger",
        ]

        dt = datetime.datetime.now().strftime("%Y_%m_%d-%I_%M_%S_%p")
        model_path_full = os.path.join(self.save_path, f"{self.name_prefix}_{dt}_FULL.zip")
        model_path_default = os.path.join(self.save_path, f"{self.name_prefix}_{dt}_DEFAULT.zip")

        print("Saving final models...")
        self.model.save(model_path_full, exclude=["env"], include=include)
        self.model.save(model_path_default)

        # 1. save the last model additionally as last_full.zip and replace always with the newest trained version
        last_full_path = os.path.join(self.save_path, "last_full.zip")
        for filename in glob.glob(last_full_path):
            os.remove(filename)
        self.model.save(last_full_path, exclude=["env"], include=include)

        # 2. save also the last default model as last_default.zip wo replay and experience buffers as default saving by model.save()
        last_def_path = os.path.join(self.save_path, "last_default.zip")
        for filename in glob.glob(last_def_path):
            os.remove(filename)
        self.model.save(last_def_path)

        print("All models are successfully saved on training end, training is finished!")


class GymirPrintStepCallback(BaseCallback):
    """
    Prints each transition with all state vars and reward parts.
    For the clear and concise output, it works only with a single environment or with a 1-dim vectorized one.

    :param verbose: Verbosity level (0 -- 2)
    """

    def __init__(self, verbose: int = 0, eval_env: VecEnv | gymnasium.Env | None = None):
        super().__init__(verbose)
        self.start_time = datetime.datetime.now()
        self.eval_env = eval_env

    def _init_callback(self):
        self.env = self.eval_env if self.eval_env is not None else self.training_env
        if isinstance(self.env, VecEnv):
            if self.env.num_envs > 1:
                raise Exception("GymirPrintStepCallback works only for a single environment")

    def _on_step(self):
        if isinstance(self.env, VecEnv):
            episodes = self.env.get_attr("episodes")[0]
            steps = self.env.get_attr("steps")[0]
            last_action = self.env.get_attr("last_action")[0]
            state = self.env.get_attr("state")[0]
            rewards = self.env.get_attr("reward_parts")[0]
            is_finished = self.env.get_attr("is_finished")[0]
        else:
            episodes = self.env.episodes
            steps = self.env.steps
            last_action = self.env.last_action
            state = self.env.state
            rewards = self.env.reward_parts
            is_finished = self.env.is_finished

        state = {k: v.tolist() for k, v in state.items()}
        time_elapsed = datetime.datetime.now() - self.start_time

        if steps > 0 and not is_finished:
            if self.verbose > 0:
                # print on every step
                print(
                    "Training step info: \n "
                    + f"Time elapsed (hh:mm:ss.ms) {time_elapsed}"
                    + "\n"
                    + f"Episodes: {episodes}"
                    + "\n"
                    + f"Step: {steps},"
                    + "\n"
                    + f"Last action: {last_action},"
                    + "\n"
                    + f"State: {state},"
                    + "\n"
                    + f"Reward: {rewards}"
                    + "\n"
                )
            else:
                # print short version on every 100th step (and 1th as well)
                if steps == 1 or (steps >= 100 and steps % 100 == 0):
                    print(
                        "Training step info: \n "
                        + f"Time elapsed (hh:mm:ss.ms) {time_elapsed}"
                        + "\n"
                        + f"Episodes: {episodes}"
                        + "\n"
                        + f"Step: {steps},"
                        + "\n"
                    )

        return True


class GymirSaveStepCallback(BaseCallback):
    """
    Saves env step info to a csv file.
    For the clear and concise output, it works only with a single environment or with a 1-dim vectorized one.

    :param save_path: Path for the folder where csv files are saved.
    :param model_name: Current model name.
    :param verbose: Verbosity level (0 -- 2)
    """

    def __init__(
        self,
        save_path: str = "./logs",
        model_name: str = "",
        verbose: int = 0,
        eval_env: VecEnv | gymnasium.Env | None = None,
    ):
        super().__init__(verbose)
        self.save_path = save_path
        self.model_name = model_name
        self.eval_env = eval_env
        self.time = time.strftime("%Y%m%d-%H%M%S")

    def _init_callback(self):
        self.env = self.eval_env if self.eval_env is not None else self.training_env
        if isinstance(self.env, VecEnv):
            if self.env.num_envs > 1:
                raise Exception("GymirSaveStepCallback works only for a single environment")
        os.makedirs(self.save_path, exist_ok=True)
        self.file_handler = open(self._get_csv_filename(), mode="a", newline="\n")
        self.csv_writer = csv.DictWriter(self.file_handler, fieldnames=self._get_step_info().keys())
        if os.stat(self._get_csv_filename()).st_size == 0:
            self.csv_writer.writeheader()
        self.file_handler.flush()

    def _on_step(self):
        steps = self.env.get_attr("steps")[0] if isinstance(self.env, VecEnv) else self.env.steps
        if steps > 0:
            # might be a bug writing zero dummy step because of vecenv wrapper
            self.csv_writer.writerow(self._get_step_info())
        return True

    def _on_training_end(self):
        self.file_handler.close()

    def _get_csv_filename(self):
        return os.path.join(self.save_path, f"drl_training_{self.model_name}_{self.time}.csv")

    def _get_step_info(self):
        if isinstance(self.env, VecEnv):
            return self.env.env_method("get_step_info")[0]
        else:
            return self.env.get_step_info()


class GymirBreakCallback(BaseCallback):
    """
    Provides a congruity between the training and observation generation (e.g., via a simulation) for breaking cases:
        a) if env(s) reached the max episode(s) (controlled with 'max_episodes' param), breaks the training
        b) if training is interrupted by some other callbacks (e.g., by some eval callback), tells corresponding env to stop.
    This callback supports multiple environments and must be always attached in case of an ipc with a simulation.

    :param verbose: Verbosity level (0 -- 2)
    """

    def __init__(self, verbose: int = 0):
        super().__init__(verbose)

    def _on_step(self):
        if all(self._is_env_finished()):
            print("GymirBreakCallback: All env(s) reached the max episode(s), breaking the training...")
            return False
        else:
            return True

    def _on_training_end(self):
        finished_envs = self._is_env_finished()
        if not all(finished_envs):
            print("GymirBreakCallback: training was interrupted, trigger env(s) to stop the sim process(es)...")
            unfinished_env_idxs = [i for i, x in enumerate(finished_envs) if not x]
            self._trigger_sim_stop(unfinished_env_idxs)

    def _is_env_finished(self):
        if isinstance(self.training_env, VecEnv):
            return self.training_env.get_attr("is_finished")
        else:
            return [self.training_env.is_finished]

    def _trigger_sim_stop(self, unfinished_indices):
        if isinstance(self.training_env, VecEnv):
            for env_idx in unfinished_indices:
                self.training_env.env_method("close")[env_idx]
        else:
            self.training_env.close()


class GymirAsyncEvalCallback(EvalCallback):
    """
    EvalCallback that fully copies sb3's EvalCallback with a one slight change:
    It calls and waits for evaluate_policy results in async mode, so it does not block and break the training during the evaluation

    :param eval_args: The arguments used for initialization of a sim runner for the env
    :param callback_on_new_best: Callback to trigger
        when there is a new best model according to the mean_reward
    :param callback_after_eval: Callback to trigger after every evaluation
    :param n_eval_episodes: The number of episodes to test the agent
    :param eval_freq: Evaluate the agent every eval_freq call of the callback.
    :param log_path: Path to a folder where the evaluations (evaluations.npz)
        will be saved. It will be updated at each evaluation.
    :param best_model_save_path: Path to a folder where the best model
        according to performance on the eval env will be saved.
    :param deterministic: Whether the evaluation should
        use a stochastic or deterministic actions.
    :param render: Whether to render or not the environment during evaluation
    :param verbose: Verbosity level: 0 for no output, 1 for indicating information about evaluation results
    :param warn: Passed to evaluate_policy (warns if eval_env has not been wrapped with a Monitor wrapper)
    """

    def __init__(
        self,
        eval_args: GymirArgs,
        callback_on_new_best: BaseCallback | None = None,
        callback_after_eval: BaseCallback | None = None,
        n_eval_episodes: int = 5,
        eval_freq: int = 10000,
        log_path: str | None = None,
        best_model_save_path: str | None = None,
        deterministic: bool = True,
        render: bool = False,
        verbose: int = 1,
        warn: bool = True,
    ):
        self.eval_args = eval_args
        self.eval_env = self._make_eval_env()
        self.eval_model = None
        self.eval_num_timesteps = 0
        self.eval_queue = multiprocessing.Queue()
        self.eval_thread = None
        self.n_evals = 0

        super().__init__(
            self.eval_env,
            callback_on_new_best,
            callback_after_eval,
            n_eval_episodes,
            eval_freq,
            log_path,
            best_model_save_path,
            deterministic,
            render,
            verbose,
            warn,
        )

    def _make_eval_env(self) -> gymnasium.Env:
        sim_runner = GymirSimRunner(
            sim_path=self.eval_args.sim_path,
            num_runs=self.eval_args.runs,
            from_run=self.eval_args.from_run,
            same_run=self.eval_args.same_run,
            scenario=self.eval_args.scenario,
            time_limit=self.eval_args.time_limit,
            streams_config_file=self.eval_args.streams_cfg,
            state_update_period=self.eval_args.update_period,
            sim_host=self.eval_args.sim_host,
            sim_port=self.eval_args.sim_port,
            adaptive_algorithm=self.eval_args.adaptive_algorithm,
            stream_log_dir="null",
            is_use_veins=self.eval_args.is_use_veins,
            name="eval_cb",
            std_output=None,
        )

        return Monitor(
            GymirEnv(
                mdp_config=self.eval_args.env_cfg,
                sim_runner=sim_runner,
                max_episodes=self.eval_args.runs,
            )
        )

    def _on_step(self) -> bool:
        if self.eval_freq > 0 and self.n_calls % self.eval_freq == 0:
            # sb3 init code
            if self.model.get_vec_normalize_env() is not None:
                try:
                    sync_envs_normalization(self.training_env, self.eval_env)
                except AttributeError as e:
                    raise AssertionError(
                        "Training and eval env are not wrapped the same way, "
                        "see https://stable-baselines3.readthedocs.io/en/master/guide/callbacks.html#evalcallback "
                        "and warning above."
                    ) from e
            self._is_success_buffer = []

            ### a thread executes evaluate policy and pushes the results to the queue
            # NOTE: only one thread! so that eval must be short and performed not so often
            if self.eval_thread is None or not self.eval_thread.is_alive():
                # a dirty hack to avoid doing a deepcopy of the model that raises an exception
                tmp_model_file = os.path.join("", "tmp_model")
                include = [
                    "policy",
                    "replay_buffer",
                    "rollout_buffer",
                    "_vec_normalize_env",
                    "_episode_storage_logger",
                    "_custom_logger",
                ]
                exclude = ["env"]
                self.model.save(tmp_model_file, exclude=exclude, include=include)
                self.eval_model = type(self.model).load(tmp_model_file)
                self.eval_env = self._make_eval_env()
                self.eval_num_timesteps = copy.deepcopy(self.num_timesteps)
                self.eval_thread = threading.Thread(
                    target=lambda: self.eval_queue.put(
                        (
                            evaluate_policy(
                                self.eval_model,
                                self.eval_env,
                                n_eval_episodes=self.n_eval_episodes,
                                render=self.render,
                                deterministic=self.deterministic,
                                return_episode_rewards=True,
                                warn=self.warn,
                                callback=self._log_success_callback,
                            )
                        )
                    ),
                    daemon=True,
                )
                self.eval_thread.start()
                os.remove(tmp_model_file + ".zip")
            else:
                warnings.warn(
                    f"GymirAsyncEvalCallback: you requested new evaluation but the previous one is not finished yet! "
                    f"Either make eval_freq smaller or check eval_thread, it could be in a livelock"
                )

        # continue_training is examined only when evaluate_policy executison is finished
        if self.eval_thread is not None and not self.eval_thread.is_alive() and not self.eval_queue.empty():
            return self._on_finished_eval()
        return True

    def _on_finished_eval(self) -> bool:
        self.eval_thread.join()
        self.n_evals += 1
        print(
            f"{self.n_evals} evaluation is finished, "
            f"eval ts: {self.eval_num_timesteps}, current ts: {self.num_timesteps}"
        )
        episode_rewards, episode_lengths = self.eval_queue.get()
        return self._on_finished_eval_sb3(episode_rewards, episode_lengths)

    def _on_finished_eval_sb3(self, episode_rewards, episode_lengths) -> bool:
        continue_training = True

        # sb3 code after evaluate_policy call except using frozen eval_model and eval_num_timesteps.
        if self.log_path is not None:
            self.evaluations_timesteps.append(self.eval_num_timesteps)
            self.evaluations_results.append(episode_rewards)
            self.evaluations_length.append(episode_lengths)

            kwargs = {}
            # Save success log if present
            if len(self._is_success_buffer) > 0:
                self.evaluations_successes.append(self._is_success_buffer)
                kwargs = dict(successes=self.evaluations_successes)

            np.savez(
                self.log_path,
                timesteps=self.evaluations_timesteps,
                results=self.evaluations_results,
                ep_lengths=self.evaluations_length,
                **kwargs,
            )

        mean_reward, std_reward = np.mean(episode_rewards), np.std(episode_rewards)
        mean_ep_length, std_ep_length = np.mean(episode_lengths), np.std(episode_lengths)
        self.last_mean_reward = mean_reward

        if self.verbose >= 1:
            print(f"Eval num_timesteps={self.eval_num_timesteps}")
            print(f"Episode_reward={mean_reward:.2f} +/- {std_reward:.2f}")
            print(f"Episode length: {mean_ep_length:.2f} +/- {std_ep_length:.2f}")
        # Add to current Logger
        self.logger.record("eval/mean_reward", float(mean_reward))
        self.logger.record("eval/mean_ep_length", mean_ep_length)

        if len(self._is_success_buffer) > 0:
            success_rate = np.mean(self._is_success_buffer)
            if self.verbose >= 1:
                print(f"Success rate: {100 * success_rate:.2f}%")
            self.logger.record("eval/success_rate", success_rate)

        if mean_reward > self.best_mean_reward:
            if self.verbose >= 1:
                print("New best mean reward!")
            if self.best_model_save_path is not None:
                self.eval_model.save(os.path.join(self.best_model_save_path, "best_model"))
            self.best_mean_reward = mean_reward
            # Trigger callback on new best model, if needed
            if self.callback_on_new_best is not None:
                continue_training = self.callback_on_new_best.on_step()

        # Trigger callback after every evaluation, if needed
        if self.callback is not None:
            continue_training = continue_training and self._on_event()
        print(f"Continue training: {continue_training}")
        return continue_training

    def _on_training_end(self) -> None:
        self.eval_env.close()


class GymirAsyncTrialEvalCallback(GymirAsyncEvalCallback):
    """
    Asynchronous EvalCallback with optuna trial reporting

    :param eval_args: The arguments used for initialization of a sim runner for the env
    :param trial: Optuna's Trial object
    :param callback_on_new_best: Callback to trigger
        when there is a new best model according to the mean_reward
    :param callback_after_eval: Callback to trigger after every evaluation
    :param n_eval_episodes: The number of episodes to test the agent
    :param eval_freq: Evaluate the agent every eval_freq call of the callback.
    :param log_path: Path to a folder where the evaluations (evaluations.npz)
        will be saved. It will be updated at each evaluation.
    :param best_model_save_path: Path to a folder where the best model
        according to performance on the eval env will be saved.
    :param deterministic: Whether the evaluation should
        use a stochastic or deterministic actions.
    :param render: Whether to render or not the environment during evaluation
    :param verbose: Verbosity level: 0 for no output, 1 for indicating information about evaluation results
    :param warn: Passed to evaluate_policy (warns if eval_env has not been wrapped with a Monitor wrapper)
    """

    def __init__(
        self,
        eval_args: GymirArgs,
        trial: optuna.trial.Trial,
        callback_on_new_best: BaseCallback | None = None,
        callback_after_eval: BaseCallback | None = None,
        n_eval_episodes: int = 5,
        eval_freq: int = 10000,
        log_path: str | None = None,
        best_model_save_path: str | None = None,
        deterministic: bool = True,
        render: bool = False,
        verbose: int = 1,
        warn: bool = True,
    ):
        super().__init__(
            eval_args,
            callback_on_new_best,
            callback_after_eval,
            n_eval_episodes,
            eval_freq,
            log_path,
            best_model_save_path,
            deterministic,
            render,
            verbose,
            warn,
        )
        self.trial = trial
        self.is_pruned = False

    def _on_finished_eval_sb3(self, episode_rewards, episode_lengths) -> bool:
        super()._on_finished_eval_sb3(episode_rewards, episode_lengths)

        self.trial.report(self.last_mean_reward, self.n_evals)
        if self.trial.should_prune():
            self.is_pruned = True
            return False
        return True


class GymirEvaluatingCallback:
    """
    auxilary class for self-producing a default callback for an old way of calling in evaluate_policy() function.
    All other evaluating callbacks must inherit this class.
    """

    def __init__(self, env, save_path: str = "./logs", model_name: str = "", verbose: int = 0):
        self.step_callback = GymirSaveStepCallback(save_path, model_name, verbose, env)
        self.step_printing_callback = GymirPrintStepCallback(verbose, env)
        self.is_env_resetted = False
        self.verbose = verbose

    def on_step(self):
        if not self.is_env_resetted:
            if self.verbose >= 1:
                self.step_printing_callback._init_callback()
            if self.verbose == 2:
                self.step_callback._init_callback()
            self.is_env_resetted = True
        if self.verbose >= 1:
            self.step_printing_callback._on_step()
        if self.verbose == 2:
            self.step_callback._on_step()
        return True
