#==============================================================================
# DarkThumbs Engine Tests - CMake Configuration
#==============================================================================

cmake_minimum_required(VERSION 3.20)

project(EngineTests LANGUAGES CXX)

#==============================================================================
# Test Executable
#==============================================================================

add_executable(EngineTests
    EngineTests.cpp
    test_gpu.cpp
)

target_link_libraries(EngineTests
    PRIVATE
        DarkThumbsEngine
)

target_include_directories(EngineTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Windows-specific settings
if(WIN32)
    target_link_libraries(EngineTests
        PRIVATE
            shlwapi
    )
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(EngineTests PRIVATE
        /W4
    )
endif()

#==============================================================================
# Register Tests
#==============================================================================

add_test(
    NAME EngineUnitTests
    COMMAND EngineTests
)
