#include "LLMClient.h"
#include <curl/curl.h>
#include <json/json.h>
#include <stdexcept>
#include <cstdlib>
#include <sstream>

namespace opencog {
namespace caichat {

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

// Base LLMClient implementation
std::string LLMClient::ask(const std::string& prompt, const std::string& model) {
    std::vector<Message> messages;
    messages.emplace_back("user", prompt);
    return chatCompletion(messages, model);
}

// OpenAI Client implementation
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

void OpenAIClient::setApiKey(const std::string& key) {
    apiKey = key;
}

std::string OpenAIClient::getProviderName() const {
    return "openai";
}

// Claude Client implementation
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

void ClaudeClient::setApiKey(const std::string& key) {
    apiKey = key;
}

std::string ClaudeClient::getProviderName() const {
    return "claude";
}

// ClientFactory implementation
std::unique_ptr<LLMClient> ClientFactory::createClient(const std::string& provider, const std::string& apiKey) {
    if (provider == "openai") {
        return std::make_unique<OpenAIClient>(apiKey);
    } else if (provider == "claude" || provider == "anthropic") {
        return std::make_unique<ClaudeClient>(apiKey);
    } else {
        throw std::runtime_error("Unknown provider: " + provider);
    }
}

} // namespace caichat
} // namespace opencog