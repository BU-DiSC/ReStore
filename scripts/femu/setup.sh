#!/bin/bash

# One-time setup for the FEMU experiment (Fig. 13): builds FEMU, creates an Ubuntu 22.04 guest VM (or 20.04 with
# FEMU_GUEST=focal) from the official cloud image, and installs ReStore (with XGBoost-FastForest) in the guest.
# Requirements: Linux x86_64 host with KVM (/dev/kvm), sudo, 8 CPU cores (FEMU's emulation threads keep about
# 7 busy), about 16 GB of free memory, 40 GB of disk, and internet access. Everything is placed in $FEMU_HOME (default: ~/restore-femu).
# Afterwards, run scripts/femu/run_fig13.sh.

set -euo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

# --- checks
[[ "$(uname -s)" == Linux && "$(uname -m)" == x86_64 ]] || { echo "Error: FEMU needs a Linux x86_64 host"; exit 1; }
[[ -e /dev/kvm ]] || { echo "Error: /dev/kvm not found; FEMU needs KVM (enable virtualization, or nested virtualization on a VM)"; exit 1; }
mem_gb=$(awk '/MemAvailable/ { printf "%d", $2 / 1024 / 1024 }' /proc/meminfo)
(( $(nproc) >= 8 )) || echo "Warning: FEMU's emulation threads keep about 7 cores busy; with $(nproc) cores, the emulated latencies may be inaccurate"
(( mem_gb >= 15 )) || { echo "Error: FEMU needs about 16 GB of free memory (guest VM 8 GB + 3 emulated SSDs of 2 GB); available: $mem_gb GB"; exit 1; }
mkdir -p "$FEMU_HOME"

# --- host packages (FEMU build dependencies, cloud-init seed tool)
log "installing host packages"
sudo apt-get update -qq
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y -qq \
    git build-essential pkg-config python3 python3-venv ninja-build libglib2.0-dev libfdt-dev libpixman-1-dev \
    zlib1g-dev libdw-dev libaio-dev libslirp-dev libnuma-dev qemu-utils cloud-image-utils wget openssh-client > /dev/null

# --- FEMU
if [[ ! -x "$FEMU_BUILD/qemu-system-x86_64" ]]; then
    log "building FEMU $FEMU_COMMIT (about 10 minutes)"
    [[ -d "$FEMU_SRC/.git" ]] || git clone -q "$FEMU_REPO" "$FEMU_SRC"
    git -C "$FEMU_SRC" checkout -q "$FEMU_COMMIT"
    mkdir -p "$FEMU_BUILD"
    (cd "$FEMU_BUILD" && cp ../femu-scripts/femu-copy-scripts.sh . && ./femu-copy-scripts.sh . > /dev/null \
        && ./femu-compile.sh > "$FEMU_HOME/femu-build.log" 2>&1) || { echo "Error: FEMU build failed, see $FEMU_HOME/femu-build.log"; exit 1; }
fi

# --- guest image, SSH key, and cloud-init configuration
if [[ ! -f "$GUEST_IMG" ]]; then
    log "downloading the Ubuntu cloud image $(basename "$GUEST_IMAGE_URL") (700 MB)"
    img="$FEMU_HOME/cloudimg.img"
    wget -q --tries=5 --waitretry=10 --timeout=60 -O "$img" "$GUEST_IMAGE_URL" \
        || { echo "Error: could not download $GUEST_IMAGE_URL"; rm -f "$img"; exit 1; }
    echo "$GUEST_IMAGE_SHA256  $img" | sha256sum -c --quiet \
        || { echo "Error: checksum mismatch for the downloaded cloud image"; rm -f "$img"; exit 1; }
    qemu-img convert -O qcow2 "$img" "$GUEST_IMG"
    qemu-img resize -q "$GUEST_IMG" 30G
    rm -f "$img"
fi
[[ -f "$GUEST_KEY" ]] || ssh-keygen -q -t ed25519 -N "" -f "$GUEST_KEY"
cat > "$FEMU_HOME/user-data" << EOF
#cloud-config
users:
  - name: $GUEST_USER
    sudo: ALL=(ALL) NOPASSWD:ALL
    shell: /bin/bash
    ssh_authorized_keys:
      - $(cat "$GUEST_KEY.pub")
package_update: true
packages: [build-essential, cmake, git, ca-certificates, libtbb-dev, python3, python3-numpy, python3-matplotlib, python3-seaborn]
EOF
printf 'instance-id: restore-femu\nlocal-hostname: restore-femu\n' > "$FEMU_HOME/meta-data"
cloud-localds "$GUEST_SEED" "$FEMU_HOME/user-data" "$FEMU_HOME/meta-data"

# --- install ReStore in the guest
trap stop_vm EXIT
start_vm restore-femu-setup
log "waiting for cloud-init in the guest (installs packages)"
guest_ssh "cloud-init status --wait > /dev/null"
log "copying ReStore to the guest"
tar czf - -C "$REPO_ROOT" --exclude=./.git --exclude=./build --exclude='./Results_*' --exclude='./results/fig*' \
    --exclude=./output --exclude=./restore_config.json --exclude=./device_config.json . \
    | guest_ssh "sudo rm -rf ReStore && mkdir ReStore && tar xzf - -C ReStore"
log "building XGBoost-FastForest and ReStore in the guest"
guest_ssh "set -e
    dpkg -s libtbb-dev > /dev/null 2>&1 || sudo DEBIAN_FRONTEND=noninteractive apt-get install -y -qq libtbb-dev > /dev/null
    rm -rf /tmp/ff && git -c advice.detachedHead=false clone -q --depth 1 --branch v0-6-0 https://github.com/guitargeek/XGBoost-FastForest.git /tmp/ff
    cmake -S /tmp/ff -B /tmp/ff/build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local > /dev/null
    cmake --build /tmp/ff/build -j > /dev/null && sudo cmake --install /tmp/ff/build > /dev/null
    cd ReStore && sudo ./install.sh --prefix /usr/local > /tmp/restore-install.log 2>&1 || { tail -20 /tmp/restore-install.log; exit 1; }
    restore list policies | head -3"
log "setup done; now run scripts/femu/run_fig13.sh"
