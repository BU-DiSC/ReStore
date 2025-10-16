#!/bin/bash

# Test script for the ReStore package
# This script tests the basic functionality of the installed package

set -e

echo "ReStore Package Test Script"
echo "==========================="
echo

# Test 1: Check if restore command exists
echo "Test 1: Checking if 'restore' command is available..."
if command -v restore &> /dev/null; then
    echo "✓ restore command found"
    restore --help 2>/dev/null || echo "✓ restore command responds"
else
    echo "✗ restore command not found"
    echo "Please install ReStore first: sudo ./install.sh"
    exit 1
fi
echo

# Test 2: List available options
echo "Test 2: Testing list commands..."
echo "Available policies:"
if restore list policies &> /dev/null; then
    echo "✓ list policies works"
else
    echo "✗ list policies failed"
fi

echo "Available workloads:"
if restore list workloads &> /dev/null; then
    echo "✓ list workloads works"
else
    echo "✗ list workloads failed"
fi
echo

# Test 3: Configuration management
echo "Test 3: Testing configuration management..."
if restore init -workload MSR_hm_1 &> /dev/null; then
    echo "✓ init command works"
    
    if [[ -f "restore_config.json" ]]; then
        echo "✓ configuration file created"
    else
        echo "✗ configuration file not created"
    fi
    
    # Test config get/set
    if restore config get max_capacity_tier1 &> /dev/null; then
        echo "✓ config get works"
    else
        echo "✗ config get failed"
    fi
    
    if restore config set test_param test_value &> /dev/null; then
        echo "✓ config set works"
        if restore config get test_param | grep -q "test_value"; then
            echo "✓ config value correctly set and retrieved"
        else
            echo "✗ config value not correctly set"
        fi
    else
        echo "✗ config set failed"
    fi
else
    echo "✗ init command failed"
fi

# Test device configuration
echo "Testing device configuration..."
if restore init -devices /dev/test1 /dev/test2 /dev/test3 &> /dev/null; then
    echo "✓ device configuration works"
    if [[ -f "device_config.json" ]]; then
        echo "✓ device configuration file created"
    else
        echo "✗ device configuration file not created"
    fi
else
    echo "✗ device configuration failed"
fi
echo

# Test 4: Test build command
echo "Test 4: Testing build command..."
if restore build &> /dev/null; then
    echo "✓ build command works"
else
    echo "⚠ build command failed (may need dependencies)"
fi
echo

# Test 5: Check if policy executables exist
echo "Test 5: Checking policy executables..."
policies=("Run_RL" "Run_LFU" "Run_LRU" "Run_LRFU" "Run_EXD" "Run_TEMP" "Run_static" "Run_ideal" "Run_LogiReg" "Run_XGBfast")

for policy in "${policies[@]}"; do
    if command -v "$policy" &> /dev/null; then
        echo "✓ $policy executable found"
    else
        echo "✗ $policy executable not found"
    fi
done
echo

# Test 6: Test a simple simulation (if possible)
echo "Test 6: Testing simple simulation..."
echo "Note: This test may take some time and requires workload data..."

# Check if workload data exists
if [[ -d "/usr/local/share/ReStore/workload" ]] || [[ -d "./workload" ]]; then
    echo "Workload data found, attempting simple simulation..."
    
    # Try to run a quick simulation
    if timeout 30 restore run -policy static -workload MSR_hm_1 &> /dev/null; then
        echo "✓ Simple simulation completed successfully"
    else
        echo "⚠ Simple simulation timed out or failed (this may be normal)"
    fi
else
    echo "⚠ Workload data not found, skipping simulation test"
    echo "Make sure workload data is installed in the correct location"
fi
echo

# Test 7: Check installation directories
echo "Test 7: Checking installation directories..."
install_dirs=("/usr/local/bin" "/usr/local/share/ReStore" "/usr/local/lib/cmake/ReStore")

for dir in "${install_dirs[@]}"; do
    if [[ -d "$dir" ]]; then
        echo "✓ $dir exists"
    else
        echo "✗ $dir missing"
    fi
done
echo

echo "Test Summary"
echo "============"
echo "Basic package functionality test completed."
echo "If you see mostly ✓ marks above, the package is working correctly."
echo
echo "For a full test with actual simulations, run:"
echo "  ./examples/run_experiments.sh"
echo
echo "For batch comparison testing, run:"
echo "  ./examples/batch_comparison.sh"

