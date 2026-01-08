#==============================================================================
# DarkThumbs Engine - CMake Build Configuration
# Copyright (c) 2026 - DarkThumbs Project
# Version: 5.3.0
#==============================================================================

cmake_minimum_required(VERSION 3.20)

project(DarkThumbsEngine
    VERSION 5.3.0
    DESCRIPTION "DarkThumbs Thumbnail Generation Engine"
    LANGUAGES CXX
)

#==============================================================================
# Project Options
#==============================================================================

option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_SHARED_LIBS "Build as shared library" OFF)
option(ENABLE_GPU "Enable GPU acceleration" ON)

#==============================================================================
# Compiler Settings
#==============================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Use /MD runtime (Multi-threaded DLL)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

#==============================================================================
# Source Files
#==============================================================================

set(ENGINE_HEADERS
    Engine.h
    Core/Types.h
    Core/IThumbnailDecoder.h
    Core/IFormatDetector.h
    Core/IGPURenderer.h
    Core/ICacheProvider.h
    
    # Pipeline
    Pipeline/ThumbnailPipeline.h
    Pipeline/DecoderRegistry.h
    Pipeline/FormatDetector.h
    
    # Decoders
    Decoders/ImageDecoder.h
    Decoders/WebPDecoder.h
    Decoders/AVIFDecoder.h
    Decoders/ArchiveDecoder.h
    Decoders/JXLDecoder.h
    Decoders/HEIFDecoder.h
    
    # GPU
    GPU/D3D11Renderer.h
    
    # Cache
    Cache/ThumbnailCache.h
)

set(ENGINE_SOURCES
    # Pipeline implementations
    Pipeline/ThumbnailPipeline.cpp
    Pipeline/DecoderRegistry.cpp
    Pipeline/FormatDetector.cpp
    
    # Decoders
    Decoders/ImageDecoder.cpp
    Decoders/WebPDecoder.cpp
    Decoders/AVIFDecoder.cpp
    Decoders/ArchiveDecoder.cpp
    Decoders/JXLDecoder.cpp
    Decoders/HEIFDecoder.cpp
    
    # GPU
    GPU/D3D11Renderer.cpp
    # GPU/CPURenderer.cpp  # Future: software fallback
    
    # Cache
    Cache/ThumbnailCache.cpp
)

#==============================================================================
# Engine Library Target
#==============================================================================

if(ENGINE_SOURCES)
    if(BUILD_SHARED_LIBS)
        add_library(DarkThumbsEngine SHARED ${ENGINE_SOURCES} ${ENGINE_HEADERS})
        target_compile_definitions(DarkThumbsEngine PRIVATE DARKTHUMBS_ENGINE_EXPORTS)
    else()
        add_library(DarkThumbsEngine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})
    endif()
    
    target_include_directories(DarkThumbsEngine
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libwebp-1.5.0/src
            ${CMAKE_CURRENT_SOURCE_DIR}/../external/compression/minizip-ng-4.0.10
    )
    
    # Windows-specific settings
    if(WIN32)
        target_compile_definitions(DarkThumbsEngine PRIVATE
            _WINDOWS
            WIN32_LEAN_AND_MEAN
            NOMINMAX
        )
        
        target_link_libraries(DarkThumbsEngine
            PRIVATE
                gdiplus
                d3d11
                dxgi
                windowscodecs
                shlwapi
                bcrypt          # For minizip-ng crypto
                crypt32         # For minizip-ng crypto
        )
        
        # Link libwebp
        target_link_directories(DarkThumbsEngine
            PUBLIC
                ${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libwebp-1.5.0/output/x64/Release/release-static/x64/lib
                ${CMAKE_CURRENT_SOURCE_DIR}/../external/compression/minizip-ng-4.0.10/build-manual
                ${CMAKE_CURRENT_SOURCE_DIR}/../external/compression/zlib-1.3.1/x64/Release
                ${CMAKE_CURRENT_SOURCE_DIR}/../external/compression/zstd-1.5.7/build-manual
        )
        
        target_link_libraries(DarkThumbsEngine
            PUBLIC
                libwebp
                libsharpyuv
                minizip
                zlibstatic
                zstd_static
        )
    endif()
    
    # Compiler warnings
    if(MSVC)
        target_compile_options(DarkThumbsEngine PRIVATE
            /W4           # Warning level 4
            /WX           # Warnings as errors
            /permissive-  # Standards conformance
        )
    endif()
else()
    # Interface library for headers-only during initial development
    add_library(DarkThumbsEngine INTERFACE)
    target_include_directories(DarkThumbsEngine
        INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )
endif()

#==============================================================================
# Unit Tests
#==============================================================================

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(Tests)
endif()

#==============================================================================
# Installation
#==============================================================================

install(TARGETS DarkThumbsEngine
    EXPORT DarkThumbsEngineTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES ${ENGINE_HEADERS}
    DESTINATION include/DarkThumbs/Engine
)

#==============================================================================
# Package Configuration (Commented out for now - not needed for development)
#==============================================================================

# include(CMakePackageConfigHelpers)
# 
# configure_package_config_file(
#     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/DarkThumbsEngineConfig.cmake.in"
#     "${CMAKE_CURRENT_BINARY_DIR}/DarkThumbsEngineConfig.cmake"
#     INSTALL_DESTINATION lib/cmake/DarkThumbsEngine
# )
# 
# write_basic_package_version_file(
#     "${CMAKE_CURRENT_BINARY_DIR}/DarkThumbsEngineConfigVersion.cmake"
#     VERSION ${PROJECT_VERSION}
#     COMPATIBILITY SameMajorVersion
# )
# 
# install(FILES
#     "${CMAKE_CURRENT_BINARY_DIR}/DarkThumbsEngineConfig.cmake"
#     "${CMAKE_CURRENT_BINARY_DIR}/DarkThumbsEngineConfigVersion.cmake"
#     DESTINATION lib/cmake/DarkThumbsEngine
# )

#==============================================================================
# Summary
#==============================================================================

message(STATUS "")
message(STATUS "DarkThumbs Engine Configuration:")
message(STATUS "  Version:           ${PROJECT_VERSION}")
message(STATUS "  Build Type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "  Shared Library:    ${BUILD_SHARED_LIBS}")
message(STATUS "  Build Tests:       ${BUILD_TESTS}")
message(STATUS "  GPU Acceleration:  ${ENABLE_GPU}")
message(STATUS "  C++ Standard:      ${CMAKE_CXX_STANDARD}")
message(STATUS "")
