# `ReStore`: A Reinforcement Learning Approach for Data Migration in Multi-Tiered Storage

## Overview
This repository contains the implementation of `ReStore`, a reinforcement learning based data migration policy designed for multi-tiered storage systems. `ReStore` dynamically manages data migration across storage tiers by learning from workload patterns, device characteristics, and system states to optimize overall system performance while minimizing migration costs. A general workflow of `ReStore` is as follow:
![ReStore](figures/MTS-DBMS+RL.png?raw=true "Title")

## Build&Run
Go into ./cpp folder, then run make for building.
```
cd cpp
make -f Makefile
```
The Makefile is tested in Linux 6.4, with compiler g++ version 12, please adjust the Makefile according to your environment.

After executables being compiled, run ```./run_simulation.sh``` to start a simple simulation experiment.

## Workloads
Synthetic workloads are generated using [workload/synthetic/workload_gen.cpp](workload/synthetic/workload_gen.cpp), feel free to change the parameters to generate various workloads.

Workloads from real I/O traces such as TPC-C or Google Thesios, please refer the corresponding folders in /workload/
