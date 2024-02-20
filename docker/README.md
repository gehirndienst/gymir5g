# Gymir5G docker 

This project serves as a reference point for dockerfiles to run Gymir5G project with a [full support of video and point clouds processing] together with OMNeT++ stack: [inet, simu5G, veins, SUMO] and python DRL pipeline: [OpenAI Gym, PyTorch, Stable-Baselines 3] via docker with X11 forwarding on the remote server.

# Build
## CPU
cd docker/ && chmod +x build-cpu && ./build-cpu

# Run
cd docker/ && chmod +x run-cpu && ./run-cpu
