This PR implements the complete foundation structure for transforming the LLM Functions repository into an OpenCog component, as outlined in the existing `OPENCOG-TRANSFORM.md` documentation.

## What's Implemented

### Core Architecture
- **C++ Core Library**: Abstract LLM client interfaces for OpenAI, Claude, and other providers
- **Scheme Interface**: Native Guile Scheme API for OpenCog integration
- **CMake Build System**: Replaces cargo-based builds with CMake for C++/Scheme ecosystem
- **Module Organization**: Proper OpenCog module structure with clear separation of concerns

### Key Components

**C++ Implementation (`opencog/caichat/`)**:
- `LLMClient.h/cc` - Abstract base class and concrete implementations for OpenAI/Claude APIs
- `ChatCompletion.h/cc` - Conversation management with history tracking
- `SchemeBindings.cc` - Guile Scheme bindings for C++ functionality

**Scheme Modules (`scm/opencog/caichat/`)**:
- `init.scm` - Main module interface with high-level functions
- `config.scm` - Configuration management with environment variable support
- `repl.scm` - Interactive REPL with command handling
- `rag.scm` - Retrieval Augmented Generation system foundation

**Build System & Examples**:
- `CMakeLists.txt` - Main CMake configuration with dependency detection
- `build-opencog.sh` - Build script with comprehensive dependency checking
- `example.scm` - Complete usage examples
- `test-caichat.scm` - Basic functionality tests

### Usage Example

```scheme
;; Load the module in OpenCog
(use-modules (opencog caichat init))

;; Quick question
(caichat-ask "What is cognitive architecture?")

;; Start interactive chat
(caichat-repl)

;; RAG with knowledge base
(caichat-rag-create-kb "research" "AI research papers")
(caichat-rag-add-doc "research" "paper1" "Content..." '())
(caichat-rag-query "research" "What are the latest developments?")
```

### Integration with Existing System

- **Minimal Changes**: Only 39 lines modified in existing `Argcfile.sh`
- **Backward Compatibility**: All existing tools and agents continue to work unchanged
- **Build Integration**: New `argc build@opencog` and `argc test@opencog` commands
- **Preserved Structure**: Original functionality maintained while adding OpenCog capabilities

### Dependencies

The implementation requires standard OpenCog development dependencies:
- CMake 3.10+
- Guile 3.0 (or 2.2)
- libcurl (for HTTP requests)
- jsoncpp (for JSON parsing)

### Status

This implements the "Foundation ✅" phase described in `OPENCOG-TRANSFORM.md`:
- ✅ Basic module structure
- ✅ Core C++ interfaces  
- ✅ Scheme bindings framework
- ✅ Configuration system

The next phase will focus on complete provider implementations, advanced AtomSpace features, and reasoning integration.

Fixes #1.

<!-- START COPILOT CODING AGENT TIPS -->
---

💡 You can make Copilot smarter by setting up custom instructions, customizing its development environment and configuring Model Context Protocol (MCP) servers. Learn more [Copilot coding agent tips](https://gh.io/copilot-coding-agent-tips) in the docs.
