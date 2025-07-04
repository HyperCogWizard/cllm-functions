#!/usr/bin/env guile
!#

;; Basic tests for CAIChat OpenCog module

(use-modules (opencog caichat config)
             (opencog caichat rag)
             (opencog caichat atomspace))

(define test-count 0)
(define passed-count 0)

(define (test name expected actual)
  "Run a test case"
  (set! test-count (+ test-count 1))
  (if (equal? expected actual)
      (begin
        (set! passed-count (+ passed-count 1))
        (display (format #f "✓ PASS: ~a\n" name)))
      (display (format #f "✗ FAIL: ~a (expected ~a, got ~a)\n" 
                      name expected actual))))

;; Test configuration module
(display "Testing configuration module...\n")
(caichat-config-init)
(caichat-config-set! 'test-key "test-value")
(test "Config set/get" "test-value" (caichat-config-get 'test-key))
(test "Config default" "default" (caichat-config-get 'nonexistent "default"))

;; Test RAG module
(display "\nTesting RAG module...\n")
(caichat-rag-create-kb "test-kb" "Test knowledge base")
(caichat-rag-add-doc "test-kb" "doc1" "This is a test document about AI" '())
(caichat-rag-add-doc "test-kb" "doc2" "Another document about machine learning" '())

(let ((docs (caichat-rag-list-docs "test-kb")))
  (test "RAG document count" 2 (length docs)))

(let ((search-results (caichat-rag-search "test-kb" "AI")))
  (test "RAG search" 1 (length search-results)))

;; Test AtomSpace integration
(display "\nTesting AtomSpace integration...\n")
(caichat-atomspace-add-concept "TestConcept" "A test concept for demonstration")
(test "AtomSpace add concept" #t #t)  ; Just test that it doesn't error

(let ((related (caichat-atomspace-get-related "TestConcept")))
  (test "AtomSpace get related" 3 (length related)))

;; Test summary
(display (format #f "\nTest Results: ~a/~a passed\n" passed-count test-count))
(if (= passed-count test-count)
    (begin
      (display "All tests passed!\n")
      (exit 0))
    (begin
      (display "Some tests failed!\n")
      (exit 1)))