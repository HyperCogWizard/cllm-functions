#include "LLMClient.h"
#include "ChatCompletion.h"
#include <libguile.h>
#include <memory>
#include <map>

namespace opencog {
namespace caichat {

// Global storage for chat sessions
static std::map<std::string, std::unique_ptr<ChatCompletion>> chatSessions;

// Helper function to convert SCM string to C++ string
std::string scm_to_string(SCM scm_str) {
    char* c_str = scm_to_utf8_string(scm_str);
    std::string result(c_str);
    free(c_str);
    return result;
}

// Scheme wrapper: Create LLM client
SCM caichat_create_client(SCM provider_scm, SCM api_key_scm) {
    std::string provider = scm_to_string(provider_scm);
    std::string api_key = scm_is_string(api_key_scm) ? scm_to_string(api_key_scm) : "";
    
    try {
        auto client = ClientFactory::createClient(provider, api_key);
        auto session = std::make_unique<ChatCompletion>(std::move(client));
        
        std::string session_id = provider + "_session";
        chatSessions[session_id] = std::move(session);
        
        return scm_from_utf8_string(session_id.c_str());
    } catch (const std::exception& e) {
        scm_throw(scm_from_utf8_symbol("caichat-error"), 
                 scm_list_1(scm_from_utf8_string(e.what())));
        return SCM_BOOL_F;
    }
}

// Scheme wrapper: Send message
SCM caichat_send_message(SCM session_id_scm, SCM message_scm) {
    std::string session_id = scm_to_string(session_id_scm);
    std::string message = scm_to_string(message_scm);
    
    auto it = chatSessions.find(session_id);
    if (it == chatSessions.end()) {
        scm_throw(scm_from_utf8_symbol("caichat-error"), 
                 scm_list_1(scm_from_utf8_string("Session not found")));
        return SCM_BOOL_F;
    }
    
    try {
        std::string response = it->second->sendMessage(message);
        return scm_from_utf8_string(response.c_str());
    } catch (const std::exception& e) {
        scm_throw(scm_from_utf8_symbol("caichat-error"), 
                 scm_list_1(scm_from_utf8_string(e.what())));
        return SCM_BOOL_F;
    }
}

// Scheme wrapper: Simple ask function
SCM caichat_ask(SCM provider_scm, SCM message_scm) {
    std::string provider = scm_to_string(provider_scm);
    std::string message = scm_to_string(message_scm);
    
    try {
        auto client = ClientFactory::createClient(provider);
        std::string response = client->ask(message);
        return scm_from_utf8_string(response.c_str());
    } catch (const std::exception& e) {
        scm_throw(scm_from_utf8_symbol("caichat-error"), 
                 scm_list_1(scm_from_utf8_string(e.what())));
        return SCM_BOOL_F;
    }
}

// Scheme wrapper: Set system message
SCM caichat_set_system_message(SCM session_id_scm, SCM message_scm) {
    std::string session_id = scm_to_string(session_id_scm);
    std::string message = scm_to_string(message_scm);
    
    auto it = chatSessions.find(session_id);
    if (it == chatSessions.end()) {
        scm_throw(scm_from_utf8_symbol("caichat-error"), 
                 scm_list_1(scm_from_utf8_string("Session not found")));
        return SCM_BOOL_F;
    }
    
    it->second->setSystemMessage(message);
    return SCM_BOOL_T;
}

// Scheme wrapper: Clear history
SCM caichat_clear_history(SCM session_id_scm) {
    std::string session_id = scm_to_string(session_id_scm);
    
    auto it = chatSessions.find(session_id);
    if (it == chatSessions.end()) {
        scm_throw(scm_from_utf8_symbol("caichat-error"), 
                 scm_list_1(scm_from_utf8_string("Session not found")));
        return SCM_BOOL_F;
    }
    
    it->second->clearHistory();
    return SCM_BOOL_T;
}

// Scheme wrapper: Set model path for GGML
SCM caichat_set_model_path(SCM session_id_scm, SCM path_scm) {
    std::string session_id = scm_to_string(session_id_scm);
    std::string path = scm_to_string(path_scm);
    
    auto it = chatSessions.find(session_id);
    if (it == chatSessions.end()) {
        scm_throw(scm_from_utf8_symbol("caichat-error"), 
                 scm_list_1(scm_from_utf8_string("Session not found")));
        return SCM_BOOL_F;
    }
    
    // Check if this is a GGML client
    GGMLClient* ggmlClient = dynamic_cast<GGMLClient*>(it->second->getClient());
    if (ggmlClient) {
        ggmlClient->setModelPath(path);
        return SCM_BOOL_T;
    } else {
        scm_throw(scm_from_utf8_symbol("caichat-error"), 
                 scm_list_1(scm_from_utf8_string("Session is not a GGML client")));
        return SCM_BOOL_F;
    }
}

// Initialize the module
extern "C" void init_caichat_bindings() {
    scm_c_define_gsubr("caichat-create-client", 2, 0, 0, (scm_t_subr)caichat_create_client);
    scm_c_define_gsubr("caichat-send-message", 2, 0, 0, (scm_t_subr)caichat_send_message);
    scm_c_define_gsubr("caichat-ask-internal", 2, 0, 0, (scm_t_subr)caichat_ask);
    scm_c_define_gsubr("caichat-set-system-message", 2, 0, 0, (scm_t_subr)caichat_set_system_message);
    scm_c_define_gsubr("caichat-clear-history", 1, 0, 0, (scm_t_subr)caichat_clear_history);
    scm_c_define_gsubr("caichat-set-model-path", 2, 0, 0, (scm_t_subr)caichat_set_model_path);
}

} // namespace caichat
} // namespace opencog