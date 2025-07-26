# GGML Integration Guide

This document provides comprehensive guidance for using the GGML local model integration in CAIChat-OpenCog.

## Overview

The GGML integration allows CAIChat to run language models locally using the llama.cpp library, providing:

- Local model inference without API dependencies
- Full control over model parameters and generation settings
- Integration with OpenCog's cognitive architecture
- Support for quantized models and GPU acceleration

## Installation

### Prerequisites

1. **llama.cpp library** (for real model loading)
   ```bash
   git clone https://github.com/ggerganov/llama.cpp
   cd llama.cpp
   make
   sudo make install
   ```

2. **GGUF format models** (download from Hugging Face or convert existing models)

### Building with GGML Support

```bash
# Set environment variables (if llama.cpp is installed in non-standard location)
export GGML_DIR=/usr/local
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# Build the project
mkdir build && cd build
cmake ..
make

# Run tests
./tests/test_ggml_integration
```

## C++ API

### Basic Usage

```cpp
#include "opencog/caichat/LLMClient.h"
using namespace opencog::caichat;

// Create GGML client
GGMLClient client;

// Configure model parameters
ModelParams params;
params.n_ctx = 4096;          // Context size
params.n_batch = 512;         // Batch size
params.n_gpu_layers = 32;     // GPU layers (-1 for auto)
params.use_mmap = true;       // Memory mapping
params.use_mlock = false;     // Lock memory

// Load model
if (client.loadModel("/path/to/model.gguf", params)) {
    std::cout << "Model loaded successfully!" << std::endl;
    
    // Set generation parameters
    GenerationParams genParams;
    genParams.n_predict = 128;      // Max tokens to generate
    genParams.temp = 0.7f;          // Temperature
    genParams.top_p = 0.95f;        // Top-p sampling
    genParams.top_k = 40;           // Top-k sampling
    genParams.repeat_penalty = 1.1f; // Repetition penalty
    
    client.setDefaultGenerationParams(genParams);
    
    // Generate text
    std::string response = client.generateText("What is artificial intelligence?");
    std::cout << "Response: " << response << std::endl;
    
    // Get model information
    ModelInfo info = client.getModelInfo();
    std::cout << "Model: " << info.path << std::endl;
    std::cout << "Parameters: " << info.parameter_count << std::endl;
    std::cout << "Memory usage: " << info.memory_usage_bytes << " bytes" << std::endl;
}
```

### Streaming Generation

```cpp
// Stream tokens as they are generated
client.streamGeneration("Tell me a story",
    [](const std::string& token, const std::string& error, bool done) {
        if (!error.empty()) {
            std::cerr << "Error: " << error << std::endl;
            return;
        }
        
        if (!token.empty()) {
            std::cout << token << std::flush;
        }
        
        if (done) {
            std::cout << std::endl << "[Generation complete]" << std::endl;
        }
    },
    genParams);
```

### Chat Interface

```cpp
// Use with conversation management
ChatCompletion chat(std::make_unique<GGMLClient>());

// Set system message
chat.setSystemMessage("You are a helpful AI assistant.");

// Send messages
std::string response1 = chat.sendMessage("Hello, how are you?");
std::string response2 = chat.sendMessage("What's the weather like?");

// Get conversation history
const auto& history = chat.getHistory();
for (const auto& msg : history) {
    std::cout << msg.role << ": " << msg.content << std::endl;
}
```

## Scheme API

### Loading the Module

```scheme
;; Load GGML-specific functions
(use-modules (opencog caichat ggml))

;; Or load all CAIChat modules
(use-modules (opencog caichat init)
             (opencog caichat rag)
             (opencog caichat atomspace))
```

### Basic Operations

```scheme
;; Load a model with custom parameters
(define session (caichat-ggml-load-model "/path/to/model.gguf"
                                        '((n_ctx . 4096)
                                          (n_batch . 512)
                                          (n_gpu_layers . 32)
                                          (use_mmap . #t))))

;; Generate text
(define response (caichat-ggml-generate session 
                                       "Explain quantum computing"
                                       '((n_predict . 200)
                                         (temp . 0.8)
                                         (top_p . 0.95))))

(display response)
```

### Model Information

```scheme
;; Get detailed model information
(define info (caichat-ggml-model-info session))
(display "Model path: ") (display (assoc-ref info 'path)) (newline)
(display "Parameters: ") (display (assoc-ref info 'parameters)) (newline)
(display "Context length: ") (display (assoc-ref info 'context-length)) (newline)
```

### Integration with Other Modules

```scheme
;; Set GGML as default provider
(caichat-ggml-setup-default "/path/to/model.gguf")

;; Now all caichat-ask calls use the local model
(caichat-ask "What is the meaning of life?")

;; Use with RAG system
(use-modules (opencog caichat rag))
(caichat-rag-create-kb "knowledge" "AI Knowledge Base")
(caichat-rag-add-doc "knowledge" "doc1" "GGML enables local AI inference..." '())
(caichat-rag-query "knowledge" "How does GGML work?")

;; Use with AtomSpace
(use-modules (opencog caichat atomspace))
(caichat-atomspace-add-concept "LocalAI" "AI running on local hardware")
```

## Configuration

### Environment Variables

```bash
# Model path (used as default)
export GGML_MODEL_PATH="/path/to/default/model.gguf"

# GGML installation directory (for cmake)
export GGML_DIR="/usr/local"

# Library path (if needed)
export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"
```

### Model Parameters Reference

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| n_ctx | int32 | 2048 | Context window size |
| n_batch | int32 | 512 | Batch size for processing |
| n_gpu_layers | int32 | -1 | GPU layers (-1 = auto) |
| use_mmap | bool | true | Use memory mapping |
| use_mlock | bool | false | Lock memory pages |
| embedding | bool | false | Enable embedding mode |
| rope_freq_base | float | 10000.0 | RoPE frequency base |
| rope_freq_scale | float | 1.0 | RoPE frequency scaling |

### Generation Parameters Reference

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| n_predict | int32 | 128 | Maximum tokens to generate |
| temp | float | 0.7 | Temperature (0.0-2.0) |
| top_p | float | 0.95 | Top-p/nucleus sampling |
| top_k | int32 | 40 | Top-k sampling |
| repeat_penalty | float | 1.1 | Repetition penalty |
| repeat_last_n | int32 | 64 | Tokens for repetition check |
| stop_words | vector | [] | Stop sequences |
| echo | bool | false | Echo the prompt |

## Performance Optimization

### GPU Acceleration

```cpp
// Enable GPU acceleration
ModelParams params;
params.n_gpu_layers = -1;  // Use all available GPU layers
// or
params.n_gpu_layers = 32;  // Use specific number of layers
```

### Memory Management

```cpp
// Optimize for memory usage
ModelParams params;
params.use_mmap = true;     // Memory mapping for large models
params.use_mlock = false;   // Don't lock memory unless needed
params.n_ctx = 2048;        // Smaller context if memory is limited
```

### Batch Processing

```cpp
// Optimize batch size for throughput
ModelParams params;
params.n_batch = 512;       // Larger batch for better GPU utilization
// or
params.n_batch = 128;       // Smaller batch for lower latency
```

## Error Handling

### Common Issues

1. **Model Loading Fails**
   ```cpp
   try {
       client.loadModel("/path/to/model.gguf");
   } catch (const std::exception& e) {
       std::cerr << "Model loading failed: " << e.what() << std::endl;
       // Check file path, permissions, model format
   }
   ```

2. **Out of Memory**
   ```cpp
   // Use smaller context or quantized models
   ModelParams params;
   params.n_ctx = 1024;        // Reduce context size
   params.n_gpu_layers = 0;    // Use CPU only
   ```

3. **Generation Timeout**
   ```cpp
   GenerationParams params;
   params.n_predict = 50;      // Reduce max tokens
   ```

### Graceful Degradation

The GGML integration is designed to work gracefully even when dependencies are missing:

- Without llama.cpp: Provides interface but throws informative errors
- Without GPU: Falls back to CPU inference
- Without sufficient memory: Suggests parameter adjustments

## Integration with OpenCog

### AtomSpace Integration

```scheme
;; Store model information in AtomSpace
(use-modules (opencog caichat atomspace))

(define model-info (caichat-ggml-model-info session))
(caichat-atomspace-add-concept "LocalModel" 
                              (format #f "GGML model with ~a parameters" 
                                      (assoc-ref model-info 'parameters)))

;; Store conversation in AtomSpace for learning
(caichat-atomspace-store-conversation "ggml-session" conversation-history)
```

### PLN Integration (Future)

```scheme
;; Use PLN reasoning with local models
(caichat-pln-analyze-conversation session-id)
(caichat-pln-learn-from-conversation session-id)
```

## Examples

See `tests/test_ggml_example.sh` for complete examples and current build status.

## Troubleshooting

### Build Issues

1. **GGML not found**
   - Install llama.cpp as described above
   - Set GGML_DIR environment variable
   - Check cmake output for detection results

2. **Linking errors**
   - Ensure llama.cpp is properly installed
   - Check LD_LIBRARY_PATH includes llama.cpp lib directory
   - Verify library compatibility (same compiler/flags)

3. **Runtime errors**
   - Check model file format (should be GGUF)
   - Verify sufficient memory/disk space
   - Check file permissions

### Model Issues

1. **Model won't load**
   - Verify GGUF format (not older GGML format)
   - Check file size and integrity
   - Try smaller model or adjust parameters

2. **Poor performance**
   - Enable GPU layers if available
   - Adjust batch size and context length
   - Consider quantized models

3. **Out of memory**
   - Use quantized models (Q4, Q5, Q8)
   - Reduce context size (n_ctx)
   - Disable GPU layers if GPU memory limited

This integration provides a solid foundation for local AI model inference within the OpenCog cognitive architecture, supporting the goals of iteration 3 while maintaining compatibility and extensibility.