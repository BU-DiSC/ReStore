#!/bin/bash

# Fig. 12: runs MSR_hm_1 and MSR_wdev_0 on three FEMU SSDs with the default configuration of Table 5, with
# single-threaded (num_threads111) and concurrent (num_threads842) I/O, and plots results/fig12/Results_Sibyl_traces.pdf
# once the Sibyl results are added (see README.md, "Comparison with Sibyl").
# Run scripts/femu/setup.sh first. Usage: scripts/femu/run_fig12.sh (about 3 hours). Results are copied to
# $RESULTS_DIR/fig12/ (default: results/).

set -euo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
RESULTS_DIR="${RESULTS_DIR:-$REPO_ROOT/results}"

[[ -x "$FEMU_BUILD/qemu-system-x86_64" && -f "$GUEST_IMG" ]] || { echo "Error: run scripts/femu/setup.sh first"; exit 1; }

trap stop_vm EXIT
start_vm restore-femu-fig12 $(femu_ssd 30 1.5 8) $(femu_ssd 200 2 4) $(femu_ssd 500 4 2)   # Table 5, default
# use the current experiment and plotting scripts and ML models of this repository in the guest
(cd "$REPO_ROOT" && tar czf - scripts results/*.py ML_models) | guest_ssh "tar xzf - -C ReStore"
guest_ssh "lsblk -dn -o NAME,SIZE /dev/nvme0n1 /dev/nvme1n1 /dev/nvme2n1" \
    || { echo "Error: the FEMU SSDs are not visible in the guest"; exit 1; }
log "running Fig. 12 (log in the guest: results/fig12.log)"
guest_ssh "mkdir -p results && sudo env PATH=\$PATH RESULTS_DIR=\$HOME/results \
    FAST_DEVICE=/dev/nvme0n1 MIDDLE_DEVICE=/dev/nvme1n1 SLOW_DEVICE=/dev/nvme2n1 \
    ReStore/scripts/reproduce.sh fig12 > results/fig12.log 2>&1; status=\$?; tail -3 results/fig12.log; exit \$status"
mkdir -p "$RESULTS_DIR"
guest_ssh "sudo tar czf - -C results --exclude='*/workloads' --exclude='*/ML_models' fig12 fig12.log" | tar xzf - -C "$RESULTS_DIR"
log "results copied to $RESULTS_DIR/fig12/"
