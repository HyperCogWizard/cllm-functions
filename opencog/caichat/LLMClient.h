#ifndef LLMCLIENT_H
#define LLMCLIENT_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <mutex>

#ifdef HAVE_GGML
#include <ggml.h>
#include <llama.h>
#endif

namespace opencog {
namespace caichat {

/**
 * Message structure for chat completions
 */
struct Message {
    std::string role;    // "user", "assistant", "system"
    std::string content;
    
    Message(const std::string& r, const std::string& c) : role(r), content(c) {}
};

/**
 * Generation parameters for text generation
 */
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
    bool isValid() const {
        return n_predict > 0 && temp >= 0.0f && top_p >= 0.0f && top_p <= 1.0f;
    }
    
    std::string toString() const {
        return "GenerationParams{n_predict=" + std::to_string(n_predict) + 
               ", temp=" + std::to_string(temp) + ", top_p=" + std::to_string(top_p) + "}";
    }
};

/**
 * Model parameters for GGML models
 */
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
    bool isValid() const {
        return n_ctx > 0 && n_batch > 0 && rope_freq_base > 0.0f && rope_freq_scale > 0.0f;
    }
    
    std::string toString() const {
        return "ModelParams{n_ctx=" + std::to_string(n_ctx) + 
               ", n_batch=" + std::to_string(n_batch) + "}";
    }
};

/**
 * Model information
 */
struct ModelInfo {
    std::string path;
    std::string architecture;
    int64_t parameter_count = 0;
    int32_t vocab_size = 0;
    int32_t context_length = 0;
    size_t memory_usage_bytes = 0;
    bool is_quantized = false;
    std::string quantization_type;
    
    std::string toString() const {
        return "ModelInfo{path=" + path + ", params=" + std::to_string(parameter_count) + 
               ", ctx_len=" + std::to_string(context_length) + "}";
    }
};

/**
 * Callback for streaming generation
 */
using StreamCallback = std::function<void(const std::string& token, const std::string& error, bool done)>;

/**
 * Abstract base class for LLM providers
 */
class LLMClient {
public:
    virtual ~LLMClient() = default;
    
    /**
     * Send a chat completion request
     * @param messages Vector of conversation messages
     * @param model Model name to use
     * @return Response content from the LLM
     */
    virtual std::string chatCompletion(const std::vector<Message>& messages, 
                                     const std::string& model = "") = 0;
    
    /**
     * Send a simple prompt
     * @param prompt The user prompt
     * @param model Model name to use  
     * @return Response content from the LLM
     */
    virtual std::string ask(const std::string& prompt, 
                           const std::string& model = "");
    
    /**
     * Set API key for the provider
     */
    virtual void setApiKey(const std::string& key) = 0;
    
    /**
     * Get provider name
     */
    virtual std::string getProviderName() const = 0;
};

/**
 * OpenAI client implementation
 */
class OpenAIClient : public LLMClient {
private:
    std::string apiKey;
    std::string baseUrl;
    
public:
    OpenAIClient(const std::string& key = "", const std::string& url = "https://api.openai.com/v1");
    
    std::string chatCompletion(const std::vector<Message>& messages, 
                             const std::string& model = "gpt-3.5-turbo") override;
    void setApiKey(const std::string& key) override;
    std::string getProviderName() const override;
};

/**
 * Claude/Anthropic client implementation
 */
class ClaudeClient : public LLMClient {
private:
    std::string apiKey;
    std::string baseUrl;
    
public:
    ClaudeClient(const std::string& key = "", const std::string& url = "https://api.anthropic.com/v1");
    
    std::string chatCompletion(const std::vector<Message>& messages, 
                             const std::string& model = "claude-3-sonnet-20240229") override;
    void setApiKey(const std::string& key) override;
    std::string getProviderName() const override;
};

/**
 * GGML-based local client implementation
 */
class GGMLClient : public LLMClient {
private:
#ifdef HAVE_GGML
    llama_model* model = nullptr;
    llama_context* context = nullptr;
    llama_sampling_context* sampling_ctx = nullptr;
#endif
    
    std::string modelPath;
    std::string modelType;
    ModelParams modelParams;
    GenerationParams defaultGenParams;
    bool modelLoaded = false;
    
    // Thread safety
    mutable std::mutex modelMutex;
    
public:
    GGMLClient(const std::string& path = "", const std::string& type = "llama");
    ~GGMLClient();
    
    // LLMClient interface
    std::string chatCompletion(const std::vector<Message>& messages, 
                             const std::string& model = "") override;
    void setApiKey(const std::string& key) override;
    std::string getProviderName() const override;
    
    // GGML-specific methods
    bool loadModel(const std::string& path, const ModelParams& params = ModelParams{});
    void unloadModel();
    bool isModelLoaded() const { return modelLoaded; }
    std::string getModelPath() const { return modelPath; }
    
    // Text generation
    std::string generateText(const std::string& prompt, 
                           const GenerationParams& params = GenerationParams{});
    
    // Streaming generation
    void streamGeneration(const std::string& prompt,
                         StreamCallback callback,
                         const GenerationParams& params = GenerationParams{});
    
    // Parameter management
    void setDefaultGenerationParams(const GenerationParams& params);
    GenerationParams getDefaultGenerationParams() const;
    void setModelParams(const ModelParams& params);
    ModelParams getModelParams() const;
    
    // Model information
    ModelInfo getModelInfo() const;
    size_t getModelMemoryUsage() const;
    
    // Legacy compatibility
    void setModelPath(const std::string& path);
};

/**
 * Client factory
 */
class ClientFactory {
public:
    static std::unique_ptr<LLMClient> createClient(const std::string& provider, 
                                                  const std::string& apiKey = "");
};

} // namespace caichat
} // namespace opencog

#endif // LLMCLIENT_H