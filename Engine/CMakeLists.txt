#==============================================================================
# ExplorerLens Engine - CMake Build Configuration
# Copyright (c) 2026 - ExplorerLens.io Project
# Version: 35.5.0
#==============================================================================

cmake_minimum_required(VERSION 3.20)

project(ExplorerLensEngine
    VERSION 35.5.0
    DESCRIPTION "ExplorerLens.io Thumbnail Generation Engine"
    LANGUAGES CXX
)

#==============================================================================
# Architecture Guard (x64-only)
#==============================================================================

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "ExplorerLensEngine is x64-only. Reconfigure with a 64-bit generator/platform.")
endif()

if(CMAKE_GENERATOR MATCHES "Visual Studio" AND DEFINED CMAKE_GENERATOR_PLATFORM)
    if(NOT CMAKE_GENERATOR_PLATFORM STREQUAL "x64")
        message(FATAL_ERROR "ExplorerLensEngine requires Visual Studio platform x64. Current: ${CMAKE_GENERATOR_PLATFORM}")
    endif()
endif()

#==============================================================================
# Project Options
#==============================================================================

option(BUILD_TESTS "Build unit tests" ON)  # Enabled for CI/CD
option(BUILD_CATCH2_TESTS "Build Catch2-based unit tests" ON)
option(BUILD_BENCHMARKS "Build performance benchmarks" ON)
option(BUILD_SHARED_LIBS "Build as shared library" ON)
option(ENABLE_GPU "Enable GPU acceleration" ON)
option(ENABLE_CACHE "Enable thumbnail caching" ON)

# Media Foundation decoders (Windows built-in — always available)
option(ENABLE_VIDEO_DECODER "Enable video thumbnail decoder" ON)
option(ENABLE_AUDIO_DECODER "Enable audio thumbnail decoder" ON)

#==============================================================================
# Auto-detect external libraries (resolves paths, sets HAS_* accordingly)
#==============================================================================

# --- JPEG XL (libjxl) ---
set(LIBJXL_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libjxl-0.11.1/install")
set(LIBJXL_ALT_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libjxl-0.11.1/build-msvc/lib")
set(LIBJXL_INCLUDE_DIR "${LIBJXL_INSTALL_DIR}/include")
if(NOT EXISTS "${LIBJXL_INCLUDE_DIR}")
    set(LIBJXL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libjxl-0.11.1/lib/include")
endif()
if((EXISTS "${LIBJXL_INSTALL_DIR}/lib/jxl.lib" OR EXISTS "${LIBJXL_ALT_LIB_DIR}/jxl.lib") AND EXISTS "${LIBJXL_INCLUDE_DIR}")
    set(HAS_LIBJXL ON CACHE BOOL "JPEG XL support auto-detected" FORCE)
else()
    set(HAS_LIBJXL OFF CACHE BOOL "JPEG XL support — libjxl not found" FORCE)
endif()

# --- libheif / libde265 ---
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

if(EXISTS "${LIBHEIF_LIBRARY_PATH}" AND EXISTS "${LIBDE265_LIBRARY_PATH}")
    set(HAS_LIBHEIF ON CACHE BOOL "HEIF/HEIC support auto-detected" FORCE)
else()
    set(HAS_LIBHEIF OFF CACHE BOOL "HEIF/HEIC support — libheif or libde265 not found" FORCE)
endif()

# --- LibRaw ---
set(LIBRAW_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/camera-libs/libraw-install")
set(LIBRAW_ALT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/camera-libs/libraw")
if((EXISTS "${LIBRAW_INSTALL_DIR}/lib/libraw_static.lib" OR EXISTS "${LIBRAW_ALT_DIR}/lib/libraw_static.lib") AND (EXISTS "${LIBRAW_INSTALL_DIR}/include" OR EXISTS "${LIBRAW_ALT_DIR}/libraw"))
    set(HAS_LIBRAW ON CACHE BOOL "Camera RAW support auto-detected" FORCE)
else()
    set(HAS_LIBRAW OFF CACHE BOOL "Camera RAW support — LibRaw not found" FORCE)
endif()

# --- libavif / dav1d ---
set(LIBAVIF_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libavif-1.3.0/install")
set(LIBAVIF_ALT_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libavif-1.3.0/build")
set(DAV1D_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/dav1d-1.5.1/install")
if((EXISTS "${LIBAVIF_INSTALL_DIR}/lib/avif.lib" OR EXISTS "${LIBAVIF_ALT_LIB_DIR}/avif.lib") AND EXISTS "${DAV1D_INSTALL_DIR}/lib/dav1d.lib")
    set(HAS_LIBAVIF ON CACHE BOOL "AVIF support auto-detected" FORCE)
else()
    set(HAS_LIBAVIF OFF CACHE BOOL "AVIF support — libavif or dav1d not found" FORCE)
endif()

# --- MuPDF ---
set(MUPDF_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../external/pdf-libs/mupdf-1.24.11-source/platform/win32/x64/Release/libmupdf.lib")
if(EXISTS "${MUPDF_LIB_PATH}")
    set(HAS_MUPDF ON CACHE BOOL "PDF support auto-detected" FORCE)
else()
    set(HAS_MUPDF OFF CACHE BOOL "PDF support — MuPDF not built (see docs/build/BUILD_GUIDE.md)" FORCE)
endif()

# --- OpenJPEG ---
set(OPENJPEG_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../x64/Release/openjp2.lib")
set(OPENJPEG_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/openjpeg-2.5.3/install")
if(EXISTS "${OPENJPEG_LIB_PATH}")
    set(HAS_OPENJPEG ON CACHE BOOL "JPEG 2000 support auto-detected" FORCE)
else()
    set(HAS_OPENJPEG OFF CACHE BOOL "JPEG 2000 support — OpenJPEG not built (run Build-OpenJPEG.ps1)" FORCE)
endif()

# --- libarchive (Multi-format archive support) ---
set(LIBARCHIVE_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/compression-libs/libarchive-3.7.6/install")
set(LIBARCHIVE_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/compression-libs/libarchive-3.7.6/build-vs")
set(LIBARCHIVE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/compression-libs/libarchive-3.7.6/libarchive")
# Check multiple possible output locations for archive_static.lib
if(EXISTS "${LIBARCHIVE_INSTALL_DIR}/lib/archive_static.lib")
    set(LIBARCHIVE_LIBRARY_PATH "${LIBARCHIVE_INSTALL_DIR}/lib/archive_static.lib")
    set(HAS_LIBARCHIVE ON CACHE BOOL "libarchive support auto-detected (install)" FORCE)
elseif(EXISTS "${LIBARCHIVE_INSTALL_DIR}/lib/archive.lib")
    set(LIBARCHIVE_LIBRARY_PATH "${LIBARCHIVE_INSTALL_DIR}/lib/archive.lib")
    set(HAS_LIBARCHIVE ON CACHE BOOL "libarchive support auto-detected (install)" FORCE)
elseif(EXISTS "${LIBARCHIVE_BUILD_DIR}/libarchive/Release/archive_static.lib")
    set(LIBARCHIVE_LIBRARY_PATH "${LIBARCHIVE_BUILD_DIR}/libarchive/Release/archive_static.lib")
    set(HAS_LIBARCHIVE ON CACHE BOOL "libarchive support auto-detected (build)" FORCE)
else()
    set(HAS_LIBARCHIVE OFF CACHE BOOL "libarchive not built (run Build-Libarchive.ps1)" FORCE)
endif()

# --- FreeType ---
set(FREETYPE_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../x64/Release/freetype.lib")
if(EXISTS "${FREETYPE_LIB_PATH}")
    set(HAS_FREETYPE ON CACHE BOOL "Enhanced font rendering auto-detected" FORCE)
else()
    set(HAS_FREETYPE OFF CACHE BOOL "Enhanced font — FreeType not built (run Build-FreeType.ps1)" FORCE)
endif()

#==============================================================================
# Compiler Settings
#==============================================================================

set(CMAKE_CXX_STANDARD 20)  # C++20 for latest features
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

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

    # Core engine components
    Core/AccessibilitySuiteV2.h
    Core/AdaptiveDecoderRouter.h
    Core/AIThumbnailEnhancer.h
    Core/AsyncShellActivation.h
    Core/AsyncShellExtension.h
    Core/BatchProcessingEngine.h
    Core/BuildConfig.h
    Core/CloudNativeSync.h
    Core/CloudStorageIntegration.h
    Core/CloudSyncProvider.h
    Core/Concepts.h
    Core/Config.h
    Core/ContentIndexer.h
    Core/D3D12PipelineActivation.h
    Core/DarkModeRendererV2.h
    Core/DeadCodeAudit.h
    Core/DeadCodeAuditor.h
    Core/DiagnosticsExporter.h
    Core/DocumentationSyncAudit.h
    Core/EncoderExportEngine.h
    Core/EngineAPI.h
    Core/ErrorContext.h
    Core/Expected.h
    Core/ETWSinkComplete.h
    Core/ETWTraceProvider.h
    Core/FormatConverterEngine.h
    Core/FormatRegistry.h
    Core/FormatTypes.h
    Core/GPUMemoryPoolV2.h
    Core/GPUPipelineV3.h
    Core/HDRDisplayPipeline.h
    Core/JumpListIntegration.h

    # Core interfaces (canonical location; forwarding stubs exist at Core/I*.h for decoder includes)
    Core/Interfaces/ICacheProvider.h
    Core/Interfaces/IFormatDetector.h
    Core/Interfaces/IGPURenderer.h
    Core/Interfaces/IThumbnailDecoder.h

    Core/LivePreviewEngine.h
    Core/Logger.h
    Core/MetadataExtractor.h
    Core/MultiGPULoadBalancer.h
    Core/NetworkProviderEngine.h
    Core/ParallelBatchProcessor.h
    Core/PerfRegressionGate.h
    Core/PerformanceBenchmarkV2.h
    Core/PerMonitorDPIV3.h
    Core/PreviewPanelV2.h
    Core/ProgressiveThumbnailLoader.h
    Core/ProgramClosureV83.h
    Core/QuickLookIntegration.h
    Core/RegistryManager.h
    Core/ReleaseReadinessDashboard.h
    Core/ReproducibleBuildVerifier.h
    Core/ResourcePoolEngine.h
    Core/RuntimeIntegrityVerifier.h
    Core/SecurityHardeningV2.h
    Core/ShaderCompilerV2.h
    Core/SharePointTeamsIntegration.h
    Core/ShellContextMenuV2.h
    Core/ShellRegistrationManager.h
    Core/SIMDAccelerationManager.h
    Core/SIMDAccelerator.h
    Core/Telemetry.h
    Core/ThemeEngine.h
    Core/ObservabilityIntegration.h
    Core/PerformanceActivation.h
    Core/ThumbnailAnimationEngineV2.h
    Core/ThumbnailQuality.h
    Core/ThumbnailBatch.h
    Core/ThumbnailRenderer.h
    Core/ThumbnailStream.h
    Core/ThumbnailMetrics.h
    Core/Types.h
    Core/UpdateEngine.h
    Core/LibraryInventoryManager.h
    Core/LibraryVersionAudit.h
    Core/VersionDriftDetector.h
    Core/VersionDriftGate.h
    Core/VersionManagement.h
    Core/VersionSynchronizer.h
    Core/VersionSyncV14.h
    Core/VulkanComputeActivation.h
    Core/WatchFolderEngine.h
    Core/WinRTAppSDKIntegrationV2.h
    Core/SmartBatchProcessor.h
    Core/WorkflowAutomationEngine.h
    Core/UserBehaviorAnalytics.h
    Core/FaultTolerantDecodeOrchestrator.h
    Core/DiagnosticTelemetryCollector.h
    Core/DecoderFaultIsolator.h
    Core/SmartFileTypeRouter.h
    Core/DecoderVersionManager.h
    Core/CrossFormatMetadataEngine.h

    # Core — Integration & Utilities
    Core/ArchiveRefactorEngine.h
    Core/BuildValidation.h
    Core/CIHardeningEngine.h
    Core/CodeCoverageEngine.h
    Core/CRTConsistencyManager.h
    Core/FFmpegIntegration.h
    Core/FreeTypeIntegration.h
    Core/GPUShaderLibrary.h
    Core/HybridUIBridge.h
    Core/LibWebPConfig.h
    Core/MuPDFIntegration.h
    Core/OpenJPEGIntegration.h
    Core/PluginHostManager.h
    Core/PluginTypes.h
    Core/PropertyStoreHandler.h
    Core/SettingsExportImport.h
    Core/SettingsImportExport.h
    Core/SystemTrayManager.h
    Core/VideoCodecRouter.h
    Core/WinUI3MigrationEngine.h
    Core/WinUI3Research.h

    # Batch 5 — Sprints 394-405
    Core/FileIntegrityMonitor.h
    Core/IntelligentPrefetchV2.h
    Core/CompressionBenchmark.h
    Core/ExplorerBandIntegration.h
    Core/HotReloadConfigEngine.h
    Core/COMDiagnosticsEngine.h

    Core/BatchRenamePreview.h
    Core/DuplicateFileDetector.h
    Core/ExplorerContextMenuExtension.h
    Core/FileTypeStatistics.h
    Core/FilePreviewRouter.h
    Core/ClipboardThumbnailManager.h
    Core/FormatConversionPipeline.h
    Core/EnterpriseAuditPipeline.h
    Core/ResourceQuotaManager.h
    Core/AccessTokenValidator.h
    Core/ExplorerPreviewPane.h

    # Sprint 589-590
    Core/IntegrationTestRunner.h
    Core/SBOMGenerator.h

    # Sprint 644 — PIpeline activation, status, SIMD detection
    Core/SIMDCapabilityDetector.h
    Core/InputSanitizer.h
    Core/PathTraversalGuard.h

    # Production pipeline & performance infrastructure
    Core/ContentAwareThumbnail.h
    Core/RuntimeSIMDDispatcher.h
    Core/BatchThumbnailOrchestrator.h
    Core/FileSignatureDetector.h
    Core/GPUResourcePoolManager.h
    Core/CacheCoherencyProtocol.h
    Core/DecoderRegistryV2.h
    Core/AudioSpectrogramRenderer.h
    Core/ArchiveHierarchyMapRenderer.h
    Core/CodeSyntaxThumbnail.h
    Core/PerceptualHashEngine.h
    Core/DominantColorExtractor.h
    Core/PhotoMosaicThumbnail.h
    Core/FontGlyphGridRenderer.h
    Core/SpreadsheetCellPreview.h
    Core/PresentationSlideStrip.h
    Core/ColorHistogramBadge.h
    Core/GeoTagMapThumbnail.h
    Core/FileAgeVisualizer.h
    Core/SmartGridLayoutEngine.h
    Core/MetadataTooltipRenderer.h
    Core/CompressedStreamAnalyzer.h
    Core/AdaptiveContrastEnhancer.h
    Core/DocumentDigestOverlay.h
    Core/ArchivePasswordDetector.h
    Core/CalendarHeatmapRenderer.h
    Core/FileSizeProportionBadge.h
    Core/IntelligentCachePruner.h
    Core/AlignedBufferPool.h
    Core/PrefetchHintEngine.h
    Core/BranchPredictor.h

    # Sprint 45-46: Error handling infrastructure
    Core/StructuredErrorDomain.h
    Core/ResultType.h
    Core/ErrorRecoveryEngineV2.h
    Core/DiagnosticCollector.h

    # Sprint 491-500 — Cross-Process Architecture (v24.1.0 "Altair-R")
    Core/OutOfProcThumbnailServer.h
    Core/CrossProcessCacheProxy.h
    Core/ProcessPoolManager.h
    Core/ProcessIsolationPolicy.h
    Core/CrossProcEventBus.h
    Core/SharedStateCoordinator.h

    # Sprint 1051-1060 — Cross-Platform Shell (v31.1.0 Achernar-R)
    Core/DecoderPriorityScheduler.h
    Core/ThumbnailColorSpace.h
    Core/MultiMonitorDPIScaler.h
    Core/ThumbnailHistogram.h
    Core/CoreLocalizationEngine.h
    Core/ThumbnailSpriteSheet.h
    Core/DecoderHealthDashboard.h
    Core/DecoderHealthMonitor.h
    Core/DecoderPriorityManager.h
    Core/FormatGalleryView.h
    Core/FormatGroupManager.h
    Core/CrashIntelligence.h

    # Pipeline stages and scheduling
    Pipeline/AsyncThumbnailProvider.h
    Pipeline/BatchProcessor.h
    Pipeline/DecoderRegistry.h
    Pipeline/ExplorerWorkScheduler.h
    Pipeline/FormatDetector.h
    Pipeline/ParallelBatchDecoder.h
    Pipeline/SmartFormatDetectorV2.h
    Pipeline/ShellProgressIndicator.h
    Pipeline/SIMDDispatchRouter.h
    Pipeline/StreamingDecodeEngine.h
    Pipeline/ThumbnailPipeline.h
    Pipeline/ZeroCopyActivation.h
    Pipeline/ZeroCopyPipeline.h
    Pipeline/FormatSignatureDetector.h
    Pipeline/AdaptiveQualityScaler.h
    Pipeline/FilesystemWatchdog.h
    Pipeline/ParallelIOPipeline.h
    Pipeline/PipelineActivator.h
    Pipeline/ParallelIOActivation.h
    Pipeline/PipelineMetricsCollector.h
    Pipeline/PipelineCircuitBreaker.h
    Pipeline/DecodeRetryPolicy.h
    Pipeline/ThumbnailRequestValidator.h
    Pipeline/DecoderOutputValidator.h
    Pipeline/ProductionPipelineV2.h
    Pipeline/RemoteRenderProxy.h
    Pipeline/LoopTuner.h
    Pipeline/PipelineErrorBoundary.h
    Pipeline/AdaptivePipelineOptimizer.h
    Pipeline/IntelligentPrefetchScheduler.h
    Pipeline/SmartRetryOrchestrator.h
    Pipeline/PipelineHealthMonitor.h
    Pipeline/StreamingDecodeCoordinator.h
    Pipeline/RenderPipelineProfiler.h

    # Format decoders
    Decoders/Advanced3DFormatDecoder.h
    Decoders/AnimatedFormatHandler.h
    Decoders/ArchiveDecoder.h
    Decoders/ArchiveGridPreview.h
    Decoders/AudioDecoder.h
    Decoders/AudioVisualizationV2.h
    Decoders/AVIFDecoder.h
    Decoders/CADFormatDecoder.h
    Decoders/CADFormatPlugin.h
    Decoders/ColorSpaceManager.h
    Decoders/DatabasePreviewDecoder.h
    Decoders/DDSDecoder.h
    Decoders/DecoderSecurityHardening.h
    Decoders/DICOMDecoderV2.h
    Decoders/DocumentDecoder.h
    Decoders/DPXDecoder.h
    Decoders/EBookCoverExtractor.h
    Decoders/EBookDecoder.h
    Decoders/EPSDecoder.h
    Decoders/EPUBDecoder.h
    Decoders/ExtendedVideoDecoder.h
    Decoders/EXRDecoder.h
    Decoders/FarbfeldDecoder.h
    Decoders/FITSDecoderV2.h
    Decoders/FontDecoder.h
    Decoders/GeospatialDecoder.h
    Decoders/GLTFModelDecoder.h
    Decoders/HDRDecoder.h
    Decoders/HEIFDecoder.h
    Decoders/ICODecoder.h
    Decoders/ImageDecoder.h
    Decoders/JPEG2000Decoder.h
    Decoders/JXLDecoder.h
    Decoders/JXRWICDecoder.h
    Decoders/KTXTextureDecoder.h
    Decoders/LegacyImageDecoder.h
    Decoders/Model3DRendererV2.h
    Decoders/ModelDecoder.h
    Decoders/ModelFormatHandler.h
    Decoders/NIfTIDecoder.h
    Decoders/NotebookPreviewDecoder.h
    Decoders/OpenRasterDecoder.h
    Decoders/OptimizedArchiveReader.h
    Decoders/PCXDecoder.h
    Decoders/PDFDecoder.h
    Decoders/PPMDecoder.h
    Decoders/PSDDecoder.h
    Decoders/QOIDecoder.h
    Decoders/RAWDecoder.h
    Decoders/ScientificDataDecoder.h
    Decoders/ScientificFormatPlugin.h
    Decoders/SGIDecoder.h
    Decoders/SpreadsheetPreviewDecoder.h
    Decoders/StructuredDataDecoder.h
    Decoders/SVGDecoder.h
    Decoders/TextPreviewDecoder.h
    Decoders/TGADecoder.h
    Decoders/USDDecoder.h
    Decoders/VectorFormatDecoder.h
    Decoders/VTFDecoder.h
    Decoders/VideoDecoder.h
    Decoders/WebPDecoder.h
    Decoders/WMFDecoder.h
    Decoders/XCFDecoder.h
    Decoders/ExampleDecoder.h
    Decoders/MultiPageStripRenderer.h
    Decoders/VideoKeyframeExtractor.h
    Decoders/AnimatedImageDecoder.h
    Decoders/ProgressiveJPEGDecoder.h
    Decoders/XPMDecoder.h
    Decoders/ICNSDecoder.h
    Decoders/CURDecoder.h
    Decoders/ANIMDecoder.h
    Decoders/MNGDecoder.h
    Decoders/HRZDecoder.h
    Decoders/PIXARDecoder.h
    Decoders/JPEG2000TileDecoderV2.h
    Decoders/FLIFDecoderV2.h

    # Sprint 481-490 — AI-Native Thumbnailing v2 (v24.0.0 "Altair")

    # GPU rendering and compute
    GPU/D3D11Renderer.h
    GPU/D3D12ComputePipeline.h
    GPU/GDIRenderer.h
    GPU/AdaptiveGPUScheduler.h
    GPU/HDRToneMapKernel.h
    GPU/LanczosGPUKernel.h
    GPU/AsyncTextureSampler.h
    GPU/ShaderCacheCompiler.h
    GPU/VulkanComputePipeline.h
    GPU/VulkanMemoryAllocator.h
    GPU/GPUWorkloadBalancer.h
    GPU/GPUDecodeAccelerationV2.h
    GPU/GPUFallbackChain.h
    GPU/GPUMemoryTracker.h
    # Sprint 1211-1220 — GPU-First Decode Pipeline (v34.1.0 "Arcturus-R")
    GPU/GPUDecodeFormatRouter.h
    GPU/GPUJPEGDecodeAccelerator.h
    GPU/GPURawDemosaicKernel.h
    GPU/GPUDecodePerformanceGate.h
    GPU/ZeroCopyGPUSurface.h

    # Sprint 1221-1230 — HDR & Wide Color Gamut Mastery (v34.2.0 "Arcturus-S")
    Core/GainmapJPEGToneMapper.h
    Core/PQToSDRToneMapper.h
    Core/HLGToSDRConverter.h
    Core/ICCv5ProfileEngine.h
    Core/ACESODTProcessor.h

    # Sprint 1231-1240 — Predictive Pre-Generation Engine (v34.3.0 "Arcturus-T")
    Pipeline/DirectoryPreScanQueue.h
    Pipeline/AdjacencyPredictor.h
    Pipeline/ScrollVelocityTracker.h
    Pipeline/IdleTimePreGenerator.h
    Pipeline/PredictivePreGenEngine.h

    # Sprint 1241-1250 — Animated & Sequence Format Suite (v34.4.0 "Arcturus-U")
    Core/HoverScrubController.h
    Core/APNGFrameCombiner.h
    Decoders/GIFAnimationDecoder.h
    Decoders/AnimatedSequenceSampler.h
    Cache/AnimatedThumbnailCache.h

    # Sprint 1251-1260 — Industrial & Scientific Formats v2 (v34.5.0 "Arcturus-V")
    Decoders/DICOMWindowingPresets.h
    Decoders/FITSZScaleStretch.h
    Decoders/LASPointCloudRenderer.h
    Decoders/OMETIFFCompositor.h
    Decoders/MHAVolumeDecoder.h

    # Sprint 1261-1270 — CAD/BIM/EDA Formats (v34.6.0 "Arcturus-W")
    Decoders/DWGHeaderParser.h
    Decoders/STEPBoundingBoxExtractor.h
    Decoders/IFCEntityCounter.h
    Decoders/GerberLayerCompositor.h
    Decoders/KiCadNetlistParser.h

    # Sprint 1271-1280 — Performance Hardening + LTS Gate (v34.7.0 "Arcturus-X")
    Core/PerfRegressionGate.h
    Core/LTSBuildValidator.h
    Cache/CacheWarmupPreloader.h
    Pipeline/DecodeLatencyProfiler.h
    Utils/BenchmarkBaseline.h

    # Sprint 1281-1290 — Streaming & Cloud-Native Thumbnails (v35.0.0 "Vega")
    Pipeline/MultiStageThumbnailEmitter.h
    Core/CloudHydrationMonitor.h
    Cache/PartialDecodeStateCache.h
    Core/ThumbnailETagValidator.h
    Pipeline/AdaptiveFidelitySelector.h

    # Sprint 1291-1300 — Real-Time Collaboration & Live Edit Sync (v35.1.0 "Vega-R")
    Core/LiveSyncTokenManager.h
    Cache/CollaborativeCacheCoordinator.h
    Pipeline/ThumbnailDeltaEncoder.h
    Core/ConflictResolutionEngine.h
    Pipeline/RealTimePreviewPipeline.h
    # Sprint 1301-1310: Network-Aware Streaming Cache
    Core/NetworkTopologyProbe.h
    Cache/StreamingCacheTierPolicy.h
    Pipeline/BandwidthThrottleGuard.h
    Cache/RemoteFileManifestCache.h
    Cache/CachePrefetchScheduler.h
    # Sprint 1311-1320: Zero-Trust Thumbnail Security
    Core/ThumbnailManifestSigner.h
    Pipeline/ZeroTrustDecodeWorker.h
    Cache/TokenBoundCacheEntry.h
    Core/ThumbnailAuditLog.h
    Core/FIPSCryptoAdapter.h
    # Sprint 1321-1330: WebAssembly / Browser Extension Pipeline
    Core/WasmDecoderShim.h
    Pipeline/BrowserThumbnailBridge.h
    Pipeline/OffscreenCanvasRenderer.h
    Cache/WasmCacheAdapter.h
    Pipeline/ProgressiveThumbnailStream.h
    # Sprint 1331-1340: Cross-Device Preview Sync
    Core/DeviceSyncManifest.h
    Cache/CrossDeviceCacheSync.h
    Pipeline/ThumbnailPackFile.h
    Core/SyncConflictResolver.h
    Core/DeviceCapabilityAdvertiser.h

    # Cache management
    Cache/AdaptiveCacheBudgetManager.h
    Cache/CacheWarmingActivation.h
    Cache/CacheWarmingService.h
    Cache/MultiTenantCacheManager.h
    Cache/MultiTierCache.h
    Cache/PersistentDiskCache.h
    Cache/PipelineStateCacheV2.h
    Cache/ThumbnailCache.h
    Cache/CacheKeyGenerator.h
    Cache/PredictiveCacheEngine.h
    Cache/PSOPersistenceManager.h
    Cache/USNCacheInvalidation.h
    Cache/CacheMigrationEngine.h
    Cache/CacheEncryptionLayer.h
    Cache/CacheTelemetryCollector.h
    Cache/SubMillisecondCacheEngine.h
    Cache/PSOCachePersistence.h
    Cache/CacheBudgetAutoTuner.h
    Cache/CacheCoherencyManager.h
    Cache/CachePrewarmScheduler.h
    Cache/CacheDiagnostics.h
    Cache/CacheLinePadding.h

    # Plugin system
    Plugin/IPC/PluginIPCProtocol.h
    Plugin/IPC/SharedMemoryManager.h
    Plugin/IsolationModeSelector.h
    Plugin/PluginActivation.h
    Plugin/PluginDecoder.h
    Plugin/PluginHostBridge.h
    Plugin/PluginHostClient.h
    Plugin/PluginLifecycle.h
    Plugin/PluginLoaderV2.h
    Plugin/PluginManager.h
    Plugin/PluginPerformanceProfiler.h
    Plugin/PluginReferencePack.h
    Plugin/PluginSDKV2.h
    Plugin/PluginSecurity.h
    Plugin/PluginNamedPipeBridge.h
    Plugin/Security/JobObjectManager.h
    Plugin/PluginDependencyResolver.h
    Plugin/PluginCrashRecovery.h
    Plugin/PluginResourceLimiter.h
    Plugin/PluginCapabilityGuard.h
    Plugin/PluginVersionNegotiator.h
    Plugin/PluginAuditLog.h

    # AI/ML modules
    AI/AIThumbnailEnhancer.h
    AI/SceneUnderstandingEngine.h
    AI/SmartCropV2.h

    # Sprint 1061-1070 — Intelligent Workflow Automation (v31.3.0 "Achernar-T")
    AI/PredictivePregenEngine.h
    AI/ContentCategorizationEngine.h
    AI/ThumbnailQualityPredictor.h

    # Sprint 1071-1080 — Contextual Intelligence & Self-Healing (v31.4.0 "Achernar-U")
    AI/ContextualRenderingEngine.h
    AI/SmartThumbnailCompositor.h
    AI/FormatComplexityAnalyzer.h

    # Sprint 1081-1090 — Format Routing & Enhanced Accessibility (v31.5.0 "Achernar-V")
    AI/AdaptiveColorProfileManager.h
    AI/ThumbnailAccessibilityEngine.h

    # Codec modules
    Codec/CodecLoader.h
    Codec/CodecModuleSpecs.h
    Codec/FormatConverter.h
    Codec/ICodecModule.h
    Codec/LazyCodecManager.h

    # Cloud integration

    # Memory management
    Memory/ArchiveMemoryCompactor.h
    Memory/BitmapPool.h
    Memory/BufferPoolAllocator.h
    Memory/DecoderHotsetManager.h
    Memory/DirectoryFormatProfiler.h
    Memory/HotModeDirectoryEngine.h
    Memory/MemoryOptimizationEngine.h
    Memory/MemoryPressureControllerV2.h
    Memory/MemorySoakValidator.h
    Memory/SmartPointerPool.h
    Memory/MemoryDefragmenter.h
    Memory/MemoryFootprintOptimizerV2.h
    Memory/MemoryBudgetEnforcer.h
    Memory/AllocationTracker.h

    # Shell integration

    # Utilities
    Utils/CIPipeline.h
    Utils/CodeCoverage.h
    Utils/FuzzingEngine.h
    Utils/InstallerManager.h
    Utils/MemorySafety.h
    Utils/QualityGates.h
    Utils/SecurityCompliance.h
    Utils/SecureAllocator.h
    Utils/InputValidator.h
    Utils/TestFramework.h
    Utils/WindowsCompat.h
    # Individual utility headers
    Utils/AuditLogger.h
    Utils/AutoDocGenerator.h
    Utils/AutoUpdateEngine.h
    Utils/CIValidator.h
    Utils/CodeCoverageIntegration.h
    Utils/CommandLineInterface.h
    Utils/ConfigMigrationEngine.h
    Utils/DiagnosticDashboard.h
    Utils/EnterpriseDeploymentManager.h
    Utils/EXIFOrientation.h
    Utils/FileHashEngine.h

    Utils/HardwareCapabilities.h
    Utils/LogRotationEngine.h
    Utils/MalformedInputHandler.h
    Utils/MemoryMappedFile.h
    Utils/MemorySafetyIntegration.h
    Utils/MSIXPackageManager.h
    Utils/NetworkDiagnostics.h
    Utils/NotificationEngine.h
    Utils/PerformanceProfiler.h
    Utils/PortableModeManager.h
    Utils/PythonSDK.h
    Utils/SIMDScaler.h
    Utils/SmallObjectPool.h
    Utils/TestInfrastructure.h
    Utils/TestSuiteExpansion.h
    Utils/ValidationHelpers.h
    Utils/VersionNormalization.h
    Utils/DecoderCircuitBreaker.h
    Utils/PerceptualHashing.h
    Utils/RemoteDesktopOptimizer.h
    Utils/PowerThrottleManager.h
    Utils/IntegrationTestFrameworkV2.h
    Utils/IntegrationTestOrchestrator.h
    Utils/ContinuousFuzzOrchestrator.h
    Utils/StaticAnalysisCIGate.h
    Utils/Win11Integration.h
    Utils/RegistrySnapshotManager.h
    Utils/ConfigDriftDetector.h
    Utils/OperationalReadinessChecker.h

    # Sprint 47-48: CI/CD Pipeline + Build Validation
    Utils/BuildValidator.h
    Utils/CITestReporter.h
    Utils/EnvironmentProbe.h

    # Sprint 591-593
    Utils/InstallerLifecycleManager.h
    Utils/DocumentationGenerator.h

    # Release gate (unified)
    Utils/ReleaseGate.h

    # Zenith Feature Headers
    Core/HDRToneMappingPipeline.h
    Core/ColorSpaceEngine.h
    Core/GPUTextureCompressionPipeline.h
    Core/NestedArchivePreview.h
    Core/MultiPageNavigator.h
    Core/AnimatedFormatController.h
    Core/MetadataExtractionPipeline.h
    Core/ContentAwareThumbnailSelector.h
    Core/FaceDetectionOrientation.h
    Core/DocumentLayoutAnalyzer.h
    Core/VisualSimilarityIndex.h
    Core/SmartQualityPredictor.h
    Pipeline/LockFreeDecodePipeline.h
    Pipeline/MemoryMappedIOOptimizer.h
    GPU/GPUTextureAtlasManager.h
    Pipeline/PredictivePrefetchEngine.h
    Pipeline/ThreadPoolOptimizer.h
    Core/WindowsSearchProtocol.h
    Core/VirtualFilesystemAbstraction.h
    Core/RoleBasedFormatPolicy.h
    Core/ContentInspectionGateway.h
    Core/CertificateTrustValidator.h
    Core/EncryptedFormatHandler.h
    Core/SelfHealingDecoder.h
    Core/PerformanceAnomalyDetector.h
    Core/PerformanceDashboard.h
    Core/DiagnosticsCollector.h
    Core/DiagnosticReportGeneratorV2.h
    Core/HealthCheckEndpoint.h
    Core/PreviewTooltipRenderer.h
    Core/QuickActionsOverlay.h
    Core/UsageAnalyticsDashboard.h
    Core/SystemResourceMonitor.h
    Core/RustFFIBridge.h
    Core/DirectStorage12Integration.h
    Core/NeuralTextureCompression.h
    Core/QuantumReadyHashPipeline.h
    GPU/WebGPUThumbnailRenderer.h
    Core/WASMDecoderSandbox.h
    Core/LivePreviewStreamingProtocol.h
    Core/DragDropThumbnailPreview.h
    Core/TaskbarPreviewManager.h
    Core/SearchFederatedProvider.h

    # Batch 8 (Sprints 430-443)
    Core/DirectShowThumbnailBridge.h
    Pipeline/AsyncIOCompletionEngine.h
    Core/ExifOrientationFixer.h
    Memory/VirtualAllocOptimizer.h
    Core/FileAssociationManager.h
    GPU/DX12FenceManager.h
    Core/WindowsSearchIntegration.h

    # Sprints 594-643 — New Decoders, Pipeline, Core, AI features
    # Batch 1: Advanced Decoders (594-598)
    Decoders/APNGDecoder.h
    Decoders/FLIFDecoder.h
    Decoders/BPGDecoder.h
    Decoders/RGBEDecoder.h
    Decoders/WebP2Decoder.h
    # Batch 2: Document & Text (599-603)
    Decoders/MarkdownPreviewRenderer.h
    Decoders/SourceCodeThumbnail.h
    Decoders/RTFDecoder.h
    Decoders/LaTeXPreviewDecoder.h
    Decoders/StructuredDataVisualizer.h
    # Batch 3: Archive & Compression (604-608)
    Decoders/ZstdFrameDecoder.h
    Decoders/BrotliStreamInspector.h
    Decoders/LZ4FrameDecoder.h
    Decoders/XZStreamDecoder.h
    Decoders/SnappyFrameDecoder.h
    # Batch 4: 3D & CAD (609-613)
    Decoders/PLYPointCloudDecoder.h
    Decoders/OBJMeshDecoder.h
    Decoders/STLMeshDecoder.h
    Decoders/COLLADADecoder.h
    Decoders/FBXInspector.h
    # Batch 5: Media Enhancement (614-618)
    Decoders/MIDIVisualizer.h
    Decoders/WaveformGenerator.h
    Decoders/SpectrogramRenderer.h
    Decoders/VideoTimelineStrip.h
    Decoders/SubtitlePreviewDecoder.h
    # Batch 6: Enterprise & Security (619-623)
    Decoders/CertificateViewer.h
    Decoders/RegistryExportViewer.h
    Decoders/ShortcutInspector.h
    Decoders/MSIPackageInspector.h
    Decoders/DiskImagePreview.h
    # Batch 7: Performance Optimization (624-628)
    Pipeline/ThreadLocalBufferPool.h
    Pipeline/DecodeMemoizationEngine.h
    Pipeline/AsyncPrefetchQueue.h
    Pipeline/MemoryMappedDecodePath.h
    # Batch 8: Quality & Observability (629-633)
    Core/DecodeLatencyHistogram.h
    Core/HealthScoreAggregator.h
    Core/PerformanceRegressionDetector.h
    # Batch 9: Smart Features (634-638)
    AI/ImageComplexityAnalyzer.h
    AI/DecodeStrategyOptimizer.h
    # Batch 10: Platform & Integration (639-643)
    Core/ClipboardMonitorIntegration.h
    Core/ExplorerStatusBarProvider.h
    Core/FileSummaryTooltipGenerator.h
    Core/BatchProgressReporter.h
    # Sprint 49-50: Dark Mode & Theme Rendering
    Core/ThemeAwareOverlay.h
    Core/ContrastValidator.h
    Core/SystemThemeMonitor.h
    Core/AdaptiveIconRenderer.h
    # Sprint 51-52: Decode Pipeline Enhancement
    Core/StreamingDecodeEngine.h
    Core/MultiPagePreview.h
    # Sprint 53-54: Memory Management
    Memory/MemoryArenaAllocator.h
    Memory/LargePageAllocator.h
    # Sprint 55-56: Pipeline Scheduling
    Pipeline/PipelineStageProfiler.h
    Pipeline/AdaptiveTimeoutController.h
    Pipeline/PipelineReplayRecorder.h
    Pipeline/DecodePriorityQueue.h
    # Sprint 57-58: Cache Intelligence
    Cache/CacheCompressionEngine.h
    Cache/CacheIntegrityVerifier.h
    # Sprint 59-60: GPU Enhancement
    GPU/GPUShaderCompiler.h
    GPU/GPUOccupancyOptimizer.h
    GPU/GPUDebugLayer.h
    GPU/GPUMemoryPoolManager.h
    GPU/GPUAsyncCopyEngine.h
    # Sprint 61-62: Enterprise & Deployment
    Utils/EnterpriseTelemetryRouter.h
    Utils/PolicyComplianceValidator.h
    Utils/SilentUpdateOrchestrator.h
    Utils/DiagnosticBundleExporter.h
    Utils/DeploymentHealthChecker.h
    # Sprint 63-64: Plugin Ecosystem
    # Sprint 65-66: Quality & Observability
    Core/StructuredDiagnosticLogger.h
    Core/ResourceLeakTracker.h
    Core/HealthCheckOrchestrator.h
    # Sprint 67-68: Final Polish
    Core/ShellIntegrationValidator.h
    Core/GracefulDegradationController.h
    Core/UserPreferenceEngine.h
    Core/StartupOptimizer.h
    Core/CrashRecoveryEngine.h
    Core/HDRToneMapper.h
    Core/ColorSpaceConverter.h
    Core/ExifOrientationHandler.h
    Core/IconBadgeRenderer.h

    # Sprint 69-88: Beyond Zenith — Next-Generation Features
    Core/NeuralThumbnailUpscaler.h
    Core/SemanticFileClassifier.h
    Core/ContentAwareCompositor.h
    Core/HDRDisplayMapper.h
    Core/VolumetricPreviewEngine.h
    Core/RealTimeCodecTranscoder.h
    Core/DistributedRenderEngine.h
    Core/QuantumSafeHasher.h
    Core/AdaptiveLODEngine.h

    # Beyond Zenith — Decoders
    Decoders/OpenEXRDecoder.h
    Decoders/VDBVolumeDecoder.h
    Decoders/AlembicDecoder.h
    Decoders/MaterialXDecoder.h
    Decoders/PointCloudStreamDecoder.h
    Decoders/GeospatialTileDecoder.h
    Decoders/HDRIEnvironmentDecoder.h
    Decoders/NeuralRadianceDecoder.h
    Decoders/NotationDecoder.h
    Decoders/PCBLayoutDecoder.h

    # Beyond Zenith — GPU
    GPU/GPUTensorAccelerator.h
    GPU/VulkanRayTracingPreview.h
    GPU/DirectStorageIntegration.h
    GPU/GPUDecompressionEngine.h
    GPU/ShaderModelValidator.h

    # Beyond Zenith — Pipeline
    Pipeline/StreamingMipChain.h
    Pipeline/AdaptiveBatchCoalescer.h
    Pipeline/ThumbnailMorphTransition.h

    # Beyond Zenith — Cache
    Cache/SemanticCacheIndex.h

    # Beyond Zenith — Memory
    Memory/MemoryCompressionEngine.h
    Memory/ZeroCostAbstractionLayer.h
    Memory/MemoryBandwidthProfiler.h

    # Beyond Zenith — AI/ML
    AI/FaceDetectionThumbnail.h
    AI/ObjectSaliencyMapper.h

    # Beyond Zenith — Plugin Ecosystem
    Plugin/PluginTelemetryBridge.h
    Plugin/PluginHostIPC.h
    Plugin/PluginRuntimeValidation.h

    # Sprint 349-393: Enhancement Plan V15 — Production Polish
    # Category A/B: Critical Fixes & Architecture (Core)
    Core/MuPDFRenderer.h
    Core/CRTLinkageValidator.h
    Core/LENSTypeRegistry.h
    Core/RegistryBatchHandler.h
    Core/PluginHostSupervisor.h
    Core/OpenJPEGRenderer.h
    Core/FreeTypeRenderer.h
    Core/FFmpegExtractor.h
    Core/InstructionSetRouter.h
    Core/OwnerDrawThemeEngine.h
    Core/AVX2ScaleKernel.h
    Core/NEONScaleKernel.h

    # Category B/C: GPU Shader Library & Rendering
    GPU/ShaderLibraryManager.h
    GPU/LanczosGPUScaler.h
    GPU/BicubicResizeKernel.h
    GPU/ACESTonemapKernel.h
    GPU/ColorConvertKernel.h

    # Category B/F: Pipeline Activation & Integration
    Pipeline/ZeroCopyActivator.h
    Pipeline/ParallelIOScheduler.h
    Pipeline/FolderWatchPredictor.h
    Pipeline/ShellPropertyProvider.h
    Pipeline/FFmpegCodecRouter.h

    # Category B/F: Memory Optimization
    Memory/SIMDMemoryAligner.h
    Memory/MemoryCompactionScheduler.h

    # Category F: Cache Performance

    # Category E: AI/ML Production
    AI/DirectMLInferenceEngine.h
    AI/ONNXModelLoader.h

    # Category D: GUI/UX Utilities
    Utils/DPIScalingManager.h
    Utils/HighContrastAdapter.h
    Utils/FormatHealthIndicator.h
    Utils/SettingsSerializer.h
    Utils/LivePerformanceTracker.h

    # Category B/D: Plugin Infrastructure

    # Category D: Shell & Decoder Enhancements
    Decoders/JPEG2000DecoderV2.h
    Decoders/UniversalVideoDecoder.h

    # Sprint 394: New Feature Headers
    Core/ColdStartOptimizer.h
    Pipeline/DecodeCancellationEngine.h
    Pipeline/RequestDeduplicator.h
    GPU/GPUPowerStateManager.h
    GPU/GPUWorkgroupOptimizer.h
    Memory/CopyOnWriteBufferPool.h
    Memory/MemoryMappedThumbnailAtlas.h

    # Sprint 395+: Enhancement Plan V15 — 50 New Feature Headers
    # Core enhancements
    Core/GlobalShortcutManager.h
    Core/FileChangeThrottler.h
    Core/SmartThumbnailPrioritizer.h
    Core/ExplorerIntegrationMonitor.h
    Core/FileConcurrencyGuard.h
    Core/ResourceThrottlePolicy.h

    # Sprint 9-14 (v15.3.0 "Zenith-T") — Resilience & Hardening
    Core/DecodeInputValidator.h
    Core/DecodeErrorCategory.h
    Core/GracefulDegradation.h
    Core/ArchiveSecurityValidator.h
    Core/ErrorCategorizationEngine.h
    Core/DecoderPerformanceCounters.h

    # Pipeline enhancements
    Pipeline/LazyDecodeInitializer.h
    Pipeline/ChunkedDecodeEngine.h
    Pipeline/DecodeProfilingHarness.h
    Pipeline/ThumbnailMergeEngine.h
    Pipeline/ExplorerQueryOptimizer.h

    # GPU enhancements
    GPU/GPUDeviceSelector.h
    GPU/GPUFeatureProbe.h
    GPU/AdaptiveShaderSelection.h
    GPU/GPUErrorRecovery.h
    GPU/ComputeShaderProfiler.h

    # Cache enhancements
    Cache/CachePartitionManager.h
    Cache/CacheGarbageCollector.h

    # Memory enhancements
    Memory/StackAllocator.h
    Memory/MemoryWatermarkTracker.h
    Memory/PoolAllocatorMetrics.h
    Memory/ScopedMemoryBudget.h
    Memory/ThreadLocalPoolAllocator.h

    # AI/ML enhancements
    AI/FileTypePredictor.h

    # Plugin enhancements

    # Utils enhancements
    Utils/PerformanceReportGenerator.h
    Utils/SystemInfoCollector.h
    Utils/RegistryRepairTool.h
    Utils/LogAnalyzer.h

    # Decoder enhancements
    Decoders/WICFallbackDecoder.h
    Decoders/ArchivePeekDecoder.h
    Decoders/ContainerInspector.h
    Decoders/MetadataOnlyDecoder.h
    Decoders/EfficientResizeDecoder.h
    Decoders/MultiFormatBatchDecoder.h
    Decoders/CacheFriendlyDecoder.h

    # Sprint 396: Enhancement Plan V15 — 50 New Feature Headers
    # Core enhancements
    Core/WindowsNotificationManager.h
    Core/FileSystemJournalReader.h
    Core/ProcessIsolationBroker.h
    Core/IncrementalDecodeEngine.h
    Core/ExplorerNavigationMonitor.h
    Core/DecoderPoolManager.h
    Core/FileTypeAssociationBroker.h
    # Pipeline enhancements
    Pipeline/RateLimitedDecodeQueue.h
    Pipeline/PipelineFusionOptimizer.h
    Pipeline/ConcurrentDecodeBarrier.h
    Pipeline/FormatSpecificPipeline.h
    Pipeline/ThumbnailInvalidationTracker.h
    Pipeline/PipelineTelemetrySink.h
    Pipeline/DecodeWorkDistributor.h
    # GPU enhancements
    GPU/GPUVendorCapabilityMap.h
    GPU/GPUBatchSubmitter.h
    GPU/GPUFenceOrchestrator.h
    GPU/ShaderPermutationManager.h
    GPU/GPUThermalMonitor.h
    GPU/GPUResourceLeakDetector.h
    # Cache enhancements
    Cache/CacheReplicationEngine.h
    # Memory enhancements
    Memory/MemoryCompactionEngine.h
    Memory/AllocatorBenchmark.h
    Memory/WorkingSetOptimizer.h
    # AI enhancements
    # Plugin enhancements
    Plugin/PluginStatePersistence.h
    # Decoder enhancements
    Decoders/ClipboardImageDecoder.h
    Decoders/ScreenshotAnalyzer.h
    Decoders/StreamingVideoDecoder.h
    Decoders/CompoundDocumentDecoder.h
    # Utils enhancements
    Utils/WindowsEventLogWriter.h
    Utils/BuildArtifactValidator.h
    # Sprint 397: Enhancement Plan V15 — Phase 2 Headers
    # Utils enhancements
    Utils/VersionMatrixValidator.h
    Utils/OrphanedFileScanner.h
    Utils/LinkageAuditEngine.h
    Utils/DependencyGraphValidator.h
    Utils/CompilerWarningTracker.h
    Utils/LibraryHashVerifier.h
    Utils/LibraryCompatMatrix.h
    Utils/TestCorpusIndexer.h
    Utils/DocCoverageAnalyzer.h
    Utils/LatencyBreakdownReporter.h
    # Core enhancements
    Core/ArchiveTypeExtractor.h
    Core/ArchiveStreamFactory.h
    Core/PropertyStoreCache.h
    Core/RegistryWriteCoalescer.h
    Core/CollapsibleSectionModel.h
    Core/SettingsProfileManager.h
    Core/PerformanceMetricAggregator.h
    Core/OwnerDrawButtonRenderer.h
    Core/SystemTrayContextProvider.h
    Core/WorkerPoolElasticScaler.h
    Core/SIMDInstructionAuditor.h
    # Pipeline enhancements
    Pipeline/NamedPipeMessageBus.h
    Pipeline/ZeroCopyBufferChain.h
    Pipeline/ParallelDirectoryScanner.h
    Pipeline/BatchDemuxOptimizer.h
    Pipeline/AccessPatternRecorder.h
    # GPU enhancements
    GPU/ShaderWarmupEngine.h
    GPU/LanczosKernelOptimizer.h
    GPU/HDRMetadataParser.h
    GPU/GPUCommandBundler.h
    GPU/GPUTimestampProfiler.h
    # Cache enhancements
    # Memory enhancements
    Memory/BitmapPoolPartitioner.h
    Memory/AddressSpaceReserver.h
    Memory/CommitChargeTracker.h
    # Decoder enhancements
    Decoders/FreeTypeLayoutEngine.h
    Decoders/FFmpegDemuxerBridge.h
    Decoders/OpenJPEGStreamDecoder.h
    Decoders/MemoryMappedBitmapReader.h
    # Plugin enhancements
    Plugin/PluginHealthReporter.h
    # Sprint 398 — EP V15 Phase 3
    # Core enhancements
    Core/FileVersionTracker.h
    Core/ContextMenuFormatter.h
    Core/ProgressReportAggregator.h
    Core/OutputColorSpaceMapper.h
    Core/BatchAbortController.h
    Core/ExtensionConflictDetector.h
    Core/DecoderFallbackChain.h
    Core/AsyncRegistrySnapshot.h
    # Pipeline enhancements
    Pipeline/AdaptiveChunkSizer.h
    Pipeline/StreamingMipGenerator.h
    Pipeline/PipelineBackpressureValve.h
    Pipeline/ExtractorPoolBalancer.h
    Pipeline/DecodeQueueInspector.h
    Pipeline/IOCompletionPortBridge.h
    Pipeline/PipelineTelemetryEmitter.h
    Pipeline/DecodeWatermarkStamper.h
    # GPU enhancements
    GPU/ShaderVariantSelector.h
    GPU/TextureSamplerCache.h
    GPU/GPUBarrierOptimizer.h
    GPU/ShaderCompileListener.h
    GPU/GPUVendorQuirksTable.h
    # Cache enhancements
    Cache/CacheSerializationCodec.h
    # Memory enhancements
    Memory/VirtualAllocTracker.h
    Memory/HeapFragmentationProbe.h
    Memory/MemoryBudgetNegotiator.h
    Memory/MemoryTrimPolicy.h
    # Decoder enhancements
    Decoders/ExifThumbnailExtractor.h
    Decoders/ProgressiveDecodeStreamer.h
    Decoders/DecoderCapabilityProbe.h
    Decoders/MultiFrameDecodeRouter.h
    Decoders/ColorProfileValidator.h
    # Plugin enhancements
    Plugin/PluginEventBus.h
    Plugin/PluginSchemaValidator.h
    Plugin/PluginFeatureToggle.h
    # Utils enhancements
    Utils/StartupTimingProfiler.h
    Utils/FeatureFlagRegistry.h
    Utils/DeploymentVerifier.h
    # Sprint 399 — EP V15 Phase 4
    # Core
    Core/FileSystemWatchdog.h
    Core/ContextualPreviewEngine.h
    Core/FileSizeEstimator.h
    Core/ConcurrentExtractScheduler.h
    # Pipeline
    Pipeline/AdaptivePipelineScheduler.h
    Pipeline/StreamingThumbnailEmitter.h
    Pipeline/PipelineLoadShedder.h
    Pipeline/DecodeThroughputRegulator.h
    Pipeline/MemoryMappedPipelineStage.h
    Pipeline/PipelineLatencyTracker.h
    # GPU
    GPU/GPUTextureMipChain.h
    GPU/ShaderHotReloader.h
    GPU/GPUOccupancyCalculator.h
    GPU/ComputeDispatchOptimizer.h
    GPU/GPUFormatConverter.h
    GPU/GPUThumbnailCompositor.h
    # Cache
    # Memory
    Memory/BitmapMemoryRecycler.h
    Memory/MemoryPressureResponder.h
    Memory/ZeroFragmentationHeap.h
    Memory/MemoryAllocationTracer.h
    Memory/NUMANodeAllocator.h
    # Decoders
    Decoders/AnimatedFormatDecoder.h
    Decoders/EmbeddedThumbnailDecoder.h
    Decoders/MultipageDocumentDecoder.h
    Decoders/SidecarMetadataDecoder.h
    Decoders/ContainerPreviewDecoder.h
    # AI
    # Plugin
    # Sprint 261-270 (v20.6.0 "Quasar-W") — Plugin Marketplace v2
    Plugin/PluginInstaller.h
    Plugin/PluginPackageManifest.h
    Plugin/PluginSearchIndex.h
    # Sprint 271-280 (v20.7.0 "Quasar-X") — Observability v2
    Core/DiagnosticsConsole.h
    Core/LatencyBudgetManager.h
    Core/NetworkAwarePrefetcher.h
    Utils/CrashReporter.h
    Utils/UsageStats.h
    Utils/FeatureFlagManager.h
    Utils/AutoUpdateManager.h
    # Sprint 281-290 (v21.0.0 "Rigel") — Format Expansion III
    Decoders/AVIFSequenceDecoder.h
    Decoders/HEIFBurstDecoder.h
    Decoders/JXLAnimationDecoder.h
    Decoders/PSDLayerDecoder.h
    Decoders/SVGRasterizer.h
    Decoders/TIFFMultiPageDecoder.h
    Decoders/WebPAnimationDecoder.h
    # Sprint 291-300 (v21.1.0 "Rigel-R") — Advanced GPU Compute v2
    GPU/D3D11DPIAdapter.h
    GPU/DirectStorageLoader.h
    GPU/VulkanComputeAccelerator.h
    GPU/VulkanComputeDecoder.h
    Core/ThreadPoolV2.h
    Core/ZeroCopyTextureUploader.h
    Core/BatchDecodeScheduler.h
    Core/SIMDImageProcessor.h
    # Sprint 301-310 (v21.2.0 "Rigel-S") — Enterprise Policy v2
    Core/EnterprisePolicyEngineV2.h
    Core/ACLManager.h
    Core/AuditLogger.h
    Core/FIPSComplianceMode.h
    Core/IntegrityVerifier.h
    Core/PrivilegeElevationGuard.h
    Core/SandboxEscapeGuard.h
    Core/CodeIntegrityChecker.h
    # Sprint 311-320 (v21.3.0 "Rigel-T") — Storage & Caching v3
    Core/PredictivePrefetcher.h
    Utils/MemoryMappedLoader.h
    Utils/ActivationService.h
    Utils/FeatureCompatMatrix.h
    # Sprint 321-330 (v22.0.0 "Sirius" MAJOR) — Cross-Platform Foundation
    Core/ColorBlindnessFilter.h
    Core/DisplayColorProfile.h
    Core/FeedbackManager.h
    Core/KeyboardNavigationHandler.h
    Core/MonitorConfigWatcher.h
    # Sprint 331-340 (v22.1.0 "Sirius-R") — Performance Profiling v2
    Core/ResponsiveLayoutManager.h
    Core/SecureDecodeContext.h
    Core/SecureStringPool.h
    Core/StackGuardPolicy.h
    Core/InputValidator.h
    Core/CodecPlatformV2.h
    Core/NetworkTrustManager.h
    # Sprint 341-350 (v22.2.0 "Sirius-S") — Security & Audit v3
    Utils/LicenseManager.h
    Utils/LocalizationValidator.h
    Utils/StoreReadinessChecker.h
    Utils/CertificatePinner.h
    Utils/ColorBlindFilter.h
    Utils/FirstRunExperience.h
    AI/BlurDetectionFilter.h
    # Sprint 351-360 (v22.3.0 "Sirius-T") — AI Inference Pipeline v2
    AI/AIModelRegistry.h
    AI/AIPerformanceProfiler.h
    AI/AIThumbnailPipeline.h
    AI/AIUpscaler.h
    # Sprint 361-370 — Advanced Scheduling & Concurrency v2 (v22.4.0 Sirius-U)
    Core/LockFreeMPMCQueue.h
    Core/CPUAffinityRouter.h
    Core/RealtimePriorityEngine.h
    Memory/HazardPointerReclaimer.h
    Pipeline/AdaptiveConcurrencyLimiter.h
    Core/CooperativeTaskScheduler.h
    Core/ThreadLocalContextPool.h
    # Sprint 371-380 — Format Expansion IV (v22.5.0 Sirius-V)
    Decoders/QOIRDecoder.h
    Decoders/JNGDecoder.h
    Decoders/JBIG2Decoder.h
    Decoders/TIFFMultiFrameDecoderV2.h
    Decoders/ILBMDecoder.h
    Decoders/SunRasterDecoder.h
    Decoders/JPEGXTDecoder.h
    # Sprint 381-390 — Windows Shell Integration v2 (v22.6.0 Sirius-W)
    Core/NamespaceWalkEngine.h
    Core/ExplorerColumnProviderV2.h
    Core/SearchIndexBridge.h
    Core/DragDropPreviewEngine.h
    # Sprint 391-400 — DevOps & Quality Engineering v2 (v22.7.0 Sirius-X)
    Utils/MutationTestingEngine.h
    Utils/PropertyBaseTestEngine.h
    Utils/ReproducibleBuildVerifierV2.h
    Utils/RegressionFingerprintEngine.h
    Utils/CycloneDXSBOMGenerator.h
    Utils/BuildTimingAnalytics.h
    Utils/ArtifactIntegrityMonitor.h
    Utils/CIEnvironmentValidator.h
    # Sprint 401-410 — Reactive Pipeline Architecture (v23.0.0 Vega)
    Pipeline/ThumbnailEventStore.h
    Pipeline/CQRSThumbnailPipeline.h
    Pipeline/ReactiveStreamEngine.h
    Pipeline/ThumbnailSagaOrchestrator.h
    Pipeline/SnapshotStoreEngine.h
    Pipeline/DomainEventBus.h
    Pipeline/ReactiveAPIGateway.h
    # Sprint 411-420 — GPU Acceleration v3 (v23.1.0 Vega-R)
    GPU/CUDATextureDecoder.h
    GPU/HIPComputeBackend.h
    GPU/MultiGPULoadBalancerV3.h
    GPU/GPUTextureAtlasBuilder.h
    GPU/GPUResourceAliasingManager.h
    GPU/AsyncDMACopyEngine.h
    GPU/GPUMemoryDefragmenterV2.h
    GPU/GPUThumbnailAtlasManager.h
    # Sprint 421-430 — Plugin Ecosystem v3 (v23.2.0 Vega-S)
    # Sprint 431-440 — Memory Optimization v3 (v23.3.0 Vega-T)
    Memory/MemoryOptimizationV3.h
    Memory/MemoryMappedBTree.h
    Memory/NVMeMemoryTier.h
    Memory/ECCErrorDetector.h
    Memory/PressureForecaster.h
    Memory/SharedMemoryRegionManager.h
    # Sprint 441-450 — Smart Cache v4 (v23.4.0 Vega-U)
    Cache/AIEvictionPolicyEngine.h
    # Sprint 451-460 — CLI & Automation v2 (v23.5.0 Vega-V)
    # Utils
    Utils/DiagnosticBundleCollector.h
    Utils/RegressionTestRunner.h

    # Sprint 561-570 — WASM Plugin Sandbox (v25.0.0 "Rigel" — delivered v25.1.0)

    # Sprint 571-580 — Neural Format Intelligence (v25.1.0 "Rigel-R")
    Core/SelfExpandingFormatRegistry.h
    Core/FormatDetectionReport.h
    Core/SyntheticDecoderGenerator.h

    # Sprint 581-590 — NPU & Heterogeneous Compute (v25.2.0 "Rigel-S")
    GPU/IntelNPUBackend.h
    GPU/HexagonDSPBackend.h
    GPU/ONNXEPRouter.h
    GPU/HardwareCapabilityProfiler.h
    Pipeline/PowerAwareScheduler.h
    Memory/NPUMemoryPool.h
    GPU/ARM64DecodeBackend.h

    # Sprint 591-600 — VCS Integration (v25.3.0 "Rigel-T")
    Core/GitStatusOverlay.h
    Core/GitBlameHeatmapOverlay.h
    Core/VCSBadgeAdapter.h
    Core/GitDiffThumbnail.h
    Core/GitLFSResolver.h
    Core/CommitBadgeCompositor.h
    Core/MergeConflictOverlay.h
    # Sprint 601-610 — Self-Healing & Adaptive Recovery (v25.4.0 "Rigel-U")
    Core/DecoderCrashPredictor.h
    Core/DecoderQuarantineManager.h
    Core/DecoderIncidentReporter.h
    Core/AdaptiveTimeoutTuner.h
    Core/HeapCorruptionSentinel.h
    Pipeline/RetryPolicyEngine.h
    Utils/COMSelfRepairValidator.h
    Utils/BootIntegritySelfTest.h
    # Sprint 611-620 — Multi-Instance & Virtual Desktop (v25.5.0 "Rigel-V")
    Core/PerMonitorDPISelectorV2.h
    Core/VirtualDesktopAwareness.h
    Core/WTSSessionIsolation.h
    Core/TabbedExplorerSync.h
    Memory/CrossSessionThumbnailPool.h
    Core/InstanceRegistry.h
    Pipeline/ForegroundPriorityInheritance.h
    Pipeline/CrossInstanceLoadBalancer.h
    # Sprint 621-630 — Collaborative Annotations & Sharing (v25.6.0 "Rigel-W")
    Core/AnnotationStore.h
    Core/AnnotationOverlayRenderer.h
    Utils/CollabWebhookBridge.h
    Core/SharedCollectionBuilder.h
    Core/AnnotationDiffViewer.h
    Utils/AnnotationExporter.h
    Utils/CollabCloudSync.h
    Core/AnnotationSchemaMigrator.h
    # Sprint 631-640 — Protocol Surface & API Ecosystem (v25.7.0 "Rigel-X")
    Utils/GRPCThumbnailService.h
    Utils/RESTThumbnailServer.h
    Utils/GraphQLQueryEngine.h
    Utils/WebSocketPushChannel.h
    Utils/OpenAPISpecGenerator.h
    Utils/SDKBindingsGenerator.h
    Utils/OAuthTokenValidator.h
    Utils/APIRateLimiter.h
    # Sprint 641-650 — Post-Quantum Security (v26.0.0 "Canopus")
    Utils/MLKEMKeyEncapsulator.h
    Utils/SLHDSASignatureVerifier.h
    Core/HybridTLSIPCChannel.h
    Utils/PQAuditTrail.h
    Utils/QuantumSafeKeyRotator.h
    Utils/FIPS140CryptoBoundary.h
    Utils/CertificateMigrationTool.h
    Utils/CryptoAgilityEngine.h
    # Sprint 651-660 — Windows Next-Gen Shell Integration (v26.1.0 "Canopus-R")
    Core/WinRTThumbnailBridge.h
    Core/AppContainerIsolation.h
    Core/WinFSMetadataStore.h
    Core/WindowsSearchV3Bridge.h
    Utils/SmartAppControlPolicy.h
    Core/MSIXStreamingPrewarmer.h
    Utils/WindowsHelloAuthBridge.h
    # Sprint 661-670 — Immersive 3D Preview Engine (v26.2.0 "Canopus-S")
    Core/ImmersivePreviewRenderer.h
    Core/VolumetricThumbnailEngine.h
    GPU/RealtimeLightingSimulator.h
    Core/HolographicProjectionEngine.h
    Core/MeshLODGeneratorV2.h
    Core/AnimationPreviewScrubber.h
    Core/MaterialPreviewEngine.h
    GPU/GPUPathTracerPreview.h
    # Sprint 671-680 — Real-Time Collaboration (v26.3.0 "Canopus-T")
    Core/CollaborationPresenceEngine.h
    Core/LiveAnnotationBroadcaster.h
    Core/SharedViewStateProtocol.h
    Core/ConflictResolutionMerger.h
    Core/PresenceAvatarRenderer.h
    Core/SessionReplayEngine.h
    Core/CollaborationTelemetryHub.h
    # Sprint 681-690 — Adaptive Performance Governor v2 (v26.4.0 "Canopus-U")
    Core/AdaptivePerformanceGovernorV2.h
    Memory/ThermalAwareMemoryScheduler.h
    GPU/WorkloadBalancerV2.h
    Core/PowerBudgetController.h
    Core/BackgroundIntelligenceService.h
    Core/QoSThrottleEngine.h
    Memory/SmartPrefetchEngine.h
    Core/FrameRateSynchronizer.h
    # Sprint 691-700 — Global I18n & Accessibility v3 (v26.5.0 "Canopus-V")
    Utils/I18nRuntimeEngine.h
    Utils/BiDiTextLayoutEngine.h
    Utils/AccessibilityNavigatorV3.h
    Utils/AccessibilityAuditPipeline.h
    Utils/ScreenReaderBridgeV2.h
    Utils/LocaleFallbackResolver.h
    Utils/A11yColorContrastEngine.h
    Utils/KeyboardNavigationMapV2.h

    # Sprint 701-710 — Extended Reality (XR) Thumbnails (v26.6.0 "Canopus-W")
    Decoders/OpenXRAssetDecoder.h
    Decoders/USDADecoder.h
    Core/XRSpatialPreviewEngine.h
    Core/ARMarkerDetectionEngine.h
    GPU/StereoscopicRenderPipeline.h
    Core/PointCloudVisualizerV2.h
    Decoders/NerfDecoder.h
    Core/XRMetadataExtractor.h
    # Sprint 711-720 — Privacy-Preserving Analytics (v26.7.0 "Canopus-X")
    Core/DifferentialPrivacyEngine.h
    Core/LocalDataAggregator.h
    Core/AnonymizationPipelineV2.h
    Core/PrivacyConsentManager.h
    Core/SecureEnclaveAnalytics.h
    Utils/GDPRComplianceEngine.h
    Core/PrivacyAuditLogger.h
    # Sprint 721-730 — Federated AI Pipeline (v27.0.0 "Sirius" MAJOR)
    # Sprint 731-740 — Streaming & Live Media Preview (v27.1.0 "Sirius-R")
    Decoders/LiveStreamDecoder.h
    Decoders/WebRTCThumbnailCapture.h
    Core/StreamingBufferOrchestrator.h
    Core/AdaptiveBitrateSelector.h
    Core/MediaTimelineRenderer.h
    Decoders/DASHStreamDecoder.h
    Core/LiveThumbnailPoller.h
    GPU/VideoTextureStreamEngine.h
    # Sprint 741-750 — Distributed Rendering Cluster (v27.2.0 "Sirius-S")
    Core/RenderClusterManager.h
    Core/RenderJobScheduler.h
    Core/NodeHealthMonitor.h
    Core/RenderResultAggregator.h
    Core/ClusterAutoScaler.h
    Core/SecureClusterChannel.h
    Core/ClusterObservabilityBus.h
    # Sprint 751-760 — Universal Plugin SDK v3 (v27.3.0 "Sirius-T")
    Plugin/PluginSandboxV3.h
    Plugin/PluginCompatibilityShimV3.h
    # Sprint 761-770 — Predictive Pre-Generation Engine (v27.4.0 "Sirius-U")
    Core/FolderPredictionModel.h
    Core/ColdStartFolderBootstrapper.h
    Core/PredictionScanOrchestrator.h
    GPU/DMADirectPreloader.h
    Core/PerUserPredictionIsolator.h
    Core/PredictionAccuracyTracker.h
    # Sprint 771-780 — Collaborative Annotation v2 (v27.5.0 "Sirius-V")
    Core/CollaborativeAnnotationEngineV2.h
    Core/AnnotationSignatureVerifier.h
    Core/AnnotationTimeline.h
    Core/PresenceIndicatorEngine.h
    Core/AnnotationTaxonomyV2.h
    Core/OfflineAnnotationSyncQueue.h
    Core/AnnotationExportPipelineV2.h
    # Sprint 781-790 — Protocol Surface v2 (v27.6.0 "Sirius-W")
    Core/GRPCProtocolServerV2.h
    Core/RESTAPIServerV2.h
    Core/GraphQLSubscriptionServer.h
    Core/OAuth2PKCEMiddleware.h
    Core/JWTValidationEngine.h
    Core/RateLimitingMiddleware.h
    Core/OpenAPICodeGenerator.h
    Core/GraphQLSchemaIntrospector.h
    # Sprint 791-800 — Neural Codec v2 (v27.7.0 "Sirius-X")
    Core/NeuralCodecV2Engine.h
    Core/ProgressiveNeuralDecoder.h
    Core/NeuralContainerFormat.h
    GPU/NeuralCodecHWAccelerator.h
    Core/CodecNegotiationProtocol.h

    # Sprint 801-810 — Cross-Platform Electron Shell (v28.0.0 "Polaris")

    # Sprint 811-820 — Generative AI Caption Synthesis (v28.1.0 "Polaris-R")

    # Sprint 821-830 — Live AR Preview Engine (v28.2.0 "Polaris-S")

    # Sprint 831-840 — Enterprise Console 2.0 (v28.3.0 "Polaris-T")

    # Sprint 841-850 — Adaptive UX & Personalization (v28.4.0 "Polaris-U")

    # Sprint 851-860 — Quantum-Safe Key Management v2 (v28.5.0 "Polaris-V")

    # Sprint 861-870 — Post-Quantum Cryptography & Signatures (v28.6.0 "Polaris-W")
    Core/PQCSignatureVerifier.h
    Core/HybridTrustChainV2.h
    Core/QuantumSafeKeyExchange.h
    Utils/DilithiumCertificateStore.h
    Utils/SignatureAuditLogger.h
    Core/CryptoAgilityBroker.h
    Utils/KeyRotationScheduler.h

    # Sprint 871-880 — Cross-Platform Preview macOS+Linux (v28.7.0 "Polaris-X")
    GPU/MetalRenderBridge.h
    GPU/LinuxVulkanPreview.h
    Core/PlatformNeutralBuffer.h
    Utils/GTK4ThumbnailWidget.h
    Core/MacOSShellBridge.h
    Core/XDGThumbnailProvider.h
    GPU/MetalShaderCompiler.h
    Utils/PlatformCapabilityProbe.h

    # Sprint 881-890 — Gen-5 Platform WinUI 4 (v29.0.0 "Capella")
    Core/AsyncPreviewBroker.h
    Core/UniversalFileProvider.h
    Core/WinUI4PreviewHandler.h
    Core/ShellPropertyHandlerV2.h
    Pipeline/PreviewPipelineV5.h
    Cache/PersistentL3Cache.h
    Core/LivePreviewUpdater.h
    Utils/ShellExtensionHealthMonitor.h

    # Sprint 891-900 — Accessibility & Inclusive Design v2 (v29.1.0 "Capella-R")
    Utils/ARIAThumbnailAnnotator.h
    Utils/HighContrastThemeAdapter.h
    Core/KeyboardNavigationController.h
    Utils/A11yTelemetryReporter.h

    # Sprint 961-970 — Gen-6 Platform Unification (v30.0.0 "Deneb")
    Core/PlatformAbstractionLayer.h
    GPU/MetalPipelineV2.h
    GPU/LinuxDRMBackend.h
    Core/UniversalWindowBroker.h
    Core/NativeFilesystemAdapter.h
    Core/CrossPlatformShellProvider.h
    Core/PlatformUIScalingEngine.h
    Utils/PlatformBuildMatrix.h

    # Sprint 971-980 — DirectStorage & GPU Decompression (v30.1.0 "Deneb-R")
    GPU/DirectStorageEngine.h
    GPU/GPUDecompressScheduler.h
    Pipeline/DirectStoragePipelineStage.h
    GPU/NvGDeflateBackend.h
    GPU/AMDDecompressBackend.h
    Cache/DirectStorageCacheTier.h
    Core/AsyncFileStreamBroker.h
    GPU/StagingBufferPoolV2.h

    # Sprint 981-990 — CLIP Semantic Search & Discovery (v30.2.0 "Deneb-S")
    AI/CLIPEmbeddingEngine.h
    AI/SemanticSearchIndex.h
    AI/NaturalLanguageQueryParser.h
    AI/VisualSimilarityGraph.h
    AI/EmbeddingCacheStore.h
    AI/MultiModalRanker.h
    AI/SearchResultDeduplicator.h
    AI/IncrementalIndexUpdater.h

    # Sprint 991-1000 — Live Preview Scrubber & Rich Media (v30.3.0 "Deneb-T")
    Core/LivePreviewScrubber.h
    Core/VideoKeyframeExtractor.h
    Core/AnimatedFrameScrubber.h
    Core/AudioWaveformRenderer.h
    Core/DocumentPagePreviewer.h
    Core/ShaderSyntaxHighlighter.h
    Core/FontGlyphSampler.h
    Core/SpreadsheetChartRenderer.h

    # Sprint 1001-1010 — Geospatial, Medical & Scientific Formats (v30.4.0 "Deneb-U")
    Decoders/GeoTIFFDecoder.h
    Decoders/NITFDecoder.h
    Decoders/DICOMAdvancedDecoder.h
    Decoders/NRRDDecoder.h
    Decoders/HDF5ThumbnailDecoder.h
    Decoders/NetCDFDecoder.h
    Decoders/ECWDecoder.h

    # Sprint 1011-1020 — Universal Format Decoder Library (v30.5.0 "Deneb-V")
    Core/UniversalDecoderFacade.h
    Core/UniversalLibraryManifest.h

    # Sprint 1021-1030 — Plugin Marketplace v4 & Commerce (v30.6.0 "Deneb-W")
    Plugin/MarketplaceClientV4.h
    Plugin/PluginRatingEngine.h
    Plugin/PluginDependencyGraph.h
    Plugin/PluginBundleInstaller.h
    Plugin/SubscriptionLicenseManagerV4.h
    Plugin/PluginReputationScorer.h
    Plugin/AutoUpdatePolicyV4.h
    Plugin/PrePublishReviewGateway.h

    # Sprint 1031-1040 — Enterprise Console v3 & Fleet Management (v30.7.0 "Deneb-X")
    Enterprise/EnterpriseConsoleV3.h
    Enterprise/FleetDeploymentManager.h
    Enterprise/ComplianceReportGenerator.h
    Enterprise/EnterpriseMetricsDashboard.h
    Enterprise/PolicyVersionControl.h
    Enterprise/RemoteDecoderControl.h
    Enterprise/UsageAnomalyDetector.h
    Enterprise/EnterpriseAuditExporter.h
    # Sprint 1041-1050 — Generative AI Thumbnails (v31.0.0 Achernar)
    AI/GenerativeThumbnailEngine.h
    AI/ContentAwareInpainter.h
    AI/StyleTransferRenderer.h
    AI/ImageDescriptionSynthesizer.h
    AI/ThumbnailPersonalisationEngine.h
    AI/GenerativeUpscalerV3.h
    AI/ContentModerationFilter.h
    AI/GenerativeAuditTrail.h
    # Sprint 1051-1060 — Cross-Platform Shell Extensions (v31.1.0 Achernar-R)
    Core/LinuxNautilusExtension.h
    Core/KDEDolphinExtension.h
    Core/ThunarThumbnailExtension.h
    Core/MacOSQuickLookV3.h
    Core/LinuxThumbnailerDaemon.h
    Core/WaylandShellExtension.h
    Core/MacOSLaunchServicesAdapter.h
    Core/XPlatformTestHarness.h

    # Consolidated — previously unregistered headers
    AI/ImageQualityAssessorV2.h
    CLI/CLICommandParser.h
    CLI/LensCLI.h
    Core/FormatFallbackEngine.h
    Core/LocalizationEngine.h
    Core/ResourceUsageProfiler.h
    Core/WorkStealingSchedulerV2.h
    Decoders/FITSDecoder.h
    Pipeline/HiDPIScaler.h
    Pipeline/ZeroCopyRenderer.h
    Plugin/PluginCompatibilityKitV2.h
    Plugin/PluginHotReloadManager.h
    Plugin/PluginMarketplaceUnified.h
    Plugin/PluginTrustChainValidator.h
    Utils/LocalizationEngine.h
    # Sprint additions — new feature headers
    Cache/PersistentCacheManager.h
    Core/ErrorReportingPipeline.h
    Core/ShellOverlayHandler.h
    Core/TelemetryAnalyticsEngine.h
    AI/AISearchIntegration.h
    Core/DarkModeEngine.h
    Core/DecoderCompatLayer.h
    Core/DecoderHotfixApplicator.h
    Core/DecoderVersionRegistry.h
    Core/FormatCapabilityMatrix.h
    Core/FormatCategoryManager.h
    Core/FormatFamilyResolver.h
    Core/FormatSchemaValidator.h
    Core/FormatStatusIndicator.h
    Core/ThumbnailQualityAnalyzer.h
    Plugin/PluginDebuggerIntegration.h
    Plugin/PluginHotReload.h
    Utils/DocumentationExcellenceV2.h
    # Sprint 1071-1080 — Post-Quantum Security & Zero-Trust (v32.0.0 "Fomalhaut")
    Core/PostQuantumCryptoProvider.h
    Core/ZeroTrustAccessBroker.h
    Core/QuantumResistantHashEngine.h
    Plugin/PluginZeroTrustSandbox.h
    Core/BinaryTrustVerifier.h
    Core/SecureConfigurationManager.h
    Core/ThreatModelingEngine.h
    Utils/SecurityPostureAnalyzer.h
    # Sprint 1081-1090 — Edge AI & Hardware-Accelerated Inference (v32.1.0 "Fomalhaut-R")
    GPU/NPUAccelerationEngine.h
    GPU/EdgeAIInferenceEngine.h
    AI/HardwareCapabilityNegotiator.h
    GPU/AMDXDNABackend.h
    GPU/QualcommAIEBackend.h
    GPU/IntelAMXBackend.h
    Pipeline/HardwareAcceleratedPipeline.h
    Core/ComputeDeviceRegistry.h
    # Sprint 1091-1100 — DirectStorage & Zero-Latency Pipeline (v32.2.0 "Fomalhaut-S")
    Core/DirectStorageManager.h
    Core/GPUDecompressKernel.h
    Pipeline/ZeroLatencyPipeline.h
    Core/ThumbnailPipelineMetrics.h
    Core/StreamingDecodeOrchestrator.h
    # Sprint 1101-1110 — Annotation, HDR Tone Mapping & Format Detection (v32.3.0 "Fomalhaut-T")
    Core/ThumbnailAnnotationOverlay.h
    Core/AdaptiveBitDepthConverter.h
    Pipeline/BatchThumbnailExporter.h
    Core/FormatSignatureDetector.h
    Core/MemoryMappedDecoder.h
    # Sprint 1111-1120 — DirectStorage Zero-Copy GPU Decompress (v32.5.0 "Fomalhaut-V")
    GPU/ZStdGPUKernel.h
    GPU/GPUDecompressOrchestrator.h
    Pipeline/DirectStorageBatchScheduler.h
    Core/DirectStorageProfiler.h
    Core/ZeroCopyDecodeSession.h
    # Sprint 1121-1130 — CLIP Semantic Search + HNSW Index (v32.6.0 "Fomalhaut-W")
    AI/HNSWIndexEngine.h
    AI/CLIPQueryProcessor.h
    AI/SemanticSearchOrchestrator.h
    AI/EmbeddingPersistenceEngine.h
    AI/VisualQueryOptimizer.h
    # Sprint 1131-1140 — Live Preview Scrubber (v32.7.0 "Fomalhaut-X")
    Media/VideoFrameExtractor.h
    Media/VideoScrubberTimeline.h
    Media/LivePreviewSession.h
    Media/ThumbnailStripGenerator.h
    Media/ScrubberCacheEngine.h
    # Platform (Windows-only)
    Platform/PlatformShellProvider.h
    Platform/Win32ShellProvider.h
    Platform/PlatformDetector.h
    Platform/PlatformGPURouter.h
    # Sprint 1161-1170 — Enterprise Console v4 (v33.2.0 "Spica-S")
    Enterprise/EnterprisePolicyV4.h
    Enterprise/GPOPolicyTemplate.h
    Enterprise/IntuneComplianceEngine.h
    Enterprise/EnterpriseAuditLogger.h
    Enterprise/ConfigMgrPolicyBridge.h
    # Sprint 1171-1180 — Generative AI Thumbnails (v33.3.0 "Spica-T")
    AI/NPUThumbnailSynthesizer.h
    AI/DiffusionModelEngine.h
    AI/ThumbnailInpaintEngine.h
    AI/OffDeviceInferenceRouter.h
    AI/AIThumbnailBatchProcessor.h
    # Sprint 1181-1190 — Plugin Marketplace v5 (v33.4.0 "Spica-U")
    Plugin/PluginMarketplaceV5.h
    Plugin/SDKCompatKit3.h
    Plugin/PluginDistributionManager.h
    Plugin/MarketplaceSearchIndex.h
    Plugin/PluginSignatureValidator.h
    # Sprint 1191-1200 — LTS Hardening + Security Audit (v33.5.0 "Spica-V")
    Utils/LTSHardeningController.h
    Utils/SecurityAuditEngine.h
    Utils/VulnerabilityFingerprintDB.h
    Utils/LTSCertificationGate.h
    Utils/SecureKeyStore.h
    # Sprint 1201-1210 — Format Coverage Blitz (v34.0.0 "Arcturus")
    Decoders/BasisUniversalDecoder.h
    Decoders/UltraHDRDecoder.h
    Decoders/IfcBimDecoder.h
    Decoders/LasPointCloudDecoder.h
    Decoders/JupyterNotebookDecoder.h
)

set(ENGINE_SOURCES
    # Core implementations
    Core/AsyncShellExtension.cpp
    Core/BatchProcessingEngine.cpp
    Core/CloudSyncProvider.cpp
    Core/Config.cpp
    Core/ContentIndexer.cpp
    Core/EncoderExportEngine.cpp
    Core/EngineAPI.cpp
    Core/FormatConverterEngine.cpp
    Core/MetadataExtractor.cpp
    Core/NetworkProviderEngine.cpp
    Core/PerformanceBenchmarkV2.cpp
    Core/RegistryManager.cpp
    Core/ResourcePoolEngine.cpp
    Core/SecurityHardeningV2.cpp
    Core/SIMDAccelerator.cpp
    Core/TelemetryEngine.cpp
    Core/ETWTraceProvider.cpp
    Core/ThemeEngine.cpp
    Core/UpdateEngine.cpp
    Core/WatchFolderEngine.cpp
    Core/ErrorCategorizationEngine.cpp
    Core/DecoderPerformanceCounters.cpp

    # Pipeline implementations
    # v31.6.0 — Streaming decode & profiling
    # v31.7.0 — Retry & health monitoring
    # v31.8.0 — Adaptive optimization & prefetch
    # v31.9.0 — Autotuning pipeline
    Pipeline/AsyncThumbnailProvider.cpp
    Pipeline/DecoderRegistry.cpp
    Pipeline/FormatDetector.cpp
    Pipeline/ParallelBatchDecoder.cpp
    Pipeline/ThumbnailPipeline.cpp

    # Decoder implementations
    Decoders/Advanced3DFormatDecoder.cpp
    Decoders/ArchiveDecoder.cpp
    Decoders/AudioDecoder.cpp
    Decoders/AVIFDecoder.cpp
    Decoders/DDSDecoder.cpp
    Decoders/DICOMDecoder.cpp
    Decoders/DocumentDecoder.cpp
    Decoders/EBookDecoder.cpp
    Decoders/EPSDecoder.cpp
    Decoders/EXRDecoder.cpp
    Decoders/FarbfeldDecoder.cpp
    Decoders/FITSDecoder.cpp
    Decoders/FontDecoder.cpp
    Decoders/GeospatialDecoder.cpp
    Decoders/HDRDecoder.cpp
    Decoders/HEIFDecoder.cpp
    Decoders/ICODecoder.cpp
    Decoders/ImageDecoder.cpp
    Decoders/JPEG2000Decoder.cpp
    Decoders/JXLDecoder.cpp
    Decoders/KTXTextureDecoder.cpp
    Decoders/ModelDecoder.cpp
    Decoders/OpenRasterDecoder.cpp
    Decoders/PCXDecoder.cpp
    Decoders/PDFDecoder.cpp
    Decoders/PPMDecoder.cpp
    Decoders/PSDDecoder.cpp
    Decoders/QOIDecoder.cpp
    Decoders/RAWDecoder.cpp
    Decoders/SGIDecoder.cpp
    Decoders/SVGDecoder.cpp
    Decoders/TGADecoder.cpp
    Decoders/VTFDecoder.cpp
    Decoders/VideoDecoder.cpp
    Decoders/WebPDecoder.cpp
    Decoders/WMFDecoder.cpp
    Decoders/ExampleDecoder.cpp
    Decoders/XCFDecoder.cpp
    Decoders/XPMDecoder.cpp

    # AI implementations
    AI/ContentAwareInpainter.cpp
    AI/ContentModerationFilter.cpp
    AI/GenerativeAuditTrail.cpp
    AI/GenerativeThumbnailEngine.cpp
    AI/GenerativeUpscalerV3.cpp
    AI/ImageDescriptionSynthesizer.cpp
    AI/StyleTransferRenderer.cpp
    AI/ThumbnailPersonalisationEngine.cpp
    # v31.6.0 — Format Routing & Enhanced Accessibility
    # v31.7.0 — Contextual Intelligence & Self-Healing
    # v31.8.0 — Intelligent Workflow Automation
    # v31.9.0 — Autonomous workflow & shell intelligence

    # Enterprise implementations
    Enterprise/EnterpriseAuditExporter.cpp
    Enterprise/EnterpriseConsoleV3.cpp
    Enterprise/EnterpriseMetricsDashboard.cpp
    Enterprise/UsageAnomalyDetector.cpp
    # v31.9.0 backlog — v30.7.0 Deneb-X enterprise TUs
    # v31.6.0 — Core routing implementations
    # v31.7.0 — Fault tolerance & diagnostics
    # v31.8.0 — Workflow automation
    # v31.9.0 — Cross-platform shell integration & backlog TUs

    # GPU rendering implementations
    GPU/D3D11Renderer.cpp
    GPU/D3D12ComputePipeline.cpp
    GPU/GDIRenderer.cpp
    GPU/VulkanComputePipeline.cpp

    # Cache implementations
    Cache/PersistentDiskCache.cpp
    Cache/ThumbnailCache.cpp

    # Plugin system implementations
    Plugin/IPC/SharedMemoryManager.cpp
    Plugin/IsolationModeSelector.cpp
    Plugin/PluginDecoder.cpp
    Plugin/PluginHostClient.cpp
    Plugin/PluginManager.cpp
    Plugin/MarketplaceClientV4.cpp
    Plugin/PluginBundleInstaller.cpp
    Plugin/PluginMarketplaceV2.cpp
    Plugin/PluginRatingEngine.cpp
    Plugin/PluginReputationScorer.cpp
    Plugin/SubscriptionLicenseManagerV4.cpp
    Plugin/Security/JobObjectManager.cpp

    # Utility implementations
    Utils/AuditLogger.cpp
    Utils/AutoDocGenerator.cpp
    Utils/CIValidator.cpp
    Utils/CodeCoverageIntegration.cpp
    Utils/CommandLineInterface.cpp
    Utils/ConfigMigrationEngine.cpp
    Utils/DiagnosticDashboard.cpp
    Utils/EnterpriseDeploymentManager.cpp
    Utils/EXIFOrientation.cpp
    Utils/FileHashEngine.cpp
    Utils/HardwareCapabilities.cpp
    Utils/LogRotationEngine.cpp
    Utils/MalformedInputHandler.cpp
    Utils/MemorySafetyIntegration.cpp
    Utils/MSIXPackageManager.cpp
    Utils/NetworkDiagnostics.cpp
    Utils/NotificationEngine.cpp
    Utils/PerformanceProfiler.cpp
    Utils/PortableModeManager.cpp
    Utils/PythonSDK.cpp

    # Memory implementations
    Memory/BitmapPool.cpp

    Utils/SIMDScaler.cpp
    Utils/LocalizationEngine.cpp
    Utils/TestSuiteExpansion.cpp
    Utils/Win11Integration.cpp

    # Release gate implementation
    Utils/ReleaseGateUnified.cpp
    # v31.9.0 — Cross-platform build validator
    # Sprint 1071-1080 — Post-Quantum Security & Zero-Trust (v32.0.0 "Fomalhaut")
    # Sprint 1081-1090 — Edge AI & Hardware-Accelerated Inference (v32.1.0 "Fomalhaut-R")
    # Sprint 1091-1100 — DirectStorage & Zero-Latency Pipeline (v32.2.0 "Fomalhaut-S")
    # Sprint 1101-1110 — Annotation, HDR Tone Mapping & Format Detection (v32.3.0 "Fomalhaut-T")
    # Sprint 1111-1120 — DirectStorage Zero-Copy GPU Decompress (v32.5.0 "Fomalhaut-V")
    GPU/GPUDecompressOrchestrator.cpp
    Pipeline/DirectStorageBatchScheduler.cpp
    Core/DirectStorageProfiler.cpp
    # Sprint 1121-1130 — CLIP Semantic Search + HNSW Index (v32.6.0 "Fomalhaut-W")
    AI/HNSWIndexEngine.cpp
    AI/CLIPQueryProcessor.cpp
    AI/SemanticSearchOrchestrator.cpp
    AI/EmbeddingPersistenceEngine.cpp
    # Sprint 1131-1140 — Live Preview Scrubber (v32.7.0 "Fomalhaut-X")
    Media/VideoFrameExtractor.cpp
    Media/VideoScrubberTimeline.cpp
    Media/ThumbnailStripGenerator.cpp
    Media/ScrubberCacheEngine.cpp
    # Platform (Windows-only)
    Platform/Win32ShellProvider.cpp
    Platform/PlatformDetector.cpp
    Platform/PlatformGPURouter.cpp
    # Sprint 1161-1170 — Enterprise Console v4 (v33.2.0 "Spica-S")
    Enterprise/EnterprisePolicyV4.cpp
    Enterprise/GPOPolicyTemplate.cpp
    Enterprise/IntuneComplianceEngine.cpp
    Enterprise/EnterpriseAuditLogger.cpp
    Enterprise/ConfigMgrPolicyBridge.cpp
    # Sprint 1171-1180 — Generative AI Thumbnails (v33.3.0 "Spica-T")
    AI/NPUThumbnailSynthesizer.cpp
    AI/DiffusionModelEngine.cpp
    AI/ThumbnailInpaintEngine.cpp
    AI/OffDeviceInferenceRouter.cpp
    AI/AIThumbnailBatchProcessor.cpp
    # Sprint 1181-1190 — Plugin Marketplace v5 (v33.4.0 "Spica-U")
    Plugin/PluginMarketplaceV5.cpp
    Plugin/SDKCompatKit3.cpp
    Plugin/PluginDistributionManager.cpp
    Plugin/MarketplaceSearchIndex.cpp
    Plugin/PluginSignatureValidator.cpp
    # Sprint 1191-1200 — LTS Hardening + Security Audit (v33.5.0 "Spica-V")
    Utils/LTSHardeningController.cpp
    Utils/SecurityAuditEngine.cpp
    Utils/VulnerabilityFingerprintDB.cpp
    Utils/LTSCertificationGate.cpp
    Utils/SecureKeyStore.cpp
    # Sprint 1201-1210 — Format Coverage Blitz (v34.0.0 "Arcturus")
    Decoders/BasisUniversalDecoder.cpp
    Decoders/UltraHDRDecoder.cpp
    Decoders/IfcBimDecoder.cpp
    Decoders/LasPointCloudDecoder.cpp
    Decoders/JupyterNotebookDecoder.cpp
    # Sprint 1211-1220 — GPU-First Decode Pipeline (v34.1.0 "Arcturus-R")
    GPU/GPUDecodeFormatRouter.cpp
    GPU/GPUJPEGDecodeAccelerator.cpp
    GPU/GPURawDemosaicKernel.cpp
    GPU/GPUDecodePerformanceGate.cpp
    GPU/ZeroCopyGPUSurface.cpp

    # Sprint 1221-1230 — HDR & Wide Color Gamut Mastery (v34.2.0 "Arcturus-S")
    Core/GainmapJPEGToneMapper.cpp
    Core/PQToSDRToneMapper.cpp
    Core/HLGToSDRConverter.cpp
    Core/ICCv5ProfileEngine.cpp
    Core/ACESODTProcessor.cpp

    # Sprint 1231-1240 — Predictive Pre-Generation Engine (v34.3.0 "Arcturus-T")
    Pipeline/DirectoryPreScanQueue.cpp
    Pipeline/AdjacencyPredictor.cpp
    Pipeline/ScrollVelocityTracker.cpp
    Pipeline/IdleTimePreGenerator.cpp
    Pipeline/PredictivePreGenEngine.cpp

    # Sprint 1241-1250 — Animated & Sequence Format Suite (v34.4.0 "Arcturus-U")
    Core/HoverScrubController.cpp
    Core/APNGFrameCombiner.cpp
    Decoders/GIFAnimationDecoder.cpp
    Decoders/AnimatedSequenceSampler.cpp
    Cache/AnimatedThumbnailCache.cpp

    # Sprint 1251-1260 — Industrial & Scientific Formats v2 (v34.5.0 "Arcturus-V")
    Decoders/DICOMWindowingPresets.cpp
    Decoders/FITSZScaleStretch.cpp
    Decoders/LASPointCloudRenderer.cpp
    Decoders/OMETIFFCompositor.cpp
    Decoders/MHAVolumeDecoder.cpp

    # Sprint 1261-1270 — CAD/BIM/EDA Formats (v34.6.0 "Arcturus-W")
    Decoders/DWGHeaderParser.cpp
    Decoders/STEPBoundingBoxExtractor.cpp
    Decoders/IFCEntityCounter.cpp
    Decoders/GerberLayerCompositor.cpp
    Decoders/KiCadNetlistParser.cpp

    # Sprint 1271-1280 — Performance Hardening + LTS Gate (v34.7.0 "Arcturus-X")
    Core/PerfRegressionGate.cpp
    Core/LTSBuildValidator.cpp
    Cache/CacheWarmupPreloader.cpp
    Pipeline/DecodeLatencyProfiler.cpp
    Utils/BenchmarkBaseline.cpp

    # Sprint 1281-1290 — Streaming & Cloud-Native Thumbnails (v35.0.0 "Vega")
    Pipeline/MultiStageThumbnailEmitter.cpp
    Core/CloudHydrationMonitor.cpp
    Cache/PartialDecodeStateCache.cpp
    Core/ThumbnailETagValidator.cpp
    Pipeline/AdaptiveFidelitySelector.cpp

    # Sprint 1291-1300 — Real-Time Collaboration & Live Edit Sync (v35.1.0 "Vega-R")
    Core/LiveSyncTokenManager.cpp
    Cache/CollaborativeCacheCoordinator.cpp
    Pipeline/ThumbnailDeltaEncoder.cpp
    Core/ConflictResolutionEngine.cpp
    Pipeline/RealTimePreviewPipeline.cpp
    # Sprint 1301-1310: Network-Aware Streaming Cache
    Core/NetworkTopologyProbe.cpp
    Cache/StreamingCacheTierPolicy.cpp
    Pipeline/BandwidthThrottleGuard.cpp
    Cache/RemoteFileManifestCache.cpp
    Cache/CachePrefetchScheduler.cpp
    # Sprint 1311-1320: Zero-Trust Thumbnail Security
    Core/ThumbnailManifestSigner.cpp
    Pipeline/ZeroTrustDecodeWorker.cpp
    Cache/TokenBoundCacheEntry.cpp
    Core/ThumbnailAuditLog.cpp
    Core/FIPSCryptoAdapter.cpp
    # Sprint 1321-1330: WebAssembly / Browser Extension Pipeline
    Core/WasmDecoderShim.cpp
    Pipeline/BrowserThumbnailBridge.cpp
    Pipeline/OffscreenCanvasRenderer.cpp
    Cache/WasmCacheAdapter.cpp
    Pipeline/ProgressiveThumbnailStream.cpp
    # Sprint 1331-1340: Cross-Device Preview Sync
    Core/DeviceSyncManifest.cpp
    Cache/CrossDeviceCacheSync.cpp
    Pipeline/ThumbnailPackFile.cpp
    Core/SyncConflictResolver.cpp
    Core/DeviceCapabilityAdvertiser.cpp
)

#==============================================================================
# Engine Library Target
#==============================================================================

if(ENGINE_SOURCES)
    if(BUILD_SHARED_LIBS)
        add_library(ExplorerLensEngine SHARED ${ENGINE_SOURCES} ${ENGINE_HEADERS})
        target_compile_definitions(ExplorerLensEngine PRIVATE ENGINE_EXPORTS)
    else()
        add_library(ExplorerLensEngine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})
        # STATIC library: do NOT define ENGINE_EXPORTS, so ENGINE_API is empty
        target_compile_definitions(ExplorerLensEngine PUBLIC ENGINE_STATIC)
    endif()

    target_include_directories(ExplorerLensEngine
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../SDK/include
            ${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libwebp-1.5.0-build/src
            ${CMAKE_CURRENT_SOURCE_DIR}/../external/compression-libs/minizip-ng-4.0.10
            # JXL includes (actual install location)
            ${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libjxl-0.11.1/install/include
            # libheif includes (HEIF/HEIC support)
            ${LIBHEIF_INCLUDE_DIR}
            ${LIBHEIF_GENERATED_INCLUDE_DIR}
            # LibRaw includes
            ${CMAKE_CURRENT_SOURCE_DIR}/../external/camera-libs/libraw-install/include
            # AVIF includes
            ${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libavif-1.3.0/include
            # MuPDF includes
            ${CMAKE_CURRENT_SOURCE_DIR}/../external/pdf-libs/mupdf-1.24.11-source/include
            # OpenJPEG includes (JPEG 2000 support)
            ${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/openjpeg-2.5.3/install/include/openjpeg-2.5
            # FreeType includes (font rendering)
            ${CMAKE_CURRENT_SOURCE_DIR}/../external/ui-libs/freetype-2.13.3/include
    )

    # Windows-specific settings
    if(WIN32)
        target_compile_definitions(ExplorerLensEngine PRIVATE
            _WINDOWS
            WIN32_LEAN_AND_MEAN
            NOMINMAX
            LIBWEBP_REBUILT_WITH_MD   # libwebp 1.5.0 rebuilt with /MD — no NODEFAULTLIB:LIBCMT needed
        )

        # Conditional decoder feature flags
        if(HAS_LIBJXL)
            target_compile_definitions(ExplorerLensEngine PRIVATE HAS_LIBJXL JXL_STATIC_DEFINE)
            message(STATUS "JPEG XL support: ENABLED")
        else()
            message(STATUS "JPEG XL support: DISABLED (set -DHAS_LIBJXL=ON when libjxl is available)")
        endif()

        if(HAS_LIBHEIF)
            target_compile_definitions(ExplorerLensEngine PRIVATE HAS_LIBHEIF LIBHEIF_STATIC_BUILD)
            message(STATUS "HEIF/HEIC support: ENABLED")
        else()
            message(STATUS "HEIF/HEIC support: DISABLED (set -DHAS_LIBHEIF=ON when libheif is available)")
        endif()

        if(HAS_LIBRAW)
            target_compile_definitions(ExplorerLensEngine PRIVATE HAS_LIBRAW)
            message(STATUS "Camera RAW support: ENABLED")
        else()
            message(STATUS "Camera RAW support: DISABLED (set -DHAS_LIBRAW=ON when LibRaw is available)")
        endif()

        if(HAS_LIBAVIF)
            target_compile_definitions(ExplorerLensEngine PRIVATE HAS_LIBAVIF)
            message(STATUS "AVIF support: ENABLED")
        else()
            message(STATUS "AVIF support: DISABLED (set -DHAS_LIBAVIF=ON when libavif is available)")
        endif()

        if(HAS_MUPDF)
            target_compile_definitions(ExplorerLensEngine PRIVATE HAS_MUPDF)
            message(STATUS "PDF support (MuPDF): ENABLED")
        else()
            message(STATUS "PDF support (MuPDF): DISABLED (set -DHAS_MUPDF=ON when MuPDF is available)")
        endif()

        if(HAS_OPENJPEG)
            target_compile_definitions(ExplorerLensEngine PRIVATE HAS_OPENJPEG)
            message(STATUS "JPEG 2000 support (OpenJPEG): ENABLED")
        else()
            message(STATUS "JPEG 2000 support (OpenJPEG): DISABLED (run Build-OpenJPEG.ps1)")
        endif()

        if(HAS_FREETYPE)
            target_compile_definitions(ExplorerLensEngine PRIVATE HAS_FREETYPE)
            message(STATUS "Enhanced font rendering (FreeType): ENABLED")
        else()
            message(STATUS "Enhanced font rendering (FreeType): DISABLED (run Build-FreeType.ps1)")
        endif()

        if(HAS_LIBARCHIVE)
            target_compile_definitions(ExplorerLensEngine PRIVATE HAS_LIBARCHIVE LIBARCHIVE_STATIC ENABLE_LIBARCHIVE_SUPPORT)
            message(STATUS "libarchive support: ENABLED (TAR/CPIO/ISO/XAR/AR/CAB)")
        else()
            message(STATUS "libarchive support: DISABLED (run Build-Libarchive.ps1)")
        endif()

        if(ENABLE_VIDEO_DECODER)
            target_compile_definitions(ExplorerLensEngine PRIVATE ENABLE_VIDEO_DECODER)
            message(STATUS "Video decoder: ENABLED")
        else()
            message(STATUS "Video decoder: DISABLED (set -DENABLE_VIDEO_DECODER=ON)")
        endif()

        if(ENABLE_AUDIO_DECODER)
            target_compile_definitions(ExplorerLensEngine PRIVATE ENABLE_AUDIO_DECODER)
            message(STATUS "Audio decoder: ENABLED")
        else()
            message(STATUS "Audio decoder: DISABLED (set -DENABLE_AUDIO_DECODER=ON)")
        endif()

        target_link_libraries(ExplorerLensEngine
            PRIVATE
                gdiplus
                d3d11
                d3d12
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
        target_link_directories(ExplorerLensEngine
            PUBLIC
                ${CMAKE_CURRENT_SOURCE_DIR}/../SDK/lib
                ${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libwebp-1.5.0-build/build-cmake/Release
                ${CMAKE_CURRENT_SOURCE_DIR}/../external/compression-libs/minizip-ng-4.0.10/build-manual
                ${CMAKE_CURRENT_SOURCE_DIR}/../external/compression-libs/zlib-1.3.1/install/lib
                ${CMAKE_CURRENT_SOURCE_DIR}/../external/compression-libs/zstd-1.5.7/install/lib
                # JXL libraries (actual install location)
                ${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libjxl-0.11.1/install/lib
                # LibRaw libraries
                ${CMAKE_CURRENT_SOURCE_DIR}/../external/camera-libs/libraw-install/lib
                # AVIF libraries
                ${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libavif-1.3.0/install/lib
                # dav1d (AV1 codec for AVIF)
                ${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/dav1d-1.5.1/install/lib
                # MuPDF libraries
                ${CMAKE_CURRENT_SOURCE_DIR}/../external/pdf-libs/mupdf-1.24.11-source/platform/win32/x64/Release
                # libarchive libraries
                ${LIBARCHIVE_INSTALL_DIR}/lib
                ${LIBARCHIVE_BUILD_DIR}/libarchive/Release
        )

        target_link_libraries(ExplorerLensEngine
            PUBLIC
                libwebp
                libsharpyuv
                minizip
                zlibstatic
                zstd_static
        )

        # Conditionally link optional decoder libraries
        if(HAS_LIBJXL)
            target_link_libraries(ExplorerLensEngine PUBLIC jxl jxl_cms jxl_threads brotlicommon brotlidec hwy)
        endif()
        if(HAS_LIBRAW)
            target_link_libraries(ExplorerLensEngine PUBLIC libraw_static)
        endif()
        if(HAS_LIBHEIF)
            if(NOT EXISTS "${LIBHEIF_LIBRARY_PATH}")
                message(FATAL_ERROR "HAS_LIBHEIF=ON but heif.lib was not found. Expected at ${LIBHEIF_LIBRARY_PATH}")
            endif()
            if(NOT EXISTS "${LIBDE265_LIBRARY_PATH}")
                message(FATAL_ERROR "HAS_LIBHEIF=ON but libde265 library was not found. Expected at ${LIBDE265_LIBRARY_PATH}")
            endif()
            target_link_libraries(ExplorerLensEngine PUBLIC "${LIBHEIF_LIBRARY_PATH}" "${LIBDE265_LIBRARY_PATH}")
        endif()
        if(HAS_LIBAVIF)
            target_link_libraries(ExplorerLensEngine PUBLIC avif dav1d)
        endif()
        if(HAS_MUPDF)
            set(MUPDF_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/pdf-libs/mupdf-1.24.11-source/platform/win32/x64/Release")
            set(MUPDF_LIB_PATH "${MUPDF_LIB_DIR}/libmupdf.lib")
            if(EXISTS "${MUPDF_LIB_PATH}")
                target_link_libraries(ExplorerLensEngine PUBLIC
                    "${MUPDF_LIB_DIR}/libmupdf.lib"
                    "${MUPDF_LIB_DIR}/libthirdparty.lib"
                    "${MUPDF_LIB_DIR}/libresources.lib"
                    "${MUPDF_LIB_DIR}/libharfbuzz.lib"
                    "${MUPDF_LIB_DIR}/libextract.lib"
                    "${MUPDF_LIB_DIR}/libpkcs7.lib"
                )
            else()
                message(WARNING "HAS_MUPDF=ON but libmupdf.lib not found at ${MUPDF_LIB_PATH}. PDF support will link but may fail.")
            endif()
        endif()
        if(HAS_OPENJPEG)
            target_link_libraries(ExplorerLensEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../x64/Release/openjp2.lib")
        endif()
        if(HAS_FREETYPE)
            target_link_libraries(ExplorerLensEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../x64/Release/freetype.lib")
        endif()
        if(HAS_LIBARCHIVE)
            target_link_libraries(ExplorerLensEngine PUBLIC "${LIBARCHIVE_LIBRARY_PATH}")
        endif()
    endif()

    # Compiler warnings & flags (MSVC v145 preferred)
    if(MSVC)
        target_compile_options(ExplorerLensEngine PRIVATE
            /W4               # Warning level 4
            /WX-              # Warnings NOT as errors during consolidation
            /permissive-      # Standards conformance
            /arch:AVX2        # Enable AVX2 instructions for SIMD operations
            /diagnostics:caret  # Rich diagnostics with source caret (better error context)
        )
        # MSVC linker flags
        target_link_options(ExplorerLensEngine PRIVATE
            /IGNORE:4099            # Ignore missing PDB warnings for external libs
        )
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(ExplorerLensEngine PRIVATE
            -Wall -Wextra -mavx2 -mfma
        )
        # clang + lld-link: use linker-passthrough for MSVC-style flags
        target_link_options(ExplorerLensEngine PRIVATE
            "LINKER:/IGNORE:4099"
        )
    endif()
else()
    # Interface library for headers-only during initial development
    add_library(ExplorerLensEngine INTERFACE)
    target_include_directories(ExplorerLensEngine
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
# Catch2 Tests (v3) — via FetchContent
#==============================================================================

if(BUILD_CATCH2_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.7.1
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(Catch2)

    # Collect Catch2 test sources
    set(CATCH2_TEST_SOURCES
        Tests/Catch2Tests/FormatDetectionTests.cpp
        Tests/Catch2Tests/CacheSubsystemTests.cpp
        Tests/Catch2Tests/DecoderUnitTests.cpp
        Tests/Catch2Tests/EngineIntegrationTests.cpp
    )

    # Only add sources that exist
    set(CATCH2_EXISTING_SOURCES "")
    foreach(src ${CATCH2_TEST_SOURCES})
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${src}")
            list(APPEND CATCH2_EXISTING_SOURCES "${src}")
        endif()
    endforeach()

    if(CATCH2_EXISTING_SOURCES)
        add_executable(EngineCatch2Tests ${CATCH2_EXISTING_SOURCES})
        target_link_libraries(EngineCatch2Tests
            PRIVATE
                ExplorerLensEngine
                Catch2::Catch2WithMain
        )
        target_compile_features(EngineCatch2Tests PRIVATE cxx_std_20)
        target_include_directories(EngineCatch2Tests PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}"
        )
        if(MSVC)
            target_compile_options(EngineCatch2Tests PRIVATE /W4 /WX- /MP)
        endif()
        include(CTest)
        include(Catch)
        catch_discover_tests(EngineCatch2Tests)
        message(STATUS "  Catch2 tests:      ON (${CATCH2_EXISTING_SOURCES})")
    else()
        message(STATUS "  Catch2 tests:      PENDING (no test source files found yet)")
    endif()
endif()

#==============================================================================
# PluginHost Executable
#==============================================================================

# Enabled in v15.0.0 "Zenith"
add_subdirectory(PluginHost)

#==============================================================================
# Installation
#==============================================================================

install(TARGETS ExplorerLensEngine
    EXPORT ExplorerLensEngineTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES ${ENGINE_HEADERS}
    DESTINATION include/ExplorerLens/Engine
)

#==============================================================================
# Package Configuration (Commented out for now - not needed for development)
#==============================================================================

# include(CMakePackageConfigHelpers)
#
# configure_package_config_file(
#     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ExplorerLensEngineConfig.cmake.in"
#     "${CMAKE_CURRENT_BINARY_DIR}/ExplorerLensEngineConfig.cmake"
#     INSTALL_DESTINATION lib/cmake/ExplorerLensEngine
# )
#
# write_basic_package_version_file(
#     "${CMAKE_CURRENT_BINARY_DIR}/ExplorerLensEngineConfigVersion.cmake"
#     VERSION ${PROJECT_VERSION}
#     COMPATIBILITY SameMajorVersion
# )
#
# install(FILES
#     "${CMAKE_CURRENT_BINARY_DIR}/ExplorerLensEngineConfig.cmake"
#     "${CMAKE_CURRENT_BINARY_DIR}/ExplorerLensEngineConfigVersion.cmake"
#     DESTINATION lib/cmake/ExplorerLensEngine
# )

#==============================================================================
# Summary
#==============================================================================

message(STATUS "")
message(STATUS "ExplorerLens Engine Configuration:")
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
message(STATUS "    HAS_LIBARCHIVE:        ${HAS_LIBARCHIVE}")
message(STATUS "    HAS_OPENJPEG:          ${HAS_OPENJPEG}")
message(STATUS "    HAS_FREETYPE:          ${HAS_FREETYPE}")
message(STATUS "    ENABLE_VIDEO_DECODER:  ${ENABLE_VIDEO_DECODER}")
message(STATUS "    ENABLE_AUDIO_DECODER:  ${ENABLE_AUDIO_DECODER}")
message(STATUS "")
