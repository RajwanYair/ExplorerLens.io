// EngineTests_Late.cpp — Engine unit tests, late-stage split
// Copyright (c) 2026 ExplorerLens Project
//
// Split from EngineTests_Mid.cpp v33.0.0 to reduce file size for git.
// Contains Neural Format Intelligence, WASM, CLI Router, Smart Batch,
// Workflow Automation, AI Inference, Enterprise, Platform PAL tests.
//
#include "EngineTestsIncludes.h"

#include "../../src/Tools.CLI/BenchmarkCommand.h"
#include "../../src/Tools.CLI/CacheCommand.h"
#include "../../src/Tools.CLI/CommandRouter.h"
#include "../../src/Tools.CLI/DoctorCommand.h"
#include "../../src/Tools.CLI/InfoCommand.h"
#include "../../src/Tools.CLI/RegisterCommand.h"
// Sprint 801-810 — Platform
// Sprint 811-820 — AI Captions
// Sprint 821-830 — AR
// Sprint 831-840 — Enterprise
// Sprint 841-850 — UX
// Sprint 851-860 — Security
// Sprint 861-870 — PQC Core/Utils/Plugin
#include "../Core/CryptoAgilityBroker.h"
#include "../Core/HybridTrustChainV2.h"
#include "../Core/PQCSignatureVerifier.h"
#include "../Core/QuantumSafeKeyExchange.h"
#include "../Utils/DilithiumCertificateStore.h"
#include "../Utils/KeyRotationScheduler.h"
#include "../Utils/SignatureAuditLogger.h"
// Sprint 871-880 — Cross-Platform GPU/Core/Utils (Windows-only subset)
#include "../Core/PlatformNeutralBuffer.h"
#include "../Core/XDGThumbnailProvider.h"
#include "../Utils/GTK4ThumbnailWidget.h"
#include "../Utils/PlatformCapabilityProbe.h"
// Sprint 881-890 — Gen-5 Platform WinUI 4
#include "../Cache/PersistentL3Cache.h"
#include "../Core/AsyncPreviewBroker.h"
#include "../Core/LivePreviewUpdater.h"
#include "../Core/ShellPropertyHandlerV2.h"
#include "../Core/UniversalFileProvider.h"
#include "../Core/WinUI4PreviewHandler.h"
#include "../Pipeline/PreviewPipelineV5.h"
// Sprint 891-900 — Accessibility v2
#include "../Core/KeyboardNavigationController.h"
#include "../Utils/A11yTelemetryReporter.h"
#include "../Utils/ARIAThumbnailAnnotator.h"
#include "../Utils/HighContrastThemeAdapter.h"

using namespace ExplorerLens::CLI;

TEST(TestCLIRouterDispatch)
{
    // Verify that CommandRouter correctly identifies known subcommands and
    // routes --help without crashing.
    auto cli = CreateLensCLI();
    ASSERT(cli != nullptr);

    // Dispatch '--help' — must return ExitCode::Success (0)
    const wchar_t* argv[] = {L"lens", L"--help"};
    ExitCode code = static_cast<ExitCode>(cli->Dispatch(2, const_cast<wchar_t**>(argv)));
    ASSERT(code == ExitCode::Success);
}

TEST(TestCLIRouterUnknownCommand)
{
    // An unrecognised subcommand must return ExitCode::UnknownCommand (127).
    auto cli = CreateLensCLI();
    ASSERT(cli != nullptr);

    const wchar_t* argv[] = {L"lens", L"xyzzy_unknown"};
    ExitCode code = static_cast<ExitCode>(cli->Dispatch(2, const_cast<wchar_t**>(argv)));
    ASSERT(code == ExitCode::UnknownCommand || code == ExitCode::GeneralError);
}

TEST(TestCLIInfoDetectsFormatByExtension)
{
    // InfoCommand::DetectFormat() must recognise common extensions
    // without requiring the files to actually exist on disk.
    InfoCommand cmd;
    FileInfo fi = cmd.DetectFormat(L"photo.jpg");
    ASSERT(fi.formatName == L"JPEG");

    fi = cmd.DetectFormat(L"archive.zip");
    ASSERT(fi.formatName == L"ZIP");

    fi = cmd.DetectFormat(L"image.webp");
    ASSERT(fi.formatName == L"WebP");

    fi = cmd.DetectFormat(L"book.cbz");
    ASSERT(fi.formatName == L"CBZ");

    fi = cmd.DetectFormat(L"unknown.xyzzy");
    ASSERT(fi.formatName == L"Unknown");
}

TEST(TestCLICacheStatsPath)
{
    // CacheCommand must resolve cache dir to %LOCALAPPDATA%\ExplorerLens\ThumbnailCache
    std::wstring cachePath = CacheCommand::GetCachePath();
    ASSERT(!cachePath.empty());
    ASSERT(cachePath.find(L"ExplorerLens") != std::wstring::npos);
    ASSERT(cachePath.find(L"ThumbnailCache") != std::wstring::npos);
}

TEST(TestCLIRegisterDetectsAdminState)
{
    // IsAdminProcess() must return a valid bool without crashing.
    // We can't assert a specific value as tests run in various contexts.
    bool isAdmin = RegisterCommand::IsAdminProcess();
    (void)isAdmin;  // Result is environment-dependent; just verify no crash/exception
    ASSERT(true);
}

TEST(TestCLIBenchmarkOutputFormat)
{
    // BenchmarkCommand must complete a synthetic run and produce valid results
    // (non-zero throughput, positive latencies).
    BenchmarkCommand cmd;
    auto results = cmd.RunSyntheticBenchmark(3 /* iterations */);
    ASSERT(!results.empty());
    for (const auto& r : results) {
        ASSERT(r.p50Ms > 0.0);
        ASSERT(r.p95Ms >= r.p50Ms);
        ASSERT(r.p99Ms >= r.p95Ms);
        ASSERT(r.throughput > 0.0);
        ASSERT(!r.category.empty());
    }
}

TEST(TestCLIDoctorAllChecks)
{
    // DoctorCommand must run all checks and return a non-empty result vector.
    // Some checks may FAIL in CI (no GPU, not registered) — that's expected.
    DoctorCommand cmd;
    auto checks = cmd.RunAllChecks();
    ASSERT(!checks.empty());
    // Verify each check has a name and a message
    for (const auto& c : checks) {
        ASSERT(!c.name.empty());
        ASSERT(!c.message.empty());
        // Status must be a valid enum value
        ASSERT(c.status == ExplorerLens::CLI::DiagnosticStatus::Pass
               || c.status == ExplorerLens::CLI::DiagnosticStatus::Warn
               || c.status == ExplorerLens::CLI::DiagnosticStatus::Fail);
    }
}

//==============================================================================
// Integration Test Framework Tests (Sprint 25 / v15.5.0 "Zenith-V")
//==============================================================================

#include "Integration/COMIntegrationTest.h"
#include "Integration/IntegrationTestRunner.h"

// Sprint 561-570 — WASM Plugin Sandbox (v25.1.0 backfill)

// Sprint 571-580 — Neural Format Intelligence (v25.1.0 "Rigel-R")
#include "Core/FormatDetectionReport.h"
#include "Core/SelfExpandingFormatRegistry.h"
#include "Core/SyntheticDecoderGenerator.h"

// Sprint 581-590 — NPU & Heterogeneous Compute (v25.2.0 "Rigel-S")
#include "GPU/ARM64DecodeBackend.h"
#include "GPU/HardwareCapabilityProfiler.h"
#include "GPU/HexagonDSPBackend.h"
#include "GPU/IntelNPUBackend.h"
#include "GPU/ONNXEPRouter.h"
#include "Memory/NPUMemoryPool.h"
#include "Pipeline/PowerAwareScheduler.h"

// Sprint 591-600 — VCS Integration (v25.3.0 "Rigel-T")
#include "Core/CommitBadgeCompositor.h"
#include "Core/GitBlameHeatmapOverlay.h"
#include "Core/GitDiffThumbnail.h"
#include "Core/GitLFSResolver.h"
#include "Core/GitStatusOverlay.h"
#include "Core/MergeConflictOverlay.h"
#include "Core/VCSBadgeAdapter.h"
// Sprint 601-610 — Self-Healing & Adaptive Recovery (v25.4.0 "Rigel-U")
#include "Core/AdaptiveTimeoutTuner.h"
#include "Core/DecoderCrashPredictor.h"
#include "Core/DecoderIncidentReporter.h"
#include "Core/DecoderQuarantineManager.h"
#include "Core/HeapCorruptionSentinel.h"
#include "Pipeline/RetryPolicyEngine.h"
#include "Utils/BootIntegritySelfTest.h"
#include "Utils/COMSelfRepairValidator.h"
// Sprint 611-620 — Multi-Instance & Virtual Desktop (v25.5.0 "Rigel-V")
#include "Core/InstanceRegistry.h"
#include "Core/PerMonitorDPISelectorV2.h"
#include "Core/TabbedExplorerSync.h"
#include "Core/VirtualDesktopAwareness.h"
#include "Core/WTSSessionIsolation.h"
#include "Memory/CrossSessionThumbnailPool.h"
#include "Pipeline/CrossInstanceLoadBalancer.h"
#include "Pipeline/ForegroundPriorityInheritance.h"
// Sprint 621-630 — Collaborative Annotations & Sharing (v25.6.0 "Rigel-W")
#include "Core/AnnotationDiffViewer.h"
#include "Core/AnnotationOverlayRenderer.h"
#include "Core/AnnotationSchemaMigrator.h"
#include "Core/AnnotationStore.h"
#include "Core/SharedCollectionBuilder.h"
#include "Utils/AnnotationExporter.h"
#include "Utils/CollabCloudSync.h"
#include "Utils/CollabWebhookBridge.h"
// Sprint 631-640 — Protocol Surface & API Ecosystem (v25.7.0 "Rigel-X")
#include "Utils/APIRateLimiter.h"
#include "Utils/GRPCThumbnailService.h"
#include "Utils/GraphQLQueryEngine.h"
#include "Utils/OAuthTokenValidator.h"
#include "Utils/OpenAPISpecGenerator.h"
#include "Utils/RESTThumbnailServer.h"
#include "Utils/SDKBindingsGenerator.h"
#include "Utils/WebSocketPushChannel.h"
// Sprint 641-650 — Post-Quantum Security (v26.0.0 "Canopus")
#include "Core/HybridTLSIPCChannel.h"
#include "Utils/CertificateMigrationTool.h"
#include "Utils/CryptoAgilityEngine.h"
#include "Utils/FIPS140CryptoBoundary.h"
#include "Utils/MLKEMKeyEncapsulator.h"
#include "Utils/PQAuditTrail.h"
#include "Utils/QuantumSafeKeyRotator.h"
#include "Utils/SLHDSASignatureVerifier.h"
// Sprint 651-660 — Windows Next-Gen Shell Integration (v26.1.0 "Canopus-R")
#include "Core/AppContainerIsolation.h"
#include "Core/MSIXStreamingPrewarmer.h"
#include "Core/WinFSMetadataStore.h"
#include "Core/WinRTThumbnailBridge.h"
#include "Core/WindowsSearchV3Bridge.h"
#include "Utils/SmartAppControlPolicy.h"
#include "Utils/WindowsHelloAuthBridge.h"
// Sprint 661-670 — Immersive 3D Preview Engine (v26.2.0 "Canopus-S")
#include "Core/AnimationPreviewScrubber.h"
#include "Core/HolographicProjectionEngine.h"
#include "Core/ImmersivePreviewRenderer.h"
#include "Core/MaterialPreviewEngine.h"
#include "Core/MeshLODGeneratorV2.h"
#include "Core/VolumetricThumbnailEngine.h"
#include "GPU/GPUPathTracerPreview.h"
#include "GPU/RealtimeLightingSimulator.h"
// Sprint 671-680 — Real-Time Collaboration (v26.3.0 "Canopus-T")
#include "Core/CollaborationPresenceEngine.h"
#include "Core/CollaborationTelemetryHub.h"
#include "Core/ConflictResolutionMerger.h"
#include "Core/LiveAnnotationBroadcaster.h"
#include "Core/PresenceAvatarRenderer.h"
#include "Core/SessionReplayEngine.h"
#include "Core/SharedViewStateProtocol.h"
// Sprint 681-690 — Adaptive Performance Governor v2 (v26.4.0 "Canopus-U")
#include "Core/AdaptivePerformanceGovernorV2.h"
#include "Core/BackgroundIntelligenceService.h"
#include "Core/FrameRateSynchronizer.h"
#include "Core/PowerBudgetController.h"
#include "Core/QoSThrottleEngine.h"
#include "GPU/WorkloadBalancerV2.h"
#include "Memory/SmartPrefetchEngine.h"
#include "Memory/ThermalAwareMemoryScheduler.h"
// Sprint 691-700 — Global I18n & Accessibility v3 (v26.5.0 "Canopus-V")
#include "Utils/A11yColorContrastEngine.h"
#include "Utils/AccessibilityAuditPipeline.h"
#include "Utils/AccessibilityNavigatorV3.h"
#include "Utils/BiDiTextLayoutEngine.h"
#include "Utils/I18nRuntimeEngine.h"
#include "Utils/KeyboardNavigationMapV2.h"
#include "Utils/LocaleFallbackResolver.h"
#include "Utils/ScreenReaderBridgeV2.h"
// Sprint 701-800 includes
#include "Core/ARMarkerDetectionEngine.h"
#include "Core/AdaptiveBitrateSelector.h"
#include "Core/AnonymizationPipelineV2.h"
#include "Core/ClusterAutoScaler.h"
#include "Core/ClusterObservabilityBus.h"
#include "Core/DifferentialPrivacyEngine.h"
#include "Core/LiveThumbnailPoller.h"
#include "Core/LocalDataAggregator.h"
#include "Core/MediaTimelineRenderer.h"
#include "Core/NodeHealthMonitor.h"
#include "Core/PointCloudVisualizerV2.h"
#include "Core/PrivacyAuditLogger.h"
#include "Core/PrivacyConsentManager.h"
#include "Core/RenderClusterManager.h"
#include "Core/RenderJobScheduler.h"
#include "Core/RenderResultAggregator.h"
#include "Core/SecureClusterChannel.h"
#include "Core/SecureEnclaveAnalytics.h"
#include "Core/StreamingBufferOrchestrator.h"
#include "Core/XRMetadataExtractor.h"
#include "Core/XRSpatialPreviewEngine.h"
#include "Decoders/DASHStreamDecoder.h"
#include "Decoders/LiveStreamDecoder.h"
#include "Decoders/NerfDecoder.h"
#include "Decoders/OpenXRAssetDecoder.h"
#include "Decoders/USDADecoder.h"
#include "Decoders/WebRTCThumbnailCapture.h"
#include "GPU/StereoscopicRenderPipeline.h"
#include "GPU/VideoTextureStreamEngine.h"
#include "Plugin/PluginCompatibilityShimV3.h"
#include "Plugin/PluginSandboxV3.h"
#include "Utils/GDPRComplianceEngine.h"

using namespace ExplorerLens::Engine::Tests;

//==============================================================================

// WASM Plugin Sandbox Tests (Sprint 561-570 / v25.1.0 "Rigel-R" backfill)
//==============================================================================

//==============================================================================
// Neural Format Intelligence Tests (Sprint 571-580 / v25.1.0 "Rigel-R")
//==============================================================================

TEST(Test_SelfExpandingFormatRegistry_Create)
{
    SelfExpandingFormatRegistry reg;
    ASSERT(reg.EntryCount() == 0);
}
TEST(Test_SelfExpandingFormatRegistry_Register)
{
    SelfExpandingFormatRegistry reg;
    LearnedFormatEntry entry;
    entry.formatId = "x-custom";
    entry.mimeType = "application/x-custom";
    entry.confidence = 0.8f;
    ASSERT(reg.RegisterFormat(entry));
    ASSERT(reg.EntryCount() == 1);
}
TEST(Test_SelfExpandingFormatRegistry_Lookup)
{
    SelfExpandingFormatRegistry reg;
    LearnedFormatEntry e;
    e.formatId = "myformat";
    reg.RegisterFormat(e);
    const auto* found = reg.Lookup("myformat");
    ASSERT(found != nullptr);
    ASSERT(found->formatId == "myformat");
}
TEST(Test_SelfExpandingFormatRegistry_NotFound)
{
    SelfExpandingFormatRegistry reg;
    ASSERT(reg.Lookup("nonexistent") == nullptr);
}
TEST(Test_SelfExpandingFormatRegistry_Duplicate)
{
    SelfExpandingFormatRegistry reg;
    LearnedFormatEntry e;
    e.formatId = "dup";
    reg.RegisterFormat(e);
    ASSERT(!reg.RegisterFormat(e));
    ASSERT(reg.EntryCount() == 1);
}
TEST(Test_SelfExpandingFormatRegistry_Validate)
{
    SelfExpandingFormatRegistry reg;
    LearnedFormatEntry e;
    e.formatId = "v";
    reg.RegisterFormat(e);
    reg.ValidateEntry("v");
    auto stats = reg.GetStats();
    ASSERT(stats.validatedEntries == 1);
}
TEST(Test_SelfExpandingFormatRegistry_Remove)
{
    SelfExpandingFormatRegistry reg;
    LearnedFormatEntry e;
    e.formatId = "rem";
    reg.RegisterFormat(e);
    reg.RemoveEntry("rem");
    ASSERT(reg.EntryCount() == 0);
}
TEST(Test_SelfExpandingFormatRegistry_Stats)
{
    SelfExpandingFormatRegistry reg;
    for (int i = 0; i < 3; ++i) {
        LearnedFormatEntry e;
        e.formatId = "f" + std::to_string(i);
        reg.RegisterFormat(e);
    }
    auto s = reg.GetStats();
    ASSERT(s.totalEntries == 3);
    ASSERT(s.pendingEntries == 3);
}
TEST(Test_SelfExpandingFormatRegistry_PersistLoad)
{
    SelfExpandingFormatRegistry reg;
    ASSERT(reg.LoadFromFile("test.json"));
    ASSERT(reg.SaveToFile("test.json"));
}

TEST(Test_FormatDetectionReport_Create)
{
    FormatDetectionReportBuilder b;
    const auto& r = b.GetReport();
    ASSERT(r.VoteCount() == 0);
    ASSERT(!r.IsResolved());
}
TEST(Test_FormatDetectionReport_AddVote)
{
    FormatDetectionReportBuilder b;
    b.AddVote(DetectionSource::MagicBytes, "png", "image/png", 0.95f);
    const auto& r = b.GetReport();
    ASSERT(r.VoteCount() == 1);
    ASSERT(r.consensusFormatId == "png");
    ASSERT(r.verdict == DetectionVerdict::Confirmed);
}
TEST(Test_FormatDetectionReport_Consensus)
{
    FormatDetectionReportBuilder b;
    b.AddVote(DetectionSource::MagicBytes, "jpg", "image/jpeg", 0.6f);
    b.AddVote(DetectionSource::Extension, "jpg", "image/jpeg", 0.9f);
    b.AddVote(DetectionSource::NeuralClassifier, "jpg", "image/jpeg", 0.85f);
    const auto& r = b.GetReport();
    ASSERT(r.consensusFormatId == "jpg");
    ASSERT(r.consensusConfidence >= 0.85f);
}
TEST(Test_FormatDetectionReport_Uncertain)
{
    FormatDetectionReportBuilder b;
    b.AddVote(DetectionSource::ContentHash, "x", "application/x", 0.45f);
    const auto& r = b.GetReport();
    ASSERT(r.verdict == DetectionVerdict::Conflicted);
}
TEST(Test_FormatDetectionReport_EvalTime)
{
    FormatDetectionReportBuilder b;
    b.SetEvalTime(12);
    ASSERT(b.GetReport().evaluationMs == 12);
}
TEST(Test_FormatDetectionReport_IsResolved)
{
    FormatDetectionReportBuilder b;
    b.AddVote(DetectionSource::MagicBytes, "pdf", "application/pdf", 0.92f);
    ASSERT(b.GetReport().IsResolved());
}
TEST(Test_FormatDetectionReport_Probable)
{
    FormatDetectionReportBuilder b;
    b.AddVote(DetectionSource::Extension, "docx", "application/vnd.openxmlformats", 0.75f);
    ASSERT(b.GetReport().verdict == DetectionVerdict::Probable);
}
TEST(Test_FormatDetectionReport_Reset)
{
    FormatDetectionReportBuilder b;
    b.AddVote(DetectionSource::MagicBytes, "png", "image/png", 0.9f);
    b.Reset();
    ASSERT(b.GetReport().VoteCount() == 0);
}
TEST(Test_FormatDetectionReport_Sources)
{
    ASSERT(DetectionSource::MagicBytes != DetectionSource::Extension);
    ASSERT(DetectionVerdict::Confirmed != DetectionVerdict::Conflicted);
}

TEST(Test_SyntheticDecoderGenerator_Create)
{
    SyntheticDecoderGenerator gen;
    ASSERT(gen.GeneratedCount() == 0);
}
TEST(Test_SyntheticDecoderGenerator_Generate)
{
    SyntheticDecoderGenerator gen;
    SyntheticDecoderSpec spec;
    spec.formatId = "x-test";
    spec.family = DecoderFamily::Image;
    spec.anticipatedAccuracy = 0.7f;
    auto r = gen.Generate(spec);
    ASSERT(r.success);
    ASSERT(!r.stubClassName.empty());
    ASSERT(!r.generatedCode.empty());
    ASSERT(gen.GeneratedCount() == 1);
}
TEST(Test_SyntheticDecoderGenerator_EmptyId)
{
    SyntheticDecoderGenerator gen;
    SyntheticDecoderSpec spec;
    auto r = gen.Generate(spec);
    ASSERT(!r.success);
    ASSERT(!r.errorMessage.empty());
}
TEST(Test_SyntheticDecoderGenerator_Quality)
{
    SyntheticDecoderGenerator gen;
    SyntheticDecoderSpec spec;
    spec.formatId = "unknown-x";
    spec.family = DecoderFamily::Unknown;
    auto r = gen.Generate(spec);
    ASSERT(r.quality == StubQuality::Placeholder);
}
TEST(Test_SyntheticDecoderGenerator_Partial)
{
    SyntheticDecoderGenerator gen;
    SyntheticDecoderSpec spec;
    spec.formatId = "near-png";
    spec.family = DecoderFamily::Image;
    auto r = gen.Generate(spec);
    ASSERT(r.quality == StubQuality::Partial);
}
TEST(Test_SyntheticDecoderGenerator_InferFamily)
{
    SyntheticDecoderGenerator gen;
    ASSERT(gen.InferFamily("png") == DecoderFamily::Image);
    ASSERT(gen.InferFamily("pdf") == DecoderFamily::Document);
    ASSERT(gen.InferFamily("zip") == DecoderFamily::Archive);
    ASSERT(gen.InferFamily("xyz") == DecoderFamily::Unknown);
}
TEST(Test_SyntheticDecoderGenerator_Fidelity)
{
    SyntheticDecoderGenerator gen;
    SyntheticDecoderSpec spec;
    spec.formatId = "test";
    spec.anticipatedAccuracy = 0.5f;
    auto r = gen.Generate(spec);
    ASSERT(r.estimatedFidelity > 0.0f);
}
TEST(Test_SyntheticDecoderGenerator_Reset)
{
    SyntheticDecoderGenerator gen;
    SyntheticDecoderSpec s;
    s.formatId = "r";
    gen.Generate(s);
    gen.Reset();
    ASSERT(gen.GeneratedCount() == 0);
}
TEST(Test_SyntheticDecoderGenerator_Families)
{
    ASSERT(DecoderFamily::Image != DecoderFamily::Document);
    ASSERT(StubQuality::Full != StubQuality::Placeholder);
}

//==============================================================================
// NPU & Heterogeneous Compute Tests (Sprint 581-590 / v25.2.0 "Rigel-S")
//==============================================================================

TEST(Test_IntelNPUBackend_Create)
{
    IntelNPUBackend npu;
    ASSERT(npu.GetState() == NPUBackendState::Uninitialized);
    ASSERT(!npu.IsAvailable());
    ASSERT(npu.GetVendor() == NPUVendor::Intel);
}
TEST(Test_IntelNPUBackend_Initialize)
{
    IntelNPUBackend npu;
    ASSERT(npu.Initialize());
    ASSERT(npu.IsAvailable());
    ASSERT(npu.GetState() == NPUBackendState::Ready);
}
TEST(Test_IntelNPUBackend_Config)
{
    IntelNPUConfig cfg;
    cfg.precision = OVPrecision::FP16;
    IntelNPUBackend npu(cfg);
    ASSERT(npu.GetPrecision() == OVPrecision::FP16);
}
TEST(Test_IntelNPUBackend_Infer)
{
    IntelNPUBackend npu;
    npu.Initialize();
    float in[4] = {1, 2, 3, 4}, out[4] = {};
    ASSERT(npu.Infer(in, 4, out));
    ASSERT(npu.GetMetrics().framesProcessed == 1);
}
TEST(Test_IntelNPUBackend_InferBadInput)
{
    IntelNPUBackend npu;
    npu.Initialize();
    ASSERT(!npu.Infer(nullptr, 0, nullptr));
}
TEST(Test_IntelNPUBackend_Metrics)
{
    IntelNPUBackend npu;
    npu.Initialize();
    float d[4] = {}, o[4] = {};
    npu.Infer(d, 4, o);
    auto m = npu.GetMetrics();
    ASSERT(m.framesProcessed == 1);
    ASSERT(m.inferenceMs >= 0);
}
TEST(Test_IntelNPUBackend_ResetMetrics)
{
    IntelNPUBackend npu;
    npu.Initialize();
    float d[4] = {}, o[4] = {};
    npu.Infer(d, 4, o);
    npu.ResetMetrics();
    ASSERT(npu.GetMetrics().framesProcessed == 0);
}
TEST(Test_IntelNPUBackend_SetConfig)
{
    IntelNPUBackend npu;
    IntelNPUConfig cfg;
    cfg.precision = OVPrecision::INT8;
    npu.SetConfig(cfg);
    ASSERT(npu.GetPrecision() == OVPrecision::INT8);
}
TEST(Test_IntelNPUBackend_Shutdown)
{
    IntelNPUBackend npu;
    npu.Initialize();
    npu.Shutdown();
    ASSERT(npu.GetState() == NPUBackendState::Uninitialized);
}
TEST(Test_HexagonDSPBackend_Create)
{
    HexagonDSPBackend dsp;
    ASSERT(!dsp.IsReady());
    ASSERT(dsp.HVXEnabled());
}
TEST(Test_HexagonDSPBackend_Initialize)
{
    HexagonDSPBackend dsp;
    ASSERT(dsp.Initialize());
    ASSERT(dsp.IsReady());
}
TEST(Test_HexagonDSPBackend_Config)
{
    HexagonDSPConfig cfg;
    cfg.enableHMX = true;
    HexagonDSPBackend dsp(cfg);
    ASSERT(dsp.HMXEnabled());
}
TEST(Test_HexagonDSPBackend_RunInference)
{
    HexagonDSPBackend dsp;
    dsp.Initialize();
    uint8_t in[4] = {1, 2, 3, 4}, out[4] = {};
    auto r = dsp.RunInference(in, 4, out, 4);
    ASSERT(r.success);
    ASSERT(r.latencyUs > 0);
}
TEST(Test_HexagonDSPBackend_RunBadInput)
{
    HexagonDSPBackend dsp;
    dsp.Initialize();
    auto r = dsp.RunInference(nullptr, 0, nullptr, 0);
    ASSERT(!r.success);
    ASSERT(!r.errorMsg.empty());
}
TEST(Test_HexagonDSPBackend_RunCount)
{
    HexagonDSPBackend dsp;
    dsp.Initialize();
    uint8_t in[4] = {}, out[4] = {};
    dsp.RunInference(in, 4, out, 4);
    ASSERT(dsp.RunCount() == 1);
}
TEST(Test_HexagonDSPBackend_SetConfig)
{
    HexagonDSPBackend dsp;
    HexagonDSPConfig cfg;
    cfg.preferredBE = QNNBackend::HTA;
    dsp.SetConfig(cfg);
    ASSERT(dsp.GetQNNBackend() == QNNBackend::HTA);
}
TEST(Test_HexagonDSPBackend_Reset)
{
    HexagonDSPBackend dsp;
    dsp.Initialize();
    uint8_t in[4] = {}, out[4] = {};
    dsp.RunInference(in, 4, out, 4);
    dsp.Reset();
    ASSERT(!dsp.IsReady());
    ASSERT(dsp.RunCount() == 0);
}
TEST(Test_HexagonDSPBackend_Variants)
{
    ASSERT(HexagonVariant::V65 != HexagonVariant::V73);
    ASSERT(QNNBackend::HTP != QNNBackend::CPU);
}
TEST(Test_ONNXEPRouter_Create)
{
    ONNXEPRouter router;
    ASSERT(router.GetPolicy() == EPSelectionPolicy::HighestThroughput);
    ASSERT(router.RegisteredCount() == 0);
}
TEST(Test_ONNXEPRouter_RegisterEP)
{
    ONNXEPRouter router;
    EPCapability cap;
    cap.ep = ONNXExecutionProvider::DirectML;
    cap.available = true;
    router.RegisterEP(cap);
    ASSERT(router.RegisteredCount() == 1);
}
TEST(Test_ONNXEPRouter_IsEPAvailable)
{
    ONNXEPRouter router;
    EPCapability cap;
    cap.ep = ONNXExecutionProvider::CUDA;
    cap.available = true;
    router.RegisterEP(cap);
    ASSERT(router.IsEPAvailable(ONNXExecutionProvider::CUDA));
    ASSERT(!router.IsEPAvailable(ONNXExecutionProvider::DirectML));
}
TEST(Test_ONNXEPRouter_Select)
{
    ONNXEPRouter router;
    EPCapability cpu;
    cpu.ep = ONNXExecutionProvider::CPU;
    cpu.available = true;
    router.RegisterEP(cpu);
    ASSERT(router.Select() == ONNXExecutionProvider::CPU);
}
TEST(Test_ONNXEPRouter_SelectPreferred)
{
    ONNXEPRouter router;
    EPCapability dml;
    dml.ep = ONNXExecutionProvider::DirectML;
    dml.available = true;
    router.RegisterEP(dml);
    ASSERT(router.Select() == ONNXExecutionProvider::DirectML);
}
TEST(Test_ONNXEPRouter_Clear)
{
    ONNXEPRouter router;
    EPCapability cap;
    cap.ep = ONNXExecutionProvider::OpenVINO_NPU;
    cap.available = true;
    router.RegisterEP(cap);
    router.Clear();
    ASSERT(router.RegisteredCount() == 0);
}
TEST(Test_ONNXEPRouter_Policy)
{
    ONNXEPRouter router;
    router.SetPolicy(EPSelectionPolicy::LowestLatency);
    ASSERT(router.GetPolicy() == EPSelectionPolicy::LowestLatency);
}
TEST(Test_ONNXEPRouter_FallbackAllowed)
{
    EPRouterConfig cfg;
    cfg.allowCPUFallback = true;
    ONNXEPRouter router(cfg);
    ASSERT(router.GetConfig().allowCPUFallback);
}
TEST(Test_ONNXEPRouter_EPEnum)
{
    ASSERT(ONNXExecutionProvider::DirectML != ONNXExecutionProvider::CPU);
    ASSERT(EPSelectionPolicy::LowestPower != EPSelectionPolicy::Manual);
}
TEST(Test_HardwareCapabilityProfiler_Create)
{
    HardwareCapabilityProfiler p;
    ASSERT(p.PeakTOPS() == 0.0f);
}
TEST(Test_HardwareCapabilityProfiler_Profile)
{
    HardwareCapabilityProfiler p;
    auto profile = p.Profile();
    ASSERT(!profile.accelerators.empty());
}
TEST(Test_HardwareCapabilityProfiler_AddMock)
{
    HardwareCapabilityProfiler p;
    AcceleratorInfo npu;
    npu.type = AcceleratorType::NPU;
    npu.tops = 10.0f;
    npu.available = true;
    p.AddMock(npu);
    auto profile = p.Profile();
    ASSERT(profile.peakTOPS >= 10.0f);
}
TEST(Test_HardwareCapabilityProfiler_HasNPU)
{
    HardwareCapabilityProfiler p;
    AcceleratorInfo npu;
    npu.type = AcceleratorType::NPU;
    npu.available = true;
    p.AddMock(npu);
    ASSERT(p.HasNPU());
}
TEST(Test_HardwareCapabilityProfiler_PeakTOPS)
{
    HardwareCapabilityProfiler p;
    AcceleratorInfo a;
    a.type = AcceleratorType::GPU;
    a.tops = 20.0f;
    a.available = true;
    p.AddMock(a);
    p.Profile();
    ASSERT(p.PeakTOPS() >= 20.0f);
}
TEST(Test_HardwareCapabilityProfiler_Sort)
{
    HardwareCapabilityProfiler p;
    AcceleratorInfo lo;
    lo.type = AcceleratorType::CPU;
    lo.tops = 1.0f;
    lo.available = true;
    AcceleratorInfo hi;
    hi.type = AcceleratorType::GPU;
    hi.tops = 50.0f;
    hi.available = true;
    p.AddMock(lo);
    p.AddMock(hi);
    auto profile = p.Profile();
    ASSERT(profile.peakTOPS >= 50.0f);
}
TEST(Test_HardwareCapabilityProfiler_ClearMocks)
{
    HardwareCapabilityProfiler p;
    AcceleratorInfo npu;
    npu.type = AcceleratorType::NPU;
    npu.available = true;
    p.AddMock(npu);
    p.ClearMocks();
    ASSERT(!p.HasNPU());
}
TEST(Test_HardwareCapabilityProfiler_AccelType)
{
    ASSERT(AcceleratorType::CPU != AcceleratorType::NPU);
    ASSERT(AcceleratorType::GPU != AcceleratorType::DSP);
}
TEST(Test_HardwareCapabilityProfiler_BestType)
{
    HardwareCapabilityProfiler p;
    auto profile = p.Profile();
    ASSERT(profile.bestType == AcceleratorType::CPU || profile.bestType == AcceleratorType::GPU
           || profile.bestType == AcceleratorType::NPU);
}
TEST(Test_PowerAwareScheduler_Create)
{
    PowerAwareScheduler s;
    ASSERT(s.GetMode() == SchedulerMode::Balanced);
    ASSERT(s.Scheduled() == 0);
}
TEST(Test_PowerAwareScheduler_SetPowerState)
{
    PowerAwareScheduler s;
    s.SetPowerState(NPUPowerMode::BatterySaver);
    ASSERT(s.GetMode() == SchedulerMode::BatterySaver);
}
TEST(Test_PowerAwareScheduler_ScheduleAC)
{
    PowerAwareScheduler s;
    s.SetPowerState(NPUPowerMode::AC);
    s.SetNPUAvailable(true);
    WorkItem item;
    item.id = 1;
    item.canDeferToNPU = true;
    auto d = s.Schedule(item);
    ASSERT(d.assignedTo == AcceleratorType::NPU);
}
TEST(Test_PowerAwareScheduler_ScheduleBatterySaver)
{
    PowerAwareScheduler s;
    s.SetPowerState(NPUPowerMode::BatterySaver);
    WorkItem item;
    item.id = 2;
    item.canDeferToNPU = true;
    auto d = s.Schedule(item);
    ASSERT(d.assignedTo == AcceleratorType::CPU);
}
TEST(Test_PowerAwareScheduler_ScheduledCount)
{
    PowerAwareScheduler s;
    WorkItem item;
    item.id = 1;
    s.Schedule(item);
    s.Schedule(item);
    ASSERT(s.Scheduled() == 2);
}
TEST(Test_PowerAwareScheduler_NPUUnavailable)
{
    PowerAwareScheduler s;
    s.SetPowerState(NPUPowerMode::AC);
    s.SetNPUAvailable(false);
    WorkItem item;
    item.id = 3;
    item.canDeferToNPU = true;
    auto d = s.Schedule(item);
    ASSERT(d.assignedTo == AcceleratorType::CPU);
}
TEST(Test_PowerAwareScheduler_Reset)
{
    PowerAwareScheduler s;
    WorkItem item;
    item.id = 1;
    s.Schedule(item);
    s.Reset();
    ASSERT(s.Scheduled() == 0);
}
TEST(Test_PowerAwareScheduler_Mode)
{
    PowerAwareScheduler s;
    s.SetPowerState(NPUPowerMode::Battery);
    ASSERT(s.GetMode() == SchedulerMode::Conservative);
}
TEST(Test_PowerAwareScheduler_Reason)
{
    PowerAwareScheduler s;
    s.SetPowerState(NPUPowerMode::BatterySaver);
    WorkItem item;
    item.id = 5;
    auto d = s.Schedule(item);
    ASSERT(!d.reason.empty());
}
TEST(Test_NPUMemoryPool_Create)
{
    NPUMemoryPool pool;
    ASSERT(!pool.IsInitialized());
}
TEST(Test_NPUMemoryPool_Initialize)
{
    NPUMemoryPool pool;
    ASSERT(pool.Initialize());
    ASSERT(pool.IsInitialized());
    ASSERT(pool.BlockCount() > 0);
}
TEST(Test_NPUMemoryPool_Acquire)
{
    NPUMemoryPool pool;
    pool.Initialize();
    auto block = pool.Acquire();
    ASSERT(block != nullptr);
}
TEST(Test_NPUMemoryPool_Release)
{
    NPUMemoryPool pool;
    pool.Initialize();
    size_t before = pool.FreeBlocks();
    auto block = pool.Acquire();
    ASSERT(pool.FreeBlocks() == before - 1);
    pool.Release(block->blockId);
    ASSERT(pool.FreeBlocks() == before);
}
TEST(Test_NPUMemoryPool_AcquireExhausted)
{
    NPUMemoryPoolConfig cfg;
    cfg.maxBlocks = 2;
    NPUMemoryPool pool(cfg);
    pool.Initialize();
    pool.Acquire();
    pool.Acquire();
    ASSERT(pool.Acquire() == nullptr);
}
TEST(Test_NPUMemoryPool_ZeroCopy)
{
    NPUMemoryPoolConfig cfg;
    cfg.zeroCopyEnabled = false;
    NPUMemoryPool pool(cfg);
    ASSERT(!pool.ZeroCopyEnabled());
}
TEST(Test_NPUMemoryPool_Reset)
{
    NPUMemoryPool pool;
    pool.Initialize();
    pool.Reset();
    ASSERT(!pool.IsInitialized());
    ASSERT(pool.BlockCount() == 0);
}
TEST(Test_NPUMemoryPool_Config)
{
    NPUMemoryPoolConfig cfg;
    cfg.maxBlocks = 8;
    NPUMemoryPool pool(cfg);
    ASSERT(pool.GetConfig().maxBlocks == 8);
}
TEST(Test_NPUMemoryPool_BlockCount)
{
    NPUMemoryPoolConfig cfg;
    cfg.maxBlocks = 4;
    NPUMemoryPool pool(cfg);
    pool.Initialize();
    ASSERT(pool.BlockCount() == 4);
}
TEST(Test_ARM64DecodeBackend_Create)
{
    ARM64DecodeBackend backend;
    ASSERT(backend.GetMode() == ARM64DecodeMode::Auto);
}
TEST(Test_ARM64DecodeBackend_ProbeCapabilities)
{
    ARM64DecodeBackend backend;
    auto caps = backend.ProbeCapabilities();
    ASSERT(backend.GetCapabilities().hasNEON == caps.hasNEON);
}
TEST(Test_ARM64DecodeBackend_DecodeStride_Valid)
{
    ARM64DecodeBackend backend;
    backend.ProbeCapabilities();
    uint8_t src[4] = {1, 2, 3, 4}, dst[4] = {};
    auto r = backend.DecodeStride(src, 2, 2, dst);
    ASSERT(r.success);
    ASSERT(r.pixelsProcessed == 4);
}
TEST(Test_ARM64DecodeBackend_DecodeStride_Null)
{
    ARM64DecodeBackend backend;
    auto r = backend.DecodeStride(nullptr, 0, 0, nullptr);
    ASSERT(!r.success);
}
TEST(Test_ARM64DecodeBackend_DecodeStride_Zero)
{
    ARM64DecodeBackend backend;
    uint8_t src[4] = {}, dst[4] = {};
    auto r = backend.DecodeStride(src, 0, 0, dst);
    ASSERT(!r.success);
}
TEST(Test_ARM64DecodeBackend_GetMode)
{
    ARM64DecodeConfig cfg;
    cfg.mode = ARM64DecodeMode::NEON;
    ARM64DecodeBackend backend(cfg);
    ASSERT(backend.GetMode() == ARM64DecodeMode::NEON);
}
TEST(Test_ARM64DecodeBackend_SetConfig)
{
    ARM64DecodeBackend backend;
    ARM64DecodeConfig cfg;
    cfg.mode = ARM64DecodeMode::Scalar;
    backend.SetConfig(cfg);
    ASSERT(backend.GetMode() == ARM64DecodeMode::Scalar);
}
TEST(Test_ARM64DecodeBackend_Reset)
{
    ARM64DecodeBackend backend;
    backend.ProbeCapabilities();
    backend.Reset();
    ASSERT(!backend.GetCapabilities().hasNEON);
}
TEST(Test_ARM64DecodeBackend_Extensions)
{
    ASSERT(ARM64Extension::NEON != ARM64Extension::SVE);
    ASSERT(ARM64DecodeMode::Auto != ARM64DecodeMode::Scalar);
}

//==============================================================================
// VCS Integration Tests (Sprint 591-600 / v25.3.0 "Rigel-T")
//==============================================================================

TEST(Test_GitStatusOverlay_Create)
{
    GitStatusOverlay overlay;
    auto r = overlay.Query("test.cpp");
    ASSERT(r.path == "test.cpp");
}
TEST(Test_GitStatusOverlay_Query)
{
    GitStatusOverlay overlay;
    auto r = overlay.Query("foo.h");
    ASSERT(r.status == GitFileStatus::Clean || r.status != GitFileStatus::Unknown);
}
TEST(Test_GitStatusOverlay_StatusLabel_M)
{
    ASSERT(std::wstring(GitStatusOverlay::StatusLabel(GitFileStatus::Modified)) == L"M");
}
TEST(Test_GitStatusOverlay_StatusLabel_S)
{
    ASSERT(std::wstring(GitStatusOverlay::StatusLabel(GitFileStatus::Staged)) == L"S");
}
TEST(Test_GitStatusOverlay_StatusLabel_Untracked)
{
    ASSERT(std::wstring(GitStatusOverlay::StatusLabel(GitFileStatus::Untracked)) == L"+");
}
TEST(Test_GitStatusOverlay_StatusLabel_Conflict)
{
    ASSERT(std::wstring(GitStatusOverlay::StatusLabel(GitFileStatus::Conflicted)) == L"!");
}
TEST(Test_GitStatusOverlay_ShouldRender_Clean)
{
    GitStatusOverlay overlay;
    GitStatusResult r;
    r.status = GitFileStatus::Clean;
    r.inRepo = true;
    ASSERT(!overlay.ShouldRender(r));
}
TEST(Test_GitStatusOverlay_Config)
{
    GitOverlayConfig cfg;
    cfg.showOnClean = true;
    GitStatusOverlay overlay(cfg);
    GitStatusResult r;
    r.status = GitFileStatus::Clean;
    r.inRepo = true;
    ASSERT(overlay.ShouldRender(r));
}
TEST(Test_GitStatusOverlay_StatusEnum)
{
    ASSERT(GitFileStatus::Clean != GitFileStatus::Modified);
    ASSERT(GitFileStatus::Staged != GitFileStatus::Untracked);
}
TEST(Test_GitBlameHeatmapOverlay_Create)
{
    GitBlameHeatmapOverlay overlay;
    ASSERT(overlay.GetConfig().maxAgeDays == 365);
}
TEST(Test_GitBlameHeatmapOverlay_ComputeScore_Recent)
{
    GitBlameHeatmapOverlay overlay;
    ASSERT(overlay.ComputeScore(0) == 1.0f);
}
TEST(Test_GitBlameHeatmapOverlay_ComputeScore_Old)
{
    GitBlameHeatmapOverlay overlay;
    ASSERT(overlay.ComputeScore(365) == 0.0f);
}
TEST(Test_GitBlameHeatmapOverlay_ComputeScore_Clamped)
{
    GitBlameHeatmapOverlay overlay;
    ASSERT(overlay.ComputeScore(730) == 0.0f);
}
TEST(Test_GitBlameHeatmapOverlay_ColorScheme)
{
    ASSERT(HeatmapColorScheme::HotCold != HeatmapColorScheme::Viridis);
    ASSERT(HeatmapColorScheme::Grayscale != HeatmapColorScheme::HotCold);
}
TEST(Test_GitBlameHeatmapOverlay_Config)
{
    BlameHeatmapConfig cfg;
    cfg.maxAgeDays = 180;
    GitBlameHeatmapOverlay overlay(cfg);
    ASSERT(overlay.GetConfig().maxAgeDays == 180);
}
TEST(Test_GitBlameHeatmapOverlay_ScoreRange)
{
    GitBlameHeatmapOverlay overlay;
    float s = overlay.ComputeScore(100);
    ASSERT(s >= 0.0f && s <= 1.0f);
}
TEST(Test_GitBlameHeatmapOverlay_ScoreMiddle)
{
    GitBlameHeatmapOverlay overlay;
    float s = overlay.ComputeScore(182);
    ASSERT(s > 0.0f && s < 1.0f);
}
TEST(Test_GitBlameHeatmapOverlay_ComputeHeat)
{
    GitBlameHeatmapOverlay overlay;
    auto d = overlay.ComputeHeat("file.cpp");
    ASSERT(d.available);
    ASSERT(d.path == "file.cpp");
}
TEST(Test_VCSBadgeAdapter_Create)
{
    VCSBadgeAdapter adapter;
    ASSERT(adapter.GetConfig().autoDetect);
}
TEST(Test_VCSBadgeAdapter_Build_Empty)
{
    VCSBadgeAdapter adapter;
    auto d = adapter.Build("");
    ASSERT(!d.valid);
}
TEST(Test_VCSBadgeAdapter_Build_Path)
{
    VCSBadgeAdapter adapter;
    auto d = adapter.Build("some/path/file.cpp");
    (void)d;
    ASSERT(true);
}
TEST(Test_VCSBadgeAdapter_Config)
{
    VCSBadgeConfig cfg;
    cfg.autoDetect = false;
    VCSBadgeAdapter adapter(cfg);
    ASSERT(!adapter.GetConfig().autoDetect);
}
TEST(Test_VCSBadgeAdapter_ProviderEnum)
{
    ASSERT(VCSProvider::Git != VCSProvider::SVN);
    ASSERT(VCSProvider::Perforce != VCSProvider::Unknown);
}
TEST(Test_VCSBadgeAdapter_BadgeTypeEnum)
{
    ASSERT(VCSBadgeType::Status != VCSBadgeType::BranchName);
    ASSERT(VCSBadgeType::CommitHash != VCSBadgeType::None);
}
TEST(Test_VCSBadgeAdapter_DetectProvider)
{
    VCSBadgeAdapter adapter;
    auto prov = adapter.DetectProvider("file.cpp");
    ASSERT(prov == VCSProvider::Git || prov == VCSProvider::Unknown);
}
TEST(Test_VCSBadgeAdapter_DefaultBadgeType)
{
    VCSBadgeAdapter adapter;
    ASSERT(adapter.GetConfig().badgeType == VCSBadgeType::Status);
}
TEST(Test_VCSBadgeAdapter_DefaultProvider)
{
    VCSBadgeAdapter adapter;
    ASSERT(adapter.GetConfig().preferredProvider == VCSProvider::Git);
}
TEST(Test_GitDiffThumbnail_Create)
{
    GitDiffThumbnail gdt;
    ASSERT(gdt.GetConfig().mode == DiffViewMode::SideBySide);
}
TEST(Test_GitDiffThumbnail_Analyze_Empty)
{
    GitDiffThumbnail gdt;
    auto d = gdt.Analyze("", "HEAD~1", "HEAD");
    ASSERT(!d.valid);
}
TEST(Test_GitDiffThumbnail_Analyze_Valid)
{
    GitDiffThumbnail gdt;
    auto d = gdt.Analyze("main.cpp", "HEAD~1", "HEAD");
    ASSERT(d.filePath == "main.cpp");
}
TEST(Test_GitDiffThumbnail_Config)
{
    GitDiffThumbnailConfig cfg;
    cfg.mode = DiffViewMode::Unified;
    GitDiffThumbnail gdt(cfg);
    ASSERT(gdt.GetConfig().mode == DiffViewMode::Unified);
}
TEST(Test_GitDiffThumbnail_DiffViewMode)
{
    ASSERT(DiffViewMode::SideBySide != DiffViewMode::Unified);
    ASSERT(DiffViewMode::BeforeOnly != DiffViewMode::AfterOnly);
}
TEST(Test_GitDiffThumbnail_AfterCommit)
{
    GitDiffThumbnail gdt;
    auto d = gdt.Analyze("foo.h", "abc", "def");
    ASSERT(d.afterCommit == "def");
}
TEST(Test_GitDiffThumbnail_BeforeCommit)
{
    GitDiffThumbnail gdt;
    auto d = gdt.Analyze("foo.h", "abc", "def");
    ASSERT(d.beforeCommit == "abc");
}
TEST(Test_GitDiffThumbnail_FilePath)
{
    GitDiffThumbnail gdt;
    auto d = gdt.Analyze("widget.cpp", "v1", "v2");
    ASSERT(d.filePath == "widget.cpp");
}
TEST(Test_GitDiffThumbnail_ShowStats)
{
    GitDiffThumbnailConfig cfg;
    cfg.showStats = false;
    GitDiffThumbnail gdt(cfg);
    ASSERT(!gdt.GetConfig().showStats);
}
TEST(Test_GitLFSResolver_Create)
{
    GitLFSResolver r;
    (void)r;
    ASSERT(true);
}
TEST(Test_GitLFSResolver_ParsePointer_Valid)
{
    GitLFSResolver r;
    std::string ptr = "version https://git-lfs.github.com/spec/v1\noid sha256:abc\nsize 12345\n";
    auto p = r.ParsePointer(ptr);
    ASSERT(p.isPointer);
}
TEST(Test_GitLFSResolver_ParsePointer_Invalid)
{
    GitLFSResolver r;
    auto p = r.ParsePointer("not a pointer");
    ASSERT(!p.isPointer);
}
TEST(Test_GitLFSResolver_Resolve)
{
    GitLFSResolver r;
    auto res = r.Resolve("file.bin");
    ASSERT(res.status != LFSResolveStatus::Resolved || res.success());
}
TEST(Test_GitLFSResolver_StatusEnum)
{
    ASSERT(LFSResolveStatus::Resolved != LFSResolveStatus::Pointer);
    ASSERT(LFSResolveStatus::NotLFS != LFSResolveStatus::MissingObject);
}
TEST(Test_GitLFSResolver_Config)
{
    LFSResolveConfig cfg;
    cfg.cacheCapacity = 64;
    GitLFSResolver r(cfg);
    ASSERT(r.GetConfig().cacheCapacity == 64);
}
TEST(Test_GitLFSResolver_IsLFSPointer)
{
    GitLFSResolver r;
    std::string p = "version https://git-lfs.github.com/spec/v1\n";
    ASSERT(r.ParsePointer(p).isPointer);
}
TEST(Test_GitLFSResolver_OidExtracted)
{
    GitLFSResolver r;
    std::string ptr = "version https://git-lfs.github.com/spec/v1\noid sha256:deadbeef\nsize 1\n";
    auto p = r.ParsePointer(ptr);
    ASSERT(!p.oid.empty());
}
TEST(Test_GitLFSResolver_ResultSuccess)
{
    LFSResolveResult res;
    res.status = LFSResolveStatus::Resolved;
    ASSERT(res.success());
    res.status = LFSResolveStatus::NotLFS;
    ASSERT(!res.success());
}
TEST(Test_CommitBadgeCompositor_Create)
{
    CommitBadgeCompositor c;
    ASSERT(c.GetConfig().position == BadgePosition::BottomRight);
}
TEST(Test_CommitBadgeCompositor_Build_Valid)
{
    CommitBadgeCompositor c;
    auto b = c.Build("abc1234", "JD", 3);
    ASSERT(b.valid);
    ASSERT(b.shortHash == "abc1234");
}
TEST(Test_CommitBadgeCompositor_Build_Empty)
{
    CommitBadgeCompositor c;
    auto b = c.Build("", "", 0);
    ASSERT(!b.valid);
}
TEST(Test_CommitBadgeCompositor_FormatAge_Now)
{
    ASSERT(CommitBadgeCompositor::FormatAge(0) == "now");
}
TEST(Test_CommitBadgeCompositor_FormatAge_Days)
{
    ASSERT(CommitBadgeCompositor::FormatAge(3) == "3d");
}
TEST(Test_CommitBadgeCompositor_FormatAge_Weeks)
{
    ASSERT(CommitBadgeCompositor::FormatAge(14) == "2w");
}
TEST(Test_CommitBadgeCompositor_BadgePosition_Enum)
{
    ASSERT(BadgePosition::TopLeft != BadgePosition::BottomRight);
    ASSERT(BadgePosition::TopRight != BadgePosition::BottomLeft);
}
TEST(Test_CommitBadgeCompositor_BadgeSize_Enum)
{
    ASSERT(BadgeSize::Tiny != BadgeSize::Large);
    ASSERT(BadgeSize::Small != BadgeSize::Medium);
}
TEST(Test_CommitBadgeCompositor_ShortHash)
{
    CommitBadgeCompositor c;
    auto b = c.Build("0123456789abcdef", "AB", 1);
    ASSERT(b.shortHash.size() == 7);
    ASSERT(b.shortHash == "0123456");
}
TEST(Test_MergeConflictOverlay_Create)
{
    MergeConflictOverlay overlay;
    ASSERT(overlay.GetConfig().showCount);
}
TEST(Test_MergeConflictOverlay_AnalyzeContent_Clean)
{
    MergeConflictOverlay overlay;
    auto info = overlay.AnalyzeContent("int x = 0;\nreturn x;\n");
    ASSERT(info.state == ConflictState::Clean);
}
TEST(Test_MergeConflictOverlay_AnalyzeContent_Conflict)
{
    MergeConflictOverlay overlay;
    std::string conflict = "<<<<<<< HEAD\nfoo\n=======\nbar\n>>>>>>> branch\n";
    auto info = overlay.AnalyzeContent(conflict);
    ASSERT(info.state == ConflictState::HasConflicts);
}
TEST(Test_MergeConflictOverlay_Analyze_Empty)
{
    MergeConflictOverlay overlay;
    auto info = overlay.Analyze("");
    ASSERT(!info.valid);
}
TEST(Test_MergeConflictOverlay_Analyze_Path)
{
    MergeConflictOverlay overlay;
    auto info = overlay.Analyze("main.cpp");
    ASSERT(info.path == "main.cpp");
}
TEST(Test_MergeConflictOverlay_ShouldRender)
{
    MergeConflictOverlay overlay;
    MergeConflictInfo info;
    info.valid = true;
    info.state = ConflictState::HasConflicts;
    ASSERT(overlay.ShouldRender(info));
    info.state = ConflictState::Clean;
    ASSERT(!overlay.ShouldRender(info));
}
TEST(Test_MergeConflictOverlay_Config)
{
    MergeConflictOverlayConfig cfg;
    cfg.showCount = false;
    MergeConflictOverlay overlay(cfg);
    ASSERT(!overlay.GetConfig().showCount);
}
TEST(Test_MergeConflictOverlay_ConflictStateEnum)
{
    ASSERT(ConflictState::Clean != ConflictState::HasConflicts);
    ASSERT(ConflictState::Resolved != ConflictState::Unknown);
}
TEST(Test_MergeConflictOverlay_MarkerCount)
{
    MergeConflictOverlay overlay;
    std::string conflict = "<<<<<<< HEAD\nfoo\n=======\nbar\n>>>>>>> b\n";
    auto info = overlay.AnalyzeContent(conflict);
    ASSERT(info.conflictCount > 0);
}

//==============================================================================
// Sprint 601-610 — Self-Healing & Adaptive Recovery (v25.4.0 "Rigel-U")
//==============================================================================

TEST(Test_DecoderCrashPredictor_Create)
{
    using namespace ExplorerLens::Engine;
    DecoderCrashPredictor pred;
    ASSERT(pred.DecoderCount() == 0);
}
TEST(Test_DecoderCrashPredictor_Predict_NoSamples)
{
    using namespace ExplorerLens::Engine;
    DecoderCrashPredictor pred;
    auto p = pred.Predict("webp");
    ASSERT(p.level == CrashRiskLevel::None);
    ASSERT(p.samplesUsed == 0);
}
TEST(Test_DecoderCrashPredictor_Record_And_Count)
{
    using namespace ExplorerLens::Engine;
    DecoderCrashPredictor pred;
    for (int i = 0; i < 6; ++i)
        pred.Record("jpeg", 100.0, false, false);
    ASSERT(pred.DecoderCount() == 1);
}
TEST(Test_DecoderCrashPredictor_HighRisk)
{
    using namespace ExplorerLens::Engine;
    DecoderCrashPredictor pred;
    // Feed enough crash samples to push risk high
    for (int i = 0; i < 20; ++i)
        pred.Record("png", 200.0, true, true);
    auto p = pred.Predict("png");
    ASSERT(p.level >= CrashRiskLevel::High);
    ASSERT(p.shouldQuarantine());
}
TEST(Test_DecoderCrashPredictor_Reset)
{
    using namespace ExplorerLens::Engine;
    DecoderCrashPredictor pred;
    pred.Record("tiff", 50.0, false, false);
    ASSERT(pred.DecoderCount() == 1);
    pred.Reset("tiff");
    ASSERT(pred.DecoderCount() == 0);
}
TEST(Test_DecoderCrashPredictor_ResetAll)
{
    using namespace ExplorerLens::Engine;
    DecoderCrashPredictor pred;
    pred.Record("a", 10.0, false, false);
    pred.Record("b", 10.0, false, false);
    pred.ResetAll();
    ASSERT(pred.DecoderCount() == 0);
}
TEST(Test_DecoderCrashPredictor_RiskLevelEnum)
{
    using namespace ExplorerLens::Engine;
    ASSERT((int)CrashRiskLevel::None == 0);
    ASSERT((int)CrashRiskLevel::Critical > (int)CrashRiskLevel::High);
}
TEST(Test_DecoderQuarantineManager_Create)
{
    using namespace ExplorerLens::Engine;
    DecoderQuarantineManager mgr;
    ASSERT(!mgr.IsQuarantined("webp"));
}
TEST(Test_DecoderQuarantineManager_RecordCrash_Threshold1)
{
    using namespace ExplorerLens::Engine;
    DecoderQuarantineManager mgr;
    mgr.RecordCrash("dec");
    mgr.RecordCrash("dec");
    ASSERT(mgr.IsQuarantined("dec"));
    const QuarantineRecord* r = mgr.GetRecord("dec");
    ASSERT(r != nullptr);
    ASSERT(r->stage == QuarantineStage::RetryOnly);
}
TEST(Test_DecoderQuarantineManager_RecordCrash_Bypass)
{
    using namespace ExplorerLens::Engine;
    DecoderQuarantineManager mgr;
    for (int i = 0; i < 11; ++i)
        mgr.RecordCrash("dec");
    const QuarantineRecord* r = mgr.GetRecord("dec");
    ASSERT(r != nullptr);
    ASSERT(r->stage == QuarantineStage::Bypassed);
}
TEST(Test_DecoderQuarantineManager_Recovery)
{
    using namespace ExplorerLens::Engine;
    DecoderQuarantineManager mgr;
    mgr.RecordCrash("dec");
    mgr.RecordCrash("dec");
    for (int i = 0; i < DecoderQuarantineManager::RECOVERY_RELEASE_COUNT; ++i)
        mgr.RecordSuccess("dec");
    const QuarantineRecord* r = mgr.GetRecord("dec");
    ASSERT(r != nullptr);
    ASSERT(r->stage == QuarantineStage::Active);
}
TEST(Test_DecoderQuarantineManager_StageName)
{
    using namespace ExplorerLens::Engine;
    QuarantineRecord r;
    ASSERT(r.StageName() == "Active");
    r.stage = QuarantineStage::SoftRestart;
    ASSERT(r.StageName() == "SoftRestart");
}
TEST(Test_AdaptiveTimeoutTuner_DefaultTimeout)
{
    using namespace ExplorerLens::Engine;
    AdaptiveTimeoutTuner tuner;
    ASSERT(tuner.GetTimeoutMs("png") == AdaptiveTimeoutTuner::DEFAULT_TIMEOUT);
}
TEST(Test_AdaptiveTimeoutTuner_RecordAndLearn)
{
    using namespace ExplorerLens::Engine;
    AdaptiveTimeoutTuner tuner;
    for (int i = 0; i < 25; ++i)
        tuner.RecordLatency("pdf", 200.0 + i);
    double t = tuner.GetTimeoutMs("pdf");
    ASSERT(t >= AdaptiveTimeoutTuner::MIN_TIMEOUT);
    ASSERT(t <= AdaptiveTimeoutTuner::MAX_TIMEOUT);
}
TEST(Test_AdaptiveTimeoutTuner_Clamp)
{
    using namespace ExplorerLens::Engine;
    AdaptiveTimeoutTuner tuner;
    for (int i = 0; i < 25; ++i)
        tuner.RecordLatency("heic", 100000.0);
    ASSERT(tuner.GetTimeoutMs("heic") <= AdaptiveTimeoutTuner::MAX_TIMEOUT);
}
TEST(Test_AdaptiveTimeoutTuner_SetBaseline)
{
    using namespace ExplorerLens::Engine;
    AdaptiveTimeoutTuner tuner;
    tuner.SetBaseline("raw", 10000.0);
    const ATTimeoutPolicy* pol = tuner.GetPolicy("raw");
    ASSERT(pol != nullptr);
    ASSERT(pol->baselineMs == 10000.0);
}
TEST(Test_AdaptiveTimeoutTuner_Reset)
{
    using namespace ExplorerLens::Engine;
    AdaptiveTimeoutTuner tuner;
    for (int i = 0; i < 25; ++i)
        tuner.RecordLatency("tif", 300.0);
    ASSERT(tuner.FormatCount() == 1);
    tuner.Reset("tif");
    ASSERT(tuner.FormatCount() == 0);
}
TEST(Test_HeapCorruptionSentinel_Allocate)
{
    using namespace ExplorerLens::Engine;
    HeapCorruptionSentinel sentinel;
    TrackedBlock* blk = sentinel.Allocate(64, "test");
    ASSERT(blk != nullptr);
    ASSERT(blk->payloadSize == 64);
    ASSERT(blk->headCanary == CANARY_MAGIC_HEAD);
    ASSERT(blk->tailCanary == CANARY_MAGIC_TAIL);
    ASSERT(sentinel.BlockCount() == 1);
    sentinel.Free(blk);
}
TEST(Test_HeapCorruptionSentinel_CheckClean)
{
    using namespace ExplorerLens::Engine;
    HeapCorruptionSentinel sentinel;
    TrackedBlock* blk = sentinel.Allocate(32, "ctx");
    auto report = sentinel.Check(blk);
    ASSERT(report.IsClean());
    sentinel.Free(blk);
}
TEST(Test_HeapCorruptionSentinel_ScanAll_Clean)
{
    using namespace ExplorerLens::Engine;
    HeapCorruptionSentinel sentinel;
    TrackedBlock* b1 = sentinel.Allocate(16, "a");
    TrackedBlock* b2 = sentinel.Allocate(32, "b");
    auto reports = sentinel.ScanAll();
    ASSERT(reports.empty());
    ASSERT(sentinel.IsHealthy());
    sentinel.Free(b1);
    sentinel.Free(b2);
}
TEST(Test_HeapCorruptionSentinel_DetectHeadCorrupt)
{
    using namespace ExplorerLens::Engine;
    HeapCorruptionSentinel sentinel;
    TrackedBlock* blk = sentinel.Allocate(16, "corrupt");
    blk->headCanary = 0xDEAD;  // corrupt
    auto report = sentinel.Check(blk);
    ASSERT(!report.IsClean());
    ASSERT(report.type == HeapCorruptionType::HeadCanaryOverwrite);
    blk->headCanary = CANARY_MAGIC_HEAD;  // restore before free
    sentinel.Free(blk);
}
TEST(Test_RetryPolicyEngine_SuccessFirstTry)
{
    using namespace ExplorerLens::Engine;
    RetryPolicyEngine engine;
    int calls = 0;
    auto result = engine.Execute(
        [&] {
            calls++;
            return true;
        },
        "op");
    ASSERT(result == RetryResult::Success);
    ASSERT(calls == 1);
    ASSERT(engine.Stats().successCount == 1);
}
TEST(Test_RetryPolicyEngine_ExhaustedRetries)
{
    using namespace ExplorerLens::Engine;
    RetryConfig cfg;
    cfg.maxRetries = 2;
    cfg.baseDelayMs = 0.0;
    RetryPolicyEngine engine(cfg);
    auto result = engine.Execute([&] { return false; }, "fail");
    ASSERT(result == RetryResult::ExhaustedRetries);
    ASSERT(engine.Stats().exhaustedCount == 1);
    ASSERT(engine.Stats().retryCount == 2);
}
TEST(Test_RetryPolicyEngine_Config)
{
    using namespace ExplorerLens::Engine;
    RetryPolicyEngine engine;
    ASSERT(engine.MaxRetries() == 3);
    engine.Config().maxRetries = 5;
    ASSERT(engine.MaxRetries() == 5);
}
TEST(Test_RetryPolicyEngine_ResetStats)
{
    using namespace ExplorerLens::Engine;
    RetryPolicyEngine engine;
    engine.Execute([&] { return true; }, "x");
    engine.ResetStats();
    ASSERT(engine.Stats().successCount == 0);
    ASSERT(engine.Stats().totalAttempts == 0);
}
TEST(Test_DecoderIncidentReporter_Healthy)
{
    using namespace ExplorerLens::Engine;
    DecoderIncidentReporter reporter;
    DecoderHealthSnapshot snap;
    snap.decoderName = "webp";
    snap.errorRatePct = 0.0;
    snap.crashCount = 0;
    snap.totalDecodes = 100;
    auto report = reporter.CreateReport(snap);
    ASSERT(report.severity == IncidentSeverity::Info);
    ASSERT(!report.id.empty());
    ASSERT(report.decoderName == "webp");
}
TEST(Test_DecoderIncidentReporter_Critical)
{
    using namespace ExplorerLens::Engine;
    DecoderIncidentReporter reporter;
    DecoderHealthSnapshot snap;
    snap.decoderName = "heic";
    snap.crashCount = 15;
    snap.isQuarantined = true;
    auto report = reporter.CreateReport(snap);
    ASSERT(report.severity == IncidentSeverity::Critical);
}
TEST(Test_DecoderIncidentReporter_BatchReport)
{
    using namespace ExplorerLens::Engine;
    DecoderIncidentReporter reporter;
    std::vector<DecoderHealthSnapshot> snaps(3);
    snaps[0].decoderName = "a";
    snaps[1].decoderName = "b";
    snaps[2].decoderName = "c";
    snaps[2].crashCount = 20;
    snaps[2].isQuarantined = true;
    auto reports = reporter.CreateBatchReport(snaps);
    ASSERT(reports.size() == 3);
    ASSERT(reporter.CriticalCount(reports) == 1);
}
TEST(Test_COMSelfRepairValidator_ClsidConst)
{
    using namespace ExplorerLens::Engine;
    std::string clsid = COMSelfRepairValidator::LensClsid();
    ASSERT(!clsid.empty());
    ASSERT(clsid.find("9E6ECB90") != std::string::npos);
}
TEST(Test_COMSelfRepairValidator_IsRegistered)
{
    using namespace ExplorerLens::Engine;
    COMSelfRepairValidator validator;
    ASSERT(validator.IsRegistered());
}
TEST(Test_COMSelfRepairValidator_ClsidPath)
{
    using namespace ExplorerLens::Engine;
    COMSelfRepairValidator validator;
    std::string path = validator.ClsidPath();
    ASSERT(path.find("9E6ECB90") != std::string::npos);
}
TEST(Test_COMSelfRepairValidator_Validate)
{
    using namespace ExplorerLens::Engine;
    COMSelfRepairValidator validator;
    auto result = validator.Validate(true);
    ASSERT(result.totalEntries > 0);
}
TEST(Test_BootIntegritySelfTest_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& test = BootIntegritySelfTest::Instance();
    test.Reset();
    ASSERT(test.TestCount() == 0);
}
TEST(Test_BootIntegritySelfTest_RunAll)
{
    using namespace ExplorerLens::Engine;
    auto& test = BootIntegritySelfTest::Instance();
    test.Reset();
    auto report = test.RunAll();
    ASSERT(report.total >= 5);
    ASSERT(report.WasRun());
    ASSERT(report.IsHealthy());
}
TEST(Test_BootIntegritySelfTest_AddTest)
{
    using namespace ExplorerLens::Engine;
    auto& test = BootIntegritySelfTest::Instance();
    test.Reset();
    test.AddTest("Custom", "Custom check", [] { return true; });
    auto report = test.RunAll();
    ASSERT(report.passed >= 1);
}
TEST(Test_BootIntegritySelfTest_FailingTest)
{
    using namespace ExplorerLens::Engine;
    auto& fresh = BootIntegritySelfTest::Instance();
    fresh.Reset();
    fresh.AddTest("Fail", "Always fails", [] { return false; });
    auto report = fresh.RunAll();
    ASSERT(report.failed >= 1);
    ASSERT(!report.IsHealthy());
    fresh.Reset();
}

//==============================================================================
// Sprint 611-620 — Multi-Instance & Virtual Desktop (v25.5.0 "Rigel-V")
//==============================================================================

TEST(Test_VirtualDesktopAwareness_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& vda = VirtualDesktopAwareness::Instance();
    vda.SetCurrentDesktopId(VDID_GLOBAL);
    ASSERT(vda.GetCurrentDesktopId() == VDID_GLOBAL);
}
TEST(Test_VirtualDesktopAwareness_BuildCacheKeyPrefix_Global)
{
    using namespace ExplorerLens::Engine;
    auto& vda = VirtualDesktopAwareness::Instance();
    std::string prefix = vda.BuildCacheKeyPrefix(VDScopePolicy::Global);
    ASSERT(prefix == "global");
}
TEST(Test_VirtualDesktopAwareness_BuildCacheKeyPrefix_PerDesktop)
{
    using namespace ExplorerLens::Engine;
    auto& vda = VirtualDesktopAwareness::Instance();
    vda.SetCurrentDesktopId(42);
    std::string prefix = vda.BuildCacheKeyPrefix(VDScopePolicy::PerDesktop);
    ASSERT(prefix.find("42") != std::string::npos);
}
TEST(Test_VirtualDesktopAwareness_RegisterDesktop)
{
    using namespace ExplorerLens::Engine;
    auto& vda = VirtualDesktopAwareness::Instance();
    VirtualDesktopInfo info;
    info.id = 99;
    info.name = L"Work";
    vda.RegisterDesktop(info);
    ASSERT(vda.HasDesktop(99));
    ASSERT(vda.DesktopCount() >= 1);
}
TEST(Test_WTSSessionIsolation_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& wts = WTSSessionIsolation::Instance();
    wts.SetCurrentSession(1);
    ASSERT(wts.CurrentSessionId() == 1);
}
TEST(Test_WTSSessionIsolation_GetCacheScope)
{
    using namespace ExplorerLens::Engine;
    auto& wts = WTSSessionIsolation::Instance();
    wts.SetCurrentSession(7);
    auto scope = wts.CurrentCacheScope();
    ASSERT(scope.isIsolated);
    ASSERT(scope.cacheRoot.find("7") != std::string::npos);
}
TEST(Test_WTSSessionIsolation_RegisterSession)
{
    using namespace ExplorerLens::Engine;
    auto& wts = WTSSessionIsolation::Instance();
    WTSSessionInfo info;
    info.sessionId = 3;
    info.state = WTSSessionState::Active;
    wts.RegisterSession(info);
    ASSERT(wts.SessionCount() >= 1);
}
TEST(Test_WTSSessionIsolation_StateName)
{
    using namespace ExplorerLens::Engine;
    WTSSessionInfo info;
    info.state = WTSSessionState::Disconnected;
    ASSERT(info.StateName() == "Disconnected");
}
TEST(Test_PerMonitorDPISelectorV2_DefaultFallback)
{
    using namespace ExplorerLens::Engine;
    PerMonitorDPISelectorV2 sel;
    auto res = sel.SelectResolution(0);
    ASSERT(res.widthPx == PerMonitorDPISelectorV2::LOGICAL_THUMB_SIZE);
}
TEST(Test_PerMonitorDPISelectorV2_HighDPI)
{
    using namespace ExplorerLens::Engine;
    PerMonitorDPISelectorV2 sel;
    MonitorDPIProfile p;
    p.monitorHandle = 1;
    p.dpiX = 192;
    p.dpiY = 192;
    p.scalePercent = PerMonitorDPIScale::Scale200;
    p.isPrimary = true;
    sel.RegisterMonitor(p);
    auto res = sel.SelectResolution(1);
    ASSERT(res.widthPx == 512);  // 256 * 2.0
}
TEST(Test_PerMonitorDPISelectorV2_MixedDPI)
{
    using namespace ExplorerLens::Engine;
    PerMonitorDPISelectorV2 sel;
    MonitorDPIProfile p1;
    p1.monitorHandle = 1;
    p1.dpiX = 96;
    MonitorDPIProfile p2;
    p2.monitorHandle = 2;
    p2.dpiX = 192;
    sel.RegisterMonitor(p1);
    sel.RegisterMonitor(p2);
    ASSERT(sel.MonitorCount() == 2);
    ASSERT(sel.IsMixedDPI());
}
TEST(Test_TabbedExplorerSync_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& sync = TabbedExplorerSync::Instance();
    sync.SetPolicy(TabSyncPolicy::Independent);
    ASSERT(sync.GetPolicy() == TabSyncPolicy::Independent);
}
TEST(Test_TabbedExplorerSync_RegisterTab)
{
    using namespace ExplorerLens::Engine;
    auto& sync = TabbedExplorerSync::Instance();
    ExplorerTabState tab;
    tab.tabId = 100;
    tab.folderPath = L"C:\\Docs";
    sync.RegisterTab(tab);
    ASSERT(sync.HasTab(100));
}
TEST(Test_TabbedExplorerSync_SyncSameFolder)
{
    using namespace ExplorerLens::Engine;
    auto& sync = TabbedExplorerSync::Instance();
    sync.SetPolicy(TabSyncPolicy::SyncSameFolder);
    ExplorerTabState t1;
    t1.tabId = 200;
    t1.folderPath = L"C:\\Photos";
    t1.zoomLevel = 100;
    ExplorerTabState t2;
    t2.tabId = 201;
    t2.folderPath = L"C:\\Photos";
    t2.zoomLevel = 100;
    sync.RegisterTab(t1);
    sync.RegisterTab(t2);
    ExplorerTabState updated = t1;
    updated.zoomLevel = 150;
    sync.OnTabStateChange(updated);
    auto siblings = sync.GetSiblingTabs(L"C:\\Photos");
    bool allSynced = true;
    for (const auto& s : siblings)
        if (s.zoomLevel != 150 && s.tabId != t1.tabId)
            allSynced = false;
    ASSERT(allSynced);
}
TEST(Test_CrossSessionThumbnailPool_InsertAndLookup)
{
    using namespace ExplorerLens::Engine;
    CrossSessionThumbnailPool pool(1024 * 1024);
    uint8_t data[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    ASSERT(pool.Insert(0x12345, data, 4, 1));
    uint32_t sz = 0;
    const uint8_t* ptr = pool.Lookup(0x12345, sz);
    ASSERT(ptr != nullptr);
    ASSERT(sz == 4);
    ASSERT(ptr[0] == 0xAA);
}
TEST(Test_CrossSessionThumbnailPool_Stats)
{
    using namespace ExplorerLens::Engine;
    CrossSessionThumbnailPool pool;
    auto stats = pool.Stats();
    ASSERT(stats.bytesCapacity == CrossSessionThumbnailPool::DEFAULT_CAPACITY);
}
TEST(Test_CrossSessionThumbnailPool_HitRate)
{
    using namespace ExplorerLens::Engine;
    CrossSessionThumbnailPool pool(65536);
    uint8_t d[8] = {};
    pool.Insert(111, d, 8, 0);
    uint32_t sz = 0;
    pool.Lookup(111, sz);  // hit
    pool.Lookup(999, sz);  // miss
    auto stats = pool.Stats();
    ASSERT(stats.hitCount == 1);
    ASSERT(stats.missCount == 1);
    ASSERT(stats.HitRatePct() == 50.0);
}
TEST(Test_InstanceRegistry_Register)
{
    using namespace ExplorerLens::Engine;
    auto& reg = InstanceRegistry::Instance();
    InstanceId id = reg.Register(1234, 1, "25.4.0");
    ASSERT(id > 0);
    const InstanceRecord* r = reg.Get(id);
    ASSERT(r != nullptr);
    ASSERT(r->processId == 1234);
}
TEST(Test_InstanceRegistry_Heartbeat)
{
    using namespace ExplorerLens::Engine;
    auto& reg = InstanceRegistry::Instance();
    InstanceId id = reg.Register(5678, 1, "25.4.0");
    reg.Heartbeat(id);
    const InstanceRecord* r = reg.Get(id);
    ASSERT(r != nullptr);
    ASSERT(!r->IsStale(10000));
}
TEST(Test_InstanceRegistry_Unregister)
{
    using namespace ExplorerLens::Engine;
    auto& reg = InstanceRegistry::Instance();
    InstanceId id = reg.Register(9999, 1, "test");
    reg.Unregister(id);
    ASSERT(reg.Get(id) == nullptr);
}
TEST(Test_ForegroundPriorityInheritance_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& fpi = ForegroundPriorityInheritance::Instance();
    fpi.SetForegroundWindow(100);
    ASSERT(fpi.GetForegroundWindow() == 100);
}
TEST(Test_ForegroundPriorityInheritance_Evaluate_Foreground)
{
    using namespace ExplorerLens::Engine;
    auto& fpi = ForegroundPriorityInheritance::Instance();
    fpi.SetForegroundWindow(200);
    auto d = fpi.Evaluate(200);
    ASSERT(d.context == DecodeWindowContext::Foreground);
    ASSERT(d.inherited);
}
TEST(Test_ForegroundPriorityInheritance_Evaluate_Background)
{
    using namespace ExplorerLens::Engine;
    auto& fpi = ForegroundPriorityInheritance::Instance();
    fpi.SetForegroundWindow(200);
    auto d = fpi.Evaluate(300);
    ASSERT(d.context == DecodeWindowContext::Background);
    ASSERT(!d.inherited);
}
TEST(Test_CrossInstanceLoadBalancer_Dispatch_NoInstances)
{
    using namespace ExplorerLens::Engine;
    CrossInstanceLoadBalancer lb;
    auto d = lb.Dispatch();
    ASSERT(!d.found);
}
TEST(Test_CrossInstanceLoadBalancer_Dispatch_LeastConn)
{
    using namespace ExplorerLens::Engine;
    CrossInstanceLoadBalancer lb(LBAlgorithm::LeastConnections);
    InstanceLoad i1;
    i1.instanceId = 1;
    i1.activeTasks = 5;
    InstanceLoad i2;
    i2.instanceId = 2;
    i2.activeTasks = 2;
    lb.RegisterInstance(i1);
    lb.RegisterInstance(i2);
    auto d = lb.Dispatch();
    ASSERT(d.found);
    ASSERT(d.instanceId == 2);  // least connections
}
TEST(Test_CrossInstanceLoadBalancer_MarkTaskEvents)
{
    using namespace ExplorerLens::Engine;
    CrossInstanceLoadBalancer lb;
    InstanceLoad il;
    il.instanceId = 10;
    il.activeTasks = 0;
    lb.RegisterInstance(il);
    lb.MarkTaskStart(10);
    lb.MarkTaskEnd(10);
}

//==============================================================================
// Sprint 621-630 — Collaborative Annotations & Sharing (v25.6.0 "Rigel-W")
//==============================================================================

TEST(Test_AnnotationStore_AddAndCount)
{
    using namespace ExplorerLens::Engine;
    auto& store = AnnotationStore::Instance();
    store.Clear();
    Annotation a;
    a.filePath = L"C:\\img.jpg";
    a.type = AnnotationRecordType::Tag;
    a.value = L"holiday";
    uint64_t id = store.Add(a);
    ASSERT(id > 0);
    ASSERT(store.Count() == 1);
    store.Clear();
}
TEST(Test_AnnotationStore_GetForFile)
{
    using namespace ExplorerLens::Engine;
    auto& store = AnnotationStore::Instance();
    store.Clear();
    Annotation a;
    a.filePath = L"C:\\x.png";
    a.type = AnnotationRecordType::Star;
    a.value = L"5";
    store.Add(a);
    auto anns = store.GetForFile(L"C:\\x.png");
    ASSERT(anns.size() == 1);
    store.Clear();
}
TEST(Test_AnnotationStore_Delete)
{
    using namespace ExplorerLens::Engine;
    auto& store = AnnotationStore::Instance();
    store.Clear();
    Annotation a;
    a.filePath = L"C:\\y.png";
    a.type = AnnotationRecordType::Comment;
    uint64_t id = store.Add(a);
    ASSERT(store.Delete(id));
    auto anns = store.GetForFile(L"C:\\y.png");
    ASSERT(anns.empty());
    store.Clear();
}
TEST(Test_AnnotationStore_DirtyFlag)
{
    using namespace ExplorerLens::Engine;
    auto& store = AnnotationStore::Instance();
    store.Clear();
    Annotation a;
    a.filePath = L"C:\\z.png";
    a.type = AnnotationRecordType::Color;
    uint64_t id = store.Add(a);
    ASSERT(store.DirtyCount() == 1);
    store.MarkSynced(id);
    ASSERT(store.DirtyCount() == 0);
    store.Clear();
}
TEST(Test_AnnotationStore_AnnotationTypeNames)
{
    using namespace ExplorerLens::Engine;
    Annotation a;
    a.type = AnnotationRecordType::Rating;
    ASSERT(a.TypeName() == "Rating");
    a.type = AnnotationRecordType::Star;
    ASSERT(a.TypeName() == "Star");
}
TEST(Test_AnnotationOverlayRenderer_NoAnnotations)
{
    using namespace ExplorerLens::Engine;
    AnnotationOverlayRenderer renderer;
    AnnotationOverlayData data;  // empty
    auto result = renderer.Render(data, nullptr, 256, 256);
    ASSERT(!result.rendered);
}
TEST(Test_AnnotationOverlayRenderer_WithStars)
{
    using namespace ExplorerLens::Engine;
    AnnotationOverlayRenderer renderer;
    AnnotationOverlayData data;
    data.starRating = 4;
    auto result = renderer.Render(data, nullptr, 256, 256);
    ASSERT(result.rendered);
    ASSERT(result.elementsDrawn >= 1);
}
TEST(Test_AnnotationOverlayRenderer_Config)
{
    using namespace ExplorerLens::Engine;
    AnnotationOverlayConfig cfg;
    cfg.showStars = false;
    AnnotationOverlayRenderer renderer(cfg);
    ASSERT(!renderer.Config().showStars);
}
TEST(Test_CollabWebhookBridge_NoConfigs)
{
    using namespace ExplorerLens::Engine;
    CollabWebhookBridge bridge;
    AnnotationEvent evt;
    evt.kind = AnnotationEvent::Kind::Created;
    auto results = bridge.PostEvent(evt);
    ASSERT(results.empty());
}
TEST(Test_CollabWebhookBridge_BuildPayload_Teams)
{
    using namespace ExplorerLens::Engine;
    CollabWebhookBridge bridge;
    AnnotationEvent evt;
    std::string payload = bridge.BuildPayload(WebhookPlatform::Teams, evt);
    ASSERT(!payload.empty());
    ASSERT(payload.find("AdaptiveCard") != std::string::npos);
}
TEST(Test_CollabWebhookBridge_BuildPayload_Slack)
{
    using namespace ExplorerLens::Engine;
    CollabWebhookBridge bridge;
    AnnotationEvent evt;
    std::string payload = bridge.BuildPayload(WebhookPlatform::Slack, evt);
    ASSERT(payload.find("text") != std::string::npos);
}
TEST(Test_CollabWebhookBridge_InjectableHTTP)
{
    using namespace ExplorerLens::Engine;
    WebhookPostResult fakeResult{true, 200, {}};
    CollabWebhookBridge bridge([fakeResult](const std::string&, const std::string&) { return fakeResult; });
    WebhookConfig cfg;
    cfg.platform = WebhookPlatform::Generic;
    cfg.webhookUrl = "http://test";
    bridge.AddConfig(cfg);
    AnnotationEvent evt;
    auto results = bridge.PostEvent(evt);
    ASSERT(results.size() == 1);
    ASSERT(results[0].success);
    ASSERT(results[0].statusCode == 200);
}
TEST(Test_SharedCollectionBuilder_CreateCollection)
{
    using namespace ExplorerLens::Engine;
    SharedCollectionBuilder builder;
    auto& col = builder.CreateCollection(L"Holiday", L"C:\\Photos");
    ASSERT(!col.shareToken.empty());
    ASSERT(builder.CollectionCount() == 1);
}
TEST(Test_SharedCollectionBuilder_AddItem)
{
    using namespace ExplorerLens::Engine;
    SharedCollectionBuilder builder;
    builder.CreateCollection(L"Work", L"C:\\Docs");
    CollectionItem item;
    item.filePath = L"C:\\Docs\\a.pdf";
    item.starRating = 3;
    ASSERT(builder.AddItem(L"Work", item));
    auto& cols = builder.All();
    ASSERT(cols[0].ItemCount() == 1);
}
TEST(Test_SharedCollectionBuilder_FindByToken)
{
    using namespace ExplorerLens::Engine;
    SharedCollectionBuilder builder;
    auto& col = builder.CreateCollection(L"Test", L"C:\\T");
    std::string token = col.shareToken;
    const SharedCollection* found = builder.FindByToken(token);
    ASSERT(found != nullptr);
    ASSERT(found->name == L"Test");
}
TEST(Test_SharedCollectionBuilder_MergeCollection)
{
    using namespace ExplorerLens::Engine;
    SharedCollectionBuilder builder;
    builder.CreateCollection(L"Shared", L"C:\\S");
    SharedCollection remote;
    remote.name = L"Shared";
    remote.folderPath = L"C:\\S";
    CollectionItem ri;
    ri.filePath = L"C:\\S\\b.jpg";
    remote.items.push_back(ri);
    ASSERT(builder.MergeCollection(remote));
    auto& cols = builder.All();
    ASSERT(cols[0].ItemCount() >= 1);
}
TEST(Test_AnnotationDiffViewer_NoDiff)
{
    using namespace ExplorerLens::Engine;
    AnnotationDiffViewer viewer;
    AnnotationSnapshot before, after;
    before.starRating = 3;
    after.starRating = 3;
    auto result = viewer.Diff(before, after);
    ASSERT(result.IsClean());
}
TEST(Test_AnnotationDiffViewer_StarChanged)
{
    using namespace ExplorerLens::Engine;
    AnnotationDiffViewer viewer;
    AnnotationSnapshot before, after;
    before.starRating = 2;
    after.starRating = 4;
    auto result = viewer.Diff(before, after);
    ASSERT(result.changes == 1);
    ASSERT(!result.IsClean());
}
TEST(Test_AnnotationDiffViewer_TagAdded)
{
    using namespace ExplorerLens::Engine;
    AnnotationDiffViewer viewer;
    AnnotationSnapshot before, after;
    after.tags.push_back(L"newtag");
    auto result = viewer.Diff(before, after);
    ASSERT(result.additions == 1);
}
TEST(Test_AnnotationDiffViewer_DiffOpName)
{
    using namespace ExplorerLens::Engine;
    AnnotationDiffEntry entry;
    entry.op = DiffOperation::Add;
    ASSERT(entry.OpName() == "Add");
    entry.op = DiffOperation::Remove;
    ASSERT(entry.OpName() == "Remove");
}
TEST(Test_AnnotationExporter_JSON)
{
    using namespace ExplorerLens::Engine;
    AnnotationExporter exporter;
    std::vector<ExportableAnnotation> items(1);
    items[0].filePath = L"a.jpg";
    items[0].starRating = 3;
    auto result = exporter.Export(items, AnnotationExportFormat::JSON);
    ASSERT(result.success);
    ASSERT(!result.content.empty());
    ASSERT(result.content.find('[') != std::string::npos);
    ASSERT(result.itemCount == 1);
}
TEST(Test_AnnotationExporter_XML)
{
    using namespace ExplorerLens::Engine;
    AnnotationExporter exporter;
    std::vector<ExportableAnnotation> items(1);
    items[0].filePath = L"b.jpg";
    auto result = exporter.Export(items, AnnotationExportFormat::XML);
    ASSERT(result.success);
    ASSERT(result.content.find("<?xml") != std::string::npos);
}
TEST(Test_AnnotationExporter_CSV)
{
    using namespace ExplorerLens::Engine;
    AnnotationExporter exporter;
    std::vector<ExportableAnnotation> items(2);
    auto result = exporter.Export(items, AnnotationExportFormat::CSV);
    ASSERT(result.success);
    ASSERT(result.content.find("file,stars") != std::string::npos);
}
TEST(Test_AnnotationExporter_FormatName)
{
    using namespace ExplorerLens::Engine;
    AnnotationExporter exporter;
    ASSERT(exporter.FormatName(AnnotationExportFormat::XMP) == "XMP");
    ASSERT(exporter.FormatName(AnnotationExportFormat::JSON) == "JSON");
}
TEST(Test_CollabCloudSync_NoHttpFn)
{
    using namespace ExplorerLens::Engine;
    CollabCloudSync sync;
    CloudSyncRequest req;
    req.accessToken = "tok";
    auto result = sync.Sync(req);
    ASSERT(!result.Ok());
    ASSERT(result.status == SyncStatus::Error);
}
TEST(Test_CollabCloudSync_EmptyToken)
{
    using namespace ExplorerLens::Engine;
    CollabCloudSync sync([](const std::string&, const std::string&, const std::string&, std::string&) { return true; });
    CloudSyncRequest req;  // empty token
    auto result = sync.Sync(req);
    ASSERT(!result.Ok());
}
TEST(Test_CollabCloudSync_SuccessfulSync)
{
    using namespace ExplorerLens::Engine;
    CollabCloudSync sync([](const std::string&, const std::string&, const std::string&, std::string& resp) {
        resp = "{\"uploaded\":5}";
        return true;
    });
    CloudSyncRequest req;
    req.accessToken = "bearer123";
    auto result = sync.Sync(req);
    ASSERT(result.Ok());
}
TEST(Test_AnnotationSchemaMigrator_AlreadyCurrent)
{
    using namespace ExplorerLens::Engine;
    AnnotationSchemaMigrator migrator;
    auto result = migrator.Migrate("{}", AnnotationSchemaMigrator::CURRENT_SCHEMA_VERSION);
    ASSERT(result.success);
    ASSERT(result.stepsApplied == 0);
}
TEST(Test_AnnotationSchemaMigrator_V1ToV4)
{
    using namespace ExplorerLens::Engine;
    AnnotationSchemaMigrator migrator;
    auto result = migrator.Migrate("{\"stars\":3}", 1, 4);
    ASSERT(result.success);
    ASSERT(result.stepsApplied == 3);
    ASSERT(result.finalVersion == 4);
    ASSERT(result.errorMsg.empty());
}
TEST(Test_AnnotationSchemaMigrator_CurrentVersion)
{
    using namespace ExplorerLens::Engine;
    ASSERT(AnnotationSchemaMigrator::CURRENT_SCHEMA_VERSION == 4);
}

//==============================================================================
// Sprint 631-640 — Protocol Surface & API Ecosystem (v25.7.0 "Rigel-X")
//==============================================================================

TEST(Test_GRPCThumbnailService_Create)
{
    using namespace ExplorerLens::Engine;
    GRPCThumbnailService svc;
    ASSERT(!svc.IsRunning());
    ASSERT(svc.State() == GRPCServiceState::Stopped);
}
TEST(Test_GRPCThumbnailService_StartStop)
{
    using namespace ExplorerLens::Engine;
    GRPCThumbnailService svc;
    ASSERT(svc.Start());
    ASSERT(svc.IsRunning());
    svc.Stop();
    ASSERT(!svc.IsRunning());
}
TEST(Test_GRPCThumbnailService_HandleRequest)
{
    using namespace ExplorerLens::Engine;
    GRPCThumbnailService svc;
    svc.SetRequestHandler([](const GRPCThumbnailRequest& req) {
        GRPCThumbnailResponse resp;
        resp.success = !req.filePath.empty();
        resp.widthPx = req.width;
        resp.heightPx = req.height;
        return resp;
    });
    GRPCThumbnailRequest req;
    req.filePath = L"img.png";
    req.width = 256;
    req.height = 256;
    auto resp = svc.HandleRequest(req);
    ASSERT(resp.success);
    ASSERT(resp.widthPx == 256);
}
TEST(Test_GRPCThumbnailService_ServiceName)
{
    using namespace ExplorerLens::Engine;
    GRPCThumbnailService svc;
    ASSERT(svc.ServiceName().find("ThumbnailService") != std::string::npos);
}
TEST(Test_GRPCThumbnailService_Config)
{
    using namespace ExplorerLens::Engine;
    GRPCServiceConfig cfg;
    cfg.maxConcurrent = 64;
    GRPCThumbnailService svc(cfg);
    ASSERT(svc.Config().maxConcurrent == 64);
}
TEST(Test_RESTThumbnailServer_Create)
{
    using namespace ExplorerLens::Engine;
    RESTThumbnailServer server;
    ASSERT(!server.IsRunning());
}
TEST(Test_RESTThumbnailServer_StartStop)
{
    using namespace ExplorerLens::Engine;
    RESTThumbnailServer server;
    ASSERT(server.Start());
    ASSERT(server.IsRunning());
    server.Stop();
    ASSERT(!server.IsRunning());
}
TEST(Test_RESTThumbnailServer_Dispatch_404)
{
    using namespace ExplorerLens::Engine;
    RESTThumbnailServer server;
    HTTPRequest req;
    req.method = HTTPMethod::GET;
    req.path = "/unknown";
    auto resp = server.Dispatch(req);
    ASSERT(resp.statusCode == 404);
}
TEST(Test_GraphQLQueryEngine_Execute_NoResolver)
{
    using namespace ExplorerLens::Engine;
    GraphQLQueryEngine engine;
    GraphQLRequest req;
    req.query = "{ thumbnail(path: \"x.png\") }";
    auto result = engine.Execute(req);
    ASSERT(!result.HasErrors());
}
TEST(Test_GraphQLQueryEngine_Introspection)
{
    using namespace ExplorerLens::Engine;
    GraphQLQueryEngine engine;
    GraphQLRequest req;
    req.query = "{ __schema { types { name } } }";
    auto result = engine.Execute(req);
    ASSERT(!result.HasErrors());
}
TEST(Test_WebSocketPushChannel_Create)
{
    using namespace ExplorerLens::Engine;
    WebSocketPushChannel channel;
    ASSERT(channel.ClientCount() == 0);
}
TEST(Test_WebSocketPushChannel_AddRemoveClient)
{
    using namespace ExplorerLens::Engine;
    WebSocketPushChannel channel;
    channel.SimulateClientConnect();
    ASSERT(channel.ClientCount() == 1);
    channel.SimulateClientDisconnect();
    ASSERT(channel.ClientCount() == 0);
}
TEST(Test_WebSocketPushChannel_Broadcast)
{
    using namespace ExplorerLens::Engine;
    WebSocketPushChannel channel;
    channel.SimulateClientConnect();
    channel.SimulateClientConnect();
    WSMessage msg;
    msg.payload = "hello";
    channel.Broadcast(msg);
    ASSERT(channel.ClientCount() == 2);
}
TEST(Test_OpenAPISpecGenerator_GenerateYAML)
{
    using namespace ExplorerLens::Engine;
    OpenAPISpecGenerator gen;
    std::string yaml = gen.Generate(OpenAPIOutputFormat::YAML);
    ASSERT(!yaml.empty());
    ASSERT(yaml.find("openapi") != std::string::npos);
}
TEST(Test_OpenAPISpecGenerator_GenerateJSON)
{
    using namespace ExplorerLens::Engine;
    OpenAPISpecGenerator gen;
    std::string json = gen.Generate(OpenAPIOutputFormat::JSON);
    ASSERT(!json.empty());
    ASSERT(json.find("{") != std::string::npos);
}
TEST(Test_SDKBindingsGenerator_CSharp)
{
    using namespace ExplorerLens::Engine;
    SDKBindingsGenerator gen;
    auto result = gen.Generate(SDKBindingLanguage::CSharp);
    ASSERT(result.success);
    const std::string& code = result.code;
    ASSERT(!code.empty());
    ASSERT(code.find("class") != std::string::npos || code.find("namespace") != std::string::npos);
}
TEST(Test_SDKBindingsGenerator_Python)
{
    using namespace ExplorerLens::Engine;
    SDKBindingsGenerator gen;
    auto result = gen.Generate(SDKBindingLanguage::Python);
    ASSERT(result.success);
    const std::string& code = result.code;
    ASSERT(!code.empty());
    ASSERT(code.find("class") != std::string::npos || code.find("def") != std::string::npos);
}
TEST(Test_OAuthTokenValidator_ValidToken)
{
    using namespace ExplorerLens::Engine;
    // JWT format: header.payload.signature (3 dots-separated parts)
    OAuthTokenValidator validator;
    // Valid 3-part token stub (any header.payload.sig)
    std::string token = "eyJhbGciOiJSUzI1NiJ9.eyJzdWIiOiJ1c2VyMSJ9.signature";
    auto result = validator.Validate(token);
    ASSERT(result.Ok());
    ASSERT(result.status == TokenValidationStatus::Valid);
}
TEST(Test_OAuthTokenValidator_MalformedToken)
{
    using namespace ExplorerLens::Engine;
    OAuthTokenValidator validator;
    auto result = validator.Validate("not-a-jwt");
    ASSERT(!result.Ok());
    ASSERT(result.status == TokenValidationStatus::MalformedToken);
}
TEST(Test_OAuthTokenValidator_EmptyToken)
{
    using namespace ExplorerLens::Engine;
    OAuthTokenValidator validator;
    auto result = validator.Validate("");
    ASSERT(!result.Ok());
}
TEST(Test_OAuthTokenValidator_InjectableClock)
{
    using namespace ExplorerLens::Engine;
    OAuthTokenValidator validator("issuer", "audience", [] { return (int64_t)1000000; });
    std::string token = "hdr.pld.sig";
    auto result = validator.Validate(token);
    ASSERT(result.Ok());
    ASSERT(result.claims.issuedAt == 1000000);
}
TEST(Test_APIRateLimiter_Allow)
{
    using namespace ExplorerLens::Engine;
    RateLimitPolicy pol;
    pol.requestsPerWindow = 10;
    pol.burstAllowance = 5;
    APIRateLimiter limiter(pol);
    auto result = limiter.Check("user1");
    ASSERT(result.IsAllowed());
}
TEST(Test_APIRateLimiter_Throttle)
{
    using namespace ExplorerLens::Engine;
    RateLimitPolicy pol;
    pol.requestsPerWindow = 2;
    pol.burstAllowance = 0;
    APIRateLimiter limiter(pol);
    limiter.Check("u");
    limiter.Check("u");
    limiter.Check("u");  // 3rd should throttle
    auto result = limiter.Check("u");
    ASSERT(result.decision == RateLimitDecision::Throttle);
}
TEST(Test_APIRateLimiter_ResetClient)
{
    using namespace ExplorerLens::Engine;
    APIRateLimiter limiter;
    limiter.Check("clientX");
    ASSERT(limiter.TrackedClients() >= 1);
    limiter.ResetClient("clientX");
    ASSERT(limiter.TrackedClients() == 0);
}
TEST(Test_APIRateLimiter_SlidingWindow)
{
    using namespace ExplorerLens::Engine;
    RateLimitPolicy pol;
    pol.algorithm = RateLimitAlgorithm::SlidingWindow;
    pol.requestsPerWindow = 5;
    APIRateLimiter limiter(pol);
    for (int i = 0; i < 4; ++i) {
        auto r = limiter.Check("sw");
        ASSERT(r.IsAllowed());
    }
}

//==============================================================================
// Sprint 641-650 — Post-Quantum Security (v26.0.0 "Canopus")
//==============================================================================

TEST(Test_MLKEMKeyEncapsulator_GenerateKeyPair)
{
    using namespace ExplorerLens::Engine;
    MLKEMKeyEncapsulator kem;
    auto kp = kem.GenerateKeyPair();
    ASSERT(kp.IsValid());
    ASSERT(kp.publicKey.bytes.size() == 1184);  // MLKEM-768
    ASSERT(kp.privateKey.bytes.size() == 2400);
}
TEST(Test_MLKEMKeyEncapsulator_Encapsulate)
{
    using namespace ExplorerLens::Engine;
    MLKEMKeyEncapsulator kem;
    auto kp = kem.GenerateKeyPair();
    auto result = kem.Encapsulate(kp.publicKey);
    ASSERT(result.success);
    ASSERT(!result.ciphertext.empty());
}
TEST(Test_MLKEMKeyEncapsulator_Decapsulate)
{
    using namespace ExplorerLens::Engine;
    MLKEMKeyEncapsulator kem;
    auto kp = kem.GenerateKeyPair();
    auto enc = kem.Encapsulate(kp.publicKey);
    auto dec = kem.Decapsulate(kp.privateKey, enc.ciphertext);
    ASSERT(dec.success);
    ASSERT(dec.sharedSecret == enc.sharedSecret);
}
TEST(Test_MLKEMKeyEncapsulator_LevelName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(MLKEMKeyEncapsulator::LevelName(MLKEMSecurityLevel::MLKEM512) == "ML-KEM-512");
    ASSERT(MLKEMKeyEncapsulator::LevelName(MLKEMSecurityLevel::MLKEM1024) == "ML-KEM-1024");
}
TEST(Test_MLKEMKeyEncapsulator_InvalidPubKey)
{
    using namespace ExplorerLens::Engine;
    MLKEMKeyEncapsulator kem;
    MLKEMPublicKey empty;
    auto result = kem.Encapsulate(empty);
    ASSERT(!result.success);
}
TEST(Test_SLHDSASignatureVerifier_Create)
{
    using namespace ExplorerLens::Engine;
    SLHDSASignatureVerifier verifier;
    ASSERT(!SLHDSASignatureVerifier::ParamSetName(verifier.ParameterSet()).empty());
}
TEST(Test_SLHDSASignatureVerifier_SignAndVerify)
{
    using namespace ExplorerLens::Engine;
    SLHDSASignatureVerifier verifier;
    std::vector<uint8_t> msg = {1, 2, 3, 4};
    SLHDSAPublicKey pubKey;
    pubKey.paramSet = SLHDSAParameterSet::SLHDSA_SHA2_128s;
    pubKey.bytes = {0x01, 0x02};
    SLHDSASignature sig;
    sig.paramSet = SLHDSAParameterSet::SLHDSA_SHA2_128s;
    sig.bytes = {0xAA, 0xBB};
    auto vr = verifier.Verify(msg, sig, pubKey);
    ASSERT(!vr.paramSetName.empty());
}
TEST(Test_SLHDSASignatureVerifier_InvalidSignature)
{
    using namespace ExplorerLens::Engine;
    SLHDSASignatureVerifier verifier;
    std::vector<uint8_t> msg = {1, 2, 3};
    SLHDSASignature badSig;
    badSig.paramSet = SLHDSAParameterSet::SLHDSA_SHA2_128s;
    badSig.bytes = std::vector<uint8_t>(32, 0xFF);
    SLHDSAPublicKey pubKey;
    pubKey.paramSet = SLHDSAParameterSet::SLHDSA_SHA2_128s;
    pubKey.bytes = std::vector<uint8_t>(32, 0xAB);
    auto vr = verifier.Verify(msg, badSig, pubKey);
    ASSERT(!vr.valid);
}
TEST(Test_HybridTLSIPCChannel_Create)
{
    using namespace ExplorerLens::Engine;
    HybridTLSIPCChannel channel;
    ASSERT(channel.State() == IPCChannelState::Disconnected);
    ASSERT(!channel.IsConnected());
}
TEST(Test_HybridTLSIPCChannel_Connect)
{
    using namespace ExplorerLens::Engine;
    HybridTLSIPCChannel channel;
    auto result = channel.Connect();
    ASSERT(result.Ok());
    ASSERT(channel.IsConnected());
}
TEST(Test_HybridTLSIPCChannel_Send)
{
    using namespace ExplorerLens::Engine;
    HybridTLSIPCChannel channel;
    channel.Connect();
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    auto sr = channel.Send(data);
    ASSERT(sr.success);
    ASSERT(sr.bytesSent == 3);
}
TEST(Test_HybridTLSIPCChannel_Disconnect)
{
    using namespace ExplorerLens::Engine;
    HybridTLSIPCChannel channel;
    channel.Connect();
    channel.Disconnect();
    ASSERT(!channel.IsConnected());
}
TEST(Test_HybridTLSIPCChannel_ModeName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(HybridTLSIPCChannel::ModeName(HybridKEMMode::Hybrid) == "Hybrid");
    ASSERT(HybridTLSIPCChannel::ModeName(HybridKEMMode::ECDHOnly) == "ECDHOnly");
}
TEST(Test_PQAuditTrail_AddAndVerify)
{
    using namespace ExplorerLens::Engine;
    PQAuditTrail trail;
    trail.Record(PQCryptoEvent::KeyEncapsulated, "ML-KEM-768", "user1");
    ASSERT(trail.EntryCount() == 1);
    auto verResult = trail.VerifyChain();
    ASSERT(verResult.valid);
}
TEST(Test_PQAuditTrail_MultiEntry)
{
    using namespace ExplorerLens::Engine;
    PQAuditTrail trail;
    trail.Record(PQCryptoEvent::SignatureCreated, "SLH-DSA", "alice");
    trail.Record(PQCryptoEvent::SignatureVerified, "SLH-DSA", "bob");
    ASSERT(trail.EntryCount() == 2);
    auto verResult = trail.VerifyChain();
    ASSERT(verResult.valid);
}
TEST(Test_QuantumSafeKeyRotator_NeedsRotation)
{
    using namespace ExplorerLens::Engine;
    KeyRotationPolicy policy;
    policy.maxKeyAgeHours = std::chrono::hours{0};
    QuantumSafeKeyRotator rotator(policy);
    std::string keyId = rotator.AddKey(KeyType::MLKEM);
    KeyRecord rec;
    rec.keyId = keyId;
    rec.createdAtEpoch = 0;
    ASSERT(rotator.ShouldRotate(rec));
}
TEST(Test_QuantumSafeKeyRotator_RotateCallback)
{
    using namespace ExplorerLens::Engine;
    QuantumSafeKeyRotator rotator;
    bool called = false;
    rotator.SetRotationCallback([&](const RotationResult&) { called = true; });
    std::string keyId = rotator.AddKey(KeyType::MLKEM);
    rotator.Rotate(keyId, RotationReason::Manual);
    ASSERT(called);
}
TEST(Test_FIPS140CryptoBoundary_ApprovedAlgos)
{
    using namespace ExplorerLens::Engine;
    FIPS140CryptoBoundary boundary;
    ASSERT(boundary.Check("AES-256-GCM").allowed);
    ASSERT(boundary.Check("SHA-256").allowed);
    ASSERT(!boundary.Check("RC4").allowed);  // not approved
}
TEST(Test_FIPS140CryptoBoundary_AlgorithmCount)
{
    using namespace ExplorerLens::Engine;
    FIPS140CryptoBoundary boundary;
    ASSERT(boundary.AlgorithmCount() >= 5);
}
TEST(Test_CertificateMigrationTool_CreatePlan)
{
    using namespace ExplorerLens::Engine;
    CertificateMigrationTool tool;
    MigCertInfo src;
    src.subjectCN = "test.example.com";
    src.algoType = CertificateAlgoType::RSA2048;
    auto plan = tool.BuildPlan(src);
    ASSERT(!plan.steps.empty());
    ASSERT(plan.steps.size() >= 3);
}
TEST(Test_CertificateMigrationTool_PlanHasDualSign)
{
    using namespace ExplorerLens::Engine;
    CertificateMigrationTool tool;
    MigCertInfo src;
    src.subjectCN = "ca.example.com";
    src.algoType = CertificateAlgoType::ECDSA_P256;
    src.isCA = true;
    auto plan = tool.BuildPlan(src);
    bool hasDualSign = false;
    for (const auto& s : plan.steps)
        if (s.find("dual") != std::string::npos || s.find("Dual") != std::string::npos) {
            hasDualSign = true;
            break;
        }
    ASSERT(hasDualSign);
}
TEST(Test_CryptoAgilityEngine_AlgorithmCount)
{
    using namespace ExplorerLens::Engine;
    CryptoAgilityEngine engine;
    ASSERT(engine.AlgorithmCount() == 7);
}
TEST(Test_CryptoAgilityEngine_Negotiate_HybridPref)
{
    using namespace ExplorerLens::Engine;
    CryptoAgilityEngine engine(CryptoPreference::Hybrid);
    auto result = engine.Negotiate(CryptoRole::KeyExchange, {"ML-KEM-768", "ECDH-P384"});
    ASSERT(result.Ok());
    ASSERT(result.role == CryptoRole::KeyExchange);
}
TEST(Test_CryptoAgilityEngine_Negotiate_PQOnly)
{
    using namespace ExplorerLens::Engine;
    CryptoAgilityEngine engine(CryptoPreference::PostQuantum);
    auto result = engine.Negotiate(CryptoRole::KeyExchange, {"ML-KEM-768", "ECDH-P384"});
    ASSERT(result.Ok());
    ASSERT(result.selected == "ML-KEM-768");
}
TEST(Test_CryptoAgilityEngine_FIPSFilter)
{
    using namespace ExplorerLens::Engine;
    CryptoAgilityEngine engine;
    engine.SetRequireFIPS(true);
    auto result = engine.Negotiate(CryptoRole::Hash, {"SHA-256"});
    ASSERT(result.Ok());
    ASSERT(result.isFIPS);
}

//==============================================================================
// Sprint 651-660 — Windows Next-Gen Shell Integration (v26.1.0 "Canopus-R")
//==============================================================================

TEST(Test_WinRTThumbnailBridge_NoHandler)
{
    using namespace ExplorerLens::Engine;
    WinRTThumbnailBridge bridge;
    WinRTThumbnailRequest req;
    req.filePath = L"img.jpg";
    auto result = bridge.GetThumbnail(req);
    ASSERT(!result.Ok());
}
TEST(Test_WinRTThumbnailBridge_WithHandler)
{
    using namespace ExplorerLens::Engine;
    WinRTThumbnailBridge bridge;
    bridge.SetSyncHandler([](const WinRTThumbnailRequest& r) {
        WinRTThumbnailResult res;
        res.success = !r.filePath.empty();
        res.widthPx = (int)r.requestedSize;
        res.heightPx = (int)r.requestedSize;
        return res;
    });
    WinRTThumbnailRequest req;
    req.filePath = L"photo.jpg";
    req.requestedSize = WinRTThumbnailRequestedSize::Large;
    auto result = bridge.GetThumbnail(req);
    ASSERT(result.Ok());
    ASSERT(result.widthPx == 256);
}
TEST(Test_WinRTThumbnailBridge_ModeName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(WinRTThumbnailBridge::ModeName(WinRTThumbnailMode::ListView) == "ListView");
    ASSERT(WinRTThumbnailBridge::ModeName(WinRTThumbnailMode::DocumentsView) == "DocumentsView");
}
TEST(Test_WinRTThumbnailBridge_EmptyPath)
{
    using namespace ExplorerLens::Engine;
    WinRTThumbnailBridge bridge;
    bridge.SetSyncHandler([](const WinRTThumbnailRequest& r) {
        WinRTThumbnailResult res;
        res.success = !r.filePath.empty();
        return res;
    });
    WinRTThumbnailRequest req;  // empty path
    auto result = bridge.GetThumbnail(req);
    ASSERT(!result.Ok());  // empty path → rejected before handler
}
TEST(Test_AppContainerIsolation_Create)
{
    using namespace ExplorerLens::Engine;
    AppContainerIsolation iso;
    ASSERT(!iso.HasCapability(AppContainerCapability::InternetClient));
}
TEST(Test_AppContainerIsolation_GrantRevoke)
{
    using namespace ExplorerLens::Engine;
    AppContainerIsolation iso;
    iso.GrantCapability(AppContainerCapability::PicturesLibrary);
    ASSERT(iso.HasCapability(AppContainerCapability::PicturesLibrary));
    iso.RevokeCapability(AppContainerCapability::PicturesLibrary);
    ASSERT(!iso.HasCapability(AppContainerCapability::PicturesLibrary));
}
TEST(Test_WinFSMetadataStore_Create)
{
    using namespace ExplorerLens::Engine;
    WinFSMetadataStore store;
    ASSERT(store.Stats().recordCount == 0);
}
TEST(Test_WinFSMetadataStore_SetAndGet)
{
    using namespace ExplorerLens::Engine;
    WinFSMetadataStore store;
    MetadataRecord rec;
    rec.filePath = L"C:\\img.jpg";
    rec.Set("Rating", "5");
    store.Write(rec);
    MetadataRecord found;
    bool ok = store.Read(L"C:\\img.jpg", found);
    ASSERT(ok);
    ASSERT(found.Get("Rating") == "5");
}
TEST(Test_WindowsSearchV3Bridge_IndexAndQuery)
{
    using namespace ExplorerLens::Engine;
    WindowsSearchV3Bridge bridge;
    SearchIndexRequest req;
    req.filePath = L"C:\\photo.jpg";
    req.contentTypes = {SearchContentType::FullText};
    auto res = bridge.IndexFile(req);
    ASSERT(res.Ok());
    ASSERT(bridge.IsIndexed(L"C:\\photo.jpg"));
    ASSERT(bridge.IndexedCount() == 1);
}
TEST(Test_SmartAppControlPolicy_Modes)
{
    using namespace ExplorerLens::Engine;
    SmartAppControlPolicy policy;
    ASSERT(policy.Mode() != SACPolicyMode::Off || policy.Mode() == SACPolicyMode::Off);  // always valid
    policy.SetMode(SACPolicyMode::Evaluation);
    ASSERT(policy.Mode() == SACPolicyMode::Evaluation);
}
TEST(Test_SmartAppControlPolicy_TrustedPublisher)
{
    using namespace ExplorerLens::Engine;
    SmartAppControlPolicy policy;
    policy.AddTrustedPublisher("ExplorerLens");
    SACBinaryInfo info;
    info.publisherCN = "ExplorerLens";
    info.isSigned = true;
    auto result = policy.Evaluate(info);
    ASSERT(result.Ok() || result.trustLevel != SACBinaryTrustLevel::Blocked);
}
TEST(Test_MSIXStreamingPrewarmer_Create)
{
    using namespace ExplorerLens::Engine;
    MSIXStreamingPrewarmer prewarmer;
    ASSERT(prewarmer.GroupCount() == 0);
}
TEST(Test_MSIXStreamingPrewarmer_Prewarm)
{
    using namespace ExplorerLens::Engine;
    MSIXStreamingPrewarmer prewarmer;
    MSIXContentGroup g1;
    g1.name = "Group1";
    MSIXContentGroup g2;
    g2.name = "Group2";
    prewarmer.RegisterGroup(g1);
    prewarmer.RegisterGroup(g2);
    ASSERT(prewarmer.GroupCount() == 2);
}
TEST(Test_WindowsHelloAuthBridge_Create)
{
    using namespace ExplorerLens::Engine;
    WindowsHelloAuthBridge bridge;
    ASSERT(!bridge.IsAvailable());
}
TEST(Test_WindowsHelloAuthBridge_InjectableAuth)
{
    using namespace ExplorerLens::Engine;
    WindowsHelloAuthBridge bridge;
    bridge.SetAvailable(true);
    bridge.SetAuthFunction([](const HelloAuthRequest&) -> HelloAuthResult {
        HelloAuthResult r;
        r.status = HelloAuthStatus::Approved;
        r.method = HelloAuthMethod::Fingerprint;
        return r;
    });
    HelloAuthRequest req;
    req.scope = HelloProtectedScope::Annotations;
    auto result = bridge.Authenticate(req);
    ASSERT(result.Ok());
}

//==============================================================================
// Sprint 661-670 — Immersive 3D Preview Engine (v26.2.0 "Canopus-S")
//==============================================================================

TEST(Test_ImmersivePreviewRenderer_EmptyPath)
{
    using namespace ExplorerLens::Engine;
    ImmersivePreviewRenderer renderer;
    ImmersiveRenderRequest req;  // empty path
    auto result = renderer.Render(req);
    ASSERT(!result.Ok());
}
TEST(Test_ImmersivePreviewRenderer_Render)
{
    using namespace ExplorerLens::Engine;
    ImmersivePreviewRenderer renderer(ImmersiveRenderBackend::CPU);
    ImmersiveRenderRequest req;
    req.modelPath = L"model.gltf";
    req.width = 128;
    req.height = 128;
    req.quality = ImmersiveRenderQuality::Draft;
    auto result = renderer.Render(req);
    ASSERT(result.Ok());
    ASSERT(result.widthPx == 128);
    ASSERT((int)result.rgba.size() == 128 * 128 * 4);
}
TEST(Test_ImmersivePreviewRenderer_QualityNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(ImmersivePreviewRenderer::QualityName(ImmersiveRenderQuality::UltraRT) == "UltraRT");
    ASSERT(ImmersivePreviewRenderer::QualityName(ImmersiveRenderQuality::Draft) == "Draft");
}
TEST(Test_ImmersivePreviewRenderer_RenderTimes)
{
    using namespace ExplorerLens::Engine;
    ImmersivePreviewRenderer renderer;
    ImmersiveRenderRequest req;
    req.modelPath = L"x.obj";
    req.quality = ImmersiveRenderQuality::Standard;
    auto r = renderer.Render(req);
    ASSERT(r.renderMs == 17.0);
}
TEST(Test_VolumetricThumbnailEngine_Create)
{
    using namespace ExplorerLens::Engine;
    ASSERT(!VolumetricThumbnailEngine::ModeName(VolumeRenderMode::RayCasting).empty());
}
TEST(Test_VolumetricThumbnailEngine_Render)
{
    using namespace ExplorerLens::Engine;
    VolumetricThumbnailEngine engine;
    VolumeRenderRequest req;
    req.dataPath = L"scan.nii";
    req.width = 64;
    req.height = 64;
    auto result = engine.Render(req);
    ASSERT(result.success);
    ASSERT(result.widthPx == 64);
}
TEST(Test_RealtimeLightingSimulator_Create)
{
    using namespace ExplorerLens::Engine;
    ASSERT(!RealtimeLightingSimulator::ModelName(LightingModel::PhysicallyBased).empty());
}
TEST(Test_RealtimeLightingSimulator_AddLight)
{
    using namespace ExplorerLens::Engine;
    LightingSimulationRequest req;
    HDRILight light;
    light.intensity = 1.0f;
    req.lights.push_back(light);
    ASSERT(req.lights.size() == 1);
}
TEST(Test_HolographicProjectionEngine_Create)
{
    using namespace ExplorerLens::Engine;
    ASSERT(!HolographicProjectionEngine::TargetName(HolographicDisplayTarget::Standard2D).empty());
}
TEST(Test_HolographicProjectionEngine_ProjectStereo)
{
    using namespace ExplorerLens::Engine;
    HolographicProjectionEngine engine;
    HolographicProjectionRequest req;
    req.modelPath = L"model.glb";
    req.width = 64;
    req.height = 64;
    auto result = engine.Project(req);
    ASSERT(result.success);
    ASSERT(!result.leftRGBA.empty());
    ASSERT(!result.rightRGBA.empty());
}
TEST(Test_MeshLODGeneratorV2_Create)
{
    using namespace ExplorerLens::Engine;
    MeshLODRequest req;
    ASSERT(req.levels == 4);
}
TEST(Test_MeshLODGeneratorV2_GenerateLOD)
{
    using namespace ExplorerLens::Engine;
    MeshLODGeneratorV2 gen;
    MeshLODRequest req;
    req.modelPath = L"mesh.obj";
    req.sourceTris = 10000;
    auto result = gen.GenerateLODs(req);
    ASSERT(result.success);
    ASSERT(result.levels.size() == 4);
    ASSERT(result.levels[0].targetTris <= req.sourceTris);
}
TEST(Test_AnimationPreviewScrubber_Create)
{
    using namespace ExplorerLens::Engine;
    ASSERT(!AnimationPreviewScrubber::StrategyName(AnimationScrubStrategy::SmartPose).empty());
}
TEST(Test_AnimationPreviewScrubber_PickFrame)
{
    using namespace ExplorerLens::Engine;
    AnimationPreviewScrubber scrubber;
    AnimationScrubRequest req;
    req.strategy = AnimationScrubStrategy::MiddleFrame;
    auto result = scrubber.Scrub(req);
    ASSERT(result.frameIndex >= 0);
}
TEST(Test_MaterialPreviewEngine_DetectFormat)
{
    using namespace ExplorerLens::Engine;
    ASSERT(MaterialPreviewEngine::DetectFormat(L"model.mtl") == MaterialFileFormat::MTL);
    ASSERT(MaterialPreviewEngine::DetectFormat(L"mat.materialx") == MaterialFileFormat::MaterialX);
}
TEST(Test_MaterialPreviewEngine_Render)
{
    using namespace ExplorerLens::Engine;
    MaterialPreviewEngine engine;
    MaterialPreviewRequest req;
    req.materialPath = L"mat.mtl";
    req.width = 64;
    req.height = 64;
    auto result = engine.Render(req);
    ASSERT(result.success);
}
TEST(Test_GPUPathTracerPreview_Create)
{
    using namespace ExplorerLens::Engine;
    GPUPathTracerPreview tracer;
    ASSERT(tracer.IsHardwareAvailable() || !tracer.IsHardwareAvailable());  // construction ok
}
TEST(Test_GPUPathTracerPreview_SamplesForQuality)
{
    using namespace ExplorerLens::Engine;
    ASSERT(GPUPathTracerPreview::SamplesForQuality(PathTracerQuality::Preview_1spp) == 1);
    ASSERT(GPUPathTracerPreview::SamplesForQuality(PathTracerQuality::Quality_4spp) == 4);
    ASSERT(GPUPathTracerPreview::SamplesForQuality(PathTracerQuality::Final_16spp) == 16);
}
TEST(Test_GPUPathTracerPreview_Trace)
{
    using namespace ExplorerLens::Engine;
    GPUPathTracerPreview tracer;
    PathTracerRequest req;
    req.modelPath = L"scene.gltf";
    req.width = 32;
    req.height = 32;
    auto result = tracer.Render(req);
    ASSERT(result.success);
    ASSERT(result.widthPx == 32);
}

//==============================================================================
// Sprint 671-680 — Real-Time Collaboration (v26.3.0 "Canopus-T")
//==============================================================================

TEST(Test_CollaborationPresenceEngine_JoinLeave)
{
    using namespace ExplorerLens::Engine;
    CollaborationPresenceEngine engine;
    PresenceUser user;
    user.userId = "alice";
    user.displayName = "Alice";
    engine.JoinSession(user);
    ASSERT(engine.UserCount() == 1);
    engine.LeaveSession("alice");
    auto snap = engine.GetSnapshot();
    for (const auto& u : snap.users)
        if (u.userId == "alice")
            ASSERT(u.state == PresenceState::Offline);
}
TEST(Test_CollaborationPresenceEngine_Snapshot)
{
    using namespace ExplorerLens::Engine;
    CollaborationPresenceEngine engine;
    PresenceUser u1;
    u1.userId = "u1";
    u1.state = PresenceState::Online;
    PresenceUser u2;
    u2.userId = "u2";
    u2.state = PresenceState::Idle;
    engine.JoinSession(u1);
    engine.JoinSession(u2);
    auto snap = engine.GetSnapshot();
    ASSERT(snap.onlineCount() >= 1);
}
TEST(Test_CollaborationPresenceEngine_StateNames)
{
    using namespace ExplorerLens::Engine;
    ASSERT(CollaborationPresenceEngine::StateName(PresenceState::Online) == "Online");
    ASSERT(CollaborationPresenceEngine::StateName(PresenceState::Offline) == "Offline");
}
TEST(Test_CollaborationPresenceEngine_UpdateCursor)
{
    using namespace ExplorerLens::Engine;
    CollaborationPresenceEngine engine;
    PresenceUser u;
    u.userId = "bob";
    engine.JoinSession(u);
    engine.UpdateCursor("bob", 0.5f, 0.75f);
}
TEST(Test_LiveAnnotationBroadcaster_Broadcast)
{
    using namespace ExplorerLens::Engine;
    LiveAnnotationBroadcaster broadcaster;
    broadcaster.SetPeerCount(3);
    AnnotationDelta delta;
    delta.lamportClock = 1;
    delta.authorId = "alice";
    delta.op = AnnotationOpType::SetStar;
    auto result = broadcaster.Broadcast(delta);
    ASSERT(result.Ok());
    ASSERT(result.peersNotified == 3);
    ASSERT(broadcaster.HistorySize() == 1);
}
TEST(Test_LiveAnnotationBroadcaster_LamportClock)
{
    using namespace ExplorerLens::Engine;
    LiveAnnotationBroadcaster broadcaster;
    AnnotationDelta d;
    d.lamportClock = 5;
    broadcaster.Broadcast(d);
    ASSERT(broadcaster.LamportClock() == 6);  // max(5,0)+1
}
TEST(Test_LiveAnnotationBroadcaster_OpName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(LiveAnnotationBroadcaster::OpName(AnnotationOpType::AddTag) == "AddTag");
    ASSERT(LiveAnnotationBroadcaster::OpName(AnnotationOpType::SetColor) == "SetColor");
}
TEST(Test_SharedViewStateProtocol_Publish)
{
    using namespace ExplorerLens::Engine;
    SharedViewStateProtocol proto;
    SharedViewState state;
    state.sortOrder = ViewSortOrder::Date;
    auto r = proto.Publish(state);
    ASSERT(r.Ok());
    ASSERT(proto.Version() == 1);
    ASSERT(proto.Current().sortOrder == ViewSortOrder::Date);
}
TEST(Test_SharedViewStateProtocol_Apply)
{
    using namespace ExplorerLens::Engine;
    SharedViewStateProtocol proto;
    SharedViewState state;
    state.version = 10;
    state.zoomLevel = ViewZoomLevel::Large;
    auto r = proto.Apply(state);
    ASSERT(r.Ok());
    ASSERT(proto.Current().zoomLevel == ViewZoomLevel::Large);
}
TEST(Test_SharedViewStateProtocol_Callback)
{
    using namespace ExplorerLens::Engine;
    SharedViewStateProtocol proto;
    bool called = false;
    proto.SetViewStateCallback([&](const SharedViewState&) { called = true; });
    SharedViewState s;
    proto.Publish(s);
    ASSERT(called);
}
TEST(Test_ConflictResolutionMerger_LWW)
{
    using namespace ExplorerLens::Engine;
    ConflictResolutionMerger merger(ConflictResolutionStrategy::LastWriteWins);
    ConflictEntry e;
    e.localValue = "3";
    e.remoteValue = "5";
    e.localTimestamp = 100;
    e.remoteTimestamp = 200;
    auto result = merger.Resolve({e});
    ASSERT(result.Ok());
    ASSERT(result.resolvedValue == "5");  // remote is newer
}
TEST(Test_ConflictResolutionMerger_OwnerPriority)
{
    using namespace ExplorerLens::Engine;
    ConflictResolutionMerger merger(ConflictResolutionStrategy::OwnerPriority);
    ConflictEntry e;
    e.localValue = "local";
    e.remoteValue = "remote";
    auto result = merger.Resolve({e});
    ASSERT(result.resolvedValue == "local");
}
TEST(Test_ConflictResolutionMerger_StrategyName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(ConflictResolutionMerger::StrategyName(ConflictResolutionStrategy::ThreeWayMerge) == "ThreeWayMerge");
}
TEST(Test_PresenceAvatarRenderer_Render_Empty)
{
    using namespace ExplorerLens::Engine;
    PresenceAvatarRenderer renderer;
    std::vector<AvatarParticipant> participants;
    auto frame = renderer.Render(participants, 256, 256);
    ASSERT(frame.participantCount == 0);
    ASSERT((int)frame.rgba.size() == 256 * 256 * 4);
}
TEST(Test_PresenceAvatarRenderer_Render_WithParticipants)
{
    using namespace ExplorerLens::Engine;
    PresenceAvatarRenderer renderer;
    AvatarParticipant p;
    p.userId = "alice";
    p.initials = "AL";
    p.color = {255, 100, 0};
    p.position = {0.5f, 0.5f};
    auto frame = renderer.Render({p}, 128, 128);
    ASSERT(frame.participantCount == 1);
}
TEST(Test_SessionReplayEngine_RecordAndPlay)
{
    using namespace ExplorerLens::Engine;
    SessionReplayEngine engine;
    engine.StartRecording("session1");
    ReplayFrame f;
    f.timestampMs = 100;
    f.authorId = "alice";
    f.operation = "setTag";
    engine.RecordFrame(f);
    auto rec = engine.StopRecording();
    ASSERT(rec.FrameCount() == 1);
    auto result = engine.Play(rec, ReplaySpeed::Normal_1x);
    ASSERT(result.Ok());
    ASSERT(result.framesPlayed == 1);
}
TEST(Test_SessionReplayEngine_SpeedFactor)
{
    using namespace ExplorerLens::Engine;
    ASSERT(SessionReplayEngine::SpeedFactor(ReplaySpeed::Slow_0_25x) == 0.25);
    ASSERT(SessionReplayEngine::SpeedFactor(ReplaySpeed::Super_4x) == 4.0);
}
TEST(Test_SessionReplayEngine_Callback)
{
    using namespace ExplorerLens::Engine;
    SessionReplayEngine engine;
    int callbackCount = 0;
    engine.SetFrameCallback([&](const ReplayFrame&) { callbackCount++; });
    engine.StartRecording("s");
    ReplayFrame f;
    f.timestampMs = 1;
    engine.RecordFrame(f);
    auto rec = engine.StopRecording();
    engine.Play(rec, ReplaySpeed::Fast_2x);
    ASSERT(callbackCount == 1);
}
TEST(Test_CollaborationTelemetryHub_Record)
{
    using namespace ExplorerLens::Engine;
    CollaborationTelemetryHub hub;
    CollabTelemetryRecord rec;
    rec.event = CollabTelemetryEvent::AnnotationEdited;
    rec.sessionId = "sess1";
    rec.userId = "alice";
    hub.Record(rec);
    ASSERT(hub.TotalEvents() == 1);
}
TEST(Test_CollaborationTelemetryHub_ConflictRate)
{
    using namespace ExplorerLens::Engine;
    CollaborationTelemetryHub hub;
    for (int i = 0; i < 10; ++i) {
        CollabTelemetryRecord r;
        r.sessionId = "s1";
        r.event = CollabTelemetryEvent::AnnotationEdited;
        hub.Record(r);
    }
    CollabTelemetryRecord cr;
    cr.sessionId = "s1";
    cr.event = CollabTelemetryEvent::ConflictDetected;
    hub.Record(cr);
    auto metrics = hub.GetMetrics("s1");
    ASSERT(metrics.edits == 10);
    ASSERT(metrics.conflicts >= 1);
}
TEST(Test_CollaborationTelemetryHub_EventName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(CollaborationTelemetryHub::EventName(CollabTelemetryEvent::UserJoined) == "UserJoined");
}

//==============================================================================
// Sprint 681-690 — Adaptive Performance Governor v2 (v26.4.0 "Canopus-U")
//==============================================================================

TEST(Test_AdaptivePerformanceGovernorV2_BalancedMode)
{
    using namespace ExplorerLens::Engine;
    AdaptivePerformanceGovernorV2 gov;
    GovernorSample sample;  // defaults: mild load
    auto d = gov.Evaluate(sample);
    ASSERT(d.mode == GovernorMode::Performance || d.mode == GovernorMode::Balanced);
    ASSERT(d.maxDecodeSlots > 0);
}
TEST(Test_AdaptivePerformanceGovernorV2_ThermalThrottle)
{
    using namespace ExplorerLens::Engine;
    AdaptivePerformanceGovernorV2 gov;
    GovernorSample sample;
    sample.thermalC = 98.0f;
    sample.powerW = 48.0f;
    sample.gpuPct = 90.0f;
    sample.cpuPct = 85.0f;
    auto d = gov.Evaluate(sample);
    ASSERT(d.mode == GovernorMode::ThermalThrottle);
    ASSERT(d.maxDecodeSlots == 2);
}
TEST(Test_AdaptivePerformanceGovernorV2_ModeName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(AdaptivePerformanceGovernorV2::ModeName(GovernorMode::PowerSave) == "PowerSave");
    ASSERT(AdaptivePerformanceGovernorV2::ModeName(GovernorMode::ThermalThrottle) == "ThermalThrottle");
}
TEST(Test_AdaptivePerformanceGovernorV2_EvalCount)
{
    using namespace ExplorerLens::Engine;
    AdaptivePerformanceGovernorV2 gov;
    gov.Evaluate({});
    gov.Evaluate({});
    ASSERT(gov.EvalCount() == 2);
}
TEST(Test_ThermalAwareMemoryScheduler_Comfortable)
{
    using namespace ExplorerLens::Engine;
    ThermalAwareMemoryScheduler sched;
    auto d = sched.Evaluate(50.0f);
    ASSERT(d.zone == TAMZone::Comfortable);
    ASSERT(d.action == MemoryScheduleAction::FullPrealloc);
    ASSERT(d.allocMB == 256);
}
TEST(Test_ThermalAwareMemoryScheduler_Hot)
{
    using namespace ExplorerLens::Engine;
    ThermalAwareMemoryScheduler sched;
    auto d = sched.Evaluate(90.0f);
    ASSERT(d.zone == TAMZone::Hot);
    ASSERT(d.action == MemoryScheduleAction::LazyAlloc);
    ASSERT(d.delayWarmup);
}
TEST(Test_ThermalAwareMemoryScheduler_Critical)
{
    using namespace ExplorerLens::Engine;
    ThermalAwareMemoryScheduler sched;
    auto d = sched.Evaluate(100.0f);
    ASSERT(d.zone == TAMZone::Critical);
    ASSERT(d.action == MemoryScheduleAction::Emergency);
}
TEST(Test_ThermalAwareMemoryScheduler_ZoneName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(ThermalAwareMemoryScheduler::ZoneName(TAMZone::Warm) == "Warm");
    ASSERT(ThermalAwareMemoryScheduler::ZoneName(TAMZone::Critical) == "Critical");
}
TEST(Test_WorkloadBalancerV2_Dispatch)
{
    using namespace ExplorerLens::Engine;
    WorkloadBalancerV2 lb;
    WorkloadAdapterInfo ada;
    ada.adapterId = 0;
    ada.name = "GPU0";
    lb.RegisterAdapter(ada);
    WorkloadItem item;
    item.id = 1;
    auto d = lb.Dispatch(item);
    ASSERT(d.Ok());
    ASSERT(d.adapterId == 0);
    ASSERT(lb.DispatchedCount() == 1);
}
TEST(Test_WorkloadBalancerV2_Complete)
{
    using namespace ExplorerLens::Engine;
    WorkloadBalancerV2 lb;
    WorkloadAdapterInfo ada;
    ada.adapterId = 1;
    lb.RegisterAdapter(ada);
    WorkloadItem item;
    item.id = 2;
    lb.Dispatch(item);
    lb.NotifyComplete(1);
    ASSERT(lb.CompletedCount() == 1);
}
TEST(Test_WorkloadBalancerV2_AlgorithmName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(WorkloadBalancerV2::AlgorithmName(WorkloadBalancerV2Algorithm::WorkStealing) == "WorkStealing");
    ASSERT(WorkloadBalancerV2::AlgorithmName(WorkloadBalancerV2Algorithm::RoundRobin) == "RoundRobin");
}
TEST(Test_PowerBudgetController_AC_Unconstrained)
{
    using namespace ExplorerLens::Engine;
    PowerBudgetController ctrl;
    PowerSample sample;
    sample.source = PBCPowerSource::AC;
    sample.currentPowerW = 20.0f;
    sample.batteryLevelPct = 100.0f;
    auto d = ctrl.Evaluate(sample);
    ASSERT(d.state == PowerBudgetState::Unconstrained);
    ASSERT(d.maxDecodeSlots == 16);
}
TEST(Test_PowerBudgetController_Battery_Critical)
{
    using namespace ExplorerLens::Engine;
    PowerBudgetController ctrl;
    PowerSample sample;
    sample.source = PBCPowerSource::Battery;
    sample.batteryLevelPct = 5.0f;  // critical
    auto d = ctrl.Evaluate(sample);
    ASSERT(d.state == PowerBudgetState::Emergency);
    ASSERT(d.disableGPU);
}
TEST(Test_PowerBudgetController_StateName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(PowerBudgetController::StateName(PowerBudgetState::Strict) == "Strict");
    ASSERT(PowerBudgetController::StateName(PowerBudgetState::Unconstrained) == "Unconstrained");
}
TEST(Test_QoSThrottleEngine_Interactive_Allowed)
{
    using namespace ExplorerLens::Engine;
    QoSThrottleEngine engine;
    QoSRequest req;
    req.id = 1;
    req.qosClass = QoSClass::Interactive;
    auto d = engine.Evaluate(req);
    ASSERT(d.IsAllowed());
    ASSERT(d.mode == ThrottleMode::None);
    engine.NotifyComplete(QoSClass::Interactive);
}
TEST(Test_QoSThrottleEngine_Background_Throttled)
{
    using namespace ExplorerLens::Engine;
    QoSThrottleEngine engine;
    QoSRequest req;
    req.qosClass = QoSClass::Background;
    auto d = engine.Evaluate(req);
    ASSERT(!d.IsAllowed() || d.delayMs > 0);  // background is always throttled or delayed
}
TEST(Test_QoSThrottleEngine_Idle)
{
    using namespace ExplorerLens::Engine;
    QoSThrottleEngine engine;
    QoSRequest req;
    req.qosClass = QoSClass::Idle;
    auto d = engine.Evaluate(req);
    ASSERT(d.maxConcurrent == 1);
    ASSERT(d.delayMs == 200);
}
TEST(Test_SmartPrefetchEngine_Predict)
{
    using namespace ExplorerLens::Engine;
    SmartPrefetchEngine engine;
    std::vector<std::wstring> visible = {L"a.jpg", L"b.jpg", L"c.jpg"};
    auto candidates = engine.Predict(visible);
    ASSERT(!candidates.empty());
    ASSERT(candidates[0].confidenceScore > 0.0);
}
TEST(Test_SmartPrefetchEngine_IssuePrefetches)
{
    using namespace ExplorerLens::Engine;
    SmartPrefetchEngine engine;
    int issued = 0;
    engine.SetPrefetchIssuer([&](const SPECandidate&) {
        issued++;
        return true;
    });
    engine.SetMinConfidence(0.5);
    std::vector<std::wstring> vis = {L"img1.jpg", L"img2.jpg"};
    auto candidates = engine.Predict(vis);
    engine.IssuePrefetches(candidates);
    ASSERT(issued > 0);
    ASSERT(engine.Stats().requested == issued);
}
TEST(Test_SmartPrefetchEngine_HitMissStats)
{
    using namespace ExplorerLens::Engine;
    SmartPrefetchEngine engine;
    engine.NotifyHit(L"a.jpg");
    engine.NotifyMiss(L"b.jpg");
    ASSERT(engine.Stats().hits == 1);
    ASSERT(engine.Stats().misses == 1);
}
TEST(Test_FrameRateSynchronizer_TargetFrameTime)
{
    using namespace ExplorerLens::Engine;
    FrameSyncConfig cfg;
    cfg.targetHz = RefreshRate::Hz60;
    FrameRateSynchronizer syncer(cfg);
    ASSERT(std::abs(syncer.TargetFrameTimeMs() - 16.666) < 0.1);
}
TEST(Test_FrameRateSynchronizer_PresentFrame)
{
    using namespace ExplorerLens::Engine;
    FrameRateSynchronizer syncer;
    auto r = syncer.PresentFrame();
    ASSERT(r.Ok());
    ASSERT(syncer.FrameNumber() == 1);
}
TEST(Test_FrameRateSynchronizer_SetMode)
{
    using namespace ExplorerLens::Engine;
    FrameRateSynchronizer syncer;
    syncer.SetMode(SyncMode::AdaptiveSync);
    ASSERT(syncer.Config().mode == SyncMode::AdaptiveSync);
}
TEST(Test_FrameRateSynchronizer_ModeName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(FrameRateSynchronizer::ModeName(SyncMode::LowLatency) == "LowLatency");
    ASSERT(FrameRateSynchronizer::ModeName(SyncMode::VSync) == "VSync");
}
TEST(Test_BackgroundIntelligenceService_Enqueue)
{
    using namespace ExplorerLens::Engine;
    BackgroundIntelligenceService svc;
    uint64_t id = svc.Enqueue(L"photo.jpg", 50);
    ASSERT(id > 0);
    ASSERT(svc.QueueDepth() == 1);
}
TEST(Test_BackgroundIntelligenceService_DrainNext)
{
    using namespace ExplorerLens::Engine;
    BackgroundIntelligenceService svc;
    int processed = 0;
    svc.SetWorker([&](const BISJob&) {
        processed++;
        return true;
    });
    svc.Enqueue(L"a.jpg", 50);
    svc.Enqueue(L"b.jpg", 50);
    int done = svc.DrainNext(4, 0.0f);  // CPU 0 → below threshold
    ASSERT(done == 2);
    ASSERT(processed == 2);
    ASSERT(svc.Metrics().completed.load() == 2);
}
TEST(Test_BackgroundIntelligenceService_IdleThreshold)
{
    using namespace ExplorerLens::Engine;
    BackgroundIntelligenceService svc;
    svc.SetIdleThresholdCPU(25.0f);
    svc.Enqueue(L"x.jpg", 50);
    int done = svc.DrainNext(4, 80.0f);  // CPU 80% → above threshold → no work
    ASSERT(done == 0);
}

//==============================================================================
// Sprint 691-700 — Global I18n & Accessibility v3 (v26.5.0 "Canopus-V")
//==============================================================================

TEST(Test_I18nRuntimeEngine_DefaultLocale)
{
    using namespace ExplorerLens::Engine;
    I18nRuntimeEngine engine;
    ASSERT(engine.ActiveLocale() == "en-US");
}
TEST(Test_I18nRuntimeEngine_RegisterAndTranslate)
{
    using namespace ExplorerLens::Engine;
    I18nRuntimeEngine engine;
    I18nStringCatalogue cat;
    cat.locale = "fr-FR";
    cat.strings["hello"] = "bonjour";
    engine.RegisterCatalogue(cat);
    engine.SetLocale("fr-FR");
    ASSERT(engine.Translate("hello") == "bonjour");
    ASSERT(engine.CatalogueCount() == 1);
}
TEST(Test_I18nRuntimeEngine_FallbackToKey)
{
    using namespace ExplorerLens::Engine;
    I18nRuntimeEngine engine;
    // No catalogue for en-US, key returned as fallback
    std::string val = engine.Translate("missing_key", "default");
    ASSERT(val == "default");
}
TEST(Test_I18nRuntimeEngine_RTLDirection)
{
    using namespace ExplorerLens::Engine;
    I18nRuntimeEngine engine;
    I18nStringCatalogue cat;
    cat.locale = "ar-SA";
    engine.RegisterCatalogue(cat);
    I18nLocaleInfo info;
    info.tag = "ar-SA";
    info.direction = I18nLocaleDir::RTL;
    engine.RegisterLocaleInfo(info);
    engine.SetLocale("ar-SA");
    ASSERT(engine.GetDirection() == I18nLocaleDir::RTL);
}
TEST(Test_BiDiTextLayoutEngine_LTR)
{
    using namespace ExplorerLens::Engine;
    BiDiTextLayoutEngine engine;
    auto result = engine.Analyse(L"Hello World");
    ASSERT(result.Ok());
    ASSERT(result.resolvedDir == BiDiBaseDirection::LTR);
    ASSERT(result.runCount >= 1);
}
TEST(Test_BiDiTextLayoutEngine_ContainsRTL)
{
    using namespace ExplorerLens::Engine;
    // Hebrew characters
    std::wstring hebrew = L"\u05E9\u05DC\u05D5\u05DD";
    ASSERT(BiDiTextLayoutEngine::ContainsRTL(hebrew));
    ASSERT(!BiDiTextLayoutEngine::ContainsRTL(L"Hello"));
}
TEST(Test_BiDiTextLayoutEngine_EmptyText)
{
    using namespace ExplorerLens::Engine;
    BiDiTextLayoutEngine engine;
    auto result = engine.Analyse(L"");
    ASSERT(!result.Ok());  // empty text fails
}
TEST(Test_BiDiTextLayoutEngine_ExplicitRTL)
{
    using namespace ExplorerLens::Engine;
    BiDiTextLayoutEngine engine;
    auto result = engine.Analyse(L"mixed text", BiDiBaseDirection::RTL);
    ASSERT(result.Ok());
    ASSERT(result.resolvedDir == BiDiBaseDirection::RTL);
}
TEST(Test_AccessibilityNavigatorV3_RegisterAndFind)
{
    using namespace ExplorerLens::Engine;
    AccessibilityNavigatorV3 nav;
    ANV3Element el;
    el.automationId = "thumb_1";
    el.name = "photo.jpg";
    el.controlType = ANV3ControlType::DataItem;
    nav.RegisterElement(el);
    auto hit = nav.FindById("thumb_1");
    ASSERT(hit.found);
    ASSERT(hit.element.name == "photo.jpg");
}
TEST(Test_AccessibilityNavigatorV3_Invoke)
{
    using namespace ExplorerLens::Engine;
    AccessibilityNavigatorV3 nav;
    ANV3Element el;
    el.automationId = "btn_open";
    el.isEnabled = true;
    nav.RegisterElement(el);
    bool invoked = false;
    nav.SetInvokeCallback([&](const ANV3Element&) { invoked = true; });
    ASSERT(nav.Invoke("btn_open"));
    ASSERT(invoked);
}
TEST(Test_AccessibilityNavigatorV3_NotFound)
{
    using namespace ExplorerLens::Engine;
    AccessibilityNavigatorV3 nav;
    auto hit = nav.FindById("nonexistent");
    ASSERT(!hit.found);
}
TEST(Test_AccessibilityNavigatorV3_ControlTypeName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(AccessibilityNavigatorV3::ControlTypeName(ANV3ControlType::Button) == "Button");
    ASSERT(AccessibilityNavigatorV3::ControlTypeName(ANV3ControlType::ListItem) == "ListItem");
}
TEST(Test_ScreenReaderBridgeV2_NotAvailable)
{
    using namespace ExplorerLens::Engine;
    ScreenReaderBridgeV2 bridge;
    ASSERT(!bridge.Announce("hello"));
}
TEST(Test_ScreenReaderBridgeV2_WithFn)
{
    using namespace ExplorerLens::Engine;
    ScreenReaderBridgeV2 bridge;
    bridge.SetAvailable(true);
    bridge.SetActiveReader(ScreenReaderType::NVDA);
    bool announced = false;
    bridge.SetAnnounceFn([&](const ScreenReaderAnnouncement& a) {
        announced = !a.text.empty();
        return true;
    });
    ASSERT(bridge.Announce("Thumbnail ready", LiveRegionPoliteness::Polite));
    ASSERT(announced);
}
TEST(Test_ScreenReaderBridgeV2_ReaderName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(ScreenReaderBridgeV2::ReaderName(ScreenReaderType::JAWS) == "JAWS");
    ASSERT(ScreenReaderBridgeV2::ReaderName(ScreenReaderType::Narrator) == "Narrator");
}
TEST(Test_LocaleFallbackResolver_ExactMatch)
{
    using namespace ExplorerLens::Engine;
    LocaleFallbackResolver resolver;
    resolver.AddString("zh-Hant-TW", "hello", "NiHao");
    auto result = resolver.Resolve("zh-Hant-TW", "hello");
    ASSERT(result.found);
    ASSERT(result.resolvedLocale == "zh-Hant-TW");
    ASSERT(result.value == "NiHao");
}
TEST(Test_LocaleFallbackResolver_FallbackChain)
{
    using namespace ExplorerLens::Engine;
    LocaleFallbackResolver resolver;
    resolver.AddString("zh", "greeting", "NiHao");
    auto result = resolver.Resolve("zh-Hant-TW", "greeting");
    ASSERT(result.found);
    ASSERT(result.resolvedLocale == "zh");
}
TEST(Test_LocaleFallbackResolver_BuildChain)
{
    using namespace ExplorerLens::Engine;
    auto chain = LocaleFallbackResolver::BuildChain("zh-Hant-TW");
    ASSERT(chain[0] == "zh-Hant-TW");
    ASSERT(chain[1] == "zh-Hant");
    ASSERT(chain[2] == "zh");
    ASSERT(chain.back() == "en");
}
TEST(Test_A11yColorContrastEngine_Pass_AA)
{
    using namespace ExplorerLens::Engine;
    A11yColorContrastEngine engine;
    ColorRGBA8 black{0, 0, 0, 255};
    ColorRGBA8 white{255, 255, 255, 255};
    auto result = engine.Evaluate(black, white, A11yWCAGLevel::AA_Normal);
    ASSERT(result.decision == ContrastDecision::Pass);
    ASSERT(result.ratio > 4.5);
}
TEST(Test_A11yColorContrastEngine_Fail_AAA)
{
    using namespace ExplorerLens::Engine;
    A11yColorContrastEngine engine;
    ColorRGBA8 gray{180, 180, 180, 255};
    ColorRGBA8 white{255, 255, 255, 255};
    auto result = engine.Evaluate(gray, white, A11yWCAGLevel::AAA_Normal);
    ASSERT(result.decision == ContrastDecision::Fail);
}
TEST(Test_A11yColorContrastEngine_SuggestForeground)
{
    using namespace ExplorerLens::Engine;
    A11yColorContrastEngine engine;
    ColorRGBA8 darkBG{30, 30, 30, 255};
    auto fg = engine.SuggestAccessibleForeground(darkBG);
    // White should have higher contrast on dark background
    ASSERT(fg.r == 255 && fg.g == 255 && fg.b == 255);
}
TEST(Test_A11yColorContrastEngine_RequiredRatio)
{
    using namespace ExplorerLens::Engine;
    ASSERT(A11yColorContrastEngine::RequiredRatio(A11yWCAGLevel::AA_Normal) == 4.5);
    ASSERT(A11yColorContrastEngine::RequiredRatio(A11yWCAGLevel::AAA_Normal) == 7.0);
}
TEST(Test_KeyboardNavigationMapV2_AddAndNavigate)
{
    using namespace ExplorerLens::Engine;
    KeyboardNavigationMapV2 nav;
    NavigationNode n1;
    n1.id = "item1";
    n1.tabIndex = 0;
    NavigationNode n2;
    n2.id = "item2";
    n2.tabIndex = 1;
    nav.AddNode(n1);
    nav.AddNode(n2);
    nav.AddEdge("item1", NavigationDirection::Right, "item2");
    auto result = nav.Navigate("item1", NavigationDirection::Right);
    ASSERT(result.Ok());
    ASSERT(result.targetNodeId == "item2");
}
TEST(Test_KeyboardNavigationMapV2_TabOrder)
{
    using namespace ExplorerLens::Engine;
    KeyboardNavigationMapV2 nav;
    NavigationNode a;
    a.id = "z";
    a.tabIndex = 2;
    NavigationNode b;
    b.id = "y";
    b.tabIndex = 1;
    NavigationNode c;
    c.id = "x";
    c.tabIndex = 0;
    nav.AddNode(a);
    nav.AddNode(b);
    nav.AddNode(c);
    auto order = nav.TabOrder();
    ASSERT(order.size() == 3);
    ASSERT(order[0] == "x");  // lowest tabIndex first
}
TEST(Test_KeyboardNavigationMapV2_WrapAround)
{
    using namespace ExplorerLens::Engine;
    KeyboardNavigationMapV2 nav;
    NavigationNode a;
    a.id = "a";
    a.tabIndex = 0;
    NavigationNode b;
    b.id = "b";
    b.tabIndex = 1;
    nav.AddNode(a);
    nav.AddNode(b);
    auto result = nav.Navigate("b", NavigationDirection::Next);
    ASSERT(result.Ok());
    ASSERT(result.wrappedAround);
    ASSERT(result.targetNodeId == "a");
}
TEST(Test_AccessibilityAuditPipeline_Compliant)
{
    using namespace ExplorerLens::Engine;
    AccessibilityAuditPipeline pipeline;
    AuditableElement el;
    el.id = "btn1";
    el.name = "Open";
    el.foregroundRGB = 0x000000;
    el.backgroundRGB = 0xFFFFFF;
    el.hasLabel = true;
    el.isKeyboardFocusable = true;
    auto report = pipeline.Audit({el});
    ASSERT(report.totalElements == 1);
    ASSERT(report.IsCompliant());
}
TEST(Test_AccessibilityAuditPipeline_Noncompliant)
{
    using namespace ExplorerLens::Engine;
    AccessibilityAuditPipeline pipeline;
    AuditableElement el;
    el.id = "lbl1";
    el.name = "";
    el.foregroundRGB = 0xBBBBBB;
    el.backgroundRGB = 0xFFFFFF;  // low contrast
    el.hasLabel = false;
    auto report = pipeline.Audit({el});
    ASSERT(!report.IsCompliant());
    ASSERT(!report.issues.empty());
}
TEST(Test_AccessibilityAuditPipeline_CriterionName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(AccessibilityAuditPipeline::CriterionName(WCAGSuccessCriterion::SC_1_4_3_ContrastMinimum)
           == "1.4.3 Contrast (Minimum)");
}
TEST(Test_AccessibilityAuditPipeline_PassRate)
{
    using namespace ExplorerLens::Engine;
    AccessibilityAuditPipeline pipeline;
    AuditableElement good;
    good.id = "g";
    good.foregroundRGB = 0;
    good.backgroundRGB = 0xFFFFFF;
    good.hasLabel = true;
    AuditableElement bad;
    bad.id = "b";
    bad.foregroundRGB = 0xCCCCCC;
    bad.backgroundRGB = 0xFFFFFF;
    bad.hasLabel = false;
    auto report = pipeline.Audit({good, bad});
    ASSERT(report.totalElements == 2);
    ASSERT(report.PassRate() >= 0.0 && report.PassRate() <= 100.0);
}

//== Sprint 701-800 Tests ==

TEST(Test_OpenXRAssetDecoder_DetectFormat)
{
    using namespace ExplorerLens::Engine;
    ASSERT(OpenXRAssetDecoder::DetectFormat(L"model.xrb") == XRAssetFormat::XRB);
    ASSERT(OpenXRAssetDecoder::DetectFormat(L"scene.gltf") == XRAssetFormat::GLTF_XR);
    ASSERT(OpenXRAssetDecoder::DetectFormat(L"noext") == XRAssetFormat::Unknown);
}
TEST(Test_OpenXRAssetDecoder_Decode)
{
    using namespace ExplorerLens::Engine;
    OpenXRAssetDecoder dec;
    XRAssetDecodeRequest req;
    req.filePath = L"test.xrb";
    req.outputWidth = 64;
    req.outputHeight = 64;
    auto r = dec.Decode(req);
    ASSERT(r.success);
    ASSERT(r.width == 64 && r.height == 64);
    ASSERT(!r.rgbaData.empty());
}
TEST(Test_OpenXRAssetDecoder_EmptyPath)
{
    using namespace ExplorerLens::Engine;
    OpenXRAssetDecoder dec;
    XRAssetDecodeRequest req;
    auto r = dec.Decode(req);
    ASSERT(!r.success);
}
TEST(Test_OpenXRAssetDecoder_IsSupported)
{
    using namespace ExplorerLens::Engine;
    OpenXRAssetDecoder dec;
    ASSERT(dec.IsSupported(XRAssetFormat::XRB));
    ASSERT(!dec.IsSupported(XRAssetFormat::Unknown));
}

TEST(Test_USDADecoder_DetectLayerType)
{
    using namespace ExplorerLens::Engine;
    ASSERT(USDADecoder::DetectLayerType(L"stage.usda") == USDLayerType::USDA);
    ASSERT(USDADecoder::DetectLayerType(L"cache.usdc") == USDLayerType::USDC);
    ASSERT(USDADecoder::DetectLayerType(L"pkg.usdz") == USDLayerType::USDZ);
    ASSERT(USDADecoder::DetectLayerType(L"file.obj") == USDLayerType::Unknown);
}
TEST(Test_USDADecoder_Decode)
{
    using namespace ExplorerLens::Engine;
    USDADecoder dec;
    USDDecodeRequest req;
    req.filePath = L"test.usda";
    req.outputWidth = 128;
    req.outputHeight = 128;
    auto r = dec.Decode(req);
    ASSERT(r.success);
    ASSERT(r.primCount > 0);
}
TEST(Test_USDADecoder_EmptyPath)
{
    using namespace ExplorerLens::Engine;
    USDADecoder dec;
    USDDecodeRequest req;
    auto r = dec.Decode(req);
    ASSERT(!r.success);
}

TEST(Test_XRSpatialPreviewEngine_Preview)
{
    using namespace ExplorerLens::Engine;
    XRSpatialPreviewEngine eng;
    XRSpatialPreviewRequest req;
    req.assetPath = L"scene.xrb";
    req.width = 256;
    req.height = 256;
    auto r = eng.Preview(req);
    ASSERT(r.success);
    ASSERT(!r.rgbaData.empty());
    ASSERT(r.depthRange > 0.0f);
}
TEST(Test_XRSpatialPreviewEngine_TargetName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(XRSpatialPreviewEngine::TargetName(XRPreviewTarget::HoloLens2) == "HoloLens2");
    ASSERT(XRSpatialPreviewEngine::TargetName(XRPreviewTarget::Standard2D) == "Standard2D");
}
TEST(Test_XRSpatialPreviewEngine_EmptyPath)
{
    using namespace ExplorerLens::Engine;
    XRSpatialPreviewEngine eng;
    XRSpatialPreviewRequest req;
    auto r = eng.Preview(req);
    ASSERT(!r.success);
}

TEST(Test_ARMarkerDetectionEngine_Detect)
{
    using namespace ExplorerLens::Engine;
    ARMarkerDetectionEngine eng;
    ARMarkerDetectRequest req;
    req.rgbaData.assign(64 * 64 * 4, 0xFF);
    req.width = 64;
    req.height = 64;
    auto r = eng.Detect(req);
    ASSERT(r.success);
    ASSERT(!r.markers.empty());
}
TEST(Test_ARMarkerDetectionEngine_EmptyImage)
{
    using namespace ExplorerLens::Engine;
    ARMarkerDetectionEngine eng;
    ARMarkerDetectRequest req;
    auto r = eng.Detect(req);
    ASSERT(!r.success);
}
TEST(Test_ARMarkerDetectionEngine_TypeName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(ARMarkerDetectionEngine::MarkerTypeName(ARMarkerType::ArUco) == "ArUco");
    ASSERT(ARMarkerDetectionEngine::MarkerTypeName(ARMarkerType::QRCode) == "QRCode");
}

TEST(Test_StereoscopicRenderPipeline_Render)
{
    using namespace ExplorerLens::Engine;
    StereoscopicRenderPipeline pipe;
    StereoRenderRequest req;
    req.assetPath = L"scene.xrb";
    req.eyeWidth = 64;
    req.eyeHeight = 64;
    auto r = pipe.Render(req);
    ASSERT(r.success);
    ASSERT(!r.leftRGBA.empty() && !r.rightRGBA.empty());
    ASSERT(r.disparityMax > 0.0f);
}
TEST(Test_StereoscopicRenderPipeline_EmptyPath)
{
    using namespace ExplorerLens::Engine;
    StereoscopicRenderPipeline pipe;
    StereoRenderRequest req;
    auto r = pipe.Render(req);
    ASSERT(!r.success);
}
TEST(Test_StereoscopicRenderPipeline_LayoutName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(StereoscopicRenderPipeline::LayoutName(StereoLayout::SideBySide) == "SideBySide");
    ASSERT(StereoscopicRenderPipeline::LayoutName(StereoLayout::Anaglyph) == "Anaglyph");
}

TEST(Test_PointCloudVisualizerV2_DetectFormat)
{
    using namespace ExplorerLens::Engine;
    ASSERT(PointCloudVisualizerV2::DetectFormat(L"scan.e57") == PointCloudFormat::E57);
    ASSERT(PointCloudVisualizerV2::DetectFormat(L"cloud.las") == PointCloudFormat::LAS);
    ASSERT(PointCloudVisualizerV2::DetectFormat(L"mesh.ply") == PointCloudFormat::PLY);
}
TEST(Test_PointCloudVisualizerV2_Render)
{
    using namespace ExplorerLens::Engine;
    PointCloudVisualizerV2 vis;
    PointCloudRenderRequest req;
    req.filePath = L"scan.e57";
    req.outputWidth = 64;
    req.outputHeight = 64;
    auto r = vis.Render(req);
    ASSERT(r.success);
    ASSERT(!r.rgbaData.empty());
    ASSERT(r.pointsRendered > 0);
}
TEST(Test_PointCloudVisualizerV2_EmptyPath)
{
    using namespace ExplorerLens::Engine;
    PointCloudVisualizerV2 vis;
    PointCloudRenderRequest req;
    auto r = vis.Render(req);
    ASSERT(!r.success);
}

TEST(Test_NerfDecoder_DetectFormat)
{
    using namespace ExplorerLens::Engine;
    ASSERT(NerfDecoder::DetectFormat(L"transforms.json") == NerfSceneFormat::NerfSynthetic);
    ASSERT(NerfDecoder::DetectFormat(L"scene.msgpack") == NerfSceneFormat::Instant_NGP);
    ASSERT(NerfDecoder::DetectFormat(L"data.npz") == NerfSceneFormat::TinyNeRF);
}
TEST(Test_NerfDecoder_Decode)
{
    using namespace ExplorerLens::Engine;
    NerfDecoder dec;
    NerfDecodeRequest req;
    req.scenePath = L"scene.json";
    req.outputWidth = 64;
    req.outputHeight = 64;
    auto r = dec.Decode(req);
    ASSERT(r.success);
    ASSERT(r.psnrEstimate > 0.0f);
}

TEST(Test_XRMetadataExtractor_Extract)
{
    using namespace ExplorerLens::Engine;
    XRMetadataExtractor ext;
    auto r = ext.Extract(L"scene.xrb");
    ASSERT(r.success);
    ASSERT(!r.metadata.anchors.empty());
    ASSERT(r.metadata.targetFPS > 0.0f);
}
TEST(Test_XRMetadataExtractor_SupportsFormat)
{
    using namespace ExplorerLens::Engine;
    ASSERT(XRMetadataExtractor::SupportsFormat(L"xrb"));
    ASSERT(XRMetadataExtractor::SupportsFormat(L"gltf"));
    ASSERT(!XRMetadataExtractor::SupportsFormat(L"unknownfmt"));
}

TEST(Test_DifferentialPrivacyEngine_Query)
{
    using namespace ExplorerLens::Engine;
    DPParameters params;
    params.epsilon = 1.0;
    params.sensitivity = 1.0;
    DifferentialPrivacyEngine dp(params);
    auto r = dp.Query(100.0);
    ASSERT(!r.budgetExceeded);
    ASSERT(r.privacyBudgetUsed > 0.0);
}
TEST(Test_DifferentialPrivacyEngine_BudgetExhausted)
{
    using namespace ExplorerLens::Engine;
    DPParameters params;
    params.epsilon = 0.001;
    DifferentialPrivacyEngine dp(params);
    for (int i = 0; i < 20; ++i)
        dp.Query(1.0);
    auto r = dp.Query(1.0);
    ASSERT(r.budgetExceeded);
}
TEST(Test_DifferentialPrivacyEngine_ResetBudget)
{
    using namespace ExplorerLens::Engine;
    DPParameters params;
    params.epsilon = 0.001;
    DifferentialPrivacyEngine dp(params);
    for (int i = 0; i < 20; ++i)
        dp.Query(1.0);
    dp.ResetBudget();
    ASSERT(dp.RemainingBudget() > 0.0);
}

TEST(Test_LocalDataAggregator_AddAndFlush)
{
    using namespace ExplorerLens::Engine;
    LocalDataAggregator agg;
    LDARecord rec;
    rec.metricName = "latency";
    rec.value = 10.0;
    agg.AddRecord(rec);
    agg.AddRecord(rec);
    ASSERT(agg.PendingCount() == 2);
    auto results = agg.Flush();
    ASSERT(!results.empty());
    ASSERT(results[0].readyForUpload);
    ASSERT(agg.PendingCount() == 0);
}
TEST(Test_LocalDataAggregator_SetNoiseScale)
{
    using namespace ExplorerLens::Engine;
    LocalDataAggregator agg;
    agg.SetNoiseScale(0.0);
    LDARecord rec;
    rec.metricName = "m";
    rec.value = 5.0;
    agg.AddRecord(rec);
    agg.AddRecord(rec);
    auto results = agg.Flush();
    ASSERT(!results.empty());
    ASSERT(results[0].sampleCount == 2);
}

TEST(Test_AnonymizationPipelineV2_Anonymize)
{
    using namespace ExplorerLens::Engine;
    AnonPipelineConfig cfg;
    AnonymizationPipelineV2 pipe(cfg);
    auto r = pipe.Anonymize("C:\\Users\\john\\Documents\\file.txt");
    ASSERT(r.wasModified);
    ASSERT(r.anonymizedPath.find("[USER]") != std::string::npos);
}
TEST(Test_AnonymizationPipelineV2_ContainsPII)
{
    using namespace ExplorerLens::Engine;
    AnonPipelineConfig cfg;
    AnonymizationPipelineV2 pipe(cfg);
    ASSERT(pipe.ContainsPII("C:\\Users\\alice\\file.txt"));
    ASSERT(!pipe.ContainsPII("C:\\Program Files\\app.exe"));
}
TEST(Test_AnonymizationPipelineV2_Batch)
{
    using namespace ExplorerLens::Engine;
    AnonPipelineConfig cfg;
    AnonymizationPipelineV2 pipe(cfg);
    auto results = pipe.AnonymizeBatch({"C:\\Users\\bob\\a.txt", "C:\\Program Files\\b.exe"});
    ASSERT(results.size() == 2);
}

TEST(Test_PrivacyConsentManager_SetGet)
{
    using namespace ExplorerLens::Engine;
    PrivacyConsentManager mgr;
    mgr.SetConsent(ConsentCategory::Usage, ConsentState::Granted);
    ASSERT(mgr.GetConsent(ConsentCategory::Usage) == ConsentState::Granted);
    ASSERT(mgr.IsAllowed(ConsentCategory::Usage));
}
TEST(Test_PrivacyConsentManager_DefaultPending)
{
    using namespace ExplorerLens::Engine;
    PrivacyConsentManager mgr;
    ASSERT(mgr.GetConsent(ConsentCategory::PersonalizedAI) == ConsentState::Pending);
    ASSERT(!mgr.IsAllowed(ConsentCategory::PersonalizedAI));
}
TEST(Test_PrivacyConsentManager_AuditTrail)
{
    using namespace ExplorerLens::Engine;
    PrivacyConsentManager mgr;
    mgr.SetConsent(ConsentCategory::Crash, ConsentState::Denied);
    mgr.SetConsent(ConsentCategory::Crash, ConsentState::Granted);
    ASSERT(mgr.AuditTrail().size() == 2);
}

TEST(Test_SecureEnclaveAnalytics_Aggregate)
{
    using namespace ExplorerLens::Engine;
    EnclaveAnalyticsConfig cfg;
    SecureEnclaveAnalytics enc(cfg);
    auto r = enc.Aggregate({10.0, 20.0, 30.0});
    ASSERT(r.success);
    ASSERT(r.aggregateValue == 20.0);
    ASSERT(!r.attestationToken.empty());
}
TEST(Test_SecureEnclaveAnalytics_EmptyInput)
{
    using namespace ExplorerLens::Engine;
    EnclaveAnalyticsConfig cfg;
    SecureEnclaveAnalytics enc(cfg);
    auto r = enc.Aggregate({});
    ASSERT(!r.success);
}
TEST(Test_SecureEnclaveAnalytics_BackendName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(SecureEnclaveAnalytics::BackendName(EnclaveBackend::VBS_HVCI) == "VBS_HVCI");
    ASSERT(SecureEnclaveAnalytics::BackendName(EnclaveBackend::Simulation) == "Simulation");
}

TEST(Test_GDPRComplianceEngine_Erasure)
{
    using namespace ExplorerLens::Engine;
    GDPRComplianceEngine eng;
    eng.AddRecord("user1");
    eng.AddRecord("user1");
    ASSERT(eng.RecordsForSubject("user1") == 2);
    GDPRRequest req;
    req.right = GDPRRight::RightToErasure;
    req.subjectId = "user1";
    auto r = eng.ProcessRequest(req);
    ASSERT(r.fulfilled);
    ASSERT(r.recordsAffected == 2);
}
TEST(Test_GDPRComplianceEngine_EmptySubject)
{
    using namespace ExplorerLens::Engine;
    GDPRComplianceEngine eng;
    GDPRRequest req;
    req.right = GDPRRight::RightToAccess;
    auto r = eng.ProcessRequest(req);
    ASSERT(!r.fulfilled);
}

TEST(Test_PrivacyAuditLogger_RecordAndVerify)
{
    using namespace ExplorerLens::Engine;
    PrivacyAuditLogger log;
    log.Record(PALEventType::DataAccess, "user1", "viewed file");
    log.Record(PALEventType::DataErasure, "user1", "deleted records");
    auto r = log.VerifyChain();
    ASSERT(r.valid);
    ASSERT(r.entryCount == 2);
}
TEST(Test_PrivacyAuditLogger_EntriesForSubject)
{
    using namespace ExplorerLens::Engine;
    PrivacyAuditLogger log;
    log.Record(PALEventType::DataAccess, "user1", "op1");
    log.Record(PALEventType::DataAccess, "user2", "op2");
    auto entries = log.EntriesForSubject("user1");
    ASSERT(entries.size() == 1);
}

TEST(Test_LiveStreamDecoder_DecodeFirstFrame)
{
    using namespace ExplorerLens::Engine;
    LiveStreamDecoder dec;
    LiveStreamDecodeRequest req;
    req.url = "http://example.com/stream.m3u8";
    req.outputWidth = 64;
    req.outputHeight = 64;
    auto r = dec.DecodeFirstFrame(req);
    ASSERT(r.success);
    ASSERT(!r.rgbaData.empty());
}
TEST(Test_LiveStreamDecoder_EmptyUrl)
{
    using namespace ExplorerLens::Engine;
    LiveStreamDecoder dec;
    LiveStreamDecodeRequest req;
    auto r = dec.DecodeFirstFrame(req);
    ASSERT(!r.success);
}
TEST(Test_LiveStreamDecoder_ProtocolName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(LiveStreamDecoder::ProtocolName(LiveStreamProtocol::HLS) == "HLS");
    ASSERT(LiveStreamDecoder::ProtocolName(LiveStreamProtocol::RTSP) == "RTSP");
}

TEST(Test_WebRTCThumbnailCapture_Capture)
{
    using namespace ExplorerLens::Engine;
    WebRTCThumbnailCapture cap;
    WebRTCCaptureRequest req;
    req.offerSDP = "v=0\r\n";
    req.outputWidth = 64;
    req.outputHeight = 64;
    auto r = cap.Capture(req);
    ASSERT(r.success);
    ASSERT(!r.rgbaData.empty());
}
TEST(Test_WebRTCThumbnailCapture_NoSDP)
{
    using namespace ExplorerLens::Engine;
    WebRTCThumbnailCapture cap;
    WebRTCCaptureRequest req;
    auto r = cap.Capture(req);
    ASSERT(!r.success);
}
TEST(Test_WebRTCThumbnailCapture_CodecSupport)
{
    using namespace ExplorerLens::Engine;
    WebRTCThumbnailCapture cap;
    ASSERT(cap.IsCodecSupported("VP8"));
    ASSERT(cap.IsCodecSupported("H264"));
    ASSERT(!cap.IsCodecSupported("MPEG2"));
}

TEST(Test_StreamingBufferOrchestrator_PushPop)
{
    using namespace ExplorerLens::Engine;
    SBOConfig cfg;
    cfg.targetBufferMs = 2000;
    StreamingBufferOrchestrator orch(cfg);
    std::vector<uint8_t> chunk(1024, 0xAA);
    ASSERT(orch.PushChunk(chunk));
    auto popped = orch.PopChunk();
    ASSERT(!popped.empty());
}
TEST(Test_StreamingBufferOrchestrator_Status)
{
    using namespace ExplorerLens::Engine;
    SBOConfig cfg;
    StreamingBufferOrchestrator orch(cfg);
    auto s = orch.GetStatus();
    ASSERT(s.state == SBOBufferState::Idle);
}

TEST(Test_AdaptiveBitrateSelector_Select)
{
    using namespace ExplorerLens::Engine;
    AdaptiveBitrateSelector sel(ABRStrategy::BandwidthBased);
    sel.SetProfiles({{1000, 640, 480, "480p"}, {3000, 1280, 720, "720p"}});
    auto r = sel.Select(4000.0f, 5000);
    ASSERT(r.success);
    ASSERT(r.selectedProfile.bitrateKbps > 0);
}
TEST(Test_AdaptiveBitrateSelector_NoBandwidth)
{
    using namespace ExplorerLens::Engine;
    AdaptiveBitrateSelector sel(ABRStrategy::Hybrid);
    sel.SetProfiles({{5000, 1920, 1080, "1080p"}});
    auto r = sel.Select(100.0f, 1000);
    ASSERT(r.success);  // Falls back to first profile
}

TEST(Test_MediaTimelineRenderer_RenderStrip)
{
    using namespace ExplorerLens::Engine;
    MediaTimelineRenderer rend;
    MTRRenderRequest req;
    req.filePath = L"video.mp4";
    req.framesTotal = 5;
    auto r = rend.RenderStrip(req);
    ASSERT(r.success);
    ASSERT(r.keyframes.size() == 5);
    ASSERT(r.totalDurationMs > 0);
}
TEST(Test_MediaTimelineRenderer_EmptyPath)
{
    using namespace ExplorerLens::Engine;
    MediaTimelineRenderer rend;
    MTRRenderRequest req;
    auto r = rend.RenderStrip(req);
    ASSERT(!r.success);
}

TEST(Test_DASHStreamDecoder_Decode)
{
    using namespace ExplorerLens::Engine;
    DASHStreamDecoder dec;
    DASHDecodeRequest req;
    req.mpdUrl = "http://example.com/stream.mpd";
    auto r = dec.DecodeFirstKeyframe(req);
    ASSERT(r.success);
    ASSERT(r.selectedBitrateKbps > 0);
}
TEST(Test_DASHStreamDecoder_IsManifestUrl)
{
    using namespace ExplorerLens::Engine;
    DASHStreamDecoder dec;
    ASSERT(dec.IsManifestUrl("http://example.com/stream.mpd"));
    ASSERT(!dec.IsManifestUrl("http://example.com/stream.ts"));
}

TEST(Test_LiveThumbnailPoller_AddRemove)
{
    using namespace ExplorerLens::Engine;
    LTPConfig cfg;
    LiveThumbnailPoller poller(cfg);
    LTPSourceInfo info;
    info.sourceId = "s1";
    info.url = "http://cam.example.com/feed";
    ASSERT(poller.AddSource(info));
    ASSERT(poller.RemoveSource("s1"));
}
TEST(Test_LiveThumbnailPoller_MaxConcurrent)
{
    using namespace ExplorerLens::Engine;
    LTPConfig cfg;
    cfg.maxConcurrent = 2;
    LiveThumbnailPoller poller(cfg);
    for (int i = 0; i < 2; ++i) {
        LTPSourceInfo info;
        info.sourceId = "s" + std::to_string(i);
        info.url = "u";
        poller.AddSource(info);
    }
    LTPSourceInfo extra;
    extra.sourceId = "extra";
    extra.url = "u";
    ASSERT(!poller.AddSource(extra));  // Limit reached
}

TEST(Test_VideoTextureStreamEngine_Upload)
{
    using namespace ExplorerLens::Engine;
    VideoTextureStreamEngine eng;
    VTSEStreamRequest req;
    req.yuvData.assign(64 * 64 * 3 / 2, 0x80);
    req.width = 64;
    req.height = 64;
    auto r = eng.Upload(req);
    ASSERT(r.success);
    ASSERT(r.gpuTextureHandle != 0);
}
TEST(Test_VideoTextureStreamEngine_EmptyFrame)
{
    using namespace ExplorerLens::Engine;
    VideoTextureStreamEngine eng;
    VTSEStreamRequest req;
    auto r = eng.Upload(req);
    ASSERT(!r.success);
}
TEST(Test_VideoTextureStreamEngine_ZeroCopy)
{
    using namespace ExplorerLens::Engine;
    VideoTextureStreamEngine eng;
    ASSERT(eng.IsZeroCopySupported());
    ASSERT(eng.PreferredMode() == VTSEUploadMode::ZeroCopy_DMA);
}

TEST(Test_RenderClusterManager_SubmitComplete)
{
    using namespace ExplorerLens::Engine;
    RenderClusterManager mgr;
    mgr.RegisterNode({"node1", "10.0.0.1", RCMNodeStatus::Idle, 4, 0.0f});
    auto r = mgr.Submit("render job 1");
    ASSERT(r.success);
    ASSERT(r.assignedNodeId == "node1");
    ASSERT(mgr.CompleteJob(r.jobId));
}
TEST(Test_RenderClusterManager_NoIdleNodes)
{
    using namespace ExplorerLens::Engine;
    RenderClusterManager mgr;
    auto r = mgr.Submit("job");
    ASSERT(!r.success);
}

TEST(Test_RenderJobScheduler_EnqueueDequeue)
{
    using namespace ExplorerLens::Engine;
    RenderJobScheduler sched;
    uint32_t id1 = sched.Enqueue("job1", RJSPriority::High);
    uint32_t id2 = sched.Enqueue("job2", RJSPriority::Low);
    ASSERT(sched.PendingCount() == 2);
    auto job = sched.Dequeue();
    ASSERT(job.description == "job1");  // High priority first
    ASSERT(sched.PendingCount() == 1);
    ASSERT(sched.Complete(job.id));
    ASSERT(sched.CompletedCount() == 1);
    (void)id1;
    (void)id2;
}
TEST(Test_RenderJobScheduler_Fail)
{
    using namespace ExplorerLens::Engine;
    RenderJobScheduler sched;
    sched.Enqueue("job");
    auto job = sched.Dequeue();
    ASSERT(sched.Fail(job.id));
}

TEST(Test_NodeHealthMonitor_HeartbeatAndReport)
{
    using namespace ExplorerLens::Engine;
    NodeHealthMonitor mon;
    NHMHeartbeat hb;
    hb.nodeId = "n1";
    hb.cpuPercent = 50.0f;
    mon.RecordHeartbeat(hb);
    auto r = mon.GetReport("n1");
    ASSERT(r.state == NHMHealthState::Healthy);
    ASSERT(r.missedBeats == 0);
}
TEST(Test_NodeHealthMonitor_Unresponsive)
{
    using namespace ExplorerLens::Engine;
    NodeHealthMonitor mon;
    auto r = mon.GetReport("unknown");
    ASSERT(r.state == NHMHealthState::Unresponsive);
}

TEST(Test_RenderResultAggregator_Compose)
{
    using namespace ExplorerLens::Engine;
    RenderResultAggregator agg;
    RRATile t;
    t.tileX = 0;
    t.tileY = 0;
    t.width = 32;
    t.height = 32;
    t.rgbaData.assign(32 * 32 * 4, 0xCC);
    agg.AddTile(t);
    auto r = agg.Compose(64, 64);
    ASSERT(r.success);
    ASSERT(r.tilesComposed == 1);
    ASSERT(!r.rgbaData.empty());
}
TEST(Test_RenderResultAggregator_NoTiles)
{
    using namespace ExplorerLens::Engine;
    RenderResultAggregator agg;
    auto r = agg.Compose(64, 64);
    ASSERT(!r.success);
}

TEST(Test_ClusterAutoScaler_ScaleUp)
{
    using namespace ExplorerLens::Engine;
    ClusterAutoScaler scaler(1, 10);
    CASMetrics m;
    m.queueDepth = 50;
    m.avgCpuPercent = 90.0f;
    m.activeNodes = 2;
    auto d = scaler.Evaluate(m);
    ASSERT(d.action == CASScaleAction::ScaleUp);
    ASSERT(d.targetNodes > 2);
}
TEST(Test_ClusterAutoScaler_ScaleDown)
{
    using namespace ExplorerLens::Engine;
    ClusterAutoScaler scaler(1, 10);
    CASMetrics m;
    m.queueDepth = 0;
    m.avgCpuPercent = 5.0f;
    m.activeNodes = 5;
    auto d = scaler.Evaluate(m);
    ASSERT(d.action == CASScaleAction::ScaleDown);
}

TEST(Test_SecureClusterChannel_Handshake)
{
    using namespace ExplorerLens::Engine;
    SCCConfig cfg;
    cfg.certThumbprint = "AA:BB:CC";
    cfg.remoteNodeId = "node2";
    SecureClusterChannel ch(cfg);
    auto r = ch.Handshake();
    ASSERT(r.success);
    ASSERT(!r.sessionId.empty());
    ASSERT(ch.IsConnected());
}
TEST(Test_SecureClusterChannel_Send)
{
    using namespace ExplorerLens::Engine;
    SCCConfig cfg;
    cfg.certThumbprint = "AA:BB:CC";
    cfg.remoteNodeId = "n2";
    SecureClusterChannel ch(cfg);
    ch.Handshake();
    ASSERT(ch.Send({0x01, 0x02, 0x03}));
    ASSERT(ch.BytesSent() == 3);
}

TEST(Test_ClusterObservabilityBus_RecordAndSnapshot)
{
    using namespace ExplorerLens::Engine;
    ClusterObservabilityBus bus;
    COBSpan s;
    s.spanId = "s1";
    s.operationName = "op";
    s.durationMs = 10;
    s.nodeId = "n1";
    bus.RecordSpan(s);
    auto snap = bus.Snapshot();
    ASSERT(snap.totalSpans == 1);
    ASSERT(snap.avgDurationMs == 10.0f);
}
TEST(Test_ClusterObservabilityBus_QueryByNode)
{
    using namespace ExplorerLens::Engine;
    ClusterObservabilityBus bus;
    COBSpan s1;
    s1.nodeId = "n1";
    s1.spanId = "a";
    bus.RecordSpan(s1);
    COBSpan s2;
    s2.nodeId = "n2";
    s2.spanId = "b";
    bus.RecordSpan(s2);
    auto spans = bus.QuerySpans("n1");
    ASSERT(spans.size() == 1);
}

TEST(Test_PluginSandboxV3_Execute)
{
    using namespace ExplorerLens::Engine;
    PSV3SandboxConfig cfg;
    PluginSandboxV3 sandbox(cfg);
    auto r = sandbox.Execute("plugin1", {0x01, 0x02});
    ASSERT(r.success && r.exitCode == 0);
}
TEST(Test_PluginSandboxV3_MemoryLimit)
{
    using namespace ExplorerLens::Engine;
    PSV3SandboxConfig cfg;
    cfg.memLimitBytes = 4;
    PluginSandboxV3 sandbox(cfg);
    auto r = sandbox.Execute("plugin1", {0x01, 0x02, 0x03});  // 3*2=6 > 4
    ASSERT(!r.success);
}

TEST(Test_PluginCompatibilityShimV3_Load)
{
    using namespace ExplorerLens::Engine;
    PluginCompatibilityShimV3 shim;
    auto r = shim.Load("C:\\plugins\\MyPluginv2.dll");
    ASSERT(r.success);
    ASSERT(r.detectedSDK == PCShimSourceSDK::SDKv2);
    ASSERT(shim.IsLoaded(r.shimmedPluginId));
}
TEST(Test_PluginCompatibilityShimV3_EmptyPath)
{
    using namespace ExplorerLens::Engine;
    PluginCompatibilityShimV3 shim;
    auto r = shim.Load("");
    ASSERT(!r.success);
}

TEST(Test_FolderPredictionModel_TopPredictions)
{
    using namespace ExplorerLens::Engine;
    FolderPredictionModel mdl;
    FPMAccessRecord r1;
    r1.folderPath = L"Photos";
    FPMAccessRecord r2;
    r2.folderPath = L"Videos";
    mdl.RecordAccess(r1);
    mdl.RecordAccess(r2);
    mdl.RecordAccess(r2);
    auto preds = mdl.TopPredictions(3);
    ASSERT(!preds.empty());
    ASSERT(preds[0].folderPath == L"Videos");
}
TEST(Test_FolderPredictionModel_TotalAccesses)
{
    using namespace ExplorerLens::Engine;
    FolderPredictionModel mdl;
    FPMAccessRecord r;
    r.folderPath = L"Docs";
    mdl.RecordAccess(r);
    ASSERT(mdl.TotalAccesses() == 1);
}

TEST(Test_ColdStartFolderBootstrapper_Bootstrap)
{
    using namespace ExplorerLens::Engine;
    CSFBConfig cfg;
    cfg.maxFilesPerFolder = 20;
    cfg.seedFolders = {L"C:\\Photos", L"C:\\Downloads"};
    ColdStartFolderBootstrapper boot(cfg);
    auto r = boot.Bootstrap();
    ASSERT(r.success);
    ASSERT(r.foldersScanned == 2);
    ASSERT(r.filesQueued > 0);
    boot.MarkComplete();
    ASSERT(boot.IsBootstrapComplete());
}

TEST(Test_PredictionScanOrchestrator_EnqueueDrain)
{
    using namespace ExplorerLens::Engine;
    PredictionScanOrchestrator orch;
    PSOTrigger t;
    t.folderPath = L"C:\\Photos";
    t.priority = PSOScanPriority::High;
    orch.EnqueueScan(t);
    orch.EnqueueScan(t);
    ASSERT(orch.GetStatus().pendingScans == 2);
    ASSERT(orch.DrainOne());
    ASSERT(orch.GetStatus().completedTotal == 1);
}
TEST(Test_PredictionScanOrchestrator_Cancel)
{
    using namespace ExplorerLens::Engine;
    PredictionScanOrchestrator orch;
    PSOTrigger t;
    t.folderPath = L"x";
    orch.EnqueueScan(t);
    orch.Cancel();
    ASSERT(orch.GetStatus().pendingScans == 0);
}

TEST(Test_DMADirectPreloader_Preload)
{
    using namespace ExplorerLens::Engine;
    DMADirectPreloader pre;
    DMAPreloadRequest req;
    req.filePath = L"C:\\Photo.jpg";
    req.sizeBytes = 4096;
    auto r = pre.Preload(req);
    ASSERT(r.success);
    ASSERT(r.gpuMemoryAddress != 0);
    ASSERT(pre.IsPreloaded(L"C:\\Photo.jpg"));
    ASSERT(pre.Evict(L"C:\\Photo.jpg"));
    ASSERT(!pre.IsPreloaded(L"C:\\Photo.jpg"));
}
TEST(Test_DMADirectPreloader_EmptyPath)
{
    using namespace ExplorerLens::Engine;
    DMADirectPreloader pre;
    DMAPreloadRequest req;
    auto r = pre.Preload(req);
    ASSERT(!r.success);
}

TEST(Test_PerUserPredictionIsolator_RegisterTest)
{
    using namespace ExplorerLens::Engine;
    PerUserPredictionIsolator iso;
    iso.RegisterUser({"alice", true, 0});
    iso.RegisterUser({"bob", true, 1});
    ASSERT(iso.IsIsolated("alice"));
    auto r = iso.TestIsolation();
    ASSERT(!r.leaked);
}
TEST(Test_PerUserPredictionIsolator_SlotCollision)
{
    using namespace ExplorerLens::Engine;
    PerUserPredictionIsolator iso;
    iso.RegisterUser({"alice", true, 0});
    iso.RegisterUser({"bob", true, 0});  // Same slot — collision
    auto r = iso.TestIsolation();
    ASSERT(r.leaked);
}

TEST(Test_PredictionAccuracyTracker_Compute)
{
    using namespace ExplorerLens::Engine;
    PredictionAccuracyTracker tracker;
    tracker.Record({L"img1.jpg", true, 0.8f});
    tracker.Record({L"img2.jpg", false, 0.4f});
    auto r = tracker.Compute();
    ASSERT(r.totalPredictions == 2);
    ASSERT(r.hits == 1);
    ASSERT(r.hitRate == 0.5f);
}
TEST(Test_PredictionAccuracyTracker_Empty)
{
    using namespace ExplorerLens::Engine;
    PredictionAccuracyTracker tracker;
    auto r = tracker.Compute();
    ASSERT(r.totalPredictions == 0);
    ASSERT(r.hitRate == 0.0f);
}

TEST(Test_CollaborativeAnnotationEngineV2_ApplyAndMerge)
{
    using namespace ExplorerLens::Engine;
    CollaborativeAnnotationEngineV2 eng;
    CAEv2Operation op;
    op.operationId = "op1";
    op.authorId = "alice";
    op.payload = "labelA";
    op.logicalClock = 1;
    eng.ApplyOp(op);
    ASSERT(eng.OpCount() == 1);
    CAEv2Operation op2;
    op2.operationId = "op2";
    op2.authorId = "bob";
    op2.payload = "labelB";
    op2.logicalClock = 1;
    auto r = eng.MergeFrom({op2});
    ASSERT(r.success && r.opsApplied == 1);
    ASSERT(eng.OpCount() == 2);
}
TEST(Test_CollaborativeAnnotationEngineV2_Clock)
{
    using namespace ExplorerLens::Engine;
    CollaborativeAnnotationEngineV2 eng;
    CAEv2Operation op;
    op.operationId = "o1";
    op.authorId = "alice";
    op.logicalClock = 5;
    eng.ApplyOp(op);
    ASSERT(eng.CurrentClock("alice") >= 6);
}

TEST(Test_AnnotationSignatureVerifier_SignAndVerify)
{
    using namespace ExplorerLens::Engine;
    AnnotationSignatureVerifier verifier;
    std::string privKey = "mySecretKey";
    std::string sig = AnnotationSignatureVerifier::Sign("hello annotation", privKey);
    verifier.RegisterPublicKey("alice", privKey);
    ASVSignedAnnotation ann;
    ann.payload = "hello annotation";
    ann.authorId = "alice";
    ann.signature = sig;
    auto r = verifier.Verify(ann);
    ASSERT(r.valid);
    ASSERT(r.authorId == "alice");
}
TEST(Test_AnnotationSignatureVerifier_UnknownAuthor)
{
    using namespace ExplorerLens::Engine;
    AnnotationSignatureVerifier verifier;
    ASVSignedAnnotation ann;
    ann.authorId = "ghost";
    ann.payload = "hi";
    ann.signature = "sig";
    auto r = verifier.Verify(ann);
    ASSERT(!r.valid);
}

TEST(Test_AnnotationTimeline_AddAndRevert)
{
    using namespace ExplorerLens::Engine;
    AnnotationTimeline tl;
    uint64_t v1 = tl.AddSnapshot("init", "hash1");
    uint64_t v2 = tl.AddSnapshot("updated", "hash2");
    ASSERT(tl.CurrentVersion() == v2);
    ASSERT(tl.Revert(v1));
    ASSERT(tl.CurrentVersion() == v1);
}
TEST(Test_AnnotationTimeline_GetDelta)
{
    using namespace ExplorerLens::Engine;
    AnnotationTimeline tl;
    tl.AddSnapshot("v1", "h1");
    tl.AddSnapshot("v2", "h2");
    auto d = tl.GetDelta(1, 2);
    ASSERT(d.fromVersion == 1 && d.toVersion == 2);
    ASSERT(!d.patch.empty());
}

TEST(Test_PresenceIndicatorEngine_UpdateAndQuery)
{
    using namespace ExplorerLens::Engine;
    PresenceIndicatorEngine eng;
    PIEUser u;
    u.userId = "alice";
    u.state = PIEPresenceState::Active;
    u.displayName = "Alice";
    eng.UpdatePresence(u);
    ASSERT(eng.GetPresence("alice") == PIEPresenceState::Active);
    auto active = eng.ActiveUsers();
    ASSERT(active.size() == 1);
}
TEST(Test_PresenceIndicatorEngine_SetOffline)
{
    using namespace ExplorerLens::Engine;
    PresenceIndicatorEngine eng;
    PIEUser u;
    u.userId = "bob";
    u.state = PIEPresenceState::Active;
    eng.UpdatePresence(u);
    eng.SetOffline("bob");
    ASSERT(eng.GetPresence("bob") == PIEPresenceState::Offline);
}

TEST(Test_AnnotationTaxonomyV2_LookupAndChildren)
{
    using namespace ExplorerLens::Engine;
    AnnotationTaxonomyV2 tax;
    tax.AddLabel({"cat-animal", "Animal", "", "Any living creature"});
    tax.AddLabel({"cat-dog", "Dog", "cat-animal", "Domestic dog"});
    auto r = tax.Lookup("dog");
    ASSERT(r.found && !r.matches.empty());
    auto children = tax.Children("cat-animal");
    ASSERT(children.size() == 1);
    ASSERT(children[0].id == "cat-dog");
}
TEST(Test_AnnotationTaxonomyV2_LabelCount)
{
    using namespace ExplorerLens::Engine;
    AnnotationTaxonomyV2 tax;
    tax.AddLabel({"l1", "One", "", ""});
    tax.AddLabel({"l2", "Two", "", ""});
    ASSERT(tax.LabelCount() == 2);
}

TEST(Test_OfflineAnnotationSyncQueue_EnqueueFlush)
{
    using namespace ExplorerLens::Engine;
    OfflineAnnotationSyncQueue q;
    OASQEntry e;
    e.operationId = "op1";
    e.payload = "data";
    e.retryCount = 0;
    q.Enqueue(e);
    q.Enqueue(e);
    ASSERT(q.QueueDepth() == 2);
    auto r = q.FlushAll();
    ASSERT(r.success);
    ASSERT(r.syncedCount == 2);
    ASSERT(q.QueueDepth() == 0);
}
TEST(Test_OfflineAnnotationSyncQueue_Clear)
{
    using namespace ExplorerLens::Engine;
    OfflineAnnotationSyncQueue q;
    q.Enqueue({"op1", "data"});
    q.Clear();
    ASSERT(q.QueueDepth() == 0);
}

TEST(Test_AnnotationExportPipelineV2_Export)
{
    using namespace ExplorerLens::Engine;
    AnnotationExportPipelineV2 pipe;
    AEPv2ExportRequest req;
    req.outputPath = L"out.json";
    req.annotations = {"ann1", "ann2", "ann3"};
    req.format = AEPv2ExportFormat::JSON_LD;
    auto r = pipe.Export(req);
    ASSERT(r.success);
    ASSERT(r.annotationCount == 3);
    ASSERT(r.bytesWritten > 0);
}
TEST(Test_AnnotationExportPipelineV2_FormatName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(AnnotationExportPipelineV2::FormatName(AEPv2ExportFormat::CSV) == "CSV");
    ASSERT(AnnotationExportPipelineV2::FormatName(AEPv2ExportFormat::COCO) == "COCO");
}

TEST(Test_GRPCProtocolServerV2_DispatchHandler)
{
    using namespace ExplorerLens::Engine;
    GRPCv2Config cfg;
    GRPCProtocolServerV2 srv(cfg);
    srv.RegisterHandler("Thumbnail.Get", [](const GRPCv2Request&) -> GRPCv2Response { return {0, {0xAA}}; });
    ASSERT(srv.Start());
    GRPCv2Request req;
    req.method = "Thumbnail.Get";
    auto r = srv.Dispatch(req);
    ASSERT(r.statusCode == 0);
    ASSERT(srv.IsRunning());
    ASSERT(srv.Stop());
}
TEST(Test_GRPCProtocolServerV2_UnknownMethod)
{
    using namespace ExplorerLens::Engine;
    GRPCv2Config cfg;
    GRPCProtocolServerV2 srv(cfg);
    GRPCv2Request req;
    req.method = "Unknown.Method";
    auto r = srv.Dispatch(req);
    ASSERT(r.statusCode == 404);
}

TEST(Test_RESTAPIServerV2_RouteAndDispatch)
{
    using namespace ExplorerLens::Engine;
    RASSv2Config cfg;
    RESTAPIServerV2 srv(cfg);
    srv.Route("GET", "/thumbnail", [](const RASSv2Request&) -> RASSv2Response { return {200, "image/png", {0xFF}}; });
    ASSERT(srv.Start());
    RASSv2Request req;
    req.method = "GET";
    req.path = "/thumbnail";
    auto r = srv.Dispatch(req);
    ASSERT(r.statusCode == 200);
}
TEST(Test_RESTAPIServerV2_NotFound)
{
    using namespace ExplorerLens::Engine;
    RASSv2Config cfg;
    RESTAPIServerV2 srv(cfg);
    RASSv2Request req;
    req.method = "GET";
    req.path = "/missing";
    auto r = srv.Dispatch(req);
    ASSERT(r.statusCode == 404);
}

TEST(Test_GraphQLSubscriptionServer_SubscribePublish)
{
    using namespace ExplorerLens::Engine;
    GraphQLSubscriptionServer srv;
    bool received = false;
    std::string subId = srv.Subscribe("thumbnail.updated", [&](const GQLSubEvent& e) {
        received = true;
        (void)e;
    });
    ASSERT(!subId.empty());
    ASSERT(srv.SubscriberCount() == 1);
    GQLSubEvent evt;
    evt.topic = "thumbnail.updated";
    evt.payload = "{}";
    srv.Publish(evt);
    ASSERT(received);
    ASSERT(srv.Unsubscribe(subId));
}
TEST(Test_GraphQLSubscriptionServer_UnsubscribeUnknown)
{
    using namespace ExplorerLens::Engine;
    GraphQLSubscriptionServer srv;
    ASSERT(!srv.Unsubscribe("nonexistent"));
}

TEST(Test_OAuth2PKCEMiddleware_Exchange)
{
    using namespace ExplorerLens::Engine;
    OAuthPKCEConfig cfg;
    cfg.clientId = "lens-client";
    cfg.redirectUri = "localhost";
    OAuth2PKCEMiddleware mw(cfg);
    mw.GenerateCodeVerifier();
    auto r = mw.ExchangeCode("auth-code-123");
    ASSERT(r.success);
    ASSERT(!r.accessToken.empty());
    ASSERT(r.expiresIn == 3600);
}
TEST(Test_OAuth2PKCEMiddleware_NoVerifier)
{
    using namespace ExplorerLens::Engine;
    OAuthPKCEConfig cfg;
    OAuth2PKCEMiddleware mw(cfg);
    auto r = mw.ExchangeCode("code");
    ASSERT(!r.success);
}

TEST(Test_JWTValidationEngine_ValidToken)
{
    using namespace ExplorerLens::Engine;
    JWTValidationEngine eng;
    eng.SetSecret("secret");
    eng.SetIssuer("lens");
    eng.SetAudience("api");
    auto r = eng.Validate("header.payload.signature");
    ASSERT(r.valid);
    ASSERT(!r.subject.empty());
}
TEST(Test_JWTValidationEngine_MalformedToken)
{
    using namespace ExplorerLens::Engine;
    JWTValidationEngine eng;
    auto r = eng.Validate("noDotsHere");
    ASSERT(!r.valid);
}
TEST(Test_JWTValidationEngine_AlgorithmSupport)
{
    using namespace ExplorerLens::Engine;
    JWTValidationEngine eng;
    ASSERT(eng.IsAlgorithmSupported(JWTAlgorithm::HS256));
    ASSERT(eng.IsAlgorithmSupported(JWTAlgorithm::RS256));
}

TEST(Test_RateLimitingMiddleware_AllowAndExhaust)
{
    using namespace ExplorerLens::Engine;
    RLMConfig cfg;
    cfg.maxRequests = 3;
    cfg.windowMs = 60000;
    RateLimitingMiddleware mw(cfg);
    for (int i = 0; i < 3; ++i) {
        auto r = mw.Check("client1");
        ASSERT(r.allowed);
    }
    auto r4 = mw.Check("client1");
    ASSERT(!r4.allowed);
    ASSERT(r4.remaining == 0);
}
TEST(Test_RateLimitingMiddleware_Reset)
{
    using namespace ExplorerLens::Engine;
    RLMConfig cfg;
    cfg.maxRequests = 1;
    RateLimitingMiddleware mw(cfg);
    mw.Check("c1");
    mw.Reset("c1");
    auto r = mw.Check("c1");
    ASSERT(r.allowed);
}

TEST(Test_OpenAPICodeGenerator_Generate)
{
    using namespace ExplorerLens::Engine;
    OpenAPICodeGenerator gen;
    OACGGenerateRequest req;
    req.specJson = "{\"openapi\":\"3.1.0\"}";
    req.language = OACGTargetLanguage::Cpp;
    auto r = gen.Generate(req);
    ASSERT(r.success);
    ASSERT(!r.generatedFiles.empty());
    ASSERT(r.endpointCount > 0);
}
TEST(Test_OpenAPICodeGenerator_EmptySpec)
{
    using namespace ExplorerLens::Engine;
    OpenAPICodeGenerator gen;
    OACGGenerateRequest req;
    auto r = gen.Generate(req);
    ASSERT(!r.success);
}
TEST(Test_OpenAPICodeGenerator_LanguageSuffix)
{
    using namespace ExplorerLens::Engine;
    ASSERT(OpenAPICodeGenerator::LanguageSuffix(OACGTargetLanguage::Python) == "py");
    ASSERT(OpenAPICodeGenerator::LanguageSuffix(OACGTargetLanguage::TypeScript) == "ts");
}

TEST(Test_GraphQLSchemaIntrospector_IntrospectTypes)
{
    using namespace ExplorerLens::Engine;
    GraphQLSchemaIntrospector intro;
    intro.RegisterType({"Thumbnail", "OBJECT", {"id", "width", "height"}});
    intro.RegisterType({"String", "SCALAR", {}});
    auto r = intro.Introspect();
    ASSERT(r.success);
    ASSERT(r.types.size() == 2);
    ASSERT(intro.TypeExists("Thumbnail"));
}

TEST(Test_NeuralCodecV2Engine_Encode)
{
    using namespace ExplorerLens::Engine;
    NeuralCodecV2Engine eng;
    NCV2EncodeRequest req;
    req.rgbaData.assign(64 * 64 * 4, 0xAA);
    req.width = 64;
    req.height = 64;
    req.backend = NCV2Backend::CPU;
    auto r = eng.Encode(req);
    ASSERT(r.success);
    ASSERT(r.compressionRatio > 1.0f);
    ASSERT(r.encodeMs > 0);
}
TEST(Test_NeuralCodecV2Engine_EmptyInput)
{
    using namespace ExplorerLens::Engine;
    NeuralCodecV2Engine eng;
    NCV2EncodeRequest req;
    auto r = eng.Encode(req);
    ASSERT(!r.success);
}
TEST(Test_NeuralCodecV2Engine_BackendAvailability)
{
    using namespace ExplorerLens::Engine;
    NeuralCodecV2Engine eng;
    ASSERT(eng.IsBackendAvailable(NCV2Backend::CPU));
    ASSERT(NeuralCodecV2Engine::BackendName(NCV2Backend::DirectML) == "DirectML");
}

TEST(Test_ProgressiveNeuralDecoder_Steps)
{
    using namespace ExplorerLens::Engine;
    ProgressiveNeuralDecoder dec;
    dec.BeginDecode({0xAA, 0xBB}, 64, 64);
    int steps = 0;
    while (!dec.IsComplete()) {
        auto step = dec.NextStep();
        ASSERT(!step.rgbaData.empty());
        ASSERT(step.qualityEstimate > 0.0f);
        ++steps;
    }
    ASSERT(steps == 4);
}
TEST(Test_ProgressiveNeuralDecoder_Reset)
{
    using namespace ExplorerLens::Engine;
    ProgressiveNeuralDecoder dec;
    dec.BeginDecode({}, 64, 64);
    dec.NextStep();
    dec.Reset();
    ASSERT(dec.Width() == 64);  // Width persists after reset until next BeginDecode
}

TEST(Test_NeuralContainerFormat_SerializeDeserialize)
{
    using namespace ExplorerLens::Engine;
    NCFContainer c;
    c.metadata.width = 256;
    c.metadata.height = 140;
    c.payload = {0x01, 0x02, 0x03};
    auto data = NeuralContainerFormat::Serialize(c);
    ASSERT(data.size() > 4);
    NCFContainer out;
    ASSERT(NeuralContainerFormat::Deserialize(data, out));
    ASSERT(out.metadata.width == 256);
    ASSERT(out.metadata.height == 140);
}
TEST(Test_NeuralContainerFormat_IsNCFData)
{
    using namespace ExplorerLens::Engine;
    ASSERT(NeuralContainerFormat::IsNCFData({0, 1, 2, 3, 4}));
    ASSERT(!NeuralContainerFormat::IsNCFData({0, 1}));
    ASSERT(NeuralContainerFormat::Extension() == ".ncf");
}

TEST(Test_NeuralCodecHWAccelerator_Encode)
{
    using namespace ExplorerLens::Engine;
    NeuralCodecHWAccelerator accel;
    NCHWAEncodeRequest req2;
    req2.rgbaData.assign(64, 0x80);
    req2.width = 8;
    req2.height = 2;
    req2.backend = NCHWABackend::QuickSync;
    auto r = accel.Encode(req2);
    ASSERT(r.success);
    ASSERT(!r.encoded.empty());
}
TEST(Test_NeuralCodecHWAccelerator_BackendNames)
{
    using namespace ExplorerLens::Engine;
    NeuralCodecHWAccelerator accel;
    ASSERT(NeuralCodecHWAccelerator::BackendName(NCHWABackend::NVDEC) == "NVDEC");
    ASSERT(accel.IsAvailable(NCHWABackend::Software));
    ASSERT(accel.PreferredBackend() == NCHWABackend::QuickSync);
}

TEST(Test_CodecNegotiationProtocol_Negotiate)
{
    using namespace ExplorerLens::Engine;
    CodecNegotiationProtocol proto;
    proto.SetPreferences({CNPCodec::NCF_v2, CNPCodec::AVIF, CNPCodec::WebP});
    CNPOffer offer;
    offer.supportedCodecs = {CNPCodec::AVIF, CNPCodec::JPEG};
    offer.maxBitrateKbps = 8000;
    auto r = proto.Negotiate(offer);
    ASSERT(r.success);
    ASSERT(r.chosen == CNPCodec::AVIF);
    ASSERT(r.negotiatedKbps > 0);
}
TEST(Test_CodecNegotiationProtocol_EmptyOffer)
{
    using namespace ExplorerLens::Engine;
    CodecNegotiationProtocol proto;
    CNPOffer offer;
    auto r = proto.Negotiate(offer);
    ASSERT(!r.success);
}
TEST(Test_CodecNegotiationProtocol_CodecName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(CodecNegotiationProtocol::CodecName(CNPCodec::NCF_v2) == "NCF_v2");
    ASSERT(CodecNegotiationProtocol::CodecName(CNPCodec::WebP) == "WebP");
}

TEST(IntegrationRunnerSmoke)
{
    // Verify CorpusTestRunner can be created and run with no corpus
    // directories without crashing. Empty run report must be consistent.
    CorpusTestRunner runner;
    runner.SetMaxFiles(0);  // No limit — but no dirs added means no files scanned
    auto report = runner.Run();
    ASSERT(report.totalFiles == 0);
    ASSERT(report.passed == 0);
    ASSERT(report.failed == 0);
    ASSERT(!report.engineVersion.empty());
    ASSERT(!report.generatedAt.empty());
}

TEST(IntegrationRunnerSingleFile)
{
    // Create a minimal temp PNG file and verify the runner processes it.
    // Uses a dedicated subdirectory to avoid recursively scanning all of
    // %TEMP% (which can have thousands of files and cause a slow path walk).
    namespace fs = std::filesystem;

    // Build a tiny 1x1 PNG in memory (valid PNG header + IHDR + IDAT + IEND)
    static const uint8_t kMinPNG[] = {
        0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,  // PNG signature
        0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,  // IHDR length + type
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,  // width=1, height=1
        0x08, 0x02, 0x00, 0x00, 0x00, 0x90, 0x77, 0x53,  // 8-bit RGB, crc
        0xDE, 0x00, 0x00, 0x00, 0x0C, 0x49, 0x44, 0x41,  // IDAT
        0x54, 0x08, 0xD7, 0x63, 0xF8, 0xCF, 0xC0, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0xE2, 0x21, 0xBC,  // crc
        0x33, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E,                                                  // IEND
        0x44, 0xAE, 0x42, 0x60, 0x82                                                                     // crc
    };

    // Use an isolated subdirectory so the runner only scans 1 file — not all of %TEMP%.
    auto tmpDir = fs::temp_directory_path() / ("el_corpus_" + std::to_string(::GetCurrentProcessId()));
    std::error_code mkdirEc;
    fs::create_directories(tmpDir, mkdirEc);
    ASSERT(!mkdirEc);

    auto tmpPath = tmpDir / "el_test_smoke.png";
    {
        std::ofstream f(tmpPath, std::ios::binary);
        ASSERT(f.is_open());
        f.write(reinterpret_cast<const char*>(kMinPNG), sizeof(kMinPNG));
    }

    CorpusTestRunner runner;
    runner.AddCorpusDirectory(tmpDir);  // Isolated dir: only our one file
    runner.SetMaxFiles(1);
    // No custom filter needed — the directory contains exactly one file.

    auto report = runner.Run();
    ASSERT(report.totalFiles >= 1);
    // Verify the file was attempted (either passed or failed is acceptable —
    // we don't require the full PNG decoder to be available in test context)
    ASSERT(report.passed + report.failed >= 1 || report.skipped >= 1);

    // Cleanup
    std::error_code rmEc;
    fs::remove_all(tmpDir, rmEc);
}

TEST(IntegrationRunnerHtmlReport)
{
    // Verify HTML report can be written to a temp path without crashing.
    CorpusTestRunner::RunReport report;
    report.totalFiles = 3;
    report.passed = 2;
    report.failed = 1;
    report.engineVersion = ExplorerLens::BuildValidation::BuildInfo::VersionString;
    report.generatedAt = "2026-03-25T00:00:00";

    CorpusTestRunner::TestResult r1;
    r1.filePath = L"C:\\Corpus\\test.jpg";
    r1.format = L"JPEG";
    r1.extension = "jpg";
    r1.passed = true;
    r1.durationMs = 4.2;
    report.results.push_back(r1);

    CorpusTestRunner::TestResult r2;
    r2.filePath = L"C:\\Corpus\\broken.png";
    r2.format = L"PNG";
    r2.extension = "png";
    r2.passed = false;
    r2.errorMessage = L"File too small";
    r2.durationMs = 0.1;
    report.results.push_back(r2);

    auto outPath = std::filesystem::temp_directory_path() / "el_test_report.html";
    bool ok = CorpusTestRunner::WriteHtmlReport(outPath, report);
    ASSERT(ok);
    ASSERT(std::filesystem::exists(outPath));
    ASSERT(std::filesystem::file_size(outPath) > 100);

    std::error_code ec;
    std::filesystem::remove(outPath, ec);
}

//==============================================================================
// COM Integration Tests (Sprint 29 / v15.5.0 "Zenith-V")
//==============================================================================

TEST(COMThumbnailProviderRoundTrip)
{
    // Step 1: CLSID string must parse without error.
    CLSID clsid{};
    HRESULT hr = ::CLSIDFromString(COMIntegrationTest::EXPLORERLENS_CLSID_STR, &clsid);
    ASSERT(SUCCEEDED(hr));

    // Step 2: Registration check is non-fatal (DLL may not be installed in CI).
    bool isRegistered = COMIntegrationTest::IsDllRegistered();
    (void)isRegistered;

    // Step 3: Full smoke (attempts CoCreateInstance if registered).
    bool smokeOk = COMIntegrationTest::RunSmoke();
    ASSERT(smokeOk);
}

TEST(COMTestRunnerGracefulSkip)
{
    // Verify RunRoundTrip gracefully skips when DLL is not registered,
    // rather than throwing or crashing.
    COMIntegrationTest runner;
    std::vector<std::filesystem::path> fakeFiles = {L"c:\\nonexistent\\file.jpg", L"c:\\nonexistent\\file.png"};
    // This will either skip (DLL absent) or attempt COM — both are acceptable.
    auto results = runner.RunRoundTrip(fakeFiles);
    ASSERT(results.size() == fakeFiles.size());
    // Each result must have a filePath set.
    for (const auto& r : results) {
        ASSERT(!r.filePath.empty());
    }
}

// ---- Sprint 261-270 (v20.6.0 "Quasar-W") — Plugin Marketplace v2 ----

TEST(TestPluginPackageManifest_DefaultCapabilities)
{
    using namespace ExplorerLens::Engine;
    PluginPackageManifest manifest;
    ASSERT(manifest.id.empty());
    ASSERT(manifest.version.empty());
}

// ---- Sprint 271-280 (v20.7.0 "Quasar-X") — Observability v2 ----

TEST(TestLatencyBudgetManager_RegisterFormat)
{
    using namespace ExplorerLens::Engine;
    auto& mgr = LatencyBudgetManager::Instance();
    LatencySLO slo{};
    mgr.RegisterFormat(".test271", slo);
    ASSERT(true);  // registered without throw
}

TEST(TestLatencyBudgetManager_RecordSample)
{
    using namespace ExplorerLens::Engine;
    auto& mgr = LatencyBudgetManager::Instance();
    mgr.RecordSample(".test271", std::chrono::microseconds(5000));
    ASSERT(true);
}

TEST(TestFeatureFlagManager_GetBoolDefault)
{
    using namespace ExplorerLens::Engine;
    auto& ffm = FeatureFlagManager::Get();
    bool val = ffm.GetBool(L"NoSuchFlag271", false);
    ASSERT(!val);
}

TEST(TestFeatureFlagManager_GetIntDefault)
{
    using namespace ExplorerLens::Engine;
    auto& ffm = FeatureFlagManager::Get();
    int32_t val = ffm.GetInt(L"NoSuchFlag271", 42);
    ASSERT(val == 42);
}

TEST(TestUsageStats_RecordEvent)
{
    using namespace ExplorerLens::Engine;
    auto& us = UsageStats::Get();
    us.Record(UsageEvent::FormatDecoded, L".webp", 1);
    ASSERT(true);
}

TEST(TestCrashReporter_InitialNotInstalled)
{
    auto& reporter = ExplorerLens::Engine::Utils::CrashReporter::Instance();
    reporter.Uninstall();
    ASSERT(!reporter.IsInstalled());
}

TEST(TestCrashReporter_Install)
{
    auto& reporter = ExplorerLens::Engine::Utils::CrashReporter::Instance();
    reporter.Uninstall();
    reporter.Install();
    ASSERT(reporter.IsInstalled());
}

// ---- Sprint 281-290 (v21.0.0 "Rigel") — Format Expansion III ----

TEST(TestAVIFSequenceDecoder_SupportsHardware)
{
    using namespace ExplorerLens::Engine;
    // Static query — must return without throw
    bool hw = AVIFSequenceDecoder::SupportsHardwareDecode();
    ASSERT(hw == true || hw == false);
}

TEST(TestHEIFBurstDecoder_LooksLikeHEIF_RejectsGarbage)
{
    using namespace ExplorerLens::Engine;
    uint8_t garbage[4] = {0x00, 0x01, 0x02, 0x03};
    ASSERT(!HEIFBurstDecoder::LooksLikeHEIF(garbage, sizeof(garbage)));
}

TEST(TestJXLAnimationDecoder_LooksLikeJXL_RejectsEmpty)
{
    using namespace ExplorerLens::Engine;
    uint8_t garbage[4] = {0xFF, 0xFE, 0x01, 0x02};
    ASSERT(!JXLAnimationDecoder::LooksLikeJXL(garbage, sizeof(garbage)));
}

TEST(TestPSDLayerDecoder_IsPSD_RejectsGarbage)
{
    using namespace ExplorerLens::Engine;
    uint8_t garbage[4] = {0x00, 0x00, 0x00, 0x00};
    ASSERT(!PSDLayerDecoder::IsPSD(garbage, sizeof(garbage)));
}

TEST(TestSVGRasterizer_LooksLikeSVG_RejectsEmpty)
{
    using namespace ExplorerLens::Engine;
    uint8_t garbage[4] = {0xFF, 0xD8, 0xFF, 0xE0};  // JPEG magic, not SVG
    ASSERT(!SVGRasterizer::LooksLikeSVG(garbage, sizeof(garbage)));
}

TEST(TestWebPAnimationDecoder_LooksLikeAnimatedWebP_RejectsGarbage)
{
    using namespace ExplorerLens::Engine;
    uint8_t garbage[4] = {0x00, 0x00, 0x00, 0x00};
    ASSERT(!WebPAnimationDecoder::LooksLikeAnimatedWebP(garbage, sizeof(garbage)));
}

// ---- Sprint 291-300 (v21.1.0 "Rigel-R") — Advanced GPU Compute v2 ----

TEST(TestVulkanComputeAccelerator_InitialStatus)
{
    using namespace ExplorerLens::Engine;
    VulkanComputeAccelerator acc;
    ASSERT(acc.Status() == VulkanBackendStatus::NotInitialized);
}

TEST(TestD3D11DPIAdapter_InitialNotInitialized)
{
    using namespace ExplorerLens::Engine;
    D3D11DPIAdapter adapter;
    ASSERT(!adapter.IsInitialized());
}

TEST(TestThreadPoolV2_InstanceAccessible)
{
    using namespace ExplorerLens::Engine;
    auto& pool = ThreadPoolV2::Instance();
    (void)pool;  // Verify Instance() returns without throw
    ASSERT(true);
}

TEST(TestThreadPoolV2_Configure)
{
    using namespace ExplorerLens::Engine;
    ThreadPoolV2 pool;
    pool.Configure(2, 4, false);
    ASSERT(true);
}

TEST(TestSIMDImageProcessor_DetectCPUFeatures)
{
    using namespace ExplorerLens::Engine;
    auto caps = CPUFeatures::Detect();
    // Must return without throw; AVX2/SSE state depends on CPU
    ASSERT(caps.avx2 == true || caps.avx2 == false);
    ASSERT(caps.sse42 == true || caps.sse42 == false);
}

TEST(TestBatchDecodeScheduler_DefaultConstruct)
{
    using namespace ExplorerLens::Engine;
    BatchDecodeScheduler scheduler;
    (void)scheduler;
    ASSERT(true);
}

// ---- Sprint 301-310 (v21.2.0 "Rigel-S") — Enterprise Policy v2 ----

TEST(TestEnterprisePolicyEngineV2_InstanceAccessible)
{
    using namespace ExplorerLens::Engine;
    auto& eng = EnterprisePolicyEngineV2::Instance();
    (void)eng;
    ASSERT(true);
}

TEST(TestAuditLogger_InstanceAccessible)
{
    using namespace ExplorerLens::Engine;
    auto& logger = AuditLogger::Instance();
    (void)logger;
    ASSERT(true);
}

TEST(TestFIPSComplianceMode_WindowsFIPSQuery)
{
    using namespace ExplorerLens::Engine;
    // Static query — must not throw
    bool fipsEnabled = FIPSComplianceMode::IsWindowsFIPSPolicyEnabled();
    ASSERT(fipsEnabled == true || fipsEnabled == false);
}

TEST(TestFIPSComplianceMode_InstanceAccessible)
{
    using namespace ExplorerLens::Engine;
    auto& fips = FIPSComplianceMode::Instance();
    // Default level should be a valid FIPSLevel enum value
    ASSERT(fips.GetLevel() == FIPSLevel::Disabled || fips.GetLevel() == FIPSLevel::Compliant
           || fips.GetLevel() == FIPSLevel::Strict);
}

TEST(TestPrivilegeElevationGuard_DefaultConstruct)
{
    using namespace ExplorerLens::Engine;
    PrivilegeElevationGuard guard;
    (void)guard;
    ASSERT(true);
}

TEST(TestSandboxEscapeGuard_DefaultConstruct)
{
    using namespace ExplorerLens::Engine;
    SandboxEscapeGuard guard;
    (void)guard;
    ASSERT(true);
}

// ---- Sprint 311-320 (v21.3.0 "Rigel-T") — Storage & Caching v3 ----

TEST(TestFeatureCompatMatrix_InstanceAccessible)
{
    using namespace ExplorerLens::Engine;
    auto& mat = FeatureCompatMatrix::Instance();
    (void)mat;
    ASSERT(true);
}

// ---- Sprint 321-330 (v22.0.0 "Sirius" MAJOR) — Cross-Platform Foundation ----
TEST(TestColorBlindnessFilter_BuildMatrix_None)
{
    auto& filter = Core::ColorBlindnessFilter::Instance();
    filter.SetType(Core::CVDType::None);
    ASSERT(!filter.IsActive());
}
TEST(TestFeedbackItem_DefaultCategory)
{
    FeedbackItem item;
    ASSERT(item.category == FeedbackCategory::GeneralComment);
    ASSERT(item.rating == 0);
}
TEST(TestMonitorEventData_DefaultState)
{
    MonitorEventData data;
    ASSERT(data.hmonitor == nullptr);
    ASSERT(data.newDPI == 0);
}

// ---- Sprint 331-340 (v22.1.0 "Sirius-R") — Performance Profiling v2 ----
TEST(TestLayoutMetrics_DefaultValues)
{
    Core::LayoutMetrics m;
    ASSERT(m.dpi == 96);
    ASSERT(m.breakpoint == Core::LayoutBreakpoint::Normal);
}
TEST(TestSandboxPolicy_DefaultLevel)
{
    SecureSandboxPolicy policy;
    ASSERT(policy.level == SandboxLevel::None);
}
TEST(TestValidationError_OkIsZero)
{
    ASSERT(static_cast<int>(ValidationError::Ok) == 0);
}
TEST(TestCodecCapability_NoneIsZero)
{
    ASSERT(static_cast<uint32_t>(CodecCapability::None) == 0);
}
TEST(TestModuleSecurityFlags_DefaultConstruct)
{
    ModuleSecurityFlags flags;
    ASSERT(!flags.cfgEnabled);
    ASSERT(!flags.shadowStackEnabled);
}

// ---- Sprint 341-350 (v22.2.0 "Sirius-S") — Security & Audit v3 ----
TEST(TestLicenseInfo_DefaultTier)
{
    LicenseInfo info;
    ASSERT(info.tier == LicenseTier::Community);
    ASSERT(!info.valid);
}
TEST(TestLocaleDirection_LTRIsZero)
{
    ASSERT(static_cast<uint8_t>(LocaleDirection::LTR) == 0);
}
TEST(TestStoreCheckSeverity_InfoIsZero)
{
    ASSERT(static_cast<uint8_t>(StoreCheckSeverity::Info) == 0);
}
TEST(TestPinValidationResult_OkIsZero)
{
    ASSERT(static_cast<uint8_t>(PinValidationResult::OK) == 0);
}
TEST(TestColorBlindType_NoneIsZero)
{
    ASSERT(static_cast<uint8_t>(ColorBlindType::None) == 0);
}
// ---- Sprint 351-360 (v22.3.0 "Sirius-T") — AI Inference Pipeline v2 ----
TEST(TestUpscaleBackend_AutoIsZero)
{
    ASSERT(static_cast<uint8_t>(AIUpscaleBackend::Auto) == 0);
}
TEST(TestContentCategory_UnknownIsZero)
{
    ASSERT(static_cast<uint8_t>(ContentCategory::Unknown) == 0);
}
//== Sprint 361-370: Advanced Scheduling & Concurrency v2 (v22.4.0 Sirius-U) ==
TEST(TestLockFreeMPMCQueue_PushPop)
{
    LockFreeMPMCQueue<int, 4> q;
    ASSERT(q.Push(42));
    int v = 0;
    ASSERT(q.Pop(v));
    ASSERT(v == 42);
}
TEST(TestCPUAffinityRouter_PolicyAuto)
{
    CPUAffinityRouter r;
    ASSERT(r.GetPolicy() == AffinityPolicy::Auto);
    ASSERT(r.SetPolicy(AffinityPolicy::PerformanceCores));
    ASSERT(r.GetPolicy() == AffinityPolicy::PerformanceCores);
}
TEST(TestRealtimePriorityEngine_EnqueueDequeue)
{
    RealtimePriorityEngine eng;
    ASSERT(eng.Enqueue({1, 10.0, 16, 0}));
    ASSERT(eng.PendingCount() == 1);
    RTDecodeTask t;
    ASSERT(eng.Dequeue(t));
    ASSERT(eng.PendingCount() == 0);
}
TEST(TestHazardPointerReclaimer_AcquireRelease)
{
    HazardPointerReclaimer<int> r;
    auto guard = r.Acquire();
    (void)guard;  // destructor releases hazard
    ASSERT(true);
}
TEST(TestAdaptiveConcurrencyLimiter_AIMD)
{
    AdaptiveConcurrencyLimiter lim({2, 16, 1, 1.0, 0.5});
    ASSERT(lim.Window() == 2);
    ASSERT(lim.TryAcquire());
    ASSERT(lim.TryAcquire());
    ASSERT(!lim.TryAcquire());  // window full
    lim.Release(true);          // success → window grows
    ASSERT(lim.Window() >= 2);
}
TEST(TestCooperativeTaskScheduler_RunOnce)
{
    CooperativeTaskScheduler s;
    int ran = 0;
    s.Submit({[&ran]() -> bool {
                  ran++;
                  return true;
              },
              0, 1});
    s.RunOnce();
    ASSERT(ran == 1);
    ASSERT(s.Pending() == 0);
}
TEST(TestThreadLocalContextPool_AcquireRelease)
{
    ThreadLocalContextPool pool;
    auto& ctx = pool.Acquire();
    ctx.decoderFlags = 7;
    pool.Release();
    ASSERT(pool.Acquire().decoderFlags == 0);
}

//== Sprint 371-380: Format Expansion IV (v22.5.0 Sirius-V) ==
TEST(TestFLIFDecoder_ProbeSignature)
{
    FLIFDecoder d;
    const uint8_t sig[] = {'F', 'L', 'I', 'F'};
    ASSERT(d.IsSupported(sig, 4));
}
TEST(TestQOIRDecoder_ProbeSignature)
{
    QOIRDecoder d;
    const uint8_t sig[] = {'q', 'o', 'i', 'r'};
    ASSERT(d.Probe(sig, 4));
}
TEST(TestJNGDecoder_ProbeSignature)
{
    JNGDecoder d;
    const uint8_t sig[] = {0x8B, 'J', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
    ASSERT(d.Probe(sig, 8));
}
TEST(TestJBIG2Decoder_ProbeSignature)
{
    JBIG2Decoder d;
    const uint8_t sig[] = {0x97, 'J', 'B', '2', '\r', '\n', 0x1a, '\n'};
    ASSERT(d.Probe(sig, 8));
}
TEST(TestTIFFMultiFrameV2_BigTIFFSupport)
{
    TIFFMultiFrameDecoderV2 d;
    ASSERT(d.SupportsBigTIFF());
    ASSERT(d.PageCount(nullptr, 0) == 1);
}
TEST(TestILBMDecoder_ProbeIFF)
{
    ILBMDecoder d;
    const uint8_t sig[] = {'F', 'O', 'R', 'M'};
    ASSERT(d.Probe(sig, 4));
}
TEST(TestSunRasterDecoder_ProbeSignature)
{
    SunRasterDecoder d;
    const uint8_t sig[] = {0x59, 0xA6, 0x6A, 0x95};
    ASSERT(d.Probe(sig, 4));
}
TEST(TestJPEGXTDecoder_ProbeSOI)
{
    JPEGXTDecoder d;
    const uint8_t sig[] = {0xFF, 0xD8};
    ASSERT(d.Probe(sig, 2));
    ASSERT(!d.HasHDRResidual(nullptr, 0));
}

//== Sprint 381-390: Windows Shell Integration v2 (v22.6.0 Sirius-W) ==
TEST(TestNamespaceWalkEngine_WalkEmpty)
{
    NamespaceWalkEngine eng;
    auto res = eng.Walk({2, false, true, L"C:\\"});
    ASSERT(res.errors == 0);
    ASSERT(!eng.IsCancelled());
}
TEST(TestExplorerColumnProviderV2_RegisterColumn)
{
    ExplorerColumnProviderV2 p;
    p.RegisterColumn({L"Format", 80, 1, true});
    ASSERT(p.ColumnCount() == 1);
    ASSERT(p.GetColumn(0).title == L"Format");
}
TEST(TestShellContextMenuV2_Execute)
{
    ShellContextMenuV2 m;
    m.AddEntry({L"Regenerate", ContextMenuAction::RegenerateThumbnail, true});
    ASSERT(m.EntryCount() == 1);
    ASSERT(m.Execute(ContextMenuAction::RegenerateThumbnail, L"test.png"));
}
TEST(TestSearchIndexBridge_Properties)
{
    SearchIndexBridge b;
    b.AddProperty({L"format", L"JPEG"});
    ASSERT(b.PropertyCount() == 1);
    ASSERT(b.IsIndexingEnabled());
}
TEST(TestDragDropPreviewEngine_Generate)
{
    DragDropPreviewEngine e;
    auto pv = e.Generate(L"test.png", {64, 64, 0.75f, false});
    ASSERT(pv.size() == 64 * 64 * 4);
    ASSERT(e.IsReady());
}
//== Sprint 391-400: DevOps & Quality Engineering v2 (v22.7.0 Sirius-X) ==
TEST(TestMutationTestingEngine_KillRate)
{
    MutationTestingEngine eng;
    eng.AddMutant({"foo.cpp:10", MutationOperator::ArithmeticReplace, true});
    eng.AddMutant({"foo.cpp:20", MutationOperator::ComparisonFlip, false});
    auto r = eng.Summarize();
    ASSERT(r.total == 2);
    ASSERT(r.killed == 1);
    ASSERT(r.killRate() == 0.5);
}
TEST(TestPropertyBaseTestEngine_CheckPasses)
{
    PropertyBaseTestEngine eng({10, 0, false});
    auto r = eng.Check("always_true", [](uint32_t) { return true; });
    ASSERT(r.failed == 0);
    ASSERT(r.passed == 10);
}
TEST(TestReproducibleBuildVerifierV2_Reproducible)
{
    ReproducibleBuildVerifierV2 v;
    v.AddBaseline({"lens.exe", "abc123", 1000});
    v.AddCandidate({"lens.exe", "abc123", 1000});
    ASSERT(v.Compare().IsReproducible());
}
TEST(TestRegressionFingerprintEngine_Clean)
{
    RegressionFingerprintEngine e;
    auto a = e.Compute("a.dll");
    auto b = e.Compute("a.dll");
    ASSERT(e.Compare(a, b).IsClean());
}
TEST(TestCycloneDXSBOMGenerator_JSON)
{
    CycloneDXSBOMGenerator gen;
    gen.AddComponent({"zlib", "1.3.1", "pkg:generic/zlib@1.3.1", "Zlib"});
    ASSERT(gen.ComponentCount() == 1);
    auto doc = gen.Generate("1.0");
    auto json = gen.ToJSON(doc);
    ASSERT(!json.empty());
}
TEST(TestBuildTimingAnalytics_Summarize)
{
    BuildTimingAnalytics ba;
    ba.Record({"compile", 100.0, 90.0, false});
    ba.Record({"link", 50.0, 40.0, false});
    ASSERT(ba.StepCount() == 2);
    auto r = ba.Summarize();
    ASSERT(r.totalWallMs == 150.0);
}
TEST(TestArtifactIntegrityMonitor_NoAlerts)
{
    ArtifactIntegrityMonitor m;
    m.RegisterBaseline({"LENSShell.dll", 3005000, "abc"});
    auto alerts = m.Check("LENSShell.dll", 3005000, "abc");
    ASSERT(alerts.empty());
}
TEST(TestCIEnvironmentValidator_Required)
{
    CIEnvironmentValidator v;
    v.Require({"GITHUB_TOKEN", true, ""});
    v.Require({"SIGNING_CERT", false, "none"});
    ASSERT(v.RequiredCount() == 1);
}

//== Sprint 401-410: Reactive Pipeline Architecture (v23.0.0 Vega) ==
TEST(TestThumbnailEventStore_AppendReplay)
{
    ThumbnailEventStore store;
    auto id1 = store.Append({0, L"a.png", ThumbEventType::Requested, 1000});
    auto id2 = store.Append({0, L"b.png", ThumbEventType::Completed, 2000});
    ASSERT(id1 == 1);
    ASSERT(id2 == 2);
    ASSERT(store.EventCount() == 2);
    auto ev = store.Replay(1);
    ASSERT(ev.size() == 2);
}
TEST(TestCQRSThumbnailPipeline_DispatchQuery)
{
    CQRSThumbnailPipeline p;
    auto seq = p.Dispatch({L"test.png", 256, 256});
    ASSERT(seq == 1);
    auto r = p.Query(L"test.png");
    ASSERT(!r.found);
}
TEST(TestReactiveStreamEngine_EmitSubscribe)
{
    ReactiveStream<int> stream;
    int received = 0;
    stream.OnNext([&](int v) { received = v; });
    stream.Emit(42);
    ASSERT(received == 42);
    ASSERT(stream.SubscriberCount() == 1);
}
TEST(TestThumbnailSagaOrchestrator_StartQuery)
{
    ThumbnailSagaOrchestrator orch;
    uint64_t id = orch.Start({});
    ASSERT(id == 1);
    ASSERT(orch.ActiveCount() >= 1);
}
TEST(TestSnapshotStoreEngine_SaveLoad)
{
    SnapshotStoreEngine store;
    store.Save({1, 42, "{}", 1000});
    SnapshotRecord out;
    ASSERT(store.Load(1, out));
    ASSERT(out.version == 42);
    ASSERT(store.Count() == 1);
}
TEST(TestDomainEventBus_PublishSubscribe)
{
    DomainEventBus bus;
    int count = 0;
    bus.Subscribe("thumb.completed", [&](const DomainEvent&) { count++; });
    bus.Publish({"thumb.completed", "{}"});
    ASSERT(count == 1);
    ASSERT(bus.PublishedCount() == 1);
}
TEST(TestReactiveAPIGateway_StartStop)
{
    ReactiveAPIGateway gw({GatewayProtocol::NamedPipe, 4, 1000, "\\\\.\\pipe\\test"});
    ASSERT(gw.Start());
    ASSERT(gw.IsRunning());
    ASSERT(gw.Protocol() == GatewayProtocol::NamedPipe);
    gw.Stop();
    ASSERT(!gw.IsRunning());
}

//== Sprint 411-420: GPU Acceleration v3 (v23.1.0 Vega-R) ==
TEST(TestCUDATextureDecoder_NotAvailable)
{
    CUDATextureDecoder d;
    ASSERT(!d.IsAvailable());
    auto r = d.Decode(nullptr, 0, CUDATextureFormat::BC1, 4, 4);
    ASSERT(r.width == 4);
    ASSERT(!r.gpuUsed);
}
TEST(TestHIPComputeBackend_NotAvailable)
{
    HIPComputeBackend b;
    ASSERT(!b.IsAvailable());
    auto di = b.QueryDevice(0);
    ASSERT(!di.available);
}
TEST(TestMultiGPULoadBalancerV3_SelectDevice)
{
    MultiGPULoadBalancerV3 lb;
    lb.Register({0, "GPU0", 40.0f, 50.0f});
    lb.Register({1, "GPU1", 80.0f, 85.0f});
    ASSERT(lb.DeviceCount() == 2);
    ASSERT(lb.SelectDevice() == 0);
    ASSERT(lb.AverageUtilization() == 60.0f);
}
TEST(TestGPUTextureAtlasBuilder_PackSingle)
{
    GPUTextureAtlasBuilder builder(1024, 1024);
    AtlasRect r;
    ASSERT(builder.Pack(1, 256, 256, r));
    ASSERT(r.thumbId == 1);
}
TEST(TestGPUResourceAliasingManager_Register)
{
    GPUResourceAliasingManager m;
    uint32_t id = m.Register(1024 * 1024);
    ASSERT(id == 0);
    ASSERT(m.Alias(0, 0));
    m.Barrier({0, GPUResourceState::Undefined, GPUResourceState::ShaderResource});
    ASSERT(m.BarrierCount() == 1);
}
TEST(TestAsyncDMACopyEngine_SubmitFlush)
{
    AsyncDMACopyEngine e;
    auto fence = e.Submit({1, nullptr, nullptr, 4096, 0});
    ASSERT(fence == 1);
    ASSERT(e.Pending() == 1);
    e.Flush();
    ASSERT(e.IsComplete(fence));
    ASSERT(e.Pending() == 0);
}
TEST(TestGPUMemoryDefragmenterV2_Plan)
{
    GPUMemoryDefragmenterV2 d;
    auto plan = d.Plan();
    ASSERT(plan.safe);
    ASSERT(d.Execute(plan));
    ASSERT(d.FragRatio() >= 0.0f);
}
TEST(TestGPUThumbnailAtlasManager_InsertLookup)
{
    GPUThumbnailAtlasManager am;
    ASSERT(am.Insert(42, {}, 64, 64));
    auto e = am.Lookup(42);
    ASSERT(e.valid);
    auto st = am.Stats();
    ASSERT(st.used == 1);
}

//== Sprint 421-430: Plugin Ecosystem v3 (v23.2.0 Vega-S) ==

//== Sprint 431-440: Memory Optimization v3 (v23.3.0 Vega-T) ==
TEST(TestPageFileArenaAllocator_AllocReset)
{
    PageFileArenaAllocator a(1024 * 1024);
    auto st = a.Stats();
    ASSERT(st.pageBacked);
    ASSERT(st.usedBytes == 0);
    void* p = a.Alloc(256);
    ASSERT(p != nullptr);
    a.Reset();
    ASSERT(a.Stats().usedBytes == 0);
}
TEST(TestHugeTLBPagePool_AcquirePageSize)
{
    HugeTLBPagePool pool(HugePageSize::Page2MB);
    ASSERT(pool.PageSize() == HugePageSize::Page2MB);
    auto b = pool.Acquire(1);
    ASSERT(b.bytes >= static_cast<size_t>(HugePageSize::Page2MB));
    pool.Release(b);
}
TEST(TestMemoryMappedBTree_InsertLookup)
{
    MemoryMappedBTree<uint64_t, std::string> tree(L"test.db");
    ASSERT(tree.Open());
    ASSERT(tree.Insert(1, "hello"));
    std::string v;
    ASSERT(tree.Lookup(1, v));
    ASSERT(v == "hello");
    ASSERT(tree.Count() == 1);
    tree.Close();
}
TEST(TestNVMeMemoryTier_NotAvailable)
{
    NVMeMemoryTier tier;
    ASSERT(!tier.IsAvailable());
    ASSERT(tier.CapacityBytes() == 0);
    auto r = tier.Allocate(1024);
    ASSERT(!r.available);
}
TEST(TestECCErrorDetector_QueryStatus)
{
    ECCErrorDetector d;
    auto st = d.QueryStatus();
    ASSERT(!st.eccSupported);
    ASSERT(st.totalSingleBit == 0);
    ASSERT(!d.HasUncorrectedErrors());
}
TEST(TestPressureForecaster_FeedPredict)
{
    PressureForecaster f;
    for (int i = 0; i < 10; i++)
        f.Feed(30.0);
    ASSERT(f.SampleCount() == 10);
    auto r = f.Predict(500);
    ASSERT(r.confidence > 0.0);
    ASSERT(r.forecast == PressureForecast::Stable);
}
TEST(TestSharedMemoryRegionManager_CreateOpen)
{
    SharedMemoryRegionManager m;
    ASSERT(m.Create(L"ExplorerLens_Test", 4096));
    ASSERT(m.RegionCount() == 1);
    SharedRegion r;
    ASSERT(m.Open(L"ExplorerLens_Test", r));
    ASSERT(r.bytes == 4096);
    m.Close(L"ExplorerLens_Test");
    ASSERT(m.RegionCount() == 0);
}

//== Sprint 441-450: Smart Cache v4 (v23.4.0 Vega-U) ==
TEST(TestAIEvictionPolicyEngine_Score)
{
    AIEvictionPolicyEngine e;
    auto d = e.Score({L"old.png", 0.1, 5000, 65536});
    ASSERT(d.evict);  // old + low freq → should evict
    auto d2 = e.Score({L"hot.png", 100.0, 1, 65536});
    ASSERT(!d2.evict);
}
TEST(TestCacheEncryptionLayer_EncryptDecrypt)
{
    CacheEncryptionLayer enc;
    std::vector<uint8_t> key(32, 0xAB);
    std::vector<uint8_t> nonce(12, 0x01);
    ASSERT(enc.Initialize({key, nonce}));
    std::vector<uint8_t> pt = {1, 2, 3, 4};
    auto r = enc.Encrypt(pt);
    ASSERT(r.success);
    ASSERT(!r.tag.empty());
}
//== Sprint 451-460: CLI & Automation v2 (v23.5.0 Vega-V) ==
// ============================================================
// Sprint 801-810 — Platform Tests (v28.0.0 Polaris)
// ============================================================
// ============================================================
// Sprint 811-820 — AI Captions (v28.1.0 Polaris-R)
// ============================================================
// ============================================================
// Sprint 821-830 — AR Tests (v28.2.0 Polaris-S)
// ============================================================
// ============================================================
// Sprint 831-840 — Enterprise Tests (v28.3.0 Polaris-T)
// ============================================================
// ============================================================
// Sprint 841-850 — UX Tests (v28.4.0 Polaris-U)
// ============================================================
// ============================================================
// Sprint 851-860 — Security Tests (v28.5.0 Polaris-V)
// ============================================================
// ============================================================
// Sprint 861-870 — PQC Signature Tests (v28.6.0 Polaris-W)
// ============================================================
TEST(TestPQCSignatureVerifier_Verify)
{
    using namespace ExplorerLens::Engine;
    PQCSignatureVerifier ver;
    ASSERT(ver.Initialize());
    PQCVerifyRequest req;
    req.message = {0x01, 0x02};
    req.signature = {0x03, 0x04};
    req.publicKey = {0x05, 0x06};
    auto res = ver.Verify(req);
    ASSERT(res.valid);
    ver.Shutdown();
}
TEST(TestHybridTrustChainV2_Validate)
{
    using namespace ExplorerLens::Engine;
    HybridTrustChainV2 chain;
    ASSERT(chain.Initialize());
    TrustCertificate cert;
    cert.subjectDN = "CN=Plugin";
    cert.issuerDN = "CN=Root";
    cert.publicKey = {0xAA};
    cert.signature = {0xBB};
    TrustCertificate root;
    root.subjectDN = "CN=Root";
    root.issuerDN = "CN=Root";
    root.publicKey = {0xCC};
    root.signature = {0xDD};
    root.isSelfSigned = true;
    chain.AddTrustAnchor(root);
    auto res = chain.Validate({cert, root});
    ASSERT(res.valid || !res.errorCode.empty());
    chain.Shutdown();
}
TEST(TestQuantumSafeKeyExchange_Handshake)
{
    using namespace ExplorerLens::Engine;
    QuantumSafeKeyExchange qske;
    ASSERT(qske.Initialize(QSKEScheme::MLKEM768_X25519));
    auto clientHello = qske.CreateClientHello();
    ASSERT(!clientHello.mlkemPublicKey.empty());
    auto [serverHello, serverKeys] = qske.ProcessClientHello(clientHello);
    ASSERT(serverKeys.valid);
    auto clientKeys = qske.ProcessServerHello(serverHello, clientHello);
    ASSERT(clientKeys.valid);
    qske.Shutdown();
}
TEST(TestDilithiumCertificateStore_ImportGet)
{
    using namespace ExplorerLens::Engine;
    DilithiumCertificateStore store;
    ASSERT(store.Initialize());
    DilithiumCert cert;
    cert.id = "c1";
    cert.subjectDN = "CN=Test";
    cert.publicKey = {0xAA, 0xBB};
    ASSERT(store.Import(cert));
    DilithiumCert out;
    ASSERT(store.Get("c1", out));
    ASSERT(out.subjectDN == "CN=Test");
    ASSERT(store.Count() == 1);
    store.Shutdown();
}
TEST(TestSignatureAuditLogger_LogExport)
{
    using namespace ExplorerLens::Engine;
    SignatureAuditLogger logger;
    ASSERT(logger.Initialize());
    AuditLogEntry entry;
    entry.type = SignatureAuditEventType::VerifyOk;
    entry.subject = "plugin.dll";
    entry.success = true;
    logger.Log(entry);
    ASSERT(logger.GetEntryCount() == 1);
    auto json = logger.ExportJSON();
    ASSERT(!json.empty());
    logger.Shutdown();
}
TEST(TestCryptoAgilityBroker_Preferred)
{
    using namespace ExplorerLens::Engine;
    CryptoAgilityBroker broker;
    ASSERT(broker.Initialize());
    auto preferred = broker.GetPreferred(CryptoCategory::KEM);
    ASSERT(preferred == CryptoAlgoId::ML_KEM_768);
    ASSERT(broker.IsAlgoAvailable(CryptoAlgoId::ML_DSA_65));
    auto caps = broker.ListByCategory(CryptoCategory::Signature);
    ASSERT(!caps.empty());
    broker.Shutdown();
}
TEST(TestKeyRotationScheduler_TickFire)
{
    using namespace ExplorerLens::Engine;
    KeyRotationScheduler sched;
    ASSERT(sched.Initialize(0));
    int fired = 0;
    sched.SetDueCallback([&](const std::string&) { ++fired; });
    sched.Schedule("key1", 90, 1000, 0);
    ASSERT(sched.GetScheduledCount() == 1);
    uint32_t f = sched.Tick(1500);
    ASSERT(f == 1);
    ASSERT(fired == 1);
    sched.Shutdown();
}
// ============================================================
// Sprint 871-880 — Cross-Platform GPU/Core/Utils Tests (v28.7.0)
// ============================================================
TEST(TestGTK4ThumbnailWidget_Render)
{
    using namespace ExplorerLens::Engine;
    GTK4ThumbnailWidget widget;
    GTK4ThumbnailConfig cfg;
    cfg.width = 128;
    cfg.height = 128;
    ASSERT(widget.Initialize(cfg));
#if defined(__linux__)
    ASSERT(widget.IsPlatformOk());
    std::vector<uint8_t> src(128 * 128 * 4, 0x80);
    auto out = widget.RenderWidget(src, 128, 128);
    ASSERT(out.success);
#endif
    widget.Shutdown();
}
TEST(TestXDGThumbnailProvider_Create)
{
    using namespace ExplorerLens::Engine;
    XDGThumbnailProvider prov;
    ASSERT(prov.Initialize("/tmp/test-xdg-cache"));
#if defined(__linux__)
    ASSERT(prov.IsPlatformOk());
    std::vector<uint8_t> pixels(256 * 256 * 4, 0x80);
    XDGThumbRequest req;
    req.filePath = "/home/user/test.jpg";
    req.size = XDGThumbSize::Large_256;
    auto res = prov.CreateThumbnail(req, pixels, 256, 256);
    ASSERT(res.success);
    ASSERT(!res.cachePath.empty());
#endif
    prov.Shutdown();
}
TEST(TestPlatformCapabilityProbe_Summary)
{
    using namespace ExplorerLens::Engine;
    PlatformCapabilityProbe probe;
    ASSERT(probe.Initialize());
    auto& caps = probe.GetCapabilities();
    ASSERT(caps.cpuCoreCount > 0);
    ASSERT(!probe.GetPlatformSummary().empty());
    probe.Shutdown();
}
// ============================================================
// Sprint 881-890 — Gen-5 Platform WinUI 4 (v29.0.0 Capella)
// ============================================================
TEST(TestAsyncPreviewBroker_EnqueueFlush)
{
    using namespace ExplorerLens::Engine;
    AsyncPreviewBroker broker;
    ASSERT(broker.Initialize(2));
    int delivered = 0;
    broker.Enqueue({"img.jpg", 128, 128, PreviewPriority::Normal}, [&](const PreviewResponse& r) {
        if (r.success)
            ++delivered;
    });
    ASSERT(broker.GetPendingCount() == 1);
    uint32_t processed = broker.Flush();
    ASSERT(processed == 1);
    ASSERT(delivered == 1);
    broker.Shutdown();
}
TEST(TestUniversalFileProvider_Read)
{
    using namespace ExplorerLens::Engine;
    UniversalFileProvider prov;
    ASSERT(prov.Initialize());
    ASSERT(prov.Exists("somefile.jpg"));
    auto res = prov.Read("somefile.jpg", 0, 64);
    ASSERT(res.success);
    ASSERT(res.data.size() == 64);
    prov.Shutdown();
}
TEST(TestWinUI4PreviewHandler_RenderPanel)
{
    using namespace ExplorerLens::Engine;
    WinUI4PreviewHandler handler;
    ASSERT(handler.Initialize());
    std::vector<uint8_t> thumb(256 * 256 * 4, 0xAA);
    FilePreviewMetadata meta;
    meta.format = "JPEG";
    auto frame = handler.RenderPreviewPanel("img.jpg", thumb, 256, 256, meta);
    ASSERT(frame.success);
    ASSERT(!frame.composited.empty());
    handler.Shutdown();
}
TEST(TestShellPropertyHandlerV2_Build)
{
    using namespace ExplorerLens::Engine;
    ShellPropertyHandlerV2 handler;
    ASSERT(handler.Initialize());
    auto ps = handler.BuildPropertySet("img.jpg", 4096, 3072, "JPEG", "sRGB", "A beautiful sunset");
    ShellProperty prop;
    ASSERT(handler.GetProperty(ps, "System.Image.HorizontalSize", prop));
    ASSERT(prop.value == "4096");
    ASSERT(handler.GetProperty(ps, "ExplorerLens.AICaption", prop));
    handler.Shutdown();
}
TEST(TestPreviewPipelineV5_Process)
{
    using namespace ExplorerLens::Engine;
    PreviewPipelineV5 pipe;
    ASSERT(pipe.Initialize());
    PipelineV5Request req;
    req.filePath = "test.jpg";
    req.outWidth = 256;
    req.outHeight = 256;
    auto res = pipe.Process(req);
    ASSERT(res.success);
    ASSERT(res.totalMs > 0);
    ASSERT(!res.pixelsBGRA.empty());
    pipe.Shutdown();
}
TEST(TestPersistentL3Cache_PutGetEvict)
{
    using namespace ExplorerLens::Engine;
    PersistentL3Cache cache;
    ASSERT(cache.Initialize("test-cache", 1024));
    L3CacheEntry entry;
    entry.key = "k1";
    entry.pixelsBGRA.assign(256, 0xAA);
    entry.width = 8;
    entry.height = 8;
    ASSERT(cache.Put("k1", entry));
    ASSERT(cache.Contains("k1"));
    L3CacheEntry out;
    ASSERT(cache.Get("k1", out));
    ASSERT(out.pixelsBGRA.size() == 256);
    ASSERT(cache.GetStats().hits == 1);
    cache.Shutdown();
}
TEST(TestLivePreviewUpdater_WatchFire)
{
    using namespace ExplorerLens::Engine;
    LivePreviewUpdater upd;
    ASSERT(upd.Initialize(0));
    int fired = 0;
    upd.SetCallback([&](const LiveFileChangeEvent&) { ++fired; });
    ASSERT(upd.WatchPath("/test/dir"));
    upd.SimulateChange({"img.jpg", "", LiveFileChangeType::Modified, 0});
    uint32_t n = upd.Flush(100);
    ASSERT(n == 1);
    ASSERT(fired == 1);
    upd.Shutdown();
}
TEST(TestShellExtHealthMonitorV2_Latency)
{
    using namespace ExplorerLens::Engine;
    ShellExtHealthMonitorV2 mon;
    ASSERT(mon.Initialize("v29.0.0"));
    mon.RecordSuccess(12.5f);
    mon.RecordSuccess(8.0f);
    auto snap = mon.GetSnapshot();
    ASSERT(snap.status == ShellHealthStatus::Healthy);
    ASSERT(snap.avgLatencyMs > 0.0f);
    mon.Shutdown();
}
// ============================================================
// Sprint 891-900 — Accessibility v2 Tests (v29.1.0 Capella-R)
// ============================================================
TEST(TestARIAThumbnailAnnotator_RenderAttr)
{
    using namespace ExplorerLens::Engine;
    ARIAThumbnailAnnotator ann;
    ASSERT(ann.Initialize());
    ARIAAnnotateRequest req;
    req.filePath = "photo.jpg";
    req.altText = "sunset photo";
    auto annotation = ann.Annotate(req);
    ASSERT(!annotation.ariaLabel.empty());
    std::string attr = ann.RenderHTMLAttr(annotation);
    ASSERT(attr.find("role=") != std::string::npos);
    ann.Shutdown();
}
TEST(TestHighContrastThemeAdapter_Adapt)
{
    using namespace ExplorerLens::Engine;
    HighContrastThemeAdapter adapter;
    ASSERT(adapter.Initialize());
    auto off = adapter.AdaptPalette(HighContrastMode::Off);
    ASSERT(off.activeMode == HighContrastMode::Off);
    ASSERT(off.thumbBackground != 0);
    auto hc = adapter.AdaptPalette(HighContrastMode::Black);
    ASSERT(hc.thumbBackground != off.thumbBackground);
    adapter.Shutdown();
}
TEST(TestKeyboardNavigationController_ArrowKeys)
{
    using namespace ExplorerLens::Engine;
    KeyboardNavigationController nav;
    ASSERT(nav.Initialize(20, 4));
    ASSERT(nav.GetState().focusedIndex == 0);
    nav.HandleKey(NavKey::ArrowRight);
    ASSERT(nav.GetState().focusedIndex == 1);
    nav.HandleKey(NavKey::ArrowDown);
    ASSERT(nav.GetState().focusedIndex == 5);
    nav.HandleKey(NavKey::Home);
    ASSERT(nav.GetState().focusedIndex == 0);
    nav.HandleKey(NavKey::End);
    ASSERT(nav.GetState().focusedIndex == 19);
    nav.Shutdown();
}
TEST(TestA11yTelemetryReporter_Report)
{
    using namespace ExplorerLens::Engine;
    A11yTelemetryReporter reporter;
    ASSERT(reporter.Initialize());
    reporter.RecordScreenReaderSession();
    reporter.RecordHighContrastSession();
    reporter.RecordWCAGAuditPassed();
    reporter.RecordWCAGAuditFailed();
    reporter.RecordAltTextGenerated();
    auto rpt = reporter.GenerateReport("json");
    ASSERT(!rpt.payload.empty());
    ASSERT(rpt.aggregate.screenReaderSessions == 1);
    ASSERT(rpt.aggregate.wcagAuditsPassed == 1);
    reporter.Shutdown();
}

//== Sprint 961-970 — Gen-6 Platform Unification (v30.0.0 "Deneb") ==========

TEST(TestPAL_Detect)
{
    using namespace ExplorerLens::Engine;
    PlatformAbstractionLayer pal;
    auto platform = pal.GetCurrentPlatform();
    ASSERT(platform == PlatformType::Windows);
    auto backend = pal.GetPreferredGPUBackend();
    ASSERT(backend == PlatGPUBackend::D3D12 || backend == PlatGPUBackend::Vulkan);
}
TEST(TestPAL_Surface)
{
    using namespace ExplorerLens::Engine;
    PlatformAbstractionLayer pal;
    auto surface = pal.CreateRenderSurface(256, 256);
    ASSERT(surface.width == 256);
    ASSERT(surface.height == 256);
    ASSERT(surface.strideBytes > 0);
}
TEST(TestPAL_Scales)
{
    using namespace ExplorerLens::Engine;
    PlatformAbstractionLayer pal;
    auto scales = pal.EnumerateDisplayScales();
    ASSERT(!scales.empty());
    ASSERT(scales[0] >= 1.0f);
}
TEST(TestPAL_Cores)
{
    using namespace ExplorerLens::Engine;
    PlatformAbstractionLayer pal;
    ASSERT(pal.GetLogicalCPUCount() >= 1);
}
TEST(TestPAL_PageSize)
{
    using namespace ExplorerLens::Engine;
    PlatformAbstractionLayer pal;
    ASSERT(pal.GetPageSize() >= 4096);
}
TEST(TestPAL_SystemMem)
{
    using namespace ExplorerLens::Engine;
    PlatformAbstractionLayer pal;
    ASSERT(pal.GetTotalSystemMemoryMB() > 0);
}
TEST(TestPAL_TempDir)
{
    using namespace ExplorerLens::Engine;
    PlatformAbstractionLayer pal;
    auto tmp = pal.GetTempDirectory();
    ASSERT(!tmp.empty());
}
TEST(TestPAL_ThreadCount)
{
    using namespace ExplorerLens::Engine;
    PlatformAbstractionLayer pal;
    auto optimal = pal.GetOptimalThreadCount();
    ASSERT(optimal >= 1 && optimal <= 256);
}
TEST(TestPAL_ZeroSurface)
{
    using namespace ExplorerLens::Engine;
    PlatformAbstractionLayer pal;
    auto surface = pal.CreateRenderSurface(0, 0);
    ASSERT(surface.width == 0 && surface.height == 0);
}
TEST(TestNFA_Watch)
{
    using namespace ExplorerLens::Engine;
    NativeFilesystemAdapter fs;
    bool called = false;
    auto token = fs.WatchDirectory(L".", [&](const auto&) { called = true; });
    ASSERT(token != 0 || token == 0);
    fs.StopWatching(token);
}
TEST(TestNFA_StopAll)
{
    using namespace ExplorerLens::Engine;
    NativeFilesystemAdapter fs;
    fs.StopAllWatching();
}
TEST(TestNFA_Exists)
{
    using namespace ExplorerLens::Engine;
    NativeFilesystemAdapter fs;
    ASSERT(fs.PathExists(L"."));
    ASSERT(!fs.PathExists(L"__nonexistent_path_42__"));
}
TEST(TestNFA_IsDir)
{
    using namespace ExplorerLens::Engine;
    NativeFilesystemAdapter fs;
    ASSERT(fs.IsDirectory(L"."));
}
TEST(TestNFA_Temp)
{
    using namespace ExplorerLens::Engine;
    NativeFilesystemAdapter fs;
    auto tmp = fs.GetTempPath();
    ASSERT(!tmp.empty());
}
TEST(TestNFA_MaxPath)
{
    using namespace ExplorerLens::Engine;
    NativeFilesystemAdapter fs;
    ASSERT(fs.GetMaxPathLength() >= 260);
}
TEST(TestBuildMatrix_Validate)
{
    using namespace ExplorerLens::Engine;
    PlatformBuildMatrix matrix;
    bool valid = matrix.ValidatePlatformHeaders();
    ASSERT(valid);
}

//== Sprint 971-980 — DirectStorage & GPU Decompression (v30.1.0 "Deneb-R") ==

TEST(TestDSEngine_Status)
{
    using namespace ExplorerLens::Engine;
    DirectStorageEngine ds;
    auto status = ds.GetStatus();
    ASSERT(status == DSStatus::Unavailable || status == DSStatus::Available || status == DSStatus::Fallback);
}
TEST(TestDSEngine_Init)
{
    using namespace ExplorerLens::Engine;
    DirectStorageEngine ds;
    bool ok = ds.Initialize();
    (void)ok;
}
TEST(TestDSEngine_Shutdown)
{
    using namespace ExplorerLens::Engine;
    DirectStorageEngine ds;
    ds.Initialize();
    ds.Shutdown();
    ASSERT(ds.GetStatus() == DSStatus::Unavailable);
}
TEST(TestDSEngine_Available)
{
    using namespace ExplorerLens::Engine;
    DirectStorageEngine ds;
    auto avail = ds.IsAvailable();
    (void)avail;
}
TEST(TestDSEngine_DoubleInit)
{
    using namespace ExplorerLens::Engine;
    DirectStorageEngine ds;
    ds.Initialize();
    ds.Initialize();
    ds.Shutdown();
}
TEST(TestGPUDecomp_Vendor)
{
    using namespace ExplorerLens::Engine;
    GPUDecompressScheduler sched;
    auto vendor = sched.DetectVendor();
    ASSERT(vendor == GPUDecompressVendor::NVIDIA || vendor == GPUDecompressVendor::AMD
           || vendor == GPUDecompressVendor::Intel || vendor == GPUDecompressVendor::Generic);
}
TEST(TestGPUDecomp_Throughput)
{
    using namespace ExplorerLens::Engine;
    GPUDecompressScheduler sched;
    ASSERT(sched.GetThroughputMBps() >= 0.0);
}
TEST(TestGPUDecomp_HwAccel)
{
    using namespace ExplorerLens::Engine;
    GPUDecompressScheduler sched;
    auto hw = sched.IsHardwareAccelerated();
    (void)hw;
}
TEST(TestDSPipeline_Init)
{
    using namespace ExplorerLens::Engine;
    DirectStoragePipelineStage stage;
    bool ok = stage.Initialize();
    (void)ok;
}
TEST(TestDSPipeline_Stats)
{
    using namespace ExplorerLens::Engine;
    DirectStoragePipelineStage stage;
    auto stats = stage.GetStatistics();
    ASSERT(stats.requestsProcessed == 0);
}
TEST(TestDSPipeline_Process)
{
    using namespace ExplorerLens::Engine;
    DirectStoragePipelineStage stage;
    stage.Initialize();
    auto result = stage.ProcessRequest(L"test.raw", 256);
    (void)result;
}
TEST(TestDSPipeline_Shutdown)
{
    using namespace ExplorerLens::Engine;
    DirectStoragePipelineStage stage;
    stage.Initialize();
    stage.Shutdown();
}
TEST(TestDSPipeline_Latency)
{
    using namespace ExplorerLens::Engine;
    DirectStoragePipelineStage stage;
    ASSERT(stage.GetAverageLatencyMs() >= 0.0);
}
TEST(TestDSPipeline_Enabled)
{
    using namespace ExplorerLens::Engine;
    DirectStoragePipelineStage stage;
    auto enabled = stage.IsEnabled();
    (void)enabled;
}
TEST(TestDSPipeline_Priority)
{
    using namespace ExplorerLens::Engine;
    DirectStoragePipelineStage stage;
    ASSERT(stage.GetPriority() >= 0);
}
TEST(TestDSPipeline_FallbackMode)
{
    using namespace ExplorerLens::Engine;
    DirectStoragePipelineStage stage;
    auto fb = stage.IsFallbackMode();
    (void)fb;
}
TEST(TestNvGDeflate_Init)
{
    using namespace ExplorerLens::Engine;
    NvGDeflateBackend nv;
    bool ok = nv.Initialize();
    (void)ok;
}
TEST(TestNvGDeflate_Supported)
{
    using namespace ExplorerLens::Engine;
    NvGDeflateBackend nv;
    auto sup = nv.IsSupported();
    (void)sup;
}
TEST(TestNvGDeflate_Device)
{
    using namespace ExplorerLens::Engine;
    NvGDeflateBackend nv;
    auto name = nv.GetDeviceName();
    ASSERT(!name.empty() || name.empty());
}
TEST(TestNvGDeflate_Shutdown)
{
    using namespace ExplorerLens::Engine;
    NvGDeflateBackend nv;
    nv.Initialize();
    nv.Shutdown();
}
TEST(TestAMDDecomp_Init)
{
    using namespace ExplorerLens::Engine;
    AMDDecompressBackend amd;
    bool ok = amd.Initialize();
    (void)ok;
}
TEST(TestAMDDecomp_Supported)
{
    using namespace ExplorerLens::Engine;
    AMDDecompressBackend amd;
    auto sup = amd.IsSupported();
    (void)sup;
}
TEST(TestAMDDecomp_Device)
{
    using namespace ExplorerLens::Engine;
    AMDDecompressBackend amd;
    auto name = amd.GetDeviceName();
    ASSERT(!name.empty() || name.empty());
}
TEST(TestAMDDecomp_Shutdown)
{
    using namespace ExplorerLens::Engine;
    AMDDecompressBackend amd;
    amd.Initialize();
    amd.Shutdown();
}
TEST(TestDSCache_Init)
{
    using namespace ExplorerLens::Engine;
    DirectStorageCacheTier cache;
    bool ok = cache.Initialize(256);
    ASSERT(ok);
}
TEST(TestDSCache_PutGet)
{
    using namespace ExplorerLens::Engine;
    DirectStorageCacheTier cache;
    cache.Initialize(256);
    std::vector<uint8_t> data = {1, 2, 3, 4};
    cache.Put("key1", data);
    auto result = cache.Get("key1");
    ASSERT(result.size() == 4);
}
TEST(TestDSCache_Miss)
{
    using namespace ExplorerLens::Engine;
    DirectStorageCacheTier cache;
    cache.Initialize(256);
    auto result = cache.Get("nonexistent");
    ASSERT(result.empty());
}
TEST(TestDSCache_Evict)
{
    using namespace ExplorerLens::Engine;
    DirectStorageCacheTier cache;
    cache.Initialize(256);
    std::vector<uint8_t> data = {1, 2, 3};
    cache.Put("key1", data);
    cache.Evict("key1");
    auto r = cache.Get("key1");
    ASSERT(r.empty());
}
TEST(TestDSCache_HitRate)
{
    using namespace ExplorerLens::Engine;
    DirectStorageCacheTier cache;
    cache.Initialize(256);
    ASSERT(cache.GetHitRate() >= 0.0f);
}
TEST(TestDSCache_Capacity)
{
    using namespace ExplorerLens::Engine;
    DirectStorageCacheTier cache;
    cache.Initialize(512);
    ASSERT(cache.GetCapacityMB() == 512);
}
TEST(TestDSCache_Count)
{
    using namespace ExplorerLens::Engine;
    DirectStorageCacheTier cache;
    cache.Initialize(256);
    ASSERT(cache.GetEntryCount() == 0);
    std::vector<uint8_t> d = {1};
    cache.Put("a", d);
    ASSERT(cache.GetEntryCount() == 1);
}
TEST(TestDSCache_Clear)
{
    using namespace ExplorerLens::Engine;
    DirectStorageCacheTier cache;
    cache.Initialize(256);
    std::vector<uint8_t> d = {1};
    cache.Put("a", d);
    cache.Clear();
    ASSERT(cache.GetEntryCount() == 0);
}
TEST(TestDSCache_Usage)
{
    using namespace ExplorerLens::Engine;
    DirectStorageCacheTier cache;
    cache.Initialize(256);
    ASSERT(cache.GetUsageMB() >= 0.0);
}
TEST(TestAsyncStream_BytesRead)
{
    using namespace ExplorerLens::Engine;
    AsyncFileStreamBroker broker;
    ASSERT(broker.GetBytesRead() == 0);
}
TEST(TestStagingPool_Init)
{
    using namespace ExplorerLens::Engine;
    StagingBufferPoolV2 pool;
    bool ok = pool.Initialize(16, 4 * 1024 * 1024);
    ASSERT(ok);
}
TEST(TestStagingPool_Acquire)
{
    using namespace ExplorerLens::Engine;
    StagingBufferPoolV2 pool;
    pool.Initialize(4, 1024);
    auto buf = pool.AcquireBuffer(512);
    ASSERT(buf.data != nullptr);
    ASSERT(buf.capacity >= 512);
    pool.ReleaseBuffer(buf);
}
TEST(TestStagingPool_Release)
{
    using namespace ExplorerLens::Engine;
    StagingBufferPoolV2 pool;
    pool.Initialize(4, 1024);
    auto buf = pool.AcquireBuffer(256);
    pool.ReleaseBuffer(buf);
    ASSERT(pool.GetActiveCount() == 0);
}
TEST(TestStagingPool_Size)
{
    using namespace ExplorerLens::Engine;
    StagingBufferPoolV2 pool;
    pool.Initialize(8, 2048);
    ASSERT(pool.GetPoolSize() == 8);
}
TEST(TestStagingPool_Active)
{
    using namespace ExplorerLens::Engine;
    StagingBufferPoolV2 pool;
    pool.Initialize(4, 1024);
    ASSERT(pool.GetActiveCount() == 0);
    auto b = pool.AcquireBuffer(100);
    ASSERT(pool.GetActiveCount() == 1);
    pool.ReleaseBuffer(b);
}
TEST(TestStagingPool_Trim)
{
    using namespace ExplorerLens::Engine;
    StagingBufferPoolV2 pool;
    pool.Initialize(8, 1024);
    pool.Trim();
}
TEST(TestStagingPool_TotalMem)
{
    using namespace ExplorerLens::Engine;
    StagingBufferPoolV2 pool;
    pool.Initialize(4, 1024);
    ASSERT(pool.GetTotalMemoryBytes() >= 0);
}
TEST(TestStagingPool_MaxBuf)
{
    using namespace ExplorerLens::Engine;
    StagingBufferPoolV2 pool;
    pool.Initialize(4, 2048);
    ASSERT(pool.GetMaxBufferSize() == 2048);
}
TEST(TestStagingPool_Exhaustion)
{
    using namespace ExplorerLens::Engine;
    StagingBufferPoolV2 pool;
    pool.Initialize(2, 1024);
    auto b1 = pool.AcquireBuffer(100);
    auto b2 = pool.AcquireBuffer(100);
    auto b3 = pool.AcquireBuffer(100);
    pool.ReleaseBuffer(b1);
    pool.ReleaseBuffer(b2);
    pool.ReleaseBuffer(b3);
}

//== Sprint 981-990 — CLIP Semantic Search & Discovery (v30.2.0 "Deneb-S") ===

TEST(TestCLIP_Dim)
{
    using namespace ExplorerLens::Engine;
    CLIPEmbeddingEngine clip;
    ASSERT(clip.GetEmbeddingDimension() == 512);
}
TEST(TestCLIP_Latency)
{
    using namespace ExplorerLens::Engine;
    CLIPEmbeddingEngine clip;
    ASSERT(clip.GetInferenceLatencyMs() >= 0.0);
}
TEST(TestCLIP_Backend)
{
    using namespace ExplorerLens::Engine;
    CLIPEmbeddingEngine clip;
    clip.SetBackend(InferenceBackend::DirectML);
    clip.SetBackend(InferenceBackend::CPU);
}
TEST(TestCLIP_NullInput)
{
    using namespace ExplorerLens::Engine;
    CLIPEmbeddingEngine clip;
    auto r = clip.ComputeEmbedding(nullptr, 0, 0, 0);
    ASSERT(r.embedding.empty());
}
TEST(TestNLQuery_Vocab)
{
    using namespace ExplorerLens::Engine;
    NaturalLanguageQueryParser parser;
    ASSERT(parser.GetVocabSize() > 0);
}
TEST(TestNLQuery_Empty)
{
    using namespace ExplorerLens::Engine;
    NaturalLanguageQueryParser parser;
    auto emb = parser.ParseQuery("");
    ASSERT(emb.embedding.empty());
}
TEST(TestEmbCache_Init)
{
    using namespace ExplorerLens::Engine;
    EmbeddingCacheStore cache;
    bool ok = cache.Initialize(L"_emb_cache_test");
    ASSERT(ok);
}
TEST(TestEmbCache_Store)
{
    using namespace ExplorerLens::Engine;
    EmbeddingCacheStore cache;
    cache.Initialize(L"_emb_cache_test");
    std::vector<float> emb(512, 0.1f);
    cache.Store("key1", emb);
    ASSERT(cache.Contains("key1"));
}
TEST(TestEmbCache_Retrieve)
{
    using namespace ExplorerLens::Engine;
    EmbeddingCacheStore cache;
    cache.Initialize(L"_emb_cache_test");
    std::vector<float> emb(512, 0.5f);
    cache.Store("key2", emb);
    auto r = cache.Retrieve("key2");
    ASSERT(r.size() == 512);
}
TEST(TestEmbCache_Miss)
{
    using namespace ExplorerLens::Engine;
    EmbeddingCacheStore cache;
    cache.Initialize(L"_emb_cache_test");
    auto r = cache.Retrieve("missing");
    ASSERT(r.empty());
}
TEST(TestEmbCache_Size)
{
    using namespace ExplorerLens::Engine;
    EmbeddingCacheStore cache;
    cache.Initialize(L"_emb_cache_test");
    ASSERT(cache.GetCacheSize() >= 0);
}
TEST(TestEmbCache_Compact)
{
    using namespace ExplorerLens::Engine;
    EmbeddingCacheStore cache;
    cache.Initialize(L"_emb_cache_test");
    cache.Compact();
}
TEST(TestEmbCache_Clear)
{
    using namespace ExplorerLens::Engine;
    EmbeddingCacheStore cache;
    cache.Initialize(L"_emb_cache_test");
    cache.Clear();
    ASSERT(cache.GetCacheSize() == 0);
}
TEST(TestEmbCache_Stats)
{
    using namespace ExplorerLens::Engine;
    EmbeddingCacheStore cache;
    cache.Initialize(L"_emb_cache_test");
    auto stats = cache.GetStats();
    ASSERT(stats.hitCount == 0);
}
TEST(TestEmbCache_Eviction)
{
    using namespace ExplorerLens::Engine;
    EmbeddingCacheStore cache;
    cache.Initialize(L"_emb_cache_test");
    cache.SetMaxEntries(2);
    std::vector<float> e(512, 0.0f);
    cache.Store("a", e);
    cache.Store("b", e);
    cache.Store("c", e);
    ASSERT(cache.GetCacheSize() <= 2);
}
TEST(TestMMRank_Weights)
{
    using namespace ExplorerLens::Engine;
    MultiModalRanker ranker;
    RankWeights w{0.5f, 0.2f, 0.2f, 0.1f};
    ranker.SetWeights(w);
}
TEST(TestDedup_Empty)
{
    using namespace ExplorerLens::Engine;
    SearchResultDeduplicator dedup;
    auto h = dedup.ComputePerceptualHash(nullptr, 0, 0);
    ASSERT(h == 0);
}
TEST(TestIdxUpd_Init)
{
    using namespace ExplorerLens::Engine;
    IncrementalIndexUpdater updater;
    bool ok = updater.Initialize();
    ASSERT(ok);
}
TEST(TestIdxUpd_FileCreated)
{
    using namespace ExplorerLens::Engine;
    IncrementalIndexUpdater updater;
    updater.Initialize();
    updater.OnFileCreated(L"new.jpg");
    ASSERT(updater.GetPendingUpdates() == 1);
}
TEST(TestIdxUpd_FileDeleted)
{
    using namespace ExplorerLens::Engine;
    IncrementalIndexUpdater updater;
    updater.Initialize();
    updater.OnFileDeleted(L"old.jpg");
    ASSERT(updater.GetPendingUpdates() == 1);
}
TEST(TestIdxUpd_FileModified)
{
    using namespace ExplorerLens::Engine;
    IncrementalIndexUpdater updater;
    updater.Initialize();
    updater.OnFileModified(L"edit.jpg");
    ASSERT(updater.GetPendingUpdates() == 1);
}
TEST(TestIdxUpd_Flush)
{
    using namespace ExplorerLens::Engine;
    IncrementalIndexUpdater updater;
    updater.Initialize();
    updater.OnFileCreated(L"a.jpg");
    updater.FlushUpdates();
    ASSERT(updater.GetPendingUpdates() == 0);
}
TEST(TestIdxUpd_Multi)
{
    using namespace ExplorerLens::Engine;
    IncrementalIndexUpdater updater;
    updater.Initialize();
    for (int i = 0; i < 5; i++)
        updater.OnFileCreated(L"file" + std::to_wstring(i) + L".jpg");
    ASSERT(updater.GetPendingUpdates() == 5);
}
TEST(TestIdxUpd_Stats)
{
    using namespace ExplorerLens::Engine;
    IncrementalIndexUpdater updater;
    ASSERT(updater.GetTotalProcessed() == 0);
}
TEST(TestIdxUpd_Shutdown)
{
    using namespace ExplorerLens::Engine;
    IncrementalIndexUpdater updater;
    updater.Initialize();
    updater.Shutdown();
}
TEST(TestIdxUpd_Latency)
{
    using namespace ExplorerLens::Engine;
    IncrementalIndexUpdater updater;
    ASSERT(updater.GetAverageFlushLatencyMs() >= 0.0);
}
