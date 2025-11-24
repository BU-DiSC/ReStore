# ReStore Package Installation Guide


## Overview

This guide explains how to install the ReStore package, which provides a single `restore` command that can run any policy with any workload using a clean, consistent interface.


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


## Support

For issues or questions:
- Check the troubleshooting section
- Review log files in Results_* directories
- Ensure all dependencies are installed
- Verify workload data is available


