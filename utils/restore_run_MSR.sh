#!/bin/bash

# running MSR workload with restore

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

# Define the list of workload
WORKLOADS=(
    "MSR_hm_1"
    #"MSR_mds_0"
    #"MSR_wdev_0"
)

# Run all policies (or specific policies)
policies=("LFU" "LRU" "LRFU" "EXD" "RL" "TEMP" "static" "ideal" "LogiReg" "XGBfast")

# Raw access to the tier block devices needs root; sudo prompts once and caches credentials.
run_as_root() {
    sudo env "PATH=$PATH" "$@"
}

for workload in "${WORKLOADS[@]}"; do
    # Initialize default configuration for a workload
    restore init -workload $workload
    echo "Initialized workload: $workload"
    for policy in "${policies[@]}"; do
        echo "Running policy: $workload with policy: $policy"
        run_as_root restore run -policy $policy -workload $workload -workdir "$REPO_ROOT"
    done
done


# # simple bash command for manual running
# for policy in LFU LRU LRFU EXD RL TEMP static ideal LogiReg XGBfast; do
#     echo "Running workload: MSR_hm_1 with policy: $policy"
#     run_as_root restore run -policy $policy -workload MSR_hm_1 -workdir "$REPO_ROOT"
# done
for policy in LFU LRU LRFU EXD RL TEMP static ideal LogiReg XGBfast; do
    echo "Running workload: 10hf90_1e4_rw1_1e6 with policy: $policy"
    run_as_root restore run -policy $policy -workload 10hf90_1e4_rw1_1e6 -workdir "$REPO_ROOT" \
    -max_capacity_tier1 300 -max_capacity_tier2 800 -max_capacity_tier3 10000
done