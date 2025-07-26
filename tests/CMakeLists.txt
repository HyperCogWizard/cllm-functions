# Test executables

# Test for GGML integration
add_executable(test_ggml_integration test_ggml_integration.cpp)
target_include_directories(test_ggml_integration PRIVATE 
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/opencog
)
target_link_libraries(test_ggml_integration caichat)

# Add test target
enable_testing()
add_test(NAME GGMLIntegrationTest COMMAND test_ggml_integration)

# Install tests
install(TARGETS test_ggml_integration
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/tests)