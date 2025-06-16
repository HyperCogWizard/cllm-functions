#!/usr/bin/env guile
!#

;; Example usage of CAIChat OpenCog module

(use-modules (opencog caichat init)
             (opencog caichat config)
             (opencog caichat repl)
             (opencog caichat rag))

;; Initialize configuration
(caichat-config-init)

;; Example 1: Simple question
(display "Example 1: Simple question\n")
(display "Question: What is cognitive architecture?\n")
(display "Answer: ")
(display (caichat-ask "What is cognitive architecture in 2 sentences?"))
(newline)
(newline)

;; Example 2: Configuration
(display "Example 2: Configuration\n")
(caichat-config-set! 'default-provider "openai")
(display (format #f "Default provider: ~a\n" 
                (caichat-config-get 'default-provider)))
(newline)

;; Example 3: RAG system
(display "Example 3: RAG system\n")
(caichat-rag-create-kb "ai-research" "AI research knowledge base")
(caichat-rag-add-doc "ai-research" "paper1" 
                     "Cognitive architecture is a framework for understanding intelligence. It provides a blueprint for building AI systems." 
                     '(("author" . "Smith") ("year" . "2024")))
(caichat-rag-add-doc "ai-research" "paper2" 
                     "OpenCog is an open-source cognitive architecture platform. It uses AtomSpace for knowledge representation." 
                     '(("author" . "Goertzel") ("year" . "2023")))

(display "RAG Query: What is OpenCog?\n")
(display "Answer: ")
(display (caichat-rag-query "ai-research" "What is OpenCog?"))
(newline)
(newline)

;; Example 4: Interactive REPL (commented out for non-interactive execution)
;; (display "Starting interactive REPL...\n")
;; (caichat-repl-start)

(display "Examples completed successfully!\n")