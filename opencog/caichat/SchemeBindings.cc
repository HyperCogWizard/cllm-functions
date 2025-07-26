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

// Scheme wrapper: Load GGML model
SCM caichat_ggml_load_model(SCM path_scm, SCM params_scm) {
    if (!scm_is_string(path_scm)) {
        return SCM_BOOL_F;
    }
    
    std::string path = scm_to_string(path_scm);
    
    // Parse parameters if provided (simplified for now)
    ModelParams params;
    if (!scm_is_null(params_scm) && scm_is_pair(params_scm)) {
        // Basic parameter extraction - could be enhanced
        // For now, use defaults
    }
    
    try {
        // Create a new GGML client and load the model
        auto client = std::make_unique<GGMLClient>();
        bool success = client->loadModel(path, params);
        
        if (success) {
            auto session = std::make_unique<ChatCompletion>(std::move(client));
            std::string session_id = "ggml_session_" + std::to_string(chatSessions.size());
            chatSessions[session_id] = std::move(session);
            return scm_from_utf8_string(session_id.c_str());
        } else {
            return SCM_BOOL_F;
        }
    } catch (const std::exception& e) {
        scm_throw(scm_from_utf8_symbol("caichat-error"), 
                 scm_list_1(scm_from_utf8_string(e.what())));
        return SCM_BOOL_F;
    }
}

// Scheme wrapper: Generate text with GGML
SCM caichat_ggml_generate(SCM session_id_scm, SCM prompt_scm, SCM params_scm) {
    if (!scm_is_string(session_id_scm) || !scm_is_string(prompt_scm)) {
        return scm_from_utf8_string("");
    }
    
    std::string session_id = scm_to_string(session_id_scm);
    std::string prompt = scm_to_string(prompt_scm);
    
    auto it = chatSessions.find(session_id);
    if (it == chatSessions.end()) {
        scm_throw(scm_from_utf8_symbol("caichat-error"), 
                 scm_list_1(scm_from_utf8_string("Session not found")));
        return scm_from_utf8_string("");
    }
    
    // Check if this is a GGML client
    GGMLClient* ggmlClient = dynamic_cast<GGMLClient*>(it->second->getClient());
    if (!ggmlClient) {
        scm_throw(scm_from_utf8_symbol("caichat-error"), 
                 scm_list_1(scm_from_utf8_string("Session is not a GGML client")));
        return scm_from_utf8_string("");
    }
    
    try {
        // Parse generation parameters if provided
        GenerationParams params;
        if (!scm_is_null(params_scm) && scm_is_pair(params_scm)) {
            // Basic parameter extraction - could be enhanced
        }
        
        std::string result = ggmlClient->generateText(prompt, params);
        return scm_from_utf8_string(result.c_str());
    } catch (const std::exception& e) {
        scm_throw(scm_from_utf8_symbol("caichat-error"), 
                 scm_list_1(scm_from_utf8_string(e.what())));
        return scm_from_utf8_string("");
    }
}

// Scheme wrapper: Get model info
SCM caichat_ggml_model_info(SCM session_id_scm) {
    if (!scm_is_string(session_id_scm)) {
        return SCM_BOOL_F;
    }
    
    std::string session_id = scm_to_string(session_id_scm);
    
    auto it = chatSessions.find(session_id);
    if (it == chatSessions.end()) {
        return SCM_BOOL_F;
    }
    
    GGMLClient* ggmlClient = dynamic_cast<GGMLClient*>(it->second->getClient());
    if (!ggmlClient || !ggmlClient->isModelLoaded()) {
        return SCM_BOOL_F;
    }
    
    try {
        ModelInfo info = ggmlClient->getModelInfo();
        
        // Create Scheme association list
        SCM result = SCM_EOL;
        result = scm_acons(scm_from_utf8_symbol("path"),
                          scm_from_utf8_string(info.path.c_str()), result);
        result = scm_acons(scm_from_utf8_symbol("parameters"),
                          scm_from_int64(info.parameter_count), result);
        result = scm_acons(scm_from_utf8_symbol("vocab-size"),
                          scm_from_int32(info.vocab_size), result);
        result = scm_acons(scm_from_utf8_symbol("context-length"),
                          scm_from_int32(info.context_length), result);
        result = scm_acons(scm_from_utf8_symbol("memory-usage"),
                          scm_from_size_t(info.memory_usage_bytes), result);
        result = scm_acons(scm_from_utf8_symbol("architecture"),
                          scm_from_utf8_string(info.architecture.c_str()), result);
        
        return result;
    } catch (const std::exception& e) {
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
    
    // New GGML-specific bindings
    scm_c_define_gsubr("caichat-ggml-load-model", 2, 0, 0, (scm_t_subr)caichat_ggml_load_model);
    scm_c_define_gsubr("caichat-ggml-generate", 3, 0, 0, (scm_t_subr)caichat_ggml_generate);
    scm_c_define_gsubr("caichat-ggml-model-info", 1, 0, 0, (scm_t_subr)caichat_ggml_model_info);
}

} // namespace caichat
} // namespace opencog