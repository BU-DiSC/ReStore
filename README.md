# `ReStore`: A Reinforcement Learning Approach for Data Migration in Multi-Tiered Storage System

## Overview
This repository contains the implementation of `ReStore`, a reinforcement learning based data migration policy designed for multi-tiered storage systems. `ReStore` dynamically manages data migration across storage tiers by learning from workload patterns, device characteristics, and system states to optimize overall system performance while minimizing migration costs. A general workflow of `ReStore` is as follow:

![ReStore](figures/MTS-DBMS+RL.png?raw=true "Title")

## Build & Run
Use the following steps to quickly install ```restore```.
```
git clone <repository-url>
cd ReStore
chmod +x install.sh
sudo ./install.sh
```
After installation you can valid by running ```restore --help```

For customized installation, please refer to [INSTALLATION_GUIDE.md](INSTALLATION_GUIDE.md) and [PACKAGE_README.md](PACKAGE_README.md).

The package is tested in Linux 6.4/Ubuntu 22.04, with compiler gcc/g++ version 9/12, please adjust the Makefile according to your environment.

After executables being compiled, run ```examples/run_simulation.sh``` to start a simulation experiment. For experiments on I/O banchmark traces, see [workload/traces/run_traces.sh](workload/traces/run_traces.sh).

## Denpendency
The XGB policy is implemented using the C++ library [XGBoost-FastForest](https://github.com/guitargeek/XGBoost-FastForest.git) for deploying XGBoost model, please install it first to avoid compiling errors.

As for the storage tier, ```restore``` by default use ```/dev/nvme0n1``` ```/dev/nvme1n1``` ```/dev/nvme2n1``` as the three tiers, please adjust to your environment by changing them in [/cpp/Sibyl_driver/Sibyl_lib_configurable.c][/cpp/Sibyl_driver/Sibyl_lib_configurable.c], or build with custom device paths via ```restore build -fast_device DEVICE1_PATH -middle_device DEVICE2_PATH -slow_device DEVICE3_PATH```.

## Workloads
Synthetic workloads are generated using [workload/synthetic/workload_gen.cpp](workload/synthetic/workload_gen.cpp), change the parameters in order to generate various workloads.

For workloads from real I/O traces such as TPC-C and Google Thesios, please refer the corresponding folders in [workload/traces/](workload/traces/)

## Results
After running ```run_simulation.sh```, execution results will be generated in corresponding folders, then use the python scripts in [results/](results/) to generate figures.
