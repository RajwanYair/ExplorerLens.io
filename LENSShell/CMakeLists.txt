#==============================================================================
# LENSShell — Shell Extension DLL (IThumbnailProvider COM Component)
# COM CLSID: 9E6ECB90-5A61-42BD-B851-D3297D9C7F39
#
# This CMakeLists.txt mirrors the MSBuild LENSShell.vcxproj configuration
# for unified CMake build system integration.
#==============================================================================

cmake_minimum_required(VERSION 3.20)
project(LENSShell LANGUAGES CXX)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "LENSShell is x64-only. Reconfigure with a 64-bit generator/platform.")
endif()

#==============================================================================
# Source Files
#==============================================================================

# Active sources (compiled in build)
set(SOURCES
    LENSShell.cpp
    LENSShellClass.cpp
    thumbnail_cache.cpp
    thumbnail_collage.cpp
    gpu_accelerator.cpp
    EngineAdapter.cpp
    PropertyStoreImpl.cpp
    LENSShellContextMenu.cpp
    libarchive_wrapper.cpp
    StdAfx.cpp
    unzip_new.cpp
)

# Legacy decoders — excluded from build since v7.0
# All decoding now handled via Engine/Decoders/ pipeline exclusively.
# Kept for reference; set LENSSHELL_LEGACY_DECODERS=ON to re-enable.
set(LEGACY_SOURCES
    video_thumbnail.cpp
    pdf_decoder.cpp
    audio_thumbnail.cpp
    document_thumbnail.cpp
    font_preview.cpp
)

option(LENSSHELL_LEGACY_DECODERS "Re-enable legacy v5.0-era decoders (not recommended)" OFF)
if(LENSSHELL_LEGACY_DECODERS)
    list(APPEND SOURCES ${LEGACY_SOURCES})
    message(WARNING "Legacy decoders enabled — prefer Engine/Decoders/ pipeline")
endif()

set(HEADERS
    # Core COM/Shell headers
    LENSArchive.h
    LENSShellClass.h
    LENSShell.h
    LENSTypes.h
    Resource.h
    StdAfx.h
    targetver.h

    # Legacy decoder headers (kept for interface references)
    svg_decoder.h
    video_thumbnail.h
    pdf_decoder.h
    audio_thumbnail.h
    document_thumbnail.h
    font_preview.h

    # Performance & Caching (v5.1.0)
    thumbnail_cache.h
    thumbnail_collage.h
    enhanced_cache.h

    # GPU Acceleration (v5.2.0)
    gpu_accelerator.h
    texture_pool.h

    # Engine Integration (v5.3.0)
    EngineAdapter.h

    # Property Store for Explorer Details Pane (v15.0.0)
    PropertyStoreImpl.h

    # Win11 Context Menu (v15.0.0)
    LENSShellContextMenu.h

    # External Library Wrappers
    libarchive_wrapper.h
    unrar.h
    unzip.h
    RarWrapper.h
    ZipWrapper.h

    # Infrastructure
    error_logger.h
    performance_profiler.h
    memory_utils.h
    metrics_collector.h

    # Helpers
    DarkModeHelper.h
    GdiplusRAII.h
    ImageTypes.h
    ModernCppHelper.h
)

set(RESOURCES
    LENSShell.rc
)

set(DEF_FILE
    LENSShell.def
)

set(IDL_FILE
    LENSShell.idl
)

#==============================================================================
# DLL Target
#==============================================================================

add_library(LENSShell SHARED
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
    ${DEF_FILE}
)

#==============================================================================
# Precompiled Header (StdAfx.h)
#==============================================================================

target_precompile_headers(LENSShell PRIVATE StdAfx.h)

#==============================================================================
# External Library Paths
#==============================================================================

set(ROOT_DIR ${CMAKE_SOURCE_DIR})
set(EXT_DIR ${ROOT_DIR}/external)
set(COMP_DIR ${EXT_DIR}/compression-libs)
set(IMG_DIR ${EXT_DIR}/image-libs)
set(CAM_DIR ${EXT_DIR}/camera-libs)
set(ARC_DIR ${EXT_DIR}/archive-libs)

# WTL include path (NuGet package or external)
set(WTL_DIR ${ROOT_DIR}/packages/wtl.10.0.10320/lib/native/include)
if(NOT EXISTS ${WTL_DIR})
    set(WTL_DIR ${ROOT_DIR}/external/ui-libs/wtl)
endif()

#==============================================================================
# Include Directories (mirrors vcxproj Release AdditionalIncludeDirectories)
#==============================================================================

target_include_directories(LENSShell PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ROOT_DIR}/Engine
    ${WTL_DIR}

    # Compression libs
    ${COMP_DIR}/zlib-1.3.1
    ${COMP_DIR}/zstd-1.5.7/lib
    ${COMP_DIR}/lz4-1.10.0/lib
    ${COMP_DIR}/bzip2-1.0.8
    ${COMP_DIR}/lzma-26.00/C
    ${COMP_DIR}/minizip-ng-4.0.10

    # Archive libs
    ${ARC_DIR}/unrar-7.2.2
    ${COMP_DIR}/libarchive-3.7.6/libarchive

    # Image libs
    ${IMG_DIR}/libwebp-1.5.0-original/src
    ${IMG_DIR}/libjxl-0.11.1/build-msvc/lib/include
    ${IMG_DIR}/libavif-1.3.0/include
    ${IMG_DIR}/dav1d-1.5.1/include

    # Camera libs
    ${CAM_DIR}/libraw-install/include
)

#==============================================================================
# Preprocessor Definitions (mirrors vcxproj Release config)
#==============================================================================

target_compile_definitions(LENSShell PRIVATE
    _WINDOWS
    _USRDLL
    ENGINE_STATIC
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>

    # Format support flags
    ENABLE_WEBP_SUPPORT
    ENABLE_HEIF_SUPPORT
    ENABLE_AVIF_SUPPORT
    ENABLE_JXL_SUPPORT
    ENABLE_PDF_SUPPORT
    ENABLE_VIDEO_SUPPORT
    ENABLE_AUDIO_SUPPORT
    ENABLE_DOCUMENT_SUPPORT
    ENABLE_FONT_SUPPORT
    ENABLE_SVG_SUPPORT
    ENABLE_RAW_SUPPORT
    ENABLE_PSD_SUPPORT
    ENABLE_DDS_SUPPORT
    ENABLE_HDR_SUPPORT
    ENABLE_PPM_SUPPORT
    ENABLE_EXR_SUPPORT

    # Library integration flags
    ENABLE_LIBARCHIVE_SUPPORT
    LIBARCHIVE_STATIC
    UNRAR
    SILENT
)

#==============================================================================
# Library Search Paths (mirrors vcxproj Release AdditionalLibraryDirectories)
#==============================================================================

target_link_directories(LENSShell PRIVATE
    # Engine / build output
    ${CMAKE_BINARY_DIR}/lib
    ${CMAKE_BINARY_DIR}/lib/$<CONFIG>

    # Compression libs
    ${COMP_DIR}/zlib-1.3.1/install/lib
    ${COMP_DIR}/lz4-1.10.0/build-vs/Release
    ${COMP_DIR}/zstd-1.5.7/build-manual
    ${COMP_DIR}/minizip-ng-4.0.10/build-manual
    ${COMP_DIR}/libarchive-3.7.6/install/lib
    ${COMP_DIR}/libarchive-3.7.6/build-vs/libarchive/Release

    # Image libs
    ${IMG_DIR}/libwebp-1.5.0-original/build-vs/Release
    ${IMG_DIR}/libjxl-0.11.1/install/lib
    ${IMG_DIR}/libheif-1.19.5/install/lib
    ${IMG_DIR}/libheif-1.19.5/build-vs/libheif/Release
    ${IMG_DIR}/libde265-1.0.15/build-vs/libde265/Release
    ${IMG_DIR}/libde265-1.0.15/install/lib

    # Camera libs
    ${CAM_DIR}/libraw-install/lib
)

#==============================================================================
# Link Libraries (mirrors vcxproj Release AdditionalDependencies)
#==============================================================================

target_link_libraries(LENSShell PRIVATE
    # Engine library (CMake target from add_subdirectory(Engine))
    ExplorerLensEngine

    # Compression/Archive libs (statically linked)
    zlibstatic
    liblz4_static
    zstd_static
    minizip
    archive_static

    # Image format libs
    webp
    sharpyuv
    libraw_static
    jxl
    jxl_cms
    jxl_threads
    brotlidec
    brotlicommon
    hwy
    heif
    de265

    # UnRAR (pre-built DLL + import lib)
    ${CMAKE_CURRENT_SOURCE_DIR}/unrar64.lib

    # DirectX / GPU
    d3d11
    dxgi
    d3dcompiler

    # Windows System
    shlwapi
    ole32
    oleaut32
    uuid
    comctl32
    gdi32
    gdiplus
    windowscodecs
    d2d1
    propsys
    bcrypt
    delayimp
    advapi32
    user32
    shell32
)

#==============================================================================
# Linker Options (mirrors vcxproj Release Link settings)
#==============================================================================

if(MSVC)
    target_link_options(LENSShell PRIVATE
        /DEF:"${CMAKE_CURRENT_SOURCE_DIR}/LENSShell.def"
        /MANIFEST:EMBED
        "/MANIFESTINPUT:${CMAKE_CURRENT_SOURCE_DIR}/LENSShell.manifest"
        /NODEFAULTLIB:LIBCMT
        /SAFESEH:NO
    )
endif()

#==============================================================================
# MIDL: Generate Type Library from IDL
#==============================================================================

if(MSVC)
    add_custom_command(
        OUTPUT
            ${CMAKE_CURRENT_BINARY_DIR}/LENSShell.tlb
            ${CMAKE_CURRENT_BINARY_DIR}/LENSShell_i.c
        COMMAND midl /nologo /char signed /env x64 /Oicf
                /h "${CMAKE_CURRENT_BINARY_DIR}/LENSShell_generated.h"
                /iid "${CMAKE_CURRENT_BINARY_DIR}/LENSShell_i.c"
                /tlb "${CMAKE_CURRENT_BINARY_DIR}/LENSShell.tlb"
                /target "NT60"
                "${CMAKE_CURRENT_SOURCE_DIR}/${IDL_FILE}"
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${IDL_FILE}
        COMMENT "MIDL: Generating type library from ${IDL_FILE}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_custom_target(LENSShell_TLB DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/LENSShell.tlb
        ${CMAKE_CURRENT_BINARY_DIR}/LENSShell_i.c
    )
    add_dependencies(LENSShell LENSShell_TLB)
endif()

#==============================================================================
# Post-Build: Copy Runtime DLLs
#==============================================================================

# Copy UnRAR64.dll to output directory
add_custom_command(TARGET LENSShell POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/UnRAR64.dll"
        "$<TARGET_FILE_DIR:LENSShell>"
    COMMENT "Copying UnRAR64.dll to output directory"
)

# Copy libde265.dll if it exists (required at runtime for HEIF decoding)
add_custom_command(TARGET LENSShell POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E true
    COMMAND $<$<BOOL:$<TARGET_EXISTS:LENSShell>>:${CMAKE_COMMAND}> -E copy_if_different
        "${IMG_DIR}/libde265-1.0.15/build-vs/libde265/Release/libde265.dll"
        "$<TARGET_FILE_DIR:LENSShell>"
    COMMENT "Copying libde265.dll to output directory (if available)"
    COMMAND_EXPAND_LISTS
)

#==============================================================================
# Installation
#==============================================================================

install(TARGETS LENSShell
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/UnRAR64.dll
    DESTINATION bin
)
