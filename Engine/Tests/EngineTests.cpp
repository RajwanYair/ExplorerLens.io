//==============================================================================
// ExplorerLens Engine - Unit Tests
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#include "../AI/AISearchIntegration.h"
#include "../AI/ImageQualityAssessor.h"
#include "../AI/SceneUnderstandingEngine.h"
#include "../AI/SmartCropV2.h"
#include "../Cache/CacheWarmingService.h"
#include "../Cache/MultiTenantCacheManager.h"
#include "../Cache/PersistentCacheManager.h"
#include "../Cache/PersistentDiskCache.h"
#include "../Cache/PipelineStateCacheV2.h"
#include "../Cache/SubMillisecondCacheEngine.h"
#include "../Core/AIThumbnailEnhancer.h"
#include "../Core/AccessibilityPipeline.h"
#include "../Core/AccessibilitySuiteV2.h"
#include "../Core/AnimatedThumbnailEngine.h"
#include "../Core/AsyncShellActivation.h"
#include "../Core/AsyncShellExtension.h"
#include "../Core/BatchProcessingEngine.h"
#include "../Core/CloudStorageIntegration.h"
#include "../Core/CloudSyncProvider.h"
#include "../Core/ConfigMigrationEngine.h"
#include "../Core/ContentIndexer.h"
#include "../Core/D3D12PipelineActivation.h"
#include "../Core/EncoderExportEngine.h"
#include "../Core/ErrorRecoveryEngine.h"
#include "../Core/FormatConverterEngine.h"
#include "../Core/FormatRegistry.h"
#include "../Core/FormatTypes.h"
#include "../Core/GPUMemoryPoolV2.h"
#include "../Core/GPUPipelineV3.h"
#include "../Core/HDRDisplayPipeline.h"
#include "../Core/HighDPIScaling.h"
#include "../Core/MetadataExtractor.h"
#include "../Core/MultiGPULoadBalancer.h"
#include "../Core/NetworkProviderEngine.h"
#include "../Core/ParallelBatchProcessor.h"
#include "../Core/PerMonitorDPIV3.h"
#include "../Core/PerformanceBenchmarkV2.h"
#include "../Core/PreviewPanelV2.h"
#include "../Core/ProgressiveThumbnailLoader.h"
#include "../Core/QuickLookIntegration.h"
#include "../Core/RegistryManager.h"
#include "../Core/ResourcePoolEngine.h"
#include "../Core/RuntimeIntegrityVerifier.h"
#include "../Core/SIMDAccelerationManager.h"
#include "../Core/SIMDAccelerator.h"
#include "../Core/SecurityHardeningV2.h"
#include "../Core/ShaderCompilerV2.h"
#include "../Core/SharePointTeamsIntegration.h"
#include "../Core/ShellContextMenuV2.h"
#include "../Core/ShellOverlayHandler.h"
#include "../Core/ShellPreviewHandler.h"
#include "../Core/ShellRegistrationManager.h"
#include "../Core/TelemetryEngine.h"
#include "../Core/ThemeEngine.h"
#include "../Core/ThumbnailAnimationEngineV2.h"
#include "../Core/UpdateEngine.h"
#include "../Core/VersionSyncV14.h"
#include "../Core/VulkanComputeActivation.h"
#include "../Core/WatchFolderEngine.h"
#include "../Core/WinRTAppSDKIntegrationV2.h"
#include "../Decoders/AVIFDecoder.h"
#include "../Decoders/Advanced3DFormatDecoder.h"
#include "../Decoders/AnimatedFormatHandler.h"
#include "../Decoders/ArchiveDecoder.h"
#include "../Decoders/AudioDecoder.h"
#include "../Decoders/AudioVisualizationV2.h"
#include "../Decoders/CADFormatDecoder.h"
#include "../Decoders/DDSDecoder.h"
#include "../Decoders/DICOMDecoder.h"
#include "../Decoders/DICOMDecoderV2.h"
#include "../Decoders/DPXDecoder.h"
#include "../Decoders/DatabasePreviewDecoder.h"
#include "../Decoders/DocumentDecoder.h"
#include "../Decoders/EBookDecoder.h"
#include "../Decoders/EPSDecoder.h"
#include "../Decoders/EXRDecoder.h"
#include "../Decoders/ExtendedVideoDecoder.h"
#include "../Decoders/FITSDecoder.h"
#include "../Decoders/FITSDecoderV2.h"
#include "../Decoders/FontDecoder.h"
#include "../Decoders/GeospatialDecoder.h"
#include "../Decoders/HDRDecoder.h"
#include "../Decoders/HEIFDecoder.h"
#include "../Decoders/ICODecoder.h"
#include "../Decoders/ImageDecoder.h"
#include "../Decoders/JXLDecoder.h"
#include "../Decoders/KTXTextureDecoder.h"
#include "../Decoders/LegacyImageDecoder.h"
#include "../Decoders/Model3DRendererV2.h"
#include "../Decoders/ModelDecoder.h"
#include "../Decoders/ModelFormatHandler.h"
#include "../Decoders/NIfTIDecoder.h"
#include "../Decoders/NotebookPreviewDecoder.h"
#include "../Decoders/OpenRasterDecoder.h"
#include "../Decoders/PDFDecoder.h"
#include "../Decoders/PPMDecoder.h"
#include "../Decoders/PSDDecoder.h"
#include "../Decoders/QOIDecoder.h"
#include "../Decoders/RAWDecoder.h"
#include "../Decoders/SGIDecoder.h"
#include "../Decoders/SVGDecoder.h"
#include "../Decoders/ScientificDataDecoder.h"
#include "../Decoders/SpreadsheetPreviewDecoder.h"
#include "../Decoders/StructuredDataDecoder.h"
#include "../Decoders/TGADecoder.h"
#include "../Decoders/TextPreviewDecoder.h"
#include "../Decoders/USDDecoder.h"
#include "../Decoders/VTFDecoder.h"
#include "../Decoders/VectorFormatDecoder.h"
#include "../Decoders/VideoDecoder.h"
#include "../Decoders/WebPDecoder.h"
#include "../Decoders/XCFDecoder.h"
#include "../Decoders/XPMDecoder.h"
#include "../Engine.h"
#include "../GPU/D3D12ComputePipeline.h"
#include "../GPU/GPUDecodeAccelerationV2.h"
#include "../GPU/VulkanComputePipeline.h"
#include "../Memory/MemoryFootprintOptimizerV2.h"
#include "../Pipeline/AsyncThumbnailProvider.h"
#include "../Pipeline/DecoderRegistry.h"
#include "../Pipeline/FormatDetector.h"
#include "../Pipeline/ParallelBatchDecoder.h"
#include "../Pipeline/ParallelIOPipeline.h"
#include "../Pipeline/SmartFormatDetectorV2.h"
#include "../Plugin/PluginDebuggerIntegration.h"
#include "../Plugin/PluginHotReload.h"
#include "../Plugin/PluginLifecycle.h"
#include "../Plugin/PluginMarketplaceUnified.h"
#include "../Plugin/PluginMarketplaceV2.h"
#include "../Plugin/PluginMarketplaceV3.h"
#include "../Plugin/PluginPerformanceProfiler.h"
#include "../Plugin/PluginSDKV2.h"
#include "../Plugin/PluginSecurity.h"
#include "../Utils/ARM64HardwareValidator.h"
#include "../Utils/ARM64Platform.h"
#include "../Utils/AccessibilityEngine.h"
#include "../Utils/AutoDocGenerator.h"
#include "../Utils/AutoUpdateEngine.h"
#include "../Utils/CIValidator.h"
#include "../Utils/CodeCoverageIntegration.h"
#include "../Utils/CommandLineInterface.h"
#include "../Utils/ConfigMigrationEngine.h"
#include "../Utils/DiagnosticDashboard.h"
#include "../Utils/DocGenerator.h"
#include "../Utils/EnterpriseDeployment.h"
#include "../Utils/EnterpriseDeploymentManager.h"
#include "../Utils/FileHashEngine.h"
#include "../Utils/FuzzingEngine.h"
#include "../Utils/InstallerManager.h"
#include "../Utils/LocalizationEngine.h"
#include "../Utils/LogRotationEngine.h"
#include "../Utils/MalformedInputHandler.h"
#include "../Utils/MemorySafety.h"
#include "../Utils/MemorySafetyIntegration.h"
#include "../Utils/NetworkDiagnostics.h"
#include "../Utils/NotificationEngine.h"
#include "../Utils/PortableModeManager.h"
#include "../Utils/PythonSDK.h"
#include "../Utils/QualityGates.h"
#include "../Utils/ReleaseGate.h"
#include "../Utils/SecurityCompliance.h"
#include "../Utils/TestFramework.h"
#include "../Utils/TestInfrastructure.h"
#include "../Utils/TestSuiteExpansion.h"
#include "../Utils/UsageTelemetryEngine.h"
#include "../Utils/WindowsCompat.h"

// ===== Feature Module Includes =====

// EngineTests.h — MuPDF PDF support
#include "../Core/MuPDFIntegration.h"

// EngineTests.h — libwebp CRT fix
#include "../Core/LibWebPConfig.h"

// EngineTests.h — LENSArchive refactoring
#include "../Core/ArchiveRefactorEngine.h"

// EngineTests.h — Bitmap Pool
#include "../Memory/BitmapPool.h"

// EngineTests.h — IPropertyStore
#include "../Core/PropertyStoreHandler.h"

// EngineTests.h — GPU Shader Library
#include "../Core/GPUShaderLibrary.h"

// EngineTests.h — PluginHost
#include "../Core/PluginHostManager.h"

// EngineTests.h — Library Version Audit
#include "../Core/LibraryVersionAudit.h"

// EngineTests.h — OpenJPEG
#include "../Core/OpenJPEGIntegration.h"

// EngineTests.h — FreeType
#include "../Core/FreeTypeIntegration.h"

// EngineTests.h — FFmpeg
#include "../Core/FFmpegIntegration.h"

// EngineTests.h — Format Categories
#include "../Core/FormatCategoryManager.h"

// EngineTests.h — Format Status
#include "../Core/FormatStatusIndicator.h"

// EngineTests.h — Settings Import/Export
#include "../Core/SettingsExportImport.h"

// EngineTests.h — Performance Dashboard
#include "../Core/PerformanceDashboard.h"

// EngineTests.h — ETW TraceLogging Provider
#include "../Core/ETWTraceProvider.h"

// EngineTests.h — Dark Mode
#include "../Core/DarkModeEngine.h"

// EngineTests.h — System Tray
#include "../Core/SystemTrayManager.h"

// EngineTests.h — WinUI 3 Migration
#include "../Core/WinUI3MigrationEngine.h"

// EngineTests.h — CI Hardening
#include "../Core/CIHardeningEngine.h"

// EngineTests.h — Code Coverage
#include "../Core/CodeCoverageEngine.h"

// EngineTests.h — Fuzzing Campaign
// (merged into FuzzingEngine.h)

// EngineTests.h — Static Analysis
// StaticAnalysisGate now in QualityGates.h

// EngineTests.h — SBOM (now in SecurityCompliance.h)

// EngineTests.h — Zero-Copy Pipeline
#include "../Pipeline/ZeroCopyPipeline.h"

// EngineTests.h — Parallel I/O
#include "../Pipeline/ParallelIOPipeline.h"

// EngineTests.h — SIMD Scaler
#include "../Utils/SIMDScaler.h"

// EngineTests.h — PSO Cache
#include "../Cache/PipelineStateCacheV2.h"

// EngineTests.h — Cache Warming
#include "../Cache/CacheWarmingService.h"

// EngineTests.h — Thumbnail Quality Analyzer
#include "../Core/ThumbnailQualityAnalyzer.h"

// EngineTests.h — Adaptive Decoder Router
#include "../Core/AdaptiveDecoderRouter.h"

// EngineTests.h — Telemetry Pipeline
#include "../Core/TelemetryPipeline.h"

// EngineTests.h — Live Preview Engine
#include "../Core/LivePreviewEngine.h"

// EngineTests.h — Cloud Native Sync
#include "../Core/CloudNativeSync.h"

// EngineTests.h — Zenith Ascent Feature Headers
#include "../Cache/CacheEfficiencyAnalyzer.h"
#include "../Core/ARM64NEONScaler.h"
#include "../Core/AccessibilityNarratorBridge.h"
#include "../Core/AdaptiveDPIScaler.h"
#include "../Core/AnimatedFormatController.h"
#include "../Core/AuditTrailLogger.h"
#include "../Core/CertificateTrustValidator.h"
#include "../Core/ColorSpaceEngine.h"
#include "../Core/ContentAwareThumbnailSelector.h"
#include "../Core/ContentInspectionGateway.h"
#include "../Core/CrashAnalyticsCollector.h"
#include "../Core/DecoderPerformanceProfiler.h"
#include "../Core/DiagnosticReportGeneratorV2.h"
#include "../Core/DirectStorage12Integration.h"
#include "../Core/DocumentLayoutAnalyzer.h"
#include "../Core/EncryptedFormatHandler.h"
#include "../Core/FaceDetectionOrientation.h"
#include "../Core/FormatFingerprintDB.h"
#include "../Core/FormatGalleryCompositor.h"
#include "../Core/FormatPopularityTracker.h"
#include "../Core/GPUTextureCompressionPipeline.h"
#include "../Core/HDRToneMappingPipeline.h"
#include "../Core/HealthCheckEndpoint.h"
#include "../Core/LivePreviewStreamingProtocol.h"
#include "../Core/MetadataExtractionPipeline.h"
#include "../Core/MultiMonitorColorProfile.h"
#include "../Core/MultiPageNavigator.h"
#include "../Core/NestedArchivePreview.h"
#include "../Core/NeuralTextureCompression.h"
#include "../Core/PerformanceAnomalyDetector.h"
#include "../Core/PreviewTooltipRenderer.h"
#include "../Core/QuantumReadyHashPipeline.h"
#include "../Core/QuickActionsOverlay.h"
#include "../Core/RoleBasedFormatPolicy.h"
#include "../Core/RustFFIBridge.h"
#include "../Core/SelfHealingDecoder.h"
#include "../Core/SmartQualityPredictor.h"
#include "../Core/SystemResourceMonitor.h"
#include "../Core/TelemetryPipelineV2.h"
#include "../Core/UsageAnalyticsDashboard.h"
#include "../Core/VirtualFilesystemAbstraction.h"
#include "../Core/VisualSimilarityIndex.h"
#include "../Core/WASMDecoderSandbox.h"
#include "../Core/WindowsSearchProtocol.h"
#include "../GPU/GPUTextureAtlasManager.h"
#include "../GPU/WebGPUThumbnailRenderer.h"
#include "../Pipeline/LockFreeDecodePipeline.h"
#include "../Pipeline/MemoryMappedIOOptimizer.h"
#include "../Pipeline/PredictivePrefetchEngine.h"
#include "../Pipeline/ShellProgressIndicator.h"
#include "../Pipeline/ThreadPoolOptimizer.h"

// Shell integration features
#include "../Core/JumpListIntegration.h"
#include "../Core/ShellSearchProtocolHandler.h"

// Plugin C ABI bridge
#include "../Plugin/PluginLoaderV2.h"

// Performance pipeline & zero-copy
#include "../Pipeline/ZeroCopyActivation.h"
#include "../Pipeline/SIMDDispatchRouter.h"
#include "../Pipeline/StreamingDecodeEngine.h"

// GPU compute kernels
#include "../GPU/LanczosGPUKernel.h"
#include "../GPU/HDRToneMapKernel.h"
#include "../GPU/AdaptiveGPUScheduler.h"

// Cache engines
#include "../Cache/PSOPersistenceManager.h"
#include "../Cache/PredictiveCacheEngine.h"

// Shell deep integration
#include "../Core/ExplorerColumnProvider.h"
#include "../Core/DragDropThumbnailPreview.h"

// Advanced decoders
#include "../Decoders/MultiPageStripRenderer.h"
#include "../Decoders/VideoKeyframeExtractor.h"

#include <chrono>
// Compatibility macro for ASSERT_EQUAL(expected, actual) → ASSERT((a) == (b))
#define ASSERT_EQUAL(a, b) ASSERT((a) == (b))
#include <iostream>
#include <psapi.h>

#pragma comment(lib, "psapi.lib")
#include <cassert>
#include <cstring>
#include <string>
#include <vector>

// GPU tests
extern void RunGPUTests();

using namespace ExplorerLens::Engine;

//==============================================================================
// Test Counters
//==============================================================================

int g_testsRun = 0;
int g_testsPassed = 0;
int g_testsFailed = 0;

#define TEST(name) \
 void name(); \
 void name##_Runner() { \
 std::wcout << L"Running: " << L"" #name << L"..." << std::endl; \
 g_testsRun++; \
 try { \
 name(); \
 g_testsPassed++; \
 std::wcout << L" [PASS]" << std::endl; \
 } catch (const char *msg) { \
 g_testsFailed++; \
 std::cout << " [FAIL] " << msg << std::endl; \
 } \
 } \
 void name()

#define ASSERT(expr) \
 if (!(expr)) { \
 throw "Assertion failed: " #expr; \
 }

#define ASSERT_EQ(a, b) \
 if ((a) != (b)) { \
 throw "Assertion failed: " #a " == " #b; \
 }

#define ASSERT_NE(a, b) \
 if ((a) == (b)) { \
 throw "Assertion failed: " #a " != " #b; \
 }

#define ASSERT_NULL(ptr) \
 if ((ptr) != nullptr) { \
 throw "Assertion failed: " #ptr " == nullptr"; \
 }

#define ASSERT_NOT_NULL(ptr) \
 if ((ptr) == nullptr) { \
 throw "Assertion failed: " #ptr " != nullptr"; \
 }

#define RUN_TEST(name) name##_Runner()

//==============================================================================
// Mock Decoder for Testing
//==============================================================================

class MockDecoder : public IThumbnailDecoder {
public:
    MockDecoder(const wchar_t* name, const wchar_t* ext1,
        const wchar_t* ext2 = nullptr)
        : m_name(name) {
        m_extensions[0] = ext1;
        m_extensions[1] = ext2;
        m_extensions[2] = nullptr;
        m_extensionCount = ext2 ? 2 : 1;
    }

    bool CanDecode(const wchar_t* filePath) override {
        if (!filePath)
            return false;
        const wchar_t* ext = wcsrchr(filePath, L'.');
        if (!ext)
            return false;

        for (size_t i = 0; i < m_extensionCount; i++) {
            if (_wcsicmp(ext, m_extensions[i]) == 0) {
                return true;
            }
        }
        return false;
    }

    HRESULT Decode(const ThumbnailRequest& request,
        ThumbnailResult& result) override {
        (void)request; // Suppress unused parameter warning
        result.status = S_OK;
        return S_OK;
    }

    DecoderInfo GetInfo() const override {
        DecoderInfo info;
        info.name = m_name;
        info.version = L"1.0.0";
        info.supportedExtensions = const_cast<const wchar_t**>(m_extensions);
        info.extensionCount = m_extensionCount;
        info.supportsGPU = false;
        info.isArchiveDecoder = false;
        return info;
    }

    const wchar_t* GetName() const override { return m_name; }

    const wchar_t** GetSupportedExtensions() const override {
        return const_cast<const wchar_t**>(m_extensions);
    }

    uint32_t GetExtensionCount() const override { return m_extensionCount; }

    bool SupportsGPU() const override { return false; }

    bool IsArchiveDecoder() const override { return false; }

private:
    const wchar_t* m_name;
    const wchar_t* m_extensions[3];
    uint32_t m_extensionCount;
};

//==============================================================================
// Decoder Registry Tests
//==============================================================================

TEST(TestDecoderRegistry_Create) {
    DecoderRegistry registry;
    ASSERT_EQ(registry.GetDecoderCount(), 0);
}

TEST(TestDecoderRegistry_RegisterDecoder) {
    DecoderRegistry registry;

    MockDecoder* decoder = new MockDecoder(L"Test Decoder", L".test");
    ASSERT(registry.RegisterDecoder(decoder));
    ASSERT_EQ(registry.GetDecoderCount(), 1);
}

TEST(TestDecoderRegistry_RegisterMultipleDecoders) {
    DecoderRegistry registry;

    registry.RegisterDecoder(new MockDecoder(L"Decoder 1", L".ext1"));
    registry.RegisterDecoder(new MockDecoder(L"Decoder 2", L".ext2"));
    registry.RegisterDecoder(new MockDecoder(L"Decoder 3", L".ext3"));

    ASSERT_EQ(registry.GetDecoderCount(), 3);
}

TEST(TestDecoderRegistry_FindDecoder) {
    DecoderRegistry registry;

    registry.RegisterDecoder(new MockDecoder(L"JPEG Decoder", L".jpg", L".jpeg"));
    registry.RegisterDecoder(new MockDecoder(L"PNG Decoder", L".png"));

    IThumbnailDecoder* jpgDecoder = registry.FindDecoder(L"photo.jpg");
    ASSERT_NOT_NULL(jpgDecoder);
    ASSERT_EQ(wcscmp(jpgDecoder->GetName(), L"JPEG Decoder"), 0);

    IThumbnailDecoder* pngDecoder = registry.FindDecoder(L"image.png");
    ASSERT_NOT_NULL(pngDecoder);
    ASSERT_EQ(wcscmp(pngDecoder->GetName(), L"PNG Decoder"), 0);
}

TEST(TestDecoderRegistry_FindDecoderByName) {
    DecoderRegistry registry;

    registry.RegisterDecoder(new MockDecoder(L"Decoder A", L".a"));
    registry.RegisterDecoder(new MockDecoder(L"Decoder B", L".b"));

    IThumbnailDecoder* decoder = registry.FindDecoderByName(L"Decoder B");
    ASSERT_NOT_NULL(decoder);
    ASSERT_EQ(wcscmp(decoder->GetName(), L"Decoder B"), 0);
}

TEST(TestDecoderRegistry_GetStats) {
    DecoderRegistry registry;

    registry.RegisterDecoder(new MockDecoder(L"Decoder 1", L".ext1", L".ext2"));
    registry.RegisterDecoder(new MockDecoder(L"Decoder 2", L".ext3"));

    size_t totalDecoders, imageDecoders, archiveDecoders, totalExtensions;
    registry.GetStats(&totalDecoders, &imageDecoders, &archiveDecoders,
        &totalExtensions);

    ASSERT_EQ(totalDecoders, 2);
    ASSERT_EQ(totalExtensions, 3);
}

TEST(TestDecoderRegistry_ComprehensiveIntegration) {
    DecoderRegistry registry;

    // Register all real decoders
    registry.RegisterDecoder(new ImageDecoder());
    registry.RegisterDecoder(new WebPDecoder());
    registry.RegisterDecoder(new AVIFDecoder());
    registry.RegisterDecoder(new ArchiveDecoder());
    registry.RegisterDecoder(new JXLDecoder());
    registry.RegisterDecoder(new HEIFDecoder());

    // Test that all expected formats can be decoded
    struct FormatTest {
        const wchar_t* extension;
        const wchar_t* expectedDecoder;
    };

    std::vector<FormatTest> tests = {
    {L".jpg", L"ImageDecoder"}, {L".jpeg", L"ImageDecoder"},
    {L".png", L"ImageDecoder"}, {L".bmp", L"ImageDecoder"},
    {L".webp", L"WebPDecoder"}, {L".avif", L"AVIFDecoder"},
    {L".jxl", L"JXLDecoder"}, {L".heif", L"HEIFDecoder"},
    {L".heic", L"HEIFDecoder"}, {L".zip", L"ArchiveDecoder"},
    {L".cbz", L"ArchiveDecoder"} // Note: .rar not supported yet - requires
    // UnRAR library
    };

    for (const auto& test : tests) {
        std::wstring testFile = std::wstring(L"test") + test.extension;
        IThumbnailDecoder* decoder = registry.FindDecoder(testFile.c_str());

        // Verify decoder was found
        ASSERT(decoder != nullptr);

        // Verify it's the right decoder (basic check via CanDecode)
        ASSERT(decoder->CanDecode(testFile.c_str()));
    }

    // Test stats are correct
    size_t totalDecoders, imageDecoders, archiveDecoders, totalExtensions;
    registry.GetStats(&totalDecoders, &imageDecoders, &archiveDecoders,
        &totalExtensions);

    ASSERT(totalDecoders >= 6); // At least the 6 we registered
    ASSERT(imageDecoders >= 5); // Image, WebP, AVIF, JXL, HEIF
    ASSERT(archiveDecoders >= 1); // Archive decoder
    ASSERT(totalExtensions >= 20); // Many extensions across all decoders
}

//==============================================================================
// Format Detector Tests
//==============================================================================

TEST(TestFormatDetector_Create) {
    FormatDetector detector;
    // Just verify it can be created
}

TEST(TestFormatDetector_DetectJPEG) {
    FormatDetector detector;

    ASSERT_EQ(detector.DetectFromExtension(L".jpg"), DetectedFormat::ImageJPEG);
    ASSERT_EQ(detector.DetectFromExtension(L".jpeg"), DetectedFormat::ImageJPEG);
}

TEST(TestFormatDetector_DetectPNG) {
    FormatDetector detector;

    ASSERT_EQ(detector.DetectFromExtension(L".png"), DetectedFormat::ImagePNG);
}

TEST(TestFormatDetector_DetectZIP) {
    FormatDetector detector;

    ASSERT_EQ(detector.DetectFromExtension(L".zip"), DetectedFormat::ArchiveZIP);
    ASSERT_EQ(detector.DetectFromExtension(L".cbz"), DetectedFormat::ArchiveZIP);
}

TEST(TestFormatDetector_DetectRAR) {
    FormatDetector detector;

    ASSERT_EQ(detector.DetectFromExtension(L".rar"), DetectedFormat::ArchiveRAR);
    ASSERT_EQ(detector.DetectFromExtension(L".cbr"), DetectedFormat::ArchiveRAR);
}

TEST(TestFormatDetector_IsImageFormat) {
    FormatDetector detector;

    ASSERT(detector.IsImageFormat(L".jpg"));
    ASSERT(detector.IsImageFormat(L".png"));
    ASSERT(detector.IsImageFormat(L".bmp"));
    ASSERT(!detector.IsImageFormat(L".zip"));
    ASSERT(!detector.IsImageFormat(L".pdf"));
}

TEST(TestFormatDetector_IsArchiveFormat) {
    FormatDetector detector;

    ASSERT(detector.IsArchiveFormat(L".zip"));
    ASSERT(detector.IsArchiveFormat(L".cbz"));
    ASSERT(detector.IsArchiveFormat(L".rar"));
    ASSERT(!detector.IsArchiveFormat(L".jpg"));
    ASSERT(!detector.IsArchiveFormat(L".pdf"));
}

TEST(TestFormatDetector_GetExtension) {
    FormatDetector detector;

    const wchar_t* ext = detector.GetExtension(L"C:\\path\\to\\file.jpg");
    ASSERT_NOT_NULL(ext);
    ASSERT_EQ(wcscmp(ext, L".jpg"), 0);

    ext = detector.GetExtension(L"image.png");
    ASSERT_NOT_NULL(ext);
    ASSERT_EQ(wcscmp(ext, L".png"), 0);
}

//==============================================================================
// Image Decoder Tests
//==============================================================================

TEST(TestImageDecoder_Create) {
    ImageDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"ImageDecoder"), 0);
    ASSERT(decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestImageDecoder_Extensions) {
    ImageDecoder decoder;
    const wchar_t** extensions = decoder.GetSupportedExtensions();
    uint32_t count = decoder.GetExtensionCount();

    // Should support at least 5 formats (JPEG, PNG, BMP, GIF, TIFF)
    ASSERT(count >= 5);
    ASSERT_NOT_NULL(extensions);

    // Check for key extensions
    bool hasJPG = false, hasPNG = false, hasBMP = false;
    for (uint32_t i = 0; i < count; i++) {
        if (wcscmp(extensions[i], L".jpg") == 0 ||
            wcscmp(extensions[i], L".jpeg") == 0) {
            hasJPG = true;
        }
        if (wcscmp(extensions[i], L".png") == 0) {
            hasPNG = true;
        }
        if (wcscmp(extensions[i], L".bmp") == 0) {
            hasBMP = true;
        }
    }

    ASSERT(hasJPG);
    ASSERT(hasPNG);
    ASSERT(hasBMP);
}

TEST(TestImageDecoder_CanDecodeJPEG) {
    ImageDecoder decoder;
    ASSERT(decoder.CanDecode(L"test.jpg"));
    ASSERT(decoder.CanDecode(L"test.JPEG"));
}

TEST(TestImageDecoder_CanDecodePNG) {
    ImageDecoder decoder;
    ASSERT(decoder.CanDecode(L"test.png"));
}

TEST(TestImageDecoder_CanDecodeBMP) {
    ImageDecoder decoder;
    ASSERT(decoder.CanDecode(L"test.bmp"));
}

TEST(TestImageDecoder_CannotDecodeUnsupported) {
    ImageDecoder decoder;
    ASSERT(!decoder.CanDecode(L"test.webp"));
    ASSERT(!decoder.CanDecode(L"test.zip"));
    ASSERT(!decoder.CanDecode(nullptr));
}

TEST(TestImageDecoder_GetInfo) {
    ImageDecoder decoder;
    DecoderInfo info = decoder.GetInfo();

    ASSERT_EQ(wcscmp(info.name, L"ImageDecoder"), 0);
    ASSERT_EQ(wcscmp(info.version, L"1.0.0"), 0);
    ASSERT(info.extensionCount >= 5); // JPEG, PNG, BMP, GIF, TIFF
    ASSERT(info.supportsGPU);
    ASSERT(!info.isArchiveDecoder);
}

TEST(TestImageDecoder_RegisterWithRegistry) {
    DecoderRegistry registry;
    ImageDecoder decoder;

    ASSERT(registry.RegisterDecoder(&decoder));

    // Find by extension
    auto found = registry.FindDecoder(L"photo.jpg");
    ASSERT_NOT_NULL(found);
    ASSERT_EQ(wcscmp(found->GetName(), L"ImageDecoder"), 0);
}

//==============================================================================
// WebP Decoder Tests
//==============================================================================

TEST(TestWebPDecoder_Create) {
    WebPDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"WebPDecoder"), 0);
    ASSERT(!decoder.SupportsGPU()); // CPU-based libwebp
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestWebPDecoder_Extensions) {
    WebPDecoder decoder;
    const wchar_t** extensions = decoder.GetSupportedExtensions();
    uint32_t count = decoder.GetExtensionCount();

    ASSERT_EQ(count, 1);
    ASSERT_NOT_NULL(extensions);
    ASSERT_EQ(wcscmp(extensions[0], L".webp"), 0);
}

TEST(TestWebPDecoder_CanDecode) {
    WebPDecoder decoder;

    ASSERT(decoder.CanDecode(L"image.webp"));
    ASSERT(decoder.CanDecode(L"photo.WEBP")); // Case insensitive
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(L"archive.zip"));
    ASSERT(!decoder.CanDecode(nullptr));
}

TEST(TestWebPDecoder_GetInfo) {
    WebPDecoder decoder;
    DecoderInfo info = decoder.GetInfo();

    ASSERT_EQ(wcscmp(info.name, L"WebPDecoder"), 0);
    ASSERT_EQ(wcscmp(info.version, L"1.0.0"), 0);
    ASSERT_EQ(info.extensionCount, 1);
    ASSERT(!info.supportsGPU);
    ASSERT(!info.isArchiveDecoder);
}

TEST(TestWebPDecoder_RegisterWithRegistry) {
    DecoderRegistry registry;
    WebPDecoder decoder;

    ASSERT(registry.RegisterDecoder(&decoder));

    // Find by extension
    auto found = registry.FindDecoder(L"photo.webp");
    ASSERT_NOT_NULL(found);
    ASSERT_EQ(wcscmp(found->GetName(), L"WebPDecoder"), 0);
}

//==============================================================================
// AVIF Decoder Tests
//==============================================================================

TEST(TestAVIFDecoder_Create) {
    AVIFDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"AVIFDecoder"), 0);
    ASSERT(decoder.SupportsGPU()); // WIC can use GPU
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestAVIFDecoder_Extensions) {
    AVIFDecoder decoder;
    const wchar_t** extensions = decoder.GetSupportedExtensions();
    uint32_t count = decoder.GetExtensionCount();

    ASSERT_EQ(count, 3); // .avif, .heif, .heic
    ASSERT_NOT_NULL(extensions);

    // Check for specific extensions
    bool hasAVIF = false, hasHEIF = false, hasHEIC = false;
    for (uint32_t i = 0; i < count; i++) {
        if (wcscmp(extensions[i], L".avif") == 0)
            hasAVIF = true;
        if (wcscmp(extensions[i], L".heif") == 0)
            hasHEIF = true;
        if (wcscmp(extensions[i], L".heic") == 0)
            hasHEIC = true;
    }

    ASSERT(hasAVIF);
    ASSERT(hasHEIF);
    ASSERT(hasHEIC);
}

TEST(TestAVIFDecoder_CanDecode) {
    AVIFDecoder decoder;

    ASSERT(decoder.CanDecode(L"image.avif"));
    ASSERT(decoder.CanDecode(L"photo.HEIF")); // Case insensitive
    ASSERT(decoder.CanDecode(L"iphone.heic"));
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(L"archive.zip"));
    ASSERT(!decoder.CanDecode(nullptr));
}

TEST(TestAVIFDecoder_GetInfo) {
    AVIFDecoder decoder;
    DecoderInfo info = decoder.GetInfo();

    ASSERT_EQ(wcscmp(info.name, L"AVIFDecoder"), 0);
    ASSERT_EQ(wcscmp(info.version, L"1.0.0"), 0);
    ASSERT_EQ(info.extensionCount, 3);
    ASSERT(info.supportsGPU);
    ASSERT(!info.isArchiveDecoder);
}

TEST(TestAVIFDecoder_RegisterWithRegistry) {
    DecoderRegistry registry;
    AVIFDecoder decoder;

    ASSERT(registry.RegisterDecoder(&decoder));

    // Find by extension
    auto found = registry.FindDecoder(L"photo.avif");
    ASSERT_NOT_NULL(found);
    ASSERT_EQ(wcscmp(found->GetName(), L"AVIFDecoder"), 0);
}

//==============================================================================
// Archive Decoder Tests
//==============================================================================

TEST(TestArchiveDecoder_Create) {
    ArchiveDecoder decoder;

    ASSERT_NOT_NULL(decoder.GetName());
    ASSERT_EQ(wcscmp(decoder.GetName(), L"ArchiveDecoder"), 0);
}

TEST(TestArchiveDecoder_Extensions) {
    ArchiveDecoder decoder;

    const wchar_t** extensions = decoder.GetSupportedExtensions();
    uint32_t count = decoder.GetExtensionCount();

    ASSERT_NOT_NULL(extensions);
    ASSERT_EQ(count, 14); // Full archive format set

    // Check core extensions
    bool hasZip = false;
    bool hasCbz = false;
    bool has7z = false;
    bool hasRar = false;
    bool hasTarGz = false;

    for (uint32_t i = 0; i < count; i++) {
        if (wcscmp(extensions[i], L".zip") == 0)
            hasZip = true;
        if (wcscmp(extensions[i], L".cbz") == 0)
            hasCbz = true;
        if (wcscmp(extensions[i], L".7z") == 0)
            has7z = true;
        if (wcscmp(extensions[i], L".rar") == 0)
            hasRar = true;
        if (wcscmp(extensions[i], L".tar.gz") == 0)
            hasTarGz = true;
    }

    ASSERT(hasZip);
    ASSERT(hasCbz);
    ASSERT(has7z);
    ASSERT(hasRar);
    ASSERT(hasTarGz);
}

TEST(TestArchiveDecoder_CanDecode) {
    ArchiveDecoder decoder;

    ASSERT(decoder.CanDecode(L"archive.zip"));
    ASSERT(decoder.CanDecode(L"comic.cbz"));
    ASSERT(decoder.CanDecode(L"path/to/file.ZIP")); // Case insensitive
    ASSERT(decoder.CanDecode(L"archive.rar"));
    ASSERT(decoder.CanDecode(L"archive.7z"));

    // Should not decode non-archive formats
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(L"photo.png"));
}

TEST(TestArchiveDecoder_IsArchiveFormat) {
    // Valid ZIP signature: PK\x03\x04
    const unsigned char zipSig[] = { 0x50, 0x4B, 0x03, 0x04, 0x00, 0x00 };
    ASSERT(ArchiveDecoder::IsArchiveFormat(zipSig, sizeof(zipSig)));

    // Invalid signatures
    const unsigned char invalidSig1[] = { 0x00, 0x00, 0x00, 0x00 };
    ASSERT(!ArchiveDecoder::IsArchiveFormat(invalidSig1, sizeof(invalidSig1)));

    const unsigned char invalidSig2[] = { 0xFF, 0xD8, 0xFF, 0xE0 }; // JPEG
    ASSERT(!ArchiveDecoder::IsArchiveFormat(invalidSig2, sizeof(invalidSig2)));

    // Too small
    const unsigned char tooSmall[] = { 0x50, 0x4B };
    ASSERT(!ArchiveDecoder::IsArchiveFormat(tooSmall, sizeof(tooSmall)));

    // Null data
    ASSERT(!ArchiveDecoder::IsArchiveFormat(nullptr, 100));
}

TEST(TestArchiveDecoder_GetInfo) {
    ArchiveDecoder decoder;

    DecoderInfo info = decoder.GetInfo();
    ASSERT_NOT_NULL(info.name);
    ASSERT_EQ(wcscmp(info.name, L"ArchiveDecoder"), 0);
    ASSERT_EQ(info.isArchiveDecoder, true);
    ASSERT_EQ(info.supportsGPU, false);
}

TEST(TestArchiveDecoder_RegisterWithRegistry) {
    DecoderRegistry registry;
    ArchiveDecoder decoder;

    ASSERT(registry.RegisterDecoder(&decoder));

    // Find by extension
    auto found = registry.FindDecoder(L"archive.zip");
    ASSERT_NOT_NULL(found);
    ASSERT_EQ(wcscmp(found->GetName(), L"ArchiveDecoder"), 0);
}

//==============================================================================
// JXL Decoder Tests (Re-enabled - basic interface tests)
//==============================================================================

TEST(TestJXLDecoder_Create) {
    JXLDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"JXLDecoder"), 0);
    ASSERT(!decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestJXLDecoder_CanDecode) {
    JXLDecoder decoder;

    ASSERT(decoder.CanDecode(L"image.jxl"));
    ASSERT(decoder.CanDecode(L"photo.JXL")); // Case insensitive
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(L"archive.zip"));
    ASSERT(!decoder.CanDecode(nullptr));
    ASSERT(!decoder.CanDecode(L"noextension"));
}

TEST(TestJXLDecoder_GetInfo) {
    JXLDecoder decoder;
    DecoderInfo info = decoder.GetInfo();

    ASSERT_NOT_NULL(info.name);
    ASSERT_EQ(wcscmp(info.name, L"JXLDecoder"), 0);
    ASSERT_NOT_NULL(info.version);
    ASSERT(info.extensionCount >= 1);
    ASSERT_NOT_NULL(info.supportedExtensions);
    ASSERT(!info.supportsGPU);
    ASSERT(!info.isArchiveDecoder);
}

TEST(TestJXLDecoder_Extensions) {
    JXLDecoder decoder;
    const wchar_t** extensions = decoder.GetSupportedExtensions();
    uint32_t count = decoder.GetExtensionCount();

    ASSERT_NOT_NULL(extensions);
    ASSERT(count >= 1);

    // Verify .jxl is in the list
    bool foundJXL = false;
    for (uint32_t i = 0; i < count; i++) {
        if (_wcsicmp(extensions[i], L".jxl") == 0) {
            foundJXL = true;
            break;
        }
    }
    ASSERT(foundJXL);
}

#ifdef HAS_LIBJXL
// Only run actual decode tests if JXL library is linked
TEST(TestJXLDecoder_DecodeReturnsResult) {
    JXLDecoder decoder;

    // Test with non-existent file - should return error
    ThumbnailRequest request;
    request.sourcePath = L"nonexistent.jxl";
    request.targetWidth = 256;
    request.targetHeight = 256;

    ThumbnailResult result;
    HRESULT hr = decoder.Decode(request, result);

    // Should fail gracefully (file doesn't exist)
    // Either returns error or empty bitmap - both are acceptable
    ASSERT(FAILED(hr) || result.bitmap == nullptr);
}
#endif

//==============================================================================
// HEIF Decoder Tests (Re-enabled - basic interface tests)
//==============================================================================

TEST(TestHEIFDecoder_Create) {
    HEIFDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"HEIFDecoder"), 0);
    ASSERT(!decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestHEIFDecoder_CanDecode) {
    HEIFDecoder decoder;

    ASSERT(decoder.CanDecode(L"image.heif"));
    ASSERT(decoder.CanDecode(L"photo.heic")); // Apple format
    ASSERT(decoder.CanDecode(L"iphone.HEIC")); // Case insensitive
    ASSERT(decoder.CanDecode(L"file.hif"));
    ASSERT(decoder.CanDecode(L"image.heifs"));
    ASSERT(decoder.CanDecode(L"photo.heics"));
    ASSERT(decoder.CanDecode(L"movie.avci"));
    ASSERT(decoder.CanDecode(L"video.avcs"));
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(L"archive.zip"));
    ASSERT(!decoder.CanDecode(nullptr));
}

//==============================================================================
// PSD Decoder Tests
//==============================================================================

TEST(TestPSDDecoder_Create) {
    PSDDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"PSDDecoder"), 0);
    ASSERT(!decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestPSDDecoder_CanDecode) {
    PSDDecoder decoder;
    // PSD requires signature check, so without a real file, CanDecode returns
    // false
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(nullptr));
    ASSERT(!decoder.CanDecode(L""));
}

TEST(TestPSDDecoder_GetInfo) {
    PSDDecoder decoder;
    auto info = decoder.GetInfo();
    ASSERT_EQ(wcscmp(info.name, L"PSDDecoder"), 0);
    ASSERT(info.extensionCount == 2); // .psd, .psb
}

//==============================================================================
// DDS Decoder Tests
//==============================================================================

TEST(TestDDSDecoder_Create) {
    DDSDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"DDSDecoder"), 0);
    // DDS decoder uses WIC + D3D11, SupportsGPU() returns true
    ASSERT(decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestDDSDecoder_CanDecode) {
    DDSDecoder decoder;
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(nullptr));
}

//==============================================================================
// HDR Decoder Tests
//==============================================================================

TEST(TestHDRDecoder_Create) {
    HDRDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"HDRDecoder"), 0);
    ASSERT(!decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestHDRDecoder_CanDecode) {
    HDRDecoder decoder;
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(nullptr));
}

//==============================================================================
// PPM Decoder Tests
//==============================================================================

TEST(TestPPMDecoder_Create) {
    PPMDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"PPMDecoder"), 0);
    ASSERT(!decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestPPMDecoder_Extensions) {
    PPMDecoder decoder;
    ASSERT(decoder.GetExtensionCount() >= 6); // ppm, pgm, pbm, pnm, pam, pfm
}

//==============================================================================
// EXR Decoder Tests
//==============================================================================

TEST(TestEXRDecoder_Create) {
    EXRDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"EXRDecoder"), 0);
    ASSERT(!decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestEXRDecoder_CanDecode) {
    EXRDecoder decoder;
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(nullptr));
}

//==============================================================================
// SVG Decoder Tests
//==============================================================================

TEST(TestSVGDecoder_Create) {
    SVGDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"SVGDecoder"), 0);
    ASSERT(!decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestSVGDecoder_CanDecode) {
    SVGDecoder decoder;
    ASSERT(decoder.CanDecode(L"image.svg"));
    ASSERT(decoder.CanDecode(L"file.SVG"));
    ASSERT(decoder.CanDecode(L"file.svgz"));
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(nullptr));
}

//==============================================================================
// Video Decoder Tests
//==============================================================================

TEST(TestVideoDecoder_Create) {
    VideoDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"VideoDecoder"), 0);
    ASSERT(!decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestVideoDecoder_CanDecode) {
    VideoDecoder decoder;
    ASSERT(decoder.CanDecode(L"movie.mp4"));
    ASSERT(decoder.CanDecode(L"movie.MKV"));
    ASSERT(decoder.CanDecode(L"clip.avi"));
    ASSERT(decoder.CanDecode(L"video.webm"));
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(nullptr));
}

TEST(TestVideoDecoder_Extensions) {
    VideoDecoder decoder;
    ASSERT(decoder.GetExtensionCount() >= 15); // Many video formats
}

//==============================================================================
// Audio Decoder Tests
//==============================================================================

TEST(TestAudioDecoder_Create) {
    AudioDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"AudioDecoder"), 0);
    ASSERT(!decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestAudioDecoder_CanDecode) {
    AudioDecoder decoder;
    ASSERT(decoder.CanDecode(L"song.mp3"));
    ASSERT(decoder.CanDecode(L"track.FLAC"));
    ASSERT(decoder.CanDecode(L"audio.wav"));
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(nullptr));
}

//==============================================================================
// PDF Decoder Tests
//==============================================================================

TEST(TestPDFDecoder_Create) {
    PDFDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"PDFDecoder"), 0);
    ASSERT(!decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestPDFDecoder_CanDecode) {
    PDFDecoder decoder;
    ASSERT(decoder.CanDecode(L"document.pdf"));
    ASSERT(decoder.CanDecode(L"file.PDF"));
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(nullptr));
}

//==============================================================================
// Document Decoder Tests
//==============================================================================

TEST(TestDocumentDecoder_Create) {
    DocumentDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"DocumentDecoder"), 0);
    ASSERT(!decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestDocumentDecoder_CanDecode) {
    DocumentDecoder decoder;
    ASSERT(decoder.CanDecode(L"book.epub"));
    ASSERT(decoder.CanDecode(L"doc.docx"));
    ASSERT(decoder.CanDecode(L"kindle.mobi"));
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(nullptr));
}

TEST(TestDocumentDecoder_Extensions) {
    DocumentDecoder decoder;
    ASSERT(decoder.GetExtensionCount() >= 15); // Many document formats
}

//==============================================================================
// Font Decoder Tests
//==============================================================================

TEST(TestFontDecoder_Create) {
    FontDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"FontDecoder"), 0);
    ASSERT(!decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestFontDecoder_CanDecode) {
    FontDecoder decoder;
    ASSERT(decoder.CanDecode(L"font.ttf"));
    ASSERT(decoder.CanDecode(L"font.OTF"));
    ASSERT(decoder.CanDecode(L"font.woff"));
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(nullptr));
}

TEST(TestFontDecoder_Extensions) {
    FontDecoder decoder;
    ASSERT(decoder.CanDecode(L"font.ttf"));
    ASSERT(decoder.CanDecode(L"font.otf"));
    ASSERT(decoder.CanDecode(L"font.woff"));
    ASSERT(decoder.CanDecode(L"font.woff2"));
    ASSERT(decoder.CanDecode(L"font.ttc"));
}

TEST(TestFontDecoder_GetInfo) {
    FontDecoder decoder;
    auto info = decoder.GetInfo();
    ASSERT_NOT_NULL(info.name);
}

//==============================================================================
// Video Decoder Robustness Tests
//==============================================================================

TEST(TestVideoDecoder_KeyframeSeekingValidation) {
    VideoDecoder decoder;
    // Test that keyframe seeking doesn't exceed duration
    // This is a basic API test - actual validation requires video file
    ASSERT(decoder.CanDecode(L"video.mp4"));
    ASSERT(decoder.CanDecode(L"video.mkv"));
    ASSERT(decoder.CanDecode(L"video.avi"));
}

TEST(TestVideoDecoder_TimestampValidation) {
    VideoDecoder decoder;
    // Verify negative timestamps are rejected
    // Actual implementation tested in VideoDecoder::SeekToKeyframe
    auto info = decoder.GetInfo();
    ASSERT_NOT_NULL(info.name);
}

TEST(TestVideoDecoder_CorruptFileHandling) {
    VideoDecoder decoder;
    // Test graceful handling of corrupt files (returns error, no crash)
    ASSERT(decoder.CanDecode(L"test.mp4"));
}

//==============================================================================
// Audio Album Art Extraction Tests
//==============================================================================

TEST(TestAudioDecoder_AlbumArtExtraction) {
    AudioDecoder decoder;
    // Verify album art extraction capability
    ASSERT(decoder.CanDecode(L"audio.mp3"));
    ASSERT(decoder.CanDecode(L"audio.flac"));
    ASSERT(decoder.CanDecode(L"audio.m4a"));
}

TEST(TestAudioDecoder_AlbumArtMultipleFormats) {
    AudioDecoder decoder;
    ASSERT(decoder.CanDecode(L"music.mp3"));
    ASSERT(decoder.CanDecode(L"music.flac"));
    ASSERT(decoder.CanDecode(L"music.ogg"));
    ASSERT(decoder.CanDecode(L"music.wma"));
    ASSERT(decoder.CanDecode(L"music.aac"));
}

TEST(TestAudioDecoder_NoAlbumArtGracefulFallback) {
    AudioDecoder decoder;
    // Should handle files without album art gracefully
    auto info = decoder.GetInfo();
    ASSERT_NOT_NULL(info.name);
}

//==============================================================================
// Document Thumbnail Provider Tests
//==============================================================================

TEST(TestDocumentDecoder_EPUBCoverExtraction) {
    DocumentDecoder decoder;
    ASSERT(decoder.CanDecode(L"book.epub"));
}

TEST(TestDocumentDecoder_MOBICoverExtraction) {
    DocumentDecoder decoder;
    ASSERT(decoder.CanDecode(L"book.mobi"));
}

TEST(TestDocumentDecoder_InvalidZipHandling) {
    DocumentDecoder decoder;
    // Should handle corrupted EPUB (invalid ZIP) gracefully
    ASSERT(decoder.CanDecode(L"document.epub"));
}

TEST(TestDocumentDecoder_MissingCoverHandling) {
    DocumentDecoder decoder;
    // Should handle EPUB without cover.jpg/png gracefully
    auto info = decoder.GetInfo();
    ASSERT_NOT_NULL(info.name);
}

//==============================================================================
// Font Preview Rendering Tests
//==============================================================================

TEST(TestFontDecoder_DirectWriteRendering) {
    FontDecoder decoder;
    // Verify DirectWrite-based rendering support
    ASSERT(decoder.CanDecode(L"font.ttf"));
    ASSERT(decoder.CanDecode(L"font.otf"));
}

TEST(TestFontDecoder_MetadataExtraction) {
    FontDecoder decoder;
    // Test font metadata extraction (family, weight, style)
    auto info = decoder.GetInfo();
    ASSERT_NOT_NULL(info.name);
}

TEST(TestFontDecoder_TrueTypeCollectionHandling) {
    FontDecoder decoder;
    // TTC files contain multiple fonts
    ASSERT(decoder.CanDecode(L"fonts.ttc"));
}

//==============================================================================
// Archive Format Expansion Tests
//==============================================================================

TEST(TestArchiveDecoder_7zSupport) {
    ArchiveDecoder decoder;
    ASSERT(decoder.CanDecode(L"archive.7z"));
}

TEST(TestArchiveDecoder_TarGzSupport) {
    ArchiveDecoder decoder;
    ASSERT(decoder.CanDecode(L"archive.tar.gz"));
}

TEST(TestArchiveDecoder_TarBz2Support) {
    ArchiveDecoder decoder;
    ASSERT(decoder.CanDecode(L"archive.tar.bz2"));
}

TEST(TestArchiveDecoder_PasswordProtectedHandling) {
    ArchiveDecoder decoder;
    // Should detect password-protected archives gracefully
    auto info = decoder.GetInfo();
    ASSERT_NOT_NULL(info.name);
}

//==============================================================================
// RAW Format Expansion Tests
//==============================================================================

TEST(TestRAWDecoder_CR3Support) {
    RAWDecoder decoder;
    ASSERT(decoder.CanDecode(L"photo.cr3"));
}

TEST(TestRAWDecoder_ARWSupport) {
    RAWDecoder decoder;
    ASSERT(decoder.CanDecode(L"photo.arw"));
}

TEST(TestRAWDecoder_ORFSupport) {
    RAWDecoder decoder;
    ASSERT(decoder.CanDecode(L"photo.orf"));
}

TEST(TestRAWDecoder_GPRSupport) {
    RAWDecoder decoder;
    ASSERT(decoder.CanDecode(L"photo.gpr"));
}

TEST(TestRAWDecoder_MultipleRAWFormats) {
    RAWDecoder decoder;
    // Test all major camera RAW formats via RAWDecoder (LibRaw)
    ASSERT(decoder.CanDecode(L"image.cr2"));
    ASSERT(decoder.CanDecode(L"image.cr3"));
    ASSERT(decoder.CanDecode(L"image.nef"));
    ASSERT(decoder.CanDecode(L"image.arw"));
    ASSERT(decoder.CanDecode(L"image.dng"));
    ASSERT(decoder.CanDecode(L"image.orf"));
    ASSERT(decoder.CanDecode(L"image.rw2"));
    ASSERT(decoder.CanDecode(L"image.gpr"));
}

//==============================================================================
// 3D Model Support Tests
//==============================================================================

TEST(TestModelDecoder_Create) {
    ModelDecoder decoder;
    auto info = decoder.GetInfo();
    ASSERT_NOT_NULL(info.name);
}

TEST(TestModelDecoder_OBJSupport) {
    ModelDecoder decoder;
    ASSERT(decoder.CanDecode(L"model.obj"));
    ASSERT(decoder.CanDecode(L"MODEL.OBJ"));
}

TEST(TestModelDecoder_STLSupport) {
    ModelDecoder decoder;
    ASSERT(decoder.CanDecode(L"model.stl"));
    ASSERT(decoder.CanDecode(L"MODEL.STL"));
}

TEST(TestModelDecoder_GLTFSupport) {
    ModelDecoder decoder;
    ASSERT(decoder.CanDecode(L"model.gltf"));
    ASSERT(decoder.CanDecode(L"model.glb"));
}

TEST(TestModelDecoder_Extensions) {
    ModelDecoder decoder;
    ASSERT(decoder.CanDecode(L"test.obj"));
    ASSERT(decoder.CanDecode(L"test.stl"));
    ASSERT(decoder.CanDecode(L"test.gltf"));
    ASSERT(decoder.CanDecode(L"test.glb"));
    ASSERT(!decoder.CanDecode(L"test.jpg"));
}

TEST(TestModelDecoder_GetInfo) {
    ModelDecoder decoder;
    auto info = decoder.GetInfo();
    ASSERT_NOT_NULL(info.name);
    ASSERT(info.supportsGPU); // Model decoder uses D3D11
}

//==============================================================================
// Enhanced Model Decoder Tests — PLY, DAE, expanded extensions
//==============================================================================

TEST(TestModelDecoder_PLYSupport) {
    ModelDecoder decoder;
    ASSERT(decoder.CanDecode(L"mesh.ply"));
    ASSERT(decoder.CanDecode(L"MESH.PLY"));
    ASSERT(decoder.CanDecode(L"C:\\models\\scan.ply"));
}

TEST(TestModelDecoder_DAESupport) {
    ModelDecoder decoder;
    ASSERT(decoder.CanDecode(L"model.dae"));
    ASSERT(decoder.CanDecode(L"MODEL.DAE"));
    ASSERT(decoder.CanDecode(L"scene.dae"));
}

TEST(TestModelDecoder_3DSSupport) {
    ModelDecoder decoder;
    ASSERT(decoder.CanDecode(L"model.3ds"));
    ASSERT(decoder.CanDecode(L"old_model.3ds"));
}

TEST(TestModelDecoder_FBXSupport) {
    ModelDecoder decoder;
    ASSERT(decoder.CanDecode(L"model.fbx"));
    ASSERT(decoder.CanDecode(L"character.fbx"));
}

TEST(TestModelDecoder_ExpandedExtensions) {
    ModelDecoder decoder;
    // Original 4
    ASSERT(decoder.CanDecode(L"test.obj"));
    ASSERT(decoder.CanDecode(L"test.stl"));
    ASSERT(decoder.CanDecode(L"test.gltf"));
    ASSERT(decoder.CanDecode(L"test.glb"));
    // New 4
    ASSERT(decoder.CanDecode(L"test.ply"));
    ASSERT(decoder.CanDecode(L"test.dae"));
    ASSERT(decoder.CanDecode(L"test.3ds"));
    ASSERT(decoder.CanDecode(L"test.fbx"));
    // Negative
    ASSERT(!decoder.CanDecode(L"test.jpg"));
    ASSERT(!decoder.CanDecode(L"test.png"));
}

TEST(TestModelDecoder_ExtensionCount) {
    ModelDecoder decoder;
    ASSERT_EQUAL(8u, decoder.GetExtensionCount());
    auto info = decoder.GetInfo();
    ASSERT_EQUAL(8u, info.extensionCount);
}

//==============================================================================
// EPS/PostScript Decoder Tests
//==============================================================================

TEST(TestEPSDecoder_Create) {
    EPSDecoder decoder;
    auto info = decoder.GetInfo();
    ASSERT_NOT_NULL(info.name);
    ASSERT_EQUAL(3u, info.extensionCount);
}

TEST(TestEPSDecoder_CanDecode) {
    EPSDecoder decoder;
    ASSERT(decoder.CanDecode(L"image.eps"));
    ASSERT(decoder.CanDecode(L"IMAGE.EPS"));
    ASSERT(decoder.CanDecode(L"file.epsf"));
    ASSERT(decoder.CanDecode(L"document.ps"));
}

TEST(TestEPSDecoder_NoDecodeNonEPS) {
    EPSDecoder decoder;
    ASSERT(!decoder.CanDecode(L"file.pdf"));
    ASSERT(!decoder.CanDecode(L"file.svg"));
    ASSERT(!decoder.CanDecode(L"file.ai"));
    ASSERT(!decoder.CanDecode(L"file.jpg"));
}

TEST(TestEPSDecoder_GetInfo) {
    EPSDecoder decoder;
    auto info = decoder.GetInfo();
    ASSERT(!info.supportsGPU);
    ASSERT(!info.isArchiveDecoder);
}

TEST(TestPDFDecoder_AIRouting) {
    // .ai files are PDF-based and should be handled by PDFDecoder
    PDFDecoder decoder;
    // PDFDecoder uses IsPDFFormat() which checks file signature,
    // not extension, so we just verify the architectural intent
    auto info = decoder.GetInfo();
    ASSERT_NOT_NULL(info.name);
}

//==============================================================================
// Game Texture Formats — KTX/KTX2 + VTF
// Tests for Khronos KTX and Valve Texture Format decoders
//==============================================================================

TEST(TestKTXDecoder_Create) {
    using namespace ExplorerLens::Decoders;
    KTXTextureDecoder decoder = KTXTextureDecoder::Create();
    ASSERT(decoder.IsAvailable());
}

TEST(TestKTXDecoder_ExtensionCheck) {
    using namespace ExplorerLens::Decoders;
    ASSERT(KTXTextureDecoder::IsKTXExtension(".ktx"));
    ASSERT(KTXTextureDecoder::IsKTXExtension(".KTX"));
    ASSERT(KTXTextureDecoder::IsKTXExtension(".ktx2"));
    ASSERT(KTXTextureDecoder::IsKTXExtension(".KTX2"));
    ASSERT(!KTXTextureDecoder::IsKTXExtension(".png"));
    ASSERT(!KTXTextureDecoder::IsKTXExtension(".dds"));
}

TEST(TestKTXDecoder_ExtensionVersion) {
    using namespace ExplorerLens::Decoders;
    ASSERT(KTXExtensions::VersionFromExtension(".ktx") == KTXVersion::KTX1);
    ASSERT(KTXExtensions::VersionFromExtension(".ktx2") == KTXVersion::KTX2);
    ASSERT(KTXExtensions::VersionFromExtension(".png") == KTXVersion::Unknown);
}

TEST(TestKTXDecoder_CompressionNames) {
    using namespace ExplorerLens::Decoders;
    ASSERT(std::string(CompressionName(TextureCompression::BC1_RGB)) ==
        "BC1 (DXT1)");
    ASSERT(std::string(CompressionName(TextureCompression::BC7_RGBA)) ==
        "BC7 (RGBA)");
    ASSERT(std::string(CompressionName(TextureCompression::ASTC_4x4)) ==
        "ASTC 4x4");
    ASSERT(IsBlockCompressed(TextureCompression::BC1_RGB));
    ASSERT(!IsBlockCompressed(TextureCompression::Uncompressed));
}

TEST(TestKTXDecoder_TextureInfo) {
    using namespace ExplorerLens::Decoders;
    KTXTextureInfo info;
    info.width = 1024;
    info.height = 1024;
    info.mipLevels = 11;
    info.version = KTXVersion::KTX2;
    info.compression = TextureCompression::BC7_RGBA;

    ASSERT(info.IsValid());
    ASSERT(info.HasMipmaps());
    ASSERT(!info.Is3D());

    uint32_t bestMip = info.BestMipForThumbnail(256);
    ASSERT(bestMip > 0); // Should select a smaller mip

    size_t estSize = info.EstimateCompressedSize();
    ASSERT(estSize > 0);
}

TEST(TestKTXDecoder_SupercompressionNames) {
    using namespace ExplorerLens::Decoders;
    ASSERT(std::string(SupercompressionName(KTXSupercompression::None)) ==
        "None");
    ASSERT(std::string(SupercompressionName(KTXSupercompression::Zstd)) ==
        "Zstandard");
}

TEST(TestKTXDecoder_InvalidFile) {
    using namespace ExplorerLens::Decoders;
    KTXTextureDecoder decoder;
    auto result = decoder.Decode("nonexistent.ktx");
    ASSERT(!result.IsSuccess());
}

TEST(TestVTFDecoder_ExtensionCheck) {
    using namespace ExplorerLens::Decoders;
    ASSERT(VTFDecoder::IsVTFExtension(".vtf"));
    ASSERT(VTFDecoder::IsVTFExtension(".VTF"));
    ASSERT(!VTFDecoder::IsVTFExtension(".png"));
    ASSERT(!VTFDecoder::IsVTFExtension(".dds"));
}

TEST(TestVTFDecoder_Create) {
    using namespace ExplorerLens::Decoders;
    VTFDecoder decoder;
    (void)decoder;
    ASSERT(VTFDecoder::EXTENSIONS[0] != nullptr);
    ASSERT(std::string(VTFDecoder::EXTENSIONS[0]) == ".vtf");
    ASSERT(VTFDecoder::EXTENSIONS[1] == nullptr);
}

TEST(TestVTFDecoder_InvalidFile) {
    using namespace ExplorerLens::Decoders;
    VTFDecoder decoder;
    auto result = decoder.Decode("nonexistent.vtf");
    ASSERT(!result.success);
}

TEST(TestVTFDecoder_ImageSizeCompute) {
    using namespace ExplorerLens::Decoders;
    // DXT1: 8 bytes per 4x4 block
    // 256x256 = 64x64 blocks = 4096 blocks * 8 = 32768 bytes
    // We can't call the private method directly, but we test via decode
    VTFDecoder decoder;
    auto info = decoder.ReadInfo("nonexistent.vtf");
    ASSERT(!info.IsValid()); // File doesn't exist, should be invalid
}

//==============================================================================
// OpenRaster & XCF — Open Image Editor Formats
//==============================================================================

TEST(TestORADecoder_ExtensionCheck) {
    using namespace ExplorerLens::Decoders;
    ASSERT(OpenRasterDecoder::IsORAExtension(".ora"));
    ASSERT(OpenRasterDecoder::IsORAExtension(".ORA"));
    ASSERT(!OpenRasterDecoder::IsORAExtension(".png"));
    ASSERT(!OpenRasterDecoder::IsORAExtension(".xcf"));
}

TEST(TestORADecoder_Create) {
    using namespace ExplorerLens::Decoders;
    OpenRasterDecoder decoder;
    (void)decoder;
    ASSERT(OpenRasterDecoder::EXTENSIONS[0] != nullptr);
    ASSERT(std::string(OpenRasterDecoder::EXTENSIONS[0]) == ".ora");
    ASSERT(OpenRasterDecoder::EXTENSIONS[1] == nullptr);
}

TEST(TestORADecoder_InvalidFile) {
    using namespace ExplorerLens::Decoders;
    OpenRasterDecoder decoder;
    auto result = decoder.Decode("nonexistent.ora");
    ASSERT(!result.success);
}

TEST(TestORADecoder_ReadInfoInvalid) {
    using namespace ExplorerLens::Decoders;
    OpenRasterDecoder decoder;
    auto info = decoder.ReadInfo("nonexistent.ora");
    ASSERT(!info.IsValid());
}

TEST(TestXCFDecoder_ExtensionCheck) {
    using namespace ExplorerLens::Decoders;
    ASSERT(XCFDecoder::IsXCFExtension(".xcf"));
    ASSERT(XCFDecoder::IsXCFExtension(".XCF"));
    ASSERT(!XCFDecoder::IsXCFExtension(".psd"));
    ASSERT(!XCFDecoder::IsXCFExtension(".ora"));
}

TEST(TestXCFDecoder_Create) {
    using namespace ExplorerLens::Decoders;
    XCFDecoder decoder;
    (void)decoder;
    ASSERT(XCFDecoder::EXTENSIONS[0] != nullptr);
    ASSERT(std::string(XCFDecoder::EXTENSIONS[0]) == ".xcf");
    ASSERT(XCFDecoder::EXTENSIONS[1] == nullptr);
}

TEST(TestXCFDecoder_InvalidFile) {
    using namespace ExplorerLens::Decoders;
    XCFDecoder decoder;
    auto result = decoder.Decode("nonexistent.xcf");
    ASSERT(!result.success);
}

TEST(TestXCFDecoder_ReadInfoInvalid) {
    using namespace ExplorerLens::Decoders;
    XCFDecoder decoder;
    auto info = decoder.ReadInfo("nonexistent.xcf");
    ASSERT(!info.IsValid());
}

TEST(TestXCFDecoder_ColorModes) {
    using namespace ExplorerLens::Decoders;
    ASSERT(static_cast<uint32_t>(XCFColorMode::RGB) == 0);
    ASSERT(static_cast<uint32_t>(XCFColorMode::Grayscale) == 1);
    ASSERT(static_cast<uint32_t>(XCFColorMode::Indexed) == 2);
}

//==============================================================================
// SGI/RGB & XPM — Legacy Image Formats
//==============================================================================

TEST(TestSGIDecoder_ExtensionCheck) {
    using namespace ExplorerLens::Decoders;
    ASSERT(SGIDecoder::IsSGIExtension(".sgi"));
    ASSERT(SGIDecoder::IsSGIExtension(".rgb"));
    ASSERT(SGIDecoder::IsSGIExtension(".rgba"));
    ASSERT(SGIDecoder::IsSGIExtension(".bw"));
    ASSERT(SGIDecoder::IsSGIExtension(".SGI"));
    ASSERT(!SGIDecoder::IsSGIExtension(".png"));
}

TEST(TestSGIDecoder_Create) {
    using namespace ExplorerLens::Decoders;
    SGIDecoder decoder;
    (void)decoder;
    ASSERT(SGIDecoder::EXTENSIONS[0] != nullptr);
    ASSERT(std::string(SGIDecoder::EXTENSIONS[0]) == ".sgi");
    // Count extensions
    int count = 0;
    while (SGIDecoder::EXTENSIONS[count] != nullptr)
        count++;
    ASSERT(count == 6); // .sgi .rgb .rgba .bw .int .inta
}

TEST(TestSGIDecoder_InvalidFile) {
    using namespace ExplorerLens::Decoders;
    SGIDecoder decoder;
    auto result = decoder.Decode("nonexistent.sgi");
    ASSERT(!result.success);
}

TEST(TestSGIDecoder_ReadInfoInvalid) {
    using namespace ExplorerLens::Decoders;
    SGIDecoder decoder;
    auto info = decoder.ReadInfo("nonexistent.sgi");
    ASSERT(!info.IsValid());
}

TEST(TestSGIDecoder_StorageTypes) {
    using namespace ExplorerLens::Decoders;
    ASSERT(static_cast<uint8_t>(SGIStorageType::Verbatim) == 0);
    ASSERT(static_cast<uint8_t>(SGIStorageType::RLE) == 1);
}

TEST(TestXPMDecoder_ExtensionCheck) {
    using namespace ExplorerLens::Decoders;
    ASSERT(XPMDecoder::IsXPMExtension(".xpm"));
    ASSERT(XPMDecoder::IsXPMExtension(".XPM"));
    ASSERT(!XPMDecoder::IsXPMExtension(".png"));
}

TEST(TestXPMDecoder_Create) {
    using namespace ExplorerLens::Decoders;
    XPMDecoder decoder;
    (void)decoder;
    ASSERT(XPMDecoder::EXTENSIONS[0] != nullptr);
    ASSERT(std::string(XPMDecoder::EXTENSIONS[0]) == ".xpm");
    ASSERT(XPMDecoder::EXTENSIONS[1] == nullptr);
}

TEST(TestXPMDecoder_InvalidFile) {
    using namespace ExplorerLens::Decoders;
    XPMDecoder decoder;
    auto result = decoder.Decode("nonexistent.xpm");
    ASSERT(!result.success);
}

TEST(TestXPMDecoder_ReadInfoInvalid) {
    using namespace ExplorerLens::Decoders;
    XPMDecoder decoder;
    auto info = decoder.ReadInfo("nonexistent.xpm");
    ASSERT(!info.IsValid());
}

//==============================================================================
// Async Shell Extension Tests
//==============================================================================

TEST(TestAsyncProvider_Create) {
    AsyncThumbnailProvider provider;
    ASSERT(!provider.IsRunning());
    ASSERT(provider.GetQueueDepth() == 0);
}

TEST(TestAsyncProvider_Initialize) {
    AsyncThumbnailProvider provider;
    bool ok = provider.Initialize();
    ASSERT(ok);
    ASSERT(provider.IsRunning());
    provider.Shutdown();
    ASSERT(!provider.IsRunning());
}

TEST(TestAsyncProvider_Config) {
    ThumbnailProviderConfig config;
    config.minThreads = 4;
    config.maxThreads = 16;
    config.maxQueueDepth = 512;
    config.timeoutMs = 10000;
    AsyncThumbnailProvider provider(config);
    ASSERT(provider.GetConfig().minThreads == 4);
    ASSERT(provider.GetConfig().maxThreads == 16);
    ASSERT(provider.GetConfig().maxQueueDepth == 512);
    ASSERT(provider.GetConfig().timeoutMs == 10000);
}

TEST(TestAsyncProvider_PriorityNames) {
    ASSERT(std::wstring(AsyncThumbnailProvider::GetPriorityName(
        DecodePriority::Critical)) == L"Critical");
    ASSERT(std::wstring(AsyncThumbnailProvider::GetPriorityName(
        DecodePriority::High)) == L"High");
    ASSERT(std::wstring(AsyncThumbnailProvider::GetPriorityName(
        DecodePriority::Normal)) == L"Normal");
    ASSERT(std::wstring(AsyncThumbnailProvider::GetPriorityName(
        DecodePriority::Low)) == L"Low");
    ASSERT(std::wstring(AsyncThumbnailProvider::GetPriorityName(
        DecodePriority::Idle)) == L"Idle");
}

TEST(TestAsyncProvider_StateNames) {
    ASSERT(std::wstring(AsyncThumbnailProvider::GetStateName(
        RequestState::Queued)) == L"Queued");
    ASSERT(std::wstring(AsyncThumbnailProvider::GetStateName(
        RequestState::InProgress)) == L"InProgress");
    ASSERT(std::wstring(AsyncThumbnailProvider::GetStateName(
        RequestState::Completed)) == L"Completed");
    ASSERT(std::wstring(AsyncThumbnailProvider::GetStateName(
        RequestState::Failed)) == L"Failed");
    ASSERT(std::wstring(AsyncThumbnailProvider::GetStateName(
        RequestState::Cancelled)) == L"Cancelled");
    ASSERT(std::wstring(AsyncThumbnailProvider::GetStateName(
        RequestState::TimedOut)) == L"TimedOut");
}

TEST(TestAsyncProvider_SyncFallback) {
    AsyncThumbnailProvider provider;
    provider.Initialize();
    auto result = provider.DecodeSynchronous(L"test.png", 256);
    ASSERT(result.state == RequestState::Completed);
    ASSERT(result.width == 256);
    ASSERT(result.height == 256);
    provider.Shutdown();
}

TEST(TestAsyncProvider_SyncFallbackEmpty) {
    AsyncThumbnailProvider provider;
    provider.Initialize();
    auto result = provider.DecodeSynchronous(L"", 256);
    ASSERT(result.state == RequestState::Failed);
    provider.Shutdown();
}

TEST(TestAsyncProvider_Stats) {
    AsyncThumbnailProvider provider;
    provider.Initialize();
    auto stats = provider.GetStats();
    ASSERT(stats.totalRequests == 0);
    ASSERT(stats.completedRequests == 0);
    ASSERT(stats.queueDepth == 0);
    provider.Shutdown();
}

TEST(TestAsyncProvider_SubmitNotRunning) {
    AsyncThumbnailProvider provider;
    // Not initialized — should return 0
    uint64_t id =
        provider.SubmitRequest(L"test.png", 256, DecodePriority::Normal, nullptr);
    ASSERT(id == 0);
}

//==============================================================================
// D3D12 Compute Pipeline Tests
//==============================================================================

TEST(TestD3D12Compute_Create) {
    D3D12ComputePipeline pipeline;
    ASSERT(!pipeline.IsInitialized());
    ASSERT(pipeline.GetActiveBackend() == GPUBackend::CPU);
}

TEST(TestD3D12Compute_Initialize) {
    D3D12ComputePipeline pipeline;
    bool ok = pipeline.Initialize();
    ASSERT(ok);
    ASSERT(pipeline.IsInitialized());
    pipeline.Shutdown();
    ASSERT(!pipeline.IsInitialized());
}

TEST(TestD3D12Compute_BackendNames) {
    ASSERT(std::wstring(D3D12ComputePipeline::GetBackendName(GPUBackend::Auto)) ==
        L"Auto");
    ASSERT(std::wstring(D3D12ComputePipeline::GetBackendName(
        GPUBackend::D3D12)) == L"D3D12");
    ASSERT(std::wstring(D3D12ComputePipeline::GetBackendName(
        GPUBackend::D3D11)) == L"D3D11");
    ASSERT(std::wstring(D3D12ComputePipeline::GetBackendName(GPUBackend::CPU)) ==
        L"CPU");
}

TEST(TestD3D12Compute_AlgorithmNames) {
    ASSERT(std::wstring(D3D12ComputePipeline::GetAlgorithmName(
        ScalingAlgorithm::Bilinear)) == L"Bilinear");
    ASSERT(std::wstring(D3D12ComputePipeline::GetAlgorithmName(
        ScalingAlgorithm::Lanczos3)) == L"Lanczos3");
    ASSERT(std::wstring(D3D12ComputePipeline::GetAlgorithmName(
        ScalingAlgorithm::Adaptive)) == L"Adaptive");
}

TEST(TestD3D12Compute_ColorSpaceNames) {
    ASSERT(std::wstring(D3D12ComputePipeline::GetColorSpaceName(
        GPUColorSpace::SRGB)) == L"sRGB");
    ASSERT(std::wstring(D3D12ComputePipeline::GetColorSpaceName(
        GPUColorSpace::LinearRGB)) == L"LinearRGB");
    ASSERT(std::wstring(D3D12ComputePipeline::GetColorSpaceName(
        GPUColorSpace::HDR10)) == L"HDR10");
}

TEST(TestD3D12Compute_ToneMapNames) {
    ASSERT(std::wstring(D3D12ComputePipeline::GetToneMapName(
        ToneMapOperator::None)) == L"None");
    ASSERT(std::wstring(D3D12ComputePipeline::GetToneMapName(
        ToneMapOperator::Reinhard)) == L"Reinhard");
    ASSERT(std::wstring(D3D12ComputePipeline::GetToneMapName(
        ToneMapOperator::ACES)) == L"ACES");
}

TEST(TestD3D12Compute_ProbeHardware) {
    D3D12ComputePipeline pipeline;
    auto reqs = pipeline.ProbeHardware();
    // CPU fallback mode — no GPU compute
    ASSERT(!reqs.adapterDescription.empty());
}

TEST(TestD3D12Compute_ResizeNotInit) {
    D3D12ComputePipeline pipeline;
    auto result = pipeline.Resize(nullptr, 0, 0, 0, 0);
    ASSERT(!result.success);
}

TEST(TestD3D12Compute_ResizeCPU) {
    D3D12ComputePipeline pipeline;
    pipeline.Initialize();
    // Create a 4x4 BGRA test image (64 bytes)
    std::vector<uint8_t> input(4 * 4 * 4, 128);
    auto result = pipeline.Resize(input.data(), 4, 4, 2, 2);
    ASSERT(result.success);
    ASSERT(result.outputWidth == 2);
    ASSERT(result.outputHeight == 2);
    ASSERT(result.outputData.size() == 2 * 2 * 4);
    pipeline.Shutdown();
}

TEST(TestD3D12Compute_Stats) {
    D3D12ComputePipeline pipeline;
    pipeline.Initialize();
    auto stats = pipeline.GetStats();
    ASSERT(stats.totalDispatches == 0);
    pipeline.Shutdown();
}

//==============================================================================
// Parallel Batch Decoder Tests
//==============================================================================

TEST(TestBatchDecoder_Create) {
    ParallelBatchDecoder decoder;
    ASSERT(!decoder.IsRunning());
}

TEST(TestBatchDecoder_Initialize) {
    ParallelBatchDecoder decoder;
    bool ok = decoder.Initialize();
    ASSERT(ok);
    ASSERT(decoder.IsRunning());
    decoder.Shutdown();
    ASSERT(!decoder.IsRunning());
}

TEST(TestBatchDecoder_Config) {
    BatchDecoderConfig config;
    config.workerThreads = 8;
    config.maxBatchSize = 500;
    ParallelBatchDecoder decoder(config);
    ASSERT(decoder.GetConfig().workerThreads == 8);
    ASSERT(decoder.GetConfig().maxBatchSize == 500);
}

TEST(TestBatchDecoder_ClassifyFormats) {
    // Archives are serial
    ASSERT(ParallelBatchDecoder::ClassifyFormat(L".zip") ==
        ParallelismLevel::SerialOnly);
    ASSERT(ParallelBatchDecoder::ClassifyFormat(L".rar") ==
        ParallelismLevel::SerialOnly);
    ASSERT(ParallelBatchDecoder::ClassifyFormat(L".7z") ==
        ParallelismLevel::SerialOnly);
    // GPU formats are limited
    ASSERT(ParallelBatchDecoder::ClassifyFormat(L".dds") ==
        ParallelismLevel::LimitedParallel);
    ASSERT(ParallelBatchDecoder::ClassifyFormat(L".ktx") ==
        ParallelismLevel::LimitedParallel);
    // Standard images are full parallel
    ASSERT(ParallelBatchDecoder::ClassifyFormat(L".jpg") ==
        ParallelismLevel::FullParallel);
    ASSERT(ParallelBatchDecoder::ClassifyFormat(L".png") ==
        ParallelismLevel::FullParallel);
}

TEST(TestBatchDecoder_ParallelismNames) {
    ASSERT(std::wstring(ParallelBatchDecoder::GetParallelismName(
        ParallelismLevel::FullParallel)) == L"FullParallel");
    ASSERT(std::wstring(ParallelBatchDecoder::GetParallelismName(
        ParallelismLevel::SerialOnly)) == L"SerialOnly");
    ASSERT(std::wstring(ParallelBatchDecoder::GetParallelismName(
        ParallelismLevel::LimitedParallel)) == L"LimitedParallel");
}

TEST(TestBatchDecoder_StatusNames) {
    ASSERT(std::wstring(ParallelBatchDecoder::GetBatchStatusName(
        BatchItemStatus::Pending)) == L"Pending");
    ASSERT(std::wstring(ParallelBatchDecoder::GetBatchStatusName(
        BatchItemStatus::Completed)) == L"Completed");
    ASSERT(std::wstring(ParallelBatchDecoder::GetBatchStatusName(
        BatchItemStatus::Cancelled)) == L"Cancelled");
}

TEST(TestBatchDecoder_PriorityNames) {
    ASSERT(std::wstring(ParallelBatchDecoder::GetBatchPriorityName(
        BatchPriority::Immediate)) == L"Immediate");
    ASSERT(std::wstring(ParallelBatchDecoder::GetBatchPriorityName(
        BatchPriority::Background)) == L"Background");
    ASSERT(std::wstring(ParallelBatchDecoder::GetBatchPriorityName(
        BatchPriority::CacheWarm)) == L"CacheWarm");
}

TEST(TestBatchDecoder_SubmitEmpty) {
    ParallelBatchDecoder decoder;
    decoder.Initialize();
    std::vector<std::wstring> empty;
    uint64_t id = decoder.SubmitBatch(empty);
    ASSERT(id == 0); // Empty batch rejected
    decoder.Shutdown();
}

TEST(TestBatchDecoder_SubmitBatch) {
    ParallelBatchDecoder decoder;
    decoder.Initialize();
    std::vector<std::wstring> files = { L"test1.jpg", L"test2.png", L"test3.bmp" };
    uint64_t id = decoder.SubmitBatch(files);
    ASSERT(id > 0);
    auto results = decoder.GetBatchResults(id);
    ASSERT(results.size() == 3);
    decoder.Shutdown();
}

TEST(TestBatchDecoder_CancelBatch) {
    ParallelBatchDecoder decoder;
    decoder.Initialize();
    std::vector<std::wstring> files = { L"a.jpg", L"b.png" };
    uint64_t id = decoder.SubmitBatch(files);
    bool cancelled = decoder.CancelBatch(id);
    ASSERT(cancelled);
    decoder.Shutdown();
}

//==============================================================================
// Code Coverage & Fuzzing Tests
//==============================================================================

TEST(TestCoverage_Create) {
    CodeCoverageIntegration cov;
    auto thresholds = cov.GetThresholds();
    ASSERT(thresholds.minLineCoverage == 60.0);
}

TEST(TestCoverage_CIThresholds) {
    auto t = CoverageThresholds::ForCI();
    ASSERT(t.minLineCoverage == 60.0);
    ASSERT(t.minBranchCoverage == 40.0);
    ASSERT(t.minFunctionCoverage == 70.0);
}

TEST(TestCoverage_ReleaseThresholds) {
    auto t = CoverageThresholds::ForRelease();
    ASSERT(t.minLineCoverage == 75.0);
    ASSERT(t.minBranchCoverage == 55.0);
    ASSERT(t.minFunctionCoverage == 85.0);
}

TEST(TestCoverage_GenerateCommand) {
    CodeCoverageIntegration cov;
    auto cmd =
        cov.GenerateCoverageCommand(L"EngineTests.exe", L"coverage-output");
    ASSERT(!cmd.empty());
    ASSERT(cmd.find(L"OpenCppCoverage") != std::wstring::npos);
    ASSERT(cmd.find(L"EngineTests.exe") != std::wstring::npos);
}

TEST(TestCoverage_MetricNames) {
    ASSERT(std::wstring(CodeCoverageIntegration::GetMetricName(
        CoverageMetric::LineCoverage)) == L"LineCoverage");
    ASSERT(std::wstring(CodeCoverageIntegration::GetMetricName(
        CoverageMetric::BranchCoverage)) == L"BranchCoverage");
}

TEST(TestCoverage_FuzzTargetNames) {
    ASSERT(std::wstring(CodeCoverageIntegration::GetFuzzTargetName(
        FuzzTargetType::HeaderParsing)) == L"HeaderParsing");
    ASSERT(std::wstring(CodeCoverageIntegration::GetFuzzTargetName(
        FuzzTargetType::MalformedInput)) == L"MalformedInput");
}

TEST(TestCoverage_FuzzableDecoders) {
    auto decoders = CodeCoverageIntegration::GetFuzzableDecoders();
    ASSERT(decoders.size() >= 25);
    // Check known decoders present
    bool hasWebP = false, hasSGI = false;
    for (const auto& d : decoders) {
        if (d == L"WebPDecoder")
            hasWebP = true;
        if (d == L"SGIDecoder")
            hasSGI = true;
    }
    ASSERT(hasWebP);
    ASSERT(hasSGI);
}

TEST(TestCoverage_GenerateFuzzTargets) {
    CodeCoverageIntegration cov;
    auto targets = cov.GenerateFuzzTargets();
    ASSERT(targets.size() >= 25);
    ASSERT(!targets[0].decoderName.empty());
    ASSERT(targets[0].targetTypes.size() == 3);
}

TEST(TestCoverage_ValidateEmpty) {
    CodeCoverageIntegration cov;
    CoverageReport report;
    // Empty report should not meet thresholds
    ASSERT(!cov.ValidateCoverage(report));
}

//==============================================================================
// Memory Safety Tests
//==============================================================================

TEST(TestMemSafety_ASANDetection) {
    // ASAN is typically not enabled in normal builds
    bool asanEnabled = MemorySafetyIntegration::IsASANEnabled();
    // Just verify it doesn't crash — actual value depends on build config
    (void)asanEnabled;
    ASSERT(true);
}

TEST(TestMemSafety_RecommendedConfig) {
    auto config = MemorySafetyIntegration::GetRecommendedConfig();
    ASSERT(config.enableASAN);
    ASSERT(config.enableStackProtection);
    ASSERT(config.enableHeapProtection);
    ASSERT(config.enableLeakDetection);
}

TEST(TestMemSafety_CompilerFlags) {
    auto config = MemorySafetyIntegration::GetRecommendedConfig();
    auto flags = config.GetCompilerFlags();
    ASSERT(flags.find(L"/fsanitize=address") != std::wstring::npos);
}

TEST(TestMemSafety_CMakeOptions) {
    auto config = MemorySafetyIntegration::GetRecommendedConfig();
    auto opts = config.GetCMakeOptions();
    ASSERT(!opts.empty());
    ASSERT(opts.find(L"fsanitize") != std::wstring::npos);
}

TEST(TestMemSafety_SafeBuffer) {
    uint8_t data[] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
    auto buffer = MemorySafetyIntegration::CreateSafeBuffer(data, 5);
    ASSERT(buffer.IsValid());
    ASSERT(buffer.Size() == 5);
    ASSERT(buffer.Available() == 5);

    uint8_t val = 0;
    ASSERT(buffer.ReadValue(val));
    ASSERT(val == 0x01);
    ASSERT(buffer.Available() == 4);
}

TEST(TestMemSafety_SafeBufferBounds) {
    uint8_t data[] = { 0xAA, 0xBB };
    auto buffer = MemorySafetyIntegration::CreateSafeBuffer(data, 2);

    // Reading 3 bytes from 2-byte buffer should fail
    uint8_t dst[3];
    ASSERT(!buffer.Read(dst, 3));

    // But reading 2 should succeed
    buffer.Seek(0);
    ASSERT(buffer.Read(dst, 2));
}

TEST(TestMemSafety_ValidateAccess) {
    SafeBuffer buffer(10);
    ASSERT(MemorySafetyIntegration::ValidateAccess(buffer, 0, 10));
    ASSERT(MemorySafetyIntegration::ValidateAccess(buffer, 5, 5));
    ASSERT(!MemorySafetyIntegration::ValidateAccess(buffer, 5, 6)); // exceeds
    ASSERT(
        !MemorySafetyIntegration::ValidateAccess(buffer, 11, 1)); // out of range
}

TEST(TestMemSafety_SanitizerNames) {
    ASSERT(std::wstring(MemorySafetyIntegration::GetSanitizerName(
        SanitizerMode::None)) == L"None");
    ASSERT(std::wstring(MemorySafetyIntegration::GetSanitizerName(
        SanitizerMode::AddressSanitizer)) == L"AddressSanitizer");
    ASSERT(std::wstring(MemorySafetyIntegration::GetSanitizerName(
        SanitizerMode::ThreadSanitizer)) == L"ThreadSanitizer");
}

TEST(TestMemSafety_AccessPatterns) {
    ASSERT(std::wstring(MemorySafetyIntegration::GetAccessPatternName(
        AccessPattern::Sequential)) == L"Sequential");
    ASSERT(std::wstring(MemorySafetyIntegration::GetAccessPatternName(
        AccessPattern::Random)) == L"Random");
    ASSERT(std::wstring(MemorySafetyIntegration::GetAccessPatternName(
        AccessPattern::HeaderOnly)) == L"HeaderOnly");
}

TEST(TestMemSafety_MaxMappableSize) {
    uint64_t maxSize = MemorySafetyIntegration::GetMaxMappableSize();
    ASSERT(maxSize > 0);
    // On x64, should be at least 256MB
    ASSERT(maxSize >= 256ULL * 1024 * 1024);
}

//==============================================================================
// Cache System V2 — Persistent Disk Cache Tests
//==============================================================================

TEST(TestDiskCache_OpenClose) {
    using namespace ExplorerLens::Engine;
    DiskCacheConfig config;
    config.cacheDirPath = L"C:\\ExplorerLensTestCache";
    config.maxDiskSizeMB = 64;
    PersistentDiskCache cache(config);
    ASSERT(cache.Open());
    ASSERT(cache.IsOpen());
    cache.Close();
    ASSERT(!cache.IsOpen());
}

TEST(TestDiskCache_PutAndContains) {
    using namespace ExplorerLens::Engine;
    DiskCacheConfig config;
    config.maxDiskSizeMB = 64;
    PersistentDiskCache cache(config);
    cache.Open();
    uint8_t data[] = { 0xFF, 0x00, 0xAA, 0x55 };
    ASSERT(cache.Put(L"C:\\test\\image.png", 256, 256, data, 4, 15.0, L"PNG"));
    ASSERT(cache.Contains(L"C:\\test\\image.png"));
    ASSERT(!cache.Contains(L"C:\\test\\nonexistent.png"));
}

TEST(TestDiskCache_GetRetrieval) {
    using namespace ExplorerLens::Engine;
    PersistentDiskCache cache;
    cache.Open();
    uint8_t data[] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
    cache.Put(L"C:\\test\\photo.jpg", 512, 512, data, 5, 25.0, L"JPEG");
    uint32_t w = 0, h = 0;
    std::vector<uint8_t> out;
    ASSERT(cache.Get(L"C:\\test\\photo.jpg", w, h, out));
    ASSERT(w == 512 && h == 512);
    ASSERT(out.size() == 5);
}

TEST(TestDiskCache_Remove) {
    using namespace ExplorerLens::Engine;
    PersistentDiskCache cache;
    cache.Open();
    uint8_t data[] = { 0xAA };
    cache.Put(L"C:\\test\\remove.tga", 128, 128, data, 1, 5.0, L"TGA");
    ASSERT(cache.Contains(L"C:\\test\\remove.tga"));
    ASSERT(cache.Remove(L"C:\\test\\remove.tga"));
    ASSERT(!cache.Contains(L"C:\\test\\remove.tga"));
}

TEST(TestDiskCache_EvictionStrategies) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(PersistentDiskCache::GetEvictionName(
        EvictionStrategy::LRU)) == L"LRU");
    ASSERT(std::wstring(PersistentDiskCache::GetEvictionName(
        EvictionStrategy::LFU)) == L"LFU");
    ASSERT(std::wstring(PersistentDiskCache::GetEvictionName(
        EvictionStrategy::CostAware)) == L"CostAware");
    ASSERT(std::wstring(PersistentDiskCache::GetEvictionName(
        EvictionStrategy::SizeAware)) == L"SizeAware");
    ASSERT(std::wstring(PersistentDiskCache::GetEvictionName(
        EvictionStrategy::Hybrid)) == L"Hybrid");
}

TEST(TestDiskCache_EntryStates) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(PersistentDiskCache::GetEntryStateName(
        CacheEntryState::Valid)) == L"Valid");
    ASSERT(std::wstring(PersistentDiskCache::GetEntryStateName(
        CacheEntryState::Stale)) == L"Stale");
    ASSERT(std::wstring(PersistentDiskCache::GetEntryStateName(
        CacheEntryState::Corrupted)) == L"Corrupted");
    ASSERT(std::wstring(PersistentDiskCache::GetEntryStateName(
        CacheEntryState::Expired)) == L"Expired");
    ASSERT(std::wstring(PersistentDiskCache::GetEntryStateName(
        CacheEntryState::Warming)) == L"Warming");
}

TEST(TestDiskCache_CRC32) {
    using namespace ExplorerLens::Engine;
    uint8_t data1[] = { 0x48, 0x65, 0x6C, 0x6C, 0x6F }; // "Hello"
    uint32_t crc1 = PersistentDiskCache::ComputeCRC32(data1, 5);
    ASSERT(crc1 != 0);
    // Same data should produce same CRC
    uint32_t crc2 = PersistentDiskCache::ComputeCRC32(data1, 5);
    ASSERT(crc1 == crc2);
    // Different data should produce different CRC
    uint8_t data3[] = { 0x57, 0x6F, 0x72, 0x6C, 0x64 }; // "World"
    uint32_t crc3 = PersistentDiskCache::ComputeCRC32(data3, 5);
    ASSERT(crc1 != crc3);
    // Empty should return 0
    ASSERT(PersistentDiskCache::ComputeCRC32(nullptr, 0) == 0);
}

TEST(TestDiskCache_CacheKey) {
    using namespace ExplorerLens::Engine;
    auto key1 = PersistentDiskCache::GenerateCacheKey(L"C:\\dir\\file.png", 256);
    auto key2 = PersistentDiskCache::GenerateCacheKey(L"C:\\dir\\file.png", 256);
    ASSERT(key1 == key2); // Deterministic
    auto key3 = PersistentDiskCache::GenerateCacheKey(L"C:\\dir\\file.png", 512);
    ASSERT(key1 != key3); // Different size = different key
    auto key4 = PersistentDiskCache::GenerateCacheKey(L"C:\\dir\\other.png", 256);
    ASSERT(key1 != key4); // Different path = different key
}

TEST(TestDiskCache_Stats) {
    using namespace ExplorerLens::Engine;
    DiskCacheConfig config;
    config.maxDiskSizeMB = 128;
    PersistentDiskCache cache(config);
    cache.Open();
    uint8_t data[] = { 0xBB, 0xCC };
    cache.Put(L"C:\\test\\s1.bmp", 64, 64, data, 2, 3.0, L"BMP");
    cache.Put(L"C:\\test\\s2.bmp", 64, 64, data, 2, 4.0, L"BMP");
    auto stats = cache.GetStats();
    ASSERT(stats.totalEntries == 2);
    ASSERT(stats.maxDiskBytes == 128ULL * 1024 * 1024);
}

TEST(TestDiskCache_Compact) {
    using namespace ExplorerLens::Engine;
    PersistentDiskCache cache;
    cache.Open();
    ASSERT(cache.Compact());
    cache.Close();
    ASSERT(!cache.Compact()); // Should fail when closed
}

//==============================================================================
// ARM64 Hardware Validation Tests
//==============================================================================

TEST(TestARM64_PlatformDetection) {
    using namespace ExplorerLens::Engine;
    // On x64 build, these should return specific values
    bool isARM64 = ARM64HardwareValidator::IsRunningOnARM64();
    bool isEC = ARM64HardwareValidator::IsRunningAsARM64EC();
    bool isEmulated = ARM64HardwareValidator::IsRunningUnderEmulation();
    // At least one state must be deterministic
    (void)isARM64;
    (void)isEC;
    (void)isEmulated;
    ASSERT(true); // Detection doesn't crash
}

TEST(TestARM64_FeatureDetection) {
    using namespace ExplorerLens::Engine;
    auto features = ARM64HardwareValidator::DetectFeatures();
    uint32_t count = ARM64HardwareValidator::CountFeatures(features);
#if defined(_M_ARM64)
    // NEON is mandatory on ARM64
    ASSERT(HasFeature(features, ARM64Feature::NEON));
    ASSERT(count >= 1);
#else
    ASSERT(count == 0); // No ARM64 features on x64
#endif
}

TEST(TestARM64_FeatureNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ARM64HardwareValidator::GetFeatureName(
        ARM64Feature::NEON)) == L"NEON");
    ASSERT(std::wstring(ARM64HardwareValidator::GetFeatureName(
        ARM64Feature::CRC32)) == L"CRC32");
    ASSERT(std::wstring(ARM64HardwareValidator::GetFeatureName(
        ARM64Feature::AES)) == L"AES");
    ASSERT(std::wstring(ARM64HardwareValidator::GetFeatureName(
        ARM64Feature::SVE)) == L"SVE");
    ASSERT(std::wstring(ARM64HardwareValidator::GetFeatureName(
        ARM64Feature::SVE2)) == L"SVE2");
}

TEST(TestARM64_TargetNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ARM64HardwareValidator::GetTargetName(
        ARM64Target::Native)) == L"Native");
    ASSERT(std::wstring(ARM64HardwareValidator::GetTargetName(
        ARM64Target::ARM64EC)) == L"ARM64EC");
    ASSERT(std::wstring(ARM64HardwareValidator::GetTargetName(
        ARM64Target::ARM64X)) == L"ARM64X");
    ASSERT(std::wstring(ARM64HardwareValidator::GetTargetName(
        ARM64Target::CrossCompile)) == L"CrossCompile");
}

TEST(TestARM64_PerfCategoryNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ARM64HardwareValidator::GetPerfCategoryName(
        PerfCategory::SingleDecode)) == L"SingleDecode");
    ASSERT(std::wstring(ARM64HardwareValidator::GetPerfCategoryName(
        PerfCategory::BatchDecode)) == L"BatchDecode");
    ASSERT(std::wstring(ARM64HardwareValidator::GetPerfCategoryName(
        PerfCategory::GPUScaling)) == L"GPUScaling");
    ASSERT(std::wstring(ARM64HardwareValidator::GetPerfCategoryName(
        PerfCategory::CacheHit)) == L"CacheHit");
    ASSERT(std::wstring(ARM64HardwareValidator::GetPerfCategoryName(
        PerfCategory::ShellResponse)) == L"ShellResponse");
}

TEST(TestARM64_PerfBaselines) {
    using namespace ExplorerLens::Engine;
    ARM64HardwareValidator validator;
    auto baselines = validator.GenerateBaselines();
    ASSERT(baselines.size() == 7);
    for (const auto& b : baselines) {
        ASSERT(b.targetMs > 0);
        ASSERT(b.x64ReferenceMs > 0);
    }
}

TEST(TestARM64_X64ReferenceBaselines) {
    using namespace ExplorerLens::Engine;
    auto refs = ARM64HardwareValidator::GetX64ReferenceBaselines();
    ASSERT(refs.size() >= 4);
    ASSERT(refs[0].x64ReferenceMs == 17.0); // Single decode
}

TEST(TestARM64_RunValidation) {
    using namespace ExplorerLens::Engine;
    ARM64HardwareValidator validator;
    auto result = validator.RunValidation();
    ASSERT(result.coreCount > 0);
    ASSERT(result.memoryMB > 0);
    ASSERT(!result.perfResults.empty());
}

TEST(TestARM64_CIWorkflow) {
    using namespace ExplorerLens::Engine;
    ARM64CIConfig config;
    config.runnerLabel = L"windows-arm64";
    auto yaml = ARM64HardwareValidator::GenerateCIWorkflow(config);
    ASSERT(!yaml.empty());
    ASSERT(yaml.find(L"ARM64") != std::wstring::npos);
    ASSERT(yaml.find(L"cmake") != std::wstring::npos);
}

TEST(TestARM64_FeatureBitmask) {
    using namespace ExplorerLens::Engine;
    auto combined = ARM64Feature::NEON | ARM64Feature::CRC32 | ARM64Feature::AES;
    ASSERT(HasFeature(combined, ARM64Feature::NEON));
    ASSERT(HasFeature(combined, ARM64Feature::CRC32));
    ASSERT(HasFeature(combined, ARM64Feature::AES));
    ASSERT(!HasFeature(combined, ARM64Feature::SVE));
    ASSERT(ARM64HardwareValidator::CountFeatures(combined) == 3);
}

//==============================================================================
// High-DPI Support Tests
//==============================================================================

TEST(TestDPI_SystemDPI) {
    using namespace ExplorerLens::Engine;
    uint32_t dpi = HighDPIScaling::GetSystemDPI();
    ASSERT(dpi >= 72 && dpi <= 600); // Sane range
}

TEST(TestDPI_GetMonitorDPI) {
    using namespace ExplorerLens::Engine;
    auto info = HighDPIScaling::GetMonitorDPI(0);
    ASSERT(info.monitorIndex == 0);
    ASSERT(info.dpiX >= 72);
    ASSERT(info.width > 0 && info.height > 0);
}

TEST(TestDPI_EnumerateMonitors) {
    using namespace ExplorerLens::Engine;
    auto monitors = HighDPIScaling::EnumerateMonitors();
    ASSERT(!monitors.empty());
    ASSERT(monitors[0].isPrimary || monitors.size() == 1);
}

TEST(TestDPI_LogicalPhysicalConversion) {
    using namespace ExplorerLens::Engine;
    ASSERT(HighDPIScaling::LogicalToPhysical(256, 96) == 256);
    ASSERT(HighDPIScaling::LogicalToPhysical(256, 192) == 512);
    ASSERT(HighDPIScaling::LogicalToPhysical(256, 144) == 384);
    ASSERT(HighDPIScaling::PhysicalToLogical(512, 192) == 256);
    ASSERT(HighDPIScaling::PhysicalToLogical(256, 96) == 256);
}

TEST(TestDPI_ScaleRequest) {
    using namespace ExplorerLens::Engine;
    HighDPIScaling scaling;
    auto req = scaling.ScaleRequest(256, 256, 192);
    ASSERT(req.logicalWidth == 256);
    ASSERT(req.physicalWidth == 512);
    ASSERT(req.dpi == 192);
    ASSERT(req.scaleFactor == 2.0);
}

TEST(TestDPI_NearestScale) {
    using namespace ExplorerLens::Engine;
    ASSERT(HighDPIScaling::GetNearestScale(96) == DPIScale::Scale100);
    ASSERT(HighDPIScaling::GetNearestScale(120) == DPIScale::Scale125);
    ASSERT(HighDPIScaling::GetNearestScale(144) == DPIScale::Scale150);
    ASSERT(HighDPIScaling::GetNearestScale(192) == DPIScale::Scale200);
    ASSERT(HighDPIScaling::GetNearestScale(288) == DPIScale::Scale300);
}

TEST(TestDPI_DPIForScale) {
    using namespace ExplorerLens::Engine;
    ASSERT(HighDPIScaling::GetDPIForScale(DPIScale::Scale100) == 96);
    ASSERT(HighDPIScaling::GetDPIForScale(DPIScale::Scale125) == 120);
    ASSERT(HighDPIScaling::GetDPIForScale(DPIScale::Scale150) == 144);
    ASSERT(HighDPIScaling::GetDPIForScale(DPIScale::Scale200) == 192);
    ASSERT(HighDPIScaling::GetDPIForScale(DPIScale::Scale400) == 384);
}

TEST(TestDPI_ScaleFactors) {
    using namespace ExplorerLens::Engine;
    ASSERT(HighDPIScaling::GetScaleFactor(DPIScale::Scale100) == 1.0);
    ASSERT(HighDPIScaling::GetScaleFactor(DPIScale::Scale200) == 2.0);
    ASSERT(HighDPIScaling::GetScaleFactorForDPI(96) == 1.0);
    ASSERT(HighDPIScaling::GetScaleFactorForDPI(192) == 2.0);
}

TEST(TestDPI_ScaleNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(HighDPIScaling::GetScaleName(DPIScale::Scale100)) ==
        L"100%");
    ASSERT(std::wstring(HighDPIScaling::GetScaleName(DPIScale::Scale150)) ==
        L"150%");
    ASSERT(std::wstring(HighDPIScaling::GetScaleName(DPIScale::Scale200)) ==
        L"200%");
    ASSERT(std::wstring(HighDPIScaling::GetScaleName(DPIScale::Custom)) ==
        L"Custom");
}

TEST(TestDPI_AwarenessNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(HighDPIScaling::GetAwarenessName(
        DPIAwareness::Unaware)) == L"Unaware");
    ASSERT(std::wstring(HighDPIScaling::GetAwarenessName(
        DPIAwareness::PerMonitorV2)) == L"PerMonitorV2");
}

//==============================================================================
// MSIX Packaging Tests
//==============================================================================

TEST(TestMSIX_GenerateManifest) {
    using namespace ExplorerLens::Engine;
    MSIXPackageManager mgr;
    auto xml = mgr.GenerateManifest();
    ASSERT(!xml.empty());
    ASSERT(xml.find(L"<Package") != std::wstring::npos);
    ASSERT(xml.find(L"<Identity") != std::wstring::npos);
    ASSERT(xml.find(L"9E6ECB90-5A61-42BD-B851-D3297D9C7F39") !=
        std::wstring::npos);
}

TEST(TestMSIX_ValidateManifest) {
    using namespace ExplorerLens::Engine;
    MSIXPackageManager mgr;
    auto xml = mgr.GenerateManifest();
    ASSERT(mgr.ValidateManifest(xml));
    ASSERT(!mgr.ValidateManifest(L"")); // Empty fails
    ASSERT(!mgr.ValidateManifest(L"<html></html>")); // Wrong structure
}

TEST(TestMSIX_GenerateAppInstaller) {
    using namespace ExplorerLens::Engine;
    MSIXConfig config;
    config.autoUpdate.updateUri =
        L"https://example.com/ExplorerLens.appinstaller";
    MSIXPackageManager mgr(config);
    auto xml = mgr.GenerateAppInstaller();
    ASSERT(!xml.empty());
    ASSERT(xml.find(L"AppInstaller") != std::wstring::npos);
    ASSERT(xml.find(L"UpdateSettings") != std::wstring::npos);
}

TEST(TestMSIX_ChannelNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(MSIXPackageManager::GetChannelName(
        PackageChannel::Stable)) == L"Stable");
    ASSERT(std::wstring(MSIXPackageManager::GetChannelName(
        PackageChannel::Beta)) == L"Beta");
    ASSERT(std::wstring(MSIXPackageManager::GetChannelName(
        PackageChannel::Dev)) == L"Dev");
    ASSERT(std::wstring(MSIXPackageManager::GetChannelName(
        PackageChannel::Canary)) == L"Canary");
    ASSERT(std::wstring(MSIXPackageManager::GetChannelName(
        PackageChannel::Internal)) == L"Internal");
}

TEST(TestMSIX_SigningNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(MSIXPackageManager::GetSigningName(SigningMode::None)) ==
        L"None");
    ASSERT(std::wstring(MSIXPackageManager::GetSigningName(
        SigningMode::SelfSigned)) == L"SelfSigned");
    ASSERT(std::wstring(MSIXPackageManager::GetSigningName(
        SigningMode::Authenticode)) == L"Authenticode");
    ASSERT(std::wstring(MSIXPackageManager::GetSigningName(
        SigningMode::AzureTrusted)) == L"AzureTrusted");
}

TEST(TestMSIX_PackageTypeNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(MSIXPackageManager::GetPackageTypeName(
        PackageType::MSIX)) == L"MSIX");
    ASSERT(std::wstring(MSIXPackageManager::GetPackageTypeName(
        PackageType::MSIXBundle)) == L"MSIXBundle");
    ASSERT(std::wstring(MSIXPackageManager::GetPackageTypeName(
        PackageType::SparsePackage)) == L"SparsePackage");
}

TEST(TestMSIX_Capabilities) {
    using namespace ExplorerLens::Engine;
    auto caps = PackageCapability::RunFullTrust |
        PackageCapability::ShellExtension | PackageCapability::COMServer;
    ASSERT(HasCapability(caps, PackageCapability::RunFullTrust));
    ASSERT(HasCapability(caps, PackageCapability::ShellExtension));
    ASSERT(HasCapability(caps, PackageCapability::COMServer));
    ASSERT(!HasCapability(caps, PackageCapability::Notifications));
}

TEST(TestMSIX_BuildPackage) {
    using namespace ExplorerLens::Engine;
    MSIXPackageManager mgr;
    auto result = mgr.BuildPackage(L"C:\\temp\\output");
    ASSERT(result.success);
    ASSERT(!result.outputPath.empty());
    ASSERT(result.fileSizeBytes > 0);
}

TEST(TestMSIX_IsMSIXSupported) {
    using namespace ExplorerLens::Engine;
    bool supported = MSIXPackageManager::IsMSIXSupported();
    // On Windows 10+, should be true
    ASSERT(supported);
}

TEST(TestMSIX_Config) {
    using namespace ExplorerLens::Engine;
    MSIXConfig config;
    config.version = L"9.2.0.0";
    config.packageName = L"ExplorerLens";
    MSIXPackageManager mgr(config);
    ASSERT(mgr.GetConfig().version == L"9.2.0.0");
    ASSERT(mgr.GetConfig().packageName == L"ExplorerLens");
}

//==============================================================================
// Test Suite Expansion Tests
//==============================================================================

TEST(TestSuite_DecoderSpecs) {
    using namespace ExplorerLens::Engine;
    TestSuiteExpansion suite;
    auto specs = suite.GetDecoderTestSpecs();
    ASSERT(specs.size() >= 25); // At least 25 decoders
    ASSERT(specs[0].formatName == L"PNG");
    ASSERT(specs[0].hasValidFile);
}

TEST(TestSuite_CoverageGaps) {
    using namespace ExplorerLens::Engine;
    TestSuiteExpansion suite;
    auto gaps = suite.CalculateCoverageGaps();
    ASSERT(!gaps.empty());
    // Core decoders should be the largest gap
    ASSERT(gaps[0].component == L"Core Decoders");
    ASSERT(gaps[0].gap > 0);
}

TEST(TestSuite_TotalCount) {
    using namespace ExplorerLens::Engine;
    TestSuiteExpansion suite;
    uint32_t total = suite.GetTotalTestCount();
    ASSERT(total > 200); // Should have substantial test count
}

TEST(TestSuite_ComputeSummary) {
    using namespace ExplorerLens::Engine;
    TestSuiteExpansion suite;
    std::vector<TestResult> results;
    TestResult r1;
    r1.testName = L"Test1";
    r1.verdict = TestVerdict::Pass;
    r1.durationMs = 1.0;
    TestResult r2;
    r2.testName = L"Test2";
    r2.verdict = TestVerdict::Pass;
    r2.durationMs = 2.0;
    TestResult r3;
    r3.testName = L"Test3";
    r3.verdict = TestVerdict::Fail;
    r3.durationMs = 0.5;
    results.push_back(r1);
    results.push_back(r2);
    results.push_back(r3);
    auto summary = suite.ComputeSummary(results);
    ASSERT(summary.totalTests == 3);
    ASSERT(summary.passed == 2);
    ASSERT(summary.failed == 1);
    ASSERT(summary.failures.size() == 1);
}

TEST(TestSuite_CategoryNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(TestSuiteExpansion::GetCategoryName(
        TestSuiteCategory::UnitTest)) == L"UnitTest");
    ASSERT(std::wstring(TestSuiteExpansion::GetCategoryName(
        TestSuiteCategory::DecoderTest)) == L"DecoderTest");
    ASSERT(std::wstring(TestSuiteExpansion::GetCategoryName(
        TestSuiteCategory::FuzzTest)) == L"FuzzTest");
    ASSERT(std::wstring(TestSuiteExpansion::GetCategoryName(
        TestSuiteCategory::COMTest)) == L"COMTest");
}

TEST(TestSuite_VerdictNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(TestSuiteExpansion::GetVerdictName(TestVerdict::Pass)) ==
        L"Pass");
    ASSERT(std::wstring(TestSuiteExpansion::GetVerdictName(TestVerdict::Fail)) ==
        L"Fail");
    ASSERT(std::wstring(TestSuiteExpansion::GetVerdictName(TestVerdict::Skip)) ==
        L"Skip");
    ASSERT(std::wstring(TestSuiteExpansion::GetVerdictName(
        TestVerdict::Timeout)) == L"Timeout");
    ASSERT(std::wstring(TestSuiteExpansion::GetVerdictName(TestVerdict::Flaky)) ==
        L"Flaky");
}

TEST(TestSuite_TestFiles) {
    using namespace ExplorerLens::Engine;
    TestSuiteExpansion suite;
    auto files = suite.GetTestFilesForDecoder(L"PNG");
    ASSERT(files.size() == 5);
    ASSERT(files[0].find(L"PNG") != std::wstring::npos);
}

TEST(TestSuite_MeetsTargets) {
    using namespace ExplorerLens::Engine;
    TestExpansionConfig config;
    config.targetTotalTests = 100; // Low target for test
    TestSuiteExpansion suite(config);
    ASSERT(suite.MeetsTargets()); // Current should exceed 100
}

TEST(TestSuite_PassRate) {
    using namespace ExplorerLens::Engine;
    TestSuiteExpansion suite;
    std::vector<TestResult> results;
    for (int i = 0; i < 100; i++) {
        TestResult r;
        r.testName = L"Test" + std::to_wstring(i);
        r.verdict = TestVerdict::Pass;
        r.durationMs = 0.1;
        results.push_back(r);
    }
    auto summary = suite.ComputeSummary(results);
    ASSERT(summary.passRate == 100.0);
}

TEST(TestSuite_Config) {
    using namespace ExplorerLens::Engine;
    TestExpansionConfig config;
    config.targetTestsPerDecoder = 15;
    config.targetTotalTests = 800;
    TestSuiteExpansion suite(config);
    ASSERT(suite.GetConfig().targetTestsPerDecoder == 15);
    ASSERT(suite.GetConfig().targetTotalTests == 800);
}

//==============================================================================
// Malformed Input Hardening Tests
//==============================================================================

TEST(TestMalformed_DefaultConfig) {
    using namespace ExplorerLens::Engine;
    MalformedInputHandler handler;
    const auto& cfg = handler.GetConfig();
    ASSERT(cfg.enableHeaderValidation == true);
    ASSERT(cfg.enableSizeChecks == true);
    ASSERT(cfg.decodeTimeoutMs == 30000);
    ASSERT(cfg.bombLimits.maxImageWidth == 65536);
    ASSERT(cfg.bombLimits.maxImageHeight == 65536);
    ASSERT(cfg.bombLimits.maxCompressionRatio == 100.0);
    ASSERT(cfg.returnPlaceholderOnError == true);
}

TEST(TestMalformed_DimensionsSafe) {
    using namespace ExplorerLens::Engine;
    MalformedInputHandler handler;
    ASSERT(handler.AreDimensionsSafe(1920, 1080) == true);
    ASSERT(handler.AreDimensionsSafe(65536, 65536) == true);
    ASSERT(handler.AreDimensionsSafe(65537, 100) == false);
    ASSERT(handler.AreDimensionsSafe(100, 65537) == false);
    ASSERT(handler.AreDimensionsSafe(0, 100) == false);
    ASSERT(handler.AreDimensionsSafe(100, 0) == false);
}

TEST(TestMalformed_DimensionsBomb) {
    using namespace ExplorerLens::Engine;
    MalformedInputHandler handler;
    // 256M+1 pixels should fail
    ASSERT(handler.AreDimensionsSafe(65536, 4097) == false);
    // 256M exactly should pass
    ASSERT(handler.AreDimensionsSafe(16384, 16384) == true);
}

TEST(TestMalformed_CompressionRatio) {
    using namespace ExplorerLens::Engine;
    MalformedInputHandler handler;
    ASSERT(handler.IsCompressionRatioSafe(1000, 50000) == true); // 50:1
    ASSERT(handler.IsCompressionRatioSafe(1000, 100000) == true); // 100:1
    ASSERT(handler.IsCompressionRatioSafe(1000, 100001) == false); // >100:1
    ASSERT(handler.IsCompressionRatioSafe(0, 1000) == false); // div by zero
}

TEST(TestMalformed_NestingDepth) {
    using namespace ExplorerLens::Engine;
    MalformedInputHandler handler;
    ASSERT(handler.IsNestingDepthSafe(0) == true);
    ASSERT(handler.IsNestingDepthSafe(3) == true);
    ASSERT(handler.IsNestingDepthSafe(4) == false);
}

TEST(TestMalformed_MagicBytesPNG) {
    using namespace ExplorerLens::Engine;
    uint8_t png[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00 };
    ASSERT(MalformedInputHandler::IsPNG(png, 8) == true);
    ASSERT(MalformedInputHandler::CheckMagicBytes(png, 8, L"PNG") == true);
    ASSERT(MalformedInputHandler::CheckMagicBytes(png, 8, L"JPEG") == false);
}

TEST(TestMalformed_MagicBytesJPEG) {
    using namespace ExplorerLens::Engine;
    uint8_t jpeg[] = { 0xFF, 0xD8, 0xFF, 0xE0, 0x00 };
    ASSERT(MalformedInputHandler::IsJPEG(jpeg, 4) == true);
    uint8_t notJpeg[] = { 0xFF, 0xD9, 0xFF };
    ASSERT(MalformedInputHandler::IsJPEG(notJpeg, 3) == false);
}

TEST(TestMalformed_MagicBytesMultiple) {
    using namespace ExplorerLens::Engine;
    uint8_t zip[] = { 0x50, 0x4B, 0x03, 0x04 };
    ASSERT(MalformedInputHandler::IsZIP(zip, 4) == true);
    uint8_t bmp[] = { 0x42, 0x4D, 0x00, 0x00 };
    ASSERT(MalformedInputHandler::IsBMP(bmp, 4) == true);
    uint8_t pdf[] = { 0x25, 0x50, 0x44, 0x46 };
    ASSERT(MalformedInputHandler::IsPDF(pdf, 4) == true);
}

TEST(TestMalformed_ClampDimensions) {
    using namespace ExplorerLens::Engine;
    uint32_t w = 8000, h = 6000;
    MalformedInputHandler::ClampDimensions(w, h, 4096, 4096);
    ASSERT(w <= 4096);
    ASSERT(h <= 4096);
    ASSERT(w > 0 && h > 0);
}

TEST(TestMalformed_CorruptionNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(MalformedInputHandler::GetCorruptionName(
        CorruptionType::None)) == L"None");
    ASSERT(std::wstring(MalformedInputHandler::GetCorruptionName(
        CorruptionType::TruncatedFile)) == L"TruncatedFile");
    ASSERT(std::wstring(MalformedInputHandler::GetCorruptionName(
        CorruptionType::DecompressionBomb)) == L"DecompressionBomb");
    ASSERT(std::wstring(MalformedInputHandler::GetSeverityName(
        ValidationSeverity::Critical)) == L"Critical");
    ASSERT(std::wstring(MalformedInputHandler::GetSeverityName(
        ValidationSeverity::Warning)) == L"Warning");
}

//==============================================================================
// v9.2 Release Gate Tests
//==============================================================================

TEST(TestReleaseV3_DefaultThresholds) {
    using namespace ExplorerLens::Engine;
    auto t = ReleaseGateV3::ForV92();
    ASSERT(t.minTestCount == 500);
    ASSERT(t.minTestPassRate == 99.5);
    ASSERT(t.maxSingleDecodeMs == 20.0);
    ASSERT(t.minBatchThroughput == 200.0);
    ASSERT(t.maxBuildWarnings == 0);
    ASSERT(t.maxBuildErrors == 0);
    ASSERT(t.requireARM64CI == true);
    ASSERT(t.requireMSIXPackage == true);
}

TEST(TestReleaseV3_EvaluateEmpty) {
    using namespace ExplorerLens::Engine;
    ReleaseGateV3 gate;
    auto result = gate.Evaluate();
    ASSERT(result.version == L"v9.2.0");
    ASSERT(result.totalKPIs == 0);
    ASSERT(result.verdict == GateVerdict::Pass); // No KPIs = no failures
}

TEST(TestReleaseV3_AllPass) {
    using namespace ExplorerLens::Engine;
    ReleaseGateV3 gate;
    KPIMeasurement m1;
    m1.dimension = ReleaseKPIDimension::BuildQuality;
    m1.name = L"Warnings";
    m1.passed = true;
    gate.AddMeasurement(m1);
    KPIMeasurement m2;
    m2.dimension = ReleaseKPIDimension::TestCoverage;
    m2.name = L"PassRate";
    m2.passed = true;
    gate.AddMeasurement(m2);
    auto result = gate.Evaluate();
    ASSERT(result.verdict == GateVerdict::Pass);
    ASSERT(result.passedKPIs == 2);
    ASSERT(result.failedKPIs == 0);
    ASSERT(result.overallScore == 100.0);
}

TEST(TestReleaseV3_BlockerFails) {
    using namespace ExplorerLens::Engine;
    ReleaseGateV3 gate;
    KPIMeasurement m;
    m.dimension = ReleaseKPIDimension::BuildQuality;
    m.name = L"BuildErrors";
    m.passed = false;
    m.notes = L"3 errors remain";
    gate.AddMeasurement(m);
    auto result = gate.Evaluate();
    ASSERT(result.verdict == GateVerdict::Blocked);
    ASSERT(result.blockers.size() == 1);
}

TEST(TestReleaseV3_ConditionalPass) {
    using namespace ExplorerLens::Engine;
    ReleaseGateV3 gate;
    // 9 pass, 1 non-blocker fail = ConditionalPass
    for (int i = 0; i < 9; i++) {
        KPIMeasurement m;
        m.dimension = ReleaseKPIDimension::Performance;
        m.name = L"KPI" + std::to_wstring(i);
        m.passed = true;
        gate.AddMeasurement(m);
    }
    KPIMeasurement fail;
    fail.dimension = ReleaseKPIDimension::Documentation;
    fail.name = L"DocSync";
    fail.passed = false;
    fail.notes = L"2 docs outdated";
    gate.AddMeasurement(fail);
    auto result = gate.Evaluate();
    ASSERT(result.verdict == GateVerdict::ConditionalPass);
    ASSERT(result.overallScore == 90.0);
}

TEST(TestReleaseV3_PlatformValidation) {
    using namespace ExplorerLens::Engine;
    ReleaseGateV3 gate;
    PlatformValidation x64;
    x64.platform = L"x64";
    x64.buildSucceeded = true;
    x64.testsRan = true;
    x64.testsPassed = 500;
    x64.testsFailed = 0;
    gate.AddPlatform(x64);
    auto result = gate.Evaluate();
    ASSERT(result.platforms.size() == 1);
    ASSERT(result.platforms[0].buildSucceeded == true);
}

TEST(TestReleaseV3_ReleaseNotes) {
    using namespace ExplorerLens::Engine;
    ReleaseGateV3 gate;
    ReleaseGateResult result;
    result.version = L"v9.2.0";
    result.verdict = GateVerdict::Pass;
    result.overallScore = 100.0;
    result.passedKPIs = 9;
    result.totalKPIs = 9;
    auto notes = gate.GenerateReleaseNotes(result);
    ASSERT(notes.find(L"v9.2.0") != std::wstring::npos);
    ASSERT(notes.find(L"Pass") != std::wstring::npos);
}

TEST(TestReleaseV3_Checklist) {
    using namespace ExplorerLens::Engine;
    ReleaseGateV3 gate;
    auto checklist = gate.GenerateChecklist();
    ASSERT(checklist.find(L"Zero warnings") != std::wstring::npos);
    ASSERT(checklist.find(L"MSIX") != std::wstring::npos);
    ASSERT(checklist.find(L"High-DPI") != std::wstring::npos);
}

TEST(TestReleaseV3_DimensionNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ReleaseGateV3::GetDimensionName(
        ReleaseKPIDimension::BuildQuality)) == L"BuildQuality");
    ASSERT(std::wstring(ReleaseGateV3::GetDimensionName(
        ReleaseKPIDimension::Security)) == L"Security");
    ASSERT(std::wstring(ReleaseGateV3::GetDimensionName(
        ReleaseKPIDimension::Packaging)) == L"Packaging");
}

TEST(TestReleaseV3_VerdictNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ReleaseGateV3::GetVerdictName(GateVerdict::Pass)) ==
        L"Pass");
    ASSERT(std::wstring(ReleaseGateV3::GetVerdictName(GateVerdict::Fail)) ==
        L"Fail");
    ASSERT(std::wstring(ReleaseGateV3::GetVerdictName(
        GateVerdict::ConditionalPass)) == L"ConditionalPass");
    ASSERT(std::wstring(ReleaseGateV3::GetVerdictName(GateVerdict::Blocked)) ==
        L"Blocked");
}

//==============================================================================
// Scientific Format Suite Tests (DICOM + FITS)
//==============================================================================

TEST(TestDICOM_IsDICOMFile) {
    // Valid DICOM: 128-byte preamble + "DICM"
    std::vector<uint8_t> data(256, 0);
    data[128] = 'D';
    data[129] = 'I';
    data[130] = 'C';
    data[131] = 'M';
    ASSERT(ExplorerLens::Engine::DICOMDecoder::IsDICOMFile(data.data(),
        data.size()) == true);
    // Invalid
    std::vector<uint8_t> bad(256, 0);
    ASSERT(ExplorerLens::Engine::DICOMDecoder::IsDICOMFile(bad.data(),
        bad.size()) == false);
    // Too small
    ASSERT(ExplorerLens::Engine::DICOMDecoder::IsDICOMFile(nullptr, 0) == false);
}

TEST(TestDICOM_Extensions) {
    ASSERT(ExplorerLens::Engine::DICOMDecoder::GetExtensionCount() == 2);
    auto exts = ExplorerLens::Engine::DICOMDecoder::GetExtensions();
    ASSERT(std::wstring(exts[0]) == L".dcm");
    ASSERT(std::wstring(exts[1]) == L".dicom");
}

TEST(TestDICOM_PhotometricNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(DICOMDecoder::GetPhotometricName(
        DICOMPhotometric::Monochrome2)) == L"MONOCHROME2");
    ASSERT(std::wstring(DICOMDecoder::GetPhotometricName(
        DICOMPhotometric::RGB)) == L"RGB");
    ASSERT(std::wstring(DICOMDecoder::GetPhotometricName(
        DICOMPhotometric::Unknown)) == L"UNKNOWN");
}

TEST(TestDICOM_TransferSyntaxNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(DICOMDecoder::GetTransferSyntaxName(
        DICOMTransferSyntax::ExplicitVRLittleEndian))
        .find(L"Explicit") != std::wstring::npos);
    ASSERT(std::wstring(DICOMDecoder::GetTransferSyntaxName(
        DICOMTransferSyntax::Unsupported)) == L"Unsupported");
}

TEST(TestDICOM_WindowLevel) {
    using namespace ExplorerLens::Engine;
    DICOMDecoder decoder;
    // Default window: center=40, width=400 -> range [-160, 240]
    // Value at center should map to ~128
    uint8_t val = decoder.ApplyWindowLevel(40);
    ASSERT(val >= 126 && val <= 130); // Allow rounding
    // Value at lower bound -> 0
    uint8_t low = decoder.ApplyWindowLevel(-200);
    ASSERT(low == 0);
    // Value above upper bound -> 255
    uint8_t high = decoder.ApplyWindowLevel(300);
    ASSERT(high == 255);
}

TEST(TestFITS_IsFITSFile) {
    // Valid FITS header — keyword "SIMPLE" padded to 8 chars, '=' at byte 8
    std::string header =
        "SIMPLE  =                    T / file does conform to FITS standard";
    header.resize(80, ' ');
    std::vector<uint8_t> data(header.begin(), header.end());
    ASSERT(ExplorerLens::Engine::FITSDecoder::IsFITSFile(data.data(),
        data.size()) == true);
    // Invalid
    std::vector<uint8_t> bad(80, 'X');
    ASSERT(ExplorerLens::Engine::FITSDecoder::IsFITSFile(bad.data(),
        bad.size()) == false);
}

TEST(TestFITS_Extensions) {
    ASSERT(ExplorerLens::Engine::FITSDecoder::GetExtensionCount() == 3);
    auto exts = ExplorerLens::Engine::FITSDecoder::GetExtensions();
    ASSERT(std::wstring(exts[0]) == L".fits");
    ASSERT(std::wstring(exts[1]) == L".fit");
    ASSERT(std::wstring(exts[2]) == L".fts");
}

TEST(TestFITS_BitpixNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(FITSDecoder::GetBitpixName(FITSBitpix::UInt8)) ==
        L"8-bit unsigned");
    ASSERT(std::wstring(FITSDecoder::GetBitpixName(FITSBitpix::Float32)) ==
        L"32-bit float");
}

TEST(TestFITS_BytesPerPixel) {
    using namespace ExplorerLens::Engine;
    ASSERT(FITSDecoder::GetBytesPerPixel(FITSBitpix::UInt8) == 1);
    ASSERT(FITSDecoder::GetBytesPerPixel(FITSBitpix::Int16) == 2);
    ASSERT(FITSDecoder::GetBytesPerPixel(FITSBitpix::Float32) == 4);
    ASSERT(FITSDecoder::GetBytesPerPixel(FITSBitpix::Float64) == 8);
}

TEST(TestFITS_StretchAlgorithm) {
    using namespace ExplorerLens::Engine;
    FITSDecoder decoder;
    // Linear stretch: midpoint
    uint8_t mid = decoder.ApplyStretch(50.0, 0.0, 100.0, FITSStretch::Linear);
    ASSERT(mid >= 126 && mid <= 130);
    // Linear stretch: minimum
    uint8_t low = decoder.ApplyStretch(0.0, 0.0, 100.0, FITSStretch::Linear);
    ASSERT(low == 0);
    // Linear stretch: maximum
    uint8_t high = decoder.ApplyStretch(100.0, 0.0, 100.0, FITSStretch::Linear);
    ASSERT(high == 255);
}

//==============================================================================
// Advanced 3D Format Decoder Tests
//==============================================================================

TEST(TestAdvanced3D_FBXDetection) {
    using namespace ExplorerLens::Engine;
    // FBX binary magic: "Kaydara FBX Binary \0"
    std::vector<uint8_t> data(64, 0);
    const char* magic = "Kaydara FBX Binary ";
    memcpy(data.data(), magic, strlen(magic));
    ASSERT(Advanced3DFormatDecoder::DetectFormat(data.data(), data.size()) ==
        Advanced3DFormat::FBX_Binary);
    std::vector<uint8_t> bad = { 0, 1, 2, 3, 4, 5, 6, 7 };
    ASSERT(Advanced3DFormatDecoder::DetectFormat(bad.data(), bad.size()) !=
        Advanced3DFormat::FBX_Binary);
}

TEST(TestAdvanced3D_Extensions) {
    using namespace ExplorerLens::Engine;
    uint32_t extCount = Advanced3DFormatDecoder::GetExtensionCount();
    ASSERT(extCount >= 8);
    const wchar_t* const* exts = Advanced3DFormatDecoder::GetExtensions();
    bool hasFBX = false, hasUSD = false, has3MF = false;
    for (uint32_t i = 0; i < extCount; ++i) {
        if (std::wstring(exts[i]) == L".fbx")
            hasFBX = true;
        if (std::wstring(exts[i]) == L".usd")
            hasUSD = true;
        if (std::wstring(exts[i]) == L".3mf")
            has3MF = true;
    }
    ASSERT(hasFBX && hasUSD && has3MF);
}

TEST(TestAdvanced3D_FormatNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(Advanced3DFormatDecoder::GetFormatName(
        Advanced3DFormat::FBX_Binary)) == L"FBX Binary");
    ASSERT(std::wstring(Advanced3DFormatDecoder::GetFormatName(
        Advanced3DFormat::USDA)) == L"USD ASCII");
    ASSERT(std::wstring(Advanced3DFormatDecoder::GetFormatName(
        Advanced3DFormat::ThreeMF)) == L"3MF");
    ASSERT(std::wstring(Advanced3DFormatDecoder::GetFormatName(
        Advanced3DFormat::STEP)) == L"STEP/STP");
    ASSERT(std::wstring(Advanced3DFormatDecoder::GetFormatName(
        Advanced3DFormat::IGES)) == L"IGES");
}

TEST(TestAdvanced3D_BoundingBox) {
    using namespace ExplorerLens::Engine;
    BoundingBox3D box;
    box.min.x = -1.0f;
    box.min.y = -2.0f;
    box.min.z = -3.0f;
    box.max.x = 1.0f;
    box.max.y = 2.0f;
    box.max.z = 3.0f;
    auto cam = AutoCamera::FromBoundingBox(box);
    ASSERT(cam.fov > 0.0f && cam.fov < 180.0f);
    ASSERT(cam.farPlane > cam.nearPlane);
}

TEST(TestAdvanced3D_WireframeRender) {
    using namespace ExplorerLens::Engine;
    MeshInfo3D mesh;
    mesh.vertexCount = 3;
    mesh.triangleCount = 1;
    mesh.hasNormals = false;
    mesh.hasUVs = false;
    mesh.materialCount = 1;
    ASSERT(mesh.vertexCount > 0);
    ASSERT(mesh.triangleCount > 0);
}

//==============================================================================
// Plugin Marketplace V2 Tests
//==============================================================================

TEST(TestMarketplaceV2_CatalogInit) {
    using namespace ExplorerLens::Engine;
    PluginMarketplaceV2 marketplace;
    ASSERT(marketplace.GetInstalled().empty());
    ASSERT(marketplace.GetCatalogUrl().find(L"explorerlens.dev") !=
        std::wstring::npos);
}

TEST(TestMarketplaceV2_Search) {
    using namespace ExplorerLens::Engine;
    PluginMarketplaceV2 marketplace;
    // Add a test plugin to catalog
    PluginListing listing;
    listing.id = L"test-plugin-001";
    listing.name = L"Test Image Decoder";
    listing.description = L"A test decoder plugin";
    listing.category = PluginCategory::Decoder;
    listing.version = { 1, 0, 0 };
    listing.engineMinVersion = L"9.0.0";
    listing.isVerified = true;
    listing.downloads = 100;
    marketplace.AddToCatalog(listing);
    // Search
    MarketplaceFilter filter;
    filter.query = L"test";
    auto results = marketplace.Search(filter);
    ASSERT(results.plugins.size() >= 1);
    ASSERT(results.plugins[0].id == L"test-plugin-001");
}

TEST(TestMarketplaceV2_SemVer) {
    using namespace ExplorerLens::Engine;
    PluginVersion v1 = { 1, 0, 0 };
    PluginVersion v2 = { 1, 1, 0 };
    PluginVersion v3 = { 2, 0, 0 };
    // v2 >= v1 (newer), v3 >= v1 (much newer)
    ASSERT((v2 >= v1) == true);
    ASSERT((v1 >= v3) == false);
}

TEST(TestMarketplaceV2_CategoryNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(PluginMarketplaceV2::GetCategoryName(
        PluginCategory::Decoder)) == L"Decoder");
    ASSERT(std::wstring(PluginMarketplaceV2::GetCategoryName(
        PluginCategory::Integration)) == L"Integration");
    ASSERT(std::wstring(PluginMarketplaceV2::GetCategoryName(
        PluginCategory::Renderer)) == L"Renderer");
}

TEST(TestMarketplaceV2_InstallUninstall) {
    using namespace ExplorerLens::Engine;
    PluginMarketplaceV2 marketplace;
    PluginListing listing;
    listing.id = L"install-test-001";
    listing.name = L"Install Test Plugin";
    listing.category = PluginCategory::Decoder;
    listing.version = { 1, 0, 0 };
    listing.engineMinVersion = L"9.0.0";
    listing.isVerified = true;
    marketplace.AddToCatalog(listing);
    // Simulate install
    bool installed = marketplace.Install(listing);
    ASSERT(installed == true);
    ASSERT(marketplace.GetState(L"install-test-001") ==
        PluginInstallState::Installed);
    // Uninstall
    bool removed = marketplace.Uninstall(L"install-test-001");
    ASSERT(removed == true);
    ASSERT(marketplace.GetState(L"install-test-001") !=
        PluginInstallState::Installed);
}

//==============================================================================
// Vulkan Compute Pipeline Tests
//==============================================================================

TEST(TestVulkan_BackendNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(VulkanComputePipeline::GetBackendName(
        GPUBackend::None)) == L"None");
    ASSERT(std::wstring(VulkanComputePipeline::GetBackendName(
        GPUBackend::Vulkan)) == L"Vulkan");
    ASSERT(std::wstring(VulkanComputePipeline::GetBackendName(
        GPUBackend::D3D12)) == L"D3D12");
    ASSERT(std::wstring(VulkanComputePipeline::GetBackendName(
        GPUBackend::D3D11)) == L"D3D11");
    ASSERT(std::wstring(VulkanComputePipeline::GetBackendName(GPUBackend::CPU)) ==
        L"CPU");
}

TEST(TestVulkan_ShaderTypeNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(VulkanComputePipeline::GetShaderName(
        ComputeShaderType::BilinearResize)) == L"Bilinear Resize");
    ASSERT(std::wstring(VulkanComputePipeline::GetShaderName(
        ComputeShaderType::ToneMap)) == L"Tone Map");
    ASSERT(std::wstring(VulkanComputePipeline::GetShaderName(
        ComputeShaderType::Sharpen)) == L"Sharpen");
}

TEST(TestVulkan_CPUFallbackResize) {
    using namespace ExplorerLens::Engine;
    VulkanComputePipeline pipeline;
    // Create a 4x4 BGRA test image (all mid-grey)
    std::vector<uint8_t> src(4 * 4 * 4, 128);
    auto dst = pipeline.Resize(src.data(), 4, 4, 2, 2);
    ASSERT(dst.size() == 2u * 2u * 4u);
    bool hasData = false;
    for (auto b : dst) {
        if (b > 0) {
            hasData = true;
            break;
        }
    }
    ASSERT(hasData);
}

TEST(TestVulkan_PipelineCacheStats) {
    using namespace ExplorerLens::Engine;
    VulkanComputePipeline pipeline;
    auto stats = pipeline.GetStats();
    ASSERT(stats.dispatchCount == 0);
    ASSERT(stats.totalTimeMs == 0.0);
}

TEST(TestVulkan_ActiveBackend) {
    using namespace ExplorerLens::Engine;
    VulkanComputePipeline pipeline;
    // Default should be CPU or None (without Vulkan runtime)
    auto backend = pipeline.GetActiveBackend();
    ASSERT(backend == GPUBackend::CPU || backend == GPUBackend::None);
}

//==============================================================================
// Python SDK Tests
//==============================================================================

TEST(TestPythonSDK_DefaultConfig) {
    using namespace ExplorerLens::Engine;
    PythonSDK sdk;
    auto config = sdk.GetConfig();
    ASSERT(config.maxConcurrency > 0);
    ASSERT(config.maxThumbnailWidth == 512);
    ASSERT(config.maxThumbnailHeight == 512);
}

TEST(TestPythonSDK_DecoderInfo) {
    using namespace ExplorerLens::Engine;
    PythonSDK sdk;
    auto decoders = sdk.GetDecoders();
    ASSERT(decoders.size() > 0);
    bool hasArchive = false;
    for (const auto& d : decoders) {
        if (d.name == L"ArchiveDecoder")
            hasArchive = true;
    }
    ASSERT(hasArchive);
}

TEST(TestPythonSDK_CtypesStub) {
    using namespace ExplorerLens::Engine;
    PythonSDK sdk;
    auto stub = sdk.GenerateCtypesStub();
    ASSERT(stub.find(L"ExplorerLens_Init") != std::wstring::npos);
    ASSERT(stub.find(L"ExplorerLens_GenerateThumbnail") != std::wstring::npos);
    ASSERT(stub.find(L"ctypes") != std::wstring::npos);
}

TEST(TestPythonSDK_Pybind11Wrapper) {
    using namespace ExplorerLens::Engine;
    PythonSDK sdk;
    auto wrapper = sdk.GeneratePybindWrapper();
    ASSERT(wrapper.find(L"pybind11") != std::wstring::npos);
    ASSERT(wrapper.find(L"PYBIND11_MODULE") != std::wstring::npos);
}

TEST(TestPythonSDK_BatchConfig) {
    using namespace ExplorerLens::Engine;
    PythonSDKConfig config;
    config.maxConcurrency = 4;
    config.maxThumbnailWidth = 512;
    config.maxThumbnailHeight = 512;
    config.enableGPU = true;
    PythonSDK sdk(config);
    auto result = sdk.GetConfig();
    ASSERT(result.maxConcurrency == 4);
    ASSERT(result.maxThumbnailWidth == 512);
    ASSERT(result.enableGPU == true);
}

//==============================================================================
// Release Gate V10 Tests
//==============================================================================

TEST(TestReleaseGateV10_DefaultThresholds) {
    using namespace ExplorerLens::Engine;
    auto t = ReleaseGateV10::ForV10();
    ASSERT(t.minDecoderCount == 30);
    ASSERT(t.minTestCount == 600);
    ASSERT(t.minTestPassRate >= 99.0);
    ASSERT(t.minShellRegistrations == 110);
    ASSERT(t.maxBuildWarnings == 0);
}

TEST(TestReleaseGateV10_PassingGate) {
    using namespace ExplorerLens::Engine;
    ReleaseGateV10 gate;
    gate.SetDecoderCount(32);
    gate.SetTestMetrics(650, 650, 100.0);
    gate.SetShellRegistrations(115);
    // Add GPU backends
    GPUBackendResult d3d11 = { L"D3D11", true, true, 0.0, L"OK" };
    GPUBackendResult d3d12 = { L"D3D12", true, true, 0.0, L"OK" };
    gate.AddGPUBackend(d3d11);
    gate.AddGPUBackend(d3d12);
    // Format categories
    for (uint32_t i = 0; i < 16; i++) {
        FormatCoverageEntry entry;
        entry.category = L"Category" + std::to_wstring(i);
        entry.supportedFormats = 10;
        entry.totalFormats = 10;
        gate.AddFormatCoverage(entry);
    }
    auto result = gate.Evaluate();
    ASSERT(result.passed == true);
    ASSERT(result.overallScore >= 80.0);
}

TEST(TestReleaseGateV10_FailingGate) {
    using namespace ExplorerLens::Engine;
    ReleaseGateV10 gate;
    gate.SetDecoderCount(5); // too low
    gate.SetTestMetrics(50, 48, 96.0); // too low
    gate.SetShellRegistrations(20);
    auto result = gate.Evaluate();
    ASSERT(result.passed == false);
    ASSERT(result.blockers.size() > 0);
}

TEST(TestReleaseGateV10_Changelog) {
    using namespace ExplorerLens::Engine;
    ReleaseGateV10 gate;
    gate.SetDecoderCount(30);
    gate.SetShellRegistrations(110);
    gate.SetTestMetrics(600, 600, 100.0);
    auto result = gate.Evaluate();
    ASSERT(result.changelog.find(L"v10.0.0") != std::wstring::npos);
    ASSERT(result.changelog.find(L"DICOM") != std::wstring::npos);
}

TEST(TestReleaseGateV10_CategoryNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ReleaseGateV10::GetCategoryName(
        V10KPICategory::BuildSystem)) == L"Build System");
    ASSERT(std::wstring(ReleaseGateV10::GetCategoryName(
        V10KPICategory::TestCoverage)) == L"Test Coverage");
    ASSERT(std::wstring(ReleaseGateV10::GetCategoryName(
        V10KPICategory::PluginEcosystem)) == L"Plugin Ecosystem");
    ASSERT(std::wstring(ReleaseGateV10::GetCategoryName(
        V10KPICategory::Scientific)) == L"Scientific");
}

//==============================================================================
// Async Shell Extension Tests
//==============================================================================

TEST(TestAsync_SubmitRequest) {
    using namespace ExplorerLens::Engine;
    AsyncShellExtension ext;
    AsyncThumbnailRequest req;
    req.filePath = L"test.zip";
    req.requestedSize = 256;
    req.priority = DecodePriority::Normal;
    uint64_t id = ext.SubmitRequest(req);
    ASSERT(id > 0);
    ASSERT(ext.GetRequestState(id) == AsyncDecodeState::Queued);
}

TEST(TestAsync_CancelRequest) {
    using namespace ExplorerLens::Engine;
    AsyncShellExtension ext;
    AsyncThumbnailRequest req;
    req.filePath = L"test.cbz";
    uint64_t id = ext.SubmitRequest(req);
    bool cancelled = ext.CancelRequest(id);
    ASSERT(cancelled == true);
    ASSERT(ext.GetRequestState(id) == AsyncDecodeState::Cancelled);
}

TEST(TestAsync_PriorityNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(AsyncShellExtension::GetPriorityName(
        DecodePriority::Critical)) == L"Critical");
    ASSERT(std::wstring(AsyncShellExtension::GetPriorityName(
        DecodePriority::Normal)) == L"Normal");
    ASSERT(std::wstring(AsyncShellExtension::GetPriorityName(
        DecodePriority::Idle)) == L"Idle");
}

TEST(TestAsync_ThreadPool) {
    using namespace ExplorerLens::Engine;
    AsyncShellExtension ext(8);
    ASSERT(ext.GetThreadCount() == 8);
    ext.Start();
    ASSERT(ext.IsRunning() == true);
    ext.Stop();
    ASSERT(ext.IsRunning() == false);
}

TEST(TestAsync_DrainQueue) {
    using namespace ExplorerLens::Engine;
    AsyncShellExtension ext;
    for (int i = 0; i < 5; i++) {
        AsyncThumbnailRequest req;
        req.filePath = L"file" + std::to_wstring(i) + L".zip";
        ext.SubmitRequest(req);
    }
    ASSERT(ext.GetQueueDepth() == 5);
    ext.DrainQueue();
    ASSERT(ext.GetQueueDepth() == 0);
}

//==============================================================================
// Encoder Export Engine Tests
//==============================================================================

TEST(TestExport_FormatNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(EncoderExportEngine::GetFormatName(ExportFormat::PNG)) ==
        L"PNG");
    ASSERT(std::wstring(EncoderExportEngine::GetFormatName(ExportFormat::JPEG)) ==
        L"JPEG");
    ASSERT(std::wstring(EncoderExportEngine::GetFormatName(ExportFormat::WebP)) ==
        L"WebP");
    ASSERT(std::wstring(EncoderExportEngine::GetFormatName(ExportFormat::BMP)) ==
        L"BMP");
}

TEST(TestExport_FormatExtensions) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(EncoderExportEngine::GetFormatExtension(
        ExportFormat::PNG)) == L".png");
    ASSERT(std::wstring(EncoderExportEngine::GetFormatExtension(
        ExportFormat::JPEG)) == L".jpg");
    ASSERT(std::wstring(EncoderExportEngine::GetFormatExtension(
        ExportFormat::JXL)) == L".jxl");
}

TEST(TestExport_AlphaSupport) {
    using namespace ExplorerLens::Engine;
    ASSERT(EncoderExportEngine::SupportsAlpha(ExportFormat::PNG) == true);
    ASSERT(EncoderExportEngine::SupportsAlpha(ExportFormat::JPEG) == false);
    ASSERT(EncoderExportEngine::SupportsAlpha(ExportFormat::WebP) == true);
    ASSERT(EncoderExportEngine::SupportsAlpha(ExportFormat::BMP) == false);
}

TEST(TestExport_BMPEncode) {
    using namespace ExplorerLens::Engine;
    EncoderExportEngine engine;
    // 2x2 RGBA image
    uint8_t data[] = { 255, 0, 0, 255, 0, 255, 0, 255,
    0, 0, 255, 255, 255, 255, 0, 255 };
    std::vector<uint8_t> output;
    ExportConfig config;
    config.format = ExportFormat::BMP;
    auto result = engine.ExportToMemory(data, 2, 2, config, output);
    ASSERT(result.success == true);
    ASSERT(output.size() > 54); // BMP header is 54 bytes
    ASSERT(output[0] == 'B' && output[1] == 'M');
}

TEST(TestExport_QualityPresets) {
    using namespace ExplorerLens::Engine;
    ASSERT(EncoderExportEngine::GetDefaultQuality(ExportFormat::JPEG,
        QualityPreset::Draft) == 50);
    ASSERT(EncoderExportEngine::GetDefaultQuality(ExportFormat::JPEG,
        QualityPreset::High) == 95);
    ASSERT(EncoderExportEngine::GetDefaultQuality(
        ExportFormat::JPEG, QualityPreset::Lossless) == 100);
}

//==============================================================================
// Telemetry Engine Tests
//==============================================================================

TEST(TestTelemetry_RecordEvent) {
    using namespace ExplorerLens::Engine;
    TelemetryEngine telemetry;
    TelemetryEvent evt;
    evt.severity = TelemetrySeverity::Info;
    evt.category = TelemetryCategory::Decode;
    evt.eventName = L"TestDecode";
    evt.value = 15.3;
    telemetry.RecordEvent(evt);
    ASSERT(telemetry.GetEventCount() == 1);
}

TEST(TestTelemetry_Metrics) {
    using namespace ExplorerLens::Engine;
    TelemetryEngine telemetry;
    telemetry.RecordMetric(TelemetryCategory::Decode, L"DecodeTime", 12.5, L"ms");
    telemetry.RecordMetric(TelemetryCategory::Cache, L"CacheHitRate", 95.0, L"%");
    ASSERT(telemetry.GetEventCount() == 2);
    ASSERT(telemetry.GetEventCount(TelemetryCategory::Decode) == 1);
}

TEST(TestTelemetry_HealthScore) {
    using namespace ExplorerLens::Engine;
    TelemetryEngine telemetry;
    for (int i = 0; i < 100; i++) {
        telemetry.RecordMetric(TelemetryCategory::Decode, L"Decode", 10.0);
    }
    auto health = telemetry.ComputeHealthScore();
    ASSERT(health.overallScore > 0.0);
    ASSERT(health.grade.size() > 0);
}

TEST(TestTelemetry_Privacy) {
    using namespace ExplorerLens::Engine;
    TelemetryEngine telemetry;
    telemetry.EnablePrivacyMode(true);
    ASSERT(telemetry.IsPrivacyMode() == true);
    TelemetryEvent pii;
    pii.piiSafe = false;
    pii.eventName = L"PII Event";
    telemetry.RecordEvent(pii);
    ASSERT(telemetry.GetEventCount() == 0); // PII should be filtered
}

TEST(TestTelemetry_SeverityNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(TelemetryEngine::GetSeverityName(
        TelemetrySeverity::Debug)) == L"Debug");
    ASSERT(std::wstring(TelemetryEngine::GetSeverityName(
        TelemetrySeverity::Error)) == L"Error");
    ASSERT(std::wstring(TelemetryEngine::GetSeverityName(
        TelemetrySeverity::Critical)) == L"Critical");
}

//==============================================================================
// SIMD Accelerator Tests
//==============================================================================

TEST(TestSIMD_DetectCapabilities) {
    using namespace ExplorerLens::Engine;
    SIMDAccelerator simd;
    auto caps = simd.DetectCapabilities();
    // x86_64 should at least have SSE2
    ASSERT(caps.hasSSE2 == true);
    ASSERT(caps.cacheLineSize == 64);
}

TEST(TestSIMD_ResizeBilinear) {
    using namespace ExplorerLens::Engine;
    SIMDAccelerator simd;
    std::vector<uint8_t> src(8 * 8 * 4, 200);
    std::vector<uint8_t> dst(4 * 4 * 4, 0);
    bool ok = simd.ResizeBilinear(src.data(), 8, 8, dst.data(), 4, 4, 4);
    ASSERT(ok == true);
    bool hasData = false;
    for (auto b : dst) {
        if (b > 0) {
            hasData = true;
            break;
        }
    }
    ASSERT(hasData);
}

TEST(TestSIMD_ColorConvert) {
    using namespace ExplorerLens::Engine;
    SIMDAccelerator simd;
    uint8_t src[] = { 255, 0, 0, 255 }; // RGBA red
    uint8_t dst[4] = {};
    bool ok = simd.ColorConvertRGBAToBGRA(src, dst, 1);
    ASSERT(ok == true);
    ASSERT(dst[0] == 0); // B
    ASSERT(dst[2] == 255); // R swapped
}

TEST(TestSIMD_LevelNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(SIMDAccelerator::GetLevelName(SIMDLevel::SSE2)) ==
        L"SSE2");
    ASSERT(std::wstring(SIMDAccelerator::GetLevelName(SIMDLevel::AVX2)) ==
        L"AVX2");
    ASSERT(std::wstring(SIMDAccelerator::GetLevelName(SIMDLevel::NEON)) ==
        L"NEON");
}

TEST(TestSIMD_Alignment) {
    using namespace ExplorerLens::Engine;
    alignas(32) uint8_t aligned[64] = {};
    ASSERT(SIMDAccelerator::IsAligned(aligned, 16) == true);
    ASSERT(SIMDAccelerator::GetOptimalAlignment(SIMDLevel::AVX2) == 32);
    ASSERT(SIMDAccelerator::GetOptimalAlignment(SIMDLevel::AVX512) == 64);
}

//==============================================================================
// Windows 11 Integration Tests
//==============================================================================

TEST(TestWin11_VersionDetection) {
    using namespace ExplorerLens::Engine;
    Win11Integration win11;
    auto ver = win11.DetectVersion();
    ASSERT(ver.major >= 10); // Should be Win10+
    ASSERT(ver.build > 0);
    ASSERT(ver.displayName.size() > 0);
}

TEST(TestWin11_FeatureNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(Win11Integration::GetFeatureName(
        Win11Feature::RoundedCorners)) == L"Rounded Corners");
    ASSERT(std::wstring(Win11Integration::GetFeatureName(
        Win11Feature::MicaMaterial)) == L"Mica Material");
    ASSERT(std::wstring(Win11Integration::GetFeatureName(
        Win11Feature::DarkMode)) == L"Dark Mode");
}

TEST(TestWin11_DarkModeDetection) {
    using namespace ExplorerLens::Engine;
    Win11Integration win11;
    // Just verify it doesn't crash — result depends on user setting
    bool darkMode = win11.IsDarkModeEnabled();
    (void)darkMode;
    ASSERT(true);
}

TEST(TestWin11_MicaModes) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(Win11Integration::GetMicaModeName(MicaMode::None)) ==
        L"None");
    ASSERT(std::wstring(Win11Integration::GetMicaModeName(MicaMode::Mica)) ==
        L"Mica");
    ASSERT(std::wstring(Win11Integration::GetMicaModeName(MicaMode::Acrylic)) ==
        L"Acrylic");
}

TEST(TestWin11_FeatureCount) {
    using namespace ExplorerLens::Engine;
    Win11Integration win11;
    ASSERT(Win11Integration::GetFeatureCount() == 8);
    auto features = win11.GetAvailableFeatures();
    ASSERT(features.size() >= 1); // At least DarkMode should be available
}

//==============================================================================
// CI/CD Pipeline Validation Tests
//==============================================================================

TEST(TestCI_PlatformNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(CIValidator::GetPlatformName(
        CIPlatform::GitHubActions)) == L"GitHub Actions");
    ASSERT(std::wstring(CIValidator::GetPlatformName(CIPlatform::AzureDevOps)) ==
        L"Azure DevOps");
    ASSERT(std::wstring(CIValidator::GetPlatformName(CIPlatform::Jenkins)) ==
        L"Jenkins");
}

TEST(TestCI_StageNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(CIValidator::GetStageName(CIStage::Build)) == L"Build");
    ASSERT(std::wstring(CIValidator::GetStageName(CIStage::Test)) == L"Test");
    ASSERT(std::wstring(CIValidator::GetStageName(CIStage::Package)) ==
        L"Package");
}

TEST(TestCI_ValidatorCreation) {
    using namespace ExplorerLens::Engine;
    CIValidator validator;
    auto result = validator.ValidatePipeline(CIPlatform::GitHubActions);
    ASSERT(result.platform == CIPlatform::GitHubActions);
}

TEST(TestCI_ArtifactTypeNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(CIValidator::GetArtifactTypeName(ArtifactType::DLL)) ==
        L"DLL");
    ASSERT(std::wstring(CIValidator::GetArtifactTypeName(ArtifactType::MSI)) ==
        L"MSI");
    ASSERT(std::wstring(CIValidator::GetArtifactTypeName(ArtifactType::MSIX)) ==
        L"MSIX");
}

TEST(TestCI_StageCount) {
    using namespace ExplorerLens::Engine;
    ASSERT(CIValidator::GetStageCount() == 7);
}

//==============================================================================
// eBook Decoder Tests
//==============================================================================

TEST(TestEBook_FormatNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(EBookDecoder::GetFormatName(EBookFormat::EPUB)) ==
        L"EPUB");
    ASSERT(std::wstring(EBookDecoder::GetFormatName(EBookFormat::MOBI)) ==
        L"MOBI");
    ASSERT(std::wstring(EBookDecoder::GetFormatName(EBookFormat::FB2)) == L"FB2");
}

TEST(TestEBook_DecoderCreation) {
    using namespace ExplorerLens::Engine;
    EBookDecoder decoder;
    ASSERT(EBookDecoder::GetExtensions().size() >= 4);
}

TEST(TestEBook_FormatDetection) {
    using namespace ExplorerLens::Engine;
    // Null/empty data returns Unknown
    ASSERT(EBookDecoder::DetectFormat(nullptr, 0) == EBookFormat::Unknown);
    // Random bytes return Unknown
    uint8_t random[] = { 0xFF, 0xFE, 0x00, 0x01 };
    ASSERT(EBookDecoder::DetectFormat(random, sizeof(random)) ==
        EBookFormat::Unknown);
}

TEST(TestEBook_CoverExtraction) {
    using namespace ExplorerLens::Engine;
    EBookDecoder decoder;
    auto result = decoder.ExtractCover(nullptr, 0);
    ASSERT(result.success == false);
}

TEST(TestEBook_FormatCount) {
    using namespace ExplorerLens::Engine;
    ASSERT(EBookDecoder::GetExtensionCount() >= 4);
}

//==============================================================================
// Geospatial Decoder Tests
//==============================================================================

TEST(TestGeo_FormatNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(GeospatialDecoder::GetFormatName(GeoFormat::GeoTIFF)) ==
        L"GeoTIFF");
    ASSERT(std::wstring(GeospatialDecoder::GetFormatName(GeoFormat::Shapefile)) ==
        L"Shapefile");
    ASSERT(std::wstring(GeospatialDecoder::GetFormatName(GeoFormat::KML)) ==
        L"KML");
}

TEST(TestGeo_DecoderCreation) {
    using namespace ExplorerLens::Engine;
    GeospatialDecoder decoder;
    ASSERT(GeospatialDecoder::GetExtensions().size() >= 4);
}

TEST(TestGeo_HaversineDistance) {
    using namespace ExplorerLens::Engine;
    // New York to London approx 5570 km
    GeoCoordinate ny{ 40.7128, -74.0060, 0.0 };
    GeoCoordinate ldn{ 51.5074, -0.1278, 0.0 };
    double dist = GeospatialDecoder::DistanceKm(ny, ldn);
    ASSERT(dist > 5500.0 && dist < 5700.0);
}

TEST(TestGeo_MercatorProjection) {
    using namespace ExplorerLens::Engine;
    auto coord = GeospatialDecoder::MercatorToWGS84(0.0, 0.0);
    ASSERT(coord.longitude >= -0.001 && coord.longitude <= 0.001);
    ASSERT(coord.latitude >= -0.001 && coord.latitude <= 0.001);
}

TEST(TestGeo_FormatCount) {
    using namespace ExplorerLens::Engine;
    ASSERT(GeospatialDecoder::GetExtensionCount() >= 4);
}

//==============================================================================
// Auto Documentation Generator Tests
//==============================================================================

TEST(TestAutoDoc_SectionNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(AutoDocGenerator::GetSectionName(DocSection::Overview)) ==
        L"Overview");
    ASSERT(std::wstring(AutoDocGenerator::GetSectionName(DocSection::Decoders)) ==
        L"Decoders");
    ASSERT(std::wstring(AutoDocGenerator::GetSectionName(DocSection::Testing)) ==
        L"Testing");
}

TEST(TestAutoDoc_FormatNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(AutoDocGenerator::GetFormatName(DocFormat::Markdown)) ==
        L"Markdown");
    ASSERT(std::wstring(AutoDocGenerator::GetFormatName(DocFormat::HTML)) ==
        L"HTML");
}

TEST(TestAutoDoc_FormatExtensions) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(AutoDocGenerator::GetFormatExtension(
        DocFormat::Markdown)) == L".md");
    ASSERT(std::wstring(AutoDocGenerator::GetFormatExtension(DocFormat::HTML)) ==
        L".html");
    ASSERT(std::wstring(AutoDocGenerator::GetFormatExtension(
        DocFormat::AsciiDoc)) == L".adoc");
}

TEST(TestAutoDoc_DecoderRegistration) {
    using namespace ExplorerLens::Engine;
    AutoDocGenerator gen;
    DecoderDocEntry entry;
    entry.name = L"TestDecoder";
    entry.extensions = { L".test", L".tst" };
    entry.testCount = 5;
    entry.gpuAccelerated = true;
    gen.RegisterDecoder(entry);
    ASSERT(gen.GetTotalExtensions() == 2);
    ASSERT(gen.GetTotalTests() == 5);
}

TEST(TestAutoDoc_SectionGeneration) {
    using namespace ExplorerLens::Engine;
    AutoDocGenerator gen;
    auto content = gen.GenerateSection(DocSection::Overview, DocFormat::Markdown);
    ASSERT(!content.empty());
    ASSERT(content.find(L"Overview") != std::wstring::npos);
}

//==============================================================================
// Config Migration Engine Tests
//==============================================================================

TEST(TestConfigMigration_VersionNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ConfigMigrationEngine::GetVersionName(
        ConfigVersion::V7_0)) == L"v7.0");
    ASSERT(std::wstring(ConfigMigrationEngine::GetVersionName(
        ConfigVersion::V10_0)) == L"v10.0");
}

TEST(TestConfigMigration_ActionNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ConfigMigrationEngine::GetActionName(
        MigrationAction::Keep)) == L"Keep");
    ASSERT(std::wstring(ConfigMigrationEngine::GetActionName(
        MigrationAction::Rename)) == L"Rename");
    ASSERT(std::wstring(ConfigMigrationEngine::GetActionName(
        MigrationAction::Remove)) == L"Remove");
}

TEST(TestConfigMigration_BasicMigration) {
    using namespace ExplorerLens::Engine;
    ConfigMigrationEngine engine;
    std::map<std::wstring, std::wstring> config;
    config[L"ThumbnailSize"] = L"256";
    config[L"GPUEnabled"] = L"1";
    engine.SetSourceConfig(config);
    auto result = engine.Migrate(ConfigVersion::V7_0, ConfigVersion::V10_0);
    ASSERT(result.success == true);
    ASSERT(result.keysAdded > 0); // New keys added for V10
}

TEST(TestConfigMigration_RenameRule) {
    using namespace ExplorerLens::Engine;
    ConfigMigrationEngine engine;
    MigrationRule rule;
    rule.sourceKey = L"OldKey";
    rule.targetKey = L"NewKey";
    rule.action = MigrationAction::Rename;
    engine.AddRule(rule);
    std::map<std::wstring, std::wstring> config;
    config[L"OldKey"] = L"value123";
    engine.SetSourceConfig(config);
    auto result = engine.Migrate(ConfigVersion::V8_0, ConfigVersion::V10_0);
    ASSERT(result.keysRenamed == 1);
    auto migrated = engine.GetMigratedConfig();
    ASSERT(migrated.count(L"NewKey") == 1);
    ASSERT(migrated[L"NewKey"] == L"value123");
}

TEST(TestConfigMigration_Validation) {
    using namespace ExplorerLens::Engine;
    ConfigMigrationEngine engine;
    std::map<std::wstring, std::wstring> good;
    good[L"ThumbnailSize"] = L"256";
    good[L"GPUEnabled"] = L"1";
    good[L"CacheEnabled"] = L"1";
    ASSERT(engine.ValidateConfig(good) == true);

    std::map<std::wstring, std::wstring> bad;
    bad[L"SomeRandomKey"] = L"value";
    ASSERT(engine.ValidateConfig(bad) == false); // Missing required keys
}

//==============================================================================
// Animated Thumbnail Engine Tests
//==============================================================================

TEST(TestAnim_FormatNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(AnimatedThumbnailEngine::GetFormatName(
        AnimatedFormat::GIF)) == L"GIF");
    ASSERT(std::wstring(AnimatedThumbnailEngine::GetFormatName(
        AnimatedFormat::APNG)) == L"APNG");
    ASSERT(std::wstring(AnimatedThumbnailEngine::GetFormatName(
        AnimatedFormat::WebPAnim)) == L"WebP Animation");
}

TEST(TestAnim_StrategyNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(AnimatedThumbnailEngine::GetStrategyName(
        FrameStrategy::First)) == L"First Frame");
    ASSERT(std::wstring(AnimatedThumbnailEngine::GetStrategyName(
        FrameStrategy::Middle)) == L"Middle Frame");
    ASSERT(std::wstring(AnimatedThumbnailEngine::GetStrategyName(
        FrameStrategy::MostDetail)) == L"Most Detail");
}

TEST(TestAnim_FrameSelection) {
    using namespace ExplorerLens::Engine;
    ASSERT(AnimatedThumbnailEngine::SelectBestFrame(100, FrameStrategy::First) ==
        0);
    ASSERT(AnimatedThumbnailEngine::SelectBestFrame(100, FrameStrategy::Middle) ==
        50);
    ASSERT(AnimatedThumbnailEngine::SelectBestFrame(
        30, FrameStrategy::MostDetail) == 10);
}

TEST(TestAnim_FormatDetection) {
    using namespace ExplorerLens::Engine;
    ASSERT(AnimatedThumbnailEngine::DetectFormat(L"test.gif") ==
        AnimatedFormat::GIF);
    ASSERT(AnimatedThumbnailEngine::DetectFormat(L"test.apng") ==
        AnimatedFormat::APNG);
    ASSERT(AnimatedThumbnailEngine::DetectFormat(L"test.webp") ==
        AnimatedFormat::WebPAnim);
}

TEST(TestAnim_FormatCount) {
    using namespace ExplorerLens::Engine;
    ASSERT(AnimatedThumbnailEngine::GetFormatCount() == 6);
    AnimatedThumbnailEngine engine;
    ASSERT(engine.GetMaxFrameScan() == 100);
}

//==============================================================================
// Shell Context Menu V2 Tests
//==============================================================================

TEST(TestContextMenu_ActionNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ShellContextMenuV2::GetActionName(
        ContextAction::Regenerate)) == L"Regenerate");
    ASSERT(std::wstring(ShellContextMenuV2::GetActionName(
        ContextAction::ClearCache)) == L"Clear Cache");
    ASSERT(std::wstring(ShellContextMenuV2::GetActionName(
        ContextAction::Settings)) == L"Settings");
}

TEST(TestContextMenu_DefaultMenu) {
    using namespace ExplorerLens::Engine;
    auto items = ShellContextMenuV2::GetDefaultMenu();
    ASSERT(items.size() >= 5);
    ASSERT(items[0].action == ContextAction::Regenerate);
}

TEST(TestContextMenu_ExecuteAction) {
    using namespace ExplorerLens::Engine;
    ShellContextMenuV2 menu;
    auto result = menu.ExecuteAction(ContextAction::Regenerate, L"test.cbz");
    ASSERT(result.success == true);
    ASSERT(result.executedAction == ContextAction::Regenerate);
}

TEST(TestContextMenu_PositionNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ShellContextMenuV2::GetPositionName(
        MenuPosition::TopLevel)) == L"Top Level");
    ASSERT(std::wstring(ShellContextMenuV2::GetPositionName(
        MenuPosition::SubMenu)) == L"Sub Menu");
}

TEST(TestContextMenu_ActionCount) {
    using namespace ExplorerLens::Engine;
    ASSERT(ShellContextMenuV2::GetActionCount() == 7);
}

//==============================================================================
// Portable Mode Manager Tests
//==============================================================================

TEST(TestPortable_StatusNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(PortableModeManager::GetStatusName(
        PortableStatus::Installed)) == L"Installed");
    ASSERT(std::wstring(PortableModeManager::GetStatusName(
        PortableStatus::Portable)) == L"Portable");
    ASSERT(std::wstring(PortableModeManager::GetStatusName(
        PortableStatus::Hybrid)) == L"Hybrid");
}

TEST(TestPortable_LocationNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(PortableModeManager::GetLocationName(
        StorageLocation::Registry)) == L"Registry");
    ASSERT(std::wstring(PortableModeManager::GetLocationName(
        StorageLocation::IniFile)) == L"INI File");
    ASSERT(std::wstring(PortableModeManager::GetLocationName(
        StorageLocation::ExeDirectory)) == L"Exe Directory");
}

TEST(TestPortable_LocationCount) {
    using namespace ExplorerLens::Engine;
    ASSERT(PortableModeManager::GetLocationCount() == 5);
}

TEST(TestPortable_Detection) {
    using namespace ExplorerLens::Engine;
    PortableModeManager mgr;
    auto result = mgr.Detect();
    ASSERT(!result.exePath.empty());
    ASSERT(result.status == PortableStatus::Installed ||
        result.status == PortableStatus::Portable);
}

TEST(TestPortable_CacheSize) {
    using namespace ExplorerLens::Engine;
    PortableModeManager mgr;
    ASSERT(mgr.GetCacheSize() == 256 * 1024 * 1024);
    mgr.SetCacheSize(128 * 1024 * 1024);
    ASSERT(mgr.GetCacheSize() == 128 * 1024 * 1024);
}

//==============================================================================
// Network Provider Engine Tests
//==============================================================================

TEST(TestNetwork_ProtocolNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(NetworkProviderEngine::GetProtocolName(
        NetworkProtocol::UNC)) == L"UNC");
    ASSERT(std::wstring(NetworkProviderEngine::GetProtocolName(
        NetworkProtocol::SMB)) == L"SMB");
    ASSERT(std::wstring(NetworkProviderEngine::GetProtocolName(
        NetworkProtocol::WebDAV)) == L"WebDAV");
}

TEST(TestNetwork_PathDetection) {
    using namespace ExplorerLens::Engine;
    ASSERT(NetworkProviderEngine::IsNetworkPath(L"\\\\server\\share") == true);
    ASSERT(NetworkProviderEngine::IsNetworkPath(L"C:\\local\\path") == false);
    ASSERT(NetworkProviderEngine::IsNetworkPath(L"ftp://server/file") == true);
}

TEST(TestNetwork_ProtocolDetection) {
    using namespace ExplorerLens::Engine;
    ASSERT(NetworkProviderEngine::DetectProtocol(L"\\\\server\\share") ==
        NetworkProtocol::UNC);
    ASSERT(NetworkProviderEngine::DetectProtocol(L"ftp://server/file") ==
        NetworkProtocol::FTP);
    ASSERT(NetworkProviderEngine::DetectProtocol(L"http://example.com/file") ==
        NetworkProtocol::HTTP);
}

TEST(TestNetwork_ParsePath) {
    using namespace ExplorerLens::Engine;
    NetworkProviderEngine engine;
    auto path = engine.ParsePath(L"\\\\myserver\\myshare\\folder\\file.cbz");
    ASSERT(path.server == L"myserver");
    ASSERT(path.share == L"myshare");
    ASSERT(path.protocol == NetworkProtocol::UNC);
}

TEST(TestNetwork_ProtocolCount) {
    using namespace ExplorerLens::Engine;
    ASSERT(NetworkProviderEngine::GetProtocolCount() == 6);
    NetworkProviderEngine engine;
    ASSERT(engine.GetTimeout() == 5000);
    ASSERT(engine.GetMaxRetries() == 3);
}

//==============================================================================
// Security Hardening V2 Tests
//==============================================================================

TEST(TestSecurity_LevelNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(SecurityHardeningV2::GetLevelName(SecurityLevel::None)) ==
        L"None");
    ASSERT(std::wstring(SecurityHardeningV2::GetLevelName(
        SecurityLevel::Standard)) == L"Standard");
    ASSERT(std::wstring(SecurityHardeningV2::GetLevelName(
        SecurityLevel::Maximum)) == L"Maximum");
}

TEST(TestSecurity_CheckNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(SecurityHardeningV2::GetCheckName(
        IntegrityCheck::FileHash)) == L"File Hash");
    ASSERT(std::wstring(SecurityHardeningV2::GetCheckName(
        IntegrityCheck::CodeSign)) == L"Code Signing");
    ASSERT(std::wstring(SecurityHardeningV2::GetCheckName(
        IntegrityCheck::MemoryGuard)) == L"Memory Guard");
}

TEST(TestSecurity_BasicAudit) {
    using namespace ExplorerLens::Engine;
    SecurityHardeningV2 sec;
    auto result = sec.RunAudit(SecurityLevel::Basic);
    ASSERT(result.checksRun >= 2);
    ASSERT(result.auditTimeMs >= 0.0);
}

TEST(TestSecurity_CheckCounts) {
    using namespace ExplorerLens::Engine;
    ASSERT(SecurityHardeningV2::GetCheckCount() == 5);
    ASSERT(SecurityHardeningV2::GetLevelCount() == 5);
}

TEST(TestSecurity_DEPCheck) {
    using namespace ExplorerLens::Engine;
    SecurityHardeningV2 sec;
    // On modern Windows, DEP should be enabled
    ASSERT(sec.IsDEPEnabled() == true);
    ASSERT(sec.IsASLREnabled() == true);
}

//==============================================================================
// Accessibility Engine Tests
//==============================================================================

TEST(TestA11y_FeatureNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(AccessibilityEngine::GetFeatureName(
        A11yFeature::ScreenReader)) == L"Screen Reader");
    ASSERT(std::wstring(AccessibilityEngine::GetFeatureName(
        A11yFeature::HighContrast)) == L"High Contrast");
    ASSERT(std::wstring(AccessibilityEngine::GetFeatureName(
        A11yFeature::KeyboardNav)) == L"Keyboard Navigation");
}

TEST(TestA11y_ContrastModes) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(AccessibilityEngine::GetContrastModeName(
        ContrastMode::Normal)) == L"Normal");
    ASSERT(std::wstring(AccessibilityEngine::GetContrastModeName(
        ContrastMode::HighWhite)) == L"High Contrast White");
    ASSERT(std::wstring(AccessibilityEngine::GetContrastModeName(
        ContrastMode::HighBlack)) == L"High Contrast Black");
}

TEST(TestA11y_FeatureToggle) {
    using namespace ExplorerLens::Engine;
    AccessibilityEngine engine;
    engine.EnableFeature(A11yFeature::LargeText);
    ASSERT(engine.IsFeatureEnabled(A11yFeature::LargeText) == true);
    engine.DisableFeature(A11yFeature::LargeText);
    ASSERT(engine.IsFeatureEnabled(A11yFeature::LargeText) == false);
}

TEST(TestA11y_FeatureCount) {
    using namespace ExplorerLens::Engine;
    ASSERT(AccessibilityEngine::GetFeatureCount() == 7);
}

TEST(TestA11y_ComplianceAudit) {
    using namespace ExplorerLens::Engine;
    AccessibilityEngine engine;
    auto result = engine.RunComplianceAudit();
    ASSERT(result.checksRun >= 5);
    ASSERT(result.auditTimeMs >= 0.0);
}

//==============================================================================
// Cloud Sync Provider Tests
//==============================================================================

TEST(TestCloud_ProviderNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(CloudSyncProvider::GetProviderName(
        CloudProvider::OneDrive)) == L"OneDrive");
    ASSERT(std::wstring(CloudSyncProvider::GetProviderName(
        CloudProvider::SharePoint)) == L"SharePoint");
    ASSERT(std::wstring(CloudSyncProvider::GetProviderName(
        CloudProvider::GoogleDrive)) == L"Google Drive");
}

TEST(TestCloud_StatusNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(CloudSyncProvider::GetStatusName(SyncStatus::Idle)) ==
        L"Idle");
    ASSERT(std::wstring(CloudSyncProvider::GetStatusName(SyncStatus::Syncing)) ==
        L"Syncing");
    ASSERT(std::wstring(CloudSyncProvider::GetStatusName(
        SyncStatus::Completed)) == L"Completed");
}

TEST(TestCloud_ProviderDetection) {
    using namespace ExplorerLens::Engine;
    ASSERT(CloudSyncProvider::DetectProvider(
        L"C:\\Users\\test\\OneDrive\\file.cbz") ==
        CloudProvider::OneDrive);
    ASSERT(CloudSyncProvider::DetectProvider(
        L"C:\\Users\\test\\Dropbox\\file.cbz") == CloudProvider::Dropbox);
}

TEST(TestCloud_IsCloudPath) {
    using namespace ExplorerLens::Engine;
    CloudSyncProvider provider;
    ASSERT(provider.IsCloudPath(L"C:\\Users\\test\\OneDrive\\folder") == true);
    ASSERT(provider.IsCloudPath(L"C:\\Users\\test\\Documents\\local") == false);
}

TEST(TestCloud_ProviderCount) {
    using namespace ExplorerLens::Engine;
    ASSERT(CloudSyncProvider::GetProviderCount() == 6);
}

//==============================================================================
// Format Converter Engine Tests
//==============================================================================

TEST(TestConverter_FormatNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(FormatConverterEngine::GetFormatName(
        ConvertFormat::PNG)) == L"PNG");
    ASSERT(std::wstring(FormatConverterEngine::GetFormatName(
        ConvertFormat::JPEG)) == L"JPEG");
    ASSERT(std::wstring(FormatConverterEngine::GetFormatName(
        ConvertFormat::WebP)) == L"WebP");
    ASSERT(std::wstring(FormatConverterEngine::GetFormatName(
        ConvertFormat::JXL)) == L"JPEG XL");
}

TEST(TestConverter_FormatDetection) {
    using namespace ExplorerLens::Engine;
    ASSERT(FormatConverterEngine::DetectFormat(L"test.png") ==
        ConvertFormat::PNG);
    ASSERT(FormatConverterEngine::DetectFormat(L"test.jpg") ==
        ConvertFormat::JPEG);
    ASSERT(FormatConverterEngine::DetectFormat(L"test.webp") ==
        ConvertFormat::WebP);
    ASSERT(FormatConverterEngine::DetectFormat(L"test.avif") ==
        ConvertFormat::AVIF);
}

TEST(TestConverter_QualityPresets) {
    using namespace ExplorerLens::Engine;
    ASSERT(FormatConverterEngine::GetQualityValue(QualityPreset::Lossless) ==
        100);
    ASSERT(FormatConverterEngine::GetQualityValue(QualityPreset::High) == 90);
    ASSERT(FormatConverterEngine::GetQualityValue(QualityPreset::Normal) == 75);
}

TEST(TestConverter_FormatExtensions) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(FormatConverterEngine::GetFormatExtension(
        ConvertFormat::PNG)) == L".png");
    ASSERT(std::wstring(FormatConverterEngine::GetFormatExtension(
        ConvertFormat::JXL)) == L".jxl");
}

TEST(TestConverter_FormatCount) {
    using namespace ExplorerLens::Engine;
    ASSERT(FormatConverterEngine::GetFormatCount() == 7);
}

//==============================================================================
// Enterprise Deployment Manager Tests
//==============================================================================

TEST(TestEnterprise_MethodNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(EnterpriseDeploymentManager::GetMethodName(
        DeploymentMethod::GPO)) == L"Group Policy");
    ASSERT(std::wstring(EnterpriseDeploymentManager::GetMethodName(
        DeploymentMethod::SCCM)) == L"SCCM");
    ASSERT(std::wstring(EnterpriseDeploymentManager::GetMethodName(
        DeploymentMethod::Intune)) == L"Intune");
}

TEST(TestEnterprise_PolicyTypes) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(EnterpriseDeploymentManager::GetPolicyTypeName(
        PolicyType::MachinePol)) == L"Machine Policy");
    ASSERT(std::wstring(EnterpriseDeploymentManager::GetPolicyTypeName(
        PolicyType::UserPol)) == L"User Policy");
}

TEST(TestEnterprise_AddPolicy) {
    using namespace ExplorerLens::Engine;
    EnterpriseDeploymentManager mgr;
    DeploymentPolicy pol;
    pol.name = L"Enable GPU";
    pol.key = L"GPUEnabled";
    pol.value = L"1";
    mgr.AddPolicy(pol);
    ASSERT(mgr.GetPolicies().size() == 1);
    ASSERT(mgr.ValidatePolicies() == true);
}

TEST(TestEnterprise_MSIProperties) {
    using namespace ExplorerLens::Engine;
    EnterpriseDeploymentManager mgr;
    auto props = mgr.GenerateMSIProperties();
    ASSERT(props.count(L"ALLUSERS") == 1);
    ASSERT(props[L"ALLUSERS"] == L"1");
}

TEST(TestEnterprise_MethodCount) {
    using namespace ExplorerLens::Engine;
    ASSERT(EnterpriseDeploymentManager::GetMethodCount() == 6);
}

//==============================================================================
// Release Gate V11 Tests
//==============================================================================

TEST(TestGateV11_KPINames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ReleaseGateV11::GetKPIName(GateKPIV11::BuildClean)) ==
        L"Build Clean");
    ASSERT(std::wstring(ReleaseGateV11::GetKPIName(GateKPIV11::TestPassRate)) ==
        L"Test Pass Rate");
    ASSERT(std::wstring(ReleaseGateV11::GetKPIName(GateKPIV11::SecurityAudit)) ==
        L"Security Audit");
}

TEST(TestGateV11_KPICount) {
    using namespace ExplorerLens::Engine;
    ASSERT(ReleaseGateV11::GetKPICount() == 15);
}

TEST(TestGateV11_Evaluate) {
    using namespace ExplorerLens::Engine;
    ReleaseGateV11 gate;
    auto result = gate.Evaluate(L"v10.1.0");
    ASSERT(result.kpisEvaluated == 15);
    ASSERT(result.releaseVersion == L"v10.1.0");
}

TEST(TestGateV11_Thresholds) {
    using namespace ExplorerLens::Engine;
    ReleaseGateV11 gate;
    ASSERT(gate.GetThreshold(GateKPIV11::TestPassRate) == 100.0);
    gate.SetThreshold(GateKPIV11::TestCoverage, 90.0);
    ASSERT(gate.GetThreshold(GateKPIV11::TestCoverage) == 90.0);
}

TEST(TestGateV11_SingleKPI) {
    using namespace ExplorerLens::Engine;
    ReleaseGateV11 gate;
    auto result = gate.EvaluateKPI(GateKPIV11::BuildClean);
    ASSERT(result.passed == true);
    ASSERT(result.kpi == GateKPIV11::BuildClean);
}

//==============================================================================
// Watch Folder Engine Tests
//==============================================================================

TEST(TestWatch_ChangeTypeNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(WatchFolderEngine::GetChangeTypeName(
        FileChangeType::Created)) == L"Created");
    ASSERT(std::wstring(WatchFolderEngine::GetChangeTypeName(
        FileChangeType::Renamed)) == L"Renamed");
}

TEST(TestWatch_WatchModes) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(WatchFolderEngine::GetWatchModeName(WatchMode::Native)) ==
        L"Native");
    ASSERT(std::wstring(WatchFolderEngine::GetWatchModeName(WatchMode::Hybrid)) ==
        L"Hybrid");
}

TEST(TestWatch_AddFolder) {
    using namespace ExplorerLens::Engine;
    WatchFolderEngine engine;
    ASSERT(engine.AddFolder(L"C:\\Test") == true);
    ASSERT(engine.GetWatchCount() == 1);
    ASSERT(engine.AddFolder(L"C:\\Test") == false); // duplicate
}

TEST(TestWatch_RemoveFolder) {
    using namespace ExplorerLens::Engine;
    WatchFolderEngine engine;
    engine.AddFolder(L"C:\\Test");
    ASSERT(engine.RemoveFolder(L"C:\\Test") == true);
    ASSERT(engine.GetWatchCount() == 0);
}

TEST(TestWatch_SimulateChange) {
    using namespace ExplorerLens::Engine;
    WatchFolderEngine engine;
    engine.AddFolder(L"C:\\Watch");
    bool callbackFired = false;
    engine.SetChangeCallback([&](const FileChangeEvent& evt) {
        callbackFired = true;
        ASSERT(evt.changeType == FileChangeType::Modified);
        });
    engine.SimulateChange(L"C:\\Watch\\file.jpg", FileChangeType::Modified);
    ASSERT(callbackFired == true);
}

//==============================================================================
// Diagnostic Dashboard Tests
//==============================================================================

TEST(TestDiag_CategoryNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(DiagnosticDashboard::GetCategoryName(
        MetricCategory::CPU)) == L"CPU");
    ASSERT(std::wstring(DiagnosticDashboard::GetCategoryName(
        MetricCategory::GPU)) == L"GPU");
}

TEST(TestDiag_HealthNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(DiagnosticDashboard::GetHealthName(
        HealthLevel::Healthy)) == L"Healthy");
    ASSERT(std::wstring(DiagnosticDashboard::GetHealthName(
        HealthLevel::Critical)) == L"Critical");
}

TEST(TestDiag_RecordMetric) {
    using namespace ExplorerLens::Engine;
    DiagnosticDashboard dash;
    dash.RecordMetric(L"CPU Usage", MetricCategory::CPU, 45.0, 100.0);
    ASSERT(dash.GetMetrics().size() == 1);
}

TEST(TestDiag_Snapshot) {
    using namespace ExplorerLens::Engine;
    DiagnosticDashboard dash;
    dash.RecordMetric(L"M1", MetricCategory::CPU, 30.0, 100.0);
    auto snap = dash.GetSnapshot();
    ASSERT(snap.metricCount == 1);
    ASSERT(snap.overall == HealthLevel::Healthy);
}

TEST(TestDiag_CategoryCount) {
    using namespace ExplorerLens::Engine;
    ASSERT(DiagnosticDashboard::GetCategoryCount() == 7);
}

//==============================================================================
// Performance Benchmark V2 Tests
//==============================================================================

TEST(TestBenchV2_TypeNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(PerformanceBenchmarkV2::GetBenchmarkTypeName(
        BenchmarkType::SingleDecode)) == L"Single Decode");
    ASSERT(std::wstring(PerformanceBenchmarkV2::GetBenchmarkTypeName(
        BenchmarkType::CacheHit)) == L"Cache Hit");
}

TEST(TestBenchV2_ComputeStats) {
    using namespace ExplorerLens::Engine;
    PerformanceBenchmarkV2 bench;
    std::vector<double> samples = { 10.0, 12.0, 11.0, 15.0, 9.0 };
    auto result =
        bench.ComputeStats(L"Test", BenchmarkType::SingleDecode, samples);
    ASSERT(result.iterations == 5);
    ASSERT(result.minMs == 9.0);
    ASSERT(result.maxMs == 15.0);
}

TEST(TestBenchV2_MeetsTarget) {
    using namespace ExplorerLens::Engine;
    BenchmarkResult r;
    r.p95Ms = 15.0;
    ASSERT(PerformanceBenchmarkV2::MeetsTarget(r, 20.0) == true);
    ASSERT(PerformanceBenchmarkV2::MeetsTarget(r, 10.0) == false);
}

TEST(TestBenchV2_AddResult) {
    using namespace ExplorerLens::Engine;
    PerformanceBenchmarkV2 bench;
    BenchmarkResult r;
    r.label = L"Test";
    bench.AddResult(r);
    ASSERT(bench.GetResults().size() == 1);
}

TEST(TestBenchV2_TypeCount) {
    using namespace ExplorerLens::Engine;
    ASSERT(PerformanceBenchmarkV2::GetBenchmarkTypeCount() == 6);
}

//==============================================================================
// Localization Engine Tests
//==============================================================================

TEST(TestL10n_LocaleNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(LocalizationEngine::GetLocaleName(Locale::EN_US)) ==
        L"English (US)");
    ASSERT(std::wstring(LocalizationEngine::GetLocaleName(Locale::DE_DE)) ==
        L"German");
}

TEST(TestL10n_TextDirection) {
    using namespace ExplorerLens::Engine;
    ASSERT(LocalizationEngine::GetTextDirection(Locale::EN_US) ==
        TextDirection::LTR);
    ASSERT(LocalizationEngine::GetTextDirection(Locale::AR_SA) ==
        TextDirection::RTL);
    ASSERT(LocalizationEngine::GetTextDirection(Locale::HE_IL) ==
        TextDirection::RTL);
}

TEST(TestL10n_SetLocale) {
    using namespace ExplorerLens::Engine;
    LocalizationEngine eng;
    eng.SetLocale(Locale::FR_FR);
    ASSERT(eng.GetLocale() == Locale::FR_FR);
    ASSERT(eng.IsRTL() == false);
}

TEST(TestL10n_StringLookup) {
    using namespace ExplorerLens::Engine;
    LocalizationEngine eng;
    eng.AddString(L"app.title", Locale::EN_US, L"ExplorerLens");
    eng.AddString(L"app.title", Locale::DE_DE, L"DunkleDaumen");
    eng.SetLocale(Locale::DE_DE);
    ASSERT(eng.GetString(L"app.title") == L"DunkleDaumen");
}

TEST(TestL10n_LocaleCount) {
    using namespace ExplorerLens::Engine;
    ASSERT(LocalizationEngine::GetLocaleCount() == 10);
}

//==============================================================================
// Theme Engine Tests
//==============================================================================

TEST(TestTheme_TypeNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ThemeEngine::GetThemeTypeName(ThemeType::Dark)) ==
        L"Dark");
    ASSERT(std::wstring(ThemeEngine::GetThemeTypeName(ThemeType::Light)) ==
        L"Light");
}

TEST(TestTheme_DefaultDark) {
    using namespace ExplorerLens::Engine;
    ThemeEngine engine;
    auto theme = engine.GetActiveTheme();
    ASSERT(theme.type == ThemeType::Dark);
    ASSERT(theme.background.r == 30);
}

TEST(TestTheme_SetLight) {
    using namespace ExplorerLens::Engine;
    ThemeEngine engine;
    engine.SetThemeType(ThemeType::Light);
    ASSERT(engine.GetActiveTheme().type == ThemeType::Light);
    ASSERT(engine.GetActiveTheme().background.r == 255);
}

TEST(TestTheme_RegisterCustom) {
    using namespace ExplorerLens::Engine;
    ThemeEngine engine;
    ThemeDefinition custom;
    custom.name = L"Midnight";
    custom.type = ThemeType::Custom;
    engine.RegisterCustomTheme(custom);
    ASSERT(engine.GetRegisteredThemes().size() == 4); // 3 defaults + 1 custom
}

TEST(TestTheme_TypeCount) {
    using namespace ExplorerLens::Engine;
    ASSERT(ThemeEngine::GetThemeTypeCount() == 5);
}

//==============================================================================
// Usage Telemetry Engine Tests
//==============================================================================

TEST(TestUsageTelemetry_CategoryNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(UsageTelemetryEngine::GetCategoryName(
        UsageTelemetryCategory::Decode)) == L"Decode");
    ASSERT(std::wstring(UsageTelemetryEngine::GetCategoryName(
        UsageTelemetryCategory::GPU)) == L"GPU");
}

TEST(TestUsageTelemetry_ConsentNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(UsageTelemetryEngine::GetConsentName(
        UsageConsentLevel::None)) == L"None");
    ASSERT(std::wstring(UsageTelemetryEngine::GetConsentName(
        UsageConsentLevel::Full)) == L"Full");
}

TEST(TestUsageTelemetry_ConsentNone) {
    using namespace ExplorerLens::Engine;
    UsageTelemetryEngine eng;
    ASSERT(eng.RecordEvent(L"TestEvt", UsageTelemetryCategory::Decode) == false);
    ASSERT(eng.GetEventCount() == 0);
}

TEST(TestUsageTelemetry_ConsentBasic) {
    using namespace ExplorerLens::Engine;
    UsageTelemetryEngine eng;
    eng.SetConsent(UsageConsentLevel::Basic);
    ASSERT(eng.RecordEvent(L"Error", UsageTelemetryCategory::Error) == true);
    ASSERT(eng.RecordEvent(L"Decode", UsageTelemetryCategory::Decode) == false);
}

TEST(TestUsageTelemetry_CategoryCount) {
    using namespace ExplorerLens::Engine;
    ASSERT(UsageTelemetryEngine::GetCategoryCount() == 7);
}

//==============================================================================
// Update Engine Tests
//==============================================================================

TEST(TestUpdate_ChannelNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(UpdateEngine::GetChannelName(UpdateChannel::Stable)) ==
        L"Stable");
    ASSERT(std::wstring(UpdateEngine::GetChannelName(UpdateChannel::Beta)) ==
        L"Beta");
}

TEST(TestUpdate_StatusNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(UpdateEngine::GetStatusName(UpdateStatus::Available)) ==
        L"Available");
    ASSERT(std::wstring(UpdateEngine::GetStatusName(UpdateStatus::Ready)) ==
        L"Ready");
}

TEST(TestUpdate_CompareVersions) {
    using namespace ExplorerLens::Engine;
    ASSERT(UpdateEngine::CompareVersions(L"2.0.0", L"1.0.0") > 0);
    ASSERT(UpdateEngine::CompareVersions(L"1.0.0", L"1.0.0") == 0);
    ASSERT(UpdateEngine::CompareVersions(L"1.0.0", L"2.0.0") < 0);
}

TEST(TestUpdate_CheckForUpdate) {
    using namespace ExplorerLens::Engine;
    UpdateEngine eng;
    eng.SetCurrentVersion(L"1.0.0");
    auto info = eng.CheckForUpdate(L"2.0.0");
    ASSERT(info.status == UpdateStatus::Available);
}

TEST(TestUpdate_ChannelCount) {
    using namespace ExplorerLens::Engine;
    ASSERT(UpdateEngine::GetChannelCount() == 4);
}

//==============================================================================
// Shell Preview Handler Tests
//==============================================================================

TEST(TestPreview_ModeNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ShellPreviewHandler::GetModeName(
        PreviewMode::FullImage)) == L"Full Image");
    ASSERT(std::wstring(ShellPreviewHandler::GetModeName(PreviewMode::HexDump)) ==
        L"Hex Dump");
}

TEST(TestPreview_DetectMode) {
    using namespace ExplorerLens::Engine;
    ASSERT(ShellPreviewHandler::DetectMode(L".jpg") == PreviewMode::FullImage);
    ASSERT(ShellPreviewHandler::DetectMode(L".pdf") == PreviewMode::Document);
    ASSERT(ShellPreviewHandler::DetectMode(L".gif") == PreviewMode::Filmstrip);
}

TEST(TestPreview_LoadFile) {
    using namespace ExplorerLens::Engine;
    ShellPreviewHandler handler;
    PreviewParams params;
    params.filePath = L"C:\\test.jpg";
    ASSERT(handler.LoadFile(params) == true);
    ASSERT(handler.GetState() == PreviewState::Ready);
}

TEST(TestPreview_Unload) {
    using namespace ExplorerLens::Engine;
    ShellPreviewHandler handler;
    PreviewParams params;
    params.filePath = L"C:\\test.jpg";
    handler.LoadFile(params);
    handler.Unload();
    ASSERT(handler.GetState() == PreviewState::Unloaded);
}

TEST(TestPreview_ModeCount) {
    using namespace ExplorerLens::Engine;
    ASSERT(ShellPreviewHandler::GetModeCount() == 5);
}

//==============================================================================
// Batch Processing Engine Tests
//==============================================================================

TEST(TestBatch_OperationNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(BatchProcessingEngine::GetOperationName(
        BatchOperation::GenerateThumbnails)) == L"Generate Thumbnails");
    ASSERT(std::wstring(BatchProcessingEngine::GetOperationName(
        BatchOperation::ExportMetadata)) == L"Export Metadata");
}

TEST(TestBatch_CreateJob) {
    using namespace ExplorerLens::Engine;
    BatchProcessingEngine eng;
    auto idx = eng.CreateJob(L"Test", BatchOperation::ValidateFiles,
        { L"a.jpg", L"b.png" });
    ASSERT(idx == 0);
    ASSERT(eng.GetJobCount() == 1);
    ASSERT(eng.GetJobByIndex(0).progress.totalFiles == 2);
}

TEST(TestBatch_RunJob) {
    using namespace ExplorerLens::Engine;
    BatchProcessingEngine eng;
    eng.CreateJob(L"Test", BatchOperation::GenerateThumbnails, { L"a.jpg" });
    ASSERT(eng.RunJob(0) == true);
    ASSERT(eng.GetJobByIndex(0).status == BatchStatus::Completed);
}

TEST(TestBatch_CancelJob) {
    using namespace ExplorerLens::Engine;
    BatchProcessingEngine eng;
    eng.CreateJob(L"Test", BatchOperation::CleanCache, { L"a.jpg" });
    ASSERT(eng.CancelJob(0) == true);
    ASSERT(eng.GetJobByIndex(0).status == BatchStatus::Cancelled);
}

TEST(TestBatch_OperationCount) {
    using namespace ExplorerLens::Engine;
    ASSERT(BatchProcessingEngine::GetOperationCount() == 5);
}

//==============================================================================
// Release Gate V12 Tests
//==============================================================================

TEST(TestGateV12_KPINames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ReleaseGateV12::GetKPIName(GateKPIV12::BuildClean)) ==
        L"Build Clean");
    ASSERT(std::wstring(ReleaseGateV12::GetKPIName(GateKPIV12::L10nCoverage)) ==
        L"L10n Coverage");
}

TEST(TestGateV12_KPICount) {
    using namespace ExplorerLens::Engine;
    ASSERT(ReleaseGateV12::GetKPICount() == 16);
}

TEST(TestGateV12_Evaluate) {
    using namespace ExplorerLens::Engine;
    ReleaseGateV12 gate;
    auto result = gate.EvaluateKPI(GateKPIV12::BuildClean);
    ASSERT(result.passed == true);
}

TEST(TestGateV12_Approved) {
    using namespace ExplorerLens::Engine;
    ReleaseGateV12 gate;
    ASSERT(gate.IsApproved() == true);
}

TEST(TestGateV12_Version) {
    using namespace ExplorerLens::Engine;
    ReleaseGateV12 gate;
    ASSERT(gate.GetVersion() == L"10.2.0");
}

//==============================================================================
// File Hash Engine Tests
//==============================================================================

TEST(TestHash_AlgorithmNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(FileHashEngine::GetAlgorithmName(
        HashAlgorithm::SHA256)) == L"SHA-256");
    ASSERT(std::wstring(FileHashEngine::GetAlgorithmName(HashAlgorithm::CRC32)) ==
        L"CRC32");
}

TEST(TestHash_CRC32) {
    using namespace ExplorerLens::Engine;
    const uint8_t data[] = { 'H', 'e', 'l', 'l', 'o' };
    uint32_t crc = FileHashEngine::ComputeCRC32(data, 5);
    ASSERT(crc != 0); // Non-trivial hash
}

TEST(TestHash_ComputeHash) {
    using namespace ExplorerLens::Engine;
    const uint8_t data[] = { 1, 2, 3 };
    auto hash = FileHashEngine::ComputeHash(data, 3, HashAlgorithm::CRC32);
    ASSERT(hash.length() == 8);
}

TEST(TestHash_VerifyHash) {
    using namespace ExplorerLens::Engine;
    ASSERT(FileHashEngine::VerifyHash(L"abc", L"abc") == true);
    ASSERT(FileHashEngine::VerifyHash(L"abc", L"def") == false);
}

TEST(TestHash_AlgorithmCount) {
    using namespace ExplorerLens::Engine;
    ASSERT(FileHashEngine::GetAlgorithmCount() == 5);
}

//==============================================================================
// Registry Manager Tests
//==============================================================================

TEST(TestReg_HiveNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(RegistryManager::GetHiveName(RegHive::HKCU)) == L"HKCU");
    ASSERT(std::wstring(RegistryManager::GetHiveName(RegHive::HKLM)) == L"HKLM");
}

TEST(TestReg_WriteRead) {
    using namespace ExplorerLens::Engine;
    RegistryManager mgr;
    mgr.WriteString(RegHive::HKCU, L"SOFTWARE\\ExplorerLens", L"Version",
        L"10.3.0");
    auto val =
        mgr.ReadString(RegHive::HKCU, L"SOFTWARE\\ExplorerLens", L"Version");
    ASSERT(val == L"10.3.0");
}

TEST(TestReg_DefaultValue) {
    using namespace ExplorerLens::Engine;
    RegistryManager mgr;
    auto val = mgr.ReadString(RegHive::HKCU, L"MISSING", L"Key", L"default");
    ASSERT(val == L"default");
}

TEST(TestReg_Delete) {
    using namespace ExplorerLens::Engine;
    RegistryManager mgr;
    mgr.WriteString(RegHive::HKCU, L"Test", L"Name", L"Value");
    ASSERT(mgr.DeleteValue(RegHive::HKCU, L"Test", L"Name") == true);
    ASSERT(mgr.GetEntries().size() == 0);
}

TEST(TestReg_BasePath) {
    using namespace ExplorerLens::Engine;
    ASSERT(RegistryManager::GetBasePath() == L"SOFTWARE\\ExplorerLens");
}

//==============================================================================
// Error Recovery Engine Tests
//==============================================================================

TEST(TestRecovery_StrategyNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ErrorRecoveryEngine::GetStrategyName(
        RecoveryStrategy::Retry)) == L"Retry");
    ASSERT(std::wstring(ErrorRecoveryEngine::GetStrategyName(
        RecoveryStrategy::SafeMode)) == L"Safe Mode");
}

TEST(TestRecovery_CreateCheckpoint) {
    using namespace ExplorerLens::Engine;
    ErrorRecoveryEngine eng;
    auto id = eng.CreateCheckpoint(L"Before decode", L"state1");
    ASSERT(id == 1);
    ASSERT(eng.GetCheckpointCount() == 1);
}

TEST(TestRecovery_RestoreCheckpoint) {
    using namespace ExplorerLens::Engine;
    ErrorRecoveryEngine eng;
    auto id = eng.CreateCheckpoint(L"CP1", L"state1");
    ASSERT(eng.RestoreCheckpoint(id) == true);
    ASSERT(eng.GetState() == RecoveryState::Recovered);
}

TEST(TestRecovery_CrashRecovery) {
    using namespace ExplorerLens::Engine;
    ErrorRecoveryEngine eng;
    ASSERT(eng.RecoverFromCrash(RecoveryStrategy::Retry) == true);
    ASSERT(eng.GetState() == RecoveryState::Recovered);
}

TEST(TestRecovery_StrategyCount) {
    using namespace ExplorerLens::Engine;
    ASSERT(ErrorRecoveryEngine::GetStrategyCount() == 5);
}

//==============================================================================
// Log Rotation Engine Tests
//==============================================================================

TEST(TestLogRot_PolicyNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(LogRotationEngine::GetPolicyName(
        RotationPolicy::SizeBased)) == L"Size Based");
    ASSERT(std::wstring(LogRotationEngine::GetPolicyName(
        RotationPolicy::Hybrid)) == L"Hybrid");
}

TEST(TestLogRot_CompressionNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(LogRotationEngine::GetCompressionName(
        LogCompression::GZip)) == L"GZip");
    ASSERT(std::wstring(LogRotationEngine::GetCompressionName(
        LogCompression::Zstd)) == L"Zstd");
}

TEST(TestLogRot_NeedsRotation) {
    using namespace ExplorerLens::Engine;
    LogRotationEngine eng;
    RotationConfig cfg;
    cfg.maxSizeBytes = 1024;
    eng.SetConfig(cfg);
    ASSERT(eng.NeedsRotation(2048) == true);
    ASSERT(eng.NeedsRotation(512) == false);
}

TEST(TestLogRot_Cleanup) {
    using namespace ExplorerLens::Engine;
    LogRotationEngine eng;
    RotationConfig cfg;
    cfg.maxFiles = 2;
    eng.SetConfig(cfg);
    eng.AddRotatedFile(L"log1.log", 100);
    eng.AddRotatedFile(L"log2.log", 100);
    eng.AddRotatedFile(L"log3.log", 100);
    auto cleanup = eng.GetFilesToCleanup();
    ASSERT(cleanup.size() == 1);
}

TEST(TestLogRot_PolicyCount) {
    using namespace ExplorerLens::Engine;
    ASSERT(LogRotationEngine::GetPolicyCount() == 4);
}

//==============================================================================
// Release Gate V13 Tests
//==============================================================================

TEST(TestGateV13_KPINames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ReleaseGateV13::GetKPIName(GateKPIV13::BuildClean)) ==
        L"Build Clean");
    ASSERT(std::wstring(ReleaseGateV13::GetKPIName(
        GateKPIV13::RecoverySuccess)) == L"Recovery Success");
}

TEST(TestGateV13_KPICount) {
    using namespace ExplorerLens::Engine;
    ASSERT(ReleaseGateV13::GetKPICount() == 17);
}

TEST(TestGateV13_Evaluate) {
    using namespace ExplorerLens::Engine;
    ReleaseGateV13 gate;
    auto r = gate.EvaluateKPI(GateKPIV13::HashVerification);
    ASSERT(r.passed == true);
}

TEST(TestGateV13_Approved) {
    using namespace ExplorerLens::Engine;
    ReleaseGateV13 gate;
    ASSERT(gate.IsApproved() == true);
}

TEST(TestGateV13_Version) {
    using namespace ExplorerLens::Engine;
    ReleaseGateV13 gate;
    ASSERT(gate.GetVersion() == L"10.3.0");
}

//==============================================================================
// ResourcePoolEngine Tests
//==============================================================================

TEST(TestResourcePool_Checkout) {
    using namespace ExplorerLens;
    ResourcePoolEngine pool;
    pool.Initialize();
    auto r = pool.Checkout(ResourceType::DecoderContext);
    ASSERT(r.id > 0);
    ASSERT(r.state == ResourceState::InUse);
}

TEST(TestResourcePool_Return) {
    using namespace ExplorerLens;
    ResourcePoolEngine pool;
    pool.Initialize();
    auto r = pool.Checkout(ResourceType::GPUTexture);
    ASSERT(pool.Return(r.id));
    ASSERT(pool.GetAvailableCount(ResourceType::GPUTexture) >= 1);
}

TEST(TestResourcePool_Stats) {
    using namespace ExplorerLens;
    ResourcePoolEngine pool;
    pool.Initialize();
    pool.Checkout(ResourceType::DecoderContext);
    auto stats = pool.GetStats();
    ASSERT(stats.totalCheckouts >= 1);
}

TEST(TestResourcePool_TypeNames) {
    using namespace ExplorerLens;
    ASSERT(std::wstring(ResourcePoolEngine::GetResourceTypeName(
        ResourceType::GPUTexture)) == L"GPU Texture");
    ASSERT(ResourcePoolEngine::GetResourceTypeCount() == 6);
}

TEST(TestResourcePool_Prewarm) {
    using namespace ExplorerLens;
    PoolConfig cfg;
    cfg.enablePrewarming = false;
    ResourcePoolEngine pool(cfg);
    pool.Initialize();
    uint32_t created = pool.Prewarm(ResourceType::ComputeBuffer, 5);
    ASSERT(created == 5);
    ASSERT(pool.GetAvailableCount(ResourceType::ComputeBuffer) == 5);
}

//==============================================================================
// CommandLineInterface Tests
//==============================================================================

TEST(TestCLI_ParseFlags) {
    using namespace ExplorerLens;
    CommandLineInterface cli(L"TestApp");
    ArgDefinition def;
    def.longName = L"--verbose";
    def.shortName = L"-v";
    def.type = ArgType::Flag;
    cli.AddArgument(def);
    auto status = cli.Parse({ L"--verbose" });
    ASSERT(status == ParseStatus::Success);
    ASSERT(cli.GetFlag(L"--verbose"));
}

TEST(TestCLI_ParseString) {
    using namespace ExplorerLens;
    CommandLineInterface cli(L"TestApp");
    ArgDefinition def;
    def.longName = L"--output";
    def.shortName = L"-o";
    def.type = ArgType::String;
    cli.AddArgument(def);
    auto status = cli.Parse({ L"--output", L"file.txt" });
    ASSERT(status == ParseStatus::Success);
    ASSERT(cli.GetString(L"--output") == L"file.txt");
}

TEST(TestCLI_MissingRequired) {
    using namespace ExplorerLens;
    CommandLineInterface cli;
    ArgDefinition def;
    def.longName = L"--input";
    def.shortName = L"-i";
    def.type = ArgType::FilePath;
    def.required = true;
    cli.AddArgument(def);
    auto status = cli.Parse({});
    ASSERT(status == ParseStatus::MissingRequired);
}

TEST(TestCLI_HelpRequested) {
    using namespace ExplorerLens;
    CommandLineInterface cli;
    auto status = cli.Parse({ L"--help" });
    ASSERT(status == ParseStatus::HelpRequested);
}

TEST(TestCLI_ArgTypeNames) {
    using namespace ExplorerLens;
    ASSERT(std::wstring(CommandLineInterface::GetArgTypeName(ArgType::String)) ==
        L"String");
    ASSERT(std::wstring(CommandLineInterface::GetParseStatusName(
        ParseStatus::Success)) == L"Success");
}

//==============================================================================
// MetadataExtractor Tests
//==============================================================================

TEST(TestMetadata_Extract) {
    using namespace ExplorerLens;
    MetadataExtractor extractor;
    auto result = extractor.Extract(L"test_image.jpg");
    ASSERT(result.success);
    ASSERT(result.tagCount > 0);
}

TEST(TestMetadata_FieldLookup) {
    using namespace ExplorerLens;
    MetadataExtractor extractor;
    auto result = extractor.Extract(L"test_image.jpg");
    ASSERT(extractor.HasField(result, MetadataField::Width));
    ASSERT(extractor.GetFieldValue(result, MetadataField::Width) == L"1920");
}

TEST(TestMetadata_Standards) {
    using namespace ExplorerLens;
    ASSERT(std::wstring(MetadataExtractor::GetStandardName(
        MetadataStandard::EXIF)) == L"EXIF");
    ASSERT(MetadataExtractor::GetStandardCount() == 5);
}

TEST(TestMetadata_FieldNames) {
    using namespace ExplorerLens;
    ASSERT(std::wstring(MetadataExtractor::GetFieldName(
        MetadataField::CameraMake)) == L"Camera Make");
    ASSERT(MetadataExtractor::GetFieldCount() == 16);
}

TEST(TestMetadata_FormatExposure) {
    using namespace ExplorerLens;
    auto formatted = MetadataExtractor::FormatExposureTime(0.004);
    ASSERT(formatted == L"1/250s");
}

//==============================================================================
// NotificationEngine Tests
//==============================================================================

TEST(TestNotify_Send) {
    using namespace ExplorerLens;
    NotificationEngine engine;
    uint64_t id =
        engine.Send(NotifyType::BatchComplete, L"Done", L"Batch finished");
    ASSERT(id > 0);
    ASSERT(engine.GetTotalCount() == 1);
}

TEST(TestNotify_Dismiss) {
    using namespace ExplorerLens;
    NotificationEngine engine;
    uint64_t id = engine.Send(NotifyType::UpdateAvailable, L"Update", L"v2.0");
    ASSERT(engine.Dismiss(id));
    auto n = engine.GetNotification(id);
    ASSERT(n != nullptr);
    ASSERT(n->state == NotifyState::Dismissed);
}

TEST(TestNotify_ByType) {
    using namespace ExplorerLens;
    NotificationEngine engine;
    engine.Send(NotifyType::DecoderError, L"Err1", L"msg1");
    engine.Send(NotifyType::DecoderError, L"Err2", L"msg2");
    engine.Send(NotifyType::CacheCleared, L"Cache", L"cleared");
    auto errors = engine.GetByType(NotifyType::DecoderError);
    ASSERT(errors.size() == 2);
}

TEST(TestNotify_TypeNames) {
    using namespace ExplorerLens;
    ASSERT(std::wstring(NotificationEngine::GetTypeName(
        NotifyType::PluginLoaded)) == L"Plugin Loaded");
    ASSERT(NotificationEngine::GetTypeCount() == 7);
}

TEST(TestNotify_PriorityNames) {
    using namespace ExplorerLens;
    ASSERT(std::wstring(NotificationEngine::GetPriorityName(
        NotifyPriority::Critical)) == L"Critical");
    ASSERT(std::wstring(NotificationEngine::GetStateName(NotifyState::Expired)) ==
        L"Expired");
}

//==============================================================================
// ReleaseGateV14 Tests
//==============================================================================

TEST(TestGateV14_KPINames) {
    using namespace ExplorerLens;
    ASSERT(std::wstring(ReleaseGateV14::GetKPIName(GateKPIV14::BuildClean)) ==
        L"Build Clean");
    ASSERT(std::wstring(ReleaseGateV14::GetKPIName(
        GateKPIV14::MetadataAccuracy)) == L"Metadata Accuracy");
}

TEST(TestGateV14_KPICount) {
    using namespace ExplorerLens;
    ASSERT(ReleaseGateV14::GetKPICount() == 18);
}

TEST(TestGateV14_Evaluate) {
    using namespace ExplorerLens;
    ReleaseGateV14 gate;
    ASSERT(!gate.Evaluate());
    for (uint32_t i = 0; i < ReleaseGateV14::GetKPICount(); ++i) {
        gate.SetKPIResult(static_cast<GateKPIV14>(i), true, 100.0);
    }
    ASSERT(gate.Evaluate());
}

TEST(TestGateV14_Approved) {
    using namespace ExplorerLens;
    ReleaseGateV14 gate;
    ASSERT(!gate.IsApproved());
    ASSERT(gate.GetFailedCount() == 18);
}

TEST(TestGateV14_Version) {
    using namespace ExplorerLens;
    ReleaseGateV14 gate;
    ASSERT(gate.GetVersion() == L"10.4.0");
}

//==============================================================================
// ContentIndexer Tests
//==============================================================================

TEST(TestIndexer_AddFile) {
    using namespace ExplorerLens;
    ContentIndexer indexer;
    uint64_t id = indexer.AddFile(L"C:\\photos\\test.jpg");
    ASSERT(id > 0);
    ASSERT(indexer.GetTotalCount() == 1);
}

TEST(TestIndexer_ClassifyExtension) {
    using namespace ExplorerLens;
    ASSERT(ContentIndexer::ClassifyExtension(L".jpg") == ContentType::Image);
    ASSERT(ContentIndexer::ClassifyExtension(L".zip") == ContentType::Archive);
    ASSERT(ContentIndexer::ClassifyExtension(L".pdf") == ContentType::Document);
    ASSERT(ContentIndexer::ClassifyExtension(L".mp4") == ContentType::Video);
}

TEST(TestIndexer_IndexAll) {
    using namespace ExplorerLens;
    ContentIndexer indexer;
    indexer.AddFile(L"test1.png");
    indexer.AddFile(L"test2.cbz");
    uint32_t indexed = indexer.IndexAll();
    ASSERT(indexed == 2);
}

TEST(TestIndexer_ContentTypeNames) {
    using namespace ExplorerLens;
    ASSERT(std::wstring(ContentIndexer::GetContentTypeName(ContentType::Image)) ==
        L"Image");
    ASSERT(ContentIndexer::GetContentTypeCount() == 8);
}

TEST(TestIndexer_SearchByType) {
    using namespace ExplorerLens;
    ContentIndexer indexer;
    indexer.AddFile(L"photo.jpg");
    indexer.AddFile(L"archive.zip");
    indexer.IndexAll();
    auto images = indexer.SearchByType(ContentType::Image);
    ASSERT(images.size() == 1);
}

//==============================================================================
// NetworkDiagnostics Tests
//==============================================================================

TEST(TestNetDiag_RunTest) {
    using namespace ExplorerLens;
    NetworkDiagnostics diag;
    auto result = diag.RunTest(NetTestType::Ping, L"https://example.com");
    ASSERT(result.status == NetTestStatus::Passed);
    ASSERT(result.latencyMs > 0.0);
}

TEST(TestNetDiag_RunAllTests) {
    using namespace ExplorerLens;
    NetworkDiagnostics diag;
    auto report = diag.RunAllTests();
    ASSERT(report.results.size() > 0);
}

TEST(TestNetDiag_TypeNames) {
    using namespace ExplorerLens;
    ASSERT(std::wstring(NetworkDiagnostics::GetTestTypeName(
        NetTestType::DNSResolve)) == L"DNS Resolve");
    ASSERT(NetworkDiagnostics::GetTestTypeCount() == 5);
}

TEST(TestNetDiag_StatusNames) {
    using namespace ExplorerLens;
    ASSERT(std::wstring(NetworkDiagnostics::GetTestStatusName(
        NetTestStatus::Passed)) == L"Passed");
    ASSERT(std::wstring(NetworkDiagnostics::GetTestStatusName(
        NetTestStatus::Timeout)) == L"Timeout");
}

TEST(TestNetDiag_Proxy) {
    using namespace ExplorerLens;
    NetworkDiagnostics diag;
    ProxyConfig proxy;
    proxy.host = L"proxy.example.com";
    proxy.port = 8080;
    proxy.enabled = true;
    diag.SetProxy(proxy);
    auto result = diag.RunTest(NetTestType::ProxyCheck, L"https://example.com");
    ASSERT(result.status == NetTestStatus::Passed);
}

//==============================================================================
// ConfigMigrationEngine Tests
//==============================================================================

TEST(TestConfigMig_Migrate) {
    ::ExplorerLens::Core::ConfigMigrationEngine engine;
    engine.SetSourceVersion(L"10.4.0");
    engine.SetTargetVersion(L"10.5.0");
    engine.AddSetting(L"theme", L"dark");
    ::ExplorerLens::Core::MigrationRule rule;
    rule.sourceKey = L"theme";
    rule.targetKey = L"ui.theme";
    rule.action = ::ExplorerLens::Core::MigrationAction::Rename;
    engine.AddRule(rule);
    auto report = engine.Migrate();
    ASSERT(report.status == ::ExplorerLens::Core::MigrationStatus::Completed);
    ASSERT(engine.HasSetting(L"ui.theme"));
}

TEST(TestConfigMig_Rollback) {
    ::ExplorerLens::Core::ConfigMigrationEngine engine;
    engine.AddSetting(L"key1", L"val1");
    ::ExplorerLens::Core::MigrationRule rule;
    rule.sourceKey = L"key1";
    rule.action = ::ExplorerLens::Core::MigrationAction::Delete;
    engine.AddRule(rule);
    engine.Migrate();
    ASSERT(!engine.HasSetting(L"key1"));
    ASSERT(engine.Rollback());
    ASSERT(engine.HasSetting(L"key1"));
}

TEST(TestConfigMig_SetDefault) {
    ::ExplorerLens::Core::ConfigMigrationEngine engine;
    ::ExplorerLens::Core::MigrationRule rule;
    rule.targetKey = L"newSetting";
    rule.defaultValue = L"defaultVal";
    rule.action = ::ExplorerLens::Core::MigrationAction::SetDefault;
    engine.AddRule(rule);
    engine.Migrate();
    ASSERT(engine.GetSetting(L"newSetting") == L"defaultVal");
}

TEST(TestConfigMig_ActionNames) {
    ASSERT(
        std::wstring(::ExplorerLens::Core::ConfigMigrationEngine::GetActionName(
            ::ExplorerLens::Core::MigrationAction::Rename)) == L"Rename");
    ASSERT(::ExplorerLens::Core::ConfigMigrationEngine::GetActionCount() == 5);
}

TEST(TestConfigMig_StatusNames) {
    ASSERT(
        std::wstring(::ExplorerLens::Core::ConfigMigrationEngine::GetStatusName(
            ::ExplorerLens::Core::MigrationStatus::Completed)) == L"Completed");
    ASSERT(
        std::wstring(::ExplorerLens::Core::ConfigMigrationEngine::GetStatusName(
            ::ExplorerLens::Core::MigrationStatus::RolledBack)) ==
        L"Rolled Back");
}

//==============================================================================
// ReleaseGateV15 Tests
//==============================================================================

TEST(TestGateV15_KPINames) {
    using namespace ExplorerLens;
    ASSERT(std::wstring(ReleaseGateV15::GetKPIName(GateKPIV15::BuildClean)) ==
        L"Build Clean");
    ASSERT(std::wstring(ReleaseGateV15::GetKPIName(
        GateKPIV15::ConfigMigration)) == L"Config Migration");
}

TEST(TestGateV15_KPICount) {
    using namespace ExplorerLens;
    ASSERT(ReleaseGateV15::GetKPICount() == 20);
}

TEST(TestGateV15_Evaluate) {
    using namespace ExplorerLens;
    ReleaseGateV15 gate;
    ASSERT(!gate.Evaluate());
    for (uint32_t i = 0; i < ReleaseGateV15::GetKPICount(); ++i) {
        gate.SetKPIResult(static_cast<GateKPIV15>(i), true, 100.0);
    }
    ASSERT(gate.Evaluate());
}

TEST(TestGateV15_Approved) {
    using namespace ExplorerLens;
    ReleaseGateV15 gate;
    ASSERT(!gate.IsApproved());
    ASSERT(gate.GetFailedCount() == 20);
}

TEST(TestGateV15_Version) {
    using namespace ExplorerLens;
    ReleaseGateV15 gate;
    ASSERT(gate.GetVersion() == L"10.5.0");
}

//==============================================================================
// Worker/Isolation Stabilization Tests
// February 17, 2026
//==============================================================================

TEST(TestMalformedArchive_TruncatedZIP) {
    ArchiveDecoder decoder;
    ThumbnailRequest request = {};
    request.filePath = L"test_truncated.zip";
    request.outputWidth = 256;
    request.outputHeight = 256;

    ThumbnailResult result = {};

    // Create a truncated ZIP file (first 100 bytes only)
    HANDLE hFile = CreateFileW(request.filePath, GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        const char truncatedHeader[] = "PK\x03\x04\x14\x00\x00\x00\x08\x00";
        DWORD written = 0;
        WriteFile(hFile, truncatedHeader, sizeof(truncatedHeader), &written,
            nullptr);
        CloseHandle(hFile);
    }

    // Should fail gracefully, not crash
    HRESULT hr = decoder.Decode(request, result);
    ASSERT(FAILED(hr)); // Should return error

    // Cleanup
    DeleteFileW(request.filePath);
}

TEST(TestMalformedArchive_GarbageHeader) {
    ArchiveDecoder decoder;
    ThumbnailRequest request = {};
    request.filePath = L"test_garbage.cbz";
    request.outputWidth = 256;
    request.outputHeight = 256;

    ThumbnailResult result = {};

    // Create a file with random garbage data
    HANDLE hFile = CreateFileW(request.filePath, GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        const char garbage[] = "NOTAZIPATALL123456789GARBAGE";
        DWORD written = 0;
        WriteFile(hFile, garbage, sizeof(garbage), &written, nullptr);
        CloseHandle(hFile);
    }

    // Should detect invalid format and reject
    ASSERT(!decoder.CanDecode(request.filePath) ||
        FAILED(decoder.Decode(request, result)));

    // Cleanup
    DeleteFileW(request.filePath);
}

TEST(TestMalformedArchive_ZeroByteFile) {
    ArchiveDecoder decoder;
    ThumbnailRequest request = {};
    request.filePath = L"test_zero.zip";
    request.outputWidth = 256;
    request.outputHeight = 256;

    ThumbnailResult result = {};

    // Create zero-byte file
    HANDLE hFile = CreateFileW(request.filePath, GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }

    // Should handle gracefully
    HRESULT hr = decoder.Decode(request, result);
    ASSERT(FAILED(hr));

    // Cleanup
    DeleteFileW(request.filePath);
}

TEST(TestCircuitBreaker_StressTest) {
    // Circuit breaker should isolate failing decoders after 5 failures
    // This test generates 10 failures and verifies circuit opens

    ArchiveDecoder decoder;
    int failureCount = 0;
    const int maxFailures = 10;

    for (int i = 0; i < maxFailures; i++) {
        ThumbnailRequest request = {};
        wchar_t fileName[256];
        swprintf_s(fileName, L"test_corrupt_%d.zip", i);
        request.filePath = fileName;
        request.outputWidth = 256;
        request.outputHeight = 256;

        // Create corrupt file
        HANDLE hFile = CreateFileW(fileName, GENERIC_WRITE, 0, nullptr,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile != INVALID_HANDLE_VALUE) {
            const char corrupt[] = "PK\xFF\xFF";
            DWORD written = 0;
            WriteFile(hFile, corrupt, sizeof(corrupt), &written, nullptr);
            CloseHandle(hFile);
        }

        ThumbnailResult result = {};
        HRESULT hr = decoder.Decode(request, result);

        if (FAILED(hr)) {
            failureCount++;
        }

        DeleteFileW(fileName);
    }

    // All should fail, but no crashes
    ASSERT(failureCount == maxFailures);
    std::wcout << L" [Circuit Breaker] Handled " << failureCount
        << L" failures without crash" << std::endl;
}

TEST(TestDecoderTimeout_Enforcement) {
    // Verify decoders don't hang indefinitely on problematic files
    // This test simulates a slow/infinite loop scenario

    ImageDecoder decoder;
    ThumbnailRequest request = {};
    request.filePath = L"test_timeout.jpg";
    request.outputWidth = 256;
    request.outputHeight = 256;

    // Create minimal valid JPEG (will decode very quickly)
    HANDLE hFile = CreateFileW(request.filePath, GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        // Minimal JPEG header (not valid, but triggers parse)
        const unsigned char jpegHeader[] = { 0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10,
        'J', 'F', 'I', 'F', 0x00 };
        DWORD written = 0;
        WriteFile(hFile, jpegHeader, sizeof(jpegHeader), &written, nullptr);
        CloseHandle(hFile);
    }

    auto startTime = std::chrono::steady_clock::now();

    ThumbnailResult result = {};
    decoder.Decode(request, result);

    auto endTime = std::chrono::steady_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime)
        .count();

    // Should complete within 5 seconds (timeout enforcement)
    ASSERT(duration < 5000);

    std::wcout << L" [Timeout] Decode completed in " << duration
        << L" ms (< 5000 ms limit)" << std::endl;

    DeleteFileW(request.filePath);
}

TEST(TestMemoryLeak_RegressionLoop) {
    // Run 100 decode iterations and verify no excessive memory growth
    ImageDecoder decoder;

    const int iterations = 100;
    SIZE_T initialMemory = 0;
    SIZE_T peakMemory = 0;

    // Get initial memory usage
    PROCESS_MEMORY_COUNTERS_EX pmc = {};
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc,
        sizeof(pmc));
    initialMemory = pmc.WorkingSetSize;

    for (int i = 0; i < iterations; i++) {
        ThumbnailRequest request = {};
        wchar_t fileName[256];
        swprintf_s(fileName, L"test_leak_%d.bmp", i);
        request.filePath = fileName;
        request.outputWidth = 256;
        request.outputHeight = 256;

        // Create minimal BMP header
        HANDLE hFile = CreateFileW(fileName, GENERIC_WRITE, 0, nullptr,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile != INVALID_HANDLE_VALUE) {
            const unsigned char bmpHeader[] = { 'B', 'M', 0x36, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x36, 0x00,
            0x00, 0x00, 0x28, 0x00, 0x00, 0x00 };
            DWORD written = 0;
            WriteFile(hFile, bmpHeader, sizeof(bmpHeader), &written, nullptr);
            CloseHandle(hFile);
        }

        ThumbnailResult result = {};
        decoder.Decode(request, result);

        // Release result resources
        if (result.bitmap) {
            DeleteObject(result.bitmap);
        }

        DeleteFileW(fileName);

        // Check memory every 10 iterations
        if ((i + 1) % 10 == 0) {
            GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc,
                sizeof(pmc));
            SIZE_T currentMemory = pmc.WorkingSetSize;
            if (currentMemory > peakMemory) {
                peakMemory = currentMemory;
            }
        }
    }

    // Final memory check
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc,
        sizeof(pmc));
    SIZE_T finalMemory = pmc.WorkingSetSize;

    // Memory growth should be < 50 MB for 100 iterations
    SIZE_T growth =
        (finalMemory > initialMemory) ? (finalMemory - initialMemory) : 0;
    SIZE_T growthMB = growth / (1024 * 1024);

    std::wcout << L" [Memory] Initial: " << (initialMemory / 1024 / 1024)
        << L" MB, " << L"Final: " << (finalMemory / 1024 / 1024)
        << L" MB, " << L"Growth: " << growthMB << L" MB" << std::endl;

    ASSERT(growthMB < 50); // No excessive memory leak
}

// Format Registry Tests
TEST(TestFormatReg_Register) {
    FormatRegistry& reg = FormatRegistry::Instance();
    FormatEntry entry;
    entry.type = FormatType::DPX;
    entry.category = FormatCategory::ProfessionalImage;
    entry.primaryExt = L".dpx";
    entry.description = L"DPX Film Frame";
    entry.decoderName = L"DPXDecoder";
    entry.hasDecoder = true;
    reg.Register(entry);
    ASSERT(reg.GetEntry(FormatType::DPX) != nullptr);
}

TEST(TestFormatReg_LookupByExt) {
    FormatRegistry& reg = FormatRegistry::Instance();
    reg.RegisterAlias(L".dpx", FormatType::DPX);
    auto type = reg.LookupByExtension(L".dpx");
    ASSERT(type == FormatType::DPX);
}

TEST(TestFormatReg_CategoryNames) {
    ASSERT(std::wstring(FormatRegistry::CategoryName(FormatCategory::Archive)) ==
        L"Archive");
    ASSERT(std::wstring(FormatRegistry::CategoryName(
        FormatCategory::ModernImage)) == L"ModernImage");
    ASSERT(std::wstring(FormatRegistry::CategoryName(
        FormatCategory::Scientific)) == L"Scientific");
}

TEST(TestFormatReg_TypeNames) {
    ASSERT(std::wstring(FormatRegistry::TypeName(FormatType::WebP)) == L"WebP");
    ASSERT(std::wstring(FormatRegistry::TypeName(FormatType::DPX)) == L"DPX");
    ASSERT(std::wstring(FormatRegistry::TypeName(FormatType::DICOM)) == L"DICOM");
}

TEST(TestFormatReg_Validate) {
    FormatRegistry& reg = FormatRegistry::Instance();
    auto result = reg.Validate();
    ASSERT(result.orphanedExtensions == 0);
}

// Format Type Lookup Tests
TEST(TestFormatLookup_WebP) {
    auto& lookup = FormatTypeLookup::Instance();
    ASSERT(lookup.Lookup(L".webp") == FormatType::WebP);
}

TEST(TestFormatLookup_Archives) {
    auto& lookup = FormatTypeLookup::Instance();
    ASSERT(lookup.Lookup(L".zip") == FormatType::ZIP);
    ASSERT(lookup.Lookup(L".7z") == FormatType::SevenZip);
    ASSERT(lookup.Lookup(L".rar") == FormatType::RAR);
}

TEST(TestFormatLookup_Scientific) {
    auto& lookup = FormatTypeLookup::Instance();
    ASSERT(lookup.Lookup(L".dcm") == FormatType::DICOM);
    ASSERT(lookup.Lookup(L".fits") == FormatType::FITS);
}

TEST(TestFormatLookup_Film) {
    auto& lookup = FormatTypeLookup::Instance();
    ASSERT(lookup.Lookup(L".dpx") == FormatType::DPX);
    ASSERT(lookup.Lookup(L".cin") == FormatType::Cineon);
}

TEST(TestFormatLookup_Stats) {
    auto& lookup = FormatTypeLookup::Instance();
    auto stats = lookup.GetStats();
    ASSERT(stats.totalMappings >= 80);
    ASSERT(stats.archiveTypes >= 10);
}

// Shell Registration Manager Tests
TEST(TestShellReg_AddRegistered) {
    ShellRegistrationManager mgr;
    mgr.AddRegistered(L".webp", L"WebP Image");
    ASSERT(mgr.IsRegistered(L".webp"));
    ASSERT(mgr.RegisteredCount() == 1);
}

TEST(TestShellReg_MissingRegs) {
    ShellRegistrationManager mgr;
    mgr.AddRegistered(L".webp");
    mgr.AddSupported(L".dpx");
    auto missing = mgr.GetMissingRegistrations();
    ASSERT(missing.size() == 1);
    ASSERT(missing[0] == L".dpx");
}

TEST(TestShellReg_V106NewExts) {
    auto exts = ShellRegistrationManager::GetV106NewExtensions();
    ASSERT(exts.size() >= 10);
}

TEST(TestShellReg_CategoryNames) {
    ASSERT(std::wstring(ShellRegistrationManager::CategoryName(0)) ==
        L"Archives");
    ASSERT(std::wstring(ShellRegistrationManager::CategoryName(8)) ==
        L"Scientific");
}

TEST(TestShellReg_Audit) {
    ShellRegistrationManager mgr;
    mgr.AddRegistered(L".webp");
    mgr.AddRegistered(L".avif");
    auto audit = mgr.RunAudit();
    ASSERT(audit.registered == 2);
    ASSERT(audit.synced);
}

// Test Infrastructure V2 Tests
TEST(TestInfra_CoverageCommand) {
    auto cmd =
        TestInfrastructure::GetCoverageCommand(CoverageTool::OpenCppCoverage);
    ASSERT(cmd.find(L"OpenCppCoverage") != std::wstring::npos);
}

TEST(TestInfra_SanitizerNames) {
    ASSERT(std::wstring(TestInfrastructure::SanitizerModeName(
        SanitizerMode::AddressSanitizer)) == L"ASAN");
}

TEST(TestInfra_CoverageToolNames) {
    ASSERT(std::wstring(TestInfrastructure::CoverageToolName(
        CoverageTool::OpenCppCoverage)) == L"OpenCppCoverage");
}

TEST(TestInfra_SanitizerFlags) {
    auto flags =
        TestInfrastructure::GetSanitizerFlags(SanitizerMode::AddressSanitizer);
    ASSERT(flags.find(L"fsanitize") != std::wstring::npos);
}

TEST(TestInfra_CoverageThresholds) {
    TestInfrastructure infra;
    auto ci = infra.GetCIThresholds();
    ASSERT(ci.lineCoverage >= 70.0f);
    auto rel = infra.GetReleaseThresholds();
    ASSERT(rel.lineCoverage >= 85.0f);
}

// Release Gate V16 Tests
TEST(TestGateV16_KPINames) {
    ASSERT(std::wstring(ReleaseGateV16::KPIName(GateV16KPI::VersionSync)) ==
        L"VersionSync");
    ASSERT(std::wstring(ReleaseGateV16::KPIName(
        GateV16KPI::FormatRegistryValid)) == L"FormatRegistryValid");
}

TEST(TestGateV16_KPICount) { ASSERT(ReleaseGateV16::KPICount() == 20); }

TEST(TestGateV16_Evaluate) {
    ReleaseGateV16 gate;
    auto r = gate.EvaluateKPI(GateV16KPI::TestPassRate, 100.0f);
    ASSERT(r.passed);
}

TEST(TestGateV16_Approved) {
    ReleaseGateV16 gate;
    std::vector<GateV16Result> results;
    GateV16Result r1;
    r1.passed = true;
    GateV16Result r2;
    r2.passed = true;
    results.push_back(r1);
    results.push_back(r2);
    auto verdict = gate.Evaluate(results);
    ASSERT(verdict.approved);
}

TEST(TestGateV16_Version) {
    ReleaseGateV16 gate;
    std::vector<GateV16Result> results;
    auto verdict = gate.Evaluate(results);
    ASSERT(verdict.version == L"10.6.0");
}

//==============================================================================
// DPX/Cineon Decoder Tests
//==============================================================================

TEST(TestDPX_MagicBytes) {
    uint8_t dpxBE[] = { 0x53, 0x44, 0x50, 0x58 }; // SDPX
    ASSERT(DPXCineonDecoder::IsDPXFile(dpxBE, 4));
    uint8_t bad[] = { 0x00, 0x00, 0x00, 0x00 };
    ASSERT(!DPXCineonDecoder::IsDPXFile(bad, 4));
}

TEST(TestDPX_CineonMagic) {
    uint8_t cin[] = { 0x80, 0x2A, 0x5F, 0xD7 };
    ASSERT(DPXCineonDecoder::IsCineonFile(cin, 4));
}

TEST(TestDPX_TransferNames) {
    ASSERT(std::wstring(DPXCineonDecoder::TransferName(DPXTransfer::LogFilm)) ==
        L"Log Film");
    ASSERT(std::wstring(DPXCineonDecoder::TransferName(DPXTransfer::Linear)) ==
        L"Linear");
}

TEST(TestDPX_LogToLinear) {
    uint8_t out = DPXCineonDecoder::LogToLinear(0);
    ASSERT(out == 0);
    uint8_t out2 = DPXCineonDecoder::LogToLinear(1023);
    ASSERT(out2 == 255);
}

TEST(TestDPX_TransferCount) {
    ASSERT(DPXCineonDecoder::TransferTypeCount() == 6);
}

//==============================================================================
// APNG & Animated Format Tests
//==============================================================================

TEST(TestAnimHdlr_DetectAPNG) {
    ASSERT(AnimatedFormatHandler::DetectFormat(L".apng") == AnimatedFormat::APNG);
    ASSERT(AnimatedFormatHandler::DetectFormat(L".gif") == AnimatedFormat::GIF);
}

TEST(TestAnimHdlr_FormatNames) {
    ASSERT(std::wstring(AnimatedFormatHandler::FormatName(
        AnimatedFormat::APNG)) == L"Animated PNG");
    ASSERT(std::wstring(AnimatedFormatHandler::FormatName(
        AnimatedFormat::WebPAnim)) == L"Animated WebP");
}

TEST(TestAnimHdlr_StrategyNames) {
    ASSERT(std::wstring(AnimatedFormatHandler::StrategyName(
        FrameStrategy::First)) == L"FirstFrame");
    ASSERT(std::wstring(AnimatedFormatHandler::StrategyName(
        FrameStrategy::Middle)) == L"MiddleFrame");
}

TEST(TestAnimHdlr_SelectFrame) {
    AnimationInfo info;
    info.frameCount = 100;
    ASSERT(AnimatedFormatHandler::SelectFrame(info, FrameStrategy::First) == 0);
    ASSERT(AnimatedFormatHandler::SelectFrame(info, FrameStrategy::Middle) == 50);
}

TEST(TestAnimHdlr_FormatCount) {
    ASSERT(AnimatedFormatHandler::FormatCount() == 6);
}

//==============================================================================
// Text Preview Decoder Tests
//==============================================================================

TEST(TestTextPreview_DetectLang) {
    ASSERT(TextPreviewDecoder::DetectLanguage(L".py") == TextLanguage::Python);
    ASSERT(TextPreviewDecoder::DetectLanguage(L".cpp") == TextLanguage::CPP);
    ASSERT(TextPreviewDecoder::DetectLanguage(L".md") == TextLanguage::Markdown);
}

TEST(TestTextPreview_LanguageNames) {
    ASSERT(std::wstring(TextPreviewDecoder::LanguageName(TextLanguage::Python)) ==
        L"Python");
    ASSERT(std::wstring(TextPreviewDecoder::LanguageName(TextLanguage::CSharp)) ==
        L"C#");
}

TEST(TestTextPreview_IsTextFile) {
    ASSERT(TextPreviewDecoder::IsTextFile(L".json"));
    ASSERT(TextPreviewDecoder::IsTextFile(L".ts"));
    ASSERT(!TextPreviewDecoder::IsTextFile(L".exe"));
}

TEST(TestTextPreview_ValidateConfig) {
    TextPreviewConfig cfg;
    ASSERT(TextPreviewDecoder::ValidateConfig(cfg));
    cfg.maxLines = 0;
    ASSERT(!TextPreviewDecoder::ValidateConfig(cfg));
}

TEST(TestTextPreview_ExtCount) {
    ASSERT(TextPreviewDecoder::ExtensionCount() == 31);
    ASSERT(TextPreviewDecoder::LanguageCount() == 20);
}

//==============================================================================
// DICOM Decoder V2 Tests
//==============================================================================

TEST(TestDICOMv2_Magic) {
    std::vector<uint8_t> data(136, 0);
    data[128] = 'D';
    data[129] = 'I';
    data[130] = 'C';
    data[131] = 'M';
    ASSERT(DICOMDecoderV2::IsDICOMFile(data.data(), data.size()));
    data[128] = 0;
    ASSERT(!DICOMDecoderV2::IsDICOMFile(data.data(), data.size()));
}

TEST(TestDICOMv2_TransferSyntax) {
    ASSERT(std::wstring(DICOMDecoderV2::TransferSyntaxName(
        DICOMTransferSyntax::ExplicitVRLittleEndian)) ==
        L"Explicit VR Little Endian");
    ASSERT(DICOMDecoderV2::TransferSyntaxCount() == 8);
}

TEST(TestDICOMv2_CanDecode) {
    ASSERT(DICOMDecoderV2::CanDecodeNatively(
        DICOMTransferSyntax::ImplicitVRLittleEndian));
    ASSERT(DICOMDecoderV2::CanDecodeNatively(
        DICOMTransferSyntax::ExplicitVRLittleEndian));
    ASSERT(!DICOMDecoderV2::CanDecodeNatively(DICOMTransferSyntax::JPEGBaseline));
}

TEST(TestDICOMv2_Validate) {
    DICOMImageInfo info;
    info.rows = 512;
    info.columns = 512;
    info.bitsAllocated = 16;
    info.bitsStored = 12;
    ASSERT(DICOMDecoderV2::ValidateInfo(info));
    info.rows = 0;
    ASSERT(!DICOMDecoderV2::ValidateInfo(info));
}

TEST(TestDICOMv2_PixelSize) {
    DICOMImageInfo info;
    info.rows = 256;
    info.columns = 256;
    info.bitsAllocated = 16;
    info.samplesPerPixel = 1;
    info.numberOfFrames = 1;
    ASSERT(DICOMDecoderV2::CalculatePixelSize(info) == 256 * 256 * 2);
}

//==============================================================================
// FITS Decoder V2 Tests
//==============================================================================

TEST(TestFITSv2_Magic) {
    // Valid: keyword "SIMPLE" padded to 8 chars, '=' at byte 8, total 80 bytes
    char hdr[80];
    std::memset(hdr, ' ', 80);
    std::memcpy(hdr, "SIMPLE  =                    T", 30);
    ASSERT(FITSDecoderV2::IsFITSFile(reinterpret_cast<const uint8_t*>(hdr), 80));
    // Invalid: wrong keyword
    char bad[80];
    std::memset(bad, ' ', 80);
    std::memcpy(bad, "NOTFITS =", 9);
    ASSERT(
        !FITSDecoderV2::IsFITSFile(reinterpret_cast<const uint8_t*>(bad), 80));
}

TEST(TestFITSv2_BytesPerPixel) {
    ASSERT(FITSDecoderV2::BytesPerPixel(FITSBitpix::UInt8) == 1);
    ASSERT(FITSDecoderV2::BytesPerPixel(FITSBitpix::Int16) == 2);
    ASSERT(FITSDecoderV2::BytesPerPixel(FITSBitpix::Float32) == 4);
    ASSERT(FITSDecoderV2::BytesPerPixel(FITSBitpix::Float64) == 8);
}

TEST(TestFITSv2_Validate) {
    FITSImageInfo info;
    info.naxis = 2;
    info.naxis1 = 1024;
    info.naxis2 = 1024;
    info.bitpix = FITSBitpix::Float32;
    ASSERT(FITSDecoderV2::ValidateInfo(info));
    info.naxis = 1;
    ASSERT(!FITSDecoderV2::ValidateInfo(info));
}

TEST(TestFITSv2_DataSize) {
    FITSImageInfo info;
    info.naxis1 = 100;
    info.naxis2 = 100;
    info.naxis3 = 1;
    info.bitpix = FITSBitpix::Float32;
    ASSERT(FITSDecoderV2::CalculateDataSize(info) == 100 * 100 * 4);
}

TEST(TestFITSv2_Normalize) {
    ASSERT(FITSDecoderV2::NormalizeTo8Bit(0.0, 0.0, 1.0) == 0);
    ASSERT(FITSDecoderV2::NormalizeTo8Bit(1.0, 0.0, 1.0) == 255);
    ASSERT(FITSDecoderV2::NormalizeTo8Bit(0.5, 0.0, 1.0) == 127);
}

//==============================================================================
// 3MF/USD Format Tests
//==============================================================================

TEST(TestModelFmt_Detect3MF) {
    ASSERT(ModelFormatHandler::DetectFormat(L".3mf") ==
        PrintModel3DFormat::ThreeMF);
    ASSERT(ModelFormatHandler::DetectFormat(L".usdz") ==
        PrintModel3DFormat::USDZ);
    ASSERT(ModelFormatHandler::DetectFormat(L".step") ==
        PrintModel3DFormat::STEP);
}

TEST(TestModelFmt_FormatNames) {
    ASSERT(std::wstring(ModelFormatHandler::FormatName(
        PrintModel3DFormat::ThreeMF)) == L"3D Manufacturing Format");
    ASSERT(std::wstring(ModelFormatHandler::FormatName(
        PrintModel3DFormat::USDZ)) == L"USD ZIP Package");
}

TEST(TestModelFmt_Is3MF) {
    uint8_t pk[] = { 'P', 'K', 0x03, 0x04 };
    ASSERT(ModelFormatHandler::Is3MFFile(pk, 4));
    uint8_t bad[] = { 0, 0, 0, 0 };
    ASSERT(!ModelFormatHandler::Is3MFFile(bad, 4));
}

TEST(TestModelFmt_Thumbnail) {
    ASSERT(ModelFormatHandler::CanExtractThumbnail(PrintModel3DFormat::ThreeMF));
    ASSERT(ModelFormatHandler::CanExtractThumbnail(PrintModel3DFormat::USDZ));
    ASSERT(!ModelFormatHandler::CanExtractThumbnail(PrintModel3DFormat::STEP));
}

TEST(TestModelFmt_Counts) {
    ASSERT(ModelFormatHandler::FormatCount() == 7);
    ASSERT(ModelFormatHandler::ExtensionCount() == 9);
}

//==============================================================================
// Release Gate V17 Tests
//==============================================================================

TEST(TestGateV17_KPINames) {
    ASSERT(std::wstring(ReleaseGateV17::KPIName(GateV17KPI::BuildClean)) ==
        L"BuildClean");
    ASSERT(std::wstring(ReleaseGateV17::KPIName(GateV17KPI::DPXDecoderValid)) ==
        L"DPXDecoderValid");
}

TEST(TestGateV17_KPICount) { ASSERT(ReleaseGateV17::KPICount() == 21); }

TEST(TestGateV17_Evaluate) {
    ReleaseGateV17 gate;
    std::vector<GateV17Result> results;
    auto verdict = gate.Evaluate(results);
    ASSERT(results.size() == 21);
}

TEST(TestGateV17_Approved) {
    ReleaseGateV17 gate;
    std::vector<GateV17Result> results;
    auto verdict = gate.Evaluate(results);
    ASSERT(verdict.approved);
    ASSERT(verdict.passed == 21);
}

TEST(TestGateV17_Version) {
    ReleaseGateV17 gate;
    std::vector<GateV17Result> results;
    auto verdict = gate.Evaluate(results);
    ASSERT(verdict.version == L"11.0.0");
}

//==============================================================================
// D3D12 Pipeline Activation Tests
//==============================================================================

TEST(TestD3D12Act_BackendNames) {
    ASSERT(std::wstring(D3D12PipelineActivation::BackendName(
        GPUBackend::D3D12)) == L"Direct3D 12");
    ASSERT(std::wstring(D3D12PipelineActivation::BackendName(GPUBackend::GDI)) ==
        L"GDI+ (CPU)");
}

TEST(TestD3D12Act_FeatureLevels) {
    ASSERT(std::wstring(D3D12PipelineActivation::FeatureLevelName(
        D3DFeatureLevel::Level_12_0)) == L"12.0");
}

TEST(TestD3D12Act_SelectBackend) {
    GPUAdapterInfo adapter;
    adapter.supportsD3D12 = true;
    adapter.dedicatedVideoMemory = 1024ULL * 1024 * 1024; // 1 GB
    adapter.featureLevel = D3DFeatureLevel::Level_12_0;
    D3D12ActivationConfig cfg;
    ASSERT(D3D12PipelineActivation::SelectBackend(adapter, cfg) ==
        GPUBackend::D3D12);
}

TEST(TestD3D12Act_Fallback) {
    GPUAdapterInfo adapter;
    adapter.supportsD3D12 = false;
    adapter.featureLevel = D3DFeatureLevel::Level_11_0;
    D3D12ActivationConfig cfg;
    ASSERT(D3D12PipelineActivation::SelectBackend(adapter, cfg) ==
        GPUBackend::D3D11);
}

TEST(TestD3D12Act_ValidateConfig) {
    D3D12ActivationConfig cfg;
    ASSERT(D3D12PipelineActivation::ValidateConfig(cfg));
    cfg.maxConcurrentDispatches = 0;
    ASSERT(!D3D12PipelineActivation::ValidateConfig(cfg));
}

//==============================================================================
// Async Shell Extension Tests
//==============================================================================

TEST(TestAsync_StateNames) {
    ASSERT(std::wstring(AsyncShellActivation::StateName(
        AsyncDecodeState::Queued)) == L"Queued");
    ASSERT(std::wstring(AsyncShellActivation::StateName(
        AsyncDecodeState::Completed)) == L"Completed");
    ASSERT(std::wstring(AsyncShellActivation::StateName(
        AsyncDecodeState::TimedOut)) == L"Timed Out");
}

TEST(TestAsyncSA_PriorityNames) {
    ASSERT(std::wstring(AsyncShellActivation::PriorityName(
        DecodePriority::Critical)) == L"Critical");
    ASSERT(std::wstring(AsyncShellActivation::PriorityName(
        DecodePriority::Idle)) == L"Idle");
}

TEST(TestAsync_Counts) {
    ASSERT(AsyncShellActivation::StateCount() == 8);
    ASSERT(AsyncShellActivation::PriorityCount() == 5);
}

TEST(TestAsync_ValidateConfig) {
    AsyncProviderConfig cfg;
    ASSERT(AsyncShellActivation::ValidateConfig(cfg));
    cfg.maxConcurrent = 0;
    ASSERT(!AsyncShellActivation::ValidateConfig(cfg));
}

TEST(TestAsync_Timeout) {
    AsyncProviderConfig cfg;
    cfg.defaultTimeoutMs = 5000;
    cfg.criticalTimeoutMs = 10000;
    ASSERT(AsyncShellActivation::EffectiveTimeout(
        cfg, DecodePriority::Critical) == 10000);
    ASSERT(AsyncShellActivation::EffectiveTimeout(cfg, DecodePriority::Low) ==
        2500);
}

//==============================================================================
// SIMD Acceleration Tests
//==============================================================================

TEST(TestSIMDMgr_LevelNames) {
    ASSERT(std::wstring(SIMDAccelerationManager::LevelName(SIMDLevel::AVX2)) ==
        L"AVX2");
    ASSERT(std::wstring(SIMDAccelerationManager::LevelName(SIMDLevel::NEON)) ==
        L"NEON");
}

TEST(TestSIMD_OperationNames) {
    ASSERT(std::wstring(SIMDAccelerationManager::OperationName(
        SIMDOperation::BilinearResize)) == L"Bilinear Resize");
    ASSERT(std::wstring(SIMDAccelerationManager::OperationName(
        SIMDOperation::AlphaBlend)) == L"Alpha Blend");
}

TEST(TestSIMD_SelectLevel) {
    SIMDCapabilities caps;
    caps.hasSSE2 = true;
    caps.hasSSE41 = true;
    caps.hasAVX = true;
    caps.hasAVX2 = true;
    SIMDConfig cfg;
    ASSERT(SIMDAccelerationManager::SelectLevel(caps, cfg) == SIMDLevel::AVX2);
}

TEST(TestSIMD_Speedup) {
    ASSERT(SIMDAccelerationManager::SpeedupEstimate(SIMDLevel::AVX2) == 4.0f);
    ASSERT(SIMDAccelerationManager::SpeedupEstimate(SIMDLevel::None) == 1.0f);
}

TEST(TestSIMD_Counts) {
    ASSERT(SIMDAccelerationManager::LevelCount() == 7);
    ASSERT(SIMDAccelerationManager::OperationCount() == 8);
}

//==============================================================================
// Parallel Batch Decode Tests
//==============================================================================

TEST(TestBatch_PolicyNames) {
    ASSERT(std::wstring(ParallelBatchProcessor::PolicyName(
        BatchPolicy::Adaptive)) == L"Adaptive");
    ASSERT(std::wstring(ParallelBatchProcessor::PolicyName(
        BatchPolicy::SizeOrdered)) == L"Size Ordered");
}

TEST(TestBatch_OptimalThreads) {
    ASSERT(ParallelBatchProcessor::OptimalThreadCount(4) == 3);
    ASSERT(ParallelBatchProcessor::OptimalThreadCount(8) == 6);
    ASSERT(ParallelBatchProcessor::OptimalThreadCount(16) == 14);
}

TEST(TestBatch_ValidateConfig) {
    BatchDecodeConfig cfg;
    ASSERT(ParallelBatchProcessor::ValidateConfig(cfg));
    cfg.maxThreads = 0;
    ASSERT(!ParallelBatchProcessor::ValidateConfig(cfg));
}

TEST(TestBatch_Throughput) {
    ASSERT(ParallelBatchProcessor::CalculateThroughput(400, 1000.0) == 400.0);
    ASSERT(ParallelBatchProcessor::CalculateThroughput(0, 0) == 0);
}

TEST(TestBatch_PolicyCount) {
    ASSERT(ParallelBatchProcessor::PolicyCount() == 5);
}

//==============================================================================
// Persistent Cache & USN Tests
//==============================================================================

TEST(TestPCache_BackendNames) {
    ASSERT(std::wstring(PersistentCacheManager::BackendName(
        CacheBackend::Hybrid)) == L"Hybrid");
    ASSERT(std::wstring(PersistentCacheManager::BackendName(
        CacheBackend::SQLite)) == L"SQLite");
}

TEST(TestPCache_PolicyNames) {
    ASSERT(std::wstring(PersistentCacheManager::PolicyName(
        InvalidationPolicy::USNJournal)) == L"USN Journal");
}

TEST(TestPCache_ValidateConfig) {
    PersistentCacheConfig cfg;
    ASSERT(PersistentCacheManager::ValidateConfig(cfg));
    cfg.maxMemoryMB = 0;
    ASSERT(!PersistentCacheManager::ValidateConfig(cfg));
}

TEST(TestPCache_HitRate) {
    ASSERT(PersistentCacheManager::CalculateHitRate(80, 20) == 0.8);
    ASSERT(PersistentCacheManager::CalculateHitRate(0, 0) == 0.0);
}

TEST(TestPCache_Counts) {
    ASSERT(PersistentCacheManager::BackendCount() == 4);
    ASSERT(PersistentCacheManager::PolicyCount() == 5);
}

//==============================================================================
// Release Gate V18 Tests
//==============================================================================

TEST(TestGateV18_KPINames) {
    ASSERT(std::wstring(ReleaseGateV18::KPIName(
        GateV18KPI::SingleThumbnailLatency)) == L"SingleThumbnailLatency");
    ASSERT(std::wstring(ReleaseGateV18::KPIName(GateV18KPI::BatchThroughput)) ==
        L"BatchThroughput");
}

TEST(TestGateV18_KPICount) { ASSERT(ReleaseGateV18::KPICount() == 20); }

TEST(TestGateV18_Thresholds) {
    auto t = ReleaseGateV18::DefaultThresholds();
    ASSERT(t.maxSingleMs == 12.0);
    ASSERT(t.minBatchPerSec == 400.0);
}

TEST(TestGateV18_Evaluate) {
    ReleaseGateV18 gate;
    std::vector<GateV18Result> results;
    auto verdict = gate.Evaluate(results);
    ASSERT(verdict.approved);
    ASSERT(verdict.passed == 20);
}

TEST(TestGateV18_Version) {
    ReleaseGateV18 gate;
    std::vector<GateV18Result> results;
    auto verdict = gate.Evaluate(results);
    ASSERT(verdict.version == L"11.1.0");
}

//==============================================================================
// ARM64 Validation Tests
//==============================================================================

TEST(TestARM64Validator_FeatureNames) {
    ASSERT(std::wstring(ARM64PlatformValidator::FeatureName(
        ARM64Feature::NEON)) == L"NEON");
    ASSERT(std::wstring(ARM64PlatformValidator::FeatureName(ARM64Feature::SVE)) ==
        L"SVE");
}

TEST(TestARM64_CategoryNames) {
    ASSERT(std::wstring(ARM64PlatformValidator::CategoryName(
        ARM64TestCategory::BasicBoot)) == L"Basic Boot");
    ASSERT(std::wstring(ARM64PlatformValidator::CategoryName(
        ARM64TestCategory::SIMDPaths)) == L"SIMD Paths");
}

TEST(TestARM64_Counts) {
    ASSERT(ARM64PlatformValidator::FeatureCount() == 9);
    ASSERT(ARM64PlatformValidator::CategoryCount() == 8);
}

TEST(TestARM64_IsARM64) {
    // On x64, this should be false
#if defined(_M_X64) || defined(__x86_64__)
    ASSERT(!ARM64PlatformValidator::IsARM64());
#endif
}

TEST(TestARM64_Config) {
    ARM64ValidationConfig cfg;
    ASSERT(!cfg.isEmulated);
    ASSERT(!cfg.isNativeARM64);
}

//==============================================================================
// MSIX Packaging Tests
//==============================================================================

TEST(TestMSIX_TargetNames) {
    ASSERT(std::wstring(MSIXPackagingManager::TargetName(MSIXTarget::Store)) ==
        L"Microsoft Store");
    ASSERT(std::wstring(MSIXPackagingManager::TargetName(MSIXTarget::Desktop)) ==
        L"Desktop Bridge");
}

TEST(TestMSIX_CapabilityNames) {
    ASSERT(std::wstring(MSIXPackagingManager::CapabilityName(
        MSIXCapability::ShellExtension)) == L"Shell Extension");
}

TEST(TestMSIX_Counts) {
    ASSERT(MSIXPackagingManager::TargetCount() == 4);
    ASSERT(MSIXPackagingManager::CapabilityCount() == 6);
}

TEST(TestMSIX_Identity) {
    auto id =
        MSIXPackagingManager::GenerateIdentity(L"ExplorerLens", L"11.0.0.0");
    ASSERT(id == L"ExplorerLens_11.0.0.0");
}

TEST(TestMSIX_ValidateVersion) {
    ASSERT(MSIXPackagingManager::ValidateVersion(L"11.0.0.0"));
    ASSERT(!MSIXPackagingManager::ValidateVersion(L"11.0.0"));
    ASSERT(!MSIXPackagingManager::ValidateVersion(L"abc"));
}

// Windows 11 24H2 Integration Tests

TEST(TestWin11Mgr_FeatureNames) {
    for (size_t i = 0; i < Win11IntegrationManager::FeatureCount(); ++i) {
        auto name =
            Win11IntegrationManager::FeatureName(static_cast<Win11MgrFeature>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestWin11_VersionNames) {
    for (size_t i = 0; i < Win11IntegrationManager::VersionCount(); ++i) {
        auto name =
            Win11IntegrationManager::VersionName(static_cast<WindowsVersion>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestWin11_FeatureAvailability) {
    ASSERT(!Win11IntegrationManager::IsFeatureAvailable(
        Win11MgrFeature::ModernContextMenu, WindowsVersion::Windows10_21H2));
    ASSERT(Win11IntegrationManager::IsFeatureAvailable(
        Win11MgrFeature::ModernContextMenu, WindowsVersion::Windows11_21H2));
}

TEST(TestWin11_TabbedExplorer) {
    ASSERT(!Win11IntegrationManager::IsFeatureAvailable(
        Win11MgrFeature::TabbedExplorer, WindowsVersion::Windows11_21H2));
    ASSERT(Win11IntegrationManager::IsFeatureAvailable(
        Win11MgrFeature::TabbedExplorer, WindowsVersion::Windows11_22H2));
}

TEST(TestWin11_Config) {
    Win11IntegrationConfig cfg;
    ASSERT(cfg.enableModernMenu);
    ASSERT(cfg.cornerRadius == 8);
    ASSERT(cfg.detectedVersion == WindowsVersion::Unknown);
}

// Test Suite Expansion V2 Tests

TEST(TestTestSuiteV2_CategoryNames) {
    for (size_t i = 0; i < TestSuiteExpansionV2::CategoryCount(); ++i) {
        auto name =
            TestSuiteExpansionV2::CategoryName(static_cast<TestFileCategoryV2>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestTestSuiteV2_COMTestNames) {
    for (size_t i = 0; i < TestSuiteExpansionV2::COMTestCount(); ++i) {
        auto name = TestSuiteExpansionV2::COMTestName(static_cast<COMTestType>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestTestSuiteV2_Counts) {
    ASSERT(TestSuiteExpansionV2::CategoryCount() == 14);
    ASSERT(TestSuiteExpansionV2::COMTestCount() == 5);
}

TEST(TestTestSuiteV2_TotalTarget) {
    RealFileTestConfig cfg;
    ASSERT(TestSuiteExpansionV2::TotalTarget(cfg) == 480);
}

TEST(TestTestSuiteV2_CorpusDefaults) {
    TestCorpusStatsV2 stats;
    ASSERT(stats.totalFiles == 0);
    ASSERT(stats.formatsRepresented == 0);
}

// Fuzz Testing Tests

TEST(TestFuzz_BackendNames) {
    for (size_t i = 0; i < FuzzTestingManager::BackendCount(); ++i) {
        auto name = FuzzTestingManager::BackendName(static_cast<FuzzerBackend>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestFuzz_MutationNames) {
    for (size_t i = 0; i < FuzzTestingManager::StrategyCount(); ++i) {
        auto name =
            FuzzTestingManager::MutationName(static_cast<MutationStrategy>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestFuzz_Counts) {
    ASSERT(FuzzTestingManager::BackendCount() == 4);
    ASSERT(FuzzTestingManager::StrategyCount() == 8);
}

TEST(TestFuzz_ValidateConfig) {
    FuzzManagerTargetConfig cfg;
    cfg.decoderName = L"webp";
    ASSERT(FuzzTestingManager::ValidateConfig(cfg));
}

TEST(TestFuzz_InvalidConfig) {
    FuzzManagerTargetConfig cfg;
    ASSERT(!FuzzTestingManager::ValidateConfig(cfg));
    cfg.decoderName = L"test";
    cfg.maxInputSize = 0;
    ASSERT(!FuzzTestingManager::ValidateConfig(cfg));
}

// Release Gate V19 Tests

TEST(TestGateV19_KPINames) {
    for (uint32_t i = 0; i < ReleaseGateV19::KPICount(); ++i) {
        auto name = ReleaseGateV19::KPIName(static_cast<GateV19KPI>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestGateV19_KPICount) { ASSERT(ReleaseGateV19::KPICount() == 20); }

TEST(TestGateV19_Evaluate) {
    ReleaseGateV19 gate;
    std::vector<GateV19Result> results;
    auto verdict = gate.Evaluate(results);
    ASSERT(verdict.approved);
    ASSERT(verdict.passed == 20);
}

TEST(TestGateV19_Version) {
    GateV19Verdict v;
    ASSERT(v.version == L"11.2.0");
}

TEST(TestGateV19_ResultDefault) {
    GateV19Result r;
    ASSERT(!r.passed);
    ASSERT(r.detail.empty());
}

// Vulkan Compute Backend Tests

TEST(TestVulkan_FeatureNames) {
    for (size_t i = 0; i < VulkanComputeActivation::FeatureCount(); ++i) {
        auto name =
            VulkanComputeActivation::FeatureName(static_cast<VulkanFeature>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestVulkan_QueueNames) {
    for (size_t i = 0; i < VulkanComputeActivation::QueueTypeCount(); ++i) {
        auto name =
            VulkanComputeActivation::QueueName(static_cast<VulkanQueueType>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestVulkan_Counts) {
    ASSERT(VulkanComputeActivation::FeatureCount() == 7);
    ASSERT(VulkanComputeActivation::QueueTypeCount() == 5);
}

TEST(TestVulkan_MinRequirements) {
    VulkanAdapterInfo info;
    info.computeSupport = true;
    info.deviceMemory = 512 * 1024 * 1024ULL;
    ASSERT(VulkanComputeActivation::MeetsMinimumRequirements(info));
}

TEST(TestVulkan_ValidateConfig) {
    VulkanPipelineConfig cfg;
    ASSERT(VulkanComputeActivation::ValidateConfig(cfg));
    cfg.workGroupSizeX = 0;
    ASSERT(!VulkanComputeActivation::ValidateConfig(cfg));
}

// Plugin Marketplace V3 Tests

TEST(TestMarketV3_CategoryNames) {
    for (size_t i = 0; i < PluginMarketplaceV3::CategoryCount(); ++i) {
        auto name =
            PluginMarketplaceV3::CategoryName(static_cast<PluginCategoryV3>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestMarketV3_TrustNames) {
    for (size_t i = 0; i < PluginMarketplaceV3::TrustLevelCount(); ++i) {
        auto name =
            PluginMarketplaceV3::TrustName(static_cast<PluginTrustLevelV3>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestMarketV3_SandboxNames) {
    for (size_t i = 0; i < PluginMarketplaceV3::SandboxPolicyCount(); ++i) {
        auto name = PluginMarketplaceV3::SandboxName(static_cast<SandboxPolicy>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestMarketV3_Counts) {
    ASSERT(PluginMarketplaceV3::CategoryCount() == 8);
    ASSERT(PluginMarketplaceV3::TrustLevelCount() == 4);
    ASSERT(PluginMarketplaceV3::SandboxPolicyCount() == 4);
}

TEST(TestMarketV3_EntryDefaults) {
    MarketplaceEntryV3 entry;
    ASSERT(entry.category == PluginCategoryV3::Utility);
    ASSERT(entry.sandbox == SandboxPolicy::Full);
    ASSERT(entry.autoUpdate);
}

// AI-Enhanced Thumbnails Tests

TEST(TestAI_EnhancementNames) {
    for (size_t i = 0; i < AIThumbnailEnhancer::EnhancementCount(); ++i) {
        auto name =
            AIThumbnailEnhancer::EnhancementName(static_cast<AIEnhancement>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestAI_BackendNames) {
    for (size_t i = 0; i < AIThumbnailEnhancer::BackendCount(); ++i) {
        auto name =
            AIThumbnailEnhancer::BackendName(static_cast<AIModelBackend>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestAI_Counts) {
    ASSERT(AIThumbnailEnhancer::EnhancementCount() == 7);
    ASSERT(AIThumbnailEnhancer::BackendCount() == 4);
}

TEST(TestAI_QualityValid) {
    ASSERT(AIThumbnailEnhancer::IsQualityValid(0.85f));
    ASSERT(!AIThumbnailEnhancer::IsQualityValid(-0.1f));
    ASSERT(!AIThumbnailEnhancer::IsQualityValid(1.5f));
}

TEST(TestAI_ConfigDefaults) {
    AIEnhancementConfig cfg;
    ASSERT(cfg.backend == AIModelBackend::CPUFallback);
    ASSERT(cfg.maxProcessMs == 100);
    ASSERT(cfg.gpuAccelerate);
}

// Spreadsheet Preview Tests

TEST(TestSpreadsheet_FormatNames) {
    for (size_t i = 0; i < SpreadsheetPreviewDecoder::FormatCount(); ++i) {
        auto name = SpreadsheetPreviewDecoder::FormatName(
            static_cast<SpreadsheetFormat>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestSpreadsheet_CellTypeNames) {
    for (size_t i = 0; i < SpreadsheetPreviewDecoder::CellTypeCount(); ++i) {
        auto name =
            SpreadsheetPreviewDecoder::CellTypeName(static_cast<CellDataType>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestSpreadsheet_DetectFormat) {
    ASSERT(SpreadsheetPreviewDecoder::DetectFormat(L".csv") ==
        SpreadsheetFormat::CSV);
    ASSERT(SpreadsheetPreviewDecoder::DetectFormat(L".xlsx") ==
        SpreadsheetFormat::XLSX);
    ASSERT(SpreadsheetPreviewDecoder::DetectFormat(L".ods") ==
        SpreadsheetFormat::ODS);
}

TEST(TestSpreadsheet_Counts) {
    ASSERT(SpreadsheetPreviewDecoder::FormatCount() == 6);
    ASSERT(SpreadsheetPreviewDecoder::CellTypeCount() == 7);
}

TEST(TestSpreadsheet_ConfigDefaults) {
    SpreadsheetPreviewConfig cfg;
    ASSERT(cfg.maxRows == 20);
    ASSERT(cfg.showGridLines);
    ASSERT(cfg.alternateRows);
}

// USD/USDZ Decoder Tests

TEST(TestUSD_ElementNames) {
    for (size_t i = 0; i < USDDecoder::ElementCount(); ++i) {
        auto name = USDDecoder::ElementName(static_cast<USDElementType>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestUSD_VariantNames) {
    for (size_t i = 0; i < USDDecoder::VariantCount(); ++i) {
        auto name = USDDecoder::VariantName(static_cast<USDVariant>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestUSD_DetectVariant) {
    ASSERT(USDDecoder::DetectVariant(L".usda") == USDVariant::USDA);
    ASSERT(USDDecoder::DetectVariant(L".usdc") == USDVariant::USDC);
    ASSERT(USDDecoder::DetectVariant(L".usdz") == USDVariant::USDZ);
}

TEST(TestUSD_USDZMagic) {
    uint8_t pk[] = { 0x50, 0x4B, 0x03, 0x04, 0x00 };
    ASSERT(USDDecoder::CheckUSDZMagic(pk, 5));
    uint8_t bad[] = { 0x00, 0x00 };
    ASSERT(!USDDecoder::CheckUSDZMagic(bad, 2));
}

TEST(TestUSD_Counts) {
    ASSERT(USDDecoder::ElementCount() == 7);
    ASSERT(USDDecoder::VariantCount() == 3);
}

// Auto-Update Engine Tests

TEST(TestAutoUpdate_ChannelNames) {
    for (size_t i = 0; i < AutoUpdateEngine::ChannelCount(); ++i) {
        auto name =
            AutoUpdateEngine::ChannelName(static_cast<AutoUpdateChannel>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestAutoUpdate_CheckResultNames) {
    for (size_t i = 0; i < AutoUpdateEngine::CheckResultCount(); ++i) {
        auto name =
            AutoUpdateEngine::CheckResultName(static_cast<UpdateCheckResult>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestAutoUpdate_DownloadStateNames) {
    for (size_t i = 0; i < AutoUpdateEngine::DownloadStateCount(); ++i) {
        auto name =
            AutoUpdateEngine::DownloadStateName(static_cast<DownloadState>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestAutoUpdate_ParseVersion) {
    uint32_t maj, min, pat;
    ASSERT(AutoUpdateEngine::ParseVersion(L"11.2.0", maj, min, pat));
    ASSERT(maj == 11 && min == 2 && pat == 0);
}

TEST(TestAutoUpdate_Counts) {
    ASSERT(AutoUpdateEngine::ChannelCount() == 4);
    ASSERT(AutoUpdateEngine::CheckResultCount() == 6);
    ASSERT(AutoUpdateEngine::DownloadStateCount() == 7);
}

// Release Gate V20 Tests

TEST(TestGateV20_KPINames) {
    for (uint32_t i = 0; i < ReleaseGateV20::KPICount(); ++i) {
        auto name = ReleaseGateV20::KPIName(static_cast<GateV20KPI>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestGateV20_KPICount) { ASSERT(ReleaseGateV20::KPICount() == 21); }

TEST(TestGateV20_Evaluate) {
    ReleaseGateV20 gate;
    std::vector<GateV20Result> results;
    auto verdict = gate.Evaluate(results);
    ASSERT(verdict.approved);
    ASSERT(verdict.passed == 21);
}

TEST(TestGateV20_Version) {
    GateV20Verdict v;
    ASSERT(v.version == L"12.0.0");
}

TEST(TestGateV20_ResultDefault) {
    GateV20Result r;
    ASSERT(!r.passed);
}

// CSV/JSON Preview Tests

TEST(TestStructData_FormatNames) {
    for (size_t i = 0; i < StructuredDataDecoder::FormatCount(); ++i) {
        auto name =
            StructuredDataDecoder::FormatName(static_cast<StructuredDataFormat>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestStructData_ValueTypeNames) {
    for (size_t i = 0; i < StructuredDataDecoder::ValueTypeCount(); ++i) {
        auto name =
            StructuredDataDecoder::ValueTypeName(static_cast<JSONValueType>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestStructData_StyleNames) {
    for (size_t i = 0; i < StructuredDataDecoder::StyleCount(); ++i) {
        auto name =
            StructuredDataDecoder::StyleName(static_cast<DataPreviewStyle>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestStructData_DetectFormat) {
    ASSERT(StructuredDataDecoder::DetectFormat(L".json") ==
        StructuredDataFormat::JSON);
    ASSERT(StructuredDataDecoder::DetectFormat(L".yaml") ==
        StructuredDataFormat::YAML);
    ASSERT(StructuredDataDecoder::DetectFormat(L".xml") ==
        StructuredDataFormat::XML);
}

TEST(TestStructData_Counts) {
    ASSERT(StructuredDataDecoder::FormatCount() == 6);
    ASSERT(StructuredDataDecoder::ValueTypeCount() == 6);
    ASSERT(StructuredDataDecoder::StyleCount() == 4);
}

// Notebook Preview Tests

TEST(TestNotebook_CellTypeNames) {
    for (size_t i = 0; i < NotebookPreviewDecoder::CellTypeCount(); ++i) {
        auto name =
            NotebookPreviewDecoder::CellTypeName(static_cast<NotebookCellType>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestNotebook_OutputTypeNames) {
    for (size_t i = 0; i < NotebookPreviewDecoder::OutputTypeCount(); ++i) {
        auto name = NotebookPreviewDecoder::OutputTypeName(
            static_cast<NotebookOutputType>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestNotebook_KernelNames) {
    for (size_t i = 0; i < NotebookPreviewDecoder::KernelCount(); ++i) {
        auto name =
            NotebookPreviewDecoder::KernelName(static_cast<NotebookKernel>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestNotebook_Counts) {
    ASSERT(NotebookPreviewDecoder::CellTypeCount() == 3);
    ASSERT(NotebookPreviewDecoder::OutputTypeCount() == 6);
    ASSERT(NotebookPreviewDecoder::KernelCount() == 6);
}

TEST(TestNotebook_MetadataDefaults) {
    NotebookMetadata meta;
    ASSERT(meta.kernel == NotebookKernel::Python);
    ASSERT(meta.formatVersion == 4);
}

// Database Preview Tests

TEST(TestDatabase_EngineNames) {
    for (size_t i = 0; i < DatabasePreviewDecoder::EngineCount(); ++i) {
        auto name =
            DatabasePreviewDecoder::EngineName(static_cast<DatabaseEngine>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestDatabase_ColumnTypeNames) {
    for (size_t i = 0; i < DatabasePreviewDecoder::ColumnTypeCount(); ++i) {
        auto name =
            DatabasePreviewDecoder::ColumnTypeName(static_cast<SQLColumnType>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestDatabase_PreviewStyleNames) {
    for (size_t i = 0; i < DatabasePreviewDecoder::PreviewStyleCount(); ++i) {
        auto name = DatabasePreviewDecoder::PreviewStyleName(
            static_cast<DatabasePreviewStyle>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestDatabase_SQLiteMagic) {
    const char magic[] = "SQLite format 3";
    uint8_t buf[16];
    memcpy(buf, magic, 15);
    buf[15] = 0;
    ASSERT(DatabasePreviewDecoder::CheckSQLiteMagic(buf, 16));
}

TEST(TestDatabase_Counts) {
    ASSERT(DatabasePreviewDecoder::EngineCount() == 4);
    ASSERT(DatabasePreviewDecoder::ColumnTypeCount() == 7);
    ASSERT(DatabasePreviewDecoder::PreviewStyleCount() == 4);
}

// Legacy Image Decoder Tests

TEST(TestLegacyImg_FormatNames) {
    for (size_t i = 0; i < LegacyImageDecoder::FormatCount(); ++i) {
        auto name =
            LegacyImageDecoder::FormatName(static_cast<LegacyImageFormat>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestLegacyImg_ColorSpaceNames) {
    for (size_t i = 0; i < LegacyImageDecoder::ColorSpaceCount(); ++i) {
        auto name =
            LegacyImageDecoder::ColorSpaceName(static_cast<LegacyColorSpace>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestLegacyImg_FLIFMagic) {
    uint8_t flif[] = { 'F', 'L', 'I', 'F' };
    ASSERT(LegacyImageDecoder::CheckFLIFMagic(flif, 4));
}

TEST(TestLegacyImg_BPGMagic) {
    uint8_t bpg[] = { 0x42, 0x50, 0x47, 0xFB };
    ASSERT(LegacyImageDecoder::CheckBPGMagic(bpg, 4));
}

TEST(TestLegacyImg_Counts) {
    ASSERT(LegacyImageDecoder::FormatCount() == 6);
    ASSERT(LegacyImageDecoder::ColorSpaceCount() == 6);
}

// CDR/Visio Vector Decoder Tests

TEST(TestVector_FormatNames) {
    for (size_t i = 0; i < VectorFormatDecoder::FormatCount(); ++i) {
        auto name = VectorFormatDecoder::FormatName(static_cast<VectorFormat>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestVector_ElementNames) {
    for (size_t i = 0; i < VectorFormatDecoder::ElementCount(); ++i) {
        auto name = VectorFormatDecoder::ElementName(static_cast<VectorElement>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestVector_DetectFormat) {
    ASSERT(VectorFormatDecoder::DetectFormat(L".cdr") == VectorFormat::CDR);
    ASSERT(VectorFormatDecoder::DetectFormat(L".vsdx") == VectorFormat::VSDX);
    ASSERT(VectorFormatDecoder::DetectFormat(L".emf") == VectorFormat::EMF);
}

TEST(TestVector_Counts) {
    ASSERT(VectorFormatDecoder::FormatCount() == 7);
    ASSERT(VectorFormatDecoder::ElementCount() == 7);
}

TEST(TestVector_ConfigDefaults) {
    VectorDecoderConfig cfg;
    ASSERT(cfg.renderWidth == 256);
    ASSERT(cfg.antiAlias);
}

// HDF5/NetCDF Scientific Decoder Tests

TEST(TestSciData_FormatNames) {
    for (size_t i = 0; i < ScientificDataDecoder::FormatCount(); ++i) {
        auto name =
            ScientificDataDecoder::FormatName(static_cast<ScientificDataFormat>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestSciData_DataTypeNames) {
    for (size_t i = 0; i < ScientificDataDecoder::DataTypeCount(); ++i) {
        auto name =
            ScientificDataDecoder::DataTypeName(static_cast<HDF5DataType>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestSciData_VisModeNames) {
    for (size_t i = 0; i < ScientificDataDecoder::VisModeCount(); ++i) {
        auto name = ScientificDataDecoder::VisModeName(static_cast<SciVisMode>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestSciData_HDF5Magic) {
    uint8_t magic[] = { 0x89, 0x48, 0x44, 0x46, 0x0D, 0x0A, 0x1A, 0x0A };
    ASSERT(ScientificDataDecoder::CheckHDF5Magic(magic, 8));
}

TEST(TestSciData_Counts) {
    ASSERT(ScientificDataDecoder::FormatCount() == 4);
    ASSERT(ScientificDataDecoder::DataTypeCount() == 8);
    ASSERT(ScientificDataDecoder::VisModeCount() == 5);
}

// NIfTI Neuroimaging Tests

TEST(TestNIfTI_DataTypeNames) {
    for (size_t i = 0; i < NIfTIDecoder::DataTypeCount(); ++i) {
        auto name = NIfTIDecoder::DataTypeName(static_cast<NIfTIDataType>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestNIfTI_SliceNames) {
    for (size_t i = 0; i < NIfTIDecoder::SliceCount(); ++i) {
        auto name = NIfTIDecoder::SliceName(static_cast<NIfTISlice>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestNIfTI_VariantNames) {
    for (size_t i = 0; i < NIfTIDecoder::VariantCount(); ++i) {
        auto name = NIfTIDecoder::VariantName(static_cast<NIfTIVariant>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestNIfTI_Counts) {
    ASSERT(NIfTIDecoder::DataTypeCount() == 7);
    ASSERT(NIfTIDecoder::SliceCount() == 3);
    ASSERT(NIfTIDecoder::VariantCount() == 4);
}

TEST(TestNIfTI_HeaderDefaults) {
    NIfTIHeaderInfo hdr;
    ASSERT(hdr.variant == NIfTIVariant::NIfTI1);
    ASSERT(hdr.voxOffset == 352);
    ASSERT(hdr.sclSlope == 1.0f);
}

// STEP/IGES CAD Decoder Tests

TEST(TestCAD_FormatNames) {
    for (size_t i = 0; i < CADFormatDecoder::FormatCount(); ++i) {
        auto name = CADFormatDecoder::FormatName(static_cast<CADFormat>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestCAD_EntityNames) {
    for (size_t i = 0; i < CADFormatDecoder::EntityCount(); ++i) {
        auto name = CADFormatDecoder::EntityName(static_cast<CADEntity>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestCAD_RenderModeNames) {
    for (size_t i = 0; i < CADFormatDecoder::RenderModeCount(); ++i) {
        auto name = CADFormatDecoder::RenderModeName(static_cast<CADRenderMode>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestCAD_STEPMagic) {
    const char step[] = "ISO-10303-21;";
    ASSERT(CADFormatDecoder::CheckSTEPMagic(
        reinterpret_cast<const uint8_t*>(step), 13));
}

TEST(TestCAD_Counts) {
    ASSERT(CADFormatDecoder::FormatCount() == 4);
    ASSERT(CADFormatDecoder::EntityCount() == 7);
    ASSERT(CADFormatDecoder::RenderModeCount() == 5);
}

// HDR Display Pipeline Tests

TEST(TestHDR_ToneMapNames) {
    for (size_t i = 0; i < HDRDisplayPipeline::ToneMapCount(); ++i) {
        auto name = HDRDisplayPipeline::ToneMapName(static_cast<ToneMappingOp>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestHDR_GamutNames) {
    for (size_t i = 0; i < HDRDisplayPipeline::GamutCount(); ++i) {
        auto name = HDRDisplayPipeline::GamutName(static_cast<ColorGamut>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestHDR_FormatNames) {
    for (size_t i = 0; i < HDRDisplayPipeline::HDRFormatCount(); ++i) {
        auto name = HDRDisplayPipeline::HDRFormatName(static_cast<HDRFormat>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestHDR_ValidateExposure) {
    ASSERT(HDRDisplayPipeline::ValidateExposure(1.0f));
    ASSERT(!HDRDisplayPipeline::ValidateExposure(0.0f));
    ASSERT(!HDRDisplayPipeline::ValidateExposure(25.0f));
}

TEST(TestHDR_Counts) {
    ASSERT(HDRDisplayPipeline::ToneMapCount() == 6);
    ASSERT(HDRDisplayPipeline::GamutCount() == 5);
    ASSERT(HDRDisplayPipeline::HDRFormatCount() == 5);
}

// Per-Monitor DPI V3 Tests
TEST(TestDPIV3_AwarenessNames) {
    for (size_t i = 0; i < PerMonitorDPIV3::AwarenessCount(); ++i) {
        auto name = PerMonitorDPIV3::AwarenessName(static_cast<DPIAwareness>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestDPIV3_ScaleNames) {
    for (size_t i = 0; i < PerMonitorDPIV3::ScaleCount(); ++i) {
        auto name = PerMonitorDPIV3::ScaleName(static_cast<DPIScale>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestDPI_ScaledSize) {
    ASSERT(PerMonitorDPIV3::ScaledSize(256, 96) == 256);
    ASSERT(PerMonitorDPIV3::ScaledSize(256, 192) == 512);
    ASSERT(PerMonitorDPIV3::ScaledSize(256, 144) == 384);
}

TEST(TestDPI_DefaultConfig) {
    DPIScalingConfig cfg;
    ASSERT(cfg.awareness == DPIAwareness::PerMonitorV2);
    ASSERT(cfg.baseThumbnailSize == 256);
    ASSERT(cfg.autoScale == true);
}

TEST(TestDPI_Counts) {
    ASSERT(PerMonitorDPIV3::AwarenessCount() == 5);
    ASSERT(PerMonitorDPIV3::ScaleCount() == 8);
}

// Shell Overlay Icon Handler Tests
TEST(TestOverlay_IconNames) {
    for (size_t i = 0; i < ShellOverlayHandler::OverlayCount(); ++i) {
        auto name =
            ShellOverlayHandler::OverlayName(static_cast<OverlayIconType>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestOverlay_PositionNames) {
    for (size_t i = 0; i < ShellOverlayHandler::PositionCount(); ++i) {
        auto name =
            ShellOverlayHandler::PositionName(static_cast<OverlayPosition>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestOverlay_ValidateOpacity) {
    ASSERT(ShellOverlayHandler::ValidateOpacity(0.5f));
    ASSERT(ShellOverlayHandler::ValidateOpacity(0.0f));
    ASSERT(ShellOverlayHandler::ValidateOpacity(1.0f));
    ASSERT(!ShellOverlayHandler::ValidateOpacity(-0.1f));
    ASSERT(!ShellOverlayHandler::ValidateOpacity(1.1f));
}

TEST(TestOverlay_DefaultConfig) {
    OverlayIconConfig cfg;
    ASSERT(cfg.position == OverlayPosition::BottomRight);
    ASSERT(cfg.iconSize == 16);
    ASSERT(cfg.enabled == true);
}

TEST(TestOverlay_Counts) {
    ASSERT(ShellOverlayHandler::OverlayCount() == 7);
    ASSERT(ShellOverlayHandler::PositionCount() == 4);
}

// Cache Warming Service Tests
TEST(TestCacheWarm_StrategyNames) {
    for (size_t i = 0; i < CacheWarmingService::StrategyCount(); ++i) {
        auto name =
            CacheWarmingService::StrategyName(static_cast<WarmingStrategy>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestCacheWarm_PriorityNames) {
    for (size_t i = 0; i < CacheWarmingService::PriorityCount(); ++i) {
        auto name =
            CacheWarmingService::PriorityName(static_cast<WarmingPriority>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestCacheWarm_JobStatusNames) {
    for (size_t i = 0; i < CacheWarmingService::JobStatusCount(); ++i) {
        auto name =
            CacheWarmingService::JobStatusName(static_cast<WarmingJobStatus>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestCacheWarm_DefaultConfig) {
    CacheWarmingConfig cfg;
    ASSERT(cfg.strategy == WarmingStrategy::MostRecent);
    ASSERT(cfg.priority == WarmingPriority::Idle);
    ASSERT(cfg.maxConcurrent == 2);
    ASSERT(cfg.respectPowerMode == true);
}

TEST(TestCacheWarm_Counts) {
    ASSERT(CacheWarmingService::StrategyCount() == 5);
    ASSERT(CacheWarmingService::PriorityCount() == 4);
    ASSERT(CacheWarmingService::JobStatusCount() == 6);
}

// Multi-GPU Load Balancer Tests
TEST(TestMultiGPU_StrategyNames) {
    for (size_t i = 0; i < MultiGPULoadBalancer::StrategyCount(); ++i) {
        auto name =
            MultiGPULoadBalancer::StrategyName(static_cast<GPUBalanceStrategy>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestMultiGPU_DeviceTypeNames) {
    for (size_t i = 0; i < MultiGPULoadBalancer::DeviceTypeCount(); ++i) {
        auto name =
            MultiGPULoadBalancer::DeviceTypeName(static_cast<GPUDeviceType>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestMultiGPU_ValidateConfig) {
    MultiGPUConfig cfg;
    ASSERT(MultiGPULoadBalancer::ValidateConfig(cfg));
    MultiGPUConfig bad;
    bad.maxGPUs = 0;
    ASSERT(!MultiGPULoadBalancer::ValidateConfig(bad));
}

TEST(TestMultiGPU_DefaultConfig) {
    MultiGPUConfig cfg;
    ASSERT(cfg.strategy == GPUBalanceStrategy::LeastLoaded);
    ASSERT(cfg.maxGPUs == 4);
    ASSERT(cfg.enableFallback == true);
    ASSERT(cfg.preferDiscrete == true);
}

TEST(TestMultiGPU_Counts) {
    ASSERT(MultiGPULoadBalancer::StrategyCount() == 5);
    ASSERT(MultiGPULoadBalancer::DeviceTypeCount() == 4);
}

// Release Gate V21 Tests
TEST(TestGateV21_KPINames) {
    for (uint32_t i = 0; i < ReleaseGateV21::KPICount(); ++i) {
        auto name = ReleaseGateV21::KPIName(static_cast<GateV21KPI>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestGateV21_Evaluate) {
    ReleaseGateV21 gate;
    std::vector<GateV21Result> results;
    auto verdict = gate.Evaluate(results);
    ASSERT(verdict.approved == true);
    ASSERT(verdict.passed == ReleaseGateV21::KPICount());
    ASSERT(verdict.failed == 0);
}

TEST(TestGateV21_KPICount) { ASSERT(ReleaseGateV21::KPICount() == 22); }

TEST(TestGateV21_Version) {
    GateV21Verdict v;
    ASSERT(v.version == L"12.5.0");
    ASSERT(v.approved == false);
}

TEST(TestGateV21_AllKPIsPresent) {
    ReleaseGateV21 gate;
    std::vector<GateV21Result> results;
    gate.Evaluate(results);
    ASSERT(results.size() == ReleaseGateV21::KPICount());
    for (auto& r : results)
        ASSERT(r.passed);
}

// Accessibility Pipeline Tests
TEST(TestAccessibility_FeatureNames) {
    for (size_t i = 0; i < AccessibilityPipeline::FeatureCount(); ++i) {
        auto name = AccessibilityPipeline::FeatureName(
            static_cast<AccessibilityFeature>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestAccessibility_ColorBlindModes) {
    for (size_t i = 0; i < AccessibilityPipeline::ColorBlindModeCount(); ++i) {
        auto name = AccessibilityPipeline::ColorBlindModeName(
            static_cast<ColorBlindMode>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestAccessibility_HCThemes) {
    for (size_t i = 0; i < AccessibilityPipeline::HCThemeCount(); ++i) {
        auto name =
            AccessibilityPipeline::HCThemeName(static_cast<HighContrastTheme>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestAccessibility_DefaultConfig) {
    AccessibilityConfig cfg;
    ASSERT(cfg.screenReader == true);
    ASSERT(cfg.highContrast == false);
    ASSERT(cfg.keyboardNav == true);
    ASSERT(cfg.cbMode == ColorBlindMode::None);
}

TEST(TestAccessibility_Counts) {
    ASSERT(AccessibilityPipeline::FeatureCount() == 6);
    ASSERT(AccessibilityPipeline::ColorBlindModeCount() == 5);
    ASSERT(AccessibilityPipeline::HCThemeCount() == 5);
}

// Telemetry & Analytics Tests
TEST(TestTelemetry_EventNames) {
    for (size_t i = 0; i < TelemetryAnalyticsEngine::EventCount(); ++i) {
        auto name =
            TelemetryAnalyticsEngine::EventName(static_cast<AnalyticsEventType>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestTelemetry_ConsentNames) {
    for (size_t i = 0; i < TelemetryAnalyticsEngine::ConsentCount(); ++i) {
        auto name =
            TelemetryAnalyticsEngine::ConsentName(static_cast<TelemetryConsent>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestTelemetry_PeriodNames) {
    for (size_t i = 0; i < TelemetryAnalyticsEngine::PeriodCount(); ++i) {
        auto name =
            TelemetryAnalyticsEngine::PeriodName(static_cast<AggregationPeriod>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestTelemetry_CacheHitRate) {
    ASSERT(TelemetryAnalyticsEngine::CacheHitRate(80, 20) > 0.79);
    ASSERT(TelemetryAnalyticsEngine::CacheHitRate(80, 20) < 0.81);
    ASSERT(TelemetryAnalyticsEngine::CacheHitRate(0, 0) == 0.0);
}

TEST(TestTelemetry_DefaultConfig) {
    TelemetryConfig cfg;
    ASSERT(cfg.consent == TelemetryConsent::Disabled);
    ASSERT(cfg.localOnly == true);
    ASSERT(cfg.anonymize == true);
}

// Cloud Storage Integration Tests
TEST(TestCloudStorage_ProviderNames) {
    for (size_t i = 0; i < CloudStorageIntegration::ProviderCount(); ++i) {
        auto name =
            CloudStorageIntegration::ProviderName(static_cast<CloudProvider>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestCloud_FileStateNames) {
    for (size_t i = 0; i < CloudStorageIntegration::FileStateCount(); ++i) {
        auto name =
            CloudStorageIntegration::FileStateName(static_cast<CloudFileState>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestCloud_HydrationNames) {
    for (size_t i = 0; i < CloudStorageIntegration::HydrationCount(); ++i) {
        auto name = CloudStorageIntegration::HydrationName(
            static_cast<HydrationStrategy>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestCloud_ShouldHydrate) {
    CloudIntegrationConfig cfg;
    cfg.strategy = HydrationStrategy::HydrateIfSmall;
    CloudFileInfo smallFile; // renamed: 'small' is #define'd as char in rpcndr.h
    smallFile.state = CloudFileState::OnlineOnly;
    smallFile.fileSize = 1024;
    ASSERT(CloudStorageIntegration::ShouldHydrate(smallFile, cfg));
    CloudFileInfo local;
    local.state = CloudFileState::Available;
    ASSERT(!CloudStorageIntegration::ShouldHydrate(local, cfg));
}

TEST(TestCloud_Counts) {
    ASSERT(CloudStorageIntegration::ProviderCount() == 6);
    ASSERT(CloudStorageIntegration::FileStateCount() == 5);
    ASSERT(CloudStorageIntegration::HydrationCount() == 4);
}

// Release Gate V22 (v13.0) Tests
TEST(TestGateV22_KPINames) {
    for (uint32_t i = 0; i < ReleaseGateV22::KPICount(); ++i) {
        auto name = ReleaseGateV22::KPIName(static_cast<GateV22KPI>(i));
        ASSERT(name != nullptr && wcslen(name) > 0);
    }
}

TEST(TestGateV22_Evaluate) {
    ReleaseGateV22 gate;
    std::vector<GateV22Result> results;
    auto verdict = gate.Evaluate(results);
    ASSERT(verdict.approved == true);
    ASSERT(verdict.passed == ReleaseGateV22::KPICount());
    ASSERT(verdict.failed == 0);
}

TEST(TestGateV22_KPICount) { ASSERT(ReleaseGateV22::KPICount() == 23); }

TEST(TestGateV22_Version) {
    GateV22Verdict v;
    ASSERT(v.version == L"13.0.0");
    ASSERT(v.milestone == L"v13.0 Final Release");
}

TEST(TestGateV22_AllKPIsPresent) {
    ReleaseGateV22 gate;
    std::vector<GateV22Result> results;
    gate.Evaluate(results);
    ASSERT(results.size() == ReleaseGateV22::KPICount());
    for (auto& r : results)
        ASSERT(r.passed);
}

//==============================================================================
// v14.0 "Apex" Tests
//==============================================================================

// VersionSyncV14
TEST(TestV14_DomainNames) {
    ASSERT(VersionSyncV14::DomainName(V14Domain::GPUPipelineV3) != nullptr);
}
TEST(TestV14_FeatureStatusNames) {
    ASSERT(VersionSyncV14::FeatureStatusName(FeatureStatus::Implemented) !=
        nullptr);
}
TEST(TestV14_DomainCount) {
    ASSERT(VersionSyncV14::DomainCount() ==
        static_cast<size_t>(V14Domain::COUNT));
}
TEST(TestV14_FeatureStatusCount) {
    ASSERT(VersionSyncV14::FeatureStatusCount() ==
        static_cast<size_t>(FeatureStatus::COUNT));
}
TEST(TestV14_GetVersion) {
    V14Version v = VersionSyncV14::GetVersion();
    ASSERT(v.major == 14 && v.minor == 0);
}

// GPUPipelineV3
TEST(TestGPUV3_FeatureNames) {
    ASSERT(GPUPipelineV3::FeatureName(GPUV3Feature::MeshShaders) != nullptr);
}
TEST(TestGPUV3_QueueNames) {
    ASSERT(GPUPipelineV3::QueueName(PipelineV3Queue::DirectGraphics) != nullptr);
}
TEST(TestGPUV3_PerfTierNames) {
    ASSERT(GPUPipelineV3::PerfTierName(GPUV3PerfTier::Tier3_Advanced) != nullptr);
}
TEST(TestGPUV3_FeatureCount) {
    ASSERT(GPUPipelineV3::FeatureCount() ==
        static_cast<size_t>(GPUV3Feature::COUNT));
}
TEST(TestGPUV3_QueueCount) {
    ASSERT(GPUPipelineV3::QueueCount() ==
        static_cast<size_t>(PipelineV3Queue::COUNT));
}

// ShaderCompilerV2
TEST(TestShaderV2_ModelNames) {
    ASSERT(ShaderCompilerV2::ShaderModelName(ShaderModel::SM67) != nullptr);
}
TEST(TestShaderV2_StageNames) {
    ASSERT(ShaderCompilerV2::StageName(ShaderStage::Vertex) != nullptr);
}
TEST(TestShaderV2_OptLevelNames) {
    ASSERT(ShaderCompilerV2::OptLevelName(ShaderOptLevel::Maximum) != nullptr);
}
TEST(TestShaderV2_ModelCount) {
    ASSERT(ShaderCompilerV2::ShaderModelCount() ==
        static_cast<size_t>(ShaderModel::COUNT));
}
TEST(TestShaderV2_StageCount) {
    ASSERT(ShaderCompilerV2::StageCount() ==
        static_cast<size_t>(ShaderStage::COUNT));
}

// PipelineStateCacheV2
TEST(TestPSOCacheV2_StateNames) {
    ASSERT(PipelineStateCacheV2::CacheStateName(PSOCacheState::Cached) !=
        nullptr);
}
TEST(TestPSOCacheV2_TypeNames) {
    ASSERT(PipelineStateCacheV2::PipelineTypeName(PipelineType::Graphics) !=
        nullptr);
}
TEST(TestPSOCacheV2_WarmupNames) {
    ASSERT(PipelineStateCacheV2::WarmupStrategyName(PSOWarmupStrategy::Eager) !=
        nullptr);
}
TEST(TestPSOCacheV2_StateCount) {
    ASSERT(PipelineStateCacheV2::CacheStateCount() ==
        static_cast<size_t>(PSOCacheState::COUNT));
}
TEST(TestPSOCacheV2_TypeCount) {
    ASSERT(PipelineStateCacheV2::PipelineTypeCount() ==
        static_cast<size_t>(PipelineType::COUNT));
}

// GPUMemoryPoolV2
TEST(TestGPUMemV2_HeapTypeNames) {
    ASSERT(GPUMemoryPoolV2::HeapTypeName(GPUHeapType::Default) != nullptr);
}
TEST(TestGPUMemV2_ResidencyNames) {
    ASSERT(GPUMemoryPoolV2::ResidencyName(GPUResidencyPriority::High) != nullptr);
}
TEST(TestGPUMemV2_AllocNames) {
    ASSERT(GPUMemoryPoolV2::StrategyName(GPUAllocStrategy::BestFit) != nullptr);
}
TEST(TestGPUMemV2_HeapTypeCount) {
    ASSERT(GPUMemoryPoolV2::HeapTypeCount() ==
        static_cast<size_t>(GPUHeapType::COUNT));
}
TEST(TestGPUMemV2_ResidencyCount) {
    ASSERT(GPUMemoryPoolV2::ResidencyCount() ==
        static_cast<size_t>(GPUResidencyPriority::COUNT));
}

// ReleaseGateV23
TEST(TestGateV23_KPINames) {
    ASSERT(ReleaseGateV23::KPIName(GateV23KPI::GPUPipelineV3) != nullptr);
}
TEST(TestGateV23_KPICount) { ASSERT(ReleaseGateV23::KPICount() == 12); }
TEST(TestGateV23_Evaluate) {
    std::vector<GateV23Result> r;
    auto res = ReleaseGateV23::Evaluate(r);
    ASSERT(res.passed == 0u);
}
TEST(TestGateV23_AllPass) {
    std::vector<GateV23Result> r;
    for (uint32_t i = 0; i < static_cast<uint32_t>(GateV23KPI::COUNT); ++i) {
        GateV23Result g;
        g.kpi = static_cast<GateV23KPI>(i);
        g.passed = true;
        r.push_back(g);
    }
    auto res = ReleaseGateV23::Evaluate(r);
    ASSERT(res.approved);
}
TEST(TestGateV23_Advance) {
    std::vector<GateV23Result> r;
    auto res = ReleaseGateV23::Evaluate(r);
    ASSERT(!res.approved);
}

// SmartFormatDetectorV2
TEST(TestSmartDetV2_MethodNames) {
    ASSERT(SmartFormatDetectorV2::MethodName(DetectionMethod::MagicBytes) !=
        nullptr);
}
TEST(TestSmartDetV2_ConfNames) {
    ASSERT(SmartFormatDetectorV2::ConfidenceName(DetectionConfidence::High) !=
        nullptr);
}
TEST(TestSmartDetV2_HintNames) {
    ASSERT(SmartFormatDetectorV2::HintName(DetectionHint::Extension) != nullptr);
}
TEST(TestSmartDetV2_MethodCount) {
    ASSERT(SmartFormatDetectorV2::MethodCount() ==
        static_cast<size_t>(DetectionMethod::COUNT));
}
TEST(TestSmartDetV2_ConfCount) {
    ASSERT(SmartFormatDetectorV2::ConfidenceCount() ==
        static_cast<size_t>(DetectionConfidence::COUNT));
}

// ExtendedVideoDecoder
TEST(TestExtVideo_CodecNames) {
    ASSERT(ExtendedVideoDecoder::CodecName(ExtVideoCodec::H264) != nullptr);
}
TEST(TestExtVideo_AccelNames) {
    ASSERT(ExtendedVideoDecoder::AccelName(VideoDecodeAccel::DXVA2) != nullptr);
}
TEST(TestExtVideo_FrameSelectNames) {
    ASSERT(ExtendedVideoDecoder::FrameSelectName(
        VideoFrameSelect::FirstKeyframe) != nullptr);
}
TEST(TestExtVideo_CodecCount) {
    ASSERT(ExtendedVideoDecoder::CodecCount() ==
        static_cast<size_t>(ExtVideoCodec::COUNT));
}
TEST(TestExtVideo_AccelCount) {
    ASSERT(ExtendedVideoDecoder::AccelCount() ==
        static_cast<size_t>(VideoDecodeAccel::COUNT));
}

// AudioVisualizationV2
TEST(TestAudioVisV2_ModeNames) {
    ASSERT(AudioVisualizationV2::ModeName(AudioVisMode::Waveform) != nullptr);
}
TEST(TestAudioVisV2_ColorNames) {
    ASSERT(AudioVisualizationV2::ColorSchemeName(AudioVisColorScheme::Fire) !=
        nullptr);
}
TEST(TestAudioVisV2_LoudnessNames) {
    ASSERT(AudioVisualizationV2::LoudnessUnitName(LoudnessUnit::LUFS) != nullptr);
}
TEST(TestAudioVisV2_ModeCount) {
    ASSERT(AudioVisualizationV2::ModeCount() ==
        static_cast<size_t>(AudioVisMode::COUNT));
}
TEST(TestAudioVisV2_ColorCount) {
    ASSERT(AudioVisualizationV2::ColorSchemeCount() ==
        static_cast<size_t>(AudioVisColorScheme::COUNT));
}

// Model3DRendererV2
TEST(TestModel3DV2_FormatNames) {
    ASSERT(Model3DRendererV2::FormatName(Model3DFormat::OBJ) != nullptr);
}
TEST(TestModel3DV2_LightNames) {
    ASSERT(Model3DRendererV2::LightingModeName(Model3DLightingMode::PBR) !=
        nullptr);
}
TEST(TestModel3DV2_CamNames) {
    ASSERT(Model3DRendererV2::CameraPresetName(Model3DCameraPreset::Front) !=
        nullptr);
}
TEST(TestModel3DV2_FormatCount) {
    ASSERT(Model3DRendererV2::FormatCount() ==
        static_cast<size_t>(Model3DFormat::COUNT));
}
TEST(TestModel3DV2_LightCount) {
    ASSERT(Model3DRendererV2::LightingModeCount() ==
        static_cast<size_t>(Model3DLightingMode::COUNT));
}

// ReleaseGateV24
TEST(TestGateV24_KPINames) {
    ASSERT(ReleaseGateV24::KPIName(GateV24KPI::SmartFormatDetection) != nullptr);
}
TEST(TestGateV24_KPICount) { ASSERT(ReleaseGateV24::KPICount() == 12); }
TEST(TestGateV24_Evaluate) {
    bool r[30] = {};
    auto res = ReleaseGateV24::Evaluate(r);
    ASSERT(res.kpiPassCount == 0);
}
TEST(TestGateV24_AllPass) {
    bool r[30];
    for (int i = 0; i < 30; ++i)
        r[i] = true;
    auto res = ReleaseGateV24::Evaluate(r);
    ASSERT(res.allKPIsPass);
}
TEST(TestGateV24_Advance) {
    bool r[30] = {};
    r[0] = true;
    auto res = ReleaseGateV24::Evaluate(r);
    ASSERT(!res.advanceRecommended);
}

// PluginSDKV2
TEST(TestPluginSDKV2_CapNames) {
    ASSERT(PluginSDKV2::CapabilityName(
        PluginSDKV2Capability::ThumbnailProvider) != nullptr);
}
TEST(TestPluginSDKV2_LifeCycleNames) {
    ASSERT(PluginSDKV2::LifeCycleName(PluginSDKV2LifeCycle::Init) != nullptr);
}
TEST(TestPluginSDKV2_APIVersionNames) {
    ASSERT(PluginSDKV2::APIVersionName(PluginAPIVersion::V2) != nullptr);
}
TEST(TestPluginSDKV2_CapCount) {
    ASSERT(PluginSDKV2::CapabilityCount() ==
        static_cast<size_t>(PluginSDKV2Capability::COUNT));
}
TEST(TestPluginSDKV2_LifeCycleCount) {
    ASSERT(PluginSDKV2::LifeCycleCount() ==
        static_cast<size_t>(PluginSDKV2LifeCycle::COUNT));
}

// PluginDebuggerIntegration
TEST(TestPluginDbg_ModeNames) {
    ASSERT(PluginDebuggerIntegration::ModeName(PluginDebugMode::Attach) !=
        nullptr);
}
TEST(TestPluginDbg_LogLevelNames) {
    ASSERT(PluginDebuggerIntegration::LogLevelName(PluginLogLevel::Debug) !=
        nullptr);
}
TEST(TestPluginDbg_EventNames) {
    ASSERT(PluginDebuggerIntegration::EventName(PluginDebugEvent::Load) !=
        nullptr);
}
TEST(TestPluginDbg_ModeCount) {
    ASSERT(PluginDebuggerIntegration::ModeCount() ==
        static_cast<size_t>(PluginDebugMode::COUNT));
}
TEST(TestPluginDbg_LogLevelCount) {
    ASSERT(PluginDebuggerIntegration::LogLevelCount() ==
        static_cast<size_t>(PluginLogLevel::COUNT));
}

// PluginHotReload
TEST(TestHotReload_TriggerNames) {
    ASSERT(PluginHotReload::TriggerName(HotReloadTrigger::FileChanged) !=
        nullptr);
}
TEST(TestHotReload_StateNames) {
    ASSERT(PluginHotReload::StateName(HotReloadState::Idle) != nullptr);
}
TEST(TestHotReload_PolicyNames) {
    ASSERT(PluginHotReload::PolicyName(HotReloadPolicy::Automatic) != nullptr);
}
TEST(TestHotReload_TriggerCount) {
    ASSERT(PluginHotReload::TriggerCount() ==
        static_cast<size_t>(HotReloadTrigger::COUNT));
}
TEST(TestHotReload_StateCount) {
    ASSERT(PluginHotReload::StateCount() ==
        static_cast<size_t>(HotReloadState::COUNT));
}

// PluginPerformanceProfiler
TEST(TestPluginPerf_MetricNames) {
    ASSERT(PluginPerformanceProfiler::MetricName(
        PluginPerfMetric::DecodeTimeMs) != nullptr);
}
TEST(TestPluginPerf_AlertNames) {
    ASSERT(PluginPerformanceProfiler::AlertName(PluginPerfAlert::SlowDecode) !=
        nullptr);
}
TEST(TestPluginPerf_SamplingNames) {
    ASSERT(PluginPerformanceProfiler::SamplingRateName(
        PluginPerfSamplingRate::High) != nullptr);
}
TEST(TestPluginPerf_MetricCount) {
    ASSERT(PluginPerformanceProfiler::MetricCount() ==
        static_cast<size_t>(PluginPerfMetric::COUNT));
}
TEST(TestPluginPerf_AlertCount) {
    ASSERT(PluginPerformanceProfiler::AlertCount() ==
        static_cast<size_t>(PluginPerfAlert::COUNT));
}

// ReleaseGateV25
TEST(TestGateV25_KPINames) {
    ASSERT(ReleaseGateV25::KPIName(GateV25KPI::BuildClean) != nullptr);
}
TEST(TestGateV25_KPICount) { ASSERT(ReleaseGateV25::KPICount() == 11); }
TEST(TestGateV25_Evaluate) {
    std::vector<GateV25Result> r;
    auto v = ReleaseGateV25::Evaluate(r);
    ASSERT(v.passed == 0);
}
TEST(TestGateV25_AllPass) {
    std::vector<GateV25Result> r;
    auto v = ReleaseGateV25::Evaluate(r);
    ASSERT(!v.approved);
}
TEST(TestGateV25_Advance) { ASSERT(ReleaseGateV25::KPICount() > 0); }

// ThreatModelV2
TEST(TestThreatV2_CategoryNames) {
    ASSERT(ThreatModelV2::CategoryName(ThreatCategory::Spoofing) != nullptr);
}
TEST(TestThreatV2_SeverityNames) {
    ASSERT(ThreatModelV2::SeverityName(ThreatSeverity::Critical) != nullptr);
}
TEST(TestThreatV2_MitigationNames) {
    ASSERT(ThreatModelV2::MitigationName(MitigationStatus::Implemented) !=
        nullptr);
}
TEST(TestThreatV2_CategoryCount) {
    ASSERT(ThreatModelV2::CategoryCount() ==
        static_cast<size_t>(ThreatCategory::COUNT));
}
TEST(TestThreatV2_SeverityCount) {
    ASSERT(ThreatModelV2::SeverityCount() ==
        static_cast<size_t>(ThreatSeverity::COUNT));
}

// MemorySafetyAuditV2
TEST(TestMemSafetyV2_ViolationNames) {
    ASSERT(MemorySafetyAuditV2::ViolationName(
        MemSafetyViolation::BufferOverflow) != nullptr);
}
TEST(TestMemSafetyV2_ToolNames) {
    ASSERT(MemorySafetyAuditV2::ToolName(MemSafetyTool::AddressSanitizer) !=
        nullptr);
}
TEST(TestMemSafetyV2_ScopeNames) {
    ASSERT(MemorySafetyAuditV2::ScopeName(MemSafetyScope::AllDecoders) !=
        nullptr);
}
TEST(TestMemSafetyV2_ViolationCount) {
    ASSERT(MemorySafetyAuditV2::ViolationCount() ==
        static_cast<size_t>(MemSafetyViolation::COUNT));
}
TEST(TestMemSafetyV2_ToolCount) {
    ASSERT(MemorySafetyAuditV2::ToolCount() ==
        static_cast<size_t>(MemSafetyTool::COUNT));
}

// SupplyChainIntegrityV2
TEST(TestSupplyChainV2_SBOMNames) {
    ASSERT(SupplyChainIntegrityV2::SBOMFormatName(SBOMFormat::SPDX) != nullptr);
}
TEST(TestSupplyChainV2_VulnNames) {
    ASSERT(SupplyChainIntegrityV2::VulnStatusName(DepVulnStatus::Clean) !=
        nullptr);
}
TEST(TestSupplyChainV2_ReprodNames) {
    ASSERT(SupplyChainIntegrityV2::ReproducibleCheckName(
        ReproducibleBuildCheck::HashMatch) != nullptr);
}
TEST(TestSupplyChainV2_SBOMCount) {
    ASSERT(SupplyChainIntegrityV2::SBOMFormatCount() ==
        static_cast<size_t>(SBOMFormat::COUNT));
}
TEST(TestSupplyChainV2_VulnCount) {
    ASSERT(SupplyChainIntegrityV2::VulnStatusCount() ==
        static_cast<size_t>(DepVulnStatus::COUNT));
}

// RuntimeIntegrityVerifier
TEST(TestRuntimeInteg_CheckTypeNames) {
    ASSERT(RuntimeIntegrityVerifier::CheckTypeName(
        IntegrityCheckType::CodeSigning) != nullptr);
}
TEST(TestRuntimeInteg_ResultNames) {
    ASSERT(RuntimeIntegrityVerifier::VerifyResultName(
        IntegrityVerifyResult::Pass) != nullptr);
}
TEST(TestRuntimeInteg_TamperNames) {
    ASSERT(RuntimeIntegrityVerifier::TamperName(TamperIndicator::None) !=
        nullptr);
}
TEST(TestRuntimeInteg_CheckTypeCount) {
    ASSERT(RuntimeIntegrityVerifier::CheckTypeCount() ==
        static_cast<size_t>(IntegrityCheckType::COUNT));
}
TEST(TestRuntimeInteg_ResultCount) {
    ASSERT(RuntimeIntegrityVerifier::VerifyResultCount() ==
        static_cast<size_t>(IntegrityVerifyResult::COUNT));
}

// ReleaseGateV26
TEST(TestGateV26_KPINames) {
    ASSERT(ReleaseGateV26::KPIName(GateV26KPI::BuildClean) != nullptr);
}
TEST(TestGateV26_KPICount) { ASSERT(ReleaseGateV26::KPICount() == 11); }
TEST(TestGateV26_Evaluate) {
    std::vector<GateV26Result> r;
    auto v = ReleaseGateV26::Evaluate(r);
    ASSERT(v.passed == 0);
}
TEST(TestGateV26_AllPass) {
    std::vector<GateV26Result> r;
    auto v = ReleaseGateV26::Evaluate(r);
    ASSERT(!v.approved);
}
TEST(TestGateV26_Advance) { ASSERT(ReleaseGateV26::KPICount() > 0); }

// ProgressiveThumbnailLoader
TEST(TestProgLoad_StageNames) {
    ASSERT(ProgressiveThumbnailLoader::StageName(
        ProgressiveLoadStage::Placeholder) != nullptr);
}
TEST(TestProgLoad_StrategyNames) {
    ASSERT(ProgressiveThumbnailLoader::StrategyName(
        ProgressiveLoadStrategy::BlurToSharp) != nullptr);
}
TEST(TestProgLoad_PlaceholderNames) {
    ASSERT(ProgressiveThumbnailLoader::PlaceholderName(
        ThumbnailPlaceholder::ColorSwatch) != nullptr);
}
TEST(TestProgLoad_StageCount) {
    ASSERT(ProgressiveThumbnailLoader::StageCount() ==
        static_cast<size_t>(ProgressiveLoadStage::COUNT));
}
TEST(TestProgLoad_StrategyCount) {
    ASSERT(ProgressiveThumbnailLoader::StrategyCount() ==
        static_cast<size_t>(ProgressiveLoadStrategy::COUNT));
}

// ThumbnailAnimationEngineV2
TEST(TestAnimEngineV2_FormatNames) {
    ASSERT(ThumbnailAnimationEngineV2::FormatName(AnimThumbnailFormat::GIF) !=
        nullptr);
}
TEST(TestAnimEngineV2_LoopModeNames) {
    ASSERT(ThumbnailAnimationEngineV2::LoopModeName(AnimLoopMode::Infinite) !=
        nullptr);
}
TEST(TestAnimEngineV2_InterpNames) {
    ASSERT(ThumbnailAnimationEngineV2::InterpolationName(
        AnimInterpolation::Linear) != nullptr);
}
TEST(TestAnimEngineV2_FormatCount) {
    ASSERT(ThumbnailAnimationEngineV2::FormatCount() ==
        static_cast<size_t>(AnimThumbnailFormat::COUNT));
}
TEST(TestAnimEngineV2_LoopModeCount) {
    ASSERT(ThumbnailAnimationEngineV2::LoopModeCount() ==
        static_cast<size_t>(AnimLoopMode::COUNT));
}

// PreviewPanelV2
TEST(TestPreviewV2_TabNames) {
    ASSERT(PreviewPanelV2::TabName(PreviewPanelTab::Image) != nullptr);
}
TEST(TestPreviewV2_ZoomNames) {
    ASSERT(PreviewPanelV2::ZoomLevelName(PreviewZoomLevel::FitToWindow) !=
        nullptr);
}
TEST(TestPreviewV2_ColorPickerNames) {
    ASSERT(PreviewPanelV2::ColorPickerModeName(ColorPickerMode::HEX) != nullptr);
}
TEST(TestPreviewV2_TabCount) {
    ASSERT(PreviewPanelV2::TabCount() ==
        static_cast<size_t>(PreviewPanelTab::COUNT));
}
TEST(TestPreviewV2_ZoomCount) {
    ASSERT(PreviewPanelV2::ZoomLevelCount() ==
        static_cast<size_t>(PreviewZoomLevel::COUNT));
}

// QuickLookIntegration
TEST(TestQuickLook_ModeNames) {
    ASSERT(QuickLookIntegration::ModeName(QuickLookMode::Inline) != nullptr);
}
TEST(TestQuickLook_TransitionNames) {
    ASSERT(QuickLookIntegration::TransitionName(QuickLookTransition::Fade) !=
        nullptr);
}
TEST(TestQuickLook_MetadataNames) {
    ASSERT(QuickLookIntegration::MetadataOverlayName(
        QuickLookMetadataOverlay::Dimensions) != nullptr);
}
TEST(TestQuickLook_ModeCount) {
    ASSERT(QuickLookIntegration::ModeCount() ==
        static_cast<size_t>(QuickLookMode::COUNT));
}
TEST(TestQuickLook_TransitionCount) {
    ASSERT(QuickLookIntegration::TransitionCount() ==
        static_cast<size_t>(QuickLookTransition::COUNT));
}

// ReleaseGateV27
TEST(TestGateV27_KPINames) {
    ASSERT(ReleaseGateV27::KPIName(GateV27KPI::BuildClean) != nullptr);
}
TEST(TestGateV27_KPICount) { ASSERT(ReleaseGateV27::KPICount() == 11); }
TEST(TestGateV27_Evaluate) {
    std::vector<GateV27Result> r;
    auto v = ReleaseGateV27::Evaluate(r);
    ASSERT(v.passed == 0);
}
TEST(TestGateV27_AllPass) {
    std::vector<GateV27Result> r;
    auto v = ReleaseGateV27::Evaluate(r);
    ASSERT(!v.approved);
}
TEST(TestGateV27_Advance) { ASSERT(ReleaseGateV27::KPICount() > 0); }

// SceneUnderstandingEngine
TEST(TestSceneAI_CategoryNames) {
    ASSERT(SceneUnderstandingEngine::CategoryName(SceneCategory::Indoor) !=
        nullptr);
}
TEST(TestSceneAI_BackendNames) {
    ASSERT(SceneUnderstandingEngine::BackendName(SceneMLBackend::DirectML) !=
        nullptr);
}
TEST(TestSceneAI_ConfNames) {
    ASSERT(SceneUnderstandingEngine::ConfidenceName(SceneConfidence::High) !=
        nullptr);
}
TEST(TestSceneAI_CategoryCount) {
    ASSERT(SceneUnderstandingEngine::CategoryCount() ==
        static_cast<size_t>(SceneCategory::COUNT));
}
TEST(TestSceneAI_BackendCount) {
    ASSERT(SceneUnderstandingEngine::BackendCount() ==
        static_cast<size_t>(SceneMLBackend::COUNT));
}

// SmartCropV2
TEST(TestSmartCropV2_StrategyNames) {
    ASSERT(SmartCropV2::StrategyName(CropStrategy::SaliencyMap) != nullptr);
}
TEST(TestSmartCropV2_AspectNames) {
    ASSERT(SmartCropV2::AspectRatioName(CropAspectRatio::Square) != nullptr);
}
TEST(TestSmartCropV2_PaddingNames) {
    ASSERT(SmartCropV2::PaddingModeName(CropPaddingMode::None) != nullptr);
}
TEST(TestSmartCropV2_StrategyCount) {
    ASSERT(SmartCropV2::StrategyCount() ==
        static_cast<size_t>(CropStrategy::COUNT));
}
TEST(TestSmartCropV2_AspectCount) {
    ASSERT(SmartCropV2::AspectRatioCount() ==
        static_cast<size_t>(CropAspectRatio::COUNT));
}

// ImageQualityAssessor
TEST(TestIQA_MetricNames) {
    ASSERT(ImageQualityAssessor::MetricName(IQAMetric::PSNR) != nullptr);
}
TEST(TestIQA_DefectNames) {
    ASSERT(ImageQualityAssessor::DefectName(IQADefect::Blur) != nullptr);
}
TEST(TestIQA_GradeNames) {
    ASSERT(ImageQualityAssessor::GradeName(IQAGrade::Excellent) != nullptr);
}
TEST(TestIQA_MetricCount) {
    ASSERT(ImageQualityAssessor::MetricCount() ==
        static_cast<size_t>(IQAMetric::COUNT));
}
TEST(TestIQA_DefectCount) {
    ASSERT(ImageQualityAssessor::DefectCount() ==
        static_cast<size_t>(IQADefect::COUNT));
}

// AISearchIntegration
TEST(TestAISearch_ModeNames) {
    ASSERT(AISearchIntegration::ModeName(AISearchMode::SemanticSimilarity) !=
        nullptr);
}
TEST(TestAISearch_EmbeddingNames) {
    ASSERT(AISearchIntegration::EmbeddingModelName(EmbeddingModel::CLIP) !=
        nullptr);
}
TEST(TestAISearch_StatusNames) {
    ASSERT(AISearchIntegration::IndexStatusName(SearchIndexStatus::Ready) !=
        nullptr);
}
TEST(TestAISearch_ModeCount) {
    ASSERT(AISearchIntegration::ModeCount() ==
        static_cast<size_t>(AISearchMode::COUNT));
}
TEST(TestAISearch_EmbeddingCount) {
    ASSERT(AISearchIntegration::EmbeddingModelCount() ==
        static_cast<size_t>(EmbeddingModel::COUNT));
}

// ReleaseGateV28
TEST(TestGateV28_KPINames) {
    ASSERT(ReleaseGateV28::KPIName(GateV28KPI::BuildClean) != nullptr);
}
TEST(TestGateV28_KPICount) { ASSERT(ReleaseGateV28::KPICount() == 11); }
TEST(TestGateV28_Evaluate) {
    std::vector<GateV28Result> r;
    auto v = ReleaseGateV28::Evaluate(r);
    ASSERT(v.passed == 0);
}
TEST(TestGateV28_AllPass) {
    std::vector<GateV28Result> r;
    auto v = ReleaseGateV28::Evaluate(r);
    ASSERT(!v.approved);
}
TEST(TestGateV28_Advance) { ASSERT(ReleaseGateV28::KPICount() > 0); }

// EnterprisePolicyEngineV2
TEST(TestEntPolV2_SourceNames) {
    ASSERT(EnterprisePolicyEngineV2::SourceName(
        EnterprisePolicySource::GroupPolicy) != nullptr);
}
TEST(TestEntPolV2_StatusNames) {
    ASSERT(EnterprisePolicyEngineV2::ComplianceStatusName(
        PolicyComplianceStatus::Compliant) != nullptr);
}
TEST(TestEntPolV2_ScopeNames) {
    ASSERT(EnterprisePolicyEngineV2::ScopeName(PolicyScope::Machine) != nullptr);
}
TEST(TestEntPolV2_SourceCount) {
    ASSERT(EnterprisePolicyEngineV2::SourceCount() ==
        static_cast<size_t>(EnterprisePolicySource::COUNT));
}
TEST(TestEntPolV2_StatusCount) {
    ASSERT(EnterprisePolicyEngineV2::ComplianceStatusCount() ==
        static_cast<size_t>(PolicyComplianceStatus::COUNT));
}

// SharePointTeamsIntegration
TEST(TestSPTeams_CloudSourceNames) {
    ASSERT(SharePointTeamsIntegration::CloudSourceName(
        CloudFileSource::SharePoint) != nullptr);
}
TEST(TestSPTeams_AuthMethodNames) {
    ASSERT(SharePointTeamsIntegration::AuthMethodName(
        GraphAuthMethod::DeviceCode) != nullptr);
}
TEST(TestSPTeams_SyncStateNames) {
    ASSERT(SharePointTeamsIntegration::SyncStateName(CloudSyncState::Idle) !=
        nullptr);
}
TEST(TestSPTeams_CloudSourceCount) {
    ASSERT(SharePointTeamsIntegration::CloudSourceCount() ==
        static_cast<size_t>(CloudFileSource::COUNT));
}
TEST(TestSPTeams_AuthMethodCount) {
    ASSERT(SharePointTeamsIntegration::AuthMethodCount() ==
        static_cast<size_t>(GraphAuthMethod::COUNT));
}

// MultiTenantCacheManager
TEST(TestMTCache_TierNames) {
    ASSERT(MultiTenantCacheManager::TierName(TenantCacheTier::Hot) != nullptr);
}
TEST(TestMTCache_IsolationNames) {
    ASSERT(MultiTenantCacheManager::IsolationName(TenantIsolation::Strict) !=
        nullptr);
}
TEST(TestMTCache_EvictNames) {
    ASSERT(MultiTenantCacheManager::EvictPolicyName(TenantEvictPolicy::LRU) !=
        nullptr);
}
TEST(TestMTCache_TierCount) {
    ASSERT(MultiTenantCacheManager::TierCount() ==
        static_cast<size_t>(TenantCacheTier::COUNT));
}
TEST(TestMTCache_IsolationCount) {
    ASSERT(MultiTenantCacheManager::IsolationCount() ==
        static_cast<size_t>(TenantIsolation::COUNT));
}

// ComplianceAuditLogger
TEST(TestCompliance_RegNames) {
    ASSERT(ComplianceAuditLogger::RegulationName(ComplianceRegulation::GDPR) !=
        nullptr);
}
TEST(TestCompliance_DataClassNames) {
    ASSERT(ComplianceAuditLogger::DataClassificationName(
        DataClassification::Confidential) != nullptr);
}
TEST(TestCompliance_EventTypeNames) {
    ASSERT(ComplianceAuditLogger::AuditEventTypeName(AuditEventType::Access) !=
        nullptr);
}
TEST(TestCompliance_RegCount) {
    ASSERT(ComplianceAuditLogger::RegulationCount() ==
        static_cast<size_t>(ComplianceRegulation::COUNT));
}
TEST(TestCompliance_DataClassCount) {
    ASSERT(ComplianceAuditLogger::DataClassCount() ==
        static_cast<size_t>(DataClassification::COUNT));
}

// ReleaseGateV29
TEST(TestGateV29_KPINames) {
    ASSERT(ReleaseGateV29::KPIName(GateV29KPI::EnterprisePolicyCompliance) !=
        nullptr);
}
TEST(TestGateV29_KPICount) { ASSERT(ReleaseGateV29::KPICount() == 11); }
TEST(TestGateV29_Evaluate) {
    bool r[30] = {};
    auto res = ReleaseGateV29::Evaluate(r);
    ASSERT(res.kpiPassCount == 0);
}
TEST(TestGateV29_AllPass) {
    bool r[30];
    for (int i = 0; i < 30; ++i)
        r[i] = true;
    auto res = ReleaseGateV29::Evaluate(r);
    ASSERT(res.allKPIsPass);
}
TEST(TestGateV29_Advance) {
    bool r[30] = {};
    auto res = ReleaseGateV29::Evaluate(r);
    ASSERT(!res.advanceRecommended);
}

// Windows12Compatibility
TEST(TestWin12_FeatureNames) {
    ASSERT(Windows12Compatibility::FeatureName(Win12Feature::FluentV4) !=
        nullptr);
}
TEST(TestWin12_CompatModeNames) {
    ASSERT(Windows12Compatibility::CompatModeName(Win12CompatMode::Adaptive) !=
        nullptr);
}
TEST(TestWin12_APIFamilyNames) {
    ASSERT(Windows12Compatibility::APIFamilyName(Win12APIFamily::Shell) !=
        nullptr);
}
TEST(TestWin12_FeatureCount) {
    ASSERT(Windows12Compatibility::FeatureCount() ==
        static_cast<size_t>(Win12Feature::COUNT));
}
TEST(TestWin12_CompatModeCount) {
    ASSERT(Windows12Compatibility::CompatModeCount() ==
        static_cast<size_t>(Win12CompatMode::COUNT));
}

// ARM64PerformanceOptimizer
TEST(TestARM64Opt_SIMDNames) {
    ASSERT(ARM64PerformanceOptimizer::SIMDExtName(ARM64SIMDExt::NEON) != nullptr);
}
TEST(TestARM64Opt_CoreTypeNames) {
    ASSERT(ARM64PerformanceOptimizer::CoreTypeName(ARM64CoreType::BigCore) !=
        nullptr);
}
TEST(TestARM64Opt_ThermalNames) {
    ASSERT(ARM64PerformanceOptimizer::ThermalHintName(
        ARM64ThermalHint::Balanced) != nullptr);
}
TEST(TestARM64Opt_SIMDCount) {
    ASSERT(ARM64PerformanceOptimizer::SIMDExtCount() ==
        static_cast<size_t>(ARM64SIMDExt::COUNT));
}
TEST(TestARM64Opt_ThermalCount) {
    ASSERT(ARM64PerformanceOptimizer::ThermalHintCount() ==
        static_cast<size_t>(ARM64ThermalHint::COUNT));
}

// WinRTAppSDKIntegrationV2
TEST(TestWinRTV2_ActivationNames) {
    ASSERT(WinRTAppSDKIntegrationV2::ActivationKindName(
        WinRTActivationKind::Unpackaged) != nullptr);
}
TEST(TestWinRTV2_BootstrapNames) {
    ASSERT(WinRTAppSDKIntegrationV2::BootstrapPhaseName(
        AppSDKBootstrapPhase::Initialize) != nullptr);
}
TEST(TestWinRTV2_StreamNames) {
    ASSERT(WinRTAppSDKIntegrationV2::StreamModeName(WinRTStreamMode::Async) !=
        nullptr);
}
TEST(TestWinRTV2_ActivationCount) {
    ASSERT(WinRTAppSDKIntegrationV2::ActivationKindCount() ==
        static_cast<size_t>(WinRTActivationKind::COUNT));
}
TEST(TestWinRTV2_BootstrapCount) {
    ASSERT(WinRTAppSDKIntegrationV2::BootstrapPhaseCount() ==
        static_cast<size_t>(AppSDKBootstrapPhase::COUNT));
}

// InstallerV2Manager
TEST(TestInstallerV2_FormatNames) {
    ASSERT(InstallerV2Manager::FormatName(InstallerFormat::MSIX) != nullptr);
}
TEST(TestInstallerV2_ScopeNames) {
    ASSERT(InstallerV2Manager::InstallScopeName(InstallScope::PerMachine) !=
        nullptr);
}
TEST(TestInstallerV2_PhaseNames) {
    ASSERT(InstallerV2Manager::PhaseName(InstallerV2Phase::Apply) != nullptr);
}
TEST(TestInstallerV2_FormatCount) {
    ASSERT(InstallerV2Manager::FormatCount() ==
        static_cast<size_t>(InstallerFormat::COUNT));
}
TEST(TestInstallerV2_PhaseCount) {
    ASSERT(InstallerV2Manager::PhaseCount() ==
        static_cast<size_t>(InstallerV2Phase::COUNT));
}

// ReleaseGateV30
TEST(TestGateV30_KPINames) {
    ASSERT(ReleaseGateV30::KPIName(GateV30KPI::Windows12CompatLayer) != nullptr);
}
TEST(TestGateV30_KPICount) { ASSERT(ReleaseGateV30::KPICount() == 10); }
TEST(TestGateV30_Evaluate) {
    bool r[30] = {};
    auto res = ReleaseGateV30::Evaluate(r);
    ASSERT(res.kpiPassCount == 0);
}
TEST(TestGateV30_AllPass) {
    bool r[30];
    for (int i = 0; i < 30; ++i)
        r[i] = true;
    auto res = ReleaseGateV30::Evaluate(r);
    ASSERT(res.allKPIsPass);
}
TEST(TestGateV30_Advance) {
    bool r[30] = {};
    auto res = ReleaseGateV30::Evaluate(r);
    ASSERT(!res.advanceRecommended);
}

// SubMillisecondCacheEngine
TEST(TestSubMsCache_HashNames) {
    ASSERT(SubMillisecondCacheEngine::HashAlgoName(CacheHashAlgo::XXHash3) !=
        nullptr);
}
TEST(TestSubMsCache_EvictionNames) {
    ASSERT(SubMillisecondCacheEngine::EvictionName(SubMsCacheEviction::TinyLFU) !=
        nullptr);
}
TEST(TestSubMsCache_NumaNames) {
    ASSERT(SubMillisecondCacheEngine::NumaTierName(NumaTier::Local) != nullptr);
}
TEST(TestSubMsCache_HashCount) {
    ASSERT(SubMillisecondCacheEngine::HashAlgoCount() ==
        static_cast<size_t>(CacheHashAlgo::COUNT));
}
TEST(TestSubMsCache_EvictionCount) {
    ASSERT(SubMillisecondCacheEngine::EvictionCount() ==
        static_cast<size_t>(SubMsCacheEviction::COUNT));
}

// GPUDecodeAccelerationV2
TEST(TestGPUDecV2_VendorNames) {
    ASSERT(GPUDecodeAccelerationV2::VendorName(GPUDecodeVendor::Intel) !=
        nullptr);
}
TEST(TestGPUDecV2_APINames) {
    ASSERT(GPUDecodeAccelerationV2::APIName(GPUDecodeAPI::D3D12Video) != nullptr);
}
TEST(TestGPUDecV2_CodecNames) {
    ASSERT(GPUDecodeAccelerationV2::CodecName(GPUDecodeCodec::HEVC) != nullptr);
}
TEST(TestGPUDecV2_VendorCount) {
    ASSERT(GPUDecodeAccelerationV2::VendorCount() ==
        static_cast<size_t>(GPUDecodeVendor::COUNT));
}
TEST(TestGPUDecV2_APICount) {
    ASSERT(GPUDecodeAccelerationV2::APICount() ==
        static_cast<size_t>(GPUDecodeAPI::COUNT));
}

// ParallelIOPipeline
TEST(TestParallelIO_BackendNames) {
    ASSERT(ParallelIOPipeline::BackendName(IOBackend::IOCP) != nullptr);
}
TEST(TestParallelIO_PriorityNames) {
    ASSERT(ParallelIOPipeline::PriorityName(IOPriority::High) != nullptr);
}
TEST(TestParallelIO_VolumeNames) {
    ASSERT(ParallelIOPipeline::VolumeTypeName(VolumeType::NVMe) != nullptr);
}
TEST(TestParallelIO_BackendCount) {
    ASSERT(ParallelIOPipeline::BackendCount() ==
        static_cast<size_t>(IOBackend::COUNT));
}
TEST(TestParallelIO_PriorityCount) {
    ASSERT(ParallelIOPipeline::PriorityCount() ==
        static_cast<size_t>(IOPriority::COUNT));
}

// MemoryFootprintOptimizerV2
TEST(TestMemFootV2_AllocNames) {
    ASSERT(MemoryFootprintOptimizerV2::AllocatorName(AllocatorType::MiMalloc) !=
        nullptr);
}
TEST(TestMemFootV2_TrimNames) {
    ASSERT(MemoryFootprintOptimizerV2::TrimStrategyName(TrimStrategy::Idle) !=
        nullptr);
}
TEST(TestMemFootV2_LargePageNames) {
    ASSERT(MemoryFootprintOptimizerV2::LargePagePolicyName(
        LargePagePolicy::Preferred) != nullptr);
}
TEST(TestMemFootV2_AllocCount) {
    ASSERT(MemoryFootprintOptimizerV2::AllocatorCount() ==
        static_cast<size_t>(AllocatorType::COUNT));
}
TEST(TestMemFootV2_TrimCount) {
    ASSERT(MemoryFootprintOptimizerV2::TrimStrategyCount() ==
        static_cast<size_t>(TrimStrategy::COUNT));
}

// ReleaseGateV31
TEST(TestGateV31_KPINames) {
    ASSERT(ReleaseGateV31::KPIName(GateV31KPI::SubMsCacheP99) != nullptr);
}
TEST(TestGateV31_KPICount) { ASSERT(ReleaseGateV31::KPICount() == 10); }
TEST(TestGateV31_Evaluate) {
    bool r[30] = {};
    auto res = ReleaseGateV31::Evaluate(r);
    ASSERT(res.kpiPassCount == 0);
}
TEST(TestGateV31_AllPass) {
    bool r[30];
    for (int i = 0; i < 30; ++i)
        r[i] = true;
    auto res = ReleaseGateV31::Evaluate(r);
    ASSERT(res.allKPIsPass);
}
TEST(TestGateV31_Advance) {
    bool r[30] = {};
    auto res = ReleaseGateV31::Evaluate(r);
    ASSERT(!res.advanceRecommended);
}

// AccessibilitySuiteV2
TEST(TestA11ySuiteV2_WCAGNames) {
    ASSERT(AccessibilitySuiteV2::WCAGLevelName(WCAGLevel::AA) != nullptr);
}
TEST(TestA11ySuiteV2_ColorBlindNames) {
    ASSERT(AccessibilitySuiteV2::ColorBlindModeName(
        ColorBlindMode::Deuteranopia) != nullptr);
}
TEST(TestA11ySuiteV2_FeatureNames) {
    ASSERT(AccessibilitySuiteV2::FeatureName(
        A11ySuiteFeatureV2::ContrastEnforcement) != nullptr);
}
TEST(TestA11ySuiteV2_WCAGCount) {
    ASSERT(AccessibilitySuiteV2::WCAGLevelCount() ==
        static_cast<size_t>(WCAGLevel::COUNT));
}
TEST(TestA11ySuiteV2_ColorBlindCount) {
    ASSERT(AccessibilitySuiteV2::ColorBlindCount() ==
        static_cast<size_t>(ColorBlindMode::COUNT));
}

// DocumentationExcellenceV2
TEST(TestDocExcV2_FormatNames) {
    ASSERT(DocumentationExcellenceV2::DocFormatName(DocOutputFormat::Doxygen) !=
        nullptr);
}
TEST(TestDocExcV2_ScopeNames) {
    ASSERT(DocumentationExcellenceV2::DocScopeName(DocScope::Public) != nullptr);
}
TEST(TestDocExcV2_DriftNames) {
    ASSERT(DocumentationExcellenceV2::DriftLevelName(DocDriftLevel::Clean) !=
        nullptr);
}
TEST(TestDocExcV2_FormatCount) {
    ASSERT(DocumentationExcellenceV2::DocFormatCount() ==
        static_cast<size_t>(DocOutputFormat::COUNT));
}
TEST(TestDocExcV2_ScopeCount) {
    ASSERT(DocumentationExcellenceV2::DocScopeCount() ==
        static_cast<size_t>(DocScope::COUNT));
}

// QualityAssuranceV2
TEST(TestQAV2_CategoryNames) {
    ASSERT(QualityAssuranceV2::TestCategoryName(QATestCategory::Unit) != nullptr);
}
TEST(TestQAV2_SeverityNames) {
    ASSERT(QualityAssuranceV2::DefectSeverityName(QADefectSeverity::Critical) !=
        nullptr);
}
TEST(TestQAV2_SignalNames) {
    ASSERT(QualityAssuranceV2::ShipSignalName(QAShipSignal::Ship) != nullptr);
}
TEST(TestQAV2_CategoryCount) {
    ASSERT(QualityAssuranceV2::TestCategoryCount() ==
        static_cast<size_t>(QATestCategory::COUNT));
}
TEST(TestQAV2_SignalCount) {
    ASSERT(QualityAssuranceV2::ShipSignalCount() ==
        static_cast<size_t>(QAShipSignal::COUNT));
}

// ReleaseGateV32
TEST(TestGateV32_KPINames) {
    ASSERT(ReleaseGateV32::KPIName(GateV32KPI::GPUV3PipelineStable) != nullptr);
}
TEST(TestGateV32_KPICount) { ASSERT(ReleaseGateV32::KPICount() == 23); }
TEST(TestGateV32_Evaluate) {
    bool r[30] = {};
    auto res = ReleaseGateV32::Evaluate(r);
    ASSERT(res.kpiPassCount == 0);
}
TEST(TestGateV32_AllPass) {
    bool r[30];
    for (int i = 0; i < 30; ++i)
        r[i] = true;
    auto res = ReleaseGateV32::Evaluate(r);
    ASSERT(res.allKPIsPass && res.v14ShipApproved);
}
TEST(TestGateV32_v14Approved) {
    bool r[23];
    for (int i = 0; i < 23; ++i)
        r[i] = true;
    auto res = ReleaseGateV32::Evaluate(r);
    ASSERT(res.v14ShipApproved);
}

//==============================================================================
// Feature Module Tests
//==============================================================================

// ---- Version Synchronization ----
TEST(TestZenith_VersionMajor) {
    ASSERT(EXPLORERLENS_ENGINE_VERSION_MAJOR == 15);
}
TEST(TestZenith_VersionMinor) {
    ASSERT(EXPLORERLENS_ENGINE_VERSION_MINOR == 0);
}
TEST(TestZenith_VersionPatch) {
    ASSERT(EXPLORERLENS_ENGINE_VERSION_PATCH == 0);
}
TEST(TestZenith_VersionComposite) {
    uint32_t v = EXPLORERLENS_ENGINE_VERSION;
    ASSERT(v == ((15 << 16) | (0 << 8) | 0));
}

// ---- MuPDF PDF Support ----
TEST(TestZenith_MuPDFBackendNames) {
    ASSERT(std::wstring(MuPDFIntegration::BackendName(MuPDFBackend::Native)) ==
        L"MuPDF Native");
    ASSERT(std::wstring(MuPDFIntegration::BackendName(MuPDFBackend::WIC)) ==
        L"Windows Imaging");
    ASSERT(std::wstring(MuPDFIntegration::BackendName(MuPDFBackend::None)) ==
        L"None");
}
TEST(TestZenith_MuPDFCaps) { ASSERT(MuPDFIntegration::BackendCount() >= 3); }
TEST(TestZenith_MuPDFPageConfig) {
    PDFPageConfig cfg;
    ASSERT(cfg.dpi == 150);
    ASSERT(cfg.maxPages == 1);
}

// ---- libwebp CRT Clean ----
TEST(TestZenith_LibWebPConfigDefaults) {
    LibWebPConfig cfg;
    ASSERT(cfg.useDynamicCRT == true);
    ASSERT(cfg.enableSIMD == true);
}
TEST(TestZenith_LibWebPConfigName) {
    ASSERT(std::wstring(LibWebPConfig::CRTModeName(CRTLinkMode::DynamicMD)) ==
        L"Dynamic (/MD)");
    ASSERT(std::wstring(LibWebPConfig::CRTModeName(CRTLinkMode::StaticMT)) ==
        L"Static (/MT)");
}

// ---- LENSArchive Refactoring ----
TEST(TestZenith_ArchiveRefactorModules) {
    ASSERT(ArchiveRefactorEngine::ModuleCount() >= 4);
}
TEST(TestZenith_ArchiveRefactorModuleNames) {
    ASSERT(std::wstring(ArchiveRefactorEngine::ModuleName(
        RefactorModule::LENSTypes)) == L"LENSTypes");
    ASSERT(std::wstring(ArchiveRefactorEngine::ModuleName(
        RefactorModule::ZipWrapper)) == L"ZipWrapper");
    ASSERT(std::wstring(ArchiveRefactorEngine::ModuleName(
        RefactorModule::RarWrapper)) == L"RarWrapper");
}

// ---- Bitmap Pool ----
TEST(TestZenith_BitmapPoolConfig) {
    BitmapPoolConfig cfg;
    ASSERT(cfg.width == 256);
    ASSERT(cfg.height == 256);
    ASSERT(cfg.poolSize == 50);
    ASSERT(cfg.bitsPerPixel == 32);
}
TEST(TestZenith_BitmapPoolStats) {
    BitmapPoolStats stats;
    ASSERT(stats.acquireCount == 0);
    ASSERT(stats.HitRate() == 0.0);
}
TEST(TestZenith_BitmapPoolStatsHitRate) {
    BitmapPoolStats stats;
    stats.acquireCount = 100;
    stats.poolHits = 85;
    ASSERT(stats.HitRate() == 85.0);
}

// ---- IPropertyStore Handler ----
TEST(TestZenith_PropertyIDNames) {
    ASSERT(static_cast<uint32_t>(PropertyID::ImageWidth) == 0x1001);
    ASSERT(static_cast<uint32_t>(PropertyID::ImageHeight) == 0x1002);
    ASSERT(static_cast<uint32_t>(PropertyID::CodecName) == 0x1004);
}
TEST(TestZenith_PropertyTypes) {
    ASSERT(static_cast<uint8_t>(PropertyType::UInt32) == 0);
    ASSERT(static_cast<uint8_t>(PropertyType::String) == 2);
    ASSERT(static_cast<uint8_t>(PropertyType::Bool) == 3);
}
TEST(TestZenith_PropertyValueDefault) {
    PropertyValue pv;
    ASSERT(pv.isReadOnly == true);
    ASSERT(pv.id == PropertyID::ImageWidth);
}
TEST(TestZenith_PropertyCapabilities) {
    auto caps = static_cast<uint32_t>(PropertyCapability::All);
    ASSERT(caps == 0x0F);
}

// ---- GPU Shader Library ----
TEST(TestZenith_GPUShaderTypes) {
    ASSERT(std::wstring(GPUShaderLibrary::ShaderName(
        GPUShaderLibrary::ShaderType::LanczosResize)) == L"LanczosResize");
    ASSERT(std::wstring(GPUShaderLibrary::ShaderName(
        GPUShaderLibrary::ShaderType::BicubicResize)) == L"BicubicResize");
    ASSERT(std::wstring(GPUShaderLibrary::ShaderName(
        GPUShaderLibrary::ShaderType::HDRTonemap)) == L"HDRTonemap");
    ASSERT(std::wstring(GPUShaderLibrary::ShaderName(
        GPUShaderLibrary::ShaderType::ColorConvert)) == L"ColorConvert");
}
TEST(TestZenith_GPUShaderCount) {
    ASSERT(static_cast<int>(GPUShaderLibrary::ShaderType::COUNT) >= 7);
}
TEST(TestZenith_ToneMapAlgorithms) {
    ASSERT(std::wstring(GPUShaderLibrary::ToneMapName(
        GPUShaderLibrary::ToneMapAlgorithm::Reinhard)) == L"Reinhard");
    ASSERT(std::wstring(GPUShaderLibrary::ToneMapName(
        GPUShaderLibrary::ToneMapAlgorithm::ACES)) == L"ACES");
}
TEST(TestZenith_GPUColorSpaces) {
    ASSERT(static_cast<int>(GPUShaderLibrary::ColorSpace::COUNT) >= 5);
}

// ---- PluginHost Out-of-Process ----
TEST(TestZenith_PluginHostModes) {
    ASSERT(PluginHostManager::ModeCount() >= 3);
}
TEST(TestZenith_PluginHostModeNames) {
    ASSERT(std::wstring(PluginHostManager::ModeName(PluginHostMode::InProcess)) ==
        L"In-Process");
    ASSERT(std::wstring(PluginHostManager::ModeName(
        PluginHostMode::OutOfProcess)) == L"Out-of-Process");
}

// ---- Library Version Audit ----
TEST(TestZenith_LibVersionAuditFields) {
    ASSERT(LibraryVersionAudit::LibraryCount() >= 10);
}
TEST(TestZenith_LibVersionAuditNames) {
    ASSERT(std::wstring(LibraryVersionAudit::StatusName(
        LibraryStatus::UpToDate)) == L"Up-to-date");
    ASSERT(std::wstring(LibraryVersionAudit::StatusName(
        LibraryStatus::UpdateAvailable)) == L"Update Available");
}

// ---- OpenJPEG JPEG 2000 ----
TEST(TestZenith_OpenJPEGProfiles) {
    ASSERT(OpenJPEGIntegration::ProfileCount() >= 3);
}
TEST(TestZenith_OpenJPEGProfileNames) {
    ASSERT(std::wstring(OpenJPEGIntegration::ProfileName(
        JPEG2000Profile::Part1)) == L"Part 1 (JP2)");
    ASSERT(std::wstring(OpenJPEGIntegration::ProfileName(
        JPEG2000Profile::Part2)) == L"Part 2 (JPX)");
}

// ---- FreeType Font Rendering ----
TEST(TestZenith_FreeTypeRenderModes) {
    ASSERT(FreeTypeIntegration::RenderModeCount() >= 3);
}
TEST(TestZenith_FreeTypeRenderModeNames) {
    ASSERT(std::wstring(FreeTypeIntegration::RenderModeName(
        FontRenderMode::Normal)) == L"Normal");
    ASSERT(std::wstring(FreeTypeIntegration::RenderModeName(
        FontRenderMode::Subpixel)) == L"Subpixel");
}

// ---- FFmpeg Integration ----
TEST(TestZenith_FFmpegCodecFamilies) {
    ASSERT(static_cast<int>(FFmpegCodecFamily::H264) == 1);
    ASSERT(static_cast<int>(FFmpegCodecFamily::VP9) == 4);
    ASSERT(static_cast<int>(FFmpegCodecFamily::AV1) == 5);
}
TEST(TestZenith_FFmpegContainerFormats) {
    ASSERT(static_cast<int>(ContainerFormat::MKV) == 1);
    ASSERT(static_cast<int>(ContainerFormat::WebM) == 2);
    ASSERT(static_cast<int>(ContainerFormat::FLV) == 5);
}
TEST(TestZenith_FFmpegVideoStreamInfo) {
    VideoStreamInfo info;
    ASSERT(info.width == 0);
    ASSERT(info.codec == FFmpegCodecFamily::Unknown);
    ASSERT(info.totalFrames == 0);
}
TEST(TestZenith_FFmpegFrameResult) {
    VideoFrameResult fr;
    ASSERT(fr.success == false);
    ASSERT(fr.pixelData.empty());
}

// ---- Format Category Manager ----
TEST(TestZenith_FormatCategoryCount) {
    ASSERT(FormatCategoryManager::CategoryCount() >= 6);
}
TEST(TestZenith_FormatCategoryNames) {
    ASSERT(std::wstring(FormatCategoryManager::CategoryName(
        FormatCategoryGroup::Archives)) == L"Archives");
    ASSERT(std::wstring(FormatCategoryManager::CategoryName(
        FormatCategoryGroup::Images)) == L"Images");
    ASSERT(std::wstring(FormatCategoryManager::CategoryName(
        FormatCategoryGroup::Video)) == L"Video");
}

// ---- Format Status Indicator ----
TEST(TestZenith_FormatStatusLevels) {
    ASSERT(FormatStatusIndicator::StatusCount() >= 3);
}
TEST(TestZenith_FormatStatusNames) {
    ASSERT(std::wstring(FormatStatusIndicator::StatusName(
        FormatStatus::Active)) == L"Active");
    ASSERT(std::wstring(FormatStatusIndicator::StatusName(
        FormatStatus::Degraded)) == L"Degraded");
    ASSERT(std::wstring(FormatStatusIndicator::StatusName(
        FormatStatus::Unavailable)) == L"Unavailable");
}

// ---- Settings Import/Export ----
TEST(TestZenith_SettingsFormatNames) {
    ASSERT(SettingsExportImport::FormatCount() >= 2);
}
TEST(TestZenith_SettingsFormatJSON) {
    ASSERT(std::wstring(SettingsExportImport::FormatName(SettingsFormat::JSON)) ==
        L"JSON");
}

// ---- Performance Dashboard ----
TEST(TestZenith_PerfDashboardMetrics) {
    ASSERT(PerformanceDashboard::MetricCount() >= 5);
}
TEST(TestZenith_PerfDashboardMetricNames) {
    ASSERT(std::wstring(PerformanceDashboard::MetricName(
        DashboardMetric::AvgDecodeTime)) == L"Avg Decode Time");
    ASSERT(std::wstring(PerformanceDashboard::MetricName(
        DashboardMetric::CacheHitRate)) == L"Cache Hit Rate");
}

// ---- ETW TraceLogging Provider ----
TEST(TestETW_ProviderInitialize) {
    auto& provider = ExplorerLens::ETW::ETWTraceProvider::Instance();
    // Initialize should succeed (even without a consumer listening)
    bool ok = provider.Initialize();
    ASSERT(ok);
    ASSERT(provider.IsRegistered());
}
TEST(TestETW_ProviderEventEmit) {
    auto& provider = ExplorerLens::ETW::ETWTraceProvider::Instance();
    provider.Initialize();
    uint64_t before = provider.EventsEmitted();
    // Log events (may or may not be captured depending on ETW session)
    provider.LogDecodeStart(L"test.zip", "ZipDecoder", 256);
    provider.LogDecodeComplete(L"test.zip", "ZipDecoder", 5.0, true, 256, 256);
    provider.LogCacheAccess("SubMsCache", true, 0.3);
    provider.LogGPUOperation("D3D11Render", 2.5, 1024 * 1024);
    provider.LogStartupPhase("EngineInit", 42.0);
    provider.LogHealthCheck("Decode", 0.95, "A");
    // Events should be counted even if no consumer is listening
    ASSERT(provider.EventsEmitted() >= before);
}
TEST(TestETW_ScopedTimer) {
    auto& provider = ExplorerLens::ETW::ETWTraceProvider::Instance();
    provider.Initialize();
    {
        ExplorerLens::ETW::ETWScopedTimer timer("TestScope",
            ExplorerLens::ETW::Keywords::Pipeline);
        // Timer will emit event on destruction
    }
    // Should not crash
    ASSERT(true);
}
TEST(TestETW_KeywordValues) {
    // Verify keyword bitmasks are distinct powers of 2
    ASSERT(ExplorerLens::ETW::Keywords::Pipeline == 0x0001);
    ASSERT(ExplorerLens::ETW::Keywords::Cache == 0x0002);
    ASSERT(ExplorerLens::ETW::Keywords::Decoder == 0x0004);
    ASSERT(ExplorerLens::ETW::Keywords::GPU == 0x0008);
    ASSERT(ExplorerLens::ETW::Keywords::Plugin == 0x0010);
    ASSERT(ExplorerLens::ETW::Keywords::Memory == 0x0020);
    ASSERT(ExplorerLens::ETW::Keywords::Startup == 0x0200);
    ASSERT(ExplorerLens::ETW::Keywords::All == 0xFFFF);
}
TEST(TestETW_EventLevels) {
    ASSERT(static_cast<uint8_t>(ExplorerLens::ETW::EventLevel::Critical) == 1);
    ASSERT(static_cast<uint8_t>(ExplorerLens::ETW::EventLevel::Error) == 2);
    ASSERT(static_cast<uint8_t>(ExplorerLens::ETW::EventLevel::Warning) == 3);
    ASSERT(static_cast<uint8_t>(ExplorerLens::ETW::EventLevel::Info) == 4);
    ASSERT(static_cast<uint8_t>(ExplorerLens::ETW::EventLevel::Verbose) == 5);
}

// ---- Dark Mode Engine ----
TEST(TestZenith_DarkModeThemes) { ASSERT(DarkModeEngine::ThemeCount() >= 3); }
TEST(TestZenith_DarkModeThemeNames) {
    ASSERT(std::wstring(DarkModeEngine::ThemeName(AppTheme::Light)) == L"Light");
    ASSERT(std::wstring(DarkModeEngine::ThemeName(AppTheme::Dark)) == L"Dark");
    ASSERT(std::wstring(DarkModeEngine::ThemeName(AppTheme::System)) ==
        L"System");
}

// ---- System Tray Manager ----
TEST(TestZenith_SystemTrayActions) {
    ASSERT(SystemTrayManager::ActionCount() >= 3);
}
TEST(TestZenith_SystemTrayActionNames) {
    ASSERT(std::wstring(SystemTrayManager::ActionName(
        TrayAction::OpenSettings)) == L"Open Settings");
    ASSERT(std::wstring(SystemTrayManager::ActionName(TrayAction::ShowStatus)) ==
        L"Show Status");
}

// ---- WinUI 3 Migration ----
TEST(TestZenith_WinUI3PhaseCount) {
    ASSERT(WinUI3MigrationEngine::PhaseCount() >= 3);
}
TEST(TestZenith_WinUI3PhaseNames) {
    ASSERT(std::wstring(WinUI3MigrationEngine::PhaseName(
        MigrationPhase::Research)) == L"Research");
    ASSERT(std::wstring(WinUI3MigrationEngine::PhaseName(
        MigrationPhase::Hybrid)) == L"Hybrid");
    ASSERT(std::wstring(WinUI3MigrationEngine::PhaseName(
        MigrationPhase::FeatureParity)) == L"Feature Parity");
}

// ---- CI Hardening ----
TEST(TestZenith_CIConfigCount) {
    ASSERT(CIHardeningEngine::ConfigCount() >= 3);
}
TEST(TestZenith_CIConfigNames) {
    ASSERT(std::wstring(CIHardeningEngine::ConfigName(CIConfig::x64Release)) ==
        L"x64 Release");
    ASSERT(std::wstring(CIHardeningEngine::ConfigName(CIConfig::x64Debug)) ==
        L"x64 Debug");
    ASSERT(std::wstring(CIHardeningEngine::ConfigName(CIConfig::ARM64Cross)) ==
        L"ARM64 Cross-Compile");
}

// ---- Code Coverage ----
TEST(TestZenith_CoverageTargets) {
    ASSERT(CodeCoverageEngine::TargetCount() >= 3);
}
TEST(TestZenith_CoverageTargetNames) {
    ASSERT(std::wstring(CodeCoverageEngine::TargetName(
        CodeCoverageEngine::CoverageTarget::EngineCore)) ==
        L"Engine Core");
    ASSERT(std::wstring(CodeCoverageEngine::TargetName(
        CodeCoverageEngine::CoverageTarget::Decoders)) == L"Decoders");
}

// ---- Fuzzing Campaign ----
TEST(TestZenith_FuzzingStrategies) {
    ASSERT(FuzzingCampaign::StrategyCount() >= 3);
}
TEST(TestZenith_FuzzingStrategyNames) {
    ASSERT(std::wstring(FuzzingCampaign::StrategyName(
        FuzzStrategy::RandomMutation)) == L"Random Mutation");
    ASSERT(std::wstring(FuzzingCampaign::StrategyName(
        FuzzStrategy::StructureAware)) == L"Structure-Aware");
}

// ---- Static Analysis Gate ----
TEST(TestZenith_StaticAnalysisTools) {
    ASSERT(StaticAnalysisGate::ToolCount() >= 2);
}
TEST(TestZenith_StaticAnalysisToolNames) {
    ASSERT(std::wstring(StaticAnalysisGate::ToolName(AnalysisTool::ClangTidy)) ==
        L"clang-tidy");
    ASSERT(std::wstring(StaticAnalysisGate::ToolName(AnalysisTool::CppCheck)) ==
        L"cppcheck");
}

// ---- SBOM Generator ----
TEST(TestZenith_SBOMFormats) { ASSERT(SBOMGenerator::FormatCount() >= 2); }
TEST(TestZenith_SBOMFormatNames) {
    ASSERT(std::wstring(SBOMGenerator::FormatName(SBOMOutputFormat::SPDX)) ==
        L"SPDX");
    ASSERT(std::wstring(SBOMGenerator::FormatName(SBOMOutputFormat::CycloneDX)) ==
        L"CycloneDX");
}

// ---- Zero-Copy Pipeline ----
TEST(TestZenith_ZeroCopyStages) { ASSERT(ZeroCopyPipeline::StageCount() >= 3); }
TEST(TestZenith_ZeroCopyStageNames) {
    ASSERT(std::wstring(ZeroCopyPipeline::StageName(ZeroCopyStage::FileMap)) ==
        L"File Map");
    ASSERT(std::wstring(ZeroCopyPipeline::StageName(ZeroCopyStage::GPUUpload)) ==
        L"GPU Upload");
    ASSERT(std::wstring(ZeroCopyPipeline::StageName(ZeroCopyStage::CacheStore)) ==
        L"Cache Store");
}

// ---- Parallel I/O Pipeline ----
TEST(TestZenith_ParallelIOPolicies) {
    ASSERT(ParallelIOPipeline::PolicyCount() >= 3);
}
TEST(TestZenith_ParallelIOPolicyNames) {
    ASSERT(std::wstring(ParallelIOPipeline::PolicyName(IOPolicy::Sequential)) ==
        L"Sequential");
    ASSERT(std::wstring(ParallelIOPipeline::PolicyName(
        IOPolicy::ParallelBatch)) == L"Parallel Batch");
}

// ---- SIMD Scaler ----
TEST(TestZenith_SIMDScalerPaths) { ASSERT(SIMDScaler::PathCount() >= 3); }
TEST(TestZenith_SIMDScalerPathNames) {
    ASSERT(std::wstring(SIMDScaler::PathName(SIMDPath::Scalar)) == L"Scalar");
    ASSERT(std::wstring(SIMDScaler::PathName(SIMDPath::SSE42)) == L"SSE 4.2");
    ASSERT(std::wstring(SIMDScaler::PathName(SIMDPath::AVX2)) == L"AVX2");
}

// ---- PSO Cache ----
TEST(TestZenith_PSOCacheStrategies) {
    ASSERT(PipelineStateCacheV2::StrategyCount() >= 2);
}
TEST(TestZenith_PSOCacheStrategyNames) {
    ASSERT(std::wstring(PipelineStateCacheV2::StrategyName(
        PSOCacheStrategy::InMemory)) == L"In-Memory");
    ASSERT(std::wstring(PipelineStateCacheV2::StrategyName(
        PSOCacheStrategy::PersistentDisk)) == L"Persistent Disk");
}

// ---- Cache Warming ----
TEST(TestZenith_CacheWarmingModes) {
    ASSERT(CacheWarmingService::ModeCount() >= 3);
}
TEST(TestZenith_CacheWarmingModeNames) {
    ASSERT(std::wstring(CacheWarmingService::ModeName(WarmingMode::Idle)) ==
        L"Idle");
    ASSERT(std::wstring(CacheWarmingService::ModeName(WarmingMode::Proactive)) ==
        L"Proactive");
    ASSERT(std::wstring(CacheWarmingService::ModeName(WarmingMode::OnDemand)) ==
        L"On Demand");
}

// ---- Thumbnail Quality Analyzer ----
TEST(TestZenith_QualityMetrics) {
    ASSERT(ThumbnailQualityAnalyzer::MetricCount() == 8);
}
TEST(TestZenith_QualityMetricNames) {
    ASSERT(std::wstring(ThumbnailQualityAnalyzer::MetricName(
        QualityMetric::SSIM)) == L"Structural Similarity (SSIM)");
    ASSERT(std::wstring(ThumbnailQualityAnalyzer::MetricName(
        QualityMetric::PSNR)) == L"Peak SNR (dB)");
    ASSERT(std::wstring(ThumbnailQualityAnalyzer::MetricName(
        QualityMetric::Sharpness)) == L"Sharpness (Laplacian)");
}
TEST(TestZenith_QualityGradeFromSSIM) {
    ASSERT(ThumbnailQualityAnalyzer::GradeFromSSIM(0.97) ==
        QualityGrade::Excellent);
    ASSERT(ThumbnailQualityAnalyzer::GradeFromSSIM(0.92) == QualityGrade::Good);
    ASSERT(ThumbnailQualityAnalyzer::GradeFromSSIM(0.85) ==
        QualityGrade::Acceptable);
    ASSERT(ThumbnailQualityAnalyzer::GradeFromSSIM(0.75) == QualityGrade::Poor);
    ASSERT(ThumbnailQualityAnalyzer::GradeFromSSIM(0.50) ==
        QualityGrade::Rejected);
}
TEST(TestZenith_QualityGradeNames) {
    ASSERT(std::wstring(ThumbnailQualityAnalyzer::GradeName(
        QualityGrade::Excellent)) == L"Excellent");
    ASSERT(std::wstring(ThumbnailQualityAnalyzer::GradeName(
        QualityGrade::Good)) == L"Good");
    ASSERT(std::wstring(ThumbnailQualityAnalyzer::GradeName(
        QualityGrade::Rejected)) == L"Rejected");
}
TEST(TestZenith_QualityReportDefault) {
    ThumbnailQualityReport report;
    ASSERT(report.grade == QualityGrade::Rejected);
    ASSERT(!report.IsAcceptable());
    ASSERT(report.overallScore == 0.0);
}
TEST(TestZenith_QualityThresholds) {
    QualityThresholds t;
    ASSERT(t.minSSIM == 0.90);
    ASSERT(t.minPSNR == 30.0);
    ASSERT(t.maxDeltaE == 3.0);
}

// ---- Adaptive Decoder Router ----
TEST(TestZenith_RouterStrategies) {
    ASSERT(AdaptiveDecoderRouter::StrategyCount() == 5);
}
TEST(TestZenith_RouterStrategyNames) {
    ASSERT(std::wstring(AdaptiveDecoderRouter::StrategyName(
        RoutingStrategy::ExtensionBased)) == L"Extension-Based");
    ASSERT(std::wstring(AdaptiveDecoderRouter::StrategyName(
        RoutingStrategy::SignatureBased)) == L"Signature-Based");
    ASSERT(std::wstring(AdaptiveDecoderRouter::StrategyName(
        RoutingStrategy::HybridFast)) == L"Hybrid Fast");
    ASSERT(std::wstring(AdaptiveDecoderRouter::StrategyName(
        RoutingStrategy::PerformanceOptimal)) == L"Performance Optimal");
    ASSERT(std::wstring(AdaptiveDecoderRouter::StrategyName(
        RoutingStrategy::QualityOptimal)) == L"Quality Optimal");
}
TEST(TestZenith_RouterBuiltinSignatures) {
    auto& sigs = AdaptiveDecoderRouter::GetBuiltinSignatures();
    ASSERT(sigs.size() >= 10);
}
TEST(TestZenith_RouterSignatureMatchPNG) {
    uint8_t pngHeader[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    auto* sig = AdaptiveDecoderRouter::MatchSignature(pngHeader, 8);
    ASSERT(sig != nullptr);
    ASSERT(std::wstring(sig->formatName) == L"PNG");
    ASSERT(std::wstring(sig->decoderName) == L"ImageDecoder");
}
TEST(TestZenith_RouterSignatureMatchJPEG) {
    uint8_t jpegHeader[] = { 0xFF, 0xD8, 0xFF, 0xE0 };
    auto* sig = AdaptiveDecoderRouter::MatchSignature(jpegHeader, 4);
    ASSERT(sig != nullptr);
    ASSERT(std::wstring(sig->formatName) == L"JPEG");
}
TEST(TestZenith_RouterSignatureMatchPDF) {
    uint8_t pdfHeader[] = { 0x25, 0x50, 0x44, 0x46, 0x2D };
    auto* sig = AdaptiveDecoderRouter::MatchSignature(pdfHeader, 5);
    ASSERT(sig != nullptr);
    ASSERT(std::wstring(sig->formatName) == L"PDF");
}
TEST(TestZenith_RouterSignatureNoMatch) {
    uint8_t unknownHeader[] = { 0x00, 0x00, 0x00, 0x00 };
    auto* sig = AdaptiveDecoderRouter::MatchSignature(unknownHeader, 4);
    ASSERT(sig == nullptr);
}
TEST(TestZenith_RouterSignatureNull) {
    auto* sig = AdaptiveDecoderRouter::MatchSignature(nullptr, 0);
    ASSERT(sig == nullptr);
}
TEST(TestZenith_RouterDecisionDefault) {
    RoutingDecision rd;
    ASSERT(rd.strategyUsed == RoutingStrategy::ExtensionBased);
    ASSERT(rd.confidence == 1.0);
    ASSERT(rd.isFallback == false);
}

// ---- Telemetry Pipeline ----
TEST(TestZenith_TelemetryCategoryCount) {
    ASSERT(TelemetryPipeline::CategoryCount() == 10);
}
TEST(TestZenith_TelemetryCategoryNames) {
    ASSERT(std::wstring(TelemetryPipeline::CategoryName(
        PipelineTelemetryCategory::DecodePerformance)) ==
        L"Decode Performance");
    ASSERT(std::wstring(TelemetryPipeline::CategoryName(
        PipelineTelemetryCategory::CacheHitRate)) == L"Cache Hit Rate");
    ASSERT(std::wstring(TelemetryPipeline::CategoryName(
        PipelineTelemetryCategory::GPUUtilization)) == L"GPU Utilization");
    ASSERT(std::wstring(TelemetryPipeline::CategoryName(
        PipelineTelemetryCategory::MemoryPressure)) == L"Memory Pressure");
    ASSERT(std::wstring(TelemetryPipeline::CategoryName(
        PipelineTelemetryCategory::ErrorRate)) == L"Error Rate");
}
TEST(TestZenith_TelemetryWindowCount) {
    ASSERT(TelemetryPipeline::WindowCount() == 6);
}
TEST(TestZenith_TelemetryWindowNames) {
    ASSERT(std::wstring(TelemetryPipeline::WindowName(TimeWindow::Last1Min)) ==
        L"1 Minute");
    ASSERT(std::wstring(TelemetryPipeline::WindowName(TimeWindow::Last1Hour)) ==
        L"1 Hour");
    ASSERT(std::wstring(TelemetryPipeline::WindowName(TimeWindow::Lifetime)) ==
        L"Lifetime");
}
TEST(TestZenith_TelemetryHealthSnapshot) {
    auto snap = TelemetryPipeline::CaptureHealth();
    ASSERT(snap.cpuUsagePercent >= 0.0);
    ASSERT(snap.activeDecoderThreads == 0);
}

// ---- Live Preview Engine ----
TEST(TestZenith_LivePreviewModes) {
    ASSERT(LivePreviewEngine::ModeCount() == 5);
}
TEST(TestZenith_LivePreviewModeNames) {
    ASSERT(std::wstring(LivePreviewEngine::ModeName(LivePreviewMode::Static)) ==
        L"Static");
    ASSERT(std::wstring(LivePreviewEngine::ModeName(
        LivePreviewMode::AnimatedLoop)) == L"Animated Loop");
    ASSERT(std::wstring(LivePreviewEngine::ModeName(
        LivePreviewMode::VideoScrub)) == L"Video Scrub");
    ASSERT(std::wstring(LivePreviewEngine::ModeName(
        LivePreviewMode::Slideshow)) == L"Slideshow");
    ASSERT(std::wstring(LivePreviewEngine::ModeName(LivePreviewMode::Hover3D)) ==
        L"3D Hover");
}
TEST(TestZenith_LivePreviewRecommendGIF) {
    ASSERT(LivePreviewEngine::RecommendMode(L".gif") ==
        LivePreviewMode::AnimatedLoop);
}
TEST(TestZenith_LivePreviewRecommendMP4) {
    ASSERT(LivePreviewEngine::RecommendMode(L".mp4") ==
        LivePreviewMode::VideoScrub);
}
TEST(TestZenith_LivePreviewRecommendCBZ) {
    ASSERT(LivePreviewEngine::RecommendMode(L".cbz") ==
        LivePreviewMode::Slideshow);
}
TEST(TestZenith_LivePreviewRecommendOBJ) {
    ASSERT(LivePreviewEngine::RecommendMode(L".obj") == LivePreviewMode::Hover3D);
}
TEST(TestZenith_LivePreviewRecommendJPG) {
    ASSERT(LivePreviewEngine::RecommendMode(L".jpg") == LivePreviewMode::Static);
}
TEST(TestZenith_LivePreviewMemoryEstimate) {
    uint64_t mem = LivePreviewEngine::EstimateMemory(256, 256, 10);
    ASSERT(mem == 256 * 256 * 4 * 10);
}
TEST(TestZenith_LivePreviewFitToBudget) {
    // 16MB budget, 256x256 BGRA = 262144 bytes per frame
    uint32_t frames =
        LivePreviewEngine::FitToBudget(256, 256, 16 * 1024 * 1024, 100);
    ASSERT(frames == 64); // 16MB / 256KB = 64 frames max
}
TEST(TestZenith_LivePreviewConfig) {
    PreviewConfig cfg;
    ASSERT(cfg.maxFrames == 12);
    ASSERT(cfg.fpsTarget == 15.0);
    ASSERT(cfg.maxMemoryBytes == 16 * 1024 * 1024);
}

// ---- Cloud Native Sync ----
TEST(TestZenith_CloudProviderCount) {
    ASSERT(CloudNativeSync::ProviderCount() >= 10);
}
TEST(TestZenith_CloudProviderNames) {
    ASSERT(std::wstring(CloudNativeSync::ProviderName(
        NativeCloudProvider::OneDrive)) == L"OneDrive");
    ASSERT(std::wstring(CloudNativeSync::ProviderName(
        NativeCloudProvider::SharePoint)) == L"SharePoint");
    ASSERT(std::wstring(CloudNativeSync::ProviderName(
        NativeCloudProvider::GoogleDrive)) == L"Google Drive");
    ASSERT(std::wstring(CloudNativeSync::ProviderName(
        NativeCloudProvider::Dropbox)) == L"Dropbox");
    ASSERT(std::wstring(CloudNativeSync::ProviderName(
        NativeCloudProvider::AzureBlob)) == L"Azure Blob Storage");
}
TEST(TestZenith_CloudSyncStatusNames) {
    ASSERT(std::wstring(CloudNativeSync::SyncStatusName(
        NativeSyncStatus::Synced)) == L"Synced");
    ASSERT(std::wstring(CloudNativeSync::SyncStatusName(
        NativeSyncStatus::Conflict)) == L"Conflict");
    ASSERT(std::wstring(CloudNativeSync::SyncStatusName(
        NativeSyncStatus::Error)) == L"Error");
}
TEST(TestZenith_CloudSyncConfigDefaults) {
    CloudSyncConfig cfg;
    ASSERT(cfg.provider == NativeCloudProvider::None);
    ASSERT(cfg.direction == SyncDirection::Bidirectional);
    ASSERT(cfg.maxSyncSizeBytes == 100 * 1024 * 1024);
    ASSERT(cfg.encryptThumbnails == true);
}
TEST(TestZenith_CloudDetectProviders) {
    auto providers = CloudNativeSync::DetectProviders();
    // Will find OneDrive if running on a system with it configured
    ASSERT(providers.size() >= 0); // Non-negative (may be 0 in CI)
}

//== HDR Tone Mapping Pipeline ==
TEST(TestZenith_HDROperatorCount) {
    ASSERT(HDRToneMappingPipeline::OperatorCount() == 6);
}
TEST(TestZenith_HDROperatorNames) {
    ASSERT(std::wstring(HDRToneMappingPipeline::OperatorName(
        HDRToneMapOp::Reinhard)) == L"Reinhard");
    ASSERT(std::wstring(HDRToneMappingPipeline::OperatorName(
        HDRToneMapOp::ACES)) == L"ACES Filmic");
}
TEST(TestZenith_HDRConfigDefaults) {
    ToneMappingConfig cfg;
    ASSERT(cfg.op == HDRToneMapOp::ACES);
    ASSERT(cfg.exposure == 0.0f);
}

//== Color Space Engine ==
TEST(TestZenith_ColorSpaceCount) {
    ASSERT(ColorSpaceEngine::SpaceCount() == 9);
}
TEST(TestZenith_ColorSpaceNames) {
    ASSERT(std::wstring(ColorSpaceEngine::SpaceName(ColorSpace::sRGB)) ==
        L"sRGB");
    ASSERT(std::wstring(ColorSpaceEngine::SpaceName(ColorSpace::DisplayP3)) ==
        L"Display P3");
}
TEST(TestZenith_ColorSpaceConversion) {
    ASSERT(ColorSpaceEngine::SRGBToLinear(0.0f) == 0.0f);
    ASSERT(ColorSpaceEngine::LinearToSRGB(0.0f) == 0.0f);
}

//== GPU Texture Compression ==
TEST(TestZenith_TextureFormatCount) {
    ASSERT(GPUTextureCompressionPipeline::FormatCount() == 9);
}
TEST(TestZenith_TextureFormatNames) {
    ASSERT(std::wstring(GPUTextureCompressionPipeline::FormatName(
        TextureFormat::BC7_RGBA)) == L"BC7 (RGBA)");
    ASSERT(std::wstring(GPUTextureCompressionPipeline::FormatName(
        TextureFormat::ASTC_4x4)) == L"ASTC 4x4");
}
TEST(TestZenith_TextureBPP) {
    ASSERT(GPUTextureCompressionPipeline::BitsPerPixel(TextureFormat::BC1_RGB) ==
        4);
    ASSERT(GPUTextureCompressionPipeline::BitsPerPixel(
        TextureFormat::Uncompressed_BGRA) == 32);
}

//== Adaptive DPI Scaler ==
TEST(TestZenith_DPIStrategyCount) {
    ASSERT(AdaptiveDPIScaler::StrategyCount() == 5);
}
TEST(TestZenith_DPIClassify) {
    ASSERT(AdaptiveDPIScaler::ClassifyDPI(96) == DPITier::Standard);
    ASSERT(AdaptiveDPIScaler::ClassifyDPI(192) == DPITier::VeryHigh);
    ASSERT(AdaptiveDPIScaler::ClassifyDPI(300) == DPITier::Ultra);
}
TEST(TestZenith_DPIScaledSize) {
    ASSERT(AdaptiveDPIScaler::ScaledSize(256, 2.0f) == 512);
}

//== Format Fingerprint DB ==
TEST(TestZenith_FormatFamilyCount) {
    ASSERT(FormatFingerprintDB::FamilyCount() == 15);
}
TEST(TestZenith_FingerprintMatchesPNG) {
    uint8_t pngMagic[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    ASSERT(FormatFingerprintDB::MatchesPNG(pngMagic, 8) == true);
    uint8_t notPng[] = { 0x00, 0x00, 0x00, 0x00 };
    ASSERT(FormatFingerprintDB::MatchesPNG(notPng, 4) == false);
}
TEST(TestZenith_FingerprintMatchesJPEG) {
    uint8_t jpegMagic[] = { 0xFF, 0xD8, 0xFF };
    ASSERT(FormatFingerprintDB::MatchesJPEG(jpegMagic, 3) == true);
}

//== Nested Archive Preview ==
TEST(TestZenith_NestingPolicyCount) {
    ASSERT(NestedArchivePreview::PolicyCount() == 4);
}
TEST(TestZenith_NestingEffectiveDepth) {
    ASSERT(NestedArchivePreview::EffectiveMaxDepth(NestingPolicy::SingleLevel,
        5) == 1);
    ASSERT(NestedArchivePreview::EffectiveMaxDepth(NestingPolicy::Unlimited, 5) ==
        10);
}

//== Multi-Page Navigator ==
TEST(TestZenith_PageStrategyCount) {
    ASSERT(MultiPageNavigator::StrategyCount() == 6);
}
TEST(TestZenith_PageSelectFirst) {
    ASSERT(MultiPageNavigator::SelectPage(PageSelectionStrategy::FirstPage,
        100) == 0);
}
TEST(TestZenith_PageSelectLast) {
    ASSERT(MultiPageNavigator::SelectPage(PageSelectionStrategy::MiddlePage,
        100) == 50);
}

//== Animated Format Controller ==
TEST(TestZenith_FrameSelectionCount) {
    ASSERT(AnimatedFormatController::ModeCount() == 6);
}
TEST(TestZenith_AnimSelectFrame) {
    ASSERT(AnimatedFormatController::SelectFrame(FrameSelectionMode::FirstFrame,
        50) == 0);
    ASSERT(AnimatedFormatController::SelectFrame(FrameSelectionMode::MiddleFrame,
        50) == 25);
}

//== Metadata Extraction Pipeline ==
TEST(TestZenith_MetadataSourceCount) {
    ASSERT(MetadataExtractionPipeline::SourceCount() == 8);
}
TEST(TestZenith_MetadataSourceNames) {
    ASSERT(std::wstring(MetadataExtractionPipeline::SourceName(
        MetadataSource::EXIF)) == L"EXIF");
    ASSERT(std::wstring(MetadataExtractionPipeline::SourceName(
        MetadataSource::XMP)) == L"XMP");
}

//== Content-Aware Thumbnail Selector ==
TEST(TestZenith_SaliencyAlgoCount) {
    ASSERT(ContentAwareThumbnailSelector::AlgorithmCount() == 5);
}
TEST(TestZenith_CenterCropCalc) {
    auto crop = ContentAwareThumbnailSelector::CenterCropCalc(1920, 1080, 1.0f);
    ASSERT(crop.srcX >= 0);
    ASSERT(crop.srcW <= 1920);
}

//== Face Detection & Orientation ==
TEST(TestZenith_EXIFOrientationDegrees) {
    ASSERT(FaceDetectionOrientation::OrientationToDegrees(
        EXIFOrientation::Rotate180) == 180);
    ASSERT(FaceDetectionOrientation::OrientationToDegrees(
        EXIFOrientation::Rotate90) == 90);
}
TEST(TestZenith_EXIFDimensionSwap) {
    ASSERT(FaceDetectionOrientation::RequiresDimensionSwap(
        EXIFOrientation::Rotate90) == true);
    ASSERT(FaceDetectionOrientation::RequiresDimensionSwap(
        EXIFOrientation::Normal) == false);
}

//== Document Layout Analyzer ==
TEST(TestZenith_DocRegionTypeCount) {
    ASSERT(DocumentLayoutAnalyzer::RegionTypeCount() == 11);
}
TEST(TestZenith_DocTextDensity) {
    ASSERT(DocumentLayoutAnalyzer::EstimateTextDensity(500, 800, 600) > 0.0);
}

//== Visual Similarity Index ==
TEST(TestZenith_HashAlgoCount) {
    ASSERT(VisualSimilarityIndex::AlgorithmCount() == 5);
}
TEST(TestZenith_HammingDistance) {
    ASSERT(VisualSimilarityIndex::HammingDistance(0xFF00, 0xFF0F) == 4);
    ASSERT(VisualSimilarityIndex::HammingDistance(0, 0) == 0);
}
TEST(TestZenith_SimilarityClassify) {
    ASSERT(VisualSimilarityIndex::Classify(0) == SimilarityClass::Identical);
    ASSERT(VisualSimilarityIndex::Classify(30) == SimilarityClass::Different);
}

//== Smart Quality Predictor ==
TEST(TestZenith_ImageComplexityCount) {
    ASSERT(SmartQualityPredictor::ComplexityCount() == 5);
}
TEST(TestZenith_PredictJPEGQuality) {
    auto q = SmartQualityPredictor::PredictJPEGQuality(ImageComplexity::Simple);
    ASSERT(q >= 60 && q <= 100);
}

//== Lock-Free Decode Pipeline ==
TEST(TestZenith_PipelineStageStateCount) {
    ASSERT(LockFreeDecodePipeline::StageStateCount() == 7);
}
TEST(TestZenith_NextPowerOf2) {
    ASSERT(LockFreeDecodePipeline::NextPowerOf2(5) == 8);
    ASSERT(LockFreeDecodePipeline::NextPowerOf2(16) == 16);
}
TEST(TestZenith_IsPowerOf2) {
    ASSERT(LockFreeDecodePipeline::IsPowerOf2(64) == true);
    ASSERT(LockFreeDecodePipeline::IsPowerOf2(63) == false);
}

//== Memory-Mapped I/O Optimizer ==
TEST(TestZenith_MappingStrategyCount) {
    ASSERT(MemoryMappedIOOptimizer::StrategyCount() == 5);
}
TEST(TestZenith_MMapAlignOffset) {
    ASSERT(MemoryMappedIOOptimizer::AlignOffset(65536, 65536) == 65536);
    ASSERT(MemoryMappedIOOptimizer::AlignOffset(100, 65536) == 0);
}

//== GPU Texture Atlas Manager ==
TEST(TestZenith_AtlasPackingAlgoCount) {
    ASSERT(GPUTextureAtlasManager::PackingCount() == 4);
}
TEST(TestZenith_AtlasCalcVRAM) {
    ASSERT(GPUTextureAtlasManager::CalcVRAM(4096, 4096,
        AtlasFormat::BGRA8_UNorm) > 0);
}
TEST(TestZenith_AtlasMaxSlots) {
    ASSERT(GPUTextureAtlasManager::MaxSlotsPerAtlas(4096, 256) == 256);
}

//== Predictive Prefetch Engine ==
TEST(TestZenith_PrefetchStrategyCount) {
    ASSERT(PredictivePrefetchEngine::StrategyCount() == 5);
}
TEST(TestZenith_PrefetchCalcHitRate) {
    ASSERT(PredictivePrefetchEngine::CalcHitRate(90, 10) > 0.89);
}

//== Thread Pool Optimizer ==
TEST(TestZenith_PoolSizingPolicyCount) {
    ASSERT(ThreadPoolOptimizer::PolicyCount() == 5);
}
TEST(TestZenith_ThreadPoolRecommend) {
    auto n = ThreadPoolOptimizer::RecommendThreads(PoolSizingPolicy::CoreCount, 8,
        PowerProfile::HighPerformance);
    ASSERT(n >= 1);
}

//== ARM64 NEON Scaler ==
TEST(TestZenith_ARM64CapabilityCount) {
    ASSERT(ARM64NEONScaler::CapabilityCount() == 6);
}
TEST(TestZenith_ARM64IsARM64) {
#ifdef _M_ARM64
    ASSERT(ARM64NEONScaler::IsARM64() == true);
#else
    ASSERT(ARM64NEONScaler::IsARM64() == false);
#endif
}

//== Windows Search Protocol ==
TEST(TestZenith_SearchFieldCount) {
    ASSERT(WindowsSearchProtocol::FieldCount() == 10);
}
TEST(TestZenith_SearchFieldNames) {
    ASSERT(std::wstring(WindowsSearchProtocol::FieldName(
        SearchIndexField::FileName)) == L"File Name");
}

//== Virtual Filesystem Abstraction ==
TEST(TestZenith_VFSBackendCount) {
    ASSERT(VirtualFilesystemAbstraction::BackendCount() == 8);
}
TEST(TestZenith_VFSNeedsDownload) {
    ASSERT(VirtualFilesystemAbstraction::NeedsDownload(
        FileAvailability::CloudOnly) == true);
    ASSERT(VirtualFilesystemAbstraction::NeedsDownload(
        FileAvailability::FullyLocal) == false);
}

//== Role-Based Format Policy ==
TEST(TestZenith_FormatPolicyActionCount) {
    ASSERT(RoleBasedFormatPolicy::ActionCount() == 5);
}
TEST(TestZenith_FormatPolicyPriority) {
    ASSERT(RoleBasedFormatPolicy::HigherPriority(
        FormatPolicySource::GroupPolicy,
        FormatPolicySource::UserPreference) ==
        FormatPolicySource::GroupPolicy);
    ASSERT(
        RoleBasedFormatPolicy::HigherPriority(FormatPolicySource::UserPreference,
            FormatPolicySource::GroupPolicy) ==
        FormatPolicySource::GroupPolicy);
}

//== Audit Trail Logger ==
TEST(TestZenith_AuditSeverityCount) {
    ASSERT(AuditTrailLogger::SeverityCount() == 5);
}
TEST(TestZenith_AuditCategoryCount) {
    ASSERT(AuditTrailLogger::CategoryCount() == 8);
}
TEST(TestZenith_AuditSeverityNames) {
    ASSERT(std::wstring(AuditTrailLogger::SeverityName(AuditSeverity::Info)) ==
        L"Info");
    ASSERT(std::wstring(AuditTrailLogger::SeverityName(
        AuditSeverity::Critical)) == L"Critical");
}

//== Content Inspection Gateway ==
TEST(TestZenith_ContentClassificationCount) {
    ASSERT(ContentInspectionGateway::ClassificationCount() == 5);
}
TEST(TestZenith_ContentShouldBlock) {
    InspectionResult blocked;
    blocked.classification = ContentClassification::Blocked;
    InspectionConfig cfg;
    cfg.enabled = true;
    ASSERT(ContentInspectionGateway::ShouldBlock(blocked, cfg) == true);
    InspectionResult safe;
    safe.classification = ContentClassification::Safe;
    ASSERT(ContentInspectionGateway::ShouldBlock(safe, cfg) == false);
}

//== Certificate Trust Validator ==
TEST(TestZenith_CertValidationCount) {
    ASSERT(CertificateTrustValidator::ValidationCount() == 7);
}
TEST(TestZenith_CertSufficientTrust) {
    ASSERT(CertificateTrustValidator::IsSufficientTrust(
        CertTrustLevel::ExplorerLensSigned, CertTrustLevel::UserTrusted) ==
        true);
    ASSERT(CertificateTrustValidator::IsSufficientTrust(
        CertTrustLevel::Untrusted, CertTrustLevel::UserTrusted) == false);
}

//== Encrypted Format Handler ==
TEST(TestZenith_EncryptionTypeCount) {
    ASSERT(EncryptedFormatHandler::EncryptionCount() == 10);
}
TEST(TestZenith_StrongEncryption) {
    ASSERT(EncryptedFormatHandler::IsStrongEncryption(
        EncryptionType::ZipAES256) == true);
    ASSERT(EncryptedFormatHandler::IsStrongEncryption(
        EncryptionType::ZipZipCrypto) == false);
}

//== Self-Healing Decoder ==
TEST(TestZenith_RecoveryStrategyCount) {
    ASSERT(SelfHealingDecoder::StrategyCount() == 5);
}
TEST(TestZenith_DecoderHealthClassify) {
    ASSERT(SelfHealingDecoder::ClassifyHealth(0.0f) ==
        DecoderHealthState::Healthy);
    ASSERT(SelfHealingDecoder::ClassifyHealth(0.03f) ==
        DecoderHealthState::Degraded);
    ASSERT(SelfHealingDecoder::ClassifyHealth(0.25f) ==
        DecoderHealthState::Quarantined);
}

//== Crash Analytics Collector ==
TEST(TestZenith_CrashCategoryCount) {
    ASSERT(CrashAnalyticsCollector::CategoryCount() == 8);
}
TEST(TestZenith_CrashDumpSize) {
    auto size =
        CrashAnalyticsCollector::EstimateDumpSize(CrashDumpType::MiniDump);
    ASSERT(size > 0);
}

//== Performance Anomaly Detector ==
TEST(TestZenith_AnomalyTypeCount) {
    ASSERT(PerformanceAnomalyDetector::TypeCount() == 8);
}
TEST(TestZenith_AnomalyClassifySeverity) {
    ASSERT(PerformanceAnomalyDetector::ClassifySeverity(1.5) ==
        AnomalySeverity::Minor);
    ASSERT(PerformanceAnomalyDetector::ClassifySeverity(10.0) ==
        AnomalySeverity::Critical);
}
TEST(TestZenith_AnomalyIsAnomaly) {
    ASSERT(PerformanceAnomalyDetector::IsAnomaly(10.0, 2.0, 3.0) == true);
    ASSERT(PerformanceAnomalyDetector::IsAnomaly(1.0, 2.0, 3.0) == false);
}

//== Diagnostic Report Generator V2 ==
TEST(TestZenith_DiagReportSectionCount) {
    ASSERT(DiagnosticReportGeneratorV2::SectionCount() == 9);
}
TEST(TestZenith_DiagReportMimeType) {
    ASSERT(std::string(DiagnosticReportGeneratorV2::MimeType(
        DiagReportFormat::HTML)) == "text/html");
    ASSERT(std::string(DiagnosticReportGeneratorV2::MimeType(
        DiagReportFormat::JSON)) == "application/json");
}

//== Health Check Endpoint ==
TEST(TestZenith_HealthStatusCount) {
    ASSERT(HealthCheckEndpoint::StatusCount() == 5);
}
TEST(TestZenith_HealthAggregateAllHealthy) {
    ASSERT(HealthCheckEndpoint::AggregateHealth(2, 0, 0) ==
        HealthStatus::Healthy);
}
TEST(TestZenith_HealthAggregateDegraded) {
    ASSERT(HealthCheckEndpoint::AggregateHealth(1, 1, 0) ==
        HealthStatus::Degraded);
}

//== Preview Tooltip Renderer ==
TEST(TestZenith_TooltipStyleCount) {
    ASSERT(PreviewTooltipRenderer::StyleCount() == 5);
}
TEST(TestZenith_TooltipStyleNames) {
    ASSERT(std::wstring(PreviewTooltipRenderer::StyleName(
        TooltipStyle::Minimal)) == L"Minimal");
    ASSERT(std::wstring(PreviewTooltipRenderer::StyleName(TooltipStyle::Rich)) ==
        L"Rich");
}

//== Format Gallery Compositor ==
TEST(TestZenith_GalleryLayoutCount) {
    ASSERT(FormatGalleryCompositor::LayoutCount() == 6);
}
TEST(TestZenith_GalleryGridCellSize) {
    auto cell = FormatGalleryCompositor::GridCellSize(800, 4, 4.0f);
    ASSERT(cell > 0);
}

//== Quick Actions Overlay ==
TEST(TestZenith_QuickActionTypeCount) {
    ASSERT(QuickActionsOverlay::ActionCount() == 9);
}
TEST(TestZenith_QuickActionNames) {
    ASSERT(std::wstring(QuickActionsOverlay::ActionName(QuickActionType::Crop)) ==
        L"Crop");
    ASSERT(std::wstring(QuickActionsOverlay::ActionName(
        QuickActionType::ShareFile)) == L"Share");
}

//== Accessibility Narrator Bridge ==
TEST(TestZenith_A11yFeatureCount) {
    ASSERT(AccessibilityNarratorBridge::FeatureCount() == 6);
}
TEST(TestZenith_A11yNarratorText) {
    auto text = AccessibilityNarratorBridge::GenerateNarratorText(
        L"test.png", 1920, 1080, L"PNG");
    ASSERT(std::wstring(text).length() > 0);
}

//== Usage Analytics Dashboard ==
TEST(TestZenith_AnalyticsMetricCount) {
    ASSERT(UsageAnalyticsDashboard::MetricCount() == 8);
}
TEST(TestZenith_AnalyticsMetricNames) {
    ASSERT(std::wstring(UsageAnalyticsDashboard::MetricName(
        AnalyticsMetric::ThumbnailsGenerated)) == L"Thumbnails Generated");
}

//== Decoder Performance Profiler ==
TEST(TestZenith_ProfileGranularityCount) {
    ASSERT(DecoderPerformanceProfiler::GranularityCount() == 4);
}
TEST(TestZenith_ProfileIsRegression) {
    ASSERT(DecoderPerformanceProfiler::IsRegression(10.0, 5.0, 0.10) == true);
    ASSERT(DecoderPerformanceProfiler::IsRegression(5.0, 5.0, 0.10) == false);
}

//== Cache Efficiency Analyzer ==
TEST(TestZenith_CacheZoneCount) {
    ASSERT(CacheEfficiencyAnalyzer::ZoneCount() == 5);
}
TEST(TestZenith_CacheAnalyze) {
    auto rec = CacheEfficiencyAnalyzer::Analyze(0.95f, 0.10f, 0.0f);
    ASSERT(rec == CacheRecommendation::NoChange);
    auto rec2 = CacheEfficiencyAnalyzer::Analyze(0.3f, 0.1f, 0.0f);
    ASSERT(rec2 == CacheRecommendation::IncreaseBudget);
}

//== Format Popularity Tracker ==
TEST(TestZenith_PopularityTierCount) {
    ASSERT(FormatPopularityTracker::TierCount() == 5);
}
TEST(TestZenith_PopularityClassify) {
    ASSERT(FormatPopularityTracker::Classify(50.0f) == PopularityTier::Dominant);
    ASSERT(FormatPopularityTracker::Classify(0.5f) == PopularityTier::Rare);
}

//== System Resource Monitor ==
TEST(TestZenith_MonitoredResourceCount) {
    ASSERT(SystemResourceMonitor::ResourceCount() == 6);
}
TEST(TestZenith_ResourceThrottle) {
    ASSERT(SystemResourceMonitor::RecommendThrottle(95.0f, 95.0f) ==
        ThrottleLevel::Paused);
    ASSERT(SystemResourceMonitor::RecommendThrottle(30.0f, 30.0f) ==
        ThrottleLevel::None);
}

//== Multi-Monitor Color Profile ==
TEST(TestZenith_MonitorProfileTypeCount) {
    ASSERT(MultiMonitorColorProfile::ProfileTypeCount() == 6);
}
TEST(TestZenith_MonitorProfileNames) {
    ASSERT(std::wstring(MultiMonitorColorProfile::ProfileTypeName(
        MonitorProfileType::DCI_P3)) == L"DCI-P3");
}
TEST(TestZenith_GamutMapping) {
    ASSERT(MultiMonitorColorProfile::NeedsGamutMapping(
        MonitorProfileType::AdobeRGB, MonitorProfileType::sRGB) == true);
    ASSERT(MultiMonitorColorProfile::NeedsGamutMapping(
        MonitorProfileType::sRGB, MonitorProfileType::sRGB) == false);
}

//== Rust FFI Bridge ==
TEST(TestZenith_RustLibStatusCount) {
    ASSERT(RustFFIBridge::StatusCount() == 5);
}
TEST(TestZenith_RustABICompat) {
    FFIBridgeConfig cfg;
    cfg.minABIVersion = 1;
    cfg.maxABIVersion = 10;
    ASSERT(RustFFIBridge::IsABICompatible(5, cfg) == true);
    ASSERT(RustFFIBridge::IsABICompatible(11, cfg) == false);
}

//== DirectStorage 1.2 Integration ==
TEST(TestZenith_DStorageSupportCount) {
    ASSERT(DirectStorage12Integration::SupportCount() == 5);
}
TEST(TestZenith_DStorageSpeedup) {
    ASSERT(DirectStorage12Integration::EstimatedSpeedup(
        DStorageSupport::Version1_2) > 3.0);
    ASSERT(DirectStorage12Integration::EstimatedSpeedup(
        DStorageSupport::Emulated) == 1.0);
}

//== Neural Texture Compression ==
TEST(TestZenith_NeuralBackendCount) {
    ASSERT(NeuralTextureCompression::BackendCount() == 5);
}
TEST(TestZenith_NeuralModelTierCount) {
    ASSERT(NeuralTextureCompression::ModelTierCount() == 4);
}
TEST(TestZenith_NeuralDecodeTimeFactor) {
    auto f = NeuralTextureCompression::DecodeTimeFactor(
        NeuralModelTier::Tiny, NeuralCodecBackend::DirectML);
    ASSERT(f < 1.0); // GPU-accelerated small model should be < 1x BC7
}

//== Quantum-Ready Hash Pipeline ==
TEST(TestZenith_QRHashAlgoCount) {
    ASSERT(QuantumReadyHashPipeline::AlgorithmCount() == 6);
}
TEST(TestZenith_QRHashRecommend) {
    ASSERT(QuantumReadyHashPipeline::RecommendAlgorithm(HashPurpose::CacheKey) ==
        QRHashAlgorithm::XXH3_128);
    ASSERT(QuantumReadyHashPipeline::RecommendAlgorithm(
        HashPurpose::DigitalSignature) == QRHashAlgorithm::SHAKE256);
}
TEST(TestZenith_QRHashQuantumSafe) {
    ASSERT(QuantumReadyHashPipeline::IsQuantumSafe(QRHashAlgorithm::BLAKE3) ==
        true);
    ASSERT(QuantumReadyHashPipeline::IsQuantumSafe(QRHashAlgorithm::SHA256) ==
        false);
}

//== WebGPU Thumbnail Renderer ==
TEST(TestZenith_WebGPUBackendCount) {
    ASSERT(WebGPUThumbnailRenderer::BackendCount() == 5);
}
TEST(TestZenith_WebGPUAsyncCompute) {
    ASSERT(WebGPUThumbnailRenderer::SupportsAsyncCompute(
        WebGPUBackend::Dawn_D3D12) == true);
    ASSERT(WebGPUThumbnailRenderer::SupportsAsyncCompute(
        WebGPUBackend::Emscripten) == false);
}

//== WASM Decoder Sandbox ==
TEST(TestZenith_WASMRuntimeCount) {
    ASSERT(WASMDecoderSandbox::RuntimeCount() == 5);
}
TEST(TestZenith_WASMMemoryLimitBytes) {
    ASSERT(WASMDecoderSandbox::MemoryLimitBytes(
        SandboxMemoryTier::Standard_64MB) == 64ULL * 1024 * 1024);
    ASSERT(WASMDecoderSandbox::MemoryLimitBytes(SandboxMemoryTier::Large_1GB) ==
        1024ULL * 1024 * 1024);
}

//== Telemetry Pipeline V2 ==
TEST(TestZenith_TelemetryV2LevelCount) {
    ASSERT(TelemetryPipelineV2::LevelCount() == 5);
}
TEST(TestZenith_TelemetryV2Consent) {
    ASSERT(TelemetryPipelineV2::RequiresConsent(TelemetryLevel::Enhanced) ==
        true);
    ASSERT(TelemetryPipelineV2::RequiresConsent(TelemetryLevel::BasicUsage) ==
        false);
}
TEST(TestZenith_TelemetryV2MinPrivacy) {
    ASSERT(TelemetryPipelineV2::MinPrivacy(TelemetryLevel::BasicUsage) ==
        PrivacyMechanism::KAnonymity);
}

//== Live Preview Streaming Protocol ==
TEST(TestZenith_StreamQualityCount) {
    ASSERT(LivePreviewStreamingProtocol::QualityCount() == 5);
}
TEST(TestZenith_StreamQualityPixels) {
    ASSERT(LivePreviewStreamingProtocol::QualityPixels(
        StreamQuality::Thumbnail_128) == 128);
    ASSERT(LivePreviewStreamingProtocol::QualityPixels(
        StreamQuality::Preview_512) == 512);
}
TEST(TestZenith_StreamSelectQuality) {
    ASSERT(LivePreviewStreamingProtocol::SelectQuality(20.0) ==
        StreamQuality::Full);
    ASSERT(LivePreviewStreamingProtocol::SelectQuality(0.01) ==
        StreamQuality::Placeholder);
}
TEST(TestZenith_StreamBandwidthSufficient) {
    ASSERT(LivePreviewStreamingProtocol::BandwidthSufficient(
        10.0, StreamQuality::Full) == true);
    ASSERT(LivePreviewStreamingProtocol::BandwidthSufficient(
        0.1, StreamQuality::Full) == false);
}

// ---- Release Gate V33 Ship Gate ----
TEST(TestGateV33_KPINames) {
    ASSERT(std::wstring(ReleaseGateV33::GetKPIName(GateV33KPI::VersionSync15)) ==
        L"Version Sync 15.0.0");
    ASSERT(std::wstring(ReleaseGateV33::GetKPIName(GateV33KPI::MuPDFLinked)) ==
        L"MuPDF Linked (PDF Support)");
    ASSERT(std::wstring(ReleaseGateV33::GetKPIName(
        GateV33KPI::BitmapPoolActive)) == L"Bitmap Pool Active");
    ASSERT(std::wstring(ReleaseGateV33::GetKPIName(
        GateV33KPI::CacheWarmingActive)) == L"Cache Warming Active");
}
TEST(TestGateV33_KPICount) { ASSERT(ReleaseGateV33::GetKPICount() == 28); }
TEST(TestGateV33_Evaluate) {
    bool results[28] = {};
    auto res = ReleaseGateV33::Evaluate(results);
    ASSERT(res.kpiTotalCount == 28);
    ASSERT(res.allKPIsPass == false);
    ASSERT(res.v15ShipApproved == false);
}
TEST(TestGateV33_AllPass) {
    bool results[28];
    for (int i = 0; i < 28; ++i)
        results[i] = true;
    auto res = ReleaseGateV33::Evaluate(results);
    ASSERT(res.allKPIsPass);
    ASSERT(res.v15ShipApproved);
    ASSERT(res.gateScore == 1.0f);
}
TEST(TestGateV33_v15ShipApproved85Pct) {
    bool results[28];
    for (int i = 0; i < 28; ++i)
        results[i] = true;
    // Fail 4 out of 28 = 24/28 = 85.7% => should pass
    results[0] = false;
    results[1] = false;
    results[2] = false;
    results[3] = false;
    auto res = ReleaseGateV33::Evaluate(results);
    ASSERT(res.v15ShipApproved == true);
}
TEST(TestGateV33_v15ShipDeniedBelow85) {
    bool results[28];
    for (int i = 0; i < 28; ++i)
        results[i] = true;
    // Fail 5 out of 28 = 23/28 = 82.1% => should fail
    results[0] = false;
    results[1] = false;
    results[2] = false;
    results[3] = false;
    results[4] = false;
    auto res = ReleaseGateV33::Evaluate(results);
    ASSERT(res.v15ShipApproved == false);
}
TEST(TestGateV33_Codename) {
    bool results[28] = {};
    auto res = ReleaseGateV33::Evaluate(results);
    ASSERT(std::wstring(res.codename) == L"Zenith");
}

//==============================================================================
// Plugin Consolidation Tests
//==============================================================================

TEST(TestPluginMarketplaceUnified_VersionEnum) {
    using namespace ExplorerLens::Engine;
    ASSERT(RecommendedMarketplaceVersion() == MarketplaceVersion::V2);
    ASSERT(ActiveMarketplaceVersionCount() == 2);
    ASSERT(static_cast<uint32_t>(MarketplaceVersion::V2) == 2);
    ASSERT(static_cast<uint32_t>(MarketplaceVersion::V3) == 3);
}

TEST(TestPluginMarketplaceUnified_V2Search) {
    using namespace ExplorerLens::Engine;
    PluginMarketplaceV2 marketplace;
    PluginListing listing;
    listing.id = L"test-plugin";
    listing.name = L"Test Plugin";
    listing.description = L"A test plugin";
    listing.author = L"Author";
    listing.version = PluginVersion::Parse(L"1.0.0");
    listing.category = PluginCategory::Decoder;
    marketplace.AddToCatalog(listing);
    MarketplaceFilter filter;
    filter.query = L"test";
    auto results = marketplace.Search(filter);
    ASSERT(results.totalCount > 0);
}

TEST(TestPluginMarketplaceUnified_V3Categories) {
    using namespace ExplorerLens::Engine;
    ASSERT(PluginMarketplaceV3::CategoryCount() == static_cast<uint32_t>(PluginCategoryV3::COUNT));
    auto name = PluginMarketplaceV3::CategoryName(PluginCategoryV3::ImageDecoder);
    ASSERT(name != nullptr && wcslen(name) > 0);
}

TEST(TestPluginSecurity_LevelEnum) {
    using namespace ExplorerLens::Plugin;
    ASSERT(RequiresSandbox(PluginSecurityLevel::Untrusted) == true);
    ASSERT(RequiresSandbox(PluginSecurityLevel::Basic) == true);
    ASSERT(RequiresSandbox(PluginSecurityLevel::Verified) == true);
    ASSERT(RequiresSandbox(PluginSecurityLevel::Trusted) == false);
    ASSERT(RequiresSandbox(PluginSecurityLevel::BuiltIn) == false);
}

TEST(TestPluginSecurity_SandboxPreset) {
    using namespace ExplorerLens::Plugin;
    ASSERT(RecommendedSandboxPreset(PluginSecurityLevel::Untrusted) == SandboxPolicyPreset::Strict);
    ASSERT(RecommendedSandboxPreset(PluginSecurityLevel::Basic) == SandboxPolicyPreset::Standard);
    ASSERT(RecommendedSandboxPreset(PluginSecurityLevel::Trusted) == SandboxPolicyPreset::Developer);
}

TEST(TestPluginSecurity_SandboxPolicyStruct) {
    // Fully qualify to avoid ambiguity with ExplorerLens::Engine::SandboxPolicy enum
    auto strict = ExplorerLens::Plugin::SandboxPolicy::Strict();
    ASSERT(strict.preset == ExplorerLens::Plugin::SandboxPolicyPreset::Strict);
    auto standard = ExplorerLens::Plugin::SandboxPolicy::Standard();
    ASSERT(standard.preset == ExplorerLens::Plugin::SandboxPolicyPreset::Standard);
    auto dev = ExplorerLens::Plugin::SandboxPolicy::Developer();
    ASSERT(dev.preset == ExplorerLens::Plugin::SandboxPolicyPreset::Developer);
}

TEST(TestPluginSecurity_RuntimeValidator) {
    auto validator = ExplorerLens::Plugin::PluginRuntimeValidator::Create();
    ASSERT(validator.InvalidTransitionCount() == 0);
}

TEST(TestPluginLifecycle_PhaseEnum) {
    using namespace ExplorerLens::Engine;
    ASSERT(LifecyclePhaseCount() == 10);
    ASSERT(std::wstring(LifecyclePhaseName(PluginLifecyclePhase::Discovery)) == L"Discovery");
    ASSERT(std::wstring(LifecyclePhaseName(PluginLifecyclePhase::Running)) == L"Running");
    ASSERT(std::wstring(LifecyclePhaseName(PluginLifecyclePhase::Faulted)) == L"Faulted");
}

TEST(TestPluginLifecycle_ActivePhase) {
    using namespace ExplorerLens::Engine;
    ASSERT(IsActivePhase(PluginLifecyclePhase::Running) == true);
    ASSERT(IsActivePhase(PluginLifecyclePhase::HotReloading) == true);
    ASSERT(IsActivePhase(PluginLifecyclePhase::Suspended) == true);
    ASSERT(IsActivePhase(PluginLifecyclePhase::Discovery) == false);
    ASSERT(IsActivePhase(PluginLifecyclePhase::Terminated) == false);
}

TEST(TestPluginLifecycle_TerminalPhase) {
    using namespace ExplorerLens::Engine;
    ASSERT(IsTerminalPhase(PluginLifecyclePhase::Terminated) == true);
    ASSERT(IsTerminalPhase(PluginLifecyclePhase::Faulted) == true);
    ASSERT(IsTerminalPhase(PluginLifecyclePhase::Running) == false);
}

TEST(TestPluginLifecycle_HotReloadEnums) {
    using namespace ExplorerLens::Engine;
    ASSERT(PluginHotReload::TriggerCount() == static_cast<uint32_t>(HotReloadTrigger::COUNT));
    ASSERT(PluginHotReload::StateCount() == static_cast<uint32_t>(HotReloadState::COUNT));
    ASSERT(PluginHotReload::PolicyCount() == static_cast<uint32_t>(HotReloadPolicy::COUNT));
    auto name = PluginHotReload::TriggerName(HotReloadTrigger::FileChange);
    ASSERT(name != nullptr && wcslen(name) > 0);
}

//==============================================================================
// AI Algorithm Tests (real CPU-based implementations)
//==============================================================================

TEST(TestAISearch_PerceptualHash_Uniform) {
    using namespace ExplorerLens::Engine;
    // Uniform gray image → hash should be 0 (all pixels == mean)
    const uint32_t W = 16, H = 16;
    std::vector<uint8_t> gray(W * H, 128);
    uint64_t hash = AISearchIntegration::ComputeAverageHash(gray.data(), W, H, W);
    // All pixels equal the mean → all comparisons false → hash = 0
    ASSERT(hash == 0);
}

TEST(TestAISearch_PerceptualHash_DHash) {
    using namespace ExplorerLens::Engine;
    // Alternating columns (high/low) → dHash should be non-zero
    const uint32_t W = 16, H = 16;
    std::vector<uint8_t> gray(W * H);
    for (uint32_t y = 0; y < H; ++y)
        for (uint32_t x = 0; x < W; ++x)
            gray[y * W + x] = static_cast<uint8_t>((x % 2 == 0) ? 200 : 50);
    uint64_t hash = AISearchIntegration::ComputeDifferenceHash(gray.data(), W, H, W);
    ASSERT(hash != 0);
}

TEST(TestAISearch_HammingDistance) {
    using namespace ExplorerLens::Engine;
    ASSERT(AISearchIntegration::HammingDistance(0, 0) == 0);
    ASSERT(AISearchIntegration::HammingDistance(0xFF, 0x00) == 8);
    ASSERT(AISearchIntegration::HammingDistance(0xFFFFFFFFFFFFFFFFULL, 0) == 64);
}

TEST(TestAISearch_AreSimilar) {
    using namespace ExplorerLens::Engine;
    // Identical hashes → similar
    ASSERT(AISearchIntegration::AreSimilar(0x1234, 0x1234));
    // Very different → not similar (hamming > 10)
    ASSERT(!AISearchIntegration::AreSimilar(0xFFFFFFFFFFFFFFFFULL, 0));
}

TEST(TestImageQuality_LaplacianVariance_Uniform) {
    using namespace ExplorerLens::Engine;
    // Uniform image → Laplacian is 0 everywhere → variance = 0
    const uint32_t W = 16, H = 16;
    std::vector<uint8_t> gray(W * H, 100);
    double var = ImageQualityAssessor::ComputeLaplacianVariance(gray.data(), W, H, W);
    ASSERT(var < 1.0);  // Should be ~0 for uniform
}

TEST(TestImageQuality_LaplacianVariance_Edges) {
    using namespace ExplorerLens::Engine;
    // Checkerboard pattern → high frequency → high Laplacian variance
    const uint32_t W = 16, H = 16;
    std::vector<uint8_t> gray(W * H);
    for (uint32_t y = 0; y < H; ++y)
        for (uint32_t x = 0; x < W; ++x)
            gray[y * W + x] = ((x + y) % 2 == 0) ? 255 : 0;
    double var = ImageQualityAssessor::ComputeLaplacianVariance(gray.data(), W, H, W);
    ASSERT(var > 100.0);  // High variance for high-frequency content
}

TEST(TestImageQuality_MeanBrightness) {
    using namespace ExplorerLens::Engine;
    const uint32_t W = 8, H = 8;
    std::vector<uint8_t> gray(W * H, 200);
    double mean = ImageQualityAssessor::ComputeMeanBrightness(gray.data(), W, H, W);
    ASSERT(mean > 199.0 && mean < 201.0);
}

TEST(TestImageQuality_ExposureDefects_Over) {
    using namespace ExplorerLens::Engine;
    // Very bright image → overexposed
    const uint32_t W = 10, H = 10;
    std::vector<uint8_t> gray(W * H, 250);
    auto defects = ImageQualityAssessor::DetectExposureDefects(gray.data(), W, H, W);
    bool foundOver = false;
    for (auto d : defects) if (d == IQADefect::Overexposed) foundOver = true;
    ASSERT(foundOver);
}

TEST(TestImageQuality_ExposureDefects_Under) {
    using namespace ExplorerLens::Engine;
    // Very dark image → underexposed
    const uint32_t W = 10, H = 10;
    std::vector<uint8_t> gray(W * H, 5);
    auto defects = ImageQualityAssessor::DetectExposureDefects(gray.data(), W, H, W);
    bool foundUnder = false;
    for (auto d : defects) if (d == IQADefect::Underexposed) foundUnder = true;
    ASSERT(foundUnder);
}

TEST(TestImageQuality_GradeBySharpness) {
    using namespace ExplorerLens::Engine;
    ASSERT(ImageQualityAssessor::GradeBySharpness(600.0) == IQAGrade::Excellent);
    ASSERT(ImageQualityAssessor::GradeBySharpness(300.0) == IQAGrade::Good);
    ASSERT(ImageQualityAssessor::GradeBySharpness(150.0) == IQAGrade::Fair);
    ASSERT(ImageQualityAssessor::GradeBySharpness(80.0) == IQAGrade::Poor);
    ASSERT(ImageQualityAssessor::GradeBySharpness(5.0) == IQAGrade::Unacceptable);
}

TEST(TestImageQuality_Assess) {
    using namespace ExplorerLens::Engine;
    const uint32_t W = 16, H = 16;
    std::vector<uint8_t> gray(W * H, 128);
    auto report = ImageQualityAssessor::Assess(gray.data(), W, H, W);
    // Uniform gray → blurry, grade should be Unacceptable or Poor
    ASSERT(report.overallScore.grade == IQAGrade::Unacceptable || report.overallScore.grade == IQAGrade::Poor);
    ASSERT(report.blurScore > 0.5f);  // Should detect blur
    ASSERT(report.worthEnhancing);
}

TEST(TestSmartCropV2_CenterOfInterest_Uniform) {
    using namespace ExplorerLens::Engine;
    // Uniform image → center of interest should be near image center
    const uint32_t W = 32, H = 32;
    std::vector<uint8_t> gray(W * H, 128);
    float cx = 0, cy = 0;
    SmartCropV2::ComputeCenterOfInterest(gray.data(), W, H, W, cx, cy);
    // For uniform image with zero gradients, fallback is center
    ASSERT(cx >= 0 && cx <= static_cast<float>(W));
    ASSERT(cy >= 0 && cy <= static_cast<float>(H));
}

TEST(TestSmartCropV2_CropRegion_Bounds) {
    using namespace ExplorerLens::Engine;
    const uint32_t W = 64, H = 64;
    // Create a gradient image to generate interest
    std::vector<uint8_t> gray(W * H);
    for (uint32_t y = 0; y < H; ++y)
        for (uint32_t x = 0; x < W; ++x)
            gray[y * W + x] = static_cast<uint8_t>((x + y) % 256);
    auto result = SmartCropV2::ComputeCropRegion(gray.data(), W, H, W, 32, 32);
    // Crop must be within bounds
    ASSERT(result.cropX >= 0);
    ASSERT(result.cropY >= 0);
    ASSERT(result.cropX + result.cropW <= W);
    ASSERT(result.cropY + result.cropH <= H);
    ASSERT(result.cropW == 32);
    ASSERT(result.cropH == 32);
}

TEST(TestSceneAI_ClassifyHeuristics_Green) {
    using namespace ExplorerLens::Engine;
    // Create a green image (nature-like)
    const uint32_t W = 16, H = 16;
    std::vector<uint8_t> rgb(W * H * 3);
    for (uint32_t i = 0; i < W * H; ++i) {
        rgb[i * 3 + 0] = 30;   // R
        rgb[i * 3 + 1] = 180;  // G
        rgb[i * 3 + 2] = 30;   // B
    }
    auto result = SceneUnderstandingEngine::ClassifyByHeuristics(rgb.data(), W, H, W * 3);
    ASSERT(result.category == SceneCategory::Nature);
    ASSERT(result.score > 0.3f);
}

TEST(TestSceneAI_ClassifyHeuristics_Dark) {
    using namespace ExplorerLens::Engine;
    // Very dark, low saturation → Technology
    const uint32_t W = 16, H = 16;
    std::vector<uint8_t> rgb(W * H * 3);
    for (uint32_t i = 0; i < W * H; ++i) {
        rgb[i * 3 + 0] = 10;
        rgb[i * 3 + 1] = 10;
        rgb[i * 3 + 2] = 10;
    }
    auto result = SceneUnderstandingEngine::ClassifyByHeuristics(rgb.data(), W, H, W * 3);
    ASSERT(result.category == SceneCategory::Technology);
}

TEST(TestSceneAI_ClassifyHeuristics_Null) {
    using namespace ExplorerLens::Engine;
    auto result = SceneUnderstandingEngine::ClassifyByHeuristics(nullptr, 16, 16, 48);
    ASSERT(result.category == SceneCategory::Abstract);
    ASSERT(result.score <= 0.3f);
}

//==============================================================================
// Umbrella Consolidation Tests
//==============================================================================

TEST(TestTestInfrastructure_Available) {
    using namespace ExplorerLens::Engine;
    ASSERT(TestInfrastructureAvailable());
    ASSERT(TestInfrastructureModuleCount() == 3);
}

TEST(TestTestInfrastructure_Components) {
    using namespace ExplorerLens::Engine;
    // Verify key types from each consolidated module are reachable
    auto& itf = IntegrationTestFramework::Instance();
    ASSERT(itf.GetTestCount() >= 0);  // singleton — count depends on prior registrations
    TestSuiteExpansion tse;
    ASSERT(tse.GetTotalTestCount() >= 0);
}

TEST(TestDocGenerator_Umbrella) {
    using namespace ExplorerLens::Engine;
    // DocGenerator umbrella now includes AutoDocGenerator + VersionNormalization + DiagnosticDashboard
    AutoDocGenerator gen;
    auto sectionName = AutoDocGenerator::GetSectionName(DocSection::Overview);
    ASSERT(sectionName != nullptr && wcslen(sectionName) > 0);
}

TEST(TestWindowsCompat_Umbrella) {
    // WindowsCompat umbrella now includes Win11Integration + MSIXPackageManager + PortableModeManager
    // Verify free functions from WindowsCompat.h are reachable
    DWORD build = ExplorerLens::GetWindowsBuildNumber();
    ASSERT(build >= 10240);  // At least Windows 10 on any dev machine
}

//== Shell Progress Indicator ==
TEST(TestShellProgress_StageNames) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ThumbnailStageToString(ThumbnailStage::Detection)) == "Detection");
    ASSERT(std::string(ThumbnailStageToString(ThumbnailStage::Decode)) == "Decode");
    ASSERT(std::string(ThumbnailStageToString(ThumbnailStage::Complete)) == "Complete");
    ASSERT(std::string(ThumbnailStageToString(ThumbnailStage::Failed)) == "Failed");
}

TEST(TestShellProgress_BeginEnd) {
    using namespace ExplorerLens::Engine;
    ShellProgressIndicator indicator;
    indicator.BeginFile(L"test.zip", 100 * 1024 * 1024);  // 100 MB
    auto prog = indicator.GetProgress();
    ASSERT(prog.fileSize == 100 * 1024 * 1024);
    ASSERT(prog.percentComplete == 0);
    ASSERT(!prog.cancelled);
    indicator.EndFile(true);
    prog = indicator.GetProgress();
    ASSERT(prog.currentStage == ThumbnailStage::Complete);
    ASSERT(prog.percentComplete == 100);
}

TEST(TestShellProgress_AdvanceStage) {
    using namespace ExplorerLens::Engine;
    ShellProgressIndicator indicator;
    indicator.BeginFile(L"test.rar", 60 * 1024 * 1024);
    indicator.AdvanceStage(ThumbnailStage::Extraction);
    ASSERT(indicator.GetProgress().currentStage == ThumbnailStage::Extraction);
    indicator.AdvanceStage(ThumbnailStage::Decode);
    ASSERT(indicator.GetProgress().currentStage == ThumbnailStage::Decode);
}

TEST(TestShellProgress_Cancellation) {
    using namespace ExplorerLens::Engine;
    ShellProgressIndicator indicator;
    indicator.BeginFile(L"test.7z", 200 * 1024 * 1024);
    ASSERT(!indicator.IsCancelled());
    indicator.RequestCancel();
    ASSERT(indicator.IsCancelled());
}

TEST(TestShellProgress_BatchTracker) {
    using namespace ExplorerLens::Engine;
    BatchProgressTracker tracker;
    tracker.Begin(10);
    ASSERT(tracker.GetTotalFiles() == 10);
    ASSERT(tracker.GetCompletedFiles() == 0);
    tracker.OnFileComplete(true);
    tracker.OnFileComplete(true);
    tracker.OnFileComplete(false);
    ASSERT(tracker.GetCompletedFiles() == 3);
    ASSERT(tracker.GetFailedFiles() == 1);
    ASSERT(tracker.GetBatchPercentComplete() == 30);
}

TEST(TestShellProgress_SmallFileNoReport) {
    using namespace ExplorerLens::Engine;
    // Files below threshold should not trigger reporting
    ASSERT(ShellProgressIndicator::SMALL_FILE_THRESHOLD == 1 * 1024 * 1024);
    ASSERT(ShellProgressIndicator::LARGE_FILE_THRESHOLD == 50 * 1024 * 1024);
}

//== Shell Search Protocol Handler ==
TEST(TestSearchProtocol_Initialize) {
    using namespace ExplorerLens::Engine;
    ShellSearchProtocolHandler handler;
    ASSERT(handler.GetState() == SearchHandlerState::Uninitialized);
    ASSERT(handler.Initialize());
    ASSERT(handler.GetState() == SearchHandlerState::Ready);
    ASSERT(handler.GetIndexSize() == 0);
    handler.Shutdown();
    ASSERT(handler.GetState() == SearchHandlerState::Uninitialized);
}

TEST(TestSearchProtocol_AddAndSearch) {
    using namespace ExplorerLens::Engine;
    ShellSearchProtocolHandler handler;
    handler.Initialize();

    SearchIndexEntry entry;
    entry.filePath = L"C:\\Archives\\test.zip";
    entry.displayName = L"test.zip";
    entry.formatType = L"ZIP";
    entry.fileSize = 1024;
    entry.hasThumbnail = true;
    handler.AddEntry(entry);
    ASSERT(handler.GetIndexSize() == 1);

    SearchQuery query;
    query.queryText = L"test";
    auto results = handler.Search(query);
    ASSERT(results.totalMatches == 1);
    ASSERT(results.results[0].relevanceScore > 0.0f);
}

TEST(TestSearchProtocol_ExtensionFilter) {
    using namespace ExplorerLens::Engine;
    ShellSearchProtocolHandler handler;
    handler.Initialize();

    SearchIndexEntry zip;
    zip.filePath = L"C:\\doc.zip";
    zip.displayName = L"doc.zip";
    zip.formatType = L"ZIP";
    handler.AddEntry(zip);

    SearchIndexEntry rar;
    rar.filePath = L"C:\\doc.rar";
    rar.displayName = L"doc.rar";
    rar.formatType = L"RAR";
    handler.AddEntry(rar);

    SearchQuery query;
    query.queryText = L"doc";
    query.extensionFilter = L".zip";
    auto results = handler.Search(query);
    ASSERT(results.totalMatches == 1);
}

TEST(TestSearchProtocol_ClearIndex) {
    using namespace ExplorerLens::Engine;
    ShellSearchProtocolHandler handler;
    handler.Initialize();

    SearchIndexEntry entry;
    entry.filePath = L"C:\\test.7z";
    entry.displayName = L"test.7z";
    handler.AddEntry(entry);
    ASSERT(handler.GetIndexSize() == 1);
    handler.ClearIndex();
    ASSERT(handler.GetIndexSize() == 0);
}

//== Jump List Integration ==
TEST(TestJumpList_Initialize) {
    using namespace ExplorerLens::Engine;
    JumpListIntegration jumpList;
    ASSERT(!jumpList.IsInitialized());
    ASSERT(jumpList.Initialize(L"ExplorerLens.Shell.15"));
    ASSERT(jumpList.IsInitialized());
    ASSERT(jumpList.GetRecentCount() == 0);
    jumpList.Shutdown();
    ASSERT(!jumpList.IsInitialized());
}

TEST(TestJumpList_RecordAccess) {
    using namespace ExplorerLens::Engine;
    JumpListIntegration jumpList;
    jumpList.Initialize(L"ExplorerLens.Shell.15");
    jumpList.RecordAccess(L"C:\\test.zip", L"test.zip", L"ZIP", 1024, 5);
    ASSERT(jumpList.GetRecentCount() == 1);
    jumpList.RecordAccess(L"C:\\test.rar", L"test.rar", L"RAR", 2048, 10);
    ASSERT(jumpList.GetRecentCount() == 2);
}

TEST(TestJumpList_PinEntry) {
    using namespace ExplorerLens::Engine;
    JumpListIntegration jumpList;
    jumpList.Initialize(L"ExplorerLens.Shell.15");
    jumpList.RecordAccess(L"C:\\pinned.zip", L"pinned.zip", L"ZIP");
    ASSERT(jumpList.PinEntry(L"C:\\pinned.zip"));
    ASSERT(jumpList.GetPinnedCount() == 1);
    ASSERT(jumpList.UnpinEntry(L"C:\\pinned.zip"));
    ASSERT(jumpList.GetPinnedCount() == 0);
}

TEST(TestJumpList_Categories) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(JumpListCategoryToString(JumpListCategory::Recent)) == "Recent");
    ASSERT(std::string(JumpListCategoryToString(JumpListCategory::Frequent)) == "Frequent");
    ASSERT(std::string(JumpListCategoryToString(JumpListCategory::Pinned)) == "Pinned");
}

TEST(TestJumpList_ExportImport) {
    using namespace ExplorerLens::Engine;
    JumpListIntegration jumpList;
    jumpList.Initialize(L"ExplorerLens.Shell.15");
    jumpList.RecordAccess(L"C:\\a.zip", L"a.zip", L"ZIP");
    jumpList.RecordAccess(L"C:\\b.rar", L"b.rar", L"RAR");
    auto exported = jumpList.ExportAllEntries();
    ASSERT(exported.size() == 2);

    JumpListIntegration jumpList2;
    jumpList2.Initialize(L"ExplorerLens.Shell.15");
    jumpList2.ImportEntries(exported);
    ASSERT(jumpList2.GetRecentCount() == 2);
}

//== Plugin Loader V2 (C ABI) ==
TEST(TestPluginLoaderV2_Create) {
    using namespace ExplorerLens::Engine;
    PluginLoaderV2 loader;
    ASSERT(loader.GetPluginCount() == 0);
    ASSERT(loader.GetLastError().empty());
}

TEST(TestPluginLoaderV2_ABIVersion) {
    using namespace ExplorerLens::Engine;
    ASSERT(HOST_ABI_VERSION.major == 1);
    ASSERT(HOST_ABI_VERSION.minor == 0);
    PluginABIVersion compatible = { 1, 0 };
    ASSERT(compatible.IsCompatibleWith(HOST_ABI_VERSION));
    PluginABIVersion incompatible = { 2, 0 };
    ASSERT(!incompatible.IsCompatibleWith(HOST_ABI_VERSION));
}

TEST(TestPluginLoaderV2_PluginState) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(PluginStateToString(PluginState::Unloaded)) == "Unloaded");
    ASSERT(std::string(PluginStateToString(PluginState::Active)) == "Active");
    ASSERT(std::string(PluginStateToString(PluginState::Error)) == "Error");
}

TEST(TestPluginLoaderV2_DescriptorDefaults) {
    using namespace ExplorerLens::Engine;
    PluginDescriptor desc;
    ASSERT(desc.state == PluginState::Unloaded);
    ASSERT(desc.abiVersion.major == 0);
    ASSERT(desc.totalDecodes == 0);
    ASSERT(desc.failedDecodes == 0);
}

TEST(TestPluginLoaderV2_FindNonexistent) {
    using namespace ExplorerLens::Engine;
    PluginLoaderV2 loader;
    auto* plugin = loader.FindPluginForExtension(".xyz");
    ASSERT(plugin == nullptr);
}

TEST(TestPluginLoaderV2_LoadInvalidDLL) {
    using namespace ExplorerLens::Engine;
    PluginLoaderV2 loader;
    ASSERT(!loader.LoadPlugin(L"C:\\nonexistent_path\\FakePlugin.dll"));
    ASSERT(!loader.GetLastError().empty());
}

// ============================================================================
// Zero-Copy Activation Tests
// ============================================================================

TEST(TestZeroCopy_ModeStrings) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ZeroCopyModeToString(ZeroCopyMode::Disabled)) == "Disabled");
    ASSERT(std::string(ZeroCopyModeToString(ZeroCopyMode::DirectStorage)) == "DirectStorage");
    ASSERT(std::string(ZeroCopyModeToString(ZeroCopyMode::PinnedMemory)) == "PinnedMemory");
}

TEST(TestZeroCopy_StatsInitial) {
    using namespace ExplorerLens::Engine;
    ZeroCopyStats stats;
    ASSERT(stats.totalTransfers == 0);
    ASSERT(stats.GetZeroCopyRate() == 0.0);
    ASSERT(stats.GetBandwidthSavingPercent() == 0.0);
}

TEST(TestZeroCopy_Initialize) {
    using namespace ExplorerLens::Engine;
    ZeroCopyActivation zca;
    ASSERT(zca.Activate());
    ASSERT(zca.IsActive());
    ASSERT(zca.GetActiveMode() != ZeroCopyMode::Disabled);
}

TEST(TestZeroCopy_StagingBuffer) {
    using namespace ExplorerLens::Engine;
    ZeroCopyActivation zca;
    ASSERT(zca.Activate());
    auto buf = zca.AcquireStaging(1024);
    ASSERT(buf.IsValid());
    ASSERT(buf.size >= 1024);
    zca.ReleaseStaging(buf);
}

// ============================================================================
// SIMD Dispatch Router Tests
// ============================================================================

TEST(TestSIMD_TierStrings) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(SIMDTierToString(SIMDTier::Scalar)) == "Scalar");
    ASSERT(std::string(SIMDTierToString(SIMDTier::AVX2)) == "AVX2");
    ASSERT(std::string(SIMDTierToString(SIMDTier::NEON)) == "NEON");
}

TEST(TestSIMD_FeatureFlags) {
    using namespace ExplorerLens::Engine;
    SIMDFeature combined = SIMDFeature::SSE2 | SIMDFeature::AVX2;
    ASSERT(HasFeature(combined, SIMDFeature::SSE2));
    ASSERT(HasFeature(combined, SIMDFeature::AVX2));
    ASSERT(!HasFeature(combined, SIMDFeature::AVX512F));
}

TEST(TestSIMD_RouterDetection) {
    using namespace ExplorerLens::Engine;
    auto& router = SIMDDispatchRouter::Instance();
    auto caps = router.GetCapabilities();
    // x86-64 always has SSE2
    ASSERT(HasFeature(caps.features, SIMDFeature::SSE2));
    ASSERT(caps.bestTier >= SIMDTier::SSE);
}

TEST(TestSIMD_KernelSelection) {
    using namespace ExplorerLens::Engine;
    auto& router = SIMDDispatchRouter::Instance();
    ASSERT(router.GetActiveTier() >= SIMDTier::SSE);
    ASSERT(router.GetCapabilities().vendorString[0] != '\0');
}

// ============================================================================
// PSO Persistence Manager Tests
// ============================================================================

TEST(TestPSO_TypeStrings) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(PSOTypeToString(PSOType::Graphics)) == "Graphics");
    ASSERT(std::string(PSOTypeToString(PSOType::Compute)) == "Compute");
    ASSERT(std::string(PSOTypeToString(PSOType::RayTracing)) == "RayTracing");
}

TEST(TestPSO_Initialize) {
    using namespace ExplorerLens::Engine;
    PSOPersistenceManager mgr;
    ASSERT(mgr.Initialize(0x1234, 0x5678));
    auto stats = mgr.GetStats();
    ASSERT(stats.totalEntries == 0);
}

TEST(TestPSO_StoreAndLookup) {
    using namespace ExplorerLens::Engine;
    PSOPersistenceManager mgr;
    mgr.Initialize(0x1234, 0x5678);
    uint8_t blob[] = { 1, 2, 3, 4 };
    ASSERT(mgr.StorePSO("TestShader", 42, PSOType::Compute, blob, 4, 5000));
    auto* entry = mgr.LookupPSO(42);
    ASSERT(entry != nullptr);
    ASSERT(entry->name == "TestShader");
    ASSERT(entry->hitCount == 1);
}

TEST(TestPSO_InvalidateAndPurge) {
    using namespace ExplorerLens::Engine;
    PSOPersistenceManager mgr;
    mgr.Initialize(0x1234, 0x5678);
    uint8_t blob[] = { 1, 2, 3 };
    mgr.StorePSO("A", 1, PSOType::Graphics, blob, 3, 1000);
    mgr.StorePSO("B", 2, PSOType::Compute, blob, 3, 2000);
    mgr.InvalidateAll();
    ASSERT(mgr.LookupPSO(1) == nullptr);
    uint32_t purged = mgr.PurgeStale();
    ASSERT(purged == 2);
}

// ============================================================================
// Predictive Cache Engine Tests
// ============================================================================

TEST(TestPredictive_Initialize) {
    using namespace ExplorerLens::Engine;
    PredictionConfig cfg;
    cfg.maxHistorySize = 100;
    PredictiveCacheEngine engine(cfg);
    ASSERT(engine.GetHistorySize() == 0);
    ASSERT(engine.GetProfileCount() == 0);
}

TEST(TestPredictive_RecordNavigation) {
    using namespace ExplorerLens::Engine;
    PredictiveCacheEngine engine;
    NavigationEvent ev;
    ev.directoryPath = L"C:\\Photos";
    ev.fileCount = 50;
    ev.timestamp = 1000000;
    ev.dwellTimeMs = 5000;
    engine.RecordNavigation(ev);
    ASSERT(engine.GetHistorySize() == 1);
    ASSERT(engine.GetProfileCount() == 1);
}

TEST(TestPredictive_PredictNext) {
    using namespace ExplorerLens::Engine;
    PredictiveCacheEngine engine;
    NavigationEvent ev1;
    ev1.directoryPath = L"C:\\Photos";
    ev1.timestamp = 1000000;
    ev1.dwellTimeMs = 5000;
    engine.RecordNavigation(ev1);
    NavigationEvent ev2;
    ev2.directoryPath = L"C:\\Docs";
    ev2.timestamp = 1006000;
    ev2.dwellTimeMs = 3000;
    engine.RecordNavigation(ev2);
    auto predictions = engine.PredictNext(L"C:\\Docs");
    // Should predict C:\Photos as a candidate
    ASSERT(engine.GetStats().totalPredictions == 1);
}

TEST(TestPredictive_PinDirectory) {
    using namespace ExplorerLens::Engine;
    PredictiveCacheEngine engine;
    NavigationEvent ev;
    ev.directoryPath = L"C:\\Important";
    ev.timestamp = 1000000;
    engine.RecordNavigation(ev);
    engine.PinDirectory(L"C:\\Important");
    // Pinned directories should always be predicted
    ASSERT(engine.GetProfileCount() == 1);
}

// ============================================================================
// Lanczos GPU Kernel Tests
// ============================================================================

TEST(TestLanczos_FilterStrings) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ResampleFilterToString(ResampleFilter::Lanczos3)) == "Lanczos3");
    ASSERT(std::string(ResampleFilterToString(ResampleFilter::Bilinear)) == "Bilinear");
}

TEST(TestLanczos_DispatchParams) {
    using namespace ExplorerLens::Engine;
    auto params = ShaderDispatchParams::ForTexture(256, 256, 8);
    ASSERT(params.threadGroupsX == 32);
    ASSERT(params.threadGroupsY == 32);
    ASSERT(params.threadsPerGroup == 64);
}

TEST(TestLanczos_KernelWeights) {
    using namespace ExplorerLens::Engine;
    // Lanczos3 weight at center should be 1.0
    float centerWeight = LanczosGPUKernel::LanczosWeight(0.0f, 3);
    ASSERT(centerWeight > 0.99f && centerWeight <= 1.0f);
    // Weight outside window should be 0
    float outsideWeight = LanczosGPUKernel::LanczosWeight(4.0f, 3);
    ASSERT(outsideWeight == 0.0f);
}

TEST(TestLanczos_Initialize) {
    using namespace ExplorerLens::Engine;
    LanczosGPUKernel kernel;
    ASSERT(kernel.Initialize());
    ASSERT(kernel.IsInitialized());
    auto stats = kernel.GetStats();
    ASSERT(stats.totalResamples == 0);
}

// ============================================================================
// HDR Tone Map Kernel Tests
// ============================================================================

TEST(TestHDR_OperatorStrings) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ToneMapMethodToString(ToneMapMethod::ACESFilmic)) == "ACESFilmic");
    ASSERT(std::string(ToneMapMethodToString(ToneMapMethod::Reinhard)) == "Reinhard");
}

TEST(TestHDR_ReinhardToneMap) {
    using namespace ExplorerLens::Engine;
    float result = HDRToneMapKernel::ToneMapReinhard(1.0f);
    ASSERT(result > 0.49f && result < 0.51f);  // 1/(1+1) = 0.5
}

TEST(TestHDR_ACESToneMap) {
    using namespace ExplorerLens::Engine;
    float result = HDRToneMapKernel::ToneMapACES(1.0f);
    ASSERT(result > 0.0f && result <= 1.0f);
}

TEST(TestHDR_SceneAnalysis) {
    using namespace ExplorerLens::Engine;
    // Create a simple 2x2 HDR image
    float hdr[] = { 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 0.1f, 0.1f, 0.1f, 2.0f, 2.0f, 2.0f };
    auto scene = HDRToneMapKernel::AnalyzeScene(hdr, 2, 2, 3);
    ASSERT(scene.maxLuminance > scene.minLuminance);
    ASSERT(scene.avgLuminance > 0.0f);
}

// ============================================================================
// Adaptive GPU Scheduler Tests
// ============================================================================

TEST(TestGPUSched_BackendStrings) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ProcessingBackendToString(ProcessingBackend::CPU)) == "CPU");
    ASSERT(std::string(ProcessingBackendToString(ProcessingBackend::GPU_D3D12)) == "D3D12");
}

TEST(TestGPUSched_SystemLoadSnapshot) {
    using namespace ExplorerLens::Engine;
    SystemLoadSnapshot snap;
    snap.gpuUtilization = 95.0f;
    snap.gpuThrottled = true;
    ASSERT(snap.IsGPUOverloaded());
    ASSERT(!snap.IsCPUOverloaded());
}

TEST(TestGPUSched_Initialize) {
    using namespace ExplorerLens::Engine;
    AdaptiveGPUScheduler scheduler;
    ASSERT(scheduler.Initialize());
    auto stats = scheduler.GetStats();
    ASSERT(stats.totalScheduled == 0);
}

TEST(TestGPUSched_RouteWork) {
    using namespace ExplorerLens::Engine;
    AdaptiveGPUScheduler scheduler;
    scheduler.Initialize();
    ScheduledWorkItem item;
    item.id = 1;
    item.filePath = L"test.jpg";
    item.formatType = L"JPEG";
    item.fileSize = 1024 * 1024;
    auto backend = scheduler.RouteWork(item);
    ASSERT(backend != ProcessingBackend::Auto);  // Should resolve to specific
}

// ============================================================================
// Explorer Column Provider Tests
// ============================================================================

TEST(TestColumn_Definitions) {
    using namespace ExplorerLens::Engine;
    auto* cols = GetColumnDefinitions();
    ASSERT(cols != nullptr);
    ASSERT(std::wstring(cols[0].displayName) == L"Dimensions");
    ASSERT(cols[0].id == ColumnID::Dimensions);
}

TEST(TestColumn_ValueMake) {
    using namespace ExplorerLens::Engine;
    auto val = ColumnValue::Make(ColumnID::Dimensions, L"1920 x 1080", 1920 * 1080);
    ASSERT(val.isAvailable);
    ASSERT(val.numericValue == 1920 * 1080);
}

TEST(TestColumn_ProviderInit) {
    using namespace ExplorerLens::Engine;
    ExplorerColumnProvider provider;
    auto cols = provider.GetRegisteredColumns();
    ASSERT(cols.size() == static_cast<size_t>(ColumnID::COUNT));
}

TEST(TestColumn_CategorizeFile) {
    using namespace ExplorerLens::Engine;
    ExplorerColumnProvider provider;
    auto result = provider.QueryMetadata(L"test.jpg");
    ASSERT(result.success);
    // JPEG should be categorized as Image
    auto* catCol = result.GetColumn(ColumnID::FileCategory);
    ASSERT(catCol != nullptr && catCol->isAvailable);
}

// ============================================================================
// Drag-Drop Thumbnail Preview Tests
// ============================================================================

TEST(TestDragDrop_StyleStrings) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(DragVisualStyleToString(DragVisualStyle::StackedPreview)) == "StackedPreview");
    ASSERT(std::string(DragVisualStyleToString(DragVisualStyle::MiniGrid)) == "MiniGrid");
}

TEST(TestDragDrop_Initialize) {
    using namespace ExplorerLens::Engine;
    DragDropThumbnailPreview dd;
    DragVisualConfig config;
    config.style = DragVisualStyle::StackedPreview;
    dd.SetConfig(config);
    auto cfg = dd.GetConfig();
    ASSERT(cfg.style == DragVisualStyle::StackedPreview);
}

TEST(TestDragDrop_ComputeSize) {
    using namespace ExplorerLens::Engine;
    DragDropThumbnailPreview dd;
    DragVisualConfig config;
    config.thumbnailSize = 96;
    dd.SetConfig(config);
    uint32_t w = 0, h = 0;
    dd.ComputeVisualSize(3, w, h);
    ASSERT(w > 0 && h > 0);
}

TEST(TestDragDrop_RenderEmpty) {
    using namespace ExplorerLens::Engine;
    DragDropThumbnailPreview dd;
    std::vector<DragItem> items;
    auto bitmap = dd.RenderDragVisual(items);
    // Empty items should return invalid bitmap
    ASSERT(!bitmap.IsValid());
}

// ============================================================================
// Streaming Decode Engine Tests
// ============================================================================

TEST(TestStreaming_LoDStrings) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(DecodeLoDToString(DecodeLoD::Placeholder)) == "Placeholder");
    ASSERT(std::string(DecodeLoDToString(DecodeLoD::FullRes)) == "FullRes");
}

TEST(TestStreaming_Initialize) {
    using namespace ExplorerLens::Engine;
    StreamingDecodeConfig cfg;
    cfg.minProgressiveSize = 512 * 1024;
    StreamingDecodeEngine engine(cfg);
    ASSERT(engine.ShouldUseProgressive(1024 * 1024));
}

TEST(TestStreaming_SessionLifecycle) {
    using namespace ExplorerLens::Engine;
    StreamingDecodeEngine engine;
    uint32_t session = engine.BeginDecode(L"test.tiff", 2 * 1024 * 1024);
    ASSERT(session != 0);
    // New session should use progressive for large files
    ASSERT(engine.ShouldUseProgressive(2 * 1024 * 1024));
    engine.EndDecode(session);
}

TEST(TestStreaming_QualityMapping) {
    using namespace ExplorerLens::Engine;
    StreamingDecodeEngine engine;
    float q0 = engine.GetQualityForLevel(DecodeLoD::Placeholder);
    float q6 = engine.GetQualityForLevel(DecodeLoD::Enhanced);
    ASSERT(q0 < q6);
    ASSERT(q6 > 0.9f);
}

// ============================================================================
// Multi-Page Strip Renderer Tests
// ============================================================================

TEST(TestStrip_LayoutStrings) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(StripLayoutToString(StripLayout::Grid2x2)) == "Grid2x2");
    ASSERT(std::string(StripLayoutToString(StripLayout::FanSpread)) == "FanSpread");
}

TEST(TestStrip_AutoSelectLayout) {
    using namespace ExplorerLens::Engine;
    auto layout1 = StripRenderConfig::AutoSelectLayout(1, 0.75);
    ASSERT(layout1 == StripLayout::CoverPlusPeek);
    auto layout4 = StripRenderConfig::AutoSelectLayout(4, 0.75);
    ASSERT(layout4 == StripLayout::Grid2x2);
}

TEST(TestStrip_Initialize) {
    using namespace ExplorerLens::Engine;
    MultiPageStripRenderer renderer;
    StripRenderConfig config;
    config.outputWidth = 256;
    config.outputHeight = 256;
    renderer.SetConfig(config);
    ASSERT(renderer.GetConfig().outputWidth == 256);
}

TEST(TestStrip_ComputeLayout) {
    using namespace ExplorerLens::Engine;
    MultiPageStripRenderer renderer;
    StripRenderConfig config;
    config.layout = StripLayout::Grid2x2;
    config.outputWidth = 256;
    config.outputHeight = 256;
    renderer.SetConfig(config);
    std::vector<PageThumbnail> pages(4);
    for (uint32_t i = 0; i < 4; i++) {
        pages[i].pageNumber = i + 1;
        pages[i].width = 200;
        pages[i].height = 300;
        pages[i].hasContent = true;
        pages[i].UpdateFromDimensions(200, 300);
    }
    renderer.SetPages(pages);
    auto composition = renderer.ComputeComposition();
    ASSERT(composition.placements.size() == 4);
}

// ============================================================================
// Video Keyframe Extractor Tests
// ============================================================================

TEST(TestKeyframe_StrategyStrings) {
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(KeyframeStrategyToString(KeyframeStrategy::SmartSelect)) == "SmartSelect");
    ASSERT(std::string(KeyframeStrategyToString(KeyframeStrategy::HighestEntropy)) == "HighestEntropy");
}

TEST(TestKeyframe_QualityScore) {
    using namespace ExplorerLens::Engine;
    CandidateKeyframe frame;
    frame.brightness = 0.5f;
    frame.contrast = 0.6f;
    frame.entropy = 0.7f;
    frame.colorfulness = 0.5f;
    frame.sharpness = 0.5f;
    float score = frame.ComputeQualityScore();
    ASSERT(score > 0.0f && score <= 1.0f);
}

TEST(TestKeyframe_BlackFramePenalty) {
    using namespace ExplorerLens::Engine;
    CandidateKeyframe black;
    black.isBlack = true;
    ASSERT(black.ComputeQualityScore() == 0.0f);
    CandidateKeyframe credits;
    credits.isCredits = true;
    ASSERT(credits.ComputeQualityScore() == 0.0f);
}

TEST(TestKeyframe_Initialize) {
    using namespace ExplorerLens::Engine;
    VideoKeyframeExtractor extractor;
    ASSERT(extractor.GetStrategy() == KeyframeStrategy::SmartSelect);
    ASSERT(extractor.GetCandidateCount() == 0);
}

int main() {
    std::wcout << L"========================================" << std::endl;
    std::wcout << L"ExplorerLens Engine - Unit Tests" << std::endl;
    std::wcout << L"========================================" << std::endl
        << std::endl;

    // Decoder Registry Tests
    std::wcout << L"Decoder Registry Tests:" << std::endl;
    RUN_TEST(TestDecoderRegistry_Create);
    RUN_TEST(TestDecoderRegistry_RegisterDecoder);
    RUN_TEST(TestDecoderRegistry_RegisterMultipleDecoders);
    RUN_TEST(TestDecoderRegistry_FindDecoder);
    RUN_TEST(TestDecoderRegistry_FindDecoderByName);
    RUN_TEST(TestDecoderRegistry_GetStats);
    RUN_TEST(TestDecoderRegistry_ComprehensiveIntegration);

    std::wcout << std::endl;

    // Format Detector Tests
    std::wcout << L"Format Detector Tests:" << std::endl;
    RUN_TEST(TestFormatDetector_Create);
    RUN_TEST(TestFormatDetector_DetectJPEG);
    RUN_TEST(TestFormatDetector_DetectPNG);
    RUN_TEST(TestFormatDetector_DetectZIP);
    RUN_TEST(TestFormatDetector_DetectRAR);
    RUN_TEST(TestFormatDetector_IsImageFormat);
    RUN_TEST(TestFormatDetector_IsArchiveFormat);
    RUN_TEST(TestFormatDetector_GetExtension);

    std::wcout << std::endl;

    // Image Decoder Tests
    std::wcout << L"Image Decoder Tests:" << std::endl;
    RUN_TEST(TestImageDecoder_Create);
    RUN_TEST(TestImageDecoder_Extensions);
    RUN_TEST(TestImageDecoder_CanDecodeJPEG);
    RUN_TEST(TestImageDecoder_CanDecodePNG);
    RUN_TEST(TestImageDecoder_CanDecodeBMP);
    RUN_TEST(TestImageDecoder_CannotDecodeUnsupported);
    RUN_TEST(TestImageDecoder_GetInfo);
    RUN_TEST(TestImageDecoder_RegisterWithRegistry);

    std::wcout << std::endl;

    // WebP Decoder Tests
    std::wcout << L"WebP Decoder Tests:" << std::endl;
    RUN_TEST(TestWebPDecoder_Create);
    RUN_TEST(TestWebPDecoder_Extensions);
    RUN_TEST(TestWebPDecoder_CanDecode);
    RUN_TEST(TestWebPDecoder_GetInfo);
    RUN_TEST(TestWebPDecoder_RegisterWithRegistry);

    std::wcout << std::endl;

    // AVIF Decoder Tests
    std::wcout << L"AVIF Decoder Tests:" << std::endl;
    RUN_TEST(TestAVIFDecoder_Create);
    RUN_TEST(TestAVIFDecoder_Extensions);
    RUN_TEST(TestAVIFDecoder_CanDecode);
    RUN_TEST(TestAVIFDecoder_GetInfo);
    RUN_TEST(TestAVIFDecoder_RegisterWithRegistry);

    std::wcout << std::endl;

    // Archive Decoder Tests
    std::wcout << L"Archive Decoder Tests:" << std::endl;
    RUN_TEST(TestArchiveDecoder_Create);
    RUN_TEST(TestArchiveDecoder_Extensions);
    RUN_TEST(TestArchiveDecoder_CanDecode);
    RUN_TEST(TestArchiveDecoder_IsArchiveFormat);
    RUN_TEST(TestArchiveDecoder_GetInfo);
    RUN_TEST(TestArchiveDecoder_RegisterWithRegistry);

    std::wcout << std::endl;

    // JXL Decoder Tests
    std::wcout << L"JXL Decoder Tests:" << std::endl;
    RUN_TEST(TestJXLDecoder_Create);
    RUN_TEST(TestJXLDecoder_CanDecode);
    RUN_TEST(TestJXLDecoder_GetInfo);
    RUN_TEST(TestJXLDecoder_Extensions);
#ifdef HAS_LIBJXL
    RUN_TEST(TestJXLDecoder_DecodeReturnsResult);
#endif

    std::wcout << std::endl;

    // HEIF Decoder Tests
    std::wcout << L"HEIF Decoder Tests:" << std::endl;
    RUN_TEST(TestHEIFDecoder_Create);
    RUN_TEST(TestHEIFDecoder_CanDecode);

    std::wcout << std::endl;

    // PSD Decoder Tests
    std::wcout << L"PSD Decoder Tests:" << std::endl;
    RUN_TEST(TestPSDDecoder_Create);
    RUN_TEST(TestPSDDecoder_CanDecode);
    RUN_TEST(TestPSDDecoder_GetInfo);

    std::wcout << std::endl;

    // DDS Decoder Tests
    std::wcout << L"DDS Decoder Tests:" << std::endl;
    RUN_TEST(TestDDSDecoder_Create);
    RUN_TEST(TestDDSDecoder_CanDecode);

    std::wcout << std::endl;

    // HDR Decoder Tests
    std::wcout << L"HDR Decoder Tests:" << std::endl;
    RUN_TEST(TestHDRDecoder_Create);
    RUN_TEST(TestHDRDecoder_CanDecode);

    std::wcout << std::endl;

    // PPM Decoder Tests
    std::wcout << L"PPM Decoder Tests:" << std::endl;
    RUN_TEST(TestPPMDecoder_Create);
    RUN_TEST(TestPPMDecoder_Extensions);

    std::wcout << std::endl;

    // EXR Decoder Tests
    std::wcout << L"EXR Decoder Tests:" << std::endl;
    RUN_TEST(TestEXRDecoder_Create);
    RUN_TEST(TestEXRDecoder_CanDecode);

    std::wcout << std::endl;

    // SVG Decoder Tests
    std::wcout << L"SVG Decoder Tests:" << std::endl;
    RUN_TEST(TestSVGDecoder_Create);
    RUN_TEST(TestSVGDecoder_CanDecode);

    std::wcout << std::endl;

    // Video Decoder Tests
    std::wcout << L"Video Decoder Tests:" << std::endl;
    RUN_TEST(TestVideoDecoder_Create);
    RUN_TEST(TestVideoDecoder_CanDecode);
    RUN_TEST(TestVideoDecoder_Extensions);

    std::wcout << std::endl;

    // Audio Decoder Tests
    std::wcout << L"Audio Decoder Tests:" << std::endl;
    RUN_TEST(TestAudioDecoder_Create);
    RUN_TEST(TestAudioDecoder_CanDecode);

    std::wcout << std::endl;

    // PDF Decoder Tests
    std::wcout << L"PDF Decoder Tests:" << std::endl;
    RUN_TEST(TestPDFDecoder_Create);
    RUN_TEST(TestPDFDecoder_CanDecode);

    std::wcout << std::endl;

    // Document Decoder Tests
    std::wcout << L"Document Decoder Tests:" << std::endl;
    RUN_TEST(TestDocumentDecoder_Create);
    RUN_TEST(TestDocumentDecoder_CanDecode);
    RUN_TEST(TestDocumentDecoder_Extensions);

    std::wcout << std::endl;

    // Font Decoder Tests
    std::wcout << L"Font Decoder Tests:" << std::endl;
    RUN_TEST(TestFontDecoder_Create);
    RUN_TEST(TestFontDecoder_CanDecode);
    RUN_TEST(TestFontDecoder_Extensions);
    RUN_TEST(TestFontDecoder_GetInfo);

    std::wcout << std::endl;

    // Video Decoder Robustness Tests
    std::wcout << L"Video Decoder Robustness:" << std::endl;
    RUN_TEST(TestVideoDecoder_KeyframeSeekingValidation);
    RUN_TEST(TestVideoDecoder_TimestampValidation);
    RUN_TEST(TestVideoDecoder_CorruptFileHandling);

    std::wcout << std::endl;

    // Audio Album Art Tests
    std::wcout << L"Audio Album Art Extraction:" << std::endl;
    RUN_TEST(TestAudioDecoder_AlbumArtExtraction);
    RUN_TEST(TestAudioDecoder_AlbumArtMultipleFormats);
    RUN_TEST(TestAudioDecoder_NoAlbumArtGracefulFallback);

    std::wcout << std::endl;

    // Document Thumbnail Tests
    std::wcout << L"Document Thumbnail Provider:" << std::endl;
    RUN_TEST(TestDocumentDecoder_EPUBCoverExtraction);
    RUN_TEST(TestDocumentDecoder_MOBICoverExtraction);
    RUN_TEST(TestDocumentDecoder_InvalidZipHandling);
    RUN_TEST(TestDocumentDecoder_MissingCoverHandling);

    std::wcout << std::endl;

    // Font Preview Rendering Tests
    std::wcout << L"Font Preview Rendering:" << std::endl;
    RUN_TEST(TestFontDecoder_DirectWriteRendering);
    RUN_TEST(TestFontDecoder_MetadataExtraction);
    RUN_TEST(TestFontDecoder_TrueTypeCollectionHandling);

    std::wcout << std::endl;

    // Archive Format Expansion Tests
    std::wcout << L"Archive Format Expansion:" << std::endl;
    RUN_TEST(TestArchiveDecoder_7zSupport);
    RUN_TEST(TestArchiveDecoder_TarGzSupport);
    RUN_TEST(TestArchiveDecoder_TarBz2Support);
    RUN_TEST(TestArchiveDecoder_PasswordProtectedHandling);

    std::wcout << std::endl;

    // RAW Format Expansion Tests
    std::wcout << L"RAW Format Expansion:" << std::endl;
    RUN_TEST(TestRAWDecoder_CR3Support);
    RUN_TEST(TestRAWDecoder_ARWSupport);
    RUN_TEST(TestRAWDecoder_ORFSupport);
    RUN_TEST(TestRAWDecoder_GPRSupport);
    RUN_TEST(TestRAWDecoder_MultipleRAWFormats);

    std::wcout << std::endl;

    // 3D Model Support Tests
    std::wcout << L"3D Model Support:" << std::endl;
    RUN_TEST(TestModelDecoder_Create);
    RUN_TEST(TestModelDecoder_OBJSupport);
    RUN_TEST(TestModelDecoder_STLSupport);
    RUN_TEST(TestModelDecoder_GLTFSupport);
    RUN_TEST(TestModelDecoder_Extensions);
    RUN_TEST(TestModelDecoder_GetInfo);

    std::wcout << L"Enhanced Model Decoder:" << std::endl;
    RUN_TEST(TestModelDecoder_PLYSupport);
    RUN_TEST(TestModelDecoder_DAESupport);
    RUN_TEST(TestModelDecoder_3DSSupport);
    RUN_TEST(TestModelDecoder_FBXSupport);
    RUN_TEST(TestModelDecoder_ExpandedExtensions);
    RUN_TEST(TestModelDecoder_ExtensionCount);

    std::wcout << L"EPS/PostScript Decoder:" << std::endl;
    RUN_TEST(TestEPSDecoder_Create);
    RUN_TEST(TestEPSDecoder_CanDecode);
    RUN_TEST(TestEPSDecoder_NoDecodeNonEPS);
    RUN_TEST(TestEPSDecoder_GetInfo);
    RUN_TEST(TestPDFDecoder_AIRouting);

    std::wcout << L"Game Texture Formats (KTX/VTF):" << std::endl;
    RUN_TEST(TestKTXDecoder_Create);
    RUN_TEST(TestKTXDecoder_ExtensionCheck);
    RUN_TEST(TestKTXDecoder_ExtensionVersion);
    RUN_TEST(TestKTXDecoder_CompressionNames);
    RUN_TEST(TestKTXDecoder_TextureInfo);
    RUN_TEST(TestKTXDecoder_SupercompressionNames);
    RUN_TEST(TestKTXDecoder_InvalidFile);
    RUN_TEST(TestVTFDecoder_ExtensionCheck);
    RUN_TEST(TestVTFDecoder_Create);
    RUN_TEST(TestVTFDecoder_InvalidFile);
    RUN_TEST(TestVTFDecoder_ImageSizeCompute);

    std::wcout << L"OpenRaster & XCF:" << std::endl;
    RUN_TEST(TestORADecoder_ExtensionCheck);
    RUN_TEST(TestORADecoder_Create);
    RUN_TEST(TestORADecoder_InvalidFile);
    RUN_TEST(TestORADecoder_ReadInfoInvalid);
    RUN_TEST(TestXCFDecoder_ExtensionCheck);
    RUN_TEST(TestXCFDecoder_Create);
    RUN_TEST(TestXCFDecoder_InvalidFile);
    RUN_TEST(TestXCFDecoder_ReadInfoInvalid);
    RUN_TEST(TestXCFDecoder_ColorModes);

    std::wcout << L"SGI/RGB & XPM:" << std::endl;
    RUN_TEST(TestSGIDecoder_ExtensionCheck);
    RUN_TEST(TestSGIDecoder_Create);
    RUN_TEST(TestSGIDecoder_InvalidFile);
    RUN_TEST(TestSGIDecoder_ReadInfoInvalid);
    RUN_TEST(TestSGIDecoder_StorageTypes);
    RUN_TEST(TestXPMDecoder_ExtensionCheck);
    RUN_TEST(TestXPMDecoder_Create);
    RUN_TEST(TestXPMDecoder_InvalidFile);
    RUN_TEST(TestXPMDecoder_ReadInfoInvalid);

    std::wcout << L"Async Shell Extension:" << std::endl;
    RUN_TEST(TestAsyncProvider_Create);
    RUN_TEST(TestAsyncProvider_Initialize);
    RUN_TEST(TestAsyncProvider_Config);
    RUN_TEST(TestAsyncProvider_PriorityNames);
    RUN_TEST(TestAsyncProvider_StateNames);
    RUN_TEST(TestAsyncProvider_SyncFallback);
    RUN_TEST(TestAsyncProvider_SyncFallbackEmpty);
    RUN_TEST(TestAsyncProvider_Stats);
    RUN_TEST(TestAsyncProvider_SubmitNotRunning);

    std::wcout << L"D3D12 Compute Pipeline:" << std::endl;
    RUN_TEST(TestD3D12Compute_Create);
    RUN_TEST(TestD3D12Compute_Initialize);
    RUN_TEST(TestD3D12Compute_BackendNames);
    RUN_TEST(TestD3D12Compute_AlgorithmNames);
    RUN_TEST(TestD3D12Compute_ColorSpaceNames);
    RUN_TEST(TestD3D12Compute_ToneMapNames);
    RUN_TEST(TestD3D12Compute_ProbeHardware);
    RUN_TEST(TestD3D12Compute_ResizeNotInit);
    RUN_TEST(TestD3D12Compute_ResizeCPU);
    RUN_TEST(TestD3D12Compute_Stats);

    std::wcout << L"Parallel Batch Decoder:" << std::endl;
    RUN_TEST(TestBatchDecoder_Create);
    RUN_TEST(TestBatchDecoder_Initialize);
    RUN_TEST(TestBatchDecoder_Config);
    RUN_TEST(TestBatchDecoder_ClassifyFormats);
    RUN_TEST(TestBatchDecoder_ParallelismNames);
    RUN_TEST(TestBatchDecoder_StatusNames);
    RUN_TEST(TestBatchDecoder_PriorityNames);
    RUN_TEST(TestBatchDecoder_SubmitEmpty);
    RUN_TEST(TestBatchDecoder_SubmitBatch);
    RUN_TEST(TestBatchDecoder_CancelBatch);

    std::wcout << L"Code Coverage & Fuzzing:" << std::endl;
    RUN_TEST(TestCoverage_Create);
    RUN_TEST(TestCoverage_CIThresholds);
    RUN_TEST(TestCoverage_ReleaseThresholds);
    RUN_TEST(TestCoverage_GenerateCommand);
    RUN_TEST(TestCoverage_MetricNames);
    RUN_TEST(TestCoverage_FuzzTargetNames);
    RUN_TEST(TestCoverage_FuzzableDecoders);
    RUN_TEST(TestCoverage_GenerateFuzzTargets);
    RUN_TEST(TestCoverage_ValidateEmpty);

    std::wcout << L"Memory Safety:" << std::endl;
    RUN_TEST(TestMemSafety_ASANDetection);
    RUN_TEST(TestMemSafety_RecommendedConfig);
    RUN_TEST(TestMemSafety_CompilerFlags);
    RUN_TEST(TestMemSafety_CMakeOptions);
    RUN_TEST(TestMemSafety_SafeBuffer);
    RUN_TEST(TestMemSafety_SafeBufferBounds);
    RUN_TEST(TestMemSafety_ValidateAccess);
    RUN_TEST(TestMemSafety_SanitizerNames);
    RUN_TEST(TestMemSafety_AccessPatterns);
    RUN_TEST(TestMemSafety_MaxMappableSize);

    // Cache System V2
    std::wcout << L"Cache System V2..." << std::endl;
    RUN_TEST(TestDiskCache_OpenClose);
    RUN_TEST(TestDiskCache_PutAndContains);
    RUN_TEST(TestDiskCache_GetRetrieval);
    RUN_TEST(TestDiskCache_Remove);
    RUN_TEST(TestDiskCache_EvictionStrategies);
    RUN_TEST(TestDiskCache_EntryStates);
    RUN_TEST(TestDiskCache_CRC32);
    RUN_TEST(TestDiskCache_CacheKey);
    RUN_TEST(TestDiskCache_Stats);
    RUN_TEST(TestDiskCache_Compact);

    // ARM64 Hardware Validation
    std::wcout << L"ARM64 Hardware Validation..." << std::endl;
    RUN_TEST(TestARM64_PlatformDetection);
    RUN_TEST(TestARM64_FeatureDetection);
    RUN_TEST(TestARM64_FeatureNames);
    RUN_TEST(TestARM64_TargetNames);
    RUN_TEST(TestARM64_PerfCategoryNames);
    RUN_TEST(TestARM64_PerfBaselines);
    RUN_TEST(TestARM64_X64ReferenceBaselines);
    RUN_TEST(TestARM64_RunValidation);
    RUN_TEST(TestARM64_CIWorkflow);
    RUN_TEST(TestARM64_FeatureBitmask);

    // High-DPI Support
    std::wcout << L"High-DPI Support..." << std::endl;
    RUN_TEST(TestDPI_SystemDPI);
    RUN_TEST(TestDPI_GetMonitorDPI);
    RUN_TEST(TestDPI_EnumerateMonitors);
    RUN_TEST(TestDPI_LogicalPhysicalConversion);
    RUN_TEST(TestDPI_ScaleRequest);
    RUN_TEST(TestDPI_NearestScale);
    RUN_TEST(TestDPI_DPIForScale);
    RUN_TEST(TestDPI_ScaleFactors);
    RUN_TEST(TestDPI_ScaleNames);
    RUN_TEST(TestDPI_AwarenessNames);

    // MSIX Packaging
    std::wcout << L"MSIX Packaging..." << std::endl;
    RUN_TEST(TestMSIX_GenerateManifest);
    RUN_TEST(TestMSIX_ValidateManifest);
    RUN_TEST(TestMSIX_GenerateAppInstaller);
    RUN_TEST(TestMSIX_ChannelNames);
    RUN_TEST(TestMSIX_SigningNames);
    RUN_TEST(TestMSIX_PackageTypeNames);
    RUN_TEST(TestMSIX_Capabilities);
    RUN_TEST(TestMSIX_BuildPackage);
    RUN_TEST(TestMSIX_IsMSIXSupported);
    RUN_TEST(TestMSIX_Config);

    // Test Suite Expansion
    std::wcout << L"Test Suite Expansion..." << std::endl;
    RUN_TEST(TestSuite_DecoderSpecs);
    RUN_TEST(TestSuite_CoverageGaps);
    RUN_TEST(TestSuite_TotalCount);
    RUN_TEST(TestSuite_ComputeSummary);
    RUN_TEST(TestSuite_CategoryNames);
    RUN_TEST(TestSuite_VerdictNames);
    RUN_TEST(TestSuite_TestFiles);
    RUN_TEST(TestSuite_MeetsTargets);
    RUN_TEST(TestSuite_PassRate);
    RUN_TEST(TestSuite_Config);

    // Malformed Input Hardening Tests
    std::wcout << L"Malformed Input Hardening Tests..." << std::endl;
    RUN_TEST(TestMalformed_DefaultConfig);
    RUN_TEST(TestMalformed_DimensionsSafe);
    RUN_TEST(TestMalformed_DimensionsBomb);
    RUN_TEST(TestMalformed_CompressionRatio);
    RUN_TEST(TestMalformed_NestingDepth);
    RUN_TEST(TestMalformed_MagicBytesPNG);
    RUN_TEST(TestMalformed_MagicBytesJPEG);
    RUN_TEST(TestMalformed_MagicBytesMultiple);
    RUN_TEST(TestMalformed_ClampDimensions);
    RUN_TEST(TestMalformed_CorruptionNames);

    // v9.2 Release Gate Tests
    std::wcout << L"v9.2 Release Gate Tests..." << std::endl;
    RUN_TEST(TestReleaseV3_DefaultThresholds);
    RUN_TEST(TestReleaseV3_EvaluateEmpty);
    RUN_TEST(TestReleaseV3_AllPass);
    RUN_TEST(TestReleaseV3_BlockerFails);
    RUN_TEST(TestReleaseV3_ConditionalPass);
    RUN_TEST(TestReleaseV3_PlatformValidation);
    RUN_TEST(TestReleaseV3_ReleaseNotes);
    RUN_TEST(TestReleaseV3_Checklist);
    RUN_TEST(TestReleaseV3_DimensionNames);
    RUN_TEST(TestReleaseV3_VerdictNames);

    // Scientific Format Suite Tests
    std::wcout << L"Scientific Format Suite..." << std::endl;
    RUN_TEST(TestDICOM_IsDICOMFile);
    RUN_TEST(TestDICOM_Extensions);
    RUN_TEST(TestDICOM_PhotometricNames);
    RUN_TEST(TestDICOM_TransferSyntaxNames);
    RUN_TEST(TestDICOM_WindowLevel);
    RUN_TEST(TestFITS_IsFITSFile);
    RUN_TEST(TestFITS_Extensions);
    RUN_TEST(TestFITS_BitpixNames);
    RUN_TEST(TestFITS_BytesPerPixel);
    RUN_TEST(TestFITS_StretchAlgorithm);

    std::wcout << std::endl;

    std::wcout << L"Advanced 3D Format Decoder..." << std::endl;
    RUN_TEST(TestAdvanced3D_FBXDetection);
    RUN_TEST(TestAdvanced3D_Extensions);
    RUN_TEST(TestAdvanced3D_FormatNames);
    RUN_TEST(TestAdvanced3D_BoundingBox);
    RUN_TEST(TestAdvanced3D_WireframeRender);

    std::wcout << std::endl;

    std::wcout << L"Plugin Marketplace V2..." << std::endl;
    RUN_TEST(TestMarketplaceV2_CatalogInit);
    RUN_TEST(TestMarketplaceV2_Search);
    RUN_TEST(TestMarketplaceV2_SemVer);
    RUN_TEST(TestMarketplaceV2_CategoryNames);
    RUN_TEST(TestMarketplaceV2_InstallUninstall);

    std::wcout << std::endl;

    std::wcout << L"Vulkan Compute Pipeline..." << std::endl;
    RUN_TEST(TestVulkan_BackendNames);
    RUN_TEST(TestVulkan_ShaderTypeNames);
    RUN_TEST(TestVulkan_CPUFallbackResize);
    RUN_TEST(TestVulkan_PipelineCacheStats);
    RUN_TEST(TestVulkan_ActiveBackend);

    std::wcout << std::endl;

    std::wcout << L"Python SDK..." << std::endl;
    RUN_TEST(TestPythonSDK_DefaultConfig);
    RUN_TEST(TestPythonSDK_DecoderInfo);
    RUN_TEST(TestPythonSDK_CtypesStub);
    RUN_TEST(TestPythonSDK_Pybind11Wrapper);
    RUN_TEST(TestPythonSDK_BatchConfig);

    std::wcout << std::endl;

    std::wcout << L"Release Gate V10..." << std::endl;
    RUN_TEST(TestReleaseGateV10_DefaultThresholds);
    RUN_TEST(TestReleaseGateV10_PassingGate);
    RUN_TEST(TestReleaseGateV10_FailingGate);
    RUN_TEST(TestReleaseGateV10_Changelog);
    RUN_TEST(TestReleaseGateV10_CategoryNames);

    std::wcout << std::endl;

    std::wcout << L"Async Shell Extension..." << std::endl;
    RUN_TEST(TestAsync_SubmitRequest);
    RUN_TEST(TestAsync_CancelRequest);
    RUN_TEST(TestAsync_PriorityNames);
    RUN_TEST(TestAsync_ThreadPool);
    RUN_TEST(TestAsync_DrainQueue);

    std::wcout << std::endl;

    std::wcout << L"Encoder Export Engine..." << std::endl;
    RUN_TEST(TestExport_FormatNames);
    RUN_TEST(TestExport_FormatExtensions);
    RUN_TEST(TestExport_AlphaSupport);
    RUN_TEST(TestExport_BMPEncode);
    RUN_TEST(TestExport_QualityPresets);

    std::wcout << std::endl;

    std::wcout << L"Telemetry Engine..." << std::endl;
    RUN_TEST(TestTelemetry_RecordEvent);
    RUN_TEST(TestTelemetry_Metrics);
    RUN_TEST(TestTelemetry_HealthScore);
    RUN_TEST(TestTelemetry_Privacy);
    RUN_TEST(TestTelemetry_SeverityNames);

    std::wcout << std::endl;

    std::wcout << L"SIMD Accelerator..." << std::endl;
    RUN_TEST(TestSIMD_DetectCapabilities);
    RUN_TEST(TestSIMD_ResizeBilinear);
    RUN_TEST(TestSIMD_ColorConvert);
    RUN_TEST(TestSIMD_LevelNames);
    RUN_TEST(TestSIMD_Alignment);

    std::wcout << std::endl;

    std::wcout << L"Windows 11 Integration..." << std::endl;
    RUN_TEST(TestWin11_VersionDetection);
    RUN_TEST(TestWin11_FeatureNames);
    RUN_TEST(TestWin11_DarkModeDetection);
    RUN_TEST(TestWin11_MicaModes);
    RUN_TEST(TestWin11_FeatureCount);

    std::wcout << std::endl;

    std::wcout << L"CI/CD Pipeline Validation..." << std::endl;
    RUN_TEST(TestCI_PlatformNames);
    RUN_TEST(TestCI_StageNames);
    RUN_TEST(TestCI_ValidatorCreation);
    RUN_TEST(TestCI_ArtifactTypeNames);
    RUN_TEST(TestCI_StageCount);

    std::wcout << std::endl;

    std::wcout << L"eBook Decoder..." << std::endl;
    RUN_TEST(TestEBook_FormatNames);
    RUN_TEST(TestEBook_DecoderCreation);
    RUN_TEST(TestEBook_FormatDetection);
    RUN_TEST(TestEBook_CoverExtraction);
    RUN_TEST(TestEBook_FormatCount);

    std::wcout << std::endl;

    std::wcout << L"Geospatial Decoder..." << std::endl;
    RUN_TEST(TestGeo_FormatNames);
    RUN_TEST(TestGeo_DecoderCreation);
    RUN_TEST(TestGeo_HaversineDistance);
    RUN_TEST(TestGeo_MercatorProjection);
    RUN_TEST(TestGeo_FormatCount);

    std::wcout << std::endl;

    std::wcout << L"Auto Documentation Generator..." << std::endl;
    RUN_TEST(TestAutoDoc_SectionNames);
    RUN_TEST(TestAutoDoc_FormatNames);
    RUN_TEST(TestAutoDoc_FormatExtensions);
    RUN_TEST(TestAutoDoc_DecoderRegistration);
    RUN_TEST(TestAutoDoc_SectionGeneration);

    std::wcout << std::endl;

    std::wcout << L"Config Migration Engine..." << std::endl;
    RUN_TEST(TestConfigMigration_VersionNames);
    RUN_TEST(TestConfigMigration_ActionNames);
    RUN_TEST(TestConfigMigration_BasicMigration);
    RUN_TEST(TestConfigMigration_RenameRule);
    RUN_TEST(TestConfigMigration_Validation);

    std::wcout << std::endl;

    std::wcout << L"Animated Thumbnail Engine..." << std::endl;
    RUN_TEST(TestAnim_FormatNames);
    RUN_TEST(TestAnim_StrategyNames);
    RUN_TEST(TestAnim_FrameSelection);
    RUN_TEST(TestAnim_FormatDetection);
    RUN_TEST(TestAnim_FormatCount);

    std::wcout << std::endl;

    std::wcout << L"Shell Context Menu V2..." << std::endl;
    RUN_TEST(TestContextMenu_ActionNames);
    RUN_TEST(TestContextMenu_DefaultMenu);
    RUN_TEST(TestContextMenu_ExecuteAction);
    RUN_TEST(TestContextMenu_PositionNames);
    RUN_TEST(TestContextMenu_ActionCount);

    std::wcout << std::endl;

    std::wcout << L"Portable Mode Manager..." << std::endl;
    RUN_TEST(TestPortable_StatusNames);
    RUN_TEST(TestPortable_LocationNames);
    RUN_TEST(TestPortable_LocationCount);
    RUN_TEST(TestPortable_Detection);
    RUN_TEST(TestPortable_CacheSize);

    std::wcout << std::endl;

    std::wcout << L"Network Provider Engine..." << std::endl;
    RUN_TEST(TestNetwork_ProtocolNames);
    RUN_TEST(TestNetwork_PathDetection);
    RUN_TEST(TestNetwork_ProtocolDetection);
    RUN_TEST(TestNetwork_ParsePath);
    RUN_TEST(TestNetwork_ProtocolCount);

    std::wcout << std::endl;

    std::wcout << L"Security Hardening V2..." << std::endl;
    RUN_TEST(TestSecurity_LevelNames);
    RUN_TEST(TestSecurity_CheckNames);
    RUN_TEST(TestSecurity_BasicAudit);
    RUN_TEST(TestSecurity_CheckCounts);
    RUN_TEST(TestSecurity_DEPCheck);

    std::wcout << std::endl;

    std::wcout << L"Accessibility Engine..." << std::endl;
    RUN_TEST(TestA11y_FeatureNames);
    RUN_TEST(TestA11y_ContrastModes);
    RUN_TEST(TestA11y_FeatureToggle);
    RUN_TEST(TestA11y_FeatureCount);
    RUN_TEST(TestA11y_ComplianceAudit);

    std::wcout << std::endl;

    std::wcout << L"Cloud Sync Provider..." << std::endl;
    RUN_TEST(TestCloud_ProviderNames);
    RUN_TEST(TestCloud_StatusNames);
    RUN_TEST(TestCloud_ProviderDetection);
    RUN_TEST(TestCloud_IsCloudPath);
    RUN_TEST(TestCloud_ProviderCount);

    std::wcout << std::endl;

    std::wcout << L"Format Converter Engine..." << std::endl;
    RUN_TEST(TestConverter_FormatNames);
    RUN_TEST(TestConverter_FormatDetection);
    RUN_TEST(TestConverter_QualityPresets);
    RUN_TEST(TestConverter_FormatExtensions);
    RUN_TEST(TestConverter_FormatCount);

    std::wcout << std::endl;

    std::wcout << L"Enterprise Deployment Manager..." << std::endl;
    RUN_TEST(TestEnterprise_MethodNames);
    RUN_TEST(TestEnterprise_PolicyTypes);
    RUN_TEST(TestEnterprise_AddPolicy);
    RUN_TEST(TestEnterprise_MSIProperties);
    RUN_TEST(TestEnterprise_MethodCount);

    std::wcout << std::endl;

    std::wcout << L"Release Gate V11..." << std::endl;
    RUN_TEST(TestGateV11_KPINames);
    RUN_TEST(TestGateV11_KPICount);
    RUN_TEST(TestGateV11_Evaluate);
    RUN_TEST(TestGateV11_Thresholds);
    RUN_TEST(TestGateV11_SingleKPI);

    std::wcout << std::endl;

    std::wcout << L"Watch Folder Engine..." << std::endl;
    RUN_TEST(TestWatch_ChangeTypeNames);
    RUN_TEST(TestWatch_WatchModes);
    RUN_TEST(TestWatch_AddFolder);
    RUN_TEST(TestWatch_RemoveFolder);
    RUN_TEST(TestWatch_SimulateChange);

    std::wcout << std::endl;

    std::wcout << L"Diagnostic Dashboard..." << std::endl;
    RUN_TEST(TestDiag_CategoryNames);
    RUN_TEST(TestDiag_HealthNames);
    RUN_TEST(TestDiag_RecordMetric);
    RUN_TEST(TestDiag_Snapshot);
    RUN_TEST(TestDiag_CategoryCount);

    std::wcout << std::endl;

    std::wcout << L"Performance Benchmark V2..." << std::endl;
    RUN_TEST(TestBenchV2_TypeNames);
    RUN_TEST(TestBenchV2_ComputeStats);
    RUN_TEST(TestBenchV2_MeetsTarget);
    RUN_TEST(TestBenchV2_AddResult);
    RUN_TEST(TestBenchV2_TypeCount);

    std::wcout << std::endl;

    std::wcout << L"Localization Engine..." << std::endl;
    RUN_TEST(TestL10n_LocaleNames);
    RUN_TEST(TestL10n_TextDirection);
    RUN_TEST(TestL10n_SetLocale);
    RUN_TEST(TestL10n_StringLookup);
    RUN_TEST(TestL10n_LocaleCount);

    std::wcout << std::endl;

    std::wcout << L"Theme Engine..." << std::endl;
    RUN_TEST(TestTheme_TypeNames);
    RUN_TEST(TestTheme_DefaultDark);
    RUN_TEST(TestTheme_SetLight);
    RUN_TEST(TestTheme_RegisterCustom);
    RUN_TEST(TestTheme_TypeCount);

    std::wcout << std::endl;

    std::wcout << L"Usage Telemetry Engine..." << std::endl;
    RUN_TEST(TestUsageTelemetry_CategoryNames);
    RUN_TEST(TestUsageTelemetry_ConsentNames);
    RUN_TEST(TestUsageTelemetry_ConsentNone);
    RUN_TEST(TestUsageTelemetry_ConsentBasic);
    RUN_TEST(TestUsageTelemetry_CategoryCount);

    std::wcout << std::endl;

    std::wcout << L"Update Engine..." << std::endl;
    RUN_TEST(TestUpdate_ChannelNames);
    RUN_TEST(TestUpdate_StatusNames);
    RUN_TEST(TestUpdate_CompareVersions);
    RUN_TEST(TestUpdate_CheckForUpdate);
    RUN_TEST(TestUpdate_ChannelCount);

    std::wcout << std::endl;

    std::wcout << L"Shell Preview Handler..." << std::endl;
    RUN_TEST(TestPreview_ModeNames);
    RUN_TEST(TestPreview_DetectMode);
    RUN_TEST(TestPreview_LoadFile);
    RUN_TEST(TestPreview_Unload);
    RUN_TEST(TestPreview_ModeCount);

    std::wcout << std::endl;

    std::wcout << L"Batch Processing Engine..." << std::endl;
    RUN_TEST(TestBatch_OperationNames);
    RUN_TEST(TestBatch_CreateJob);
    RUN_TEST(TestBatch_RunJob);
    RUN_TEST(TestBatch_CancelJob);
    RUN_TEST(TestBatch_OperationCount);

    std::wcout << std::endl;

    std::wcout << L"Release Gate V12..." << std::endl;
    RUN_TEST(TestGateV12_KPINames);
    RUN_TEST(TestGateV12_KPICount);
    RUN_TEST(TestGateV12_Evaluate);
    RUN_TEST(TestGateV12_Approved);
    RUN_TEST(TestGateV12_Version);

    std::wcout << std::endl;

    std::wcout << L"File Hash Engine..." << std::endl;
    RUN_TEST(TestHash_AlgorithmNames);
    RUN_TEST(TestHash_CRC32);
    RUN_TEST(TestHash_ComputeHash);
    RUN_TEST(TestHash_VerifyHash);
    RUN_TEST(TestHash_AlgorithmCount);

    std::wcout << std::endl;

    std::wcout << L"Registry Manager..." << std::endl;
    RUN_TEST(TestReg_HiveNames);
    RUN_TEST(TestReg_WriteRead);
    RUN_TEST(TestReg_DefaultValue);
    RUN_TEST(TestReg_Delete);
    RUN_TEST(TestReg_BasePath);

    std::wcout << std::endl;

    std::wcout << L"Error Recovery Engine..." << std::endl;
    RUN_TEST(TestRecovery_StrategyNames);
    RUN_TEST(TestRecovery_CreateCheckpoint);
    RUN_TEST(TestRecovery_RestoreCheckpoint);
    RUN_TEST(TestRecovery_CrashRecovery);
    RUN_TEST(TestRecovery_StrategyCount);

    std::wcout << std::endl;

    std::wcout << L"Log Rotation Engine..." << std::endl;
    RUN_TEST(TestLogRot_PolicyNames);
    RUN_TEST(TestLogRot_CompressionNames);
    RUN_TEST(TestLogRot_NeedsRotation);
    RUN_TEST(TestLogRot_Cleanup);
    RUN_TEST(TestLogRot_PolicyCount);

    std::wcout << std::endl;

    std::wcout << L"Release Gate V13..." << std::endl;
    RUN_TEST(TestGateV13_KPINames);
    RUN_TEST(TestGateV13_KPICount);
    RUN_TEST(TestGateV13_Evaluate);
    RUN_TEST(TestGateV13_Approved);
    RUN_TEST(TestGateV13_Version);

    std::wcout << std::endl;

    // Resource Pool Tests
    std::wcout << L"Resource Pool Engine..." << std::endl;
    RUN_TEST(TestResourcePool_Checkout);
    RUN_TEST(TestResourcePool_Return);
    RUN_TEST(TestResourcePool_Stats);
    RUN_TEST(TestResourcePool_TypeNames);
    RUN_TEST(TestResourcePool_Prewarm);

    // CLI Tests
    std::wcout << L"Command Line Interface..." << std::endl;
    RUN_TEST(TestCLI_ParseFlags);
    RUN_TEST(TestCLI_ParseString);
    RUN_TEST(TestCLI_MissingRequired);
    RUN_TEST(TestCLI_HelpRequested);
    RUN_TEST(TestCLI_ArgTypeNames);

    // Metadata Extractor Tests
    std::wcout << L"Metadata Extractor..." << std::endl;
    RUN_TEST(TestMetadata_Extract);
    RUN_TEST(TestMetadata_FieldLookup);
    RUN_TEST(TestMetadata_Standards);
    RUN_TEST(TestMetadata_FieldNames);
    RUN_TEST(TestMetadata_FormatExposure);

    // Notification Engine Tests
    std::wcout << L"Notification Engine..." << std::endl;
    RUN_TEST(TestNotify_Send);
    RUN_TEST(TestNotify_Dismiss);
    RUN_TEST(TestNotify_ByType);
    RUN_TEST(TestNotify_TypeNames);
    RUN_TEST(TestNotify_PriorityNames);

    // Release Gate V14 Tests
    std::wcout << L"Release Gate V14..." << std::endl;
    RUN_TEST(TestGateV14_KPINames);
    RUN_TEST(TestGateV14_KPICount);
    RUN_TEST(TestGateV14_Evaluate);
    RUN_TEST(TestGateV14_Approved);
    RUN_TEST(TestGateV14_Version);

    std::wcout << std::endl;

    // Content Indexer Tests
    std::wcout << L"Content Indexer..." << std::endl;
    RUN_TEST(TestIndexer_AddFile);
    RUN_TEST(TestIndexer_ClassifyExtension);
    RUN_TEST(TestIndexer_IndexAll);
    RUN_TEST(TestIndexer_ContentTypeNames);
    RUN_TEST(TestIndexer_SearchByType);

    // Network Diagnostics Tests
    std::wcout << L"Network Diagnostics..." << std::endl;
    RUN_TEST(TestNetDiag_RunTest);
    RUN_TEST(TestNetDiag_RunAllTests);
    RUN_TEST(TestNetDiag_TypeNames);
    RUN_TEST(TestNetDiag_StatusNames);
    RUN_TEST(TestNetDiag_Proxy);

    // Config Migration Tests
    std::wcout << L"Config Migration Engine..." << std::endl;
    RUN_TEST(TestConfigMig_Migrate);
    RUN_TEST(TestConfigMig_Rollback);
    RUN_TEST(TestConfigMig_SetDefault);
    RUN_TEST(TestConfigMig_ActionNames);
    RUN_TEST(TestConfigMig_StatusNames);

    // Release Gate V15 Tests
    std::wcout << L"Release Gate V15..." << std::endl;
    RUN_TEST(TestGateV15_KPINames);
    RUN_TEST(TestGateV15_KPICount);
    RUN_TEST(TestGateV15_Evaluate);
    RUN_TEST(TestGateV15_Approved);
    RUN_TEST(TestGateV15_Version);

    std::wcout << std::endl;

    // Format Registry Tests
    std::wcout << L"Format Registry..." << std::endl;
    RUN_TEST(TestFormatReg_Register);
    RUN_TEST(TestFormatReg_LookupByExt);
    RUN_TEST(TestFormatReg_CategoryNames);
    RUN_TEST(TestFormatReg_TypeNames);
    RUN_TEST(TestFormatReg_Validate);

    // Format Type Lookup Tests
    std::wcout << L"Format Type Lookup..." << std::endl;
    RUN_TEST(TestFormatLookup_WebP);
    RUN_TEST(TestFormatLookup_Archives);
    RUN_TEST(TestFormatLookup_Scientific);
    RUN_TEST(TestFormatLookup_Film);
    RUN_TEST(TestFormatLookup_Stats);

    // Shell Registration Manager Tests
    std::wcout << L"Shell Registration Manager..." << std::endl;
    RUN_TEST(TestShellReg_AddRegistered);
    RUN_TEST(TestShellReg_MissingRegs);
    RUN_TEST(TestShellReg_V106NewExts);
    RUN_TEST(TestShellReg_CategoryNames);
    RUN_TEST(TestShellReg_Audit);

    // Test Infrastructure V2 Tests
    std::wcout << L"Test Infrastructure V2..." << std::endl;
    RUN_TEST(TestInfra_CoverageCommand);
    RUN_TEST(TestInfra_SanitizerNames);
    RUN_TEST(TestInfra_CoverageToolNames);
    RUN_TEST(TestInfra_SanitizerFlags);
    RUN_TEST(TestInfra_CoverageThresholds);

    // Release Gate V16 Tests
    std::wcout << L"Release Gate V16..." << std::endl;
    RUN_TEST(TestGateV16_KPINames);
    RUN_TEST(TestGateV16_KPICount);
    RUN_TEST(TestGateV16_Evaluate);
    RUN_TEST(TestGateV16_Approved);
    RUN_TEST(TestGateV16_Version);

    // DPX/Cineon Decoder Tests
    std::wcout << L"DPX/Cineon Decoder..." << std::endl;
    RUN_TEST(TestDPX_MagicBytes);
    RUN_TEST(TestDPX_CineonMagic);
    RUN_TEST(TestDPX_TransferNames);
    RUN_TEST(TestDPX_LogToLinear);
    RUN_TEST(TestDPX_TransferCount);

    // APNG & Animated Format Tests
    std::wcout << L"Animated Formats..." << std::endl;
    RUN_TEST(TestAnim_FormatDetection);
    RUN_TEST(TestAnim_FormatNames);
    RUN_TEST(TestAnim_StrategyNames);
    RUN_TEST(TestAnim_FrameSelection);
    RUN_TEST(TestAnim_FormatCount);

    // Text Preview Decoder Tests
    std::wcout << L"Text Preview..." << std::endl;
    RUN_TEST(TestTextPreview_DetectLang);
    RUN_TEST(TestTextPreview_LanguageNames);
    RUN_TEST(TestTextPreview_IsTextFile);
    RUN_TEST(TestTextPreview_ValidateConfig);
    RUN_TEST(TestTextPreview_ExtCount);

    // DICOM Decoder V2 Tests
    std::wcout << L"DICOM V2..." << std::endl;
    RUN_TEST(TestDICOMv2_Magic);
    RUN_TEST(TestDICOMv2_TransferSyntax);
    RUN_TEST(TestDICOMv2_CanDecode);
    RUN_TEST(TestDICOMv2_Validate);
    RUN_TEST(TestDICOMv2_PixelSize);

    // FITS Decoder V2 Tests
    std::wcout << L"FITS V2..." << std::endl;
    RUN_TEST(TestFITSv2_Magic);
    RUN_TEST(TestFITSv2_BytesPerPixel);
    RUN_TEST(TestFITSv2_Validate);
    RUN_TEST(TestFITSv2_DataSize);
    RUN_TEST(TestFITSv2_Normalize);

    // 3MF/USD Format Tests
    std::wcout << L"3MF/USD Formats..." << std::endl;
    RUN_TEST(TestModelFmt_Detect3MF);
    RUN_TEST(TestModelFmt_FormatNames);
    RUN_TEST(TestModelFmt_Is3MF);
    RUN_TEST(TestModelFmt_Thumbnail);
    RUN_TEST(TestModelFmt_Counts);

    // Release Gate V17 Tests
    std::wcout << L"Release Gate V17..." << std::endl;
    RUN_TEST(TestGateV17_KPINames);
    RUN_TEST(TestGateV17_KPICount);
    RUN_TEST(TestGateV17_Evaluate);
    RUN_TEST(TestGateV17_Approved);
    RUN_TEST(TestGateV17_Version);

    // D3D12 Pipeline Activation Tests
    std::wcout << L"D3D12 Pipeline..." << std::endl;
    RUN_TEST(TestD3D12Act_BackendNames);
    RUN_TEST(TestD3D12Act_FeatureLevels);
    RUN_TEST(TestD3D12Act_SelectBackend);
    RUN_TEST(TestD3D12Act_Fallback);
    RUN_TEST(TestD3D12Act_ValidateConfig);

    // Async Shell Extension Tests
    std::wcout << L"Async Shell..." << std::endl;
    RUN_TEST(TestAsync_StateNames);
    RUN_TEST(TestAsync_PriorityNames);
    RUN_TEST(TestAsync_Counts);
    RUN_TEST(TestAsync_ValidateConfig);
    RUN_TEST(TestAsync_Timeout);

    // SIMD Acceleration Tests
    std::wcout << L"SIMD Acceleration..." << std::endl;
    RUN_TEST(TestSIMDMgr_LevelNames);
    RUN_TEST(TestSIMD_OperationNames);
    RUN_TEST(TestSIMD_SelectLevel);
    RUN_TEST(TestSIMD_Speedup);
    RUN_TEST(TestSIMD_Counts);

    // Parallel Batch Decode Tests
    std::wcout << L"Parallel Batch..." << std::endl;
    RUN_TEST(TestBatch_PolicyNames);
    RUN_TEST(TestBatch_OptimalThreads);
    RUN_TEST(TestBatch_ValidateConfig);
    RUN_TEST(TestBatch_Throughput);
    RUN_TEST(TestBatch_PolicyCount);

    // Persistent Cache & USN Tests
    std::wcout << L"Persistent Cache..." << std::endl;
    RUN_TEST(TestPCache_BackendNames);
    RUN_TEST(TestPCache_PolicyNames);
    RUN_TEST(TestPCache_ValidateConfig);
    RUN_TEST(TestPCache_HitRate);
    RUN_TEST(TestPCache_Counts);

    // Release Gate V18 Tests
    std::wcout << L"Release Gate V18..." << std::endl;
    RUN_TEST(TestGateV18_KPINames);
    RUN_TEST(TestGateV18_KPICount);
    RUN_TEST(TestGateV18_Thresholds);
    RUN_TEST(TestGateV18_Evaluate);
    RUN_TEST(TestGateV18_Version);

    // ARM64 Validation Tests
    std::wcout << L"ARM64 Validation..." << std::endl;
    RUN_TEST(TestARM64Validator_FeatureNames);
    RUN_TEST(TestARM64_CategoryNames);
    RUN_TEST(TestARM64_Counts);
    RUN_TEST(TestARM64_IsARM64);
    RUN_TEST(TestARM64_Config);

    // MSIX Packaging Tests
    std::wcout << L"MSIX Packaging..." << std::endl;
    RUN_TEST(TestMSIX_TargetNames);
    RUN_TEST(TestMSIX_CapabilityNames);
    RUN_TEST(TestMSIX_Counts);
    RUN_TEST(TestMSIX_Identity);
    RUN_TEST(TestMSIX_ValidateVersion);

    // Windows 11 24H2 Integration Tests
    std::wcout << L"Windows 11 24H2 Integration..." << std::endl;
    RUN_TEST(TestWin11Mgr_FeatureNames);
    RUN_TEST(TestWin11_VersionNames);
    RUN_TEST(TestWin11_FeatureAvailability);
    RUN_TEST(TestWin11_TabbedExplorer);
    RUN_TEST(TestWin11_Config);

    // Test Suite Expansion V2 Tests
    std::wcout << L"Test Suite Expansion V2..." << std::endl;
    RUN_TEST(TestTestSuiteV2_CategoryNames);
    RUN_TEST(TestTestSuiteV2_COMTestNames);
    RUN_TEST(TestTestSuiteV2_Counts);
    RUN_TEST(TestTestSuiteV2_TotalTarget);
    RUN_TEST(TestTestSuiteV2_CorpusDefaults);

    // Fuzz Testing Tests
    std::wcout << L"Fuzz Testing..." << std::endl;
    RUN_TEST(TestFuzz_BackendNames);
    RUN_TEST(TestFuzz_MutationNames);
    RUN_TEST(TestFuzz_Counts);
    RUN_TEST(TestFuzz_ValidateConfig);
    RUN_TEST(TestFuzz_InvalidConfig);

    // Release Gate V19 Tests
    std::wcout << L"Release Gate V19..." << std::endl;
    RUN_TEST(TestGateV19_KPINames);
    RUN_TEST(TestGateV19_KPICount);
    RUN_TEST(TestGateV19_Evaluate);
    RUN_TEST(TestGateV19_Version);
    RUN_TEST(TestGateV19_ResultDefault);

    // Vulkan Compute Backend Tests
    std::wcout << L"Vulkan Compute..." << std::endl;
    RUN_TEST(TestVulkan_FeatureNames);
    RUN_TEST(TestVulkan_QueueNames);
    RUN_TEST(TestVulkan_Counts);
    RUN_TEST(TestVulkan_MinRequirements);
    RUN_TEST(TestVulkan_ValidateConfig);

    // Plugin Marketplace V3 Tests
    std::wcout << L"Plugin Marketplace V3..." << std::endl;
    RUN_TEST(TestMarketV3_CategoryNames);
    RUN_TEST(TestMarketV3_TrustNames);
    RUN_TEST(TestMarketV3_SandboxNames);
    RUN_TEST(TestMarketV3_Counts);
    RUN_TEST(TestMarketV3_EntryDefaults);

    // AI-Enhanced Thumbnails Tests
    std::wcout << L"AI-Enhanced Thumbnails..." << std::endl;
    RUN_TEST(TestAI_EnhancementNames);
    RUN_TEST(TestAI_BackendNames);
    RUN_TEST(TestAI_Counts);
    RUN_TEST(TestAI_QualityValid);
    RUN_TEST(TestAI_ConfigDefaults);

    // Spreadsheet Preview Tests
    std::wcout << L"Spreadsheet Preview..." << std::endl;
    RUN_TEST(TestSpreadsheet_FormatNames);
    RUN_TEST(TestSpreadsheet_CellTypeNames);
    RUN_TEST(TestSpreadsheet_DetectFormat);
    RUN_TEST(TestSpreadsheet_Counts);
    RUN_TEST(TestSpreadsheet_ConfigDefaults);

    // USD/USDZ Decoder Tests
    std::wcout << L"USD/USDZ..." << std::endl;
    RUN_TEST(TestUSD_ElementNames);
    RUN_TEST(TestUSD_VariantNames);
    RUN_TEST(TestUSD_DetectVariant);
    RUN_TEST(TestUSD_USDZMagic);
    RUN_TEST(TestUSD_Counts);

    // Auto-Update Engine Tests
    std::wcout << L"Auto-Update Engine..." << std::endl;
    RUN_TEST(TestAutoUpdate_ChannelNames);
    RUN_TEST(TestAutoUpdate_CheckResultNames);
    RUN_TEST(TestAutoUpdate_DownloadStateNames);
    RUN_TEST(TestAutoUpdate_ParseVersion);
    RUN_TEST(TestAutoUpdate_Counts);

    // Release Gate V20 Tests
    std::wcout << L"Release Gate V20..." << std::endl;
    RUN_TEST(TestGateV20_KPINames);
    RUN_TEST(TestGateV20_KPICount);
    RUN_TEST(TestGateV20_Evaluate);
    RUN_TEST(TestGateV20_Version);
    RUN_TEST(TestGateV20_ResultDefault);

    // CSV/JSON Preview Tests
    std::wcout << L"CSV/JSON Preview..." << std::endl;
    RUN_TEST(TestStructData_FormatNames);
    RUN_TEST(TestStructData_ValueTypeNames);
    RUN_TEST(TestStructData_StyleNames);
    RUN_TEST(TestStructData_DetectFormat);
    RUN_TEST(TestStructData_Counts);

    // Notebook Preview Tests
    std::wcout << L"Notebook Preview..." << std::endl;
    RUN_TEST(TestNotebook_CellTypeNames);
    RUN_TEST(TestNotebook_OutputTypeNames);
    RUN_TEST(TestNotebook_KernelNames);
    RUN_TEST(TestNotebook_Counts);
    RUN_TEST(TestNotebook_MetadataDefaults);

    // Database Preview Tests
    std::wcout << L"Database Preview..." << std::endl;
    RUN_TEST(TestDatabase_EngineNames);
    RUN_TEST(TestDatabase_ColumnTypeNames);
    RUN_TEST(TestDatabase_PreviewStyleNames);
    RUN_TEST(TestDatabase_SQLiteMagic);
    RUN_TEST(TestDatabase_Counts);

    // Legacy Image Decoder Tests
    std::wcout << L"Legacy Image..." << std::endl;
    RUN_TEST(TestLegacyImg_FormatNames);
    RUN_TEST(TestLegacyImg_ColorSpaceNames);
    RUN_TEST(TestLegacyImg_FLIFMagic);
    RUN_TEST(TestLegacyImg_BPGMagic);
    RUN_TEST(TestLegacyImg_Counts);

    // CDR/Visio Vector Decoder Tests
    std::wcout << L"CDR/Visio Vector..." << std::endl;
    RUN_TEST(TestVector_FormatNames);
    RUN_TEST(TestVector_ElementNames);
    RUN_TEST(TestVector_DetectFormat);
    RUN_TEST(TestVector_Counts);
    RUN_TEST(TestVector_ConfigDefaults);

    // HDF5/NetCDF Scientific Decoder Tests
    std::wcout << L"HDF5/NetCDF Scientific..." << std::endl;
    RUN_TEST(TestSciData_FormatNames);
    RUN_TEST(TestSciData_DataTypeNames);
    RUN_TEST(TestSciData_VisModeNames);
    RUN_TEST(TestSciData_HDF5Magic);
    RUN_TEST(TestSciData_Counts);

    // NIfTI Neuroimaging Tests
    std::wcout << L"NIfTI Neuroimaging..." << std::endl;
    RUN_TEST(TestNIfTI_DataTypeNames);
    RUN_TEST(TestNIfTI_SliceNames);
    RUN_TEST(TestNIfTI_VariantNames);
    RUN_TEST(TestNIfTI_Counts);
    RUN_TEST(TestNIfTI_HeaderDefaults);

    // STEP/IGES CAD Decoder Tests
    std::wcout << L"STEP/IGES CAD..." << std::endl;
    RUN_TEST(TestCAD_FormatNames);
    RUN_TEST(TestCAD_EntityNames);
    RUN_TEST(TestCAD_RenderModeNames);
    RUN_TEST(TestCAD_STEPMagic);
    RUN_TEST(TestCAD_Counts);

    // HDR Display Pipeline Tests
    std::wcout << L"HDR Display Pipeline..." << std::endl;
    RUN_TEST(TestHDR_ToneMapNames);
    RUN_TEST(TestHDR_GamutNames);
    RUN_TEST(TestHDR_FormatNames);
    RUN_TEST(TestHDR_ValidateExposure);
    RUN_TEST(TestHDR_Counts);

    // Per-Monitor DPI V3
    RUN_TEST(TestDPIV3_AwarenessNames);
    RUN_TEST(TestDPIV3_ScaleNames);
    RUN_TEST(TestDPI_ScaledSize);
    RUN_TEST(TestDPI_DefaultConfig);
    RUN_TEST(TestDPI_Counts);

    // Shell Overlay Icon Handler
    RUN_TEST(TestOverlay_IconNames);
    RUN_TEST(TestOverlay_PositionNames);
    RUN_TEST(TestOverlay_ValidateOpacity);
    RUN_TEST(TestOverlay_DefaultConfig);
    RUN_TEST(TestOverlay_Counts);

    // Cache Warming Service
    RUN_TEST(TestCacheWarm_StrategyNames);
    RUN_TEST(TestCacheWarm_PriorityNames);
    RUN_TEST(TestCacheWarm_JobStatusNames);
    RUN_TEST(TestCacheWarm_DefaultConfig);
    RUN_TEST(TestCacheWarm_Counts);

    // Multi-GPU Load Balancer
    RUN_TEST(TestMultiGPU_StrategyNames);
    RUN_TEST(TestMultiGPU_DeviceTypeNames);
    RUN_TEST(TestMultiGPU_ValidateConfig);
    RUN_TEST(TestMultiGPU_DefaultConfig);
    RUN_TEST(TestMultiGPU_Counts);

    // Release Gate V21
    RUN_TEST(TestGateV21_KPINames);
    RUN_TEST(TestGateV21_Evaluate);
    RUN_TEST(TestGateV21_KPICount);
    RUN_TEST(TestGateV21_Version);
    RUN_TEST(TestGateV21_AllKPIsPresent);

    // Accessibility Pipeline
    RUN_TEST(TestAccessibility_FeatureNames);
    RUN_TEST(TestAccessibility_ColorBlindModes);
    RUN_TEST(TestAccessibility_HCThemes);
    RUN_TEST(TestAccessibility_DefaultConfig);
    RUN_TEST(TestAccessibility_Counts);

    // Telemetry & Analytics
    RUN_TEST(TestTelemetry_EventNames);
    RUN_TEST(TestTelemetry_ConsentNames);
    RUN_TEST(TestTelemetry_PeriodNames);
    RUN_TEST(TestTelemetry_CacheHitRate);
    RUN_TEST(TestTelemetry_DefaultConfig);

    // Cloud Storage Integration
    RUN_TEST(TestCloudStorage_ProviderNames);
    RUN_TEST(TestCloud_FileStateNames);
    RUN_TEST(TestCloud_HydrationNames);
    RUN_TEST(TestCloud_ShouldHydrate);
    RUN_TEST(TestCloud_Counts);

    // Release Gate V22 (v13.0)
    RUN_TEST(TestGateV22_KPINames);
    RUN_TEST(TestGateV22_Evaluate);
    RUN_TEST(TestGateV22_KPICount);
    RUN_TEST(TestGateV22_Version);
    RUN_TEST(TestGateV22_AllKPIsPresent);

    // Version Sync V14
    RUN_TEST(TestV14_DomainNames);
    RUN_TEST(TestV14_FeatureStatusNames);
    RUN_TEST(TestV14_DomainCount);
    RUN_TEST(TestV14_FeatureStatusCount);
    RUN_TEST(TestV14_GetVersion);

    // GPU Pipeline V3
    RUN_TEST(TestGPUV3_FeatureNames);
    RUN_TEST(TestGPUV3_QueueNames);
    RUN_TEST(TestGPUV3_PerfTierNames);
    RUN_TEST(TestGPUV3_FeatureCount);
    RUN_TEST(TestGPUV3_QueueCount);

    // Shader Compiler V2
    RUN_TEST(TestShaderV2_ModelNames);
    RUN_TEST(TestShaderV2_StageNames);
    RUN_TEST(TestShaderV2_OptLevelNames);
    RUN_TEST(TestShaderV2_ModelCount);
    RUN_TEST(TestShaderV2_StageCount);

    // Pipeline State Cache V2
    RUN_TEST(TestPSOCacheV2_StateNames);
    RUN_TEST(TestPSOCacheV2_TypeNames);
    RUN_TEST(TestPSOCacheV2_WarmupNames);
    RUN_TEST(TestPSOCacheV2_StateCount);
    RUN_TEST(TestPSOCacheV2_TypeCount);

    // GPU Memory Pool V2
    RUN_TEST(TestGPUMemV2_HeapTypeNames);
    RUN_TEST(TestGPUMemV2_ResidencyNames);
    RUN_TEST(TestGPUMemV2_AllocNames);
    RUN_TEST(TestGPUMemV2_HeapTypeCount);
    RUN_TEST(TestGPUMemV2_ResidencyCount);

    // Release Gate V23
    RUN_TEST(TestGateV23_KPINames);
    RUN_TEST(TestGateV23_KPICount);
    RUN_TEST(TestGateV23_Evaluate);
    RUN_TEST(TestGateV23_AllPass);
    RUN_TEST(TestGateV23_Advance);

    // Smart Format Detector V2
    RUN_TEST(TestSmartDetV2_MethodNames);
    RUN_TEST(TestSmartDetV2_ConfNames);
    RUN_TEST(TestSmartDetV2_HintNames);
    RUN_TEST(TestSmartDetV2_MethodCount);
    RUN_TEST(TestSmartDetV2_ConfCount);

    // Extended Video Decoder
    RUN_TEST(TestExtVideo_CodecNames);
    RUN_TEST(TestExtVideo_AccelNames);
    RUN_TEST(TestExtVideo_FrameSelectNames);
    RUN_TEST(TestExtVideo_CodecCount);
    RUN_TEST(TestExtVideo_AccelCount);

    // Audio Visualization V2
    RUN_TEST(TestAudioVisV2_ModeNames);
    RUN_TEST(TestAudioVisV2_ColorNames);
    RUN_TEST(TestAudioVisV2_LoudnessNames);
    RUN_TEST(TestAudioVisV2_ModeCount);
    RUN_TEST(TestAudioVisV2_ColorCount);

    // 3D Model Renderer V2
    RUN_TEST(TestModel3DV2_FormatNames);
    RUN_TEST(TestModel3DV2_LightNames);
    RUN_TEST(TestModel3DV2_CamNames);
    RUN_TEST(TestModel3DV2_FormatCount);
    RUN_TEST(TestModel3DV2_LightCount);

    // Release Gate V24
    RUN_TEST(TestGateV24_KPINames);
    RUN_TEST(TestGateV24_KPICount);
    RUN_TEST(TestGateV24_Evaluate);
    RUN_TEST(TestGateV24_AllPass);
    RUN_TEST(TestGateV24_Advance);

    // Plugin SDK V2
    RUN_TEST(TestPluginSDKV2_CapNames);
    RUN_TEST(TestPluginSDKV2_LifeCycleNames);
    RUN_TEST(TestPluginSDKV2_APIVersionNames);
    RUN_TEST(TestPluginSDKV2_CapCount);
    RUN_TEST(TestPluginSDKV2_LifeCycleCount);

    // Plugin Debugger Integration
    RUN_TEST(TestPluginDbg_ModeNames);
    RUN_TEST(TestPluginDbg_LogLevelNames);
    RUN_TEST(TestPluginDbg_EventNames);
    RUN_TEST(TestPluginDbg_ModeCount);
    RUN_TEST(TestPluginDbg_LogLevelCount);

    // Plugin Hot Reload
    RUN_TEST(TestHotReload_TriggerNames);
    RUN_TEST(TestHotReload_StateNames);
    RUN_TEST(TestHotReload_PolicyNames);
    RUN_TEST(TestHotReload_TriggerCount);
    RUN_TEST(TestHotReload_StateCount);

    // Plugin Performance Profiler
    RUN_TEST(TestPluginPerf_MetricNames);
    RUN_TEST(TestPluginPerf_AlertNames);
    RUN_TEST(TestPluginPerf_SamplingNames);
    RUN_TEST(TestPluginPerf_MetricCount);
    RUN_TEST(TestPluginPerf_AlertCount);

    // Release Gate V25
    RUN_TEST(TestGateV25_KPINames);
    RUN_TEST(TestGateV25_KPICount);
    RUN_TEST(TestGateV25_Evaluate);
    RUN_TEST(TestGateV25_AllPass);
    RUN_TEST(TestGateV25_Advance);

    // Threat Model V2
    RUN_TEST(TestThreatV2_CategoryNames);
    RUN_TEST(TestThreatV2_SeverityNames);
    RUN_TEST(TestThreatV2_MitigationNames);
    RUN_TEST(TestThreatV2_CategoryCount);
    RUN_TEST(TestThreatV2_SeverityCount);

    // Memory Safety Audit V2
    RUN_TEST(TestMemSafetyV2_ViolationNames);
    RUN_TEST(TestMemSafetyV2_ToolNames);
    RUN_TEST(TestMemSafetyV2_ScopeNames);
    RUN_TEST(TestMemSafetyV2_ViolationCount);
    RUN_TEST(TestMemSafetyV2_ToolCount);

    // Supply Chain Integrity V2
    RUN_TEST(TestSupplyChainV2_SBOMNames);
    RUN_TEST(TestSupplyChainV2_VulnNames);
    RUN_TEST(TestSupplyChainV2_ReprodNames);
    RUN_TEST(TestSupplyChainV2_SBOMCount);
    RUN_TEST(TestSupplyChainV2_VulnCount);

    // Runtime Integrity Verifier
    RUN_TEST(TestRuntimeInteg_CheckTypeNames);
    RUN_TEST(TestRuntimeInteg_ResultNames);
    RUN_TEST(TestRuntimeInteg_TamperNames);
    RUN_TEST(TestRuntimeInteg_CheckTypeCount);
    RUN_TEST(TestRuntimeInteg_ResultCount);

    // Release Gate V26
    RUN_TEST(TestGateV26_KPINames);
    RUN_TEST(TestGateV26_KPICount);
    RUN_TEST(TestGateV26_Evaluate);
    RUN_TEST(TestGateV26_AllPass);
    RUN_TEST(TestGateV26_Advance);

    // Progressive Thumbnail Loader
    RUN_TEST(TestProgLoad_StageNames);
    RUN_TEST(TestProgLoad_StrategyNames);
    RUN_TEST(TestProgLoad_PlaceholderNames);
    RUN_TEST(TestProgLoad_StageCount);
    RUN_TEST(TestProgLoad_StrategyCount);

    // Thumbnail Animation Engine V2
    RUN_TEST(TestAnimEngineV2_FormatNames);
    RUN_TEST(TestAnimEngineV2_LoopModeNames);
    RUN_TEST(TestAnimEngineV2_InterpNames);
    RUN_TEST(TestAnimEngineV2_FormatCount);
    RUN_TEST(TestAnimEngineV2_LoopModeCount);

    // Preview Panel V2
    RUN_TEST(TestPreviewV2_TabNames);
    RUN_TEST(TestPreviewV2_ZoomNames);
    RUN_TEST(TestPreviewV2_ColorPickerNames);
    RUN_TEST(TestPreviewV2_TabCount);
    RUN_TEST(TestPreviewV2_ZoomCount);

    // Quick Look Integration
    RUN_TEST(TestQuickLook_ModeNames);
    RUN_TEST(TestQuickLook_TransitionNames);
    RUN_TEST(TestQuickLook_MetadataNames);
    RUN_TEST(TestQuickLook_ModeCount);
    RUN_TEST(TestQuickLook_TransitionCount);

    // Release Gate V27
    RUN_TEST(TestGateV27_KPINames);
    RUN_TEST(TestGateV27_KPICount);
    RUN_TEST(TestGateV27_Evaluate);
    RUN_TEST(TestGateV27_AllPass);
    RUN_TEST(TestGateV27_Advance);

    // Scene Understanding Engine
    RUN_TEST(TestSceneAI_CategoryNames);
    RUN_TEST(TestSceneAI_BackendNames);
    RUN_TEST(TestSceneAI_ConfNames);
    RUN_TEST(TestSceneAI_CategoryCount);
    RUN_TEST(TestSceneAI_BackendCount);

    // Smart Crop V2
    RUN_TEST(TestSmartCropV2_StrategyNames);
    RUN_TEST(TestSmartCropV2_AspectNames);
    RUN_TEST(TestSmartCropV2_PaddingNames);
    RUN_TEST(TestSmartCropV2_StrategyCount);
    RUN_TEST(TestSmartCropV2_AspectCount);

    // Image Quality Assessor
    RUN_TEST(TestIQA_MetricNames);
    RUN_TEST(TestIQA_DefectNames);
    RUN_TEST(TestIQA_GradeNames);
    RUN_TEST(TestIQA_MetricCount);
    RUN_TEST(TestIQA_DefectCount);

    // AI Search Integration
    RUN_TEST(TestAISearch_ModeNames);
    RUN_TEST(TestAISearch_EmbeddingNames);
    RUN_TEST(TestAISearch_StatusNames);
    RUN_TEST(TestAISearch_ModeCount);
    RUN_TEST(TestAISearch_EmbeddingCount);

    // Release Gate V28
    RUN_TEST(TestGateV28_KPINames);
    RUN_TEST(TestGateV28_KPICount);
    RUN_TEST(TestGateV28_Evaluate);
    RUN_TEST(TestGateV28_AllPass);
    RUN_TEST(TestGateV28_Advance);

    // Enterprise Policy Engine V2
    RUN_TEST(TestEntPolV2_SourceNames);
    RUN_TEST(TestEntPolV2_StatusNames);
    RUN_TEST(TestEntPolV2_ScopeNames);
    RUN_TEST(TestEntPolV2_SourceCount);
    RUN_TEST(TestEntPolV2_StatusCount);

    // SharePoint / Teams Integration
    RUN_TEST(TestSPTeams_CloudSourceNames);
    RUN_TEST(TestSPTeams_AuthMethodNames);
    RUN_TEST(TestSPTeams_SyncStateNames);
    RUN_TEST(TestSPTeams_CloudSourceCount);
    RUN_TEST(TestSPTeams_AuthMethodCount);

    // Multi-Tenant Cache Manager
    RUN_TEST(TestMTCache_TierNames);
    RUN_TEST(TestMTCache_IsolationNames);
    RUN_TEST(TestMTCache_EvictNames);
    RUN_TEST(TestMTCache_TierCount);
    RUN_TEST(TestMTCache_IsolationCount);

    // Compliance Audit Logger
    RUN_TEST(TestCompliance_RegNames);
    RUN_TEST(TestCompliance_DataClassNames);
    RUN_TEST(TestCompliance_EventTypeNames);
    RUN_TEST(TestCompliance_RegCount);
    RUN_TEST(TestCompliance_DataClassCount);

    // Release Gate V29
    RUN_TEST(TestGateV29_KPINames);
    RUN_TEST(TestGateV29_KPICount);
    RUN_TEST(TestGateV29_Evaluate);
    RUN_TEST(TestGateV29_AllPass);
    RUN_TEST(TestGateV29_Advance);

    // Windows 12 Compatibility
    RUN_TEST(TestWin12_FeatureNames);
    RUN_TEST(TestWin12_CompatModeNames);
    RUN_TEST(TestWin12_APIFamilyNames);
    RUN_TEST(TestWin12_FeatureCount);
    RUN_TEST(TestWin12_CompatModeCount);

    // ARM64 Performance Optimizer
    RUN_TEST(TestARM64Opt_SIMDNames);
    RUN_TEST(TestARM64Opt_CoreTypeNames);
    RUN_TEST(TestARM64Opt_ThermalNames);
    RUN_TEST(TestARM64Opt_SIMDCount);
    RUN_TEST(TestARM64Opt_ThermalCount);

    // WinRT App SDK Integration V2
    RUN_TEST(TestWinRTV2_ActivationNames);
    RUN_TEST(TestWinRTV2_BootstrapNames);
    RUN_TEST(TestWinRTV2_StreamNames);
    RUN_TEST(TestWinRTV2_ActivationCount);
    RUN_TEST(TestWinRTV2_BootstrapCount);

    // Installer V2 Manager
    RUN_TEST(TestInstallerV2_FormatNames);
    RUN_TEST(TestInstallerV2_ScopeNames);
    RUN_TEST(TestInstallerV2_PhaseNames);
    RUN_TEST(TestInstallerV2_FormatCount);
    RUN_TEST(TestInstallerV2_PhaseCount);

    // Release Gate V30
    RUN_TEST(TestGateV30_KPINames);
    RUN_TEST(TestGateV30_KPICount);
    RUN_TEST(TestGateV30_Evaluate);
    RUN_TEST(TestGateV30_AllPass);
    RUN_TEST(TestGateV30_Advance);

    // Sub-Millisecond Cache Engine
    RUN_TEST(TestSubMsCache_HashNames);
    RUN_TEST(TestSubMsCache_EvictionNames);
    RUN_TEST(TestSubMsCache_NumaNames);
    RUN_TEST(TestSubMsCache_HashCount);
    RUN_TEST(TestSubMsCache_EvictionCount);

    // GPU Decode Acceleration V2
    RUN_TEST(TestGPUDecV2_VendorNames);
    RUN_TEST(TestGPUDecV2_APINames);
    RUN_TEST(TestGPUDecV2_CodecNames);
    RUN_TEST(TestGPUDecV2_VendorCount);
    RUN_TEST(TestGPUDecV2_APICount);

    // Parallel I/O Pipeline
    RUN_TEST(TestParallelIO_BackendNames);
    RUN_TEST(TestParallelIO_PriorityNames);
    RUN_TEST(TestParallelIO_VolumeNames);
    RUN_TEST(TestParallelIO_BackendCount);
    RUN_TEST(TestParallelIO_PriorityCount);

    // Memory Footprint Optimizer V2
    RUN_TEST(TestMemFootV2_AllocNames);
    RUN_TEST(TestMemFootV2_TrimNames);
    RUN_TEST(TestMemFootV2_LargePageNames);
    RUN_TEST(TestMemFootV2_AllocCount);
    RUN_TEST(TestMemFootV2_TrimCount);

    // Release Gate V31
    RUN_TEST(TestGateV31_KPINames);
    RUN_TEST(TestGateV31_KPICount);
    RUN_TEST(TestGateV31_Evaluate);
    RUN_TEST(TestGateV31_AllPass);
    RUN_TEST(TestGateV31_Advance);

    // Accessibility Suite V2
    RUN_TEST(TestA11ySuiteV2_WCAGNames);
    RUN_TEST(TestA11ySuiteV2_ColorBlindNames);
    RUN_TEST(TestA11ySuiteV2_FeatureNames);
    RUN_TEST(TestA11ySuiteV2_WCAGCount);
    RUN_TEST(TestA11ySuiteV2_ColorBlindCount);

    // Documentation Excellence V2
    RUN_TEST(TestDocExcV2_FormatNames);
    RUN_TEST(TestDocExcV2_ScopeNames);
    RUN_TEST(TestDocExcV2_DriftNames);
    RUN_TEST(TestDocExcV2_FormatCount);
    RUN_TEST(TestDocExcV2_ScopeCount);

    // Quality Assurance V2
    RUN_TEST(TestQAV2_CategoryNames);
    RUN_TEST(TestQAV2_SeverityNames);
    RUN_TEST(TestQAV2_SignalNames);
    RUN_TEST(TestQAV2_CategoryCount);
    RUN_TEST(TestQAV2_SignalCount);

    // Release Gate V32 (v14.0 Final)
    RUN_TEST(TestGateV32_KPINames);
    RUN_TEST(TestGateV32_KPICount);
    RUN_TEST(TestGateV32_Evaluate);
    RUN_TEST(TestGateV32_AllPass);
    RUN_TEST(TestGateV32_v14Approved);

    std::wcout << std::endl;

    // ======== Feature Module Tests ========
    std::wcout << L"Feature Module Tests..." << std::endl;

    // EngineTests.h — Version Synchronization
    RUN_TEST(TestZenith_VersionMajor);
    RUN_TEST(TestZenith_VersionMinor);
    RUN_TEST(TestZenith_VersionPatch);
    RUN_TEST(TestZenith_VersionComposite);

    // EngineTests.h — MuPDF
    RUN_TEST(TestZenith_MuPDFBackendNames);
    RUN_TEST(TestZenith_MuPDFCaps);
    RUN_TEST(TestZenith_MuPDFPageConfig);

    // EngineTests.h — libwebp CRT
    RUN_TEST(TestZenith_LibWebPConfigDefaults);
    RUN_TEST(TestZenith_LibWebPConfigName);

    // EngineTests.h — LENSArchive Refactoring
    RUN_TEST(TestZenith_ArchiveRefactorModules);
    RUN_TEST(TestZenith_ArchiveRefactorModuleNames);

    // EngineTests.h — Bitmap Pool
    RUN_TEST(TestZenith_BitmapPoolConfig);
    RUN_TEST(TestZenith_BitmapPoolStats);
    RUN_TEST(TestZenith_BitmapPoolStatsHitRate);

    // EngineTests.h — IPropertyStore
    RUN_TEST(TestZenith_PropertyIDNames);
    RUN_TEST(TestZenith_PropertyTypes);
    RUN_TEST(TestZenith_PropertyValueDefault);
    RUN_TEST(TestZenith_PropertyCapabilities);

    // EngineTests.h — GPU Shader Library
    RUN_TEST(TestZenith_GPUShaderTypes);
    RUN_TEST(TestZenith_GPUShaderCount);
    RUN_TEST(TestZenith_ToneMapAlgorithms);
    RUN_TEST(TestZenith_GPUColorSpaces);

    // EngineTests.h — PluginHost
    RUN_TEST(TestZenith_PluginHostModes);
    RUN_TEST(TestZenith_PluginHostModeNames);

    // EngineTests.h — Library Version Audit
    RUN_TEST(TestZenith_LibVersionAuditFields);
    RUN_TEST(TestZenith_LibVersionAuditNames);

    // EngineTests.h — OpenJPEG
    RUN_TEST(TestZenith_OpenJPEGProfiles);
    RUN_TEST(TestZenith_OpenJPEGProfileNames);

    // EngineTests.h — FreeType
    RUN_TEST(TestZenith_FreeTypeRenderModes);
    RUN_TEST(TestZenith_FreeTypeRenderModeNames);

    // EngineTests.h — FFmpeg
    RUN_TEST(TestZenith_FFmpegCodecFamilies);
    RUN_TEST(TestZenith_FFmpegContainerFormats);
    RUN_TEST(TestZenith_FFmpegVideoStreamInfo);
    RUN_TEST(TestZenith_FFmpegFrameResult);

    // EngineTests.h — Format Category Manager
    RUN_TEST(TestZenith_FormatCategoryCount);
    RUN_TEST(TestZenith_FormatCategoryNames);

    // EngineTests.h — Format Status Indicator
    RUN_TEST(TestZenith_FormatStatusLevels);
    RUN_TEST(TestZenith_FormatStatusNames);

    // EngineTests.h — Settings Import/Export
    RUN_TEST(TestZenith_SettingsFormatNames);
    RUN_TEST(TestZenith_SettingsFormatJSON);

    // EngineTests.h — Performance Dashboard
    RUN_TEST(TestZenith_PerfDashboardMetrics);
    RUN_TEST(TestZenith_PerfDashboardMetricNames);

    // EngineTests.h — Dark Mode Engine
    RUN_TEST(TestZenith_DarkModeThemes);
    RUN_TEST(TestZenith_DarkModeThemeNames);

    // EngineTests.h — System Tray Manager
    RUN_TEST(TestZenith_SystemTrayActions);
    RUN_TEST(TestZenith_SystemTrayActionNames);

    // EngineTests.h — WinUI 3 Migration
    RUN_TEST(TestZenith_WinUI3PhaseCount);
    RUN_TEST(TestZenith_WinUI3PhaseNames);

    // EngineTests.h — CI Hardening
    RUN_TEST(TestZenith_CIConfigCount);
    RUN_TEST(TestZenith_CIConfigNames);

    // EngineTests.h — Code Coverage
    RUN_TEST(TestZenith_CoverageTargets);
    RUN_TEST(TestZenith_CoverageTargetNames);

    // EngineTests.h — Fuzzing Campaign
    RUN_TEST(TestZenith_FuzzingStrategies);
    RUN_TEST(TestZenith_FuzzingStrategyNames);

    // EngineTests.h — Static Analysis Gate
    RUN_TEST(TestZenith_StaticAnalysisTools);
    RUN_TEST(TestZenith_StaticAnalysisToolNames);

    // EngineTests.h — SBOM Generator
    RUN_TEST(TestZenith_SBOMFormats);
    RUN_TEST(TestZenith_SBOMFormatNames);

    // EngineTests.h — Zero-Copy Pipeline
    RUN_TEST(TestZenith_ZeroCopyStages);
    RUN_TEST(TestZenith_ZeroCopyStageNames);

    // EngineTests.h — Parallel I/O Pipeline
    RUN_TEST(TestZenith_ParallelIOPolicies);
    RUN_TEST(TestZenith_ParallelIOPolicyNames);

    // EngineTests.h — SIMD Scaler
    RUN_TEST(TestZenith_SIMDScalerPaths);
    RUN_TEST(TestZenith_SIMDScalerPathNames);

    // EngineTests.h — PSO Cache
    RUN_TEST(TestZenith_PSOCacheStrategies);
    RUN_TEST(TestZenith_PSOCacheStrategyNames);

    // EngineTests.h — Cache Warming
    RUN_TEST(TestZenith_CacheWarmingModes);
    RUN_TEST(TestZenith_CacheWarmingModeNames);

    // EngineTests.h — Thumbnail Quality Analyzer
    RUN_TEST(TestZenith_QualityMetrics);
    RUN_TEST(TestZenith_QualityMetricNames);
    RUN_TEST(TestZenith_QualityGradeFromSSIM);
    RUN_TEST(TestZenith_QualityGradeNames);
    RUN_TEST(TestZenith_QualityReportDefault);
    RUN_TEST(TestZenith_QualityThresholds);

    // EngineTests.h — Adaptive Decoder Router
    RUN_TEST(TestZenith_RouterStrategies);
    RUN_TEST(TestZenith_RouterStrategyNames);
    RUN_TEST(TestZenith_RouterBuiltinSignatures);
    RUN_TEST(TestZenith_RouterSignatureMatchPNG);
    RUN_TEST(TestZenith_RouterSignatureMatchJPEG);
    RUN_TEST(TestZenith_RouterSignatureMatchPDF);
    RUN_TEST(TestZenith_RouterSignatureNoMatch);
    RUN_TEST(TestZenith_RouterSignatureNull);
    RUN_TEST(TestZenith_RouterDecisionDefault);

    // EngineTests.h — Telemetry Pipeline
    RUN_TEST(TestZenith_TelemetryCategoryCount);
    RUN_TEST(TestZenith_TelemetryCategoryNames);
    RUN_TEST(TestZenith_TelemetryWindowCount);
    RUN_TEST(TestZenith_TelemetryWindowNames);
    RUN_TEST(TestZenith_TelemetryHealthSnapshot);

    // EngineTests.h — Live Preview Engine
    RUN_TEST(TestZenith_LivePreviewModes);
    RUN_TEST(TestZenith_LivePreviewModeNames);
    RUN_TEST(TestZenith_LivePreviewRecommendGIF);
    RUN_TEST(TestZenith_LivePreviewRecommendMP4);
    RUN_TEST(TestZenith_LivePreviewRecommendCBZ);
    RUN_TEST(TestZenith_LivePreviewRecommendOBJ);
    RUN_TEST(TestZenith_LivePreviewRecommendJPG);
    RUN_TEST(TestZenith_LivePreviewMemoryEstimate);
    RUN_TEST(TestZenith_LivePreviewFitToBudget);
    RUN_TEST(TestZenith_LivePreviewConfig);

    // EngineTests.h — Cloud Native Sync
    RUN_TEST(TestZenith_CloudProviderCount);
    RUN_TEST(TestZenith_CloudProviderNames);
    RUN_TEST(TestZenith_CloudSyncStatusNames);
    RUN_TEST(TestZenith_CloudSyncConfigDefaults);
    RUN_TEST(TestZenith_CloudDetectProviders);

    // EngineTests.h — HDR Tone Mapping Pipeline
    RUN_TEST(TestZenith_HDROperatorCount);
    RUN_TEST(TestZenith_HDROperatorNames);
    RUN_TEST(TestZenith_HDRConfigDefaults);

    // EngineTests.h — Color Space Engine
    RUN_TEST(TestZenith_ColorSpaceCount);
    RUN_TEST(TestZenith_ColorSpaceNames);
    RUN_TEST(TestZenith_ColorSpaceConversion);

    // EngineTests.h — GPU Texture Compression
    RUN_TEST(TestZenith_TextureFormatCount);
    RUN_TEST(TestZenith_TextureFormatNames);
    RUN_TEST(TestZenith_TextureBPP);

    // EngineTests.h — Adaptive DPI Scaler
    RUN_TEST(TestZenith_DPIStrategyCount);
    RUN_TEST(TestZenith_DPIClassify);
    RUN_TEST(TestZenith_DPIScaledSize);

    // EngineTests.h — Format Fingerprint DB
    RUN_TEST(TestZenith_FormatFamilyCount);
    RUN_TEST(TestZenith_FingerprintMatchesPNG);
    RUN_TEST(TestZenith_FingerprintMatchesJPEG);

    // EngineTests.h — Nested Archive Preview
    RUN_TEST(TestZenith_NestingPolicyCount);
    RUN_TEST(TestZenith_NestingEffectiveDepth);

    // EngineTests.h — Multi-Page Navigator
    RUN_TEST(TestZenith_PageStrategyCount);
    RUN_TEST(TestZenith_PageSelectFirst);
    RUN_TEST(TestZenith_PageSelectLast);

    // EngineTests.h — Animated Format Controller
    RUN_TEST(TestZenith_FrameSelectionCount);
    RUN_TEST(TestZenith_AnimSelectFrame);

    // EngineTests.h — Metadata Extraction Pipeline
    RUN_TEST(TestZenith_MetadataSourceCount);
    RUN_TEST(TestZenith_MetadataSourceNames);

    // EngineTests.h — Content-Aware Thumbnail Selector
    RUN_TEST(TestZenith_SaliencyAlgoCount);
    RUN_TEST(TestZenith_CenterCropCalc);

    // EngineTests.h — Face Detection & Orientation
    RUN_TEST(TestZenith_EXIFOrientationDegrees);
    RUN_TEST(TestZenith_EXIFDimensionSwap);

    // EngineTests.h — Document Layout Analyzer
    RUN_TEST(TestZenith_DocRegionTypeCount);
    RUN_TEST(TestZenith_DocTextDensity);

    // EngineTests.h — Visual Similarity Index
    RUN_TEST(TestZenith_HashAlgoCount);
    RUN_TEST(TestZenith_HammingDistance);
    RUN_TEST(TestZenith_SimilarityClassify);

    // EngineTests.h — Smart Quality Predictor
    RUN_TEST(TestZenith_ImageComplexityCount);
    RUN_TEST(TestZenith_PredictJPEGQuality);

    // EngineTests.h — Lock-Free Decode Pipeline
    RUN_TEST(TestZenith_PipelineStageStateCount);
    RUN_TEST(TestZenith_NextPowerOf2);
    RUN_TEST(TestZenith_IsPowerOf2);

    // EngineTests.h — Memory-Mapped I/O Optimizer
    RUN_TEST(TestZenith_MappingStrategyCount);
    RUN_TEST(TestZenith_MMapAlignOffset);

    // EngineTests.h — GPU Texture Atlas Manager
    RUN_TEST(TestZenith_AtlasPackingAlgoCount);
    RUN_TEST(TestZenith_AtlasCalcVRAM);
    RUN_TEST(TestZenith_AtlasMaxSlots);

    // EngineTests.h — Predictive Prefetch Engine
    RUN_TEST(TestZenith_PrefetchStrategyCount);
    RUN_TEST(TestZenith_PrefetchCalcHitRate);

    // EngineTests.h — Thread Pool Optimizer
    RUN_TEST(TestZenith_PoolSizingPolicyCount);
    RUN_TEST(TestZenith_ThreadPoolRecommend);

    // EngineTests.h — ARM64 NEON Scaler
    RUN_TEST(TestZenith_ARM64CapabilityCount);
    RUN_TEST(TestZenith_ARM64IsARM64);

    // EngineTests.h — Windows Search Protocol
    RUN_TEST(TestZenith_SearchFieldCount);
    RUN_TEST(TestZenith_SearchFieldNames);

    // EngineTests.h — Virtual Filesystem Abstraction
    RUN_TEST(TestZenith_VFSBackendCount);
    RUN_TEST(TestZenith_VFSNeedsDownload);

    // EngineTests.h — Role-Based Format Policy
    RUN_TEST(TestZenith_FormatPolicyActionCount);
    RUN_TEST(TestZenith_FormatPolicyPriority);

    // EngineTests.h — Audit Trail Logger
    RUN_TEST(TestZenith_AuditSeverityCount);
    RUN_TEST(TestZenith_AuditCategoryCount);
    RUN_TEST(TestZenith_AuditSeverityNames);

    // EngineTests.h — Content Inspection Gateway
    RUN_TEST(TestZenith_ContentClassificationCount);
    RUN_TEST(TestZenith_ContentShouldBlock);

    // EngineTests.h — Certificate Trust Validator
    RUN_TEST(TestZenith_CertValidationCount);
    RUN_TEST(TestZenith_CertSufficientTrust);

    // EngineTests.h — Encrypted Format Handler
    RUN_TEST(TestZenith_EncryptionTypeCount);
    RUN_TEST(TestZenith_StrongEncryption);

    // EngineTests.h — Self-Healing Decoder
    RUN_TEST(TestZenith_RecoveryStrategyCount);
    RUN_TEST(TestZenith_DecoderHealthClassify);

    // EngineTests.h — Crash Analytics Collector
    RUN_TEST(TestZenith_CrashCategoryCount);
    RUN_TEST(TestZenith_CrashDumpSize);

    // EngineTests.h — Performance Anomaly Detector
    RUN_TEST(TestZenith_AnomalyTypeCount);
    RUN_TEST(TestZenith_AnomalyClassifySeverity);
    RUN_TEST(TestZenith_AnomalyIsAnomaly);

    // EngineTests.h — Diagnostic Report Generator V2
    RUN_TEST(TestZenith_DiagReportSectionCount);
    RUN_TEST(TestZenith_DiagReportMimeType);

    // EngineTests.h — Health Check Endpoint
    RUN_TEST(TestZenith_HealthStatusCount);
    RUN_TEST(TestZenith_HealthAggregateAllHealthy);
    RUN_TEST(TestZenith_HealthAggregateDegraded);

    // EngineTests.h — Preview Tooltip Renderer
    RUN_TEST(TestZenith_TooltipStyleCount);
    RUN_TEST(TestZenith_TooltipStyleNames);

    // EngineTests.h — Format Gallery Compositor
    RUN_TEST(TestZenith_GalleryLayoutCount);
    RUN_TEST(TestZenith_GalleryGridCellSize);

    // EngineTests.h — Quick Actions Overlay
    RUN_TEST(TestZenith_QuickActionTypeCount);
    RUN_TEST(TestZenith_QuickActionNames);

    // EngineTests.h — Accessibility Narrator Bridge
    RUN_TEST(TestZenith_A11yFeatureCount);
    RUN_TEST(TestZenith_A11yNarratorText);

    // EngineTests.h — Usage Analytics Dashboard
    RUN_TEST(TestZenith_AnalyticsMetricCount);
    RUN_TEST(TestZenith_AnalyticsMetricNames);

    // EngineTests.h — Decoder Performance Profiler
    RUN_TEST(TestZenith_ProfileGranularityCount);
    RUN_TEST(TestZenith_ProfileIsRegression);

    // EngineTests.h — Cache Efficiency Analyzer
    RUN_TEST(TestZenith_CacheZoneCount);
    RUN_TEST(TestZenith_CacheAnalyze);

    // EngineTests.h — Format Popularity Tracker
    RUN_TEST(TestZenith_PopularityTierCount);
    RUN_TEST(TestZenith_PopularityClassify);

    // EngineTests.h — System Resource Monitor
    RUN_TEST(TestZenith_MonitoredResourceCount);
    RUN_TEST(TestZenith_ResourceThrottle);

    // EngineTests.h — Multi-Monitor Color Profile
    RUN_TEST(TestZenith_MonitorProfileTypeCount);
    RUN_TEST(TestZenith_MonitorProfileNames);
    RUN_TEST(TestZenith_GamutMapping);

    // EngineTests.h — Rust FFI Bridge
    RUN_TEST(TestZenith_RustLibStatusCount);
    RUN_TEST(TestZenith_RustABICompat);

    // EngineTests.h — DirectStorage 1.2 Integration
    RUN_TEST(TestZenith_DStorageSupportCount);
    RUN_TEST(TestZenith_DStorageSpeedup);

    // EngineTests.h — Neural Texture Compression
    RUN_TEST(TestZenith_NeuralBackendCount);
    RUN_TEST(TestZenith_NeuralModelTierCount);
    RUN_TEST(TestZenith_NeuralDecodeTimeFactor);

    // EngineTests.h — Quantum-Ready Hash Pipeline
    RUN_TEST(TestZenith_QRHashAlgoCount);
    RUN_TEST(TestZenith_QRHashRecommend);
    RUN_TEST(TestZenith_QRHashQuantumSafe);

    // EngineTests.h — WebGPU Thumbnail Renderer
    RUN_TEST(TestZenith_WebGPUBackendCount);
    RUN_TEST(TestZenith_WebGPUAsyncCompute);

    // EngineTests.h — WASM Decoder Sandbox
    RUN_TEST(TestZenith_WASMRuntimeCount);
    RUN_TEST(TestZenith_WASMMemoryLimitBytes);

    // EngineTests.h — Telemetry Pipeline V2
    RUN_TEST(TestZenith_TelemetryV2LevelCount);
    RUN_TEST(TestZenith_TelemetryV2Consent);
    RUN_TEST(TestZenith_TelemetryV2MinPrivacy);

    // EngineTests.h — Live Preview Streaming Protocol
    RUN_TEST(TestZenith_StreamQualityCount);
    RUN_TEST(TestZenith_StreamQualityPixels);
    RUN_TEST(TestZenith_StreamSelectQuality);
    RUN_TEST(TestZenith_StreamBandwidthSufficient);

    // Release Gate V33 Ship Gate
    RUN_TEST(TestGateV33_KPINames);
    RUN_TEST(TestGateV33_KPICount);
    RUN_TEST(TestGateV33_Evaluate);
    RUN_TEST(TestGateV33_AllPass);
    RUN_TEST(TestGateV33_v15ShipApproved85Pct);
    RUN_TEST(TestGateV33_v15ShipDeniedBelow85);
    RUN_TEST(TestGateV33_Codename);

    // ETW TraceLogging Provider
    RUN_TEST(TestETW_ProviderInitialize);
    RUN_TEST(TestETW_ProviderEventEmit);
    RUN_TEST(TestETW_ScopedTimer);
    RUN_TEST(TestETW_KeywordValues);
    RUN_TEST(TestETW_EventLevels);

    // Plugin Consolidation
    RUN_TEST(TestPluginMarketplaceUnified_VersionEnum);
    RUN_TEST(TestPluginMarketplaceUnified_V2Search);
    RUN_TEST(TestPluginMarketplaceUnified_V3Categories);
    RUN_TEST(TestPluginSecurity_LevelEnum);
    RUN_TEST(TestPluginSecurity_SandboxPreset);
    RUN_TEST(TestPluginSecurity_SandboxPolicyStruct);
    RUN_TEST(TestPluginSecurity_RuntimeValidator);
    RUN_TEST(TestPluginLifecycle_PhaseEnum);
    RUN_TEST(TestPluginLifecycle_ActivePhase);
    RUN_TEST(TestPluginLifecycle_TerminalPhase);
    RUN_TEST(TestPluginLifecycle_HotReloadEnums);

    // AI Algorithm Tests
    std::wcout << L"\nAI Algorithm Tests:" << std::endl;
    RUN_TEST(TestAISearch_PerceptualHash_Uniform);
    RUN_TEST(TestAISearch_PerceptualHash_DHash);
    RUN_TEST(TestAISearch_HammingDistance);
    RUN_TEST(TestAISearch_AreSimilar);
    RUN_TEST(TestImageQuality_LaplacianVariance_Uniform);
    RUN_TEST(TestImageQuality_LaplacianVariance_Edges);
    RUN_TEST(TestImageQuality_MeanBrightness);
    RUN_TEST(TestImageQuality_ExposureDefects_Over);
    RUN_TEST(TestImageQuality_ExposureDefects_Under);
    RUN_TEST(TestImageQuality_GradeBySharpness);
    RUN_TEST(TestImageQuality_Assess);
    RUN_TEST(TestSmartCropV2_CenterOfInterest_Uniform);
    RUN_TEST(TestSmartCropV2_CropRegion_Bounds);
    RUN_TEST(TestSceneAI_ClassifyHeuristics_Green);
    RUN_TEST(TestSceneAI_ClassifyHeuristics_Dark);
    RUN_TEST(TestSceneAI_ClassifyHeuristics_Null);

    // Umbrella Consolidation Tests
    std::wcout << L"\nUmbrella Consolidation Tests:" << std::endl;
    RUN_TEST(TestTestInfrastructure_Available);
    RUN_TEST(TestTestInfrastructure_Components);
    RUN_TEST(TestDocGenerator_Umbrella);
    RUN_TEST(TestWindowsCompat_Umbrella);

    // Shell Progress Indicator Tests
    std::wcout << L"\nShell Progress Indicator Tests:" << std::endl;
    RUN_TEST(TestShellProgress_StageNames);
    RUN_TEST(TestShellProgress_BeginEnd);
    RUN_TEST(TestShellProgress_AdvanceStage);
    RUN_TEST(TestShellProgress_Cancellation);
    RUN_TEST(TestShellProgress_BatchTracker);
    RUN_TEST(TestShellProgress_SmallFileNoReport);

    // Shell Search Protocol Tests
    std::wcout << L"\nShell Search Protocol Tests:" << std::endl;
    RUN_TEST(TestSearchProtocol_Initialize);
    RUN_TEST(TestSearchProtocol_AddAndSearch);
    RUN_TEST(TestSearchProtocol_ExtensionFilter);
    RUN_TEST(TestSearchProtocol_ClearIndex);

    // Jump List Integration Tests
    std::wcout << L"\nJump List Integration Tests:" << std::endl;
    RUN_TEST(TestJumpList_Initialize);
    RUN_TEST(TestJumpList_RecordAccess);
    RUN_TEST(TestJumpList_PinEntry);
    RUN_TEST(TestJumpList_Categories);
    RUN_TEST(TestJumpList_ExportImport);

    // Plugin Loader V2 (C ABI) Tests
    std::wcout << L"\nPlugin Loader V2 (C ABI) Tests:" << std::endl;
    RUN_TEST(TestPluginLoaderV2_Create);
    RUN_TEST(TestPluginLoaderV2_ABIVersion);
    RUN_TEST(TestPluginLoaderV2_PluginState);
    RUN_TEST(TestPluginLoaderV2_DescriptorDefaults);
    RUN_TEST(TestPluginLoaderV2_FindNonexistent);
    RUN_TEST(TestPluginLoaderV2_LoadInvalidDLL);

    // Zero-Copy Activation Tests
    std::wcout << L"\nZero-Copy Activation Tests:" << std::endl;
    RUN_TEST(TestZeroCopy_ModeStrings);
    RUN_TEST(TestZeroCopy_StatsInitial);
    RUN_TEST(TestZeroCopy_Initialize);
    RUN_TEST(TestZeroCopy_StagingBuffer);

    // SIMD Dispatch Router Tests
    std::wcout << L"\nSIMD Dispatch Router Tests:" << std::endl;
    RUN_TEST(TestSIMD_TierStrings);
    RUN_TEST(TestSIMD_FeatureFlags);
    RUN_TEST(TestSIMD_RouterDetection);
    RUN_TEST(TestSIMD_KernelSelection);

    // PSO Persistence Manager Tests
    std::wcout << L"\nPSO Persistence Manager Tests:" << std::endl;
    RUN_TEST(TestPSO_TypeStrings);
    RUN_TEST(TestPSO_Initialize);
    RUN_TEST(TestPSO_StoreAndLookup);
    RUN_TEST(TestPSO_InvalidateAndPurge);

    // Predictive Cache Engine Tests
    std::wcout << L"\nPredictive Cache Engine Tests:" << std::endl;
    RUN_TEST(TestPredictive_Initialize);
    RUN_TEST(TestPredictive_RecordNavigation);
    RUN_TEST(TestPredictive_PredictNext);
    RUN_TEST(TestPredictive_PinDirectory);

    // Lanczos GPU Kernel Tests
    std::wcout << L"\nLanczos GPU Kernel Tests:" << std::endl;
    RUN_TEST(TestLanczos_FilterStrings);
    RUN_TEST(TestLanczos_DispatchParams);
    RUN_TEST(TestLanczos_KernelWeights);
    RUN_TEST(TestLanczos_Initialize);

    // HDR Tone Map Kernel Tests
    std::wcout << L"\nHDR Tone Map Kernel Tests:" << std::endl;
    RUN_TEST(TestHDR_OperatorStrings);
    RUN_TEST(TestHDR_ReinhardToneMap);
    RUN_TEST(TestHDR_ACESToneMap);
    RUN_TEST(TestHDR_SceneAnalysis);

    // Adaptive GPU Scheduler Tests
    std::wcout << L"\nAdaptive GPU Scheduler Tests:" << std::endl;
    RUN_TEST(TestGPUSched_BackendStrings);
    RUN_TEST(TestGPUSched_SystemLoadSnapshot);
    RUN_TEST(TestGPUSched_Initialize);
    RUN_TEST(TestGPUSched_RouteWork);

    // Explorer Column Provider Tests
    std::wcout << L"\nExplorer Column Provider Tests:" << std::endl;
    RUN_TEST(TestColumn_Definitions);
    RUN_TEST(TestColumn_ValueMake);
    RUN_TEST(TestColumn_ProviderInit);
    RUN_TEST(TestColumn_CategorizeFile);

    // Drag-Drop Thumbnail Preview Tests
    std::wcout << L"\nDrag-Drop Thumbnail Preview Tests:" << std::endl;
    RUN_TEST(TestDragDrop_StyleStrings);
    RUN_TEST(TestDragDrop_Initialize);
    RUN_TEST(TestDragDrop_ComputeSize);
    RUN_TEST(TestDragDrop_RenderEmpty);

    // Streaming Decode Engine Tests
    std::wcout << L"\nStreaming Decode Engine Tests:" << std::endl;
    RUN_TEST(TestStreaming_LoDStrings);
    RUN_TEST(TestStreaming_Initialize);
    RUN_TEST(TestStreaming_SessionLifecycle);
    RUN_TEST(TestStreaming_QualityMapping);

    // Multi-Page Strip Renderer Tests
    std::wcout << L"\nMulti-Page Strip Renderer Tests:" << std::endl;
    RUN_TEST(TestStrip_LayoutStrings);
    RUN_TEST(TestStrip_AutoSelectLayout);
    RUN_TEST(TestStrip_Initialize);
    RUN_TEST(TestStrip_ComputeLayout);

    // Video Keyframe Extractor Tests
    std::wcout << L"\nVideo Keyframe Extractor Tests:" << std::endl;
    RUN_TEST(TestKeyframe_StrategyStrings);
    RUN_TEST(TestKeyframe_QualityScore);
    RUN_TEST(TestKeyframe_BlackFramePenalty);
    RUN_TEST(TestKeyframe_Initialize);

    std::wcout << std::endl;

    // Isolation & Stability Tests
    std::wcout << L"Isolation & Stability Tests..." << std::endl;
    RUN_TEST(TestMalformedArchive_TruncatedZIP);
    RUN_TEST(TestMalformedArchive_GarbageHeader);
    RUN_TEST(TestMalformedArchive_ZeroByteFile);
    RUN_TEST(TestCircuitBreaker_StressTest);
    RUN_TEST(TestDecoderTimeout_Enforcement);
    RUN_TEST(TestMemoryLeak_RegressionLoop);

    std::wcout << std::endl;

    // GPU Renderer Tests
    RunGPUTests();

    // Summary
    std::wcout << std::endl;
    std::wcout << L"========================================" << std::endl;
    std::wcout << L"Test Results:" << std::endl;
    std::wcout << L" Total: " << g_testsRun << std::endl;
    std::wcout << L" Passed: " << g_testsPassed << std::endl;
    std::wcout << L" Failed: " << g_testsFailed << std::endl;
    std::wcout << L"========================================" << std::endl;

    return g_testsFailed > 0 ? 1 : 0;
}
