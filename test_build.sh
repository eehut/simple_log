#!/bin/bash

# Test build script for different CMake configurations
# Tests all combinations of SLOG_SINK_SPDLOG and SLOG_EXTERNAL_LIBFMT options

set -e  # Exit immediately if a command exits with a non-zero status

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Get the script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Clean up function
cleanup() {
    if [ -d "build_test" ]; then
        echo -e "${YELLOW}Cleaning up build_test directory...${NC}"
        rm -rf build_test
    fi
}

# Trap to cleanup on exit
trap cleanup EXIT

# Test configurations @0 SPDLOG @1 LIBFMT
declare -a configs=(
    "ON ON"
    "OFF ON"
    "OFF OFF"
)

echo -e "${GREEN}Starting build tests for all configuration combinations...${NC}"
echo ""

# Test each configuration
for config in "${configs[@]}"; do
    read -r spdlog_flag fmt_flag <<< "$config"
    
    build_dir="build_test/spdlog_${spdlog_flag}_fmt_${fmt_flag}"
    
    echo -e "${YELLOW}========================================${NC}"
    echo -e "${YELLOW}Testing configuration:${NC}"
    echo -e "  SLOG_SINK_SPDLOG=${spdlog_flag}"
    echo -e "  SLOG_EXTERNAL_LIBFMT=${fmt_flag}"
    echo -e "${YELLOW}========================================${NC}"
    
    # Create build directory
    mkdir -p "$build_dir"
    cd "$build_dir"
    
    # Configure with cmake
    echo "Running cmake configuration..."
    cmake "$SCRIPT_DIR" \
        -DSLOG_SINK_SPDLOG="${spdlog_flag}" \
        -DSLOG_EXTERNAL_LIBFMT="${fmt_flag}" \
        -DCMAKE_BUILD_TYPE=Release \
        -DSLOG_BUILD_TEST=ON \
        -DSLOG_BUILD_EXAMPLES=ON
        
    if [ $? -ne 0 ]; then
        echo -e "${RED}ERROR: cmake configuration failed for SLOG_SINK_SPDLOG=${spdlog_flag}, SLOG_EXTERNAL_LIBFMT=${fmt_flag}${NC}"
        exit 1
    fi
    
    # Build
    echo "Running build..."
    cmake --build . -j$(nproc)
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}ERROR: build failed for SLOG_SINK_SPDLOG=${spdlog_flag}, SLOG_EXTERNAL_LIBFMT=${fmt_flag}${NC}"
        exit 1
    fi
    
    echo -e "${GREEN}✓ Configuration test passed${NC}"
    echo ""
    
    # Return to script directory
    cd "$SCRIPT_DIR"
done

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}All configuration tests passed!${NC}"
echo -e "${GREEN}========================================${NC}"

