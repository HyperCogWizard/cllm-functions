#ifndef CHATCOMPLETION_H
#define CHATCOMPLETION_H

#include "LLMClient.h"
#include <memory>
#include <vector>

namespace opencog {
namespace caichat {

/**
 * Chat session management
 */
class ChatCompletion {
private:
    std::unique_ptr<LLMClient> client;
    std::vector<Message> conversationHistory;
    std::string defaultModel;
    
public:
    ChatCompletion(std::unique_ptr<LLMClient> llmClient, const std::string& model = "");
    
    /**
     * Send a message and get response
     */
    std::string sendMessage(const std::string& message, const std::string& role = "user");
    
    /**
     * Get conversation history
     */
    const std::vector<Message>& getHistory() const;
    
    /**
     * Clear conversation history
     */
    void clearHistory();
    
    /**
     * Set system message
     */
    void setSystemMessage(const std::string& message);
    
    /**
     * Get the underlying client
     */
    LLMClient* getClient() const;
};

} // namespace caichat
} // namespace opencog

#endif // CHATCOMPLETION_H