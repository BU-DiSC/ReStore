# ReStore Package Installation and Usage

This document explains how to install and use the ReStore package as a unified command-line tool.

## Installation

### Quick Install
```bash
# Clone the repository
git clone <repository-url>
cd ReStore

# Run the installation script
sudo ./install.sh
```

### Custom Installation
```bash
# Install to a custom directory
./install.sh --prefix /opt/restore

# Use custom build directory
./install.sh --build-dir my_build
```

### Manual Installation
```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local

# Build and install
make -j$(nproc)
sudo make install
```

## Usage

### Basic Commands

#### List Available Options
```bash
# List all available policies
restore list policies

# List all available workloads
restore list workloads
```

#### Initialize Configuration
```bash
# Initialize default configuration for a workload
restore init -workload MSR_hm_1

# Initialize with custom parameters
restore init -workload MSR_hm_1 -max_capacity_tier1 10000 -max_capacity_tier2 20000

# Configure device paths for Sibyl library
restore init -devices /dev/nvme0n1 /dev/nvme1n1 /dev/nvme2n1
```

#### Run Simulations
```bash
# Run with default configuration
restore run -policy LFU -workload MSR_hm_1

# Run with custom parameters
restore run -policy RL -workload MSR_hm_1 -max_capacity_tier1 5000 -RL_update_freqs 1000

# Run multiple policies on the same workload
restore run -policy LFU -workload MSR_hm_1
restore run -policy LRU -workload MSR_hm_1
restore run -policy RL -workload MSR_hm_1
```

#### Configuration Management
```bash
# Get a configuration value
restore config get max_capacity_tier1

# Set a configuration value
restore config set max_capacity_tier1 10000

# View current configuration
cat restore_config.json
```

#### Building and Rebuilding
```bash
# Build with default device paths
restore build

# Build with custom device paths
restore build -fast_device /dev/sda -middle_device /dev/sdb -slow_device /dev/sdc

# Rebuild after changing device configuration
restore init -devices /dev/sda /dev/sdb /dev/sdc
restore build
```

### Advanced Usage

#### Batch Processing
```bash
#!/bin/bash
# Run multiple policies on multiple workloads

workloads=("MSR_hm_1" "MSR_wdev_0" "MSR_mds_0")
policies=("LFU" "LRU" "RL" "EXD")

for workload in "${workloads[@]}"; do
    for policy in "${policies[@]}"; do
        echo "Running $policy on $workload"
        restore run -policy $policy -workload $workload
    done
done
```

#### Custom Configuration Files
```bash
# Create a custom configuration
restore init -workload MSR_hm_1 -max_capacity_tier1 8000 -max_capacity_tier2 16000

# Copy and modify configuration
cp restore_config.json my_custom_config.json
# Edit my_custom_config.json as needed

# Use custom configuration
RESTORE_CONFIG=my_custom_config.json restore run -policy RL -workload MSR_hm_1
```

## Available Policies

| Policy | Description | Executable |
|--------|-------------|------------|
| RL | Reinforcement Learning | Run_RL |
| LFU | Least Frequently Used | Run_LFU |
| LRU | Least Recently Used | Run_LRU |
| LRFU | Least Recently/Frequently Used | Run_LRFU |
| EXD | EXponential Decay | Run_EXD |
| TEMP | Temperature-based | Run_TEMP |
| static | Static allocation | Run_static |
| ideal | Ideal (oracle) policy | Run_ideal |
| LogiReg | Logistic Regression | Run_LogiReg |
| XGBfast | XGBoost Fast | Run_XGBfast |

## Available Workloads

| Workload | Description | Default Configuration |
|----------|-------------|---------------------|
| MSR_hm_1 | MSR Cambridge Home Machine 1 | 5K/15K/60K tiers, 51K pages |
| MSR_wdev_0 | MSR Cambridge Web Development 0 | 5K/15K/60K tiers, 79K pages |
| MSR_mds_0 | MSR Cambridge Media Server 0 | 80K/240K/900K tiers, 802K pages |

## Configuration Parameters

### Device Configuration
- `fast_device`: Path to fast storage device (default: `/dev/nvme0n1`)
- `middle_device`: Path to middle storage device (default: `/dev/nvme1n1`)
- `slow_device`: Path to slow storage device (default: `/dev/nvme2n1`)

### Tier Configuration
- `max_capacity_tier1`, `max_capacity_tier2`, `max_capacity_tier3`: Maximum capacity for each tier
- `read_time_tier1`, `read_time_tier2`, `read_time_tier3`: Read time for each tier

### Workload Configuration
- `total_num_pages`: Total number of pages in the workload
- `total_num_reqs`: Total number of requests in the workload

### Policy-Specific Parameters

#### Reinforcement Learning (RL)
- `RL_update_freqs`: Frequency of RL updates
- `RL_init_rounds`: Number of initialization rounds
- `a_b_update_freq_s1`, `a_b_update_freq_s2`: Update frequencies for states
- `num_elements_to_consider_s1`, `num_elements_to_consider_s2`: Elements to consider

#### Temperature-based (TEMP)
- `temp_incr_alpha`: Temperature increment alpha
- `temp_incr_buffersize`: Temperature increment buffer size
- `temp_drop_thrd`: Temperature drop threshold
- `temp_drop_freqs`: Temperature drop frequency

## Output and Results

### Log Files
Results are automatically saved to:
```
Results_<workload>/capacity_tests_<read_times>/<capacities>/output_<workload>_<policy>.log
```

### Configuration Files
- Default: `restore_config.json`
- Custom: Specify with `RESTORE_CONFIG` environment variable

## Troubleshooting

### Common Issues

1. **Command not found**: Add installation directory to PATH
   ```bash
   export PATH="/usr/local/bin:$PATH"
   ```

2. **Permission denied**: Ensure executables have proper permissions
   ```bash
   chmod +x /usr/local/bin/restore
   ```

3. **Missing dependencies**: Install required libraries
   ```bash
   sudo apt-get install libfastforest-dev  # For XGBfast policy
   ```

4. **Workload files not found**: Ensure workload data is installed
   ```bash
   ls /usr/local/share/ReStore/workload/
   ```

### Debug Mode
Run with verbose output:
```bash
restore run -policy RL -workload MSR_hm_1 --verbose
```

## Migration from Old Scripts

### Before (Old Script)
```bash
./Run_RL -workload=MSR_hm_1 -max_capacity_tier1=5000 -max_capacity_tier2=15000 -total_num_pages=51733
```

### After (New Package)
```bash
restore run -policy RL -workload MSR_hm_1 -max_capacity_tier1 5000 -max_capacity_tier2 15000 -total_num_pages 51733
```

## Development

### Building from Source
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

### Adding New Policies
1. Create new `Run_NewPolicy.cpp` in `cpp/Run_IO/`
2. Add to `EXECUTABLES` list in `CMakeLists.txt`
3. Create `NewPolicyRunner` class in `src/simulator.cpp`
4. Add to policy runners map

### Adding New Workloads
1. Add workload data to `workload/` directory
2. Add default configuration to `ConfigManager::initializeWorkloadDefaults()`
3. Update documentation

## Support

For issues and questions:
- Check the troubleshooting section above
- Review log files in the results directory
- Ensure all dependencies are installed
- Verify workload data is available

