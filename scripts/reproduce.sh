#!/bin/bash

# Runs the experiments of the ReStore paper (SIGMOD 2026) and plots the corresponding figures.
#
# Usage: scripts/reproduce.sh <experiment>...
#   container        the experiment included in our Docker container: fig7
#   fig7             Fig. 7a/7b  static workloads, 6 tier-capacity settings          (simulated tiers)
#   fig8             Fig. 8      dynamic workloads A-D, 6 tier-capacity settings     (simulated tiers)
#   fig10            Fig. 10     scalability, 10^4 to 10^7 pages                     (simulated tiers; long)
#   fig11            Fig. 11     TPC-C, TPC-E, Google Thesios, YCSB traces            (simulated tiers; long)
#   fig12            Fig. 12     MSR traces, single-threaded and concurrent, vs Sibyl (tier devices, e.g. FEMU)
#   fig13 <config>   Fig. 13     MSR_wdev_0 on FEMU configured as <config> = default|config1|config2|config3
#   fig15            Fig. 15     10hf90, MSR_hm_1, Thesios on cloud volumes           (tier devices)
#   plots            re-plot all figures whose results exist
#
# Each experiment writes its results to results/<experiment>/ (set RESULTS_DIR to change the parent
# folder) and runs the figure's plotting script there. A run whose log already exists is skipped,
# so an interrupted experiment can be resumed by running it again.
#
# Experiments on tier devices (fig12, fig13, fig15) take the devices (or files on the tiers' file
# systems) from FAST_DEVICE, MIDDLE_DEVICE, and SLOW_DEVICE; raw devices need root, e.g.
#   sudo env "PATH=$PATH" FAST_DEVICE=/dev/nvme1n1 MIDDLE_DEVICE=/dev/nvme2n1 SLOW_DEVICE=/dev/nvme3n1 \
#       scripts/reproduce.sh fig12
# WARNING: the contents of these devices are overwritten (see README.md, "Tier devices").

set -uo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
RESULTS_DIR="${RESULTS_DIR:-$REPO_ROOT/results}"
PLOT_DIR="$REPO_ROOT/results"

# Use an installed `restore`, or the one in ./build
if command -v restore &> /dev/null; then
    RESTORE="$(command -v restore)"
elif [[ -x "$REPO_ROOT/build/restore" ]]; then
    RESTORE="$REPO_ROOT/build/restore"
else
    echo "Error: 'restore' not found. Build it first, e.g.: ./install.sh --prefix ~/.local (see README.md)"
    exit 1
fi
RESTORE_BIN_DIR="$(dirname "$RESTORE")"

# The plotting scripts assign the logs in a folder to the bars by their sorted file names, so all
# of these policies must be run (ideal = Oracle, RL = ReStore).
POLICIES=(LFU LRU LRFU EXD LogiReg XGBfast ideal TEMP RL)
# On tier devices (Figs. 12, 13, 15), ReStore is the RL_device version, which produced these figures.
# They run in the order of the paper's runs, on the same devices: RL_device after LFU, LRU, LRFU, and EXD.
DEVICE_POLICIES=(LFU LRU LRFU EXD RL_device TEMP ideal LogiReg XGBfast)
# Pre-trained models of LogiReg/XGBfast (percentage of the workload they were trained on): the ones used
# for each figure of the paper. EXPERIMENT is set by each experiment below.
EXPERIMENT=""
train_percentage() {  # <policy> <workload>
    case "$EXPERIMENT/$1/$2" in
        fig7/XGBfast/10hf90_1e4_rw1_1e6)             echo 5 ;;
        fig7/*|fig10/*)                              echo 1 ;;
        fig8/LogiReg/*)                              echo 10 ;;
        fig8/XGBfast/*)                              echo 5 ;;
        fig11/*/TPCC)                                echo 2 ;;
        fig11/*/TPCE)                                echo 5 ;;
        fig11/*/thesios|fig11/*/YCSB)                echo 1 ;;
        fig15/LogiReg/thesios)                       echo 1 ;;
        *)                                           echo 5 ;;   # fig12, fig13, fig15
    esac
}

# Tier capacities (pages) for the 10^4-page synthetic workloads: Tier1 1-12%, Tier2 4-16% (Fig. 7, 8)
CAPACITIES=("100 400" "100 800" "300 800" "300 1600" "600 800" "1200 1600")

FAILED=()

log() { echo "[$(date +%H:%M:%S)] $*"; }

check_policies() {
    if [[ ! -x "$RESTORE_BIN_DIR/Run_XGBfast" ]] && ! command -v Run_XGBfast &> /dev/null; then
        echo "Error: the XGBfast policy is not built. Install XGBoost-FastForest and rebuild ReStore"
        echo "(see README.md, \"ML-based policies\"); the figures need all policies."
        exit 1
    fi
}

check_devices() {
    if [[ -z "${FAST_DEVICE:-}" || -z "${MIDDLE_DEVICE:-}" || -z "${SLOW_DEVICE:-}" ]]; then
        echo "Error: $1 runs on tier devices: set FAST_DEVICE, MIDDLE_DEVICE, and SLOW_DEVICE (see README.md, \"Tier devices\")."
        exit 1
    fi
    DEVICE_ARGS=(-fast_device "$FAST_DEVICE" -middle_device "$MIDDLE_DEVICE" -slow_device "$SLOW_DEVICE")
}

has_workload() {
    [[ -n "$(find -L "$REPO_ROOT/workloads" -name "workload_$1.txt" -print -quit)" ]]
}

require_workloads() {
    local missing=()
    for w in "$@"; do has_workload "$w" || missing+=("$w"); done
    if (( ${#missing[@]} )); then
        echo "Error: workload(s) not found under workloads/: ${missing[*]}"
        echo "See README.md (\"Workloads\") for how to generate or download and convert them."
        exit 1
    fi
}

# Results folder of an experiment; workloads/ and ML_models/ are linked into it, so it can be used
# as the working directory of `restore` (which writes Results_<workload>/ there).
prepare_dir() {
    local dir="$RESULTS_DIR/$1"
    mkdir -p "$dir"
    ln -sfn "$REPO_ROOT/workloads" "$dir/workloads"
    ln -sfn "$REPO_ROOT/ML_models" "$dir/ML_models"
    echo "$dir"
}

# run <dir> <log subfolder> <policy> <workload> [restore options...]
run() {
    local dir=$1 sub=$2 policy=$3 workload=$4; shift 4
    local args=("$@")
    local suffix=""
    if [[ $policy == LogiReg || $policy == XGBfast ]]; then
        local pct; pct=$(train_percentage "$policy" "$workload")
        args+=(-train_percentage "$pct")
        suffix="_$pct%"
    fi
    local logfile="$dir/Results_$workload/$sub/output_${workload}_${policy}${suffix}.log"
    if [[ -f "$logfile" ]] && grep -q "Total requests time" "$logfile"; then
        log "skip (done)  $policy  $workload  $sub"
        return
    fi
    log "run  $policy  $workload  $sub ${args[*]}"
    if ! (cd "$dir" && "$RESTORE" run -policy "$policy" -workload "$workload" "${args[@]}" >> "$dir/reproduce.log" 2>&1); then
        log "FAILED  $policy  $workload  $sub (see $dir/reproduce.log)"
        FAILED+=("$policy/$workload/$sub")
    fi
}

plot() {  # <dir> <plotting script>...
    local dir=$1; shift
    for script in "$@"; do
        log "plot $script in $dir"
        (cd "$dir" && python3 "$PLOT_DIR/$script") >> "$dir/reproduce.log" 2>&1 \
            || log "plotting with $script failed (see $dir/reproduce.log)"
    done
}

# TEMP promotion thresholds of the paper's simulated-tier experiments (Figs. 7 and 8): a page is promoted
# after 3 requests within the request window; the device experiments use the defaults (Tier1 4, Tier2 2)
temp_args() {  # <policy>
    [[ $1 == TEMP ]] && echo "-min_access_toT1 2 -min_access_toT2 2"
}

# ReStore parameters tuned per setting in the paper (Fig. 8): a_b_update_freq and num_elements_to_consider of both
# state variables; settings not listed (and all of Fig. 7, which the defaults reproduce best) use the defaults
RL_TUNED="
5hf90+10hf80_1e4_rw1_1e6    300-800 6 1000  300-1600 3 1500  600-800 6 1000
5hf90*10_1e4_rw1_1e6        100-400 5 2000  100-800 5 2000  300-800 6 2000  300-1600 6 1500
"
rl_args() {  # <policy> <workload> <tier1> <tier2>
    [[ $1 == RL ]] || return 0
    local xy
    xy=$(awk -v w="$2" -v c="$3-$4" '$1 == w { for (i = 2; i < NF; i += 3) if ($i == c) print $(i+1), $(i+2) }' <<< "$RL_TUNED")
    [[ -n $xy ]] || return 0
    set -- $xy
    echo "-a_b_update_freq_s1 $1 -a_b_update_freq_s2 $1 -num_elements_to_consider_s1 $2 -num_elements_to_consider_s2 $2"
}

# Synthetic workloads at the 6 capacity settings (Fig. 7 and 8)
capacity_tests() {  # <experiment> <workload>...
    local dir; dir=$(prepare_dir "$1"); shift
    require_workloads "$@"
    for workload in "$@"; do
        for cap in "${CAPACITIES[@]}"; do
            set -- $cap
            for policy in "${POLICIES[@]}"; do
                run "$dir" "capacity_tests_30-200-500/$1-$2" "$policy" "$workload" -max_capacity_tier1 "$1" -max_capacity_tier2 "$2" \
                    $(temp_args "$policy") $(rl_args "$policy" "$workload" "$1" "$2")
            done
        done
    done
}

fig7() {
    EXPERIMENT=fig7
    check_policies
    capacity_tests fig7 5hf90_1e4_rw1_1e6 10hf90_1e4_rw1_1e6 10hf80_1e4_rw1_1e6
    plot "$RESULTS_DIR/fig7" plot_capacity_test.py plot_capacity_test_migrations.py
}

fig8() {
    EXPERIMENT=fig8
    check_policies
    capacity_tests fig8 "5hf90+10hf80_1e4_rw1_1e6" "5hf90+10hf80+20hf80_1e4_rw1_1e6" "5hf90*10_1e4_rw1_1e6" "5hf90*50_1e4_rw1_1e6"
    plot "$RESULTS_DIR/fig8" plot_dynamic_workload.py
}

fig10() {
    EXPERIMENT=fig10
    check_policies
    local dir; dir=$(prepare_dir fig10)
    # <workload> <#pages> <#requests>; Tier1/Tier2 = 3%/8% of the pages
    local settings=("5hf90_1e4_rw1_1e6 10000 1000000" "5hf90_1e5_rw1_1e7 100000 10000000"
                    "5hf90_1e6_rw1_1e8 1000000 100000000" "5hf90_1e7_rw1_1e8 10000000 100000000")
    for s in "${settings[@]}"; do set -- $s; require_workloads "$1"; done
    for s in "${settings[@]}"; do
        set -- $s
        local w=$1 pages=$2 reqs=$3 t1=$(( $2 * 3 / 100 )) t2=$(( $2 * 8 / 100 ))
        for policy in "${POLICIES[@]}"; do
            run "$dir" "capacity_tests_30-200-500/$t1-$t2" "$policy" "$w" -total_num_pages "$pages" -total_num_reqs "$reqs" \
                -max_capacity_tier1 "$t1" -max_capacity_tier2 "$t2" -max_capacity_tier3 $(( 2 * pages ))
        done
    done
    plot "$dir" plot_scalability_test.py
}

# Parameters of the real-world traces (Fig. 11, 15)
trace_args() {
    case $1 in
        TPCC) echo "-total_num_pages 16842331 -total_num_reqs 62925641
                    -max_capacity_tier1 2000000 -max_capacity_tier2 8000000 -max_capacity_tier3 20000000
                    -temp_incr_alpha 0.2 -temp_incr_buffersize 10000000 -temp_drop_thrd 1000000 -temp_drop_freqs 2000000
                    -RL_update_freqs 1000 -RL_init_rounds 100
                    -a_b_update_freq_s1 10 -num_elements_to_consider_s1 1000 -a_b_update_freq_s2 10 -num_elements_to_consider_s2 2000" ;;
        TPCE) echo "-total_num_pages 19274163 -total_num_reqs 31884418
                    -max_capacity_tier1 2000000 -max_capacity_tier2 8000000 -max_capacity_tier3 20000000
                    -temp_incr_alpha 0.2 -temp_incr_buffersize 10000000 -temp_drop_thrd 1000000 -temp_drop_freqs 2000000
                    -RL_update_freqs 1000 -RL_init_rounds 100
                    -a_b_update_freq_s1 10 -num_elements_to_consider_s1 1000 -a_b_update_freq_s2 10 -num_elements_to_consider_s2 2000" ;;
        YCSB) echo "-total_num_pages 61960292 -total_num_reqs 92728323
                    -max_capacity_tier1 10000000 -max_capacity_tier2 20000000 -max_capacity_tier3 70000000
                    -temp_incr_alpha 0.2 -temp_incr_buffersize 10000000 -temp_drop_thrd 1000000 -temp_drop_freqs 2000000
                    -RL_update_freqs 1000 -RL_init_rounds 100
                    -a_b_update_freq_s1 10 -num_elements_to_consider_s1 1000 -a_b_update_freq_s2 10 -num_elements_to_consider_s2 2000" ;;
        thesios) echo "-total_num_pages 16417 -total_num_reqs 95970071
                    -max_capacity_tier1 1500 -max_capacity_tier2 5000 -max_capacity_tier3 20000
                    -temp_incr_alpha 0.05 -temp_incr_buffersize 10000 -temp_drop_thrd 5000 -temp_drop_freqs 10000
                    -RL_update_freqs 500 -RL_init_rounds 100
                    -a_b_update_freq_s1 5 -num_elements_to_consider_s1 500 -a_b_update_freq_s2 5 -num_elements_to_consider_s2 1000" ;;
    esac
}

trace_caps() {  # results subfolder of a trace
    case $1 in
        TPCC|TPCE) echo "capacity_tests_30-200-500/2000000-8000000" ;;
        YCSB) echo "capacity_tests_30-200-500/10000000-20000000" ;;
        thesios) echo "capacity_tests_30-200-500/1500-5000" ;;
    esac
}

fig11() {
    EXPERIMENT=fig11
    check_policies
    local dir; dir=$(prepare_dir fig11)
    require_workloads TPCC TPCE thesios YCSB
    for w in TPCC TPCE thesios YCSB; do
        for policy in "${POLICIES[@]}"; do
            run "$dir" "$(trace_caps $w)" "$policy" "$w" $(trace_args $w)
        done
    done
    plot "$dir" plot_traces.py
}

# RL_device parameters of the paper's device experiments, beyond the `restore init` defaults
rl_device_args() {  # <policy> <workload>
    if [[ $1 == RL_device && $2 == MSR_wdev_0 ]]; then
        echo "-a_b_update_freq_s1 50 -a_b_update_freq_s2 50 -num_elements_to_consider_s1 1000 -num_elements_to_consider_s2 1000"
    fi
}

# MSR traces with their `restore init` defaults (Tier1/Tier2 = 5000/15000 pages)
msr_init() {  # <dir> <workload>
    (cd "$1" && "$RESTORE" init -workload "$2" > /dev/null)
}

fig12() {
    EXPERIMENT=fig12
    check_policies
    check_devices fig12
    local dir; dir=$(prepare_dir fig12)
    require_workloads MSR_hm_1 MSR_wdev_0
    for w in MSR_hm_1 MSR_wdev_0; do
        msr_init "$dir" "$w"
        # single-threaded (_ST) and concurrent (_Con) I/O
        for threads in "1 1 1" "8 4 2"; do
            set -- $threads
            local sub="num_threads$1$2$3/5000-15000"
            for policy in LFU LRU LRFU EXD LogiReg XGBfast TEMP RL_device; do
                run "$dir" "$sub" "$policy" "$w" "${DEVICE_ARGS[@]}" $(rl_device_args "$policy" "$w") \
                    -num_threads_tier1 "$1" -num_threads_tier2 "$2" -num_threads_tier3 "$3"
                # restore names the folder after read times and capacities; keep runs with different threads apart
                local src="$dir/Results_$w/capacity_tests_30-200-500/5000-15000"
                if compgen -G "$src/*.log" > /dev/null; then
                    mkdir -p "$dir/Results_$w/$sub" && mv "$src"/*.log "$dir/Results_$w/$sub/"
                fi
            done
        done
    done
    if compgen -G "$dir/Results_MSR_*/num_threads*/5000-15000/output_*_Sibyl.log" > /dev/null; then
        plot "$dir" plot_MSR_Sibyl.py
    else
        log "Add the Sibyl results as output_<workload>_Sibyl.log to each $dir/Results_MSR_*/num_threads*/5000-15000/ folder,"
        log "then run: scripts/reproduce.sh plots (see README.md, \"Comparison with Sibyl\")"
    fi
}

fig13() {
    EXPERIMENT=fig13
    local config=${1:-}
    # Table 5: Tier1/2/3 read latency (us) and concurrency; R/W asymmetry is 1.5/2/4 in all configurations
    local args
    case $config in
        default) args="-read_time_tier1 30 -read_time_tier2 200 -read_time_tier3 500 -num_threads_tier1 8 -num_threads_tier2 4 -num_threads_tier3 2" ;;
        config1) args="-read_time_tier1 7 -read_time_tier2 200 -read_time_tier3 500 -num_threads_tier1 8 -num_threads_tier2 8 -num_threads_tier3 2" ;;
        config2) args="-read_time_tier1 30 -read_time_tier2 200 -read_time_tier3 500 -num_threads_tier1 8 -num_threads_tier2 8 -num_threads_tier3 2" ;;
        config3) args="-read_time_tier1 7 -read_time_tier2 200 -read_time_tier3 500 -num_threads_tier1 8 -num_threads_tier2 4 -num_threads_tier3 2" ;;
        *) echo "Usage: scripts/reproduce.sh fig13 <default|config1|config2|config3>"; exit 1 ;;
    esac
    check_policies
    check_devices fig13
    local dir; dir=$(prepare_dir "fig13/$config")
    require_workloads MSR_wdev_0
    msr_init "$dir" MSR_wdev_0
    local rt; rt=$(echo "$args" | awk '{print $2"-"$4"-"$6}')
    for policy in "${DEVICE_POLICIES[@]}"; do
        run "$dir" "capacity_tests_$rt/5000-15000" "$policy" MSR_wdev_0 "${DEVICE_ARGS[@]}" $args $(rl_device_args "$policy" MSR_wdev_0)
    done
    plot_fig13
}

plot_fig13() {  # once all four configurations have been run
    local base="$RESULTS_DIR/fig13" c
    for c in default config1 config2 config3; do
        if ! compgen -G "$base/$c/Results_MSR_wdev_0/capacity_tests_*/5000-15000/output_MSR_wdev_0_RL_device.log" > /dev/null; then
            log "Fig. 13 is plotted once all of default, config1, config2, and config3 have been run (missing: $c)"
            return 0
        fi
    done
    plot "$base" plot_femu_configs.py
}

fig15() {
    EXPERIMENT=fig15
    check_policies
    check_devices fig15
    local dir; dir=$(prepare_dir fig15)
    require_workloads 10hf90_1e4_rw1_1e6 MSR_hm_1 thesios
    msr_init "$dir" MSR_hm_1
    for policy in "${DEVICE_POLICIES[@]}"; do
        run "$dir" capacity_tests_30-200-500/300-800 "$policy" 10hf90_1e4_rw1_1e6 "${DEVICE_ARGS[@]}" -max_capacity_tier1 300 -max_capacity_tier2 800
        run "$dir" capacity_tests_30-200-500/5000-15000 "$policy" MSR_hm_1 "${DEVICE_ARGS[@]}"
        run "$dir" "$(trace_caps thesios)" "$policy" thesios "${DEVICE_ARGS[@]}" $(trace_args thesios)
    done
    plot "$dir" plot_cloud_test.py
}

plots() {
    [[ -d "$RESULTS_DIR/fig7" ]] && plot "$RESULTS_DIR/fig7" plot_capacity_test.py plot_capacity_test_migrations.py
    [[ -d "$RESULTS_DIR/fig8" ]] && plot "$RESULTS_DIR/fig8" plot_dynamic_workload.py
    [[ -d "$RESULTS_DIR/fig10" ]] && plot "$RESULTS_DIR/fig10" plot_scalability_test.py
    [[ -d "$RESULTS_DIR/fig11" ]] && plot "$RESULTS_DIR/fig11" plot_traces.py
    [[ -d "$RESULTS_DIR/fig12" ]] && plot "$RESULTS_DIR/fig12" plot_MSR_Sibyl.py
    [[ -d "$RESULTS_DIR/fig13" ]] && plot_fig13
    [[ -d "$RESULTS_DIR/fig15" ]] && plot "$RESULTS_DIR/fig15" plot_cloud_test.py
    return 0
}

if (( $# == 0 )); then
    sed -n '3,25p' "$0" | sed 's/^# \{0,1\}//'
    exit 1
fi

while (( $# )); do
    case $1 in
        container) fig7 ;;
        fig7|fig8|fig10|fig11|fig12|fig15|plots) $1 ;;
        fig13) fig13 "${2:-}"; shift ;;
        *) echo "Unknown experiment: $1 (run without arguments for usage)"; exit 1 ;;
    esac
    shift
done

if (( ${#FAILED[@]} )); then
    log "${#FAILED[@]} run(s) failed: ${FAILED[*]}"
    exit 1
fi
log "done; results and figures are in $RESULTS_DIR/"
