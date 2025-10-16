#!/bin/bash

# Function to run sudo command with password
sudo_with_password() {
    echo "femu" | sudo -S "$@"
}

# # First precondition the FEMU SSDs using fio
# echo "Preconditioning FEMU SSDs via fio precondition_3nvme.fio"
# sudo_with_password fio /home/femu/precondition_3nvme.fio

# Change to ReStore directory
cd /home/femu/ReStore_IO/

# list of workloads
workloads=("MSR_hm_1" "MSR_wdev_0")

# Run all policies
policies=("LFU" "LRU" "LRFU" "EXD" "RL" "TEMP" "static" "ideal" "LogiReg" "XGBfast")

for workload in "${workloads[@]}"; do
    echo "Running workload: $workload"
    # Initialize workload    
    sudo_with_password restore init -workload $workload

    # First run with multiple threads
    for policy in "${policies[@]}"; do
        echo "Running policy: $policy"
        sudo_with_password restore run -policy $policy -workload $workload -workdir /home/femu/ReStore_IO \
            -read_time_tier1 30 -asym_tier1 1.5 -num_threads_tier1 8 \
            -read_time_tier2 200 -asym_tier2 2 -num_threads_tier2 4 \
            -read_time_tier3 500 -asym_tier3 4 -num_threads_tier3 2
    done
    # Rename results folder to include workload name
    sudo_with_password mkdir -p /home/femu/ReStore_IO/Results_$workload/num_threads842/
    sudo_with_password mv /home/femu/ReStore_IO/Results_$workload/capacity_tests_30-200-500/5000-15000 /home/femu/ReStore_IO/Results_$workload/num_threads842/
    # # create a new folder for single thread experiments
    # mkdir -p /home/femu/ReStore_IO/Results_$workload/capacity_tests_30-200-500/5000-15000/
    # for policy in "${policies[@]}"; do
    #     echo "Running policy: $policy"
    #     sudo_with_password restore run -policy $policy -workload $workload -workdir /home/femu/ReStore_IO \
    #         -read_time_tier1 30 -asym_tier1 1.5 -num_threads_tier1 1 \
    #         -read_time_tier2 200 -asym_tier2 2 -num_threads_tier2 1 \
    #         -read_time_tier3 500 -asym_tier3 4 -num_threads_tier3 1
    # done
    # # Rename results folder to include workload name
    # sudo_with_passwordmkdir -p /home/femu/ReStore_IO/Results_$workload/num_threads111/
    # sudo_with_password mv /home/femu/ReStore_IO/Results_$workload/capacity_tests_30-200-500/5000-15000 /home/femu/ReStore_IO/Results_$workload/num_threads111/
done

echo "All experiments completed!"
