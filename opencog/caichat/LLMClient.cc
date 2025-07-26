#include "LLMClient.h"
#ifdef HAVE_CURL
#include <curl/curl.h>
#endif
#ifdef HAVE_JSONCPP
#include <json/json.h>
#endif
#include <stdexcept>
#include <cstdlib>
#include <sstream>

namespace opencog {
namespace caichat {

#if defined(HAVE_CURL) && defined(HAVE_JSONCPP)

// Helper function for HTTP requests
struct HTTPResponse {
    std::string data;
    long code;
};

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, HTTPResponse* response) {
    size_t totalSize = size * nmemb;
    response->data.append((char*)contents, totalSize);
    return totalSize;
}

static HTTPResponse makeHTTPRequest(const std::string& url, const std::string& postData, 
                                   const std::vector<std::string>& headers) {
    CURL* curl;
    HTTPResponse response;
    
    curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    if (!postData.empty()) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
    }
    
    struct curl_slist* headerList = nullptr;
    for (const auto& header : headers) {
        headerList = curl_slist_append(headerList, header.c_str());
    }
    if (headerList) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    }
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.code);
    
    if (headerList) {
        curl_slist_free_all(headerList);
    }
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        throw std::runtime_error("HTTP request failed: " + std::string(curl_easy_strerror(res)));
    }
    
    return response;
}

#endif // HAVE_CURL && HAVE_JSONCPP

// Base LLMClient implementation
std::string LLMClient::ask(const std::string& prompt, const std::string& model) {
    std::vector<Message> messages;
    messages.emplace_back("user", prompt);
    return chatCompletion(messages, model);
}

// OpenAI Client implementation
#if defined(HAVE_CURL) && defined(HAVE_JSONCPP)

OpenAIClient::OpenAIClient(const std::string& key, const std::string& url) 
    : apiKey(key), baseUrl(url) {
    if (apiKey.empty()) {
        const char* envKey = std::getenv("OPENAI_API_KEY");
        if (envKey) {
            apiKey = envKey;
        }
    }
}

std::string OpenAIClient::chatCompletion(const std::vector<Message>& messages, const std::string& model) {
    if (apiKey.empty()) {
        throw std::runtime_error("OpenAI API key not set");
    }
    
    Json::Value request;
    request["model"] = model.empty() ? "gpt-3.5-turbo" : model;
    
    Json::Value jsonMessages(Json::arrayValue);
    for (const auto& msg : messages) {
        Json::Value jsonMsg;
        jsonMsg["role"] = msg.role;
        jsonMsg["content"] = msg.content;
        jsonMessages.append(jsonMsg);
    }
    request["messages"] = jsonMessages;
    
    Json::StreamWriterBuilder builder;
    std::string postData = Json::writeString(builder, request);
    
    std::vector<std::string> headers = {
        "Content-Type: application/json",
        "Authorization: Bearer " + apiKey
    };
    
    HTTPResponse response = makeHTTPRequest(baseUrl + "/chat/completions", postData, headers);
    
    if (response.code != 200) {
        throw std::runtime_error("OpenAI API request failed with code: " + std::to_string(response.code));
    }
    
    Json::Value jsonResponse;
    Json::CharReaderBuilder readerBuilder;
    std::string errors;
    std::istringstream stream(response.data);
    
    if (!Json::parseFromStream(readerBuilder, stream, &jsonResponse, &errors)) {
        throw std::runtime_error("Failed to parse OpenAI response: " + errors);
    }
    
    return jsonResponse["choices"][0]["message"]["content"].asString();
}

#else

OpenAIClient::OpenAIClient(const std::string& key, const std::string& url) 
    : apiKey(key), baseUrl(url) {
}

std::string OpenAIClient::chatCompletion(const std::vector<Message>& messages, const std::string& model) {
    throw std::runtime_error("OpenAI client not available - CURL and JsonCpp required");
}

#endif

void OpenAIClient::setApiKey(const std::string& key) {
    apiKey = key;
}

std::string OpenAIClient::getProviderName() const {
    return "openai";
}

// Claude Client implementation
#if defined(HAVE_CURL) && defined(HAVE_JSONCPP)

ClaudeClient::ClaudeClient(const std::string& key, const std::string& url) 
    : apiKey(key), baseUrl(url) {
    if (apiKey.empty()) {
        const char* envKey = std::getenv("ANTHROPIC_API_KEY");
        if (envKey) {
            apiKey = envKey;
        }
    }
}

std::string ClaudeClient::chatCompletion(const std::vector<Message>& messages, const std::string& model) {
    if (apiKey.empty()) {
        throw std::runtime_error("Anthropic API key not set");
    }
    
    Json::Value request;
    request["model"] = model.empty() ? "claude-3-sonnet-20240229" : model;
    request["max_tokens"] = 1000;
    
    Json::Value jsonMessages(Json::arrayValue);
    for (const auto& msg : messages) {
        Json::Value jsonMsg;
        jsonMsg["role"] = msg.role;
        jsonMsg["content"] = msg.content;
        jsonMessages.append(jsonMsg);
    }
    request["messages"] = jsonMessages;
    
    Json::StreamWriterBuilder builder;
    std::string postData = Json::writeString(builder, request);
    
    std::vector<std::string> headers = {
        "Content-Type: application/json",
        "x-api-key: " + apiKey,
        "anthropic-version: 2023-06-01"
    };
    
    HTTPResponse response = makeHTTPRequest(baseUrl + "/messages", postData, headers);
    
    if (response.code != 200) {
        throw std::runtime_error("Claude API request failed with code: " + std::to_string(response.code));
    }
    
    Json::Value jsonResponse;
    Json::CharReaderBuilder readerBuilder;
    std::string errors;
    std::istringstream stream(response.data);
    
    if (!Json::parseFromStream(readerBuilder, stream, &jsonResponse, &errors)) {
        throw std::runtime_error("Failed to parse Claude response: " + errors);
    }
    
    return jsonResponse["content"][0]["text"].asString();
}

#else

ClaudeClient::ClaudeClient(const std::string& key, const std::string& url) 
    : apiKey(key), baseUrl(url) {
}

std::string ClaudeClient::chatCompletion(const std::vector<Message>& messages, const std::string& model) {
    throw std::runtime_error("Claude client not available - CURL and JsonCpp required");
}

#endif

void ClaudeClient::setApiKey(const std::string& key) {
    apiKey = key;
}

std::string ClaudeClient::getProviderName() const {
    return "claude";
}

// GGML Client implementation
GGMLClient::GGMLClient(const std::string& path, const std::string& type) 
    : modelPath(path), modelType(type) {
    if (modelPath.empty()) {
        const char* envPath = std::getenv("GGML_MODEL_PATH");
        if (envPath) {
            modelPath = envPath;
        }
    }
    
#ifdef HAVE_GGML
    // Initialize llama backend
    llama_backend_init();
#endif
}

GGMLClient::~GGMLClient() {
    unloadModel();
#ifdef HAVE_GGML
    llama_backend_free();
#endif
}

bool GGMLClient::loadModel(const std::string& path, const ModelParams& params) {
    std::lock_guard<std::mutex> lock(modelMutex);
    
    // Validate parameters
    if (!params.isValid()) {
        throw std::runtime_error("Invalid model parameters");
    }
    
    // Unload existing model
    if (modelLoaded) {
        unloadModel();
    }
    
#ifdef HAVE_GGML
    try {
        // Set up model parameters
        llama_model_params model_params = llama_model_default_params();
        model_params.n_gpu_layers = params.n_gpu_layers;
        model_params.use_mmap = params.use_mmap;
        model_params.use_mlock = params.use_mlock;
        
        // Load model
        model = llama_load_model_from_file(path.c_str(), model_params);
        if (!model) {
            throw std::runtime_error("Failed to load model from: " + path);
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
            throw std::runtime_error("Failed to create context for model");
        }
        
        // Initialize sampling context
        llama_sampling_params sampling_params = {};
        sampling_ctx = llama_sampling_init(sampling_params);
        
        modelPath = path;
        modelParams = params;
        modelLoaded = true;
        
        return true;
    } catch (const std::exception& e) {
        // Cleanup on failure
        if (context) {
            llama_free(context);
            context = nullptr;
        }
        if (model) {
            llama_free_model(model);
            model = nullptr;
        }
        throw;
    }
#else
    // Fallback for when GGML is not available
    throw std::runtime_error("GGML support not compiled in - cannot load model");
#endif
}

void GGMLClient::unloadModel() {
    std::lock_guard<std::mutex> lock(modelMutex);
    
#ifdef HAVE_GGML
    if (sampling_ctx) {
        llama_sampling_free(sampling_ctx);
        sampling_ctx = nullptr;
    }
    
    if (context) {
        llama_free(context);
        context = nullptr;
    }
    
    if (model) {
        llama_free_model(model);
        model = nullptr;
    }
#endif
    
    modelLoaded = false;
}

std::string GGMLClient::generateText(const std::string& prompt, const GenerationParams& params) {
    std::lock_guard<std::mutex> lock(modelMutex);
    
    if (!modelLoaded) {
        throw std::runtime_error("Model not loaded");
    }
    
    // Validate generation parameters
    if (!params.isValid()) {
        throw std::runtime_error("Invalid generation parameters");
    }
    
#ifdef HAVE_GGML
    try {
        // Tokenize prompt
        std::vector<llama_token> tokens = llama_tokenize(context, prompt, true, true);
        
        if (tokens.empty()) {
            throw std::runtime_error("Failed to tokenize prompt");
        }
        
        // Check context size
        if (tokens.size() >= (size_t)llama_n_ctx(context)) {
            throw std::runtime_error("Prompt too long for context size");
        }
        
        // Evaluate prompt
        llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size(), 0, 0);
        if (llama_decode(context, batch) != 0) {
            throw std::runtime_error("Failed to evaluate prompt");
        }
        
        // Generate tokens
        std::string result;
        for (int i = 0; i < params.n_predict; ++i) {
            // Sample next token
            llama_token new_token = llama_sampling_sample(sampling_ctx, context, nullptr);
            
            // Check for end of sequence
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
            
            // Add token to context for next iteration
            llama_batch next_batch = llama_batch_get_one(&new_token, 1, tokens.size() + i, 0);
            if (llama_decode(context, next_batch) != 0) {
                break;
            }
        }
        
        return result;
    } catch (const std::exception& e) {
        throw std::runtime_error("Text generation failed: " + std::string(e.what()));
    }
#else
    // Fallback when GGML is not available
    return "GGML not available - using placeholder response for prompt: " + prompt;
#endif
}

std::string GGMLClient::chatCompletion(const std::vector<Message>& messages, const std::string& model) {
    if (!modelLoaded && !modelPath.empty()) {
        // Try to auto-load the model
        try {
            loadModel(modelPath, modelParams);
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to auto-load model: " + std::string(e.what()));
        }
    }
    
    if (!modelLoaded) {
        throw std::runtime_error("GGML model not loaded and no path specified");
    }
    
    // Convert messages to prompt format
    std::string prompt = "";
    for (const auto& msg : messages) {
        if (msg.role == "system") {
            prompt += "System: " + msg.content + "\n";
        } else if (msg.role == "user") {
            prompt += "User: " + msg.content + "\n";
        } else if (msg.role == "assistant") {
            prompt += "Assistant: " + msg.content + "\n";
        }
    }
    prompt += "Assistant: ";
    
    return generateText(prompt, defaultGenParams);
}

void GGMLClient::streamGeneration(const std::string& prompt, StreamCallback callback, const GenerationParams& params) {
    std::lock_guard<std::mutex> lock(modelMutex);
    
    if (!modelLoaded) {
        callback("", "Model not loaded", true);
        return;
    }
    
#ifdef HAVE_GGML
    try {
        // Tokenize and setup (similar to generateText)
        std::vector<llama_token> tokens = llama_tokenize(context, prompt, true, true);
        
        if (tokens.empty()) {
            callback("", "Failed to tokenize prompt", true);
            return;
        }
        
        // Evaluate prompt
        llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size(), 0, 0);
        if (llama_decode(context, batch) != 0) {
            callback("", "Failed to evaluate prompt", true);
            return;
        }
        
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
            llama_batch next_batch = llama_batch_get_one(&new_token, 1, tokens.size() + i, 0);
            if (llama_decode(context, next_batch) != 0) {
                callback("", "Generation failed", true);
                break;
            }
        }
    } catch (const std::exception& e) {
        callback("", e.what(), true);
    }
#else
    // Fallback for non-GGML builds
    callback("GGML not available - placeholder token", "", false);
    callback("", "", true);
#endif
}

ModelInfo GGMLClient::getModelInfo() const {
    std::lock_guard<std::mutex> lock(modelMutex);
    
    ModelInfo info;
    info.path = modelPath;
    
#ifdef HAVE_GGML
    if (model && context) {
        info.parameter_count = llama_model_n_params(model);
        info.vocab_size = llama_n_vocab(model);
        info.context_length = llama_n_ctx(context);
        info.memory_usage_bytes = llama_get_state_size(context);
        info.architecture = "llama"; // Could be made more specific
    }
#endif
    
    return info;
}

size_t GGMLClient::getModelMemoryUsage() const {
    std::lock_guard<std::mutex> lock(modelMutex);
    
#ifdef HAVE_GGML
    if (context) {
        return llama_get_state_size(context);
    }
#endif
    return 0;
}

void GGMLClient::setDefaultGenerationParams(const GenerationParams& params) {
    if (params.isValid()) {
        defaultGenParams = params;
    } else {
        throw std::runtime_error("Invalid generation parameters");
    }
}

GenerationParams GGMLClient::getDefaultGenerationParams() const {
    return defaultGenParams;
}

void GGMLClient::setModelParams(const ModelParams& params) {
    if (params.isValid()) {
        modelParams = params;
    } else {
        throw std::runtime_error("Invalid model parameters");
    }
}

ModelParams GGMLClient::getModelParams() const {
    return modelParams;
}

void GGMLClient::setApiKey(const std::string& key) {
    // GGML models don't use API keys, but we can use this for model path or other config
    if (!key.empty()) {
        modelPath = key;
    }
}

std::string GGMLClient::getProviderName() const {
    return "ggml";
}

void GGMLClient::setModelPath(const std::string& path) {
    modelPath = path;
}

// ClientFactory implementation
std::unique_ptr<LLMClient> ClientFactory::createClient(const std::string& provider, const std::string& apiKey) {
    if (provider == "openai") {
        return std::make_unique<OpenAIClient>(apiKey);
    } else if (provider == "claude" || provider == "anthropic") {
        return std::make_unique<ClaudeClient>(apiKey);
    } else if (provider == "ggml" || provider == "local") {
        return std::make_unique<GGMLClient>(apiKey);  // apiKey is used as model path for GGML
    } else {
        throw std::runtime_error("Unknown provider: " + provider);
    }
}

} // namespace caichat
} // namespace opencog