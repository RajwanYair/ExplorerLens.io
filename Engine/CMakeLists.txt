#==============================================================================
# ExplorerLens Engine - CMake Build Configuration
# Copyright (c) 2026 - ExplorerLens.io Project
# Version: 15.0.0
#==============================================================================

cmake_minimum_required(VERSION 3.20)

project(ExplorerLensEngine
    VERSION 15.0.0
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
    Core/Accessibility.h
    Core/AccessibilityNarratorBridge.h
    Core/AccessibilityPipeline.h
    Core/AccessibilitySuiteV2.h
    Core/AdaptiveDecoderRouter.h
    Core/AIThumbnailEnhancer.h
    Core/AnimatedThumbnailEngine.h
    Core/AsyncShellActivation.h
    Core/AsyncShellExtension.h
    Core/BatchProcessingEngine.h
    Core/BuildConfig.h
    Core/CloudNativeSync.h
    Core/CloudStorageIntegration.h
    Core/CloudSyncProvider.h
    Core/Config.h
    Core/ConfigMigrationEngine.h
    Core/ContentIndexer.h
    Core/D3D12PipelineActivation.h
    Core/DarkMode.h
    Core/DarkModeControls.h
    Core/DarkModeEngine.h
    Core/DarkModeRendererV2.h
    Core/DeadCodeAudit.h
    Core/DeadCodeAuditor.h
    Core/DecoderHealthDashboard.h
    Core/DecoderHealthMonitor.h
    Core/DiagnosticsExporter.h
    Core/DocumentationSyncAudit.h
    Core/EncoderExportEngine.h
    Core/EngineAPI.h
    Core/ErrorContext.h
    Core/ErrorRecoveryEngine.h
    Core/ETWSinkComplete.h
    Core/ETWTraceProvider.h
    Core/FormatConverterEngine.h
    Core/FormatRegistry.h
    Core/FormatTypes.h
    Core/GPUMemoryPoolV2.h
    Core/GPUPipelineV3.h
    Core/HDRDisplayPipeline.h
    Core/HighDPIScaling.h
    Core/ICacheProvider.h
    Core/IFormatDetector.h
    Core/IGPURenderer.h
    Core/IThumbnailDecoder.h
    Core/JumpListIntegration.h

    # Core interfaces (canonical location)
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
    Core/ShellOverlayHandler.h
    Core/ShellPreviewHandler.h
    Core/ShellRegistrationManager.h
    Core/ShellSearchProtocolHandler.h
    Core/SIMDAccelerationManager.h
    Core/SIMDAccelerator.h
    Core/Telemetry.h
    Core/ThemeEngine.h
    Core/ObservabilityIntegration.h
    Core/PerformanceActivation.h
    Core/ThumbnailQualityAnalyzer.h
    Core/ThumbnailAnimationEngineV2.h
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

    # Dead code audit — previously unregistered Core headers
    Core/ArchiveRefactorEngine.h
    Core/BuildValidation.h
    Core/CIHardeningEngine.h
    Core/CodeCoverageEngine.h
    Core/CRTConsistencyManager.h
    Core/DecoderPriority.h
    Core/FFmpegIntegration.h
    Core/FormatCategoryManager.h
    Core/FormatGalleryView.h
    Core/FormatGroupManager.h
    Core/FormatStatusIndicator.h
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
    Core/ThumbnailDiffEngine.h
    Core/DecoderSandboxPolicy.h
    Core/IntelligentPrefetchV2.h
    Core/CompressionBenchmark.h
    Core/ExplorerBandIntegration.h
    Core/ThumbnailStreamProtocol.h
    Core/HotReloadConfigEngine.h
    Core/COMDiagnosticsEngine.h

    Core/ThumbnailWatermark.h
    Core/BatchRenamePreview.h
    Core/DuplicateFileDetector.h
    Core/ThumbnailAnnotation.h
    Core/ExplorerContextMenuExtension.h
    Core/ThumbnailCompareView.h
    Core/FileTypeStatistics.h
    Core/ShellNotificationEngine.h
    Core/ThumbnailExportEngine.h
    Core/ThumbnailVersionControl.h
    Core/FilePreviewRouter.h
    Core/ClipboardThumbnailManager.h
    Core/FormatConversionPipeline.h
    Core/ErrorReportingPipeline.h
    Core/EnterpriseAuditPipeline.h
    Core/ResourceQuotaManager.h
    Core/AccessTokenValidator.h
    Core/ExplorerPreviewPane.h
    Core/FormatFallbackEngine.h

    # Sprint 589-590
    Core/IntegrationTestRunner.h
    Core/SBOMGenerator.h

    # Sprint 644 — PIpeline activation, status, SIMD detection
    Core/FormatStatusProvider.h
    Core/SIMDCapabilityDetector.h
    Core/InputSanitizer.h
    Core/PathTraversalGuard.h

    # Production pipeline & performance infrastructure
    Core/ContentAwareThumbnail.h
    Core/RuntimeSIMDDispatcher.h
    Core/DecoderPerformanceCounters.h
    Core/ThumbnailQualityGate.h
    Core/BatchThumbnailOrchestrator.h
    Core/FileSignatureDetector.h
    Core/GPUResourcePoolManager.h
    Core/CacheCoherencyProtocol.h
    Core/ThumbnailPipelineProfiler.h
    Core/FormatNegotiator.h
    Core/TelemetryAggregator.h
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
    Core/ThumbnailBlurDetector.h
    Core/ColorHistogramBadge.h
    Core/GeoTagMapThumbnail.h
    Core/FileAgeVisualizer.h
    Core/ThumbnailLensEffect.h
    Core/SmartGridLayoutEngine.h
    Core/MetadataTooltipRenderer.h
    Core/CompressedStreamAnalyzer.h
    Core/AdaptiveContrastEnhancer.h
    Core/DocumentDigestOverlay.h
    Core/ThumbnailSignatureVerifier.h
    Core/ArchivePasswordDetector.h
    Core/ThumbnailStitcher.h
    Core/CalendarHeatmapRenderer.h
    Core/FileSizeProportionBadge.h
    Core/IntelligentCachePruner.h
    Core/AlignedBufferPool.h
    Core/PrefetchHintEngine.h
    Core/BranchPredictor.h

    # Pipeline stages and scheduling
    Pipeline/AsyncThumbnailProvider.h
    Pipeline/BatchProcessor.h
    Pipeline/DecoderRegistry.h
    Pipeline/ExplorerWorkScheduler.h
    Pipeline/FormatDetector.h
    Pipeline/FormatFallbackEngine.h
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
    Pipeline/DecoderPriorityScheduler.h
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
    Pipeline/LoopTuner.h

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
    Decoders/DICOMDecoder.h
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
    Decoders/FITSDecoder.h
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

    # Cache management
    Cache/AdaptiveCacheBudgetManager.h
    Cache/CacheWarmingService.h
    Cache/MultiTenantCacheManager.h
    Cache/MultiTierCache.h
    Cache/PersistentCacheManager.h
    Cache/PersistentDiskCache.h
    Cache/PipelineStateCacheV2.h
    Cache/ThumbnailCache.h
    Cache/CacheKeyGenerator.h
    Cache/PredictiveCacheEngine.h
    Cache/PSOPersistenceManager.h
    Cache/ThumbnailPersistenceLayer.h
    Cache/USNCacheInvalidation.h
    Cache/CacheMigrationEngine.h
    Cache/CacheEncryptionLayer.h
    Cache/SubMillisecondCacheEngine.h
    Cache/PSOCachePersistence.h
    Cache/CacheWarmingActivation.h
    Cache/CacheBudgetAutoTuner.h
    Cache/CacheCoherencyManager.h
    Cache/CachePrewarmScheduler.h
    Cache/CacheDiagnostics.h
    Cache/CacheLinePadding.h

    # Plugin system
    Plugin/CrashHandler.h
    Plugin/CrashIntelligence.h
    Plugin/IPC/PluginIPCProtocol.h
    Plugin/IPC/SharedMemoryManager.h
    Plugin/IsolationModeSelector.h
    Plugin/PluginActivation.h
    Plugin/PluginCompatibilityKitV2.h
    Plugin/PluginDebuggerIntegration.h
    Plugin/PluginDecoder.h
    Plugin/PluginHostBridge.h
    Plugin/PluginHostClient.h
    Plugin/PluginHostIPC.h
    Plugin/PluginHotReload.h
    Plugin/PluginLifecycle.h
    Plugin/PluginLoaderV2.h
    Plugin/PluginManager.h
    Plugin/PluginMarketplace.h
    Plugin/PluginMarketplaceUnified.h
    Plugin/PluginMarketplaceV2.h
    Plugin/PluginMarketplaceV3.h
    Plugin/PluginPerformanceProfiler.h
    Plugin/PluginReferencePack.h
    Plugin/PluginRuntimeTestMatrix.h
    Plugin/PluginRuntimeValidation.h
    Plugin/PluginSandboxPolicy.h
    Plugin/PluginSDKV2.h
    Plugin/PluginSecurity.h
    Plugin/PluginNamedPipeBridge.h
    Plugin/PluginTrustChain.h
    Plugin/CrashIntelligenceEngine.h
    Plugin/PluginHotReloadManager.h
    Plugin/PluginCompatibilityKit.h
    Plugin/PluginTrustChainValidator.h
    Plugin/Security/JobObjectManager.h
    Plugin/PluginDependencyResolver.h
    Plugin/PluginCrashRecovery.h
    Plugin/PluginResourceLimiter.h

    # AI/ML modules
    AI/AISearchIntegration.h
    AI/AIThumbnailEnhancer.h
    AI/ImageQualityAssessor.h
    AI/ImageQualityAssessorV2.h
    AI/SceneClassifierEngine.h
    AI/SceneUnderstandingEngine.h
    AI/SmartCropEngine.h
    AI/SmartCropV2.h
    AI/ThumbnailSearchIndex.h

    # Codec modules
    Codec/CodecLoader.h
    Codec/CodecModuleSpecs.h
    Codec/FormatConverter.h
    Codec/ICodecModule.h
    Codec/LazyCodecManager.h

    # Cloud integration
    Cloud/CloudThumbnailProvider.h
    Cloud/NetworkThumbnailProvider.h

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
    Shell/COMApartmentAudit.h

    # Utilities
    Utils/AccessibilityEngine.h
    Utils/ARM64HardwareValidator.h
    Utils/ARM64Platform.h
    Utils/CIPipeline.h
    Utils/CodeCoverage.h
    Utils/DocGenerator.h
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
    Utils/EnterpriseDeployment.h
    Utils/EnterpriseDeploymentManager.h
    Utils/EXIFOrientation.h
    Utils/FileHashEngine.h

    Utils/HardwareCapabilities.h
    Utils/LocalizationEngine.h
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
    Utils/UsageTelemetryEngine.h
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
    Utils/DeploymentPreflightCheck.h
    Utils/OperationalReadinessChecker.h

    # Sprint 591-593
    Utils/InstallerLifecycleManager.h
    Utils/TelemetryEngine.h
    Utils/DocumentationGenerator.h

    # Release gate (unified)
    Utils/ReleaseGate.h

    # Zenith Feature Headers
    Core/HDRToneMappingPipeline.h
    Core/ColorSpaceEngine.h
    Core/GPUTextureCompressionPipeline.h
    Core/AdaptiveDPIScaler.h
    Core/FormatFingerprintDB.h
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
    Core/ARM64NEONScaler.h
    Core/WindowsSearchProtocol.h
    Core/VirtualFilesystemAbstraction.h
    Core/RoleBasedFormatPolicy.h
    Core/AuditTrailLogger.h
    Core/ContentInspectionGateway.h
    Core/CertificateTrustValidator.h
    Core/EncryptedFormatHandler.h
    Core/SelfHealingDecoder.h
    Core/CrashAnalyticsCollector.h
    Core/PerformanceAnomalyDetector.h
    Core/PerformanceDashboard.h
    Core/ThumbnailPreviewEngine.h
    Core/DiagnosticsCollector.h
    Core/DiagnosticReportGeneratorV2.h
    Core/HealthCheckEndpoint.h
    Core/PreviewTooltipRenderer.h
    Core/FormatGalleryCompositor.h
    Core/QuickActionsOverlay.h
    Core/AccessibilityNarratorBridge.h
    Core/UsageAnalyticsDashboard.h
    Core/DecoderPerformanceProfiler.h
    Core/FormatPopularityTracker.h
    Core/SystemResourceMonitor.h
    Core/MultiMonitorColorProfile.h
    Core/RustFFIBridge.h
    Core/DirectStorage12Integration.h
    Core/NeuralTextureCompression.h
    Core/QuantumReadyHashPipeline.h
    GPU/WebGPUThumbnailRenderer.h
    Core/WASMDecoderSandbox.h
    Core/LivePreviewStreamingProtocol.h
    Core/DragDropThumbnailPreview.h
    Core/ExplorerColumnProvider.h
    Core/TaskbarPreviewManager.h
    Core/SearchFederatedProvider.h
    Core/ThumbnailQualityValidator.h

    # Batch 8 (Sprints 430-443)
    Core/DirectShowThumbnailBridge.h
    Core/ShellExtensionHealthMonitor.h
    Core/ThumbnailColorSpace.h
    Pipeline/AsyncIOCompletionEngine.h
    Core/ExifOrientationFixer.h
    Core/MultiMonitorDPIScaler.h
    Memory/VirtualAllocOptimizer.h
    Core/ThumbnailHistogram.h
    Core/FileAssociationManager.h
    GPU/DX12FenceManager.h
    Core/LocalizationEngine.h
    Core/ThumbnailSpriteSheet.h
    Cache/CacheTelemetryCollector.h
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
    Pipeline/PriorityDecodeScheduler.h
    Pipeline/MemoryMappedDecodePath.h
    # Batch 8: Quality & Observability (629-633)
    Core/DecodeLatencyHistogram.h
    Core/ErrorCategorizationEngine.h
    Core/HealthScoreAggregator.h
    Core/PerformanceRegressionDetector.h
    Core/ResourceUsageProfiler.h
    # Batch 9: Smart Features (634-638)
    AI/ThumbnailRelevanceScorer.h
    AI/ColorPaletteExtractor.h
    AI/ImageComplexityAnalyzer.h
    AI/FormatMigrationAdvisor.h
    AI/DecodeStrategyOptimizer.h
    # Batch 10: Platform & Integration (639-643)
    Core/ClipboardMonitorIntegration.h
    Core/ShellNotificationProvider.h
    Core/ExplorerStatusBarProvider.h
    Core/FileSummaryTooltipGenerator.h
    Core/BatchProgressReporter.h
)

set(ENGINE_SOURCES
    # Core implementations
    Core/AnimatedThumbnailEngine.cpp
    Core/AsyncShellExtension.cpp
    Core/BatchProcessingEngine.cpp
    Core/CloudSyncProvider.cpp
    Core/Config.cpp
    Core/ConfigMigrationEngine.cpp
    Core/ContentIndexer.cpp
    Core/EncoderExportEngine.cpp
    Core/EngineAPI.cpp
    Core/ErrorRecoveryEngine.cpp
    Core/FormatConverterEngine.cpp
    Core/HighDPIScaling.cpp
    Core/MetadataExtractor.cpp
    Core/NetworkProviderEngine.cpp
    Core/PerformanceBenchmarkV2.cpp
    Core/RegistryManager.cpp
    Core/ResourcePoolEngine.cpp
    Core/SecurityHardeningV2.cpp
    Core/ShellContextMenuV2.cpp
    Core/ShellPreviewHandler.cpp
    Core/SIMDAccelerator.cpp
    Core/TelemetryEngine.cpp
    Core/ETWTraceProvider.cpp
    Core/ThemeEngine.cpp
    Core/UpdateEngine.cpp
    Core/WatchFolderEngine.cpp

    # Pipeline implementations
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
    Decoders/XCFDecoder.cpp
    Decoders/XPMDecoder.cpp

    # GPU rendering implementations
    GPU/D3D11Renderer.cpp
    GPU/D3D12ComputePipeline.cpp
    GPU/GDIRenderer.cpp
    GPU/VulkanComputePipeline.cpp

    # Cache implementations
    Cache/PersistentDiskCache.cpp
    Cache/ThumbnailCache.cpp

    # Plugin system implementations
    Plugin/CrashHandler.cpp
    Plugin/IPC/SharedMemoryManager.cpp
    Plugin/IsolationModeSelector.cpp
    Plugin/PluginDecoder.cpp
    Plugin/PluginHostClient.cpp
    Plugin/PluginManager.cpp
    Plugin/PluginMarketplaceV2.cpp
    Plugin/Security/JobObjectManager.cpp

    # Utility implementations
    Utils/AccessibilityEngine.cpp
    Utils/ARM64HardwareValidator.cpp
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
    Utils/LocalizationEngine.cpp
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
    Utils/TestSuiteExpansion.cpp
    Utils/UsageTelemetryEngine.cpp
    Utils/Win11Integration.cpp

    # Release gate implementation
    Utils/ReleaseGateUnified.cpp
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
            ${CMAKE_CURRENT_SOURCE_DIR}/../external/image-libs/libwebp-1.5.0-original/src
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
                ${CMAKE_CURRENT_SOURCE_DIR}/../external/compression-libs/zstd-1.5.7/build-manual
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
            /W4           # Warning level 4
            /WX-          # Warnings NOT as errors during consolidation
            /permissive-  # Standards conformance
            /arch:AVX2    # Enable AVX2 instructions for SIMD operations
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
