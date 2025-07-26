# GGML Integration Technical Specification

This document provides detailed technical specifications for integrating real GGML model loading and inference into CAIChat.

## Overview

Replace the current placeholder GGML implementation with full model loading, inference, and management capabilities using the llama.cpp library.

## Architecture

### Current State
```cpp
// Placeholder implementation in LLMClient.cc
class GGMLClient : public LLMClient {
    std::string sendMessage(const std::string& message) override {
        return "This is a placeholder response from GGML model...";
    }
};
```

### Target Architecture
```cpp
// Real GGML implementation
class GGMLClient : public LLMClient {
private:
    llama_model* model = nullptr;
    llama_context* context = nullptr;
    llama_sampling_context* sampling_ctx = nullptr;
    
    std::string model_path;
    ModelParams model_params;
    GenerationParams generation_params;
    bool model_loaded = false;
    
    // Thread safety
    std::mutex model_mutex;
    
public:
    GGMLClient();
    ~GGMLClient();
    
    // Model management
    bool loadModel(const std::string& path, const ModelParams& params = {});
    void unloadModel();
    bool isModelLoaded() const { return model_loaded; }
    std::string getModelPath() const { return model_path; }
    
    // Text generation
    std::string sendMessage(const std::string& message) override;
    std::string generateText(const std::string& prompt, 
                           const GenerationParams& params = {});
    
    // Streaming generation
    void streamGeneration(const std::string& prompt,
                         StreamCallback callback,
                         const GenerationParams& params = {});
    
    // Parameter management
    void setDefaultGenerationParams(const GenerationParams& params);
    GenerationParams getDefaultGenerationParams() const;
    
    // Model information
    ModelInfo getModelInfo() const;
    size_t getModelMemoryUsage() const;
};
```

## Data Structures

### Model Parameters
```cpp
struct ModelParams {
    int32_t n_ctx = 2048;           // Context size
    int32_t n_batch = 512;          // Batch size for prompt processing
    int32_t n_gpu_layers = -1;      // Number of layers to store in VRAM
    bool use_mmap = true;           // Use memory mapping
    bool use_mlock = false;         // Lock memory
    bool embedding = false;         // Enable embedding mode
    float rope_freq_base = 10000.0f; // RoPE base frequency
    float rope_freq_scale = 1.0f;   // RoPE frequency scaling
    
    // Validation
    bool isValid() const;
    std::string toString() const;
};
```

### Generation Parameters  
```cpp
struct GenerationParams {
    int32_t n_predict = 128;        // Number of tokens to predict
    int32_t top_k = 40;             // Top-k sampling
    float top_p = 0.95f;            // Top-p sampling
    float temp = 0.7f;              // Temperature
    float repeat_penalty = 1.1f;    // Repetition penalty
    int32_t repeat_last_n = 64;     // Tokens to consider for repetition
    
    std::vector<std::string> stop_words; // Stop sequences
    bool echo = false;              // Echo the prompt
    
    // Validation
    bool isValid() const;
    std::string toString() const;
};
```

### Model Information
```cpp
struct ModelInfo {
    std::string path;
    std::string architecture;
    int64_t parameter_count;
    int32_t vocab_size;
    int32_t context_length;
    size_t memory_usage_bytes;
    bool is_quantized;
    std::string quantization_type;
    
    std::string toString() const;
};
```

## Implementation Details

### Model Loading Process
```cpp
bool GGMLClient::loadModel(const std::string& path, const ModelParams& params) {
    std::lock_guard<std::mutex> lock(model_mutex);
    
    // Validate parameters
    if (!params.isValid()) {
        return false;
    }
    
    // Unload existing model
    if (model_loaded) {
        unloadModel();
    }
    
    // Initialize llama backend
    llama_backend_init();
    
    // Set up model parameters
    llama_model_params model_params = llama_model_default_params();
    model_params.n_gpu_layers = params.n_gpu_layers;
    model_params.use_mmap = params.use_mmap;
    model_params.use_mlock = params.use_mlock;
    
    // Load model
    model = llama_load_model_from_file(path.c_str(), model_params);
    if (!model) {
        llama_backend_free();
        return false;
    }
    
    // Create context
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = params.n_ctx;
    ctx_params.n_batch = params.n_batch;
    ctx_params.embedding = params.embedding;
    ctx_params.rope_freq_base = params.rope_freq_base;
    ctx_params.rope_freq_scale = params.rope_freq_scale;
    
    context = llama_new_context_with_model(model, ctx_params);
    if (!context) {
        llama_free_model(model);
        model = nullptr;
        llama_backend_free();
        return false;
    }
    
    // Initialize sampling context
    sampling_ctx = llama_sampling_init({});
    
    model_path = path;
    model_params = params;
    model_loaded = true;
    
    return true;
}
```

### Text Generation
```cpp
std::string GGMLClient::generateText(const std::string& prompt, 
                                   const GenerationParams& params) {
    std::lock_guard<std::mutex> lock(model_mutex);
    
    if (!model_loaded) {
        throw std::runtime_error("Model not loaded");
    }
    
    // Tokenize prompt
    std::vector<llama_token> tokens = llama_tokenize(
        context, prompt.c_str(), true, true);
    
    if (tokens.empty()) {
        throw std::runtime_error("Failed to tokenize prompt");
    }
    
    // Check context size
    if (tokens.size() >= llama_n_ctx(context)) {
        throw std::runtime_error("Prompt too long for context");
    }
    
    // Evaluate prompt
    if (llama_decode(context, llama_batch_get_one(tokens.data(), 
                    tokens.size(), 0, 0)) != 0) {
        throw std::runtime_error("Failed to evaluate prompt");
    }
    
    // Generate tokens
    std::string result;
    for (int i = 0; i < params.n_predict; ++i) {
        // Sample next token
        llama_token new_token = llama_sampling_sample(sampling_ctx, context, nullptr);
        
        // Check for stop conditions
        if (new_token == llama_token_eos(model)) {
            break;
        }
        
        // Convert to text
        char buf[256];
        int len = llama_token_to_piece(model, new_token, buf, sizeof(buf), false);
        if (len > 0) {
            result.append(buf, len);
        }
        
        // Check for stop words
        bool should_stop = false;
        for (const auto& stop_word : params.stop_words) {
            if (result.find(stop_word) != std::string::npos) {
                should_stop = true;
                break;
            }
        }
        if (should_stop) break;
        
        // Add token to context
        if (llama_decode(context, llama_batch_get_one(&new_token, 1, 
                        tokens.size() + i, 0)) != 0) {
            break;
        }
    }
    
    return result;
}
```

### Streaming Generation
```cpp
void GGMLClient::streamGeneration(const std::string& prompt,
                                 StreamCallback callback,
                                 const GenerationParams& params) {
    std::lock_guard<std::mutex> lock(model_mutex);
    
    if (!model_loaded) {
        callback("", "Model not loaded", true);
        return;
    }
    
    try {
        // Tokenize and setup (similar to generateText)
        std::vector<llama_token> tokens = llama_tokenize(
            context, prompt.c_str(), true, true);
        
        // Evaluate prompt
        llama_decode(context, llama_batch_get_one(tokens.data(), 
                    tokens.size(), 0, 0));
        
        // Stream tokens
        for (int i = 0; i < params.n_predict; ++i) {
            llama_token new_token = llama_sampling_sample(sampling_ctx, context, nullptr);
            
            if (new_token == llama_token_eos(model)) {
                callback("", "", true); // End of sequence
                break;
            }
            
            // Convert token to text
            char buf[256];
            int len = llama_token_to_piece(model, new_token, buf, sizeof(buf), false);
            
            if (len > 0) {
                std::string token_text(buf, len);
                callback(token_text, "", false); // Stream token
            }
            
            // Continue generation
            llama_decode(context, llama_batch_get_one(&new_token, 1, 
                        tokens.size() + i, 0));
        }
    } catch (const std::exception& e) {
        callback("", e.what(), true);
    }
}
```

## Scheme Integration

### Updated Scheme Bindings
```cpp
// SchemeBindings.cc additions
extern "C" {
    
SCM caichat_ggml_load_model(SCM path_scm, SCM params_scm) {
    if (!scm_is_string(path_scm)) {
        return SCM_BOOL_F;
    }
    
    std::string path = scm_to_locale_string(path_scm);
    
    // Parse parameters if provided
    ModelParams params;
    if (!scm_is_null(params_scm)) {
        // Extract parameters from Scheme association list
        // params.n_ctx = extract_int_param(params_scm, "n_ctx", 2048);
        // ... other parameters
    }
    
    // Get or create GGML client
    auto client = get_ggml_client();
    bool success = client->loadModel(path, params);
    
    return success ? SCM_BOOL_T : SCM_BOOL_F;
}

SCM caichat_ggml_generate(SCM prompt_scm, SCM params_scm) {
    if (!scm_is_string(prompt_scm)) {
        return scm_from_locale_string("");
    }
    
    std::string prompt = scm_to_locale_string(prompt_scm);
    
    // Parse generation parameters
    GenerationParams params;
    if (!scm_is_null(params_scm)) {
        // Extract parameters
    }
    
    try {
        auto client = get_ggml_client();
        std::string result = client->generateText(prompt, params);
        return scm_from_locale_string(result.c_str());
    } catch (const std::exception& e) {
        return scm_from_locale_string("");
    }
}

SCM caichat_ggml_model_info(void) {
    try {
        auto client = get_ggml_client();
        if (!client->isModelLoaded()) {
            return SCM_BOOL_F;
        }
        
        ModelInfo info = client->getModelInfo();
        
        // Create Scheme association list
        SCM result = SCM_EOL;
        result = scm_acons(scm_from_locale_symbol("path"),
                          scm_from_locale_string(info.path.c_str()), result);
        result = scm_acons(scm_from_locale_symbol("parameters"),
                          scm_from_int64(info.parameter_count), result);
        result = scm_acons(scm_from_locale_symbol("memory-usage"),
                          scm_from_size_t(info.memory_usage_bytes), result);
        // ... other fields
        
        return result;
    } catch (const std::exception& e) {
        return SCM_BOOL_F;
    }
}

} // extern "C"
```

### Enhanced Scheme API
```scheme
;; scm/opencog/caichat/ggml.scm
(define-module (opencog caichat ggml)
  #:use-module (system foreign)
  #:export (caichat-ggml-load-model
           caichat-ggml-generate
           caichat-ggml-stream
           caichat-ggml-model-info
           caichat-ggml-unload-model))

(define (caichat-ggml-load-model path . params)
  "Load a GGML model from the specified path with optional parameters"
  (let ((param-alist (if (null? params) '() (car params))))
    (caichat-ggml-load-model-c path param-alist)))

(define (caichat-ggml-generate prompt . params)
  "Generate text using the loaded GGML model"
  (let ((param-alist (if (null? params) '() (car params))))
    (caichat-ggml-generate-c prompt param-alist)))

(define (caichat-ggml-setup-default-model path)
  "Set up GGML as the default provider with the specified model"
  (caichat-ggml-load-model path)
  (caichat-config-set! 'default-provider 'ggml)
  (caichat-config-set! 'ggml-model-path path))

(define (caichat-ggml-model-info)
  "Get information about the currently loaded model"
  (caichat-ggml-model-info-c))
```

## Testing Strategy

### Unit Tests
```cpp
// tests/test_ggml_client.cpp
#include <gtest/gtest.h>
#include "opencog/caichat/LLMClient.h"

class GGMLClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        client = std::make_unique<GGMLClient>();
    }
    
    std::unique_ptr<GGMLClient> client;
    const std::string test_model_path = "test-data/tiny-model.gguf";
};

TEST_F(GGMLClientTest, LoadValidModel) {
    EXPECT_TRUE(client->loadModel(test_model_path));
    EXPECT_TRUE(client->isModelLoaded());
    EXPECT_EQ(client->getModelPath(), test_model_path);
}

TEST_F(GGMLClientTest, LoadInvalidModel) {
    EXPECT_FALSE(client->loadModel("nonexistent.gguf"));
    EXPECT_FALSE(client->isModelLoaded());
}

TEST_F(GGMLClientTest, GenerateText) {
    ASSERT_TRUE(client->loadModel(test_model_path));
    
    std::string result = client->generateText("Hello", {.n_predict = 10});
    EXPECT_FALSE(result.empty());
    EXPECT_GT(result.length(), 0);
}

TEST_F(GGMLClientTest, StreamGeneration) {
    ASSERT_TRUE(client->loadModel(test_model_path));
    
    std::string accumulated;
    bool completed = false;
    
    client->streamGeneration("Hello", 
        [&](const std::string& token, const std::string& error, bool done) {
            if (!error.empty()) {
                FAIL() << "Error in streaming: " << error;
            }
            accumulated += token;
            completed = done;
        }, {.n_predict = 10});
    
    EXPECT_TRUE(completed);
    EXPECT_FALSE(accumulated.empty());
}
```

### Integration Tests
```scheme
;; tests/test-ggml-integration.scm
(use-modules (opencog caichat ggml)
             (opencog caichat config)
             (srfi srfi-64))

(test-begin "ggml-integration")

(test-assert "load-model"
  (caichat-ggml-load-model "test-data/tiny-model.gguf"))

(test-assert "generate-text"
  (let ((result (caichat-ggml-generate "Hello world")))
    (and (string? result) (> (string-length result) 0))))

(test-assert "model-info"
  (let ((info (caichat-ggml-model-info)))
    (and (list? info) (assoc 'path info))))

(test-end "ggml-integration")
```

## Performance Requirements

### Benchmarks
- **Model Loading**: <30 seconds for 7B parameter models
- **First Token**: <500ms latency
- **Token Generation**: >20 tokens/second
- **Memory Usage**: <2GB additional RAM for 7B models
- **Concurrent Requests**: Support 4 simultaneous generations

### Optimization Strategies
1. **Memory Management**: Model quantization and context reuse
2. **GPU Acceleration**: CUDA/Metal support where available
3. **Caching**: Token and embedding caching
4. **Batching**: Process multiple requests together

## Error Handling

### Error Categories
1. **Model Loading Errors**: File not found, corruption, insufficient memory
2. **Generation Errors**: Context overflow, tokenization failure
3. **Parameter Errors**: Invalid ranges, incompatible settings
4. **System Errors**: Out of memory, hardware failures

### Error Recovery
- Graceful degradation to placeholder responses
- Automatic model reloading on corruption
- Memory cleanup on failures
- Detailed logging for debugging

## Security Considerations

### Input Validation
- Path traversal prevention for model loading
- Prompt injection mitigation
- Resource usage limits
- Memory bounds checking

### Resource Limits
- Maximum model size limits
- Context length restrictions
- Generation timeout controls
- Memory usage monitoring

This specification provides the foundation for implementing a robust, production-ready GGML integration that maintains compatibility with the existing OpenCog architecture while adding powerful local model capabilities.