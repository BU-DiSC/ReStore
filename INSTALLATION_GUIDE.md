# ReStore Package Installation Guide

## Overview

This guide explains how to transform your ReStore project from individual executables into a unified, installable package. The package provides a single `restore` command that can run any policy with any workload using a clean, consistent interface.

## What's Been Created

### 1. Main CLI Application (`src/main.cpp`)
- Unified command-line interface
- Commands: `init`, `run`, `list`, `config`
- Handles argument parsing and delegation

### 2. Configuration System (`src/config_manager.*`)
- Manages workload-specific default configurations
- Supports configuration file persistence
- Provides easy parameter management

### 3. Simulator Interface (`src/simulator.*`)
- Abstracts policy execution
- Provides consistent interface for all policies
- Handles command-line argument building

### 4. Build System (`CMakeLists.txt`)
- Modern CMake-based build system
- Builds all executables and the main CLI
- Supports installation and packaging

### 5. Installation Script (`install.sh`)
- Automated installation process
- Dependency checking
- System-wide or user installation

### 6. Documentation and Examples
- Comprehensive usage documentation
- Example scripts for common use cases
- Migration guide from old scripts

## Installation Process

### Quick Start
```bash
# Make installation script executable
chmod +x install.sh

# Install system-wide (requires sudo)
sudo ./install.sh

# Or install to user directory
./install.sh --prefix ~/.local
```

### Manual Installation
```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local

# Build
make -j$(nproc)

# Install
sudo make install
```

## Usage Examples

### Before (Old Way)
```bash
# Multiple separate commands
./Run_RL -workload=MSR_hm_1 -max_capacity_tier1=5000 -max_capacity_tier2=15000 -total_num_pages=51733
./Run_LFU -workload=MSR_hm_1 -max_capacity_tier1=5000 -max_capacity_tier2=15000 -total_num_pages=51733
./Run_LRU -workload=MSR_hm_1 -max_capacity_tier1=5000 -max_capacity_tier2=15000 -total_num_pages=51733
```

### After (New Package)
```bash
# Initialize configuration once
restore init -workload MSR_hm_1

# Run different policies with same configuration
restore run -policy RL -workload MSR_hm_1
restore run -policy LFU -workload MSR_hm_1
restore run -policy LRU -workload MSR_hm_1

# Or override specific parameters
restore run -policy RL -workload MSR_hm_1 -max_capacity_tier1 8000
```

## Key Benefits

### 1. Unified Interface
- Single `restore` command for all operations
- Consistent argument format across all policies
- Easy to remember and use

### 2. Configuration Management
- Default configurations for each workload
- Persistent configuration files
- Easy parameter modification

### 3. Better Organization
- All policies accessible through one interface
- Centralized configuration management
- Cleaner project structure

### 4. Easy Installation
- One-command installation
- System-wide or user installation
- Automatic dependency checking

### 5. Extensibility
- Easy to add new policies
- Simple to add new workloads
- Modular design

## File Structure After Installation

```
/usr/local/
├── bin/
│   ├── restore              # Main CLI command
│   ├── Run_RL              # Policy executables
│   ├── Run_LFU
│   ├── Run_LRU
│   └── ... (other policies)
├── share/ReStore/
│   ├── workload/           # Workload data
│   └── results/            # Results templates
└── lib/
    ├── cmake/ReStore/      # CMake configuration
    └── pkgconfig/          # pkg-config files
```

## Testing the Installation

### Basic Test
```bash
# Test if restore command works
restore list policies
```

### Full Test
```bash
# Run the test script
./examples/test_package.sh
```

### Example Usage
```bash
# Run example experiments
./examples/run_experiments.sh
```

## Migration from Existing Scripts

### 1. Update Your Scripts
Replace calls like:
```bash
./Run_RL -workload=MSR_hm_1 -max_capacity_tier1=5000
```

With:
```bash
restore run -policy RL -workload MSR_hm_1 -max_capacity_tier1 5000
```

### 2. Use Configuration Files
Instead of passing all parameters every time:
```bash
# Initialize once
restore init -workload MSR_hm_1

# Then just specify policy
restore run -policy RL -workload MSR_hm_1
```

### 3. Batch Operations
Use the provided example scripts for batch processing:
```bash
./examples/batch_comparison.sh
```

## Troubleshooting

### Common Issues

1. **Command not found**: Add installation directory to PATH
   ```bash
   export PATH="/usr/local/bin:$PATH"
   ```

2. **Permission denied**: Check file permissions
   ```bash
   chmod +x /usr/local/bin/restore
   ```

3. **Missing dependencies**: Install required libraries
   ```bash
   sudo apt-get install libfastforest-dev
   ```

4. **Build errors**: Check compiler version
   ```bash
   g++ --version  # Should be >= 9.0
   ```

### Getting Help
- Check the `PACKAGE_README.md` for detailed usage
- Run `restore --help` for command help
- Use `restore list policies` to see available options

## Next Steps

1. **Install the package**: Run `sudo ./install.sh`
2. **Test the installation**: Run `./examples/test_package.sh`
3. **Try examples**: Run `./examples/run_experiments.sh`
4. **Run experiments**: run experiments with `restore` commands
5. **Customize configurations**: Use `restore init` and `restore config` commands

## Support

For issues or questions:
- Check the troubleshooting section
- Review log files in Results_* directories
- Ensure all dependencies are installed
- Verify workload data is available


