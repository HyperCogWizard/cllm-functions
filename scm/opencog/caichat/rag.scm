;; RAG (Retrieval Augmented Generation) module
(define-module (opencog caichat rag)
  #:use-module (opencog caichat init)
  #:use-module (opencog caichat config)
  #:export (caichat-rag-create-kb
            caichat-rag-add-doc
            caichat-rag-query
            caichat-rag-search
            caichat-rag-list-docs))

;; Knowledge base storage (simple in-memory for now)
(define *knowledge-bases* '())

;; Document structure: (id content metadata embeddings)
(define (make-document id content metadata)
  "Create a document structure"
  (list id content metadata '()))

;; Create knowledge base
(define (caichat-rag-create-kb name description)
  "Create a new knowledge base"
  (let ((kb (list name description '())))  ; (name description documents)
    (set! *knowledge-bases* 
          (assoc-set! *knowledge-bases* name kb))
    (display (format #f "Created knowledge base: ~a\n" name))
    #t))

;; Add document to knowledge base
(define (caichat-rag-add-doc kb-name doc-id content metadata)
  "Add a document to the specified knowledge base"
  (let ((kb (assoc-ref *knowledge-bases* kb-name)))
    (if kb
        (let* ((documents (caddr kb))
               (new-doc (make-document doc-id content metadata))
               (updated-docs (cons new-doc documents))
               (updated-kb (list (car kb) (cadr kb) updated-docs)))
          (set! *knowledge-bases* 
                (assoc-set! *knowledge-bases* kb-name updated-kb))
          (display (format #f "Added document '~a' to knowledge base '~a'\n" 
                          doc-id kb-name))
          #t)
        (begin
          (display (format #f "Knowledge base '~a' not found\n" kb-name))
          #f))))

;; Simple text search (placeholder for semantic search)
(define (caichat-rag-search kb-name query)
  "Search for relevant documents in the knowledge base"
  (let ((kb (assoc-ref *knowledge-bases* kb-name)))
    (if kb
        (let ((documents (caddr kb)))
          (filter (lambda (doc)
                    (let ((content (cadr doc)))
                      (string-contains content query)))
                  documents))
        '())))

;; RAG query with context
(define (caichat-rag-query kb-name query . args)
  "Query the LLM with relevant context from the knowledge base"
  (let* ((provider (if (null? args) 
                       (caichat-config-get 'default-provider "openai")
                       (car args)))
         (relevant-docs (caichat-rag-search kb-name query))
         (context (if (null? relevant-docs)
                      ""
                      (string-join 
                       (map (lambda (doc)
                              (format #f "Document: ~a\nContent: ~a\n" 
                                     (car doc) (cadr doc)))
                            relevant-docs)
                       "\n")))
         (augmented-query (if (string=? context "")
                             query
                             (format #f "Context:\n~a\n\nQuestion: ~a" 
                                    context query))))
    (caichat-ask provider augmented-query)))

;; List documents in knowledge base
(define (caichat-rag-list-docs kb-name)
  "List all documents in the specified knowledge base"
  (let ((kb (assoc-ref *knowledge-bases* kb-name)))
    (if kb
        (let ((documents (caddr kb)))
          (map (lambda (doc)
                 (list (car doc)  ; doc-id
                       (length (cadr doc))  ; content length
                       (cadddr doc)))  ; metadata
               documents))
        '())))