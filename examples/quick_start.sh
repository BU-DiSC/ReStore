#!/bin/bash

# Quick functional check: runs the LFU baseline and ReStore (RL) on the bundled MSR_hm_1 trace,
# with the default (simulated) tiers and the default MSR_hm_1 configuration, and prints a summary.
# Results: Results_MSR_hm_1/output_MSR_hm_1_<policy>.log

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

# Use an installed `restore`, or the one in ./build
if command -v restore &> /dev/null; then
    RESTORE=restore
elif [[ -x build/restore ]]; then
    RESTORE=build/restore
else
    echo "Error: 'restore' not found. Build it first, e.g.: ./install.sh --prefix ~/.local (see README.md)"
    exit 1
fi

WORKLOAD=MSR_hm_1
POLICIES=(LFU RL)
OUT_DIR=Results_${WORKLOAD}

$RESTORE init -workload $WORKLOAD
for policy in "${POLICIES[@]}"; do
    echo
    $RESTORE run -policy $policy -workload $WORKLOAD
done

# restore keeps runs with different settings apart, in
# Results_<workload>/capacity_tests_<read times>/<tier1>-<tier2 capacity>/; this example uses a
# single setting (the MSR_hm_1 defaults), so move its logs directly into Results_<workload>/.
RUN_DIR=$OUT_DIR/capacity_tests_30-200-500/5000-15000
for policy in "${POLICIES[@]}"; do
    mv "$RUN_DIR/output_${WORKLOAD}_${policy}.log" "$OUT_DIR/"
done
rmdir "$RUN_DIR" "$OUT_DIR/capacity_tests_30-200-500" 2> /dev/null || true

echo
echo "Summary (full logs in $OUT_DIR/):"
for policy in "${POLICIES[@]}"; do
    echo "== $policy"
    grep -E "Total requests time|page hits in Tier|Tier2 to Tier1|Tier3 to Tier2" "$OUT_DIR/output_${WORKLOAD}_${policy}.log"
done
