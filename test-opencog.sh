#!/bin/bash

# Test script for CAIChat OpenCog module

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
SCM_DIR="$SCRIPT_DIR/scm"

echo "CAIChat OpenCog Test Script"
echo "=========================="

# Check if build exists
if [ ! -f "$BUILD_DIR/opencog/libcaichat.so" ]; then
    echo "Library not found. Building..."
    ./build-opencog.sh
fi

echo "Running tests..."

# Set environment variables
export GUILE_LOAD_PATH="$SCM_DIR"
export CAICHAT_LIB_PATH="$BUILD_DIR/opencog/libcaichat.so"

# Run the test
echo "Running basic functionality tests..."
guile test-caichat.scm

echo "Running example script..."
guile example.scm

echo "Testing individual modules..."

echo "  - Testing config module..."
guile -c "(use-modules (opencog caichat config)) (display \"Config module loaded successfully\\n\")"

echo "  - Testing RAG module..."
guile -c "(use-modules (opencog caichat rag)) (display \"RAG module loaded successfully\\n\")"

echo "  - Testing AtomSpace module..."
guile -c "(use-modules (opencog caichat atomspace)) (display \"AtomSpace module loaded successfully\\n\")"

echo "  - Testing REPL module..."
guile -c "(use-modules (opencog caichat repl)) (display \"REPL module loaded successfully\\n\")"

echo "  - Testing init module..."
guile -c "(use-modules (opencog caichat init)) (display \"Init module loaded successfully\\n\")"

echo ""
echo "All tests completed successfully!"
echo ""
echo "To use interactively:"
echo "  export GUILE_LOAD_PATH=\"$SCM_DIR\""
echo "  export CAICHAT_LIB_PATH=\"$BUILD_DIR/opencog/libcaichat.so\""
echo "  guile"
echo "  (use-modules (opencog caichat init))"
echo "  (caichat-setup-ggml \"/path/to/model.gguf\")"
echo "  (caichat-ask \"Hello!\")"