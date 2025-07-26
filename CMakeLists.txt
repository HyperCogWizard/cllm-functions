cmake_minimum_required(VERSION 3.10)
project(caichat-opencog VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)

# Find OpenCog (optional for now)
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(OPENCOG opencog)
endif()

# Find Guile
pkg_check_modules(GUILE guile-3.0)
if(NOT GUILE_FOUND)
    pkg_check_modules(GUILE guile-2.2)
endif()

if(GUILE_FOUND)
    message(STATUS "Found Guile: ${GUILE_LIBRARIES}")
    add_definitions(-DHAVE_GUILE)
    set(GUILE_AVAILABLE TRUE)
else()
    message(STATUS "Guile not found - Scheme bindings will be disabled")
    set(GUILE_AVAILABLE FALSE)
endif()

# Find other dependencies
find_package(CURL QUIET)
if(CURL_FOUND)
    # Check if curl headers are actually available
    include(CheckIncludeFile)
    set(CMAKE_REQUIRED_INCLUDES ${CURL_INCLUDE_DIRS})
    check_include_file("curl/curl.h" HAVE_CURL_HEADER)
    if(HAVE_CURL_HEADER)
        message(STATUS "Found CURL with headers: ${CURL_LIBRARIES}")
        add_definitions(-DHAVE_CURL)
        set(CURL_AVAILABLE TRUE)
    else()
        message(STATUS "CURL found but headers not available")
        set(CURL_AVAILABLE FALSE)
    endif()
else()
    message(STATUS "CURL not found - HTTP clients will be disabled")
    set(CURL_AVAILABLE FALSE)
endif()

pkg_check_modules(JSONCPP jsoncpp)
if(JSONCPP_FOUND)
    # Check if json headers are actually available
    include(CheckIncludeFile)
    set(CMAKE_REQUIRED_INCLUDES ${JSONCPP_INCLUDE_DIRS})
    check_include_file("json/json.h" HAVE_JSON_HEADER)
    if(HAVE_JSON_HEADER)
        message(STATUS "Found JsonCpp with headers: ${JSONCPP_LIBRARIES}")
        add_definitions(-DHAVE_JSONCPP)
        set(JSONCPP_AVAILABLE TRUE)
    else()
        message(STATUS "JsonCpp found but headers not available")
        set(JSONCPP_AVAILABLE FALSE)
    endif()
else()
    message(STATUS "JsonCpp not found - JSON parsing will be disabled")
    set(JSONCPP_AVAILABLE FALSE)
endif()

# Find GGML/llama.cpp (optional for local model support)
find_path(GGML_INCLUDE_DIR
    NAMES ggml.h llama.h
    PATHS 
        /usr/local/include
        /usr/include
        ${CMAKE_SOURCE_DIR}/third_party/llama.cpp
        ${CMAKE_SOURCE_DIR}/external/llama.cpp
        $ENV{GGML_DIR}/include
    PATH_SUFFIXES ggml llama.cpp
)

find_library(GGML_LIBRARY
    NAMES ggml llama
    PATHS
        /usr/local/lib
        /usr/lib
        ${CMAKE_SOURCE_DIR}/third_party/llama.cpp
        ${CMAKE_SOURCE_DIR}/external/llama.cpp
        $ENV{GGML_DIR}/lib
    PATH_SUFFIXES ggml llama.cpp
)

if(GGML_INCLUDE_DIR AND GGML_LIBRARY)
    set(GGML_FOUND TRUE)
    message(STATUS "Found GGML: ${GGML_LIBRARY}")
    add_definitions(-DHAVE_GGML)
else()
    set(GGML_FOUND FALSE)
    message(STATUS "GGML not found - GGML features will be disabled")
endif()

# Include directories
include_directories(${GUILE_INCLUDE_DIRS})
include_directories(${JSONCPP_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/opencog)

# Add subdirectories
add_subdirectory(opencog)
add_subdirectory(tests)

# Install Scheme modules (only if Guile is available)
if(GUILE_AVAILABLE)
    install(DIRECTORY scm/ DESTINATION ${CMAKE_INSTALL_PREFIX}/share/guile/site/3.0/opencog
            FILES_MATCHING PATTERN "*.scm")
endif()

# Configuration
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/caichat-config.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/caichat-config.cmake @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/caichat-config.cmake
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/caichat)