#include "ChatCompletion.h"

namespace opencog {
namespace caichat {

ChatCompletion::ChatCompletion(std::unique_ptr<LLMClient> llmClient, const std::string& model)
    : client(std::move(llmClient)), defaultModel(model) {
}

std::string ChatCompletion::sendMessage(const std::string& message, const std::string& role) {
    // Add user message to history
    conversationHistory.emplace_back(role, message);
    
    // Get response from LLM
    std::string response = client->chatCompletion(conversationHistory, defaultModel);
    
    // Add assistant response to history
    conversationHistory.emplace_back("assistant", response);
    
    return response;
}

const std::vector<Message>& ChatCompletion::getHistory() const {
    return conversationHistory;
}

void ChatCompletion::clearHistory() {
    conversationHistory.clear();
}

void ChatCompletion::setSystemMessage(const std::string& message) {
    // Remove existing system message if any
    auto it = conversationHistory.begin();
    if (it != conversationHistory.end() && it->role == "system") {
        conversationHistory.erase(it);
    }
    
    // Add new system message at the beginning
    conversationHistory.insert(conversationHistory.begin(), Message("system", message));
}

LLMClient* ChatCompletion::getClient() const {
    return client.get();
}

} // namespace caichat
} // namespace opencog