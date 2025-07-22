#!/usr/bin/env guile
!#

;; Example usage of CAIChat OpenCog module with GGML support

(use-modules (opencog caichat init)
             (opencog caichat config)
             (opencog caichat repl)
             (opencog caichat rag))

;; Initialize configuration
(caichat-config-init)

;; Example 1: Simple question with different providers
(display "Example 1: Simple question with different providers\n")
(display "Question: What is cognitive architecture?\n")

;; Try OpenAI (requires API key)
(display "OpenAI Answer: ")
(catch 'caichat-error
  (lambda ()
    (display (caichat-ask "What is cognitive architecture in 2 sentences?")))
  (lambda (key . args)
    (display "Error: API key required")))
(newline)

;; Try GGML local model (placeholder implementation)
(display "GGML Answer: ")
(catch 'caichat-error
  (lambda ()
    (caichat-setup-ggml "/path/to/local/model.gguf")
    (display (caichat-ask "What is cognitive architecture in 2 sentences?")))
  (lambda (key . args)
    (display "Error: Local model not available")))
(newline)
(newline)

;; Example 2: Configuration with GGML
(display "Example 2: Configuration with GGML support\n")
(caichat-config-set! 'default-provider "ggml")
(caichat-config-set! 'ggml-model-path "/path/to/model.gguf")
(display (format #f "Default provider: ~a\n" 
                (caichat-config-get 'default-provider)))
(display (format #f "GGML model path: ~a\n" 
                (caichat-config-get 'ggml-model-path)))
(newline)

;; Example 3: RAG system with local models
(display "Example 3: RAG system with local models\n")
(caichat-rag-create-kb "ai-research" "AI research knowledge base")
(caichat-rag-add-doc "ai-research" "paper1" 
                     "Cognitive architecture is a framework for understanding intelligence. It provides a blueprint for building AI systems." 
                     '(("author" . "Smith") ("year" . "2024")))
(caichat-rag-add-doc "ai-research" "paper2" 
                     "OpenCog is an open-source cognitive architecture platform. It uses AtomSpace for knowledge representation." 
                     '(("author" . "Goertzel") ("year" . "2023")))
(caichat-rag-add-doc "ai-research" "paper3" 
                     "GGML is a tensor library for machine learning. It enables efficient inference of large language models on consumer hardware." 
                     '(("author" . "Vasilev") ("year" . "2023")))

(display "RAG Query: What is GGML?\n")
(display "Answer: ")
(catch 'caichat-error
  (lambda ()
    (display (caichat-rag-query "ai-research" "What is GGML?")))
  (lambda (key . args)
    (display "Error: Model not available")))
(newline)
(newline)

;; Example 4: OpenCog integration (placeholder)
(display "Example 4: OpenCog integration\n")
(display "This example demonstrates how CAIChat integrates with OpenCog's AtomSpace.\n")
(display "Future enhancements will include:\n")
(display "- Storing conversations as atoms\n")
(display "- Using PLN for reasoning about knowledge\n")
(display "- Integrating with OpenCog's learning systems\n")
(display "- Supporting multi-agent scenarios\n")
(newline)

;; Example 5: Interactive REPL (commented out for non-interactive execution)
;; (display "Starting interactive REPL...\n")
;; (caichat-repl-start)

(display "Examples completed successfully!\n")