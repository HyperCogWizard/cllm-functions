;; Configuration management module
(define-module (opencog caichat config)
  #:export (caichat-config-set!
            caichat-config-get
            caichat-config-load
            caichat-config-save
            caichat-config-init))

;; Global configuration storage
(define *caichat-config* '())

;; Set configuration value
(define (caichat-config-set! key value)
  "Set a configuration value"
  (set! *caichat-config* 
        (assoc-set! *caichat-config* key value)))

;; Get configuration value
(define (caichat-config-get key . default)
  "Get a configuration value with optional default"
  (let ((value (assoc-ref *caichat-config* key)))
    (if value
        value
        (if (null? default) #f (car default)))))

;; Load configuration from file
(define (caichat-config-load filename)
  "Load configuration from a Scheme file"
  (when (file-exists? filename)
    (set! *caichat-config* (with-input-from-file filename read))))

;; Save configuration to file
(define (caichat-config-save filename)
  "Save configuration to a Scheme file"
  (with-output-to-file filename
    (lambda ()
      (write *caichat-config*))))

;; Initialize default configuration
(define (caichat-config-init)
  "Initialize with default configuration values"
  (caichat-config-set! 'default-provider "openai")
  (caichat-config-set! 'default-model "gpt-3.5-turbo")
  (caichat-config-set! 'max-tokens 1000)
  (caichat-config-set! 'temperature 0.7)
  
  ;; Load from environment variables if available
  (let ((openai-key (getenv "OPENAI_API_KEY"))
        (claude-key (getenv "ANTHROPIC_API_KEY")))
    (when openai-key
      (caichat-config-set! 'openai-api-key openai-key))
    (when claude-key
      (caichat-config-set! 'claude-api-key claude-key))))