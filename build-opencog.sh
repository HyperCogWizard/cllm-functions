#!/bin/bash
# Build script for CAIChat OpenCog module

set -e

echo "Building CAIChat OpenCog Module..."

# Check dependencies
check_dependencies() {
    echo "Checking dependencies..."
    
    local missing_deps=()
    
    if ! command -v cmake &> /dev/null; then
        missing_deps+=("cmake")
    fi
    
    if ! command -v pkg-config &> /dev/null; then
        missing_deps+=("pkg-config")
    fi
    
    if ! pkg-config --exists guile-3.0 && ! pkg-config --exists guile-2.2; then
        missing_deps+=("guile-dev")
    fi
    
    if ! pkg-config --exists libcurl; then
        missing_deps+=("libcurl-dev")
    fi
    
    if ! pkg-config --exists jsoncpp; then
        missing_deps+=("libjsoncpp-dev")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        echo "Missing dependencies: ${missing_deps[*]}"
        echo "Please install them first:"
        echo "  Ubuntu/Debian: sudo apt-get install cmake build-essential pkg-config guile-3.0-dev libcurl4-openssl-dev libjsoncpp-dev"
        echo "  Fedora: sudo dnf install cmake gcc-c++ pkg-config guile-devel libcurl-devel jsoncpp-devel"
        echo "  macOS: brew install cmake pkg-config guile curl jsoncpp"
        exit 1
    fi
    
    echo "All dependencies found!"
}

# Build function
build_project() {
    echo "Creating build directory..."
    mkdir -p build
    cd build
    
    echo "Running CMake..."
    cmake ..
    
    echo "Building..."
    make -j$(nproc 2>/dev/null || echo 4)
    
    echo "Build completed successfully!"
    echo "To install: sudo make install"
}

# Test function (if Guile is available)
test_modules() {
    if command -v guile &> /dev/null; then
        echo "Testing Scheme modules..."
        cd ..
        guile test-caichat.scm
    else
        echo "Guile not found, skipping tests"
    fi
}

# Main execution
main() {
    check_dependencies
    build_project
    test_modules
}

# Run only if executed directly
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi