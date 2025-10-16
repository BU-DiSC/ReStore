#!/bin/bash

# Test script for measuring read/write latency on /dev/vdx using Sibyl library
# This script compiles the library with /dev/vdx and runs latency tests

set -e  # Exit on any error


# Configuration
DEVICE_PATH="/dev/vdb"
TEST_SIZE_KB=4096  # 4MB test size
NUM_ITERATIONS=100
TEST_OFFSET=0  # Start from beginning of device

echo "=== Latency Test for $DEVICE_PATH via Sibyl_driver ==="
echo "Date: $(date)"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if device exists
check_device() {
    print_status "Checking if device $DEVICE_PATH exists..."
    if [ ! -b "$DEVICE_PATH" ]; then
        print_error "Device $DEVICE_PATH does not exist or is not a block device"
        print_status "Available block devices:"
        ls -la /dev/vd* /dev/nvme* /dev/sd* 2>/dev/null || echo "No common block devices found"
        exit 1
    fi
    print_success "Device $DEVICE_PATH found"
}

# Check permissions
check_permissions() {
    print_status "Checking device permissions..."
    if [ ! -r "$DEVICE_PATH" ] || [ ! -w "$DEVICE_PATH" ]; then
        print_warning "Insufficient permissions for $DEVICE_PATH"
        print_status "You may need to run with sudo or change device permissions"
        print_status "Trying to run with sudo..."
        if [ "$EUID" -ne 0 ]; then
            print_error "This script needs to be run as root or with sudo for device access"
            exit 1
        fi
    fi
    print_success "Device permissions OK"
}

# Compile the test program
compile_test() {
    print_status "Compiling Sibyl library with $DEVICE_PATH..."
    
    # Create a temporary test program
    cat > test_latency.c << 'EOF'
#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

// Device paths will be defined at compile time

// Include the Sibyl functions
int openFastDevice();
int openMiddleDevice();
int openSlowDevice();
int sibyl_read(int fd, unsigned long byte_offset, unsigned int nSize);
int sibyl_write(int fd, unsigned long byte_offset, unsigned int nSize);
void closeDevice(int fd);

// Function to get current time in microseconds
long long get_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000LL + tv.tv_usec;
}

// Function to calculate statistics
void calculate_stats(long long *times, int count, double *avg, double *min, double *max, double *stddev) {
    long long sum = 0;
    *min = times[0];
    *max = times[0];
    
    for (int i = 0; i < count; i++) {
        sum += times[i];
        if (times[i] < *min) *min = times[i];
        if (times[i] > *max) *max = times[i];
    }
    
    *avg = (double)sum / count;
    
    // Calculate standard deviation
    double variance = 0;
    for (int i = 0; i < count; i++) {
        double diff = times[i] - *avg;
        variance += diff * diff;
    }
    *stddev = sqrt(variance / count);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <test_size_kb> <num_iterations> [offset]\n", argv[0]);
        printf("Example: %s 4096 100 0\n", argv[0]);
        return 1;
    }
    
    unsigned int test_size = atoi(argv[1]) * 1024;  // Convert KB to bytes
    int num_iterations = atoi(argv[2]);
    unsigned long offset = argc > 3 ? atol(argv[3]) : 0;
    
    printf("=== Sibyl Latency Test ===\n");
    printf("Device: %s\n", FAST_DEVICE_PATH);
    printf("Test size: %u bytes (%u KB)\n", test_size, test_size / 1024);
    printf("Iterations: %d\n", num_iterations);
    printf("Offset: %lu bytes\n", offset);
    printf("================================\n\n");
    
    // Open device
    int fd = openFastDevice(O_RDWR|O_SYNC|O_DIRECT);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }
    
    // Allocate arrays for timing data
    long long *read_times = malloc(num_iterations * sizeof(long long));
    long long *write_times = malloc(num_iterations * sizeof(long long));
    
    if (!read_times || !write_times) {
        perror("Memory allocation failed");
        closeDevice(fd);
        return 1;
    }
    
    printf("Starting latency measurements...\n");
    
    // Test write latency
    printf("\n--- Write Latency Test ---\n");
    for (int i = 0; i < num_iterations; i++) {
        long long start = get_time_us();
        int result = sibyl_write(fd, offset, test_size);
        long long end = get_time_us();
        
        if (result < 0) {
            printf("Write failed at iteration %d: %s\n", i, strerror(errno));
            continue;
        }
        
        write_times[i] = end - start;
        
        if (i % 10 == 0) {
            printf("Write iteration %d: %lld μs\n", i, write_times[i]);
        }
    }
    
    // Test read latency
    printf("\n--- Read Latency Test ---\n");
    for (int i = 0; i < num_iterations; i++) {
        long long start = get_time_us();
        int result = sibyl_read(fd, offset, test_size);
        long long end = get_time_us();
        
        if (result < 0) {
            printf("Read failed at iteration %d: %s\n", i, strerror(errno));
            continue;
        }
        
        read_times[i] = end - start;
        
        if (i % 10 == 0) {
            printf("Read iteration %d: %lld μs\n", i, read_times[i]);
        }
    }
    
    // Calculate and display statistics
    double write_avg, write_min, write_max, write_stddev;
    double read_avg, read_min, read_max, read_stddev;
    
    calculate_stats(write_times, num_iterations, &write_avg, &write_min, &write_max, &write_stddev);
    calculate_stats(read_times, num_iterations, &read_avg, &read_min, &read_max, &read_stddev);
    
    printf("\n=== RESULTS ===\n");
    printf("Write Latency (μs):\n");
    printf("  Average: %.2f\n", write_avg);
    printf("  Min:     %.2f\n", write_min);
    printf("  Max:     %.2f\n", write_max);
    printf("  StdDev:  %.2f\n", write_stddev);
    
    printf("\nRead Latency (μs):\n");
    printf("  Average: %.2f\n", read_avg);
    printf("  Min:     %.2f\n", read_min);
    printf("  Max:     %.2f\n", read_max);
    printf("  StdDev:  %.2f\n", read_stddev);
    
    // Calculate throughput
    double write_throughput = (test_size / 1024.0 / 1024.0) / (write_avg / 1000000.0);  // MB/s
    double read_throughput = (test_size / 1024.0 / 1024.0) / (read_avg / 1000000.0);   // MB/s
    
    printf("\nThroughput:\n");
    printf("  Write: %.2f MB/s\n", write_throughput);
    printf("  Read:  %.2f MB/s\n", read_throughput);
    
    // Cleanup
    closeDevice(fd);
    free(read_times);
    free(write_times);
    
    printf("\nTest completed successfully!\n");
    return 0;
}
EOF

    # Compile the test program with the Sibyl library
    print_status "Compiling test program..."
    gcc -O2 -DFAST_DEVICE_PATH=\"$DEVICE_PATH\" -DMIDDLE_DEVICE_PATH=\"$DEVICE_PATH\" -DSLOW_DEVICE_PATH=\"$DEVICE_PATH\" -o test_latency test_latency.c Sibyl_lib_configurable.c -lrt -lm
    
    if [ $? -eq 0 ]; then
        print_success "Compilation successful"
    else
        print_error "Compilation failed"
        exit 1
    fi
}

# Run the latency test
run_test() {
    print_status "Running latency test..."
    echo ""
    
    # Run the test with specified parameters
    ./test_latency $TEST_SIZE_KB $NUM_ITERATIONS $TEST_OFFSET
    
    if [ $? -eq 0 ]; then
        print_success "Latency test completed successfully"
    else
        print_error "Latency test failed"
        exit 1
    fi
}

# Cleanup function
cleanup() {
    print_status "Cleaning up..."
    rm -f test_latency.c test_latency
    print_success "Cleanup completed"
}

# Main execution
main() {
    echo "Starting Sibyl latency test for $DEVICE_PATH"
    echo "Test parameters:"
    echo "  Device: $DEVICE_PATH"
    echo "  Test size: ${TEST_SIZE_KB}KB"
    echo "  Iterations: $NUM_ITERATIONS"
    echo "  Offset: ${TEST_OFFSET} bytes"
    echo ""
    
    check_device
    check_permissions
    compile_test
    run_test
    cleanup
    
    print_success "All tests completed successfully!"
}

# Handle script arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -s|--size)
            TEST_SIZE_KB="$2"
            shift 2
            ;;
        -n|--iterations)
            NUM_ITERATIONS="$2"
            shift 2
            ;;
        -o|--offset)
            TEST_OFFSET="$2"
            shift 2
            ;;
        -d|--device)
            DEVICE_PATH="$2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  -s, --size SIZE        Test size in KB (default: 4096)"
            echo "  -n, --iterations NUM   Number of iterations (default: 100)"
            echo "  -o, --offset OFFSET    Starting offset in bytes (default: 0)"
            echo "  -d, --device DEVICE    Device path (default: /dev/vdb)"
            echo "  -h, --help             Show this help message"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Use -h or --help for usage information"
            exit 1
            ;;
    esac
done

# Run main function
main
