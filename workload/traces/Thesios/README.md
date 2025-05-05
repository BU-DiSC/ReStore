# Processing banchmark traces Thesios

## Overview
This folder contains the way of processing and converting Google Thesios traces into page-level Read/Write workloads

## Usage
Firstly, the Thesios traces can be obtained from [google-research-datasets/thesios](https://github.com/google-research-datasets/thesios). After downloading the csv files, one can use [workload/traces/Thesios/Trace_to_pagerw.py](workload/traces/Thesios/Trace_to_pagerw.py) to convert block I/O into page I/O (Note that the file name in the script might need to be changed).

After the page I/O workload being generated, run [workload/traces/Thesios/workload_analysis.py](workload/traces/Thesios/workload_analysis.py) to analysis the workload and generate .pageid and .allpageid files containing the high-frequently used page ids and all page ids of the workload.
