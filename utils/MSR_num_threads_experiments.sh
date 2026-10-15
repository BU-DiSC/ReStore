#!/bin/bash

# Raw access to the tier block devices needs root; sudo prompts once and caches credentials.
run_as_root() {
    sudo env "PATH=$PATH" "$@"
}

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# # First precondition the FEMU SSDs using fio
# echo "Preconditioning FEMU SSDs via fio precondition_3nvme.fio"
# run_as_root fio "$REPO_ROOT/utils/precondition_3nvme.fio"

# Change to ReStore directory
cd "$REPO_ROOT"

# list of workloads
workloads=("MSR_hm_1" "MSR_wdev_0")

# Run all policies
policies=("LFU" "LRU" "LRFU" "EXD" "RL" "TEMP" "static" "ideal" "LogiReg" "XGBfast")

for workload in "${workloads[@]}"; do
    echo "Running workload: $workload"
    # Initialize workload    
    run_as_root restore init -workload $workload

    # First run with multiple threads
    for policy in "${policies[@]}"; do
        echo "Running policy: $policy"
        run_as_root restore run -policy $policy -workload $workload -workdir $REPO_ROOT \
            -read_time_tier1 30 -asym_tier1 1.5 -num_threads_tier1 8 \
            -read_time_tier2 200 -asym_tier2 2 -num_threads_tier2 4 \
            -read_time_tier3 500 -asym_tier3 4 -num_threads_tier3 2
    done
    # Rename results folder to include workload name
    run_as_root mkdir -p $REPO_ROOT/Results_$workload/num_threads842/
    run_as_root mv $REPO_ROOT/Results_$workload/capacity_tests_30-200-500/5000-15000 $REPO_ROOT/Results_$workload/num_threads842/
    # # create a new folder for single thread experiments
    # mkdir -p $REPO_ROOT/Results_$workload/capacity_tests_30-200-500/5000-15000/
    # for policy in "${policies[@]}"; do
    #     echo "Running policy: $policy"
    #     run_as_root restore run -policy $policy -workload $workload -workdir $REPO_ROOT \
    #         -read_time_tier1 30 -asym_tier1 1.5 -num_threads_tier1 1 \
    #         -read_time_tier2 200 -asym_tier2 2 -num_threads_tier2 1 \
    #         -read_time_tier3 500 -asym_tier3 4 -num_threads_tier3 1
    # done
    # # Rename results folder to include workload name
    # run_as_root mkdir -p $REPO_ROOT/Results_$workload/num_threads111/
    # run_as_root mv $REPO_ROOT/Results_$workload/capacity_tests_30-200-500/5000-15000 $REPO_ROOT/Results_$workload/num_threads111/
done

echo "All experiments completed!"
