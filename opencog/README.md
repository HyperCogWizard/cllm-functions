# CAIChat OpenCog Module

This directory contains the OpenCog implementation of CAIChat, transforming the original LLM Functions tools into a cognitive architecture component.

## Structure

- `caichat/` - C++ core library
  - `LLMClient.h/cc` - LLM provider interfaces and implementations
  - `ChatCompletion.h/cc` - Conversation management
  - `SchemeBindings.cc` - Guile Scheme bindings

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Dependencies

- CMake 3.10+
- C++14 compiler
- Guile 3.0 (or 2.2)
- libcurl
- jsoncpp

## Usage

After building and installing, the module can be used in Guile:

```scheme
(use-modules (opencog caichat init))
(caichat-ask "Hello, how are you?")
```

See `../example.scm` for more comprehensive examples.