#==============================================================================
# DarkThumbs Engine - CMake Build Configuration
# Copyright (c) 2026 - DarkThumbs Project
# Version: 7.0.0
#==============================================================================

cmake_minimum_required(VERSION 3.20)

project(DarkThumbsEngine
    VERSION 7.0.0
    DESCRIPTION "DarkThumbs Thumbnail Generation Engine"
    LANGUAGES CXX
)

#==============================================================================
# Project Options
#==============================================================================

option(BUILD_TESTS "Build unit tests" ON)  # Enabled for CI/CD
option(BUILD_BENCHMARKS "Build performance benchmarks" ON)
option(BUILD_SHARED_LIBS "Build as shared library" OFF)
option(ENABLE_GPU "Enable GPU acceleration" ON)
option(ENABLE_CACHE "Enable thumbnail caching" ON)

# Optional decoder library flags
# Set these to ON when the corresponding libraries are built and available
option(HAS_LIBJXL "Enable JPEG XL support (requires libjxl)" ON)
option(HAS_LIBHEIF "Enable HEIF/HEIC support (requires libheif)" ON)
option(HAS_LIBRAW "Enable Camera RAW support (requires LibRaw)" ON)

# libheif/libde265 path resolution (install-first, build fallback)
set(LIBHEIF_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libheif-1.19.5/install/include")
if(NOT EXISTS "${LIBHEIF_INCLUDE_DIR}/libheif/heif.h")
    set(LIBHEIF_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libheif-1.19.5/libheif/api")
endif()

set(LIBHEIF_GENERATED_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libheif-1.19.5/build-vs")

set(LIBHEIF_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libheif-1.19.5/install/lib/heif.lib")
if(NOT EXISTS "${LIBHEIF_LIBRARY_PATH}")
    set(LIBHEIF_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libheif-1.19.5/build-vs/libheif/Release/heif.lib")
endif()

set(LIBDE265_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libde265-1.0.15/build-vs/libde265/Release/de265.lib")
if(NOT EXISTS "${LIBDE265_LIBRARY_PATH}")
    set(LIBDE265_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libde265-1.0.15/install/lib/de265.lib")
endif()
if(NOT EXISTS "${LIBDE265_LIBRARY_PATH}")
    set(LIBDE265_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libde265-1.0.15/install/lib/libde265.lib")
endif()

#==============================================================================
# Compiler Settings
#==============================================================================

set(CMAKE_CXX_STANDARD 20)  # C++20 for latest features
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Use /MD runtime (Multi-threaded DLL)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# Windows 11 optimizations
add_compile_definitions(
    WINVER=0x0A00
    _WIN32_WINNT=0x0A00
    NTDDI_VERSION=0x0A000000
)

#==============================================================================
# Source Files
#==============================================================================

set(ENGINE_HEADERS
    Engine.h
    Core/Types.h
    Core/Config.h
    Core/EngineAPI.h
    Core/BuildConfig.h
    Core/DecoderHealthMonitor.h
    Core/ErrorContext.h
    Core/IThumbnailDecoder.h
    Core/IFormatDetector.h
    Core/IGPURenderer.h
    Core/ICacheProvider.h
    Core/Logger.h
    Core/HighDPIScaling.h              # Sprint 194 — Per-monitor DPI scaling
    
    # Pipeline
    Pipeline/ThumbnailPipeline.h
    Pipeline/DecoderRegistry.h
    Pipeline/FormatDetector.h
    Pipeline/AsyncThumbnailProvider.h  # Sprint 187 — Async shell extension
    Pipeline/ParallelBatchDecoder.h    # Sprint 189 — Parallel batch decode
    
    # Decoders
    Decoders/ImageDecoder.h
    Decoders/WebPDecoder.h
    Decoders/AVIFDecoder.h
    Decoders/ArchiveDecoder.h
    Decoders/RAWDecoder.h         # Camera RAW decoder (LibRaw)
    Decoders/JXLDecoder.h         # JPEG XL decoder
    Decoders/HEIFDecoder.h        # HEIF/HEIC decoder
    Decoders/ICODecoder.h         # Windows Icon/Cursor decoder
    Decoders/TGADecoder.h         # Targa image decoder
    Decoders/QOIDecoder.h         # Quite OK Image decoder
    Decoders/PSDDecoder.h         # Adobe Photoshop PSD/PSB decoder
    Decoders/DDSDecoder.h         # DirectDraw Surface decoder
    Decoders/HDRDecoder.h         # Radiance HDR (RGBE) decoder
    Decoders/PPMDecoder.h         # Netpbm format decoder
    Decoders/EXRDecoder.h         # OpenEXR decoder (WIC)
    Decoders/SVGDecoder.h         # SVG/SVGZ vector decoder
    Decoders/VideoDecoder.h       # Video frame extraction (MF)
    Decoders/AudioDecoder.h       # Audio album art/waveform
    Decoders/PDFDecoder.h         # PDF document thumbnail
    Decoders/DocumentDecoder.h    # Document format thumbnails
    Decoders/FontDecoder.h        # Font preview renderer
    Decoders/ModelDecoder.h       # 3D model decoder (Sprint 12)
    Decoders/ColorSpaceManager.h  # Sprint 40 — Color space awareness & HDR tone mapping
    Decoders/EPUBDecoder.h        # Sprint 15 — EPUB cover extractor
    Decoders/OptimizedArchiveReader.h # Sprint 14 — Memory-mapped archive reader
    # Decoders/ExampleDecoder.h   # Reference template (not compiled into engine)
    Decoders/WMFDecoder.h         # Sprint 180 — Windows Metafile (WMF/EMF)
    Decoders/PCXDecoder.h         # Sprint 180 — ZSoft PCX image
    Decoders/FarbfeldDecoder.h    # Sprint 180 — Farbfeld (.ff) image
    Decoders/EPSDecoder.h         # Sprint 183 — EPS/PostScript vector
    Decoders/VTFDecoder.h          # Sprint 184 — Valve Texture Format
    Decoders/OpenRasterDecoder.h   # Sprint 185 — OpenRaster (.ora)
    Decoders/XCFDecoder.h          # Sprint 185 — GIMP XCF native
    Decoders/SGIDecoder.h          # Sprint 186 — SGI/RGB image
    Decoders/XPMDecoder.h          # Sprint 186 — X PixMap
    
    # GPU
    GPU/D3D11Renderer.h
    GPU/GDIRenderer.h
    GPU/D3D12ComputePipeline.h   # Sprint 188 — D3D12 compute pipeline
    
    # Cache
    Cache/ThumbnailCache.h
    Cache/PersistentDiskCache.h   # Sprint 192 — persistent disk cache with eviction
    
    # Plugin System
    Plugin/PluginManager.h
    Plugin/PluginDecoder.h
    Plugin/PluginHostClient.h
    Plugin/CrashHandler.h
    Plugin/IsolationModeSelector.h
    Plugin/IPC/PluginIPCProtocol.h
    Plugin/IPC/SharedMemoryManager.h
    Plugin/Security/JobObjectManager.h
    
    # Utils
    Utils/PerformanceProfiler.h
    Utils/SIMDScaler.h
    Utils/HardwareCapabilities.h
    Utils/EXIFOrientation.h
    Utils/MemoryMappedFile.h
    Utils/SmallObjectPool.h
    Utils/CodeCoverageIntegration.h  # Sprint 190 — Code coverage & fuzzing
    Utils/MemorySafetyIntegration.h  # Sprint 191 — ASAN + memory-mapped I/O
    Utils/ARM64HardwareValidator.h    # Sprint 193 — ARM64 hardware CI validation
    Utils/MSIXPackageManager.h        # Sprint 195 — MSIX packaging & auto-update
    Utils/TestSuiteExpansion.h         # Sprint 196 — Test framework expansion
    Utils/MalformedInputHandler.h      # Sprint 197 — Malformed input hardening

    # Sprint 8-12: Foundation & Activation
    Utils/GUIHardening.h              # Sprint 8  — Dark mode, high-DPI
    Utils/VersionNormalization.h      # Sprint 9  — Version audit, release notes
    Utils/ReleaseGovernance.h         # Sprint 10 — Quality gates, CI pipeline
    Plugin/PluginActivation.h         # Sprint 11 — Feature flags, IPC, lifecycle
    Utils/ObservabilityPipeline.h     # Sprint 12 — ETW, JSON logger, privacy

    # Sprint 23-25: AI & Store & OIIO
    AI/AIThumbnailEnhancer.h          # Sprint 23 — DirectML/ONNX super-resolution

    # Sprint 26-35: Advanced Features
    Cloud/CloudThumbnailProvider.h    # Sprint 26 — OneDrive/GDrive/Dropbox
    Cache/MultiTierCache.h            # Sprint 27 — Bloom filter, WAL SQLite
    Decoders/VideoEnhancer.h          # Sprint 28 — Scene detection, HDR tone map
    Plugin/PluginMarketplace.h        # Sprint 29 — Signing, security scan, REST
    Utils/AccessibilityFramework.h    # Sprint 30 — UIA, WCAG, 5 languages
    Utils/EnterpriseDeployment.h      # Sprint 31 — ADMX, GPO, network cache
    Utils/PerformancePolish.h         # Sprint 32 — Micro-profiler, memory pool
    Plugin/CrashIntelligence.h        # Sprint 33 — Minidump, symbols, bucketing
    Utils/SupplyChainSecurity.h       # Sprint 34 — SBOM SPDX/CycloneDX
    Cache/USNCacheInvalidation.h      # Sprint 35 — NTFS journal, file identity

    # Sprint 36: Modular Codecs & Memory
    Codec/ICodecModule.h              # Sprint 36 — C ABI codec interface
    Codec/CodecLoader.h               # Sprint 36 — Demand-loading registry
    Codec/CodecModuleSpecs.h          # Sprint 36 — 17 codec DLL specs
    Codec/LazyCodecManager.h          # Sprint 36 — Directory-aware preloading
    Memory/MemoryOptimizationEngine.h # Sprint 36 — Per-subsystem accounting

    # Sprint 37-39: UX Enhancements
    Shell/ContextMenuHandler.h        # Sprint 37 — Explorer right-click actions
    Decoders/AnimatedThumbnailDecoder.h # Sprint 38 — Animated WebP/JXL/GIF
    Decoders/ArchiveGridPreview.h     # Sprint 39 — 2x2 grid composite

    # Sprint 40-49: Platform Maturity
    Utils/PortableMode.h              # Sprint 42 — INI config, badges
    Pipeline/BatchProcessor.h         # Sprint 43 — Priority queue, rate limit
    Cloud/NetworkThumbnailProvider.h   # Sprint 44 — URL/proxy/retry/throttle
    Shell/PreviewPaneHandler.h        # Sprint 45 — EXIF, tooltips, columns
    Codec/FormatConverter.h           # Sprint 46 — 9 output formats, profiles
    Utils/AccessibilityI18n.h         # Sprint 47 — WCAG, RTL, keyboard nav
    Core/TelemetryDashboard.h         # Sprint 48 — Health scoring, diagnostics

    # Sprint 125-149: v8.2 Program
    Core/VersionDriftGate.h           # Sprint 125 — Version drift CI gate
    Core/ETWSinkComplete.h            # Sprint 126 — ETW sink completion
    Shell/COMApartmentAudit.h         # Sprint 127 — COM apartment audit
    Plugin/PluginRuntimeValidation.h  # Sprint 128 — Plugin E2E validation
    Memory/DirectoryFormatProfiler.h  # Sprint 129 — Directory format profiler
    Memory/DecoderHotsetManager.h     # Sprint 130 — Decoder hotset manager
    Memory/BufferPoolAllocator.h      # Sprint 131 — Buffer pool & slab allocator
    Pipeline/ExplorerWorkScheduler.h  # Sprint 132 — Explorer work scheduler
    Decoders/JPEG2000Decoder.h        # Sprint 133 — JPEG 2000 decoder
    Decoders/KTXTextureDecoder.h      # Sprint 134 — KTX/KTX2 texture decoder
    Decoders/JXRWICDecoder.h          # Sprint 135 — JPEG XR WIC decoder
    Decoders/EBookCoverExtractor.h    # Sprint 136 — Extended eBook support
    Utils/ContinuousFuzzEngine.h      # Sprint 137 — Continuous fuzzing engine
    Utils/CompatibilityMatrix.h       # Sprint 138 — Win11 compatibility matrix
    Memory/MemorySoakValidator.h      # Sprint 140 — Memory soak validation
    Utils/DarkModeManagerV2.h         # Sprint 141 — DarkMode V2 complete
    Utils/PerMonitorDPIManager.h      # Sprint 142 — Per-monitor DPI V2
    Core/DecoderHealthDashboard.h     # Sprint 143 — Decoder health dashboard
    Core/DiagnosticsExporter.h        # Sprint 144 — Diagnostics export finalization
    Core/VersionDriftDetector.h       # Sprint 145 — Version drift CI integration
    Core/PerfRegressionGate.h         # Sprint 146 — Performance regression CI gate V2
    Core/ReproducibleBuildVerifier.h   # Sprint 147 — Reproducible build verification
    Core/ReleaseReadinessDashboard.h   # Sprint 148 — Release readiness dashboard

    # Sprint 150-174: v8.3.0 Block — Plugin Ecosystem, ARM64, Format Expansion, Memory Excellence, Release
    Plugin/PluginRuntimeTestMatrix.h       # Sprint 150 — Plugin runtime test matrix
    Plugin/PluginSandboxPolicy.h           # Sprint 151 — Plugin sandbox policy framework
    Plugin/PluginCompatibilityKitV2.h      # Sprint 152 — Plugin compatibility kit V2
    Plugin/PluginReferencePack.h           # Sprint 153 — Plugin reference implementation pack
    Plugin/PluginTrustChain.h              # Sprint 154 — Plugin trust chain validation
    Utils/ARM64BuildConfig.h              # Sprint 155 — ARM64 build configuration
    Utils/ARM64LibraryMatrix.h            # Sprint 156 — ARM64 library compatibility matrix
    Utils/ARM64RuntimeValidator.h         # Sprint 157 — ARM64 runtime validator
    Utils/ARM64PerformanceBaseline.h      # Sprint 158 — ARM64 performance baseline
    Utils/ARM64CIIntegration.h            # Sprint 159 — ARM64 CI integration
    Decoders/CADFormatPlugin.h            # Sprint 161 — CAD format plugin (DWG/DXF/STEP/IGES)
    Decoders/GLTFModelDecoder.h           # Sprint 162 — glTF/GLB 3D model decoder
    Decoders/ScientificFormatPlugin.h     # Sprint 163 — Scientific format plugin (FITS/NIfTI)
    Pipeline/FormatFallbackEngine.h       # Sprint 164 — Format fallback intelligence engine
    Memory/ArchiveMemoryCompactor.h       # Sprint 165 — Archive memory compactor
    Pipeline/ZeroCopyPipeline.h           # Sprint 166 — Zero-copy GPU upload pipeline
    Cache/AdaptiveCacheBudgetManager.h    # Sprint 167 — Adaptive cache budget manager
    Memory/HotModeDirectoryEngine.h       # Sprint 168 — Hot-mode directory pre-warm engine
    Memory/MemoryPressureControllerV2.h   # Sprint 169 — Memory pressure controller V2
    Utils/MatrixValidationFramework.h     # Sprint 170 — Matrix validation framework
    Utils/InstallerLifecycleAutomation.h  # Sprint 171 — Installer lifecycle automation
    Core/ReleaseGateV2.h                  # Sprint 172 — Release gate V2 (v8.3.0 KPIs)
    Core/DocumentationSyncAudit.h         # Sprint 173 — Documentation sync audit
    Core/ProgramClosureV83.h              # Sprint 174 — v8.3.0 program closure
)

set(ENGINE_SOURCES
    # Core implementations
    Core/Config.cpp
    Core/EngineAPI.cpp
    Core/HighDPIScaling.cpp              # Sprint 194 — Per-monitor DPI scaling
    
    # Pipeline implementations
    Pipeline/ThumbnailPipeline.cpp
    Pipeline/DecoderRegistry.cpp
    Pipeline/FormatDetector.cpp
    Pipeline/AsyncThumbnailProvider.cpp  # Sprint 187 — Async shell extension
    Pipeline/ParallelBatchDecoder.cpp    # Sprint 189 — Parallel batch decode
    
    # Decoders
    Decoders/ImageDecoder.cpp
    Decoders/WebPDecoder.cpp
    Decoders/AVIFDecoder.cpp
    Decoders/ArchiveDecoder.cpp
    Decoders/RAWDecoder.cpp       # Camera RAW decoder (LibRaw)
    Decoders/JXLDecoder.cpp       # JPEG XL decoder
    Decoders/HEIFDecoder.cpp      # HEIF/HEIC decoder
    Decoders/ICODecoder.cpp       # Windows Icon/Cursor decoder
    Decoders/TGADecoder.cpp       # Targa image decoder
    Decoders/QOIDecoder.cpp       # Quite OK Image decoder
    Decoders/PSDDecoder.cpp       # Adobe Photoshop PSD/PSB decoder
    Decoders/DDSDecoder.cpp       # DirectDraw Surface decoder
    Decoders/HDRDecoder.cpp       # Radiance HDR (RGBE) decoder
    Decoders/PPMDecoder.cpp       # Netpbm format decoder
    Decoders/EXRDecoder.cpp       # OpenEXR decoder (WIC)
    Decoders/SVGDecoder.cpp       # SVG/SVGZ vector decoder
    Decoders/VideoDecoder.cpp     # Video frame extraction (MF)
    Decoders/AudioDecoder.cpp     # Audio album art/waveform
    Decoders/PDFDecoder.cpp       # PDF document thumbnail
    Decoders/DocumentDecoder.cpp  # Document format thumbnails
    Decoders/FontDecoder.cpp      # Font preview renderer
    Decoders/ModelDecoder.cpp     # 3D model decoder (Sprint 12)
    Decoders/WMFDecoder.cpp       # Sprint 180 — Windows Metafile (WMF/EMF)
    Decoders/PCXDecoder.cpp       # Sprint 180 — ZSoft PCX image
    Decoders/FarbfeldDecoder.cpp  # Sprint 180 — Farbfeld (.ff) image
    Decoders/JPEG2000Decoder.cpp  # Sprint 181 — JPEG 2000 (JP2/J2K/JPX)
    Decoders/EPSDecoder.cpp       # Sprint 183 — EPS/PostScript vector
    Decoders/KTXTextureDecoder.cpp # Sprint 184 — KTX/KTX2 texture decoder
    Decoders/VTFDecoder.cpp        # Sprint 184 — Valve Texture Format
    Decoders/OpenRasterDecoder.cpp # Sprint 185 — OpenRaster (.ora)
    Decoders/XCFDecoder.cpp        # Sprint 185 — GIMP XCF native
    Decoders/SGIDecoder.cpp        # Sprint 186 — SGI/RGB image
    Decoders/XPMDecoder.cpp        # Sprint 186 — X PixMap
    
    # GPU
    GPU/D3D11Renderer.cpp
    GPU/GDIRenderer.cpp
    GPU/D3D12ComputePipeline.cpp   # Sprint 188 — D3D12 compute pipeline
    
    # Cache
    Cache/ThumbnailCache.cpp
    Cache/PersistentDiskCache.cpp   # Sprint 192 — persistent disk cache with eviction
    
    # Plugin System
    Plugin/PluginManager.cpp
    Plugin/PluginDecoder.cpp
    Plugin/PluginHostClient.cpp
    Plugin/CrashHandler.cpp
    Plugin/IsolationModeSelector.cpp
    Plugin/IPC/SharedMemoryManager.cpp
    Plugin/Security/JobObjectManager.cpp
    
    # Utils
    Utils/PerformanceProfiler.cpp
    Utils/SIMDScaler.cpp
    Utils/HardwareCapabilities.cpp
    Utils/EXIFOrientation.cpp
    Utils/CodeCoverageIntegration.cpp  # Sprint 190 — Code coverage & fuzzing
    Utils/MemorySafetyIntegration.cpp  # Sprint 191 — ASAN + memory-mapped I/O
    Utils/ARM64HardwareValidator.cpp    # Sprint 193 — ARM64 hardware CI validation
    Utils/MSIXPackageManager.cpp        # Sprint 195 — MSIX packaging & auto-update
    Utils/TestSuiteExpansion.cpp         # Sprint 196 — Test framework expansion
    Utils/MalformedInputHandler.cpp      # Sprint 197 — Malformed input hardening
)

#==============================================================================
# Engine Library Target
#==============================================================================

if(ENGINE_SOURCES)
    if(BUILD_SHARED_LIBS)
        add_library(DarkThumbsEngine SHARED ${ENGINE_SOURCES} ${ENGINE_HEADERS})
        target_compile_definitions(DarkThumbsEngine PRIVATE ENGINE_EXPORTS)
    else()
        add_library(DarkThumbsEngine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})
        # STATIC library: do NOT define ENGINE_EXPORTS, so ENGINE_API is empty
        target_compile_definitions(DarkThumbsEngine PUBLIC ENGINE_STATIC)
    endif()
    
    target_include_directories(DarkThumbsEngine
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../SDK/include
            ${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libwebp-1.5.0-original/src
            ${CMAKE_CURRENT_SOURCE_DIR}/../external/compression-libs/minizip-ng-4.0.10
            # JXL includes (actual install location)
            ${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libjxl-0.11.1/install/include
            # libheif includes (HEIF/HEIC support)
            ${LIBHEIF_INCLUDE_DIR}
            ${LIBHEIF_GENERATED_INCLUDE_DIR}
            # LibRaw includes
            ${CMAKE_CURRENT_SOURCE_DIR}/../external/camera-libs/libraw-install/include
    )
    
    # Windows-specific settings
    if(WIN32)
        target_compile_definitions(DarkThumbsEngine PRIVATE
            _WINDOWS
            WIN32_LEAN_AND_MEAN
            NOMINMAX
        )
        
        # Conditional decoder feature flags
        if(HAS_LIBJXL)
            target_compile_definitions(DarkThumbsEngine PRIVATE HAS_LIBJXL JXL_STATIC_DEFINE)
            message(STATUS "JPEG XL support: ENABLED")
        else()
            message(STATUS "JPEG XL support: DISABLED (set -DHAS_LIBJXL=ON when libjxl is available)")
        endif()
        
        if(HAS_LIBHEIF)
            target_compile_definitions(DarkThumbsEngine PRIVATE HAS_LIBHEIF LIBHEIF_STATIC_BUILD)
            message(STATUS "HEIF/HEIC support: ENABLED")
        else()
            message(STATUS "HEIF/HEIC support: DISABLED (set -DHAS_LIBHEIF=ON when libheif is available)")
        endif()
        
        if(HAS_LIBRAW)
            target_compile_definitions(DarkThumbsEngine PRIVATE HAS_LIBRAW)
            message(STATUS "Camera RAW support: ENABLED")
        else()
            message(STATUS "Camera RAW support: DISABLED (set -DHAS_LIBRAW=ON when LibRaw is available)")
        endif()

        if(HAS_LIBAVIF)
            target_compile_definitions(DarkThumbsEngine PRIVATE HAS_LIBAVIF)
            message(STATUS "AVIF support: ENABLED")
        else()
            message(STATUS "AVIF support: DISABLED (set -DHAS_LIBAVIF=ON when libavif is available)")
        endif()

        if(HAS_MUPDF)
            target_compile_definitions(DarkThumbsEngine PRIVATE HAS_MUPDF)
            message(STATUS "PDF support (MuPDF): ENABLED")
        else()
            message(STATUS "PDF support (MuPDF): DISABLED (set -DHAS_MUPDF=ON when MuPDF is available)")
        endif()

        if(ENABLE_VIDEO_DECODER)
            target_compile_definitions(DarkThumbsEngine PRIVATE ENABLE_VIDEO_DECODER)
            message(STATUS "Video decoder: ENABLED")
        else()
            message(STATUS "Video decoder: DISABLED (set -DENABLE_VIDEO_DECODER=ON)")
        endif()

        if(ENABLE_AUDIO_DECODER)
            target_compile_definitions(DarkThumbsEngine PRIVATE ENABLE_AUDIO_DECODER)
            message(STATUS "Audio decoder: ENABLED")
        else()
            message(STATUS "Audio decoder: DISABLED (set -DENABLE_AUDIO_DECODER=ON)")
        endif()
        
        target_link_libraries(DarkThumbsEngine
            PRIVATE
                gdiplus
                d3d11
                dxgi
                windowscodecs
                shlwapi
                bcrypt          # For minizip-ng crypto
                crypt32         # For minizip-ng crypto and signatures
                wintrust        # For Authenticode signature verification
                mfplat          # Media Foundation for video decoding
                mfreadwrite     # MF Source Reader
                mfuuid          # MF GUIDs
                mf              # Media Foundation
                propsys         # Property System for audio metadata
        )
        
        # Link libwebp
        target_link_directories(DarkThumbsEngine
            PUBLIC
                ${CMAKE_CURRENT_SOURCE_DIR}/../SDK/lib
                ${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libwebp-1.5.0-original/output/x64/Release/release-static/x64/lib
                ${CMAKE_CURRENT_SOURCE_DIR}/../external/compression-libs/minizip-ng-4.0.10/build-manual
                ${CMAKE_CURRENT_SOURCE_DIR}/../external/compression-libs/zlib-1.3.1/install/lib
                ${CMAKE_CURRENT_SOURCE_DIR}/../external/compression-libs/zstd-1.5.7/build-manual
                # JXL libraries (actual install location)
                ${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libjxl-0.11.1/install/lib
                # LibRaw libraries
                ${CMAKE_CURRENT_SOURCE_DIR}/../external/camera-libs/libraw-install/lib
        )
        
        target_link_libraries(DarkThumbsEngine
            PUBLIC
                libwebp
                libsharpyuv
                minizip
                zlibstatic
                zstd_static
        )
        
        # Conditionally link optional decoder libraries
        if(HAS_LIBJXL)
            target_link_libraries(DarkThumbsEngine PUBLIC jxl jxl_cms jxl_threads brotlicommon brotlidec hwy)
        endif()
        if(HAS_LIBRAW)
            target_link_libraries(DarkThumbsEngine PUBLIC libraw_static)
        endif()
        if(HAS_LIBHEIF)
            if(NOT EXISTS "${LIBHEIF_LIBRARY_PATH}")
                message(FATAL_ERROR "HAS_LIBHEIF=ON but heif.lib was not found. Expected at ${LIBHEIF_LIBRARY_PATH}")
            endif()
            if(NOT EXISTS "${LIBDE265_LIBRARY_PATH}")
                message(FATAL_ERROR "HAS_LIBHEIF=ON but libde265 library was not found. Expected at ${LIBDE265_LIBRARY_PATH}")
            endif()
            target_link_libraries(DarkThumbsEngine PUBLIC "${LIBHEIF_LIBRARY_PATH}" "${LIBDE265_LIBRARY_PATH}")
        endif()
    endif()
    
    # Compiler warnings
    if(MSVC OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(DarkThumbsEngine PRIVATE
                -Wall         # Enable all warnings
                -Wextra       # Extra warnings
                -mavx2        # Enable AVX2 instructions for SIMD operations
                -mfma         # Enable FMA instructions
            )
        else()
            target_compile_options(DarkThumbsEngine PRIVATE
                /W4           # Warning level 4
                /WX           # Warnings as errors - TEMPORARILY ENABLED TO FIND ALL WARNINGS
                /permissive-  # Standards conformance
                /arch:AVX2    # Enable AVX2 instructions for SIMD operations
            )
        endif()
        
        # CRT Runtime Linking Fix
        # NOTE: This is a temporary workaround. The proper fix is to rebuild
        # all external libraries with /MD (dynamic CRT) instead of /MT (static CRT).
        # Affected libraries: libwebp, zlib, zstd, minizip-ng, brotli, highway
        # 
        # Proper solution:
        #   1. Rerun CMake for each external library with: -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreadedDLL"
        #   2. Or add set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL") to their CMakeLists.txt
        #   3. Rebuild all Release libraries
        # 
        # Until fixed, we must ignore LIBCMT to avoid LNK4098 warnings/errors
        target_link_options(DarkThumbsEngine PRIVATE
            /NODEFAULTLIB:LIBCMT    # Ignore static CRT conflicts
            /IGNORE:4099            # Ignore missing PDB warnings for external libs
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
# PluginHost Executable
#==============================================================================

# API mismatches need to be fixed - deferred to Sprint 17
# add_subdirectory(PluginHost)

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
message(STATUS "  Feature Flags:")
message(STATUS "    HAS_LIBJXL:            ${HAS_LIBJXL}")
message(STATUS "    HAS_LIBHEIF:           ${HAS_LIBHEIF}")
message(STATUS "    HAS_LIBRAW:            ${HAS_LIBRAW}")
message(STATUS "    HAS_LIBAVIF:           ${HAS_LIBAVIF}")
message(STATUS "    HAS_MUPDF:             ${HAS_MUPDF}")
message(STATUS "    ENABLE_VIDEO_DECODER:  ${ENABLE_VIDEO_DECODER}")
message(STATUS "    ENABLE_AUDIO_DECODER:  ${ENABLE_AUDIO_DECODER}")
message(STATUS "")
