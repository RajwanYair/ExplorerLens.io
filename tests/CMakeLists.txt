cmake_minimum_required(VERSION 3.15)
project(DarkThumbsTests VERSION 1.0.0 LANGUAGES CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable testing
enable_testing()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/../CBXShell
    ${CMAKE_SOURCE_DIR}/../packages/wtl.10.0.10320/lib/native/include
)

# Find required libraries
find_library(SHLWAPI_LIB shlwapi)
find_library(OLE32_LIB ole32)

# Unit Tests executable
add_executable(UnitTests
    UnitTests.cpp
)

target_compile_definitions(UnitTests PRIVATE
    _UNICODE
    UNICODE
    WIN32_LEAN_AND_MEAN
)

target_compile_options(UnitTests PRIVATE
    /W3          # Warning level 3
    /EHsc        # Exception handling
    /Zi          # Debug information
)

target_link_libraries(UnitTests
    ${SHLWAPI_LIB}
)

# Integration Tests executable
add_executable(IntegrationTests
    IntegrationTests.cpp
)

target_compile_definitions(IntegrationTests PRIVATE
    _UNICODE
    UNICODE
    WIN32_LEAN_AND_MEAN
)

target_compile_options(IntegrationTests PRIVATE
    /W3
    /EHsc
    /Zi
)

target_link_libraries(IntegrationTests
    ${SHLWAPI_LIB}
    ${OLE32_LIB}
)

# Register tests with CTest
add_test(
    NAME UnitTests
    COMMAND UnitTests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_test(
    NAME IntegrationTests
    COMMAND IntegrationTests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Test properties
set_tests_properties(UnitTests PROPERTIES
    TIMEOUT 30
    LABELS "unit;core"
)

set_tests_properties(IntegrationTests PROPERTIES
    TIMEOUT 60
    LABELS "integration;system"
)

# Custom target to run all tests
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
    DEPENDS UnitTests IntegrationTests
    COMMENT "Running all DarkThumbs tests..."
)

# Installation (optional)
install(TARGETS UnitTests IntegrationTests
    RUNTIME DESTINATION bin/tests
)

# Print configuration
message(STATUS "DarkThumbs Test Suite Configuration")
message(STATUS "====================================")
message(STATUS "C++ Standard:     ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type:       ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler:         ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Shlwapi Library:  ${SHLWAPI_LIB}")
message(STATUS "Ole32 Library:    ${OLE32_LIB}")
message(STATUS "====================================")
