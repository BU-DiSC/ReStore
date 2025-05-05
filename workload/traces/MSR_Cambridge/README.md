# Processing banchmark traces MSR Cambridge

## Overview
This folder contains the way of processing and converting MSR Cambridge traces into page-level Read/Write workloads

## Usage
Firstly, the MSR Cambridge traces can be downloaded from [SNIA Trace Data Files](https://iotta.snia.org/traces/block-io/388). After extracting the csv files from the downloaded zip files, one can use [workload/traces/MSR_Cambridge/Trace_to_pagerw.py](workload/traces/MSR_Cambridge/Trace_to_pagerw.py) to convert block I/O into page I/O (Note that the file name in the script might need to be changed).

After the page I/O workload being generated, run [workload/traces/MSR_Cambridge/workload_analysis.py](workload/traces/MSR_Cambridge/workload_analysis.py) to analysis the workload and generate .pageid and .allpageid files containing the high-frequently used page ids and all page ids of the workload.
