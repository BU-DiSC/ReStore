#!/bin/bash

# Entry point of the Docker image (see Dockerfile).
#   (no argument)    the experiment included in the container (Fig. 7), see scripts/reproduce.sh
#   quick            the quick start (scripts/quick_start.sh)
#   <experiment>...  any experiment of scripts/reproduce.sh, e.g. fig8
# Results are written to $RESULTS_DIR (/output; mount a host folder there).

set -uo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
RESULTS_DIR="${RESULTS_DIR:-/output}"
mkdir -p "$RESULTS_DIR"

if [[ "${1:-}" == "quick" ]]; then
    "$REPO_ROOT/scripts/quick_start.sh"
    status=$?
    rm -rf "$RESULTS_DIR/quick_start" && mkdir -p "$RESULTS_DIR/quick_start"
    cp -r "$REPO_ROOT/Results_MSR_hm_1" "$RESULTS_DIR/quick_start/" 2> /dev/null
    exit $status
fi

exec "$REPO_ROOT/scripts/reproduce.sh" "$@"
