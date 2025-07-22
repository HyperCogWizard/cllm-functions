# CAIChat OpenCog Module

This directory contains the OpenCog implementation of CAIChat, providing LLM functionality with cognitive architecture integration.

## Features

- **Multiple LLM Providers**: OpenAI, Claude/Anthropic, and GGML local models
- **OpenCog Integration**: AtomSpace knowledge representation
- **RAG System**: Retrieval Augmented Generation with knowledge bases
- **Interactive REPL**: Chat interface with conversation management
- **Configuration Management**: Persistent settings and API key management

## Installation

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get install cmake build-essential pkg-config guile-3.0-dev libcurl4-openssl-dev libjsoncpp-dev

# Fedora
sudo dnf install cmake gcc-c++ pkg-config guile-devel libcurl-devel jsoncpp-devel

# macOS
brew install cmake pkg-config guile curl jsoncpp
```

### Build

```bash
./build-opencog.sh
```

Or manually:

```bash
mkdir -p build && cd build
cmake ..
make
```

## Quick Start

### Basic Usage

```scheme
#!/usr/bin/env guile
!#

;; Load the modules
(use-modules (opencog caichat init))

;; Set up your preferred provider
(caichat-setup-openai "your-api-key")
;; or
(caichat-setup-claude "your-api-key")
;; or for local models
(caichat-setup-ggml "/path/to/model.gguf")

;; Ask a question
(display (caichat-ask "What is cognitive architecture?"))
```

### Configuration

```scheme
(use-modules (opencog caichat config))

;; Initialize configuration
(caichat-config-init)

;; Set custom values
(caichat-config-set! 'temperature 0.8)
(caichat-config-set! 'max-tokens 2000)

;; Get values
(caichat-config-get 'temperature)

;; Save/load configuration
(caichat-config-save "~/.caichat/config.scm")
(caichat-config-load "~/.caichat/config.scm")
```

### Interactive REPL

```scheme
(use-modules (opencog caichat repl))

;; Start interactive chat
(caichat-repl-start)

;; Or with a custom system message
(caichat-repl-with-system-message 
  "You are an expert in cognitive science and AI.")
```

### RAG (Retrieval Augmented Generation)

```scheme
(use-modules (opencog caichat rag))

;; Create a knowledge base
(caichat-rag-create-kb "research" "AI research papers")

;; Add documents
(caichat-rag-add-doc "research" "paper1" 
  "Content of research paper about neural networks..."
  '(("author" . "Smith et al.")
    ("year" . "2024")
    ("topic" . "neural-networks")))

;; Query with context
(caichat-rag-query "research" "What are the latest developments in neural networks?")

;; Search documents
(caichat-rag-search "research" "neural networks")

;; List all documents
(caichat-rag-list-docs "research")
```

### AtomSpace Integration

```scheme
(use-modules (opencog caichat atomspace))

;; Add concepts to AtomSpace
(caichat-atomspace-add-concept "MachineLearning" 
  "A method of data analysis that automates analytical model building")

;; Store conversations
(caichat-atomspace-store-conversation "session1" 
  '(("user" "What is AI?")
    ("assistant" "AI is artificial intelligence...")))

;; Query knowledge
(caichat-atomspace-query-knowledge "cognitive architecture")

;; Get related concepts
(caichat-atomspace-get-related "MachineLearning")
```

## Providers

### OpenAI

```scheme
;; Set API key
(caichat-setup-openai "sk-your-openai-api-key")

;; Or via environment variable
(setenv "OPENAI_API_KEY" "sk-your-openai-api-key")
```

### Claude/Anthropic

```scheme
;; Set API key
(caichat-setup-claude "your-claude-api-key")

;; Or via environment variable
(setenv "ANTHROPIC_API_KEY" "your-claude-api-key")
```

### GGML Local Models

```scheme
;; Set model path
(caichat-setup-ggml "/path/to/llama-model.gguf")

;; Or via environment variable
(setenv "GGML_MODEL_PATH" "/path/to/llama-model.gguf")

;; Create session with specific model
(define session (caichat-create-client "ggml" "/path/to/model.gguf"))
```

## Session Management

```scheme
;; Create a session
(define session (caichat-create-client "openai" "your-api-key"))

;; Send messages
(caichat-send-message session "Hello!")

;; Set system message
(caichat-set-system-message session "You are a helpful assistant.")

;; Clear conversation history
(caichat-clear-history session)
```

## Examples

See `../example.scm` for comprehensive usage examples covering all features.

## Testing

```bash
# Run basic tests
GUILE_LOAD_PATH=../scm guile ../test-caichat.scm

# Run with specific library path
CAICHAT_LIB_PATH=../build/opencog/libcaichat.so GUILE_LOAD_PATH=../scm guile ../test-caichat.scm
```

## Architecture

### C++ Core

- `LLMClient.h/cc`: Abstract base class and provider implementations
- `ChatCompletion.h/cc`: Session management and conversation handling
- `SchemeBindings.cc`: Guile Scheme bindings for C++ functions

### Scheme Modules

- `../scm/opencog/caichat/init.scm`: Main module with core functions
- `../scm/opencog/caichat/config.scm`: Configuration management
- `../scm/opencog/caichat/rag.scm`: Retrieval Augmented Generation
- `../scm/opencog/caichat/repl.scm`: Interactive chat interface
- `../scm/opencog/caichat/atomspace.scm`: OpenCog AtomSpace integration

## Development

### Adding New Providers

1. Create a new class inheriting from `LLMClient` in `LLMClient.h`
2. Implement required methods in `LLMClient.cc`
3. Add to `ClientFactory::createClient()`
4. Add Scheme setup function in `../scm/opencog/caichat/init.scm`

### Extending AtomSpace Integration

1. Add new functions to `../scm/opencog/caichat/atomspace.scm`
2. For C++ integration, add bindings in `SchemeBindings.cc`
3. Update exports in module definitions

## Troubleshooting

### Library Not Found

```bash
# Set library path
export CAICHAT_LIB_PATH=/full/path/to/build/opencog/libcaichat.so

# Or copy to system library directory
sudo cp build/opencog/libcaichat.so /usr/local/lib/
sudo ldconfig
```

### Module Import Errors

```bash
# Set Guile load path
export GUILE_LOAD_PATH=/path/to/cllm-functions/scm:$GUILE_LOAD_PATH
```

### API Key Issues

```bash
# Set environment variables
export OPENAI_API_KEY="your-key"
export ANTHROPIC_API_KEY="your-key"
export GGML_MODEL_PATH="/path/to/model"
```

## Future Enhancements

- Real GGML model loading and inference
- Complete OpenCog AtomSpace operations
- PLN (Probabilistic Logic Networks) integration
- Vector embeddings for enhanced RAG
- Multi-agent conversation support
- Learning from conversations
- Automated knowledge extraction

## License

This project maintains the same dual license as the original:
- MIT License
- Apache License 2.0