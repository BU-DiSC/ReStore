#!/bin/bash

# running traces workload with restore

# Define the list of workload
WORKLOADS=(
    "TPCC"
    "TPCE"
    "thesios"
    "YCSB"
    "MSR_hm_1"
    "MSR_wdev_0"
)

# Configuration for each workload
TPCC=(
    "-max_capacity_tier1=2000000 -max_capacity_tier2=8000000 -max_capacity_tier3=20000000
    -total_num_pages=16842331 -total_num_reqs=62925641 
    -temp_incr_alpha=0.2 -temp_incr_buffersize=10000000 -temp_drop_thrd=1000000 -temp_drop_freqs=2000000
    -RL_update_freqs=1000 -RL_init_rounds=100
    -a_b_update_freq_s1=10 -num_elements_to_consider_s1=1000 
    -a_b_update_freq_s2=10 -num_elements_to_consider_s2=2000"
)
TPCE=(
    "-max_capacity_tier1=2000000 -max_capacity_tier2=8000000 -max_capacity_tier3=20000000
    -total_num_pages=19274163 -total_num_reqs=31884418 
    -temp_incr_alpha=0.2 -temp_incr_buffersize=10000000 -temp_drop_thrd=1000000 -temp_drop_freqs=2000000
    -RL_update_freqs=1000 -RL_init_rounds=100
    -a_b_update_freq_s1=10 -num_elements_to_consider_s1=1000 
    -a_b_update_freq_s2=10 -num_elements_to_consider_s2=2000"
)

thesios=(
    "-max_capacity_tier1=1500 -max_capacity_tier2=5000 -max_capacity_tier3=20000 
    -total_num_pages=16417 -total_num_reqs=95970071
    -temp_incr_alpha=0.05 -temp_incr_buffersize=10000 -temp_drop_thrd=5000 -temp_drop_freqs=10000
    -RL_update_freqs=500 -RL_init_rounds=100
    -a_b_update_freq_s1=5 -num_elements_to_consider_s1=500 
    -a_b_update_freq_s2=5 -num_elements_to_consider_s2=1000"
)
YCSB=(
    "-max_capacity_tier1=2000000 -max_capacity_tier2=8000000 -max_capacity_tier3=20000000
    -total_num_pages=16842331 -total_num_reqs=62925641 
    -temp_incr_alpha=0.2 -temp_incr_buffersize=10000000 -temp_drop_thrd=1000000 -temp_drop_freqs=2000000
    -RL_update_freqs=1000 -RL_init_rounds=100
    -a_b_update_freq_s1=10 -num_elements_to_consider_s1=1000 
    -a_b_update_freq_s2=10 -num_elements_to_consider_s2=2000"
)
MSR_hm_1=(
    "-max_capacity_tier1=5000 -max_capacity_tier2=15000 -max_capacity_tier3=60000
    -total_num_pages=51733 -total_num_reqs=2308560 
    -temp_incr_alpha=0.1 -temp_incr_buffersize=50000 -temp_drop_thrd=10000 -temp_drop_freqs=20000
    -RL_update_freqs=500 -RL_init_rounds=100
    -a_b_update_freq_s1=5 -num_elements_to_consider_s1=1000 
    -a_b_update_freq_s2=5 -num_elements_to_consider_s2=1000"
)
MSR_wdev_0=(
    "-max_capacity_tier1=5000 -max_capacity_tier2=15000 -max_capacity_tier3=60000
    -total_num_pages=79677 -total_num_reqs=695995 
    -temp_incr_alpha=0.1 -temp_incr_buffersize=50000 -temp_drop_thrd=10000 -temp_drop_freqs=20000
    -RL_update_freqs=500 -RL_init_rounds=100
    -a_b_update_freq_s1=5 -num_elements_to_consider_s1=1000 
    -a_b_update_freq_s2=5 -num_elements_to_consider_s2=1000"
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
        sudo_with_password restore run -policy $policy -workload $workload -workdir=/home/femu/ReStore ${${workload}[@]}
    done
done