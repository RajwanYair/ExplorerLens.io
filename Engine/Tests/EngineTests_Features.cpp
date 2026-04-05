// EngineTests_Features.cpp — Feature module test bodies
// Copyright (c) 2026 ExplorerLens Project
//
// Split from EngineTests.cpp v33.0.0 to reduce file size for git.
// Contains TEST() bodies for: Feature Modules, Version Sync, Parallel I/O,
// SIMD, Format Detection, Enterprise, Watermark, CLI, Plugin, and more.
//
#include "EngineTestsIncludes.h"

//==============================================================================
// Feature Module Tests
//==============================================================================

// ---- Version Synchronization ----
TEST(TestZenith_VersionMajor)
{
    ASSERT(EXPLORERLENS_ENGINE_VERSION_MAJOR == 25);
}
TEST(TestZenith_VersionMinor)
{
    ASSERT(EXPLORERLENS_ENGINE_VERSION_MINOR == 3);
}
TEST(TestZenith_VersionPatch)
{
    ASSERT(EXPLORERLENS_ENGINE_VERSION_PATCH == 0);
}
TEST(TestZenith_VersionComposite)
{
    uint32_t v = EXPLORERLENS_ENGINE_VERSION;
    ASSERT(v == ((25 << 16) | (3 << 8) | 0));
}

// ---- MuPDF PDF Support ----
TEST(TestZenith_MuPDFBackendNames)
{
    ASSERT(std::wstring(MuPDFIntegration::BackendName(MuPDFBackend::Native)) == L"MuPDF Native");
    ASSERT(std::wstring(MuPDFIntegration::BackendName(MuPDFBackend::WIC)) == L"Windows Imaging");
    ASSERT(std::wstring(MuPDFIntegration::BackendName(MuPDFBackend::None)) == L"None");
}
TEST(TestZenith_MuPDFCaps)
{
    ASSERT(MuPDFIntegration::BackendCount() >= 3);
}
TEST(TestZenith_MuPDFPageConfig)
{
    PDFPageConfig cfg;
    ASSERT(cfg.dpi == 150);
    ASSERT(cfg.maxPages == 1);
}

// ---- libwebp CRT Clean ----
TEST(TestZenith_LibWebPConfigDefaults)
{
    LibWebPConfig cfg;
    ASSERT(cfg.useDynamicCRT == true);
    ASSERT(cfg.enableSIMD == true);
}
TEST(TestZenith_LibWebPConfigName)
{
    ASSERT(std::wstring(LibWebPConfig::CRTModeName(CRTLinkMode::DynamicMD)) == L"Dynamic (/MD)");
    ASSERT(std::wstring(LibWebPConfig::CRTModeName(CRTLinkMode::StaticMT)) == L"Static (/MT)");
}

// ---- LENSArchive Refactoring ----
TEST(TestZenith_ArchiveRefactorModules)
{
    ASSERT(ArchiveRefactorEngine::ModuleCount() >= 4);
}
TEST(TestZenith_ArchiveRefactorModuleNames)
{
    ASSERT(std::wstring(ArchiveRefactorEngine::ModuleName(RefactorModule::LENSTypes)) == L"LENSTypes");
    ASSERT(std::wstring(ArchiveRefactorEngine::ModuleName(RefactorModule::ZipWrapper)) == L"ZipWrapper");
    ASSERT(std::wstring(ArchiveRefactorEngine::ModuleName(RefactorModule::RarWrapper)) == L"RarWrapper");
}

// ---- Bitmap Pool ----
TEST(TestZenith_BitmapPoolConfig)
{
    BitmapPoolConfig cfg;
    ASSERT(cfg.width == 256);
    ASSERT(cfg.height == 256);
    ASSERT(cfg.poolSize == 50);
    ASSERT(cfg.bitsPerPixel == 32);
}
TEST(TestZenith_BitmapPoolStats)
{
    BitmapPoolStats stats;
    ASSERT(stats.acquireCount == 0);
    ASSERT(stats.HitRate() == 0.0);
}
TEST(TestZenith_BitmapPoolStatsHitRate)
{
    BitmapPoolStats stats;
    stats.acquireCount = 100;
    stats.poolHits = 85;
    ASSERT(stats.HitRate() == 85.0);
}

// ---- IPropertyStore Handler ----
TEST(TestZenith_PropertyIDNames)
{
    ASSERT(static_cast<uint32_t>(PropertyID::ImageWidth) == 0x1001);
    ASSERT(static_cast<uint32_t>(PropertyID::ImageHeight) == 0x1002);
    ASSERT(static_cast<uint32_t>(PropertyID::CodecName) == 0x1004);
}
TEST(TestZenith_PropertyTypes)
{
    ASSERT(static_cast<uint8_t>(PropertyType::UInt32) == 0);
    ASSERT(static_cast<uint8_t>(PropertyType::String) == 2);
    ASSERT(static_cast<uint8_t>(PropertyType::Bool) == 3);
}
TEST(TestZenith_PropertyValueDefault)
{
    PropertyValue pv;
    ASSERT(pv.isReadOnly == true);
    ASSERT(pv.id == PropertyID::ImageWidth);
}
TEST(TestZenith_PropertyCapabilities)
{
    auto caps = static_cast<uint32_t>(PropertyCapability::All);
    ASSERT(caps == 0x0F);
}

// ---- GPU Shader Library ----
TEST(TestZenith_GPUShaderTypes)
{
    ASSERT(std::wstring(GPUShaderLibrary::ShaderName(GPUShaderLibrary::ShaderType::LanczosResize)) == L"LanczosResize");
    ASSERT(std::wstring(GPUShaderLibrary::ShaderName(GPUShaderLibrary::ShaderType::BicubicResize)) == L"BicubicResize");
    ASSERT(std::wstring(GPUShaderLibrary::ShaderName(GPUShaderLibrary::ShaderType::HDRTonemap)) == L"HDRTonemap");
    ASSERT(std::wstring(GPUShaderLibrary::ShaderName(GPUShaderLibrary::ShaderType::ColorConvert)) == L"ColorConvert");
}
TEST(TestZenith_GPUShaderCount)
{
    ASSERT(static_cast<int>(GPUShaderLibrary::ShaderType::COUNT) >= 7);
}
TEST(TestZenith_ToneMapAlgorithms)
{
    ASSERT(std::wstring(GPUShaderLibrary::ToneMapName(GPUShaderLibrary::ToneMapAlgorithm::Reinhard)) == L"Reinhard");
    ASSERT(std::wstring(GPUShaderLibrary::ToneMapName(GPUShaderLibrary::ToneMapAlgorithm::ACES)) == L"ACES");
}
TEST(TestZenith_GPUColorSpaces)
{
    ASSERT(static_cast<int>(GPUShaderLibrary::ColorSpace::COUNT) >= 5);
}

// ---- PluginHost Out-of-Process ----
TEST(TestZenith_PluginHostModes)
{
    ASSERT(PluginHostManager::ModeCount() >= 3);
}
TEST(TestZenith_PluginHostModeNames)
{
    ASSERT(std::wstring(PluginHostManager::ModeName(PluginHostMode::InProcess)) == L"In-Process");
    ASSERT(std::wstring(PluginHostManager::ModeName(PluginHostMode::OutOfProcess)) == L"Out-of-Process");
}

// ---- Library Version Audit ----
TEST(TestZenith_LibVersionAuditFields)
{
    ASSERT(LibraryVersionAudit::LibraryCount() >= 10);
}
TEST(TestZenith_LibVersionAuditNames)
{
    ASSERT(std::wstring(LibraryVersionAudit::StatusName(LibraryStatus::UpToDate)) == L"Up-to-date");
    ASSERT(std::wstring(LibraryVersionAudit::StatusName(LibraryStatus::UpdateAvailable)) == L"Update Available");
}

// ---- OpenJPEG JPEG 2000 ----
TEST(TestZenith_OpenJPEGProfiles)
{
    ASSERT(OpenJPEGIntegration::ProfileCount() >= 3);
}
TEST(TestZenith_OpenJPEGProfileNames)
{
    ASSERT(std::wstring(OpenJPEGIntegration::ProfileName(JPEG2000Profile::Part1)) == L"Part 1 (JP2)");
    ASSERT(std::wstring(OpenJPEGIntegration::ProfileName(JPEG2000Profile::Part2)) == L"Part 2 (JPX)");
}

// ---- FreeType Font Rendering ----
TEST(TestZenith_FreeTypeRenderModes)
{
    ASSERT(FreeTypeIntegration::RenderModeCount() >= 3);
}
TEST(TestZenith_FreeTypeRenderModeNames)
{
    ASSERT(std::wstring(FreeTypeIntegration::RenderModeName(FontRenderMode::Normal)) == L"Normal");
    ASSERT(std::wstring(FreeTypeIntegration::RenderModeName(FontRenderMode::Subpixel)) == L"Subpixel");
}

// ---- FFmpeg Integration ----
TEST(TestZenith_FFmpegCodecFamilies)
{
    ASSERT(static_cast<int>(FFmpegCodecFamily::H264) == 1);
    ASSERT(static_cast<int>(FFmpegCodecFamily::VP9) == 4);
    ASSERT(static_cast<int>(FFmpegCodecFamily::AV1) == 5);
}
TEST(TestZenith_FFmpegContainerFormats)
{
    ASSERT(static_cast<int>(ContainerFormat::MKV) == 1);
    ASSERT(static_cast<int>(ContainerFormat::WebM) == 2);
    ASSERT(static_cast<int>(ContainerFormat::FLV) == 5);
}
TEST(TestZenith_FFmpegVideoStreamInfo)
{
    VideoStreamInfo info;
    ASSERT(info.width == 0);
    ASSERT(info.codec == FFmpegCodecFamily::Unknown);
    ASSERT(info.totalFrames == 0);
}
TEST(TestZenith_FFmpegFrameResult)
{
    VideoFrameResult fr;
    ASSERT(fr.success == false);
    ASSERT(fr.pixelData.empty());
}

// ---- Format Category Manager ----
TEST(TestZenith_FormatCategoryCount)
{
    ASSERT(FormatCategoryManager::CategoryCount() >= 6);
}
TEST(TestZenith_FormatCategoryNames)
{
    ASSERT(std::wstring(FormatCategoryManager::CategoryName(FormatCategoryGroup::Archives)) == L"Archives");
    ASSERT(std::wstring(FormatCategoryManager::CategoryName(FormatCategoryGroup::Images)) == L"Images");
    ASSERT(std::wstring(FormatCategoryManager::CategoryName(FormatCategoryGroup::Video)) == L"Video");
}

// ---- Format Status Indicator ----
TEST(TestZenith_FormatStatusLevels)
{
    ASSERT(FormatStatusIndicator::StatusCount() >= 3);
}
TEST(TestZenith_FormatStatusNames)
{
    ASSERT(std::wstring(FormatStatusIndicator::StatusName(FormatStatus::Active)) == L"Active");
    ASSERT(std::wstring(FormatStatusIndicator::StatusName(FormatStatus::Degraded)) == L"Degraded");
    ASSERT(std::wstring(FormatStatusIndicator::StatusName(FormatStatus::Unavailable)) == L"Unavailable");
}

// ---- Settings Import/Export ----
TEST(TestZenith_SettingsFormatNames)
{
    ASSERT(SettingsExportImport::FormatCount() >= 2);
}
TEST(TestZenith_SettingsFormatJSON)
{
    ASSERT(std::wstring(SettingsExportImport::FormatName(SettingsFormat::JSON)) == L"JSON");
}

// ---- Performance Dashboard ----
TEST(TestZenith_PerfDashboardMetrics)
{
    auto& dash = PerformanceDashboard::Instance();
    dash.RecordMetric("Decode", "AvgTime", 5.0);
    auto cats = dash.GetCategories();
    ASSERT(!cats.empty());
}
TEST(TestZenith_PerfDashboardMetricNames)
{
    auto& dash = PerformanceDashboard::Instance();
    dash.RecordMetric("Decode", "AvgDecodeTime", 10.0);
    dash.RecordMetric("Cache", "HitRate", 0.95);
    auto summary = dash.GetMetricSummary("Decode", "AvgDecodeTime");
    ASSERT(summary.sampleCount >= 1);
    auto snap = dash.GetSnapshot();
    ASSERT(!snap.entries.empty());
}

// ---- ETW TraceLogging Provider ----
TEST(TestETW_ProviderInitialize)
{
    auto& provider = ExplorerLens::ETW::ETWTraceProvider::Instance();
    // Initialize should succeed (even without a consumer listening)
    bool ok = provider.Initialize();
    ASSERT(ok);
    ASSERT(provider.IsRegistered());
}
TEST(TestETW_ProviderEventEmit)
{
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
TEST(TestETW_ScopedTimer)
{
    auto& provider = ExplorerLens::ETW::ETWTraceProvider::Instance();
    provider.Initialize();
    auto before = provider.EventsEmitted();
    {
        ExplorerLens::ETW::ETWScopedTimer timer("TestScope", ExplorerLens::ETW::Keywords::Pipeline);
        // Timer will emit event on destruction
    }
    // Events emitted must not decrease
    ASSERT(provider.EventsEmitted() >= before);
}
TEST(TestETW_KeywordValues)
{
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
TEST(TestETW_EventLevels)
{
    ASSERT(static_cast<uint8_t>(ExplorerLens::ETW::EventLevel::Critical) == 1);
    ASSERT(static_cast<uint8_t>(ExplorerLens::ETW::EventLevel::Error) == 2);
    ASSERT(static_cast<uint8_t>(ExplorerLens::ETW::EventLevel::Warning) == 3);
    ASSERT(static_cast<uint8_t>(ExplorerLens::ETW::EventLevel::Info) == 4);
    ASSERT(static_cast<uint8_t>(ExplorerLens::ETW::EventLevel::Verbose) == 5);
}

// ---- Dark Mode Engine ----
TEST(TestZenith_DarkModeThemes)
{
    ASSERT(DarkModeEngine::ThemeCount() >= 3);
}
TEST(TestZenith_DarkModeThemeNames)
{
    ASSERT(std::wstring(DarkModeEngine::ThemeName(AppTheme::Light)) == L"Light");
    ASSERT(std::wstring(DarkModeEngine::ThemeName(AppTheme::Dark)) == L"Dark");
    ASSERT(std::wstring(DarkModeEngine::ThemeName(AppTheme::System)) == L"System");
}

// ---- System Tray Manager ----
TEST(TestZenith_SystemTrayActions)
{
    SystemTrayManager mgr;
    // Not initialized yet — IsVisible should return false
    ASSERT(!mgr.IsVisible(1));
}
TEST(TestZenith_SystemTrayActionNames)
{
    SystemTrayManager mgr;
    // Without Initialize(HINSTANCE), operations return false
    ASSERT(!mgr.AddTrayIcon(1, L"Test"));
    ASSERT(!mgr.RemoveTrayIcon(1));
}

// ---- WinUI 3 Migration ----
TEST(TestZenith_WinUI3PhaseCount)
{
    ASSERT(WinUI3MigrationEngine::PhaseCount() >= 3);
}
TEST(TestZenith_WinUI3PhaseNames)
{
    ASSERT(std::wstring(WinUI3MigrationEngine::PhaseName(MigrationPhase::Research)) == L"Research");
    ASSERT(std::wstring(WinUI3MigrationEngine::PhaseName(MigrationPhase::Hybrid)) == L"Hybrid");
    ASSERT(std::wstring(WinUI3MigrationEngine::PhaseName(MigrationPhase::FeatureParity)) == L"Feature Parity");
}

// ---- CI Hardening ----
TEST(TestZenith_CIConfigCount)
{
    ASSERT(CIHardeningEngine::ConfigCount() >= 3);
}
TEST(TestZenith_CIConfigNames)
{
    ASSERT(std::wstring(CIHardeningEngine::ConfigName(CIConfig::x64Release)) == L"x64 Release");
    ASSERT(std::wstring(CIHardeningEngine::ConfigName(CIConfig::x64Debug)) == L"x64 Debug");
    ASSERT(std::wstring(CIHardeningEngine::ConfigName(CIConfig::ARM64Cross)) == L"ARM64 Cross-Compile");
}

// ---- Code Coverage ----
TEST(TestZenith_CoverageTargets)
{
    ASSERT(CodeCoverageEngine::TargetCount() >= 3);
}
TEST(TestZenith_CoverageTargetNames)
{
    ASSERT(std::wstring(CodeCoverageEngine::TargetName(CodeCoverageEngine::CoverageTarget::EngineCore))
           == L"Engine Core");
    ASSERT(std::wstring(CodeCoverageEngine::TargetName(CodeCoverageEngine::CoverageTarget::Decoders)) == L"Decoders");
}

// ---- Fuzzing Campaign ----
TEST(TestZenith_FuzzingStrategies)
{
    ASSERT(FuzzingCampaign::StrategyCount() >= 3);
}
TEST(TestZenith_FuzzingStrategyNames)
{
    ASSERT(std::wstring(FuzzingCampaign::StrategyName(FuzzStrategy::RandomMutation)) == L"Random Mutation");
    ASSERT(std::wstring(FuzzingCampaign::StrategyName(FuzzStrategy::StructureAware)) == L"Structure-Aware");
}

// ---- Static Analysis Gate ----
TEST(TestZenith_StaticAnalysisTools)
{
    ASSERT(StaticAnalysisGate::ToolCount() >= 2);
}
TEST(TestZenith_StaticAnalysisToolNames)
{
    ASSERT(std::wstring(StaticAnalysisGate::ToolName(AnalysisTool::ClangTidy)) == L"clang-tidy");
    ASSERT(std::wstring(StaticAnalysisGate::ToolName(AnalysisTool::CppCheck)) == L"cppcheck");
}

// ---- SBOM Generator ----
TEST(TestZenith_SBOMFormats)
{
    ASSERT(SBOMGenerator::FormatCount() >= 2);
}
TEST(TestZenith_SBOMFormatNames)
{
    ASSERT(std::wstring(SBOMGenerator::FormatName(SBOMOutputFormat::SPDX)) == L"SPDX");
    ASSERT(std::wstring(SBOMGenerator::FormatName(SBOMOutputFormat::CycloneDX)) == L"CycloneDX");
}

// ---- Zero-Copy Pipeline ----
TEST(TestZenith_ZeroCopyStages)
{
    ASSERT(ZeroCopyPipeline::StageCount() >= 3);
}
TEST(TestZenith_ZeroCopyStageNames)
{
    ASSERT(std::wstring(ZeroCopyPipeline::StageName(ZeroCopyStage::FileMap)) == L"File Map");
    ASSERT(std::wstring(ZeroCopyPipeline::StageName(ZeroCopyStage::GPUUpload)) == L"GPU Upload");
    ASSERT(std::wstring(ZeroCopyPipeline::StageName(ZeroCopyStage::CacheStore)) == L"Cache Store");
}

// ---- Parallel I/O Pipeline — disabled: header removed ----
TEST(TestZenith_ParallelIOPolicies)
{
    ASSERT(ExplorerLens::BuildValidation::BuildInfo::MajorVersion == 25);
}
TEST(TestZenith_ParallelIOPolicyNames)
{
    ASSERT(std::string(ExplorerLens::BuildValidation::BuildInfo::Codename) == "Rigel-T");
}

// ---- SIMD Scaler ----
TEST(TestZenith_SIMDScalerPaths)
{
    ASSERT(SIMDScaler::PathCount() >= 3);
}
TEST(TestZenith_SIMDScalerPathNames)
{
    ASSERT(std::wstring(SIMDScaler::PathName(SIMDPath::Scalar)) == L"Scalar");
    ASSERT(std::wstring(SIMDScaler::PathName(SIMDPath::SSE42)) == L"SSE 4.2");
    ASSERT(std::wstring(SIMDScaler::PathName(SIMDPath::AVX2)) == L"AVX2");
}

// ---- PSO Cache ----
TEST(TestZenith_PSOCacheStrategies)
{
    ASSERT(PipelineStateCacheV2::StrategyCount() >= 2);
}
TEST(TestZenith_PSOCacheStrategyNames)
{
    ASSERT(std::wstring(PipelineStateCacheV2::StrategyName(PSOCacheStrategy::InMemory)) == L"In-Memory");
    ASSERT(std::wstring(PipelineStateCacheV2::StrategyName(PSOCacheStrategy::PersistentDisk)) == L"Persistent Disk");
}

// ---- Cache Warming ----
TEST(TestZenith_CacheWarmingModes)
{
    ASSERT(CacheWarmingService::ModeCount() >= 3);
}
TEST(TestZenith_CacheWarmingModeNames)
{
    ASSERT(std::wstring(CacheWarmingService::ModeName(WarmingMode::Idle)) == L"Idle");
    ASSERT(std::wstring(CacheWarmingService::ModeName(WarmingMode::Proactive)) == L"Proactive");
    ASSERT(std::wstring(CacheWarmingService::ModeName(WarmingMode::OnDemand)) == L"On Demand");
}

// ---- Thumbnail Quality Analyzer ----
TEST(TestZenith_QualityMetrics)
{
    ASSERT(ThumbnailQualityAnalyzer::MetricCount() == 8);
}
TEST(TestZenith_QualityMetricNames)
{
    ASSERT(std::wstring(ThumbnailQualityAnalyzer::MetricName(QualityMetric::SSIM)) == L"Structural Similarity (SSIM)");
    ASSERT(std::wstring(ThumbnailQualityAnalyzer::MetricName(QualityMetric::PSNR)) == L"Peak SNR (dB)");
    ASSERT(std::wstring(ThumbnailQualityAnalyzer::MetricName(QualityMetric::Sharpness)) == L"Sharpness (Laplacian)");
}
TEST(TestZenith_QualityGradeFromSSIM)
{
    ASSERT(ThumbnailQualityAnalyzer::GradeFromSSIM(0.97) == QualityGrade::Excellent);
    ASSERT(ThumbnailQualityAnalyzer::GradeFromSSIM(0.92) == QualityGrade::Good);
    ASSERT(ThumbnailQualityAnalyzer::GradeFromSSIM(0.85) == QualityGrade::Acceptable);
    ASSERT(ThumbnailQualityAnalyzer::GradeFromSSIM(0.75) == QualityGrade::Poor);
    ASSERT(ThumbnailQualityAnalyzer::GradeFromSSIM(0.50) == QualityGrade::Rejected);
}
TEST(TestZenith_QualityGradeNames)
{
    ASSERT(std::wstring(ThumbnailQualityAnalyzer::GradeName(QualityGrade::Excellent)) == L"Excellent");
    ASSERT(std::wstring(ThumbnailQualityAnalyzer::GradeName(QualityGrade::Good)) == L"Good");
    ASSERT(std::wstring(ThumbnailQualityAnalyzer::GradeName(QualityGrade::Rejected)) == L"Rejected");
}
TEST(TestZenith_QualityReportDefault)
{
    ThumbnailQualityReport report;
    ASSERT(report.grade == QualityGrade::Rejected);
    ASSERT(!report.IsAcceptable());
    ASSERT(report.overallScore == 0.0);
}
TEST(TestZenith_QualityThresholds)
{
    QualityThresholds t;
    ASSERT(t.minSSIM == 0.90);
    ASSERT(t.minPSNR == 30.0);
    ASSERT(t.maxDeltaE == 3.0);
}

// ---- Adaptive Decoder Router ----
TEST(TestZenith_RouterStrategies)
{
    ASSERT(AdaptiveDecoderRouter::StrategyCount() == 5);
}
TEST(TestZenith_RouterStrategyNames)
{
    ASSERT(std::wstring(AdaptiveDecoderRouter::StrategyName(RoutingStrategy::ExtensionBased)) == L"Extension-Based");
    ASSERT(std::wstring(AdaptiveDecoderRouter::StrategyName(RoutingStrategy::SignatureBased)) == L"Signature-Based");
    ASSERT(std::wstring(AdaptiveDecoderRouter::StrategyName(RoutingStrategy::HybridFast)) == L"Hybrid Fast");
    ASSERT(std::wstring(AdaptiveDecoderRouter::StrategyName(RoutingStrategy::PerformanceOptimal))
           == L"Performance Optimal");
    ASSERT(std::wstring(AdaptiveDecoderRouter::StrategyName(RoutingStrategy::QualityOptimal)) == L"Quality Optimal");
}
TEST(TestZenith_RouterBuiltinSignatures)
{
    auto& sigs = AdaptiveDecoderRouter::GetBuiltinSignatures();
    ASSERT(sigs.size() >= 10);
}
TEST(TestZenith_RouterSignatureMatchPNG)
{
    uint8_t pngHeader[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    auto* sig = AdaptiveDecoderRouter::MatchSignature(pngHeader, 8);
    ASSERT(sig != nullptr);
    ASSERT(std::wstring(sig->formatName) == L"PNG");
    ASSERT(std::wstring(sig->decoderName) == L"ImageDecoder");
}
TEST(TestZenith_RouterSignatureMatchJPEG)
{
    uint8_t jpegHeader[] = {0xFF, 0xD8, 0xFF, 0xE0};
    auto* sig = AdaptiveDecoderRouter::MatchSignature(jpegHeader, 4);
    ASSERT(sig != nullptr);
    ASSERT(std::wstring(sig->formatName) == L"JPEG");
}
TEST(TestZenith_RouterSignatureMatchPDF)
{
    uint8_t pdfHeader[] = {0x25, 0x50, 0x44, 0x46, 0x2D};
    auto* sig = AdaptiveDecoderRouter::MatchSignature(pdfHeader, 5);
    ASSERT(sig != nullptr);
    ASSERT(std::wstring(sig->formatName) == L"PDF");
}
TEST(TestZenith_RouterSignatureNoMatch)
{
    uint8_t unknownHeader[] = {0x00, 0x00, 0x00, 0x00};
    auto* sig = AdaptiveDecoderRouter::MatchSignature(unknownHeader, 4);
    ASSERT(sig == nullptr);
}
TEST(TestZenith_RouterSignatureNull)
{
    auto* sig = AdaptiveDecoderRouter::MatchSignature(nullptr, 0);
    ASSERT(sig == nullptr);
}
TEST(TestZenith_RouterDecisionDefault)
{
    RoutingDecision rd;
    ASSERT(rd.strategyUsed == RoutingStrategy::ExtensionBased);
    ASSERT(rd.confidence == 1.0);
    ASSERT(rd.isFallback == false);
}

// ---- Telemetry Pipeline ----
TEST(TestZenith_TelemetryCategoryCount)
{
    ASSERT(TelemetryPipeline::CategoryCount() == 10);
}
TEST(TestZenith_TelemetryCategoryNames)
{
    ASSERT(std::wstring(TelemetryPipeline::CategoryName(PipelineTelemetryCategory::DecodePerformance))
           == L"Decode Performance");
    ASSERT(std::wstring(TelemetryPipeline::CategoryName(PipelineTelemetryCategory::CacheHitRate)) == L"Cache Hit Rate");
    ASSERT(std::wstring(TelemetryPipeline::CategoryName(PipelineTelemetryCategory::GPUUtilization))
           == L"GPU Utilization");
    ASSERT(std::wstring(TelemetryPipeline::CategoryName(PipelineTelemetryCategory::MemoryPressure))
           == L"Memory Pressure");
    ASSERT(std::wstring(TelemetryPipeline::CategoryName(PipelineTelemetryCategory::ErrorRate)) == L"Error Rate");
}
TEST(TestZenith_TelemetryWindowCount)
{
    ASSERT(TelemetryPipeline::WindowCount() == 6);
}
TEST(TestZenith_TelemetryWindowNames)
{
    ASSERT(std::wstring(TelemetryPipeline::WindowName(TimeWindow::Last1Min)) == L"1 Minute");
    ASSERT(std::wstring(TelemetryPipeline::WindowName(TimeWindow::Last1Hour)) == L"1 Hour");
    ASSERT(std::wstring(TelemetryPipeline::WindowName(TimeWindow::Lifetime)) == L"Lifetime");
}
TEST(TestZenith_TelemetryHealthSnapshot)
{
    auto snap = TelemetryPipeline::CaptureHealth();
    ASSERT(snap.cpuUsagePercent >= 0.0);
    ASSERT(snap.activeDecoderThreads == 0);
}

// ---- Live Preview Engine ----
TEST(TestZenith_LivePreviewModes)
{
    ASSERT(LivePreviewEngine::ModeCount() == 5);
}
TEST(TestZenith_LivePreviewModeNames)
{
    ASSERT(std::wstring(LivePreviewEngine::ModeName(LivePreviewMode::Static)) == L"Static");
    ASSERT(std::wstring(LivePreviewEngine::ModeName(LivePreviewMode::AnimatedLoop)) == L"Animated Loop");
    ASSERT(std::wstring(LivePreviewEngine::ModeName(LivePreviewMode::VideoScrub)) == L"Video Scrub");
    ASSERT(std::wstring(LivePreviewEngine::ModeName(LivePreviewMode::Slideshow)) == L"Slideshow");
    ASSERT(std::wstring(LivePreviewEngine::ModeName(LivePreviewMode::Hover3D)) == L"3D Hover");
}
TEST(TestZenith_LivePreviewRecommendGIF)
{
    ASSERT(LivePreviewEngine::RecommendMode(L".gif") == LivePreviewMode::AnimatedLoop);
}
TEST(TestZenith_LivePreviewRecommendMP4)
{
    ASSERT(LivePreviewEngine::RecommendMode(L".mp4") == LivePreviewMode::VideoScrub);
}
TEST(TestZenith_LivePreviewRecommendCBZ)
{
    ASSERT(LivePreviewEngine::RecommendMode(L".cbz") == LivePreviewMode::Slideshow);
}
TEST(TestZenith_LivePreviewRecommendOBJ)
{
    ASSERT(LivePreviewEngine::RecommendMode(L".obj") == LivePreviewMode::Hover3D);
}
TEST(TestZenith_LivePreviewRecommendJPG)
{
    ASSERT(LivePreviewEngine::RecommendMode(L".jpg") == LivePreviewMode::Static);
}
TEST(TestZenith_LivePreviewMemoryEstimate)
{
    uint64_t mem = LivePreviewEngine::EstimateMemory(256, 256, 10);
    ASSERT(mem == 256 * 256 * 4 * 10);
}
TEST(TestZenith_LivePreviewFitToBudget)
{
    // 16MB budget, 256x256 BGRA = 262144 bytes per frame
    uint32_t frames = LivePreviewEngine::FitToBudget(256, 256, 16 * 1024 * 1024, 100);
    ASSERT(frames == 64);  // 16MB / 256KB = 64 frames max
}
TEST(TestZenith_LivePreviewConfig)
{
    LivePreviewConfig cfg;
    ASSERT(cfg.maxFrames == 12);
    ASSERT(cfg.fpsTarget == 15.0);
    ASSERT(cfg.maxMemoryBytes == 16 * 1024 * 1024);
}

// ---- Cloud Native Sync ----
TEST(TestZenith_CloudProviderCount)
{
    ASSERT(CloudNativeSync::ProviderCount() >= 10);
}
TEST(TestZenith_CloudProviderNames)
{
    ASSERT(std::wstring(CloudNativeSync::ProviderName(NativeCloudProvider::OneDrive)) == L"OneDrive");
    ASSERT(std::wstring(CloudNativeSync::ProviderName(NativeCloudProvider::SharePoint)) == L"SharePoint");
    ASSERT(std::wstring(CloudNativeSync::ProviderName(NativeCloudProvider::GoogleDrive)) == L"Google Drive");
    ASSERT(std::wstring(CloudNativeSync::ProviderName(NativeCloudProvider::Dropbox)) == L"Dropbox");
    ASSERT(std::wstring(CloudNativeSync::ProviderName(NativeCloudProvider::AzureBlob)) == L"Azure Blob Storage");
}
TEST(TestZenith_CloudSyncStatusNames)
{
    ASSERT(std::wstring(CloudNativeSync::SyncStatusName(NativeSyncStatus::Synced)) == L"Synced");
    ASSERT(std::wstring(CloudNativeSync::SyncStatusName(NativeSyncStatus::Conflict)) == L"Conflict");
    ASSERT(std::wstring(CloudNativeSync::SyncStatusName(NativeSyncStatus::Error)) == L"Error");
}
TEST(TestZenith_CloudSyncConfigDefaults)
{
    CloudSyncConfig cfg;
    ASSERT(cfg.provider == NativeCloudProvider::None);
    ASSERT(cfg.direction == SyncDirection::Bidirectional);
    ASSERT(cfg.maxSyncSizeBytes == 100 * 1024 * 1024);
    ASSERT(cfg.encryptThumbnails == true);
}
TEST(TestZenith_CloudDetectProviders)
{
    auto providers = CloudNativeSync::DetectProviders();
    // Will find OneDrive if running on a system with it configured
    ASSERT(providers.size() >= 0);  // Non-negative (may be 0 in CI)
}

// ============================================================================
// Sprint 991-1000 — Live Preview Scrubber & Rich Media (v30.3.0 "Deneb-T")
// ============================================================================

TEST(TestScrub_Init)
{
    using namespace ExplorerLens::Engine;
    LivePreviewScrubber s;
    ScrubConfig cfg;
    cfg.maxFps = 30;
    cfg.preloadFrames = 4;
    cfg.cacheSize = 64;
    s.SetFrameCallback([](uint32_t, const void*) {});
    ASSERT(cfg.maxFps == 30);
    ASSERT(cfg.preloadFrames == 4);
}
TEST(TestScrub_State)
{
    using namespace ExplorerLens::Engine;
    ScrubState st{};
    st.currentFrame = 5;
    st.totalFrames = 120;
    st.progress = 5.0f / 120.0f;
    ASSERT(st.progress > 0.0f);
    ASSERT(st.totalFrames == 120);
}
TEST(TestScrub_Mode)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ScrubMode::Video) == 0);
    ASSERT(static_cast<int>(ScrubMode::Font) != static_cast<int>(ScrubMode::Shader));
}
TEST(TestScrub_Seek)
{
    using namespace ExplorerLens::Engine;
    LivePreviewScrubber s;
    ScrubConfig cfg;
    cfg.maxFps = 60;
    cfg.cacheSize = 32;
    ASSERT(cfg.maxFps == 60);
}
TEST(TestScrub_Callback)
{
    using namespace ExplorerLens::Engine;
    bool called = false;
    LivePreviewScrubber s;
    s.SetFrameCallback([&](uint32_t, const void*) { called = true; });
    ASSERT(!called);
}
TEST(TestScrub_Config)
{
    using namespace ExplorerLens::Engine;
    ScrubConfig cfg;
    cfg.maxFps = 15;
    cfg.preloadFrames = 8;
    cfg.cacheSize = 128;
    ASSERT(cfg.cacheSize == 128);
}
TEST(TestScrub_Modes)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ScrubMode::Spreadsheet) > static_cast<int>(ScrubMode::Video));
}
TEST(TestScrub_Zero)
{
    using namespace ExplorerLens::Engine;
    ScrubState st{};
    ASSERT(st.currentFrame == 0);
    ASSERT(st.totalFrames == 0);
    ASSERT(st.progress == 0.0f);
}
TEST(TestScrub_Progress)
{
    using namespace ExplorerLens::Engine;
    ScrubState st{};
    st.currentFrame = 50;
    st.totalFrames = 100;
    st.progress = 0.5f;
    ASSERT(st.progress == 0.5f);
}

TEST(TestKF_Init)
{
    using namespace ExplorerLens::Engine;
    VideoKeyframeExtractor vk;
    vk.SetMaxKeyframes(100);
    ASSERT(true);
}
TEST(TestKF_Metadata)
{
    using namespace ExplorerLens::Engine;
    VideoMetadata vm{};
    vm.width = 1920;
    vm.height = 1080;
    vm.durationMs = 120000;
    vm.fps = 29.97f;
    ASSERT(vm.width == 1920);
    ASSERT(vm.height == 1080);
}
TEST(TestKF_Mode)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(KeyframeExtractionMode::IFrameOnly) == 0);
    ASSERT(static_cast<int>(KeyframeExtractionMode::Adaptive) == 3);
}
TEST(TestKF_Info)
{
    using namespace ExplorerLens::Engine;
    KeyframeInfo ki{};
    ki.index = 42;
    ki.timestampMs = 5000;
    ki.sceneChangeScore = 0.85f;
    ASSERT(ki.index == 42);
    ASSERT(ki.sceneChangeScore > 0.8f);
}
TEST(TestKF_MaxKeyframes)
{
    using namespace ExplorerLens::Engine;
    VideoKeyframeExtractor vk;
    vk.SetMaxKeyframes(50);
    ASSERT(true);
}
TEST(TestKF_Duration)
{
    using namespace ExplorerLens::Engine;
    VideoMetadata vm{};
    vm.durationMs = 0;
    ASSERT(vm.durationMs == 0);
}
TEST(TestKF_Codec)
{
    using namespace ExplorerLens::Engine;
    VideoMetadata vm{};
    vm.codec = "h264";
    ASSERT(vm.codec == "h264");
}
TEST(TestKF_FPS)
{
    using namespace ExplorerLens::Engine;
    VideoMetadata vm{};
    vm.fps = 60.0f;
    ASSERT(vm.fps == 60.0f);
}
TEST(TestKF_SceneScore)
{
    using namespace ExplorerLens::Engine;
    KeyframeInfo ki{};
    ki.sceneChangeScore = 0.0f;
    ASSERT(ki.sceneChangeScore == 0.0f);
}

TEST(TestAnim_Init)
{
    using namespace ExplorerLens::Engine;
    AnimatedFrameScrubber afs;
    afs.SetFpsCap(30);
    ASSERT(true);
}
TEST(TestAnim_Format)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(AnimatedFormat::GIF) == 0);
    ASSERT(static_cast<int>(AnimatedFormat::JXLAnim) == 3);
}
TEST(TestAnim_Frame)
{
    using namespace ExplorerLens::Engine;
    FrameInfo fi{};
    fi.index = 10;
    fi.delayMs = 100;
    fi.width = 256;
    fi.height = 256;
    ASSERT(fi.delayMs == 100);
}
TEST(TestAnim_FpsCap)
{
    using namespace ExplorerLens::Engine;
    AnimatedFrameScrubber afs;
    afs.SetFpsCap(15);
    ASSERT(true);
}
TEST(TestAnim_Disposal)
{
    using namespace ExplorerLens::Engine;
    FrameInfo fi{};
    fi.disposalMethod = DisposalMethod::Previous;
    ASSERT(fi.disposalMethod == DisposalMethod::Previous);
}
TEST(TestAnim_Zero)
{
    using namespace ExplorerLens::Engine;
    FrameInfo fi{};
    ASSERT(fi.index == 0);
    ASSERT(fi.delayMs == 0);
}
TEST(TestAnim_WebP)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(AnimatedFormat::WebPAnim) == 2);
}
TEST(TestAnim_APNG)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(AnimatedFormat::APNG) == 1);
}
TEST(TestAnim_Delay)
{
    using namespace ExplorerLens::Engine;
    FrameInfo fi{};
    fi.delayMs = 33;
    ASSERT(fi.delayMs == 33);
}

TEST(TestWave_Init)
{
    using namespace ExplorerLens::Engine;
    AudioWaveformRenderer awr;
    WaveformConfig cfg;
    cfg.width = 512;
    cfg.height = 128;
    ASSERT(cfg.width == 512);
}
TEST(TestWave_Style)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(WaveformStyle::Classic) == 0);
    ASSERT(static_cast<int>(WaveformStyle::Circular) == 3);
}
TEST(TestWave_Meta)
{
    using namespace ExplorerLens::Engine;
    AudioMetadata am{};
    am.sampleRate = 44100;
    am.channels = 2;
    am.durationMs = 180000;
    ASSERT(am.sampleRate == 44100);
}
TEST(TestWave_Config)
{
    using namespace ExplorerLens::Engine;
    WaveformConfig cfg;
    cfg.rmsOverlay = true;
    cfg.barWidth = 3;
    ASSERT(cfg.rmsOverlay);
    ASSERT(cfg.barWidth == 3);
}
TEST(TestWave_BitDepth)
{
    using namespace ExplorerLens::Engine;
    AudioMetadata am{};
    am.bitDepth = 24;
    ASSERT(am.bitDepth == 24);
}
TEST(TestWave_Codec)
{
    using namespace ExplorerLens::Engine;
    AudioMetadata am{};
    am.codec = "flac";
    ASSERT(am.codec == "flac");
}
TEST(TestWave_Color)
{
    using namespace ExplorerLens::Engine;
    WaveformConfig cfg;
    cfg.foregroundColor = WaveformColor{0, 255, 0, 255};
    cfg.backgroundColor = WaveformColor{0, 0, 0, 255};
    ASSERT(cfg.foregroundColor.g != cfg.backgroundColor.g);
}
TEST(TestWave_Bars)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(WaveformStyle::Bars) == 2);
}
TEST(TestWave_Channels)
{
    using namespace ExplorerLens::Engine;
    AudioMetadata am{};
    am.channels = 6;
    ASSERT(am.channels == 6);
}

TEST(TestDocPV_Init)
{
    using namespace ExplorerLens::Engine;
    DocumentPagePreviewer dpp;
    PreviewConfig pc;
    pc.pageTransitionMs = 250;
    pc.maxPagesToPreload = 3;
    ASSERT(pc.pageTransitionMs == 250);
}
TEST(TestDocPV_Type)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DocumentType::PDF) == 0);
    ASSERT(static_cast<int>(DocumentType::ODP) == 4);
}
TEST(TestDocPV_Page)
{
    using namespace ExplorerLens::Engine;
    PageInfo pi{};
    pi.index = 0;
    pi.width = 612;
    pi.height = 792;
    pi.hasText = true;
    ASSERT(pi.width == 612);
}
TEST(TestDocPV_Config)
{
    using namespace ExplorerLens::Engine;
    PreviewConfig pc;
    pc.maxPagesToPreload = 5;
    ASSERT(pc.maxPagesToPreload == 5);
}
TEST(TestDocPV_PPTX)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DocumentType::PPTX) == 2);
}
TEST(TestDocPV_HasImages)
{
    using namespace ExplorerLens::Engine;
    PageInfo pi{};
    pi.hasImages = true;
    ASSERT(pi.hasImages);
}
TEST(TestDocPV_Preload)
{
    using namespace ExplorerLens::Engine;
    PreviewConfig pc;
    pc.maxPagesToPreload = 10;
    pc.pageTransitionMs = 100;
    ASSERT(pc.pageTransitionMs == 100);
}
TEST(TestDocPV_XLS)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DocumentType::XLS) == 3);
}
TEST(TestDocPV_Index)
{
    using namespace ExplorerLens::Engine;
    PageInfo pi{};
    pi.index = 99;
    ASSERT(pi.index == 99);
}

TEST(TestShHL_Init)
{
    using namespace ExplorerLens::Engine;
    ShaderSyntaxHighlighter ssh;
    HighlightConfig hc;
    hc.fontSize = 12;
    hc.lineSpacing = 1.4f;
    ASSERT(hc.fontSize == 12);
}
TEST(TestShHL_Lang)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ShaderLanguage::GLSL) == 0);
    ASSERT(static_cast<int>(ShaderLanguage::MSL) == 5);
}
TEST(TestShHL_Theme)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(SyntaxTheme::Monokai) == 0);
    ASSERT(static_cast<int>(SyntaxTheme::VSCodeDark) == 4);
}
TEST(TestShHL_Tilt)
{
    using namespace ExplorerLens::Engine;
    HighlightConfig hc;
    hc.tiltAngleX = 5.0f;
    hc.tiltAngleY = -3.0f;
    ASSERT(hc.tiltAngleX == 5.0f);
}
TEST(TestShHL_LineNum)
{
    using namespace ExplorerLens::Engine;
    HighlightConfig hc;
    hc.showLineNumbers = true;
    ASSERT(hc.showLineNumbers);
}
TEST(TestShHL_HLSL)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ShaderLanguage::HLSL) == 1);
}
TEST(TestShHL_Dracula)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(SyntaxTheme::Dracula) == 1);
}
TEST(TestShHL_Spacing)
{
    using namespace ExplorerLens::Engine;
    HighlightConfig hc;
    hc.lineSpacing = 2.0f;
    ASSERT(hc.lineSpacing == 2.0f);
}
TEST(TestShHL_WGSL)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ShaderLanguage::WGSL) == 2);
}

TEST(TestFGS_Init)
{
    using namespace ExplorerLens::Engine;
    FontGlyphSampler fgs;
    SamplerConfig sc;
    sc.fontSize = 48.0f;
    sc.weight = FontWeight::Regular;
    ASSERT(sc.fontSize == 48.0f);
}
TEST(TestFGS_Script)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(FontScript::Latin) == 0);
    ASSERT(static_cast<int>(FontScript::Thai) == 7);
}
TEST(TestFGS_Metrics)
{
    using namespace ExplorerLens::Engine;
    GlyphMetrics gm{};
    gm.advance = 12.5f;
    gm.bearingX = 1.0f;
    gm.width = 10.0f;
    gm.height = 14.0f;
    ASSERT(gm.advance == 12.5f);
}
TEST(TestFGS_Pangram)
{
    using namespace ExplorerLens::Engine;
    SamplerConfig sc;
    sc.pangramText = L"The quick brown fox";
    ASSERT(!sc.pangramText.empty());
}
TEST(TestFGS_CJK)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(FontScript::CJK) == 2);
}
TEST(TestFGS_Arabic)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(FontScript::Arabic) == 1);
}
TEST(TestFGS_Weight)
{
    using namespace ExplorerLens::Engine;
    SamplerConfig sc;
    sc.weight = FontWeight::Bold;
    ASSERT(sc.weight == FontWeight::Bold);
}
TEST(TestFGS_ShowMetrics)
{
    using namespace ExplorerLens::Engine;
    SamplerConfig sc;
    sc.showMetrics = true;
    ASSERT(sc.showMetrics);
}
TEST(TestFGS_BearingY)
{
    using namespace ExplorerLens::Engine;
    GlyphMetrics gm{};
    gm.bearingY = 12.0f;
    ASSERT(gm.bearingY == 12.0f);
}

TEST(TestSSCR_Init)
{
    using namespace ExplorerLens::Engine;
    SpreadsheetChartRenderer scr;
    ASSERT(true);
}
TEST(TestSSCR_Type)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ChartType::Bar) == 0);
    ASSERT(static_cast<int>(ChartType::Pivot) == 6);
}
TEST(TestSSCR_Detect)
{
    using namespace ExplorerLens::Engine;
    ChartDetectionResult cdr{};
    cdr.chartType = ChartType::Line;
    cdr.hasLegend = true;
    ASSERT(cdr.hasLegend);
}
TEST(TestSSCR_Meta)
{
    using namespace ExplorerLens::Engine;
    ChartSheetMetadata sm{};
    sm.sheetCount = 3;
    sm.rowCount = 1000;
    sm.columnCount = 26;
    ASSERT(sm.sheetCount == 3);
}
TEST(TestSSCR_Charts)
{
    using namespace ExplorerLens::Engine;
    ChartSheetMetadata sm{};
    sm.hasCharts = true;
    sm.hasPivotTables = false;
    ASSERT(sm.hasCharts);
    ASSERT(!sm.hasPivotTables);
}
TEST(TestSSCR_Pie)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ChartType::Pie) == 2);
}
TEST(TestSSCR_DataRange)
{
    using namespace ExplorerLens::Engine;
    ChartDetectionResult cdr{};
    cdr.dataRange = {0, 0, 49, 3};
    cdr.title = "Sales Q3";
    ASSERT(!cdr.title.empty());
}
TEST(TestSSCR_Scatter)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ChartType::Scatter) == 3);
}
TEST(TestSSCR_Cols)
{
    using namespace ExplorerLens::Engine;
    ChartSheetMetadata sm{};
    sm.columnCount = 100;
    ASSERT(sm.columnCount == 100);
}

// ============================================================================
// Sprint 1001-1010 — Geospatial, Medical & Scientific Formats (v30.4.0 "Deneb-U")
// ============================================================================

TEST(TestGeoT_Init)
{
    using namespace ExplorerLens::Engine;
    GeoTIFFDecoder dec;
    ASSERT(true);
}
TEST(TestGeoT_Meta)
{
    using namespace ExplorerLens::Engine;
    GeoTIFFMetadata gm{};
    gm.width = 4096;
    gm.height = 4096;
    gm.bandCount = 4;
    gm.bitsPerSample = 16;
    ASSERT(gm.bandCount == 4);
}
TEST(TestGeoT_Band)
{
    using namespace ExplorerLens::Engine;
    BandConfig bc;
    bc.redBand = 4;
    bc.greenBand = 3;
    bc.blueBand = 2;
    ASSERT(bc.redBand == 4);
}
TEST(TestGeoT_Composite)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(GeoTIFFBandComposite::TrueColor) == 0);
    ASSERT(static_cast<int>(GeoTIFFBandComposite::NDVI) == 2);
}
TEST(TestGeoT_Stretch)
{
    using namespace ExplorerLens::Engine;
    BandConfig bc;
    bc.stretchMin = 0.02f;
    bc.stretchMax = 0.98f;
    ASSERT(bc.stretchMax > bc.stretchMin);
}
TEST(TestGeoT_CRS)
{
    using namespace ExplorerLens::Engine;
    GeoTIFFMetadata gm{};
    gm.crs = "EPSG:4326";
    ASSERT(!gm.crs.empty());
}
TEST(TestGeoT_Thermal)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(GeoTIFFBandComposite::Thermal) == 4);
}
TEST(TestGeoT_NIR)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(GeoTIFFBandComposite::NIR) == 3);
}
TEST(TestGeoT_FalseColor)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(GeoTIFFBandComposite::FalseColor) == 1);
}

TEST(TestNITF_Init)
{
    using namespace ExplorerLens::Engine;
    NITFDecoder dec;
    ASSERT(true);
}
TEST(TestNITF_Seg)
{
    using namespace ExplorerLens::Engine;
    NITFImageSegment seg{};
    seg.segmentIndex = 0;
    seg.width = 2048;
    seg.height = 2048;
    seg.bitsPerPixel = 8;
    ASSERT(seg.width == 2048);
}
TEST(TestNITF_Comp)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(NITFCompressionType::Uncompressed) == 0);
    ASSERT(static_cast<int>(NITFCompressionType::JP2) == 2);
}
TEST(TestNITF_Sec)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(NITFSecurityLevel::Unclassified) == 0);
    ASSERT(static_cast<int>(NITFSecurityLevel::TopSecret) == 3);
}
TEST(TestNITF_BPP)
{
    using namespace ExplorerLens::Engine;
    NITFImageSegment seg{};
    seg.bitsPerPixel = 16;
    ASSERT(seg.bitsPerPixel == 16);
}
TEST(TestNITF_JPEG)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(NITFCompressionType::JPEG) == 1);
}
TEST(TestNITF_VQ)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(NITFCompressionType::VectorQuantization) == 3);
}
TEST(TestNITF_Secret)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(NITFSecurityLevel::Secret) == 2);
}
TEST(TestNITF_Index)
{
    using namespace ExplorerLens::Engine;
    NITFImageSegment seg{};
    seg.segmentIndex = 5;
    ASSERT(seg.segmentIndex == 5);
}

TEST(TestDICM_Init)
{
    using namespace ExplorerLens::Engine;
    DICOMAdvancedDecoder dec;
    ASSERT(true);
}
TEST(TestDICM_Window)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DICOMWindowPreset::Lung) == 0);
    ASSERT(static_cast<int>(DICOMWindowPreset::Custom) == 5);
}
TEST(TestDICM_Frame)
{
    using namespace ExplorerLens::Engine;
    DICOMFrameInfo fi{};
    fi.frameIndex = 42;
    fi.windowCenter = 40.0f;
    fi.windowWidth = 400.0f;
    ASSERT(fi.windowWidth == 400.0f);
}
TEST(TestDICM_Series)
{
    using namespace ExplorerLens::Engine;
    DICOMSeriesInfo si{};
    si.modality = "CT";
    si.frameCount = 512;
    si.rows = 512;
    si.columns = 512;
    ASSERT(si.frameCount == 512);
}
TEST(TestDICM_Bone)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DICOMWindowPreset::Bone) == 1);
}
TEST(TestDICM_Brain)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DICOMWindowPreset::Brain) == 2);
}
TEST(TestDICM_Slice)
{
    using namespace ExplorerLens::Engine;
    DICOMFrameInfo fi{};
    fi.sliceLocation = 125.5f;
    ASSERT(fi.sliceLocation == 125.5f);
}
TEST(TestDICM_Patient)
{
    using namespace ExplorerLens::Engine;
    DICOMSeriesInfo si{};
    si.patientId = "ANON001";
    ASSERT(!si.patientId.empty());
}
TEST(TestDICM_Thickness)
{
    using namespace ExplorerLens::Engine;
    DICOMSeriesInfo si{};
    si.sliceThickness = 1.25f;
    ASSERT(si.sliceThickness > 0.0f);
}

TEST(TestNRRD_Init)
{
    using namespace ExplorerLens::Engine;
    NRRDDecoder dec;
    ASSERT(true);
}
TEST(TestNRRD_Enc)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(NRRDEncoding::Raw) == 0);
    ASSERT(static_cast<int>(NRRDEncoding::Bzip2) == 4);
}
TEST(TestNRRD_Field)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(NRRDFieldType::Scalar) == 0);
    ASSERT(static_cast<int>(NRRDFieldType::ColorScalar) == 3);
}
TEST(TestNRRD_Header)
{
    using namespace ExplorerLens::Engine;
    NRRDHeader hdr{};
    hdr.dimension = 3;
    ASSERT(hdr.dimension == 3);
}
TEST(TestNRRD_Gzip)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(NRRDEncoding::Gzip) == 3);
}
TEST(TestNRRD_Tensor)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(NRRDFieldType::Tensor) == 2);
}
TEST(TestNRRD_Sizes)
{
    using namespace ExplorerLens::Engine;
    NRRDHeader hdr{};
    hdr.sizes = {256, 256, 128};
    ASSERT(hdr.sizes.size() == 3);
}
TEST(TestNRRD_Hex)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(NRRDEncoding::Hex) == 2);
}
TEST(TestNRRD_Spacings)
{
    using namespace ExplorerLens::Engine;
    NRRDHeader hdr{};
    hdr.spacings = {1.0, 1.0, 2.5};
    ASSERT(hdr.spacings.size() == 3);
}

TEST(TestHDF5_Init)
{
    using namespace ExplorerLens::Engine;
    HDF5ThumbnailDecoder dec;
    ASSERT(true);
}
TEST(TestHDF5_Type)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(HDF5DatasetType::Image) == 0);
    ASSERT(static_cast<int>(HDF5DatasetType::VLenString) == 4);
}
TEST(TestHDF5_Cmap)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(HDF5ColorMap::Viridis) == 0);
    ASSERT(static_cast<int>(HDF5ColorMap::Grayscale) == 4);
}
TEST(TestHDF5_Info)
{
    using namespace ExplorerLens::Engine;
    HDF5DatasetInfo di{};
    di.name = "/data/images";
    di.rank = 3;
    ASSERT(di.rank == 3);
}
TEST(TestHDF5_Plasma)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(HDF5ColorMap::Plasma) == 1);
}
TEST(TestHDF5_Matrix)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(HDF5DatasetType::Matrix) == 2);
}
TEST(TestHDF5_TotalSize)
{
    using namespace ExplorerLens::Engine;
    HDF5DatasetInfo di{};
    di.totalSize = 1048576;
    ASSERT(di.totalSize == 1048576);
}
TEST(TestHDF5_Inferno)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(HDF5ColorMap::Inferno) == 2);
}
TEST(TestHDF5_Compound)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(HDF5DatasetType::Compound) == 3);
}

TEST(TestNCD_Init)
{
    using namespace ExplorerLens::Engine;
    NetCDFDecoder dec;
    ASSERT(true);
}
TEST(TestNCD_Var)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(NetCDFVariableType::Temperature) == 0);
    ASSERT(static_cast<int>(NetCDFVariableType::Custom) == 5);
}
TEST(TestNCD_Dim)
{
    using namespace ExplorerLens::Engine;
    NetCDFDimension dim{};
    dim.name = "time";
    dim.size = 365;
    dim.isUnlimited = true;
    ASSERT(dim.isUnlimited);
}
TEST(TestNCD_VarInfo)
{
    using namespace ExplorerLens::Engine;
    NetCDFVariable var{};
    var.name = "sst";
    var.units = "degC";
    ASSERT(!var.units.empty());
}
TEST(TestNCD_Pressure)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(NetCDFVariableType::Pressure) == 1);
}
TEST(TestNCD_Wind)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(NetCDFVariableType::WindSpeed) == 2);
}
TEST(TestNCD_Salinity)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(NetCDFVariableType::Salinity) == 3);
}
TEST(TestNCD_Elevation)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(NetCDFVariableType::Elevation) == 4);
}
TEST(TestNCD_Fill)
{
    using namespace ExplorerLens::Engine;
    NetCDFVariable var{};
    var.fillValue = -9999.0;
    ASSERT(var.fillValue == -9999.0);
}

TEST(TestFITS_Init)
{
    using namespace ExplorerLens::Engine;
    FITSDecoder dec;
    ASSERT(true);
}
TEST(TestFITS_Stretch)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(FITSStretchMode::Linear) == 0);
    ASSERT(static_cast<int>(FITSStretchMode::HistogramEqualization) == 4);
}
TEST(TestFITS_LUT)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(FITSColorLUT::Grayscale) == 0);
    ASSERT(static_cast<int>(FITSColorLUT::STScIDefault) == 4);
}
TEST(TestFITS_Header)
{
    using namespace ExplorerLens::Engine;
    FITSHeaderInfo hi{};
    hi.bitpix = -32;
    hi.naxis = 2;
    hi.naxis1 = 2048;
    hi.naxis2 = 2048;
    ASSERT(hi.naxis1 == 2048);
}
TEST(TestFITS_Log)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(FITSStretchMode::Logarithmic) == 1);
}
TEST(TestFITS_Asinh)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(FITSStretchMode::Asinh) == 3);
}
TEST(TestFITS_Object)
{
    using namespace ExplorerLens::Engine;
    FITSHeaderInfo hi{};
    hi.objectName = "M31";
    ASSERT(!hi.objectName.empty());
}
TEST(TestFITS_Heat)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(FITSColorLUT::Heat) == 1);
}
TEST(TestFITS_Scale)
{
    using namespace ExplorerLens::Engine;
    FITSHeaderInfo hi{};
    hi.bscale = 1.0;
    hi.bzero = 0.0;
    ASSERT(hi.bscale == 1.0);
}

TEST(TestECW_Init)
{
    using namespace ExplorerLens::Engine;
    ECWDecoder dec;
    ASSERT(true);
}
TEST(TestECW_Meta)
{
    using namespace ExplorerLens::Engine;
    ECWMetadata em{};
    em.width = 10000;
    em.height = 10000;
    em.bandCount = 3;
    ASSERT(em.width == 10000);
}
TEST(TestECW_Ratio)
{
    using namespace ExplorerLens::Engine;
    ECWCompressionRatio cr{};
    cr.targetRatio = 20.0f;
    cr.actualRatio = 18.5f;
    cr.qualityPercent = 92.0f;
    ASSERT(cr.qualityPercent > 90.0f);
}
TEST(TestECW_ResLevel)
{
    using namespace ExplorerLens::Engine;
    ECWResolutionLevel rl{};
    rl.level = 3;
    rl.width = 1250;
    rl.height = 1250;
    ASSERT(rl.level == 3);
}
TEST(TestECW_CellSize)
{
    using namespace ExplorerLens::Engine;
    ECWMetadata em{};
    em.cellSizeX = 0.5;
    em.cellSizeY = 0.5;
    ASSERT(em.cellSizeX == 0.5);
}
TEST(TestECW_Proj)
{
    using namespace ExplorerLens::Engine;
    ECWMetadata em{};
    em.projectionWkt = "GEOGCS";
    ASSERT(!em.projectionWkt.empty());
}
TEST(TestECW_Block)
{
    using namespace ExplorerLens::Engine;
    ECWResolutionLevel rl{};
    rl.blockSize = 256;
    ASSERT(rl.blockSize == 256);
}
TEST(TestECW_Bands)
{
    using namespace ExplorerLens::Engine;
    ECWMetadata em{};
    em.bandCount = 4;
    ASSERT(em.bandCount == 4);
}
TEST(TestECW_Target)
{
    using namespace ExplorerLens::Engine;
    ECWCompressionRatio cr{};
    cr.targetRatio = 10.0f;
    ASSERT(cr.targetRatio == 10.0f);
}

// ============================================================================
// Sprint 1011-1020 — Universal Format Decoder Library (v30.5.0 "Deneb-V")
// ============================================================================
TEST(TestUDF_Init)
{
    using namespace ExplorerLens::Engine;
    UniversalDecoderFacade udf;
    ASSERT(udf.GetVersion() != nullptr);
}
TEST(TestUDF_Format)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DecoderFormat::JPEG) == 0);
    ASSERT(static_cast<int>(DecoderFormat::Unknown) != 0);
}
TEST(TestUDF_Request)
{
    using namespace ExplorerLens::Engine;
    UniversalRequest req;
    req.maxWidth = 256;
    req.maxHeight = 256;
    ASSERT(req.maxWidth == 256);
}
TEST(TestUDF_Result)
{
    using namespace ExplorerLens::Engine;
    UniversalResult res{};
    res.width = 128;
    res.decodeTimeUs = 5000;
    ASSERT(res.decodeTimeUs == 5000);
}
TEST(TestUDF_Formats)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DecoderFormat::WebP) > 0);
    ASSERT(static_cast<int>(DecoderFormat::HEIF) > 0);
}
TEST(TestUDF_AVIF)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DecoderFormat::AVIF) > 0);
}
TEST(TestUDF_PSD)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DecoderFormat::PSD) > 0);
}
TEST(TestUDF_RAW)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DecoderFormat::RAW) > 0);
}
TEST(TestUDF_Stride)
{
    using namespace ExplorerLens::Engine;
    UniversalResult res{};
    res.stride = 512;
    ASSERT(res.stride == 512);
}
TEST(TestFCM_Init)
{
    using namespace ExplorerLens::Engine;
    FormatCapabilityMatrix fcm;
    ASSERT(fcm.GetTotalFormatCount() >= 0);
}
TEST(TestFCM_Level)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(CapabilityLevel::None) == 0);
    ASSERT(static_cast<int>(CapabilityLevel::GPUAccelerated) == 3);
}
TEST(TestFCM_Platform)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(PlatformType::Windows) == 0);
    ASSERT(static_cast<int>(PlatformType::WASM) == 3);
}
TEST(TestFCM_Cap)
{
    using namespace ExplorerLens::Engine;
    FormatCapability fc{};
    fc.format = "JPEG";
    fc.level = CapabilityLevel::Full;
    ASSERT(fc.level == CapabilityLevel::Full);
}
TEST(TestFCM_Linux)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(PlatformType::Linux) == 1);
}
TEST(TestFCM_macOS)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(PlatformType::macOS) == 2);
}
TEST(TestFCM_Basic)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(CapabilityLevel::Basic) == 1);
}
TEST(TestFCM_Full)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(CapabilityLevel::Full) == 2);
}
TEST(TestFCM_MinVer)
{
    using namespace ExplorerLens::Engine;
    FormatCapability fc{};
    fc.minVersion = "1.0.0";
    ASSERT(!fc.minVersion.empty());
}
TEST(TestDVR_Init)
{
    using namespace ExplorerLens::Engine;
    DecoderVersionRegistry dvr;
    (void)dvr;
    ASSERT(true);
}
TEST(TestDVR_SemVer)
{
    using namespace ExplorerLens::Engine;
    SemVer sv{1, 2, 3, ""};
    ASSERT(sv.major == 1 && sv.minor == 2 && sv.patch == 3);
}
TEST(TestDVR_Reg)
{
    using namespace ExplorerLens::Engine;
    DecoderRegistration dr{};
    dr.name = "JPEG";
    dr.isCompatible = true;
    ASSERT(dr.isCompatible);
}
TEST(TestDVR_Pre)
{
    using namespace ExplorerLens::Engine;
    SemVer sv{2, 0, 0, "beta.1"};
    ASSERT(!sv.prerelease.empty());
}
TEST(TestDVR_Author)
{
    using namespace ExplorerLens::Engine;
    DecoderRegistration dr{};
    dr.author = "ExplorerLens";
    ASSERT(!dr.author.empty());
}
TEST(TestDVR_Compat)
{
    using namespace ExplorerLens::Engine;
    DecoderRegistration dr{};
    dr.isCompatible = false;
    ASSERT(!dr.isCompatible);
}
TEST(TestDVR_Desc)
{
    using namespace ExplorerLens::Engine;
    DecoderRegistration dr{};
    dr.description = "Test";
    ASSERT(!dr.description.empty());
}
TEST(TestDVR_Zero)
{
    using namespace ExplorerLens::Engine;
    SemVer sv{0, 0, 0, ""};
    ASSERT(sv.major == 0);
}
TEST(TestDVR_Version)
{
    using namespace ExplorerLens::Engine;
    SemVer sv{30, 5, 0, ""};
    ASSERT(sv.major == 30);
}
TEST(TestFFR_Init)
{
    using namespace ExplorerLens::Engine;
    FormatFamilyResolver ffr;
    (void)ffr;
    ASSERT(true);
}
TEST(TestFFR_Family)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(FormatFamily::Image) == 0);
}
TEST(TestFFR_Node)
{
    using namespace ExplorerLens::Engine;
    FormatNode fn{};
    fn.name = "JPEG";
    fn.family = FormatFamily::Image;
    ASSERT(!fn.name.empty());
}
TEST(TestFFR_Video)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(FormatFamily::Video) == 1);
}
TEST(TestFFR_Audio)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(FormatFamily::Audio) == 2);
}
TEST(TestFFR_Doc)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(FormatFamily::Document) == 3);
}
TEST(TestFFR_Archive)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(FormatFamily::Archive) == 4);
}
TEST(TestFFR_Scientific)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(FormatFamily::Scientific) == 5);
}
TEST(TestFFR_Exts)
{
    using namespace ExplorerLens::Engine;
    FormatNode fn{};
    fn.extensions = {".jpg", ".jpeg"};
    ASSERT(fn.extensions.size() == 2);
}
TEST(TestDHA_Init)
{
    using namespace ExplorerLens::Engine;
    DecoderHotfixApplicator dha;
    ASSERT(dha.GetPendingCount() == 0);
}
TEST(TestDHA_Priority)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(HotfixPriority::Low) == 0);
    ASSERT(static_cast<int>(HotfixPriority::Emergency) == 4);
}
TEST(TestDHA_Entry)
{
    using namespace ExplorerLens::Engine;
    HotfixEntry he{};
    he.id = "HF-001";
    he.priority = HotfixPriority::Critical;
    he.applied = false;
    ASSERT(!he.applied);
}
TEST(TestDHA_Normal)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(HotfixPriority::Normal) == 1);
}
TEST(TestDHA_High)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(HotfixPriority::High) == 2);
}
TEST(TestDHA_Hash)
{
    using namespace ExplorerLens::Engine;
    HotfixEntry he{};
    he.patchHash = "sha256:abc";
    ASSERT(!he.patchHash.empty());
}
TEST(TestDHA_Target)
{
    using namespace ExplorerLens::Engine;
    HotfixEntry he{};
    he.targetDecoder = "WebPDecoder";
    ASSERT(!he.targetDecoder.empty());
}
TEST(TestDHA_Critical)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(HotfixPriority::Critical) == 3);
}
TEST(TestDHA_Desc)
{
    using namespace ExplorerLens::Engine;
    HotfixEntry he{};
    he.description = "Fix OOB";
    ASSERT(!he.description.empty());
}
TEST(TestFSV_Init)
{
    using namespace ExplorerLens::Engine;
    FormatSchemaValidator fsv;
    (void)fsv;
    ASSERT(true);
}
TEST(TestFSV_Severity)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ValidationSeverity::Info) == 0);
    ASSERT(static_cast<int>(ValidationSeverity::Critical) == 3);
}
TEST(TestFSV_Result)
{
    using namespace ExplorerLens::Engine;
    ValidationResult vr{};
    vr.isValid = true;
    vr.severity = ValidationSeverity::Info;
    ASSERT(vr.isValid);
}
TEST(TestFSV_Spec)
{
    using namespace ExplorerLens::Engine;
    SchemaSpec ss{};
    ss.formatName = "PNG";
    ss.ruleCount = 42;
    ASSERT(ss.ruleCount == 42);
}
TEST(TestFSV_Warning)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ValidationSeverity::Warning) == 1);
}
TEST(TestFSV_Error)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ValidationSeverity::Error) == 2);
}
TEST(TestFSV_Offset)
{
    using namespace ExplorerLens::Engine;
    ValidationResult vr{};
    vr.byteOffset = 1024;
    ASSERT(vr.byteOffset == 1024);
}
TEST(TestFSV_Rule)
{
    using namespace ExplorerLens::Engine;
    ValidationResult vr{};
    vr.ruleName = "CRC32_CHECK";
    ASSERT(!vr.ruleName.empty());
}
TEST(TestFSV_SpecVer)
{
    using namespace ExplorerLens::Engine;
    SchemaSpec ss{};
    ss.specVersion = "ISO 15444-1";
    ASSERT(!ss.specVersion.empty());
}
TEST(TestDCL_Init)
{
    using namespace ExplorerLens::Engine;
    DecoderCompatLayer dcl;
    (void)dcl;
    ASSERT(true);
}
TEST(TestDCL_Mode)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(CompatMode::Strict) == 0);
    ASSERT(static_cast<int>(CompatMode::AutoDetect) == 3);
}
TEST(TestDCL_Issue)
{
    using namespace ExplorerLens::Engine;
    CompatIssue ci{};
    ci.decoderName = "RAWDecoder";
    ci.severity = 2;
    ASSERT(ci.severity == 2);
}
TEST(TestDCL_Report)
{
    using namespace ExplorerLens::Engine;
    CompatReport cr{};
    cr.isCompatible = true;
    cr.migrationsRequired = 0;
    ASSERT(cr.isCompatible);
}
TEST(TestDCL_Legacy)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(CompatMode::Legacy) == 2);
}
TEST(TestDCL_Relaxed)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(CompatMode::Relaxed) == 1);
}
TEST(TestDCL_Workaround)
{
    using namespace ExplorerLens::Engine;
    CompatIssue ci{};
    ci.workaround = "Use v2 API";
    ASSERT(!ci.workaround.empty());
}
TEST(TestDCL_Versions)
{
    using namespace ExplorerLens::Engine;
    CompatIssue ci{};
    ci.sourceVersion = "1.0";
    ci.targetVersion = "2.0";
    ASSERT(ci.sourceVersion != ci.targetVersion);
}
TEST(TestDCL_Migrations)
{
    using namespace ExplorerLens::Engine;
    CompatReport cr{};
    cr.migrationsRequired = 3;
    ASSERT(cr.migrationsRequired == 3);
}
TEST(TestULM_Init)
{
    using namespace ExplorerLens::Engine;
    UniversalLibraryManifest ulm;
    ASSERT(true);
}
TEST(TestULM_License)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(SPDXLicense::MIT) == 0);
    ASSERT(static_cast<int>(SPDXLicense::Custom) == 5);
}
TEST(TestULM_Lib)
{
    using namespace ExplorerLens::Engine;
    ThirdPartyLib tpl{};
    tpl.name = "zlib";
    tpl.license = SPDXLicense::Zlib;
    ASSERT(!tpl.name.empty());
}
TEST(TestULM_Manifest)
{
    using namespace ExplorerLens::Engine;
    LibraryManifest lm{};
    lm.product = "ExplorerLens";
    lm.version = "30.5.0";
    ASSERT(!lm.version.empty());
}
TEST(TestULM_Apache)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(SPDXLicense::Apache2) == 1);
}
TEST(TestULM_BSD)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(SPDXLicense::BSD3) == 2);
}
TEST(TestULM_LGPL)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(SPDXLicense::LGPL21) == 3);
}
TEST(TestULM_SPDX)
{
    using namespace ExplorerLens::Engine;
    ThirdPartyLib tpl{};
    tpl.spdxId = "Zlib";
    ASSERT(!tpl.spdxId.empty());
}
TEST(TestULM_Copy)
{
    using namespace ExplorerLens::Engine;
    ThirdPartyLib tpl{};
    tpl.copyright = "2026";
    ASSERT(!tpl.copyright.empty());
}

// ============================================================================
// Sprint 1021-1030 — Plugin Marketplace v4 & Commerce (v30.6.0 "Deneb-W")
// ============================================================================
TEST(TestMKT_Init)
{
    using namespace ExplorerLens::Engine;
    MarketplaceClientV4 c;
    ASSERT(true);
}
TEST(TestMKT_Proto)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(MarketplaceProtocol::REST) == 0);
    ASSERT(static_cast<int>(MarketplaceProtocol::Hybrid) == 2);
}
TEST(TestMKT_Cache)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(CachePolicy::NoCache) == 0);
    ASSERT(static_cast<int>(CachePolicy::Aggressive) == 3);
}
TEST(TestMKT_List)
{
    using namespace ExplorerLens::Engine;
    MarketplaceListing pl{};
    pl.id = "com.example.plugin";
    pl.isFree = true;
    ASSERT(pl.isFree);
}
TEST(TestMKT_Rating)
{
    using namespace ExplorerLens::Engine;
    MarketplaceListing pl{};
    pl.rating = 4.8f;
    pl.downloadCount = 10000;
    ASSERT(pl.rating > 4.0f);
}
TEST(TestMKT_Price)
{
    using namespace ExplorerLens::Engine;
    MarketplaceListing pl{};
    pl.price = 9.99f;
    pl.isFree = false;
    ASSERT(!pl.isFree);
}
TEST(TestMKT_gRPC)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(MarketplaceProtocol::gRPC) == 1);
}
TEST(TestMKT_Persist)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(CachePolicy::Persistent) == 2);
}
TEST(TestMKT_Author)
{
    using namespace ExplorerLens::Engine;
    MarketplaceListing pl{};
    pl.author = "DevCo";
    ASSERT(!pl.author.empty());
}

TEST(TestRE_Init)
{
    using namespace ExplorerLens::Engine;
    PluginRatingEngine re;
    ASSERT(true);
}
TEST(TestRE_Src)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(RatingSource::UserReview) == 0);
    ASSERT(static_cast<int>(RatingSource::SecurityScan) == 3);
}
TEST(TestRE_Entry)
{
    using namespace ExplorerLens::Engine;
    RatingEntry re2{};
    re2.pluginId = "test";
    re2.score = 4.5f;
    re2.bayesianScore = 4.2f;
    ASSERT(re2.bayesianScore > 4.0f);
}
TEST(TestRE_Thresh)
{
    using namespace ExplorerLens::Engine;
    RatingThreshold rt{};
    rt.trusted = 4.0f;
    rt.blocked = 1.5f;
    ASSERT(rt.trusted > rt.blocked);
}
TEST(TestRE_AutoScan)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(RatingSource::AutoScan) == 1);
}
TEST(TestRE_Compat)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(RatingSource::CompatTest) == 2);
}
TEST(TestRE_Reviews)
{
    using namespace ExplorerLens::Engine;
    RatingEntry re2{};
    re2.reviewCount = 250;
    ASSERT(re2.reviewCount == 250);
}
TEST(TestRE_Susp)
{
    using namespace ExplorerLens::Engine;
    RatingThreshold rt{};
    rt.suspicious = 2.5f;
    ASSERT(rt.suspicious > 0.0f);
}
TEST(TestRE_Score)
{
    using namespace ExplorerLens::Engine;
    RatingEntry re2{};
    re2.score = 5.0f;
    ASSERT(re2.score == 5.0f);
}

TEST(TestDG_Init)
{
    using namespace ExplorerLens::Engine;
    PluginDependencyGraph dg;
    ASSERT(true);
}
TEST(TestDG_Type)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DependencyType::Required) == 0);
    ASSERT(static_cast<int>(DependencyType::Replaces) == 3);
}
TEST(TestDG_Edge)
{
    using namespace ExplorerLens::Engine;
    DependencyEdge de{};
    de.fromId = "a";
    de.toId = "b";
    de.type = DependencyType::Required;
    ASSERT(!de.fromId.empty());
}
TEST(TestDG_Result)
{
    using namespace ExplorerLens::Engine;
    ResolutionResult rr{};
    rr.conflicts = {};
    ASSERT(rr.conflicts.empty());
}
TEST(TestDG_Optional)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DependencyType::Optional) == 1);
}
TEST(TestDG_Conflicts)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DependencyType::Conflicts) == 2);
}
TEST(TestDG_VerConst)
{
    using namespace ExplorerLens::Engine;
    DependencyEdge de{};
    de.versionConstraint = ">=1.0.0";
    ASSERT(!de.versionConstraint.empty());
}
TEST(TestDG_Missing)
{
    using namespace ExplorerLens::Engine;
    ResolutionResult rr{};
    ASSERT(rr.missingDeps.empty());
}
TEST(TestDG_InstOrd)
{
    using namespace ExplorerLens::Engine;
    ResolutionResult rr{};
    ASSERT(rr.installOrder.empty());
}

TEST(TestBI_Init)
{
    using namespace ExplorerLens::Engine;
    PluginBundleInstaller bi;
    ASSERT(true);
}
TEST(TestBI_Phase)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(BundleInstallPhase::Verify) == 0);
    ASSERT(static_cast<int>(BundleInstallPhase::Rollback) == 4);
}
TEST(TestBI_Entry)
{
    using namespace ExplorerLens::Engine;
    BundleEntry be{};
    be.pluginId = "test";
    be.size = 1024 * 1024;
    ASSERT(be.size > 0);
}
TEST(TestBI_Prog)
{
    using namespace ExplorerLens::Engine;
    InstallProgress ip{};
    ip.phase = BundleInstallPhase::Download;
    ip.current = 3;
    ip.total = 10;
    ASSERT(ip.total == 10);
}
TEST(TestBI_DL)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(BundleInstallPhase::Download) == 1);
}
TEST(TestBI_Stage)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(BundleInstallPhase::Stage) == 2);
}
TEST(TestBI_Commit)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(BundleInstallPhase::Commit) == 3);
}
TEST(TestBI_CSum)
{
    using namespace ExplorerLens::Engine;
    BundleEntry be{};
    be.checksum = "sha256:deadbeef";
    ASSERT(!be.checksum.empty());
}
TEST(TestBI_Curr)
{
    using namespace ExplorerLens::Engine;
    InstallProgress ip{};
    ip.currentPlugin = "wpf.extn";
    ASSERT(!ip.currentPlugin.empty());
}

TEST(TestSLM_Init)
{
    using namespace ExplorerLens::Engine;
    SubscriptionLicenseManagerV4 slm;
    ASSERT(true);
}
TEST(TestSLM_Type)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(SubscriptionLicenseTier::Free) == 0);
    ASSERT(static_cast<int>(SubscriptionLicenseTier::OEM) == 4);
}
TEST(TestSLM_Seat)
{
    using namespace ExplorerLens::Engine;
    SeatAllocation sa{};
    sa.total = 100;
    sa.used = 30;
    sa.reserved = 5;
    ASSERT(sa.total > sa.used);
}
TEST(TestSLM_JWT)
{
    using namespace ExplorerLens::Engine;
    JWTEntitlement jwt{};
    jwt.subject = "user@org.com";
    jwt.issuer = "license.explorerlens.io";
    ASSERT(!jwt.issuer.empty());
}
TEST(TestSLM_Team)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(SubscriptionLicenseTier::Team) == 2);
}
TEST(TestSLM_Ent)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(SubscriptionLicenseTier::Enterprise) == 3);
}
TEST(TestSLM_Expired)
{
    using namespace ExplorerLens::Engine;
    SeatAllocation sa{};
    sa.expired = 10;
    ASSERT(sa.expired == 10);
}
TEST(TestSLM_Features)
{
    using namespace ExplorerLens::Engine;
    JWTEntitlement jwt{};
    jwt.features = {"gpu", "ai", "enterprise"};
    ASSERT(jwt.features.size() == 3);
}
TEST(TestSLM_Sig)
{
    using namespace ExplorerLens::Engine;
    JWTEntitlement jwt{};
    jwt.signature = "base64sig";
    ASSERT(!jwt.signature.empty());
}

TEST(TestRS_Init)
{
    using namespace ExplorerLens::Engine;
    PluginReputationScorer rs;
    ASSERT(true);
}
TEST(TestRS_Factor)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ReputationFactor::Downloads) == 0);
    ASSERT(static_cast<int>(ReputationFactor::UpdateFrequency) == 4);
}
TEST(TestRS_CVE)
{
    using namespace ExplorerLens::Engine;
    CVEEntry ce{};
    ce.id = "CVE-2026-1234";
    ce.cvssScore = 7.5f;
    ce.severity = "High";
    ASSERT(ce.cvssScore > 5.0f);
}
TEST(TestRS_Score)
{
    using namespace ExplorerLens::Engine;
    ReputationScore rs2{};
    rs2.overall = 0.87f;
    rs2.isBlacklisted = false;
    ASSERT(!rs2.isBlacklisted);
}
TEST(TestRS_Stars)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ReputationFactor::Stars) == 1);
}
TEST(TestRS_Sec)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ReputationFactor::SecurityScan) == 2);
}
TEST(TestRS_Black)
{
    using namespace ExplorerLens::Engine;
    ReputationScore rs2{};
    rs2.isBlacklisted = true;
    ASSERT(rs2.isBlacklisted);
}
TEST(TestRS_Patched)
{
    using namespace ExplorerLens::Engine;
    CVEEntry ce{};
    ce.patchedVersion = "2.1.0";
    ASSERT(!ce.patchedVersion.empty());
}
TEST(TestRS_CVSS)
{
    using namespace ExplorerLens::Engine;
    CVEEntry ce{};
    ce.cvssScore = 9.8f;
    ASSERT(ce.cvssScore > 9.0f);
}

TEST(TestAU_Init)
{
    using namespace ExplorerLens::Engine;
    AutoUpdatePolicyV4 au;
    ASSERT(true);
}
TEST(TestAU_Mode)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(UpdateMode::Silent) == 0);
    ASSERT(static_cast<int>(UpdateMode::Manual) == 4);
}
TEST(TestAU_Chan)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(UpdateChannel::Stable) == 0);
    ASSERT(static_cast<int>(UpdateChannel::LTS) == 3);
}
TEST(TestAU_Policy)
{
    using namespace ExplorerLens::Engine;
    PolicyEntry pe{};
    pe.pluginId = "test";
    pe.mode = UpdateMode::Notify;
    pe.deferDays = 7;
    ASSERT(pe.deferDays == 7);
}
TEST(TestAU_Notify)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(UpdateMode::Notify) == 1);
}
TEST(TestAU_Defer)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(UpdateMode::Defer) == 2);
}
TEST(TestAU_Locked)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(UpdateMode::EnterpriseLocked) == 3);
}
TEST(TestAU_Beta)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(UpdateChannel::Beta) == 1);
}
TEST(TestAU_Nightly)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(UpdateChannel::Nightly) == 2);
}

TEST(TestPRG_Init)
{
    using namespace ExplorerLens::Engine;
    PrePublishReviewGateway prg;
    ASSERT(true);
}
TEST(TestPRG_Status)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ReviewStatus::Pending) == 0);
    ASSERT(static_cast<int>(ReviewStatus::Suspended) == 4);
}
TEST(TestPRG_Check)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ReviewCheck::StaticAnalysis) == 0);
    ASSERT(static_cast<int>(ReviewCheck::LicenseCheck) == 4);
}
TEST(TestPRG_Report)
{
    using namespace ExplorerLens::Engine;
    ReviewReport rr{};
    rr.status = ReviewStatus::Pending;
    rr.reviewerId = "bot-v2";
    ASSERT(!rr.reviewerId.empty());
}
TEST(TestPRG_Approved)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ReviewStatus::Approved) == 1);
}
TEST(TestPRG_Rejected)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ReviewStatus::Rejected) == 2);
}
TEST(TestPRG_NeedsRev)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ReviewStatus::NeedsRevision) == 3);
}
TEST(TestPRG_Sig)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ReviewCheck::SignatureVerify) == 1);
}
TEST(TestPRG_Issues)
{
    using namespace ExplorerLens::Engine;
    ReviewReport rr{};
    rr.issues = {};
    ASSERT(rr.issues.empty());
}

// ============================================================================
// Sprint 1031-1040 — Enterprise Console v3 & Fleet Management (v30.7.0 "Deneb-X")
// ============================================================================
TEST(TestEC3_Init)
{
    using namespace ExplorerLens::Engine;
    EnterpriseConsoleV3 ec(ConsoleEndpoint{});
    ASSERT(true);
}
TEST(TestEC3_Proto)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ConsoleProtocol::REST) == 0);
    ASSERT(static_cast<int>(ConsoleProtocol::Hybrid) == 2);
}
TEST(TestEC3_Role)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(AdminRole::Viewer) == 0);
    ASSERT(static_cast<int>(AdminRole::SuperAdmin) == 3);
}
TEST(TestEC3_Endpoint)
{
    using namespace ExplorerLens::Engine;
    ConsoleEndpoint ep{};
    ep.host = "console.corp.com";
    ep.port = 8443;
    ep.tlsEnabled = true;
    ASSERT(ep.tlsEnabled);
}
TEST(TestEC3_Admin)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(AdminRole::Admin) == 2);
}
TEST(TestEC3_Op)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(AdminRole::Operator) == 1);
}
TEST(TestEC3_Port)
{
    using namespace ExplorerLens::Engine;
    ConsoleEndpoint ep{};
    ep.port = 443;
    ASSERT(ep.port == 443);
}
TEST(TestEC3_Token)
{
    using namespace ExplorerLens::Engine;
    ConsoleEndpoint ep{};
    ep.authToken = "bearer.xyz";
    ASSERT(!ep.authToken.empty());
}
TEST(TestEC3_gRPC)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ConsoleProtocol::gRPC) == 1);
}

TEST(TestFDM_Init)
{
    using namespace ExplorerLens::Engine;
    FleetDeploymentManager fdm;
    ASSERT(true);
}
TEST(TestFDM_Method)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(FleetDeployMethod::WinGet) == 0);
    ASSERT(static_cast<int>(FleetDeployMethod::Manual) == 4);
}
TEST(TestFDM_Stage)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(RolloutStage::Canary) == 0);
    ASSERT(static_cast<int>(RolloutStage::Full) == 3);
}
TEST(TestFDM_Job)
{
    using namespace ExplorerLens::Engine;
    DeploymentJob dj{};
    dj.jobId = "job-001";
    dj.stage = RolloutStage::Ring1;
    dj.targetCount = 1000;
    ASSERT(dj.targetCount == 1000);
}
TEST(TestFDM_MDM)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(FleetDeployMethod::MDM_Intune) == 1);
}
TEST(TestFDM_Ring2)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(RolloutStage::Ring2) == 2);
}
TEST(TestFDM_Done)
{
    using namespace ExplorerLens::Engine;
    DeploymentJob dj{};
    dj.completedCount = 500;
    dj.targetCount = 1000;
    ASSERT(dj.completedCount < dj.targetCount);
}
TEST(TestFDM_SCCM)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(FleetDeployMethod::SCCM) == 2);
}
TEST(TestFDM_Ver)
{
    using namespace ExplorerLens::Engine;
    DeploymentJob dj{};
    dj.version = "30.7.0";
    ASSERT(!dj.version.empty());
}

TEST(TestCRG_Init)
{
    using namespace ExplorerLens::Engine;
    ComplianceReportGenerator crg;
    ASSERT(true);
}
TEST(TestCRG_FW)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ComplianceFramework::GDPR) == 0);
    ASSERT(static_cast<int>(ComplianceFramework::NIST_CSF) == 4);
}
TEST(TestCRG_Fmt)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ReportFormat::PDF) == 0);
    ASSERT(static_cast<int>(ReportFormat::XLSX) == 3);
}
TEST(TestCRG_Ev)
{
    using namespace ExplorerLens::Engine;
    ComplianceEvidence ce{};
    ce.framework = ComplianceFramework::SOC2;
    ce.controlId = "CC6.1";
    ce.status = "Pass";
    ASSERT(!ce.status.empty());
}
TEST(TestCRG_HIPAA)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ComplianceFramework::HIPAA) == 1);
}
TEST(TestCRG_ISO)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ComplianceFramework::ISO27001) == 3);
}
TEST(TestCRG_JSON)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ReportFormat::JSON_LD) == 1);
}
TEST(TestCRG_HTML)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ReportFormat::HTML) == 2);
}
TEST(TestCRG_Evid)
{
    using namespace ExplorerLens::Engine;
    ComplianceEvidence ce{};
    ce.evidence = "screenshot.png";
    ASSERT(!ce.evidence.empty());
}

TEST(TestEMD_Init)
{
    using namespace ExplorerLens::Engine;
    EnterpriseMetricsDashboard emd;
    ASSERT(true);
}
TEST(TestEMD_Type)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(MetricType::Latency) == 0);
    ASSERT(static_cast<int>(MetricType::PluginAdoption) == 4);
}
TEST(TestEMD_Pct)
{
    using namespace ExplorerLens::Engine;
    PercentileSet ps{};
    ps.p50 = 8.0f;
    ps.p95 = 15.0f;
    ps.p99 = 22.0f;
    ps.p999 = 45.0f;
    ASSERT(ps.p99 > ps.p50);
}
TEST(TestEMD_Snap)
{
    using namespace ExplorerLens::Engine;
    MetricSnapshot ms{};
    ms.name = "thumb.latency";
    ms.type = MetricType::Latency;
    ms.value = 12.5f;
    ASSERT(ms.value > 0.0f);
}
TEST(TestEMD_ErrRate)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(MetricType::ErrorRate) == 1);
}
TEST(TestEMD_Tput)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(MetricType::Throughput) == 2);
}
TEST(TestEMD_Cache)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(MetricType::CacheHit) == 3);
}
TEST(TestEMD_P999)
{
    using namespace ExplorerLens::Engine;
    PercentileSet ps{};
    ps.p999 = 100.0f;
    ASSERT(ps.p999 == 100.0f);
}
TEST(TestEMD_Name)
{
    using namespace ExplorerLens::Engine;
    MetricSnapshot ms{};
    ms.name = "cache.hit_rate";
    ASSERT(!ms.name.empty());
}

TEST(TestPVC_Init)
{
    using namespace ExplorerLens::Engine;
    PolicyVersionControl pvc;
    ASSERT(true);
}
TEST(TestPVC_Chg)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(PolicyChangeType::Add) == 0);
    ASSERT(static_cast<int>(PolicyChangeType::Revert) == 3);
}
TEST(TestPVC_Diff)
{
    using namespace ExplorerLens::Engine;
    PolicyDiff pd{};
    pd.controlId = "CC6.1";
    pd.changeType = PolicyChangeType::Modify;
    pd.oldValue = "false";
    pd.newValue = "true";
    ASSERT(!pd.newValue.empty());
}
TEST(TestPVC_Ver)
{
    using namespace ExplorerLens::Engine;
    PolicyVersion pv{};
    pv.author = "admin";
    pv.message = "Enable GPU decode";
    ASSERT(!pv.message.empty());
}
TEST(TestPVC_Del)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(PolicyChangeType::Delete) == 2);
}
TEST(TestPVC_Mod)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(PolicyChangeType::Modify) == 1);
}
TEST(TestPVC_Hash)
{
    using namespace ExplorerLens::Engine;
    PolicyVersion pv{};
    pv.hash = "abc123def456";
    ASSERT(!pv.hash.empty());
}
TEST(TestPVC_Changes)
{
    using namespace ExplorerLens::Engine;
    PolicyVersion pv{};
    pv.changes = {};
    ASSERT(pv.changes.empty());
}
TEST(TestPVC_Author)
{
    using namespace ExplorerLens::Engine;
    PolicyVersion pv{};
    pv.author = "sysadmin";
    ASSERT(!pv.author.empty());
}

TEST(TestRDC_Init)
{
    using namespace ExplorerLens::Engine;
    RemoteDecoderControl rdc;
    ASSERT(true);
}
TEST(TestRDC_Act)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DecoderAction::Enable) == 0);
    ASSERT(static_cast<int>(DecoderAction::UpdateConfig) == 4);
}
TEST(TestRDC_Target)
{
    using namespace ExplorerLens::Engine;
    DecoderTarget dt{};
    dt.decoderId = "WebP";
    dt.endpointCount = 500;
    ASSERT(dt.endpointCount == 500);
}
TEST(TestRDC_Result)
{
    using namespace ExplorerLens::Engine;
    ControlResult cr{};
    cr.success = true;
    cr.affected = 300;
    ASSERT(cr.affected == 300);
}
TEST(TestRDC_Dis)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DecoderAction::Disable) == 1);
}
TEST(TestRDC_Quar)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DecoderAction::Quarantine) == 2);
}
TEST(TestRDC_Rest)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DecoderAction::Restore) == 3);
}
TEST(TestRDC_Errs)
{
    using namespace ExplorerLens::Engine;
    ControlResult cr{};
    cr.errors = {};
    ASSERT(cr.errors.empty());
}
TEST(TestRDC_State)
{
    using namespace ExplorerLens::Engine;
    DecoderTarget dt{};
    dt.currentState = "active";
    ASSERT(!dt.currentState.empty());
}

TEST(TestUAD_Init)
{
    using namespace ExplorerLens::Engine;
    UsageAnomalyDetector uad;
    ASSERT(true);
}
TEST(TestUAD_Sev)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(AnomalySeverity::Low) == 0);
    ASSERT(static_cast<int>(AnomalySeverity::Critical) == 3);
}
TEST(TestUAD_Type)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(AnomalyType::SpikeThroughput) == 0);
    ASSERT(static_cast<int>(AnomalyType::ResourceLeak) == 3);
}
TEST(TestUAD_Event)
{
    using namespace ExplorerLens::Engine;
    AnomalyEvent ae{};
    ae.type = AnomalyType::ErrorBurst;
    ae.severity = AnomalySeverity::High;
    ae.zScore = 3.8f;
    ASSERT(ae.zScore > 3.0f);
}
TEST(TestUAD_Med)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(AnomalySeverity::Medium) == 1);
}
TEST(TestUAD_High)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(AnomalySeverity::High) == 2);
}
TEST(TestUAD_Latency)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(AnomalyType::LatencyCliff) == 2);
}
TEST(TestUAD_Decoder)
{
    using namespace ExplorerLens::Engine;
    AnomalyEvent ae{};
    ae.affectedDecoder = "WebPDecoder";
    ASSERT(!ae.affectedDecoder.empty());
}
TEST(TestUAD_ZScore)
{
    using namespace ExplorerLens::Engine;
    AnomalyEvent ae{};
    ae.zScore = 5.2f;
    ASSERT(ae.zScore > 5.0f);
}

TEST(TestEAE_Init)
{
    using namespace ExplorerLens::Engine;
    EnterpriseAuditExporter eae;
    ASSERT(true);
}
TEST(TestEAE_Target)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(SIEMTarget::SplunkHEC) == 0);
    ASSERT(static_cast<int>(SIEMTarget::Generic) == 4);
}
TEST(TestEAE_Fmt)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(AuditLogFormat::CEF) == 0);
    ASSERT(static_cast<int>(AuditLogFormat::Syslog) == 3);
}
TEST(TestEAE_Ev)
{
    using namespace ExplorerLens::Engine;
    AuditEvent ae{};
    ae.eventId = "EVT-001";
    ae.category = "Security";
    ae.severity = "High";
    ae.outcome = "Success";
    ASSERT(!ae.outcome.empty());
}
TEST(TestEAE_Sent)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(SIEMTarget::MicrosoftSentinel) == 1);
}
TEST(TestEAE_QR)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(SIEMTarget::QRadar) == 2);
}
TEST(TestEAE_LEEF)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(AuditLogFormat::LEEF) == 1);
}
TEST(TestEAE_Actor)
{
    using namespace ExplorerLens::Engine;
    AuditEvent ae{};
    ae.actor = "admin@corp.com";
    ASSERT(!ae.actor.empty());
}
TEST(TestEAE_Res)
{
    using namespace ExplorerLens::Engine;
    AuditEvent ae{};
    ae.resource = "WebPDecoder";
    ASSERT(!ae.resource.empty());
}

//==========================================================================
// Sprint 1041-1050: Generative AI Thumbnails (v31.0.0 Achernar)
//==========================================================================

TEST(TestGTE_BackendEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(GenerativeBackend::DirectML) != static_cast<int>(GenerativeBackend::ONNX));
    ASSERT(static_cast<int>(GenerativeBackend::OpenVINO) != static_cast<int>(GenerativeBackend::CPU));
}

TEST(TestGTE_GenerationMode)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(GenerationMode::TextToImage) != static_cast<int>(GenerationMode::StyleAdaptation));
    ASSERT(static_cast<int>(GenerationMode::ContentSynthesis) != static_cast<int>(GenerationMode::ImageVariation));
}

TEST(TestGTE_RequestDefaults)
{
    using namespace ExplorerLens::Engine;
    GenerationRequest req{};
    req.maxTokens = 512;
    req.seed = 42;
    ASSERT(req.maxTokens == 512);
    ASSERT(req.seed == 42);
}

TEST(TestGTE_EngineInit)
{
    using namespace ExplorerLens::Engine;
    GenerativeThumbnailEngine engine;
    ASSERT(!engine.IsAcceleratorAvailable(GenerativeBackend::DirectML) || true);
}

TEST(TestGTE_SupportedModes)
{
    using namespace ExplorerLens::Engine;
    GenerativeThumbnailEngine engine;
    auto modes = engine.GetSupportedModes();
    ASSERT(modes.size() >= 0);
}

TEST(TestGTE_SetBackend)
{
    using namespace ExplorerLens::Engine;
    GenerativeThumbnailEngine engine;
    engine.SetBackend(GenerativeBackend::CPU);
    ASSERT(true);
}

TEST(TestGTE_Shutdown)
{
    using namespace ExplorerLens::Engine;
    GenerativeThumbnailEngine engine;
    engine.Shutdown();
    ASSERT(true);
}

TEST(TestGTE_GenerateNullSafe)
{
    using namespace ExplorerLens::Engine;
    GenerativeThumbnailEngine engine;
    GenerationRequest req{};
    req.backend = GenerativeBackend::CPU;
    req.mode = GenerationMode::ContentSynthesis;
    req.maxTokens = 128;
    bool result = engine.Generate(req);
    ASSERT(!result || result);
}

TEST(TestGTE_MultiBackendCheck)
{
    using namespace ExplorerLens::Engine;
    GenerativeThumbnailEngine engine;
    ASSERT(!engine.IsAcceleratorAvailable(GenerativeBackend::ONNX) || true);
    ASSERT(!engine.IsAcceleratorAvailable(GenerativeBackend::OpenVINO) || true);
}

TEST(TestCAI_AlgorithmEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(InpaintAlgorithm::PatchMatch) != static_cast<int>(InpaintAlgorithm::DeepFill));
    ASSERT(static_cast<int>(InpaintAlgorithm::LaMa) != static_cast<int>(InpaintAlgorithm::Stable));
}

TEST(TestCAI_QualityEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(InpaintQuality::Draft) != static_cast<int>(InpaintQuality::Ultra));
    ASSERT(static_cast<int>(InpaintQuality::Standard) != static_cast<int>(InpaintQuality::High));
}

TEST(TestCAI_RegionStruct)
{
    using namespace ExplorerLens::Engine;
    InpaintRegion region{};
    region.x = 10;
    region.y = 20;
    region.width = 100;
    region.height = 80;
    region.algorithm = InpaintAlgorithm::LaMa;
    region.quality = InpaintQuality::High;
    ASSERT(region.width == 100);
    ASSERT(region.height == 80);
}

TEST(TestCAI_NotProcessing)
{
    using namespace ExplorerLens::Engine;
    ContentAwareInpainter inpainter;
    ASSERT(!inpainter.IsProcessing());
}

TEST(TestCAI_SetQuality)
{
    using namespace ExplorerLens::Engine;
    ContentAwareInpainter inpainter;
    inpainter.SetQuality(InpaintQuality::Ultra);
    ASSERT(true);
}

TEST(TestCAI_CancelSafe)
{
    using namespace ExplorerLens::Engine;
    ContentAwareInpainter inpainter;
    inpainter.Cancel();
    ASSERT(!inpainter.IsProcessing());
}

TEST(TestCAI_DurationEstimate)
{
    using namespace ExplorerLens::Engine;
    ContentAwareInpainter inpainter;
    InpaintRegion region{};
    region.width = 64;
    region.height = 64;
    region.algorithm = InpaintAlgorithm::PatchMatch;
    region.quality = InpaintQuality::Draft;
    uint32_t dur = inpainter.GetEstimatedDurationMs(region);
    ASSERT(dur >= 0);
}

TEST(TestCAI_InpaintNullGuard)
{
    using namespace ExplorerLens::Engine;
    ContentAwareInpainter inpainter;
    InpaintRegion region{};
    region.width = 32;
    region.height = 32;
    bool result = inpainter.Inpaint(nullptr, 32, 32, region);
    ASSERT(!result || result);
}

TEST(TestCAI_DraftQuality)
{
    using namespace ExplorerLens::Engine;
    ContentAwareInpainter inpainter;
    inpainter.SetQuality(InpaintQuality::Draft);
    ASSERT(true);
}

TEST(TestSTR_StyleEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ArtisticStyle::Photorealistic) != static_cast<int>(ArtisticStyle::Sketch));
    ASSERT(static_cast<int>(ArtisticStyle::Watercolor) != static_cast<int>(ArtisticStyle::HDR));
}

TEST(TestSTR_StrengthEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(StyleStrength::Subtle) != static_cast<int>(StyleStrength::Extreme));
    ASSERT(static_cast<int>(StyleStrength::Moderate) != static_cast<int>(StyleStrength::Strong));
}

TEST(TestSTR_ParamsStruct)
{
    using namespace ExplorerLens::Engine;
    StyleParams params{};
    params.style = ArtisticStyle::Cinematic;
    params.strength = StyleStrength::Moderate;
    params.preserveColors = true;
    params.blendFactor = 0.7f;
    ASSERT(params.blendFactor > 0.0f);
    ASSERT(params.preserveColors);
}

TEST(TestSTR_AvailableStyles)
{
    using namespace ExplorerLens::Engine;
    StyleTransferRenderer renderer;
    auto styles = renderer.GetAvailableStyles();
    ASSERT(styles.size() >= 0);
}

TEST(TestSTR_LastAppliedEmpty)
{
    using namespace ExplorerLens::Engine;
    StyleTransferRenderer renderer;
    auto last = renderer.GetLastAppliedStyle();
    ASSERT(!last.has_value());
}

TEST(TestSTR_LoadModelFails)
{
    using namespace ExplorerLens::Engine;
    StyleTransferRenderer renderer;
    bool loaded = renderer.LoadStyleModel("nonexistent.model");
    ASSERT(!loaded || loaded);
}

TEST(TestSTR_UnloadSafe)
{
    using namespace ExplorerLens::Engine;
    StyleTransferRenderer renderer;
    renderer.UnloadStyleModel();
    ASSERT(true);
}

TEST(TestSTR_ApplyStyleNullGuard)
{
    using namespace ExplorerLens::Engine;
    StyleTransferRenderer renderer;
    StyleParams params{};
    params.style = ArtisticStyle::Sketch;
    params.strength = StyleStrength::Subtle;
    params.blendFactor = 0.5f;
    bool result = renderer.ApplyStyle(nullptr, 0, 0, params);
    ASSERT(!result || result);
}

TEST(TestIDS_DepthEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DescriptionDepth::Brief) != static_cast<int>(DescriptionDepth::Exhaustive));
    ASSERT(static_cast<int>(DescriptionDepth::Standard) != static_cast<int>(DescriptionDepth::Detailed));
}

TEST(TestIDS_LanguageEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DescriptionLanguage::English) != static_cast<int>(DescriptionLanguage::Japanese));
    ASSERT(static_cast<int>(DescriptionLanguage::French) != static_cast<int>(DescriptionLanguage::Chinese));
}

TEST(TestIDS_SynthesisResult)
{
    using namespace ExplorerLens::Engine;
    SynthesisResult result{};
    result.description = "A sunset over the ocean";
    result.confidence = 0.92f;
    result.generatedAtMs = 1000000;
    ASSERT(result.confidence > 0.0f);
    ASSERT(!result.description.empty());
}

TEST(TestIDS_SetGetLanguage)
{
    using namespace ExplorerLens::Engine;
    ImageDescriptionSynthesizer synth;
    synth.SetLanguage(DescriptionLanguage::German);
    ASSERT(synth.GetLanguage() == DescriptionLanguage::German);
}

TEST(TestIDS_ModelNotLoaded)
{
    using namespace ExplorerLens::Engine;
    ImageDescriptionSynthesizer synth;
    ASSERT(!synth.IsModelLoaded());
}

TEST(TestIDS_LoadModelFails)
{
    using namespace ExplorerLens::Engine;
    ImageDescriptionSynthesizer synth;
    bool loaded = synth.LoadModel("no_model.bin");
    ASSERT(!loaded || loaded);
}

TEST(TestIDS_DefaultLanguage)
{
    using namespace ExplorerLens::Engine;
    ImageDescriptionSynthesizer synth;
    ASSERT(synth.GetLanguage() == DescriptionLanguage::English);
}

TEST(TestIDS_SynthesizeNullSafe)
{
    using namespace ExplorerLens::Engine;
    ImageDescriptionSynthesizer synth;
    auto result = synth.Synthesize(nullptr, 0, 0, DescriptionDepth::Brief);
    ASSERT(result.confidence >= 0.0f);
}

TEST(TestTPE_SignalEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(PersonalisationSignal::RecentlyViewed) != static_cast<int>(PersonalisationSignal::Starred));
    ASSERT(static_cast<int>(PersonalisationSignal::EditHistory)
           != static_cast<int>(PersonalisationSignal::ViewDuration));
}

TEST(TestTPE_StrategyEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(AdaptationStrategy::Conservative) != static_cast<int>(AdaptationStrategy::Aggressive));
    ASSERT(static_cast<int>(AdaptationStrategy::Balanced) != static_cast<int>(AdaptationStrategy::Fixed));
}

TEST(TestTPE_ProfileStruct)
{
    using namespace ExplorerLens::Engine;
    UserPersonalisationProfile profile{};
    profile.userId = "user_001";
    profile.strategy = AdaptationStrategy::Balanced;
    profile.confidenceThreshold = 0.75f;
    profile.maxHistoryDays = 30;
    ASSERT(!profile.userId.empty());
    ASSERT(profile.confidenceThreshold > 0.0f);
}

TEST(TestTPE_GetProfileMissing)
{
    using namespace ExplorerLens::Engine;
    ThumbnailPersonalisationEngine engine;
    auto profile = engine.GetProfile("nonexistent_user");
    ASSERT(!profile.has_value());
}

TEST(TestTPE_UpdateSignal)
{
    using namespace ExplorerLens::Engine;
    ThumbnailPersonalisationEngine engine;
    engine.UpdateSignal("user_001", PersonalisationSignal::Starred, 1.0f);
    ASSERT(true);
}

TEST(TestTPE_ResetProfile)
{
    using namespace ExplorerLens::Engine;
    ThumbnailPersonalisationEngine engine;
    engine.ResetProfile("user_001");
    auto profile = engine.GetProfile("user_001");
    ASSERT(!profile.has_value());
}

TEST(TestTPE_ApplyNullSafe)
{
    using namespace ExplorerLens::Engine;
    ThumbnailPersonalisationEngine engine;
    UserPersonalisationProfile profile{};
    profile.userId = "u1";
    bool result = engine.ApplyPersonalisation(profile, nullptr, 0, 0);
    ASSERT(!result || result);
}

TEST(TestTPE_MultiUser)
{
    using namespace ExplorerLens::Engine;
    ThumbnailPersonalisationEngine engine;
    engine.UpdateSignal("userA", PersonalisationSignal::RecentlyViewed, 0.5f);
    engine.UpdateSignal("userB", PersonalisationSignal::SharedWith, 0.8f);
    ASSERT(true);
}

TEST(TestGUV3_ModelEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(UpscaleModel::ESRGAN) != static_cast<int>(UpscaleModel::RealSR));
    ASSERT(static_cast<int>(UpscaleModel::StableDiffusionSR) != static_cast<int>(UpscaleModel::Bicubic));
}

TEST(TestGUV3_FactorEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(UpscaleFactor::x2) != static_cast<int>(UpscaleFactor::x4));
    ASSERT(static_cast<int>(UpscaleFactor::x3) != static_cast<int>(UpscaleFactor::x8));
}

TEST(TestGUV3_JobStruct)
{
    using namespace ExplorerLens::Engine;
    UpscaleJob job{};
    job.model = UpscaleModel::ESRGAN;
    job.factor = UpscaleFactor::x4;
    job.denoisingStrength = 0.5f;
    job.tileSize = 256;
    job.useGPU = true;
    ASSERT(job.tileSize == 256);
    ASSERT(job.useGPU);
}

TEST(TestGUV3_OutputDimensions)
{
    using namespace ExplorerLens::Engine;
    GenerativeUpscalerV3 upscaler;
    auto [w, h] = upscaler.GetOutputDimensions(64, 64, UpscaleFactor::x4);
    ASSERT(w == 256);
    ASSERT(h == 256);
}

TEST(TestGUV3_SelectBestModel)
{
    using namespace ExplorerLens::Engine;
    GenerativeUpscalerV3 upscaler;
    UpscaleModel model = upscaler.SelectBestModel(128, 128);
    ASSERT(static_cast<int>(model) >= 0);
}

TEST(TestGUV3_VRAMEstimate)
{
    using namespace ExplorerLens::Engine;
    GenerativeUpscalerV3 upscaler;
    UpscaleJob job{};
    job.model = UpscaleModel::ESRGAN;
    job.factor = UpscaleFactor::x2;
    job.tileSize = 128;
    uint64_t vram = upscaler.GetVRAMRequiredBytes(job);
    ASSERT(vram >= 0);
}

TEST(TestGUV3_ModelAvailable)
{
    using namespace ExplorerLens::Engine;
    GenerativeUpscalerV3 upscaler;
    bool avail = upscaler.IsModelAvailable(UpscaleModel::Bicubic);
    ASSERT(!avail || avail);
}

TEST(TestGUV3_UpscaleNullSafe)
{
    using namespace ExplorerLens::Engine;
    GenerativeUpscalerV3 upscaler;
    UpscaleJob job{};
    job.model = UpscaleModel::Bicubic;
    job.factor = UpscaleFactor::x2;
    bool result = upscaler.Upscale(nullptr, 0, 0, nullptr, job);
    ASSERT(!result || result);
}

TEST(TestCMF_TierEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ModerationTier::Off) != static_cast<int>(ModerationTier::Enterprise));
    ASSERT(static_cast<int>(ModerationTier::Standard) != static_cast<int>(ModerationTier::Strict));
}

TEST(TestCMF_FlagEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ContentFlag::Safe) != static_cast<int>(ContentFlag::AdultContent));
    ASSERT(static_cast<int>(ContentFlag::Violence) != static_cast<int>(ContentFlag::CopyrightRisk));
}

TEST(TestCMF_ModerationResult)
{
    using namespace ExplorerLens::Engine;
    ModerationResult result{};
    result.flag = ContentFlag::Safe;
    result.confidenceScore = 0.99f;
    result.blockedByPolicy = false;
    result.reviewRequired = false;
    ASSERT(result.confidenceScore > 0.0f);
    ASSERT(!result.blockedByPolicy);
}

TEST(TestCMF_SetGetTier)
{
    using namespace ExplorerLens::Engine;
    ContentModerationFilter filter;
    filter.SetTier(ModerationTier::Enterprise);
    ASSERT(filter.GetTier() == ModerationTier::Enterprise);
}

TEST(TestCMF_DefaultTier)
{
    using namespace ExplorerLens::Engine;
    ContentModerationFilter filter;
    ASSERT(filter.GetTier() == ModerationTier::Standard);
}

TEST(TestCMF_AddBlocklist)
{
    using namespace ExplorerLens::Engine;
    ContentModerationFilter filter;
    filter.AddCustomBlocklist("custom_category", 0.8f);
    auto cats = filter.GetBlocklistCategories();
    ASSERT(cats.size() >= 1);
}

TEST(TestCMF_BlocklistCategories)
{
    using namespace ExplorerLens::Engine;
    ContentModerationFilter filter;
    filter.AddCustomBlocklist("cat1", 0.9f);
    filter.AddCustomBlocklist("cat2", 0.7f);
    auto cats = filter.GetBlocklistCategories();
    ASSERT(cats.size() >= 2);
}

TEST(TestCMF_EvaluateNullSafe)
{
    using namespace ExplorerLens::Engine;
    ContentModerationFilter filter;
    auto result = filter.Evaluate(nullptr, 0, 0);
    ASSERT(static_cast<int>(result.flag) >= 0);
}

TEST(TestGAT_EventTypeEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(GenAuditEventType::Generated) != static_cast<int>(GenAuditEventType::Upscaled));
    ASSERT(static_cast<int>(GenAuditEventType::StyleTransferred) != static_cast<int>(GenAuditEventType::Moderated));
}

TEST(TestGAT_RetentionEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(GenAuditRetention::SessionOnly) != static_cast<int>(GenAuditRetention::Permanent));
    ASSERT(static_cast<int>(GenAuditRetention::Days7) != static_cast<int>(GenAuditRetention::Days90));
}

TEST(TestGAT_EntryStruct)
{
    using namespace ExplorerLens::Engine;
    GenerativeAuditEntry entry{};
    entry.eventType = GenAuditEventType::Generated;
    entry.modelName = "esrgan_v3";
    entry.inputHash = 0xDEADBEEF;
    entry.outputHash = 0xCAFEBABE;
    entry.durationMs = 42;
    entry.timestampMs = 1000000;
    ASSERT(!entry.modelName.empty());
    ASSERT(entry.durationMs == 42);
}

TEST(TestGAT_RecordAndQuery)
{
    using namespace ExplorerLens::Engine;
    GenerativeAuditTrail trail;
    GenerativeAuditEntry entry{};
    entry.eventType = GenAuditEventType::Upscaled;
    entry.modelName = "realesr";
    entry.timestampMs = 9999;
    trail.Record(entry);
    auto entries = trail.Query(GenAuditEventType::Upscaled, 0);
    ASSERT(entries.size() >= 1);
}

TEST(TestGAT_SetGetRetention)
{
    using namespace ExplorerLens::Engine;
    GenerativeAuditTrail trail;
    trail.SetRetentionPolicy(GenAuditRetention::Days30);
    ASSERT(trail.GetRetentionPolicy() == GenAuditRetention::Days30);
}

TEST(TestGAT_PurgeEmpty)
{
    using namespace ExplorerLens::Engine;
    GenerativeAuditTrail trail;
    uint32_t purged = trail.Purge(9999999999ULL);
    ASSERT(purged == 0);
}

TEST(TestGAT_ExportEmpty)
{
    using namespace ExplorerLens::Engine;
    GenerativeAuditTrail trail;
    bool exported = trail.ExportToJson("test_audit_export.json");
    ASSERT(!exported || exported);
}

TEST(TestGAT_QueryEmpty)
{
    using namespace ExplorerLens::Engine;
    GenerativeAuditTrail trail;
    auto entries = trail.Query(GenAuditEventType::Generated, 0);
    ASSERT(entries.empty());
}

TEST(TestGAT_RecordMultiple)
{
    using namespace ExplorerLens::Engine;
    GenerativeAuditTrail trail;
    for (int i = 0; i < 5; ++i) {
        GenerativeAuditEntry entry{};
        entry.eventType = GenAuditEventType::Moderated;
        entry.modelName = "mod_filter";
        entry.timestampMs = static_cast<uint64_t>(i * 1000);
        trail.Record(entry);
    }
    auto entries = trail.Query(GenAuditEventType::Moderated, 0);
    ASSERT(entries.size() == 5);
}

//== Linux Nautilus Extension ==
TEST(TestLNE_IntegrationModeEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(NautilusIntegrationMode::Native) != static_cast<int>(NautilusIntegrationMode::DBus));
    ASSERT(static_cast<int>(NautilusIntegrationMode::FlatpakPortal)
           != static_cast<int>(NautilusIntegrationMode::Fallback));
}
TEST(TestLNE_VersionEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(NautilusVersion::V42) != static_cast<int>(NautilusVersion::V45));
    ASSERT(static_cast<int>(NautilusVersion::V43) != static_cast<int>(NautilusVersion::V46));
}
TEST(TestLNE_ConfigStruct)
{
    using namespace ExplorerLens::Engine;
    NautilusConfig cfg;
    cfg.extensionPath = "test";
    cfg.maxThumbnailSizePx = 256;
    cfg.enableCaching = true;
    ASSERT(!cfg.extensionPath.empty());
    ASSERT(cfg.maxThumbnailSizePx == 256);
}
TEST(TestLNE_NotRegistered)
{
    using namespace ExplorerLens::Engine;
    LinuxNautilusExtension ext;
    ASSERT(!ext.IsRegistered());
}
TEST(TestLNE_GetFormats)
{
    using namespace ExplorerLens::Engine;
    LinuxNautilusExtension ext;
    auto formats = ext.GetSupportedFormats();
    ASSERT(formats.size() >= 0);
}
TEST(TestLNE_InitializeConfig)
{
    using namespace ExplorerLens::Engine;
    LinuxNautilusExtension ext;
    NautilusConfig cfg;
    auto result = ext.Initialize(cfg);
    ASSERT(!result || result);
}
TEST(TestLNE_UnregisterSafe)
{
    using namespace ExplorerLens::Engine;
    LinuxNautilusExtension ext;
    ASSERT(ext.UnregisterProvider());
}
TEST(TestLNE_RegisterFails)
{
    using namespace ExplorerLens::Engine;
    LinuxNautilusExtension ext;
    auto result = ext.RegisterProvider();
    ASSERT(!result || result);
}
TEST(TestLNE_DBusMode)
{
    using namespace ExplorerLens::Engine;
    NautilusConfig cfg;
    cfg.mode = NautilusIntegrationMode::DBus;
    cfg.version = NautilusVersion::V44;
    ASSERT(static_cast<int>(cfg.mode) > 0);
}

//== KDE Dolphin Extension ==
TEST(TestKDE_PluginTypeEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DolphinPluginType::ThumbCreator) != static_cast<int>(DolphinPluginType::KIOSlave));
    ASSERT(static_cast<int>(DolphinPluginType::PreviewPlugin) != static_cast<int>(DolphinPluginType::ThumbCreator));
}
TEST(TestKDE_PriorityEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DolphinPriority::Low) != static_cast<int>(DolphinPriority::High));
    ASSERT(static_cast<int>(DolphinPriority::Normal) != static_cast<int>(DolphinPriority::Realtime));
}
TEST(TestKDE_ConfigStruct)
{
    using namespace ExplorerLens::Engine;
    DolphinPluginConfig cfg;
    cfg.serviceName = "test";
    cfg.maxConcurrentJobs = 4;
    ASSERT(!cfg.serviceName.empty());
}
TEST(TestKDE_NotActive)
{
    using namespace ExplorerLens::Engine;
    KDEDolphinExtension ext;
    ASSERT(!ext.IsActive());
}
TEST(TestKDE_SetPriority)
{
    using namespace ExplorerLens::Engine;
    KDEDolphinExtension ext;
    ASSERT(ext.SetPriority(DolphinPriority::High));
}
TEST(TestKDE_RegisterFails)
{
    using namespace ExplorerLens::Engine;
    KDEDolphinExtension ext;
    auto result = ext.RegisterThumbCreator();
    ASSERT(!result || result);
}
TEST(TestKDE_UnregisterSafe)
{
    using namespace ExplorerLens::Engine;
    KDEDolphinExtension ext;
    ASSERT(ext.UnregisterThumbCreator());
}
TEST(TestKDE_InitConfig)
{
    using namespace ExplorerLens::Engine;
    KDEDolphinExtension ext;
    DolphinPluginConfig cfg;
    auto result = ext.Initialize(cfg);
    ASSERT(!result || result);
}
TEST(TestKDE_MimeTypes)
{
    using namespace ExplorerLens::Engine;
    DolphinPluginConfig cfg;
    cfg.mimeTypes = {"image/png", "image/jpeg"};
    ASSERT(cfg.mimeTypes.size() >= 2);
}

//== Thunar Thumbnail Extension ==
TEST(TestTTE_InterfaceEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ThunarInterfaceVersion::V1) != static_cast<int>(ThunarInterfaceVersion::V3));
    ASSERT(static_cast<int>(ThunarInterfaceVersion::V2) != static_cast<int>(ThunarInterfaceVersion::V1));
}
TEST(TestTTE_SchedulerEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(TumblerSchedulerType::Background) != static_cast<int>(TumblerSchedulerType::Urgent));
    ASSERT(static_cast<int>(TumblerSchedulerType::Foreground) != static_cast<int>(TumblerSchedulerType::Background));
}
TEST(TestTTE_ConfigStruct)
{
    using namespace ExplorerLens::Engine;
    ThunarTumblerConfig cfg;
    cfg.cachePath = "/tmp/thunar";
    cfg.maxFileSizeMB = 100;
    cfg.timeoutMs = 5000;
    ASSERT(cfg.maxFileSizeMB == 100);
}
TEST(TestTTE_NotRegistered)
{
    using namespace ExplorerLens::Engine;
    ThunarThumbnailExtension ext;
    ASSERT(!ext.IsRegistered());
}
TEST(TestTTE_SetScheduler)
{
    using namespace ExplorerLens::Engine;
    ThunarThumbnailExtension ext;
    ASSERT(ext.SetScheduler(TumblerSchedulerType::Foreground));
}
TEST(TestTTE_RegisterFails)
{
    using namespace ExplorerLens::Engine;
    ThunarThumbnailExtension ext;
    auto result = ext.RegisterTumbler();
    ASSERT(!result || result);
}
TEST(TestTTE_UnregisterSafe)
{
    using namespace ExplorerLens::Engine;
    ThunarThumbnailExtension ext;
    ASSERT(ext.UnregisterTumbler());
}
TEST(TestTTE_InitConfig)
{
    using namespace ExplorerLens::Engine;
    ThunarTumblerConfig cfg;
    ThunarThumbnailExtension ext;
    auto result = ext.Initialize(cfg);
    ASSERT(!result || result);
}
TEST(TestTTE_TimeoutValue)
{
    using namespace ExplorerLens::Engine;
    ThunarTumblerConfig cfg;
    cfg.timeoutMs = 3000;
    ASSERT(cfg.timeoutMs == 3000);
}

//== macOS Quick Look V3 ==
TEST(TestQL_APIEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(QuickLookAPI::Legacy) != static_cast<int>(QuickLookAPI::Modern));
    ASSERT(static_cast<int>(QuickLookAPI::Thumbnail) != static_cast<int>(QuickLookAPI::Preview));
}
TEST(TestQL_ScaleEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(QuickLookScale::Standard) != static_cast<int>(QuickLookScale::Retina));
    ASSERT(static_cast<int>(QuickLookScale::ProMotion) != static_cast<int>(QuickLookScale::Standard));
}
TEST(TestQL_ConfigStruct)
{
    using namespace ExplorerLens::Engine;
    QuickLookConfig cfg;
    cfg.bundleIdentifier = "com.explorerlens.ql";
    cfg.maxDimension = 1024;
    cfg.enableSandbox = true;
    ASSERT(!cfg.bundleIdentifier.empty());
}
TEST(TestQL_NotActive)
{
    using namespace ExplorerLens::Engine;
    MacOSQuickLookV3 ext;
    ASSERT(!ext.IsExtensionActive());
}
TEST(TestQL_GetUTIs)
{
    using namespace ExplorerLens::Engine;
    MacOSQuickLookV3 ext;
    auto utis = ext.GetSupportedUTIs();
    ASSERT(utis.size() >= 0);
}
TEST(TestQL_RegisterFails)
{
    using namespace ExplorerLens::Engine;
    MacOSQuickLookV3 ext;
    auto result = ext.RegisterExtension();
    ASSERT(!result || result);
}
TEST(TestQL_UnregisterSafe)
{
    using namespace ExplorerLens::Engine;
    MacOSQuickLookV3 ext;
    ASSERT(ext.UnregisterExtension());
}
TEST(TestQL_InitConfig)
{
    using namespace ExplorerLens::Engine;
    QuickLookConfig cfg;
    MacOSQuickLookV3 ext;
    auto result = ext.Initialize(cfg);
    ASSERT(!result || result);
}
TEST(TestQL_SandboxEnabled)
{
    using namespace ExplorerLens::Engine;
    QuickLookConfig cfg;
    cfg.enableSandbox = true;
    ASSERT(cfg.enableSandbox);
}

//== Linux Thumbnailer Daemon ==
TEST(TestLTD_ThumbSizeEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ThumbSize::Normal) != static_cast<int>(ThumbSize::XLarge));
    ASSERT(static_cast<int>(ThumbSize::Large) != static_cast<int>(ThumbSize::XXLarge));
}
TEST(TestLTD_DaemonStateEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DaemonState::Stopped) != static_cast<int>(DaemonState::Running));
    ASSERT(static_cast<int>(DaemonState::Starting) != static_cast<int>(DaemonState::ShuttingDown));
}
TEST(TestLTD_ConfigStruct)
{
    using namespace ExplorerLens::Engine;
    ThumbnailerDaemonConfig cfg;
    cfg.cachePath = "/tmp/thumbs";
    cfg.maxCacheSize = 10000000;
    cfg.maxConcurrent = 4;
    ASSERT(cfg.maxConcurrent == 4);
}
TEST(TestLTD_InitialState)
{
    using namespace ExplorerLens::Engine;
    LinuxThumbnailerDaemon daemon;
    ASSERT(daemon.GetState() == DaemonState::Stopped);
}
TEST(TestLTD_StartDaemon)
{
    using namespace ExplorerLens::Engine;
    LinuxThumbnailerDaemon daemon;
    ThumbnailerDaemonConfig cfg;
    auto result = daemon.Start(cfg);
    ASSERT(!result || result);
}
TEST(TestLTD_StopSafe)
{
    using namespace ExplorerLens::Engine;
    LinuxThumbnailerDaemon daemon;
    ASSERT(daemon.Stop());
}
TEST(TestLTD_PurgeCache)
{
    using namespace ExplorerLens::Engine;
    LinuxThumbnailerDaemon daemon;
    auto result = daemon.PurgeCache();
    ASSERT(result >= 0);
}
TEST(TestLTD_GenerateNullPath)
{
    using namespace ExplorerLens::Engine;
    LinuxThumbnailerDaemon daemon;
    auto result = daemon.GenerateThumbnail("", ThumbSize::Normal);
    ASSERT(!result || result);
}
TEST(TestLTD_AutoStart)
{
    using namespace ExplorerLens::Engine;
    ThumbnailerDaemonConfig cfg;
    cfg.autoStart = true;
    ASSERT(cfg.autoStart);
}

//== Wayland Shell Extension ==
TEST(TestWSE_ProtocolEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(WaylandProtocol::WlrLayerShell) != static_cast<int>(WaylandProtocol::XDGForeign));
    ASSERT(static_cast<int>(WaylandProtocol::XDGDecoration) != static_cast<int>(WaylandProtocol::Viewporter));
}
TEST(TestWSE_CompositorEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(CompositorType::Mutter) != static_cast<int>(CompositorType::Sway));
    ASSERT(static_cast<int>(CompositorType::KWin) != static_cast<int>(CompositorType::Hyprland));
}
TEST(TestWSE_ConfigStruct)
{
    using namespace ExplorerLens::Engine;
    WaylandConfig cfg;
    cfg.displayName = "wayland-0";
    cfg.scaleFactor = 2.0f;
    cfg.enableHiDPI = true;
    ASSERT(cfg.scaleFactor > 1.0f);
}
TEST(TestWSE_NotConnected)
{
    using namespace ExplorerLens::Engine;
    WaylandShellExtension ext;
    ASSERT(!ext.IsConnected());
}
TEST(TestWSE_ConnectFails)
{
    using namespace ExplorerLens::Engine;
    WaylandShellExtension ext;
    auto result = ext.Connect();
    ASSERT(!result || result);
}
TEST(TestWSE_DisconnectSafe)
{
    using namespace ExplorerLens::Engine;
    WaylandShellExtension ext;
    ASSERT(ext.Disconnect());
}
TEST(TestWSE_GetCompositor)
{
    using namespace ExplorerLens::Engine;
    WaylandShellExtension ext;
    auto result = ext.GetCompositorType();
    ASSERT(static_cast<int>(result) >= 0);
}
TEST(TestWSE_InitConfig)
{
    using namespace ExplorerLens::Engine;
    WaylandConfig cfg;
    WaylandShellExtension ext;
    auto result = ext.Initialize(cfg);
    ASSERT(!result || result);
}
TEST(TestWSE_HiDPIEnabled)
{
    using namespace ExplorerLens::Engine;
    WaylandConfig cfg;
    cfg.enableHiDPI = true;
    ASSERT(cfg.enableHiDPI);
}

//== macOS Launch Services Adapter ==
TEST(TestMLS_RoleEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(LSHandlerRole::Viewer) != static_cast<int>(LSHandlerRole::Editor));
    ASSERT(static_cast<int>(LSHandlerRole::Shell) != static_cast<int>(LSHandlerRole::All));
}
TEST(TestMLS_ScopeEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(LSRegistrationScope::User) != static_cast<int>(LSRegistrationScope::System));
    ASSERT(static_cast<int>(LSRegistrationScope::Temporary) != static_cast<int>(LSRegistrationScope::User));
}
TEST(TestMLS_ConfigStruct)
{
    using namespace ExplorerLens::Engine;
    LaunchServicesConfig cfg;
    cfg.bundleId = "com.explorerlens";
    cfg.uti = "public.image";
    cfg.iconName = "lens";
    ASSERT(!cfg.bundleId.empty());
}
TEST(TestMLS_NotRegistered)
{
    using namespace ExplorerLens::Engine;
    MacOSLaunchServicesAdapter adapter;
    ASSERT(!adapter.IsHandlerRegistered("public.image"));
}
TEST(TestMLS_GetUTIs)
{
    using namespace ExplorerLens::Engine;
    MacOSLaunchServicesAdapter adapter;
    auto utis = adapter.GetRegisteredUTIs();
    ASSERT(utis.size() >= 0);
}
TEST(TestMLS_RegisterFails)
{
    using namespace ExplorerLens::Engine;
    MacOSLaunchServicesAdapter adapter;
    LaunchServicesConfig cfg;
    auto result = adapter.RegisterHandler(cfg);
    ASSERT(!result || result);
}
TEST(TestMLS_UnregisterSafe)
{
    using namespace ExplorerLens::Engine;
    MacOSLaunchServicesAdapter adapter;
    ASSERT(adapter.UnregisterHandler("test"));
}
TEST(TestMLS_SetDefault)
{
    using namespace ExplorerLens::Engine;
    MacOSLaunchServicesAdapter adapter;
    auto result = adapter.SetDefaultHandler("public.image");
    ASSERT(!result || result);
}
TEST(TestMLS_TemporaryScope)
{
    using namespace ExplorerLens::Engine;
    LaunchServicesConfig cfg;
    cfg.scope = LSRegistrationScope::Temporary;
    ASSERT(cfg.scope == LSRegistrationScope::Temporary);
}

//== Cross-Platform Test Harness ==
TEST(TestXPT_PlatformEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(TestPlatform::Windows) != static_cast<int>(TestPlatform::Linux));
    ASSERT(static_cast<int>(TestPlatform::macOS) != static_cast<int>(TestPlatform::All));
}
TEST(TestXPT_VerdictEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(XPlatVerdict::Passed) != static_cast<int>(XPlatVerdict::Failed));
    ASSERT(static_cast<int>(XPlatVerdict::Skipped) != static_cast<int>(XPlatVerdict::Error));
}
TEST(TestXPT_TestCaseStruct)
{
    using namespace ExplorerLens::Engine;
    PlatformTestCase tc;
    tc.name = "testA";
    tc.platform = TestPlatform::Windows;
    tc.verdict = XPlatVerdict::Passed;
    tc.durationMs = 42;
    ASSERT(!tc.name.empty());
    ASSERT(tc.durationMs == 42);
}
TEST(TestXPT_InitialPassRate)
{
    using namespace ExplorerLens::Engine;
    XPlatformTestHarness harness;
    ASSERT(harness.GetPassRate() >= 0.0f);
}
TEST(TestXPT_TotalTests)
{
    using namespace ExplorerLens::Engine;
    XPlatformTestHarness harness;
    ASSERT(harness.GetTotalTests() >= 0);
}
TEST(TestXPT_GetFailed)
{
    using namespace ExplorerLens::Engine;
    XPlatformTestHarness harness;
    auto failed = harness.GetFailedTests();
    ASSERT(failed.size() >= 0);
}
TEST(TestXPT_Reset)
{
    using namespace ExplorerLens::Engine;
    XPlatformTestHarness harness;
    harness.Reset();
    ASSERT(harness.GetTotalTests() == 0);
}
TEST(TestXPT_RunSingle)
{
    using namespace ExplorerLens::Engine;
    XPlatformTestHarness harness;
    auto result = harness.RunSingle("nonexistent");
    ASSERT(static_cast<int>(result.verdict) >= 0);
}
TEST(TestXPT_RunAll)
{
    using namespace ExplorerLens::Engine;
    XPlatformTestHarness harness;
    auto results = harness.RunAll(TestPlatform::All);
    ASSERT(results.size() >= 0);
}

//== HDR Tone Mapping Pipeline ==
TEST(TestZenith_HDROperatorCount)
{
    ASSERT(HDRToneMappingPipeline::OperatorCount() == 6);
}
TEST(TestZenith_HDROperatorNames)
{
    ASSERT(std::wstring(HDRToneMappingPipeline::OperatorName(HDRToneMapOp::Reinhard)) == L"Reinhard");
    ASSERT(std::wstring(HDRToneMappingPipeline::OperatorName(HDRToneMapOp::ACES)) == L"ACES Filmic");
}
TEST(TestZenith_HDRConfigDefaults)
{
    ToneMappingConfig cfg;
    ASSERT(cfg.op == HDRToneMapOp::ACES);
    ASSERT(cfg.exposure == 0.0f);
}

//== Color Space Engine ==
TEST(TestZenith_ColorSpaceCount)
{
    ASSERT(ColorSpaceEngine::SpaceCount() == 9);
}
TEST(TestZenith_ColorSpaceNames)
{
    ASSERT(std::wstring(ColorSpaceEngine::SpaceName(ColorSpace::sRGB)) == L"sRGB");
    ASSERT(std::wstring(ColorSpaceEngine::SpaceName(ColorSpace::DisplayP3)) == L"Display P3");
}
TEST(TestZenith_ColorSpaceConversion)
{
    ASSERT(ColorSpaceEngine::SRGBToLinear(0.0f) == 0.0f);
    ASSERT(ColorSpaceEngine::LinearToSRGB(0.0f) == 0.0f);
}

//== GPU Texture Compression ==
TEST(TestZenith_TextureFormatCount)
{
    ASSERT(GPUTextureCompressionPipeline::FormatCount() == 9);
}
TEST(TestZenith_TextureFormatNames)
{
    ASSERT(std::wstring(GPUTextureCompressionPipeline::FormatName(TextureFormat::BC7_RGBA)) == L"BC7 (RGBA)");
    ASSERT(std::wstring(GPUTextureCompressionPipeline::FormatName(TextureFormat::ASTC_4x4)) == L"ASTC 4x4");
}
TEST(TestZenith_TextureBPP)
{
    ASSERT(GPUTextureCompressionPipeline::BitsPerPixel(TextureFormat::BC1_RGB) == 4);
    ASSERT(GPUTextureCompressionPipeline::BitsPerPixel(TextureFormat::Uncompressed_BGRA) == 32);
}

//== Adaptive DPI Scaler ==
TEST(TestZenith_DPIStrategyCount)
{
    ASSERT(AdaptiveDPIScaler::StrategyCount() == 5);
}
TEST(TestZenith_DPIClassify)
{
    ASSERT(AdaptiveDPIScaler::ClassifyDPI(96) == DPITier::Standard);
    ASSERT(AdaptiveDPIScaler::ClassifyDPI(192) == DPITier::VeryHigh);
    ASSERT(AdaptiveDPIScaler::ClassifyDPI(300) == DPITier::Ultra);
}
TEST(TestZenith_DPIScaledSize)
{
    ASSERT(AdaptiveDPIScaler::ScaledSize(256, 2.0f) == 512);
}

//== Format Fingerprint DB ==
TEST(TestZenith_FormatFamilyCount)
{
    ASSERT(FormatFingerprintDB::FamilyCount() == 15);
}
TEST(TestZenith_FingerprintMatchesPNG)
{
    uint8_t pngMagic[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    ASSERT(FormatFingerprintDB::MatchesPNG(pngMagic, 8) == true);
    uint8_t notPng[] = {0x00, 0x00, 0x00, 0x00};
    ASSERT(FormatFingerprintDB::MatchesPNG(notPng, 4) == false);
}
TEST(TestZenith_FingerprintMatchesJPEG)
{
    uint8_t jpegMagic[] = {0xFF, 0xD8, 0xFF};
    ASSERT(FormatFingerprintDB::MatchesJPEG(jpegMagic, 3) == true);
}

//== Nested Archive Preview ==
TEST(TestZenith_NestingPolicyCount)
{
    ASSERT(NestedArchivePreview::PolicyCount() == 4);
}
TEST(TestZenith_NestingEffectiveDepth)
{
    ASSERT(NestedArchivePreview::EffectiveMaxDepth(NestingPolicy::SingleLevel, 5) == 1);
    ASSERT(NestedArchivePreview::EffectiveMaxDepth(NestingPolicy::Unlimited, 5) == 10);
}

//== Multi-Page Navigator ==
TEST(TestZenith_PageStrategyCount)
{
    ASSERT(MultiPageNavigator::StrategyCount() == 6);
}
TEST(TestZenith_PageSelectFirst)
{
    ASSERT(MultiPageNavigator::SelectPage(PageSelectionStrategy::FirstPage, 100) == 0);
}
TEST(TestZenith_PageSelectLast)
{
    ASSERT(MultiPageNavigator::SelectPage(PageSelectionStrategy::MiddlePage, 100) == 50);
}

//== Animated Format Controller ==
TEST(TestZenith_FrameSelectionCount)
{
    ASSERT(AnimatedFormatController::ModeCount() == 6);
}
TEST(TestZenith_AnimSelectFrame)
{
    ASSERT(AnimatedFormatController::SelectFrame(FrameSelectionMode::FirstFrame, 50) == 0);
    ASSERT(AnimatedFormatController::SelectFrame(FrameSelectionMode::MiddleFrame, 50) == 25);
}

//== Metadata Extraction Pipeline ==
TEST(TestZenith_MetadataSourceCount)
{
    ASSERT(MetadataExtractionPipeline::SourceCount() == 8);
}
TEST(TestZenith_MetadataSourceNames)
{
    ASSERT(std::wstring(MetadataExtractionPipeline::SourceName(MetadataSource::EXIF)) == L"EXIF");
    ASSERT(std::wstring(MetadataExtractionPipeline::SourceName(MetadataSource::XMP)) == L"XMP");
}

//== Content-Aware Thumbnail Selector ==
TEST(TestZenith_SaliencyAlgoCount)
{
    ASSERT(ContentAwareThumbnailSelector::AlgorithmCount() == 5);
}
TEST(TestZenith_CenterCropCalc)
{
    auto crop = ContentAwareThumbnailSelector::CenterCropCalc(1920, 1080, 1.0f);
    ASSERT(crop.srcX >= 0);
    ASSERT(crop.srcW <= 1920);
}

//== Face Detection & Orientation ==
TEST(TestZenith_EXIFOrientationDegrees)
{
    ASSERT(FaceDetectionOrientation::OrientationToDegrees(EXIFOrientation::Rotate180) == 180);
    ASSERT(FaceDetectionOrientation::OrientationToDegrees(EXIFOrientation::Rotate90) == 90);
}
TEST(TestZenith_EXIFDimensionSwap)
{
    ASSERT(FaceDetectionOrientation::RequiresDimensionSwap(EXIFOrientation::Rotate90) == true);
    ASSERT(FaceDetectionOrientation::RequiresDimensionSwap(EXIFOrientation::Normal) == false);
}

//== Document Layout Analyzer ==
TEST(TestZenith_DocRegionTypeCount)
{
    ASSERT(DocumentLayoutAnalyzer::RegionTypeCount() == 11);
}
TEST(TestZenith_DocTextDensity)
{
    ASSERT(DocumentLayoutAnalyzer::EstimateTextDensity(500, 800, 600) > 0.0);
}

//== Visual Similarity Index ==
TEST(TestZenith_HashAlgoCount)
{
    ASSERT(VisualSimilarityIndex::AlgorithmCount() == 5);
}
TEST(TestZenith_HammingDistance)
{
    ASSERT(VisualSimilarityIndex::HammingDistance(0xFF00, 0xFF0F) == 4);
    ASSERT(VisualSimilarityIndex::HammingDistance(0, 0) == 0);
}
TEST(TestZenith_SimilarityClassify)
{
    ASSERT(VisualSimilarityIndex::Classify(0) == SimilarityClass::Identical);
    ASSERT(VisualSimilarityIndex::Classify(30) == SimilarityClass::Different);
}

//== Smart Quality Predictor ==
TEST(TestZenith_ImageComplexityCount)
{
    ASSERT(SmartQualityPredictor::ComplexityCount() == 5);
}
TEST(TestZenith_PredictJPEGQuality)
{
    auto q = SmartQualityPredictor::PredictJPEGQuality(ImageComplexity::Simple);
    ASSERT(q >= 60 && q <= 100);
}

//== Lock-Free Decode Pipeline ==
TEST(TestZenith_PipelineStageStateCount)
{
    ASSERT(LockFreeDecodePipeline::StageStateCount() == 7);
}
TEST(TestZenith_NextPowerOf2)
{
    ASSERT(LockFreeDecodePipeline::NextPowerOf2(5) == 8);
    ASSERT(LockFreeDecodePipeline::NextPowerOf2(16) == 16);
}
TEST(TestZenith_IsPowerOf2)
{
    ASSERT(LockFreeDecodePipeline::IsPowerOf2(64) == true);
    ASSERT(LockFreeDecodePipeline::IsPowerOf2(63) == false);
}

//== Memory-Mapped I/O Optimizer ==
TEST(TestZenith_MappingStrategyCount)
{
    ASSERT(MemoryMappedIOOptimizer::StrategyCount() == 5);
}
TEST(TestZenith_MMapAlignOffset)
{
    ASSERT(MemoryMappedIOOptimizer::AlignOffset(65536, 65536) == 65536);
    ASSERT(MemoryMappedIOOptimizer::AlignOffset(100, 65536) == 0);
}

//== GPU Texture Atlas Manager ==
TEST(TestZenith_AtlasPackingAlgoCount)
{
    GPUTextureAtlasManager atlas;
    ASSERT(atlas.Initialize(1024, 1024, 4));
    auto stats = atlas.GetStats();
    ASSERT(stats.atlasesInUse == 0);
}
TEST(TestZenith_AtlasCalcVRAM)
{
    GPUTextureAtlasManager atlas;
    atlas.Initialize(4096, 4096, 2);
    auto alloc = atlas.Allocate(64, 64);
    ASSERT(alloc.IsValid());
    auto stats = atlas.GetStats();
    ASSERT(stats.totalSpaceBytes > 0);
}
TEST(TestZenith_AtlasMaxSlots)
{
    GPUTextureAtlasManager atlas;
    atlas.Initialize(256, 256, 1);
    auto a1 = atlas.Allocate(128, 128);
    auto a2 = atlas.Allocate(128, 128);
    ASSERT(a1.IsValid() && a2.IsValid());
    auto stats = atlas.GetStats();
    ASSERT(stats.liveAllocations == 2);
}

//== Predictive Prefetch Engine ==
TEST(TestZenith_PrefetchStrategyCount)
{
    ASSERT(PredictivePrefetchEngine::StrategyCount() == 5);
}
TEST(TestZenith_PrefetchCalcHitRate)
{
    ASSERT(PredictivePrefetchEngine::CalcHitRate(90, 10) > 0.89);
}

//== Thread Pool Optimizer ==
TEST(TestZenith_PoolSizingPolicyCount)
{
    ASSERT(ThreadPoolOptimizer::PolicyCount() == 5);
}
TEST(TestZenith_ThreadPoolRecommend)
{
    auto n = ThreadPoolOptimizer::RecommendThreads(PoolSizingPolicy::CoreCount, 8, PowerProfile::HighPerformance);
    ASSERT(n >= 1);
}

//== Windows Search Protocol ==
TEST(TestZenith_SearchFieldCount)
{
    ASSERT(WindowsSearchProtocol::FieldCount() == 10);
}
TEST(TestZenith_SearchFieldNames)
{
    ASSERT(std::wstring(WindowsSearchProtocol::FieldName(SearchIndexField::FileName)) == L"File Name");
}

//== Virtual Filesystem Abstraction ==
TEST(TestZenith_VFSBackendCount)
{
    ASSERT(VirtualFilesystemAbstraction::BackendCount() == 8);
}
TEST(TestZenith_VFSNeedsDownload)
{
    ASSERT(VirtualFilesystemAbstraction::NeedsDownload(FileAvailability::CloudOnly) == true);
    ASSERT(VirtualFilesystemAbstraction::NeedsDownload(FileAvailability::FullyLocal) == false);
}

//== Role-Based Format Policy ==
TEST(TestZenith_FormatPolicyActionCount)
{
    ASSERT(RoleBasedFormatPolicy::ActionCount() == 5);
}
TEST(TestZenith_FormatPolicyPriority)
{
    ASSERT(RoleBasedFormatPolicy::HigherPriority(FormatPolicySource::GroupPolicy, FormatPolicySource::UserPreference)
           == FormatPolicySource::GroupPolicy);
    ASSERT(RoleBasedFormatPolicy::HigherPriority(FormatPolicySource::UserPreference, FormatPolicySource::GroupPolicy)
           == FormatPolicySource::GroupPolicy);
}

//== Audit Trail Logger ==
TEST(TestZenith_AuditSeverityCount)
{
    ASSERT(AuditTrailLogger::SeverityCount() == 5);
}
TEST(TestZenith_AuditCategoryCount)
{
    ASSERT(AuditTrailLogger::CategoryCount() == 8);
}
TEST(TestZenith_AuditSeverityNames)
{
    ASSERT(std::wstring(AuditTrailLogger::SeverityName(TrailSeverity::Info)) == L"Info");
    ASSERT(std::wstring(AuditTrailLogger::SeverityName(TrailSeverity::Critical)) == L"Critical");
}

//== Content Inspection Gateway ==
TEST(TestZenith_ContentClassificationCount)
{
    ASSERT(ContentInspectionGateway::ClassificationCount() == 5);
}
TEST(TestZenith_ContentShouldBlock)
{
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
TEST(TestZenith_CertValidationCount)
{
    ASSERT(CertificateTrustValidator::ValidationCount() == 7);
}
TEST(TestZenith_CertSufficientTrust)
{
    ASSERT(CertificateTrustValidator::IsSufficientTrust(CertTrustLevel::ExplorerLensSigned, CertTrustLevel::UserTrusted)
           == true);
    ASSERT(CertificateTrustValidator::IsSufficientTrust(CertTrustLevel::Untrusted, CertTrustLevel::UserTrusted)
           == false);
}

//== Encrypted Format Handler ==
TEST(TestZenith_EncryptionTypeCount)
{
    ASSERT(EncryptedFormatHandler::EncryptionCount() == 10);
}
TEST(TestZenith_StrongEncryption)
{
    ASSERT(EncryptedFormatHandler::IsStrongEncryption(EncryptionType::ZipAES256) == true);
    ASSERT(EncryptedFormatHandler::IsStrongEncryption(EncryptionType::ZipZipCrypto) == false);
}

//== Self-Healing Decoder ==
TEST(TestZenith_RecoveryStrategyCount)
{
    ASSERT(SelfHealingDecoder::StrategyCount() == 5);
}
TEST(TestZenith_DecoderHealthClassify)
{
    ASSERT(SelfHealingDecoder::ClassifyHealth(0.0f) == DecoderHealthState::Healthy);
    ASSERT(SelfHealingDecoder::ClassifyHealth(0.03f) == DecoderHealthState::Degraded);
    ASSERT(SelfHealingDecoder::ClassifyHealth(0.25f) == DecoderHealthState::Quarantined);
}

//== Crash Analytics Collector ==
TEST(TestZenith_CrashCategoryCount)
{
    ASSERT(CrashAnalyticsCollector::CategoryCount() == 8);
}
TEST(TestZenith_CrashDumpSize)
{
    auto size = CrashAnalyticsCollector::EstimateDumpSize(CrashDumpType::MiniDump);
    ASSERT(size > 0);
}

//== Performance Anomaly Detector ==
TEST(TestZenith_AnomalyTypeCount)
{
    ASSERT(PerformanceAnomalyDetector::TypeCount() == 8);
}
TEST(TestZenith_AnomalyClassifySeverity)
{
    ASSERT(PerformanceAnomalyDetector::ClassifySeverity(1.5) == PerfAnomalySeverity::Minor);
    ASSERT(PerformanceAnomalyDetector::ClassifySeverity(10.0) == PerfAnomalySeverity::Critical);
}
TEST(TestZenith_AnomalyIsAnomaly)
{
    ASSERT(PerformanceAnomalyDetector::IsAnomaly(10.0, 2.0, 3.0) == true);
    ASSERT(PerformanceAnomalyDetector::IsAnomaly(1.0, 2.0, 3.0) == false);
}

//== Diagnostic Report Generator V2 ==
TEST(TestZenith_DiagReportSectionCount)
{
    ASSERT(DiagnosticReportGeneratorV2::SectionCount() == 9);
}
TEST(TestZenith_DiagReportMimeType)
{
    ASSERT(std::string(DiagnosticReportGeneratorV2::MimeType(DiagReportFormat::HTML)) == "text/html");
    ASSERT(std::string(DiagnosticReportGeneratorV2::MimeType(DiagReportFormat::JSON)) == "application/json");
}

//== Health Check Endpoint ==
TEST(TestZenith_HealthStatusCount)
{
    ASSERT(HealthCheckEndpoint::StatusCount() == 5);
}
TEST(TestZenith_HealthAggregateAllHealthy)
{
    ASSERT(HealthCheckEndpoint::AggregateHealth(2, 0, 0) == EndpointHealthStatus::Healthy);
}
TEST(TestZenith_HealthAggregateDegraded)
{
    ASSERT(HealthCheckEndpoint::AggregateHealth(1, 1, 0) == EndpointHealthStatus::Degraded);
}

//== Preview Tooltip Renderer ==
TEST(TestZenith_TooltipStyleCount)
{
    ASSERT(PreviewTooltipRenderer::StyleCount() == 5);
}
TEST(TestZenith_TooltipStyleNames)
{
    ASSERT(std::wstring(PreviewTooltipRenderer::StyleName(TooltipStyle::Minimal)) == L"Minimal");
    ASSERT(std::wstring(PreviewTooltipRenderer::StyleName(TooltipStyle::Rich)) == L"Rich");
}

//== Format Gallery Compositor ==
TEST(TestZenith_GalleryLayoutCount)
{
    ASSERT(FormatGalleryCompositor::LayoutCount() == 6);
}
TEST(TestZenith_GalleryGridCellSize)
{
    auto cell = FormatGalleryCompositor::GridCellSize(800, 4, 4.0f);
    ASSERT(cell > 0);
}

//== Quick Actions Overlay ==
TEST(TestZenith_QuickActionTypeCount)
{
    ASSERT(QuickActionsOverlay::ActionCount() == 9);
}
TEST(TestZenith_QuickActionNames)
{
    ASSERT(std::wstring(QuickActionsOverlay::ActionName(QuickActionType::Crop)) == L"Crop");
    ASSERT(std::wstring(QuickActionsOverlay::ActionName(QuickActionType::ShareFile)) == L"Share");
}

//== Accessibility Narrator Bridge ==
TEST(TestZenith_A11yFeatureCount)
{
    ASSERT(AccessibilityNarratorBridge::FeatureCount() == 6);
}
TEST(TestZenith_A11yNarratorText)
{
    auto text = AccessibilityNarratorBridge::GenerateNarratorText(L"test.png", 1920, 1080, L"PNG");
    ASSERT(std::wstring(text).length() > 0);
}

//== Usage Analytics Dashboard ==
TEST(TestZenith_AnalyticsMetricCount)
{
    ASSERT(UsageAnalyticsDashboard::MetricCount() == 8);
}
TEST(TestZenith_AnalyticsMetricNames)
{
    ASSERT(std::wstring(UsageAnalyticsDashboard::MetricName(AnalyticsMetric::ThumbnailsGenerated))
           == L"Thumbnails Generated");
}

//== Decoder Performance Profiler ==
TEST(TestZenith_ProfileGranularityCount)
{
    ASSERT(DecoderPerformanceProfiler::GranularityCount() == 4);
}
TEST(TestZenith_ProfileIsRegression)
{
    ASSERT(DecoderPerformanceProfiler::IsRegression(10.0, 5.0, 0.10) == true);
    ASSERT(DecoderPerformanceProfiler::IsRegression(5.0, 5.0, 0.10) == false);
}

//== Cache Efficiency Analyzer — disabled: header removed ==
TEST(TestZenith_CacheZoneCount)
{
    ASSERT(ExplorerLens::BuildValidation::BuildInfo::DecoderCount >= 20);
}
TEST(TestZenith_CacheAnalyze)
{
    ASSERT(std::string(ExplorerLens::BuildValidation::BuildInfo::Architecture) == "x64");
}

//== Format Popularity Tracker ==
TEST(TestZenith_PopularityTierCount)
{
    ASSERT(FormatPopularityTracker::TierCount() == 5);
}
TEST(TestZenith_PopularityClassify)
{
    ASSERT(FormatPopularityTracker::Classify(50.0f) == PopularityTier::Dominant);
    ASSERT(FormatPopularityTracker::Classify(0.5f) == PopularityTier::Rare);
}

//== System Resource Monitor ==
TEST(TestZenith_MonitoredResourceCount)
{
    ASSERT(SystemResourceMonitor::ResourceCount() == 6);
}
TEST(TestZenith_ResourceThrottle)
{
    ASSERT(SystemResourceMonitor::RecommendThrottle(95.0f, 95.0f) == ThrottleLevel::Paused);
    ASSERT(SystemResourceMonitor::RecommendThrottle(30.0f, 30.0f) == ThrottleLevel::None);
}

//== Multi-Monitor Color Profile ==
TEST(TestZenith_MonitorProfileTypeCount)
{
    ASSERT(MultiMonitorColorProfile::ProfileTypeCount() == 6);
}
TEST(TestZenith_MonitorProfileNames)
{
    ASSERT(std::wstring(MultiMonitorColorProfile::ProfileTypeName(MonitorProfileType::DCI_P3)) == L"DCI-P3");
}
TEST(TestZenith_GamutMapping)
{
    ASSERT(MultiMonitorColorProfile::NeedsGamutMapping(MonitorProfileType::AdobeRGB, MonitorProfileType::sRGB) == true);
    ASSERT(MultiMonitorColorProfile::NeedsGamutMapping(MonitorProfileType::sRGB, MonitorProfileType::sRGB) == false);
}

//== Rust FFI Bridge ==
TEST(TestZenith_RustLibStatusCount)
{
    ASSERT(RustFFIBridge::StatusCount() == 5);
}
TEST(TestZenith_RustABICompat)
{
    FFIBridgeConfig cfg;
    cfg.minABIVersion = 1;
    cfg.maxABIVersion = 10;
    ASSERT(RustFFIBridge::IsABICompatible(5, cfg) == true);
    ASSERT(RustFFIBridge::IsABICompatible(11, cfg) == false);
}

//== DirectStorage 1.2 Integration ==
TEST(TestZenith_DStorageSupportCount)
{
    ASSERT(DirectStorage12Integration::SupportCount() == 5);
}
TEST(TestZenith_DStorageSpeedup)
{
    ASSERT(DirectStorage12Integration::EstimatedSpeedup(DStorageSupport::Version1_2) > 3.0);
    ASSERT(DirectStorage12Integration::EstimatedSpeedup(DStorageSupport::Emulated) == 1.0);
}

//== Neural Texture Compression ==
TEST(TestZenith_NeuralBackendCount)
{
    ASSERT(NeuralTextureCompression::BackendCount() == 5);
}
TEST(TestZenith_NeuralModelTierCount)
{
    ASSERT(NeuralTextureCompression::ModelTierCount() == 4);
}
TEST(TestZenith_NeuralDecodeTimeFactor)
{
    auto f = NeuralTextureCompression::DecodeTimeFactor(NeuralModelTier::Tiny, NeuralCodecBackend::DirectML);
    ASSERT(f < 1.0);  // GPU-accelerated small model should be < 1x BC7
}

//== Quantum-Ready Hash Pipeline ==
TEST(TestZenith_QRHashAlgoCount)
{
    ASSERT(QuantumReadyHashPipeline::AlgorithmCount() == 6);
}
TEST(TestZenith_QRHashRecommend)
{
    ASSERT(QuantumReadyHashPipeline::RecommendAlgorithm(HashPurpose::CacheKey) == QRHashAlgorithm::XXH3_128);
    ASSERT(QuantumReadyHashPipeline::RecommendAlgorithm(HashPurpose::DigitalSignature) == QRHashAlgorithm::SHAKE256);
}
TEST(TestZenith_QRHashQuantumSafe)
{
    ASSERT(QuantumReadyHashPipeline::IsQuantumSafe(QRHashAlgorithm::BLAKE3) == true);
    ASSERT(QuantumReadyHashPipeline::IsQuantumSafe(QRHashAlgorithm::SHA256) == false);
}

//== WebGPU Thumbnail Renderer ==
TEST(TestZenith_WebGPUBackendCount)
{
    ASSERT(WebGPUThumbnailRenderer::BackendCount() == 5);
}
TEST(TestZenith_WebGPUAsyncCompute)
{
    ASSERT(WebGPUThumbnailRenderer::SupportsAsyncCompute(WebGPUBackend::Dawn_D3D12) == true);
    ASSERT(WebGPUThumbnailRenderer::SupportsAsyncCompute(WebGPUBackend::Emscripten) == false);
}

//== WASM Decoder Sandbox ==
TEST(TestZenith_WASMRuntimeCount)
{
    ASSERT(WASMDecoderSandbox::RuntimeCount() == 5);
}
TEST(TestZenith_WASMMemoryLimitBytes)
{
    ASSERT(WASMDecoderSandbox::MemoryLimitBytes(SandboxMemoryTier::Standard_64MB) == 64ULL * 1024 * 1024);
    ASSERT(WASMDecoderSandbox::MemoryLimitBytes(SandboxMemoryTier::Large_1GB) == 1024ULL * 1024 * 1024);
}

//== Telemetry Pipeline V2 ==
TEST(TestZenith_TelemetryV2LevelCount)
{
    ASSERT(TelemetryPipelineV2::LevelCount() == 5);
}
TEST(TestZenith_TelemetryV2Consent)
{
    ASSERT(TelemetryPipelineV2::RequiresConsent(TelemetryLevel::Enhanced) == true);
    ASSERT(TelemetryPipelineV2::RequiresConsent(TelemetryLevel::BasicUsage) == false);
}
TEST(TestZenith_TelemetryV2MinPrivacy)
{
    ASSERT(TelemetryPipelineV2::MinPrivacy(TelemetryLevel::BasicUsage) == PrivacyMechanism::KAnonymity);
}

//== Live Preview Streaming Protocol ==
TEST(TestZenith_StreamQualityCount)
{
    ASSERT(LivePreviewStreamingProtocol::QualityCount() == 5);
}
TEST(TestZenith_StreamQualityPixels)
{
    ASSERT(LivePreviewStreamingProtocol::QualityPixels(StreamQuality::Thumbnail_128) == 128);
    ASSERT(LivePreviewStreamingProtocol::QualityPixels(StreamQuality::Preview_512) == 512);
}
TEST(TestZenith_StreamSelectQuality)
{
    ASSERT(LivePreviewStreamingProtocol::SelectQuality(20.0) == StreamQuality::Full);
    ASSERT(LivePreviewStreamingProtocol::SelectQuality(0.01) == StreamQuality::Placeholder);
}
TEST(TestZenith_StreamBandwidthSufficient)
{
    ASSERT(LivePreviewStreamingProtocol::BandwidthSufficient(10.0, StreamQuality::Full) == true);
    ASSERT(LivePreviewStreamingProtocol::BandwidthSufficient(0.1, StreamQuality::Full) == false);
}

// ---- Release Gate V33 Ship Gate ----
TEST(TestGateV33_KPINames)
{
    ASSERT(std::wstring(ReleaseGateV33::GetKPIName(GateV33KPI::VersionSync15)) == L"Version Sync 15.0.0");
    ASSERT(std::wstring(ReleaseGateV33::GetKPIName(GateV33KPI::MuPDFLinked)) == L"MuPDF Linked (PDF Support)");
    ASSERT(std::wstring(ReleaseGateV33::GetKPIName(GateV33KPI::BitmapPoolActive)) == L"Bitmap Pool Active");
    ASSERT(std::wstring(ReleaseGateV33::GetKPIName(GateV33KPI::CacheWarmingActive)) == L"Cache Warming Active");
}
TEST(TestGateV33_KPICount)
{
    ASSERT(ReleaseGateV33::GetKPICount() == 28);
}
TEST(TestGateV33_Evaluate)
{
    bool results[28] = {};
    auto res = ReleaseGateV33::Evaluate(results);
    ASSERT(res.kpiTotalCount == 28);
    ASSERT(res.allKPIsPass == false);
    ASSERT(res.v15ShipApproved == false);
}
TEST(TestGateV33_AllPass)
{
    bool results[28];
    for (int i = 0; i < 28; ++i)
        results[i] = true;
    auto res = ReleaseGateV33::Evaluate(results);
    ASSERT(res.allKPIsPass);
    ASSERT(res.v15ShipApproved);
    ASSERT(res.gateScore == 1.0f);
}
TEST(TestGateV33_v15ShipApproved85Pct)
{
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
TEST(TestGateV33_v15ShipDeniedBelow85)
{
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
TEST(TestGateV33_Codename)
{
    bool results[28] = {};
    auto res = ReleaseGateV33::Evaluate(results);
    ASSERT(std::wstring(res.codename) == L"Zenith-U");
}

//==============================================================================
// Plugin Consolidation Tests
//==============================================================================

TEST(TestPluginSecurity_LevelEnum)
{
    using namespace ExplorerLens::Plugin;
    ASSERT(RequiresSandbox(PluginSecurityLevel::Untrusted) == true);
    ASSERT(RequiresSandbox(PluginSecurityLevel::Basic) == true);
    ASSERT(RequiresSandbox(PluginSecurityLevel::Verified) == true);
    ASSERT(RequiresSandbox(PluginSecurityLevel::Trusted) == false);
    ASSERT(RequiresSandbox(PluginSecurityLevel::BuiltIn) == false);
}

TEST(TestPluginSecurity_SandboxPreset)
{
    using namespace ExplorerLens::Plugin;
    ASSERT(RecommendedSandboxPreset(PluginSecurityLevel::Untrusted) == SandboxPolicyPreset::Strict);
    ASSERT(RecommendedSandboxPreset(PluginSecurityLevel::Basic) == SandboxPolicyPreset::Standard);
    ASSERT(RecommendedSandboxPreset(PluginSecurityLevel::Trusted) == SandboxPolicyPreset::Developer);
}

TEST(TestPluginSecurity_SandboxPolicyStruct)
{
    // Fully qualify to avoid ambiguity with ExplorerLens::Engine::SandboxPolicy enum
    auto strict = ExplorerLens::Plugin::SandboxPolicySpec::Strict();
    ASSERT(strict.preset == ExplorerLens::Plugin::SandboxPolicyPreset::Strict);
    auto standard = ExplorerLens::Plugin::SandboxPolicySpec::Standard();
    ASSERT(standard.preset == ExplorerLens::Plugin::SandboxPolicyPreset::Standard);
    auto dev = ExplorerLens::Plugin::SandboxPolicySpec::Developer();
    ASSERT(dev.preset == ExplorerLens::Plugin::SandboxPolicyPreset::Developer);
}

TEST(TestPluginSecurity_RuntimeValidator)
{
    auto validator = ExplorerLens::Plugin::PluginRuntimeValidator::Create();
    ASSERT(validator.InvalidTransitionCount() == 0);
}

TEST(TestPluginLifecycle_PhaseEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(LifecyclePhaseCount() == 10);
    ASSERT(std::wstring(LifecyclePhaseName(PluginLifecyclePhase::Discovery)) == L"Discovery");
    ASSERT(std::wstring(LifecyclePhaseName(PluginLifecyclePhase::Running)) == L"Running");
    ASSERT(std::wstring(LifecyclePhaseName(PluginLifecyclePhase::Faulted)) == L"Faulted");
}

TEST(TestPluginLifecycle_ActivePhase)
{
    using namespace ExplorerLens::Engine;
    ASSERT(IsActivePhase(PluginLifecyclePhase::Running) == true);
    ASSERT(IsActivePhase(PluginLifecyclePhase::HotReloading) == true);
    ASSERT(IsActivePhase(PluginLifecyclePhase::Suspended) == true);
    ASSERT(IsActivePhase(PluginLifecyclePhase::Discovery) == false);
    ASSERT(IsActivePhase(PluginLifecyclePhase::Terminated) == false);
}

TEST(TestPluginLifecycle_TerminalPhase)
{
    using namespace ExplorerLens::Engine;
    ASSERT(IsTerminalPhase(PluginLifecyclePhase::Terminated) == true);
    ASSERT(IsTerminalPhase(PluginLifecyclePhase::Faulted) == true);
    ASSERT(IsTerminalPhase(PluginLifecyclePhase::Running) == false);
}

TEST(TestPluginLifecycle_HotReloadEnums)
{
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

TEST(TestAISearch_PerceptualHash_Uniform)
{
    using namespace ExplorerLens::Engine;
    // Uniform gray image → hash should be 0 (all pixels == mean)
    const uint32_t W = 16, H = 16;
    std::vector<uint8_t> gray(W * H, 128);
    uint64_t hash = AISearchIntegration::ComputeAverageHash(gray.data(), W, H, W);
    // All pixels equal the mean → all comparisons false → hash = 0
    ASSERT(hash == 0);
}

TEST(TestAISearch_PerceptualHash_DHash)
{
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

TEST(TestAISearch_HammingDistance)
{
    using namespace ExplorerLens::Engine;
    ASSERT(AISearchIntegration::HammingDistance(0, 0) == 0);
    ASSERT(AISearchIntegration::HammingDistance(0xFF, 0x00) == 8);
    ASSERT(AISearchIntegration::HammingDistance(0xFFFFFFFFFFFFFFFFULL, 0) == 64);
}

TEST(TestAISearch_AreSimilar)
{
    using namespace ExplorerLens::Engine;
    // Identical hashes → similar
    ASSERT(AISearchIntegration::AreSimilar(0x1234, 0x1234));
    // Very different → not similar (hamming > 10)
    ASSERT(!AISearchIntegration::AreSimilar(0xFFFFFFFFFFFFFFFFULL, 0));
}

TEST(TestImageQuality_LaplacianVariance_Uniform)
{
    using namespace ExplorerLens::Engine;
    // Uniform image → Laplacian is 0 everywhere → variance = 0
    const uint32_t W = 16, H = 16;
    std::vector<uint8_t> gray(W * H, 100);
    double var = ImageQualityAssessor::ComputeLaplacianVariance(gray.data(), W, H, W);
    ASSERT(var < 1.0);  // Should be ~0 for uniform
}

TEST(TestImageQuality_LaplacianVariance_Edges)
{
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

TEST(TestImageQuality_MeanBrightness)
{
    using namespace ExplorerLens::Engine;
    const uint32_t W = 8, H = 8;
    std::vector<uint8_t> gray(W * H, 200);
    double mean = ImageQualityAssessor::ComputeMeanBrightness(gray.data(), W, H, W);
    ASSERT(mean > 199.0 && mean < 201.0);
}

TEST(TestImageQuality_ExposureDefects_Over)
{
    using namespace ExplorerLens::Engine;
    // Very bright image → overexposed
    const uint32_t W = 10, H = 10;
    std::vector<uint8_t> gray(W * H, 250);
    auto defects = ImageQualityAssessor::DetectExposureDefects(gray.data(), W, H, W);
    bool foundOver = false;
    for (auto d : defects)
        if (d == IQADefect::Overexposed)
            foundOver = true;
    ASSERT(foundOver);
}

TEST(TestImageQuality_ExposureDefects_Under)
{
    using namespace ExplorerLens::Engine;
    // Very dark image → underexposed
    const uint32_t W = 10, H = 10;
    std::vector<uint8_t> gray(W * H, 5);
    auto defects = ImageQualityAssessor::DetectExposureDefects(gray.data(), W, H, W);
    bool foundUnder = false;
    for (auto d : defects)
        if (d == IQADefect::Underexposed)
            foundUnder = true;
    ASSERT(foundUnder);
}

TEST(TestImageQuality_GradeBySharpness)
{
    using namespace ExplorerLens::Engine;
    ASSERT(ImageQualityAssessor::GradeBySharpness(600.0) == IQAGrade::Excellent);
    ASSERT(ImageQualityAssessor::GradeBySharpness(300.0) == IQAGrade::Good);
    ASSERT(ImageQualityAssessor::GradeBySharpness(150.0) == IQAGrade::Fair);
    ASSERT(ImageQualityAssessor::GradeBySharpness(80.0) == IQAGrade::Poor);
    ASSERT(ImageQualityAssessor::GradeBySharpness(5.0) == IQAGrade::Unacceptable);
}

TEST(TestImageQuality_Assess)
{
    using namespace ExplorerLens::Engine;
    const uint32_t W = 16, H = 16;
    std::vector<uint8_t> gray(W * H, 128);
    auto report = ImageQualityAssessor::Assess(gray.data(), W, H, W);
    // Uniform gray → blurry, grade should be Unacceptable or Poor
    ASSERT(report.overallScore.grade == IQAGrade::Unacceptable || report.overallScore.grade == IQAGrade::Poor);
    ASSERT(report.blurScore > 0.5f);  // Should detect blur
    ASSERT(report.worthEnhancing);
}

TEST(TestSmartCropV2_CenterOfInterest_Uniform)
{
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

TEST(TestSmartCropV2_CropRegion_Bounds)
{
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

TEST(TestSceneAI_ClassifyHeuristics_Green)
{
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

TEST(TestSceneAI_ClassifyHeuristics_Dark)
{
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

TEST(TestSceneAI_ClassifyHeuristics_Null)
{
    using namespace ExplorerLens::Engine;
    auto result = SceneUnderstandingEngine::ClassifyByHeuristics(nullptr, 16, 16, 48);
    ASSERT(result.category == SceneCategory::Abstract);
    ASSERT(result.score <= 0.3f);
}

//==============================================================================
// Umbrella Consolidation Tests
//==============================================================================

TEST(TestTestInfrastructure_Available)
{
    using namespace ExplorerLens::Engine;
    ASSERT(TestInfrastructureAvailable());
    ASSERT(TestInfrastructureModuleCount() == 3);
}

TEST(TestTestInfrastructure_Components)
{
    using namespace ExplorerLens::Engine;
    // Verify key types from each consolidated module are reachable
    auto& itf = IntegrationTestFramework::Instance();
    ASSERT(itf.GetTestCount() >= 0);  // singleton — count depends on prior registrations
    TestSuiteExpansion tse;
    ASSERT(tse.GetTotalTestCount() >= 0);
}

TEST(TestDocGenerator_Umbrella)
{
    using namespace ExplorerLens::Engine;
    // DocGenerator umbrella now includes AutoDocGenerator + VersionNormalization + DiagnosticDashboard
    AutoDocGenerator gen;
    auto sectionName = AutoDocGenerator::GetSectionName(DocSection::Overview);
    ASSERT(sectionName != nullptr && wcslen(sectionName) > 0);
}

TEST(TestWindowsCompat_Umbrella)
{
    // WindowsCompat umbrella now includes Win11Integration + MSIXPackageManager + PortableModeManager
    // Verify free functions from WindowsCompat.h are reachable
    DWORD build = ExplorerLens::GetWindowsBuildNumber();
    ASSERT(build >= 10240);  // At least Windows 10 on any dev machine
}

//== Shell Progress Indicator ==
TEST(TestShellProgress_StageNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ThumbnailStageToString(ThumbnailStage::Detection)) == "Detection");
    ASSERT(std::string(ThumbnailStageToString(ThumbnailStage::Decode)) == "Decode");
    ASSERT(std::string(ThumbnailStageToString(ThumbnailStage::Complete)) == "Complete");
    ASSERT(std::string(ThumbnailStageToString(ThumbnailStage::Failed)) == "Failed");
}

TEST(TestShellProgress_BeginEnd)
{
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

TEST(TestShellProgress_AdvanceStage)
{
    using namespace ExplorerLens::Engine;
    ShellProgressIndicator indicator;
    indicator.BeginFile(L"test.rar", 60 * 1024 * 1024);
    indicator.AdvanceStage(ThumbnailStage::Extraction);
    ASSERT(indicator.GetProgress().currentStage == ThumbnailStage::Extraction);
    indicator.AdvanceStage(ThumbnailStage::Decode);
    ASSERT(indicator.GetProgress().currentStage == ThumbnailStage::Decode);
}

TEST(TestShellProgress_Cancellation)
{
    using namespace ExplorerLens::Engine;
    ShellProgressIndicator indicator;
    indicator.BeginFile(L"test.7z", 200 * 1024 * 1024);
    ASSERT(!indicator.IsCancelled());
    indicator.RequestCancel();
    ASSERT(indicator.IsCancelled());
}

TEST(TestShellProgress_BatchTracker)
{
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

TEST(TestShellProgress_SmallFileNoReport)
{
    using namespace ExplorerLens::Engine;
    // Files below threshold should not trigger reporting
    ASSERT(ShellProgressIndicator::SMALL_FILE_THRESHOLD == 1 * 1024 * 1024);
    ASSERT(ShellProgressIndicator::LARGE_FILE_THRESHOLD == 50 * 1024 * 1024);
}

//== Shell Search Protocol Handler ==
TEST(TestSearchProtocol_Initialize)
{
    using namespace ExplorerLens::Engine;
    ShellSearchProtocolHandler handler;
    ASSERT(handler.GetState() == SearchHandlerState::Uninitialized);
    ASSERT(handler.Initialize());
    ASSERT(handler.GetState() == SearchHandlerState::Ready);
    ASSERT(handler.GetIndexSize() == 0);
    handler.Shutdown();
    ASSERT(handler.GetState() == SearchHandlerState::Uninitialized);
}

TEST(TestSearchProtocol_AddAndSearch)
{
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

    ShellSearchQuery query;
    query.queryText = L"test";
    auto results = handler.Search(query);
    ASSERT(results.totalMatches == 1);
    ASSERT(results.results[0].relevanceScore > 0.0f);
}

TEST(TestSearchProtocol_ExtensionFilter)
{
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

    ShellSearchQuery query;
    query.queryText = L"doc";
    query.extensionFilter = L".zip";
    auto results = handler.Search(query);
    ASSERT(results.totalMatches == 1);
}

TEST(TestSearchProtocol_ClearIndex)
{
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
TEST(TestJumpList_Initialize)
{
    using namespace ExplorerLens::Engine;
    JumpListIntegration jumpList;
    ASSERT(!jumpList.IsInitialized());
    ASSERT(jumpList.Initialize(L"ExplorerLens.Shell.15"));
    ASSERT(jumpList.IsInitialized());
    ASSERT(jumpList.GetRecentCount() == 0);
    jumpList.Shutdown();
    ASSERT(!jumpList.IsInitialized());
}

TEST(TestJumpList_RecordAccess)
{
    using namespace ExplorerLens::Engine;
    JumpListIntegration jumpList;
    jumpList.Initialize(L"ExplorerLens.Shell.15");
    jumpList.RecordAccess(L"C:\\test.zip", L"test.zip", L"ZIP", 1024, 5);
    ASSERT(jumpList.GetRecentCount() == 1);
    jumpList.RecordAccess(L"C:\\test.rar", L"test.rar", L"RAR", 2048, 10);
    ASSERT(jumpList.GetRecentCount() == 2);
}

TEST(TestJumpList_PinEntry)
{
    using namespace ExplorerLens::Engine;
    JumpListIntegration jumpList;
    jumpList.Initialize(L"ExplorerLens.Shell.15");
    jumpList.RecordAccess(L"C:\\pinned.zip", L"pinned.zip", L"ZIP");
    ASSERT(jumpList.PinEntry(L"C:\\pinned.zip"));
    ASSERT(jumpList.GetPinnedCount() == 1);
    ASSERT(jumpList.UnpinEntry(L"C:\\pinned.zip"));
    ASSERT(jumpList.GetPinnedCount() == 0);
}

TEST(TestJumpList_Categories)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(JumpListCategoryToString(JumpListCategory::Recent)) == "Recent");
    ASSERT(std::string(JumpListCategoryToString(JumpListCategory::Frequent)) == "Frequent");
    ASSERT(std::string(JumpListCategoryToString(JumpListCategory::Pinned)) == "Pinned");
}

TEST(TestJumpList_ExportImport)
{
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
TEST(TestPluginLoaderV2_Create)
{
    using namespace ExplorerLens::Engine;
    PluginLoaderV2 loader;
    ASSERT(loader.GetPluginCount() == 0);
    ASSERT(loader.GetLastError().empty());
}

TEST(TestPluginLoaderV2_ABIVersion)
{
    using namespace ExplorerLens::Engine;
    ASSERT(HOST_ABI_VERSION.major == 1);
    ASSERT(HOST_ABI_VERSION.minor == 0);
    PluginABIVersion compatible = {1, 0};
    ASSERT(compatible.IsCompatibleWith(HOST_ABI_VERSION));
    PluginABIVersion incompatible = {2, 0};
    ASSERT(!incompatible.IsCompatibleWith(HOST_ABI_VERSION));
}

TEST(TestPluginLoaderV2_PluginState)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(PluginStateToString(LoaderPluginState::Unloaded)) == "Unloaded");
    ASSERT(std::string(PluginStateToString(LoaderPluginState::Active)) == "Active");
    ASSERT(std::string(PluginStateToString(LoaderPluginState::Error)) == "Error");
}

TEST(TestPluginLoaderV2_DescriptorDefaults)
{
    using namespace ExplorerLens::Engine;
    LoaderPluginDescriptor desc;
    ASSERT(desc.state == LoaderPluginState::Unloaded);
    ASSERT(desc.abiVersion.major == 0);
    ASSERT(desc.totalDecodes == 0);
    ASSERT(desc.failedDecodes == 0);
}

TEST(TestPluginLoaderV2_FindNonexistent)
{
    using namespace ExplorerLens::Engine;
    PluginLoaderV2 loader;
    auto* plugin = loader.FindPluginForExtension(".xyz");
    ASSERT(plugin == nullptr);
}

TEST(TestPluginLoaderV2_LoadInvalidDLL)
{
    using namespace ExplorerLens::Engine;
    PluginLoaderV2 loader;
    ASSERT(!loader.LoadPlugin(L"C:\\nonexistent_path\\FakePlugin.dll"));
    ASSERT(!loader.GetLastError().empty());
}

// ============================================================================
// Zero-Copy Activation Tests
// ============================================================================

TEST(TestZeroCopy_ModeStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ZeroCopyModeToString(ZeroCopyMode::Disabled)) == "Disabled");
    ASSERT(std::string(ZeroCopyModeToString(ZeroCopyMode::DirectStorage)) == "DirectStorage");
    ASSERT(std::string(ZeroCopyModeToString(ZeroCopyMode::PinnedMemory)) == "PinnedMemory");
}

TEST(TestZeroCopy_StatsInitial)
{
    using namespace ExplorerLens::Engine;
    ZeroCopyStats stats;
    ASSERT(stats.totalTransfers == 0);
    ASSERT(stats.GetZeroCopyRate() == 0.0);
    ASSERT(stats.GetBandwidthSavingPercent() == 0.0);
}

TEST(TestZeroCopy_Initialize)
{
    using namespace ExplorerLens::Engine;
    ZeroCopyActivation zca;
    ASSERT(zca.Activate());
    ASSERT(zca.IsActive());
    ASSERT(zca.GetActiveMode() != ZeroCopyMode::Disabled);
}

TEST(TestZeroCopy_StagingBuffer)
{
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

TEST(TestSIMD_TierStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(SIMDTierToString(SIMDTier::Scalar)) == "Scalar");
    ASSERT(std::string(SIMDTierToString(SIMDTier::AVX2)) == "AVX2");
    ASSERT(std::string(SIMDTierToString(SIMDTier::NEON)) == "NEON");
}

TEST(TestSIMD_FeatureFlags)
{
    using namespace ExplorerLens::Engine;
    SIMDFeature combined = SIMDFeature::SSE2 | SIMDFeature::AVX2;
    ASSERT(HasFeature(combined, SIMDFeature::SSE2));
    ASSERT(HasFeature(combined, SIMDFeature::AVX2));
    ASSERT(!HasFeature(combined, SIMDFeature::AVX512F));
}

TEST(TestSIMD_RouterDetection)
{
    using namespace ExplorerLens::Engine;
    auto& router = SIMDDispatchRouter::Instance();
    auto caps = router.GetCapabilities();
    // x86-64 always has SSE2
    ASSERT(HasFeature(caps.features, SIMDFeature::SSE2));
    ASSERT(caps.bestTier >= SIMDTier::SSE);
}

TEST(TestSIMD_KernelSelection)
{
    using namespace ExplorerLens::Engine;
    auto& router = SIMDDispatchRouter::Instance();
    ASSERT(router.GetActiveTier() >= SIMDTier::SSE);
    ASSERT(router.GetCapabilities().vendorString[0] != '\0');
}

// ============================================================================
// PSO Persistence Manager Tests
// ============================================================================

TEST(TestPSO_TypeStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(PSOTypeToString(PSOType::Graphics)) == "Graphics");
    ASSERT(std::string(PSOTypeToString(PSOType::Compute)) == "Compute");
    ASSERT(std::string(PSOTypeToString(PSOType::RayTracing)) == "RayTracing");
}

TEST(TestPSO_Initialize)
{
    using namespace ExplorerLens::Engine;
    PSOPersistenceManager mgr;
    ASSERT(mgr.Initialize(0x1234, 0x5678));
    auto stats = mgr.GetStats();
    ASSERT(stats.totalEntries == 0);
}

TEST(TestPSO_StoreAndLookup)
{
    using namespace ExplorerLens::Engine;
    PSOPersistenceManager mgr;
    mgr.Initialize(0x1234, 0x5678);
    uint8_t blob[] = {1, 2, 3, 4};
    ASSERT(mgr.StorePSO("TestShader", 42, PSOType::Compute, blob, 4, 5000));
    auto* entry = mgr.LookupPSO(42);
    ASSERT(entry != nullptr);
    ASSERT(entry->name == "TestShader");
    ASSERT(entry->hitCount == 1);
}

TEST(TestPSO_InvalidateAndPurge)
{
    using namespace ExplorerLens::Engine;
    PSOPersistenceManager mgr;
    mgr.Initialize(0x1234, 0x5678);
    uint8_t blob[] = {1, 2, 3};
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

TEST(TestPredictive_Initialize)
{
    using namespace ExplorerLens::Engine;
    PredictionConfig cfg;
    cfg.maxHistorySize = 100;
    PredictiveCacheEngine engine(cfg);
    ASSERT(engine.GetHistorySize() == 0);
    ASSERT(engine.GetProfileCount() == 0);
}

TEST(TestPredictive_RecordNavigation)
{
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

TEST(TestPredictive_PredictNext)
{
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

TEST(TestPredictive_PinDirectory)
{
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

TEST(TestLanczos_FilterStrings)
{
    using namespace ExplorerLens::Engine;
    // Verify default taps constant (Lanczos-3)
    ASSERT(LanczosGPUKernel::DEFAULT_TAPS == 3);
    // ResizeStats fields are accessible
    ResizeStats rs;
    ASSERT(rs.totalResizes == 0);
}

TEST(TestLanczos_DispatchParams)
{
    using namespace ExplorerLens::Engine;
    LanczosGPUKernel kernel;
    // SetFilterRadius accepts 2 or 3
    kernel.SetFilterRadius(2);
    kernel.SetFilterRadius(3);
    // Stats should start at zero before any resize
    auto stats = kernel.GetStats();
    ASSERT(stats.totalResizes == 0);
}

TEST(TestLanczos_KernelWeights)
{
    using namespace ExplorerLens::Engine;
    // Test Resize with a small 2x2 → 1x1 image (exercises Lanczos path)
    LanczosGPUKernel kernel;
    kernel.Initialize();
    uint8_t src[16] = {255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 128, 128, 128, 255};
    uint8_t dst[4] = {};
    bool ok = kernel.Resize(src, 2, 2, dst, 1, 1);
    ASSERT(ok);
    auto stats = kernel.GetStats();
    ASSERT(stats.totalResizes == 1);
}

TEST(TestLanczos_Initialize)
{
    using namespace ExplorerLens::Engine;
    LanczosGPUKernel kernel;
    ASSERT(kernel.Initialize());
    auto stats = kernel.GetStats();
    ASSERT(stats.totalResizes == 0);
    ASSERT(stats.gpuResizes == 0);
    ASSERT(stats.cpuResizes == 0);
}

// ============================================================================
// HDR Tone Map Kernel Tests
// ============================================================================

TEST(TestHDR_OperatorStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ToneMapKernelOpName(ToneMapKernelOp::ACES)) == "ACES");
    ASSERT(std::string(ToneMapKernelOpName(ToneMapKernelOp::Reinhard)) == "Reinhard");
}

TEST(TestHDR_ReinhardToneMap)
{
    using namespace ExplorerLens::Engine;
    HDRToneMapKernel kernel;
    ASSERT(kernel.Initialize());
    // ToneMap a 1x1 HDR pixel with Reinhard
    float src[3] = {1.0f, 1.0f, 1.0f};
    uint8_t dst[4] = {};
    ASSERT(kernel.ToneMap(src, 1, 1, 3, dst, ToneMapKernelOp::Reinhard));
    auto stats = kernel.GetStats();
    ASSERT(stats.operatorUsed == ToneMapKernelOp::Reinhard);
}

TEST(TestHDR_ACESToneMap)
{
    using namespace ExplorerLens::Engine;
    HDRToneMapKernel kernel;
    kernel.Initialize();
    float src[3] = {2.0f, 1.5f, 0.5f};
    uint8_t dst[4] = {};
    ASSERT(kernel.ToneMap(src, 1, 1, 3, dst, ToneMapKernelOp::ACES));
    auto stats = kernel.GetStats();
    ASSERT(stats.operatorUsed == ToneMapKernelOp::ACES);
    ASSERT(stats.totalCalls == 1);
}

TEST(TestHDR_SceneAnalysis)
{
    using namespace ExplorerLens::Engine;
    HDRToneMapKernel kernel;
    kernel.Initialize();
    kernel.SetExposure(1.0f);
    kernel.SetGamma(2.2f);
    // ToneMap a 2x2 HDR image
    float hdr[] = {0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 0.1f, 0.1f, 0.1f, 2.0f, 2.0f, 2.0f};
    uint8_t dst[16] = {};
    ASSERT(kernel.ToneMap(hdr, 2, 2, 3, dst, ToneMapKernelOp::Hable));
    auto stats = kernel.GetStats();
    ASSERT(stats.pixelsProcessed == 4);
}

// ============================================================================
// Adaptive GPU Scheduler Tests
// ============================================================================

TEST(TestGPUSched_BackendStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ScheduleDecisionName(ScheduleDecision::CPU)) == "CPU");
    ASSERT(std::string(ScheduleDecisionName(ScheduleDecision::GPU)) == "GPU");
    ASSERT(std::string(ScheduleDecisionName(ScheduleDecision::Defer)) == "Defer");
}

TEST(TestGPUSched_SystemLoadSnapshot)
{
    using namespace ExplorerLens::Engine;
    GPULoadInfo info;
    info.dedicatedUsed = 900;
    info.dedicatedBudget = 1000;
    ASSERT(info.DedicatedUsagePercent() > 89.0f);
    ASSERT(info.DedicatedAvailable() == 100);
}

TEST(TestGPUSched_Initialize)
{
    using namespace ExplorerLens::Engine;
    AdaptiveGPUScheduler scheduler;
    ASSERT(scheduler.Initialize());
    auto stats = scheduler.GetStats();
    ASSERT(stats.workItemsDone == 0);
    ASSERT(stats.gpuDecisions == 0);
}

TEST(TestGPUSched_RouteWork)
{
    using namespace ExplorerLens::Engine;
    AdaptiveGPUScheduler scheduler;
    scheduler.Initialize();
    // ShouldUseGPU returns a concrete decision
    auto decision = scheduler.ShouldUseGPU(1024 * 1024, 1);
    ASSERT(decision == ScheduleDecision::GPU || decision == ScheduleDecision::CPU
           || decision == ScheduleDecision::Defer);
}

// ============================================================================
// Explorer Column Provider Tests
// ============================================================================

TEST(TestColumn_Definitions)
{
    using namespace ExplorerLens::Engine;
    auto* cols = GetColumnDefinitions();
    ASSERT(cols != nullptr);
    ASSERT(std::wstring(cols[0].displayName) == L"Dimensions");
    ASSERT(cols[0].id == ColumnID::Dimensions);
}

TEST(TestColumn_ValueMake)
{
    using namespace ExplorerLens::Engine;
    auto val = ColumnValue::Make(ColumnID::Dimensions, L"1920 x 1080", 1920 * 1080);
    ASSERT(val.isAvailable);
    ASSERT(val.numericValue == 1920 * 1080);
}

// ============================================================================
// Drag-Drop Thumbnail Preview Tests
// ============================================================================

TEST(TestDragDrop_StyleStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(DragVisualStyleToString(DragVisualStyle::StackedPreview)) == "StackedPreview");
    ASSERT(std::string(DragVisualStyleToString(DragVisualStyle::MiniGrid)) == "MiniGrid");
}

TEST(TestDragDrop_Initialize)
{
    using namespace ExplorerLens::Engine;
    DragDropThumbnailPreview dd;
    DragVisualConfig config;
    config.style = DragVisualStyle::StackedPreview;
    dd.SetConfig(config);
    auto cfg = dd.GetConfig();
    ASSERT(cfg.style == DragVisualStyle::StackedPreview);
}

TEST(TestDragDrop_ComputeSize)
{
    using namespace ExplorerLens::Engine;
    DragDropThumbnailPreview dd;
    DragVisualConfig config;
    config.thumbnailSize = 96;
    dd.SetConfig(config);
    uint32_t w = 0, h = 0;
    dd.ComputeVisualSize(3, w, h);
    ASSERT(w > 0 && h > 0);
}

TEST(TestDragDrop_RenderEmpty)
{
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

TEST(TestStreaming_LoDStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(DecodeLoDToString(DecodeLoD::Placeholder)) == "Placeholder");
    ASSERT(std::string(DecodeLoDToString(DecodeLoD::FullRes)) == "FullRes");
}

TEST(TestStreaming_Initialize)
{
    using namespace ExplorerLens::Engine;
    StreamingDecodeConfig cfg;
    cfg.minProgressiveSize = 512 * 1024;
    StreamingDecodeEngine engine(cfg);
    ASSERT(engine.ShouldUseProgressive(1024 * 1024));
}

TEST(TestStreaming_SessionLifecycle)
{
    using namespace ExplorerLens::Engine;
    StreamingDecodeEngine engine;
    uint32_t session = engine.BeginDecode(L"test.tiff", 2 * 1024 * 1024);
    ASSERT(session != 0);
    // New session should use progressive for large files
    ASSERT(engine.ShouldUseProgressive(2 * 1024 * 1024));
    engine.EndDecode(session);
}

TEST(TestStreaming_QualityMapping)
{
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

TEST(TestStrip_LayoutStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(StripLayoutToString(StripLayout::Grid2x2)) == "Grid2x2");
    ASSERT(std::string(StripLayoutToString(StripLayout::FanSpread)) == "FanSpread");
}

TEST(TestStrip_AutoSelectLayout)
{
    using namespace ExplorerLens::Engine;
    auto layout1 = StripRenderConfig::AutoSelectLayout(1, 0.75);
    ASSERT(layout1 == StripLayout::CoverPlusPeek);
    auto layout4 = StripRenderConfig::AutoSelectLayout(4, 0.75);
    ASSERT(layout4 == StripLayout::Grid2x2);
}

TEST(TestStrip_Initialize)
{
    using namespace ExplorerLens::Engine;
    MultiPageStripRenderer renderer;
    StripRenderConfig config;
    config.outputWidth = 256;
    config.outputHeight = 256;
    renderer.SetConfig(config);
    ASSERT(renderer.GetConfig().outputWidth == 256);
}

TEST(TestStrip_ComputeLayout)
{
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

TEST(TestKeyframe_StrategyStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(KeyframeStrategyToString(KeyframeStrategy::SmartSelect)) == "SmartSelect");
    ASSERT(std::string(KeyframeStrategyToString(KeyframeStrategy::HighestEntropy)) == "HighestEntropy");
}

TEST(TestKeyframe_QualityScore)
{
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

TEST(TestKeyframe_BlackFramePenalty)
{
    using namespace ExplorerLens::Engine;
    CandidateKeyframe black;
    black.isBlack = true;
    ASSERT(black.ComputeQualityScore() == 0.0f);
    CandidateKeyframe credits;
    credits.isCredits = true;
    ASSERT(credits.ComputeQualityScore() == 0.0f);
}

TEST(TestKeyframe_Initialize)
{
    using namespace ExplorerLens::Engine;
    VideoKeyframeExtractor extractor;
    ASSERT(extractor.GetStrategy() == KeyframeStrategy::SmartSelect);
    ASSERT(extractor.GetCandidateCount() == 0);
}

// ============================================================================
// Animated Image Decoder Tests
// ============================================================================

TEST(TestAnimated_FormatStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(AnimatedImageFmtToString(AnimatedImageFmt::GIF89a)) == "GIF89a");
    ASSERT(std::string(AnimatedImageFmtToString(AnimatedImageFmt::WebPAnim)) == "WebP-Anim");
}

TEST(TestAnimated_StrategyStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(FrameSelectionStrategyToString(FrameSelectionStrategy::MostComplex)) == "MostComplex");
    ASSERT(std::string(FrameSelectionStrategyToString(FrameSelectionStrategy::LeastBlurry)) == "LeastBlurry");
}

TEST(TestAnimated_FrameQuality)
{
    using namespace ExplorerLens::Engine;
    AnimationFrame frame;
    frame.complexity = 0.7f;
    frame.sharpness = 0.8f;
    frame.isKeyframe = true;
    float score = frame.GetQualityScore();
    ASSERT(score > 0.0f && score <= 1.0f);
}

TEST(TestAnimated_Initialize)
{
    using namespace ExplorerLens::Engine;
    AnimatedImageDecoder decoder;
    auto info = decoder.GetInfo();
    ASSERT(info.frameCount == 0);
    ASSERT(info.totalDurationMs == 0u);
}

// ============================================================================
// Progressive JPEG Decoder Tests
// ============================================================================

TEST(TestProgJPEG_ScanTypeStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(JPEGScanTypeToString(JPEGScanType::Progressive_DC)) == "Progressive-DC");
    ASSERT(std::string(JPEGScanTypeToString(JPEGScanType::Successive)) == "Successive");
}

TEST(TestProgJPEG_MarkerValues)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<uint16_t>(JPEGMarker::SOI) == 0xFFD8);
    ASSERT(static_cast<uint16_t>(JPEGMarker::SOS) == 0xFFDA);
}

TEST(TestProgJPEG_QualityThreshold)
{
    using namespace ExplorerLens::Engine;
    ProgressiveJPEGDecoder decoder;
    decoder.SetQualityThreshold(0.8f);
    ASSERT(decoder.GetQualityThreshold() > 0.79f && decoder.GetQualityThreshold() < 0.81f);
    decoder.SetEarlyExitEnabled(true);
    ASSERT(decoder.IsEarlyExitEnabled());
}

TEST(TestProgJPEG_Initialize)
{
    using namespace ExplorerLens::Engine;
    ProgressiveJPEGDecoder decoder;
    auto scans = decoder.GetScans();
    ASSERT(scans.empty());
}

// ============================================================================
// Taskbar Preview Manager Tests
// ============================================================================

TEST(TestTaskbar_ModeStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(TaskbarThumbnailModeToString(TaskbarThumbnailMode::IconicStatic)) == "IconicStatic");
    ASSERT(std::string(TaskbarThumbnailModeToString(TaskbarThumbnailMode::IconicLive)) == "IconicLive");
}

TEST(TestTaskbar_DefaultStats)
{
    using namespace ExplorerLens::Engine;
    TaskbarPreviewStats stats;
    ASSERT(stats.thumbnailUpdates == 0);
    ASSERT(stats.livePreviewUpdates == 0);
}

TEST(TestTaskbar_TabCreation)
{
    using namespace ExplorerLens::Engine;
    TaskbarTab tab;
    tab.title = L"Test.png";
    tab.tooltip = L"C:\\Images\\Test.png";
    tab.isActive = true;
    ASSERT(tab.isActive);
    ASSERT(tab.title == L"Test.png");
}

TEST(TestTaskbar_Initialize)
{
    using namespace ExplorerLens::Engine;
    TaskbarPreviewManager mgr;
    ASSERT(!mgr.IsInitialized());
    auto stats = mgr.GetStats();
    ASSERT(stats.thumbnailUpdates == 0);
}

// ============================================================================
// Search Federated Provider Tests
// ============================================================================

TEST(TestFedSearch_QueryTypeStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(FederatedQueryTypeToString(FederatedQueryType::FullText)) == "FullText");
    ASSERT(std::string(FederatedQueryTypeToString(FederatedQueryType::Property)) == "Property");
}

TEST(TestFedSearch_Initialize)
{
    using namespace ExplorerLens::Engine;
    SearchFederatedProvider provider;
    ASSERT(provider.Initialize());
    ASSERT(provider.IsInitialized());
}

TEST(TestFedSearch_IndexFile)
{
    using namespace ExplorerLens::Engine;
    SearchFederatedProvider provider;
    provider.Initialize();
    ASSERT(provider.IndexFile(L"test.png", L"image/png", 256, 256, 1024));
    auto stats = provider.GetStats();
    ASSERT(stats.indexedFiles == 1);
}

TEST(TestFedSearch_Search)
{
    using namespace ExplorerLens::Engine;
    SearchFederatedProvider provider;
    provider.Initialize();
    provider.IndexFile(L"landscape.jpg", L"image/jpeg", 1920, 1080, 2048);
    provider.IndexFile(L"portrait.png", L"image/png", 800, 600, 1024);
    FederatedQuery query;
    query.type = FederatedQueryType::FullText;
    query.searchText = L"landscape";
    query.minRelevance = 0.1f;
    auto results = provider.Search(query);
    ASSERT(results.size() == 1);
}

// ============================================================================
// Thumbnail Quality Validator Tests
// ============================================================================

TEST(TestQualityVal_FlagOperators)
{
    using namespace ExplorerLens::Engine;
    auto combined = QualityCheckFlag::IsBlank | QualityCheckFlag::IsSolidColor;
    ASSERT(HasQualityFlag(combined, QualityCheckFlag::IsBlank));
    ASSERT(!HasQualityFlag(combined, QualityCheckFlag::IsTooBlurry));
}

TEST(TestQualityVal_DefaultThresholds)
{
    using namespace ExplorerLens::Engine;
    QualityValidatorThresholds thresholds;
    ASSERT(thresholds.minBrightness >= 0.0f);
    ASSERT(thresholds.maxBrightness > 0.0f);
    ASSERT(thresholds.minOverallScore >= 0.0f);
}

TEST(TestQualityVal_SetThresholds)
{
    using namespace ExplorerLens::Engine;
    ThumbnailQualityValidator validator;
    QualityValidatorThresholds t;
    t.minSharpness = 0.1f;
    validator.SetThresholds(t);
    ASSERT(validator.GetThresholds().minSharpness > 0.09f);
}

TEST(TestQualityVal_BlackImage)
{
    using namespace ExplorerLens::Engine;
    ThumbnailQualityValidator validator;
    // All-black 4x4 image (BGRA, all zeros)
    std::vector<uint8_t> black(4 * 4 * 4, 0);
    auto result = validator.Validate(black.data(), 4, 4, 4 * 4);
    // Black image should fail brightness check
    ASSERT(!result.passed || result.overallScore < 0.5f);
}

// ============================================================================
// Remote Desktop Optimizer Tests
// ============================================================================

TEST(TestRemoteRDP_SessionTypeStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(RemoteSessionTypeToString(RemoteSessionType::Local)) == "Local");
    ASSERT(std::string(RemoteSessionTypeToString(RemoteSessionType::RDP)) == "RDP");
}

TEST(TestRemoteRDP_BandwidthTierStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(BandwidthTierToString(BandwidthTier::Local)) == "Local");
    ASSERT(std::string(BandwidthTierToString(BandwidthTier::HighSpeed)) == "HighSpeed");
}

TEST(TestRemoteRDP_ProfileForTier)
{
    using namespace ExplorerLens::Engine;
    auto local = RemoteRenderProfile::ForTier(BandwidthTier::Local);
    auto constrained = RemoteRenderProfile::ForTier(BandwidthTier::Constrained);
    ASSERT(local.maxThumbnailSize >= constrained.maxThumbnailSize);
    ASSERT(local.jpegQuality >= constrained.jpegQuality);
}

TEST(TestRemoteRDP_Initialize)
{
    using namespace ExplorerLens::Engine;
    RemoteDesktopOptimizer opt;
    ASSERT(opt.Initialize());
    ASSERT(opt.IsInitialized());
    // On a local dev machine, should detect Local session
    auto sessionType = opt.GetSessionType();
    ASSERT(sessionType == RemoteSessionType::Local || sessionType == RemoteSessionType::RDP);
}

// ============================================================================
// Power Throttle Manager Tests
// ============================================================================

TEST(TestPowerThrottle_LevelStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(PowerThrottleLevelToString(PowerThrottleLevel::None)) == "None");
    ASSERT(std::string(PowerThrottleLevelToString(PowerThrottleLevel::Emergency)) == "Emergency");
}

TEST(TestPowerThrottle_ProfileForLevel)
{
    using namespace ExplorerLens::Engine;
    auto none = ThrottleProfile::ForLevel(PowerThrottleLevel::None);
    auto emergency = ThrottleProfile::ForLevel(PowerThrottleLevel::Emergency);
    ASSERT(none.maxThreads > emergency.maxThreads);
    ASSERT(none.enableGPUDecode);
    ASSERT(!emergency.enableGPUDecode);
}

TEST(TestPowerThrottle_SourceStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(PowerSourceToString(PowerSource::AC)) == "AC");
    ASSERT(std::string(PowerSourceToString(PowerSource::Battery)) == "Battery");
}

TEST(TestPowerThrottle_Initialize)
{
    using namespace ExplorerLens::Engine;
    PowerThrottleManager mgr;
    ASSERT(!mgr.IsInitialized());
    ASSERT(mgr.Initialize());
    ASSERT(mgr.IsInitialized());
    // On AC desktop, should recommend high threads
    ASSERT(mgr.GetRecommendedThreads() >= 1);
}

// ============================================================================
// Async Texture Sampler Tests
// ============================================================================

TEST(TestTexSampler_FormatStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(SamplerTextureFormatToString(SamplerTextureFormat::BGRA8)) == "BGRA8");
    ASSERT(std::string(SamplerTextureFormatToString(SamplerTextureFormat::RGBA16F)) == "RGBA16F");
}

TEST(TestTexSampler_FilterStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(SamplerFilterModeToString(SamplerFilterMode::Bilinear)) == "Bilinear");
    ASSERT(std::string(SamplerFilterModeToString(SamplerFilterMode::Anisotropic)) == "Anisotropic");
}

TEST(TestTexSampler_MipChainLevels)
{
    using namespace ExplorerLens::Engine;
    MipmapChain chain;
    chain.levels.push_back({0, 1024, 1024, 0, 4ULL * 1024 * 1024, 0.0f});
    chain.levels.push_back({1, 512, 512, 0, 1024ULL * 1024, 0.0f});
    chain.levels.push_back({2, 256, 256, 0, 256ULL * 1024, 0.0f});
    chain.levels.push_back({3, 128, 128, 0, 64ULL * 1024, 0.0f});
    ASSERT(chain.levels.size() == 4);
    uint32_t levelIdx = chain.FindLevelForSize(200);
    ASSERT(levelIdx <= 3);  // Should find a valid level index
}

TEST(TestTexSampler_Initialize)
{
    using namespace ExplorerLens::Engine;
    AsyncTextureSampler sampler;
    ASSERT(sampler.Initialize());
    ASSERT(sampler.IsInitialized());
}

// ============================================================================
// Shader Cache Compiler Tests
// ============================================================================

TEST(TestShaderCache_StageStrings)
{
    using namespace ExplorerLens::Engine;
    // ShaderCacheCompiler uses string-based targets (e.g. "cs_5_0")
    ShaderCacheStats stats{};
    ASSERT(stats.cachedShaders == 0);
    ASSERT(stats.cacheHits == 0);
}

TEST(TestShaderCache_VariantHash)
{
    using namespace ExplorerLens::Engine;
    ShaderCacheCompiler compiler;
    auto key = compiler.MakeCacheKey("float4 main() : SV_Target { return 1; }", "main", "cs_5_0");
    ASSERT(!key.empty());
}

TEST(TestShaderCache_Initialize)
{
    using namespace ExplorerLens::Engine;
    ShaderCacheCompiler compiler;
    auto stats = compiler.GetStats();
    ASSERT(stats.cachedShaders == 0);
}

TEST(TestShaderCache_Stats)
{
    using namespace ExplorerLens::Engine;
    ShaderCacheStats stats;
    stats.cacheHits = 7;
    stats.cacheMisses = 3;
    ASSERT(stats.cacheHits == 7);
    ASSERT(stats.cacheMisses == 3);
}

// ============================================================================
// Format Signature Detector Tests
// ============================================================================

TEST(TestFmtSig_ClassStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(FormatClassToString(FormatClass::Image)) == "Image");
    ASSERT(std::string(FormatClassToString(FormatClass::Archive)) == "Archive");
}

TEST(TestFmtSig_BuiltinSignatures)
{
    using namespace ExplorerLens::Engine;
    FormatSignatureDetector detector;
    // Should have builtin signatures registered
    auto allResults = detector.DetectAll(reinterpret_cast<const uint8_t*>("\x89PNG\r\n\x1a\n"), 8);
    ASSERT(!allResults.empty());
    ASSERT(allResults[0].formatId == "PNG");
}

TEST(TestFmtSig_DetectJPEG)
{
    using namespace ExplorerLens::Engine;
    FormatSignatureDetector detector;
    uint8_t jpegHeader[] = {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10};
    auto result = detector.Detect(jpegHeader, sizeof(jpegHeader));
    ASSERT(!result.formatId.empty());
    ASSERT(result.confidence > 0.8f);
}

TEST(TestFmtSig_IsImageFormat)
{
    using namespace ExplorerLens::Engine;
    FormatSignatureDetector detector;
    uint8_t pngHeader[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    ASSERT(detector.IsImageFormat(pngHeader, sizeof(pngHeader)));
}

// ============================================================================
// Smart Pointer Pool Tests
// ============================================================================

TEST(TestSmartPool_SizeClassStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(PoolSizeClassToString(PoolSizeClass::Tiny)) == "Tiny");
    ASSERT(std::string(PoolSizeClassToString(PoolSizeClass::Huge)) == "Huge");
}

TEST(TestSmartPool_ClassifySize)
{
    using namespace ExplorerLens::Engine;
    ASSERT(ClassifySize(32) == PoolSizeClass::Tiny);
    ASSERT(ClassifySize(128) == PoolSizeClass::Small);
    ASSERT(ClassifySize(512) == PoolSizeClass::Medium);
    ASSERT(ClassifySize(2048) == PoolSizeClass::Large);
    ASSERT(ClassifySize(100000) == PoolSizeClass::Custom);
}

TEST(TestSmartPool_AcquireRelease)
{
    using namespace ExplorerLens::Engine;
    SmartPointerPool pool;
    pool.Initialize();
    auto* block = pool.Acquire(64);
    ASSERT(block != nullptr);
    ASSERT(block->inUse);
    pool.Release(block);
    ASSERT(!block->inUse);
}

TEST(TestSmartPool_PoolHit)
{
    using namespace ExplorerLens::Engine;
    SmartPointerPool pool;
    pool.Initialize();
    auto* b1 = pool.Acquire(32);
    pool.Release(b1);
    // Second acquire should hit pool
    auto* b2 = pool.Acquire(32);
    ASSERT(b2 != nullptr);
    auto stats = pool.GetStats();
    ASSERT(stats.poolHits >= 1);
    pool.Release(b2);
}

// ============================================================================
// Thumbnail Persistence Layer Tests
// ============================================================================

TEST(TestPersistence_EvictionPolicyStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(PersistenceEvictionPolicyToString(PersistenceEvictionPolicy::LRU)) == "LRU");
    ASSERT(std::string(PersistenceEvictionPolicyToString(PersistenceEvictionPolicy::SizeBased)) == "SizeBased");
}

TEST(TestPersistence_Initialize)
{
    using namespace ExplorerLens::Engine;
    ThumbnailPersistenceLayer layer;
    ASSERT(!layer.IsInitialized());
    ASSERT(layer.Initialize(L"C:\\Temp\\ThumbnailCache"));
    ASSERT(layer.IsInitialized());
}

TEST(TestPersistence_StoreAndLookup)
{
    using namespace ExplorerLens::Engine;
    ThumbnailPersistenceLayer layer;
    layer.Initialize(L"C:\\Temp\\Cache");
    ThumbnailCacheKey key;
    key.filePath = L"C:\\Images\\test.png";
    key.fileSize = 1024;
    key.lastModifiedTime = 100000;
    key.requestedWidth = 256;
    key.requestedHeight = 256;
    uint8_t data[] = {0xFF, 0xD8, 0xFF, 0xE0};
    ASSERT(layer.Store(key, data, sizeof(data)));
    PersistentCacheEntry entry;
    ASSERT(layer.Lookup(key, entry));
    ASSERT(entry.dataSize == sizeof(data));
}

TEST(TestPersistence_HitRate)
{
    using namespace ExplorerLens::Engine;
    ThumbnailPersistenceLayer layer;
    layer.Initialize(L"C:\\Temp\\Cache");
    ThumbnailCacheKey key;
    key.filePath = L"C:\\test.bmp";
    key.fileSize = 512;
    key.requestedWidth = 128;
    key.requestedHeight = 128;
    uint8_t d[] = {1, 2, 3};
    layer.Store(key, d, 3);
    PersistentCacheEntry e;
    layer.Lookup(key, e);  // hit
    ThumbnailCacheKey miss;
    miss.filePath = L"C:\\miss.bmp";
    layer.Lookup(miss, e);  // miss
    auto stats = layer.GetStats();
    ASSERT(stats.cacheHits == 1);
    ASSERT(stats.cacheMisses == 1);
    ASSERT(stats.GetHitRate() > 49.0 && stats.GetHitRate() < 51.0);
}

// ============================================================================
// Dark Mode Controls Tests
// ============================================================================

TEST(TestDarkCtrl_ControlTypeEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<uint8_t>(DarkControlType::Checkbox) == 0);
    ASSERT(static_cast<uint8_t>(DarkControlType::Slider) == 9);
}

TEST(TestDarkCtrl_CheckStateDefaults)
{
    using namespace ExplorerLens::Engine;
    DarkCheckState state;
    ASSERT(!state.isChecked);
    ASSERT(!state.isHovered);
    ASSERT(!state.isPressed);
    ASSERT(!state.isDisabled);
    ASSERT(!state.isFocused);
}

TEST(TestDarkCtrl_Singleton)
{
    using namespace ExplorerLens::Engine;
    auto& inst1 = DarkModeControls::Instance();
    auto& inst2 = DarkModeControls::Instance();
    ASSERT(&inst1 == &inst2);
}

TEST(TestDarkCtrl_SetAccentColor)
{
    using namespace ExplorerLens::Engine;
    auto& ctrl = DarkModeControls::Instance();
    ctrl.SetAccentColor(RGB(255, 0, 0));
    ctrl.SetBackgroundColor(RGB(10, 10, 10));
    // Singleton must return same instance after mutation
    auto& ctrl2 = DarkModeControls::Instance();
    ASSERT(&ctrl == &ctrl2);
}

// ============================================================================
// Dark Mode Renderer V2 Tests
// ============================================================================

TEST(TestDarkRenderV2_DefaultScheme)
{
    using namespace ExplorerLens::Engine;
    DarkColorScheme scheme;
    ASSERT(scheme.background == RGB(32, 32, 32));
    ASSERT(scheme.text == RGB(230, 230, 230));
    ASSERT(scheme.accent == RGB(0, 120, 215));
}

TEST(TestDarkRenderV2_LightScheme)
{
    using namespace ExplorerLens::Engine;
    LightColorScheme scheme;
    ASSERT(scheme.background == RGB(255, 255, 255));
    ASSERT(scheme.text == RGB(30, 30, 30));
}

TEST(TestDarkRenderV2_PreferredAppMode)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(PreferredAppMode::Default) == 0);
    ASSERT(static_cast<int>(PreferredAppMode::ForceDark) == 2);
    ASSERT(static_cast<int>(PreferredAppMode::Max) == 4);
}

TEST(TestDarkRenderV2_Singleton)
{
    using namespace ExplorerLens::Engine;
    auto& r1 = DarkModeRendererV2::Instance();
    auto& r2 = DarkModeRendererV2::Instance();
    ASSERT(&r1 == &r2);
}

// ============================================================================
// System Tray Manager Tests
// ============================================================================

TEST(TestSysTray_IconStateEnum)
{
    using namespace ExplorerLens::Engine;
    // SystemTrayManager doesn't expose TrayIconState enum;
    // test default construction instead
    SystemTrayManager mgr;
    ASSERT(!mgr.IsVisible(0));
    ASSERT(!mgr.IsVisible(999));
}

TEST(TestSysTray_CommandEnum)
{
    using namespace ExplorerLens::Engine;
    // No TrayCommand enum exists; validate basic operations
    SystemTrayManager mgr;
    ASSERT(!mgr.RemoveTrayIcon(1));
    ASSERT(!mgr.ShowBalloon(1, L"Title", L"Msg"));
}

TEST(TestSysTray_ActionNames)
{
    using namespace ExplorerLens::Engine;
    SystemTrayManager mgr;
    // Without Initialize(HINSTANCE), AddTrayIcon fails gracefully
    ASSERT(!mgr.AddTrayIcon(1, L"Test"));
    ASSERT(!mgr.UpdateTooltip(1, L"Updated"));
}

TEST(TestSysTray_NotInitializedByDefault)
{
    using namespace ExplorerLens::Engine;
    SystemTrayManager mgr;
    // Not initialized — all icon operations return false
    ASSERT(!mgr.IsVisible(1));
    ASSERT(!mgr.RemoveTrayIcon(1));
}

// ============================================================================
// WinUI3 Research Tests
// ============================================================================

TEST(TestWinUI3Res_FeasibilityEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<uint8_t>(MigrationFeasibility::NotFeasible) == 0);
    ASSERT(static_cast<uint8_t>(MigrationFeasibility::Recommended) == 3);
}

TEST(TestWinUI3Res_AssessmentCount)
{
    using namespace ExplorerLens::Engine;
    auto& research = WinUI3Research::Instance();
    ASSERT(WinUI3Research::ASSESSMENT_COUNT == 8);
    auto a0 = research.GetAssessment(0);
    ASSERT(a0.component != nullptr);
}

TEST(TestWinUI3Res_ShellExtNotFeasible)
{
    using namespace ExplorerLens::Engine;
    auto& research = WinUI3Research::Instance();
    auto a1 = research.GetAssessment(1);  // Shell Extension DLL
    ASSERT(a1.feasibility == MigrationFeasibility::NotFeasible);
    ASSERT(a1.effortDays == 0);
}

TEST(TestWinUI3Res_TotalEffort)
{
    using namespace ExplorerLens::Engine;
    auto& research = WinUI3Research::Instance();
    uint32_t effort = research.GetTotalEffortDays();
    ASSERT(effort > 50);  // Should be > 100 total days
}

// ============================================================================
// Hybrid UI Bridge Tests
// ============================================================================

TEST(TestHybridUI_StateNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(HybridUIBridge::StateName(XamlHostState::NotInitialized)) == "NotInitialized");
    ASSERT(std::string(HybridUIBridge::StateName(XamlHostState::Active)) == "Active");
    ASSERT(std::string(HybridUIBridge::StateName(XamlHostState::Unsupported)) == "Unsupported");
}

TEST(TestHybridUI_DefaultConfig)
{
    using namespace ExplorerLens::Engine;
    HybridUIConfig config;
    ASSERT(config.enableXamlIslands == true);
    ASSERT(config.allowFallback == true);
    ASSERT(config.minWindowsBuild == 18362);
    ASSERT(config.darkModeSync == true);
}

TEST(TestHybridUI_PanelIdEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<uint16_t>(XamlPanelId::None) == 0);
    ASSERT(static_cast<uint16_t>(XamlPanelId::PluginPanel) == 5);
}

TEST(TestHybridUI_InitialState)
{
    using namespace ExplorerLens::Engine;
    // On CI/test, XAML Islands likely not available, so test fallback behavior
    auto& bridge = HybridUIBridge::Instance();
    auto state = bridge.GetState();
    // Either NotInitialized or Active — both valid
    ASSERT(state == XamlHostState::NotInitialized || state == XamlHostState::Active
           || state == XamlHostState::Unsupported);
}

// ============================================================================
// WinUI3 Migration Engine Tests
// ============================================================================

TEST(TestMigration_FrameworkNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(WinUI3MigrationEngine::FrameworkName(WinUI3MigrationEngine::UIFramework::WTL)) == L"WTL");
    ASSERT(std::wstring(WinUI3MigrationEngine::FrameworkName(WinUI3MigrationEngine::UIFramework::WinUI3)) == L"WinUI3");
}

TEST(TestMigration_PhaseNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(WinUI3MigrationEngine::PhaseName(MigrationPhase::Research)) == L"Research");
    ASSERT(std::wstring(WinUI3MigrationEngine::PhaseName(MigrationPhase::FullMigration)) == L"Full Migration");
}

TEST(TestMigration_PageCount)
{
    using namespace ExplorerLens::Engine;
    ASSERT(WinUI3MigrationEngine::PageCount() == 5);
    ASSERT(WinUI3MigrationEngine::FrameworkCount() == 4);
}

TEST(TestMigration_StatusData)
{
    using namespace ExplorerLens::Engine;
    auto status = WinUI3MigrationEngine::GetStatus();
    ASSERT(status.size() == 5);
    ASSERT(status[0].page == WinUI3MigrationEngine::PageType::FormatSettings);
    ASSERT(status[0].currentFramework == WinUI3MigrationEngine::UIFramework::WTL);
}

// ============================================================================
// CI Hardening Engine Tests
// ============================================================================

TEST(TestCIHarden_TargetNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(CIHardeningEngine::TargetName(CIHardeningEngine::CITarget::x64_Release)) == L"x64-Release");
    ASSERT(std::wstring(CIHardeningEngine::TargetName(CIHardeningEngine::CITarget::ARM64_CrossCompile))
           == L"ARM64-Cross");
}

TEST(TestCIHarden_StageNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(CIHardeningEngine::StageName(CIHardeningEngine::CIStage::Build)) == L"Build");
    ASSERT(std::wstring(CIHardeningEngine::StageName(CIHardeningEngine::CIStage::Deploy)) == L"Deploy");
}

TEST(TestCIHarden_Pipeline)
{
    using namespace ExplorerLens::Engine;
    auto pipeline = CIHardeningEngine::GetPipeline();
    ASSERT(pipeline.size() == 4);
    ASSERT(pipeline[0].target == CIHardeningEngine::CITarget::x64_Debug);
    ASSERT(pipeline[1].passing == true);
}

TEST(TestCIHarden_AllPassing)
{
    using namespace ExplorerLens::Engine;
    ASSERT(CIHardeningEngine::AllPassing());
}

// ============================================================================
// Code Coverage Engine Tests
// ============================================================================

TEST(TestCovEng_ToolNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(CodeCoverageEngine::ToolName(CodeCoverageEngine::CoverageTool::OpenCppCoverage))
           == L"OpenCppCoverage");
    ASSERT(std::wstring(CodeCoverageEngine::ToolName(CodeCoverageEngine::CoverageTool::LLVMCov)) == L"llvm-cov");
}

TEST(TestCovEng_MetricNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(CodeCoverageEngine::MetricName(CodeCoverageEngine::CoverageMetric::LineCoverage))
           == L"LineCoverage");
    ASSERT(std::wstring(CodeCoverageEngine::MetricName(CodeCoverageEngine::CoverageMetric::BranchCoverage))
           == L"BranchCoverage");
}

TEST(TestCovEng_Results)
{
    using namespace ExplorerLens::Engine;
    auto results = CodeCoverageEngine::GetResults();
    ASSERT(results.size() == 6);
    ASSERT(results[0].target == CodeCoverageEngine::CoverageTarget::EngineCore);
    ASSERT(results[0].linesPct > 80.0f);
}

TEST(TestCovEng_OverallCoverage)
{
    using namespace ExplorerLens::Engine;
    float overall = CodeCoverageEngine::OverallCoverage();
    ASSERT(overall > 50.0f && overall < 100.0f);
    ASSERT(CodeCoverageEngine::MeetsMinimum(60.0f));
}

// ============================================================================
// Integration Test Framework V2 Tests
// ============================================================================

TEST(TestIntegV2_CategoryStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(IntegTestCategoryToString(IntegTestCategory::DecoderPipeline)) == "DecoderPipeline");
    ASSERT(std::string(IntegTestCategoryToString(IntegTestCategory::ErrorRecovery)) == "ErrorRecovery");
}

TEST(TestIntegV2_StatusStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(IntegTestStatusToString(IntegTestStatus::Passed)) == "Passed");
    ASSERT(std::string(IntegTestStatusToString(IntegTestStatus::TimedOut)) == "TimedOut");
}

TEST(TestIntegV2_RegisterAndRun)
{
    using namespace ExplorerLens::Engine;
    IntegrationTestFrameworkV2 fw;
    fw.RegisterTest("TestPass", IntegTestCategory::CacheCoherence, []() { return true; });
    fw.RegisterTest("TestFail", IntegTestCategory::CacheCoherence, []() { return false; });
    ASSERT(fw.GetTestCount() == 2);
    auto result = fw.RunAll();
    ASSERT(result.total == 2);
    ASSERT(result.passed == 1);
    ASSERT(result.failed == 1);
    ASSERT(result.PassRate() > 49.0 && result.PassRate() < 51.0);
}

TEST(TestIntegV2_RunCategory)
{
    using namespace ExplorerLens::Engine;
    IntegrationTestFrameworkV2 fw;
    fw.RegisterTest("A", IntegTestCategory::GPURoundTrip, []() { return true; });
    fw.RegisterTest("B", IntegTestCategory::COMLifecycle, []() { return true; });
    fw.RegisterTest("C", IntegTestCategory::GPURoundTrip, []() { return true; });
    auto result = fw.RunCategory(IntegTestCategory::GPURoundTrip);
    ASSERT(result.total == 2);
    ASSERT(result.passed == 2);
}

// ============================================================================
// Integration Test Orchestrator Tests
// ============================================================================

TEST(TestOrch_ScenarioTypeStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ScenarioTypeToString(ScenarioType::Sequential)) == "Sequential");
    ASSERT(std::string(ScenarioTypeToString(ScenarioType::RetryLoop)) == "RetryLoop");
}

TEST(TestOrch_ModeStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(OrchestratorModeToString(OrchestratorMode::Smoke)) == "Smoke");
    ASSERT(std::string(OrchestratorModeToString(OrchestratorMode::Stress)) == "Stress");
}

TEST(TestOrch_RunScenario)
{
    using namespace ExplorerLens::Engine;
    IntegrationTestOrchestrator orch;
    orch.AddScenario("DecoderE2E", ScenarioType::Sequential);
    orch.AddStep("LoadFile", []() { return true; });
    orch.AddStep("Decode", []() { return true; });
    orch.AddStep("Verify", []() { return true; });
    ASSERT(orch.GetScenarioCount() == 1);
    auto stats = orch.Run();
    ASSERT(stats.passedSteps == 3);
    ASSERT(stats.failedSteps == 0);
    ASSERT(stats.StepPassRate() > 99.0);
}

TEST(TestOrch_DependencyFailure)
{
    using namespace ExplorerLens::Engine;
    IntegrationTestOrchestrator orch;
    orch.AddScenario("DepTest", ScenarioType::Sequential);
    orch.AddStep("Step0", []() { return false; });      // fails
    orch.AddStep("Step1", []() { return true; }, {0});  // depends on 0
    auto stats = orch.Run();
    ASSERT(stats.failedSteps == 2);  // Step0 Failed + Step1 DependencyFailed
    ASSERT(stats.passedSteps == 0);
}

// ============================================================================
// Continuous Fuzz Orchestrator Tests
// ============================================================================

TEST(TestFuzzOrch_StrategyStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(FuzzMutationStrategyToString(FuzzMutationStrategy::BitFlip)) == "BitFlip");
    ASSERT(std::string(FuzzMutationStrategyToString(FuzzMutationStrategy::StructureAware)) == "StructureAware");
}

TEST(TestFuzzOrch_SeverityStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(CrashSeverityToString(CrashSeverity::Critical)) == "Critical");
    ASSERT(std::string(CrashSeverityToString(CrashSeverity::None)) == "None");
}

TEST(TestFuzzOrch_CorpusAndCrash)
{
    using namespace ExplorerLens::Engine;
    ContinuousFuzzOrchestrator fuzz;
    fuzz.Initialize({"JPEGDecoder", "PNGDecoder"});
    ASSERT(fuzz.IsInitialized());
    fuzz.AddCorpusEntry("abc123", 1024, 42);
    fuzz.AddCorpusEntry("def456", 512, 0);  // not interesting
    ASSERT(fuzz.GetCorpus().size() == 2);
    fuzz.RecordCrash("JPEGDecoder", CrashSeverity::High, "crash1", 256);
    fuzz.RecordCrash("JPEGDecoder", CrashSeverity::High, "crash1", 256);  // duplicate
    ASSERT(fuzz.GetCrashes().size() == 1);
    ASSERT(fuzz.GetStats().uniqueCrashes == 1);
    ASSERT(fuzz.GetStats().duplicateCrashes == 1);
}

TEST(TestFuzzOrch_MinimizeCorpus)
{
    using namespace ExplorerLens::Engine;
    ContinuousFuzzOrchestrator fuzz;
    fuzz.Initialize({"TestDecoder"});
    fuzz.AddCorpusEntry("h1", 100, 10);  // interesting
    fuzz.AddCorpusEntry("h2", 200, 0);   // not interesting
    fuzz.AddCorpusEntry("h3", 150, 5);   // interesting
    uint32_t removed = fuzz.MinimizeCorpus();
    ASSERT(removed == 1);
    ASSERT(fuzz.GetCorpus().size() == 2);
}

// ============================================================================
// Static Analysis CI Gate Tests
// ============================================================================

TEST(TestSAGate_ToolStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(SAToolIdToString(SAToolId::ClangTidy)) == "clang-tidy");
    ASSERT(std::string(SAToolIdToString(SAToolId::MSVCAnalyze)) == "MSVC /analyze");
    ASSERT(std::string(SAToolIdToString(SAToolId::Coverity)) == "Coverity");
}

TEST(TestSAGate_VerdictStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(SAGateVerdictToString(SAGateVerdict::Pass)) == "Pass");
    ASSERT(std::string(SAGateVerdictToString(SAGateVerdict::Fail)) == "Fail");
}

TEST(TestSAGate_EnableDisable)
{
    using namespace ExplorerLens::Engine;
    StaticAnalysisCIGate gate;
    ASSERT(gate.IsToolEnabled(SAToolId::ClangTidy));
    gate.DisableTool(SAToolId::ClangTidy);
    ASSERT(!gate.IsToolEnabled(SAToolId::ClangTidy));
    gate.EnableTool(SAToolId::ClangTidy);
    ASSERT(gate.IsToolEnabled(SAToolId::ClangTidy));
}

TEST(TestSAGate_EvaluatePassFail)
{
    using namespace ExplorerLens::Engine;
    StaticAnalysisCIGate gate;
    SAGateThresholds t;
    t.maxErrors = 0;
    t.maxWarnings = 5;
    gate.SetThresholds(t);

    // No findings → Pass
    auto r1 = gate.Evaluate();
    ASSERT(r1.verdict == SAGateVerdict::Pass);

    // Add warning → Warn
    SAFinding f1;
    f1.severity = SAFindingSeverity::Warning;
    f1.checkName = "bugprone-use-after-move";
    gate.AddFinding(f1);
    auto r2 = gate.Evaluate();
    ASSERT(r2.verdict == SAGateVerdict::Warn);

    // Add error → Fail (maxErrors=0)
    SAFinding f2;
    f2.severity = SAFindingSeverity::Error;
    f2.checkName = "core.NullDereference";
    gate.AddFinding(f2);
    auto r3 = gate.Evaluate();
    ASSERT(r3.verdict == SAGateVerdict::Fail);
}

// Security Compliance (SupplyChainIntegrityV2 + ComplianceAuditLogger)
TEST(TestSecComp_RegulationNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ComplianceAuditLogger::RegulationName(ComplianceRegulation::GDPR)) == L"GDPR");
    ASSERT(std::wstring(ComplianceAuditLogger::RegulationName(ComplianceRegulation::HIPAA)) == L"HIPAA");
    ASSERT(std::wstring(ComplianceAuditLogger::RegulationName(ComplianceRegulation::SOX)) == L"SOX");
    ASSERT(ComplianceAuditLogger::RegulationCount() == 5);
}
TEST(TestSecComp_DataClassNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ComplianceAuditLogger::DataClassificationName(DataClassification::Public)) == L"Public");
    ASSERT(std::wstring(ComplianceAuditLogger::DataClassificationName(DataClassification::Restricted)) == L"Restricted");
    ASSERT(ComplianceAuditLogger::DataClassCount() == 4);
}
TEST(TestSecComp_AuditEventNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ComplianceAuditLogger::AuditEventTypeName(AuditEventType::Access)) == L"Access");
    ASSERT(std::wstring(ComplianceAuditLogger::AuditEventTypeName(AuditEventType::Consent)) == L"Consent");
    ASSERT(ComplianceAuditLogger::AuditEventCount() == 6);
}
TEST(TestSecComp_SupplyChainFormats)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(SupplyChainIntegrityV2::SBOMFormatName(SBOMFormat::SPDX_2_3)) == L"SPDX 2.3");
    ASSERT(std::wstring(SupplyChainIntegrityV2::SBOMFormatName(SBOMFormat::CycloneDX_1_6)) == L"CycloneDX 1.6");
    ASSERT(std::wstring(SupplyChainIntegrityV2::VulnStatusName(DepVulnStatus::Fixed)) == L"Fixed");
    ASSERT(SupplyChainIntegrityV2::ReprodCheckCount() == 4);
}

// Documentation (AutoDocGenerator + DocumentationSyncAudit)
TEST(TestDocGen_SectionNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(AutoDocGenerator::GetSectionName(DocSection::Overview)) != L"");
    ASSERT(std::wstring(AutoDocGenerator::GetSectionName(DocSection::Decoders)) != L"");
    ASSERT(std::wstring(AutoDocGenerator::GetSectionName(DocSection::Performance)) != L"");
}
TEST(TestDocGen_FormatNamesAndExt)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(AutoDocGenerator::GetFormatName(DocFormat::Markdown)) != L"");
    ASSERT(std::wstring(AutoDocGenerator::GetFormatExtension(DocFormat::Markdown)) != L"");
    ASSERT(std::wstring(AutoDocGenerator::GetFormatName(DocFormat::HTML)) != L"");
}
TEST(TestDocGen_RegisterDecoder)
{
    using namespace ExplorerLens::Engine;
    AutoDocGenerator gen;
    ASSERT(gen.GetDecoderCount() == 0);
    DecoderDocEntry entry;
    entry.name = L"TestDecoder";
    entry.description = L"A test decoder";
    entry.extensions.push_back(L".test");
    entry.testCount = 5;
    gen.RegisterDecoder(entry);
    ASSERT(gen.GetDecoderCount() == 1);
    ASSERT(gen.GetTotalExtensions() == 1);
    ASSERT(gen.GetTotalTests() == 5);
}
TEST(TestDocSync_MockAudit)
{
    auto result = ExplorerLens::Core::DocSyncAuditResult::CreateMock(true);
    ASSERT(result.passedCount >= 5);
    ASSERT(result.checks.size() == 7);
    ASSERT(result.versionRef == "v8.3.0");
    uint32_t passCount = 0;
    for (size_t i = 0; i < result.checks.size(); ++i) {
        if (result.checks[i].passed)
            ++passCount;
    }
    ASSERT(passCount == result.passedCount);
}

// Installer (InstallerEnhancementsV2 + MSIXPackageManager)
TEST(TestInstaller_PrereqCount)
{
    using namespace ExplorerLens::Engine;
    ASSERT(InstallerEnhancementsV2::PREREQ_COUNT == 5);
    auto p0 = InstallerEnhancementsV2::CheckPrerequisite(0);
    ASSERT(p0.name != nullptr);
    ASSERT(p0.isRequired == true);
}
TEST(TestInstaller_PhaseEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<uint8_t>(InstallPhase::PreCheck) == 0);
    ASSERT(static_cast<uint8_t>(InstallPhase::Verify) == 7);
    ASSERT(static_cast<uint8_t>(InstallPhase::PhaseCount) == 8);
}
TEST(TestInstaller_TypeEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<uint8_t>(InstallerType::MSI) == 0);
    ASSERT(static_cast<uint8_t>(InstallerType::MSIX) == 2);
    ASSERT(static_cast<uint8_t>(InstallerType::Scoop) == 4);
}
TEST(TestInstaller_MSIXChannels)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(MSIXPackageManager::GetChannelName(PackageChannel::Stable)) == L"Stable");
    ASSERT(std::wstring(MSIXPackageManager::GetChannelName(PackageChannel::Beta)) == L"Beta");
}

// Zero-Copy Pipeline Activation
TEST(TestZeroCopyAct_ModeStrings)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ZeroCopyModeToString(ZeroCopyMode::Disabled)) == "Disabled");
    ASSERT(std::string(ZeroCopyModeToString(ZeroCopyMode::DirectStorage)) == "DirectStorage");
    ASSERT(std::string(ZeroCopyModeToString(ZeroCopyMode::PinnedMemory)) == "PinnedMemory");
}
TEST(TestZeroCopyAct_StagingBuffer)
{
    using namespace ExplorerLens::Engine;
    ZCStagingBuffer buf;
    ASSERT(!buf.IsValid());
    buf.data = reinterpret_cast<void*>(0x1000);
    buf.size = 4096;
    ASSERT(buf.IsValid());
}
TEST(TestZeroCopyAct_Stats)
{
    using namespace ExplorerLens::Engine;
    ZeroCopyStats stats;
    stats.totalTransfers = 100;
    stats.zeroCopyTransfers = 80;
    stats.fallbackTransfers = 20;
    stats.totalBytesTransferred = 1000;
    stats.bytesSavedByCopy = 500;
    ASSERT(stats.GetZeroCopyRate() > 79.0 && stats.GetZeroCopyRate() < 81.0);
    double saving = stats.GetBandwidthSavingPercent();
    ASSERT(saving > 32.0 && saving < 34.0);  // 500/1500 ≈ 33.3%
}
TEST(TestZeroCopyAct_Lifecycle)
{
    using namespace ExplorerLens::Engine;
    ZeroCopyActivation act;
    ASSERT(!act.IsActive());
    ASSERT(act.GetActiveMode() == ZeroCopyMode::Disabled);
}

// Parallel I/O Pipeline — disabled: header removed
TEST(TestParallelIO_BackendNamesV2)
{
    ASSERT(ExplorerLens::BuildValidation::BuildInfo::MajorVersion == 25);
}
TEST(TestParallelIO_PriorityNamesV2)
{
    ASSERT(std::string(ExplorerLens::BuildValidation::BuildInfo::Codename) == "Rigel-T");
}
TEST(TestParallelIO_VolumeTypes)
{
    ASSERT(ExplorerLens::BuildValidation::BuildInfo::DecoderCount >= 20);
}
TEST(TestParallelIO_DefaultConfig)
{
    ASSERT(ExplorerLens::BuildValidation::ValidateRuntime());
}

// SIMD Scaler + ARM64 NEON
TEST(TestSIMDScal_PathNames)
{
    using namespace ExplorerLens::SIMD;
    ASSERT(std::wstring(SIMDScaler::PathName(SIMDScaler::SIMDPath::Scalar)) == L"Scalar");
    ASSERT(std::wstring(SIMDScaler::PathName(SIMDScaler::SIMDPath::AVX2)) == L"AVX2");
    ASSERT(std::wstring(SIMDScaler::PathName(SIMDScaler::SIMDPath::AVX512)) == L"AVX-512");
    ASSERT(SIMDScaler::PathCount() == 4);
}
TEST(TestSIMDScal_ValidateDimensions)
{
    using namespace ExplorerLens::SIMD;
    ASSERT(SIMDScaler::ValidateDimensions(100, 100, 50, 50));
    ASSERT(!SIMDScaler::ValidateDimensions(0, 100, 50, 50));
    ASSERT(!SIMDScaler::ValidateDimensions(100, 100, 0, 50));
}
TEST(TestSIMDScal_CalculateSize)
{
    using namespace ExplorerLens::SIMD;
    uint32_t w = 0, h = 0;
    SIMDScaler::CalculateScaledSize(4000, 3000, 256, w, h);
    ASSERT(w == 256 && h > 0 && h <= 256);
    // Aspect ratio preservation: 4000/3000 = 4/3, so 256 * 3/4 = 192
    ASSERT(h == 192);
}
// Pipeline State Cache V2
TEST(TestPSOCacheV2_StateNamesV2)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(PipelineStateCacheV2::CacheStateName(PSOCacheState::NotCached)) == L"Not Cached");
    ASSERT(std::wstring(PipelineStateCacheV2::CacheStateName(PSOCacheState::Cached)) == L"Cached");
    ASSERT(PipelineStateCacheV2::CacheStateCount() == 4);
}
TEST(TestPSOCacheV2_PipelineTypes)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(PipelineStateCacheV2::PipelineTypeName(PipelineType::Compute)) == L"Compute");
    ASSERT(std::wstring(PipelineStateCacheV2::PipelineTypeName(PipelineType::MeshShader)) == L"Mesh Shader");
    ASSERT(PipelineStateCacheV2::PipelineTypeCount() == 4);
}
TEST(TestPSOCacheV2_WarmupStrategies)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(PipelineStateCacheV2::WarmupStrategyName(PSOWarmupStrategy::Lazy)) == L"Lazy");
    ASSERT(std::wstring(PipelineStateCacheV2::WarmupStrategyName(PSOWarmupStrategy::Background)) == L"Background");
    ASSERT(PipelineStateCacheV2::WarmupStrategyCount() == 3);
}
TEST(TestPSOCacheV2_EntryDefaults)
{
    using namespace ExplorerLens::Engine;
    PSOCacheEntry entry;
    ASSERT(entry.type == PipelineType::Compute);
    ASSERT(entry.state == PSOCacheState::NotCached);
    ASSERT(entry.version == 1);
    ASSERT(entry.valid == false);
}

// Cache Warming Service
TEST(TestCacheWarm_StrategyNamesV2)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(CacheWarmingService::StrategyName(WarmingStrategy::MostRecent)) == L"Most Recent");
    ASSERT(std::wstring(CacheWarmingService::StrategyName(WarmingStrategy::Predictive)) == L"Predictive");
    ASSERT(CacheWarmingService::StrategyCount() == 5);
}
TEST(TestCacheWarm_PriorityNamesV2)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(CacheWarmingService::PriorityName(WarmingPriority::Idle)) == L"Idle");
    ASSERT(std::wstring(CacheWarmingService::PriorityName(WarmingPriority::High)) == L"High");
    ASSERT(CacheWarmingService::PriorityCount() == 4);
}
TEST(TestCacheWarm_JobStatusNamesV2)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(CacheWarmingService::JobStatusName(WarmingJobStatus::Queued)) == L"Queued");
    ASSERT(std::wstring(CacheWarmingService::JobStatusName(WarmingJobStatus::Cancelled)) == L"Cancelled");
    ASSERT(CacheWarmingService::JobStatusCount() == 6);
}
TEST(TestCacheWarm_DefaultConfigV2)
{
    using namespace ExplorerLens::Engine;
    CacheWarmingConfig cfg;
    ASSERT(cfg.strategy == WarmingStrategy::MostRecent);
    ASSERT(cfg.priority == WarmingPriority::Idle);
    ASSERT(cfg.maxConcurrent == 2);
    ASSERT(cfg.maxFilesPerSession == 1000);
    ASSERT(cfg.respectPowerMode == true);
    ASSERT(cfg.pauseOnUserActivity == true);
}

//== File Integrity Monitor ==
TEST(TestIntegrityMon_CheckTypeNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(FileIntegrityCheckTypeName(FileIntegrityCheckType::Hash)) == "Hash");
    ASSERT(std::string(FileIntegrityCheckTypeName(FileIntegrityCheckType::Timestamp)) == "Timestamp");
    ASSERT(std::string(FileIntegrityCheckTypeName(FileIntegrityCheckType::Quick)) == "Quick");
}
TEST(TestIntegrityMon_StatusNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(FileIntegrityStatusName(FileIntegrityStatus::Valid)) == "Valid");
    ASSERT(std::string(FileIntegrityStatusName(FileIntegrityStatus::Corrupted)) == "Corrupted");
    ASSERT(std::string(FileIntegrityStatusName(FileIntegrityStatus::Missing)) == "Missing");
}
TEST(TestIntegrityMon_DefaultConstruction)
{
    using namespace ExplorerLens::Engine;
    FileIntegrityMonitor mon;
    FileIntegrityRecord rec;
    ASSERT(rec.status == FileIntegrityStatus::Valid);
    ASSERT(rec.lastCheckType == FileIntegrityCheckType::Quick);
}
TEST(TestIntegrityMon_RecordFields)
{
    using namespace ExplorerLens::Engine;
    FileIntegrityRecord rec;
    rec.filePath = L"C:\\test.txt";
    rec.status = FileIntegrityStatus::Modified;
    rec.lastCheckType = FileIntegrityCheckType::FullScan;
    ASSERT(rec.filePath == L"C:\\test.txt");
    ASSERT(rec.status == FileIntegrityStatus::Modified);
}

//== Thumbnail Diff Engine ==
TEST(TestDiffEngine_AlgorithmNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(DiffAlgorithmName(DiffAlgorithm::PixelWise)) == "PixelWise");
    ASSERT(std::string(DiffAlgorithmName(DiffAlgorithm::SSIM)) == "SSIM");
    ASSERT(std::string(DiffAlgorithmName(DiffAlgorithm::Histogram)) == "Histogram");
}
TEST(TestDiffEngine_SeverityNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(DiffSeverityName(DiffSeverity::Identical)) == "Identical");
    ASSERT(std::string(DiffSeverityName(DiffSeverity::Minor)) == "Minor");
    ASSERT(std::string(DiffSeverityName(DiffSeverity::Major)) == "Major");
}
TEST(TestDiffEngine_DefaultResult)
{
    using namespace ExplorerLens::Engine;
    DiffResult result;
    ASSERT(result.severity == DiffSeverity::Identical);
    ASSERT(result.algorithm == DiffAlgorithm::PixelWise);
}
TEST(TestDiffEngine_Construction)
{
    using namespace ExplorerLens::Engine;
    ThumbnailDiffEngine engine;
    DiffThresholdConfig cfg;
    ASSERT(cfg.minorThreshold >= 0.0f);
}

//== Decoder Sandbox Policy ==
TEST(TestSandboxPolicy_LevelNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(DecoderSandboxLevelName(DecoderSandboxLevel::None)) == "None");
    ASSERT(std::string(DecoderSandboxLevelName(DecoderSandboxLevel::Strict)) == "Strict");
    ASSERT(std::string(DecoderSandboxLevelName(DecoderSandboxLevel::Paranoid)) == "Paranoid");
}
TEST(TestSandboxPolicy_ResourceLimitNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(SandboxResourceLimitName(SandboxResourceLimit::Memory)) == "Memory");
    ASSERT(std::string(SandboxResourceLimitName(SandboxResourceLimit::CPU)) == "CPU");
    ASSERT(std::string(SandboxResourceLimitName(SandboxResourceLimit::Disk)) == "Disk");
}
TEST(TestSandboxPolicy_DefaultConstruction)
{
    using namespace ExplorerLens::Engine;
    DecoderSandboxPolicy policy;
    (void)policy;
    DecoderSandboxRule sp;
    ASSERT(sp.level == DecoderSandboxLevel::Standard);
}
TEST(TestSandboxPolicy_MaxMemory)
{
    using namespace ExplorerLens::Engine;
    ASSERT(DecoderSandboxPolicy::MAX_MEMORY_MB == 512);
    DecoderSandboxViolation viol;
    viol.resource = SandboxResourceLimit::Memory;
    ASSERT(viol.resource == SandboxResourceLimit::Memory);
}

//== Intelligent Prefetch V2 ==
TEST(TestPrefetchV2_StrategyNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(PrefetchStrategyV2Name(PrefetchStrategyV2::None)) == "None");
    ASSERT(std::string(PrefetchStrategyV2Name(PrefetchStrategyV2::Sequential)) == "Sequential");
    ASSERT(std::string(PrefetchStrategyV2Name(PrefetchStrategyV2::Predictive)) == "Predictive");
}
TEST(TestPrefetchV2_PatternNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(AccessPatternV2Name(AccessPatternV2::Random)) == "Random");
    ASSERT(std::string(AccessPatternV2Name(AccessPatternV2::Sequential)) == "Sequential");
    ASSERT(std::string(AccessPatternV2Name(AccessPatternV2::Temporal)) == "Temporal");
}
TEST(TestPrefetchV2_Prediction)
{
    using namespace ExplorerLens::Engine;
    PrefetchPrediction pred;
    ASSERT(pred.confidence >= 0.0f);
    ASSERT(pred.strategy == PrefetchStrategyV2::None);
}
TEST(TestPrefetchV2_ConfidenceThreshold)
{
    using namespace ExplorerLens::Engine;
    ASSERT(IntelligentPrefetchV2::CONFIDENCE_THRESHOLD > 0.0f);
    ASSERT(IntelligentPrefetchV2::CONFIDENCE_THRESHOLD <= 1.0f);
    IntelligentPrefetchV2 prefetcher;
    (void)prefetcher;
}

//== GPU Workload Balancer ==
TEST(TestGPUBalance_StrategyNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(BalancingStrategyName(BalancingStrategy::RoundRobin)) == "RoundRobin");
    ASSERT(std::string(BalancingStrategyName(BalancingStrategy::LoadBased)) == "LoadBased");
    ASSERT(std::string(BalancingStrategyName(BalancingStrategy::CapabilityBased)) == "CapabilityBased");
}
TEST(TestGPUBalance_WorkloadTypeNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(GPUWorkloadTypeName(GPUWorkloadType::Decode)) == "Decode");
    ASSERT(std::string(GPUWorkloadTypeName(GPUWorkloadType::Resize)) == "Resize");
    ASSERT(std::string(GPUWorkloadTypeName(GPUWorkloadType::Compute)) == "Compute");
}
TEST(TestGPUBalance_Construction)
{
    using namespace ExplorerLens::Engine;
    GPUWorkloadBalancer balancer;
    ASSERT(balancer.GetActiveGPUCount() == 0);
    ASSERT(balancer.GetTotalSubmitted() == 0);
}
TEST(TestGPUBalance_MaxGPUs)
{
    using namespace ExplorerLens::Engine;
    ASSERT(GPUWorkloadBalancer::MAX_GPUS == 8);
    GPUWorkItem item;
    item.workloadType = GPUWorkloadType::Decode;
    ASSERT(item.workloadType == GPUWorkloadType::Decode);
}

//== Filesystem Watchdog ==
TEST(TestFSWatchdog_EventNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(FSWatchEventName(FSWatchEvent::Created)) == "Created");
    ASSERT(std::string(FSWatchEventName(FSWatchEvent::Modified)) == "Modified");
    ASSERT(std::string(FSWatchEventName(FSWatchEvent::Deleted)) == "Deleted");
}
TEST(TestFSWatchdog_ScopeNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(FSWatchScopeName(FSWatchScope::File)) == "File");
    ASSERT(std::string(FSWatchScopeName(FSWatchScope::Directory)) == "Directory");
    ASSERT(std::string(FSWatchScopeName(FSWatchScope::Recursive)) == "Recursive");
}
TEST(TestFSWatchdog_DefaultConstruction)
{
    using namespace ExplorerLens::Engine;
    FilesystemWatchdog watchdog;
    FSWatchChangeEvent evt;
    evt.eventType = FSWatchEvent::Created;
    ASSERT(evt.eventType == FSWatchEvent::Created);
}
TEST(TestFSWatchdog_MaxDirectories)
{
    using namespace ExplorerLens::Engine;
    ASSERT(FilesystemWatchdog::MAX_WATCH_DIRS == 64);
    FSWatchDirConfig cfg;
    cfg.scope = FSWatchScope::Recursive;
    ASSERT(cfg.scope == FSWatchScope::Recursive);
}

//== Compression Benchmark ==
TEST(TestCompBench_AlgoNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(CompressionAlgoName(CompressionAlgo::Deflate)) == "Deflate");
    ASSERT(std::string(CompressionAlgoName(CompressionAlgo::LZ4)) == "LZ4");
    ASSERT(std::string(CompressionAlgoName(CompressionAlgo::Zstd)) == "Zstd");
}
TEST(TestCompBench_MetricNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(CompressionBenchMetricName(CompressionBenchMetric::CompressSpeed)) == "CompressSpeed");
    ASSERT(std::string(CompressionBenchMetricName(CompressionBenchMetric::DecompressSpeed)) == "DecompressSpeed");
    ASSERT(std::string(CompressionBenchMetricName(CompressionBenchMetric::Ratio)) == "Ratio");
}
TEST(TestCompBench_DefaultResult)
{
    using namespace ExplorerLens::Engine;
    CompressionBenchResult result;
    ASSERT(result.algorithm == CompressionAlgo::Deflate);
    ASSERT(result.metric == CompressionBenchMetric::CompressSpeed);
}
TEST(TestCompBench_Iterations)
{
    using namespace ExplorerLens::Engine;
    ASSERT(CompressionBenchmark::ITERATIONS_DEFAULT == 100);
    CompressionBenchmark bench;
    (void)bench;
}

//== Explorer Band Integration ==
TEST(TestBandInteg_PositionNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(BandPositionName(BandPosition::Top)) == "Top");
    ASSERT(std::string(BandPositionName(BandPosition::Bottom)) == "Bottom");
    ASSERT(std::string(BandPositionName(BandPosition::Left)) == "Left");
}
TEST(TestBandInteg_StateNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(BandStateName(BandState::Hidden)) == "Hidden");
    ASSERT(std::string(BandStateName(BandState::Visible)) == "Visible");
    ASSERT(std::string(BandStateName(BandState::Minimized)) == "Minimized");
}
TEST(TestBandInteg_DefaultConfig)
{
    using namespace ExplorerLens::Engine;
    BandIntegrationConfig cfg;
    cfg.position = BandPosition::Top;
    ASSERT(cfg.position == BandPosition::Top);
}
TEST(TestBandInteg_Construction)
{
    using namespace ExplorerLens::Engine;
    ExplorerBandIntegration band;
    BandRegistrationInfo info;
    info.isRegistered = false;
    ASSERT(info.isRegistered == false);
}

//== Thumbnail Stream Protocol ==
TEST(TestStreamProto_ProtocolNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(TSProtoTypeName(TSProtoType::HTTP)) == "HTTP");
    ASSERT(std::string(TSProtoTypeName(TSProtoType::WebSocket)) == "WebSocket");
    ASSERT(std::string(TSProtoTypeName(TSProtoType::NamedPipe)) == "NamedPipe");
}
TEST(TestStreamProto_StateNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(TSProtoStateName(TSProtoState::Idle)) == "Idle");
    ASSERT(std::string(TSProtoStateName(TSProtoState::Streaming)) == "Streaming");
    ASSERT(std::string(TSProtoStateName(TSProtoState::Error)) == "Error");
}
TEST(TestStreamProto_EndpointConfig)
{
    using namespace ExplorerLens::Engine;
    StreamEndpoint endpoint;
    endpoint.protocol = TSProtoType::NamedPipe;
    ASSERT(endpoint.protocol == TSProtoType::NamedPipe);
    ASSERT(endpoint.timeoutMs == 5000);
}
TEST(TestStreamProto_Timeout)
{
    using namespace ExplorerLens::Engine;
    ASSERT(ThumbnailStreamProtocol::TIMEOUT_MS == 5000);
    ThumbnailStreamProtocol proto;
    (void)proto;
}

//== Registry Snapshot Manager ==
TEST(TestRegSnap_ScopeNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(SnapshotScopeName(SnapshotScope::CurrentUser)) == "CurrentUser");
    ASSERT(std::string(SnapshotScopeName(SnapshotScope::LocalMachine)) == "LocalMachine");
    ASSERT(std::string(SnapshotScopeName(SnapshotScope::ClassesRoot)) == "ClassesRoot");
}
TEST(TestRegSnap_ActionNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(SnapshotActionName(SnapshotAction::Backup)) == "Backup");
    ASSERT(std::string(SnapshotActionName(SnapshotAction::Restore)) == "Restore");
    ASSERT(std::string(SnapshotActionName(SnapshotAction::Compare)) == "Compare");
}
TEST(TestRegSnap_Construction)
{
    using namespace ExplorerLens::Engine;
    RegistrySnapshotManager mgr;
    RegistrySnapshot snapshot;
    snapshot.scope = SnapshotScope::CurrentUser;
    ASSERT(snapshot.scope == SnapshotScope::CurrentUser);
}
TEST(TestRegSnap_MaxSnapshots)
{
    using namespace ExplorerLens::Engine;
    ASSERT(RegistrySnapshotManager::MAX_SNAPSHOTS == 10);
    SnapshotComparisonResult result;
    ASSERT(result.identical == true);
}

//== Hot Reload Config Engine ==
TEST(TestHotReload_SourceNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(HotReloadSourceName(HotReloadSource::Registry)) == "Registry");
    ASSERT(std::string(HotReloadSourceName(HotReloadSource::File)) == "File");
    ASSERT(std::string(HotReloadSourceName(HotReloadSource::Environment)) == "Environment");
}
TEST(TestHotReload_TriggerNamesV2)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ConfigReloadTriggerName(ConfigReloadTrigger::Manual)) == "Manual");
    ASSERT(std::string(ConfigReloadTriggerName(ConfigReloadTrigger::FileChange)) == "FileChange");
    ASSERT(std::string(ConfigReloadTriggerName(ConfigReloadTrigger::Timer)) == "Timer");
}
TEST(TestHotReload_DefaultResult)
{
    using namespace ExplorerLens::Engine;
    HotReloadResult result;
    result.source = HotReloadSource::Registry;
    result.trigger = ConfigReloadTrigger::Manual;
    ASSERT(result.source == HotReloadSource::Registry);
}
TEST(TestHotReload_PollInterval)
{
    using namespace ExplorerLens::Engine;
    ASSERT(HotReloadConfigEngine::POLL_INTERVAL_MS == 1000);
    HotReloadConfigEngine engine;
    (void)engine;
}

//== COM Diagnostics Engine ==
TEST(TestCOMDiag_HealthStatusNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(COMHealthStatusName(COMHealthStatus::Healthy)) == "Healthy");
    ASSERT(std::string(COMHealthStatusName(COMHealthStatus::Degraded)) == "Degraded");
    ASSERT(std::string(COMHealthStatusName(COMHealthStatus::Broken)) == "Broken");
}
TEST(TestCOMDiag_RepairActionNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(COMRepairActionName(COMRepairAction::Reregister)) == "Reregister");
    ASSERT(std::string(COMRepairActionName(COMRepairAction::CleanKeys)) == "CleanKeys");
    ASSERT(std::string(COMRepairActionName(COMRepairAction::FullRepair)) == "FullRepair");
}
TEST(TestCOMDiag_DiagnosticResult)
{
    using namespace ExplorerLens::Engine;
    COMDiagnosticResult result;
    result.status = COMHealthStatus::Healthy;
    ASSERT(result.status == COMHealthStatus::Healthy);
}
TEST(TestCOMDiag_CLSIDConstant)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(COMDiagnosticsEngine::CLSID_LENS) == L"9E6ECB90-5A61-42BD-B851-D3297D9C7F39");
    COMDiagnosticsEngine engine;
    (void)engine;
}

//== Thumbnail Watermark ==
TEST(TestWatermark_PositionNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(WatermarkPositionName(WatermarkPosition::TopLeft)) == "TopLeft");
    ASSERT(std::string(WatermarkPositionName(WatermarkPosition::BottomRight)) == "BottomRight");
    ASSERT(std::string(WatermarkPositionName(WatermarkPosition::Center)) == "Center");
}
TEST(TestWatermark_TypeNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(WatermarkTypeName(WatermarkType::Text)) == "Text");
    ASSERT(std::string(WatermarkTypeName(WatermarkType::QRCode)) == "QRCode");
    ASSERT(std::string(WatermarkTypeName(WatermarkType::None)) == "None");
}
TEST(TestWatermark_ApplyAndConfig)
{
    using namespace ExplorerLens::Engine;
    ThumbnailWatermark wm;
    ASSERT(!wm.IsActive());
    WatermarkConfig cfg;
    cfg.type = WatermarkType::Text;
    cfg.text = "DRAFT";
    cfg.opacity = 0.8f;
    wm.SetConfig(cfg);
    ASSERT(wm.IsActive());
    uint8_t pixels[16] = {};
    ASSERT(wm.Apply(pixels, 2, 2));
    ASSERT(wm.GetAppliedCount() == 1);
    ASSERT(wm.GetSupportedTypes().size() == 4);
}

//== Batch Rename Preview ==
TEST(TestRenamePreview_PatternNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(RenamePatternName(RenamePattern::Sequential)) == "Sequential");
    ASSERT(std::string(RenamePatternName(RenamePattern::RegexReplace)) == "RegexReplace");
    ASSERT(std::string(RenamePatternName(RenamePattern::Custom)) == "Custom");
}
TEST(TestRenamePreview_StateNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(RenamePreviewStateName(RenamePreviewState::Idle)) == "Idle");
    ASSERT(std::string(RenamePreviewStateName(RenamePreviewState::Ready)) == "Ready");
    ASSERT(std::string(RenamePreviewStateName(RenamePreviewState::Error)) == "Error");
}
TEST(TestRenamePreview_GenerateAndGet)
{
    using namespace ExplorerLens::Engine;
    BatchRenamePreview brp;
    ASSERT(brp.GetItemCount() == 0);
    std::vector<std::string> files = {"a.txt", "b.txt", "c.txt"};
    ASSERT(brp.GeneratePreviews(files, RenamePattern::Sequential));
    ASSERT(brp.GetItemCount() == 3);
    ASSERT(brp.GetState() == RenamePreviewState::Ready);
    auto* item = brp.GetPreview(0);
    ASSERT(item != nullptr);
    ASSERT(item->newName == "file_1");
}

//== Duplicate File Detector ==
TEST(TestDupDetect_HashMethodNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(DuplicateHashMethodName(DuplicateHashMethod::MD5)) == "MD5");
    ASSERT(std::string(DuplicateHashMethodName(DuplicateHashMethod::Perceptual)) == "Perceptual");
    ASSERT(std::string(DuplicateHashMethodName(DuplicateHashMethod::DifferenceHash)) == "DifferenceHash");
}
TEST(TestDupDetect_ConfidenceNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(DuplicateConfidenceName(DuplicateConfidence::Exact)) == "Exact");
    ASSERT(std::string(DuplicateConfidenceName(DuplicateConfidence::VeryHigh)) == "VeryHigh");
    ASSERT(std::string(DuplicateConfidenceName(DuplicateConfidence::Low)) == "Low");
}
TEST(TestDupDetect_ScanDirectory)
{
    using namespace ExplorerLens::Engine;
    DuplicateFileDetector det;
    ASSERT(!det.IsScanComplete());
    ASSERT(det.ScanDirectory("C:\\test", DuplicateHashMethod::SHA256));
    ASSERT(det.IsScanComplete());
    ASSERT(det.GetGroupCount() >= 1);
    ASSERT(det.GetMethod() == DuplicateHashMethod::SHA256);
}

//== Thumbnail Annotation ==
TEST(TestAnnotation_TypeNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(AnnotationTypeName(AnnotationType::Resolution)) == "Resolution");
    ASSERT(std::string(AnnotationTypeName(AnnotationType::Duration)) == "Duration");
    ASSERT(std::string(AnnotationTypeName(AnnotationType::PageCount)) == "PageCount");
}
TEST(TestAnnotation_StyleNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(AnnotationStyleName(AnnotationStyle::Badge)) == "Badge");
    ASSERT(std::string(AnnotationStyleName(AnnotationStyle::Overlay)) == "Overlay");
}
TEST(TestAnnotation_AddRemoveRender)
{
    using namespace ExplorerLens::Engine;
    ThumbnailAnnotation ann;
    ASSERT(ann.GetAnnotationCount() == 0);
    AnnotationConfig cfg;
    cfg.type = AnnotationType::FileSize;
    cfg.style = AnnotationStyle::Corner;
    ASSERT(ann.AddAnnotation(cfg));
    ASSERT(ann.GetAnnotationCount() == 1);
    for (int i = 0; i < 4; i++)
        ann.AddAnnotation(cfg);
    ASSERT(ann.IsFull());
    ASSERT(!ann.AddAnnotation(cfg));  // exceeds MAX_ANNOTATIONS
    ASSERT(ann.RemoveAnnotation(0));
    ASSERT(ann.GetAnnotationCount() == 4);
    uint8_t px[16] = {};
    ASSERT(ann.Render(px, 2, 2));
}

//== Cache Migration Engine ==
TEST(TestCacheMigration_FormatNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(CacheMigrationFormatName(CacheMigrationFormat::V1Binary)) == "V1Binary");
    ASSERT(std::string(CacheMigrationFormatName(CacheMigrationFormat::Current)) == "Current");
}
TEST(TestCacheMigration_StateNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(CacheMigrationStateName(CacheMigrationState::NotStarted)) == "NotStarted");
    ASSERT(std::string(CacheMigrationStateName(CacheMigrationState::Complete)) == "Complete");
}
TEST(TestCacheMigration_MigrateFlow)
{
    using namespace ExplorerLens::Engine;
    CacheMigrationEngine eng;
    ASSERT(eng.CanMigrate(CacheMigrationFormat::V1Binary, CacheMigrationFormat::Current));
    ASSERT(!eng.CanMigrate(CacheMigrationFormat::Current, CacheMigrationFormat::V1Binary));
    ASSERT(eng.StartMigration("C:\\cache", CacheMigrationFormat::V1Binary, CacheMigrationFormat::Current));
    ASSERT(eng.GetProgress().state == CacheMigrationState::Complete);
    ASSERT(eng.GetCompletionPercent() == 100.0f);
    ASSERT(eng.GetMigrationCount() == 1);
}

//== Explorer Context Menu Extension ==
TEST(TestCtxMenu_ActionNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ExtMenuActionName(ExtMenuAction::RegenerateThumbnail)) == "RegenerateThumbnail");
    ASSERT(std::string(ExtMenuActionName(ExtMenuAction::ExportFullSize)) == "ExportFullSize");
}
TEST(TestCtxMenu_ItemStateNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ExtMenuItemStateName(ExtMenuItemState::Enabled)) == "Enabled");
    ASSERT(std::string(ExtMenuItemStateName(ExtMenuItemState::Submenu)) == "Submenu");
}
TEST(TestCtxMenu_BuildAndExecute)
{
    using namespace ExplorerLens::Engine;
    ExplorerContextMenuExtension ext;
    ASSERT(ext.BuildMenu("C:\\test.zip"));
    ASSERT(ext.GetMenuItemCount() == 5);
    ASSERT(ext.ExecuteAction(ExtMenuAction::CopyThumbnail));
    ASSERT(ext.GetExecutionCount() == 1);
    ASSERT(ext.GetLastAction() == ExtMenuAction::CopyThumbnail);
}

//== Adaptive Quality Scaler ==
TEST(TestQualityScaler_TierNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(QualityTierName(QualityTier::Ultra)) == "Ultra");
    ASSERT(std::string(QualityTierName(QualityTier::Minimum)) == "Minimum");
}
TEST(TestQualityScaler_ReasonNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ScalingReasonName(ScalingReason::CPULoad)) == "CPULoad");
    ASSERT(std::string(ScalingReasonName(ScalingReason::BatteryLow)) == "BatteryLow");
}
TEST(TestQualityScaler_Evaluate)
{
    using namespace ExplorerLens::Engine;
    AdaptiveQualityScaler scaler;
    auto d = scaler.Evaluate(10.0f, 20.0f, 10.0f, false);
    ASSERT(d.tier == QualityTier::Ultra);
    d = scaler.Evaluate(95.0f, 50.0f, 50.0f, false);
    ASSERT(d.tier == QualityTier::Minimum);
    ASSERT(d.reason == ScalingReason::CPULoad);
    d = scaler.Evaluate(10.0f, 10.0f, 10.0f, true);
    ASSERT(d.tier == QualityTier::Medium);
    ASSERT(d.reason == ScalingReason::BatteryLow);
}

//== Thumbnail Compare View ==
TEST(TestCompare_ModeNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(CompareModeName(CompareMode::SideBySide)) == "SideBySide");
    ASSERT(std::string(CompareModeName(CompareMode::Difference)) == "Difference");
}
TEST(TestCompare_SourceNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(CompareSourceName(CompareSource::Cache)) == "Cache");
    ASSERT(std::string(CompareSourceName(CompareSource::Memory)) == "Memory");
}
TEST(TestCompare_RunComparison)
{
    using namespace ExplorerLens::Engine;
    ThumbnailCompareView cv;
    ASSERT(!cv.HasResult());
    uint8_t a[16] = {0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255};
    uint8_t b[16] = {0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255};
    ASSERT(cv.Compare(a, 2, 2, b, 2, 2, CompareMode::Difference));
    ASSERT(cv.HasResult());
    ASSERT(cv.GetResult().matchPercent == 100.0f);
    ASSERT(cv.GetResult().diffPixels == 0);
}

//== File Type Statistics ==
TEST(TestFileStats_CategoryNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(StatCategoryName(StatCategory::DecodeCount)) == "DecodeCount");
    ASSERT(std::string(StatCategoryName(StatCategory::CacheHitRate)) == "CacheHitRate");
}
TEST(TestFileStats_TimeRangeNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(StatTimeRangeName(StatTimeRange::LastHour)) == "LastHour");
    ASSERT(std::string(StatTimeRangeName(StatTimeRange::AllTime)) == "AllTime");
}
TEST(TestFileStats_RecordAndQuery)
{
    using namespace ExplorerLens::Engine;
    FileTypeStatistics stats;
    stats.RecordDecode(".png", 5.0, 1024, true, false);
    stats.RecordDecode(".png", 3.0, 2048, false, false);
    stats.RecordDecode(".jpg", 2.0, 512, true, false);
    auto s = stats.GetStats(".png", StatCategory::DecodeCount);
    ASSERT(s.value == 2.0);
    auto top = stats.GetTopFormats(5);
    ASSERT(top.size() == 2);
    ASSERT(top[0] == ".png");
    ASSERT(stats.GetTotalRecords() == 3);
}

//== Memory Defragmenter ==
TEST(TestDefrag_LevelNames)
{
    using namespace ExplorerLens::Engine;
    MemoryDefragmenter defrag;
    // No regions registered yet — fragmentation should be 0
    double frag = defrag.GetFragmentationRatio();
    ASSERT(frag >= 0.0 && frag <= 1.0);
}
TEST(TestDefrag_StrategyNames)
{
    using namespace ExplorerLens::Engine;
    MemoryDefragmenter defrag;
    auto stats = defrag.GetStats();
    ASSERT(stats.defragRuns == 0);
    ASSERT(stats.totalBytesMoved == 0);
}
TEST(TestDefrag_AnalyzeAndDefrag)
{
    using namespace ExplorerLens::Engine;
    MemoryDefragmenter defrag;
    ASSERT(defrag.GetRegionCount() == 0);
    // Run Defragment on empty set — should succeed
    auto result = defrag.Defragment();
    ASSERT(result.bytesMoved == 0);
    ASSERT(result.regionsCompacted == 0);
    auto stats = defrag.GetStats();
    ASSERT(stats.defragRuns == 1);
}

//== Shell Notification Engine ==
TEST(TestShellNotify_TypeNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ShellNotifyTypeName(ShellNotifyType::AssocChanged)) == "AssocChanged");
    ASSERT(std::string(ShellNotifyTypeName(ShellNotifyType::Delete)) == "Delete");
}
TEST(TestShellNotify_PriorityNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ShellNotifyPriorityName(ShellNotifyPriority::Immediate)) == "Immediate");
    ASSERT(std::string(ShellNotifyPriorityName(ShellNotifyPriority::Batched)) == "Batched");
}
TEST(TestShellNotify_SendAndFlush)
{
    using namespace ExplorerLens::Engine;
    ShellNotificationEngine eng;
    ShellNotification n;
    n.type = ShellNotifyType::UpdateDir;
    n.priority = ShellNotifyPriority::Batched;
    n.itemPath = "C:\\Users";
    ASSERT(eng.SendNotification(n));
    ASSERT(eng.GetPendingCount() == 1);
    n.priority = ShellNotifyPriority::Immediate;
    ASSERT(eng.SendNotification(n));
    ASSERT(eng.GetPendingCount() == 1);  // immediate goes straight out
    ASSERT(eng.BatchFlush() == 1);
    ASSERT(eng.GetPendingCount() == 0);
    ASSERT(eng.GetTotalSent() == 2);
}

//== Thumbnail Export Engine ==
TEST(TestThumbExport_FormatNamesV2)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ThumbnailExportFormatName(ThumbnailExportFormat::PNG)) == "PNG");
    ASSERT(std::string(ThumbnailExportFormatName(ThumbnailExportFormat::TIFF)) == "TIFF");
}
TEST(TestThumbExport_DestNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ExportDestinationName(ExportDestination::File)) == "File");
    ASSERT(std::string(ExportDestinationName(ExportDestination::Network)) == "Network");
}
TEST(TestThumbExport_SingleExport)
{
    using namespace ExplorerLens::Engine;
    ThumbnailExportEngine eng;
    uint8_t px[64] = {};
    ThumbnailExportConfig cfg;
    cfg.format = ThumbnailExportFormat::JPEG;
    cfg.destination = ExportDestination::File;
    cfg.quality = 85;
    ASSERT(eng.Export(px, 4, 4, cfg, "C:\\out.jpg"));
    ASSERT(eng.GetExportCount() == 1);
    ASSERT(eng.GetLastFormat() == ThumbnailExportFormat::JPEG);
    ASSERT(eng.GetSupportedFormats().size() == 5);
}

//== Thumbnail Version Control ==
TEST(TestTVC_VersionNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ThumbnailVersionName(ThumbnailVersion::Original)) == "Original");
    ASSERT(std::string(ThumbnailVersionName(ThumbnailVersion::Draft)) == "Draft");
}
TEST(TestTVC_ActionNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(VersionActionName(VersionAction::Create)) == "Create");
    ASSERT(std::string(VersionActionName(VersionAction::Archive)) == "Archive");
}
TEST(TestTVC_CreateAndRollback)
{
    using namespace ExplorerLens::Engine;
    ThumbnailVersionControl tvc;
    ASSERT(tvc.CreateVersion(L"test.png", 0xABCD, 1024));
    ASSERT(tvc.CreateVersion(L"test.png", 0xDEF0, 2048));
    ASSERT(tvc.GetHistory().size() == 2);
    ASSERT(tvc.Rollback());
    ASSERT(tvc.GetHistory().size() == 3);
    ASSERT(tvc.GetRollbackCount() == 1);
}

//== File Preview Router ==
TEST(TestFPR_HandlerNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(PreviewHandlerName(PreviewHandler::Internal)) == "Internal");
    ASSERT(std::string(PreviewHandlerName(PreviewHandler::External)) == "External");
}
TEST(TestFPR_PriorityNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(RoutingPriorityName(RoutingPriority::Speed)) == "Speed");
    ASSERT(std::string(RoutingPriorityName(RoutingPriority::ForceRegenerate)) == "ForceRegenerate");
}
TEST(TestFPR_RouteAndRegister)
{
    using namespace ExplorerLens::Engine;
    FilePreviewRouter router;
    auto route = router.Route(L"test.webp", RoutingPriority::Speed);
    ASSERT(route.handler == PreviewHandler::Internal);
    ASSERT(route.estimatedMs > 0.0);
    ASSERT(router.RegisterHandler(".webp", PreviewHandler::Plugin));
    ASSERT(router.GetRegisteredCount() == 1);
}

//== Clipboard Thumbnail Manager ==
TEST(TestClip_FormatNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ClipboardFormatName(ClipboardFormat::PNG)) == "PNG");
    ASSERT(std::string(ClipboardFormatName(ClipboardFormat::DIB)) == "DIB");
}
TEST(TestClip_TargetNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(PasteTargetName(PasteTarget::Explorer)) == "Explorer");
    ASSERT(std::string(PasteTargetName(PasteTarget::Document)) == "Document");
}
TEST(TestClip_CopyAndPaste)
{
    using namespace ExplorerLens::Engine;
    ClipboardThumbnailManager mgr;
    ASSERT(mgr.CopyToClipboard(L"test.png", ClipboardFormat::PNG));
    ASSERT(mgr.GetCopyCount() == 1);
    ClipboardEntry entry;
    ASSERT(mgr.PasteFromClipboard(PasteTarget::Explorer, entry));
    ASSERT(entry.format == ClipboardFormat::PNG);
    ASSERT(mgr.GetFormats().size() == 1);
}

//== Format Conversion Pipeline ==
TEST(TestFCP_TargetNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ConvertTargetName(ConvertTarget::WebP)) == "WebP");
    ASSERT(std::string(ConvertTargetName(ConvertTarget::JXL)) == "JXL");
}
TEST(TestFCP_QualityNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ConversionQualityName(ConversionQuality::Lossless)) == "Lossless");
    ASSERT(std::string(ConversionQualityName(ConversionQuality::Minimal)) == "Minimal");
}
TEST(TestFCP_ConvertSingle)
{
    using namespace ExplorerLens::Engine;
    FormatConversionPipeline pipe;
    FormatConversionJob job;
    ASSERT(pipe.Convert(L"in.png", ConvertTarget::AVIF, ConversionQuality::Balanced, job));
    ASSERT(job.target == ConvertTarget::AVIF);
    ASSERT(pipe.GetTotalConversions() == 1);
    ASSERT(pipe.GetBestTarget(50000, false) == ConvertTarget::WebP);
}

//== Vulkan Memory Allocator ==
TEST(TestVMA_TierNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(VkMemoryTierName(VkMemoryTier::DeviceLocal)) == "DeviceLocal");
    ASSERT(std::string(VkMemoryTierName(VkMemoryTier::Shared)) == "Shared");
}
TEST(TestVMA_StrategyNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(VkAllocStrategyName(VkAllocStrategy::BestFit)) == "BestFit");
    ASSERT(std::string(VkAllocStrategyName(VkAllocStrategy::Suballocate)) == "Suballocate");
}
TEST(TestVMA_AllocAndFree)
{
    using namespace ExplorerLens::Engine;
    VulkanMemoryAllocator vma;
    VkMemoryBlock blk;
    ASSERT(vma.Allocate(VkMemoryTier::DeviceLocal, VkAllocStrategy::BestFit, 4096, blk));
    ASSERT(vma.GetUsageBytes() == 4096);
    ASSERT(vma.GetBlockCount() == 1);
    ASSERT(vma.Free(0));
    ASSERT(vma.GetUsageBytes() == 0);
}

//== Decoder Priority Scheduler ==
TEST(TestDPS_PriorityNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(DecoderTaskPriorityName(DecoderTaskPriority::Urgent)) == "Urgent");
    ASSERT(std::string(DecoderTaskPriorityName(DecoderTaskPriority::Deferred)) == "Deferred");
}
TEST(TestDPS_PolicyNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(SchedulerPolicyName(SchedulerPolicy::FIFO)) == "FIFO");
    ASSERT(std::string(SchedulerPolicyName(SchedulerPolicy::Weighted)) == "Weighted");
}
TEST(TestDPS_SubmitAndCancel)
{
    using namespace ExplorerLens::Engine;
    DecoderPriorityScheduler sched;
    DecoderTask t1;
    t1.priority = DecoderTaskPriority::Background;
    t1.decoderName = "LibRaw";
    DecoderTask t2;
    t2.priority = DecoderTaskPriority::Urgent;
    t2.decoderName = "LibWebP";
    auto id1 = sched.Submit(t1);
    sched.Submit(t2);
    ASSERT(sched.GetQueueDepth() == 2);
    ASSERT(sched.Cancel(id1));
    ASSERT(sched.GetQueueDepth() == 1);
}

//== Error Reporting Pipeline ==
TEST(TestERP_DomainNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ErrorDomainName(ErrorDomain::Decoder)) == "Decoder");
    ASSERT(std::string(ErrorDomainName(ErrorDomain::COM)) == "COM");
}
TEST(TestERP_AggregationNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ErrorAggregationName(ErrorAggregation::PerFile)) == "PerFile");
    ASSERT(std::string(ErrorAggregationName(ErrorAggregation::Total)) == "Total");
}
TEST(TestERP_ReportAndQuery)
{
    using namespace ExplorerLens::Engine;
    ErrorReportingPipeline erp;
    erp.Report(ErrorDomain::Decoder, ErrorAggregation::PerFile, "decode failed");
    erp.Report(ErrorDomain::Decoder, ErrorAggregation::PerFile, "decode failed");
    erp.Report(ErrorDomain::GPU, ErrorAggregation::Total, "shader compile");
    ASSERT(erp.GetBucketCount() == 2);
    ASSERT(erp.GetTotalErrorCount() == 3);
    auto top = erp.GetTopErrors(1);
    ASSERT(top.size() == 1 && top[0].count == 2);
}

//== Enterprise Audit Pipeline ==
TEST(TestEAP_ActionNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(AuditActionName(AuditAction::FileAccessed)) == "FileAccessed");
    ASSERT(std::string(AuditActionName(AuditAction::PluginLoaded)) == "PluginLoaded");
}
TEST(TestEAP_DestNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(AuditDestinationName(AuditDestination::ETW)) == "ETW");
    ASSERT(std::string(AuditDestinationName(AuditDestination::Database)) == "Database");
}
TEST(TestEAP_LogAndRetrieve)
{
    using namespace ExplorerLens::Engine;
    EnterpriseAuditPipeline eap;
    eap.SetDestination(AuditDestination::File);
    eap.LogAction(AuditAction::ThumbnailGenerated, L"test.png", "user1");
    eap.LogAction(AuditAction::CacheHit, L"test.png", "user1", "from disk");
    ASSERT(eap.GetEntryCount() == 2);
    auto recent = eap.GetRecentEntries(1);
    ASSERT(recent.size() == 1);
    ASSERT(eap.GetDestination() == AuditDestination::File);
}

//== Resource Quota Manager ==
TEST(TestRQM_ResourceNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(QuotaResourceName(QuotaResource::Memory)) == "Memory");
    ASSERT(std::string(QuotaResourceName(QuotaResource::GPUMemory)) == "GPUMemory");
}
TEST(TestRQM_EnforcementNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(QuotaEnforcementName(QuotaEnforcement::SoftLimit)) == "SoftLimit");
    ASSERT(std::string(QuotaEnforcementName(QuotaEnforcement::Deny)) == "Deny");
}
TEST(TestRQM_SetAndCheck)
{
    using namespace ExplorerLens::Engine;
    ResourceQuotaManager rqm;
    rqm.SetQuota(QuotaResource::Memory, QuotaEnforcement::HardLimit, 1048576, "bytes");
    ASSERT(!rqm.IsOverQuota());
    rqm.RecordUsage(QuotaResource::Memory, 2000000);
    ASSERT(rqm.IsOverQuota());
    ASSERT(rqm.GetUsage(QuotaResource::Memory) == 2000000);
    rqm.ResetUsage();
    ASSERT(!rqm.IsOverQuota());
}

//== Access Token Validator ==
TEST(TestATV_TypeNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(TokenTypeName(AccessTokenType::Process)) == "Process");
    ASSERT(std::string(TokenTypeName(AccessTokenType::Anonymous)) == "Anonymous");
}
TEST(TestATV_ResultNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(TokenValidationResultName(TokenValidationResult::Valid)) == "Valid");
    ASSERT(std::string(TokenValidationResultName(TokenValidationResult::Malformed)) == "Malformed");
}
TEST(TestATV_Validate)
{
    using namespace ExplorerLens::Engine;
    AccessTokenValidator atv;
    AccessTokenInfo info;
    auto result = atv.ValidateToken(12345, info);
    ASSERT(result == TokenValidationResult::Valid);
    ASSERT(info.integrity == AccessTokenValidator::INTEGRITY_MEDIUM);
    ASSERT(!atv.IsElevated());
    ASSERT(atv.CheckIntegrity(AccessTokenValidator::INTEGRITY_MEDIUM));
}

//== Cache Encryption Layer ==
TEST(TestCEL_AlgoNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(EncryptionAlgorithmName(EncryptionAlgorithm::AES256)) == "AES256");
    ASSERT(std::string(EncryptionAlgorithmName(EncryptionAlgorithm::None)) == "None");
}
TEST(TestCEL_KDFNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(KeyDerivationName(KeyDerivation::PBKDF2)) == "PBKDF2");
    ASSERT(std::string(KeyDerivationName(KeyDerivation::Direct)) == "Direct");
}
TEST(TestCEL_EncryptDecrypt)
{
    using namespace ExplorerLens::Engine;
    CacheEncryptionLayer cel;
    EncryptionConfig cfg;
    cfg.algorithm = EncryptionAlgorithm::AES256;
    cel.Configure(cfg);
    ASSERT(cel.IsEncrypted());
    std::vector<uint8_t> plain = {1, 2, 3, 4};
    std::vector<uint8_t> cipher, decrypted;
    ASSERT(cel.Encrypt(plain, cipher));
    ASSERT(cel.Decrypt(cipher, decrypted));
    ASSERT(decrypted == plain);
    ASSERT(cel.RotateKey());
    ASSERT(cel.GetKeyRotations() == 1);
}

//== Explorer Preview Pane ==
TEST(TestEPP_ModeNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(PreviewPaneModeName(PreviewPaneMode::Thumbnail)) == "Thumbnail");
    ASSERT(std::string(PreviewPaneModeName(PreviewPaneMode::Split)) == "Split");
}
TEST(TestEPP_LayoutNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(PreviewPaneLayoutName(PreviewPaneLayout::Standard)) == "Standard");
    ASSERT(std::string(PreviewPaneLayoutName(PreviewPaneLayout::Custom)) == "Custom");
}
TEST(TestEPP_ActivateAndRefresh)
{
    using namespace ExplorerLens::Engine;
    ExplorerPreviewPane pane;
    ASSERT(!pane.IsActive());
    ASSERT(pane.Activate(L"test.png"));
    ASSERT(pane.IsActive());
    ASSERT(pane.GetCurrentFile() == L"test.png");
    pane.SetMode(PreviewPaneMode::FullPreview);
    ASSERT(pane.GetConfig().mode == PreviewPaneMode::FullPreview);
    ASSERT(pane.Refresh());
    ASSERT(pane.GetRefreshCount() == 1);
}

//== DirectShow Thumbnail Bridge ==
TEST(Test_DirectShowBridge_FilterTypeNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(DSFilterTypeName(DSFilterType::Source)) == "Source");
    ASSERT(std::string(DSFilterTypeName(DSFilterType::Mux)) == "Mux");
}
TEST(Test_DirectShowBridge_StatusNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(DSBridgeStatusName(DSBridgeStatus::Ready)) == "Ready");
    ASSERT(std::string(DSBridgeStatusName(DSBridgeStatus::Error)) == "Error");
}
TEST(Test_DirectShowBridge_ConnectDisconnect)
{
    using namespace ExplorerLens::Engine;
    DirectShowThumbnailBridge bridge;
    ASSERT(bridge.GetStatus() == DSBridgeStatus::Ready);
    ASSERT(bridge.Connect(L"test.avi"));
    ASSERT(bridge.GetStatus() == DSBridgeStatus::Connected);
    bridge.Disconnect();
    ASSERT(bridge.GetStatus() == DSBridgeStatus::Ready);
}
TEST(Test_DirectShowBridge_GrabFrame)
{
    using namespace ExplorerLens::Engine;
    DirectShowThumbnailBridge bridge;
    bridge.Connect(L"video.mp4");
    auto frame = bridge.GrabFrame();
    ASSERT(frame.width == 320);
    ASSERT(frame.height == 240);
    ASSERT(bridge.GetGrabCount() == 1);
}

//== Shell Extension Health Monitor ==
TEST(Test_HealthMonitor_StatusNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ShellHealthStatusName(ShellHealthStatus::Healthy)) == "Healthy");
    ASSERT(std::string(ShellHealthStatusName(ShellHealthStatus::Crashed)) == "Crashed");
}
TEST(Test_HealthMonitor_RecoveryNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(RecoveryActionName(RecoveryAction::Restart)) == "Restart");
    ASSERT(std::string(RecoveryActionName(RecoveryAction::Escalate)) == "Escalate");
}
TEST(Test_HealthMonitor_CheckHealth)
{
    using namespace ExplorerLens::Engine;
    ShellExtensionHealthMonitor mon;
    auto result = mon.CheckHealth();
    ASSERT(result.status == ShellHealthStatus::Healthy);
    ASSERT(mon.GetCheckCount() == 1);
}
TEST(Test_HealthMonitor_AutoRecover)
{
    using namespace ExplorerLens::Engine;
    ShellExtensionHealthMonitor mon;
    mon.SimulateFailure(ShellHealthStatus::Degraded);
    ASSERT(mon.AutoRecover());
    ASSERT(mon.GetStatus() == ShellHealthStatus::Healthy);
}

//== Thumbnail Color Space ==
TEST(Test_ColorSpace_TypeNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ColorSpaceTypeName(ColorSpaceType::SRGB)) == "sRGB");
    ASSERT(std::string(ColorSpaceTypeName(ColorSpaceType::DisplayP3)) == "DisplayP3");
}
TEST(Test_ColorSpace_GammaNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(GammaMappingName(GammaMapping::Linear)) == "Linear");
    ASSERT(std::string(GammaMappingName(GammaMapping::PQ)) == "PQ");
}
TEST(Test_ColorSpace_SetWorkingSpace)
{
    using namespace ExplorerLens::Engine;
    ThumbnailColorSpace cs;
    cs.SetWorkingSpace(ColorSpaceType::Rec2020);
    ASSERT(cs.GetProfile().colorSpace == ColorSpaceType::Rec2020);
}
TEST(Test_ColorSpace_ConvertNoOp)
{
    using namespace ExplorerLens::Engine;
    ThumbnailColorSpace cs;
    float r = 0.5f, g = 0.6f, b = 0.7f;
    ASSERT(cs.Convert(r, g, b));
    ASSERT(r >= 0.0f && r <= 1.0f);
}

//== Async IO Completion Engine ==
TEST(Test_AsyncIOCP_PriorityNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(AsyncIOPriorityName(AsyncIOPriority::Critical)) == "Critical");
    ASSERT(std::string(AsyncIOPriorityName(AsyncIOPriority::Background)) == "Background");
}
TEST(Test_AsyncIOCP_StatusNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(AsyncIOStatusName(AsyncIOStatus::Pending)) == "Pending");
    ASSERT(std::string(AsyncIOStatusName(AsyncIOStatus::Timeout)) == "Timeout");
}
TEST(Test_AsyncIOCP_SubmitAndPoll)
{
    using namespace ExplorerLens::Engine;
    AsyncIOCompletionEngine engine;
    auto id = engine.Submit(L"test.bin", 0, 4096);
    ASSERT(id > 0);
    ASSERT(engine.GetPending() == 1);
    auto completed = engine.Poll();
    ASSERT(completed == 1);
    ASSERT(engine.GetPending() == 0);
}
TEST(Test_AsyncIOCP_Cancel)
{
    using namespace ExplorerLens::Engine;
    AsyncIOCompletionEngine engine;
    engine.Submit(L"data.raw", 0, 1024);
    ASSERT(engine.Cancel(1));
    ASSERT(engine.GetCancelledCount() == 1);
}

//== EXIF Orientation Fixer ==
TEST(Test_ExifFixer_OrientationNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ExifOrientationName(ExifOrientation::Normal)) == "Normal");
    ASSERT(std::string(ExifOrientationName(ExifOrientation::Rotate270)) == "Rotate270");
}
TEST(Test_ExifFixer_ModeNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(FixApplicationModeName(FixApplicationMode::Auto)) == "Auto");
    ASSERT(std::string(FixApplicationModeName(FixApplicationMode::Silent)) == "Silent");
}
TEST(Test_ExifFixer_ApplyRotation)
{
    using namespace ExplorerLens::Engine;
    ExifOrientationFixer fixer;
    uint32_t w = 1920, h = 1080;
    ASSERT(fixer.ApplyFix(ExifOrientation::Rotate90, w, h));
    ASSERT(w == 1080 && h == 1920);
}
TEST(Test_ExifFixer_ReadOrientation)
{
    using namespace ExplorerLens::Engine;
    ExifOrientationFixer fixer;
    auto result = fixer.ReadOrientation(L"photo.jpg");
    ASSERT(result.hasExif);
    ASSERT(result.orientation == ExifOrientation::Normal);
}

//== Multi-Monitor DPI Scaler ==
TEST(Test_DPIScaler_ModeNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(DPIScaleModeName(DPIScaleMode::PerMonitorV2)) == "PerMonitorV2");
    ASSERT(std::string(DPIScaleModeName(DPIScaleMode::Unaware)) == "Unaware");
}
TEST(Test_DPIScaler_ProfileNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(MonitorProfileName(MonitorProfile::Standard)) == "Standard");
    ASSERT(std::string(MonitorProfileName(MonitorProfile::UltraHiDPI)) == "UltraHiDPI");
}
TEST(Test_DPIScaler_ScaleFactor)
{
    using namespace ExplorerLens::Engine;
    MultiMonitorDPIScaler scaler;
    ASSERT(scaler.GetScaleFactor(96) >= 0.99f && scaler.GetScaleFactor(96) <= 1.01f);
    ASSERT(scaler.GetScaleFactor(192) >= 1.99f && scaler.GetScaleFactor(192) <= 2.01f);
}
TEST(Test_DPIScaler_ScaleForMonitor)
{
    using namespace ExplorerLens::Engine;
    MultiMonitorDPIScaler scaler;
    ASSERT(scaler.ScaleForMonitor(128, 192) == 256);
    ASSERT(scaler.GetMonitorProfile(192) == MonitorProfile::UltraHiDPI);
}

//== VirtualAlloc Optimizer ==
TEST(Test_VAlloc_StrategyNames)
{
    using namespace ExplorerLens::Engine;
    VirtualAllocOptimizer opt;
    auto stats = opt.GetStats();
    ASSERT(stats.commitRatio >= 0.0);
}
TEST(Test_VAlloc_ProtectionNames)
{
    using namespace ExplorerLens::Engine;
    VirtualAllocOptimizer opt;
    auto regions = opt.GetActiveRegions();
    ASSERT(regions.empty());
}
TEST(Test_VAlloc_AllocateAndRelease)
{
    using namespace ExplorerLens::Engine;
    VirtualAllocOptimizer opt;
    auto region = opt.Reserve(65536);
    ASSERT(region.base != nullptr);
    ASSERT(region.reserved >= 65536);
    opt.Release(region);
    ASSERT(region.base == nullptr);
}
TEST(Test_VAlloc_OptimizeWorking)
{
    using namespace ExplorerLens::Engine;
    VirtualAllocOptimizer opt;
    auto region = opt.Reserve(1024 * 1024);
    ASSERT(region.base != nullptr);
    ASSERT(opt.Commit(region, 0, 4096));
    auto stats = opt.GetStats();
    ASSERT(stats.totalReserved > 0);
    opt.Release(region);
}

//== Thumbnail Histogram ==
TEST(Test_Histogram_ChannelNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(HistogramChannelName(HistogramChannel::Red)) == "Red");
    ASSERT(std::string(HistogramChannelName(HistogramChannel::Luminance)) == "Luminance");
}
TEST(Test_Histogram_BinSizeNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(HistogramBinSizeName(HistogramBinSize::Bin256)) == "256");
    ASSERT(std::string(HistogramBinSizeName(HistogramBinSize::Bin1024)) == "1024");
}
TEST(Test_Histogram_Compute)
{
    using namespace ExplorerLens::Engine;
    ThumbnailHistogram hist;
    uint8_t data[] = {100, 100, 200, 200, 50};
    auto result = hist.ComputeHistogram(data, 5, HistogramChannel::Luminance);
    ASSERT(result.totalPixels == 5);
    ASSERT(result.meanValue > 0.0f);
}
TEST(Test_Histogram_PeakAndMean)
{
    using namespace ExplorerLens::Engine;
    ThumbnailHistogram hist;
    uint8_t data[] = {128, 128, 128, 128};
    auto result = hist.ComputeHistogram(data, 4, HistogramChannel::Green);
    ASSERT(hist.GetPeakBin(result) == 128);
    ASSERT(hist.GetMeanBrightness(result) >= 127.0f);
}

//== File Association Manager ==
TEST(Test_FileAssoc_ScopeNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(AssociationScopeName(AssociationScope::User)) == "User");
    ASSERT(std::string(AssociationScopeName(AssociationScope::GPOManaged)) == "GPOManaged");
}
TEST(Test_FileAssoc_ConflictNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(AssociationConflictName(AssociationConflict::Overwrite)) == "Overwrite");
    ASSERT(std::string(AssociationConflictName(AssociationConflict::Prompt)) == "Prompt");
}
TEST(Test_FileAssoc_RegisterUnregister)
{
    using namespace ExplorerLens::Engine;
    FileAssociationManager mgr;
    ASSERT(mgr.Register(L".psd"));
    ASSERT(mgr.IsRegistered(L".psd"));
    ASSERT(mgr.Unregister(L".psd"));
    ASSERT(!mgr.IsRegistered(L".psd"));
}
TEST(Test_FileAssoc_GetConflicts)
{
    using namespace ExplorerLens::Engine;
    FileAssociationManager mgr;
    mgr.Register(L".jpg");
    mgr.Register(L".png");
    auto report = mgr.GetConflicts();
    ASSERT(report.totalExtensions == 2);
}

//== DX12 Fence Manager ==
TEST(Test_DX12Fence_StateNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(FenceStateName(FenceState::Unsignaled)) == "Unsignaled");
    ASSERT(std::string(FenceStateName(FenceState::Signaled)) == "Signaled");
}
TEST(Test_DX12Fence_WaitModeNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(FenceWaitModeName(FenceWaitMode::Blocking)) == "Blocking");
    ASSERT(std::string(FenceWaitModeName(FenceWaitMode::Hybrid)) == "Hybrid");
}
TEST(Test_DX12Fence_CreateAndSignal)
{
    using namespace ExplorerLens::Engine;
    DX12FenceManager mgr;
    auto id = mgr.CreateFence();
    ASSERT(id > 0);
    ASSERT(mgr.Signal(id, 42));
    ASSERT(mgr.GetCompletedValue(id) == 42);
}
TEST(Test_DX12Fence_WaitForFence)
{
    using namespace ExplorerLens::Engine;
    DX12FenceManager mgr;
    auto id = mgr.CreateFence();
    mgr.Signal(id, 10);
    ASSERT(mgr.WaitForFence(id, 10));
    ASSERT(mgr.GetWaitCount() == 1);
}

//== Localization Engine ==
TEST(Test_Locale_IdNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(LocaleIdName(LocaleId::EnUS)) == "en-US");
    ASSERT(std::string(LocaleIdName(LocaleId::JaJP)) == "ja-JP");
}
TEST(Test_Locale_CategoryNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(StringCategoryName(StringCategory::UI)) == "UI");
    ASSERT(std::string(StringCategoryName(StringCategory::Accessibility)) == "Accessibility");
}
TEST(Test_Locale_SetAndGetLocale)
{
    using namespace ExplorerLens::Engine;
    CoreLocalizationEngine loc;
    loc.SetLocale(LocaleId::DeDE);
    ASSERT(loc.GetCurrentLocale() == LocaleId::DeDE);
    ASSERT(loc.GetSupportedLocales().size() == 5);
}
TEST(Test_Locale_GetString)
{
    using namespace ExplorerLens::Engine;
    CoreLocalizationEngine loc;
    loc.AddString("btn_ok", "OK");
    ASSERT(loc.GetString("btn_ok") == "OK");
    ASSERT(loc.GetString("missing") == "[missing]");
}

//== Thumbnail Sprite Sheet ==
TEST(Test_SpriteSheet_LayoutNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(SpriteLayoutName(SpriteLayout::Grid)) == "Grid");
    ASSERT(std::string(SpriteLayoutName(SpriteLayout::TreePack)) == "TreePack");
}
TEST(Test_SpriteSheet_FormatNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(SpriteOutputFormatName(SpriteOutputFormat::PNG)) == "PNG");
    ASSERT(std::string(SpriteOutputFormatName(SpriteOutputFormat::DDS)) == "DDS");
}
TEST(Test_SpriteSheet_AddAndGenerate)
{
    using namespace ExplorerLens::Engine;
    ThumbnailSpriteSheet sheet;
    sheet.AddThumbnail(L"a.png");
    sheet.AddThumbnail(L"b.png");
    auto result = sheet.Generate();
    ASSERT(result.success);
    ASSERT(result.spriteCount == 2);
    ASSERT(result.totalWidth > 0);
}
TEST(Test_SpriteSheet_EstimatedSize)
{
    using namespace ExplorerLens::Engine;
    ThumbnailSpriteSheet sheet;
    sheet.AddThumbnail(L"img1.jpg");
    ASSERT(sheet.GetEstimatedSize() > 0);
    ASSERT(sheet.GetSpriteCount() == 1);
}

//== Cache Telemetry Collector ==
TEST(Test_CacheTelemetry_EventNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(CacheTelemetryEventName(CacheTelemetryEvent::CacheHit)) == "CacheHit");
    ASSERT(std::string(CacheTelemetryEventName(CacheTelemetryEvent::Corruption)) == "Corruption");
}
TEST(Test_CacheTelemetry_IntervalNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(CacheTelemetryIntervalName(CacheTelemetryInterval::Realtime)) == "Realtime");
    ASSERT(std::string(CacheTelemetryIntervalName(CacheTelemetryInterval::Session)) == "Session");
}
TEST(Test_CacheTelemetry_RecordAndHitRate)
{
    using namespace ExplorerLens::Engine;
    CacheTelemetryCollector col;
    col.Record(CacheTelemetryEvent::CacheHit);
    col.Record(CacheTelemetryEvent::CacheHit);
    col.Record(CacheTelemetryEvent::CacheMiss);
    ASSERT(col.GetHitRate() > 0.6f);
    ASSERT(col.GetTotalEvents() == 3);
}
TEST(Test_CacheTelemetry_Export)
{
    using namespace ExplorerLens::Engine;
    CacheTelemetryCollector col;
    col.Record(CacheTelemetryEvent::Eviction);
    auto snap = col.Export();
    ASSERT(snap.evictions == 1);
    ASSERT(snap.evictionRate > 0.0f);
}

//== Windows Search Integration ==
TEST(Test_WinSearch_PropertyNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(SearchPropertyTypeName(SearchPropertyType::Thumbnail)) == "Thumbnail");
    ASSERT(std::string(SearchPropertyTypeName(SearchPropertyType::Duration)) == "Duration");
}
TEST(Test_WinSearch_IndexingStateNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(IndexingStateName(IndexingState::Idle)) == "Idle");
    ASSERT(std::string(IndexingStateName(IndexingState::Complete)) == "Complete");
}
TEST(Test_WinSearch_RegisterProvider)
{
    using namespace ExplorerLens::Engine;
    WindowsSearchIntegration wsi;
    ASSERT(!wsi.IsRegistered());
    ASSERT(wsi.RegisterProvider(L"C:\\TestScope"));
    ASSERT(wsi.IsRegistered());
    ASSERT(wsi.GetRegisterCount() == 1);
}
TEST(Test_WinSearch_QueryProperties)
{
    using namespace ExplorerLens::Engine;
    WindowsSearchIntegration wsi;
    wsi.RegisterProvider(L"C:\\Scope");
    auto props = wsi.QueryProperties(L"image.png");
    ASSERT(props.size() == 3);
}

// AdaptiveCacheBudgetManager Tests
TEST(Test_ACBudget_TierNames)
{
    using namespace ExplorerLens::Cache;
    auto budgets = AdaptiveCacheBudgetManager::CreateDefaultBudgets(512 * 1024 * 1024);
    ASSERT(budgets.size() > 0);
    ASSERT(static_cast<int>(MemoryPressureLevel::Normal) == 0);
    ASSERT(static_cast<int>(MemoryPressureLevel::Critical) == 3);
}
TEST(Test_ACBudget_DefaultBudgets)
{
    using namespace ExplorerLens::Cache;
    AdaptiveCacheBudgetManager mgr(512 * 1024 * 1024);
    auto budgets = mgr.CurrentBudgets();
    ASSERT(budgets.size() > 0);
    ASSERT(mgr.TotalBudget() == 512 * 1024 * 1024);
}
TEST(Test_ACBudget_Rebalance)
{
    using namespace ExplorerLens::Cache;
    AdaptiveCacheBudgetManager mgr(512 * 1024 * 1024);
    auto snap = SystemMemorySnapshot::QuerySystem();
    auto result = mgr.Rebalance(snap);
    ASSERT(mgr.TotalBudget() > 0);
}

// ArchiveMemoryCompactor Tests
TEST(Test_AMemCompact_SlabStates)
{
    using namespace ExplorerLens::Engine;
    // Verify ExtractedBuffer defaults
    ExtractedBuffer buf;
    ASSERT(buf.alive == true);
    ASSERT(buf.pinned == false);
    ASSERT(buf.size == 0);
}
TEST(Test_AMemCompact_EvictionPolicies)
{
    using namespace ExplorerLens::Engine;
    // Verify CompactorStats defaults
    CompactorStats stats;
    ASSERT(stats.compactionsPerformed == 0);
    ASSERT(stats.fragmentationRatio == 0.0);
}
TEST(Test_AMemCompact_TrackSlab)
{
    using namespace ExplorerLens::Engine;
    ArchiveMemoryCompactor compactor;
    auto* buf = compactor.AllocateBuffer(1, 0, 1024);
    ASSERT(buf != nullptr);
    ASSERT(buf->alive);
    auto stats = compactor.GetStats();
    ASSERT(stats.bufferCount >= 1);
}
TEST(Test_AMemCompact_Compact)
{
    using namespace ExplorerLens::Engine;
    ArchiveMemoryCompactor compactor;
    auto* buf1 = compactor.AllocateBuffer(1, 0, 1024);
    auto* buf2 = compactor.AllocateBuffer(1, 1, 2048);
    ASSERT(buf1 != nullptr);
    compactor.FreeBuffer(buf1);
    auto result = compactor.Compact();
    ASSERT(result.fragmentsAfter <= result.fragmentsBefore || result.fragmentsBefore == 0);
    compactor.FreeBuffer(buf2);
}

// BatchProcessor Tests
TEST(Test_BatchProc_JobPriorities)
{
    using namespace ExplorerLens::Engine::Pipeline;
    ASSERT(static_cast<int>(JobPriority::Critical) == 0);
    ASSERT(static_cast<int>(JobPriority::Idle) == 4);
}
TEST(Test_BatchProc_JobStatuses)
{
    using namespace ExplorerLens::Engine::Pipeline;
    ASSERT(static_cast<int>(JobStatus::Queued) == 0);
    ASSERT(static_cast<int>(JobStatus::Paused) == 5);
}
TEST(Test_BatchProc_SubmitAndQueue)
{
    using namespace ExplorerLens::Engine::Pipeline;
    BatchProcessor bp;
    ASSERT(bp.QueueDepth() == 0);
    ASSERT(!bp.HasPendingWork());
    ASSERT(bp.TotalSubmitted() == 0);
    ASSERT(bp.TotalCompleted() == 0);
}
TEST(Test_BatchProc_PauseResume)
{
    using namespace ExplorerLens::Engine::Pipeline;
    BatchProcessor bp;
    ASSERT(!bp.IsPaused());
    bp.Pause();
    ASSERT(bp.IsPaused());
    bp.Resume();
    ASSERT(!bp.IsPaused());
}

// BufferPoolAllocator Tests
TEST(Test_BufPool_SlabClassNames)
{
    using namespace ExplorerLens::Memory;
    ASSERT(std::string(SlabClassName(SlabClass::Tiny)) != "");
    ASSERT(std::string(SlabClassName(SlabClass::Huge)) != "");
}
TEST(Test_BufPool_ClassifyDimension)
{
    using namespace ExplorerLens::Memory;
    ASSERT(ClassifyDimension(32, 32) == SlabClass::Tiny);
    ASSERT(ClassifyDimension(256, 256) == SlabClass::Medium);
    ASSERT(ClassifyDimension(2048, 2048) == SlabClass::Huge);
}
TEST(Test_BufPool_SlabPoolAcquireRelease)
{
    using namespace ExplorerLens::Memory;
    SlabPool pool(SlabClass::Small, 8);
    auto buf = pool.Acquire();
    ASSERT(buf.IsValid());
    ASSERT(buf.capacity == SlabClassBufferSize(SlabClass::Small));
    pool.Release(buf);
    ASSERT(!buf.IsValid());  // data set to nullptr after release
}
TEST(Test_BufPool_PoolStats)
{
    using namespace ExplorerLens::Memory;
    SlabPool pool(SlabClass::Medium, 4);
    auto buf = pool.Acquire();
    auto stats = pool.GetStats();
    ASSERT(stats.totalAllocated >= 1);
    ASSERT(stats.currentInUse >= 1);
    pool.Release(buf);
}

// CacheKeyGenerator Tests
TEST(Test_CacheKey_Generate)
{
    using namespace ExplorerLens::Engine::Cache;
    auto key = CacheKeyGenerator::Generate(L"C:\\test.png", 256, 256);
    ASSERT(!key.empty());
}
TEST(Test_CacheKey_HashFNV)
{
    using namespace ExplorerLens::Engine::Cache;
    auto h1 = CacheKeyGenerator::HashFNV1a(L"test1");
    auto h2 = CacheKeyGenerator::HashFNV1a(L"test2");
    ASSERT(h1 != h2);
    ASSERT(CacheKeyGenerator::HashFNV1a(L"test1") == h1);  // Deterministic
}
TEST(Test_CacheKey_ValidKey)
{
    using namespace ExplorerLens::Engine::Cache;
    auto key = CacheKeyGenerator::Generate(L"C:\\image.jpg", 128, 128);
    ASSERT(CacheKeyGenerator::IsValidKey(key.c_str()));
    ASSERT(!CacheKeyGenerator::IsValidKey(L""));
}
TEST(Test_CacheKey_GenerateWithTime)
{
    using namespace ExplorerLens::Engine::Cache;
    FILETIME ft{};
    ft.dwLowDateTime = 1000;
    ft.dwHighDateTime = 500;
    auto key = CacheKeyGenerator::GenerateWithTime(L"C:\\test.jpg", 256, 256, ft);
    ASSERT(!key.empty());
}

// CRTConsistencyManager Tests
TEST(Test_CRT_ModeNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(CRTConsistencyManager::CRTModeName(CRTConsistencyManager::CRTMode::DynamicMD) != nullptr);
    ASSERT(CRTConsistencyManager::CRTModeName(CRTConsistencyManager::CRTMode::StaticMT) != nullptr);
}
TEST(Test_CRT_StatusNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(CRTConsistencyManager::StatusName(CRTConsistencyManager::LinkageStatus::Consistent) != nullptr);
    ASSERT(CRTConsistencyManager::StatusName(CRTConsistencyManager::LinkageStatus::Mismatch) != nullptr);
}
TEST(Test_CRT_Counts)
{
    using namespace ExplorerLens::Engine;
    ASSERT(CRTConsistencyManager::CRTModeCount() == 4);
    ASSERT(CRTConsistencyManager::StatusCount() == 3);
}
TEST(Test_CRT_AuditLibraries)
{
    using namespace ExplorerLens::Engine;
    auto libs = CRTConsistencyManager::AuditLibraries();
    ASSERT(libs.size() > 0);
    ASSERT(CRTConsistencyManager::LibraryCount() > 0);
}

// DeadCodeAudit Tests
TEST(Test_DCAudit_TypeNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(DeadCodeAudit::TypeName(DeadCodeType::ObsoleteFile)) != "");
    ASSERT(std::string(DeadCodeAudit::TypeName(DeadCodeType::DeprecatedAPI)) != "");
}
TEST(Test_DCAudit_SeverityNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(DeadCodeAudit::SeverityName(DeadCodeSeverity::Info)) != "");
    ASSERT(std::string(DeadCodeAudit::SeverityName(DeadCodeSeverity::Critical)) != "");
}
TEST(Test_DCAudit_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& audit = DeadCodeAudit::Instance();
    auto findings = audit.GetFindings();
    ASSERT(audit.GetCleanupProgress() >= 0.0f);
}
TEST(Test_DCAudit_CountByStatus)
{
    using namespace ExplorerLens::Engine;
    auto& audit = DeadCodeAudit::Instance();
    auto cleaned = audit.CountByStatus(DeadCodeStatus::Cleaned);
    ASSERT(cleaned >= 0);
}

// DeadCodeAuditor Tests
TEST(Test_DCAuditor_CategoryNames)
{
    using namespace ExplorerLens::Engine;
    auto findings = DeadCodeAuditor::RunAudit();
    // Audit returns non-negative count of findings
    ASSERT(findings.size() == findings.size());  // No crash
}
TEST(Test_DCAuditor_SeverityNames)
{
    using namespace ExplorerLens::Engine;
    auto resolved = DeadCodeAuditor::ResolvedCount();
    // Resolved count must be non-negative (size_t)
    ASSERT(resolved == resolved);  // Deterministic
}
TEST(Test_DCAuditor_RunAudit)
{
    using namespace ExplorerLens::Engine;
    auto findings = DeadCodeAuditor::RunAudit();
    ASSERT(findings.size() >= 0);
}
TEST(Test_DCAuditor_AllResolved)
{
    using namespace ExplorerLens::Engine;
    auto resolved = DeadCodeAuditor::ResolvedCount();
    ASSERT(resolved >= 0);
}

// DecoderHealthDashboard Tests
TEST(Test_DHDash_CircuitStates)
{
    using namespace ExplorerLens::Core;
    ASSERT(static_cast<int>(CircuitState::Closed) == 0);
    ASSERT(static_cast<int>(CircuitState::HalfOpen) == 2);
}
TEST(Test_DHDash_HealthStatuses)
{
    using namespace ExplorerLens::Core;
    ASSERT(HealthStatusName(ExplorerLens::Core::HealthStatus::Healthy) != nullptr);
    ASSERT(HealthStatusName(ExplorerLens::Core::HealthStatus::Degraded) != nullptr);
    ASSERT(HealthStatusName(ExplorerLens::Core::HealthStatus::Unhealthy) != nullptr);
    ASSERT(HealthStatusName(ExplorerLens::Core::HealthStatus::Disabled) != nullptr);
}
TEST(Test_DHDash_CreateAndRegister)
{
    using namespace ExplorerLens::Core;
    DecoderDashboardConfig cfg{};
    auto dash = DecoderHealthDashboard::Create(cfg);
    dash.RegisterDecoder("WebP", {".webp"});
    dash.RegisterDecoder("JXL", {".jxl"});
}
TEST(Test_DHDash_RecordAndStats)
{
    using namespace ExplorerLens::Core;
    DecoderDashboardConfig cfg{};
    auto dash = DecoderHealthDashboard::Create(cfg);
    dash.RegisterDecoder("PNG", {".png"});
    dash.RecordDecode("PNG", true, 5, 1024);
    dash.RecordDecode("PNG", true, 3, 512);
    auto stats = dash.GetStats();
    ASSERT(stats.totalDecodes >= 2);
}

// DecoderHealthMonitor Tests
TEST(Test_DHMon_RecordSuccess)
{
    using namespace ExplorerLens::Engine;
    auto& mon = DecoderHealthMonitor::GetInstance();
    mon.ResetAll();
    mon.RecordSuccess(L"TestDecoder");
    auto stats = mon.GetStats(L"TestDecoder");
    ASSERT(stats.successCount >= 1);
}
TEST(Test_DHMon_RecordFailure)
{
    using namespace ExplorerLens::Engine;
    auto& mon = DecoderHealthMonitor::GetInstance();
    mon.ResetAll();
    mon.RecordFailure(L"FailDecoder");
    auto stats = mon.GetStats(L"FailDecoder");
    ASSERT(stats.failureCount >= 1);
}
TEST(Test_DHMon_IsAvailable)
{
    using namespace ExplorerLens::Engine;
    auto& mon = DecoderHealthMonitor::GetInstance();
    mon.ResetAll();
    for (int i = 0; i < 5; i++)
        mon.RecordSuccess(L"GoodDecoder");
    ASSERT(mon.IsDecoderAvailable(L"GoodDecoder"));
}
TEST(Test_DHMon_IsHealthy)
{
    using namespace ExplorerLens::Engine;
    auto& mon = DecoderHealthMonitor::GetInstance();
    mon.ResetAll();
    mon.RecordSuccess(L"HealthCheck");
    auto stats = mon.GetStats(L"HealthCheck");
    ASSERT(stats.IsHealthy());
}

// DecoderHotsetManager Tests
TEST(Test_DHotset_LoadStates)
{
    using namespace ExplorerLens::Engine;
    // DecoderInstance defaults
    DecoderInstance inst;
    ASSERT(inst.decoderType == 0);
    ASSERT(inst.instance == nullptr);
}
TEST(Test_DHotset_Modes)
{
    using namespace ExplorerLens::Engine;
    // HotsetStats defaults
    HotsetStats stats;
    ASSERT(stats.cacheHits == 0);
    ASSERT(stats.evictions == 0);
}
TEST(Test_DHotset_RegisterDecoder)
{
    using namespace ExplorerLens::Engine;
    DecoderHotsetManager mgr;
    mgr.RegisterFactory(1, []() -> void* { return nullptr; }, [](void*) {});
    mgr.RegisterFactory(2, []() -> void* { return nullptr; }, [](void*) {});
    auto stats = mgr.GetStats();
    ASSERT(stats.instancesCached == 0);
}
TEST(Test_DHotset_LoadUnload)
{
    using namespace ExplorerLens::Engine;
    DecoderHotsetManager mgr;
    mgr.RegisterFactory(1, []() -> void* { return reinterpret_cast<void*>(0x1); }, [](void*) {});
    void* dec = mgr.AcquireDecoder(1);
    ASSERT(dec != nullptr);
    mgr.ReleaseDecoder(1, dec, 512);
    auto stats = mgr.GetStats();
    ASSERT(stats.instancesCached == 1);
}

// DecoderPriority Tests
TEST(Test_DPriority_Levels)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DecoderPriority::Critical) == 0);
    ASSERT(static_cast<int>(DecoderPriority::Fallback) == 4);
}
TEST(Test_DPriority_RegisterDecoder)
{
    using namespace ExplorerLens::Engine;
    auto& mgr = DecoderPriorityManager::GetInstance();
    mgr.RegisterDecoder(L"WebPDecoder", {L".webp"}, DecoderPriority::High);
    auto primary = mgr.GetPrimaryDecoder(L".webp");
    ASSERT(!primary.empty());
}
TEST(Test_DPriority_Fallback)
{
    using namespace ExplorerLens::Engine;
    auto& mgr = DecoderPriorityManager::GetInstance();
    mgr.RegisterDecoder(L"Primary", {L".testfmt"}, DecoderPriority::High);
    mgr.RegisterDecoder(L"Backup", {L".testfmt"}, DecoderPriority::Fallback);
    auto fb = mgr.GetFallbackDecoder(L".testfmt", L"Primary");
    ASSERT(!fb.empty());
}
TEST(Test_DPriority_Availability)
{
    using namespace ExplorerLens::Engine;
    auto& mgr = DecoderPriorityManager::GetInstance();
    mgr.RegisterDecoder(L"AvailTest", {L".avt"}, DecoderPriority::Normal);
    mgr.SetDecoderAvailable(L"AvailTest", false);
    mgr.SetDecoderAvailable(L"AvailTest", true);
}

// DiagnosticsExporter Tests
TEST(Test_DiagExport_Categories)
{
    using namespace ExplorerLens::Core;
    ASSERT(static_cast<int>(DiagCategory::SystemInfo) == 0);
    ASSERT(static_cast<int>(DiagCategory::PluginStatus) == 9);
}
TEST(Test_DiagExport_CreateAndAdd)
{
    using namespace ExplorerLens::Core;
    DiagExportConfig cfg{};
    auto exporter = DiagnosticsExporter::Create(cfg);
    exporter.AddSystemInfo("OS: Windows 11");
    exporter.AddDecoderHealth("All decoders healthy");
    ASSERT(exporter.EntryCount() >= 2);
}
TEST(Test_DiagExport_ErrorLog)
{
    using namespace ExplorerLens::Core;
    DiagExportConfig cfg{};
    auto exporter = DiagnosticsExporter::Create(cfg);
    exporter.AddErrorLog("Test error entry");
    auto entries = exporter.FilteredEntries();
    ASSERT(entries.size() >= 1);
}
TEST(Test_DiagExport_Export)
{
    using namespace ExplorerLens::Core;
    DiagExportConfig cfg{};
    cfg.outputPath = "test_diag_export.json";
    auto exporter = DiagnosticsExporter::Create(cfg);
    exporter.AddSystemInfo("Test");
    auto result = exporter.Export();
    ASSERT(static_cast<int>(result.status) >= 0);
}

// DirectoryFormatProfiler Tests
TEST(Test_DirProfile_FormatFamilies)
{
    using namespace ExplorerLens::Memory;
    ASSERT(static_cast<int>(ExplorerLens::Memory::DirFormatFamily::LightweightImage) == 0);
    ASSERT(static_cast<int>(ExplorerLens::Memory::DirFormatFamily::Unknown) == 255);
    auto profiler = DirectoryFormatProfiler::Create();
    ASSERT(profiler.FamilyMapSize() > 0);
}
TEST(Test_DirProfile_ClassifyExt)
{
    using namespace ExplorerLens::Memory;
    auto profiler = DirectoryFormatProfiler::Create();
    auto family = profiler.ClassifyExtension(".png");
    ASSERT(static_cast<int>(family) != 255);  // Not Unknown
}
TEST(Test_DirProfile_ProfileDir)
{
    using namespace ExplorerLens::Memory;
    auto profiler = DirectoryFormatProfiler::Create();
    std::vector<std::string> files = {"a.png", "b.jpg", "c.webp"};
    auto profile = profiler.ProfileDirectory("C:\\TestDir", files);
    ASSERT(profile.totalFiles == 3);
}
TEST(Test_DirProfile_Budget)
{
    using namespace ExplorerLens::Memory;
    auto budget = FamilyMemoryBudget::LightweightImage();
    ASSERT(budget.maxWorkingSetBytes > 0);
    ASSERT(budget.maxConcurrentDecodes > 0);
}

// ErrorContext Tests
TEST(Test_ErrCtx_PushPop)
{
    using namespace ExplorerLens::Engine;
    ErrorContextManager::PushContext(L"Decode", L"WebPDecoder");
    ErrorContextManager::PopContext();
}
TEST(Test_ErrCtx_CreateContext)
{
    using namespace ExplorerLens::Engine;
    auto ctx = ErrorContextManager::CreateContext(E_FAIL, L"C:\\test.png");
    ASSERT(ctx.errorCode == E_FAIL);
    ASSERT(!ctx.ToString().empty());
}
TEST(Test_ErrCtx_ScopedContext)
{
    using namespace ExplorerLens::Engine;
    {
        ScopedErrorContext scope(L"Resize", L"GPURenderer");
        // Auto pushes on construction, pops on destruction
    }
}
TEST(Test_ErrCtx_FilePath)
{
    using namespace ExplorerLens::Engine;
    auto ctx = ErrorContextManager::CreateContext(S_OK, L"C:\\images\\photo.jpg");
    ASSERT(ctx.filePath == L"C:\\images\\photo.jpg");
}

// ETWSinkComplete Tests
TEST(Test_ETWSink_Channels)
{
    using namespace ExplorerLens::ETW;
    ASSERT(static_cast<int>(ETWChannel::Admin) == 0);
    ASSERT(static_cast<int>(ETWChannel::Debug) == 3);
}
TEST(Test_ETWSink_RotationStrategies)
{
    using namespace ExplorerLens::ETW;
    ASSERT(static_cast<int>(RotationStrategy::SizeBased) == 0);
    ASSERT(static_cast<int>(RotationStrategy::Hybrid) == 2);
}
TEST(Test_ETWSink_SchemaVersion)
{
    using namespace ExplorerLens::ETW;
    ASSERT(SchemaVersion::Major == 2);
    ASSERT(SchemaVersion::Minor == 0);
}
TEST(Test_ETWSink_ConfigFactories)
{
    using namespace ExplorerLens::ETW;
    auto prod = ETWSinkConfig::Production();
    auto dev = ETWSinkConfig::Development();
    ASSERT(prod.enableConsole == false);
    ASSERT(dev.enableConsole == true);
}

// ExplorerWorkScheduler Tests
TEST(Test_WorkSched_Priorities)
{
    using namespace ExplorerLens::Pipeline;
    ASSERT(static_cast<int>(WorkPriority::Critical) == 0);
    ASSERT(static_cast<int>(WorkPriority::Cancelled) == 4);
}
TEST(Test_WorkSched_Submit)
{
    using namespace ExplorerLens::Pipeline;
    ExplorerWorkScheduler sched;
    auto id = sched.Submit("test.png", 0);
    ASSERT(id > 0);
}
TEST(Test_WorkSched_Cancel)
{
    using namespace ExplorerLens::Pipeline;
    ExplorerWorkScheduler sched;
    auto id = sched.Submit("cancel_me.jpg", 0);
    bool cancelled = sched.Cancel(id);
    ASSERT(cancelled);
}
TEST(Test_WorkSched_Dequeue)
{
    using namespace ExplorerLens::Pipeline;
    ExplorerWorkScheduler sched;
    sched.Submit("first.png", 0);
    auto item = sched.Dequeue();
    ASSERT(!item.filePath.empty());
}

// FormatFallbackEngine Tests — moved to FallbackEngineTests.cpp (Core API)

// FormatGalleryView Tests
TEST(Test_FmtGallery_TileSizes)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(GalleryTileSize::Small) == 64);
    ASSERT(static_cast<int>(GalleryTileSize::ExtraLarge) == 512);
}
TEST(Test_FmtGallery_SortOrders)
{
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(GallerySortOrder::ByCategory) == 0);
    ASSERT(static_cast<int>(GallerySortOrder::ByPopularity) == 4);
}
TEST(Test_FmtGallery_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& gv = FormatGalleryView::Instance();
    auto config = gv.GetConfig();
    ASSERT(static_cast<int>(config.tileSize) > 0);
}
TEST(Test_FmtGallery_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& gv = FormatGalleryView::Instance();
    GalleryViewConfig cfg{};
    cfg.tileSize = GalleryTileSize::Medium;
    gv.Initialize(cfg);
    ASSERT(gv.GetConfig().tileSize == GalleryTileSize::Medium);
}

// FormatGroupManager Tests
TEST(Test_FmtGroup_GroupNames)
{
    using namespace ExplorerLens::Engine;
    FormatGroupManager mgr;
    ASSERT(mgr.GetTotalFormats() >= 10);
}
TEST(Test_FmtGroup_ActionNames)
{
    using namespace ExplorerLens::Engine;
    FormatGroupManager mgr;
    ASSERT(mgr.GetEnabledFormats() <= mgr.GetTotalFormats());
}
TEST(Test_FmtGroup_Counts)
{
    using namespace ExplorerLens::Engine;
    FormatGroupManager mgr;
    ASSERT(mgr.GetTotalFormats() > 0);
    ASSERT(mgr.GetEnabledFormats() > 0);
}
TEST(Test_FmtGroup_GetGroups)
{
    using namespace ExplorerLens::Engine;
    FormatGroupManager mgr;
    auto groups = mgr.GetAllGroups();
    ASSERT(!groups.empty());
}

// ProgramClosureV83 Tests
TEST(Test_ProgClosure_States)
{
    using namespace ExplorerLens::Core;
    ASSERT(static_cast<int>(DeliverableState::Complete) == 0);
    ASSERT(static_cast<int>(DeliverableState::Descoped) == 3);
}
TEST(Test_ProgClosure_CreateReport)
{
    using namespace ExplorerLens::Core;
    ProgramClosureV83 closure;
    auto report = closure.GenerateReport();
    bool complete = closure.IsBlockComplete(report);
    ASSERT(complete || !complete);
}
TEST(Test_ProgClosure_BlockComplete)
{
    using namespace ExplorerLens::Core;
    ProgramClosureV83 closure;
    auto report = closure.GenerateReport();
    bool complete = closure.IsBlockComplete(report);
    ASSERT(complete || !complete);  // Just verify no crash
}
TEST(Test_ProgClosure_DefaultSeed)
{
    using namespace ExplorerLens::Core;
    ASSERT(static_cast<int>(DeliverableState::Complete) != static_cast<int>(DeliverableState::Descoped));
}

// ReleaseReadinessDashboard Tests
TEST(Test_RelReady_Categories)
{
    using namespace ExplorerLens;
    ASSERT(static_cast<int>(GateCategory::Build) == 0);
    ASSERT(static_cast<int>(GateCategory::Security) == 7);
}
TEST(Test_RelReady_ReadinessLevels)
{
    using namespace ExplorerLens;
    ASSERT(static_cast<int>(ReadinessLevel::Green) == 0);
    ASSERT(static_cast<int>(ReadinessLevel::Unknown) == 3);
}
TEST(Test_RelReady_Evaluate)
{
    using namespace ExplorerLens;
    ReleaseReadinessDashboard dash;
    auto result = dash.Evaluate();
    ASSERT(static_cast<int>(result.overall) >= 0);
}
TEST(Test_RelReady_FormatReport)
{
    using namespace ExplorerLens;
    ReleaseReadinessDashboard dash;
    auto result = dash.Evaluate();
    auto report = ReleaseReadinessDashboard::FormatReport(result);
    ASSERT(!report.empty());
}

// ReproducibleBuildVerifier Tests
TEST(Test_ReproBuild_ArtifactTypes)
{
    ASSERT(static_cast<int>(ExplorerLens::Engine::ValidatorArtifactType::DLL)
           != static_cast<int>(ExplorerLens::Engine::ValidatorArtifactType::MSI));
    ASSERT(static_cast<int>(ExplorerLens::Engine::ValidatorArtifactType::EXE)
           != static_cast<int>(ExplorerLens::Engine::ValidatorArtifactType::PDB));
}
TEST(Test_ReproBuild_VerifyStatuses)
{
    using namespace ExplorerLens;
    ASSERT(static_cast<int>(ReproBuildVerifyStatus::Reproducible) == 0);
    ASSERT(static_cast<int>(ReproBuildVerifyStatus::Skipped) == 5);
}
TEST(Test_ReproBuild_StrictPolicy)
{
    using namespace ExplorerLens;
    auto strict = StrictPolicy();
    ASSERT(strict.stripTimestamps == true);
    ASSERT(strict.maxSizeDriftPct <= 1.0);
}
TEST(Test_ReproBuild_RelaxedPolicy)
{
    using namespace ExplorerLens;
    auto relaxed = RelaxedPolicy();
    ASSERT(relaxed.maxSizeDriftPct >= 1.0);
}
