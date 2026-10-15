# Processing benchmark traces Thesios

## Overview
This folder contains the way of processing and converting Google Thesios traces into page-level Read/Write workloads

## Usage
Firstly, the Thesios traces can be obtained from [google-research-datasets/thesios](https://github.com/google-research-datasets/thesios). After downloading the csv files, one can use [Trace_to_pagerw.py](Trace_to_pagerw.py) to convert block I/O into page I/O (Note that the file name in the script might need to be changed).

After the page I/O workload being generated, run [workload_analysis.py](workload_analysis.py) to analyze the workload and generate the .pageids and .allpageids files containing the high-frequently used page ids and all page ids of the workload.

Keep the resulting `workload_<name>.txt`, `.allpageids`, and `.pageids` files anywhere under `workloads/` (subfolders are searched), and run the policies with `-workload <name>`; the parameters we used for each trace are in `trace_args()` of [`scripts/reproduce.sh`](../../../scripts/reproduce.sh).
