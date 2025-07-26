#include <iostream>
#include <string>
#include "../opencog/caichat/LLMClient.h"

using namespace opencog::caichat;

void test_ggml_basic_functionality() {
    std::cout << "=== Testing GGML Client Basic Functionality ===" << std::endl;
    
    try {
        // Test creating GGML client
        GGMLClient client;
        std::cout << "✓ GGMLClient created successfully" << std::endl;
        
        // Test provider name
        std::string provider = client.getProviderName();
        if (provider == "ggml") {
            std::cout << "✓ Provider name correct: " << provider << std::endl;
        } else {
            std::cout << "✗ Provider name incorrect: " << provider << std::endl;
        }
        
        // Test initial state
        if (!client.isModelLoaded()) {
            std::cout << "✓ Model initially not loaded (correct)" << std::endl;
        } else {
            std::cout << "✗ Model should not be loaded initially" << std::endl;
        }
        
        // Test setting model path
        client.setModelPath("/test/path/model.gguf");
        if (client.getModelPath() == "/test/path/model.gguf") {
            std::cout << "✓ Model path set correctly" << std::endl;
        } else {
            std::cout << "✗ Model path not set correctly" << std::endl;
        }
        
        // Test parameter management
        GenerationParams genParams;
        genParams.n_predict = 50;
        genParams.temp = 0.8f;
        genParams.top_p = 0.9f;
        
        if (genParams.isValid()) {
            client.setDefaultGenerationParams(genParams);
            GenerationParams retrieved = client.getDefaultGenerationParams();
            
            if (retrieved.n_predict == 50 && retrieved.temp == 0.8f) {
                std::cout << "✓ Generation parameters set and retrieved correctly" << std::endl;
            } else {
                std::cout << "✗ Generation parameters not preserved correctly" << std::endl;
            }
        }
        
        ModelParams modelParams;
        modelParams.n_ctx = 4096;
        modelParams.n_batch = 256;
        
        if (modelParams.isValid()) {
            client.setModelParams(modelParams);
            ModelParams retrieved = client.getModelParams();
            
            if (retrieved.n_ctx == 4096 && retrieved.n_batch == 256) {
                std::cout << "✓ Model parameters set and retrieved correctly" << std::endl;
            } else {
                std::cout << "✗ Model parameters not preserved correctly" << std::endl;
            }
        }
        
        // Test model loading with invalid path (should fail gracefully)
        try {
            bool loaded = client.loadModel("/nonexistent/model.gguf");
            if (!loaded) {
                std::cout << "✓ Model loading failed gracefully for invalid path" << std::endl;
            } else {
                std::cout << "✗ Model loading should fail for invalid path" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "✓ Model loading threw exception for invalid path: " << e.what() << std::endl;
        }
        
        // Test chat completion without loaded model
        try {
            std::vector<Message> messages;
            messages.emplace_back("user", "Hello");
            std::string response = client.chatCompletion(messages);
            
            // Should either work with fallback or throw appropriate error
            if (!response.empty()) {
                std::cout << "✓ Chat completion returned response (fallback mode): " << response.substr(0, 50) << "..." << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "✓ Chat completion threw appropriate error: " << e.what() << std::endl;
        }
        
        std::cout << "=== GGML Basic Functionality Tests Complete ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "✗ Unexpected error in GGML tests: " << e.what() << std::endl;
    }
}

void test_client_factory() {
    std::cout << "\n=== Testing Client Factory ===" << std::endl;
    
    try {
        // Test creating GGML client through factory
        auto client = ClientFactory::createClient("ggml", "/test/model.gguf");
        if (client && client->getProviderName() == "ggml") {
            std::cout << "✓ ClientFactory created GGML client successfully" << std::endl;
        } else {
            std::cout << "✗ ClientFactory failed to create GGML client" << std::endl;
        }
        
        // Test creating OpenAI client (should work even without CURL)
        try {
            auto openai_client = ClientFactory::createClient("openai", "test-key");
            if (openai_client && openai_client->getProviderName() == "openai") {
                std::cout << "✓ ClientFactory created OpenAI client successfully" << std::endl;
            } else {
                std::cout << "✗ ClientFactory failed to create OpenAI client" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "✓ OpenAI client creation handled gracefully: " << e.what() << std::endl;
        }
        
        // Test creating Claude client
        try {
            auto claude_client = ClientFactory::createClient("claude", "test-key");
            if (claude_client && claude_client->getProviderName() == "claude") {
                std::cout << "✓ ClientFactory created Claude client successfully" << std::endl;
            } else {
                std::cout << "✗ ClientFactory failed to create Claude client" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "✓ Claude client creation handled gracefully: " << e.what() << std::endl;
        }
        
        // Test invalid provider
        try {
            auto invalid_client = ClientFactory::createClient("invalid", "key");
            std::cout << "✗ Should have thrown exception for invalid provider" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "✓ Invalid provider handled correctly: " << e.what() << std::endl;
        }
        
        std::cout << "=== Client Factory Tests Complete ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "✗ Unexpected error in factory tests: " << e.what() << std::endl;
    }
}

void test_parameter_validation() {
    std::cout << "\n=== Testing Parameter Validation ===" << std::endl;
    
    // Test valid parameters
    GenerationParams validGen;
    validGen.n_predict = 100;
    validGen.temp = 0.7f;
    validGen.top_p = 0.95f;
    
    if (validGen.isValid()) {
        std::cout << "✓ Valid generation parameters accepted" << std::endl;
    } else {
        std::cout << "✗ Valid generation parameters rejected" << std::endl;
    }
    
    // Test invalid parameters
    GenerationParams invalidGen;
    invalidGen.n_predict = -1;  // Invalid
    invalidGen.temp = -0.5f;    // Invalid
    
    if (!invalidGen.isValid()) {
        std::cout << "✓ Invalid generation parameters rejected" << std::endl;
    } else {
        std::cout << "✗ Invalid generation parameters accepted" << std::endl;
    }
    
    // Test model parameters
    ModelParams validModel;
    validModel.n_ctx = 2048;
    validModel.n_batch = 512;
    
    if (validModel.isValid()) {
        std::cout << "✓ Valid model parameters accepted" << std::endl;
    } else {
        std::cout << "✗ Valid model parameters rejected" << std::endl;
    }
    
    ModelParams invalidModel;
    invalidModel.n_ctx = -1;      // Invalid
    invalidModel.rope_freq_base = -1.0f;  // Invalid
    
    if (!invalidModel.isValid()) {
        std::cout << "✓ Invalid model parameters rejected" << std::endl;
    } else {
        std::cout << "✗ Invalid model parameters accepted" << std::endl;
    }
    
    std::cout << "=== Parameter Validation Tests Complete ===" << std::endl;
}

int main() {
    std::cout << "GGML Integration Test Suite" << std::endl;
    std::cout << "===========================" << std::endl;
    
    test_ggml_basic_functionality();
    test_client_factory();
    test_parameter_validation();
    
    std::cout << "\n=== All Tests Complete ===" << std::endl;
    std::cout << "Note: GGML library not linked - only testing interface compatibility" << std::endl;
    
    return 0;
}