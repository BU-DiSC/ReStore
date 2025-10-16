#!/bin/bash

# Example script demonstrating device configuration and building
# This script shows how to configure different device paths and rebuild

set -e

echo "ReStore Device Configuration Example"
echo "===================================="
echo

# Check if restore command is available
if ! command -v restore &> /dev/null; then
    echo "Error: 'restore' command not found. Please install ReStore first."
    echo "Run: sudo ./install.sh"
    exit 1
fi

echo "1. Listing current device configuration..."
if [[ -f "device_config.json" ]]; then
    echo "Current device configuration:"
    cat device_config.json
else
    echo "No device configuration found (using defaults)"
fi
echo

echo "2. Configuring custom device paths..."
echo "Setting up devices: /dev/sda (fast), /dev/sdb (middle), /dev/sdc (slow)"
restore init -devices /dev/sda /dev/sdb /dev/sdc
echo

echo "3. Verifying device configuration..."
if [[ -f "device_config.json" ]]; then
    echo "Device configuration saved:"
    cat device_config.json
else
    echo "Error: Device configuration file not created"
    exit 1
fi
echo

echo "4. Building with new device configuration..."
echo "This will compile the Sibyl library with the new device paths..."
restore build
echo

echo "5. Testing with different device configurations..."

# Example 1: NVMe devices
echo "Example 1: NVMe devices"
restore init -devices /dev/nvme0n1 /dev/nvme1n1 /dev/nvme2n1
restore build
echo

# Example 2: SATA devices
echo "Example 2: SATA devices"
restore init -devices /dev/sda /dev/sdb /dev/sdc
restore build
echo

# Example 3: Mixed devices
echo "Example 3: Mixed devices (NVMe + SATA)"
restore init -devices /dev/nvme0n1 /dev/sda /dev/sdb
restore build
echo

echo "6. Building with command-line device specification..."
echo "Building with custom device paths directly:"
restore build -fast_device /dev/nvme0n1 -middle_device /dev/nvme1n1 -slow_device /dev/nvme2n1
echo

echo "7. Running a simulation with configured devices..."
echo "Initializing workload configuration..."
restore init -workload MSR_hm_1
echo

echo "Running a simple simulation..."
restore run -policy static -workload MSR_hm_1
echo

echo "Device configuration example completed!"
echo
echo "Summary of what was demonstrated:"
echo "- Device path configuration with 'restore init -devices'"
echo "- Building with device configuration using 'restore build'"
echo "- Command-line device specification"
echo "- Running simulations with configured devices"
echo
echo "The Sibyl library is now compiled with your device paths and"
echo "will use them when accessing storage devices during simulations."
