#!/bin/bash

# OpenStack Volume Creation Script for Cloud Testing
# Creates three volumes with different performance characteristics

# Manual creation
openstack volume create \
    --size 10 \
    --type "rbd_alburnus" \
    --property "read_iops_sec=33333" \
    --property "write_iops_sec=22222" \
    --property "read_bytes_sec=136533333" \
    --property "write_bytes_sec=91022222" \
    "fast_tier"
openstack volume create \
    --size 10 \
    --type "rbd_alburnus" \
    --property "read_iops_sec=5000" \
    --property "write_iops_sec=2500" \
    --property "read_bytes_sec=20480000" \
    --property "write_bytes_sec=10240000" \
    "middle_tier"
openstack volume create \
    --size 10 \
    --type "rbd_alburnus" \
    --property "read_iops_sec=2000" \
    --property "write_iops_sec=500" \
    --property "read_bytes_sec=8192000" \
    --property "write_bytes_sec=2048000" \
    "slow_tier"

set -e  # Exit on any error

# Configuration
VOLUME_SIZE_GB=10
VOLUME_PREFIX="restore"
INSTANCE_NAME=$(hostname)
PROJECT_ID=$(openstack project show -f value -c id $(openstack token issue -f value -c project_id) 2>/dev/null || echo "")

# Performance tier configurations
# Fast tier: High IOPS, high throughput
FAST_IOPS_READ=33333       # 30μs = ~33,333 IOPS theoretical max
FAST_IOPS_WRITE=22222      # 45μs = ~22,222 IOPS theoretical max
FAST_BYTES_READ=136533333  # 4KB * 33,333 IOPS
FAST_BYTES_WRITE=91022222  # 4KB * 22,222 IOPS

# Middle tier: Medium IOPS, medium throughput
MIDDLE_IOPS_READ=5000       # 200μs = ~5000 IOPS theoretical max
MIDDLE_IOPS_WRITE=2500      # 400μs = ~2500 IOPS theoretical max
MIDDLE_BYTES_READ=20480000  # 4KB * 5000 IOPS
MIDDLE_BYTES_WRITE=10240000 # 4KB * 2500 IOPS

# Slow tier: Low IOPS, low throughput
SLOW_IOPS_READ=2000         # 500μs = ~2000 IOPS theoretical max
SLOW_IOPS_WRITE=500         # 2000μs = ~500 IOPS theoretical max
SLOW_BYTES_READ=8192000     # 4KB * 2000 IOPS
SLOW_BYTES_WRITE=2048000    # 4KB * 500 IOPS

# Concurrency settings for different tiers
FAST_CONCURRENCY=16          # High concurrency for fast tier
MIDDLE_CONCURRENCY=8         # Medium concurrency for middle tier
SLOW_CONCURRENCY=2           # Low concurrency for slow tier


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

# Check if OpenStack CLI is available
check_openstack_cli() {
    print_status "Checking OpenStack CLI availability..."
    if ! command -v openstack &> /dev/null; then
        print_error "OpenStack CLI not found. Please install it first:"
        echo "  pip install python-openstackclient"
        echo "  or"
        echo "  apt install python3-openstackclient"
        exit 1
    fi
    print_success "OpenStack CLI found"
}

# Check authentication
check_auth() {
    print_status "Checking OpenStack authentication..."
    if ! openstack token issue &> /dev/null; then
        print_error "Not authenticated with OpenStack. Please run:"
        echo "  source your-openstack-rc-file"
        echo "  or"
        echo "  export OS_AUTH_URL=..."
        echo "  export OS_USERNAME=..."
        echo "  export OS_PASSWORD=..."
        echo "  export OS_PROJECT_NAME=..."
        exit 1
    fi
    print_success "OpenStack authentication OK"
}

# Get current instance ID
get_instance_id() {
    print_status "Getting current instance information..."
    
    # Try to get instance ID from metadata
    INSTANCE_ID=$(curl -s http://169.254.169.254/openstack/latest/meta_data.json | python3 -c "import sys, json; print(json.load(sys.stdin)['uuid'])" 2>/dev/null || echo "")
    
    if [ -z "$INSTANCE_ID" ]; then
        # Fallback: try to find instance by name
        INSTANCE_ID=$(openstack server list --name "$INSTANCE_NAME" -f value -c ID 2>/dev/null | head -1 || echo "")
    fi
    
    if [ -z "$INSTANCE_ID" ]; then
        print_error "Could not determine instance ID. Please provide it manually:"
        echo "  export INSTANCE_ID=your-instance-id"
        echo "  or run: openstack server list"
        exit 1
    fi
    
    print_success "Instance ID: $INSTANCE_ID"
}

# Set volume type to rbd_alburnus
set_volume_type() {
    print_status "Setting volume type to rbd_alburnus..."
    
    VOLUME_TYPE="rbd_alburnus"
    
    # Check if rbd_alburnus type exists
    if openstack volume type show "$VOLUME_TYPE" &>/dev/null; then
        print_success "Volume type $VOLUME_TYPE found"
    else
        print_warning "Volume type $VOLUME_TYPE not found. Available types:"
        openstack volume type list -f table
        print_error "Please ensure rbd_alburnus volume type exists or modify the script"
        exit 1
    fi
}

# Create volumes with performance limits
create_volumes() {
    print_status "Creating test volumes with performance limits..."
    
    # Create fast volume with high performance limits
    print_status "Creating fast volume (${VOLUME_PREFIX}-fast) with high IOPS/throughput..."
    FAST_VOLUME_ID=$(openstack volume create \
        --size $VOLUME_SIZE_GB \
        --type "$VOLUME_TYPE" \
        --property "read_iops_sec=$FAST_IOPS_READ" \
        --property "write_iops_sec=$FAST_IOPS_WRITE" \
        --property "read_bytes_sec=$FAST_BYTES_READ" \
        --property "write_bytes_sec=$FAST_BYTES_WRITE" \
        --description "ReStore test - Fast volume (High IOPS/Throughput/Concurrency)" \
        "${VOLUME_PREFIX}-fast" \
        -f value -c id)
    
    if [ -z "$FAST_VOLUME_ID" ]; then
        print_error "Failed to create fast volume"
        exit 1
    fi
    print_success "Fast volume created: $FAST_VOLUME_ID"
    print_status "Fast volume limits: ${FAST_IOPS_READ} read IOPS, ${FAST_IOPS_WRITE} write IOPS, ${FAST_BYTES_READ} bytes/sec"
    
    # Create middle volume with medium performance limits
    print_status "Creating middle volume (${VOLUME_PREFIX}-middle) with medium IOPS/throughput..."
    MIDDLE_VOLUME_ID=$(openstack volume create \
        --size $VOLUME_SIZE_GB \
        --type "$VOLUME_TYPE" \
        --property "read_iops_sec=$MIDDLE_IOPS_READ" \
        --property "write_iops_sec=$MIDDLE_IOPS_WRITE" \
        --property "read_bytes_sec=$MIDDLE_BYTES_READ" \
        --property "write_bytes_sec=$MIDDLE_BYTES_WRITE" \
        --description "ReStore test - Middle volume (Medium IOPS/Throughput/Concurrency)" \
        "${VOLUME_PREFIX}-middle" \
        -f value -c id)
    
    if [ -z "$MIDDLE_VOLUME_ID" ]; then
        print_error "Failed to create middle volume"
        exit 1
    fi
    print_success "Middle volume created: $MIDDLE_VOLUME_ID"
    print_status "Middle volume limits: ${MIDDLE_IOPS_READ} read IOPS, ${MIDDLE_IOPS_WRITE} write IOPS, ${MIDDLE_BYTES_READ} bytes/sec"
    
    # Create slow volume with low performance limits
    print_status "Creating slow volume (${VOLUME_PREFIX}-slow) with low IOPS/throughput..."
    SLOW_VOLUME_ID=$(openstack volume create \
        --size $VOLUME_SIZE_GB \
        --type "$VOLUME_TYPE" \
        --property "read_iops_sec=$SLOW_IOPS_READ" \
        --property "write_iops_sec=$SLOW_IOPS_WRITE" \
        --property "read_bytes_sec=$SLOW_BYTES_READ" \
        --property "write_bytes_sec=$SLOW_BYTES_WRITE" \
        --description "ReStore test - Slow volume (Low IOPS/Throughput/Concurrency)" \
        "${VOLUME_PREFIX}-slow" \
        -f value -c id)
    
    if [ -z "$SLOW_VOLUME_ID" ]; then
        print_error "Failed to create slow volume"
        exit 1
    fi
    print_success "Slow volume created: $SLOW_VOLUME_ID"
    print_status "Slow volume limits: ${SLOW_IOPS_READ} read IOPS, ${SLOW_IOPS_WRITE} write IOPS, ${SLOW_BYTES_READ} bytes/sec"
}

# Wait for volumes to be available
wait_for_volumes() {
    print_status "Waiting for volumes to be available..."
    
    for volume_id in "$FAST_VOLUME_ID" "$MIDDLE_VOLUME_ID" "$SLOW_VOLUME_ID"; do
        print_status "Waiting for volume $volume_id..."
        while true; do
            status=$(openstack volume show $volume_id -f value -c status 2>/dev/null || echo "error")
            if [ "$status" = "available" ]; then
                print_success "Volume $volume_id is available"
                break
            elif [ "$status" = "error" ]; then
                print_error "Volume $volume_id is in error state"
                exit 1
            else
                print_status "Volume $volume_id status: $status (waiting...)"
                sleep 5
            fi
        done
    done
}

# Attach volumes to instance
attach_volumes() {
    print_status "Attaching volumes to instance..."
    
    # Attach fast volume
    print_status "Attaching fast volume..."
    openstack server add volume "$INSTANCE_ID" "$FAST_VOLUME_ID" --device /dev/vdb
    print_success "Fast volume attached as /dev/vdb"
    
    # Attach middle volume
    print_status "Attaching middle volume..."
    openstack server add volume "$INSTANCE_ID" "$MIDDLE_VOLUME_ID" --device /dev/vdc
    print_success "Middle volume attached as /dev/vdc"
    
    # Attach slow volume
    print_status "Attaching slow volume..."
    openstack server add volume "$INSTANCE_ID" "$SLOW_VOLUME_ID" --device /dev/vdd
    print_success "Slow volume attached as /dev/vdd"
}

# Wait for volumes to appear in the system
wait_for_devices() {
    print_status "Waiting for devices to appear in the system..."
    
    for device in /dev/vdb /dev/vdc /dev/vdd; do
        print_status "Waiting for $device..."
        timeout=60
        while [ $timeout -gt 0 ]; do
            if [ -b "$device" ]; then
                print_success "$device is available"
                break
            fi
            sleep 1
            timeout=$((timeout - 1))
        done
        
        if [ $timeout -eq 0 ]; then
            print_error "Timeout waiting for $device"
            exit 1
        fi
    done
}

# Create filesystems and mount points
setup_filesystems() {
    print_status "Setting up filesystems..."
    
    # Create mount points
    sudo mkdir -p /mnt/restore-fast
    sudo mkdir -p /mnt/restore-middle
    sudo mkdir -p /mnt/restore-slow
    
    # Create filesystems (ext4 for compatibility)
    print_status "Creating filesystem on /dev/vdb..."
    sudo mkfs.ext4 -F /dev/vdb
    print_status "Creating filesystem on /dev/vdc..."
    sudo mkfs.ext4 -F /dev/vdc
    print_status "Creating filesystem on /dev/vdd..."
    sudo mkfs.ext4 -F /dev/vdd
    
    # Mount volumes
    print_status "Mounting volumes..."
    sudo mount /dev/vdb /mnt/restore-fast
    sudo mount /dev/vdc /mnt/restore-middle
    sudo mount /dev/vdd /mnt/restore-slow
    
    # Set permissions
    sudo chown -R $USER:$USER /mnt/restore-*
    
    print_success "All volumes mounted and ready for testing"
}

# Create test script for the new volumes
create_volume_test_script() {
    print_status "Creating test script for the new volumes..."
    
    cat > test_volume_latency.sh << 'EOF'
#!/bin/bash

# Test script for Sibyl latency testing on mounted volumes
# This script tests the raw block devices, not the mounted filesystems

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_status() { echo -e "${BLUE}[INFO]${NC} $1"; }
print_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
print_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
print_error() { echo -e "${RED}[ERROR]${NC} $1"; }

# Test parameters
TEST_SIZE_KB=4096
NUM_ITERATIONS=50
TEST_OFFSET=0

# Device mapping
DEVICES=(
    "/dev/vdb:Fast"
    "/dev/vdc:Middle" 
    "/dev/vdd:Slow"
)

print_status "Starting Sibyl latency tests on mounted volumes..."
echo "Test parameters:"
echo "  Test size: ${TEST_SIZE_KB}KB"
echo "  Iterations: $NUM_ITERATIONS"
echo "  Offset: ${TEST_OFFSET} bytes"
echo ""

for device_info in "${DEVICES[@]}"; do
    IFS=':' read -r device name <<< "$device_info"
    
    if [ ! -b "$device" ]; then
        print_warning "Device $device not found, skipping $name volume test"
        continue
    fi
    
    print_status "Testing $name volume ($device)..."
    
    # Run the latency test
    cd /home/ubuntu/ReStore/cpp/Run_IO/cpp/Sibyl_driver/
    sudo ./test_vda_latency.sh -d "$device" -s $TEST_SIZE_KB -n $NUM_ITERATIONS -o $TEST_OFFSET
    
    echo ""
    echo "=========================================="
    echo ""
done

print_success "All volume tests completed!"
EOF

    chmod +x test_volume_latency.sh
    print_success "Volume test script created: test_volume_latency.sh"
}

# Save volume information
save_volume_info() {
    print_status "Saving volume information..."
    
    cat > volume_info.txt << EOF
# Sibyl Test Volumes Information
# Created: $(date)
# Instance: $INSTANCE_NAME ($INSTANCE_ID)

Fast Volume:
  ID: $FAST_VOLUME_ID
  Device: /dev/vdb
  Mount: /mnt/restore-fast
  Type: $VOLUME_TYPE
  IOPS: ${FAST_IOPS_READ} read, ${FAST_IOPS_WRITE} write
  Throughput: ${FAST_BYTES_READ} bytes/sec
  Concurrency: $FAST_CONCURRENCY

Middle Volume:
  ID: $MIDDLE_VOLUME_ID
  Device: /dev/vdc
  Mount: /mnt/restore-middle
  Type: $VOLUME_TYPE
  IOPS: ${MIDDLE_IOPS_READ} read, ${MIDDLE_IOPS_WRITE} write
  Throughput: ${MIDDLE_BYTES_READ} bytes/sec
  Concurrency: $MIDDLE_CONCURRENCY

Slow Volume:
  ID: $SLOW_VOLUME_ID
  Device: /dev/vdd
  Mount: /mnt/restore-slow
  Type: $VOLUME_TYPE
  IOPS: ${SLOW_IOPS_READ} read, ${SLOW_IOPS_WRITE} write
  Throughput: ${SLOW_BYTES_READ} bytes/sec
  Concurrency: $SLOW_CONCURRENCY

# To run tests:
cd /home/ubuntu/ReStore/utils/
./test_volume_latency.sh

# To cleanup volumes:
openstack volume delete $FAST_VOLUME_ID $MIDDLE_VOLUME_ID $SLOW_VOLUME_ID
EOF

    print_success "Volume information saved to volume_info.txt"
}

# Main execution
main() {
    echo "=== OpenStack Volume Creation for Sibyl Testing ==="
    echo "Date: $(date)"
    echo ""
    
    check_openstack_cli
    check_auth
    get_instance_id
    set_volume_type
    create_volumes
    wait_for_volumes
    attach_volumes
    wait_for_devices
    setup_filesystems
    create_volume_test_script
    save_volume_info
    
    print_success "All volumes created and ready for testing!"
    echo ""
    echo "Next steps:"
    echo "1. Run the test: ./test_volume_latency.sh"
    echo "2. Check results in the output"
    echo "3. Cleanup when done: see volume_info.txt"
}

# Handle script arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -s|--size)
            VOLUME_SIZE_GB="$2"
            shift 2
            ;;
        -p|--prefix)
            VOLUME_PREFIX="$2"
            shift 2
            ;;
        -i|--instance)
            INSTANCE_ID="$2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  -s, --size SIZE        Volume size in GB (default: 10)"
            echo "  -p, --prefix PREFIX    Volume name prefix (default: sibyl-test)"
            echo "  -i, --instance ID      Instance ID (auto-detected if not provided)"
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
