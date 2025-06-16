;; Main CAIChat module
(define-module (opencog caichat init)
  #:use-module (system foreign)
  #:export (caichat-ask
            caichat-repl
            caichat-ask-about-atom
            caichat-create-knowledge-base
            caichat-add-document
            caichat-rag-query
            caichat-setup-openai
            caichat-setup-claude
            caichat-save-config
            caichat-load-config))

;; Load the C++ bindings
(eval-when (load eval compile)
  (load-extension "libcaichat" "init_caichat_bindings"))

;; Default provider
(define *default-provider* "openai")

;; Simple ask function with default provider
(define (caichat-ask message)
  "Ask a question using the default LLM provider"
  (caichat-ask *default-provider* message))

;; Interactive REPL
(define (caichat-repl)
  "Start an interactive chat session"
  (let ((session-id (caichat-create-client *default-provider* "")))
    (display "CAIChat OpenCog Interface\n")
    (display "Type 'quit' or 'exit' to end the session\n\n")
    (let loop ()
      (display "You: ")
      (let ((input (read-line)))
        (cond
         ((or (string=? input "quit") (string=? input "exit"))
          (display "Goodbye!\n"))
         ((string=? input "")
          (loop))
         (else
          (display "Assistant: ")
          (display (caichat-send-message session-id input))
          (newline)
          (newline)
          (loop)))))))

;; Work with AtomSpace atoms (placeholder for now)
(define (caichat-ask-about-atom atom question)
  "Ask a question about a specific atom"
  (let ((atom-str (format #f "~a" atom)))
    (caichat-ask (string-append "Regarding " atom-str ": " question))))

;; RAG system (placeholder implementations)
(define (caichat-create-knowledge-base name description)
  "Create a knowledge base for RAG"
  (display (format #f "Created knowledge base: ~a (~a)\n" name description))
  #t)

(define (caichat-add-document doc-id content metadata)
  "Add a document to the knowledge base"
  (display (format #f "Added document: ~a\n" doc-id))
  #t)

(define (caichat-rag-query query)
  "Query the knowledge base with RAG"
  (caichat-ask (string-append "Using available knowledge: " query)))

;; Configuration management
(define (caichat-setup-openai api-key)
  "Set up OpenAI provider with API key"
  (setenv "OPENAI_API_KEY" api-key)
  (set! *default-provider* "openai")
  #t)

(define (caichat-setup-claude api-key)
  "Set up Claude provider with API key"
  (setenv "ANTHROPIC_API_KEY" api-key)
  (set! *default-provider* "claude")
  #t)

(define (caichat-save-config filename)
  "Save configuration to file"
  (with-output-to-file filename
    (lambda ()
      (write `((default-provider . ,*default-provider*)))))
  #t)

(define (caichat-load-config filename)
  "Load configuration from file"
  (when (file-exists? filename)
    (let ((config (with-input-from-file filename read)))
      (let ((provider (assoc-ref config 'default-provider)))
        (when provider
          (set! *default-provider* provider)))))
  #t)