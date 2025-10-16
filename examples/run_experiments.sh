#!/bin/bash

# Example script showing how to use the ReStore package
# This script demonstrates various usage patterns

set -e

echo "ReStore Package Usage Examples"
echo "=============================="
echo

# Check if restore command is available
if ! command -v restore &> /dev/null; then
    echo "Error: 'restore' command not found. Please install ReStore first."
    echo "Run: sudo ./install.sh"
    exit 1
fi

echo "1. Listing available policies..."
echo "Available policies:"
restore list policies
echo

echo "2. Initializing configuration for MSR_hm_1 workload..."
restore init -workload MSR_hm_1
echo "Configuration saved to restore_config.json"
echo

echo "3. Running LFU policy on MSR_hm_1 workload..."
restore run -policy LFU -workload MSR_hm_1
echo

echo "4. Running RL policy with custom parameters..."
restore run -policy RL -workload MSR_hm_1 -max_capacity_tier1 8000 -RL_update_freqs 1000
echo

echo "5. Configuration management..."
echo "Current max_capacity_tier1:"
restore config get max_capacity_tier1
echo "Setting max_capacity_tier1 to 10000..."
restore config set max_capacity_tier1 10000
echo "New max_capacity_tier1:"
restore config get max_capacity_tier1
echo

echo "6. Running multiple policies comparison..."
policies=("LFU" "LRU" "RL")
for policy in "${policies[@]}"; do
    echo "Running $policy policy..."
    restore run -policy $policy -workload MSR_hm_1
    echo "Completed $policy"
    echo
done

echo "7. Running on different workloads..."
workloads=("MSR_hm_1" "MSR_wdev_0")
for workload in "${workloads[@]}"; do
    echo "Running LFU on $workload..."
    restore run -policy LFU -workload $workload
    echo "Completed $workload"
    echo
done

echo "All experiments completed!"
echo "Check the Results_* directories for output files."

