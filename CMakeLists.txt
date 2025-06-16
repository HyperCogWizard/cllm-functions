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
pkg_check_modules(GUILE REQUIRED guile-3.0)
if(NOT GUILE_FOUND)
    pkg_check_modules(GUILE REQUIRED guile-2.2)
endif()

# Find other dependencies
find_package(CURL REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# Include directories
include_directories(${GUILE_INCLUDE_DIRS})
include_directories(${JSONCPP_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/opencog)

# Add subdirectories
add_subdirectory(opencog)

# Install Scheme modules
install(DIRECTORY scm/ DESTINATION ${CMAKE_INSTALL_PREFIX}/share/guile/site/3.0/opencog
        FILES_MATCHING PATTERN "*.scm")

# Configuration
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/caichat-config.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/caichat-config.cmake @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/caichat-config.cmake
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/caichat)