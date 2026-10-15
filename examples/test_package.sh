#!/bin/bash

# Smoke test for the ReStore CLI (a few seconds; does not run a simulation).
# For a short end-to-end run, use ./examples/quick_start.sh.

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
FAILED=0

check() {
    local description="$1"; shift
    if "$@" &> /dev/null; then
        echo "✓ $description"
    else
        echo "✗ $description"
        FAILED=1
    fi
}

echo "ReStore Package Test Script"
echo "==========================="

# Use an installed `restore`, or the one in ./build
if command -v restore &> /dev/null; then
    RESTORE="$(command -v restore)"
elif [[ -x "$REPO_ROOT/build/restore" ]]; then
    RESTORE="$REPO_ROOT/build/restore"
else
    echo "✗ restore command not found. Build it first, e.g.: ./install.sh --prefix ~/.local"
    exit 1
fi
echo "Using: $RESTORE"

check "restore --help" "$RESTORE" --help
check "restore list policies" "$RESTORE" list policies
check "restore list workloads" "$RESTORE" list workloads -workdir "$REPO_ROOT"

# Configuration commands write restore_config.json to the current directory: use a scratch dir
TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT
(
    cd "$TMP_DIR"
    check "restore init -workload MSR_hm_1" "$RESTORE" init -workload MSR_hm_1
    check "configuration file created" test -f restore_config.json
    check "restore config get" "$RESTORE" config get max_capacity_tier1
    check "restore config set" "$RESTORE" config set max_capacity_tier1 6000
    check "config value updated" bash -c "\"$RESTORE\" config get max_capacity_tier1 | grep -qx 6000"
    exit $FAILED
) || FAILED=1

# Policy executables are looked up next to `restore`, then on PATH
RESTORE_DIR="$(dirname "$RESTORE")"
for policy in Run_RL Run_LFU Run_LRU Run_LRFU Run_EXD Run_TEMP Run_static Run_ideal Run_LogiReg; do
    check "$policy executable" bash -c "[[ -x \"$RESTORE_DIR/$policy\" ]] || command -v $policy"
done
if [[ -x "$RESTORE_DIR/Run_XGBfast" ]] || command -v Run_XGBfast &> /dev/null; then
    echo "✓ Run_XGBfast executable"
else
    echo "- Run_XGBfast not built (optional; needs XGBoost-FastForest)"
fi

echo
if [[ $FAILED -eq 0 ]]; then
    echo "All checks passed."
else
    echo "Some checks failed."
fi
exit $FAILED
