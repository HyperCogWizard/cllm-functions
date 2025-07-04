;; REPL (Read-Eval-Print Loop) module
(define-module (opencog caichat repl)
  #:use-module (opencog caichat init)
  #:use-module (opencog caichat config)
  #:use-module (ice-9 rdelim)
  #:export (caichat-repl-start
            caichat-repl-with-system-message
            caichat-repl-session))

;; Enhanced REPL with configuration support
(define (caichat-repl-start . args)
  "Start an interactive REPL with optional provider"
  (let ((provider (if (null? args) 
                      (caichat-config-get 'default-provider "openai")
                      (car args))))
    (caichat-repl-session provider)))

;; REPL with custom system message
(define (caichat-repl-with-system-message system-message . args)
  "Start REPL with a custom system message"
  (let* ((provider (if (null? args) 
                       (caichat-config-get 'default-provider "openai")
                       (car args)))
         (session-id (caichat-create-client provider "")))
    (caichat-set-system-message session-id system-message)
    (caichat-repl-interactive session-id provider)))

;; Main REPL session handler
(define (caichat-repl-session provider)
  "Handle a REPL session with the specified provider"
  (let ((session-id (caichat-create-client provider "")))
    (caichat-repl-interactive session-id provider)))

;; Interactive REPL loop
(define (caichat-repl-interactive session-id provider)
  "Run the interactive REPL loop"
  (display (format #f "CAIChat OpenCog Interface (~a)\n" provider))
  (display "Commands:\n")
  (display "  /help    - Show this help\n")
  (display "  /clear   - Clear conversation history\n")
  (display "  /system <msg> - Set system message\n")
  (display "  /quit or /exit - End session\n\n")
  
  (let loop ()
    (display "You: ")
    (force-output)
    (let ((input (read-line)))
      (cond
       ;; Handle commands
       ((or (string=? input "/quit") (string=? input "/exit"))
        (display "Goodbye!\n"))
       
       ((string=? input "/help")
        (display "Available commands:\n")
        (display "  /help    - Show this help\n")
        (display "  /clear   - Clear conversation history\n")
        (display "  /system <msg> - Set system message\n")
        (display "  /quit or /exit - End session\n\n")
        (loop))
       
       ((string=? input "/clear")
        (caichat-clear-history session-id)
        (display "Conversation history cleared.\n\n")
        (loop))
       
       ((string-prefix? "/system " input)
        (let ((system-msg (substring input 8)))
          (caichat-set-system-message session-id system-msg)
          (display "System message set.\n\n")
          (loop)))
       
       ;; Empty input
       ((string=? input "")
        (loop))
       
       ;; Regular chat message
       (else
        (display "Assistant: ")
        (force-output)
        (catch 'caichat-error
          (lambda ()
            (let ((response (caichat-send-message session-id input)))
              (display response)
              (newline)
              (newline)))
          (lambda (key . args)
            (display "Error: ")
            (display (car args))
            (newline)
            (newline)))
        (loop))))))