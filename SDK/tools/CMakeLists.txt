#==============================================================================
# Plugin Tester Tool - CMake Build Configuration
#==============================================================================

cmake_minimum_required(VERSION 3.20)

project(PluginTester
    VERSION 1.0.0
    DESCRIPTION "DarkThumbs Plugin Testing Tool"
    LANGUAGES CXX
)

#==============================================================================
# Compiler Settings
#==============================================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#==============================================================================
# Executable
#==============================================================================

add_executable(plugin-tester plugin-tester.cpp)

target_include_directories(plugin-tester
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

if(WIN32)
    target_compile_definitions(plugin-tester PRIVATE
        _WINDOWS
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        UNICODE
        _UNICODE
    )
endif()

if(MSVC)
    target_compile_options(plugin-tester PRIVATE
        /W4
        /permissive-
    )
endif()

#==============================================================================
# Installation
#==============================================================================

install(TARGETS plugin-tester
    RUNTIME DESTINATION SDK/tools
)

message(STATUS "Plugin Tester configured")
