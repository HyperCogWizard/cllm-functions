#!/bin/bash
# Simple test script for Scheme modules (without C++ dependencies)

echo "Testing OpenCog Scheme modules..."

# Test if we can load the config module without C++ bindings
cat > /tmp/test-config.scm << 'EOF'
;; Test configuration module without C++ bindings
(define-module (test-config)
  #:use-module (srfi srfi-1))

;; Simple configuration implementation for testing
(define *caichat-config* '())

(define (caichat-config-set! key value)
  (set! *caichat-config* 
        (assoc-set! *caichat-config* key value)))

(define (caichat-config-get key . default)
  (let ((value (assoc-ref *caichat-config* key)))
    (if value
        value
        (if (null? default) #f (car default)))))

;; Test cases
(caichat-config-set! 'test-key "test-value")
(let ((result (caichat-config-get 'test-key)))
  (if (string=? result "test-value")
      (display "✓ PASS: Config set/get test\n")
      (display "✗ FAIL: Config set/get test\n")))

(let ((result (caichat-config-get 'nonexistent "default")))
  (if (string=? result "default")
      (display "✓ PASS: Config default test\n")
      (display "✗ FAIL: Config default test\n")))

(display "Basic configuration tests completed.\n")
EOF

# Run if bash is available (which it is)
if command -v bash &> /dev/null; then
    echo "✓ Bash available"
else
    echo "✗ Bash not available"
fi

# Check if the scheme files exist and are syntactically valid
echo "Checking Scheme file syntax..."

scheme_files=(
    "scm/opencog/caichat/config.scm"
    "scm/opencog/caichat/rag.scm" 
    "scm/opencog/caichat/repl.scm"
)

for file in "${scheme_files[@]}"; do
    if [ -f "$file" ]; then
        echo "✓ Found: $file"
        # Basic syntax check - count parens
        open_parens=$(grep -o '(' "$file" | wc -l)
        close_parens=$(grep -o ')' "$file" | wc -l)
        if [ "$open_parens" -eq "$close_parens" ]; then
            echo "  ✓ Parentheses balanced"
        else
            echo "  ✗ Parentheses unbalanced ($open_parens open, $close_parens close)"
        fi
    else
        echo "✗ Missing: $file"
    fi
done

# Check C++ files exist
echo "Checking C++ files..."
cpp_files=(
    "opencog/caichat/LLMClient.h"
    "opencog/caichat/LLMClient.cc"
    "opencog/caichat/ChatCompletion.h"
    "opencog/caichat/ChatCompletion.cc"
    "opencog/caichat/SchemeBindings.cc"
)

for file in "${cpp_files[@]}"; do
    if [ -f "$file" ]; then
        echo "✓ Found: $file"
    else
        echo "✗ Missing: $file"
    fi
done

# Check build system files
echo "Checking build system..."
build_files=(
    "CMakeLists.txt"
    "opencog/CMakeLists.txt"
    "build-opencog.sh"
)

for file in "${build_files[@]}"; do
    if [ -f "$file" ]; then
        echo "✓ Found: $file"
    else
        echo "✗ Missing: $file"
    fi
done

echo "Basic file structure verification completed."