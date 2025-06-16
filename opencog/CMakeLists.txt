# C++ Core Library

add_library(caichat SHARED
    caichat/LLMClient.cc
    caichat/ChatCompletion.cc
    caichat/SchemeBindings.cc
)

# Include directories
target_include_directories(caichat PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GUILE_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(caichat
    ${GUILE_LIBRARIES}
    ${JSONCPP_LIBRARIES}
    ${CURL_LIBRARIES}
)

# Compile flags
target_compile_options(caichat PRIVATE ${GUILE_CFLAGS_OTHER})

# Install library
install(TARGETS caichat
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

# Install headers
install(FILES caichat/LLMClient.h caichat/ChatCompletion.h
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/opencog/caichat)