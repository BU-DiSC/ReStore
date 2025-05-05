# Generating Synthetic Workloads

## Overview
This folder contains the way of generating synthetic page-level Read/Write workloads with different skewness, drifting, and read/write ratio.

## Usage
Modify [workload/synthetic/workload_gen.cpp](workload/synthetic/workload_gen.cpp) with desired settings of skewness, drifting, and read/write ratio, then compile it using ```g++ -O3 -std=c++20 workload_gen workload_gen.cpp```. Run ```./workload_gen``` to generate workload file (.txt) and page ids (.pageids).
