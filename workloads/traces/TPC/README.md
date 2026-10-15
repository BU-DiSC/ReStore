# Processing benchmark traces TPC-C & TPC-E

## Overview
This folder contains the way of processing and converting TPC traces C&E into page-level Read/Write workloads

## Usage
Firstly, the TPC traces can be downloaded from [SNIA Trace Data Files](https://iotta.snia.org/traces/block-io/130). After extracting the csv files from the downloaded zip files, one can use the [Extract_DiskReadWrite.py](Extract_DiskReadWrite.py) to filter out only DiskRead/DiskWrite operations from the trace file (Note that the file name in the script might need to be changed).

Then use [Trace_to_pagerw.py](Trace_to_pagerw.py) to convert block I/O into page I/O.

After the page I/O workload being generated, run [workload_analysis.py](workload_analysis.py) to analyze the workload and generate the .pageids and .allpageids files containing the high-frequently used page ids and all page ids of the workload.

Keep the resulting `workload_<name>.txt`, `.allpageids`, and `.pageids` files anywhere under `workloads/` (subfolders are searched), and run the policies with `-workload <name>`; the parameters we used for each trace are in `trace_args()` of [`scripts/reproduce.sh`](../../../scripts/reproduce.sh).
