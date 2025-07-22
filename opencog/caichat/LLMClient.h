#ifndef LLMCLIENT_H
#define LLMCLIENT_H

#include <string>
#include <vector>
#include <map>
#include <memory>

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
    std::string modelPath;
    std::string modelType;
    
public:
    GGMLClient(const std::string& path = "", const std::string& type = "llama");
    
    std::string chatCompletion(const std::vector<Message>& messages, 
                             const std::string& model = "") override;
    void setApiKey(const std::string& key) override;
    std::string getProviderName() const override;
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