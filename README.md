# ReStore: A Reinforcement Learning Approach for Data Migration in Multi-Tiered Storage

This repository contains the implementation of `ReStore`, a reinforcement-learning-based data migration policy for multi-tiered storage systems, together with the baseline policies, workloads, pre-trained baseline models, and the scripts to run the experiments and plot the figures of the paper.

**Paper:** [ReStore: A Reinforcement Learning Approach for Data Migration in Multi-Tiered Storage](https://dl.acm.org/doi/10.1145/3802104), SIGMOD 2026 (Proc. ACM Manag. Data 4(3), Article 227). **Demo:** [ReStore: RL Guided Data Migration in Tiered Storage](https://disc-projects.bu.edu/ReStore/index.html) (VLDB demo). See [Citation](#citation).

`ReStore` dynamically manages data migration across storage tiers by learning from workload patterns, device characteristics, and system states, to optimize overall system performance while minimizing migration costs. The general workflow of `ReStore`:

![ReStore](figures/MTS-DBMS+RL.png?raw=true "ReStore workflow")

> [!WARNING]
> When you give ReStore real storage devices as tiers, it accesses them with direct I/O and **overwrites their contents**. Never point it at a disk that holds data; see [Tier devices](#tier-devices). By default the tiers are simulated and no device is touched.

**Reproducing the paper's results.** We provide a Docker container that builds ReStore, runs the experiments of the paper on simulated tiers, and plots the figures with a single command. Alternatively, you can install ReStore on your own server, set up the storage tiers, and run the experiments with one script. Both ways are described in [Reproducing the paper's experiments](#reproducing-the-papers-experiments).

## Contents

| Path | Description |
|------|-------------|
| [`cpp/`](cpp/) | Policy implementations (`Run_<policy>.cpp`), the RL agent, the tier model, and the tier device driver ([`cpp/device_driver/`](cpp/device_driver/)) |
| [`src/`](src/) | The `restore` command-line interface |
| [`workloads/`](workloads/) | Synthetic workloads and their generator, and the real-world traces with their conversion scripts |
| [`ML_models/`](ML_models/) | Pre-trained models for the `LogiReg` and `XGBfast` baselines |
| [`scripts/`](scripts/) | [`reproduce.sh`](scripts/reproduce.sh) (runs the experiments of the paper and plots the figures), [`quick_start.sh`](scripts/quick_start.sh), and [`test_package.sh`](scripts/test_package.sh) (smoke test) |
| [`results/`](results/) | Plotting scripts for the figures; results of `scripts/reproduce.sh` are written here |
| [`utils/`](utils/) | Helper scripts for our testbeds (FEMU preconditioning, OpenStack volumes, latency checks) |

## Requirements

**Software**
- Linux (the device driver uses Linux `O_DIRECT`). Tested on Ubuntu 22.04 (kernel 6.8), Ubuntu 24.04, and openSUSE Leap 15.6 (kernel 6.4).
- `g++` with C++20 support (tested with g++ 9, 12, and 13), CMake >= 3.16 (tested up to 4.4), `make`.
- [XGBoost-FastForest](https://github.com/guitargeek/XGBoost-FastForest) for the `XGBfast` baseline, see [ML-based policies](#ml-based-policies). ReStore builds without it, but the paper's figures include `XGBfast`.
- For the plots: Python 3 with `matplotlib`, `seaborn`, and `numpy`.

**Hardware**
- *Simulated tiers* (default): any Linux machine with >2 GB of memory.
- *Real tiers*: three storage devices (or file systems) as the fast, middle, and slow tier, see [Tier devices](#tier-devices). In the paper, we use NVMe SSDs emulated with [FEMU](https://github.com/MoatLab/FEMU) and QoS-configured cloud volumes.

## Build

```bash
git clone https://github.com/BU-DiSC/ReStore.git
cd ReStore
./install.sh --prefix ~/.local          # or: sudo ./install.sh   (installs to /usr/local)
export PATH="$HOME/.local/bin:$PATH"
restore --help
```

Install XGBoost-FastForest to include the `XGBfast` policy (see [ML-based policies](#ml-based-policies)). Alternatively, build with CMake directly; the binaries in `build/` can be used without installing:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/restore --help
```

## Quick start

From the repository root:

```bash
./scripts/quick_start.sh
```

This runs the `LFU` baseline and `ReStore` (`RL`) on the bundled MSR Cambridge `hm_1` trace (2.3M requests) with simulated tiers and the default configuration of this trace (Tier 1/2/3 capacity 5,000/15,000/60,000 pages, read latency 30/200/500 µs), and prints a summary (about 10 minutes). The logs are in `Results_MSR_hm_1/output_MSR_hm_1_<policy>.log`; each log ends with the total request time, the number of reads and writes, the page hits in each tier, the number of migrations between tiers, and the policy's decision overhead. `./scripts/test_package.sh` runs a few-second smoke test of the CLI.

The equivalent manual steps are:

```bash
restore init -workload MSR_hm_1               # writes ./restore_config.json with the trace's default configuration
restore run -policy LFU -workload MSR_hm_1
restore run -policy RL  -workload MSR_hm_1
```

You can always pass your own configuration, e.g. `restore run -policy RL -workload MSR_hm_1 -max_capacity_tier1 8000 -read_time_tier3 2000` (see [Parameters](#parameters)). To keep runs with different configurations apart, `restore` writes each log to `Results_<workload>/capacity_tests_<read_time_tier1>-<read_time_tier2>-<read_time_tier3>/<max_capacity_tier1>-<max_capacity_tier2>/` (the quick start moves its logs from there to `Results_MSR_hm_1/`). Run `restore` from the repository root, or pass `-workdir <path-to-repo>`, so that `workloads/` and `ML_models/` are found and the results are written there.

## Usage

```
restore init -workload <workload> [-<param> <value> ...]   Write ./restore_config.json with the workload's defaults
restore run -policy <policy> -workload <workload> [-<param> <value> ...]
            [-fast_device <path> -middle_device <path> -slow_device <path>]
restore list policies | workloads
restore config get <key> | set <key> <value>
```

Parameters given on the command line (`-key value` or `-key=value`) override `restore_config.json`. `restore run` returns a non-zero exit code if the policy fails.

### Policies

| Policy | Description |
|--------|-------------|
| `RL` | ReStore (reinforcement learning) |
| `LFU` | tLFU: tiered Least Frequently Used |
| `LRU` | tLRU: tiered Least Recently Used |
| `LRFU` | Least Recently/Frequently Used |
| `EXD` | Exponential Decay |
| `TEMP` | Migration based on ReStore's temperature model only (ablation of ReStore) |
| `LogiReg` | Logistic regression (Logi), see [ML-based policies](#ml-based-policies) |
| `XGBfast` | XGBoost (XGB), see [ML-based policies](#ml-based-policies) |
| `ideal` | Oracle: optimal initial placement with known access frequencies, no migrations |
| `static` | Static placement, no migrations |

#### ML-based policies

`LogiReg` and `XGBfast` predict whether a page will be accessed again within a fixed future window, from the intervals between its five most recent accesses, and migrate pages accordingly. Their pre-trained models are in [`ML_models/`](ML_models/), named after the workload and the percentage of the workload (from its start) they were trained on, e.g. `ML_models/Logistic_models/logistic_model_MSR_hm_1_5%.json` and `ML_models/XGBoost_models/xgboost_model_MSR_hm_1_5%.txt`. Select the model with `-train_percentage` (default 5):

```bash
restore run -policy LogiReg -workload MSR_hm_1 -train_percentage 5
restore run -policy XGBfast -workload MSR_hm_1 -train_percentage 5
```

`XGBfast` evaluates the XGBoost model with [XGBoost-FastForest](https://github.com/guitargeek/XGBoost-FastForest). Install it, then build or rebuild ReStore, which finds it in `~/.local` or `/usr/local`:

```bash
git clone --branch v0-6-0 https://github.com/guitargeek/XGBoost-FastForest.git
cmake -S XGBoost-FastForest -B XGBoost-FastForest/build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build XGBoost-FastForest/build -j
cmake --install XGBoost-FastForest/build
./install.sh --prefix ~/.local                # rebuild ReStore; the CMake output should say "FastForest found: building Run_XGBfast"
```

### Workloads

`restore list workloads` lists the workloads under `workloads/` (files `workload_<name>.txt`; subfolders are searched). Pass the name with `-workload <name>`; names with `*` or `+` must be quoted in the shell.

**Synthetic workloads** ([`workloads/synthetic/`](workloads/synthetic/), 10,000 pages, 1,000,000 requests, 50% reads). `xhfy` means that y% of the accesses go to x% of the pages:

| Workload | Name | Type | Description |
|----------|------|------|-------------|
| 5hf90 | `5hf90_1e4_rw1_1e6` | static | 90% of the accesses on 5% of the pages |
| 10hf90 | `10hf90_1e4_rw1_1e6` | static | 90% of the accesses on 10% of the pages |
| 10hf80 | `10hf80_1e4_rw1_1e6` | static | 80% of the accesses on 10% of the pages |
| A | `5hf90+10hf80_1e4_rw1_1e6` | dynamic | 5hf90, then 10hf80 |
| B | `5hf90+10hf80+20hf80_1e4_rw1_1e6` | dynamic | 5hf90, then 10hf80, then 20hf80 |
| C | `5hf90*10_1e4_rw1_1e6` | dynamic | 10 phases of 5hf90, each on a different 5% of the pages |
| D | `5hf90*50_1e4_rw1_1e6` | dynamic | 50 phases of 5hf90 |

Larger 5hf90 workloads (used for the scalability experiment) are generated with [`workload_gen.cpp`](workloads/synthetic/workload_gen.cpp), see [Fig. 10](#fig-10-scalability).

**Real-world traces** ([`workloads/traces/`](workloads/traces/)), converted to page-level requests (4 KB pages). Each folder contains the download and conversion instructions:

| Folder | Workloads | Source | #pages | #requests |
|--------|-----------|--------|-------:|----------:|
| [`MSR_Cambridge/`](workloads/traces/MSR_Cambridge/) | `MSR_hm_1`, `MSR_wdev_0` | [SNIA](https://iotta.snia.org/traces/block-io/388) | 51,733 / 79,677 | 2,308,560 / 695,995 |
| [`TPC/`](workloads/traces/TPC/) | `TPCC`, `TPCE` | [SNIA](https://iotta.snia.org/traces/block-io/130) | 16,842,331 / 19,274,163 | 62,925,641 / 31,884,418 |
| [`YCSB/`](workloads/traces/YCSB/) | `YCSB` | [SNIA](https://iotta.snia.org/traces/block-io/28568) | 61,960,292 | 92,728,323 |
| [`Thesios/`](workloads/traces/Thesios/) | `thesios` | [Google](https://github.com/google-research-datasets/thesios) | 16,417 | 95,970,071 |

`restore init` has default configurations for `MSR_hm_1` and `MSR_wdev_0`; the configurations we used for the other traces are in `trace_args()` of [`scripts/reproduce.sh`](scripts/reproduce.sh). For the synthetic workloads, the built-in defaults of the policies apply.

### Parameters

| Parameter | Meaning |
|-----------|---------|
| `max_capacity_tier1`, `max_capacity_tier2`, `max_capacity_tier3` | Capacity of each tier, in pages |
| `read_time_tier1`, `read_time_tier2`, `read_time_tier3` | Read latency of each tier (µs) |
| `asym_tier1`, `asym_tier2`, `asym_tier3` | Write/read asymmetry of each tier (write latency = read latency × asymmetry) |
| `num_threads_tier1`, `num_threads_tier2`, `num_threads_tier3` | Concurrency (I/O threads) of each tier |
| `total_num_pages`, `total_num_reqs` | Size of the workload |
| `RL_update_freqs`, `RL_init_rounds` | ReStore: model update frequency and number of initial rounds |
| `a_b_update_freq_s1`, `num_elements_to_consider_s1`, `a_b_update_freq_s2`, `num_elements_to_consider_s2` | ReStore: state normalization updates |
| `temp_incr_alpha`, `temp_incr_buffersize`, `temp_drop_thrd`, `temp_drop_freqs` | Page temperature tracking (TEMP and RL) |
| `train_percentage` | LogiReg / XGBfast: which pre-trained model to load (default 5) |

`read_time_*` and `asym_*` define the latencies of the **simulated tiers**: each read takes `read_time` µs and each write `read_time × asym` µs on one of the tier's `num_threads` I/O threads. When **real devices** are set as tiers, every read and write is a real direct I/O on the device, so the measured times are the devices' real I/O times; `read_time_*` and `asym_*` then only tell ReStore's RL agent the relative speed of the tiers, so set them to match the devices.

### Tier devices

By default, all three tiers are simulated (see [Parameters](#parameters)). ReStore can equally run on a system that already has multiple storage tiers attached: pass one path per tier, either a block device or a file on the tier's mounted file system:

```bash
restore init -workload MSR_hm_1               # the trace's configuration (sizes, capacities), as in the quick start

# block devices (raw access needs root)
sudo env "PATH=$PATH" restore run -policy RL -workload MSR_hm_1 \
    -fast_device /dev/nvme1n1 -middle_device /dev/nvme2n1 -slow_device /dev/sdb

# files on the tiers' mounted file systems; create them once, large enough for the tier capacity (4 KB per page)
dd if=/dev/urandom of=/mnt/tier3/restore.dat bs=1M count=1024 oflag=direct    # likewise for tier 1 and 2
restore run -policy RL -workload MSR_hm_1 \
    -fast_device /mnt/tier1/restore.dat -middle_device /mnt/tier2/restore.dat -slow_device /mnt/tier3/restore.dat
```

Each run's log starts with the setup of each tier, e.g. `Tier1: simulated (read 30 us, write 45 us, 8 I/O threads)` or `Tier1: device /dev/nvme1n1 (8 I/O threads)`. If a given path cannot be used, the run stops with an error.

> [!WARNING]
> ReStore reads and writes the tier devices **directly** (4 KB `O_DIRECT` I/O starting at offset 0). On a block device, this destroys any partition table and file system on it. Only use dedicated, empty (e.g. emulated) devices, or files.
> As a safeguard, ReStore refuses a block device that, or a partition of which, is mounted or otherwise in use (e.g. the system disk, LVM, swap). If you are sure (e.g. scratch volumes that are mounted), set `RESTORE_ALLOW_BUSY_DEVICE=1`.

**FEMU.** In the paper, we use [FEMU](https://github.com/MoatLab/FEMU) to emulate different SSDs (configurations in Table 5 of the paper, see [Fig. 13](#fig-13-different-femu-ssds)); FEMU devices appear to the host as standard NVMe SSDs. Set up three FEMU NVMe devices following the FEMU documentation, precondition them with [`utils/precondition_3nvme.fio`](utils/precondition_3nvme.fio) (`sudo fio utils/precondition_3nvme.fio`, which overwrites 95% of each device; adjust the device names), and pass them as tiers as above.

Device paths can also be compiled in as defaults, with `cmake -DFAST_DEVICE_PATH=... -DMIDDLE_DEVICE_PATH=... -DSLOW_DEVICE_PATH=...` or `restore build -fast_device ... -middle_device ... -slow_device ...`.

## Reproducing the paper's experiments

The experiments are run with [`scripts/reproduce.sh`](scripts/reproduce.sh), which runs all policies for an experiment, writes the results to `results/<experiment>/`, and plots the figure there (the plotting scripts are in [`results/`](results/)). A run whose log already exists is skipped, so an interrupted experiment can be resumed by running it again. Run `scripts/reproduce.sh` without arguments for its usage.

**Option 1: Docker container.** Our container runs the experiments on simulated tiers that need no special hardware and finish within the review period (Figs. 7 and 8, i.e. `scripts/reproduce.sh container`, about 30 hours), and plots the figures, with a single command.

**Option 2: your own server.** [Build](#build) ReStore with [XGBoost-FastForest](#ml-based-policies), set up the [tier devices](#tier-devices) for the device experiments, and run:

```bash
scripts/reproduce.sh fig7 fig8                 # or any of: fig10 fig11 fig12 "fig13 <config>" fig15
```

| Figure | Experiment | Tiers | Command | In container | Time\* |
|--------|------------|-------|---------|--------------|--------|
| Fig. 7a, 7b | Static workloads, 6 tier-capacity settings | simulated | `fig7` | yes | ~9 h |
| Fig. 8 | Dynamic workloads A-D, 6 tier-capacity settings | simulated | `fig8` | yes | ~22 h |
| Fig. 10 | Scalability, 10^4 to 10^7 pages | simulated | `fig10` | no (long) | ~3 days |
| Fig. 11 | TPC-C, TPC-E, Google Thesios, YCSB | simulated | `fig11` | no (long; traces must be downloaded) | ~5 days |
| Fig. 12 | MSR traces vs Sibyl, single-threaded and concurrent | FEMU | `fig12` | no (devices) | ~4 h |
| Fig. 13 | MSR_wdev_0 on 4 FEMU configurations | FEMU | `fig13 <config>` | no (devices) | ~1 h per configuration |
| Fig. 15 | 10hf90, MSR_hm_1, Thesios on cloud volumes | cloud volumes | `fig15` | no (devices) | ~17 h |

\* Estimated from the run times of our experiments; the runs execute one after another. On simulated tiers, a run of a 10^6-request synthetic workload takes 1 to 10 minutes, depending on the policy and the tier capacities.

The ML-based policies use the models trained on 5% of each workload; for the workloads where no 5% model is available (XGB for the scalability workloads and TPC-C, both models for YCSB), the script uses the largest available one (2% or 1%), as training on 5% of these workloads is not feasible.

### Fig. 7: static workloads

`scripts/reproduce.sh fig7` runs the static workloads 5hf90, 10hf90, and 10hf80 with Tier 1/Tier 2 capacities of 1%/4%, 1%/8%, 3%/8%, 3%/16%, 6%/8%, and 12%/16% of the pages (default tier latencies 30/200/500 µs, asymmetry 1.5/2/4, concurrency 8/4/2), and plots `results/fig7/Results_capacity_tests_static_2x3_newcolor.pdf` (Fig. 7a, runtime) and `results/fig7/Migration_capacity_tests_static_2x3_newcolor.pdf` (Fig. 7b, migrations).

### Fig. 8: dynamic workloads

`scripts/reproduce.sh fig8` runs the dynamic workloads A-D with the same configurations as Fig. 7, and plots `results/fig8/Results_capacity_tests_changings_2x3_newcolor.pdf`.

### Fig. 10: scalability

Not included in the container because of its runtime (up to 10^8 requests per run). The larger 5hf90 workloads are not bundled (about 2 GB); generate them first:

```bash
cd workloads/synthetic
g++ -O3 -std=c++20 -o workload_gen workload_gen.cpp
./workload_gen 1e5 1e7 5hf90_1e5_rw1_1e7
./workload_gen 1e6 1e8 5hf90_1e6_rw1_1e8
./workload_gen 1e7 1e8 5hf90_1e7_rw1_1e8
cd ../.. && scripts/reproduce.sh fig10
```

This runs the four workloads with Tier 1/Tier 2 capacities of 3%/8% of the pages and plots `results/fig10/Results_scalability_tests_5hf90_new.pdf`.

### Fig. 11: real-world traces

Not included in the container because of the size of the traces and their runtime. Download and convert TPC-C, TPC-E, YCSB, and Google Thesios as described in [`workloads/traces/`](workloads/traces/), then run `scripts/reproduce.sh fig11`, which plots `results/fig11/Results_real_traces.pdf`. The tier capacities are 2M/8M/20M pages for TPC-C and TPC-E, 10M/20M/70M for YCSB, and 1.5K/5K/20K for Thesios.

### Fig. 12: comparison with Sibyl

Runs on three FEMU devices in the default configuration of Table 5 (read latency 30/200/500 µs). `scripts/reproduce.sh fig12` runs MSR_hm_1 and MSR_wdev_0 with single-threaded tiers (`hm_1_ST`, `wdev_0_ST`) and with concurrency 8/4/2 (`_Con`), and then plots `results/fig12/Results_Sibyl_traces.pdf`, which normalizes all latencies to [Sibyl](https://github.com/CMU-SAFARI/Sibyl) (Singh et al., ISCA 2022). Sibyl is a separate system and not part of this repository; we ran its public implementation under the same storage configuration. Add its results to each folder `results/fig12/Results_<workload>/num_threads{111,842}/5000-15000/` as `output_<workload>_Sibyl.log`, with the lines `Total requests time: <N> microseconds` and `For loop time: <N> microseconds`, and run `scripts/reproduce.sh plots`. (The plotting script assigns the logs to the bars by their alphabetical file order: `EXD`, `LFU`, `LRFU`, `LRU`, `LogiReg`, `RL`, `Sibyl`, `TEMP`, `XGBfast`.)

### Fig. 13: different FEMU SSDs

Configure the three FEMU devices as one of the configurations of Table 5 (`default`, `config1`, `config2`, `config3`) and run `scripts/reproduce.sh fig13 <config>`; this runs MSR_wdev_0 with the matching read latencies and concurrency, and writes the results to `results/fig13/<config>/`. Repeat for each configuration.

### Fig. 15: cloud volumes

Runs on three cloud volumes with different OpenStack QoS limits (fast: 33,333 read / 22,222 write IOPS; medium: 5,000 / 2,500; slow: 2,000 / 500), created e.g. with [`utils/create_openstack_volumes.sh`](utils/create_openstack_volumes.sh). `scripts/reproduce.sh fig15` runs 10hf90, MSR_hm_1, and Thesios (converted as for Fig. 11) and plots `results/fig15/Results_cloud_test.pdf`.

### Device experiments

`fig12`, `fig13`, and `fig15` take the tier devices from the environment; raw devices need root:

```bash
sudo env "PATH=$PATH" FAST_DEVICE=/dev/nvme1n1 MIDDLE_DEVICE=/dev/nvme2n1 SLOW_DEVICE=/dev/nvme3n1 scripts/reproduce.sh fig12
```

### Not included

- Fig. 1 summarizes results of Figs. 8 and 11.
- Fig. 9 (robustness to the read/write ratio) and Fig. 14 (extension to a 2-tier setup) are supplementary robustness and extensibility results and are not part of this artifact.
- Table 8 (CPU time and memory per policy) is not scripted; the policies' decision overhead is reported at the end of each log.

## Citation

If you use ReStore, please cite our SIGMOD 2026 paper:

```bibtex
@article{zhang2026restore,
  author    = {Zhang, Tianru and Papon, Tarikul Islam and Bagashvili, Teona and Toor, Salman and Athanassoulis, Manos},
  title     = {ReStore: A Reinforcement Learning Approach for Data Migration in Multi-Tiered Storage},
  journal   = {Proceedings of the ACM on Management of Data},
  volume    = {4},
  number    = {3},
  articleno = {227},
  numpages  = {28},
  year      = {2026},
  month     = jun,
  doi       = {10.1145/3802104},
  url       = {https://doi.org/10.1145/3802104}
}
```

Our VLDB demo of ReStore, *ReStore: RL Guided Data Migration in Tiered Storage*, is on the [ReStore demo page](https://disc-projects.bu.edu/ReStore/index.html).

## License

ReStore is released under the [MIT License](LICENSE). It includes [nlohmann/json](https://github.com/nlohmann/json) (`cpp/json.hpp`, MIT) and [BS::thread_pool](https://github.com/bshoshany/thread-pool) (`cpp/BS_thread_pool.hpp`, MIT).
