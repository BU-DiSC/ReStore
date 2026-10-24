#!/bin/bash

# Fig. 13: runs MSR_wdev_0 on three FEMU SSDs for each SSD configuration of Table 5 of the paper, and
# plots results/fig13/Results_femu_4config.pdf once all configurations have been run.
# Run scripts/femu/setup.sh first. Usage: scripts/femu/run_fig13.sh [default] [config1] [config2] [config3]
# (default: all four; about 1 hour each). Results are copied to $RESULTS_DIR/fig13/ (default: results/).
# Each configuration boots the guest VM with its own FEMU SSDs and runs `scripts/reproduce.sh fig13 <config>`
# there, on the SSDs /dev/nvme0n1 (Tier 1), /dev/nvme1n1 (Tier 2), and /dev/nvme2n1 (Tier 3).

set -euo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
RESULTS_DIR="${RESULTS_DIR:-$REPO_ROOT/results}"

[[ -x "$FEMU_BUILD/qemu-system-x86_64" && -f "$GUEST_IMG" ]] || { echo "Error: run scripts/femu/setup.sh first"; exit 1; }

# Table 5: per tier, read latency (us), R/W asymmetry, concurrency (FEMU channels)
ssd_args() {
    case $1 in
        default) echo "$(femu_ssd 30 1.5 8) $(femu_ssd 200 2 4) $(femu_ssd 500 4 2)" ;;
        config1) echo "$(femu_ssd 7 1.5 8)  $(femu_ssd 200 2 8) $(femu_ssd 500 4 2)" ;;
        config2) echo "$(femu_ssd 30 1.5 8) $(femu_ssd 200 2 8) $(femu_ssd 500 4 2)" ;;
        config3) echo "$(femu_ssd 7 1.5 8)  $(femu_ssd 200 2 4) $(femu_ssd 500 4 2)" ;;
        *) echo "Unknown configuration: $1 (default, config1, config2, config3)" >&2; exit 1 ;;
    esac
}

configs=("$@")
(( ${#configs[@]} )) || configs=(default config1 config2 config3)
for c in "${configs[@]}"; do ssd_args "$c" > /dev/null; done

trap stop_vm EXIT
for c in "${configs[@]}"; do
    start_vm "restore-femu-$c" $(ssd_args "$c")
    # use the current experiment and plotting scripts and ML models of this repository in the guest
    (cd "$REPO_ROOT" && tar czf - scripts results/*.py ML_models) | guest_ssh "tar xzf - -C ReStore"
    guest_ssh "lsblk -dn -o NAME,SIZE /dev/nvme0n1 /dev/nvme1n1 /dev/nvme2n1" \
        || { echo "Error: the FEMU SSDs are not visible in the guest"; exit 1; }
    log "running Fig. 13, $c (log in the guest: results/fig13-$c.log)"
    guest_ssh "mkdir -p results && sudo env PATH=\$PATH RESULTS_DIR=\$HOME/results \
        FAST_DEVICE=/dev/nvme0n1 MIDDLE_DEVICE=/dev/nvme1n1 SLOW_DEVICE=/dev/nvme2n1 \
        ReStore/scripts/reproduce.sh fig13 $c > results/fig13-$c.log 2>&1; status=\$?; tail -3 results/fig13-$c.log; exit \$status"
    mkdir -p "$RESULTS_DIR"
    guest_ssh "sudo tar czf - -C results --exclude='*/workloads' --exclude='*/ML_models' fig13 fig13-$c.log" | tar xzf - -C "$RESULTS_DIR"
    log "results of $c copied to $RESULTS_DIR/fig13/$c/"
    stop_vm
done
ls "$RESULTS_DIR"/fig13/*.pdf 2> /dev/null || log "Fig. 13 is plotted once default, config1, config2, and config3 have all been run"
