# `ReStore`: A Reinforcement Learning Approach for Data Migration in Multi-Tiered Storage System

## Overview
This repository contains the implementation of `ReStore`, a reinforcement learning based data migration policy designed for multi-tiered storage systems. `ReStore` dynamically manages data migration across storage tiers by learning from workload patterns, device characteristics, and system states to optimize overall system performance while minimizing migration costs. A general workflow of `ReStore` is as follow:

![ReStore](figures/MTS-DBMS+RL.png?raw=true "Title")

## Build & Run
Go into ./cpp folder, then run make for compiling.
```
cd cpp
make -f Makefile
```
The Makefile is tested in Linux 6.4, with compiler g++ version 12, please adjust the Makefile according to your environment.

After executables being compiled, run ```./run_simulation.sh``` to start a simulation experiment. For experiments on I/O banchmark traces, see [workload/traces/run_traces.sh](workload/traces/run_traces.sh).

## Workloads
Synthetic workloads are generated using [workload/synthetic/workload_gen.cpp](workload/synthetic/workload_gen.cpp), change the parameters in order to generate various workloads.

For workloads from real I/O traces such as TPC-C and Google Thesios, please refer the corresponding folders in [workload/traces/](workload/traces/)

## Results
After running ```run_simulation.sh```, execution results will be generated in corresponding folders, then use the python scripts in [results/](results/) to generate figures.