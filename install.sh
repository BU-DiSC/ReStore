#!/bin/bash

# ReStore Installation Script
# This script builds and installs the ReStore package

set -e  # Exit on any error

# Default installation directory
INSTALL_PREFIX="${INSTALL_PREFIX:-/usr/local}"
BUILD_DIR="${BUILD_DIR:-build}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if running as root for system-wide installation
check_permissions() {
    if [[ "$INSTALL_PREFIX" == "/usr"* ]] && [[ $EUID -ne 0 ]]; then
        print_error "System-wide installation requires root privileges."
        print_error "Please run with sudo or set INSTALL_PREFIX to a user directory."
        exit 1
    fi
}

# Check dependencies
check_dependencies() {
    print_status "Checking dependencies..."
    
    # Check for CMake
    if ! command -v cmake &> /dev/null; then
        print_error "CMake is required but not installed."
        print_error "Please install CMake: sudo apt-get install cmake"
        exit 1
    fi
    
    # Check for C++ compiler
    if ! command -v g++ &> /dev/null; then
        print_error "g++ compiler is required but not installed."
        print_error "Please install g++: sudo apt-get install g++"
        exit 1
    fi
    
    # Check for make
    if ! command -v make &> /dev/null; then
        print_error "make is required but not installed."
        print_error "Please install make: sudo apt-get install make"
        exit 1
    fi
    
    print_status "All dependencies found."
}

# Build the project
build_project() {
    print_status "Building ReStore..."
    
    # Create build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    # Configure with CMake (with default device paths)
    print_status "Configuring with CMake..."
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
        -DCMAKE_CXX_COMPILER=g++ \
        -DFAST_DEVICE_PATH="/dev/nvme0n1" \
        -DMIDDLE_DEVICE_PATH="/dev/nvme1n1" \
        -DSLOW_DEVICE_PATH="/dev/nvme2n1"
    
    # Build
    print_status "Compiling..."
    make -j$(nproc)
    
    cd ..
    print_status "Build completed successfully."
}

# Install the project
install_project() {
    print_status "Installing ReStore to $INSTALL_PREFIX..."
    
    cd "$BUILD_DIR"
    make install
    cd ..
    
    print_status "Installation completed successfully."
}

# Create symlinks for easy access
create_symlinks() {
    if [[ "$INSTALL_PREFIX" != "/usr/local" ]]; then
        print_status "Creating symlinks in /usr/local/bin for easy access..."
        
        # Create symlinks if we have permission
        if [[ -w "/usr/local/bin" ]] || sudo -n true 2>/dev/null; then
            sudo ln -sf "$INSTALL_PREFIX/bin/restore" "/usr/local/bin/restore" 2>/dev/null || true
        else
            print_warning "Could not create symlinks in /usr/local/bin"
            print_warning "You may need to add $INSTALL_PREFIX/bin to your PATH"
        fi
    fi
}

# Verify installation
verify_installation() {
    print_status "Verifying installation..."
    
    if [[ -f "$INSTALL_PREFIX/bin/restore" ]]; then
        print_status "✓ restore executable found"
    else
        print_error "✗ restore executable not found"
        return 1
    fi
    
    # Test the restore command with timeout (no args shows usage)
    print_status "Testing restore command (this may take a moment)..."
    if timeout 5 "$INSTALL_PREFIX/bin/restore" &> /dev/null; then
        print_status "✓ restore command working correctly"
    else
        print_warning "✗ restore command test failed or timed out"
        print_warning "This may be normal - you can test manually later with: restore list policies"
    fi
    
    print_status "Installation verification completed."
}

# Print usage information
print_usage_info() {
    echo
    print_status "ReStore has been successfully installed!"
    echo
    echo "Usage examples:"
    echo "  restore list policies                    # List available policies"
    echo "  restore init -workload my_workload      # Initialize configuration"
    echo "  restore init -devices /dev/sda /dev/sdb /dev/sdc  # Configure device paths"
    echo "  restore build                            # Build/rebuild executables"
    echo "  restore run -policy LFU -workload my_workload  # Run simulation"
    echo
    echo "Configuration file: $INSTALL_PREFIX/share/ReStore/restore_config.json"
    echo "Note: Provide your own workload data when running experiments"
    echo
    
    if [[ "$INSTALL_PREFIX" != "/usr/local" ]]; then
        echo "Note: Add $INSTALL_PREFIX/bin to your PATH to use 'restore' command:"
        echo "  export PATH=\"$INSTALL_PREFIX/bin:\$PATH\""
        echo
    fi
}

# Main installation process
main() {
    echo "ReStore Installation Script"
    echo "=========================="
    echo
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            --prefix)
                INSTALL_PREFIX="$2"
                shift 2
                ;;
            --build-dir)
                BUILD_DIR="$2"
                shift 2
                ;;
            --help)
                echo "Usage: $0 [options]"
                echo "Options:"
                echo "  --prefix DIR      Installation prefix (default: /usr/local)"
                echo "  --build-dir DIR   Build directory (default: build)"
                echo "  --help           Show this help message"
                exit 0
                ;;
            *)
                print_error "Unknown option: $1"
                exit 1
                ;;
        esac
    done
    
    print_status "Installation prefix: $INSTALL_PREFIX"
    print_status "Build directory: $BUILD_DIR"
    echo
    
    check_permissions
    check_dependencies
    build_project
    install_project
    create_symlinks
    verify_installation
    print_usage_info
}

# Run main function
main "$@"

