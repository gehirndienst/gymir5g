import concurrent.futures as cf
import os
import psutil
import socket
import subprocess
import time
from typing import List


class GymirSimRunner:
    """a class to concurrently run Gymir5G OMNeT++ simulation in a non-blocking way"""

    def __init__(
        self,
        # run script params, look at any run.sh script in gymir5g/simulations/*/
        sim_path: str,
        num_runs: int,
        scenario: str,
        time_limit: int,
        streams_config_file: str,
        from_run: int = 0,
        same_run: int = -1,
        state_update_period: float = 1.0,
        sim_host: str = "127.0.0.1",
        sim_port: int = 5555,
        adaptive_algorithm: str = "drl_base",
        stream_log_dir: str = "",
        is_use_veins: bool = False,
        is_view: bool = False,
        # class params
        name: str = "sim",
        max_timeout: float = 2.0,
        std_output: str = None,
        busy_ports: List[int] = [],
    ):
        self.sim_dir, self.sim_file = os.path.split(sim_path)
        self.num_runs = num_runs
        self.from_run = from_run
        self.same_run = same_run
        self.scenario = scenario
        self.time_limit = time_limit
        self.streams_config_file = streams_config_file
        self.state_update_period = state_update_period
        self.adaptive_algorithm = adaptive_algorithm
        self.stream_log_dir = stream_log_dir
        self.is_use_veins = is_use_veins
        self.is_view = is_view

        self.sim_host = sim_host
        self.sim_port = sim_port
        self.busy_ports = busy_ports

        self.sim_address = self._get_valid_sim_address()
        self.cmd = self._build_cmd()

        self.name = name
        self.max_timeout = max_timeout
        self.std_output = std_output

        self.sim_process = None
        self.veins_process = None

    def start(self):
        # run the simulation
        try:
            if self.std_output:
                with open(self.std_output, "w+") as std_output:
                    self.sim_process = subprocess.Popen(
                        self.cmd,
                        cwd=self.sim_dir,
                        stdout=std_output,
                        stderr=subprocess.STDOUT,
                    )
            else:
                self.sim_process = subprocess.Popen(
                    self.cmd,
                    cwd=self.sim_dir,
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL,
                )

            # run SUMO daemon via veins proxy if needed
            if self.is_use_veins:
                sumo_home, veins_home = self.get_veins_env_vars()
                veins_cmd = ["./veins_launchd", "-vv", "-c", os.path.join(sumo_home, "bin", "sumo")]
                self.veins_process = subprocess.Popen(
                    veins_cmd,
                    cwd=os.path.join(veins_home, "bin"),
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL,
                )
                # check that SUMO daemon is running by checking its default port
                time.sleep(0.5)
                if self.check_if_port_is_free(9999):
                    self.stop()
                    raise Exception("SimRunner: SUMO daemon process has NOT been started via veins proxy")

            # check if the simulation is actually running
            if not self.check_if_process_is_running_with_timeout("gymir5g", self.max_timeout):
                self.stop()
                raise TimeoutError(f"Failed to find Gymir sim process {self.name} during specified timeout")
        except FileNotFoundError:
            raise FileNotFoundError(f"SimRunner: Failed to find Gymir OMNeT++ simulation in dir {self.sim_dir}")
        except Exception as e:
            raise Exception(f"SimRunner: Error starting a sim process, msg: {e}")

    def stop(self):
        if self.sim_process is None:
            print(f"SimRunner: Sim process {self.name} is not running")
            return
        try:
            if self.check_if_process_is_running_with_timeout("gymir5g", self.max_timeout):
                raise TimeoutError(f"Failed to stop sim process {self.name} during specified timeout")
        except Exception as e:
            print(f"SimRunner: Sim process {self.name} hasn't stopped automatically, msg: {e}, killing..")
            try:
                # process.kill() may not work for scripts chain, this always works
                subprocess.run(['kill', str(self.get_pid_by_name("gymir5g"))])
            except Exception as e:
                raise Exception(f"SimRunner: Error stopping sim process {self.name} with kill(), msg: {e}")
        finally:
            if self.veins_process is not None:
                try:
                    self.veins_process.kill()
                except Exception as e:
                    raise Exception(f"SimRunner: Error stopping veins proces with kill(), msg: {e}")
            self.veins_process = None
            self.sim_process = None

    def is_running(self):
        return False if self.sim_process is None else self.sim_process.poll() is None

    def _build_cmd(self):
        return [
            "./run",
            "-c",
            self.scenario,
            "-r",
            str(self.num_runs),
            "-f",
            str(self.from_run),
            "-s",
            str(self.same_run),
            "-t",
            str(self.time_limit),
            "-o",
            self.sim_file,
            "-sc",
            self.streams_config_file,
            "-su",
            str(self.state_update_period),
            "-host",
            self.sim_address,
            "-ad",
            str(self.adaptive_algorithm),
            "-l",
            self.stream_log_dir,
            "-view",
            "true" if self.is_view else "false",
        ]

    def _get_valid_sim_address(self):
        # check if the given port for an ipc is free: if not, add + 1 and check the next one until a free port is found
        is_valid_sim_address = False
        initial_port = self.sim_port
        while not is_valid_sim_address:
            if not self.check_if_port_is_free(self.sim_port, self.sim_host) or self.sim_port in self.busy_ports:
                self.sim_port += 1
            else:
                is_valid_sim_address = True
                if initial_port != self.sim_port:
                    print(f"SimRunner: the initial port {initial_port} was busy, found a free port at {self.sim_port}")
        return self.sim_host + ":" + str(self.sim_port)

    @staticmethod
    def check_if_process_is_running_with_timeout(process_name, max_timeout=3.0):
        # a dirty hack: check ps aux output, won't work on Windows
        start_time = time.time()
        while time.time() - start_time < max_timeout:
            for process in psutil.process_iter():
                if process.name() == process_name:
                    return True
            time.sleep(max_timeout / 10)
        return False

    @staticmethod
    def get_pid_by_name(process_name):
        for process in psutil.process_iter():
            if process.name() == process_name:
                return process.pid
        return None

    @staticmethod
    def check_if_port_is_free(port, host="localhost"):
        # check that the given port is free before opening the corresponding subprocess(es)
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            try:
                s.bind((host, port))
                return True
            except OSError:
                return False

    @staticmethod
    def get_veins_env_vars():
        sumo_home = os.environ.get("SUMO_HOME")
        veins_home = os.environ.get("VEINS_HOME")
        if sumo_home is None:
            raise Exception("SimRunner: SUMO_HOME environment variable is not set, can't run veins proxy")
        if veins_home is None:
            raise Exception("SimRunner: VEINS_HOME environment variable is not set, can't run veins proxy")
        return sumo_home, veins_home


class GymirSimRunnerPool:
    """a pool used to parallelize the execution of multiple sim runners, doesn't block the current thread"""

    def __init__(self, sim_runners: list[GymirSimRunner]):
        self.sim_runners = sim_runners

    def run(self):
        with cf.ProcessPoolExecutor(max_workers=len(self.sim_runners)) as executor:
            self.futures = []
            for sim_runner in self.sim_runners:
                self.futures.append(executor.submit(sim_runner.start))
                time.sleep(0.1)  # add a little delay in order not to mess with sim loggers

            for future in cf.as_completed(self.futures):
                try:
                    _ = future.result()
                except Exception as e:
                    print(f"GymirSimRunnerPool: An exception occurred in parallel execution of SimRunners, msg: {e}")
                    return
