#!/bin/bash

# Settings and helpers shared by scripts/femu/setup.sh and scripts/femu/run_fig13.sh.
# FEMU (https://github.com/vtess/femu) emulates NVMe SSDs inside a QEMU guest VM; ReStore runs in the
# guest and uses three FEMU SSDs as its tiers.

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
FEMU_HOME="${FEMU_HOME:-$HOME/restore-femu}"     # FEMU build, guest image, and SSH key
FEMU_SRC="$FEMU_HOME/femu"
FEMU_BUILD="$FEMU_SRC/build-femu"
FEMU_REPO="https://github.com/vtess/femu.git"
FEMU_COMMIT="805b9c8e6"                         # the FEMU version used for the paper
# Guest OS: Ubuntu 22.04 (default) or 20.04 (FEMU_GUEST=focal, the release of the paper's guest)
case "${FEMU_GUEST:-jammy}" in
    jammy)
        GUEST_IMAGE_URL="https://cloud-images.ubuntu.com/releases/jammy/release-20261004/ubuntu-22.04-server-cloudimg-amd64.img"
        GUEST_IMAGE_SHA256="012d81fade7e8ff4428fc35dfeee711d66d1f47d5ea9c5c8ced96a85b959835c"
        GUEST_IMG="$FEMU_HOME/guest.qcow2" ;;
    focal)
        GUEST_IMAGE_URL="https://cloud-images.ubuntu.com/releases/focal/release-20250624/ubuntu-20.04-server-cloudimg-amd64.img"
        GUEST_IMAGE_SHA256="18f2977d77dfea1b74aee14533bd21c34f789139e949c57023b7364894b7e5e9"
        GUEST_IMG="$FEMU_HOME/guest-focal.qcow2" ;;
    *) echo "Error: FEMU_GUEST must be jammy (Ubuntu 22.04) or focal (Ubuntu 20.04)"; exit 1 ;;
esac
GUEST_SEED="$FEMU_HOME/seed.iso"
GUEST_KEY="$FEMU_HOME/guest_key"
GUEST_USER="restore"
GUEST_SSH_PORT="${FEMU_SSH_PORT:-8080}"
VM_CPUS="${FEMU_VM_CPUS:-4}"
VM_MEM="${FEMU_VM_MEM:-8G}"
VM_PID="$FEMU_HOME/vm.pid"

log() { echo "[$(date +%H:%M:%S)] $*"; }

guest_ssh() {
    ssh -i "$GUEST_KEY" -p "$GUEST_SSH_PORT" -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null \
        -o LogLevel=ERROR -o ConnectTimeout=5 "$GUEST_USER@localhost" "$@"
}

# FEMU black-box SSD with the given read latency (us), write/read asymmetry, and number of channels
# (concurrency); the write latency is truncated to whole microseconds.
femu_ssd() {  # <read latency us> <asymmetry> <channels>
    local read_ns=$(( $1 * 1000 ))
    local write_ns=$(awk -v r="$1" -v a="$2" 'BEGIN { printf "%d", int(r * a) * 1000 }')
    echo "-device femu,devsz_mb=2048,femu_mode=1,secsz=512,secs_per_pg=8,pgs_per_blk=256,blks_per_pl=$(( 256 / $3 )),pls_per_lun=1,luns_per_ch=8,nchs=$3,pg_rd_lat=$read_ns,pg_wr_lat=$write_ns,blk_er_lat=2000000,ch_xfer_lat=0,gc_thres_pcent=75"
}

start_vm() {  # <name> [extra QEMU arguments, e.g. FEMU SSDs]
    local name=$1; shift
    if sudo test -f "$VM_PID" && sudo kill -0 "$(sudo cat "$VM_PID")" 2> /dev/null; then
        echo "Error: a FEMU VM is already running (pid $(sudo cat "$VM_PID")); stop it first"; exit 1
    fi
    log "starting VM $name (log: $FEMU_HOME/$name.log)"
    sudo "$FEMU_BUILD/qemu-system-x86_64" -name "$name" -enable-kvm -cpu host -smp "$VM_CPUS" -m "$VM_MEM" \
        -drive file="$GUEST_IMG",if=virtio,cache=none,aio=native,format=qcow2 \
        -drive file="$GUEST_SEED",if=virtio,format=raw,readonly=on \
        "$@" \
        -net user,hostfwd=tcp::"$GUEST_SSH_PORT"-:22 -net nic,model=virtio \
        -display none -serial file:"$FEMU_HOME/$name.log" -daemonize -pidfile "$VM_PID" \
        || { echo "Error: could not start the VM"; exit 1; }
    local i
    for i in $(seq 1 120); do
        guest_ssh true 2> /dev/null && return 0
        sleep 5
    done
    echo "Error: the VM did not become reachable over SSH (see $FEMU_HOME/$name.log)"; stop_vm; exit 1
}

stop_vm() {
    sudo test -f "$VM_PID" || return 0  # written by QEMU, which runs as root
    local pid; pid=$(sudo cat "$VM_PID")
    guest_ssh sudo poweroff 2> /dev/null || true
    local i
    for i in $(seq 1 60); do
        sudo kill -0 "$pid" 2> /dev/null || break
        sleep 2
    done
    sudo kill "$pid" 2> /dev/null || true
    sudo rm -f "$VM_PID"
    log "VM stopped"
}
