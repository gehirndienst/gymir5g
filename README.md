# PREFACE
If you are looking for the repo for the paper Nikita Smirnov and Sven Tomforde: **Real-time rate control of WebRTC video streams in 5G networks: Improving quality of experience with Deep Reinforcement Learning**, *Journal of Systems Architecture, vol 148, 2024*, it has moved to another repo, please follow the link: [https://github.com/gehirndienst/gymir5g_jsa](https://github.com/gehirndienst/gymir5g_jsa). This repo contains the full source code for Gymir5G that was used in this paper in a compiled form for the evaluation of the DRL model.

# Gymir5G
A code for the paper: Nikita Smirnov and Sven Tomforde: **Gymir5G: A simulation platform to study data transmission in 5G networks with Deep Learning assistance**

# Installation
Please go to the [docker](docker) folder and follow the instructions there.

# Docs
The full documentation is in the process of being written... please read the paper for the details of the project.

# Usage
In the `src` folder you will find the source  C++ code for all the OMNeT++-related components of the Gymir5G project. Please follow the "unwrapping" approach and start inspecting the code from the `apps` folder. It contains the sender and the receiver applications. The code for these applications includes the code for the stream management, the code for the RTC set and the code for the communicator and adaptive streaming.

In the `simulations` folder you will find examples of the scenarios mentioned in the paper. Please follow the paper to understand the scenarios and the outcomes. Inspect NED and INI files to understand the scenarios and the parameters that one can set for the applications.

In the `drl` folder you will find the code for the DRL part of the project, namely a subproject gymirdrl. It includes the code for the Gymnasium environment, the code for the DRL agent in stable-baselines3, the code for receiving and processing the observation from the simulation and the code for sending the action to the simulation. Also reward shaping and the code for the training and testing of the DRL agent are included. Please refer to the `run.py` and `run.sh` files for the usage of the DRL part of the project (inside the container). It will run the DRL agent and the simulation in parallel and will show/save the results of the DRL agent's training.

# License
This project is licensed under the GPLv3 License - see the [LICENSE](LICENSE) file for details.

# Author
M.Sc. Nikita Smirnov, Intelligent Systems AG, Department of Computer Science, Kiel University, Germany.

Please contact me in case of any questions or bug reports: [Nikita Smirnov](mailto:nsm@informatik.uni-kiel.de)