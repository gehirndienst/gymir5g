An official repository for the code for the paper:

Nikita Smirnov, Sven Tomforde: Real-Time Rate Control of WebRTC Video Streams in 5G Networks: Improving Quality of Experience with Deep Reinforcement Learning, Journal of Systems Architecture (in process)

## Installation (DOCKER ONLY)
Pull docker image from Docker Hub:
```
docker pull gehirndienst/gymir5g_arcs23:latest
```
then run a container:
```
docker run -it --rm --name gymir5g_arcs23 gehirndienst/gymir5g_arcs23:latest bash
```

attach vs-code to the container and install vs-code python and jupyter extensions. Selected pre-installed poetry environment as python interpreter

## Usage
### gymir5g
Gymir5G is still under the active development and its source code is not yet published. However, there is a pre-built execution file of the latest snapshot to the date of paper submission (16.10.2023) that can be used in a container to run the simulation.

### gymirdrl
The full code for python part is presented in `gymirdrl` folder.

### experiments
This folder contains the code, which was used to test the trained Soft Actor-Critic model for the paper (/arcs23). It is copied to the container and is located in /opt/omnetpp-workspace/gymir5g/experiments/arcs23 folder.

- logs_plots folder contains the experiments results and plots referenced in ARCS23 paper. It also contains the Jupyter notebook (plot.ipynb) used to generate the plots.
- training_logs folder contains the logs, metric, tb and omnetpp logs from the training.
- env_cfg.json is the configuration file for the environment.
- hparam_cfg.json is the configuration file for the hyperparameters used in the training.
- model.zip is the trained sb3 SAC model.
- paper.ipynb is the jupyter notebook with the code used to test the models and GCC on 3 validation scenarios mentioned in ARCS23 paper. NOTE: works only in the above mentioned container.
- stream_cfg.json is the configuration file for the stream used in the experiments. NOTE: it contains some not-omitted parameters that are not used in the experiments for ARCS23 paper.

### gcc
This folder contains the C++ code for the GCC (Google Congestion Control) implementation used in the experiments for ARCS23 paper. Based on [https://github.com/thegreatwb/HRCC/](https://github.com/thegreatwb/HRCC/).

## License
GPLv3 (see LICENSE file)

## Authors
Nikita Smirnov, Sven Tomforde
Department of Computer Science, University of Kiel, Germany

If you have any questions, please contact Mr. Nikita Smirnov (nsm@informatik.uni-kiel.de)