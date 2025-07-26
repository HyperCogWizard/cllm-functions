# C++ Core Library

set(CORE_SOURCES
    caichat/LLMClient.cc
    caichat/ChatCompletion.cc
)

# Add Scheme bindings only if Guile is available
if(GUILE_AVAILABLE)
    list(APPEND CORE_SOURCES caichat/SchemeBindings.cc)
endif()

add_library(caichat SHARED ${CORE_SOURCES})

# Include directories
target_include_directories(caichat PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add Guile include directories if available
if(GUILE_AVAILABLE)
    target_include_directories(caichat PRIVATE ${GUILE_INCLUDE_DIRS})
endif()

# Add optional include directories
if(CURL_AVAILABLE)
    target_include_directories(caichat PRIVATE ${CURL_INCLUDE_DIRS})
endif()

if(JSONCPP_AVAILABLE)
    target_include_directories(caichat PRIVATE ${JSONCPP_INCLUDE_DIRS})
endif()

# Add GGML include directory if found
if(GGML_FOUND)
    target_include_directories(caichat PRIVATE ${GGML_INCLUDE_DIR})
endif()

# Link libraries - only link Guile if available
if(GUILE_AVAILABLE)
    target_link_libraries(caichat ${GUILE_LIBRARIES})
endif()

# Link optional libraries if available
if(CURL_AVAILABLE)
    target_link_libraries(caichat ${CURL_LIBRARIES})
endif()

if(JSONCPP_AVAILABLE)
    target_link_libraries(caichat ${JSONCPP_LIBRARIES})
endif()

# Link GGML library if found
if(GGML_FOUND)
    target_link_libraries(caichat ${GGML_LIBRARY})
endif()

# Compile flags
target_compile_options(caichat PRIVATE ${GUILE_CFLAGS_OTHER})

# Install library
install(TARGETS caichat
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

# Install headers
install(FILES caichat/LLMClient.h caichat/ChatCompletion.h
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/opencog/caichat)