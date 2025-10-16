#!/bin/bash

# running MSR workload with restore

# Define the list of workload
WORKLOADS=(
    "MSR_hm_1"
    #"MSR_mds_0"
    #"MSR_wdev_0"
)

# Run all policies (or specific policies)
policies=("LFU" "LRU" "LRFU" "EXD" "RL" "TEMP" "static" "ideal" "LogiReg" "XGBfast")

# Function to run sudo command with password (femu user password: femu)
sudo_with_password() {
    echo "femu" | sudo -S "$@"
}

for workload in "${WORKLOADS[@]}"; do
    # Initialize default configuration for a workload
    restore init -workload $workload
    echo "Initialized workload: $workload"
    for policy in "${policies[@]}"; do
        echo "Running policy: $workload with policy: $policy"
        sudo_with_password restore run -policy $policy -workload $workload
    done
done


# # simple bash command for manual running
# for policy in LFU LRU LRFU EXD RL TEMP static ideal LogiReg XGBfast; do
#     echo "Running workload: MSR_hm_1 with policy: $policy"
#     sudo restore run -policy $policy -workload MSR_hm_1 -workdir /home/ubuntu/ReStore
# done
for policy in LFU LRU LRFU EXD RL TEMP static ideal LogiReg XGBfast; do
    echo "Running workload: 10hf90_1e4_rw1_1e6 with policy: $policy"
    sudo restore run -policy $policy -workload 10hf90_1e4_rw1_1e6 -workdir /home/ubuntu/ReStore \
    -max_capacity_tier1 300 -max_capacity_tier2 800 -max_capacity_tier3 10000
done