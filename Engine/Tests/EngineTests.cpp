//==============================================================================
// ExplorerLens Engine - Unit Tests (Harness)
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#include "EngineTestsIncludes.h"

// Test counter definitions (declared extern in EngineTestsMacros.h)
int g_testsRun   = 0;
int g_testsPassed = 0;
int g_testsFailed = 0;


// Forward declarations for test runner functions defined in EngineTests_Mid.cpp

#include "EngineTestsExterns.h"

int main()
{
    std::wcout << L"========================================" << std::endl;
    std::wcout << L"ExplorerLens Engine - Unit Tests" << std::endl;
    std::wcout << L"========================================" << std::endl << std::endl;

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

    // High-DPI Support
    std::wcout << L"High-DPI Support..." << std::endl;
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

    std::wcout << L"Update Engine..." << std::endl;
    RUN_TEST(TestUpdate_ChannelNames);
    RUN_TEST(TestUpdate_StatusNames);
    RUN_TEST(TestUpdate_CompareVersions);
    RUN_TEST(TestUpdate_CheckForUpdate);
    RUN_TEST(TestUpdate_ChannelCount);

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

    // Config Migration Tests (Core:: tests removed — forwarding to Utils now)

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

    // APNG & Animated Format Handler Tests
    std::wcout << L"Animated Format Handler..." << std::endl;
    RUN_TEST(TestAnimHdlr_DetectAPNG);
    RUN_TEST(TestAnimHdlr_FormatNames);
    RUN_TEST(TestAnimHdlr_StrategyNames);
    RUN_TEST(TestAnimHdlr_SelectFrame);
    RUN_TEST(TestAnimHdlr_FormatCount);

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
    RUN_TEST(TestAsyncSA_PriorityNames);
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

    // Animated Image Decoder Tests
    std::wcout << L"\nAnimated Image Decoder Tests:" << std::endl;
    RUN_TEST(TestAnimated_FormatStrings);
    RUN_TEST(TestAnimated_StrategyStrings);
    RUN_TEST(TestAnimated_FrameQuality);
    RUN_TEST(TestAnimated_Initialize);

    // Progressive JPEG Decoder Tests
    std::wcout << L"\nProgressive JPEG Decoder Tests:" << std::endl;
    RUN_TEST(TestProgJPEG_ScanTypeStrings);
    RUN_TEST(TestProgJPEG_MarkerValues);
    RUN_TEST(TestProgJPEG_QualityThreshold);
    RUN_TEST(TestProgJPEG_Initialize);

    // Taskbar Preview Manager Tests
    std::wcout << L"\nTaskbar Preview Manager Tests:" << std::endl;
    RUN_TEST(TestTaskbar_ModeStrings);
    RUN_TEST(TestTaskbar_DefaultStats);
    RUN_TEST(TestTaskbar_TabCreation);
    RUN_TEST(TestTaskbar_Initialize);

    // Search Federated Provider Tests
    std::wcout << L"\nSearch Federated Provider Tests:" << std::endl;
    RUN_TEST(TestFedSearch_QueryTypeStrings);
    RUN_TEST(TestFedSearch_Initialize);
    RUN_TEST(TestFedSearch_IndexFile);
    RUN_TEST(TestFedSearch_Search);

    // Thumbnail Quality Validator Tests
    std::wcout << L"\nThumbnail Quality Validator Tests:" << std::endl;
    RUN_TEST(TestQualityVal_FlagOperators);
    RUN_TEST(TestQualityVal_DefaultThresholds);
    RUN_TEST(TestQualityVal_SetThresholds);
    RUN_TEST(TestQualityVal_BlackImage);

    // Remote Desktop Optimizer Tests
    std::wcout << L"\nRemote Desktop Optimizer Tests:" << std::endl;
    RUN_TEST(TestRemoteRDP_SessionTypeStrings);
    RUN_TEST(TestRemoteRDP_BandwidthTierStrings);
    RUN_TEST(TestRemoteRDP_ProfileForTier);
    RUN_TEST(TestRemoteRDP_Initialize);

    // Power Throttle Manager Tests
    std::wcout << L"\nPower Throttle Manager Tests:" << std::endl;
    RUN_TEST(TestPowerThrottle_LevelStrings);
    RUN_TEST(TestPowerThrottle_ProfileForLevel);
    RUN_TEST(TestPowerThrottle_SourceStrings);
    RUN_TEST(TestPowerThrottle_Initialize);

    // Async Texture Sampler Tests
    std::wcout << L"\nAsync Texture Sampler Tests:" << std::endl;
    RUN_TEST(TestTexSampler_FormatStrings);
    RUN_TEST(TestTexSampler_FilterStrings);
    RUN_TEST(TestTexSampler_MipChainLevels);
    RUN_TEST(TestTexSampler_Initialize);

    // Shader Cache Compiler Tests
    std::wcout << L"\nShader Cache Compiler Tests:" << std::endl;
    RUN_TEST(TestShaderCache_StageStrings);
    RUN_TEST(TestShaderCache_VariantHash);
    RUN_TEST(TestShaderCache_Initialize);
    RUN_TEST(TestShaderCache_Stats);

    // Format Signature Detector Tests
    std::wcout << L"\nFormat Signature Detector Tests:" << std::endl;
    RUN_TEST(TestFmtSig_ClassStrings);
    RUN_TEST(TestFmtSig_BuiltinSignatures);
    RUN_TEST(TestFmtSig_DetectJPEG);
    RUN_TEST(TestFmtSig_IsImageFormat);

    // Smart Pointer Pool Tests
    std::wcout << L"\nSmart Pointer Pool Tests:" << std::endl;
    RUN_TEST(TestSmartPool_SizeClassStrings);
    RUN_TEST(TestSmartPool_ClassifySize);
    RUN_TEST(TestSmartPool_AcquireRelease);
    RUN_TEST(TestSmartPool_PoolHit);

    // Thumbnail Persistence Layer Tests
    std::wcout << L"\nThumbnail Persistence Layer Tests:" << std::endl;
    RUN_TEST(TestPersistence_EvictionPolicyStrings);
    RUN_TEST(TestPersistence_Initialize);
    RUN_TEST(TestPersistence_StoreAndLookup);
    RUN_TEST(TestPersistence_HitRate);

    // Dark Mode Controls Tests
    std::wcout << L"\nDark Mode Controls Tests:" << std::endl;
    RUN_TEST(TestDarkCtrl_ControlTypeEnum);
    RUN_TEST(TestDarkCtrl_CheckStateDefaults);
    RUN_TEST(TestDarkCtrl_Singleton);
    RUN_TEST(TestDarkCtrl_SetAccentColor);

    // Dark Mode Renderer V2 Tests
    std::wcout << L"\nDark Mode Renderer V2 Tests:" << std::endl;
    RUN_TEST(TestDarkRenderV2_DefaultScheme);
    RUN_TEST(TestDarkRenderV2_LightScheme);
    RUN_TEST(TestDarkRenderV2_PreferredAppMode);
    RUN_TEST(TestDarkRenderV2_Singleton);

    // System Tray Manager Tests
    std::wcout << L"\nSystem Tray Manager Tests:" << std::endl;
    RUN_TEST(TestSysTray_IconStateEnum);
    RUN_TEST(TestSysTray_CommandEnum);
    RUN_TEST(TestSysTray_ActionNames);
    RUN_TEST(TestSysTray_NotInitializedByDefault);

    // WinUI3 Research Tests
    std::wcout << L"\nWinUI3 Research Tests:" << std::endl;
    RUN_TEST(TestWinUI3Res_FeasibilityEnum);
    RUN_TEST(TestWinUI3Res_AssessmentCount);
    RUN_TEST(TestWinUI3Res_ShellExtNotFeasible);
    RUN_TEST(TestWinUI3Res_TotalEffort);

    // Hybrid UI Bridge Tests
    std::wcout << L"\nHybrid UI Bridge Tests:" << std::endl;
    RUN_TEST(TestHybridUI_StateNames);
    RUN_TEST(TestHybridUI_DefaultConfig);
    RUN_TEST(TestHybridUI_PanelIdEnum);
    RUN_TEST(TestHybridUI_InitialState);

    // WinUI3 Migration Engine Tests
    std::wcout << L"\nWinUI3 Migration Engine Tests:" << std::endl;
    RUN_TEST(TestMigration_FrameworkNames);
    RUN_TEST(TestMigration_PhaseNames);
    RUN_TEST(TestMigration_PageCount);
    RUN_TEST(TestMigration_StatusData);

    // CI Hardening Engine Tests
    std::wcout << L"\nCI Hardening Engine Tests:" << std::endl;
    RUN_TEST(TestCIHarden_TargetNames);
    RUN_TEST(TestCIHarden_StageNames);
    RUN_TEST(TestCIHarden_Pipeline);
    RUN_TEST(TestCIHarden_AllPassing);

    // Code Coverage Engine Tests
    std::wcout << L"\nCode Coverage Engine Tests:" << std::endl;
    RUN_TEST(TestCovEng_ToolNames);
    RUN_TEST(TestCovEng_MetricNames);
    RUN_TEST(TestCovEng_Results);
    RUN_TEST(TestCovEng_OverallCoverage);

    // Integration Test Framework V2 Tests
    std::wcout << L"\nIntegration Test Framework V2 Tests:" << std::endl;
    RUN_TEST(TestIntegV2_CategoryStrings);
    RUN_TEST(TestIntegV2_StatusStrings);
    RUN_TEST(TestIntegV2_RegisterAndRun);
    RUN_TEST(TestIntegV2_RunCategory);

    // Integration Test Orchestrator Tests
    std::wcout << L"\nIntegration Test Orchestrator Tests:" << std::endl;
    RUN_TEST(TestOrch_ScenarioTypeStrings);
    RUN_TEST(TestOrch_ModeStrings);
    RUN_TEST(TestOrch_RunScenario);
    RUN_TEST(TestOrch_DependencyFailure);

    // Continuous Fuzz Orchestrator Tests
    std::wcout << L"\nContinuous Fuzz Orchestrator Tests:" << std::endl;
    RUN_TEST(TestFuzzOrch_StrategyStrings);
    RUN_TEST(TestFuzzOrch_SeverityStrings);
    RUN_TEST(TestFuzzOrch_CorpusAndCrash);
    RUN_TEST(TestFuzzOrch_MinimizeCorpus);

    // Static Analysis CI Gate Tests
    std::wcout << L"\nStatic Analysis CI Gate Tests:" << std::endl;
    RUN_TEST(TestSAGate_ToolStrings);
    RUN_TEST(TestSAGate_VerdictStrings);
    RUN_TEST(TestSAGate_EnableDisable);
    RUN_TEST(TestSAGate_EvaluatePassFail);

    // Security Compliance Tests
    std::wcout << L"\nSecurity Compliance Tests:" << std::endl;
    RUN_TEST(TestSecComp_RegulationNames);
    RUN_TEST(TestSecComp_DataClassNames);
    RUN_TEST(TestSecComp_AuditEventNames);
    RUN_TEST(TestSecComp_SupplyChainFormats);

    // Documentation Generator Tests
    std::wcout << L"\nDocumentation Generator Tests:" << std::endl;
    RUN_TEST(TestDocGen_SectionNames);
    RUN_TEST(TestDocGen_FormatNamesAndExt);
    RUN_TEST(TestDocGen_RegisterDecoder);
    RUN_TEST(TestDocSync_MockAudit);

    // Installer Tests
    std::wcout << L"\nInstaller Tests:" << std::endl;
    RUN_TEST(TestInstaller_PrereqCount);
    RUN_TEST(TestInstaller_PhaseEnum);
    RUN_TEST(TestInstaller_TypeEnum);
    RUN_TEST(TestInstaller_MSIXChannels);

    // Zero-Copy Activation Tests
    std::wcout << L"\nZero-Copy Activation Tests:" << std::endl;
    RUN_TEST(TestZeroCopyAct_ModeStrings);
    RUN_TEST(TestZeroCopyAct_StagingBuffer);
    RUN_TEST(TestZeroCopyAct_Stats);
    RUN_TEST(TestZeroCopyAct_Lifecycle);

    // Parallel I/O Pipeline Tests
    std::wcout << L"\nParallel I/O Pipeline Tests:" << std::endl;
    RUN_TEST(TestParallelIO_BackendNamesV2);
    RUN_TEST(TestParallelIO_PriorityNamesV2);
    RUN_TEST(TestParallelIO_VolumeTypes);
    RUN_TEST(TestParallelIO_DefaultConfig);

    // SIMD Scaler + ARM64 Tests
    std::wcout << L"\nSIMD Scaler Tests:" << std::endl;
    RUN_TEST(TestSIMDScal_PathNames);
    RUN_TEST(TestSIMDScal_ValidateDimensions);
    RUN_TEST(TestSIMDScal_CalculateSize);

    // PSO Cache V2 Tests
    std::wcout << L"\nPSO Cache V2 Tests:" << std::endl;
    RUN_TEST(TestPSOCacheV2_StateNamesV2);
    RUN_TEST(TestPSOCacheV2_PipelineTypes);
    RUN_TEST(TestPSOCacheV2_WarmupStrategies);
    RUN_TEST(TestPSOCacheV2_EntryDefaults);

    // Cache Warming Service Tests
    std::wcout << L"\nCache Warming Service Tests:" << std::endl;
    RUN_TEST(TestCacheWarm_StrategyNamesV2);
    RUN_TEST(TestCacheWarm_PriorityNamesV2);
    RUN_TEST(TestCacheWarm_JobStatusNamesV2);
    RUN_TEST(TestCacheWarm_DefaultConfigV2);

    // File Integrity Monitor Tests
    std::wcout << L"\nFile Integrity Monitor Tests:" << std::endl;
    RUN_TEST(TestIntegrityMon_CheckTypeNames);
    RUN_TEST(TestIntegrityMon_StatusNames);
    RUN_TEST(TestIntegrityMon_DefaultConstruction);
    RUN_TEST(TestIntegrityMon_RecordFields);

    // Thumbnail Diff Engine Tests
    std::wcout << L"\nThumbnail Diff Engine Tests:" << std::endl;
    RUN_TEST(TestDiffEngine_AlgorithmNames);
    RUN_TEST(TestDiffEngine_SeverityNames);
    RUN_TEST(TestDiffEngine_DefaultResult);
    RUN_TEST(TestDiffEngine_Construction);

    // Decoder Sandbox Policy Tests
    std::wcout << L"\nDecoder Sandbox Policy Tests:" << std::endl;
    RUN_TEST(TestSandboxPolicy_LevelNames);
    RUN_TEST(TestSandboxPolicy_ResourceLimitNames);
    RUN_TEST(TestSandboxPolicy_DefaultConstruction);
    RUN_TEST(TestSandboxPolicy_MaxMemory);

    // Intelligent Prefetch V2 Tests
    std::wcout << L"\nIntelligent Prefetch V2 Tests:" << std::endl;
    RUN_TEST(TestPrefetchV2_StrategyNames);
    RUN_TEST(TestPrefetchV2_PatternNames);
    RUN_TEST(TestPrefetchV2_Prediction);
    RUN_TEST(TestPrefetchV2_ConfidenceThreshold);

    // GPU Workload Balancer Tests
    std::wcout << L"\nGPU Workload Balancer Tests:" << std::endl;
    RUN_TEST(TestGPUBalance_StrategyNames);
    RUN_TEST(TestGPUBalance_WorkloadTypeNames);
    RUN_TEST(TestGPUBalance_Construction);
    RUN_TEST(TestGPUBalance_MaxGPUs);

    // Filesystem Watchdog Tests
    std::wcout << L"\nFilesystem Watchdog Tests:" << std::endl;
    RUN_TEST(TestFSWatchdog_EventNames);
    RUN_TEST(TestFSWatchdog_ScopeNames);
    RUN_TEST(TestFSWatchdog_DefaultConstruction);
    RUN_TEST(TestFSWatchdog_MaxDirectories);

    // Compression Benchmark Tests
    std::wcout << L"\nCompression Benchmark Tests:" << std::endl;
    RUN_TEST(TestCompBench_AlgoNames);
    RUN_TEST(TestCompBench_MetricNames);
    RUN_TEST(TestCompBench_DefaultResult);
    RUN_TEST(TestCompBench_Iterations);

    // Explorer Band Integration Tests
    std::wcout << L"\nExplorer Band Integration Tests:" << std::endl;
    RUN_TEST(TestBandInteg_PositionNames);
    RUN_TEST(TestBandInteg_StateNames);
    RUN_TEST(TestBandInteg_DefaultConfig);
    RUN_TEST(TestBandInteg_Construction);

    // Thumbnail Stream Protocol Tests
    std::wcout << L"\nThumbnail Stream Protocol Tests:" << std::endl;
    RUN_TEST(TestStreamProto_ProtocolNames);
    RUN_TEST(TestStreamProto_StateNames);
    RUN_TEST(TestStreamProto_EndpointConfig);
    RUN_TEST(TestStreamProto_Timeout);

    // Registry Snapshot Manager Tests
    std::wcout << L"\nRegistry Snapshot Manager Tests:" << std::endl;
    RUN_TEST(TestRegSnap_ScopeNames);
    RUN_TEST(TestRegSnap_ActionNames);
    RUN_TEST(TestRegSnap_Construction);
    RUN_TEST(TestRegSnap_MaxSnapshots);

    // Hot Reload Config Engine Tests
    std::wcout << L"\nHot Reload Config Engine Tests:" << std::endl;
    RUN_TEST(TestHotReload_SourceNames);
    RUN_TEST(TestHotReload_TriggerNamesV2);
    RUN_TEST(TestHotReload_DefaultResult);
    RUN_TEST(TestHotReload_PollInterval);

    // COM Diagnostics Engine Tests
    std::wcout << L"\nCOM Diagnostics Engine Tests:" << std::endl;
    RUN_TEST(TestCOMDiag_HealthStatusNames);
    RUN_TEST(TestCOMDiag_RepairActionNames);
    RUN_TEST(TestCOMDiag_DiagnosticResult);
    RUN_TEST(TestCOMDiag_CLSIDConstant);

    // Thumbnail Watermark Tests
    std::wcout << L"\nThumbnail Watermark Tests:" << std::endl;
    RUN_TEST(TestWatermark_PositionNames);
    RUN_TEST(TestWatermark_TypeNames);
    RUN_TEST(TestWatermark_ApplyAndConfig);

    // Batch Rename Preview Tests
    std::wcout << L"\nBatch Rename Preview Tests:" << std::endl;
    RUN_TEST(TestRenamePreview_PatternNames);
    RUN_TEST(TestRenamePreview_StateNames);
    RUN_TEST(TestRenamePreview_GenerateAndGet);

    // Duplicate File Detector Tests
    std::wcout << L"\nDuplicate File Detector Tests:" << std::endl;
    RUN_TEST(TestDupDetect_HashMethodNames);
    RUN_TEST(TestDupDetect_ConfidenceNames);
    RUN_TEST(TestDupDetect_ScanDirectory);

    // Thumbnail Annotation Tests
    std::wcout << L"\nThumbnail Annotation Tests:" << std::endl;
    RUN_TEST(TestAnnotation_TypeNames);
    RUN_TEST(TestAnnotation_StyleNames);
    RUN_TEST(TestAnnotation_AddRemoveRender);

    // Cache Migration Engine Tests
    std::wcout << L"\nCache Migration Engine Tests:" << std::endl;
    RUN_TEST(TestCacheMigration_FormatNames);
    RUN_TEST(TestCacheMigration_StateNames);
    RUN_TEST(TestCacheMigration_MigrateFlow);

    // Explorer Context Menu Tests
    std::wcout << L"\nExplorer Context Menu Tests:" << std::endl;
    RUN_TEST(TestCtxMenu_ActionNames);
    RUN_TEST(TestCtxMenu_ItemStateNames);
    RUN_TEST(TestCtxMenu_BuildAndExecute);

    // Adaptive Quality Scaler Tests
    std::wcout << L"\nAdaptive Quality Scaler Tests:" << std::endl;
    RUN_TEST(TestQualityScaler_TierNames);
    RUN_TEST(TestQualityScaler_ReasonNames);
    RUN_TEST(TestQualityScaler_Evaluate);

    // Thumbnail Compare View Tests
    std::wcout << L"\nThumbnail Compare View Tests:" << std::endl;
    RUN_TEST(TestCompare_ModeNames);
    RUN_TEST(TestCompare_SourceNames);
    RUN_TEST(TestCompare_RunComparison);

    // File Type Statistics Tests
    std::wcout << L"\nFile Type Statistics Tests:" << std::endl;
    RUN_TEST(TestFileStats_CategoryNames);
    RUN_TEST(TestFileStats_TimeRangeNames);
    RUN_TEST(TestFileStats_RecordAndQuery);

    // Memory Defragmenter Tests
    std::wcout << L"\nMemory Defragmenter Tests:" << std::endl;
    RUN_TEST(TestDefrag_LevelNames);
    RUN_TEST(TestDefrag_StrategyNames);
    RUN_TEST(TestDefrag_AnalyzeAndDefrag);

    // Shell Notification Engine Tests
    std::wcout << L"\nShell Notification Engine Tests:" << std::endl;
    RUN_TEST(TestShellNotify_TypeNames);
    RUN_TEST(TestShellNotify_PriorityNames);
    RUN_TEST(TestShellNotify_SendAndFlush);

    // Thumbnail Export Engine Tests
    std::wcout << L"\nThumbnail Export Engine Tests:" << std::endl;
    RUN_TEST(TestThumbExport_FormatNamesV2);
    RUN_TEST(TestThumbExport_DestNames);
    RUN_TEST(TestThumbExport_SingleExport);

    // Thumbnail Version Control Tests
    std::wcout << L"\nThumbnail Version Control Tests:" << std::endl;
    RUN_TEST(TestTVC_VersionNames);
    RUN_TEST(TestTVC_ActionNames);
    RUN_TEST(TestTVC_CreateAndRollback);

    // File Preview Router Tests
    std::wcout << L"\nFile Preview Router Tests:" << std::endl;
    RUN_TEST(TestFPR_HandlerNames);
    RUN_TEST(TestFPR_PriorityNames);
    RUN_TEST(TestFPR_RouteAndRegister);

    // Clipboard Thumbnail Manager Tests
    std::wcout << L"\nClipboard Thumbnail Manager Tests:" << std::endl;
    RUN_TEST(TestClip_FormatNames);
    RUN_TEST(TestClip_TargetNames);
    RUN_TEST(TestClip_CopyAndPaste);

    // Format Conversion Pipeline Tests
    std::wcout << L"\nFormat Conversion Pipeline Tests:" << std::endl;
    RUN_TEST(TestFCP_TargetNames);
    RUN_TEST(TestFCP_QualityNames);
    RUN_TEST(TestFCP_ConvertSingle);

    // Vulkan Memory Allocator Tests
    std::wcout << L"\nVulkan Memory Allocator Tests:" << std::endl;
    RUN_TEST(TestVMA_TierNames);
    RUN_TEST(TestVMA_StrategyNames);
    RUN_TEST(TestVMA_AllocAndFree);

    // Decoder Priority Scheduler Tests
    std::wcout << L"\nDecoder Priority Scheduler Tests:" << std::endl;
    RUN_TEST(TestDPS_PriorityNames);
    RUN_TEST(TestDPS_PolicyNames);
    RUN_TEST(TestDPS_SubmitAndCancel);

    // Error Reporting Pipeline Tests
    std::wcout << L"\nError Reporting Pipeline Tests:" << std::endl;
    RUN_TEST(TestERP_DomainNames);
    RUN_TEST(TestERP_AggregationNames);
    RUN_TEST(TestERP_ReportAndQuery);

    // Enterprise Audit Pipeline Tests
    std::wcout << L"\nEnterprise Audit Pipeline Tests:" << std::endl;
    RUN_TEST(TestEAP_ActionNames);
    RUN_TEST(TestEAP_DestNames);
    RUN_TEST(TestEAP_LogAndRetrieve);

    // Resource Quota Manager Tests
    std::wcout << L"\nResource Quota Manager Tests:" << std::endl;
    RUN_TEST(TestRQM_ResourceNames);
    RUN_TEST(TestRQM_EnforcementNames);
    RUN_TEST(TestRQM_SetAndCheck);

    // Access Token Validator Tests
    std::wcout << L"\nAccess Token Validator Tests:" << std::endl;
    RUN_TEST(TestATV_TypeNames);
    RUN_TEST(TestATV_ResultNames);
    RUN_TEST(TestATV_Validate);

    // Cache Encryption Layer Tests
    std::wcout << L"\nCache Encryption Layer Tests:" << std::endl;
    RUN_TEST(TestCEL_AlgoNames);
    RUN_TEST(TestCEL_KDFNames);
    RUN_TEST(TestCEL_EncryptDecrypt);

    // Explorer Preview Pane Tests
    std::wcout << L"\nExplorer Preview Pane Tests:" << std::endl;
    RUN_TEST(TestEPP_ModeNames);
    RUN_TEST(TestEPP_LayoutNames);
    RUN_TEST(TestEPP_ActivateAndRefresh);

    // DirectShow Thumbnail Bridge Tests
    std::wcout << L"\nDirectShow Thumbnail Bridge Tests:" << std::endl;
    RUN_TEST(Test_DirectShowBridge_FilterTypeNames);
    RUN_TEST(Test_DirectShowBridge_StatusNames);
    RUN_TEST(Test_DirectShowBridge_ConnectDisconnect);
    RUN_TEST(Test_DirectShowBridge_GrabFrame);

    // Shell Extension Health Monitor Tests
    std::wcout << L"\nShell Extension Health Monitor Tests:" << std::endl;
    RUN_TEST(Test_HealthMonitor_StatusNames);
    RUN_TEST(Test_HealthMonitor_RecoveryNames);
    RUN_TEST(Test_HealthMonitor_CheckHealth);
    RUN_TEST(Test_HealthMonitor_AutoRecover);

    // Thumbnail Color Space Tests
    std::wcout << L"\nThumbnail Color Space Tests:" << std::endl;
    RUN_TEST(Test_ColorSpace_TypeNames);
    RUN_TEST(Test_ColorSpace_GammaNames);
    RUN_TEST(Test_ColorSpace_SetWorkingSpace);
    RUN_TEST(Test_ColorSpace_ConvertNoOp);

    // Async IO Completion Engine Tests
    std::wcout << L"\nAsync IO Completion Engine Tests:" << std::endl;
    RUN_TEST(Test_AsyncIOCP_PriorityNames);
    RUN_TEST(Test_AsyncIOCP_StatusNames);
    RUN_TEST(Test_AsyncIOCP_SubmitAndPoll);
    RUN_TEST(Test_AsyncIOCP_Cancel);

    // EXIF Orientation Fixer Tests
    std::wcout << L"\nEXIF Orientation Fixer Tests:" << std::endl;
    RUN_TEST(Test_ExifFixer_OrientationNames);
    RUN_TEST(Test_ExifFixer_ModeNames);
    RUN_TEST(Test_ExifFixer_ApplyRotation);
    RUN_TEST(Test_ExifFixer_ReadOrientation);

    // Multi-Monitor DPI Scaler Tests
    std::wcout << L"\nMulti-Monitor DPI Scaler Tests:" << std::endl;
    RUN_TEST(Test_DPIScaler_ModeNames);
    RUN_TEST(Test_DPIScaler_ProfileNames);
    RUN_TEST(Test_DPIScaler_ScaleFactor);
    RUN_TEST(Test_DPIScaler_ScaleForMonitor);

    // VirtualAlloc Optimizer Tests
    std::wcout << L"\nVirtualAlloc Optimizer Tests:" << std::endl;
    RUN_TEST(Test_VAlloc_StrategyNames);
    RUN_TEST(Test_VAlloc_ProtectionNames);
    RUN_TEST(Test_VAlloc_AllocateAndRelease);
    RUN_TEST(Test_VAlloc_OptimizeWorking);

    // Thumbnail Histogram Tests
    std::wcout << L"\nThumbnail Histogram Tests:" << std::endl;
    RUN_TEST(Test_Histogram_ChannelNames);
    RUN_TEST(Test_Histogram_BinSizeNames);
    RUN_TEST(Test_Histogram_Compute);
    RUN_TEST(Test_Histogram_PeakAndMean);

    // File Association Manager Tests
    std::wcout << L"\nFile Association Manager Tests:" << std::endl;
    RUN_TEST(Test_FileAssoc_ScopeNames);
    RUN_TEST(Test_FileAssoc_ConflictNames);
    RUN_TEST(Test_FileAssoc_RegisterUnregister);
    RUN_TEST(Test_FileAssoc_GetConflicts);

    // DX12 Fence Manager Tests
    std::wcout << L"\nDX12 Fence Manager Tests:" << std::endl;
    RUN_TEST(Test_DX12Fence_StateNames);
    RUN_TEST(Test_DX12Fence_WaitModeNames);
    RUN_TEST(Test_DX12Fence_CreateAndSignal);
    RUN_TEST(Test_DX12Fence_WaitForFence);

    // Localization Engine Tests
    std::wcout << L"\nLocalization Engine Tests:" << std::endl;
    RUN_TEST(Test_Locale_IdNames);
    RUN_TEST(Test_Locale_CategoryNames);
    RUN_TEST(Test_Locale_SetAndGetLocale);
    RUN_TEST(Test_Locale_GetString);

    // Thumbnail Sprite Sheet Tests
    std::wcout << L"\nThumbnail Sprite Sheet Tests:" << std::endl;
    RUN_TEST(Test_SpriteSheet_LayoutNames);
    RUN_TEST(Test_SpriteSheet_FormatNames);
    RUN_TEST(Test_SpriteSheet_AddAndGenerate);
    RUN_TEST(Test_SpriteSheet_EstimatedSize);

    // Cache Telemetry Collector Tests
    std::wcout << L"\nCache Telemetry Collector Tests:" << std::endl;
    RUN_TEST(Test_CacheTelemetry_EventNames);
    RUN_TEST(Test_CacheTelemetry_IntervalNames);
    RUN_TEST(Test_CacheTelemetry_RecordAndHitRate);
    RUN_TEST(Test_CacheTelemetry_Export);

    // Windows Search Integration Tests
    std::wcout << L"\nWindows Search Integration Tests:" << std::endl;
    RUN_TEST(Test_WinSearch_PropertyNames);
    RUN_TEST(Test_WinSearch_IndexingStateNames);
    RUN_TEST(Test_WinSearch_RegisterProvider);
    RUN_TEST(Test_WinSearch_QueryProperties);

    // AdaptiveCacheBudgetManager Tests
    std::wcout << L"\nAdaptive Cache Budget Manager Tests:" << std::endl;
    RUN_TEST(Test_ACBudget_TierNames);
    RUN_TEST(Test_ACBudget_DefaultBudgets);
    RUN_TEST(Test_ACBudget_Rebalance);

    // ArchiveMemoryCompactor Tests
    std::wcout << L"\nArchive Memory Compactor Tests:" << std::endl;
    RUN_TEST(Test_AMemCompact_SlabStates);
    RUN_TEST(Test_AMemCompact_EvictionPolicies);
    RUN_TEST(Test_AMemCompact_TrackSlab);
    RUN_TEST(Test_AMemCompact_Compact);

    // BatchProcessor Tests
    std::wcout << L"\nBatch Processor Tests:" << std::endl;
    RUN_TEST(Test_BatchProc_JobPriorities);
    RUN_TEST(Test_BatchProc_JobStatuses);
    RUN_TEST(Test_BatchProc_SubmitAndQueue);
    RUN_TEST(Test_BatchProc_PauseResume);

    // BufferPoolAllocator Tests
    std::wcout << L"\nBuffer Pool Allocator Tests:" << std::endl;
    RUN_TEST(Test_BufPool_SlabClassNames);
    RUN_TEST(Test_BufPool_ClassifyDimension);
    RUN_TEST(Test_BufPool_SlabPoolAcquireRelease);
    RUN_TEST(Test_BufPool_PoolStats);

    // CacheKeyGenerator Tests
    std::wcout << L"\nCache Key Generator Tests:" << std::endl;
    RUN_TEST(Test_CacheKey_Generate);
    RUN_TEST(Test_CacheKey_HashFNV);
    RUN_TEST(Test_CacheKey_ValidKey);
    RUN_TEST(Test_CacheKey_GenerateWithTime);

    // CRTConsistencyManager Tests
    std::wcout << L"\nCRT Consistency Manager Tests:" << std::endl;
    RUN_TEST(Test_CRT_ModeNames);
    RUN_TEST(Test_CRT_StatusNames);
    RUN_TEST(Test_CRT_Counts);
    RUN_TEST(Test_CRT_AuditLibraries);

    // DeadCodeAudit Tests
    std::wcout << L"\nDead Code Audit Tests:" << std::endl;
    RUN_TEST(Test_DCAudit_TypeNames);
    RUN_TEST(Test_DCAudit_SeverityNames);
    RUN_TEST(Test_DCAudit_Instance);
    RUN_TEST(Test_DCAudit_CountByStatus);

    // DeadCodeAuditor Tests
    std::wcout << L"\nDead Code Auditor Tests:" << std::endl;
    RUN_TEST(Test_DCAuditor_CategoryNames);
    RUN_TEST(Test_DCAuditor_SeverityNames);
    RUN_TEST(Test_DCAuditor_RunAudit);
    RUN_TEST(Test_DCAuditor_AllResolved);

    // DecoderHealthDashboard Tests
    std::wcout << L"\nDecoder Health Dashboard Tests:" << std::endl;
    RUN_TEST(Test_DHDash_CircuitStates);
    RUN_TEST(Test_DHDash_HealthStatuses);
    RUN_TEST(Test_DHDash_CreateAndRegister);
    RUN_TEST(Test_DHDash_RecordAndStats);

    // DecoderHealthMonitor Tests
    std::wcout << L"\nDecoder Health Monitor Tests:" << std::endl;
    RUN_TEST(Test_DHMon_RecordSuccess);
    RUN_TEST(Test_DHMon_RecordFailure);
    RUN_TEST(Test_DHMon_IsAvailable);
    RUN_TEST(Test_DHMon_IsHealthy);

    // DecoderHotsetManager Tests
    std::wcout << L"\nDecoder Hotset Manager Tests:" << std::endl;
    RUN_TEST(Test_DHotset_LoadStates);
    RUN_TEST(Test_DHotset_Modes);
    RUN_TEST(Test_DHotset_RegisterDecoder);
    RUN_TEST(Test_DHotset_LoadUnload);

    // DecoderPriority Tests
    std::wcout << L"\nDecoder Priority Manager Tests:" << std::endl;
    RUN_TEST(Test_DPriority_Levels);
    RUN_TEST(Test_DPriority_RegisterDecoder);
    RUN_TEST(Test_DPriority_Fallback);
    RUN_TEST(Test_DPriority_Availability);

    // DiagnosticsExporter Tests
    std::wcout << L"\nDiagnostics Exporter Tests:" << std::endl;
    RUN_TEST(Test_DiagExport_Categories);
    RUN_TEST(Test_DiagExport_CreateAndAdd);
    RUN_TEST(Test_DiagExport_ErrorLog);
    RUN_TEST(Test_DiagExport_Export);

    // DirectoryFormatProfiler Tests
    std::wcout << L"\nDirectory Format Profiler Tests:" << std::endl;
    RUN_TEST(Test_DirProfile_FormatFamilies);
    RUN_TEST(Test_DirProfile_ClassifyExt);
    RUN_TEST(Test_DirProfile_ProfileDir);
    RUN_TEST(Test_DirProfile_Budget);

    // ErrorContext Tests
    std::wcout << L"\nError Context Tests:" << std::endl;
    RUN_TEST(Test_ErrCtx_PushPop);
    RUN_TEST(Test_ErrCtx_CreateContext);
    RUN_TEST(Test_ErrCtx_ScopedContext);
    RUN_TEST(Test_ErrCtx_FilePath);

    // ETWSinkComplete Tests
    std::wcout << L"\nETW Sink Complete Tests:" << std::endl;
    RUN_TEST(Test_ETWSink_Channels);
    RUN_TEST(Test_ETWSink_RotationStrategies);
    RUN_TEST(Test_ETWSink_SchemaVersion);
    RUN_TEST(Test_ETWSink_ConfigFactories);

    // ExplorerWorkScheduler Tests
    std::wcout << L"\nExplorer Work Scheduler Tests:" << std::endl;
    RUN_TEST(Test_WorkSched_Priorities);
    RUN_TEST(Test_WorkSched_Submit);
    RUN_TEST(Test_WorkSched_Cancel);
    RUN_TEST(Test_WorkSched_Dequeue);

    // FormatFallbackEngine Tests — moved to FallbackEngineTests.cpp

    // FormatGalleryView Tests
    std::wcout << L"\nFormat Gallery View Tests:" << std::endl;
    RUN_TEST(Test_FmtGallery_TileSizes);
    RUN_TEST(Test_FmtGallery_SortOrders);
    RUN_TEST(Test_FmtGallery_Instance);
    RUN_TEST(Test_FmtGallery_Initialize);

    // FormatGroupManager Tests
    std::wcout << L"\nFormat Group Manager Tests:" << std::endl;
    RUN_TEST(Test_FmtGroup_GroupNames);
    RUN_TEST(Test_FmtGroup_ActionNames);
    RUN_TEST(Test_FmtGroup_Counts);
    RUN_TEST(Test_FmtGroup_GetGroups);

    // ProgramClosureV83 Tests
    std::wcout << L"\nProgram Closure V83 Tests:" << std::endl;
    RUN_TEST(Test_ProgClosure_States);
    RUN_TEST(Test_ProgClosure_CreateReport);
    RUN_TEST(Test_ProgClosure_BlockComplete);
    RUN_TEST(Test_ProgClosure_DefaultSeed);

    // ReleaseReadinessDashboard Tests
    std::wcout << L"\nRelease Readiness Dashboard Tests:" << std::endl;
    RUN_TEST(Test_RelReady_Categories);
    RUN_TEST(Test_RelReady_ReadinessLevels);
    RUN_TEST(Test_RelReady_Evaluate);
    RUN_TEST(Test_RelReady_FormatReport);

    // ReproducibleBuildVerifier Tests
    std::wcout << L"\nReproducible Build Verifier Tests:" << std::endl;
    RUN_TEST(Test_ReproBuild_ArtifactTypes);
    RUN_TEST(Test_ReproBuild_VerifyStatuses);
    RUN_TEST(Test_ReproBuild_StrictPolicy);
    RUN_TEST(Test_ReproBuild_RelaxedPolicy);

    // SettingsImportExport Tests
    std::wcout << L"\nSettings Import/Export Tests:" << std::endl;
    RUN_TEST(Test_Settings_CategoryNames);
    RUN_TEST(Test_Settings_ActionNames);
    RUN_TEST(Test_Settings_FormatNames);
    RUN_TEST(Test_Settings_ValidateJSON);

    // HotModeDirectoryEngine Tests
    std::wcout << L"\nHot Mode Directory Engine Tests:" << std::endl;
    RUN_TEST(Test_HotModeDir_ChangeTypes);
    RUN_TEST(Test_HotModeDir_Thresholds);
    RUN_TEST(Test_HotModeDir_IndexDirectory);
    RUN_TEST(Test_HotModeDir_IsHotMode);

    // MemoryOptimizationEngine Tests
    std::wcout << L"\nMemory Optimization Engine Tests:" << std::endl;
    RUN_TEST(Test_MemOpt_Config);
    RUN_TEST(Test_MemOpt_SubsystemEnum);
    RUN_TEST(Test_MemOpt_Create);
    RUN_TEST(Test_MemOpt_BudgetCheck);

    // MemorySoakValidator Tests
    std::wcout << L"\nMemory Soak Validator Tests:" << std::endl;
    RUN_TEST(Test_MemSoak_Verdict);
    RUN_TEST(Test_MemSoak_Config);
    RUN_TEST(Test_MemSoak_Snapshot);
    RUN_TEST(Test_MemSoak_Evaluate);

    // CrashIntelligence Tests
    std::wcout << L"\nCrash Intelligence Tests:" << std::endl;
    RUN_TEST(Test_CrashInt_StackFrame);
    RUN_TEST(Test_CrashInt_Metadata);
    RUN_TEST(Test_CrashInt_Signature);
    RUN_TEST(Test_CrashInt_Bucket);

    // IsolationModeSelector Tests
    std::wcout << L"\nIsolation Mode Selector Tests:" << std::endl;
    RUN_TEST(Test_IsoMode_Enum);
    RUN_TEST(Test_IsoMode_Name);
    RUN_TEST(Test_IsoMode_Instance);
    RUN_TEST(Test_IsoMode_Trust);

    // SmallObjectPool Tests
    std::wcout << L"\nSmall Object Pool Tests:" << std::endl;
    RUN_TEST(Test_SmallPool_Create);
    RUN_TEST(Test_SmallPool_Allocate);
    RUN_TEST(Test_SmallPool_PoolPtr);
    RUN_TEST(Test_SmallPool_Stats);

    // ValidationHelpers Tests
    std::wcout << L"\nValidation Helpers Tests:" << std::endl;
    RUN_TEST(Test_ValHelp_FilePath);
    RUN_TEST(Test_ValHelp_Dimensions);
    RUN_TEST(Test_ValHelp_Buffer);
    RUN_TEST(Test_ValHelp_Extension);

    // VersionDriftDetector Tests
    std::wcout << L"\nVersion Drift Detector Tests:" << std::endl;
    RUN_TEST(Test_VDDetect_SemVer);
    RUN_TEST(Test_VDDetect_Severity);
    RUN_TEST(Test_VDDetect_Policy);
    RUN_TEST(Test_VDDetect_Scan);

    // VersionDriftGate Tests
    std::wcout << L"\nVersion Drift Gate Tests:" << std::endl;
    RUN_TEST(Test_VDGate_Create);
    RUN_TEST(Test_VDGate_Severity);
    RUN_TEST(Test_VDGate_Register);
    RUN_TEST(Test_VDGate_Policy);

    // PluginActivation Tests
    std::wcout << L"\nPlugin Activation Tests:" << std::endl;
    RUN_TEST(Test_PlugAct_Flags);
    RUN_TEST(Test_PlugAct_State);
    RUN_TEST(Test_PlugAct_Discovery);
    RUN_TEST(Test_PlugAct_Lifecycle);

    // PluginHostBridge Tests
    std::wcout << L"\nPlugin Host Bridge Tests:" << std::endl;
    RUN_TEST(Test_PHBridge_States);
    RUN_TEST(Test_PHBridge_Config);
    RUN_TEST(Test_PHBridge_StateName);
    RUN_TEST(Test_PHBridge_Instance);

    // PluginHostClient Tests
    std::wcout << L"\nPlugin Host Client Tests:" << std::endl;
    RUN_TEST(Test_PHClient_Compile);
    RUN_TEST(Test_PHClient_Types);
    RUN_TEST(Test_PHClient_NullCheck);
    RUN_TEST(Test_PHClient_Size);

    // PluginHostIPC Tests
    std::wcout << L"\nPlugin Host IPC Tests:" << std::endl;
    RUN_TEST(Test_PHIPC_MsgType);
    RUN_TEST(Test_PHIPC_Header);
    RUN_TEST(Test_PHIPC_ConnState);
    RUN_TEST(Test_PHIPC_MsgName);

    // PluginRuntimeValidation Tests
    std::wcout << L"\nPlugin Runtime Validation Tests:" << std::endl;
    RUN_TEST(Test_PRunVal_State);
    RUN_TEST(Test_PRunVal_Transport);
    RUN_TEST(Test_PRunVal_Scenario);
    RUN_TEST(Test_PRunVal_Validator);

    // EXIFOrientation Tests
    std::wcout << L"\nEXIF Orientation Tests:" << std::endl;
    RUN_TEST(Test_EXIF_Normal);
    RUN_TEST(Test_EXIF_Values);
    RUN_TEST(Test_EXIF_Transpose);
    RUN_TEST(Test_EXIF_AllCases);

    // AuditLogger Tests
    std::wcout << L"\nAudit Logger Tests:" << std::endl;
    RUN_TEST(Test_AuditLog_Events);
    RUN_TEST(Test_AuditLog_Instance);
    RUN_TEST(Test_AuditLog_Enabled);
    RUN_TEST(Test_AuditLog_LogAccess);

    // CIPipeline Tests
    std::wcout << L"\nCI Pipeline Tests:" << std::endl;
    RUN_TEST(Test_CI_Stages);
    RUN_TEST(Test_CI_Scanners);
    RUN_TEST(Test_CI_Config);
    RUN_TEST(Test_CI_Flags);

    // CodeCoverage Tests
    std::wcout << L"\nCode Coverage Tests:" << std::endl;
    RUN_TEST(Test_CodeCov_Tool);
    RUN_TEST(Test_CodeCov_Metric);
    RUN_TEST(Test_CodeCov_Report);
    RUN_TEST(Test_CodeCov_Threshold);

    // BitmapPool Tests
    std::wcout << L"\nBitmap Pool Tests:" << std::endl;
    RUN_TEST(Test_BmpPool_Config);
    RUN_TEST(Test_BmpPool_Stats);
    RUN_TEST(Test_BmpPool_HitRate);
    RUN_TEST(Test_BmpPool_Instance);

    // DecoderCircuitBreaker Tests
    std::wcout << L"\nDecoder Circuit Breaker Tests:" << std::endl;
    RUN_TEST(Test_CircBreak_States);
    RUN_TEST(Test_CircBreak_Create);
    RUN_TEST(Test_CircBreak_Available);
    RUN_TEST(Test_CircBreak_Reset);

    // MemoryPressureControllerV2 Tests
    std::wcout << L"\nMemory Pressure Controller V2 Tests:" << std::endl;
    RUN_TEST(Test_MemPressV2_Levels);
    RUN_TEST(Test_MemPressV2_Actions);
    RUN_TEST(Test_MemPressV2_Ladder);
    RUN_TEST(Test_MemPressV2_Evaluate);

    // PerformanceActivation Tests
    std::wcout << L"\nPerformance Activation Tests:" << std::endl;
    RUN_TEST(Test_PerfAct_SIMD);
    RUN_TEST(Test_PerfAct_Profile);
    RUN_TEST(Test_PerfAct_Instance);
    RUN_TEST(Test_PerfAct_Scaler);

    // PerformanceProfiler Tests
    std::wcout << L"\nPerformance Profiler Tests:" << std::endl;
    RUN_TEST(Test_PerfProf_Components);
    RUN_TEST(Test_PerfProf_Stats);
    RUN_TEST(Test_PerfProf_Instance);
    RUN_TEST(Test_PerfProf_Enabled);

    // PerfRegressionGate Tests
    std::wcout << L"\nPerf Regression Gate Tests:" << std::endl;
    RUN_TEST(Test_PerfReg_KPIs);
    RUN_TEST(Test_PerfReg_Thresholds);
    RUN_TEST(Test_PerfReg_Verdict);
    RUN_TEST(Test_PerfReg_Evaluate);

    // PluginCompatibilityKitV2 Tests
    std::wcout << L"\nPlugin Compatibility Kit V2 Tests:" << std::endl;
    RUN_TEST(Test_PlugCompat_ABIVersion);
    RUN_TEST(Test_PlugCompat_Surface);
    RUN_TEST(Test_PlugCompat_PerfGate);
    RUN_TEST(Test_PlugCompat_MemGate);

    // PluginSandboxPolicy Tests
    std::wcout << L"\nPlugin Sandbox Policy Tests:" << std::endl;
    RUN_TEST(Test_PlugSandbox_Limits);
    RUN_TEST(Test_PlugSandbox_Presets);
    RUN_TEST(Test_PlugSandbox_Teardown);
    RUN_TEST(Test_PlugSandbox_Validate);

    // MultiTierCache Tests
    std::wcout << L"\nMulti-Tier Cache Tests:" << std::endl;
    RUN_TEST(Test_MTC_Create);
    RUN_TEST(Test_MTC_Tiers);
    RUN_TEST(Test_MTC_MemoryEviction);
    RUN_TEST(Test_MTC_Manager);
    RUN_TEST(Test_MTC_BloomFilter);
    RUN_TEST(Test_MTC_Policy);

    // ThumbnailCache Tests
    std::wcout << L"\nThumbnail Cache Tests:" << std::endl;
    RUN_TEST(Test_ThumbCache_Create);
    RUN_TEST(Test_ThumbCache_Lookup);
    RUN_TEST(Test_ThumbCache_Evict);
    RUN_TEST(Test_ThumbCache_Stats);

    // USNCacheInvalidation Tests
    std::wcout << L"\nUSN Cache Invalidation Tests:" << std::endl;
    RUN_TEST(Test_USNCache_FileIdentity);
    RUN_TEST(Test_USNCache_VolumeHandle);
    RUN_TEST(Test_USNCache_Journal);
    RUN_TEST(Test_USNCache_Track);

    // CodecLoader Tests
    std::wcout << L"\nCodec Loader Tests:" << std::endl;
    RUN_TEST(Test_CodecLoad_State);
    RUN_TEST(Test_CodecLoad_Create);
    RUN_TEST(Test_CodecLoad_Enum);
    RUN_TEST(Test_CodecLoad_Unload);

    // CodecModuleSpecs Tests
    std::wcout << L"\nCodec Module Specs Tests:" << std::endl;
    RUN_TEST(Test_CodecSpec_Create);
    RUN_TEST(Test_CodecSpec_Version);
    RUN_TEST(Test_CodecSpec_Formats);
    RUN_TEST(Test_CodecSpec_Validate);

    // FormatConverter Tests
    std::wcout << L"\nFormat Converter Tests:" << std::endl;
    RUN_TEST(Test_FmtConv_OutputFmt);
    RUN_TEST(Test_FmtConv_Create);
    RUN_TEST(Test_FmtConv_Convert);
    RUN_TEST(Test_FmtConv_Pipeline);

    // ICodecModule Tests
    std::wcout << L"\nCodec ABI Tests:" << std::endl;
    RUN_TEST(Test_CodecABI_Version);
    RUN_TEST(Test_CodecABI_PixelFmt);
    RUN_TEST(Test_CodecABI_Result);
    RUN_TEST(Test_CodecABI_Macros);

    // LazyCodecManager Tests
    std::wcout << L"\nLazy Codec Manager Tests:" << std::endl;
    RUN_TEST(Test_LazyCodec_Create);
    RUN_TEST(Test_LazyCodec_Census);
    RUN_TEST(Test_LazyCodec_Load);
    RUN_TEST(Test_LazyCodec_Scan);

    // Accessibility Tests
    std::wcout << L"\nAccessibility Tests:" << std::endl;
    RUN_TEST(Test_A11y_Include);
    RUN_TEST(Test_A11y_Engine);
    RUN_TEST(Test_A11y_Suite);
    RUN_TEST(Test_A11y_Pipeline);

    // BuildConfig Tests
    std::wcout << L"\nBuild Config Tests:" << std::endl;
    RUN_TEST(Test_BuildCfg_Macros);
    RUN_TEST(Test_BuildCfg_Platform);
    RUN_TEST(Test_BuildCfg_Config);
    RUN_TEST(Test_BuildCfg_Inline);

    // BuildValidation Tests
    std::wcout << L"\nBuild Validation Tests:" << std::endl;
    RUN_TEST(Test_BuildVal_Info);
    RUN_TEST(Test_BuildVal_Runtime);
    RUN_TEST(Test_BuildVal_Version);
    RUN_TEST(Test_BuildVal_Flags);

    // Config Tests
    std::wcout << L"\nConfig Tests:" << std::endl;
    RUN_TEST(Test_Config_Create);
    RUN_TEST(Test_Config_Features);
    RUN_TEST(Test_Config_Defaults);
    RUN_TEST(Test_Config_MaxSize);

    // DarkMode Tests
    std::wcout << L"\nDark Mode Tests:" << std::endl;
    RUN_TEST(Test_DarkMode_Include);
    RUN_TEST(Test_DarkMode_Engine);
    RUN_TEST(Test_DarkMode_Renderer);
    RUN_TEST(Test_DarkMode_Controls);

    // DeadCodeAnalysis Tests
    std::wcout << L"\nDead Code Analysis Tests:" << std::endl;
    RUN_TEST(Test_DCA_Include);
    RUN_TEST(Test_DCA_Audit);
    RUN_TEST(Test_DCA_Auditor);
    RUN_TEST(Test_DCA_Report);

    // EngineAPI Tests
    std::wcout << L"\nEngine API Tests:" << std::endl;
    RUN_TEST(Test_API_Version);
    RUN_TEST(Test_API_BuildDate);
    RUN_TEST(Test_API_Macros);
    RUN_TEST(Test_API_Config);

    // ICacheProvider Tests
    std::wcout << L"\nICacheProvider Tests:" << std::endl;
    RUN_TEST(Test_ICache_Include);
    RUN_TEST(Test_ICache_Interface);
    RUN_TEST(Test_ICache_Size);
    RUN_TEST(Test_ICache_Null);

    // IFormatDetector Tests
    std::wcout << L"\nIFormatDetector Tests:" << std::endl;
    RUN_TEST(Test_IFmtDet_Include);
    RUN_TEST(Test_IFmtDet_Interface);
    RUN_TEST(Test_IFmtDet_Size);
    RUN_TEST(Test_IFmtDet_Null);

    // IGPURenderer Tests
    std::wcout << L"\nIGPURenderer Tests:" << std::endl;
    RUN_TEST(Test_IGPURend_Include);
    RUN_TEST(Test_IGPURend_Interface);
    RUN_TEST(Test_IGPURend_Size);
    RUN_TEST(Test_IGPURend_Null);

    // IThumbnailDecoder Tests
    std::wcout << L"\nIThumbnailDecoder Tests:" << std::endl;
    RUN_TEST(Test_IThumbDec_Include);
    RUN_TEST(Test_IThumbDec_Interface);
    RUN_TEST(Test_IThumbDec_Size);
    RUN_TEST(Test_IThumbDec_Null);

    // LibraryInventoryManager Tests
    std::wcout << L"\nLibrary Inventory Manager Tests:" << std::endl;
    RUN_TEST(Test_LibInv_BuildStatus);
    RUN_TEST(Test_LibInv_Category);
    RUN_TEST(Test_LibInv_Entry);
    RUN_TEST(Test_LibInv_Manager);

    // Logger Tests
    std::wcout << L"\nLogger Tests:" << std::endl;
    RUN_TEST(Test_Logger_Macros);
    RUN_TEST(Test_Logger_Info);
    RUN_TEST(Test_Logger_Error);
    RUN_TEST(Test_Logger_Levels);

    // ObservabilityIntegration Tests
    std::wcout << L"\nObservability Integration Tests:" << std::endl;
    RUN_TEST(Test_Obs_Level);
    RUN_TEST(Test_Obs_Privacy);
    RUN_TEST(Test_Obs_Event);
    RUN_TEST(Test_Obs_Sink);

    // PluginTypes Tests
    std::wcout << L"\nPlugin Types Tests:" << std::endl;
    RUN_TEST(Test_PlugTypes_Transfer);
    RUN_TEST(Test_PlugTypes_Status);
    RUN_TEST(Test_PlugTypes_Convert);
    RUN_TEST(Test_PlugTypes_Enum);

    // Telemetry Tests
    std::wcout << L"\nTelemetry Tests:" << std::endl;
    RUN_TEST(Test_Telem_Severity);
    RUN_TEST(Test_Telem_Category);
    RUN_TEST(Test_Telem_Event);
    RUN_TEST(Test_Telem_Pipeline);

    // TelemetryDashboard Tests
    std::wcout << L"\nTelemetry Dashboard Tests:" << std::endl;
    RUN_TEST(Test_TelemDash_Include);
    RUN_TEST(Test_TelemDash_Forward);
    RUN_TEST(Test_TelemDash_Compat);
    RUN_TEST(Test_TelemDash_Load);

    // Types Tests
    std::wcout << L"\nTypes Tests:" << std::endl;
    RUN_TEST(Test_Types_DetectedFmt);
    RUN_TEST(Test_Types_ForwardDecls);
    RUN_TEST(Test_Types_Enum);
    RUN_TEST(Test_Types_Include);

    // VersionManagement Tests
    std::wcout << L"\nVersion Management Tests:" << std::endl;
    RUN_TEST(Test_VerMgmt_Include);
    RUN_TEST(Test_VerMgmt_Sync);
    RUN_TEST(Test_VerMgmt_Drift);
    RUN_TEST(Test_VerMgmt_Audit);

    // VideoCodecRouter Tests
    std::wcout << L"\nVideo Codec Router Tests:" << std::endl;
    RUN_TEST(Test_VidCodec_Backend);
    RUN_TEST(Test_VidCodec_Route);
    RUN_TEST(Test_VidCodec_Router);
    RUN_TEST(Test_VidCodec_Config);

    // ArchiveGridPreview Tests
    std::wcout << L"\nArchive Grid Preview Tests:" << std::endl;
    RUN_TEST(Test_ArchGrid_Format);
    RUN_TEST(Test_ArchGrid_Create);
    RUN_TEST(Test_ArchGrid_Layout);
    RUN_TEST(Test_ArchGrid_Render);

    // ColorSpaceManager Tests
    std::wcout << L"\nColor Space Manager Tests:" << std::endl;
    RUN_TEST(Test_ColorSpc_Enum);
    RUN_TEST(Test_ColorSpc_Manager);
    RUN_TEST(Test_ColorSpc_Convert);
    RUN_TEST(Test_ColorSpc_Tone);

    // EBookCoverExtractor Tests
    std::wcout << L"\nEBook Cover Extractor Tests:" << std::endl;
    RUN_TEST(Test_EBook_Format);
    RUN_TEST(Test_EBook_Status);
    RUN_TEST(Test_EBook_Extract);
    RUN_TEST(Test_EBook_Cover);

    // ExampleDecoder Tests
    std::wcout << L"\nExample Decoder Tests:" << std::endl;
    RUN_TEST(Test_ExDec_Create);
    RUN_TEST(Test_ExDec_Name);
    RUN_TEST(Test_ExDec_Extensions);
    RUN_TEST(Test_ExDec_Decode);

    // FarbfeldDecoder Tests
    std::wcout << L"\nFarbfeld Decoder Tests:" << std::endl;
    RUN_TEST(Test_Farbfeld_Create);
    RUN_TEST(Test_Farbfeld_Format);
    RUN_TEST(Test_Farbfeld_Decode);
    RUN_TEST(Test_Farbfeld_Validate);

    // JPEG2000Decoder Tests
    std::wcout << L"\nJPEG2000 Decoder Tests:" << std::endl;
    RUN_TEST(Test_JP2_Format);
    RUN_TEST(Test_JP2_Extensions);
    RUN_TEST(Test_JP2_Decode);
    RUN_TEST(Test_JP2_Validate);

    // JXRWICDecoder Tests
    std::wcout << L"\nJXR WIC Decoder Tests:" << std::endl;
    RUN_TEST(Test_JXR_Format);
    RUN_TEST(Test_JXR_Pixel);
    RUN_TEST(Test_JXR_Decode);
    RUN_TEST(Test_JXR_Validate);

    // OptimizedArchiveReader Tests
    std::wcout << L"\nOptimized Archive Reader Tests:" << std::endl;
    RUN_TEST(Test_OptArch_Create);
    RUN_TEST(Test_OptArch_FileEntry);
    RUN_TEST(Test_OptArch_Read);
    RUN_TEST(Test_OptArch_Scan);

    // PCXDecoder Tests
    std::wcout << L"\nPCX Decoder Tests:" << std::endl;
    RUN_TEST(Test_PCX_Header);
    RUN_TEST(Test_PCX_Create);
    RUN_TEST(Test_PCX_Decode);
    RUN_TEST(Test_PCX_Validate);

    // WMFDecoder Tests
    std::wcout << L"\nWMF Decoder Tests:" << std::endl;
    RUN_TEST(Test_WMF_Create);
    RUN_TEST(Test_WMF_Format);
    RUN_TEST(Test_WMF_Decode);
    RUN_TEST(Test_WMF_Validate);

    // D3D11Renderer Tests
    std::wcout << L"\nD3D11 Renderer Tests:" << std::endl;
    RUN_TEST(Test_D3D11_Create);
    RUN_TEST(Test_D3D11_BatchReq);
    RUN_TEST(Test_D3D11_Render);
    RUN_TEST(Test_D3D11_Config);

    // GDIRenderer Tests
    std::wcout << L"\nGDI Renderer Tests:" << std::endl;
    RUN_TEST(Test_GDIRend_Create);
    RUN_TEST(Test_GDIRend_Render);
    RUN_TEST(Test_GDIRend_Scale);
    RUN_TEST(Test_GDIRend_Config);

    // ThumbnailPipeline Tests
    std::wcout << L"\nThumbnail Pipeline Tests:" << std::endl;
    RUN_TEST(Test_ThumbPipe_Config);
    RUN_TEST(Test_ThumbPipe_Create);
    RUN_TEST(Test_ThumbPipe_Generate);
    RUN_TEST(Test_ThumbPipe_Stats);

    // PluginManager Tests
    std::wcout << L"\nPlugin Manager Tests:" << std::endl;
    RUN_TEST(Test_PlugMgr_Create);
    RUN_TEST(Test_PlugMgr_Load);
    RUN_TEST(Test_PlugMgr_Unload);
    RUN_TEST(Test_PlugMgr_List);

    // PluginMarketplace Tests
    std::wcout << L"\nPlugin Marketplace Tests:" << std::endl;
    RUN_TEST(Test_Marketplace_PkgType);
    RUN_TEST(Test_Marketplace_Arch);
    RUN_TEST(Test_Marketplace_Version);
    RUN_TEST(Test_Marketplace_Create);

    // PluginTrustChain Tests
    std::wcout << L"\nPlugin Trust Chain Tests:" << std::endl;
    RUN_TEST(Test_TrustChain_Level);
    RUN_TEST(Test_TrustChain_Cert);
    RUN_TEST(Test_TrustChain_Chain);
    RUN_TEST(Test_TrustChain_Verify);

    // COMApartmentAudit Tests
    std::wcout << L"\nCOM Apartment Audit Tests:" << std::endl;
    RUN_TEST(Test_COMAudit_ApartType);
    RUN_TEST(Test_COMAudit_ThreadSafe);
    RUN_TEST(Test_COMAudit_Entry);
    RUN_TEST(Test_COMAudit_Scenario);

    // HardwareCapabilities Tests
    std::wcout << L"\nHardware Capabilities Tests:" << std::endl;
    RUN_TEST(Test_HWCaps_Create);
    RUN_TEST(Test_HWCaps_CPU);
    RUN_TEST(Test_HWCaps_SIMD);
    RUN_TEST(Test_HWCaps_Detect);

    // PerceptualHashing Tests
    std::wcout << L"\nPerceptual Hashing Tests:" << std::endl;
    RUN_TEST(Test_PHash_Algo);
    RUN_TEST(Test_PHash_Struct);
    RUN_TEST(Test_PHash_Compute);
    RUN_TEST(Test_PHash_Compare);

    // ======== Deep Functional Tests ========
    std::wcout << std::endl;
    std::wcout << L"Deep Functional Tests:" << std::endl;

    // SmartCropEngine
    RUN_TEST(TestSmartCrop_FindBest);
    RUN_TEST(TestSmartCrop_TopCrops);
    RUN_TEST(TestSmartCrop_NullInput);
    RUN_TEST(TestSmartCrop_Stats);

    // ImageQualityAssessorV2
    RUN_TEST(TestIQAv2_AssessBlack);
    RUN_TEST(TestIQAv2_AssessMidGray);
    RUN_TEST(TestIQAv2_SubMetrics);
    RUN_TEST(TestIQAv2_Stats);

    // PluginNamedPipeBridge
    RUN_TEST(TestPipeBridge_InitState);
    RUN_TEST(TestPipeBridge_Stats);

    // CrashIntelligenceEngine
    RUN_TEST(TestCrashIntel_Singleton);
    RUN_TEST(TestCrashIntel_Initialize);
    RUN_TEST(TestCrashIntel_CaptureTrace);
    RUN_TEST(TestCrashIntel_Stats);

    // PluginHotReloadManager
    RUN_TEST(TestHotReload_InitStats);
    RUN_TEST(TestHotReload_SetDir);
    RUN_TEST(TestHotReload_RegisterHash);

    // PluginCompatibilityKit
    RUN_TEST(TestCompatKit_ABIVersion);
    RUN_TEST(TestCompatKit_EmptyStats);
    RUN_TEST(TestCompatKit_ValidateNonExistent);

    // PluginPerformanceProfiler
    RUN_TEST(TestPluginProfiler_NoRecords);
    RUN_TEST(TestPluginProfiler_BeginEnd);

    // PluginTrustChainValidator
    RUN_TEST(TestTrustChain_DefaultPolicy);
    RUN_TEST(TestTrustChain_SetPolicy);
    RUN_TEST(TestTrustChain_MeetsPolicy);
    RUN_TEST(TestTrustChain_Publisher);
    RUN_TEST(TestTrustChain_Stats);

    // PerformanceDashboard
    RUN_TEST(TestPerfDash_Singleton);
    RUN_TEST(TestPerfDash_RecordAndGet);
    RUN_TEST(TestPerfDash_Reset);

    // SystemTrayManager
    RUN_TEST(TestSysTray_Singleton);

    // ThumbnailPreviewEngine
    RUN_TEST(TestPreview_LoadImage);
    RUN_TEST(TestPreview_LoadNull);
    RUN_TEST(TestPreview_ZoomState);
    RUN_TEST(TestPreview_ZoomClamp);

    // FormatGroupManager
    RUN_TEST(TestFmtGroup_NonZeroGroups);

    // DiagnosticsCollector
    RUN_TEST(TestDiagCollect_SystemInfo);
    RUN_TEST(TestDiagCollect_Version);
    RUN_TEST(TestDiagCollect_Decoders);
    RUN_TEST(TestDiagCollect_FormatReport);

    // IntegrationTestRunner
    RUN_TEST(TestIntegRunner_EmptyRun);
    RUN_TEST(TestIntegRunner_AddCase);
    RUN_TEST(TestIntegRunner_Stats);

    // SBOMGenerator
    RUN_TEST(TestSBOM_Defaults);
    RUN_TEST(TestSBOM_AddDep);
    RUN_TEST(TestSBOM_GenerateSPDX);
    RUN_TEST(TestSBOM_ProjectInfo);

    // InstallerLifecycleManager
    RUN_TEST(TestInstaller_DetectState);
    RUN_TEST(TestInstaller_CLSID);
    RUN_TEST(TestInstaller_AppKey);

    // TelemetryEngine
    RUN_TEST(TestTelemetryV2_PrivacyMode);
    RUN_TEST(TestTelemetryV2_RecordAndCount);
    RUN_TEST(TestTelemetryV2_Purge);

    // DocumentationGenerator
    RUN_TEST(TestDocGen_Endpoints);
    RUN_TEST(TestDocGen_Markdown);

    // Advanced Decoders
    RUN_TEST(TestAPNG_CanDecode);
    RUN_TEST(TestAPNG_GetName);
    RUN_TEST(TestFLIF_CanDecode);
    RUN_TEST(TestFLIF_GetName);
    RUN_TEST(TestBPG_CanDecode);
    RUN_TEST(TestBPG_GetName);
    RUN_TEST(TestRGBE_CanDecode);
    RUN_TEST(TestRGBE_GetName);
    RUN_TEST(TestWebP2_CanDecode);

    // Document & Text
    RUN_TEST(TestMarkdown_CanDecode);
    RUN_TEST(TestMarkdown_GetName);
    RUN_TEST(TestSourceCode_CanRender);
    RUN_TEST(TestSourceCode_Analyze);
    RUN_TEST(TestRTF_CanDecode);
    RUN_TEST(TestLaTeX_CanDecode);
    RUN_TEST(TestStructuredData_CanDecode);

    // Archive & Compression
    RUN_TEST(TestZstd_CanDecode);
    RUN_TEST(TestBrotli_CanDecode);
    RUN_TEST(TestLZ4Frame_CanDecode);
    RUN_TEST(TestXZ_CanDecode);
    RUN_TEST(TestSnappy_CanDecode);

    // 3D & CAD
    RUN_TEST(TestCOLLADA_CanDecode);
    RUN_TEST(TestFBX_CanDecode);

    // Media Enhancement
    RUN_TEST(TestMIDI_CanDecode);
    RUN_TEST(TestWaveform_CanDecode);
    RUN_TEST(TestSpectrogram_GetName);
    RUN_TEST(TestVideoTimeline_GetName);
    RUN_TEST(TestSubtitle_CanDecode);

    // Enterprise & Security
    RUN_TEST(TestCert_CanDecode);
    RUN_TEST(TestRegExport_CanDecode);
    RUN_TEST(TestRegExport_Parse);
    RUN_TEST(TestShortcut_CanDecode);
    RUN_TEST(TestMSI_CanDecode);
    RUN_TEST(TestDiskImage_CanDecode);

    // Performance Optimization
    RUN_TEST(TestBufferPool_AcquireRelease);
    RUN_TEST(TestMemoization_GetName);
    RUN_TEST(TestMemoization_Stats);
    RUN_TEST(TestPrefetch_EnqueueDequeue);
    RUN_TEST(TestScheduler_Submit);
    RUN_TEST(TestMMap_ShouldUse);

    // Quality & Observability
    RUN_TEST(TestHistogram_Record);
    RUN_TEST(TestHealth_Signal);
    RUN_TEST(TestRegression_GetName);
    RUN_TEST(TestResourceProfiler_Snapshot);

    // Smart Features
    RUN_TEST(TestComplexity_Estimate);
    RUN_TEST(TestStrategy_GetName);

    // Platform & Integration
    RUN_TEST(TestClipboard_GetName);
    RUN_TEST(TestStatusBar_Generate);
    RUN_TEST(TestTooltip_GetName);
    RUN_TEST(TestBatchProgress_Lifecycle);

    // V15 Zenith Feature Tests — Batch 50
    std::wcout << std::endl;
    std::wcout << L"V15 Zenith Features (Batch 50)..." << std::endl;
    RUN_TEST(TestAudioSpectrogram_Generate);
    RUN_TEST(TestAudioSpectrogram_PeakAmplitude);
    RUN_TEST(TestArchiveHierarchyMap_AddEntries);
    RUN_TEST(TestArchiveHierarchyMap_MaxDepth);
    RUN_TEST(TestCodeSyntax_Classify);
    RUN_TEST(TestCodeSyntax_Keywords);
    RUN_TEST(TestPerceptualHashEngine_AHash);
    RUN_TEST(TestPerceptualHashEngine_Hamming);
    RUN_TEST(TestDominantColor_Average);
    RUN_TEST(TestDominantColor_Distance);
    RUN_TEST(TestPhotoMosaic_Grid);
    RUN_TEST(TestPhotoMosaic_Layout);
    RUN_TEST(TestFontGlyph_Supported);
    RUN_TEST(TestFontGlyph_Characters);
    RUN_TEST(TestSpreadsheet_Layout);
    RUN_TEST(TestSpreadsheet_VisibleCells);
    RUN_TEST(TestSlideStrip_Layout);
    RUN_TEST(TestSlideStrip_Visible);
    RUN_TEST(TestColorHistogram_Compute);
    RUN_TEST(TestColorHistogram_Monochrome);
    RUN_TEST(TestGeoTag_Validate);
    RUN_TEST(TestGeoTag_Distance);
    RUN_TEST(TestFileAge_Category);
    RUN_TEST(TestFileAge_Label);
    RUN_TEST(TestSmartGrid_Layout);
    RUN_TEST(TestSmartGrid_Coverage);
    RUN_TEST(TestMetadataTooltip_Format);
    RUN_TEST(TestMetadataTooltip_HasField);
    RUN_TEST(TestCompressedStream_Ratio);
    RUN_TEST(TestCompressedStream_Savings);
    RUN_TEST(TestAdaptiveContrast_Brightness);
    RUN_TEST(TestAdaptiveContrast_NeedsEnhance);
    RUN_TEST(TestDocDigest_WordCount);
    RUN_TEST(TestDocDigest_ReadTime);
    RUN_TEST(TestArchivePassword_ZipEncrypted);
    RUN_TEST(TestArchivePassword_ZipNotEncrypted);
    RUN_TEST(TestCalendarHeatmap_DayOfWeek);
    RUN_TEST(TestCalendarHeatmap_HeatLevel);
    RUN_TEST(TestFileSizeBadge_Format);
    RUN_TEST(TestFileSizeBadge_Category);
    RUN_TEST(TestCachePruner_Eviction);
    RUN_TEST(TestCachePruner_Score);

    std::wcout << std::endl;

    // Cache Subsystem Tests
    std::wcout << L"Cache Subsystem Tests..." << std::endl;
    RUN_TEST(Test_AdaptiveCacheBudget_InitDefault);
    RUN_TEST(Test_CacheKeyGenerator_Consistency);
    RUN_TEST(Test_CacheKeyGenerator_Uniqueness);
    RUN_TEST(Test_PredictiveCache_BasicPrefetch);
    RUN_TEST(Test_CacheWarmingService_Startup);
    RUN_TEST(Test_PersistentCache_PutGet);
    RUN_TEST(Test_SubMsCache_Performance);
    RUN_TEST(Test_CacheTelemetry_HitMiss);
    RUN_TEST(Test_CacheEncryption_RoundTrip);
    RUN_TEST(Test_MultiTenantCache_Isolation);

    // Pipeline Subsystem Tests
    std::wcout << L"Pipeline Subsystem Tests..." << std::endl;
    RUN_TEST(Test_AsyncPrefetchQueue_Enqueue);
    RUN_TEST(Test_ParallelBatchDecoder_Throughput);
    RUN_TEST(Test_PipelineCircuitBreaker_Trip);
    RUN_TEST(Test_PipelineCircuitBreaker_Reset);
    RUN_TEST(Test_FormatSignature_PNG);
    RUN_TEST(Test_FormatSignature_JPEG);
    RUN_TEST(Test_ThreadPoolOptimizer_CoreCount);
    RUN_TEST(Test_StreamingDecode_Chunked);
    RUN_TEST(Test_DecodeMemoization_CacheHit);

    // Memory Subsystem Tests
    std::wcout << L"Memory Subsystem Tests..." << std::endl;
    RUN_TEST(Test_BufferPool_AllocFree);
    RUN_TEST(Test_BufferPool_Reuse);
    RUN_TEST(Test_MemoryDefragmenter_Compact);
    RUN_TEST(Test_AllocationTracker_Stats);
    RUN_TEST(Test_SmartPointerPool_RAII);
    RUN_TEST(Test_MemoryPressure_Detection);

    // Plugin Subsystem Tests
    std::wcout << L"Plugin Subsystem Tests..." << std::endl;
    RUN_TEST(Test_PluginPerformanceProfiler_Timing);
    RUN_TEST(Test_SharedMemory_CreateOpen);

    // AI Module Tests
    std::wcout << L"AI Module Tests..." << std::endl;
    RUN_TEST(Test_SmartCropV2_CenterFallback);
    RUN_TEST(Test_IQA_ScoreRange);
    RUN_TEST(Test_ImageComplexity_Range);
    RUN_TEST(Test_DecodeStrategy_Optimizer);
    RUN_TEST(Test_SceneUnderstanding_Labels);

    // Decoder Subsystem Tests
    std::wcout << L"Decoder Subsystem Tests..." << std::endl;
    RUN_TEST(Test_APNGDecoder_Create);
    RUN_TEST(Test_JPEG2000Decoder_Create);
    RUN_TEST(Test_EXRDecoder_HDR);
    RUN_TEST(Test_QOIDecoder_MagicBytes);
    RUN_TEST(Test_ICODecoder_MultiRes);
    RUN_TEST(Test_PPMDecoder_Formats);
    RUN_TEST(Test_PCXDecoder_Header);
    RUN_TEST(Test_SGIDecoder_MagicBytes);
    RUN_TEST(Test_XPMDecoder_StringFormat);
    RUN_TEST(Test_BPGDecoder_Detection);
    RUN_TEST(Test_DICOMDecoderV2_Tags);
    RUN_TEST(Test_FITSDecoderV2_Header);
    RUN_TEST(Test_CADFormat_Detection);

    std::wcout << std::endl;

    // GPU Subsystem Tests (Sprint 27)
    std::wcout << L"GPU Subsystem Tests..." << std::endl;
    RUN_TEST(Test_GPUTextureAtlas_Create);
    RUN_TEST(Test_GPUWorkloadBalancer_Distribute);
    RUN_TEST(Test_ShaderCacheCompiler_Directory);
    RUN_TEST(Test_HDRToneMap_Parameters);
    RUN_TEST(Test_LanczosGPU_KernelSize);
    RUN_TEST(Test_AdaptiveGPUScheduler_TaskQueue);
    RUN_TEST(Test_DX12FenceManager_Create);
    RUN_TEST(Test_AsyncTextureSampler_Config);

    // Enterprise Tests (Sprint 27)
    std::wcout << L"Enterprise Tests..." << std::endl;
    RUN_TEST(Test_EnterprisePolicyEngine_GPODefault);
    RUN_TEST(Test_EnterpriseAudit_EventRecord);
    RUN_TEST(Test_ErrorReporting_Severity);

    // Performance Benchmark Tests (Sprint 27)
    std::wcout << L"Performance Benchmark Tests..." << std::endl;
    RUN_TEST(Test_Perf_CacheLookup_Under1ms);
    RUN_TEST(Test_Perf_BloomFilter_Throughput);
    RUN_TEST(Test_Perf_FormatDetection_Fast);
    RUN_TEST(Test_Perf_BatchProcessor_Scaling);
    RUN_TEST(Test_Perf_InputValidation_Fast);
    RUN_TEST(Test_Perf_SecureAlloc_Overhead);

    std::wcout << std::endl;

    // Sprint 31-32: Decoder Security Hardening Tests
    std::wcout << L"Decoder Security Hardening Tests..." << std::endl;
    RUN_TEST(Test_Security_SafeMul32_Basic);
    RUN_TEST(Test_Security_SafeMul64_Basic);
    RUN_TEST(Test_Security_SafeAdd64_Basic);
    RUN_TEST(Test_Security_SafeMulTriple_Overflow);
    RUN_TEST(Test_Security_SafePixelBufferSize);
    RUN_TEST(Test_Security_ValidateDimensions_ZeroAndValid);
    RUN_TEST(Test_Security_ValidatePixelAllocation);
    RUN_TEST(Test_Security_ValidateFileSize);
    RUN_TEST(Test_Security_ValidateBufferAccess);
    RUN_TEST(Test_Security_ValidateBufferRead);
    RUN_TEST(Test_Security_ValidateMagic);
    RUN_TEST(Test_Security_ValidateMagicAt);
    RUN_TEST(Test_Security_ValidatePtr);
    RUN_TEST(Test_Security_SafeRLEReader_Basic);
    RUN_TEST(Test_Security_SafeRLEReader_Overflow);
    RUN_TEST(Test_Security_SafeRLEReader_WriteRepeat);
    RUN_TEST(Test_Security_SafeRLEReader_CopyFromSrc);
    RUN_TEST(Test_Security_Constants);

    // Sprint 33-34: Utils Hardening Tests
    std::wcout << L"\nSprint 33-34 Utils Hardening Tests..." << std::endl;
    RUN_TEST(Test_S33_RegistrySnapshot_ScopeNames);
    RUN_TEST(Test_S33_RegistrySnapshot_CreateAndCompare);
    RUN_TEST(Test_S33_RemoteDesktop_SessionTypeNames);
    RUN_TEST(Test_S33_RemoteDesktop_BandwidthProfiles);
    RUN_TEST(Test_S33_RemoteDesktop_Initialize);
    RUN_TEST(Test_S33_SmallObjectPool_AllocDealloc);
    RUN_TEST(Test_S33_SmallObjectPool_Overflow);
    RUN_TEST(Test_S33_SmallObjectPool_PoolPtr);
    RUN_TEST(Test_S33_ValidationHelpers_FilePath);
    RUN_TEST(Test_S33_ValidationHelpers_Dimensions);
    RUN_TEST(Test_S33_ValidationHelpers_Extension);
    RUN_TEST(Test_S33_ValidationHelpers_SanitizePath);
    RUN_TEST(Test_S33_VersionScanner_StaleDetection);
    RUN_TEST(Test_S33_VersionScanner_Canonical);
    RUN_TEST(Test_S33_VersionScanner_CountStale);
    RUN_TEST(Test_S33_DecoderStatusRegistry_Badges);
    RUN_TEST(Test_S33_VersionInfo_ToString);
    RUN_TEST(Test_S33_MemoryMappedFile_DefaultState);
    RUN_TEST(Test_S33_MemoryMappedFile_InvalidPath);
    RUN_TEST(Test_S33_MemoryMappedFile_MoveSemantics);
    RUN_TEST(Test_S33_InstallerLifecycle_Constants);
    RUN_TEST(Test_S33_InstallerLifecycle_DetectState);
    RUN_TEST(Test_S33_InstallerLifecycle_NotifyShell);
    RUN_TEST(Test_S33_OperationalReadiness_Init);
    RUN_TEST(Test_S33_OperationalReadiness_CheckAll);
    RUN_TEST(Test_S33_ConfigDrift_BaselineAndCheck);
    RUN_TEST(Test_S33_ConfigDrift_DetectChange);

    // Sprint 37-38: Test Coverage Expansion (30 headers, 75 tests)
    std::wcout << L"\nSprint 37-38 Coverage Expansion..." << std::endl;

    // Cache: CacheEncryptionLayer
    RUN_TEST(Test_S37_CacheEncryption_Configure);
    RUN_TEST(Test_S37_CacheEncryption_RoundTrip);
    RUN_TEST(Test_S37_CacheEncryption_NoneMode);
    RUN_TEST(Test_S37_CacheEncryption_RotateKey);
    // Cache: CacheMigrationEngine
    RUN_TEST(Test_S37_CacheMigration_CanMigrate);
    RUN_TEST(Test_S37_CacheMigration_StartAndProgress);
    RUN_TEST(Test_S37_CacheMigration_Reset);
    RUN_TEST(Test_S37_CacheTelemetry_Record);
    RUN_TEST(Test_S37_CacheTelemetry_Export);
    // Cache: MultiTierCache / BloomFilter
    RUN_TEST(Test_S37_BloomFilter_InsertAndContain);
    RUN_TEST(Test_S37_BloomFilter_Clear);
    RUN_TEST(Test_S37_BloomFilter_Serialize);
    RUN_TEST(Test_S37_BloomFilter_FalsePositiveRate);
    // Cache: USNCacheInvalidation / FileIdentity
    RUN_TEST(Test_S37_FileIdentity_CacheKey);
    RUN_TEST(Test_S37_FileIdentity_Equality);
    RUN_TEST(Test_S37_FileIdentity_Stale);
    // Pipeline: DecodeMemoizationEngine
    RUN_TEST(Test_S37_Memoization_StoreAndLookup);
    RUN_TEST(Test_S37_Memoization_Miss);
    RUN_TEST(Test_S37_Memoization_Clear);
    RUN_TEST(Test_S37_Memoization_Stats);
    // Pipeline: ThreadLocalBufferPool
    RUN_TEST(Test_S37_BufferPool_AcquireRelease);
    RUN_TEST(Test_S37_BufferPool_SizeClasses);
    RUN_TEST(Test_S37_BufferPool_Reset);
    // Pipeline: AsyncPrefetchQueue
    RUN_TEST(Test_S37_PrefetchQueue_EnqueueDequeue);
    RUN_TEST(Test_S37_PrefetchQueue_Full);
    RUN_TEST(Test_S37_PrefetchQueue_Stats);
    // Pipeline: PriorityDecodeScheduler
    // Pipeline: StreamingDecodeEngine
    RUN_TEST(Test_S37_DecodeLoD_ToString);
    RUN_TEST(Test_S37_StreamChunk_Defaults);
    RUN_TEST(Test_S37_ProgressiveResult_IsValid);
    // Pipeline: MemoryMappedDecodePath
    RUN_TEST(Test_S37_MmapDecode_ShouldUse);
    // Pipeline: AdaptiveQualityScaler
    RUN_TEST(Test_S37_QualityScaler_LowLoad);
    RUN_TEST(Test_S37_QualityScaler_HighCPU);
    RUN_TEST(Test_S37_QualityScaler_Battery);
    RUN_TEST(Test_S37_QualityTier_Names);
    // Pipeline: BatchProcessor
    RUN_TEST(Test_S37_BatchProcessor_JobPriority);
    RUN_TEST(Test_S37_BatchProcessor_JobStatus);
    RUN_TEST(Test_S37_BatchProcessor_ThumbnailJob);
    RUN_TEST(Test_S37_BatchProcessor_BatchRequest);
    // Core: VersionSynchronizer
    RUN_TEST(Test_S37_VersionSync_Validate);
    RUN_TEST(Test_S37_VersionSync_PackedVersion);
    RUN_TEST(Test_S37_VersionSync_Audit);
    RUN_TEST(Test_S37_VersionSync_ComponentName);
    // Core: DecodeLatencyHistogram
    RUN_TEST(Test_S37_LatencyHistogram_Record);
    RUN_TEST(Test_S37_LatencyHistogram_Percentiles);
    RUN_TEST(Test_S37_LatencyHistogram_Reset);
    // Core: HealthScoreAggregator
    RUN_TEST(Test_S37_HealthScore_Assess);
    RUN_TEST(Test_S37_HealthScore_ClassifyScore);
    RUN_TEST(Test_S37_HealthScore_LevelName);
    // Core: FormatFallbackEngine

    // Sprint 394: New Feature Tests
    std::wcout << std::endl;
    std::wcout << L"Sprint 394: New Feature Tests..." << std::endl;
    // ColdStartOptimizer
    RUN_TEST(Test_S394_ColdStart_Instance);
    RUN_TEST(Test_S394_ColdStart_DefaultCodecs);
    RUN_TEST(Test_S394_ColdStart_Stats);
    // DecodeCancellationEngine
    RUN_TEST(Test_S394_Cancel_CreateToken);
    RUN_TEST(Test_S394_Cancel_CancelFile);
    RUN_TEST(Test_S394_Cancel_CancelAll);
    // RequestDeduplicator
    RUN_TEST(Test_S394_Dedup_SingleRequest);
    RUN_TEST(Test_S394_Dedup_Stats);
    // GPUPowerStateManager
    RUN_TEST(Test_S394_GPUPower_Instance);
    RUN_TEST(Test_S394_GPUPower_RouteWork);
    RUN_TEST(Test_S394_GPUPower_RouteTarget);
    // GPUWorkgroupOptimizer
    RUN_TEST(Test_S394_Workgroup_Optimize2D);
    RUN_TEST(Test_S394_Workgroup_Optimize1D);
    RUN_TEST(Test_S394_Workgroup_Stats);
    // CopyOnWriteBufferPool
    RUN_TEST(Test_S394_COW_WriteAndRead);
    RUN_TEST(Test_S394_COW_ShareAndCOW);
    RUN_TEST(Test_S394_COW_Pool);
    // MemoryMappedThumbnailAtlas
    RUN_TEST(Test_S394_Atlas_Lifecycle);
    RUN_TEST(Test_S394_Atlas_ReadUnmapped);
    // ThumbnailAestheticScorer

    // Sprint 395+: Enhancement Plan V15 — 50 New Feature Tests
    std::wcout << std::endl;
    std::wcout << L"Sprint 395+ Enhancement Tests:" << std::endl;
    // Core
    RUN_TEST(Test_S395_GlobalShortcutManager);
    RUN_TEST(Test_S395_FileChangeThrottler);
    RUN_TEST(Test_S395_SmartThumbnailPrioritizer);
    RUN_TEST(Test_S395_ExplorerIntegrationMonitor);
    RUN_TEST(Test_S395_FileConcurrencyGuard);
    RUN_TEST(Test_S395_ResourceThrottlePolicy);
    // Pipeline
    RUN_TEST(Test_S395_LazyDecodeInitializer);
    RUN_TEST(Test_S395_ChunkedDecodeEngine);
    RUN_TEST(Test_S395_DecodeProfilingHarness);
    RUN_TEST(Test_S395_ThumbnailMergeEngine);
    RUN_TEST(Test_S395_ExplorerQueryOptimizer);
    // GPU
    RUN_TEST(Test_S395_GPUDeviceSelector);
    RUN_TEST(Test_S395_GPUFeatureProbe);
    RUN_TEST(Test_S395_AdaptiveShaderSelection);
    RUN_TEST(Test_S395_GPUErrorRecovery);
    RUN_TEST(Test_S395_ComputeShaderProfiler);
    // Cache
    RUN_TEST(Test_S395_CachePartitionManager);
    RUN_TEST(Test_S395_CacheGarbageCollector);
    // Memory
    RUN_TEST(Test_S395_StackAllocator);
    RUN_TEST(Test_S395_MemoryWatermarkTracker);
    RUN_TEST(Test_S395_PoolAllocatorMetrics);
    RUN_TEST(Test_S395_ScopedMemoryBudget);
    RUN_TEST(Test_S395_ThreadLocalPoolAllocator);
    // AI
    RUN_TEST(Test_S395_FileTypePredictor);
    // Plugin
    // Utils
    RUN_TEST(Test_S395_PerformanceReportGenerator);
    RUN_TEST(Test_S395_SystemInfoCollector);
    RUN_TEST(Test_S395_RegistryRepairTool);
    RUN_TEST(Test_S395_LogAnalyzer);
    // Decoders
    RUN_TEST(Test_S395_WICFallbackDecoder);
    RUN_TEST(Test_S395_ArchivePeekDecoder);
    RUN_TEST(Test_S395_ContainerInspector);
    RUN_TEST(Test_S395_MetadataOnlyDecoder);
    RUN_TEST(Test_S395_EfficientResizeDecoder);
    RUN_TEST(Test_S395_MultiFormatBatchDecoder);
    RUN_TEST(Test_S395_CacheFriendlyDecoder);

    // Sprint 396 tests
    std::wcout << std::endl;
    std::wcout << L"Sprint 396 Tests..." << std::endl;
    // Core
    RUN_TEST(Test_S396_WindowsNotificationManager);
    RUN_TEST(Test_S396_FileSystemJournalReader);
    RUN_TEST(Test_S396_ProcessIsolationBroker);
    RUN_TEST(Test_S396_IncrementalDecodeEngine);
    RUN_TEST(Test_S396_ExplorerNavigationMonitor);
    RUN_TEST(Test_S396_DecoderPoolManager);
    RUN_TEST(Test_S396_FileTypeAssociationBroker);
    // Pipeline
    RUN_TEST(Test_S396_RateLimitedDecodeQueue);
    RUN_TEST(Test_S396_PipelineFusionOptimizer);
    RUN_TEST(Test_S396_ConcurrentDecodeBarrier);
    RUN_TEST(Test_S396_FormatSpecificPipeline);
    RUN_TEST(Test_S396_ThumbnailInvalidationTracker);
    RUN_TEST(Test_S396_PipelineTelemetrySink);
    RUN_TEST(Test_S396_DecodeWorkDistributor);
    // GPU
    RUN_TEST(Test_S396_GPUVendorCapabilityMap);
    RUN_TEST(Test_S396_GPUBatchSubmitter);
    RUN_TEST(Test_S396_GPUFenceOrchestrator);
    RUN_TEST(Test_S396_ShaderPermutationManager);
    RUN_TEST(Test_S396_GPUThermalMonitor);
    RUN_TEST(Test_S396_GPUResourceLeakDetector);
    // Cache
    RUN_TEST(Test_S396_CacheReplicationEngine);
    // Memory
    RUN_TEST(Test_S396_MemoryCompactionEngine);
    RUN_TEST(Test_S396_AllocatorBenchmark);
    RUN_TEST(Test_S396_WorkingSetOptimizer);
    // AI
    // Plugin
    RUN_TEST(Test_S396_PluginStatePersistence);
    // Decoders
    RUN_TEST(Test_S396_ClipboardImageDecoder);
    RUN_TEST(Test_S396_ScreenshotAnalyzer);
    RUN_TEST(Test_S396_StreamingVideoDecoder);
    RUN_TEST(Test_S396_CompoundDocumentDecoder);
    // Utils
    RUN_TEST(Test_S396_WindowsEventLogWriter);
    RUN_TEST(Test_S396_BuildArtifactValidator);

    // Sprint 397-399: Consolidated Initialization Tests
    std::wcout << std::endl;
    std::wcout << L"Sprint 397-399: Consolidated Init Tests..." << std::endl;

    // Sprint 399: Domain-Specific Tests
    std::wcout << std::endl;
    std::wcout << L"Sprint 399: Domain Tests..." << std::endl;
    RUN_TEST(Test_S399_FileSizeEstimator);
    RUN_TEST(Test_S399_PipelineLoadShedder);
    RUN_TEST(Test_S399_GPUTextureMipChain);
    RUN_TEST(Test_S399_GPUOccupancyCalculator);
    RUN_TEST(Test_S399_ComputeDispatchOptimizer);
    RUN_TEST(Test_S399_DiagnosticBundleCollector);
    RUN_TEST(Test_S399_RegressionTestRunner);
    std::wcout << std::endl;

    // Sprint 400: C++20 Refactoring Tests
    std::wcout << L"Sprint 400: C++20 Refactoring Tests..." << std::endl;
    RUN_TEST(Test_Concepts_DecoderConcept_Exists);
    RUN_TEST(Test_Expected_ErrorCategory_Values);
    RUN_TEST(Test_Expected_EngineError_Make);
    RUN_TEST(Test_Expected_HResult_Conversion);
    RUN_TEST(Test_Expected_IsRetryable);
    RUN_TEST(Test_Expected_DecodeResult_Ok);
    RUN_TEST(Test_Expected_DecodeResult_Error);
    RUN_TEST(Test_Expected_VoidResult_Success);
    RUN_TEST(Test_Expected_FromHResult);
    RUN_TEST(Test_FormatRegistry_NotEmpty);
    RUN_TEST(Test_FormatRegistry_LookupJPEG);
    RUN_TEST(Test_FormatRegistry_LookupPNG);
    RUN_TEST(Test_FormatRegistry_LookupZIP);
    RUN_TEST(Test_FormatRegistry_CategoryName);
    RUN_TEST(Test_FormatRegistry_UnknownExt);
    RUN_TEST(Test_FormatRegistry_ShellRegistered);
    RUN_TEST(Test_FormatRegistry_WebP);
    RUN_TEST(Test_FormatRegistry_EntryAccess);
    std::wcout << std::endl;

    // Sprint 261-270 — Plugin Marketplace v2 Tests
    std::wcout << L"Plugin Marketplace v2 Tests (Sprint 261-270):" << std::endl;
    RUN_TEST(TestPluginPackageManifest_DefaultCapabilities);
    std::wcout << std::endl;

    // Sprint 271-280 — Observability v2 Tests
    std::wcout << L"Observability v2 Tests (Sprint 271-280):" << std::endl;
    RUN_TEST(TestLatencyBudgetManager_RegisterFormat);
    RUN_TEST(TestLatencyBudgetManager_RecordSample);
    RUN_TEST(TestFeatureFlagManager_GetBoolDefault);
    RUN_TEST(TestFeatureFlagManager_GetIntDefault);
    RUN_TEST(TestUsageStats_RecordEvent);
    RUN_TEST(TestCrashReporter_InitialNotInstalled);
    RUN_TEST(TestCrashReporter_Install);
    std::wcout << std::endl;

    // Sprint 281-290 — Format Expansion III Tests
    std::wcout << L"Format Expansion III Tests (Sprint 281-290):" << std::endl;
    RUN_TEST(TestAVIFSequenceDecoder_SupportsHardware);
    RUN_TEST(TestHEIFBurstDecoder_LooksLikeHEIF_RejectsGarbage);
    RUN_TEST(TestJXLAnimationDecoder_LooksLikeJXL_RejectsEmpty);
    RUN_TEST(TestPSDLayerDecoder_IsPSD_RejectsGarbage);
    RUN_TEST(TestSVGRasterizer_LooksLikeSVG_RejectsEmpty);
    RUN_TEST(TestWebPAnimationDecoder_LooksLikeAnimatedWebP_RejectsGarbage);
    std::wcout << std::endl;

    // Sprint 291-300 — Advanced GPU Compute v2 Tests
    std::wcout << L"Advanced GPU Compute v2 Tests (Sprint 291-300):" << std::endl;
    RUN_TEST(TestVulkanComputeAccelerator_InitialStatus);
    RUN_TEST(TestD3D11DPIAdapter_InitialNotInitialized);
    RUN_TEST(TestThreadPoolV2_InstanceAccessible);
    RUN_TEST(TestThreadPoolV2_Configure);
    RUN_TEST(TestSIMDImageProcessor_DetectCPUFeatures);
    RUN_TEST(TestBatchDecodeScheduler_DefaultConstruct);
    std::wcout << std::endl;

    // Sprint 301-310 — Enterprise Policy v2 Tests
    std::wcout << L"Enterprise Policy v2 Tests (Sprint 301-310):" << std::endl;
    RUN_TEST(TestEnterprisePolicyEngineV2_InstanceAccessible);
    RUN_TEST(TestAuditLogger_InstanceAccessible);
    RUN_TEST(TestFIPSComplianceMode_WindowsFIPSQuery);
    RUN_TEST(TestFIPSComplianceMode_InstanceAccessible);
    RUN_TEST(TestPrivilegeElevationGuard_DefaultConstruct);
    RUN_TEST(TestSandboxEscapeGuard_DefaultConstruct);
    std::wcout << std::endl;

    // Sprint 311-320 — Storage & Caching v3 Tests
    std::wcout << L"Storage & Caching v3 Tests (Sprint 311-320):" << std::endl;
    RUN_TEST(TestFeatureCompatMatrix_InstanceAccessible);
    std::wcout << std::endl;

    // Sprint 321-330 — Cross-Platform Foundation Tests
    std::wcout << L"Cross-Platform Foundation Tests (Sprint 321-330):" << std::endl;
    RUN_TEST(TestColorBlindnessFilter_BuildMatrix_None);
    RUN_TEST(TestFeedbackItem_DefaultCategory);
    RUN_TEST(TestMonitorEventData_DefaultState);
    std::wcout << std::endl;

    // Sprint 331-340 — Performance Profiling v2 Tests
    std::wcout << L"Performance Profiling v2 Tests (Sprint 331-340):" << std::endl;
    RUN_TEST(TestLayoutMetrics_DefaultValues);
    RUN_TEST(TestSandboxPolicy_DefaultLevel);
    RUN_TEST(TestValidationError_OkIsZero);
    RUN_TEST(TestCodecCapability_NoneIsZero);
    RUN_TEST(TestModuleSecurityFlags_DefaultConstruct);
    std::wcout << std::endl;

    // Sprint 341-350 — Security & Audit v3 Tests
    std::wcout << L"Security & Audit v3 Tests (Sprint 341-350):" << std::endl;
    RUN_TEST(TestLicenseInfo_DefaultTier);
    RUN_TEST(TestLocaleDirection_LTRIsZero);
    RUN_TEST(TestStoreCheckSeverity_InfoIsZero);
    RUN_TEST(TestPinValidationResult_OkIsZero);
    RUN_TEST(TestColorBlindType_NoneIsZero);
    std::wcout << std::endl;

    // Sprint 351-360 — AI Inference Pipeline v2 Tests
    std::wcout << L"AI Inference Pipeline v2 Tests (Sprint 351-360):" << std::endl;
    RUN_TEST(TestUpscaleBackend_AutoIsZero);
    RUN_TEST(TestContentCategory_UnknownIsZero);

    // Sprint 361-370: Advanced Scheduling & Concurrency v2 (v22.4.0 Sirius-U)
    RUN_TEST(TestLockFreeMPMCQueue_PushPop);
    RUN_TEST(TestCPUAffinityRouter_PolicyAuto);
    RUN_TEST(TestRealtimePriorityEngine_EnqueueDequeue);
    RUN_TEST(TestHazardPointerReclaimer_AcquireRelease);
    RUN_TEST(TestAdaptiveConcurrencyLimiter_AIMD);
    RUN_TEST(TestCooperativeTaskScheduler_RunOnce);
    RUN_TEST(TestThreadLocalContextPool_AcquireRelease);

    // Sprint 371-380: Format Expansion IV (v22.5.0 Sirius-V)
    RUN_TEST(TestFLIFDecoder_ProbeSignature);
    RUN_TEST(TestQOIRDecoder_ProbeSignature);
    RUN_TEST(TestJNGDecoder_ProbeSignature);
    RUN_TEST(TestJBIG2Decoder_ProbeSignature);
    RUN_TEST(TestTIFFMultiFrameV2_BigTIFFSupport);
    RUN_TEST(TestILBMDecoder_ProbeIFF);
    RUN_TEST(TestSunRasterDecoder_ProbeSignature);
    RUN_TEST(TestJPEGXTDecoder_ProbeSOI);

    // Sprint 381-390: Windows Shell Integration v2 (v22.6.0 Sirius-W)
    RUN_TEST(TestNamespaceWalkEngine_WalkEmpty);
    RUN_TEST(TestExplorerColumnProviderV2_RegisterColumn);
    RUN_TEST(TestShellContextMenuV2_Execute);
    RUN_TEST(TestSearchIndexBridge_Properties);
    RUN_TEST(TestDragDropPreviewEngine_Generate);

    // Sprint 391-400: DevOps & Quality Engineering v2 (v22.7.0 Sirius-X)
    RUN_TEST(TestMutationTestingEngine_KillRate);
    RUN_TEST(TestPropertyBaseTestEngine_CheckPasses);
    RUN_TEST(TestReproducibleBuildVerifierV2_Reproducible);
    RUN_TEST(TestRegressionFingerprintEngine_Clean);
    RUN_TEST(TestCycloneDXSBOMGenerator_JSON);
    RUN_TEST(TestBuildTimingAnalytics_Summarize);
    RUN_TEST(TestArtifactIntegrityMonitor_NoAlerts);
    RUN_TEST(TestCIEnvironmentValidator_Required);

    // Sprint 401-410: Reactive Pipeline Architecture (v23.0.0 Vega)
    RUN_TEST(TestThumbnailEventStore_AppendReplay);
    RUN_TEST(TestCQRSThumbnailPipeline_DispatchQuery);
    RUN_TEST(TestReactiveStreamEngine_EmitSubscribe);
    RUN_TEST(TestThumbnailSagaOrchestrator_StartQuery);
    RUN_TEST(TestSnapshotStoreEngine_SaveLoad);
    RUN_TEST(TestDomainEventBus_PublishSubscribe);
    RUN_TEST(TestReactiveAPIGateway_StartStop);

    // Sprint 411-420: GPU Acceleration v3 (v23.1.0 Vega-R)
    RUN_TEST(TestCUDATextureDecoder_NotAvailable);
    RUN_TEST(TestHIPComputeBackend_NotAvailable);
    RUN_TEST(TestMultiGPULoadBalancerV3_SelectDevice);
    RUN_TEST(TestGPUTextureAtlasBuilder_PackSingle);
    RUN_TEST(TestGPUResourceAliasingManager_Register);
    RUN_TEST(TestAsyncDMACopyEngine_SubmitFlush);
    RUN_TEST(TestGPUMemoryDefragmenterV2_Plan);
    RUN_TEST(TestGPUThumbnailAtlasManager_InsertLookup);

    // Sprint 421-430: Plugin Ecosystem v3 (v23.2.0 Vega-S)

    // Sprint 431-440: Memory Optimization v3 (v23.3.0 Vega-T)
    RUN_TEST(TestPageFileArenaAllocator_AllocReset);
    RUN_TEST(TestHugeTLBPagePool_AcquirePageSize);
    RUN_TEST(TestMemoryMappedBTree_InsertLookup);
    RUN_TEST(TestNVMeMemoryTier_NotAvailable);
    RUN_TEST(TestECCErrorDetector_QueryStatus);
    RUN_TEST(TestPressureForecaster_FeedPredict);
    RUN_TEST(TestSharedMemoryRegionManager_CreateOpen);

    // Sprint 441-450: Smart Cache v4 (v23.4.0 Vega-U)
    RUN_TEST(TestAIEvictionPolicyEngine_Score);
    RUN_TEST(TestCacheEncryptionLayer_EncryptDecrypt);

    // Sprint 451-460: CLI & Automation v2 (v23.5.0 Vega-V)

    // Sprint 471-480 — Format Expansion V (v23.7.0 "Vega-X")
    std::wcout << L"Format Expansion V (v23.7.0 Vega-X)..." << std::endl;
    RUN_TEST(TestICNSDecoder_Extensions);
    RUN_TEST(TestICNSDecoder_Create);
    RUN_TEST(TestICNSDecoder_InvalidFile);
    RUN_TEST(TestCURDecoder_Extensions);
    RUN_TEST(TestCURDecoder_Create);
    RUN_TEST(TestCURDecoder_InvalidFile);
    RUN_TEST(TestANIMDecoder_Extensions);
    RUN_TEST(TestANIMDecoder_Create);
    RUN_TEST(TestANIMDecoder_InvalidFile);
    RUN_TEST(TestMNGDecoder_Extensions);
    RUN_TEST(TestMNGDecoder_Create);
    RUN_TEST(TestMNGDecoder_InvalidFile);
    RUN_TEST(TestHRZDecoder_Extensions);
    RUN_TEST(TestHRZDecoder_Dimensions);
    RUN_TEST(TestHRZDecoder_InvalidFile);
    RUN_TEST(TestPIXARDecoder_Extensions);
    RUN_TEST(TestPIXARDecoder_Create);
    RUN_TEST(TestPIXARDecoder_InvalidFile);
    RUN_TEST(TestJPEG2000TileV2_Extensions);
    RUN_TEST(TestJPEG2000TileV2_Create);
    RUN_TEST(TestJPEG2000TileV2_InvalidFile);
    RUN_TEST(TestFLIFDecoderV2_Extensions);
    RUN_TEST(TestFLIFDecoderV2_Create);
    RUN_TEST(TestFLIFDecoderV2_InvalidFile);
    // Sprint 481-490 — AI-Native Thumbnailing v2 (v24.0.0 "Altair")

    // Sprint 491-500 — Cross-Process Architecture (v24.1.0 "Altair-R")
    RUN_TEST(TestOutOfProcServer_Defaults);
    RUN_TEST(TestOutOfProcServer_Constants);
    RUN_TEST(TestOutOfProcServer_GetModeName);
    RUN_TEST(TestOutOfProcServer_GetStateName);
    RUN_TEST(TestOutOfProcServer_SetMode);
    RUN_TEST(TestOutOfProcServer_StartStop);
    RUN_TEST(TestOutOfProcServer_ProcessRequestNull);
    RUN_TEST(TestOutOfProcServer_ProcessRequestValid);
    RUN_TEST(TestOutOfProcServer_SetModeHybrid);

    RUN_TEST(TestCrossProcCacheProxy_Defaults);
    RUN_TEST(TestCrossProcCacheProxy_Constants);
    RUN_TEST(TestCrossProcCacheProxy_GetModeName);
    RUN_TEST(TestCrossProcCacheProxy_GetStateName);
    RUN_TEST(TestCrossProcCacheProxy_ConnectDisconnect);
    RUN_TEST(TestCrossProcCacheProxy_PutGetCycle);
    RUN_TEST(TestCrossProcCacheProxy_PutNullFail);
    RUN_TEST(TestCrossProcCacheProxy_InvalidateKey);
    RUN_TEST(TestCrossProcCacheProxy_PutWhileDisconnected);

    RUN_TEST(TestProcessPoolManager_Defaults);
    RUN_TEST(TestProcessPoolManager_Constants);
    RUN_TEST(TestProcessPoolManager_GetWorkerStateName);
    RUN_TEST(TestProcessPoolManager_GetPriorityName);
    RUN_TEST(TestProcessPoolManager_StartStop);
    RUN_TEST(TestProcessPoolManager_Submit);
    RUN_TEST(TestProcessPoolManager_SubmitEmpty);
    RUN_TEST(TestProcessPoolManager_Cancel);
    RUN_TEST(TestProcessPoolManager_SetPriority);

    RUN_TEST(TestIsolationPolicy_DefaultLevel);
    RUN_TEST(TestIsolationPolicy_Constants);
    RUN_TEST(TestIsolationPolicy_GetLevelName);
    RUN_TEST(TestIsolationPolicy_GetActionName);
    RUN_TEST(TestIsolationPolicy_EvaluateSafe);
    RUN_TEST(TestIsolationPolicy_EvaluateArchive);
    RUN_TEST(TestIsolationPolicy_SetDefaultLevel);
    RUN_TEST(TestIsolationPolicy_AddRule);
    RUN_TEST(TestIsolationPolicy_ShouldSandbox);

    RUN_TEST(TestCrossProcEventBus_Defaults);
    RUN_TEST(TestCrossProcEventBus_Constants);
    RUN_TEST(TestCrossProcEventBus_GetModeName);
    RUN_TEST(TestCrossProcEventBus_GetPriorityName);
    RUN_TEST(TestCrossProcEventBus_StartStop);
    RUN_TEST(TestCrossProcEventBus_SubscribeUnsubscribe);
    RUN_TEST(TestCrossProcEventBus_Publish);
    RUN_TEST(TestCrossProcEventBus_GetSubscriberCount);
    RUN_TEST(TestCrossProcEventBus_GetQueueDepth);

    RUN_TEST(TestSharedState_DefaultStrategy);
    RUN_TEST(TestSharedState_Constants);
    RUN_TEST(TestSharedState_GetStrategyName);
    RUN_TEST(TestSharedState_GetReasonName);
    RUN_TEST(TestSharedState_SetGet);
    RUN_TEST(TestSharedState_Remove);
    RUN_TEST(TestSharedState_GetVersion);
    RUN_TEST(TestSharedState_Synchronize);
    RUN_TEST(TestSharedState_EntryCount);

    RUN_TEST(TestRemoteRenderProxy_Defaults);
    RUN_TEST(TestRemoteRenderProxy_Constants);
    RUN_TEST(TestRemoteRenderProxy_GetTransportName);
    RUN_TEST(TestRemoteRenderProxy_GetRenderStateName);
    RUN_TEST(TestRemoteRenderProxy_ConnectDisconnect);
    RUN_TEST(TestRemoteRenderProxy_RenderFileEmpty);
    RUN_TEST(TestRemoteRenderProxy_RenderWhileDisconnected);
    RUN_TEST(TestRemoteRenderProxy_SetTransport);
    RUN_TEST(TestRemoteRenderProxy_GetLastState);

    std::wcout << std::endl;

    // Sprint 1061-1070: Intelligent Workflow Automation (v31.3.0 "Achernar-T")
    std::wcout << L"Intelligent Workflow Automation Tests..." << std::endl;

    RUN_TEST(TestPregen_Instance);
    RUN_TEST(TestPregen_Initialize);
    RUN_TEST(TestPregen_InitWithStrategy);
    RUN_TEST(TestPregen_RecordNavigation);
    RUN_TEST(TestPregen_Predict);
    RUN_TEST(TestPregen_PredictEmpty);
    RUN_TEST(TestPregen_Stats);
    RUN_TEST(TestPregen_Shutdown);
    RUN_TEST(TestPregen_PredictZeroResults);

    RUN_TEST(TestContentCat_Instance);
    RUN_TEST(TestContentCat_Initialize);
    RUN_TEST(TestContentCat_Categorize);
    RUN_TEST(TestContentCat_CategorizeUninitialized);
    RUN_TEST(TestContentCat_ToString);
    RUN_TEST(TestContentCat_Stats);
    RUN_TEST(TestContentCat_Shutdown);
    RUN_TEST(TestContentCat_MultipleCategories);

    RUN_TEST(TestQualPred_Instance);
    RUN_TEST(TestQualPred_Initialize);
    RUN_TEST(TestQualPred_PredictGood);
    RUN_TEST(TestQualPred_PredictUnusable);
    RUN_TEST(TestQualPred_PredictUninitialized);
    RUN_TEST(TestQualPred_ToString);
    RUN_TEST(TestQualPred_Stats);
    RUN_TEST(TestQualPred_Shutdown);

    RUN_TEST(TestSmartBatch_Instance);
    RUN_TEST(TestSmartBatch_Initialize);
    RUN_TEST(TestSmartBatch_SubmitAndStart);
    RUN_TEST(TestSmartBatch_PauseResume);
    RUN_TEST(TestSmartBatch_Cancel);
    RUN_TEST(TestSmartBatch_EmptySubmit);
    RUN_TEST(TestSmartBatch_ToString);
    RUN_TEST(TestSmartBatch_Stats);
    RUN_TEST(TestSmartBatch_Progress);

    RUN_TEST(TestWorkflow_Instance);
    RUN_TEST(TestWorkflow_Initialize);
    RUN_TEST(TestWorkflow_AddRule);
    RUN_TEST(TestWorkflow_RemoveRule);
    RUN_TEST(TestWorkflow_EnableDisable);
    RUN_TEST(TestWorkflow_RemoveInvalid);
    RUN_TEST(TestWorkflow_Stats);
    RUN_TEST(TestWorkflow_Shutdown);
    RUN_TEST(TestWorkflow_AddUninitialized);

    RUN_TEST(TestUserBehavior_Instance);
    RUN_TEST(TestUserBehavior_Initialize);
    RUN_TEST(TestUserBehavior_RecordEvent);
    RUN_TEST(TestUserBehavior_RecordUninitialized);
    RUN_TEST(TestUserBehavior_GetTopPatterns);
    RUN_TEST(TestUserBehavior_EmptyPatterns);
    RUN_TEST(TestUserBehavior_Stats);
    RUN_TEST(TestUserBehavior_Shutdown);

    RUN_TEST(TestAdaptPipe_Instance);
    RUN_TEST(TestAdaptPipe_Initialize);
    RUN_TEST(TestAdaptPipe_InitCustomConfig);
    RUN_TEST(TestAdaptPipe_RecordMetric);
    RUN_TEST(TestAdaptPipe_OptimizeReduceConcurrency);
    RUN_TEST(TestAdaptPipe_OptimizeIncreaseConcurrency);
    RUN_TEST(TestAdaptPipe_Stats);
    RUN_TEST(TestAdaptPipe_Shutdown);
    RUN_TEST(TestAdaptPipe_OptimizeUninitialized);

    RUN_TEST(TestPrefetch_Instance);
    RUN_TEST(TestPrefetch_Initialize);
    RUN_TEST(TestPrefetch_Schedule);
    RUN_TEST(TestPrefetch_ScheduleBatch);
    RUN_TEST(TestPrefetch_ScheduleUninitialized);
    RUN_TEST(TestPrefetch_Flush);
    RUN_TEST(TestPrefetch_Stats);
    RUN_TEST(TestPrefetch_Eviction);
    RUN_TEST(TestPrefetch_Shutdown);

    // Sprint 1071-1080 — Contextual Intelligence & Self-Healing Tests (v31.4.0)
    std::wcout << L"\nContextual Intelligence & Self-Healing Tests..." << std::endl;
    RUN_TEST(TestCtxRender_Initialize);
    RUN_TEST(TestCtxRender_EvaluateDefault);
    RUN_TEST(TestCtxRender_HighDPI);
    RUN_TEST(TestCtxRender_BatterySaver);
    RUN_TEST(TestCtxRender_LowMemory);
    RUN_TEST(TestCtxRender_ContextSwitch);
    RUN_TEST(TestCtxRender_DPIScale);
    RUN_TEST(TestCtxRender_Background);
    RUN_TEST(TestCtxRender_Shutdown);
    RUN_TEST(TestCompositor_Initialize);
    RUN_TEST(TestCompositor_SinglePage);
    RUN_TEST(TestCompositor_MultiPage);
    RUN_TEST(TestCompositor_GridLayout);
    RUN_TEST(TestCompositor_LargeGrid);
    RUN_TEST(TestCompositor_Stats);
    RUN_TEST(TestCompositor_Shutdown);
    RUN_TEST(TestFmtComplexity_Initialize);
    RUN_TEST(TestFmtComplexity_Trivial);
    RUN_TEST(TestFmtComplexity_Extreme);
    RUN_TEST(TestFmtComplexity_Moderate);
    RUN_TEST(TestFmtComplexity_Complex);
    RUN_TEST(TestFmtComplexity_StatsAverage);
    RUN_TEST(TestFmtComplexity_Simple);
    RUN_TEST(TestFmtComplexity_Shutdown);
    RUN_TEST(TestFaultTolerant_Initialize);
    RUN_TEST(TestFaultTolerant_RecordSuccess);
    RUN_TEST(TestFaultTolerant_RecordFallback);
    RUN_TEST(TestFaultTolerant_Reliability);
    RUN_TEST(TestFaultTolerant_FallbackRecommendation);
    RUN_TEST(TestFaultTolerant_OverallReliability);
    RUN_TEST(TestFaultTolerant_PermanentFailure);
    RUN_TEST(TestFaultTolerant_Shutdown);
    RUN_TEST(TestDiagTelemetry_Initialize);
    RUN_TEST(TestDiagTelemetry_Record);
    RUN_TEST(TestDiagTelemetry_FilterByCategory);
    RUN_TEST(TestDiagTelemetry_ErrorTracking);
    RUN_TEST(TestDiagTelemetry_RecentErrors);
    RUN_TEST(TestDiagTelemetry_RingBufferEviction);
    RUN_TEST(TestDiagTelemetry_SequenceIds);
    RUN_TEST(TestDiagTelemetry_Shutdown);
    RUN_TEST(TestFaultIsolator_Initialize);
    RUN_TEST(TestFaultIsolator_RecordFault);
    RUN_TEST(TestFaultIsolator_Quarantine);
    RUN_TEST(TestFaultIsolator_FatalImmediate);
    RUN_TEST(TestFaultIsolator_ReleaseFromQuarantine);
    RUN_TEST(TestFaultIsolator_PermanentDisable);
    RUN_TEST(TestFaultIsolator_SuccessResetsConsecutive);
    RUN_TEST(TestFaultIsolator_Shutdown);
    RUN_TEST(TestSmartRetry_Initialize);
    RUN_TEST(TestSmartRetry_BasicRetry);
    RUN_TEST(TestSmartRetry_ExponentialBackoff);
    RUN_TEST(TestSmartRetry_CorruptNotRetryable);
    RUN_TEST(TestSmartRetry_GPUFallback);
    RUN_TEST(TestSmartRetry_Exhausted);
    RUN_TEST(TestSmartRetry_CircuitBreaker);
    RUN_TEST(TestSmartRetry_ResetDecoder);
    RUN_TEST(TestSmartRetry_Shutdown);
    RUN_TEST(TestPipeHealth_Initialize);
    RUN_TEST(TestPipeHealth_RecordSample);
    RUN_TEST(TestPipeHealth_HealthySnapshot);
    RUN_TEST(TestPipeHealth_HighErrorRate);
    RUN_TEST(TestPipeHealth_HighLatencyAlert);
    RUN_TEST(TestPipeHealth_Alerts);
    RUN_TEST(TestPipeHealth_PeakAlertLevel);
    RUN_TEST(TestPipeHealth_Shutdown);

    // Sprint 1081-1090 — Format Routing & Enhanced Accessibility (v31.5.0)
    std::wcout << L"\nFormat Routing & Enhanced Accessibility Tests..." << std::endl;
    RUN_TEST(TestColorProfile_Initialize);
    RUN_TEST(TestColorProfile_MatchSRGB);
    RUN_TEST(TestColorProfile_HDRtoSDR);
    RUN_TEST(TestColorProfile_HDRtoHDR);
    RUN_TEST(TestColorProfile_GamutMapping);
    RUN_TEST(TestColorProfile_Stats);
    RUN_TEST(TestColorProfile_Shutdown);
    RUN_TEST(TestAccessibility_Initialize);
    RUN_TEST(TestAccessibility_Standard);
    RUN_TEST(TestAccessibility_HighContrast);
    RUN_TEST(TestAccessibility_ColorBlind);
    RUN_TEST(TestAccessibility_AltText);
    RUN_TEST(TestAccessibility_Settings);
    RUN_TEST(TestAccessibility_Shutdown);
    RUN_TEST(TestFileRouter_Initialize);
    RUN_TEST(TestFileRouter_RegisterAndRoute);
    RUN_TEST(TestFileRouter_UnknownFormat);
    RUN_TEST(TestFileRouter_RecordSuccess);
    RUN_TEST(TestFileRouter_RecordFailure);
    RUN_TEST(TestFileRouter_MultipleRoutes);
    RUN_TEST(TestFileRouter_Shutdown);
    RUN_TEST(TestDecVersion_Initialize);
    RUN_TEST(TestDecVersion_Register);
    RUN_TEST(TestDecVersion_CompatFull);
    RUN_TEST(TestDecVersion_CompatBackward);
    RUN_TEST(TestDecVersion_Deprecated);
    RUN_TEST(TestDecVersion_Unsupported);
    RUN_TEST(TestDecVersion_Shutdown);
    RUN_TEST(TestCrossMeta_Initialize);
    RUN_TEST(TestCrossMeta_JPEG);
    RUN_TEST(TestCrossMeta_PNG);
    RUN_TEST(TestCrossMeta_PDF);
    RUN_TEST(TestCrossMeta_RAW);
    RUN_TEST(TestCrossMeta_Video);
    RUN_TEST(TestCrossMeta_Stats);
    RUN_TEST(TestSDC_Initialize);
    RUN_TEST(TestSDC_StartStream);
    RUN_TEST(TestSDC_PartialRender);
    RUN_TEST(TestSDC_CompleteStream);
    RUN_TEST(TestSDC_Stats);
    RUN_TEST(TestSDC_ProgressTracking);
    RUN_TEST(TestSDC_Shutdown);
    RUN_TEST(TestRPP_Initialize);
    RUN_TEST(TestRPP_RecordStage);
    RUN_TEST(TestRPP_Snapshot);
    RUN_TEST(TestRPP_Bottleneck);
    RUN_TEST(TestRPP_Stats);
    RUN_TEST(TestRPP_AllStages);
    RUN_TEST(TestRPP_Shutdown);

    // v31.9.0 — Autonomous Workflow Orchestrator
    RUN_TEST(TestAWO_Initialize);
    RUN_TEST(TestAWO_Enqueue);
    RUN_TEST(TestAWO_Dispatch);
    RUN_TEST(TestAWO_Complete);
    RUN_TEST(TestAWO_SetPolicy);
    RUN_TEST(TestAWO_ConcurrencyLimit);
    RUN_TEST(TestAWO_Reset);
    RUN_TEST(TestAWO_MultiBatch);
    RUN_TEST(TestAWO_PolicyGain);
    // v31.9.0 — Shell Intelligence Adapter
    RUN_TEST(TestSIA_Initialize);
    RUN_TEST(TestSIA_QueryHint);
    RUN_TEST(TestSIA_InjectPregen);
    RUN_TEST(TestSIA_NotifyRender);
    RUN_TEST(TestSIA_MacOS);
    RUN_TEST(TestSIA_Linux);
    RUN_TEST(TestSIA_StatAccumulate);
    RUN_TEST(TestSIA_MultiPlatform);
    RUN_TEST(TestSIA_LargeRender);
    // v31.9.0 — Thumbnail Relevance Ranker
    RUN_TEST(TestTRR_Initialize);
    RUN_TEST(TestTRR_Rank);
    RUN_TEST(TestTRR_RecordAccess);
    RUN_TEST(TestTRR_Weights);
    RUN_TEST(TestTRR_EmptyInput);
    RUN_TEST(TestTRR_Reset);
    RUN_TEST(TestTRR_ScoreRange);
    RUN_TEST(TestTRR_LargeBatch);
    RUN_TEST(TestTRR_Stats);
    // v31.9.0 — Adaptive Shell Integration Engine
    RUN_TEST(TestASIE_Initialize);
    RUN_TEST(TestASIE_Probe);
    RUN_TEST(TestASIE_Compatibility);
    RUN_TEST(TestASIE_ForceMode);
    RUN_TEST(TestASIE_Stats);
    RUN_TEST(TestASIE_ForceMacOS);
    RUN_TEST(TestASIE_ForceLinux);
    RUN_TEST(TestASIE_MultiProbe);
    RUN_TEST(TestASIE_Fallback);
    // v31.9.0 — Shell Extension Lifecycle Manager
    RUN_TEST(TestSELM_Instance);
    RUN_TEST(TestSELM_Register);
    RUN_TEST(TestSELM_Unregister);
    RUN_TEST(TestSELM_GetState);
    RUN_TEST(TestSELM_Heartbeat);
    RUN_TEST(TestSELM_Recover);
    RUN_TEST(TestSELM_Suspend);
    RUN_TEST(TestSELM_MultipleExtensions);
    RUN_TEST(TestSELM_Stats);
    // v31.9.0 — Autotuning Pipeline Engine
    RUN_TEST(TestAPE_Initialize);
    RUN_TEST(TestAPE_Observe);
    RUN_TEST(TestAPE_Step);
    RUN_TEST(TestAPE_BestParams);
    RUN_TEST(TestAPE_Convergence);
    RUN_TEST(TestAPE_Reset);
    RUN_TEST(TestAPE_ThroughputTracking);
    RUN_TEST(TestAPE_LatencyTarget);
    RUN_TEST(TestAPE_MultiObserve);
    // v31.9.0 — Cross-Platform Build Validator
    RUN_TEST(TestCPBV_Instance);
    RUN_TEST(TestCPBV_Validate);
    RUN_TEST(TestCPBV_HasErrors);
    RUN_TEST(TestCPBV_Summary);
    RUN_TEST(TestCPBV_ReportResults);
    RUN_TEST(TestCPBV_Windows);
    RUN_TEST(TestCPBV_ValidationCount);
    RUN_TEST(TestCPBV_MultiValidate);
    RUN_TEST(TestCPBV_NoFatalErrors);
    // v32.0.0 — Post-Quantum Security & Zero-Trust
    RUN_TEST(TestPQCP_Init);
    RUN_TEST(TestPQCP_KeyGen_Kyber);
    RUN_TEST(TestPQCP_KeyGen_Dilithium);
    RUN_TEST(TestPQCP_Sign);
    RUN_TEST(TestPQCP_VerifyOk);
    RUN_TEST(TestPQCP_VerifyFail_Empty);
    RUN_TEST(TestPQCP_Stats_KeyGenCount);
    RUN_TEST(TestPQCP_Stats_SignCount);
    RUN_TEST(TestPQCP_Reset);
    RUN_TEST(TestZTAB_Instance);
    RUN_TEST(TestZTAB_Issue);
    RUN_TEST(TestZTAB_Validate_Valid);
    RUN_TEST(TestZTAB_Validate_Revoked);
    RUN_TEST(TestZTAB_Revoke);
    RUN_TEST(TestZTAB_RevokeMiss);
    RUN_TEST(TestZTAB_Stats_Issued);
    RUN_TEST(TestZTAB_Stats_Denied);
    RUN_TEST(TestZTAB_MultiToken);
    RUN_TEST(TestQRHE_Init);
    RUN_TEST(TestQRHE_HashSHA3);
    RUN_TEST(TestQRHE_HashBLAKE3);
    RUN_TEST(TestQRHE_HashK12);
    RUN_TEST(TestQRHE_Default);
    RUN_TEST(TestQRHE_ConstantTimeEq);
    RUN_TEST(TestQRHE_ConstantTimeNeq);
    RUN_TEST(TestQRHE_Stats);
    RUN_TEST(TestQRHE_Reset);
    RUN_TEST(TestPZTS_Instance);
    RUN_TEST(TestPZTS_DefaultPolicy);
    RUN_TEST(TestPZTS_SetPolicy);
    RUN_TEST(TestPZTS_Allow_WithToken);
    RUN_TEST(TestPZTS_Deny_NoToken);
    RUN_TEST(TestPZTS_Deny_EmptyPlugin);
    RUN_TEST(TestPZTS_Deny_Empty_Cap);
    RUN_TEST(TestPZTS_NotQuarantined);
    RUN_TEST(TestPZTS_Stats);
    RUN_TEST(TestBTV_Init);
    RUN_TEST(TestBTV_Verify_Valid);
    RUN_TEST(TestBTV_Verify_Empty);
    RUN_TEST(TestBTV_TamperDetect);
    RUN_TEST(TestBTV_TamperReason);
    RUN_TEST(TestBTV_SignerName);
    RUN_TEST(TestBTV_Stats_Ok);
    RUN_TEST(TestBTV_Stats_Fail);
    RUN_TEST(TestBTV_Reset);
    RUN_TEST(TestSCM_Instance);
    RUN_TEST(TestSCM_Init);
    RUN_TEST(TestSCM_SetAndGet);
    RUN_TEST(TestSCM_GetMissing);
    RUN_TEST(TestSCM_Backend_Platform);
    RUN_TEST(TestSCM_MultiKey);
    RUN_TEST(TestSCM_OverwriteKey);
    RUN_TEST(TestSCM_Stats_Writes);
    RUN_TEST(TestSCM_Stats_Reads);
    RUN_TEST(TestTME_Init);
    RUN_TEST(TestTME_Analyze_NotEmpty);
    RUN_TEST(TestTME_PipelineSafe);
    RUN_TEST(TestTME_Spoofing_Sim);
    RUN_TEST(TestTME_Spoofing_Unmitigated);
    RUN_TEST(TestTME_Spoofing_Severity);
    RUN_TEST(TestTME_Stats_AnalyzeCount);
    RUN_TEST(TestTME_Stats_Found);
    RUN_TEST(TestTME_Reset);
    RUN_TEST(TestSPA_Instance);
    RUN_TEST(TestSPA_Analyze_NonEmpty);
    RUN_TEST(TestSPA_Score_Range);
    RUN_TEST(TestSPA_PatchLevel);
    RUN_TEST(TestSPA_Schema);
    RUN_TEST(TestSPA_IsCompliant);
    RUN_TEST(TestSPA_Serialize);
    RUN_TEST(TestSPA_Stats);
    RUN_TEST(TestSPA_ScoreComponents);
    // v32.1.0 — Edge AI & Hardware-Accelerated Inference
    RUN_TEST(TestNPUAE_Init);
    RUN_TEST(TestNPUAE_Dispatch);
    RUN_TEST(TestNPUAE_Stats_Dispatched);
    RUN_TEST(TestNPUAE_CPUFallback);
    RUN_TEST(TestNPUAE_SetMode);
    RUN_TEST(TestNPUAE_Reset);
    RUN_TEST(TestNPUAE_AvailableCheck);
    RUN_TEST(TestNPUAE_EmptyWorkload);
    RUN_TEST(TestNPUAE_MultiDispatch);
    RUN_TEST(TestEAIE_Init);
    RUN_TEST(TestEAIE_CreateSession);
    RUN_TEST(TestEAIE_CreateSession_Empty);
    RUN_TEST(TestEAIE_RunInference);
    RUN_TEST(TestEAIE_RunInference_ErrorState);
    RUN_TEST(TestEAIE_MemMapped);
    RUN_TEST(TestEAIE_Destroy);
    RUN_TEST(TestEAIE_Stats);
    RUN_TEST(TestEAIE_Reset);
    RUN_TEST(TestHCN_Init);
    RUN_TEST(TestHCN_Negotiate_Embedding);
    RUN_TEST(TestHCN_Negotiate_Render);
    RUN_TEST(TestHCN_Negotiate_Default);
    RUN_TEST(TestHCN_PrefersNPU);
    RUN_TEST(TestHCN_Score_Avail);
    RUN_TEST(TestHCN_Stats_Total);
    RUN_TEST(TestHCN_Stats_NPUCount);
    RUN_TEST(TestHCN_Reset);
    RUN_TEST(TestXDNA_Init);
    RUN_TEST(TestXDNA_DeviceName);
    RUN_TEST(TestXDNA_TOPS);
    RUN_TEST(TestXDNA_ExecuteKernel);
    RUN_TEST(TestXDNA_MLIR);
    RUN_TEST(TestXDNA_Stats);
    RUN_TEST(TestXDNA_Reset);
    RUN_TEST(TestXDNA_TileModes);
    RUN_TEST(TestXDNA_AvgLatency);
    RUN_TEST(TestQAIE_Init);
    RUN_TEST(TestQAIE_DeviceName);
    RUN_TEST(TestQAIE_RunModel);
    RUN_TEST(TestQAIE_HTPPath);
    RUN_TEST(TestQAIE_GPUPath);
    RUN_TEST(TestQAIE_Quantization);
    RUN_TEST(TestQAIE_Stats);
    RUN_TEST(TestQAIE_Reset);
    RUN_TEST(TestQAIE_AvgLatency_HTP);
    RUN_TEST(TestAMX_Init);
    RUN_TEST(TestAMX_MatMul_BF16);
    RUN_TEST(TestAMX_MatMul_INT8);
    RUN_TEST(TestAMX_MatMul_Empty);
    RUN_TEST(TestAMX_Throughput);
    RUN_TEST(TestAMX_Stats);
    RUN_TEST(TestAMX_Reset);
    RUN_TEST(TestAMX_SupportFlag);
    RUN_TEST(TestAMX_AvgLatency);
    RUN_TEST(TestHAP_Init);
    RUN_TEST(TestHAP_Process_Infer);
    RUN_TEST(TestHAP_NPURouting);
    RUN_TEST(TestHAP_GPURouting);
    RUN_TEST(TestHAP_CPUFallback_Flag);
    RUN_TEST(TestHAP_Stats);
    RUN_TEST(TestHAP_Reset);
    RUN_TEST(TestHAP_Empty_Input);
    RUN_TEST(TestHAP_Composite);
    RUN_TEST(TestCDR_Instance);
    RUN_TEST(TestCDR_Init);
    RUN_TEST(TestCDR_HasDevices);
    RUN_TEST(TestCDR_HasCPU);
    RUN_TEST(TestCDR_CPUCount);
    RUN_TEST(TestCDR_Stats_Enumerated);
    RUN_TEST(TestCDR_Devices_Valid);
    RUN_TEST(TestCDR_FindBest_Fallback);
    RUN_TEST(TestCDR_MultiInit);

    // DirectStorage & Zero-Latency Pipeline Tests (v32.2.0 Fomalhaut-S)
    RUN_TEST(TestDSM_ProbeCapabilities);
    RUN_TEST(TestDSM_InitShutdown);
    RUN_TEST(TestDSM_SubmitRequest_CPUFallback);
    RUN_TEST(TestDSM_BackendName);
    RUN_TEST(TestDSM_SupportedCompression);
    RUN_TEST(TestGDK_ProbeCapabilities);
    RUN_TEST(TestGDK_InitSelectsVendor);
    RUN_TEST(TestGDK_Decompress_CPUFallback);
    RUN_TEST(TestGDK_EstimateOutputSize);
    RUN_TEST(TestGDK_VendorName);
    RUN_TEST(TestZLP_Initialize);
    RUN_TEST(TestZLP_Process_CPUPath);
    RUN_TEST(TestZLP_Metrics_Tracked);
    RUN_TEST(TestZLP_RecommendedThumbSize);
    RUN_TEST(TestZLP_Shutdown_Resets);
    RUN_TEST(TestTPM_RecordAndStats);
    RUN_TEST(TestTPM_StageName);
    RUN_TEST(TestTPM_BottleneckDetect_IO);
    RUN_TEST(TestTPM_Reset_ClearsStats);
    RUN_TEST(TestTPM_DescribeBottleneck);
    RUN_TEST(TestSDO_Probe_RAW);
    RUN_TEST(TestSDO_Probe_FITS);
    RUN_TEST(TestSDO_Probe_Unknown);
    RUN_TEST(TestSDO_Decode_Returns_Pixels);
    RUN_TEST(TestSDO_EstimateMinBytes);

    // Annotation, HDR Tone Mapping & Format Detection Tests (v32.3.0 Fomalhaut-T)
    RUN_TEST(TestTAO_NoBadges);
    RUN_TEST(TestTAO_DRMBadge);
    RUN_TEST(TestTAO_MultiBadge);
    RUN_TEST(TestTAO_RecommendedSize);
    RUN_TEST(TestTAO_BadgeMaskForFile);
    RUN_TEST(TestABD_SDR_PassThrough);
    RUN_TEST(TestABD_DetectSourceFormat);
    RUN_TEST(TestABD_ToneMappingName);
    RUN_TEST(TestABD_SourceFormatName);
    RUN_TEST(TestABD_HDR_Convert_BufferSize);
    RUN_TEST(TestBTE_FormatExtension);
    RUN_TEST(TestBTE_FormatSupportsLossless);
    RUN_TEST(TestBTE_Export_SingleSize);
    RUN_TEST(TestBTE_Export_MultiSize);
    RUN_TEST(TestBTE_TotalExported);
    RUN_TEST(TestFSD_DetectJPEG);
    RUN_TEST(TestFSD_DetectPNG);
    RUN_TEST(TestFSD_DetectPDF);
    RUN_TEST(TestFSD_DetectUnknown);
    RUN_TEST(TestFSD_ExtensionMatch);
    RUN_TEST(TestMMD_Initialize);
    RUN_TEST(TestMMD_Decode_ReturnsPixels);
    RUN_TEST(TestMMD_Stats_Tracked);
    RUN_TEST(TestMMD_RecommendedForFile);
    RUN_TEST(TestMMD_BackendName);
    // Sprint 1111-1120: DirectStorage Zero-Copy GPU Decompress (v32.5.0 "Fomalhaut-V")
    RUN_TEST(TestZSK_VendorName);
    RUN_TEST(TestZSK_FallbackWhenUnavailable);
    RUN_TEST(TestZSK_DecompressReturnsBytesOnFallback);
    RUN_TEST(TestGDO_DefaultBackendIsCPU);
    RUN_TEST(TestGDO_BackendName);
    RUN_TEST(TestGDO_DecompressCPUPath);
    RUN_TEST(TestGDO_DecompressInvalidSizeFails);
    RUN_TEST(TestDSBS_AddAndFlush);
    RUN_TEST(TestDSBS_EmptyFlush);
    RUN_TEST(TestDSBS_PendingCount);
    RUN_TEST(TestDSBS_ResultsAfterFlush);
    RUN_TEST(TestDSBS_ResetClearsAll);
    RUN_TEST(TestDSP_PathName);
    RUN_TEST(TestDSP_RecommendedPath);
    RUN_TEST(TestDSP_AddSampleAndStats);
    RUN_TEST(TestDSP_ResetClearsSamples);
    RUN_TEST(TestDSP_P99LargerThanP50);
    RUN_TEST(TestZCS_DefaultState);
    RUN_TEST(TestZCS_TotalMs);
    RUN_TEST(TestZCS_IsTerminal);
    RUN_TEST(TestZCS_StateName);
    RUN_TEST(TestZCS_GpuDecompressFlag);
    // Sprint 1121-1130: CLIP Semantic Search + HNSW Index (v32.6.0 "Fomalhaut-W")
    RUN_TEST(TestHNSW_InsertAndCount);
    RUN_TEST(TestHNSW_Remove);
    RUN_TEST(TestHNSW_QueryTopK);
    RUN_TEST(TestHNSW_SaveLoad);
    RUN_TEST(TestHNSW_Reset);
    RUN_TEST(TestCQP_BackendName);
    RUN_TEST(TestCQP_LoadModelFails_EmptyPath);
    RUN_TEST(TestCQP_LoadModelSucceeds);
    RUN_TEST(TestCQP_QueryEmpty_WhenNotLoaded);
    RUN_TEST(TestCQP_LastEmbedMsDefault);
    RUN_TEST(TestSSO_InitializeSucceeds);
    RUN_TEST(TestSSO_IsReadyAfterInit);
    RUN_TEST(TestSSO_IndexFile);
    RUN_TEST(TestSSO_SearchEmpty_BeforeIndex);
    RUN_TEST(TestSSO_IndexedCountTracked);
    RUN_TEST(TestEPE_OpenAndClose);
    RUN_TEST(TestEPE_AppendEntry);
    RUN_TEST(TestEPE_FlushUpdatesStats);
    RUN_TEST(TestEPE_LoadAllEmpty);
    RUN_TEST(TestEPE_StatsJournalBytes);
    RUN_TEST(TestVQO_DefaultActive);
    RUN_TEST(TestVQO_SetActive);
    RUN_TEST(TestVQO_PruneNoHint);
    RUN_TEST(TestVQO_PruneReducesCandidates);
    RUN_TEST(TestVQO_PruneResultFields);

    // Sprint 1131-1140: Live Preview Scrubber (v32.7.0 "Fomalhaut-X")
    RUN_TEST(TestVFE_BackendName_DXVA2);
    RUN_TEST(TestVFE_BackendName_MFSoftware);
    RUN_TEST(TestVFE_BackendName_Unavailable);
    RUN_TEST(TestVFE_ExtractFrame_ValidPath);
    RUN_TEST(TestVFE_ExtractFrame_EmptyPath);
    RUN_TEST(TestVST_BuildEmpty);
    RUN_TEST(TestVST_BuildPopulates);
    RUN_TEST(TestVST_KeyframeAt_Valid);
    RUN_TEST(TestVST_KeyframeAt_OutOfBounds);
    RUN_TEST(TestVST_NearestKeyframePts);
    RUN_TEST(TestLPS_OpenEmpty);
    RUN_TEST(TestLPS_OpenValid);
    RUN_TEST(TestLPS_SeekNotOpen);
    RUN_TEST(TestLPS_SeekOpen);
    RUN_TEST(TestLPS_Close);
    RUN_TEST(TestTSG_ResetClearsState);
    RUN_TEST(TestTSG_GenerateEmpty);
    RUN_TEST(TestTSG_GenerateValid);
    RUN_TEST(TestTSG_StripWidth);
    RUN_TEST(TestTSG_FrameCount);
    RUN_TEST(TestSCE_InitialEmpty);
    RUN_TEST(TestSCE_PutAndGet);
    RUN_TEST(TestSCE_GetMiss);
    RUN_TEST(TestSCE_Evict);
    RUN_TEST(TestSCE_HitRate);
    // Sprint 1141-1150: Cross-Platform Shell PAL (v33.0.0 "Spica")
    RUN_TEST(TestPSP_Win32PlatformKind);
    RUN_TEST(TestPSP_Win32PlatformName);
    RUN_TEST(TestPSP_Win32NotRegisteredByDefault);
    RUN_TEST(TestPSP_Win32RegisterUnregister);
    RUN_TEST(TestPSP_Win32ThumbnailEmpty);
    RUN_TEST(TestPSP_Win32ThumbnailPath);
    RUN_TEST(TestPSP_Win32ThumbnailNonzeroSize);
    RUN_TEST(TestPD_DetectCurrentPlatform);
    RUN_TEST(TestPD_PlatformNameWindows);
    RUN_TEST(TestPD_MakeWin32Provider);
    RUN_TEST(TestPD_PlatformDescString);
    RUN_TEST(TestPD_CurrentProviderForPlatform);
    // Sprint 1151-1160: Platform GPU Router (v33.1.0 Windows-only)
    RUN_TEST(TestPGR_SelectsD3D12OnWindows);
    RUN_TEST(TestPGR_BackendNameNotNull);
    // Sprint 1161-1170: Enterprise Console v4 (v33.2.0 "Spica-S")
    RUN_TEST(TestEPV4_Initialize);
    RUN_TEST(TestEPV4_ApplyPolicy);
    RUN_TEST(TestGPOT_AddSetting);
    RUN_TEST(TestGPOT_GenerateADMX);
    RUN_TEST(TestICE_AddRule);
    RUN_TEST(TestICE_Evaluate);
    RUN_TEST(TestEAL_InitializeLog);
    RUN_TEST(TestEAL_LogEvent);
    RUN_TEST(TestCMPB_Initialize);
    RUN_TEST(TestCMPB_Synchronize);
    // Sprint 1171-1180: Generative AI Thumbnails (v33.3.0 "Spica-T")
    RUN_TEST(TestNPU_Initialize);
    RUN_TEST(TestNPU_Synthesize);
    RUN_TEST(TestDME_LoadModel);
    RUN_TEST(TestDME_EncodePrompt);
    RUN_TEST(TestTIE_Initialize);
    RUN_TEST(TestTIE_Inpaint);
    RUN_TEST(TestOIR_Initialize);
    RUN_TEST(TestOIR_RouteInference);
    RUN_TEST(TestAIBP_Enqueue);
    RUN_TEST(TestAIBP_ProcessBatch);
    // Sprint 1181-1190: Plugin Marketplace v5 (v33.4.0 "Spica-U")
    RUN_TEST(TestPMV5_Initialize);
    RUN_TEST(TestPMV5_Search);
    RUN_TEST(TestSCK3_Initialize);
    RUN_TEST(TestSCK3_DetectVersion);
    RUN_TEST(TestPDM_Install);
    RUN_TEST(TestPDM_Rollback);
    RUN_TEST(TestMSI_IndexAndQuery);
    RUN_TEST(TestMSI_DocumentCount);
    RUN_TEST(TestPSV_ValidatePlugin);
    RUN_TEST(TestPSV_AddTrustedThumbprint);
    // Sprint 1191-1200: LTS Hardening + Security Audit (v33.5.0 "Spica-V")
    RUN_TEST(TestLHC_Initialize);
    RUN_TEST(TestLHC_GateEvaluate);
    RUN_TEST(TestSAE_Initialize);
    RUN_TEST(TestSAE_RunAudit);
    RUN_TEST(TestVFDB_AddRecord);
    RUN_TEST(TestVFDB_QueryByLibrary);
    RUN_TEST(TestLCG_RunGate);
    RUN_TEST(TestLCG_IsCertified);
    RUN_TEST(TestSKS_StoreRetrieve);
    RUN_TEST(TestSKS_DeleteKey);
    // Sprint 1201-1210: Format Coverage Blitz (v34.0.0 "Arcturus")
    RUN_TEST(TestBasisUniversalDecoder_Extensions);
    RUN_TEST(TestBasisUniversalDecoder_CanDecode);
    RUN_TEST(TestUltraHDRDecoder_Extensions);
    RUN_TEST(TestUltraHDRDecoder_CanDecode);
    RUN_TEST(TestIfcBimDecoder_Extensions);
    RUN_TEST(TestIfcBimDecoder_CanDecode);
    RUN_TEST(TestLasPointCloudDecoder_Extensions);
    RUN_TEST(TestLasPointCloudDecoder_CanDecode);
    RUN_TEST(TestJupyterNotebookDecoder_Extensions);
    RUN_TEST(TestJupyterNotebookDecoder_CanDecode);
    // Sprint 1211-1220: GPU-First Decode Pipeline (v34.1.0 "Arcturus-R")
    RUN_TEST(TestGPUDecodeFormatRouter_RouteJPEG);
    RUN_TEST(TestGPUDecodeFormatRouter_FallbackCPU);
    RUN_TEST(TestGPUJPEGDecodeAccelerator_Init);
    RUN_TEST(TestGPUJPEGDecodeAccelerator_Caps);
    RUN_TEST(TestGPURawDemosaicKernel_Identity);
    RUN_TEST(TestGPURawDemosaicKernel_BayerMasks);
    RUN_TEST(TestGPUDecodePerformanceGate_Pass);
    RUN_TEST(TestGPUDecodePerformanceGate_Block);
    RUN_TEST(TestZeroCopyGPUSurface_Alloc);
    RUN_TEST(TestZeroCopyGPUSurface_MapUnmap);

    // Sprint 1221-1230: HDR & Wide Color Gamut Mastery (v34.2.0 "Arcturus-S")
    RUN_TEST(TestGainmapJPEGToneMapper_UltraHDRDetect);
    RUN_TEST(TestGainmapJPEGToneMapper_ParseMetadata);
    RUN_TEST(TestPQToSDRToneMapper_LUTBuild);
    RUN_TEST(TestPQToSDRToneMapper_SinglePixel);
    RUN_TEST(TestHLGToSDRConverter_Identity);
    RUN_TEST(TestHLGToSDRConverter_OOTFGamma);
    RUN_TEST(TestICCv5ProfileEngine_LoadBuiltIn);
    RUN_TEST(TestICCv5ProfileEngine_DetectColorspace);
    RUN_TEST(TestACESODTProcessor_DetectFromString);
    RUN_TEST(TestACESODTProcessor_ACEScgToSRGB);

    // Sprint 1231-1240: Predictive Pre-Generation Engine (v34.3.0 "Arcturus-T")
    RUN_TEST(TestDirectoryPreScanQueue_NetworkDetect);
    RUN_TEST(TestDirectoryPreScanQueue_StartStop);
    RUN_TEST(TestAdjacencyPredictor_Record);
    RUN_TEST(TestAdjacencyPredictor_PredictEmpty);
    RUN_TEST(TestScrollVelocityTracker_ZeroVelocity);
    RUN_TEST(TestScrollVelocityTracker_EMASmoothing);
    RUN_TEST(TestIdleTimePreGenerator_Stats);
    RUN_TEST(TestIdleTimePreGenerator_BatteryCheck);
    RUN_TEST(TestPredictivePreGenEngine_InitStats);
    RUN_TEST(TestPredictivePreGenEngine_CacheHitCount);

    // Sprint 1241-1250: Animated & Sequence Format Suite (v34.4.0 "Arcturus-U")
    RUN_TEST(TestHoverScrubController_PosToFrame);
    RUN_TEST(TestHoverScrubController_MouseLeave);
    RUN_TEST(TestAPNGFrameCombiner_SelectKeyFrames);
    RUN_TEST(TestAPNGFrameCombiner_ProbeFrameCount);
    RUN_TEST(TestGIFAnimationDecoder_IsGIF);
    RUN_TEST(TestGIFAnimationDecoder_ProbeFrameCount);
    RUN_TEST(TestAnimatedSequenceSampler_DetectGIF);
    RUN_TEST(TestAnimatedSequenceSampler_SampleGIF);
    RUN_TEST(TestAnimatedThumbnailCache_PutGet);
    RUN_TEST(TestAnimatedThumbnailCache_Eviction);

    // Sprint 1251-1260: Industrial & Scientific Formats v2 (v34.5.0 "Arcturus-V")
    RUN_TEST(TestDICOMWindowingPresets_GetPreset);
    RUN_TEST(TestDICOMWindowingPresets_BuildLUT);
    RUN_TEST(TestFITSZScaleStretch_HeatMap);
    RUN_TEST(TestFITSZScaleStretch_Stretch);
    RUN_TEST(TestLASPointCloudRenderer_IsLAS);
    RUN_TEST(TestLASPointCloudRenderer_ProbePointCount);
    RUN_TEST(TestOMETIFFCompositor_WavelengthToBGR);
    RUN_TEST(TestOMETIFFCompositor_IsOMETIFF);
    RUN_TEST(TestMHAVolumeDecoder_IsMHA);
    RUN_TEST(TestMHAVolumeDecoder_ParseHeader);

    // Sprint 1261-1270: CAD/BIM/EDA Formats (v34.6.0 "Arcturus-W")
    RUN_TEST(TestDWGHeaderParser_IsDWG);
    RUN_TEST(TestDWGHeaderParser_Parse);
    RUN_TEST(TestSTEPBoundingBoxExtractor_DetectFormat);
    RUN_TEST(TestSTEPBoundingBoxExtractor_ExtractSTEP);
    RUN_TEST(TestIFCEntityCounter_IsIFC);
    RUN_TEST(TestIFCEntityCounter_Count);
    RUN_TEST(TestGerberLayerCompositor_IsGerber);
    RUN_TEST(TestGerberLayerCompositor_ProbeLayer);
    RUN_TEST(TestKiCadNetlistParser_IsKiCad);
    RUN_TEST(TestKiCadNetlistParser_Parse);

    // Sprint 1271-1280: Performance Hardening + LTS Gate (v34.7.0 "Arcturus-X")
    RUN_TEST(TestPerfRegressionGate_Thresholds);
    RUN_TEST(TestPerfRegressionGate_BlockOnFail);
    RUN_TEST(TestLTSBuildValidator_AllGatesPass);
    RUN_TEST(TestLTSBuildValidator_FailOnCoverage);
    RUN_TEST(TestCacheWarmupPreloader_StartStop);
    RUN_TEST(TestCacheWarmupPreloader_Stats);
    RUN_TEST(TestDecodeLatencyProfiler_RecordAndQuery);
    RUN_TEST(TestDecodeLatencyProfiler_Reset);
    RUN_TEST(TestBenchmarkBaseline_LoadCompare);
    RUN_TEST(TestBenchmarkBaseline_NoRegression);

    // Sprint 1281-1290: Streaming & Cloud-Native Thumbnails (v35.0.0 "Vega")
    RUN_TEST(TestMultiStageThumbnailEmitter_Stages);
    RUN_TEST(TestMultiStageThumbnailEmitter_Cancel);
    RUN_TEST(TestCloudHydrationMonitor_Detect);
    RUN_TEST(TestCloudHydrationMonitor_Defer);
    RUN_TEST(TestPartialDecodeStateCache_SaveRestore);
    RUN_TEST(TestPartialDecodeStateCache_Eviction);
    RUN_TEST(TestThumbnailETagValidator_Match);
    RUN_TEST(TestThumbnailETagValidator_Invalidate);
    RUN_TEST(TestAdaptiveFidelitySelector_HighBudget);
    RUN_TEST(TestAdaptiveFidelitySelector_LowBudget);

    // Sprint 1291-1300: Real-Time Collaboration & Live Edit Sync (v35.1.0 "Vega-R")
    RUN_TEST(TestLiveSyncTokenManager_Issue);
    RUN_TEST(TestLiveSyncTokenManager_Expire);
    RUN_TEST(TestCollaborativeCacheCoordinator_Invalidate);
    RUN_TEST(TestCollaborativeCacheCoordinator_Sync);
    RUN_TEST(TestThumbnailDeltaEncoder_Encode);
    RUN_TEST(TestThumbnailDeltaEncoder_Decode);
    RUN_TEST(TestConflictResolutionEngine_Merge);
    RUN_TEST(TestConflictResolutionEngine_PickLatest);
    RUN_TEST(TestRealTimePreviewPipeline_Subscribe);
    RUN_TEST(TestRealTimePreviewPipeline_Backpressure);
    // Sprint 1301-1310: Network-Aware Streaming Cache
    RUN_TEST(TestNetworkTopologyProbe_Probe);
    RUN_TEST(TestNetworkTopologyProbe_ForceTopology);
    RUN_TEST(TestStreamingCacheTierPolicy_Derive);
    RUN_TEST(TestStreamingCacheTierPolicy_Override);
    RUN_TEST(TestBandwidthThrottleGuard_Allow);
    RUN_TEST(TestBandwidthThrottleGuard_Throttle);
    RUN_TEST(TestRemoteFileManifestCache_Store);
    RUN_TEST(TestRemoteFileManifestCache_Stale);
    RUN_TEST(TestCachePrefetchScheduler_Enqueue);
    RUN_TEST(TestCachePrefetchScheduler_Backpressure);
    // Sprint 1311-1320: Zero-Trust Thumbnail Security
    RUN_TEST(TestThumbnailManifestSigner_Sign);
    RUN_TEST(TestThumbnailManifestSigner_Verify);
    RUN_TEST(TestZeroTrustDecodeWorker_Spawn);
    RUN_TEST(TestZeroTrustDecodeWorker_Decode);
    RUN_TEST(TestTokenBoundCacheEntry_Store);
    RUN_TEST(TestTokenBoundCacheEntry_CrossTenant);
    RUN_TEST(TestThumbnailAuditLog_Record);
    RUN_TEST(TestThumbnailAuditLog_Query);
    RUN_TEST(TestFIPSCryptoAdapter_Hash);
    RUN_TEST(TestFIPSCryptoAdapter_Hmac);
    // Sprint 1321-1330: WebAssembly / Browser Extension Pipeline
    RUN_TEST(TestWasmDecoderShim_Register);
    RUN_TEST(TestWasmDecoderShim_Decode);
    RUN_TEST(TestBrowserThumbnailBridge_PostMessage);
    RUN_TEST(TestBrowserThumbnailBridge_AsyncReply);
    RUN_TEST(TestOffscreenCanvasRenderer_FrameSize);
    RUN_TEST(TestOffscreenCanvasRenderer_ErrorOnNull);
    RUN_TEST(TestWasmCacheAdapter_Store);
    RUN_TEST(TestWasmCacheAdapter_Evict);
    RUN_TEST(TestProgressiveThumbnailStream_Emit);
    RUN_TEST(TestProgressiveThumbnailStream_Complete);

    // Sprint 1331-1340: Cross-Device Preview Sync
    RUN_TEST(TestDeviceSyncManifest_Upsert);
    RUN_TEST(TestDeviceSyncManifest_Serialize);
    RUN_TEST(TestCrossDeviceCacheSync_Upload);
    RUN_TEST(TestCrossDeviceCacheSync_Bidirectional);
    RUN_TEST(TestThumbnailPackFile_PackExtract);
    RUN_TEST(TestThumbnailPackFile_InvalidMagic);
    RUN_TEST(TestSyncConflictResolver_LatestEtag);
    RUN_TEST(TestSyncConflictResolver_RemoteWins);
    RUN_TEST(TestDeviceCapabilityAdvertiser_Probe);
    RUN_TEST(TestDeviceCapabilityAdvertiser_FindBest);

    std::wcout << std::endl;
    std::wcout << L"Isolation & Stability Tests..." << std::endl;
    RUN_TEST(TestMalformedArchive_TruncatedZIP);
    RUN_TEST(TestMalformedArchive_GarbageHeader);
    RUN_TEST(TestMalformedArchive_ZeroByteFile);
    RUN_TEST(TestCircuitBreaker_StressTest);
    RUN_TEST(TestDecoderTimeout_Enforcement);
    RUN_TEST(TestMemoryLeak_RegressionLoop);

    std::wcout << std::endl;

    // Production Pipeline & Cache Tests
    std::wcout << L"Production Pipeline & Cache Tests..." << std::endl;
    RUN_TEST(TestPSOCachePersistence_Lifecycle);
    RUN_TEST(TestPipelineActivator_State);
    RUN_TEST(TestParallelIOActivation_Init);
    RUN_TEST(TestCacheBudgetAutoTuner_Tier);
    RUN_TEST(TestSIMDCapabilityDetector_Detect);

    // Reliability, Cache, GPU, Memory, Plugin, Security & Operations Tests
    std::wcout << L"Reliability & Operations Tests..." << std::endl;
    RUN_TEST(TestPipelineMetricsCollector_Init);
    RUN_TEST(TestPipelineCircuitBreaker_State);
    RUN_TEST(TestDecodeRetryPolicy_Evaluate);
    RUN_TEST(TestThumbnailRequestValidator_Validate);
    RUN_TEST(TestDecoderOutputValidator_Init);
    RUN_TEST(TestCacheCoherencyManager_Init);
    RUN_TEST(TestCachePrewarmScheduler_Init);
    RUN_TEST(TestCacheDiagnostics_Health);
    RUN_TEST(TestGPUFallbackChain_Select);
    RUN_TEST(TestGPUMemoryTracker_Budget);
    RUN_TEST(TestMemoryBudgetEnforcer_Level);
    RUN_TEST(TestAllocationTracker_Track);
    RUN_TEST(TestPluginDependencyResolver_Resolve);
    RUN_TEST(TestPluginCrashRecovery_Report);
    RUN_TEST(TestPluginResourceLimiter_Quota);
    RUN_TEST(TestInputSanitizer_Path);
    RUN_TEST(TestPathTraversalGuard_Detect);
    RUN_TEST(TestConfigDriftDetector_Drift);
    RUN_TEST(TestOperationalReadinessChecker_Check);

    std::wcout << std::endl;

    // V15 Zenith - Production Infrastructure Tests
    std::wcout << L"\nV15 Zenith - Production Infrastructure Tests..." << std::endl;

    // Content-Aware Thumbnail
    RUN_TEST(TestContentAwareThumbnail_Analyze);
    RUN_TEST(TestContentAwareThumbnail_NullInput);

    // Runtime SIMD Dispatcher
    RUN_TEST(TestRuntimeSIMDDispatcher_Init);
    RUN_TEST(TestRuntimeSIMDDispatcher_Describe);

    // Batch Thumbnail Orchestrator
    RUN_TEST(TestBatchOrchestrator_Init);

    // File Signature Detector
    RUN_TEST(TestFileSignatureDetector_PNG);
    RUN_TEST(TestFileSignatureDetector_JPEG);
    RUN_TEST(TestFileSignatureDetector_ZIP);
    RUN_TEST(TestFileSignatureDetector_Unknown);
    RUN_TEST(TestFileSignatureDetector_Null);

    // GPU Resource Pool Manager
    RUN_TEST(TestGPUResourcePoolManager_Config);

    // Cache Coherency Protocol
    RUN_TEST(TestCacheCoherencyProtocol_Config);
    RUN_TEST(TestCacheCoherencyProtocol_Init);

    // Format Negotiator

    // Telemetry Aggregator

    // Decoder Registry V2
    RUN_TEST(TestDecoderRegistryV2_Register);
    RUN_TEST(TestDecoderRegistryV2_GetInfo);
    RUN_TEST(TestDecoderRegistryV2_Unregister);

    // Production Pipeline V2
    RUN_TEST(TestProductionPipelineV2_Init);
    RUN_TEST(TestProductionPipelineV2_Stages);

    // Sprint 41-42: Performance Tuning
    std::wcout << L"\nSprint 41-42: Performance Tuning..." << std::endl;
    // AlignedBufferPool
    RUN_TEST(Test_S41_AlignedBufferPool_AcquireAligned);
    RUN_TEST(Test_S41_AlignedBufferPool_TierSelection);
    RUN_TEST(Test_S41_AlignedBufferPool_PoolReuse);
    RUN_TEST(Test_S41_AlignedBufferPool_PooledBufferRAII);
    // PrefetchHintEngine
    RUN_TEST(Test_S41_PrefetchHint_FileHeader);
    RUN_TEST(Test_S41_PrefetchHint_NullSafety);
    RUN_TEST(Test_S41_PrefetchHint_StrategyNames);
    // BranchPredictor
    RUN_TEST(Test_S41_BranchPredictor_SortedLookup);
    RUN_TEST(Test_S41_BranchPredictor_BranchlessMinMax);
    RUN_TEST(Test_S41_BranchPredictor_HotPathCounter);
    // LoopTuner
    RUN_TEST(Test_S41_LoopTuner_TileProcessor);
    RUN_TEST(Test_S41_LoopTuner_OperationBatcher);
    RUN_TEST(Test_S41_LoopTuner_RowBatches);
    // CacheLinePadding
    RUN_TEST(Test_S41_CacheLinePadding_Alignment);
    RUN_TEST(Test_S41_CacheLinePadding_PaddedAtomic);
    RUN_TEST(Test_S41_CacheLinePadding_CacheLineArray);

    // Sprint 43-44: Plugin SDK Hardening
    std::wcout << L"\nPlugin SDK Hardening Tests..." << std::endl;
    // PluginCapabilityGuard
    RUN_TEST(Test_S43_CapabilityGuard_GrantRevoke);
    RUN_TEST(Test_S43_CapabilityGuard_RAIIValidation);
    RUN_TEST(Test_S43_CapabilityGuard_AuditTrail);
    RUN_TEST(Test_S43_CapabilityName_Strings);
    // PluginResourceLimiter (Enhanced)
    RUN_TEST(Test_S43_ResourceLimiter_Budget);
    RUN_TEST(Test_S43_ResourceLimiter_ViolationCallback);
    RUN_TEST(Test_S43_ResourceLimiter_Checkpoint);
    // PluginVersionNegotiator
    RUN_TEST(Test_S43_VersionNegotiator_Compat);
    RUN_TEST(Test_S43_VersionNegotiator_Parse);
    RUN_TEST(Test_S43_VersionNegotiator_Migration);
    // PluginCrashIsolation
    // PluginAuditLog
    RUN_TEST(Test_S43_AuditLog_AppendAndQuery);
    RUN_TEST(Test_S43_AuditLog_RingBuffer);
    RUN_TEST(Test_S43_AuditLog_JSONExport);
    RUN_TEST(Test_S43_AuditLog_SeverityCount);

    // Sprint 45-46: Error Handling Infrastructure
    std::wcout << L"\nSprint 45-46: Error Handling Infrastructure..." << std::endl;
    // StructuredErrorDomain
    RUN_TEST(Test_S45_StructuredErrorDomain_CreateAndFormat);
    RUN_TEST(Test_S45_StructuredErrorDomain_InnerErrorChain);
    RUN_TEST(Test_S45_StructuredErrorDomain_SeverityNames);
    // ResultType
    RUN_TEST(Test_S45_ResultType_OkAndErr);
    RUN_TEST(Test_S45_ResultType_MapAndChain);
    RUN_TEST(Test_S45_ResultType_ValueOr);
    // ErrorRecoveryEngineV2
    // DiagnosticCollector
    RUN_TEST(Test_S45_DiagnosticCollector_RingBuffer);
    RUN_TEST(Test_S45_DiagnosticCollector_SnapshotAndJson);
    RUN_TEST(Test_S45_DiagnosticCollector_Summary);
    // PipelineErrorBoundary
    RUN_TEST(Test_S45_PipelineErrorBoundary_SuccessPath);
    RUN_TEST(Test_S45_PipelineErrorBoundary_FallbackOnError);
    RUN_TEST(Test_S45_PipelineErrorBoundary_FatalPromotion);
    RUN_TEST(Test_S45_PipelineErrorBoundaryManager_AggregateMetrics);

    // Sprint 47-48: CI/CD Pipeline + Build Validation
    std::wcout << L"\nSprint 47-48: CI/CD Pipeline + Build Validation..." << std::endl;
    // BuildValidator
    RUN_TEST(Test_S47_BuildValidator_GetBuildInfo);
    RUN_TEST(Test_S47_BuildValidator_ValidateBuildEnvironment);
    RUN_TEST(Test_S47_BuildValidator_ValidateRuntimeEnvironment);
    RUN_TEST(Test_S47_BuildValidator_CRTConsistency);
    // CITestReporter
    RUN_TEST(Test_S47_CITestReporter_CollectResults);
    RUN_TEST(Test_S47_CITestReporter_JUnitXMLExport);
    RUN_TEST(Test_S47_CITestReporter_JSONExport);
    RUN_TEST(Test_S47_CITestReporter_ConsoleSummary);
    // EnvironmentProbe
    RUN_TEST(Test_S47_EnvironmentProbe_DetectCIProvider);
    RUN_TEST(Test_S47_EnvironmentProbe_DetectSIMD);
    RUN_TEST(Test_S47_EnvironmentProbe_DetectWindowsVersion);
    RUN_TEST(Test_S47_EnvironmentProbe_ProbeEnvironment);
    RUN_TEST(Test_S47_EnvironmentProbe_DisplayDPI);

    // Sprint 49-50: Dark Mode & Theme Rendering
    std::wcout << L"\nSprint 49-50 Tests (Dark Mode & Theme)..." << std::endl;
    RUN_TEST(Test_S49_ThemeAwareOverlay_ComputeColors);
    RUN_TEST(Test_S49_ContrastValidator_CheckRatio);
    RUN_TEST(Test_S50_SystemThemeMonitor_Detect);
    RUN_TEST(Test_S50_AdaptiveIconRenderer_RenderSize);

    // Sprint 51-52: Decode Pipeline Enhancement
    std::wcout << L"\nSprint 51-52 Tests (Decode Pipeline)..." << std::endl;
    RUN_TEST(Test_S51_StreamingDecodeEngine_Init);
    RUN_TEST(Test_S52_MultiPagePreview_Config);

    // Sprint 53-54: Memory Management
    std::wcout << L"\nSprint 53-54 Tests (Memory Management)..." << std::endl;
    RUN_TEST(Test_S53_MemoryArenaAllocator_AllocFree);
    RUN_TEST(Test_S53_LargePageAllocator_Query);

    // Sprint 55-56: Pipeline Scheduling
    std::wcout << L"\nSprint 55-56 Tests (Pipeline Scheduling)..." << std::endl;
    RUN_TEST(Test_S55_PipelineStageProfiler_Record);
    RUN_TEST(Test_S55_AdaptiveTimeoutController_Compute);
    RUN_TEST(Test_S55_PipelineReplayRecorder_RecordReplay);
    RUN_TEST(Test_S56_DecodePriorityQueue_PushPop);

    // Sprint 57-58: Cache Intelligence
    std::wcout << L"\nSprint 57-58 Tests (Cache Intelligence)..." << std::endl;
    RUN_TEST(Test_S57_CacheCompressionEngine_Ratio);
    RUN_TEST(Test_S57_CacheIntegrityVerifier_Verify);

    // Sprint 59-60: GPU Enhancement
    std::wcout << L"\nSprint 59-60 Tests (GPU Enhancement)..." << std::endl;
    RUN_TEST(Test_S59_GPUShaderCompiler_Compile);
    RUN_TEST(Test_S59_GPUOccupancyOptimizer_Optimize);
    RUN_TEST(Test_S59_GPUDebugLayer_RecordMessages);
    RUN_TEST(Test_S60_GPUMemoryPoolManager_AllocFree);
    RUN_TEST(Test_S60_GPUAsyncCopyEngine_SubmitComplete);

    // Sprint 61-62: Enterprise & Deployment
    std::wcout << L"\nSprint 61-62 Tests (Enterprise & Deployment)..." << std::endl;
    RUN_TEST(Test_S61_EnterpriseTelemetryRouter_Route);
    RUN_TEST(Test_S61_PolicyComplianceValidator_Validate);
    RUN_TEST(Test_S62_SilentUpdateOrchestrator_CheckUpdates);
    RUN_TEST(Test_S62_DiagnosticBundleExporter_Export);
    RUN_TEST(Test_S62_DeploymentHealthChecker_RunAll);

    // Sprint 63-64: Plugin Ecosystem
    std::wcout << L"\nSprint 63-64 Tests (Plugin Ecosystem)..." << std::endl;

    // Sprint 65-66: Quality & Observability
    std::wcout << L"\nSprint 65-66 Tests (Quality & Observability)..." << std::endl;
    RUN_TEST(Test_S65_StructuredDiagnosticLogger_Log);
    RUN_TEST(Test_S65_ResourceLeakTracker_DetectLeak);
    RUN_TEST(Test_S66_HealthCheckOrchestrator_Summarize);

    // Sprint 67-68: Final Polish
    std::wcout << L"\nSprint 67-68 Tests (Final Polish)..." << std::endl;
    RUN_TEST(Test_S67_ShellIntegrationValidator_ValidateAll);
    RUN_TEST(Test_S67_GracefulDegradationController_Degrade);
    RUN_TEST(Test_S67_UserPreferenceEngine_SetGet);
    RUN_TEST(Test_S68_StartupOptimizer_Metrics);
    RUN_TEST(Test_S68_CrashRecoveryEngine_Checkpoint);
    RUN_TEST(Test_S68_HDRToneMapper_ACES);
    RUN_TEST(Test_S68_ColorSpaceConverter_Identity);
    RUN_TEST(Test_S68_ExifOrientationHandler_Rotate90);
    RUN_TEST(Test_S68_IconBadgeRenderer_ScaledPlacement);

    // Sprint 9-14: Resilience & Hardening (v15.3.0 "Zenith-T")
    std::wcout << L"\nSprint 9-14 Tests (Resilience & Hardening)..." << std::endl;
    RUN_TEST(Test_S9_DecodeInputValidator_DimensionLimits);
    RUN_TEST(Test_S9_DecodeInputValidator_FileSizeLimit);
    RUN_TEST(Test_S9_DecodeInputValidator_BitDepth);
    RUN_TEST(Test_S10_DecodeErrorCategory_Names);
    RUN_TEST(Test_S10_DecodeErrorCategory_SecurityFlags);
    RUN_TEST(Test_S13_GracefulDegradation_AllModes);
    RUN_TEST(Test_S13_GracefulDegradation_FaultInjection);
    RUN_TEST(Test_S14_ArchiveSecurityValidator_ZipBombRejection);
    RUN_TEST(Test_S14_ArchiveSecurityValidator_PathTraversalBlocked);
    RUN_TEST(Test_S14_ArchiveSecurityValidator_SymlinkAttackPrevented);

    // Security Hardening Tests
    std::wcout << L"\nSecurity Hardening Tests..." << std::endl;
    RUN_TEST(Test_SecureAllocator_ZeroOnFree);
    RUN_TEST(Test_SecureAllocator_SizeLimit);
    RUN_TEST(Test_InputValidator_PathTraversal);
    RUN_TEST(Test_InputValidator_NullBytes);
    RUN_TEST(Test_InputValidator_FileSizeLimit);
    RUN_TEST(Test_InputValidator_ImageDimensions);
    RUN_TEST(Test_InputValidator_ThumbnailSize);
    RUN_TEST(Test_MemorySafety_LeakDetection);

    // Sprint 69-88: Beyond Zenith Tests
    std::wcout << L"\nBeyond Zenith Tests..." << std::endl;
    RUN_TEST(Test_BZ_NeuralUpscaler_Init);
    RUN_TEST(Test_BZ_NeuralUpscaler_Quality);
    RUN_TEST(Test_BZ_PerceptualHash_Compute);
    RUN_TEST(Test_BZ_PerceptualHash_Similarity);
    RUN_TEST(Test_BZ_SemanticClassifier_Init);
    RUN_TEST(Test_BZ_SemanticClassifier_Classify);
    RUN_TEST(Test_BZ_ContentCompositor_Init);
    RUN_TEST(Test_BZ_ContentCompositor_Blend);
    RUN_TEST(Test_BZ_HDRMapper_Init);
    RUN_TEST(Test_BZ_HDRMapper_ToneMap);
    RUN_TEST(Test_BZ_VolumetricPreview_Init);
    RUN_TEST(Test_BZ_VolumetricPreview_Render);
    RUN_TEST(Test_BZ_CodecTranscoder_Init);
    RUN_TEST(Test_BZ_CodecTranscoder_Format);
    RUN_TEST(Test_BZ_DistributedRender_Init);
    RUN_TEST(Test_BZ_DistributedRender_Local);
    RUN_TEST(Test_BZ_QuantumHash_Init);
    RUN_TEST(Test_BZ_QuantumHash_Compute);
    RUN_TEST(Test_BZ_AdaptiveLOD_Init);
    RUN_TEST(Test_BZ_AdaptiveLOD_DPI);
    RUN_TEST(Test_BZ_OpenEXR_Init);
    RUN_TEST(Test_BZ_OpenEXR_Layers);
    RUN_TEST(Test_BZ_VDBVolume_Init);
    RUN_TEST(Test_BZ_VDBVolume_Format);
    RUN_TEST(Test_BZ_Alembic_Init);
    RUN_TEST(Test_BZ_Alembic_Ext);
    RUN_TEST(Test_BZ_MaterialX_Init);
    RUN_TEST(Test_BZ_MaterialX_Format);
    RUN_TEST(Test_BZ_PointCloud_Init);
    RUN_TEST(Test_BZ_PointCloud_Format);
    RUN_TEST(Test_BZ_Geospatial_Init);
    RUN_TEST(Test_BZ_Geospatial_Format);
    RUN_TEST(Test_BZ_HDRI_Init);
    RUN_TEST(Test_BZ_HDRI_Format);
    RUN_TEST(Test_BZ_NeuralRadiance_Init);
    RUN_TEST(Test_BZ_NeuralRadiance_Format);
    RUN_TEST(Test_BZ_Notation_Init);
    RUN_TEST(Test_BZ_Notation_Format);
    RUN_TEST(Test_BZ_PCBLayout_Init);
    RUN_TEST(Test_BZ_PCBLayout_Format);
    RUN_TEST(Test_BZ_GPUTensor_Init);
    RUN_TEST(Test_BZ_GPUTensor_Backend);
    RUN_TEST(Test_BZ_VulkanRT_Init);
    RUN_TEST(Test_BZ_VulkanRT_Settings);
    RUN_TEST(Test_BZ_DirectStorage_Init);
    RUN_TEST(Test_BZ_DirectStorage_Queue);
    RUN_TEST(Test_BZ_GPUDecomp_Init);
    RUN_TEST(Test_BZ_GPUDecomp_Fallback);
    RUN_TEST(Test_BZ_ShaderValidator_Init);
    RUN_TEST(Test_BZ_ShaderValidator_Check);
    RUN_TEST(Test_BZ_StreamMip_Init);
    RUN_TEST(Test_BZ_StreamMip_Config);
    RUN_TEST(Test_BZ_Prefetch_Init);
    RUN_TEST(Test_BZ_Prefetch_Config);
    RUN_TEST(Test_BZ_BatchCoalescer_Init);
    RUN_TEST(Test_BZ_BatchCoalescer_Adapt);
    RUN_TEST(Test_BZ_MorphTransition_Init);
    RUN_TEST(Test_BZ_MorphTransition_Config);
    RUN_TEST(Test_BZ_SemanticIndex_Init);
    RUN_TEST(Test_BZ_SemanticIndex_Query);
    RUN_TEST(Test_BZ_MemCompress_Init);
    RUN_TEST(Test_BZ_MemCompress_Config);
    RUN_TEST(Test_BZ_ZeroCost_Init);
    RUN_TEST(Test_BZ_ZeroCost_Verify);
    RUN_TEST(Test_BZ_BandwidthProfiler_Init);
    RUN_TEST(Test_BZ_BandwidthProfiler_Sample);
    RUN_TEST(Test_BZ_FaceDetect_Init);
    RUN_TEST(Test_BZ_FaceDetect_Config);
    RUN_TEST(Test_BZ_Saliency_Init);
    RUN_TEST(Test_BZ_Saliency_Config);
    RUN_TEST(Test_BZ_DepResolver_Init);
    RUN_TEST(Test_BZ_DepResolver_Cycle);
    RUN_TEST(Test_BZ_Telemetry_Init);
    RUN_TEST(Test_BZ_Telemetry_Config);

    // Sprint 349-393: Enhancement Plan V15 Tests
    std::wcout << L"\nEnhancement Plan V15 Tests..." << std::endl;
    RUN_TEST(Test_EP_MuPDFRenderer_Validate);
    RUN_TEST(Test_EP_MuPDFRenderer_Props);
    RUN_TEST(Test_EP_CRTLinkageValidator_Validate);
    RUN_TEST(Test_EP_CRTLinkageValidator_Props);
    RUN_TEST(Test_EP_LENSTypeRegistry_Validate);
    RUN_TEST(Test_EP_LENSTypeRegistry_Props);
    RUN_TEST(Test_EP_RegistryBatchHandler_Validate);
    RUN_TEST(Test_EP_RegistryBatchHandler_Props);
    RUN_TEST(Test_EP_PluginHostSupervisor_Validate);
    RUN_TEST(Test_EP_PluginHostSupervisor_Props);
    RUN_TEST(Test_EP_OpenJPEGRenderer_Validate);
    RUN_TEST(Test_EP_OpenJPEGRenderer_Props);
    RUN_TEST(Test_EP_FreeTypeRenderer_Validate);
    RUN_TEST(Test_EP_FreeTypeRenderer_Props);
    RUN_TEST(Test_EP_FFmpegExtractor_Validate);
    RUN_TEST(Test_EP_FFmpegExtractor_Props);
    RUN_TEST(Test_EP_InstructionSetRouter_Validate);
    RUN_TEST(Test_EP_InstructionSetRouter_Props);
    RUN_TEST(Test_EP_OwnerDrawThemeEngine_Validate);
    RUN_TEST(Test_EP_OwnerDrawThemeEngine_Props);
    RUN_TEST(Test_EP_AVX2ScaleKernel_Validate);
    RUN_TEST(Test_EP_AVX2ScaleKernel_Props);
    RUN_TEST(Test_EP_NEONScaleKernel_Validate);
    RUN_TEST(Test_EP_NEONScaleKernel_Props);
    RUN_TEST(Test_EP_ShaderLibraryManager_Validate);
    RUN_TEST(Test_EP_ShaderLibraryManager_Props);
    RUN_TEST(Test_EP_LanczosGPUScaler_Validate);
    RUN_TEST(Test_EP_LanczosGPUScaler_Props);
    RUN_TEST(Test_EP_BicubicResizeKernel_Validate);
    RUN_TEST(Test_EP_BicubicResizeKernel_Props);
    RUN_TEST(Test_EP_ACESTonemapKernel_Validate);
    RUN_TEST(Test_EP_ACESTonemapKernel_Props);
    RUN_TEST(Test_EP_ColorConvertKernel_Validate);
    RUN_TEST(Test_EP_ColorConvertKernel_Props);
    RUN_TEST(Test_EP_ZeroCopyActivator_Validate);
    RUN_TEST(Test_EP_ZeroCopyActivator_Props);
    RUN_TEST(Test_EP_ParallelIOScheduler_Validate);
    RUN_TEST(Test_EP_ParallelIOScheduler_Props);
    RUN_TEST(Test_EP_FolderWatchPredictor_Validate);
    RUN_TEST(Test_EP_FolderWatchPredictor_Props);
    RUN_TEST(Test_EP_ShellPropertyProvider_Validate);
    RUN_TEST(Test_EP_ShellPropertyProvider_Props);
    RUN_TEST(Test_EP_FFmpegCodecRouter_Validate);
    RUN_TEST(Test_EP_FFmpegCodecRouter_Props);
    RUN_TEST(Test_EP_SIMDMemoryAligner_Validate);
    RUN_TEST(Test_EP_SIMDMemoryAligner_Props);
    RUN_TEST(Test_EP_MemoryCompactionScheduler_Validate);
    RUN_TEST(Test_EP_MemoryCompactionScheduler_Props);
    RUN_TEST(Test_EP_DirectMLInferenceEngine_Validate);
    RUN_TEST(Test_EP_DirectMLInferenceEngine_Props);
    RUN_TEST(Test_EP_ONNXModelLoader_Validate);
    RUN_TEST(Test_EP_ONNXModelLoader_Props);
    RUN_TEST(Test_EP_DPIScalingManager_Validate);
    RUN_TEST(Test_EP_DPIScalingManager_Props);
    RUN_TEST(Test_EP_HighContrastAdapter_Validate);
    RUN_TEST(Test_EP_HighContrastAdapter_Props);
    RUN_TEST(Test_EP_FormatHealthIndicator_Validate);
    RUN_TEST(Test_EP_FormatHealthIndicator_Props);
    RUN_TEST(Test_EP_SettingsSerializer_Validate);
    RUN_TEST(Test_EP_SettingsSerializer_Props);
    RUN_TEST(Test_EP_LivePerformanceTracker_Validate);
    RUN_TEST(Test_EP_LivePerformanceTracker_Props);
    RUN_TEST(Test_EP_JPEG2000DecoderV2_Validate);
    RUN_TEST(Test_EP_JPEG2000DecoderV2_Props);
    RUN_TEST(Test_EP_UniversalVideoDecoder_Validate);
    RUN_TEST(Test_EP_UniversalVideoDecoder_Props);

    // CLI Tool Tests (Sprint 24 / v15.4.0 "Zenith-U")
    std::wcout << std::endl;
    std::wcout << L"  [CLI Tool Tests]" << std::endl;
    RUN_TEST(TestCLIRouterDispatch);
    RUN_TEST(TestCLIRouterUnknownCommand);
    RUN_TEST(TestCLIInfoDetectsFormatByExtension);
    RUN_TEST(TestCLICacheStatsPath);
    RUN_TEST(TestCLIRegisterDetectsAdminState);
    RUN_TEST(TestCLIBenchmarkOutputFormat);
    RUN_TEST(TestCLIDoctorAllChecks);

    // WASM Plugin Sandbox Tests (Sprint 561-570 / v25.1.0 "Rigel-R")
    std::wcout << L"WASM Plugin Sandbox Tests..." << std::endl;

    // Neural Format Intelligence Tests (Sprint 571-580 / v25.1.0 "Rigel-R")
    std::wcout << L"Neural Format Intelligence Tests..." << std::endl;
    RUN_TEST(Test_SelfExpandingFormatRegistry_Create);
    RUN_TEST(Test_SelfExpandingFormatRegistry_Register);
    RUN_TEST(Test_SelfExpandingFormatRegistry_Lookup);
    RUN_TEST(Test_SelfExpandingFormatRegistry_NotFound);
    RUN_TEST(Test_SelfExpandingFormatRegistry_Duplicate);
    RUN_TEST(Test_SelfExpandingFormatRegistry_Validate);
    RUN_TEST(Test_SelfExpandingFormatRegistry_Remove);
    RUN_TEST(Test_SelfExpandingFormatRegistry_Stats);
    RUN_TEST(Test_SelfExpandingFormatRegistry_PersistLoad);
    RUN_TEST(Test_FormatDetectionReport_Create);
    RUN_TEST(Test_FormatDetectionReport_AddVote);
    RUN_TEST(Test_FormatDetectionReport_Consensus);
    RUN_TEST(Test_FormatDetectionReport_Uncertain);
    RUN_TEST(Test_FormatDetectionReport_EvalTime);
    RUN_TEST(Test_FormatDetectionReport_IsResolved);
    RUN_TEST(Test_FormatDetectionReport_Probable);
    RUN_TEST(Test_FormatDetectionReport_Reset);
    RUN_TEST(Test_FormatDetectionReport_Sources);
    RUN_TEST(Test_SyntheticDecoderGenerator_Create);
    RUN_TEST(Test_SyntheticDecoderGenerator_Generate);
    RUN_TEST(Test_SyntheticDecoderGenerator_EmptyId);
    RUN_TEST(Test_SyntheticDecoderGenerator_Quality);
    RUN_TEST(Test_SyntheticDecoderGenerator_Partial);
    RUN_TEST(Test_SyntheticDecoderGenerator_InferFamily);
    RUN_TEST(Test_SyntheticDecoderGenerator_Fidelity);
    RUN_TEST(Test_SyntheticDecoderGenerator_Reset);
    RUN_TEST(Test_SyntheticDecoderGenerator_Families);

    // NPU & Heterogeneous Compute Tests (Sprint 581-590 / v25.2.0 "Rigel-S")
    std::wcout << L"NPU & Heterogeneous Compute Tests..." << std::endl;
    RUN_TEST(Test_IntelNPUBackend_Create);
    RUN_TEST(Test_IntelNPUBackend_Initialize);
    RUN_TEST(Test_IntelNPUBackend_Config);
    RUN_TEST(Test_IntelNPUBackend_Infer);
    RUN_TEST(Test_IntelNPUBackend_InferBadInput);
    RUN_TEST(Test_IntelNPUBackend_Metrics);
    RUN_TEST(Test_IntelNPUBackend_ResetMetrics);
    RUN_TEST(Test_IntelNPUBackend_SetConfig);
    RUN_TEST(Test_IntelNPUBackend_Shutdown);
    RUN_TEST(Test_HexagonDSPBackend_Create);
    RUN_TEST(Test_HexagonDSPBackend_Initialize);
    RUN_TEST(Test_HexagonDSPBackend_Config);
    RUN_TEST(Test_HexagonDSPBackend_RunInference);
    RUN_TEST(Test_HexagonDSPBackend_RunBadInput);
    RUN_TEST(Test_HexagonDSPBackend_RunCount);
    RUN_TEST(Test_HexagonDSPBackend_SetConfig);
    RUN_TEST(Test_HexagonDSPBackend_Reset);
    RUN_TEST(Test_HexagonDSPBackend_Variants);
    RUN_TEST(Test_ONNXEPRouter_Create);
    RUN_TEST(Test_ONNXEPRouter_RegisterEP);
    RUN_TEST(Test_ONNXEPRouter_IsEPAvailable);
    RUN_TEST(Test_ONNXEPRouter_Select);
    RUN_TEST(Test_ONNXEPRouter_SelectPreferred);
    RUN_TEST(Test_ONNXEPRouter_Clear);
    RUN_TEST(Test_ONNXEPRouter_Policy);
    RUN_TEST(Test_ONNXEPRouter_FallbackAllowed);
    RUN_TEST(Test_ONNXEPRouter_EPEnum);
    RUN_TEST(Test_HardwareCapabilityProfiler_Create);
    RUN_TEST(Test_HardwareCapabilityProfiler_Profile);
    RUN_TEST(Test_HardwareCapabilityProfiler_AddMock);
    RUN_TEST(Test_HardwareCapabilityProfiler_HasNPU);
    RUN_TEST(Test_HardwareCapabilityProfiler_PeakTOPS);
    RUN_TEST(Test_HardwareCapabilityProfiler_Sort);
    RUN_TEST(Test_HardwareCapabilityProfiler_ClearMocks);
    RUN_TEST(Test_HardwareCapabilityProfiler_AccelType);
    RUN_TEST(Test_HardwareCapabilityProfiler_BestType);
    RUN_TEST(Test_PowerAwareScheduler_Create);
    RUN_TEST(Test_PowerAwareScheduler_SetPowerState);
    RUN_TEST(Test_PowerAwareScheduler_ScheduleAC);
    RUN_TEST(Test_PowerAwareScheduler_ScheduleBatterySaver);
    RUN_TEST(Test_PowerAwareScheduler_ScheduledCount);
    RUN_TEST(Test_PowerAwareScheduler_NPUUnavailable);
    RUN_TEST(Test_PowerAwareScheduler_Reset);
    RUN_TEST(Test_PowerAwareScheduler_Mode);
    RUN_TEST(Test_PowerAwareScheduler_Reason);
    RUN_TEST(Test_NPUMemoryPool_Create);
    RUN_TEST(Test_NPUMemoryPool_Initialize);
    RUN_TEST(Test_NPUMemoryPool_Acquire);
    RUN_TEST(Test_NPUMemoryPool_Release);
    RUN_TEST(Test_NPUMemoryPool_AcquireExhausted);
    RUN_TEST(Test_NPUMemoryPool_ZeroCopy);
    RUN_TEST(Test_NPUMemoryPool_Reset);
    RUN_TEST(Test_NPUMemoryPool_Config);
    RUN_TEST(Test_NPUMemoryPool_BlockCount);
    RUN_TEST(Test_ARM64DecodeBackend_Create);
    RUN_TEST(Test_ARM64DecodeBackend_ProbeCapabilities);
    RUN_TEST(Test_ARM64DecodeBackend_DecodeStride_Valid);
    RUN_TEST(Test_ARM64DecodeBackend_DecodeStride_Null);
    RUN_TEST(Test_ARM64DecodeBackend_DecodeStride_Zero);
    RUN_TEST(Test_ARM64DecodeBackend_GetMode);
    RUN_TEST(Test_ARM64DecodeBackend_SetConfig);
    RUN_TEST(Test_ARM64DecodeBackend_Reset);
    RUN_TEST(Test_ARM64DecodeBackend_Extensions);

    // VCS Integration Tests (Sprint 591-600 / v25.3.0 "Rigel-T")
    std::wcout << L"VCS Integration Tests..." << std::endl;
    RUN_TEST(Test_GitStatusOverlay_Create);
    RUN_TEST(Test_GitStatusOverlay_Query);
    RUN_TEST(Test_GitStatusOverlay_StatusLabel_M);
    RUN_TEST(Test_GitStatusOverlay_StatusLabel_S);
    RUN_TEST(Test_GitStatusOverlay_StatusLabel_Untracked);
    RUN_TEST(Test_GitStatusOverlay_StatusLabel_Conflict);
    RUN_TEST(Test_GitStatusOverlay_ShouldRender_Clean);
    RUN_TEST(Test_GitStatusOverlay_Config);
    RUN_TEST(Test_GitStatusOverlay_StatusEnum);
    RUN_TEST(Test_GitBlameHeatmapOverlay_Create);
    RUN_TEST(Test_GitBlameHeatmapOverlay_ComputeScore_Recent);
    RUN_TEST(Test_GitBlameHeatmapOverlay_ComputeScore_Old);
    RUN_TEST(Test_GitBlameHeatmapOverlay_ComputeScore_Clamped);
    RUN_TEST(Test_GitBlameHeatmapOverlay_ColorScheme);
    RUN_TEST(Test_GitBlameHeatmapOverlay_Config);
    RUN_TEST(Test_GitBlameHeatmapOverlay_ScoreRange);
    RUN_TEST(Test_GitBlameHeatmapOverlay_ScoreMiddle);
    RUN_TEST(Test_GitBlameHeatmapOverlay_ComputeHeat);
    RUN_TEST(Test_VCSBadgeAdapter_Create);
    RUN_TEST(Test_VCSBadgeAdapter_Build_Empty);
    RUN_TEST(Test_VCSBadgeAdapter_Build_Path);
    RUN_TEST(Test_VCSBadgeAdapter_Config);
    RUN_TEST(Test_VCSBadgeAdapter_ProviderEnum);
    RUN_TEST(Test_VCSBadgeAdapter_BadgeTypeEnum);
    RUN_TEST(Test_VCSBadgeAdapter_DetectProvider);
    RUN_TEST(Test_VCSBadgeAdapter_DefaultBadgeType);
    RUN_TEST(Test_VCSBadgeAdapter_DefaultProvider);
    RUN_TEST(Test_GitDiffThumbnail_Create);
    RUN_TEST(Test_GitDiffThumbnail_Analyze_Empty);
    RUN_TEST(Test_GitDiffThumbnail_Analyze_Valid);
    RUN_TEST(Test_GitDiffThumbnail_Config);
    RUN_TEST(Test_GitDiffThumbnail_DiffViewMode);
    RUN_TEST(Test_GitDiffThumbnail_AfterCommit);
    RUN_TEST(Test_GitDiffThumbnail_BeforeCommit);
    RUN_TEST(Test_GitDiffThumbnail_FilePath);
    RUN_TEST(Test_GitDiffThumbnail_ShowStats);
    RUN_TEST(Test_GitLFSResolver_Create);
    RUN_TEST(Test_GitLFSResolver_ParsePointer_Valid);
    RUN_TEST(Test_GitLFSResolver_ParsePointer_Invalid);
    RUN_TEST(Test_GitLFSResolver_Resolve);
    RUN_TEST(Test_GitLFSResolver_StatusEnum);
    RUN_TEST(Test_GitLFSResolver_Config);
    RUN_TEST(Test_GitLFSResolver_IsLFSPointer);
    RUN_TEST(Test_GitLFSResolver_OidExtracted);
    RUN_TEST(Test_GitLFSResolver_ResultSuccess);
    RUN_TEST(Test_CommitBadgeCompositor_Create);
    RUN_TEST(Test_CommitBadgeCompositor_Build_Valid);
    RUN_TEST(Test_CommitBadgeCompositor_Build_Empty);
    RUN_TEST(Test_CommitBadgeCompositor_FormatAge_Now);
    RUN_TEST(Test_CommitBadgeCompositor_FormatAge_Days);
    RUN_TEST(Test_CommitBadgeCompositor_FormatAge_Weeks);
    RUN_TEST(Test_CommitBadgeCompositor_BadgePosition_Enum);
    RUN_TEST(Test_CommitBadgeCompositor_BadgeSize_Enum);
    RUN_TEST(Test_CommitBadgeCompositor_ShortHash);
    RUN_TEST(Test_MergeConflictOverlay_Create);
    RUN_TEST(Test_MergeConflictOverlay_AnalyzeContent_Clean);
    RUN_TEST(Test_MergeConflictOverlay_AnalyzeContent_Conflict);
    RUN_TEST(Test_MergeConflictOverlay_Analyze_Empty);
    RUN_TEST(Test_MergeConflictOverlay_Analyze_Path);
    RUN_TEST(Test_MergeConflictOverlay_ShouldRender);
    RUN_TEST(Test_MergeConflictOverlay_Config);
    RUN_TEST(Test_MergeConflictOverlay_ConflictStateEnum);
    RUN_TEST(Test_MergeConflictOverlay_MarkerCount);

    // Sprint 601-610 — Self-Healing & Adaptive Recovery (v25.4.0)
    RUN_TEST(Test_DecoderCrashPredictor_Create);
    RUN_TEST(Test_DecoderCrashPredictor_Predict_NoSamples);
    RUN_TEST(Test_DecoderCrashPredictor_Record_And_Count);
    RUN_TEST(Test_DecoderCrashPredictor_HighRisk);
    RUN_TEST(Test_DecoderCrashPredictor_Reset);
    RUN_TEST(Test_DecoderCrashPredictor_ResetAll);
    RUN_TEST(Test_DecoderCrashPredictor_RiskLevelEnum);
    RUN_TEST(Test_DecoderQuarantineManager_Create);
    RUN_TEST(Test_DecoderQuarantineManager_RecordCrash_Threshold1);
    RUN_TEST(Test_DecoderQuarantineManager_RecordCrash_Bypass);
    RUN_TEST(Test_DecoderQuarantineManager_Recovery);
    RUN_TEST(Test_DecoderQuarantineManager_StageName);
    RUN_TEST(Test_AdaptiveTimeoutTuner_DefaultTimeout);
    RUN_TEST(Test_AdaptiveTimeoutTuner_RecordAndLearn);
    RUN_TEST(Test_AdaptiveTimeoutTuner_Clamp);
    RUN_TEST(Test_AdaptiveTimeoutTuner_SetBaseline);
    RUN_TEST(Test_AdaptiveTimeoutTuner_Reset);
    RUN_TEST(Test_HeapCorruptionSentinel_Allocate);
    RUN_TEST(Test_HeapCorruptionSentinel_CheckClean);
    RUN_TEST(Test_HeapCorruptionSentinel_ScanAll_Clean);
    RUN_TEST(Test_HeapCorruptionSentinel_DetectHeadCorrupt);
    RUN_TEST(Test_RetryPolicyEngine_SuccessFirstTry);
    RUN_TEST(Test_RetryPolicyEngine_ExhaustedRetries);
    RUN_TEST(Test_RetryPolicyEngine_Config);
    RUN_TEST(Test_RetryPolicyEngine_ResetStats);
    RUN_TEST(Test_DecoderIncidentReporter_Healthy);
    RUN_TEST(Test_DecoderIncidentReporter_Critical);
    RUN_TEST(Test_DecoderIncidentReporter_BatchReport);
    RUN_TEST(Test_COMSelfRepairValidator_ClsidConst);
    RUN_TEST(Test_COMSelfRepairValidator_IsRegistered);
    RUN_TEST(Test_COMSelfRepairValidator_ClsidPath);
    RUN_TEST(Test_COMSelfRepairValidator_Validate);
    RUN_TEST(Test_BootIntegritySelfTest_Instance);
    RUN_TEST(Test_BootIntegritySelfTest_RunAll);
    RUN_TEST(Test_BootIntegritySelfTest_AddTest);
    RUN_TEST(Test_BootIntegritySelfTest_FailingTest);

    // Sprint 611-620 — Multi-Instance & Virtual Desktop (v25.5.0)
    RUN_TEST(Test_VirtualDesktopAwareness_Instance);
    RUN_TEST(Test_VirtualDesktopAwareness_BuildCacheKeyPrefix_Global);
    RUN_TEST(Test_VirtualDesktopAwareness_BuildCacheKeyPrefix_PerDesktop);
    RUN_TEST(Test_VirtualDesktopAwareness_RegisterDesktop);
    RUN_TEST(Test_WTSSessionIsolation_Instance);
    RUN_TEST(Test_WTSSessionIsolation_GetCacheScope);
    RUN_TEST(Test_WTSSessionIsolation_RegisterSession);
    RUN_TEST(Test_WTSSessionIsolation_StateName);
    RUN_TEST(Test_PerMonitorDPISelectorV2_DefaultFallback);
    RUN_TEST(Test_PerMonitorDPISelectorV2_HighDPI);
    RUN_TEST(Test_PerMonitorDPISelectorV2_MixedDPI);
    RUN_TEST(Test_TabbedExplorerSync_Instance);
    RUN_TEST(Test_TabbedExplorerSync_RegisterTab);
    RUN_TEST(Test_TabbedExplorerSync_SyncSameFolder);
    RUN_TEST(Test_CrossSessionThumbnailPool_InsertAndLookup);
    RUN_TEST(Test_CrossSessionThumbnailPool_Stats);
    RUN_TEST(Test_CrossSessionThumbnailPool_HitRate);
    RUN_TEST(Test_InstanceRegistry_Register);
    RUN_TEST(Test_InstanceRegistry_Heartbeat);
    RUN_TEST(Test_InstanceRegistry_Unregister);
    RUN_TEST(Test_ForegroundPriorityInheritance_Instance);
    RUN_TEST(Test_ForegroundPriorityInheritance_Evaluate_Foreground);
    RUN_TEST(Test_ForegroundPriorityInheritance_Evaluate_Background);
    RUN_TEST(Test_CrossInstanceLoadBalancer_Dispatch_NoInstances);
    RUN_TEST(Test_CrossInstanceLoadBalancer_Dispatch_LeastConn);
    RUN_TEST(Test_CrossInstanceLoadBalancer_MarkTaskEvents);

    // Sprint 621-630 — Collaborative Annotations (v25.6.0)
    RUN_TEST(Test_AnnotationStore_AddAndCount);
    RUN_TEST(Test_AnnotationStore_GetForFile);
    RUN_TEST(Test_AnnotationStore_Delete);
    RUN_TEST(Test_AnnotationStore_DirtyFlag);
    RUN_TEST(Test_AnnotationStore_AnnotationTypeNames);
    RUN_TEST(Test_AnnotationOverlayRenderer_NoAnnotations);
    RUN_TEST(Test_AnnotationOverlayRenderer_WithStars);
    RUN_TEST(Test_AnnotationOverlayRenderer_Config);
    RUN_TEST(Test_CollabWebhookBridge_NoConfigs);
    RUN_TEST(Test_CollabWebhookBridge_BuildPayload_Teams);
    RUN_TEST(Test_CollabWebhookBridge_BuildPayload_Slack);
    RUN_TEST(Test_CollabWebhookBridge_InjectableHTTP);
    RUN_TEST(Test_SharedCollectionBuilder_CreateCollection);
    RUN_TEST(Test_SharedCollectionBuilder_AddItem);
    RUN_TEST(Test_SharedCollectionBuilder_FindByToken);
    RUN_TEST(Test_SharedCollectionBuilder_MergeCollection);
    RUN_TEST(Test_AnnotationDiffViewer_NoDiff);
    RUN_TEST(Test_AnnotationDiffViewer_StarChanged);
    RUN_TEST(Test_AnnotationDiffViewer_TagAdded);
    RUN_TEST(Test_AnnotationDiffViewer_DiffOpName);
    RUN_TEST(Test_AnnotationExporter_JSON);
    RUN_TEST(Test_AnnotationExporter_XML);
    RUN_TEST(Test_AnnotationExporter_CSV);
    RUN_TEST(Test_AnnotationExporter_FormatName);
    RUN_TEST(Test_CollabCloudSync_NoHttpFn);
    RUN_TEST(Test_CollabCloudSync_EmptyToken);
    RUN_TEST(Test_CollabCloudSync_SuccessfulSync);
    RUN_TEST(Test_AnnotationSchemaMigrator_AlreadyCurrent);
    RUN_TEST(Test_AnnotationSchemaMigrator_V1ToV4);
    RUN_TEST(Test_AnnotationSchemaMigrator_CurrentVersion);

    // Sprint 631-640 — Protocol Surface & API Ecosystem (v25.7.0)
    RUN_TEST(Test_GRPCThumbnailService_Create);
    RUN_TEST(Test_GRPCThumbnailService_StartStop);
    RUN_TEST(Test_GRPCThumbnailService_HandleRequest);
    RUN_TEST(Test_GRPCThumbnailService_ServiceName);
    RUN_TEST(Test_GRPCThumbnailService_Config);
    RUN_TEST(Test_RESTThumbnailServer_Create);
    RUN_TEST(Test_RESTThumbnailServer_StartStop);
    RUN_TEST(Test_RESTThumbnailServer_Dispatch_404);
    RUN_TEST(Test_GraphQLQueryEngine_Execute_NoResolver);
    RUN_TEST(Test_GraphQLQueryEngine_Introspection);
    RUN_TEST(Test_WebSocketPushChannel_Create);
    RUN_TEST(Test_WebSocketPushChannel_AddRemoveClient);
    RUN_TEST(Test_WebSocketPushChannel_Broadcast);
    RUN_TEST(Test_OpenAPISpecGenerator_GenerateYAML);
    RUN_TEST(Test_OpenAPISpecGenerator_GenerateJSON);
    RUN_TEST(Test_SDKBindingsGenerator_CSharp);
    RUN_TEST(Test_SDKBindingsGenerator_Python);
    RUN_TEST(Test_OAuthTokenValidator_ValidToken);
    RUN_TEST(Test_OAuthTokenValidator_MalformedToken);
    RUN_TEST(Test_OAuthTokenValidator_EmptyToken);
    RUN_TEST(Test_OAuthTokenValidator_InjectableClock);
    RUN_TEST(Test_APIRateLimiter_Allow);
    RUN_TEST(Test_APIRateLimiter_Throttle);
    RUN_TEST(Test_APIRateLimiter_ResetClient);
    RUN_TEST(Test_APIRateLimiter_SlidingWindow);

    // Sprint 641-650 — Post-Quantum Security (v26.0.0)
    RUN_TEST(Test_MLKEMKeyEncapsulator_GenerateKeyPair);
    RUN_TEST(Test_MLKEMKeyEncapsulator_Encapsulate);
    RUN_TEST(Test_MLKEMKeyEncapsulator_Decapsulate);
    RUN_TEST(Test_MLKEMKeyEncapsulator_LevelName);
    RUN_TEST(Test_MLKEMKeyEncapsulator_InvalidPubKey);
    RUN_TEST(Test_SLHDSASignatureVerifier_Create);
    RUN_TEST(Test_SLHDSASignatureVerifier_SignAndVerify);
    RUN_TEST(Test_SLHDSASignatureVerifier_InvalidSignature);
    RUN_TEST(Test_HybridTLSIPCChannel_Create);
    RUN_TEST(Test_HybridTLSIPCChannel_Connect);
    RUN_TEST(Test_HybridTLSIPCChannel_Send);
    RUN_TEST(Test_HybridTLSIPCChannel_Disconnect);
    RUN_TEST(Test_HybridTLSIPCChannel_ModeName);
    RUN_TEST(Test_PQAuditTrail_AddAndVerify);
    RUN_TEST(Test_PQAuditTrail_MultiEntry);
    RUN_TEST(Test_QuantumSafeKeyRotator_NeedsRotation);
    RUN_TEST(Test_QuantumSafeKeyRotator_RotateCallback);
    RUN_TEST(Test_FIPS140CryptoBoundary_ApprovedAlgos);
    RUN_TEST(Test_FIPS140CryptoBoundary_AlgorithmCount);
    RUN_TEST(Test_CertificateMigrationTool_CreatePlan);
    RUN_TEST(Test_CertificateMigrationTool_PlanHasDualSign);
    RUN_TEST(Test_CryptoAgilityEngine_AlgorithmCount);
    RUN_TEST(Test_CryptoAgilityEngine_Negotiate_HybridPref);
    RUN_TEST(Test_CryptoAgilityEngine_Negotiate_PQOnly);
    RUN_TEST(Test_CryptoAgilityEngine_FIPSFilter);

    // Sprint 651-660 — Windows Next-Gen Shell (v26.1.0)
    RUN_TEST(Test_WinRTThumbnailBridge_NoHandler);
    RUN_TEST(Test_WinRTThumbnailBridge_WithHandler);
    RUN_TEST(Test_WinRTThumbnailBridge_ModeName);
    RUN_TEST(Test_WinRTThumbnailBridge_EmptyPath);
    RUN_TEST(Test_AppContainerIsolation_Create);
    RUN_TEST(Test_AppContainerIsolation_GrantRevoke);
    RUN_TEST(Test_WinFSMetadataStore_Create);
    RUN_TEST(Test_WinFSMetadataStore_SetAndGet);
    RUN_TEST(Test_WindowsSearchV3Bridge_IndexAndQuery);
    RUN_TEST(Test_SmartAppControlPolicy_Modes);
    RUN_TEST(Test_SmartAppControlPolicy_TrustedPublisher);
    RUN_TEST(Test_MSIXStreamingPrewarmer_Create);
    RUN_TEST(Test_MSIXStreamingPrewarmer_Prewarm);
    RUN_TEST(Test_WindowsHelloAuthBridge_Create);
    RUN_TEST(Test_WindowsHelloAuthBridge_InjectableAuth);

    // Sprint 661-670 — Immersive 3D Preview Engine (v26.2.0)
    RUN_TEST(Test_ImmersivePreviewRenderer_EmptyPath);
    RUN_TEST(Test_ImmersivePreviewRenderer_Render);
    RUN_TEST(Test_ImmersivePreviewRenderer_QualityNames);
    RUN_TEST(Test_ImmersivePreviewRenderer_RenderTimes);
    RUN_TEST(Test_VolumetricThumbnailEngine_Create);
    RUN_TEST(Test_VolumetricThumbnailEngine_Render);
    RUN_TEST(Test_RealtimeLightingSimulator_Create);
    RUN_TEST(Test_RealtimeLightingSimulator_AddLight);
    RUN_TEST(Test_HolographicProjectionEngine_Create);
    RUN_TEST(Test_HolographicProjectionEngine_ProjectStereo);
    RUN_TEST(Test_MeshLODGeneratorV2_Create);
    RUN_TEST(Test_MeshLODGeneratorV2_GenerateLOD);
    RUN_TEST(Test_AnimationPreviewScrubber_Create);
    RUN_TEST(Test_AnimationPreviewScrubber_PickFrame);
    RUN_TEST(Test_MaterialPreviewEngine_DetectFormat);
    RUN_TEST(Test_MaterialPreviewEngine_Render);
    RUN_TEST(Test_GPUPathTracerPreview_Create);
    RUN_TEST(Test_GPUPathTracerPreview_SamplesForQuality);
    RUN_TEST(Test_GPUPathTracerPreview_Trace);

    // Sprint 671-680 — Real-Time Collaboration (v26.3.0)
    RUN_TEST(Test_CollaborationPresenceEngine_JoinLeave);
    RUN_TEST(Test_CollaborationPresenceEngine_Snapshot);
    RUN_TEST(Test_CollaborationPresenceEngine_StateNames);
    RUN_TEST(Test_CollaborationPresenceEngine_UpdateCursor);
    RUN_TEST(Test_LiveAnnotationBroadcaster_Broadcast);
    RUN_TEST(Test_LiveAnnotationBroadcaster_LamportClock);
    RUN_TEST(Test_LiveAnnotationBroadcaster_OpName);
    RUN_TEST(Test_SharedViewStateProtocol_Publish);
    RUN_TEST(Test_SharedViewStateProtocol_Apply);
    RUN_TEST(Test_SharedViewStateProtocol_Callback);
    RUN_TEST(Test_ConflictResolutionMerger_LWW);
    RUN_TEST(Test_ConflictResolutionMerger_OwnerPriority);
    RUN_TEST(Test_ConflictResolutionMerger_StrategyName);
    RUN_TEST(Test_PresenceAvatarRenderer_Render_Empty);
    RUN_TEST(Test_PresenceAvatarRenderer_Render_WithParticipants);
    RUN_TEST(Test_SessionReplayEngine_RecordAndPlay);
    RUN_TEST(Test_SessionReplayEngine_SpeedFactor);
    RUN_TEST(Test_SessionReplayEngine_Callback);
    RUN_TEST(Test_CollaborationTelemetryHub_Record);
    RUN_TEST(Test_CollaborationTelemetryHub_ConflictRate);
    RUN_TEST(Test_CollaborationTelemetryHub_EventName);

    // Sprint 681-690 — Adaptive Performance Governor v2 (v26.4.0)
    RUN_TEST(Test_AdaptivePerformanceGovernorV2_BalancedMode);
    RUN_TEST(Test_AdaptivePerformanceGovernorV2_ThermalThrottle);
    RUN_TEST(Test_AdaptivePerformanceGovernorV2_ModeName);
    RUN_TEST(Test_AdaptivePerformanceGovernorV2_EvalCount);
    RUN_TEST(Test_ThermalAwareMemoryScheduler_Comfortable);
    RUN_TEST(Test_ThermalAwareMemoryScheduler_Hot);
    RUN_TEST(Test_ThermalAwareMemoryScheduler_Critical);
    RUN_TEST(Test_ThermalAwareMemoryScheduler_ZoneName);
    RUN_TEST(Test_WorkloadBalancerV2_Dispatch);
    RUN_TEST(Test_WorkloadBalancerV2_Complete);
    RUN_TEST(Test_WorkloadBalancerV2_AlgorithmName);
    RUN_TEST(Test_PowerBudgetController_AC_Unconstrained);
    RUN_TEST(Test_PowerBudgetController_Battery_Critical);
    RUN_TEST(Test_PowerBudgetController_StateName);
    RUN_TEST(Test_QoSThrottleEngine_Interactive_Allowed);
    RUN_TEST(Test_QoSThrottleEngine_Background_Throttled);
    RUN_TEST(Test_QoSThrottleEngine_Idle);
    RUN_TEST(Test_SmartPrefetchEngine_Predict);
    RUN_TEST(Test_SmartPrefetchEngine_IssuePrefetches);
    RUN_TEST(Test_SmartPrefetchEngine_HitMissStats);
    RUN_TEST(Test_FrameRateSynchronizer_TargetFrameTime);
    RUN_TEST(Test_FrameRateSynchronizer_PresentFrame);
    RUN_TEST(Test_FrameRateSynchronizer_SetMode);
    RUN_TEST(Test_FrameRateSynchronizer_ModeName);
    RUN_TEST(Test_BackgroundIntelligenceService_Enqueue);
    RUN_TEST(Test_BackgroundIntelligenceService_DrainNext);
    RUN_TEST(Test_BackgroundIntelligenceService_IdleThreshold);

    // Sprint 691-700 — Global I18n & Accessibility v3 (v26.5.0)
    RUN_TEST(Test_I18nRuntimeEngine_DefaultLocale);
    RUN_TEST(Test_I18nRuntimeEngine_RegisterAndTranslate);
    RUN_TEST(Test_I18nRuntimeEngine_FallbackToKey);
    RUN_TEST(Test_I18nRuntimeEngine_RTLDirection);
    RUN_TEST(Test_BiDiTextLayoutEngine_LTR);
    RUN_TEST(Test_BiDiTextLayoutEngine_ContainsRTL);
    RUN_TEST(Test_BiDiTextLayoutEngine_EmptyText);
    RUN_TEST(Test_BiDiTextLayoutEngine_ExplicitRTL);
    RUN_TEST(Test_AccessibilityNavigatorV3_RegisterAndFind);
    RUN_TEST(Test_AccessibilityNavigatorV3_Invoke);
    RUN_TEST(Test_AccessibilityNavigatorV3_NotFound);
    RUN_TEST(Test_AccessibilityNavigatorV3_ControlTypeName);
    RUN_TEST(Test_ScreenReaderBridgeV2_NotAvailable);
    RUN_TEST(Test_ScreenReaderBridgeV2_WithFn);
    RUN_TEST(Test_ScreenReaderBridgeV2_ReaderName);
    RUN_TEST(Test_LocaleFallbackResolver_ExactMatch);
    RUN_TEST(Test_LocaleFallbackResolver_FallbackChain);
    RUN_TEST(Test_LocaleFallbackResolver_BuildChain);
    RUN_TEST(Test_A11yColorContrastEngine_Pass_AA);
    RUN_TEST(Test_A11yColorContrastEngine_Fail_AAA);
    RUN_TEST(Test_A11yColorContrastEngine_SuggestForeground);
    RUN_TEST(Test_A11yColorContrastEngine_RequiredRatio);
    RUN_TEST(Test_KeyboardNavigationMapV2_AddAndNavigate);
    RUN_TEST(Test_KeyboardNavigationMapV2_TabOrder);
    RUN_TEST(Test_KeyboardNavigationMapV2_WrapAround);
    RUN_TEST(Test_AccessibilityAuditPipeline_Compliant);
    RUN_TEST(Test_AccessibilityAuditPipeline_Noncompliant);
    RUN_TEST(Test_AccessibilityAuditPipeline_CriterionName);
    RUN_TEST(Test_AccessibilityAuditPipeline_PassRate);

    // Sprint 701-800 Tests
    RUN_TEST(Test_OpenXRAssetDecoder_DetectFormat);
    RUN_TEST(Test_OpenXRAssetDecoder_Decode);
    RUN_TEST(Test_OpenXRAssetDecoder_EmptyPath);
    RUN_TEST(Test_OpenXRAssetDecoder_IsSupported);
    RUN_TEST(Test_USDADecoder_DetectLayerType);
    RUN_TEST(Test_USDADecoder_Decode);
    RUN_TEST(Test_USDADecoder_EmptyPath);
    RUN_TEST(Test_XRSpatialPreviewEngine_Preview);
    RUN_TEST(Test_XRSpatialPreviewEngine_TargetName);
    RUN_TEST(Test_XRSpatialPreviewEngine_EmptyPath);
    RUN_TEST(Test_ARMarkerDetectionEngine_Detect);
    RUN_TEST(Test_ARMarkerDetectionEngine_EmptyImage);
    RUN_TEST(Test_ARMarkerDetectionEngine_TypeName);
    RUN_TEST(Test_StereoscopicRenderPipeline_Render);
    RUN_TEST(Test_StereoscopicRenderPipeline_EmptyPath);
    RUN_TEST(Test_StereoscopicRenderPipeline_LayoutName);
    RUN_TEST(Test_PointCloudVisualizerV2_DetectFormat);
    RUN_TEST(Test_PointCloudVisualizerV2_Render);
    RUN_TEST(Test_PointCloudVisualizerV2_EmptyPath);
    RUN_TEST(Test_NerfDecoder_DetectFormat);
    RUN_TEST(Test_NerfDecoder_Decode);
    RUN_TEST(Test_XRMetadataExtractor_Extract);
    RUN_TEST(Test_XRMetadataExtractor_SupportsFormat);
    RUN_TEST(Test_DifferentialPrivacyEngine_Query);
    RUN_TEST(Test_DifferentialPrivacyEngine_BudgetExhausted);
    RUN_TEST(Test_DifferentialPrivacyEngine_ResetBudget);
    RUN_TEST(Test_LocalDataAggregator_AddAndFlush);
    RUN_TEST(Test_LocalDataAggregator_SetNoiseScale);
    RUN_TEST(Test_AnonymizationPipelineV2_Anonymize);
    RUN_TEST(Test_AnonymizationPipelineV2_ContainsPII);
    RUN_TEST(Test_AnonymizationPipelineV2_Batch);
    RUN_TEST(Test_PrivacyConsentManager_SetGet);
    RUN_TEST(Test_PrivacyConsentManager_DefaultPending);
    RUN_TEST(Test_PrivacyConsentManager_AuditTrail);
    RUN_TEST(Test_SecureEnclaveAnalytics_Aggregate);
    RUN_TEST(Test_SecureEnclaveAnalytics_EmptyInput);
    RUN_TEST(Test_SecureEnclaveAnalytics_BackendName);
    RUN_TEST(Test_GDPRComplianceEngine_Erasure);
    RUN_TEST(Test_GDPRComplianceEngine_EmptySubject);
    RUN_TEST(Test_PrivacyAuditLogger_RecordAndVerify);
    RUN_TEST(Test_PrivacyAuditLogger_EntriesForSubject);
    RUN_TEST(Test_LiveStreamDecoder_DecodeFirstFrame);
    RUN_TEST(Test_LiveStreamDecoder_EmptyUrl);
    RUN_TEST(Test_LiveStreamDecoder_ProtocolName);
    RUN_TEST(Test_WebRTCThumbnailCapture_Capture);
    RUN_TEST(Test_WebRTCThumbnailCapture_NoSDP);
    RUN_TEST(Test_WebRTCThumbnailCapture_CodecSupport);
    RUN_TEST(Test_StreamingBufferOrchestrator_PushPop);
    RUN_TEST(Test_StreamingBufferOrchestrator_Status);
    RUN_TEST(Test_AdaptiveBitrateSelector_Select);
    RUN_TEST(Test_AdaptiveBitrateSelector_NoBandwidth);
    RUN_TEST(Test_MediaTimelineRenderer_RenderStrip);
    RUN_TEST(Test_MediaTimelineRenderer_EmptyPath);
    RUN_TEST(Test_DASHStreamDecoder_Decode);
    RUN_TEST(Test_DASHStreamDecoder_IsManifestUrl);
    RUN_TEST(Test_LiveThumbnailPoller_AddRemove);
    RUN_TEST(Test_LiveThumbnailPoller_MaxConcurrent);
    RUN_TEST(Test_VideoTextureStreamEngine_Upload);
    RUN_TEST(Test_VideoTextureStreamEngine_EmptyFrame);
    RUN_TEST(Test_VideoTextureStreamEngine_ZeroCopy);
    RUN_TEST(Test_RenderClusterManager_SubmitComplete);
    RUN_TEST(Test_RenderClusterManager_NoIdleNodes);
    RUN_TEST(Test_RenderJobScheduler_EnqueueDequeue);
    RUN_TEST(Test_RenderJobScheduler_Fail);
    RUN_TEST(Test_NodeHealthMonitor_HeartbeatAndReport);
    RUN_TEST(Test_NodeHealthMonitor_Unresponsive);
    RUN_TEST(Test_RenderResultAggregator_Compose);
    RUN_TEST(Test_RenderResultAggregator_NoTiles);
    RUN_TEST(Test_ClusterAutoScaler_ScaleUp);
    RUN_TEST(Test_ClusterAutoScaler_ScaleDown);
    RUN_TEST(Test_SecureClusterChannel_Handshake);
    RUN_TEST(Test_SecureClusterChannel_Send);
    RUN_TEST(Test_ClusterObservabilityBus_RecordAndSnapshot);
    RUN_TEST(Test_ClusterObservabilityBus_QueryByNode);
    RUN_TEST(Test_PluginSandboxV3_Execute);
    RUN_TEST(Test_PluginSandboxV3_MemoryLimit);
    RUN_TEST(Test_PluginCompatibilityShimV3_Load);
    RUN_TEST(Test_PluginCompatibilityShimV3_EmptyPath);
    RUN_TEST(Test_FolderPredictionModel_TopPredictions);
    RUN_TEST(Test_FolderPredictionModel_TotalAccesses);
    RUN_TEST(Test_ColdStartFolderBootstrapper_Bootstrap);
    RUN_TEST(Test_PredictionScanOrchestrator_EnqueueDrain);
    RUN_TEST(Test_PredictionScanOrchestrator_Cancel);
    RUN_TEST(Test_DMADirectPreloader_Preload);
    RUN_TEST(Test_DMADirectPreloader_EmptyPath);
    RUN_TEST(Test_PerUserPredictionIsolator_RegisterTest);
    RUN_TEST(Test_PerUserPredictionIsolator_SlotCollision);
    RUN_TEST(Test_PredictionAccuracyTracker_Compute);
    RUN_TEST(Test_PredictionAccuracyTracker_Empty);
    RUN_TEST(Test_CollaborativeAnnotationEngineV2_ApplyAndMerge);
    RUN_TEST(Test_CollaborativeAnnotationEngineV2_Clock);
    RUN_TEST(Test_AnnotationSignatureVerifier_SignAndVerify);
    RUN_TEST(Test_AnnotationSignatureVerifier_UnknownAuthor);
    RUN_TEST(Test_AnnotationTimeline_AddAndRevert);
    RUN_TEST(Test_AnnotationTimeline_GetDelta);
    RUN_TEST(Test_PresenceIndicatorEngine_UpdateAndQuery);
    RUN_TEST(Test_PresenceIndicatorEngine_SetOffline);
    RUN_TEST(Test_AnnotationTaxonomyV2_LookupAndChildren);
    RUN_TEST(Test_AnnotationTaxonomyV2_LabelCount);
    RUN_TEST(Test_OfflineAnnotationSyncQueue_EnqueueFlush);
    RUN_TEST(Test_OfflineAnnotationSyncQueue_Clear);
    RUN_TEST(Test_AnnotationExportPipelineV2_Export);
    RUN_TEST(Test_AnnotationExportPipelineV2_FormatName);
    RUN_TEST(Test_GRPCProtocolServerV2_DispatchHandler);
    RUN_TEST(Test_GRPCProtocolServerV2_UnknownMethod);
    RUN_TEST(Test_RESTAPIServerV2_RouteAndDispatch);
    RUN_TEST(Test_RESTAPIServerV2_NotFound);
    RUN_TEST(Test_GraphQLSubscriptionServer_SubscribePublish);
    RUN_TEST(Test_GraphQLSubscriptionServer_UnsubscribeUnknown);
    RUN_TEST(Test_OAuth2PKCEMiddleware_Exchange);
    RUN_TEST(Test_OAuth2PKCEMiddleware_NoVerifier);
    RUN_TEST(Test_JWTValidationEngine_ValidToken);
    RUN_TEST(Test_JWTValidationEngine_MalformedToken);
    RUN_TEST(Test_JWTValidationEngine_AlgorithmSupport);
    RUN_TEST(Test_RateLimitingMiddleware_AllowAndExhaust);
    RUN_TEST(Test_RateLimitingMiddleware_Reset);
    RUN_TEST(Test_OpenAPICodeGenerator_Generate);
    RUN_TEST(Test_OpenAPICodeGenerator_EmptySpec);
    RUN_TEST(Test_OpenAPICodeGenerator_LanguageSuffix);
    RUN_TEST(Test_GraphQLSchemaIntrospector_IntrospectTypes);
    RUN_TEST(Test_NeuralCodecV2Engine_Encode);
    RUN_TEST(Test_NeuralCodecV2Engine_EmptyInput);
    RUN_TEST(Test_NeuralCodecV2Engine_BackendAvailability);
    RUN_TEST(Test_ProgressiveNeuralDecoder_Steps);
    RUN_TEST(Test_ProgressiveNeuralDecoder_Reset);
    RUN_TEST(Test_NeuralContainerFormat_SerializeDeserialize);
    RUN_TEST(Test_NeuralContainerFormat_IsNCFData);
    RUN_TEST(Test_NeuralCodecHWAccelerator_Encode);
    RUN_TEST(Test_NeuralCodecHWAccelerator_BackendNames);
    RUN_TEST(Test_CodecNegotiationProtocol_Negotiate);
    RUN_TEST(Test_CodecNegotiationProtocol_EmptyOffer);
    RUN_TEST(Test_CodecNegotiationProtocol_CodecName);

    // Sprint 801-810 — Platform (v28.0.0 Polaris)
    // Sprint 811-820 — AI Captions (v28.1.0 Polaris-R)
    // Sprint 821-830 — AR (v28.2.0 Polaris-S)
    // Sprint 831-840 — Enterprise (v28.3.0 Polaris-T)
    // Sprint 841-850 — UX (v28.4.0 Polaris-U)
    // Sprint 851-860 — Security (v28.5.0 Polaris-V)
    // Sprint 861-870 — PQC Signatures (v28.6.0 Polaris-W)
    RUN_TEST(TestPQCSignatureVerifier_Verify);
    RUN_TEST(TestHybridTrustChainV2_Validate);
    RUN_TEST(TestQuantumSafeKeyExchange_Handshake);
    RUN_TEST(TestDilithiumCertificateStore_ImportGet);
    RUN_TEST(TestSignatureAuditLogger_LogExport);
    RUN_TEST(TestCryptoAgilityBroker_Preferred);
    RUN_TEST(TestKeyRotationScheduler_TickFire);
    // Sprint 871-880 — Cross-Platform (v28.7.0 Polaris-X)
    RUN_TEST(TestGTK4ThumbnailWidget_Render);
    RUN_TEST(TestXDGThumbnailProvider_Create);
    RUN_TEST(TestPlatformCapabilityProbe_Summary);
    // Sprint 881-890 — Gen-5 WinUI 4 (v29.0.0 Capella)
    RUN_TEST(TestAsyncPreviewBroker_EnqueueFlush);
    RUN_TEST(TestUniversalFileProvider_Read);
    RUN_TEST(TestWinUI4PreviewHandler_RenderPanel);
    RUN_TEST(TestShellPropertyHandlerV2_Build);
    RUN_TEST(TestPreviewPipelineV5_Process);
    RUN_TEST(TestPersistentL3Cache_PutGetEvict);
    RUN_TEST(TestLivePreviewUpdater_WatchFire);
    RUN_TEST(TestShellExtHealthMonitorV2_Latency);
    // Sprint 891-900 — Accessibility v2 (v29.1.0 Capella-R)
    RUN_TEST(TestARIAThumbnailAnnotator_RenderAttr);
    RUN_TEST(TestHighContrastThemeAdapter_Adapt);
    RUN_TEST(TestKeyboardNavigationController_ArrowKeys);
    RUN_TEST(TestA11yTelemetryReporter_Report);

    // Sprint 961-970 — Gen-6 Platform Unification (v30.0.0 "Deneb")
    RUN_TEST(TestPAL_Detect);
    RUN_TEST(TestPAL_Surface);
    RUN_TEST(TestPAL_Scales);
    RUN_TEST(TestPAL_Cores);
    RUN_TEST(TestPAL_PageSize);
    RUN_TEST(TestPAL_SystemMem);
    RUN_TEST(TestPAL_TempDir);
    RUN_TEST(TestPAL_ThreadCount);
    RUN_TEST(TestPAL_ZeroSurface);
    RUN_TEST(TestNFA_Watch);
    RUN_TEST(TestNFA_StopAll);
    RUN_TEST(TestNFA_Exists);
    RUN_TEST(TestNFA_IsDir);
    RUN_TEST(TestNFA_Temp);
    RUN_TEST(TestNFA_MaxPath);
    RUN_TEST(TestBuildMatrix_Validate);

    // Sprint 971-980 — DirectStorage & GPU Decompression (v30.1.0 "Deneb-R")
    RUN_TEST(TestDSEngine_Status);
    RUN_TEST(TestDSEngine_Init);
    RUN_TEST(TestDSEngine_Shutdown);
    RUN_TEST(TestDSEngine_Available);
    RUN_TEST(TestDSEngine_DoubleInit);
    RUN_TEST(TestGPUDecomp_Vendor);
    RUN_TEST(TestGPUDecomp_Throughput);
    RUN_TEST(TestGPUDecomp_HwAccel);
    RUN_TEST(TestDSPipeline_Init);
    RUN_TEST(TestDSPipeline_Stats);
    RUN_TEST(TestDSPipeline_Process);
    RUN_TEST(TestDSPipeline_Shutdown);
    RUN_TEST(TestDSPipeline_Latency);
    RUN_TEST(TestDSPipeline_Enabled);
    RUN_TEST(TestDSPipeline_Priority);
    RUN_TEST(TestDSPipeline_FallbackMode);
    RUN_TEST(TestNvGDeflate_Init);
    RUN_TEST(TestNvGDeflate_Supported);
    RUN_TEST(TestNvGDeflate_Device);
    RUN_TEST(TestNvGDeflate_Shutdown);
    RUN_TEST(TestAMDDecomp_Init);
    RUN_TEST(TestAMDDecomp_Supported);
    RUN_TEST(TestAMDDecomp_Device);
    RUN_TEST(TestAMDDecomp_Shutdown);
    RUN_TEST(TestDSCache_Init);
    RUN_TEST(TestDSCache_PutGet);
    RUN_TEST(TestDSCache_Miss);
    RUN_TEST(TestDSCache_Evict);
    RUN_TEST(TestDSCache_HitRate);
    RUN_TEST(TestDSCache_Capacity);
    RUN_TEST(TestDSCache_Count);
    RUN_TEST(TestDSCache_Clear);
    RUN_TEST(TestDSCache_Usage);
    RUN_TEST(TestAsyncStream_BytesRead);
    RUN_TEST(TestStagingPool_Init);
    RUN_TEST(TestStagingPool_Acquire);
    RUN_TEST(TestStagingPool_Release);
    RUN_TEST(TestStagingPool_Size);
    RUN_TEST(TestStagingPool_Active);
    RUN_TEST(TestStagingPool_Trim);
    RUN_TEST(TestStagingPool_TotalMem);
    RUN_TEST(TestStagingPool_MaxBuf);
    RUN_TEST(TestStagingPool_Exhaustion);

    // Sprint 981-990 — CLIP Semantic Search & Discovery (v30.2.0 "Deneb-S")
    RUN_TEST(TestCLIP_Dim);
    RUN_TEST(TestCLIP_Latency);
    RUN_TEST(TestCLIP_Backend);
    RUN_TEST(TestCLIP_NullInput);
    RUN_TEST(TestNLQuery_Vocab);
    RUN_TEST(TestNLQuery_Empty);
    RUN_TEST(TestEmbCache_Init);
    RUN_TEST(TestEmbCache_Store);
    RUN_TEST(TestEmbCache_Retrieve);
    RUN_TEST(TestEmbCache_Miss);
    RUN_TEST(TestEmbCache_Size);
    RUN_TEST(TestEmbCache_Compact);
    RUN_TEST(TestEmbCache_Clear);
    RUN_TEST(TestEmbCache_Stats);
    RUN_TEST(TestEmbCache_Eviction);
    RUN_TEST(TestMMRank_Weights);
    RUN_TEST(TestDedup_Empty);
    RUN_TEST(TestIdxUpd_Init);
    RUN_TEST(TestIdxUpd_FileCreated);
    RUN_TEST(TestIdxUpd_FileDeleted);
    RUN_TEST(TestIdxUpd_FileModified);
    RUN_TEST(TestIdxUpd_Flush);
    RUN_TEST(TestIdxUpd_Multi);
    RUN_TEST(TestIdxUpd_Stats);
    RUN_TEST(TestIdxUpd_Shutdown);
    RUN_TEST(TestIdxUpd_Latency);

    // Sprint 991-1000 — Live Preview Scrubber & Rich Media (v30.3.0 "Deneb-T")
    RUN_TEST(TestScrub_Init);
    RUN_TEST(TestScrub_State);
    RUN_TEST(TestScrub_Mode);
    RUN_TEST(TestScrub_Seek);
    RUN_TEST(TestScrub_Callback);
    RUN_TEST(TestScrub_Config);
    RUN_TEST(TestScrub_Modes);
    RUN_TEST(TestScrub_Zero);
    RUN_TEST(TestScrub_Progress);
    RUN_TEST(TestKF_Init);
    RUN_TEST(TestKF_Metadata);
    RUN_TEST(TestKF_Mode);
    RUN_TEST(TestKF_Info);
    RUN_TEST(TestKF_MaxKeyframes);
    RUN_TEST(TestKF_Duration);
    RUN_TEST(TestKF_Codec);
    RUN_TEST(TestKF_FPS);
    RUN_TEST(TestKF_SceneScore);
    RUN_TEST(TestAnim_Init);
    RUN_TEST(TestAnim_Format);
    RUN_TEST(TestAnim_Frame);
    RUN_TEST(TestAnim_FpsCap);
    RUN_TEST(TestAnim_Disposal);
    RUN_TEST(TestAnim_Zero);
    RUN_TEST(TestAnim_WebP);
    RUN_TEST(TestAnim_APNG);
    RUN_TEST(TestAnim_Delay);
    RUN_TEST(TestWave_Init);
    RUN_TEST(TestWave_Style);
    RUN_TEST(TestWave_Meta);
    RUN_TEST(TestWave_Config);
    RUN_TEST(TestWave_BitDepth);
    RUN_TEST(TestWave_Codec);
    RUN_TEST(TestWave_Color);
    RUN_TEST(TestWave_Bars);
    RUN_TEST(TestWave_Channels);
    RUN_TEST(TestDocPV_Init);
    RUN_TEST(TestDocPV_Type);
    RUN_TEST(TestDocPV_Page);
    RUN_TEST(TestDocPV_Config);
    RUN_TEST(TestDocPV_PPTX);
    RUN_TEST(TestDocPV_HasImages);
    RUN_TEST(TestDocPV_Preload);
    RUN_TEST(TestDocPV_XLS);
    RUN_TEST(TestDocPV_Index);
    RUN_TEST(TestShHL_Init);
    RUN_TEST(TestShHL_Lang);
    RUN_TEST(TestShHL_Theme);
    RUN_TEST(TestShHL_Tilt);
    RUN_TEST(TestShHL_LineNum);
    RUN_TEST(TestShHL_HLSL);
    RUN_TEST(TestShHL_Dracula);
    RUN_TEST(TestShHL_Spacing);
    RUN_TEST(TestShHL_WGSL);
    RUN_TEST(TestFGS_Init);
    RUN_TEST(TestFGS_Script);
    RUN_TEST(TestFGS_Metrics);
    RUN_TEST(TestFGS_Pangram);
    RUN_TEST(TestFGS_CJK);
    RUN_TEST(TestFGS_Arabic);
    RUN_TEST(TestFGS_Weight);
    RUN_TEST(TestFGS_ShowMetrics);
    RUN_TEST(TestFGS_BearingY);
    RUN_TEST(TestSSCR_Init);
    RUN_TEST(TestSSCR_Type);
    RUN_TEST(TestSSCR_Detect);
    RUN_TEST(TestSSCR_Meta);
    RUN_TEST(TestSSCR_Charts);
    RUN_TEST(TestSSCR_Pie);
    RUN_TEST(TestSSCR_DataRange);
    RUN_TEST(TestSSCR_Scatter);
    RUN_TEST(TestSSCR_Cols);

    // Sprint 1001-1010 — Geospatial, Medical & Scientific Formats (v30.4.0 "Deneb-U")
    RUN_TEST(TestGeoT_Init);
    RUN_TEST(TestGeoT_Meta);
    RUN_TEST(TestGeoT_Band);
    RUN_TEST(TestGeoT_Composite);
    RUN_TEST(TestGeoT_Stretch);
    RUN_TEST(TestGeoT_CRS);
    RUN_TEST(TestGeoT_Thermal);
    RUN_TEST(TestGeoT_NIR);
    RUN_TEST(TestGeoT_FalseColor);
    RUN_TEST(TestNITF_Init);
    RUN_TEST(TestNITF_Seg);
    RUN_TEST(TestNITF_Comp);
    RUN_TEST(TestNITF_Sec);
    RUN_TEST(TestNITF_BPP);
    RUN_TEST(TestNITF_JPEG);
    RUN_TEST(TestNITF_VQ);
    RUN_TEST(TestNITF_Secret);
    RUN_TEST(TestNITF_Index);
    RUN_TEST(TestDICM_Init);
    RUN_TEST(TestDICM_Window);
    RUN_TEST(TestDICM_Frame);
    RUN_TEST(TestDICM_Series);
    RUN_TEST(TestDICM_Bone);
    RUN_TEST(TestDICM_Brain);
    RUN_TEST(TestDICM_Slice);
    RUN_TEST(TestDICM_Patient);
    RUN_TEST(TestDICM_Thickness);
    RUN_TEST(TestNRRD_Init);
    RUN_TEST(TestNRRD_Enc);
    RUN_TEST(TestNRRD_Field);
    RUN_TEST(TestNRRD_Header);
    RUN_TEST(TestNRRD_Gzip);
    RUN_TEST(TestNRRD_Tensor);
    RUN_TEST(TestNRRD_Sizes);
    RUN_TEST(TestNRRD_Hex);
    RUN_TEST(TestNRRD_Spacings);
    RUN_TEST(TestHDF5_Init);
    RUN_TEST(TestHDF5_Type);
    RUN_TEST(TestHDF5_Cmap);
    RUN_TEST(TestHDF5_Info);
    RUN_TEST(TestHDF5_Plasma);
    RUN_TEST(TestHDF5_Matrix);
    RUN_TEST(TestHDF5_TotalSize);
    RUN_TEST(TestHDF5_Inferno);
    RUN_TEST(TestHDF5_Compound);
    RUN_TEST(TestNCD_Init);
    RUN_TEST(TestNCD_Var);
    RUN_TEST(TestNCD_Dim);
    RUN_TEST(TestNCD_VarInfo);
    RUN_TEST(TestNCD_Pressure);
    RUN_TEST(TestNCD_Wind);
    RUN_TEST(TestNCD_Salinity);
    RUN_TEST(TestNCD_Elevation);
    RUN_TEST(TestNCD_Fill);
    RUN_TEST(TestFITS_Init);
    RUN_TEST(TestFITS_Stretch);
    RUN_TEST(TestFITS_LUT);
    RUN_TEST(TestFITS_Header);
    RUN_TEST(TestFITS_Log);
    RUN_TEST(TestFITS_Asinh);
    RUN_TEST(TestFITS_Object);
    RUN_TEST(TestFITS_Heat);
    RUN_TEST(TestFITS_Scale);
    RUN_TEST(TestECW_Init);
    RUN_TEST(TestECW_Meta);
    RUN_TEST(TestECW_Ratio);
    RUN_TEST(TestECW_ResLevel);
    RUN_TEST(TestECW_CellSize);
    RUN_TEST(TestECW_Proj);
    RUN_TEST(TestECW_Block);
    RUN_TEST(TestECW_Bands);
    RUN_TEST(TestECW_Target);

    // Sprint 1011-1020 — Universal Format Decoder Library (v30.5.0 "Deneb-V")
    RUN_TEST(TestUDF_Init);
    RUN_TEST(TestUDF_Format);
    RUN_TEST(TestUDF_Request);
    RUN_TEST(TestUDF_Result);
    RUN_TEST(TestUDF_Formats);
    RUN_TEST(TestUDF_AVIF);
    RUN_TEST(TestUDF_PSD);
    RUN_TEST(TestUDF_RAW);
    RUN_TEST(TestUDF_Stride);
    RUN_TEST(TestFCM_Init);
    RUN_TEST(TestFCM_Level);
    RUN_TEST(TestFCM_Platform);
    RUN_TEST(TestFCM_Cap);
    RUN_TEST(TestFCM_Linux);
    RUN_TEST(TestFCM_macOS);
    RUN_TEST(TestFCM_Basic);
    RUN_TEST(TestFCM_Full);
    RUN_TEST(TestFCM_MinVer);
    RUN_TEST(TestDVR_Init);
    RUN_TEST(TestDVR_SemVer);
    RUN_TEST(TestDVR_Reg);
    RUN_TEST(TestDVR_Pre);
    RUN_TEST(TestDVR_Author);
    RUN_TEST(TestDVR_Compat);
    RUN_TEST(TestDVR_Desc);
    RUN_TEST(TestDVR_Zero);
    RUN_TEST(TestDVR_Version);
    RUN_TEST(TestFFR_Init);
    RUN_TEST(TestFFR_Family);
    RUN_TEST(TestFFR_Node);
    RUN_TEST(TestFFR_Video);
    RUN_TEST(TestFFR_Audio);
    RUN_TEST(TestFFR_Doc);
    RUN_TEST(TestFFR_Archive);
    RUN_TEST(TestFFR_Scientific);
    RUN_TEST(TestFFR_Exts);
    RUN_TEST(TestDHA_Init);
    RUN_TEST(TestDHA_Priority);
    RUN_TEST(TestDHA_Entry);
    RUN_TEST(TestDHA_Normal);
    RUN_TEST(TestDHA_High);
    RUN_TEST(TestDHA_Hash);
    RUN_TEST(TestDHA_Target);
    RUN_TEST(TestDHA_Critical);
    RUN_TEST(TestDHA_Desc);
    RUN_TEST(TestFSV_Init);
    RUN_TEST(TestFSV_Severity);
    RUN_TEST(TestFSV_Result);
    RUN_TEST(TestFSV_Spec);
    RUN_TEST(TestFSV_Warning);
    RUN_TEST(TestFSV_Error);
    RUN_TEST(TestFSV_Offset);
    RUN_TEST(TestFSV_Rule);
    RUN_TEST(TestFSV_SpecVer);
    RUN_TEST(TestDCL_Init);
    RUN_TEST(TestDCL_Mode);
    RUN_TEST(TestDCL_Issue);
    RUN_TEST(TestDCL_Report);
    RUN_TEST(TestDCL_Legacy);
    RUN_TEST(TestDCL_Relaxed);
    RUN_TEST(TestDCL_Workaround);
    RUN_TEST(TestDCL_Versions);
    RUN_TEST(TestDCL_Migrations);
    RUN_TEST(TestULM_Init);
    RUN_TEST(TestULM_License);
    RUN_TEST(TestULM_Lib);
    RUN_TEST(TestULM_Manifest);
    RUN_TEST(TestULM_Apache);
    RUN_TEST(TestULM_BSD);
    RUN_TEST(TestULM_LGPL);
    RUN_TEST(TestULM_SPDX);
    RUN_TEST(TestULM_Copy);
    // Sprint 1021-1030 — Plugin Marketplace v4 & Commerce (v30.6.0 "Deneb-W")
    RUN_TEST(TestMKT_Init);
    RUN_TEST(TestMKT_Proto);
    RUN_TEST(TestMKT_Cache);
    RUN_TEST(TestMKT_List);
    RUN_TEST(TestMKT_Rating);
    RUN_TEST(TestMKT_Price);
    RUN_TEST(TestMKT_gRPC);
    RUN_TEST(TestMKT_Persist);
    RUN_TEST(TestMKT_Author);
    RUN_TEST(TestRE_Init);
    RUN_TEST(TestRE_Src);
    RUN_TEST(TestRE_Entry);
    RUN_TEST(TestRE_Thresh);
    RUN_TEST(TestRE_AutoScan);
    RUN_TEST(TestRE_Compat);
    RUN_TEST(TestRE_Reviews);
    RUN_TEST(TestRE_Susp);
    RUN_TEST(TestRE_Score);
    RUN_TEST(TestDG_Init);
    RUN_TEST(TestDG_Type);
    RUN_TEST(TestDG_Edge);
    RUN_TEST(TestDG_Result);
    RUN_TEST(TestDG_Optional);
    RUN_TEST(TestDG_Conflicts);
    RUN_TEST(TestDG_VerConst);
    RUN_TEST(TestDG_Missing);
    RUN_TEST(TestDG_InstOrd);
    RUN_TEST(TestBI_Init);
    RUN_TEST(TestBI_Phase);
    RUN_TEST(TestBI_Entry);
    RUN_TEST(TestBI_Prog);
    RUN_TEST(TestBI_DL);
    RUN_TEST(TestBI_Stage);
    RUN_TEST(TestBI_Commit);
    RUN_TEST(TestBI_CSum);
    RUN_TEST(TestBI_Curr);
    RUN_TEST(TestSLM_Init);
    RUN_TEST(TestSLM_Type);
    RUN_TEST(TestSLM_Seat);
    RUN_TEST(TestSLM_JWT);
    RUN_TEST(TestSLM_Team);
    RUN_TEST(TestSLM_Ent);
    RUN_TEST(TestSLM_Expired);
    RUN_TEST(TestSLM_Features);
    RUN_TEST(TestSLM_Sig);
    RUN_TEST(TestRS_Init);
    RUN_TEST(TestRS_Factor);
    RUN_TEST(TestRS_CVE);
    RUN_TEST(TestRS_Score);
    RUN_TEST(TestRS_Stars);
    RUN_TEST(TestRS_Sec);
    RUN_TEST(TestRS_Black);
    RUN_TEST(TestRS_Patched);
    RUN_TEST(TestRS_CVSS);
    RUN_TEST(TestAU_Init);
    RUN_TEST(TestAU_Mode);
    RUN_TEST(TestAU_Chan);
    RUN_TEST(TestAU_Policy);
    RUN_TEST(TestAU_Notify);
    RUN_TEST(TestAU_Defer);
    RUN_TEST(TestAU_Locked);
    RUN_TEST(TestAU_Beta);
    RUN_TEST(TestAU_Nightly);
    RUN_TEST(TestPRG_Init);
    RUN_TEST(TestPRG_Status);
    RUN_TEST(TestPRG_Check);
    RUN_TEST(TestPRG_Report);
    RUN_TEST(TestPRG_Approved);
    RUN_TEST(TestPRG_Rejected);
    RUN_TEST(TestPRG_NeedsRev);
    RUN_TEST(TestPRG_Sig);
    RUN_TEST(TestPRG_Issues);
    // Sprint 1031-1040 — Enterprise Console v3 & Fleet Management (v30.7.0 "Deneb-X")
    RUN_TEST(TestEC3_Init);
    RUN_TEST(TestEC3_Proto);
    RUN_TEST(TestEC3_Role);
    RUN_TEST(TestEC3_Endpoint);
    RUN_TEST(TestEC3_Admin);
    RUN_TEST(TestEC3_Op);
    RUN_TEST(TestEC3_Port);
    RUN_TEST(TestEC3_Token);
    RUN_TEST(TestEC3_gRPC);
    RUN_TEST(TestFDM_Init);
    RUN_TEST(TestFDM_Method);
    RUN_TEST(TestFDM_Stage);
    RUN_TEST(TestFDM_Job);
    RUN_TEST(TestFDM_MDM);
    RUN_TEST(TestFDM_Ring2);
    RUN_TEST(TestFDM_Done);
    RUN_TEST(TestFDM_SCCM);
    RUN_TEST(TestFDM_Ver);
    RUN_TEST(TestCRG_Init);
    RUN_TEST(TestCRG_FW);
    RUN_TEST(TestCRG_Fmt);
    RUN_TEST(TestCRG_Ev);
    RUN_TEST(TestCRG_HIPAA);
    RUN_TEST(TestCRG_ISO);
    RUN_TEST(TestCRG_JSON);
    RUN_TEST(TestCRG_HTML);
    RUN_TEST(TestCRG_Evid);
    RUN_TEST(TestEMD_Init);
    RUN_TEST(TestEMD_Type);
    RUN_TEST(TestEMD_Pct);
    RUN_TEST(TestEMD_Snap);
    RUN_TEST(TestEMD_ErrRate);
    RUN_TEST(TestEMD_Tput);
    RUN_TEST(TestEMD_Cache);
    RUN_TEST(TestEMD_P999);
    RUN_TEST(TestEMD_Name);
    RUN_TEST(TestPVC_Init);
    RUN_TEST(TestPVC_Chg);
    RUN_TEST(TestPVC_Diff);
    RUN_TEST(TestPVC_Ver);
    RUN_TEST(TestPVC_Del);
    RUN_TEST(TestPVC_Mod);
    RUN_TEST(TestPVC_Hash);
    RUN_TEST(TestPVC_Changes);
    RUN_TEST(TestPVC_Author);
    RUN_TEST(TestRDC_Init);
    RUN_TEST(TestRDC_Act);
    RUN_TEST(TestRDC_Target);
    RUN_TEST(TestRDC_Result);
    RUN_TEST(TestRDC_Dis);
    RUN_TEST(TestRDC_Quar);
    RUN_TEST(TestRDC_Rest);
    RUN_TEST(TestRDC_Errs);
    RUN_TEST(TestRDC_State);
    RUN_TEST(TestUAD_Init);
    RUN_TEST(TestUAD_Sev);
    RUN_TEST(TestUAD_Type);
    RUN_TEST(TestUAD_Event);
    RUN_TEST(TestUAD_Med);
    RUN_TEST(TestUAD_High);
    RUN_TEST(TestUAD_Latency);
    RUN_TEST(TestUAD_Decoder);
    RUN_TEST(TestUAD_ZScore);
    RUN_TEST(TestEAE_Init);
    RUN_TEST(TestEAE_Target);
    RUN_TEST(TestEAE_Fmt);
    RUN_TEST(TestEAE_Ev);
    RUN_TEST(TestEAE_Sent);
    RUN_TEST(TestEAE_QR);
    RUN_TEST(TestEAE_LEEF);
    RUN_TEST(TestEAE_Actor);
    RUN_TEST(TestEAE_Res);
    RUN_TEST(TestGTE_BackendEnum);
    RUN_TEST(TestGTE_GenerationMode);
    RUN_TEST(TestGTE_RequestDefaults);
    RUN_TEST(TestGTE_EngineInit);
    RUN_TEST(TestGTE_SupportedModes);
    RUN_TEST(TestGTE_SetBackend);
    RUN_TEST(TestGTE_Shutdown);
    RUN_TEST(TestGTE_GenerateNullSafe);
    RUN_TEST(TestGTE_MultiBackendCheck);
    RUN_TEST(TestCAI_AlgorithmEnum);
    RUN_TEST(TestCAI_QualityEnum);
    RUN_TEST(TestCAI_RegionStruct);
    RUN_TEST(TestCAI_NotProcessing);
    RUN_TEST(TestCAI_SetQuality);
    RUN_TEST(TestCAI_CancelSafe);
    RUN_TEST(TestCAI_DurationEstimate);
    RUN_TEST(TestCAI_InpaintNullGuard);
    RUN_TEST(TestCAI_DraftQuality);
    RUN_TEST(TestSTR_StyleEnum);
    RUN_TEST(TestSTR_StrengthEnum);
    RUN_TEST(TestSTR_ParamsStruct);
    RUN_TEST(TestSTR_AvailableStyles);
    RUN_TEST(TestSTR_LastAppliedEmpty);
    RUN_TEST(TestSTR_LoadModelFails);
    RUN_TEST(TestSTR_UnloadSafe);
    RUN_TEST(TestSTR_ApplyStyleNullGuard);
    RUN_TEST(TestIDS_DepthEnum);
    RUN_TEST(TestIDS_LanguageEnum);
    RUN_TEST(TestIDS_SynthesisResult);
    RUN_TEST(TestIDS_SetGetLanguage);
    RUN_TEST(TestIDS_ModelNotLoaded);
    RUN_TEST(TestIDS_LoadModelFails);
    RUN_TEST(TestIDS_DefaultLanguage);
    RUN_TEST(TestIDS_SynthesizeNullSafe);
    RUN_TEST(TestTPE_SignalEnum);
    RUN_TEST(TestTPE_StrategyEnum);
    RUN_TEST(TestTPE_ProfileStruct);
    RUN_TEST(TestTPE_GetProfileMissing);
    RUN_TEST(TestTPE_UpdateSignal);
    RUN_TEST(TestTPE_ResetProfile);
    RUN_TEST(TestTPE_ApplyNullSafe);
    RUN_TEST(TestTPE_MultiUser);
    RUN_TEST(TestGUV3_ModelEnum);
    RUN_TEST(TestGUV3_FactorEnum);
    RUN_TEST(TestGUV3_JobStruct);
    RUN_TEST(TestGUV3_OutputDimensions);
    RUN_TEST(TestGUV3_SelectBestModel);
    RUN_TEST(TestGUV3_VRAMEstimate);
    RUN_TEST(TestGUV3_ModelAvailable);
    RUN_TEST(TestGUV3_UpscaleNullSafe);
    RUN_TEST(TestCMF_TierEnum);
    RUN_TEST(TestCMF_FlagEnum);
    RUN_TEST(TestCMF_ModerationResult);
    RUN_TEST(TestCMF_SetGetTier);
    RUN_TEST(TestCMF_DefaultTier);
    RUN_TEST(TestCMF_AddBlocklist);
    RUN_TEST(TestCMF_BlocklistCategories);
    RUN_TEST(TestCMF_EvaluateNullSafe);
    RUN_TEST(TestGAT_EventTypeEnum);
    RUN_TEST(TestGAT_RetentionEnum);
    RUN_TEST(TestGAT_EntryStruct);
    RUN_TEST(TestGAT_RecordAndQuery);
    RUN_TEST(TestGAT_SetGetRetention);
    RUN_TEST(TestGAT_PurgeEmpty);
    RUN_TEST(TestGAT_ExportEmpty);
    RUN_TEST(TestGAT_QueryEmpty);
    RUN_TEST(TestGAT_RecordMultiple);
    // Cross-Platform Shell Extensions (Sprint 1001-1010 / v31.1.0)
    RUN_TEST(TestLNE_IntegrationModeEnum);
    RUN_TEST(TestLNE_VersionEnum);
    RUN_TEST(TestLNE_ConfigStruct);
    RUN_TEST(TestLNE_NotRegistered);
    RUN_TEST(TestLNE_GetFormats);
    RUN_TEST(TestLNE_InitializeConfig);
    RUN_TEST(TestLNE_UnregisterSafe);
    RUN_TEST(TestLNE_RegisterFails);
    RUN_TEST(TestLNE_DBusMode);
    RUN_TEST(TestKDE_PluginTypeEnum);
    RUN_TEST(TestKDE_PriorityEnum);
    RUN_TEST(TestKDE_ConfigStruct);
    RUN_TEST(TestKDE_NotActive);
    RUN_TEST(TestKDE_SetPriority);
    RUN_TEST(TestKDE_RegisterFails);
    RUN_TEST(TestKDE_UnregisterSafe);
    RUN_TEST(TestKDE_InitConfig);
    RUN_TEST(TestKDE_MimeTypes);
    RUN_TEST(TestTTE_InterfaceEnum);
    RUN_TEST(TestTTE_SchedulerEnum);
    RUN_TEST(TestTTE_ConfigStruct);
    RUN_TEST(TestTTE_NotRegistered);
    RUN_TEST(TestTTE_SetScheduler);
    RUN_TEST(TestTTE_RegisterFails);
    RUN_TEST(TestTTE_UnregisterSafe);
    RUN_TEST(TestTTE_InitConfig);
    RUN_TEST(TestTTE_TimeoutValue);
    RUN_TEST(TestQL_APIEnum);
    RUN_TEST(TestQL_ScaleEnum);
    RUN_TEST(TestQL_ConfigStruct);
    RUN_TEST(TestQL_NotActive);
    RUN_TEST(TestQL_GetUTIs);
    RUN_TEST(TestQL_RegisterFails);
    RUN_TEST(TestQL_UnregisterSafe);
    RUN_TEST(TestQL_InitConfig);
    RUN_TEST(TestQL_SandboxEnabled);
    RUN_TEST(TestLTD_ThumbSizeEnum);
    RUN_TEST(TestLTD_DaemonStateEnum);
    RUN_TEST(TestLTD_ConfigStruct);
    RUN_TEST(TestLTD_InitialState);
    RUN_TEST(TestLTD_StartDaemon);
    RUN_TEST(TestLTD_StopSafe);
    RUN_TEST(TestLTD_PurgeCache);
    RUN_TEST(TestLTD_GenerateNullPath);
    RUN_TEST(TestLTD_AutoStart);
    RUN_TEST(TestWSE_ProtocolEnum);
    RUN_TEST(TestWSE_CompositorEnum);
    RUN_TEST(TestWSE_ConfigStruct);
    RUN_TEST(TestWSE_NotConnected);
    RUN_TEST(TestWSE_ConnectFails);
    RUN_TEST(TestWSE_DisconnectSafe);
    RUN_TEST(TestWSE_GetCompositor);
    RUN_TEST(TestWSE_InitConfig);
    RUN_TEST(TestWSE_HiDPIEnabled);
    RUN_TEST(TestMLS_RoleEnum);
    RUN_TEST(TestMLS_ScopeEnum);
    RUN_TEST(TestMLS_ConfigStruct);
    RUN_TEST(TestMLS_NotRegistered);
    RUN_TEST(TestMLS_GetUTIs);
    RUN_TEST(TestMLS_RegisterFails);
    RUN_TEST(TestMLS_UnregisterSafe);
    RUN_TEST(TestMLS_SetDefault);
    RUN_TEST(TestMLS_TemporaryScope);
    RUN_TEST(TestXPT_PlatformEnum);
    RUN_TEST(TestXPT_VerdictEnum);
    RUN_TEST(TestXPT_TestCaseStruct);
    RUN_TEST(TestXPT_InitialPassRate);
    RUN_TEST(TestXPT_TotalTests);
    RUN_TEST(TestXPT_GetFailed);
    RUN_TEST(TestXPT_Reset);
    RUN_TEST(TestXPT_RunSingle);
    RUN_TEST(TestXPT_RunAll);
    // Integration Test Framework + COM Tests (Sprint 25+29 / v15.4.1)
    std::wcout << std::endl;
    std::wcout << L"  [Integration + COM Tests]" << std::endl;
    RUN_TEST(IntegrationRunnerSmoke);
    RUN_TEST(IntegrationRunnerSingleFile);
    RUN_TEST(IntegrationRunnerHtmlReport);
    RUN_TEST(COMThumbnailProviderRoundTrip);
    RUN_TEST(COMTestRunnerGracefulSkip);

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
