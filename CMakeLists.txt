#==============================================================================
# DarkThumbs - Root CMake Configuration
# Copyright (c) 2026 - DarkThumbs Project
# Version: 6.0.0
#==============================================================================

cmake_minimum_required(VERSION 3.20)

project(DarkThumbs
    VERSION 6.0.0
    DESCRIPTION "DarkThumbs - Advanced Thumbnail Generator for Windows"
    LANGUAGES CXX
    HOMEPAGE_URL "https://github.com/yourusername/DarkThumbs"
)

#==============================================================================
# Project-wide Options
#==============================================================================

option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_BENCHMARKS "Build performance benchmarks" ON)
option(BUILD_ENGINE_ONLY "Build only the engine library" OFF)
option(BUILD_MODERN_RUNTIME "Build worker/shellhost/local-api runtime components" OFF)
option(ENABLE_GPU "Enable GPU acceleration (DirectX 11)" ON)
option(ENABLE_CACHE "Enable thumbnail caching" ON)

#==============================================================================
# CMake Settings
#==============================================================================

set(CMAKE_CXX_STANDARD 20)  # C++20 for latest features (modules, concepts, coroutines)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Use /MD runtime (Multi-threaded DLL) to match existing project
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# Target modern Windows 11 (10.0.22000+)
add_compile_definitions(
    WINVER=0x0A00           # Windows 10+
    _WIN32_WINNT=0x0A00     # Windows 10+
    NTDDI_VERSION=0x0A000000 # Windows 10
)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Per-configuration output directories to match VS structure
foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin/${CONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
endforeach()

#==============================================================================
# Compiler Warnings and Settings
#==============================================================================

if(MSVC)
    add_compile_options(
        /W4                 # Warning level 4
        /WX-                # Don't treat warnings as errors (for flexibility)
        /permissive-        # Standards conformance
        /Zc:__cplusplus     # Enable correct __cplusplus macro
        /Zc:inline          # Remove unreferenced functions/data
        /Zc:preprocessor    # Conforming preprocessor
        /MP                 # Multi-processor compilation
        /EHsc               # Exception handling model
        /GS-                # Disable buffer security checks for performance
        /fp:fast            # Fast floating point model
        /arch:AVX2          # Enable AVX2 instructions (modern CPUs)
        $<$<CONFIG:Release>:/O2>      # Maximum optimization
        $<$<CONFIG:Release>:/Oi>      # Enable intrinsic functions
        $<$<CONFIG:Release>:/Ot>      # Favor fast code
        $<$<CONFIG:Release>:/Oy>      # Omit frame pointers
        $<$<CONFIG:Release>:/GL>      # Whole program optimization
        $<$<CONFIG:Release>:/Gw>      # Optimize global data
        $<$<CONFIG:Release>:/GF>      # String pooling
        $<$<CONFIG:Release>:/Gy>      # Function-level linking
        $<$<CONFIG:Release>:/Qpar>    # Automatic parallelization
        $<$<CONFIG:Release>:/Zc:inline> # Remove unreferenced code
    )
    add_link_options(
        $<$<CONFIG:Release>:/LTCG>        # Link-time code generation
        $<$<CONFIG:Release>:/OPT:REF>     # Remove unreferenced functions
        $<$<CONFIG:Release>:/OPT:ICF>     # Identical COMDAT folding
        $<$<CONFIG:Release>:/INCREMENTAL:NO> # Disable incremental linking
        $<$<CONFIG:Release>:/RELEASE>     # Set checksum in header
        /DYNAMICBASE        # ASLR support
        /NXCOMPAT          # DEP support
        /HIGHENTROPYVA     # 64-bit ASLR
        /NODEFAULTLIB:LIBCMT              # Fix LIBCMT conflict
        /IGNORE:4099                      # Ignore missing PDB warnings
    )
    
    # Profile-Guided Optimization support (optional)
    option(ENABLE_PGO "Enable Profile-Guided Optimization" OFF)
    if(ENABLE_PGO)
        add_compile_options($<$<CONFIG:Release>:/GL>)
        add_link_options($<$<CONFIG:Release>:/LTCG:PGOptimize>)
    endif()
endif()

#==============================================================================
# External Library Paths
#==============================================================================

set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)
set(COMPRESSION_DIR ${EXTERNAL_DIR}/compression)
set(IMAGE_LIBS_DIR ${EXTERNAL_DIR}/image-libs)

# Find external libraries
set(ZLIB_DIR ${COMPRESSION_DIR}/zlib-1.3.1)
set(LZ4_DIR ${COMPRESSION_DIR}/lz4-1.10.0)
set(ZSTD_DIR ${COMPRESSION_DIR}/zstd-1.5.7)
set(LZMA_DIR ${COMPRESSION_DIR}/xz-5.6.3)
set(WEBP_DIR ${IMAGE_LIBS_DIR}/libwebp-1.5.0)
set(MINIZIP_DIR ${COMPRESSION_DIR}/minizip-ng-4.0.10)

# Windows SDK and DirectX
find_package(WindowsSDK QUIET)

#==============================================================================
# Build Components
#==============================================================================

# Always build the Engine
add_subdirectory(Engine)

if(BUILD_MODERN_RUNTIME)
    add_subdirectory(src)
endif()

if(NOT BUILD_ENGINE_ONLY)
    # Build the shell extension and manager
    # Note: These require additional COM setup and are primarily built via MSBuild
    # This CMake integration is for CI/CD and cross-platform build testing
    
    message(STATUS "Full project build enabled")
    message(STATUS "  - Engine library: ON")
    message(STATUS "  - Shell extension: Via MSBuild (COM dependency)")
    message(STATUS "  - Manager app: Via MSBuild (WTL dependency)")
endif()

#==============================================================================
# Installation Rules
#==============================================================================

if(NOT BUILD_ENGINE_ONLY)
    install(TARGETS DarkThumbsEngine
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
    )
    
    install(DIRECTORY Engine/Core Engine/Decoders Engine/Pipeline Engine/GPU Engine/Cache Engine/Utils
        DESTINATION include/DarkThumbs
        FILES_MATCHING PATTERN "*.h"
    )
    
    install(FILES Engine/Engine.h
        DESTINATION include/DarkThumbs
    )
endif()

#==============================================================================
# CTest Integration
#==============================================================================

if(BUILD_TESTS)
    enable_testing()
    
    # Engine tests are built in the Engine subdirectory
    add_test(NAME EngineUnitTests
        COMMAND $<TARGET_FILE:EngineTests>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    
    if(BUILD_BENCHMARKS)
        add_test(NAME EngineBenchmarks
            COMMAND $<TARGET_FILE:EngineBenchmark>
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        set_tests_properties(EngineBenchmarks PROPERTIES
            LABELS "benchmark"
            TIMEOUT 60
        )
    endif()
    
    set_tests_properties(EngineUnitTests PROPERTIES
        LABELS "unit"
        TIMEOUT 30
    )
endif()

#==============================================================================
# Package Configuration
#==============================================================================

set(CPACK_PACKAGE_NAME "DarkThumbs")
set(CPACK_PACKAGE_VENDOR "DarkThumbs Project")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Advanced Thumbnail Generator for Windows")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "DarkThumbs")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

# Windows-specific packaging
if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "DarkThumbs ${PROJECT_VERSION}")
    set(CPACK_NSIS_PACKAGE_NAME "DarkThumbs")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
endif()

include(CPack)

#==============================================================================
# Build Summary
#==============================================================================

message(STATUS "")
message(STATUS "========================================")
message(STATUS "DarkThumbs Build Configuration")
message(STATUS "========================================")
message(STATUS "Version:           ${PROJECT_VERSION}")
message(STATUS "Build type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard:      C++${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler:          ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "")
message(STATUS "Options:")
message(STATUS "  BUILD_TESTS:        ${BUILD_TESTS}")
message(STATUS "  BUILD_BENCHMARKS:   ${BUILD_BENCHMARKS}")
message(STATUS "  BUILD_ENGINE_ONLY:  ${BUILD_ENGINE_ONLY}")
message(STATUS "  BUILD_MODERN_RUNTIME: ${BUILD_MODERN_RUNTIME}")
message(STATUS "  ENABLE_GPU:         ${ENABLE_GPU}")
message(STATUS "  ENABLE_CACHE:       ${ENABLE_CACHE}")
message(STATUS "")
message(STATUS "Output directories:")
message(STATUS "  Runtime:  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "  Library:  ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
message(STATUS "  Archive:  ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
message(STATUS "========================================")
message(STATUS "")
