#!/bin/bash

# GGML Integration Example Script
# ===============================
# This script demonstrates how to use the GGML integration features
# once the llama.cpp library is properly installed.

echo "CAIChat GGML Integration Example"
echo "================================="
echo

# Check if the library was built with GGML support
if ./build/tests/test_ggml_integration | grep -q "GGML support not compiled"; then
    echo "ℹ️  GGML support is currently not compiled in."
    echo "   To enable real GGML model loading:"
    echo
    echo "1. Install llama.cpp:"
    echo "   git clone https://github.com/ggerganov/llama.cpp"
    echo "   cd llama.cpp"
    echo "   make"
    echo "   sudo make install"
    echo
    echo "2. Set environment variables:"
    echo "   export GGML_DIR=/usr/local"
    echo "   export LD_LIBRARY_PATH=/usr/local/lib:\$LD_LIBRARY_PATH"
    echo
    echo "3. Rebuild the project:"
    echo "   mkdir build && cd build"
    echo "   cmake .."
    echo "   make"
    echo
    echo "📝 Example usage with real GGML model:"
    echo
    echo "C++ Example:"
    echo "----------"
    cat << 'EOF'
#include "opencog/caichat/LLMClient.h"
using namespace opencog::caichat;

int main() {
    GGMLClient client;
    
    // Configure model parameters
    ModelParams params;
    params.n_ctx = 4096;
    params.n_gpu_layers = 32;  // Use GPU if available
    
    // Load a model
    if (client.loadModel("/path/to/model.gguf", params)) {
        std::cout << "Model loaded successfully!" << std::endl;
        
        // Generate text
        std::string response = client.generateText("Hello, world!");
        std::cout << "Response: " << response << std::endl;
        
        // Get model info
        ModelInfo info = client.getModelInfo();
        std::cout << "Model has " << info.parameter_count << " parameters" << std::endl;
    }
    
    return 0;
}
EOF
    echo
    echo "Scheme Example:"
    echo "-------------"
    cat << 'EOF'
;; Load the GGML module
(use-modules (opencog caichat ggml))

;; Load a model
(define session (caichat-ggml-load-model "/path/to/model.gguf" 
                                       '((n_ctx . 4096) 
                                         (n_gpu_layers . 32))))

;; Generate text
(caichat-ggml-generate session "What is artificial intelligence?"
                      '((n_predict . 100) 
                        (temp . 0.7)))

;; Get model information  
(caichat-ggml-model-info session)

;; Set as default provider
(caichat-ggml-setup-default "/path/to/model.gguf")
(caichat-ask "Tell me about quantum computing")
EOF
    echo
else
    echo "✅ GGML support is compiled in and ready to use!"
    echo
    echo "📝 To use with a real model, you would:"
    echo "1. Download a GGUF model (e.g., from Hugging Face)"
    echo "2. Use the C++ or Scheme APIs as shown above"
    echo
fi

echo "🧪 Current Test Results:"
echo "======================="
./build/tests/test_ggml_integration

echo
echo "📖 Available Scheme Modules:"
echo "==========================="
echo "- (opencog caichat init)      - Main interface"
echo "- (opencog caichat ggml)      - GGML-specific functions"
echo "- (opencog caichat config)    - Configuration management"
echo "- (opencog caichat rag)       - Retrieval Augmented Generation"
echo "- (opencog caichat atomspace) - OpenCog AtomSpace integration"
echo "- (opencog caichat repl)      - Interactive chat interface"
echo

echo "🔧 Build Configuration:"
echo "====================="
echo "GGML Support:    $(if [ -f build/CMakeCache.txt ] && grep -q "GGML_FOUND:BOOL=TRUE" build/CMakeCache.txt; then echo "✅ Enabled"; else echo "❌ Disabled (llama.cpp not found)"; fi)"
echo "CURL Support:    $(if [ -f build/CMakeCache.txt ] && grep -q "CURL_AVAILABLE:INTERNAL=TRUE" build/CMakeCache.txt; then echo "✅ Enabled"; else echo "❌ Disabled (headers not found)"; fi)"
echo "JsonCpp Support: $(if [ -f build/CMakeCache.txt ] && grep -q "JSONCPP_AVAILABLE:INTERNAL=TRUE" build/CMakeCache.txt; then echo "✅ Enabled"; else echo "❌ Disabled (library not found)"; fi)"
echo "Guile Support:   $(if [ -f build/CMakeCache.txt ] && grep -q "GUILE_AVAILABLE:INTERNAL=TRUE" build/CMakeCache.txt; then echo "✅ Enabled"; else echo "❌ Disabled (guile not found)"; fi)"

echo
echo "This demonstrates iteration 3 part 2 progress:"
echo "✅ Real GGML implementation with llama.cpp integration"
echo "✅ Graceful degradation when dependencies are missing"
echo "✅ Comprehensive C++ API with parameter management"
echo "✅ Scheme bindings ready for OpenCog integration"
echo "✅ Thread-safe model operations and resource management"