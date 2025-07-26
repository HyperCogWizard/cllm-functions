;; GGML-specific module for local model support
(define-module (opencog caichat ggml)
  #:use-module (system foreign)
  #:use-module (ice-9 textual-ports)
  #:use-module (opencog caichat init)
  #:export (caichat-ggml-load-model
           caichat-ggml-generate
           caichat-ggml-model-info
           caichat-ggml-setup-default
           caichat-ggml-is-available
           caichat-ggml-unload-model))

;; Check if GGML support is available
(define (caichat-ggml-is-available)
  "Check if GGML support is compiled in"
  ;; Try to load a model with an invalid path to test availability
  (catch #t
    (lambda ()
      (caichat-ggml-load-model "/invalid/path/test.gguf" '())
      #f) ; If it doesn't throw an error about GGML not being available, it's available
    (lambda (key . args)
      (let ((error-msg (if (pair? args) (car args) "")))
        (not (string-contains error-msg "GGML support not compiled"))))))

;; Load a GGML model with parameters
(define (caichat-ggml-load-model path . params)
  "Load a GGML model from the specified path with optional parameters
   
   Parameters can be provided as an association list:
   '((n_ctx . 2048) (n_batch . 512) (n_gpu_layers . -1) ...)
   
   Returns a session ID on success, #f on failure"
  (let ((param-alist (if (null? params) '() (car params))))
    (caichat-ggml-load-model path param-alist)))

;; Generate text using GGML model
(define (caichat-ggml-generate session-id prompt . params)
  "Generate text using the loaded GGML model
   
   Parameters can be provided as an association list:
   '((n_predict . 128) (temp . 0.7) (top_p . 0.95) ...)
   
   Returns generated text"
  (let ((param-alist (if (null? params) '() (car params))))
    (caichat-ggml-generate session-id prompt param-alist)))

;; Get model information
(define (caichat-ggml-model-info session-id)
  "Get information about the currently loaded model
   
   Returns an association list with model details:
   ((path . \"/path/to/model.gguf\")
    (parameters . 7000000000)
    (vocab-size . 32000)
    (context-length . 2048)
    (memory-usage . 4294967296)
    (architecture . \"llama\"))"
  (caichat-ggml-model-info session-id))

;; Set up GGML as the default provider
(define (caichat-ggml-setup-default model-path . params)
  "Set up GGML as the default provider with the specified model
   
   This loads the model and configures it as the default for caichat-ask"
  (let* ((param-alist (if (null? params) '() (car params)))
         (session-id (caichat-ggml-load-model model-path param-alist)))
    (if session-id
        (begin
          (caichat-setup-ggml model-path)
          session-id)
        #f)))

;; Unload model (cleanup)
(define (caichat-ggml-unload-model session-id)
  "Unload the GGML model and free resources"
  ;; For now, we don't have a direct unload function
  ;; The model will be unloaded when the session is destroyed
  ;; This is a placeholder for future implementation
  #t)

;; Convenience functions for common model parameters
(define (caichat-ggml-default-params)
  "Get default GGML model parameters"
  '((n_ctx . 2048)
    (n_batch . 512)
    (n_gpu_layers . -1)
    (use_mmap . #t)
    (use_mlock . #f)))

(define (caichat-ggml-default-generation-params)
  "Get default text generation parameters"
  '((n_predict . 128)
    (temp . 0.7)
    (top_p . 0.95)
    (top_k . 40)
    (repeat_penalty . 1.1)))

;; High-level convenience function
(define (caichat-ggml-quick-setup model-path)
  "Quick setup with default parameters for testing"
  (caichat-ggml-setup-default model-path (caichat-ggml-default-params)))

;; Example usage function
(define (caichat-ggml-example)
  "Show example usage of GGML functions"
  (display "GGML Example Usage:\n\n")
  (display ";; Load a model\n")
  (display "(define session (caichat-ggml-load-model \"/path/to/model.gguf\"))\n\n")
  (display ";; Generate text\n")
  (display "(caichat-ggml-generate session \"Hello, world!\")\n\n")
  (display ";; Get model info\n")
  (display "(caichat-ggml-model-info session)\n\n")
  (display ";; Set as default\n")
  (display "(caichat-ggml-setup-default \"/path/to/model.gguf\")\n")
  (display "(caichat-ask \"What is AI?\")\n\n"))