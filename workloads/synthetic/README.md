# Generating Synthetic Workloads

## Overview
This folder contains the way of generating synthetic page-level Read/Write workloads with different skewness, drifting, and read/write ratio.

## Usage
Modify [workload_gen.cpp](workload_gen.cpp) with desired settings of skewness, drifting, and read/write ratio, then compile it using ```g++ -O3 -std=c++20 -o workload_gen workload_gen.cpp```. Run ```./workload_gen``` to generate workload file (.txt) and page ids (.pageids).

Workloads are named `<hot>hf<share>_<pages>_rw<ratio>_<requests>`, e.g. `5hf90_1e4_rw1_1e6`: 5% of the pages receive 90% of the accesses, 10^4 pages, read:write = 1:1, 10^6 requests. The generator draws its seed from `std::random_device`, so regenerated workloads differ from the bundled ones in the exact page IDs.
