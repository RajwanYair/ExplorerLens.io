#==============================================================================
# ExplorerLens - Root CMake Configuration
# Copyright (c) 2026 - ExplorerLens.io Project
# Version: 28.4.0 "Polaris-U"
#
# vcpkg Integration (Optional):
#   cmake -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
#   cmake --build build --config Release
#==============================================================================

cmake_minimum_required(VERSION 3.20)

project(ExplorerLens
    VERSION 28.4.0
    DESCRIPTION "ExplorerLens.io - Universal File Preview Engine for Windows Explorer"
    LANGUAGES CXX
    HOMEPAGE_URL "https://github.com/ExplorerLens/ExplorerLens.io"
)

#==============================================================================
# Architecture Guard (x64-only)
#==============================================================================

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "ExplorerLens is x64-only. Reconfigure with a 64-bit generator/platform.")
endif()

if(CMAKE_GENERATOR MATCHES "Visual Studio" AND DEFINED CMAKE_GENERATOR_PLATFORM)
    if(NOT CMAKE_GENERATOR_PLATFORM STREQUAL "x64")
        message(FATAL_ERROR "ExplorerLens requires Visual Studio platform x64. Current: ${CMAKE_GENERATOR_PLATFORM}")
    endif()
endif()

#==============================================================================
# Project-wide Options
#==============================================================================

option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_BENCHMARKS "Build performance benchmarks" ON)
option(BUILD_ENGINE_ONLY "Build only the engine library" OFF)
option(BUILD_MODERN_RUNTIME "Build worker/shellhost/local-api runtime components" ON)
option(ENABLE_GPU "Enable GPU acceleration (DirectX 11)" ON)
option(ENABLE_CACHE "Enable thumbnail caching" ON)

#==============================================================================
# CMake Settings
#==============================================================================

set(CMAKE_CXX_STANDARD 20)  # C++20 for latest features (modules, concepts, coroutines)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Use /MD runtime (Multi-threaded DLL) to match existing project
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# Target modern Windows 11 (10.0.22000+)
# Note: WINVER/_WIN32_WINNT set per-target (Engine, Shell, Manager)
# to avoid conflicts with per-component stdafx.h definitions

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

# Report detected compiler
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ Compiler Path: ${CMAKE_CXX_COMPILER}")
if(MSVC)
    message(STATUS "MSVC Toolset: ${CMAKE_VS_PLATFORM_TOOLSET}")
    message(STATUS "Windows SDK: ${CMAKE_SYSTEM_VERSION}")
endif()

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
        /IGNORE:4099                      # Ignore missing PDB warnings
    )

    # Profile-Guided Optimization support
    # Usage:
    #   Phase 1 (Instrument):  cmake -B build -DPGO_PHASE=INSTRUMENT && cmake --build build --config Release
    #   Phase 2 (Train):       Run the instrumented binary with representative workloads
    #   Phase 3 (Optimize):    cmake -B build -DPGO_PHASE=OPTIMIZE && cmake --build build --config Release
    #   Disabled:              cmake -B build -DPGO_PHASE=OFF (default — uses standard LTCG)
    set(PGO_PHASE "OFF" CACHE STRING "PGO phase: OFF, INSTRUMENT, or OPTIMIZE")
    set_property(CACHE PGO_PHASE PROPERTY STRINGS OFF INSTRUMENT OPTIMIZE)

    if(PGO_PHASE STREQUAL "INSTRUMENT")
        message(STATUS "PGO Phase: INSTRUMENT — building with /GENPROFILE for profiling")
        add_compile_options($<$<CONFIG:Release>:/GL>)
        add_link_options($<$<CONFIG:Release>:/LTCG> $<$<CONFIG:Release>:/GENPROFILE>)
    elseif(PGO_PHASE STREQUAL "OPTIMIZE")
        message(STATUS "PGO Phase: OPTIMIZE — building with /USEPROFILE from collected data")
        add_compile_options($<$<CONFIG:Release>:/GL>)
        add_link_options($<$<CONFIG:Release>:/LTCG> $<$<CONFIG:Release>:/USEPROFILE>)
    else()
        message(STATUS "PGO Phase: OFF — using standard LTCG optimization")
        # /GL and /LTCG already set in Release flags above
    endif()
endif()

#==============================================================================
# External Library Paths
#==============================================================================

set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)
set(COMPRESSION_DIR ${EXTERNAL_DIR}/compression-libs)
set(IMAGE_LIBS_DIR ${EXTERNAL_DIR}/image-libs)

# Find external libraries
set(ZLIB_DIR ${COMPRESSION_DIR}/zlib-1.3.1)
set(LZ4_DIR ${COMPRESSION_DIR}/lz4-1.10.0)
set(ZSTD_DIR ${COMPRESSION_DIR}/zstd-1.5.7)
set(LZMA_DIR ${COMPRESSION_DIR}/xz-5.6.3)
set(WEBP_DIR ${IMAGE_LIBS_DIR}/libwebp-1.5.0-build)
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

    # Cross-subdirectory link: allow EngineTests to unit-test the CLI layer.
    # LensCLILib is defined in src/Tools.CLI after add_subdirectory(src) above,
    # so adding it here (after both Engine + src are processed) is safe.
    target_link_libraries(EngineTests PRIVATE LensCLILib)
    target_include_directories(EngineTests PRIVATE ${CMAKE_SOURCE_DIR}/src/Tools.CLI)
endif()

if(NOT BUILD_ENGINE_ONLY)
    # Build the shell extension and manager via unified CMake
    # OFF by default — LENSShell has pre-existing source issues under /W4 /permissive-
    # Enable with: cmake -DBUILD_SHELL=ON -DBUILD_MANAGER=ON
    option(BUILD_SHELL "Build LENSShell COM DLL (requires ATL)" OFF)
    option(BUILD_MANAGER "Build LENSManager GUI app (requires WTL)" OFF)

    if(BUILD_SHELL)
        add_subdirectory(LENSShell)
    endif()

    if(BUILD_MANAGER)
        add_subdirectory(LENSManager)
    endif()

    message(STATUS "Full project build enabled")
    message(STATUS "  - Engine library: ON")
    message(STATUS "  - Shell extension: ${BUILD_SHELL}")
    message(STATUS "  - Manager app: ${BUILD_MANAGER}")
endif()

#==============================================================================
# Installation Rules
#==============================================================================

if(NOT BUILD_ENGINE_ONLY)
    install(TARGETS ExplorerLensEngine
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
    )

    install(DIRECTORY Engine/Core Engine/Decoders Engine/Pipeline Engine/GPU Engine/Cache Engine/Utils
        DESTINATION include/ExplorerLens
        FILES_MATCHING PATTERN "*.h"
    )

    install(FILES Engine/Engine.h
        DESTINATION include/ExplorerLens
    )
endif()

#==============================================================================
# Plugin SDK Installation (cmake --install build --component SDK)
#==============================================================================

install(FILES SDK/plugin_api.h
    DESTINATION sdk/include
    COMPONENT SDK
)

install(FILES SDK/include/ExplorerLensPlugin.h
    DESTINATION sdk/include
    COMPONENT SDK
)

install(DIRECTORY SDK/docs/
    DESTINATION sdk/docs
    COMPONENT SDK
)

install(DIRECTORY SDK/examples/
    DESTINATION sdk/examples
    COMPONENT SDK
)

install(FILES SDK/README.md
    DESTINATION sdk
    COMPONENT SDK
)

#==============================================================================
# CTest Integration
#==============================================================================

if(BUILD_TESTS)
    enable_testing()

    # Smoke-test: lens --help must exit with code 0
    add_test(NAME CLIHelp
        COMMAND lens --help
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set_tests_properties(CLIHelp PROPERTIES TIMEOUT 10)

    # Smoke-test: lens info on a non-existent path must exit with non-zero (graceful error)
    add_test(NAME CLIInfoMissingFile
        COMMAND lens info nonexistent_file_for_test.xyz
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set_tests_properties(CLIInfoMissingFile PROPERTIES
        TIMEOUT 10
        WILL_FAIL TRUE)

    # Smoke-test: lens doctor (no admin needed — just verifies system checks run)
    add_test(NAME CLIDoctor
        COMMAND lens doctor
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set_tests_properties(CLIDoctor PROPERTIES
        TIMEOUT 15
        SKIP_REGULAR_EXPRESSION "not found adjacent to lens\.exe")
endif()

#==============================================================================
# Package Configuration
#==============================================================================

set(CPACK_PACKAGE_NAME "ExplorerLens.io")
set(CPACK_PACKAGE_VENDOR "ExplorerLens.io Project")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Advanced Thumbnail Generator for Windows")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "ExplorerLens.io")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

# Windows-specific packaging
if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "ExplorerLens ${PROJECT_VERSION}")
    set(CPACK_NSIS_PACKAGE_NAME "ExplorerLens")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
endif()

include(CPack)

#==============================================================================
# Build Summary
#==============================================================================

message(STATUS "")
message(STATUS "========================================")
message(STATUS "ExplorerLens Build Configuration")
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
