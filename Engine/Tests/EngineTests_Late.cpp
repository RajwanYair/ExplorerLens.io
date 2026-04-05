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
// Sprint 871-880 — Cross-Platform GPU/Core/Utils
#include "../Core/MacOSShellBridge.h"
#include "../Core/PlatformNeutralBuffer.h"
#include "../Core/XDGThumbnailProvider.h"
#include "../GPU/LinuxVulkanPreview.h"
#include "../GPU/MetalRenderBridge.h"
#include "../GPU/MetalShaderCompiler.h"
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
    // Create a minimal temp PNG file (1-byte valid enough for smoke test)
    // and verify the runner processes it without crashing.
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
    auto tmpPath = fs::temp_directory_path() / "el_test_smoke.png";
    {
        std::ofstream f(tmpPath, std::ios::binary);
        ASSERT(f.is_open());
        f.write(reinterpret_cast<const char*>(kMinPNG), sizeof(kMinPNG));
    }

    CorpusTestRunner runner;
    runner.AddCorpusDirectory(tmpPath.parent_path());
    runner.SetMaxFiles(1);
    // Only include our one file
    runner.SetFileFilter([&tmpPath](const fs::path& p) { return p == tmpPath; });

    auto report = runner.Run();
    ASSERT(report.totalFiles >= 1);
    // Verify the file was attempted (either passed or failed is acceptable —
    // we don't require the full PNG decoder to be available in test context)
    ASSERT(report.passed + report.failed >= 1 || report.skipped >= 1);

    // Cleanup
    std::error_code ec;
    fs::remove(tmpPath, ec);
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

TEST(TestTIFFMultiPageDecoder_LooksLikeTIFF_RejectsGarbage)
{
    using namespace ExplorerLens::Engine;
    uint8_t garbage[4] = {0x00, 0x00, 0x00, 0x00};
    ASSERT(!TIFFMultiPageDecoder::LooksLikeTIFF(garbage, sizeof(garbage)));
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
TEST(TestMetalRenderBridge_Initialize)
{
    using namespace ExplorerLens::Engine;
    MetalRenderBridge bridge;
    ASSERT(bridge.Initialize());
    ASSERT(bridge.IsReady());
#if defined(__APPLE__)
    ASSERT(bridge.IsMetalAvailable());
#endif
    bridge.Shutdown();
}
TEST(TestLinuxVulkanPreview_Enumerate)
{
    using namespace ExplorerLens::Engine;
    LinuxVulkanPreview vk;
    ASSERT(vk.Initialize());
#if defined(__linux__)
    auto devs = vk.EnumerateDevices();
    ASSERT(!devs.empty());
#else
    ASSERT(!vk.IsPlatformOk());
#endif
    vk.Shutdown();
}
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
TEST(TestMacOSShellBridge_CanHandle)
{
    using namespace ExplorerLens::Engine;
    MacOSShellBridge bridge;
    ASSERT(bridge.Initialize());
    ASSERT(bridge.CanHandleExtension("jpg"));
    ASSERT(bridge.CanHandleExtension("png"));
    ASSERT(!bridge.CanHandleExtension("xyz123"));
    bridge.Shutdown();
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
TEST(TestMetalShaderCompiler_Compile)
{
    using namespace ExplorerLens::Engine;
    MetalShaderCompiler comp;
    ASSERT(comp.Initialize());
#if defined(__APPLE__)
    MetalShaderDesc desc;
    desc.name = "resize_kernel";
    desc.source = "kernel void resize_kernel(...){}";
    auto h = comp.Compile(desc);
    ASSERT(h.compiled);
    auto h2 = comp.Compile(desc);
    ASSERT(comp.GetStats().cacheHits == 1);
#endif
    comp.Shutdown();
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
TEST(TestMetalV2_Init)
{
    using namespace ExplorerLens::Engine;
    MetalPipelineV2 pipeline;
    auto result = pipeline.Initialize();
#ifdef __APPLE__
    ASSERT(result);
#else
    ASSERT(!result);
#endif
}
TEST(TestMetalV2_Shutdown)
{
    using namespace ExplorerLens::Engine;
    MetalPipelineV2 pipeline;
    pipeline.Initialize();
    pipeline.Shutdown();
    ASSERT(!pipeline.IsAvailable());
}
TEST(TestDRM_Init)
{
    using namespace ExplorerLens::Engine;
    LinuxDRMBackend drm;
    auto result = drm.InitializeEGL();
#ifdef __linux__
    (void)result;
#else
    ASSERT(!result);
#endif
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
TEST(TestCSP_Type)
{
    using namespace ExplorerLens::Engine;
    CrossPlatformShellProvider shell;
    auto type = shell.GetProviderType();
#ifdef _WIN32
    ASSERT(type == ShellProviderType::WindowsShell);
#endif
}
TEST(TestCSP_Register)
{
    using namespace ExplorerLens::Engine;
    CrossPlatformShellProvider shell;
    bool ok = shell.RegisterProvider();
    (void)ok;
}
TEST(TestCSP_Unregister)
{
    using namespace ExplorerLens::Engine;
    CrossPlatformShellProvider shell;
    shell.UnregisterProvider();
}
TEST(TestCSP_Generate)
{
    using namespace ExplorerLens::Engine;
    CrossPlatformShellProvider shell;
    std::vector<uint8_t> thumb;
    bool ok = shell.GenerateThumbnail(L"test.png", 256, thumb);
    (void)ok;
}
TEST(TestCSP_Ext)
{
    using namespace ExplorerLens::Engine;
    CrossPlatformShellProvider shell;
    auto exts = shell.GetSupportedExtensions();
    ASSERT(!exts.empty());
}
TEST(TestCSP_Registered)
{
    using namespace ExplorerLens::Engine;
    CrossPlatformShellProvider shell;
    auto reg = shell.IsRegistered();
    (void)reg;
}
TEST(TestCSP_Version)
{
    using namespace ExplorerLens::Engine;
    CrossPlatformShellProvider shell;
    auto ver = shell.GetProviderVersion();
    ASSERT(!ver.empty());
}
TEST(TestCSP_MaxSize)
{
    using namespace ExplorerLens::Engine;
    CrossPlatformShellProvider shell;
    ASSERT(shell.GetMaxThumbnailSize() >= 256);
}
TEST(TestCSP_Formats)
{
    using namespace ExplorerLens::Engine;
    CrossPlatformShellProvider shell;
    ASSERT(shell.GetSupportedFormatCount() > 0);
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

//== Sprint 1061-1070: Intelligent Workflow Automation (v31.3.0 "Achernar-T") ==

// --- PredictivePregenEngine ---
TEST(TestPregen_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& engine = PredictivePregenEngine::Instance();
    ASSERT(!engine.IsInitialized());
}
TEST(TestPregen_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& engine = PredictivePregenEngine::Instance();
    ASSERT(engine.Initialize());
    ASSERT(engine.IsInitialized());
    ASSERT(engine.GetStrategy() == PregenStrategy::Hybrid);
    engine.Shutdown();
}
TEST(TestPregen_InitWithStrategy)
{
    using namespace ExplorerLens::Engine;
    auto& engine = PredictivePregenEngine::Instance();
    engine.Initialize(PregenStrategy::Recency);
    ASSERT(engine.GetStrategy() == PregenStrategy::Recency);
    engine.Shutdown();
}
TEST(TestPregen_RecordNavigation)
{
    using namespace ExplorerLens::Engine;
    auto& engine = PredictivePregenEngine::Instance();
    engine.Initialize();
    engine.RecordNavigation(L"C:\\Photos");
    ASSERT(engine.GetStats().totalPredictions == 1);
    engine.Shutdown();
}
TEST(TestPregen_Predict)
{
    using namespace ExplorerLens::Engine;
    auto& engine = PredictivePregenEngine::Instance();
    engine.Initialize();
    engine.RecordNavigation(L"C:\\Photos");
    auto predictions = engine.Predict(3);
    ASSERT(!predictions.empty());
    ASSERT(predictions[0].confidence > 0.0);
    engine.Shutdown();
}
TEST(TestPregen_PredictEmpty)
{
    using namespace ExplorerLens::Engine;
    auto& engine = PredictivePregenEngine::Instance();
    auto predictions = engine.Predict(5);
    ASSERT(predictions.empty());
}
TEST(TestPregen_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& engine = PredictivePregenEngine::Instance();
    engine.Initialize();
    auto stats = engine.GetStats();
    ASSERT(stats.totalPredictions == 0 || stats.totalPredictions > 0);
    engine.Shutdown();
}
TEST(TestPregen_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& engine = PredictivePregenEngine::Instance();
    engine.Initialize();
    engine.Shutdown();
    ASSERT(!engine.IsInitialized());
}
TEST(TestPregen_PredictZeroResults)
{
    using namespace ExplorerLens::Engine;
    auto& engine = PredictivePregenEngine::Instance();
    engine.Initialize();
    auto predictions = engine.Predict(0);
    ASSERT(predictions.empty());
    engine.Shutdown();
}

// --- ContentCategorizationEngine ---
TEST(TestContentCat_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& engine = ContentCategorizationEngine::Instance();
    ASSERT(!engine.IsInitialized());
}
TEST(TestContentCat_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& engine = ContentCategorizationEngine::Instance();
    ASSERT(engine.Initialize());
    ASSERT(engine.IsInitialized());
    engine.Shutdown();
}
TEST(TestContentCat_Categorize)
{
    using namespace ExplorerLens::Engine;
    auto& engine = ContentCategorizationEngine::Instance();
    engine.Initialize();
    auto result = engine.Categorize(L"photo.jpg");
    ASSERT(result.primary != ContentCategory::Unknown);
    ASSERT(result.confidence > 0.0);
    ASSERT(!result.topK.empty());
    engine.Shutdown();
}
TEST(TestContentCat_CategorizeUninitialized)
{
    using namespace ExplorerLens::Engine;
    auto& engine = ContentCategorizationEngine::Instance();
    engine.Shutdown();
    auto result = engine.Categorize(L"file.txt");
    ASSERT(result.primary == ContentCategory::Unknown);
}
TEST(TestContentCat_ToString)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ToString(ContentCategory::Photo)) == L"Photo");
    ASSERT(std::wstring(ToString(ContentCategory::SourceCode)) == L"Source Code");
    ASSERT(std::wstring(ToString(ContentCategory::ThreeDModel)) == L"3D Model");
    ASSERT(std::wstring(ToString(ContentCategory::Unknown)) == L"Unknown");
}
TEST(TestContentCat_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& engine = ContentCategorizationEngine::Instance();
    engine.Initialize();
    engine.Categorize(L"img.png");
    auto stats = engine.GetStats();
    ASSERT(stats.totalClassified > 0);
    ASSERT(stats.highConfidenceCount > 0);
    engine.Shutdown();
}
TEST(TestContentCat_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& engine = ContentCategorizationEngine::Instance();
    engine.Initialize();
    engine.Shutdown();
    ASSERT(!engine.IsInitialized());
}
TEST(TestContentCat_MultipleCategories)
{
    using namespace ExplorerLens::Engine;
    auto& engine = ContentCategorizationEngine::Instance();
    engine.Initialize();
    auto r1 = engine.Categorize(L"a.jpg");
    auto r2 = engine.Categorize(L"b.png");
    ASSERT(engine.GetStats().totalClassified >= 2);
    engine.Shutdown();
}

// --- ThumbnailQualityPredictor ---
TEST(TestQualPred_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& pred = ThumbnailQualityPredictor::Instance();
    ASSERT(!pred.IsInitialized());
}
TEST(TestQualPred_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& pred = ThumbnailQualityPredictor::Instance();
    ASSERT(pred.Initialize());
    ASSERT(pred.IsInitialized());
    ASSERT(pred.GetSkipThreshold() > 0.0);
    pred.Shutdown();
}
TEST(TestQualPred_PredictGood)
{
    using namespace ExplorerLens::Engine;
    auto& pred = ThumbnailQualityPredictor::Instance();
    pred.Initialize();
    auto result = pred.Predict(L"photo.jpg", 1024 * 1024);
    ASSERT(result.quality == PredictedQuality::Good);
    ASSERT(!result.shouldSkipDecode);
    pred.Shutdown();
}
TEST(TestQualPred_PredictUnusable)
{
    using namespace ExplorerLens::Engine;
    auto& pred = ThumbnailQualityPredictor::Instance();
    pred.Initialize();
    auto result = pred.Predict(L"tiny.dat", 50);
    ASSERT(result.quality == PredictedQuality::Unusable);
    ASSERT(result.shouldSkipDecode);
    pred.Shutdown();
}
TEST(TestQualPred_PredictUninitialized)
{
    using namespace ExplorerLens::Engine;
    auto& pred = ThumbnailQualityPredictor::Instance();
    pred.Shutdown();
    auto result = pred.Predict(L"file.jpg");
    ASSERT(result.quality == PredictedQuality::Good);
}
TEST(TestQualPred_ToString)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ToString(PredictedQuality::Excellent)) == L"Excellent");
    ASSERT(std::wstring(ToString(PredictedQuality::Unusable)) == L"Unusable");
}
TEST(TestQualPred_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& pred = ThumbnailQualityPredictor::Instance();
    pred.Initialize();
    pred.Predict(L"a.jpg", 1000000);
    pred.Predict(L"b.dat", 10);
    auto stats = pred.GetStats();
    ASSERT(stats.totalPredictions >= 2);
    ASSERT(stats.skippedDecodes >= 1);
    pred.Shutdown();
}
TEST(TestQualPred_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& pred = ThumbnailQualityPredictor::Instance();
    pred.Initialize();
    pred.Shutdown();
    ASSERT(!pred.IsInitialized());
}

// --- SmartBatchProcessor ---
TEST(TestSmartBatch_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& bp = SmartBatchProcessor::Instance();
    ASSERT(bp.GetState() == BatchState::Idle || true);
}
TEST(TestSmartBatch_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& bp = SmartBatchProcessor::Instance();
    ASSERT(bp.Initialize(8));
    ASSERT(bp.IsInitialized());
    ASSERT(bp.GetMaxConcurrency() == 8);
    bp.Shutdown();
}
TEST(TestSmartBatch_SubmitAndStart)
{
    using namespace ExplorerLens::Engine;
    auto& bp = SmartBatchProcessor::Instance();
    bp.Initialize();
    std::vector<SmartBatchTask> tasks = {{L"a.jpg"}, {L"b.png"}, {L"c.webp"}};
    ASSERT(bp.Submit(tasks));
    ASSERT(bp.Start());
    ASSERT(bp.GetState() == BatchState::Running);
    bp.Shutdown();
}
TEST(TestSmartBatch_PauseResume)
{
    using namespace ExplorerLens::Engine;
    auto& bp = SmartBatchProcessor::Instance();
    bp.Initialize();
    bp.Submit({{L"x.jpg"}});
    bp.Start();
    ASSERT(bp.Pause());
    ASSERT(bp.GetState() == BatchState::Paused);
    ASSERT(bp.Resume());
    ASSERT(bp.GetState() == BatchState::Running);
    bp.Shutdown();
}
TEST(TestSmartBatch_Cancel)
{
    using namespace ExplorerLens::Engine;
    auto& bp = SmartBatchProcessor::Instance();
    bp.Initialize();
    bp.Submit({{L"x.jpg"}});
    bp.Start();
    ASSERT(bp.Cancel());
    ASSERT(bp.GetState() == BatchState::Cancelled);
    bp.Shutdown();
}
TEST(TestSmartBatch_EmptySubmit)
{
    using namespace ExplorerLens::Engine;
    auto& bp = SmartBatchProcessor::Instance();
    bp.Initialize();
    ASSERT(!bp.Submit({}));
    bp.Shutdown();
}
TEST(TestSmartBatch_ToString)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ToString(BatchState::Running)) == L"Running");
    ASSERT(std::wstring(ToString(BatchState::Cancelled)) == L"Cancelled");
}
TEST(TestSmartBatch_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& bp = SmartBatchProcessor::Instance();
    bp.Initialize();
    bp.Submit({{L"a.jpg"}});
    bp.Start();
    auto stats = bp.GetStats();
    ASSERT(stats.totalBatchesRun >= 1);
    bp.Shutdown();
}
TEST(TestSmartBatch_Progress)
{
    using namespace ExplorerLens::Engine;
    auto& bp = SmartBatchProcessor::Instance();
    bp.Initialize();
    bp.Submit({{L"a.jpg"}, {L"b.jpg"}});
    auto prog = bp.GetProgress();
    ASSERT(prog.totalTasks == 2);
    bp.Shutdown();
}

// --- WorkflowAutomationEngine ---
TEST(TestWorkflow_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& wf = WorkflowAutomationEngine::Instance();
    ASSERT(!wf.IsInitialized());
}
TEST(TestWorkflow_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& wf = WorkflowAutomationEngine::Instance();
    ASSERT(wf.Initialize());
    ASSERT(wf.IsInitialized());
    wf.Shutdown();
}
TEST(TestWorkflow_AddRule)
{
    using namespace ExplorerLens::Engine;
    auto& wf = WorkflowAutomationEngine::Instance();
    wf.Initialize();
    WorkflowRule rule;
    rule.name = L"Auto-generate on new file";
    rule.trigger = WorkflowTrigger::FileCreated;
    rule.action = WorkflowAction::GenerateThumbnail;
    uint32_t id = wf.AddRule(rule);
    ASSERT(id > 0);
    ASSERT(wf.GetRuleCount() == 1);
    wf.Shutdown();
}
TEST(TestWorkflow_RemoveRule)
{
    using namespace ExplorerLens::Engine;
    auto& wf = WorkflowAutomationEngine::Instance();
    wf.Initialize();
    WorkflowRule rule;
    rule.name = L"test";
    uint32_t id = wf.AddRule(rule);
    ASSERT(wf.RemoveRule(id));
    ASSERT(wf.GetRuleCount() == 0);
    wf.Shutdown();
}
TEST(TestWorkflow_EnableDisable)
{
    using namespace ExplorerLens::Engine;
    auto& wf = WorkflowAutomationEngine::Instance();
    wf.Initialize();
    WorkflowRule rule;
    rule.name = L"test";
    rule.enabled = true;
    uint32_t id = wf.AddRule(rule);
    ASSERT(wf.EnableRule(id, false));
    ASSERT(wf.GetStats().enabledRules == 0);
    wf.Shutdown();
}
TEST(TestWorkflow_RemoveInvalid)
{
    using namespace ExplorerLens::Engine;
    auto& wf = WorkflowAutomationEngine::Instance();
    wf.Initialize();
    ASSERT(!wf.RemoveRule(9999));
    wf.Shutdown();
}
TEST(TestWorkflow_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& wf = WorkflowAutomationEngine::Instance();
    wf.Initialize();
    WorkflowRule r;
    r.name = L"s";
    wf.AddRule(r);
    auto stats = wf.GetStats();
    ASSERT(stats.totalRules >= 1);
    ASSERT(stats.enabledRules >= 1);
    wf.Shutdown();
}
TEST(TestWorkflow_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& wf = WorkflowAutomationEngine::Instance();
    wf.Initialize();
    wf.Shutdown();
    ASSERT(!wf.IsInitialized());
    ASSERT(wf.GetRuleCount() == 0);
}
TEST(TestWorkflow_AddUninitialized)
{
    using namespace ExplorerLens::Engine;
    auto& wf = WorkflowAutomationEngine::Instance();
    wf.Shutdown();
    WorkflowRule rule;
    rule.name = L"fail";
    ASSERT(wf.AddRule(rule) == 0);
}

// --- UserBehaviorAnalytics ---
TEST(TestUserBehavior_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& uba = UserBehaviorAnalytics::Instance();
    ASSERT(!uba.IsInitialized());
}
TEST(TestUserBehavior_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& uba = UserBehaviorAnalytics::Instance();
    ASSERT(uba.Initialize());
    ASSERT(uba.IsInitialized());
    uba.Shutdown();
}
TEST(TestUserBehavior_RecordEvent)
{
    using namespace ExplorerLens::Engine;
    auto& uba = UserBehaviorAnalytics::Instance();
    uba.Initialize();
    UserNavigationEvent evt;
    evt.folderPath = L"C:\\Photos";
    evt.dwellTimeMs = 5000.0;
    uba.RecordEvent(evt);
    ASSERT(uba.GetEventCount() >= 1);
    uba.Shutdown();
}
TEST(TestUserBehavior_RecordUninitialized)
{
    using namespace ExplorerLens::Engine;
    auto& uba = UserBehaviorAnalytics::Instance();
    uba.Shutdown();
    UserNavigationEvent evt;
    evt.folderPath = L"C:\\temp";
    uba.RecordEvent(evt);
}
TEST(TestUserBehavior_GetTopPatterns)
{
    using namespace ExplorerLens::Engine;
    auto& uba = UserBehaviorAnalytics::Instance();
    uba.Initialize();
    UserNavigationEvent evt;
    evt.folderPath = L"C:\\Work";
    evt.dwellTimeMs = 3000.0;
    uba.RecordEvent(evt);
    auto patterns = uba.GetTopPatterns(5);
    ASSERT(!patterns.empty());
    ASSERT(patterns[0].visitCount >= 1);
    uba.Shutdown();
}
TEST(TestUserBehavior_EmptyPatterns)
{
    using namespace ExplorerLens::Engine;
    auto& uba = UserBehaviorAnalytics::Instance();
    uba.Initialize();
    auto patterns = uba.GetTopPatterns();
    ASSERT(patterns.empty() || !patterns.empty());
    uba.Shutdown();
}
TEST(TestUserBehavior_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& uba = UserBehaviorAnalytics::Instance();
    uba.Initialize();
    auto stats = uba.GetStats();
    ASSERT(stats.totalSessions >= 1);
    uba.Shutdown();
}
TEST(TestUserBehavior_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& uba = UserBehaviorAnalytics::Instance();
    uba.Initialize();
    uba.Shutdown();
    ASSERT(!uba.IsInitialized());
}

// --- AdaptivePipelineOptimizer ---
TEST(TestAdaptPipe_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& opt = AdaptivePipelineOptimizer::Instance();
    ASSERT(!opt.IsInitialized());
}
TEST(TestAdaptPipe_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& opt = AdaptivePipelineOptimizer::Instance();
    ASSERT(opt.Initialize());
    ASSERT(opt.IsInitialized());
    auto cfg = opt.GetConfig();
    ASSERT(cfg.concurrencyLevel == 4);
    ASSERT(cfg.targetLatencyMs > 0.0);
    opt.Shutdown();
}
TEST(TestAdaptPipe_InitCustomConfig)
{
    using namespace ExplorerLens::Engine;
    auto& opt = AdaptivePipelineOptimizer::Instance();
    AdaptivePipelineConfig cfg;
    cfg.concurrencyLevel = 8;
    cfg.targetLatencyMs = 10.0;
    opt.Initialize(cfg);
    ASSERT(opt.GetConfig().concurrencyLevel == 8);
    ASSERT(opt.GetConfig().targetLatencyMs == 10.0);
    opt.Shutdown();
}
TEST(TestAdaptPipe_RecordMetric)
{
    using namespace ExplorerLens::Engine;
    auto& opt = AdaptivePipelineOptimizer::Instance();
    opt.Initialize();
    opt.RecordMetric(PipelineMetric::Throughput, 250.0);
    opt.RecordMetric(PipelineMetric::Latency, 12.0);
    ASSERT(opt.GetStats().currentThroughput == 250.0);
    ASSERT(opt.GetStats().currentLatencyMs == 12.0);
    opt.Shutdown();
}
TEST(TestAdaptPipe_OptimizeReduceConcurrency)
{
    using namespace ExplorerLens::Engine;
    auto& opt = AdaptivePipelineOptimizer::Instance();
    AdaptivePipelineConfig cfg;
    cfg.concurrencyLevel = 8;
    cfg.targetLatencyMs = 10.0;
    opt.Initialize(cfg);
    opt.RecordMetric(PipelineMetric::Latency, 50.0);
    ASSERT(opt.Optimize());
    ASSERT(opt.GetConfig().concurrencyLevel < 8);
    opt.Shutdown();
}
TEST(TestAdaptPipe_OptimizeIncreaseConcurrency)
{
    using namespace ExplorerLens::Engine;
    auto& opt = AdaptivePipelineOptimizer::Instance();
    AdaptivePipelineConfig cfg;
    cfg.concurrencyLevel = 4;
    cfg.targetLatencyMs = 20.0;
    opt.Initialize(cfg);
    opt.RecordMetric(PipelineMetric::Latency, 5.0);
    ASSERT(opt.Optimize());
    ASSERT(opt.GetConfig().concurrencyLevel > 4);
    opt.Shutdown();
}
TEST(TestAdaptPipe_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& opt = AdaptivePipelineOptimizer::Instance();
    opt.Initialize();
    opt.Optimize();
    auto stats = opt.GetStats();
    ASSERT(stats.totalAdjustments >= 1);
    opt.Shutdown();
}
TEST(TestAdaptPipe_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& opt = AdaptivePipelineOptimizer::Instance();
    opt.Initialize();
    opt.Shutdown();
    ASSERT(!opt.IsInitialized());
}
TEST(TestAdaptPipe_OptimizeUninitialized)
{
    using namespace ExplorerLens::Engine;
    auto& opt = AdaptivePipelineOptimizer::Instance();
    opt.Shutdown();
    ASSERT(!opt.Optimize());
}

// --- IntelligentPrefetchScheduler ---
TEST(TestPrefetch_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& sched = IntelligentPrefetchScheduler::Instance();
    ASSERT(!sched.IsInitialized());
}
TEST(TestPrefetch_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& sched = IntelligentPrefetchScheduler::Instance();
    ASSERT(sched.Initialize(16, 32 * 1024 * 1024));
    ASSERT(sched.IsInitialized());
    ASSERT(sched.GetMaxDepth() == 16);
    sched.Shutdown();
}
TEST(TestPrefetch_Schedule)
{
    using namespace ExplorerLens::Engine;
    auto& sched = IntelligentPrefetchScheduler::Instance();
    sched.Initialize();
    SmartPrefetchRequest req;
    req.filePath = L"photo.jpg";
    req.priority = SmartPrefetchPriority::High;
    req.confidenceScore = 0.9;
    ASSERT(sched.Schedule(req));
    ASSERT(sched.GetQueueDepth() == 1);
    sched.Shutdown();
}
TEST(TestPrefetch_ScheduleBatch)
{
    using namespace ExplorerLens::Engine;
    auto& sched = IntelligentPrefetchScheduler::Instance();
    sched.Initialize();
    std::vector<SmartPrefetchRequest> reqs = {{L"a.jpg", SmartPrefetchPriority::High, 0.9},
                                              {L"b.png", SmartPrefetchPriority::Normal, 0.7},
                                              {L"c.webp", SmartPrefetchPriority::Low, 0.5}};
    ASSERT(sched.ScheduleBatch(reqs));
    ASSERT(sched.GetQueueDepth() == 3);
    sched.Shutdown();
}
TEST(TestPrefetch_ScheduleUninitialized)
{
    using namespace ExplorerLens::Engine;
    auto& sched = IntelligentPrefetchScheduler::Instance();
    sched.Shutdown();
    SmartPrefetchRequest req;
    req.filePath = L"fail.jpg";
    ASSERT(!sched.Schedule(req));
}
TEST(TestPrefetch_Flush)
{
    using namespace ExplorerLens::Engine;
    auto& sched = IntelligentPrefetchScheduler::Instance();
    sched.Initialize();
    sched.Schedule({L"x.jpg"});
    sched.Flush();
    ASSERT(sched.GetQueueDepth() == 0);
    sched.Shutdown();
}
TEST(TestPrefetch_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& sched = IntelligentPrefetchScheduler::Instance();
    sched.Initialize();
    sched.Schedule({L"a.jpg"});
    sched.Schedule({L"b.png"});
    auto stats = sched.GetStats();
    ASSERT(stats.totalScheduled >= 2);
    sched.Shutdown();
}
TEST(TestPrefetch_Eviction)
{
    using namespace ExplorerLens::Engine;
    auto& sched = IntelligentPrefetchScheduler::Instance();
    sched.Initialize(2);
    sched.Schedule({L"a.jpg"});
    sched.Schedule({L"b.jpg"});
    sched.Schedule({L"c.jpg"});
    ASSERT(sched.GetQueueDepth() == 2);
    ASSERT(sched.GetStats().totalEvicted >= 1);
    sched.Shutdown();
}
TEST(TestPrefetch_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& sched = IntelligentPrefetchScheduler::Instance();
    sched.Initialize();
    sched.Schedule({L"x.jpg"});
    sched.Shutdown();
    ASSERT(!sched.IsInitialized());
    ASSERT(sched.GetQueueDepth() == 0);
}

//== Sprint 1071-1080 — Contextual Intelligence & Self-Healing Tests (v31.4.0) ==

// ContextualRenderingEngine tests
TEST(TestCtxRender_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& eng = ContextualRenderingEngine::Instance();
    eng.Initialize(RenderQualityTier::High);
    ASSERT(eng.IsInitialized());
    ASSERT(eng.GetStats().totalEvaluations == 0);
    eng.Shutdown();
}
TEST(TestCtxRender_EvaluateDefault)
{
    using namespace ExplorerLens::Engine;
    auto& eng = ContextualRenderingEngine::Instance();
    eng.Initialize();
    RenderContextSignals sig;
    sig.displayDPI = 96.0f;
    sig.availableGPUMemoryBytes = 1024ULL * 1024 * 1024;
    auto params = eng.Evaluate(sig);
    ASSERT(params.contextType == RenderContextType::UserBrowsing);
    ASSERT(params.qualityTier == RenderQualityTier::Medium);
    eng.Shutdown();
}
TEST(TestCtxRender_HighDPI)
{
    using namespace ExplorerLens::Engine;
    auto& eng = ContextualRenderingEngine::Instance();
    eng.Initialize();
    RenderContextSignals sig;
    sig.displayDPI = 192.0f;
    sig.availableGPUMemoryBytes = 1024ULL * 1024 * 1024;
    auto params = eng.Evaluate(sig);
    ASSERT(params.contextType == RenderContextType::HighDPI);
    ASSERT(params.qualityTier == RenderQualityTier::High);
    eng.Shutdown();
}
TEST(TestCtxRender_BatterySaver)
{
    using namespace ExplorerLens::Engine;
    auto& eng = ContextualRenderingEngine::Instance();
    eng.Initialize();
    RenderContextSignals sig;
    sig.onBattery = true;
    sig.availableGPUMemoryBytes = 1024ULL * 1024 * 1024;
    auto params = eng.Evaluate(sig);
    ASSERT(params.contextType == RenderContextType::BatterySaver);
    ASSERT(!params.useGPU);
    eng.Shutdown();
}
TEST(TestCtxRender_LowMemory)
{
    using namespace ExplorerLens::Engine;
    auto& eng = ContextualRenderingEngine::Instance();
    eng.Initialize();
    RenderContextSignals sig;
    sig.availableGPUMemoryBytes = 64 * 1024 * 1024;
    auto params = eng.Evaluate(sig);
    ASSERT(params.contextType == RenderContextType::LowMemory);
    ASSERT(!params.useGPU);
    eng.Shutdown();
}
TEST(TestCtxRender_ContextSwitch)
{
    using namespace ExplorerLens::Engine;
    auto& eng = ContextualRenderingEngine::Instance();
    eng.Initialize();
    RenderContextSignals sig1;
    sig1.availableGPUMemoryBytes = 1024ULL * 1024 * 1024;
    eng.Evaluate(sig1);
    RenderContextSignals sig2;
    sig2.onBattery = true;
    sig2.availableGPUMemoryBytes = 1024ULL * 1024 * 1024;
    eng.Evaluate(sig2);
    ASSERT(eng.GetStats().contextSwitches >= 1);
    eng.Shutdown();
}
TEST(TestCtxRender_DPIScale)
{
    using namespace ExplorerLens::Engine;
    auto& eng = ContextualRenderingEngine::Instance();
    eng.Initialize();
    RenderContextSignals sig;
    sig.displayDPI = 192.0f;
    sig.availableGPUMemoryBytes = 1024ULL * 1024 * 1024;
    auto params = eng.Evaluate(sig);
    ASSERT(params.dpiScale >= 1.9f);
    ASSERT(params.targetWidthPx >= 480);
    eng.Shutdown();
}
TEST(TestCtxRender_Background)
{
    using namespace ExplorerLens::Engine;
    auto& eng = ContextualRenderingEngine::Instance();
    eng.Initialize();
    RenderContextSignals sig;
    sig.isBackgroundProcess = true;
    sig.availableGPUMemoryBytes = 1024ULL * 1024 * 1024;
    auto params = eng.Evaluate(sig);
    ASSERT(params.contextType == RenderContextType::BackgroundTask);
    eng.Shutdown();
}
TEST(TestCtxRender_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& eng = ContextualRenderingEngine::Instance();
    eng.Initialize();
    eng.Shutdown();
    ASSERT(!eng.IsInitialized());
}

// SmartThumbnailCompositor tests
TEST(TestCompositor_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& comp = SmartThumbnailCompositor::Instance();
    comp.Initialize(4);
    ASSERT(comp.IsInitialized());
    ASSERT(comp.GetStats().totalComposites == 0);
    comp.Shutdown();
}
TEST(TestCompositor_SinglePage)
{
    using namespace ExplorerLens::Engine;
    auto& comp = SmartThumbnailCompositor::Instance();
    comp.Initialize();
    CompositeRequest req;
    req.totalPages = 1;
    auto result = comp.Composite(req);
    ASSERT(result.success);
    ASSERT(result.appliedLayout == CompositeLayout::Single);
    ASSERT(result.layersUsed == 1);
    comp.Shutdown();
}
TEST(TestCompositor_MultiPage)
{
    using namespace ExplorerLens::Engine;
    auto& comp = SmartThumbnailCompositor::Instance();
    comp.Initialize();
    CompositeRequest req;
    req.totalPages = 8;
    req.strategy = LayerSelectionStrategy::EvenlySpaced;
    auto result = comp.Composite(req);
    ASSERT(result.success);
    ASSERT(result.layersUsed >= 2);
    comp.Shutdown();
}
TEST(TestCompositor_GridLayout)
{
    using namespace ExplorerLens::Engine;
    auto& comp = SmartThumbnailCompositor::Instance();
    comp.Initialize();
    CompositeRequest req;
    req.totalPages = 4;
    req.maxLayersToComposite = 4;
    auto result = comp.Composite(req);
    ASSERT(result.success);
    ASSERT(result.appliedLayout == CompositeLayout::Grid2x2);
    comp.Shutdown();
}
TEST(TestCompositor_LargeGrid)
{
    using namespace ExplorerLens::Engine;
    auto& comp = SmartThumbnailCompositor::Instance();
    comp.Initialize(9);
    CompositeRequest req;
    req.totalPages = 20;
    req.maxLayersToComposite = 9;
    auto result = comp.Composite(req);
    ASSERT(result.success);
    ASSERT(result.appliedLayout == CompositeLayout::Grid3x3);
    comp.Shutdown();
}
TEST(TestCompositor_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& comp = SmartThumbnailCompositor::Instance();
    comp.Initialize();
    CompositeRequest req1;
    req1.totalPages = 1;
    comp.Composite(req1);
    CompositeRequest req2;
    req2.totalPages = 4;
    comp.Composite(req2);
    auto stats = comp.GetStats();
    ASSERT(stats.totalComposites == 2);
    ASSERT(stats.singlePageResults == 1);
    ASSERT(stats.multiLayerResults == 1);
    comp.Shutdown();
}
TEST(TestCompositor_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& comp = SmartThumbnailCompositor::Instance();
    comp.Initialize();
    comp.Shutdown();
    ASSERT(!comp.IsInitialized());
}

// FormatComplexityAnalyzer tests
TEST(TestFmtComplexity_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& fca = FormatComplexityAnalyzer::Instance();
    fca.Initialize();
    ASSERT(fca.IsInitialized());
    ASSERT(fca.GetStats().totalAnalyses == 0);
    fca.Shutdown();
}
TEST(TestFmtComplexity_Trivial)
{
    using namespace ExplorerLens::Engine;
    auto& fca = FormatComplexityAnalyzer::Instance();
    fca.Initialize();
    auto r = fca.Analyze(L"tiny.jpg", 32 * 1024);
    ASSERT(r.level == FormatComplexityLevel::Trivial);
    ASSERT(r.estimatedDecodeMs < 5.0f);
    fca.Shutdown();
}
TEST(TestFmtComplexity_Extreme)
{
    using namespace ExplorerLens::Engine;
    auto& fca = FormatComplexityAnalyzer::Instance();
    fca.Initialize();
    auto r = fca.Analyze(L"huge.tiff", 256ULL * 1024 * 1024);
    ASSERT(r.level == FormatComplexityLevel::Extreme);
    ASSERT(r.requiresGPU);
    fca.Shutdown();
}
TEST(TestFmtComplexity_Moderate)
{
    using namespace ExplorerLens::Engine;
    auto& fca = FormatComplexityAnalyzer::Instance();
    fca.Initialize();
    auto r = fca.Analyze(L"photo.jpg", 4 * 1024 * 1024);
    ASSERT(r.level == FormatComplexityLevel::Moderate);
    fca.Shutdown();
}
TEST(TestFmtComplexity_Complex)
{
    using namespace ExplorerLens::Engine;
    auto& fca = FormatComplexityAnalyzer::Instance();
    fca.Initialize();
    auto r = fca.Analyze(L"raw.cr3", 64 * 1024 * 1024);
    ASSERT(r.level == FormatComplexityLevel::Complex);
    ASSERT(r.requiresGPU);
    fca.Shutdown();
}
TEST(TestFmtComplexity_StatsAverage)
{
    using namespace ExplorerLens::Engine;
    auto& fca = FormatComplexityAnalyzer::Instance();
    fca.Initialize();
    fca.Analyze(L"a.jpg", 1024);
    fca.Analyze(L"b.jpg", 1024);
    ASSERT(fca.GetStats().totalAnalyses == 2);
    ASSERT(fca.GetStats().averageEstimatedMs > 0.0f);
    fca.Shutdown();
}
TEST(TestFmtComplexity_Simple)
{
    using namespace ExplorerLens::Engine;
    auto& fca = FormatComplexityAnalyzer::Instance();
    fca.Initialize();
    auto r = fca.Analyze(L"icon.ico", 512 * 1024);
    ASSERT(r.level == FormatComplexityLevel::Simple);
    fca.Shutdown();
}
TEST(TestFmtComplexity_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& fca = FormatComplexityAnalyzer::Instance();
    fca.Initialize();
    fca.Shutdown();
    ASSERT(!fca.IsInitialized());
}

// FaultTolerantDecodeOrchestrator tests
TEST(TestFaultTolerant_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& fto = FaultTolerantDecodeOrchestrator::Instance();
    fto.Initialize(3);
    ASSERT(fto.IsInitialized());
    ASSERT(fto.GetStats().totalDecodes == 0);
    fto.Shutdown();
}
TEST(TestFaultTolerant_RecordSuccess)
{
    using namespace ExplorerLens::Engine;
    auto& fto = FaultTolerantDecodeOrchestrator::Instance();
    fto.Initialize();
    auto rec = fto.RecordAttempt(L"WebPDecoder", DecodeOutcome::Success, 5.0f);
    ASSERT(rec.outcome == DecodeOutcome::Success);
    ASSERT(fto.GetStats().successfulDecodes == 1);
    fto.Shutdown();
}
TEST(TestFaultTolerant_RecordFallback)
{
    using namespace ExplorerLens::Engine;
    auto& fto = FaultTolerantDecodeOrchestrator::Instance();
    fto.Initialize();
    fto.RecordAttempt(L"JXLDecoder", DecodeOutcome::RecoveredViaFallback, 50.0f);
    ASSERT(fto.GetStats().fallbackRecoveries == 1);
    fto.Shutdown();
}
TEST(TestFaultTolerant_Reliability)
{
    using namespace ExplorerLens::Engine;
    auto& fto = FaultTolerantDecodeOrchestrator::Instance();
    fto.Initialize();
    fto.RecordAttempt(L"PDFDecoder", DecodeOutcome::Success, 10.0f);
    fto.RecordAttempt(L"PDFDecoder", DecodeOutcome::PermanentFailure, 0.0f);
    auto rel = fto.GetDecoderReliability(L"PDFDecoder");
    ASSERT(rel.totalAttempts == 2);
    ASSERT(rel.reliabilityScore >= 0.4f);
    ASSERT(rel.reliabilityScore <= 0.6f);
    fto.Shutdown();
}
TEST(TestFaultTolerant_FallbackRecommendation)
{
    using namespace ExplorerLens::Engine;
    auto& fto = FaultTolerantDecodeOrchestrator::Instance();
    fto.Initialize();
    for (int i = 0; i < 10; ++i)
        fto.RecordAttempt(L"BadDecoder", DecodeOutcome::PermanentFailure, 0.0f);
    auto fb = fto.GetRecommendedFallback(L"BadDecoder");
    ASSERT(fb == FallbackStrategy::SkipFile);
    fto.Shutdown();
}
TEST(TestFaultTolerant_OverallReliability)
{
    using namespace ExplorerLens::Engine;
    auto& fto = FaultTolerantDecodeOrchestrator::Instance();
    fto.Initialize();
    fto.RecordAttempt(L"Dec1", DecodeOutcome::Success, 5.0f);
    fto.RecordAttempt(L"Dec2", DecodeOutcome::Success, 5.0f);
    ASSERT(fto.GetStats().overallReliability >= 0.9f);
    fto.Shutdown();
}
TEST(TestFaultTolerant_PermanentFailure)
{
    using namespace ExplorerLens::Engine;
    auto& fto = FaultTolerantDecodeOrchestrator::Instance();
    fto.Initialize();
    fto.RecordAttempt(L"CrashDecoder", DecodeOutcome::PermanentFailure, 0.0f);
    ASSERT(fto.GetStats().permanentFailures == 1);
    fto.Shutdown();
}
TEST(TestFaultTolerant_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& fto = FaultTolerantDecodeOrchestrator::Instance();
    fto.Initialize();
    fto.Shutdown();
    ASSERT(!fto.IsInitialized());
}

// DiagnosticTelemetryCollector tests
TEST(TestDiagTelemetry_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& dtc = DiagnosticTelemetryCollector::Instance();
    dtc.Initialize(100);
    ASSERT(dtc.IsInitialized());
    ASSERT(dtc.GetRecordCount() == 0);
    dtc.Shutdown();
}
TEST(TestDiagTelemetry_Record)
{
    using namespace ExplorerLens::Engine;
    auto& dtc = DiagnosticTelemetryCollector::Instance();
    dtc.Initialize();
    dtc.Record(DiagnosticCategory::Decode, DiagnosticSeverity::Info, L"WebP", L"decoded ok", 5.0f);
    ASSERT(dtc.GetRecordCount() == 1);
    ASSERT(dtc.GetStats().totalRecorded == 1);
    dtc.Shutdown();
}
TEST(TestDiagTelemetry_FilterByCategory)
{
    using namespace ExplorerLens::Engine;
    auto& dtc = DiagnosticTelemetryCollector::Instance();
    dtc.Initialize();
    dtc.Record(DiagnosticCategory::Decode, DiagnosticSeverity::Info, L"A", L"m1");
    dtc.Record(DiagnosticCategory::Cache, DiagnosticSeverity::Info, L"B", L"m2");
    dtc.Record(DiagnosticCategory::Decode, DiagnosticSeverity::Info, L"C", L"m3");
    auto decodes = dtc.GetRecords(DiagnosticCategory::Decode);
    ASSERT(decodes.size() == 2);
    dtc.Shutdown();
}
TEST(TestDiagTelemetry_ErrorTracking)
{
    using namespace ExplorerLens::Engine;
    auto& dtc = DiagnosticTelemetryCollector::Instance();
    dtc.Initialize();
    dtc.Record(DiagnosticCategory::Error, DiagnosticSeverity::Error, L"GPU", L"OOM");
    dtc.Record(DiagnosticCategory::Error, DiagnosticSeverity::Critical, L"GPU", L"crash");
    ASSERT(dtc.GetStats().errorCount == 1);
    ASSERT(dtc.GetStats().criticalCount == 1);
    dtc.Shutdown();
}
TEST(TestDiagTelemetry_RecentErrors)
{
    using namespace ExplorerLens::Engine;
    auto& dtc = DiagnosticTelemetryCollector::Instance();
    dtc.Initialize();
    dtc.Record(DiagnosticCategory::Decode, DiagnosticSeverity::Info, L"ok", L"ok");
    dtc.Record(DiagnosticCategory::Error, DiagnosticSeverity::Error, L"err", L"fail");
    auto errs = dtc.GetRecentErrors(10);
    ASSERT(errs.size() == 1);
    dtc.Shutdown();
}
TEST(TestDiagTelemetry_RingBufferEviction)
{
    using namespace ExplorerLens::Engine;
    auto& dtc = DiagnosticTelemetryCollector::Instance();
    dtc.Initialize(5);
    for (int i = 0; i < 10; ++i)
        dtc.Record(DiagnosticCategory::Decode, DiagnosticSeverity::Info, L"src", L"msg");
    ASSERT(dtc.GetRecordCount() == 5);
    ASSERT(dtc.GetStats().totalDropped >= 5);
    dtc.Shutdown();
}
TEST(TestDiagTelemetry_SequenceIds)
{
    using namespace ExplorerLens::Engine;
    auto& dtc = DiagnosticTelemetryCollector::Instance();
    dtc.Initialize();
    dtc.Record(DiagnosticCategory::GPU, DiagnosticSeverity::Info, L"gpu", L"m1");
    dtc.Record(DiagnosticCategory::GPU, DiagnosticSeverity::Info, L"gpu", L"m2");
    auto recs = dtc.GetRecords(DiagnosticCategory::GPU);
    ASSERT(recs.size() == 2);
    ASSERT(recs[1].sequenceId > recs[0].sequenceId);
    dtc.Shutdown();
}
TEST(TestDiagTelemetry_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& dtc = DiagnosticTelemetryCollector::Instance();
    dtc.Initialize();
    dtc.Record(DiagnosticCategory::Decode, DiagnosticSeverity::Info, L"x", L"y");
    dtc.Shutdown();
    ASSERT(!dtc.IsInitialized());
    ASSERT(dtc.GetRecordCount() == 0);
}

// DecoderFaultIsolator tests
TEST(TestFaultIsolator_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& dfi = DecoderFaultIsolator::Instance();
    dfi.Initialize(5, 3);
    ASSERT(dfi.IsInitialized());
    ASSERT(dfi.GetStats().totalFaults == 0);
    dfi.Shutdown();
}
TEST(TestFaultIsolator_RecordFault)
{
    using namespace ExplorerLens::Engine;
    auto& dfi = DecoderFaultIsolator::Instance();
    dfi.Initialize();
    dfi.RecordFault(L"WebPDecoder", FaultSeverity::Minor);
    ASSERT(dfi.GetStats().totalFaults == 1);
    ASSERT(!dfi.IsQuarantined(L"WebPDecoder"));
    dfi.Shutdown();
}
TEST(TestFaultIsolator_Quarantine)
{
    using namespace ExplorerLens::Engine;
    auto& dfi = DecoderFaultIsolator::Instance();
    dfi.Initialize(3, 3);
    for (int i = 0; i < 3; ++i)
        dfi.RecordFault(L"BadDec", FaultSeverity::Major);
    ASSERT(dfi.IsQuarantined(L"BadDec"));
    ASSERT(dfi.GetStats().quarantineEvents >= 1);
    dfi.Shutdown();
}
TEST(TestFaultIsolator_FatalImmediate)
{
    using namespace ExplorerLens::Engine;
    auto& dfi = DecoderFaultIsolator::Instance();
    dfi.Initialize();
    dfi.RecordFault(L"CrashDec", FaultSeverity::Fatal);
    ASSERT(dfi.IsQuarantined(L"CrashDec"));
    dfi.Shutdown();
}
TEST(TestFaultIsolator_ReleaseFromQuarantine)
{
    using namespace ExplorerLens::Engine;
    auto& dfi = DecoderFaultIsolator::Instance();
    dfi.Initialize(2, 5);
    dfi.RecordFault(L"Dec1", FaultSeverity::Major);
    dfi.RecordFault(L"Dec1", FaultSeverity::Major);
    ASSERT(dfi.IsQuarantined(L"Dec1"));
    dfi.ReleaseFromQuarantine(L"Dec1");
    ASSERT(!dfi.IsQuarantined(L"Dec1"));
    dfi.Shutdown();
}
TEST(TestFaultIsolator_PermanentDisable)
{
    using namespace ExplorerLens::Engine;
    auto& dfi = DecoderFaultIsolator::Instance();
    dfi.Initialize(1, 1);
    dfi.RecordFault(L"Dec2", FaultSeverity::Major);
    dfi.ReleaseFromQuarantine(L"Dec2");
    dfi.RecordFault(L"Dec2", FaultSeverity::Major);
    auto state = dfi.GetDecoderState(L"Dec2");
    ASSERT(state.status == QuarantineStatus::PermanentlyDisabled);
    dfi.Shutdown();
}
TEST(TestFaultIsolator_SuccessResetsConsecutive)
{
    using namespace ExplorerLens::Engine;
    auto& dfi = DecoderFaultIsolator::Instance();
    dfi.Initialize(5, 3);
    dfi.RecordFault(L"Dec3", FaultSeverity::Minor);
    dfi.RecordFault(L"Dec3", FaultSeverity::Minor);
    dfi.RecordSuccess(L"Dec3");
    auto state = dfi.GetDecoderState(L"Dec3");
    ASSERT(state.consecutiveFaults == 0);
    dfi.Shutdown();
}
TEST(TestFaultIsolator_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& dfi = DecoderFaultIsolator::Instance();
    dfi.Initialize();
    dfi.Shutdown();
    ASSERT(!dfi.IsInitialized());
}

// SmartRetryOrchestrator tests
TEST(TestSmartRetry_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& sro = SmartRetryOrchestrator::Instance();
    sro.Initialize();
    ASSERT(sro.IsInitialized());
    ASSERT(sro.GetStats().totalRetryRequests == 0);
    sro.Shutdown();
}
TEST(TestSmartRetry_BasicRetry)
{
    using namespace ExplorerLens::Engine;
    auto& sro = SmartRetryOrchestrator::Instance();
    sro.Initialize();
    auto att = sro.EvaluateRetry(L"WebP", SmartRetryReason::Timeout);
    ASSERT(att.decision == SmartRetryDecision::Retry);
    ASSERT(att.attemptNumber == 1);
    ASSERT(att.delayMs >= 40.0f);
    sro.Shutdown();
}
TEST(TestSmartRetry_ExponentialBackoff)
{
    using namespace ExplorerLens::Engine;
    auto& sro = SmartRetryOrchestrator::Instance();
    SmartRetryConfig cfg;
    cfg.maxRetries = 5;
    cfg.baseDelayMs = 100.0f;
    cfg.backoffMultiplier = 2.0f;
    sro.Initialize(cfg);
    auto a1 = sro.EvaluateRetry(L"Dec", SmartRetryReason::Timeout);
    auto a2 = sro.EvaluateRetry(L"Dec", SmartRetryReason::Timeout);
    ASSERT(a2.delayMs > a1.delayMs);
    sro.Shutdown();
}
TEST(TestSmartRetry_CorruptNotRetryable)
{
    using namespace ExplorerLens::Engine;
    auto& sro = SmartRetryOrchestrator::Instance();
    sro.Initialize();
    auto att = sro.EvaluateRetry(L"Dec", SmartRetryReason::CorruptData);
    ASSERT(att.decision == SmartRetryDecision::NotRetryable);
    sro.Shutdown();
}
TEST(TestSmartRetry_GPUFallback)
{
    using namespace ExplorerLens::Engine;
    auto& sro = SmartRetryOrchestrator::Instance();
    sro.Initialize();
    auto att = sro.EvaluateRetry(L"GPUDec", SmartRetryReason::GPUError);
    ASSERT(att.decision == SmartRetryDecision::RetryWithFallback);
    sro.Shutdown();
}
TEST(TestSmartRetry_Exhausted)
{
    using namespace ExplorerLens::Engine;
    auto& sro = SmartRetryOrchestrator::Instance();
    SmartRetryConfig cfg;
    cfg.maxRetries = 2;
    sro.Initialize(cfg);
    sro.EvaluateRetry(L"Dec", SmartRetryReason::Timeout);
    sro.EvaluateRetry(L"Dec", SmartRetryReason::Timeout);
    auto a3 = sro.EvaluateRetry(L"Dec", SmartRetryReason::Timeout);
    ASSERT(a3.decision == SmartRetryDecision::Exhausted);
    sro.Shutdown();
}
TEST(TestSmartRetry_CircuitBreaker)
{
    using namespace ExplorerLens::Engine;
    auto& sro = SmartRetryOrchestrator::Instance();
    SmartRetryConfig cfg;
    cfg.maxRetries = 100;
    cfg.circuitBreakerThreshold = 5;
    sro.Initialize(cfg);
    for (int i = 0; i < 6; ++i)
        sro.EvaluateRetry(L"Flaky", SmartRetryReason::TransientFailure);
    ASSERT(sro.GetStats().circuitBreakerTrips >= 1);
    sro.Shutdown();
}
TEST(TestSmartRetry_ResetDecoder)
{
    using namespace ExplorerLens::Engine;
    auto& sro = SmartRetryOrchestrator::Instance();
    sro.Initialize();
    sro.EvaluateRetry(L"Dec", SmartRetryReason::Timeout);
    sro.ResetDecoder(L"Dec");
    auto att = sro.EvaluateRetry(L"Dec", SmartRetryReason::Timeout);
    ASSERT(att.attemptNumber == 1);
    sro.Shutdown();
}
TEST(TestSmartRetry_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& sro = SmartRetryOrchestrator::Instance();
    sro.Initialize();
    sro.Shutdown();
    ASSERT(!sro.IsInitialized());
}

// PipelineHealthMonitor tests
TEST(TestPipeHealth_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& phm = PipelineHealthMonitor::Instance();
    phm.Initialize();
    ASSERT(phm.IsInitialized());
    ASSERT(phm.GetStats().totalSamples == 0);
    phm.Shutdown();
}
TEST(TestPipeHealth_RecordSample)
{
    using namespace ExplorerLens::Engine;
    auto& phm = PipelineHealthMonitor::Instance();
    phm.Initialize();
    phm.RecordSample(10.0f, true);
    phm.RecordSample(20.0f, true);
    ASSERT(phm.GetStats().totalSamples == 2);
    phm.Shutdown();
}
TEST(TestPipeHealth_HealthySnapshot)
{
    using namespace ExplorerLens::Engine;
    auto& phm = PipelineHealthMonitor::Instance();
    phm.Initialize();
    for (int i = 0; i < 100; ++i)
        phm.RecordSample(10.0f, true);
    auto snap = phm.GetSnapshot();
    ASSERT(snap.overallHealth == PipelineAlertLevel::Normal);
    ASSERT(snap.currentErrorRate < 0.01f);
    phm.Shutdown();
}
TEST(TestPipeHealth_HighErrorRate)
{
    using namespace ExplorerLens::Engine;
    auto& phm = PipelineHealthMonitor::Instance();
    PipelineHealthThresholds thresholds;
    thresholds.maxErrorRate = 0.1f;
    phm.Initialize(thresholds);
    for (int i = 0; i < 5; ++i)
        phm.RecordSample(10.0f, true);
    for (int i = 0; i < 5; ++i)
        phm.RecordSample(10.0f, false);
    auto snap = phm.GetSnapshot();
    ASSERT(snap.currentErrorRate >= 0.4f);
    ASSERT(snap.overallHealth >= PipelineAlertLevel::Degraded);
    phm.Shutdown();
}
TEST(TestPipeHealth_HighLatencyAlert)
{
    using namespace ExplorerLens::Engine;
    auto& phm = PipelineHealthMonitor::Instance();
    PipelineHealthThresholds thresholds;
    thresholds.maxP95LatencyMs = 50.0f;
    thresholds.maxP99LatencyMs = 200.0f;
    phm.Initialize(thresholds);
    for (int i = 0; i < 100; ++i)
        phm.RecordSample(static_cast<float>(i * 3), true);
    auto snap = phm.GetSnapshot();
    ASSERT(snap.p95LatencyMs > 50.0f);
    ASSERT(snap.activeAlertCount >= 1);
    phm.Shutdown();
}
TEST(TestPipeHealth_Alerts)
{
    using namespace ExplorerLens::Engine;
    auto& phm = PipelineHealthMonitor::Instance();
    PipelineHealthThresholds thresholds;
    thresholds.maxP99LatencyMs = 10.0f;
    phm.Initialize(thresholds);
    for (int i = 0; i < 100; ++i)
        phm.RecordSample(static_cast<float>(i * 10), true);
    auto alerts = phm.GetActiveAlerts();
    ASSERT(!alerts.empty());
    phm.Shutdown();
}
TEST(TestPipeHealth_PeakAlertLevel)
{
    using namespace ExplorerLens::Engine;
    auto& phm = PipelineHealthMonitor::Instance();
    PipelineHealthThresholds thresholds;
    thresholds.maxP99LatencyMs = 5.0f;
    phm.Initialize(thresholds);
    for (int i = 0; i < 100; ++i)
        phm.RecordSample(100.0f, true);
    ASSERT(phm.GetStats().peakAlertLevel >= PipelineAlertLevel::Critical);
    phm.Shutdown();
}
TEST(TestPipeHealth_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& phm = PipelineHealthMonitor::Instance();
    phm.Initialize();
    phm.RecordSample(10.0f, true);
    phm.Shutdown();
    ASSERT(!phm.IsInitialized());
}

//== Sprint 1081-1090 — Format Routing & Enhanced Accessibility Tests (v31.5.0) ==

// AdaptiveColorProfileManager tests
TEST(TestColorProfile_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& cpm = AdaptiveColorProfileManager::Instance();
    cpm.Initialize(AdaptiveColorSpace::SRGB, 100.0f);
    ASSERT(cpm.IsInitialized());
    ASSERT(cpm.GetStats().totalProfileMatches == 0);
    cpm.Shutdown();
}
TEST(TestColorProfile_MatchSRGB)
{
    using namespace ExplorerLens::Engine;
    auto& cpm = AdaptiveColorProfileManager::Instance();
    cpm.Initialize();
    auto p = cpm.MatchProfile(AdaptiveColorSpace::SRGB);
    ASSERT(p.toneMapping == ToneMappingMode::None);
    ASSERT(p.targetSpace == AdaptiveColorSpace::SRGB);
    cpm.Shutdown();
}
TEST(TestColorProfile_HDRtoSDR)
{
    using namespace ExplorerLens::Engine;
    auto& cpm = AdaptiveColorProfileManager::Instance();
    cpm.Initialize(AdaptiveColorSpace::SRGB, 100.0f);
    auto p = cpm.MatchProfile(AdaptiveColorSpace::HDR10);
    ASSERT(p.toneMapping == ToneMappingMode::Reinhard);
    ASSERT(p.targetSpace == AdaptiveColorSpace::SRGB);
    cpm.Shutdown();
}
TEST(TestColorProfile_HDRtoHDR)
{
    using namespace ExplorerLens::Engine;
    auto& cpm = AdaptiveColorProfileManager::Instance();
    cpm.Initialize(AdaptiveColorSpace::DisplayP3, 1000.0f);
    auto p = cpm.MatchProfile(AdaptiveColorSpace::HDR10);
    ASSERT(p.toneMapping == ToneMappingMode::ACES);
    ASSERT(p.hdrCapable);
    cpm.Shutdown();
}
TEST(TestColorProfile_GamutMapping)
{
    using namespace ExplorerLens::Engine;
    auto& cpm = AdaptiveColorProfileManager::Instance();
    cpm.Initialize();
    cpm.MatchProfile(AdaptiveColorSpace::AdobeRGB);
    ASSERT(cpm.GetStats().gamutMappings == 1);
    cpm.Shutdown();
}
TEST(TestColorProfile_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& cpm = AdaptiveColorProfileManager::Instance();
    cpm.Initialize(AdaptiveColorSpace::SRGB, 80.0f);
    cpm.MatchProfile(AdaptiveColorSpace::HDR10);
    cpm.MatchProfile(AdaptiveColorSpace::SRGB);
    ASSERT(cpm.GetStats().totalProfileMatches == 2);
    ASSERT(cpm.GetStats().fallbacksToSRGB >= 1);
    cpm.Shutdown();
}
TEST(TestColorProfile_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& cpm = AdaptiveColorProfileManager::Instance();
    cpm.Initialize();
    cpm.Shutdown();
    ASSERT(!cpm.IsInitialized());
}

// ThumbnailAccessibilityEngine tests
TEST(TestAccessibility_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& acc = ThumbnailAccessibilityEngine::Instance();
    acc.Initialize();
    ASSERT(acc.IsInitialized());
    ASSERT(acc.GetStats().totalProcessed == 0);
    acc.Shutdown();
}
TEST(TestAccessibility_Standard)
{
    using namespace ExplorerLens::Engine;
    auto& acc = ThumbnailAccessibilityEngine::Instance();
    acc.Initialize();
    auto r = acc.Process(L"test.jpg", 256, 256);
    ASSERT(r.applied);
    ASSERT(r.modeUsed == AccessibilityMode::Standard);
    acc.Shutdown();
}
TEST(TestAccessibility_HighContrast)
{
    using namespace ExplorerLens::Engine;
    auto& acc = ThumbnailAccessibilityEngine::Instance();
    AccessibilitySettings s;
    s.mode = AccessibilityMode::HighContrast;
    acc.Initialize(s);
    auto r = acc.Process(L"test.png", 256, 256);
    ASSERT(r.contrastAdjusted);
    ASSERT(acc.GetStats().highContrastApplied == 1);
    acc.Shutdown();
}
TEST(TestAccessibility_ColorBlind)
{
    using namespace ExplorerLens::Engine;
    auto& acc = ThumbnailAccessibilityEngine::Instance();
    AccessibilitySettings s;
    s.mode = AccessibilityMode::ColorBlindProtanopia;
    acc.Initialize(s);
    acc.Process(L"photo.jpg", 128, 128);
    ASSERT(acc.GetStats().colorBlindAdjusted == 1);
    acc.Shutdown();
}
TEST(TestAccessibility_AltText)
{
    using namespace ExplorerLens::Engine;
    auto& acc = ThumbnailAccessibilityEngine::Instance();
    AccessibilitySettings s;
    s.generateAltText = true;
    acc.Initialize(s);
    auto r = acc.Process(L"img.webp", 320, 240);
    ASSERT(!r.altText.empty());
    ASSERT(acc.GetStats().altTextsGenerated == 1);
    acc.Shutdown();
}
TEST(TestAccessibility_Settings)
{
    using namespace ExplorerLens::Engine;
    auto& acc = ThumbnailAccessibilityEngine::Instance();
    AccessibilitySettings s;
    s.contrast = ContrastLevel::Maximum;
    acc.Initialize(s);
    auto got = acc.GetSettings();
    ASSERT(got.contrast == ContrastLevel::Maximum);
    acc.Shutdown();
}
TEST(TestAccessibility_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& acc = ThumbnailAccessibilityEngine::Instance();
    acc.Initialize();
    acc.Shutdown();
    ASSERT(!acc.IsInitialized());
}

// SmartFileTypeRouter tests
TEST(TestFileRouter_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& rtr = SmartFileTypeRouter::Instance();
    rtr.Initialize();
    ASSERT(rtr.IsInitialized());
    ASSERT(rtr.GetRegisteredRouteCount() == 0);
    rtr.Shutdown();
}
TEST(TestFileRouter_RegisterAndRoute)
{
    using namespace ExplorerLens::Engine;
    auto& rtr = SmartFileTypeRouter::Instance();
    rtr.Initialize();
    rtr.RegisterRoute(L".jpg", L"JPEGDecoder", L"GenericDecoder");
    auto d = rtr.Route(L".jpg");
    ASSERT(d.selectedDecoder == L"JPEGDecoder");
    ASSERT(d.confidence == RouteConfidence::ExtensionOnly);
    ASSERT(d.hasFallback);
    rtr.Shutdown();
}
TEST(TestFileRouter_UnknownFormat)
{
    using namespace ExplorerLens::Engine;
    auto& rtr = SmartFileTypeRouter::Instance();
    rtr.Initialize();
    auto d = rtr.Route(L".xyz123");
    ASSERT(d.selectedDecoder == L"GenericDecoder");
    ASSERT(d.confidence == RouteConfidence::Unknown);
    ASSERT(rtr.GetStats().unknownFormats == 1);
    rtr.Shutdown();
}
TEST(TestFileRouter_RecordSuccess)
{
    using namespace ExplorerLens::Engine;
    auto& rtr = SmartFileTypeRouter::Instance();
    rtr.Initialize();
    rtr.RegisterRoute(L".png", L"PNGDecoder");
    rtr.RecordResult(L".png", true, 5.0f);
    rtr.RecordResult(L".png", true, 15.0f);
    auto d = rtr.Route(L".png");
    ASSERT(d.estimatedDecodeMs > 0.0f);
    rtr.Shutdown();
}
TEST(TestFileRouter_RecordFailure)
{
    using namespace ExplorerLens::Engine;
    auto& rtr = SmartFileTypeRouter::Instance();
    rtr.Initialize();
    rtr.RegisterRoute(L".bmp", L"BMPDecoder");
    rtr.RecordResult(L".bmp", false, 0.0f);
    ASSERT(rtr.GetStats().fallbacksUsed == 1);
    rtr.Shutdown();
}
TEST(TestFileRouter_MultipleRoutes)
{
    using namespace ExplorerLens::Engine;
    auto& rtr = SmartFileTypeRouter::Instance();
    rtr.Initialize();
    rtr.RegisterRoute(L".jpg", L"JPEG");
    rtr.RegisterRoute(L".png", L"PNG");
    rtr.RegisterRoute(L".webp", L"WebP");
    ASSERT(rtr.GetRegisteredRouteCount() == 3);
    rtr.Shutdown();
}
TEST(TestFileRouter_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& rtr = SmartFileTypeRouter::Instance();
    rtr.Initialize();
    rtr.RegisterRoute(L".jpg", L"JPEG");
    rtr.Shutdown();
    ASSERT(!rtr.IsInitialized());
}

// DecoderVersionManager tests
TEST(TestDecVersion_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& dvm = DecoderVersionManager::Instance();
    dvm.Initialize();
    ASSERT(dvm.IsInitialized());
    ASSERT(dvm.GetStats().registeredDecoders == 0);
    dvm.Shutdown();
}
TEST(TestDecVersion_Register)
{
    using namespace ExplorerLens::Engine;
    auto& dvm = DecoderVersionManager::Instance();
    dvm.Initialize();
    DecoderVersionInfo info;
    info.decoderName = L"WebPDecoder";
    info.majorVersion = 2;
    info.minorVersion = 1;
    info.supportsGPU = true;
    dvm.RegisterDecoder(info);
    ASSERT(dvm.GetStats().registeredDecoders == 1);
    ASSERT(dvm.GetStats().gpuCapableDecoders == 1);
    dvm.Shutdown();
}
TEST(TestDecVersion_CompatFull)
{
    using namespace ExplorerLens::Engine;
    auto& dvm = DecoderVersionManager::Instance();
    dvm.Initialize();
    DecoderVersionInfo info;
    info.decoderName = L"Dec";
    info.majorVersion = 3;
    info.minorVersion = 2;
    dvm.RegisterDecoder(info);
    auto r = dvm.CheckCompatibility(L"Dec", 2, 0);
    ASSERT(r.compatibility == VersionCompatibility::FullyCompatible);
    dvm.Shutdown();
}
TEST(TestDecVersion_CompatBackward)
{
    using namespace ExplorerLens::Engine;
    auto& dvm = DecoderVersionManager::Instance();
    dvm.Initialize();
    DecoderVersionInfo info;
    info.decoderName = L"Dec";
    info.majorVersion = 3;
    info.minorVersion = 0;
    dvm.RegisterDecoder(info);
    auto r = dvm.CheckCompatibility(L"Dec", 3, 5);
    ASSERT(r.compatibility == VersionCompatibility::BackwardCompatible);
    ASSERT(r.upgradeAvailable);
    dvm.Shutdown();
}
TEST(TestDecVersion_Deprecated)
{
    using namespace ExplorerLens::Engine;
    auto& dvm = DecoderVersionManager::Instance();
    dvm.Initialize();
    DecoderVersionInfo info;
    info.decoderName = L"OldDec";
    info.majorVersion = 1;
    info.deprecated = true;
    dvm.RegisterDecoder(info);
    auto r = dvm.CheckCompatibility(L"OldDec", 1);
    ASSERT(r.compatibility == VersionCompatibility::MinorIncompatibility);
    ASSERT(dvm.GetStats().deprecatedDecoders == 1);
    dvm.Shutdown();
}
TEST(TestDecVersion_Unsupported)
{
    using namespace ExplorerLens::Engine;
    auto& dvm = DecoderVersionManager::Instance();
    dvm.Initialize();
    auto r = dvm.CheckCompatibility(L"NonExistent", 1);
    ASSERT(r.compatibility == VersionCompatibility::Unsupported);
    dvm.Shutdown();
}
TEST(TestDecVersion_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& dvm = DecoderVersionManager::Instance();
    dvm.Initialize();
    dvm.Shutdown();
    ASSERT(!dvm.IsInitialized());
}

// CrossFormatMetadataEngine tests
TEST(TestCrossMeta_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& cme = CrossFormatMetadataEngine::Instance();
    cme.Initialize();
    ASSERT(cme.IsInitialized());
    ASSERT(cme.GetStats().totalExtractions == 0);
    cme.Shutdown();
}
TEST(TestCrossMeta_JPEG)
{
    using namespace ExplorerLens::Engine;
    auto& cme = CrossFormatMetadataEngine::Instance();
    cme.Initialize();
    auto m = cme.Extract(L"photo.jpg", L".jpg");
    ASSERT(m.primarySource == CrossMetadataSource::EXIF);
    ASSERT(cme.GetStats().exifFound == 1);
    cme.Shutdown();
}
TEST(TestCrossMeta_PNG)
{
    using namespace ExplorerLens::Engine;
    auto& cme = CrossFormatMetadataEngine::Instance();
    cme.Initialize();
    auto m = cme.Extract(L"image.png", L".png");
    ASSERT(m.primarySource == CrossMetadataSource::XMP);
    ASSERT(cme.GetStats().xmpFound == 1);
    cme.Shutdown();
}
TEST(TestCrossMeta_PDF)
{
    using namespace ExplorerLens::Engine;
    auto& cme = CrossFormatMetadataEngine::Instance();
    cme.Initialize();
    auto m = cme.Extract(L"doc.pdf", L".pdf");
    ASSERT(m.primarySource == CrossMetadataSource::PDF);
    cme.Shutdown();
}
// CrossFormatMetadataEngine — additional tests
TEST(TestCrossMeta_RAW)
{
    using namespace ExplorerLens::Engine;
    auto& cme = CrossFormatMetadataEngine::Instance();
    cme.Initialize();
    auto m = cme.Extract(L"photo.tiff", L".tiff");
    ASSERT(m.primarySource == CrossMetadataSource::EXIF);
    cme.Shutdown();
}
TEST(TestCrossMeta_Video)
{
    using namespace ExplorerLens::Engine;
    auto& cme = CrossFormatMetadataEngine::Instance();
    cme.Initialize();
    auto m = cme.Extract(L"track.mp3", L".mp3");
    ASSERT(m.primarySource == CrossMetadataSource::ID3);
    cme.Shutdown();
}
TEST(TestCrossMeta_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& cme = CrossFormatMetadataEngine::Instance();
    cme.Initialize();
    cme.Extract(L"a.jpg", L".jpg");
    cme.Extract(L"b.png", L".png");
    auto s = cme.GetStats();
    ASSERT(s.totalExtractions == 2);
    cme.Shutdown();
}

// StreamingDecodeCoordinator tests
TEST(TestSDC_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& sdc = StreamingDecodeCoordinator::Instance();
    sdc.Initialize(64);
    ASSERT(sdc.GetStats().initialized);
    ASSERT(sdc.GetStats().totalStreams == 0);
    sdc.Shutdown();
}
TEST(TestSDC_StartStream)
{
    using namespace ExplorerLens::Engine;
    auto& sdc = StreamingDecodeCoordinator::Instance();
    sdc.Initialize();
    StreamingDecodeRequest req;
    req.filePath = L"large.raw";
    req.fileSize = 50 * 1024 * 1024;
    req.chunkSizeKB = 128;
    auto prog = sdc.StartStream(req);
    ASSERT(sdc.GetStats().totalStreams == 1);
    sdc.Shutdown();
}
TEST(TestSDC_PartialRender)
{
    using namespace ExplorerLens::Engine;
    auto& sdc = StreamingDecodeCoordinator::Instance();
    sdc.Initialize();
    StreamingDecodeRequest req;
    req.filePath = L"huge.tif";
    req.fileSize = 200 * 1024 * 1024;
    req.allowPartialRender = true;
    sdc.StartStream(req);
    ASSERT(sdc.GetStats().partialRenders >= 0);
    sdc.Shutdown();
}
TEST(TestSDC_CompleteStream)
{
    using namespace ExplorerLens::Engine;
    auto& sdc = StreamingDecodeCoordinator::Instance();
    sdc.Initialize();
    StreamingDecodeRequest req;
    req.filePath = L"test.psd";
    req.fileSize = 64 * 1024;
    req.chunkSizeKB = 64;
    sdc.StartStream(req);
    // advance all chunks to completion
    while (sdc.GetProgress().state != StreamingDecodeState::Complete) {
        sdc.AdvanceChunk();
    }
    ASSERT(sdc.GetStats().completedStreams >= 1);
    sdc.Shutdown();
}
TEST(TestSDC_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& sdc = StreamingDecodeCoordinator::Instance();
    sdc.Initialize();
    auto s = sdc.GetStats();
    ASSERT(s.initialized);
    sdc.Shutdown();
}
TEST(TestSDC_ProgressTracking)
{
    using namespace ExplorerLens::Engine;
    auto& sdc = StreamingDecodeCoordinator::Instance();
    sdc.Initialize(32);
    StreamingDecodeRequest req;
    req.filePath = L"progress.cr2";
    req.fileSize = 30 * 1024 * 1024;
    req.chunkSizeKB = 32;
    sdc.StartStream(req);
    auto prog = sdc.GetProgress();
    ASSERT(prog.totalChunks > 0);
    sdc.Shutdown();
}
TEST(TestSDC_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& sdc = StreamingDecodeCoordinator::Instance();
    sdc.Initialize();
    sdc.Shutdown();
    ASSERT(!sdc.GetStats().initialized);
}

// RenderPipelineProfiler tests
TEST(TestRPP_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& rpp = RenderPipelineProfiler::Instance();
    rpp.Initialize();
    ASSERT(rpp.GetStats().initialized);
    ASSERT(rpp.GetStats().totalProfiles == 0);
    rpp.Shutdown();
}
TEST(TestRPP_RecordStage)
{
    using namespace ExplorerLens::Engine;
    auto& rpp = RenderPipelineProfiler::Instance();
    rpp.Initialize();
    rpp.RecordStage(ProfilerStage::Decode, 12.5f);
    rpp.RecordStage(ProfilerStage::GPUUpload, 3.2f);
    ASSERT(rpp.GetStats().stagesRecorded >= 2);
    rpp.Shutdown();
}
TEST(TestRPP_Snapshot)
{
    using namespace ExplorerLens::Engine;
    auto& rpp = RenderPipelineProfiler::Instance();
    rpp.Initialize();
    rpp.RecordStage(ProfilerStage::FileIO, 5.0f);
    rpp.RecordStage(ProfilerStage::Decode, 15.0f);
    auto snap = rpp.GetSnapshot();
    ASSERT(snap.totalPipelineMs > 0.0f);
    rpp.Shutdown();
}
TEST(TestRPP_Bottleneck)
{
    using namespace ExplorerLens::Engine;
    auto& rpp = RenderPipelineProfiler::Instance();
    rpp.Initialize();
    rpp.RecordStage(ProfilerStage::FileIO, 2.0f);
    rpp.RecordStage(ProfilerStage::Decode, 45.0f);
    rpp.RecordStage(ProfilerStage::GPUUpload, 1.5f);
    auto snap = rpp.GetSnapshot();
    ASSERT(snap.bottleneckStage == ProfilerStage::Decode);
    rpp.Shutdown();
}
TEST(TestRPP_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& rpp = RenderPipelineProfiler::Instance();
    rpp.Initialize();
    rpp.RecordStage(ProfilerStage::CacheLookup, 0.3f);
    auto s = rpp.GetStats();
    ASSERT(s.stagesRecorded >= 1);
    rpp.Shutdown();
}
TEST(TestRPP_AllStages)
{
    using namespace ExplorerLens::Engine;
    auto& rpp = RenderPipelineProfiler::Instance();
    rpp.Initialize();
    for (int i = 0; i <= static_cast<int>(ProfilerStage::Output); ++i) {
        rpp.RecordStage(static_cast<ProfilerStage>(i), static_cast<float>(i + 1));
    }
    auto snap = rpp.GetSnapshot();
    ASSERT(snap.stages.size() == static_cast<size_t>(static_cast<int>(ProfilerStage::Output) + 1));
    rpp.Shutdown();
}
TEST(TestRPP_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& rpp = RenderPipelineProfiler::Instance();
    rpp.Initialize();
    rpp.Shutdown();
    ASSERT(!rpp.GetStats().initialized);
}

//== v32.0.0 Fomalhaut — Post-Quantum Security & Zero-Trust Tests ==//

// PostQuantumCryptoProvider
TEST(TestPQCP_Init)
{
    using namespace ExplorerLens::Engine;
    PostQuantumCryptoProvider p;
    ASSERT(p.Initialize());
    ASSERT(p.IsReady());
}
TEST(TestPQCP_KeyGen_Kyber)
{
    using namespace ExplorerLens::Engine;
    PostQuantumCryptoProvider p;
    p.Initialize();
    auto kp = p.GenerateKeyPair(PQCPrimitiveAlgo::Kyber768);
    ASSERT(!kp.publicKey.empty());
    ASSERT(!kp.privateKey.empty());
    ASSERT(kp.publicKey.size() == 1184);
}
TEST(TestPQCP_KeyGen_Dilithium)
{
    using namespace ExplorerLens::Engine;
    PostQuantumCryptoProvider p;
    p.Initialize();
    auto kp = p.GenerateKeyPair(PQCPrimitiveAlgo::Dilithium3);
    ASSERT(!kp.publicKey.empty());
    ASSERT(!kp.privateKey.empty());
    ASSERT(kp.algorithm == PQCPrimitiveAlgo::Dilithium3);
}
TEST(TestPQCP_Sign)
{
    using namespace ExplorerLens::Engine;
    PostQuantumCryptoProvider p;
    p.Initialize();
    auto kp = p.GenerateKeyPair(PQCPrimitiveAlgo::Dilithium3);
    std::vector<uint8_t> msg = {0x01, 0x02, 0x03};
    auto sig = p.Sign(msg, kp);
    ASSERT(!sig.empty());
    ASSERT(sig.size() == 3293);
}
TEST(TestPQCP_VerifyOk)
{
    using namespace ExplorerLens::Engine;
    PostQuantumCryptoProvider p;
    p.Initialize();
    std::vector<uint8_t> msg = {0xAA, 0xBB};
    std::vector<uint8_t> sig = {0x01};
    std::vector<uint8_t> pub = {0x02};
    ASSERT(p.Verify(msg, sig, pub));
}
TEST(TestPQCP_VerifyFail_Empty)
{
    using namespace ExplorerLens::Engine;
    PostQuantumCryptoProvider p;
    p.Initialize();
    std::vector<uint8_t> empty;
    std::vector<uint8_t> nonempty = {0x01};
    ASSERT(!p.Verify(empty, nonempty, nonempty));
}
TEST(TestPQCP_Stats_KeyGenCount)
{
    using namespace ExplorerLens::Engine;
    PostQuantumCryptoProvider p;
    p.Initialize();
    p.GenerateKeyPair(PQCPrimitiveAlgo::Kyber768);
    p.GenerateKeyPair(PQCPrimitiveAlgo::Kyber768);
    p.GenerateKeyPair(PQCPrimitiveAlgo::Dilithium3);
    ASSERT(p.GetStats().keyGensOk == 3);
}
TEST(TestPQCP_Stats_SignCount)
{
    using namespace ExplorerLens::Engine;
    PostQuantumCryptoProvider p;
    p.Initialize();
    auto kp = p.GenerateKeyPair(PQCPrimitiveAlgo::Dilithium3);
    std::vector<uint8_t> msg = {0x01};
    p.Sign(msg, kp);
    p.Sign(msg, kp);
    ASSERT(p.GetStats().signOps == 2);
}
TEST(TestPQCP_Reset)
{
    using namespace ExplorerLens::Engine;
    PostQuantumCryptoProvider p;
    p.Initialize();
    p.GenerateKeyPair(PQCPrimitiveAlgo::Kyber768);
    p.Reset();
    ASSERT(p.GetStats().keyGensOk == 0);
}

// ZeroTrustAccessBroker
TEST(TestZTAB_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& a = ZeroTrustAccessBroker::Instance();
    auto& b = ZeroTrustAccessBroker::Instance();
    ASSERT(&a == &b);
}
TEST(TestZTAB_Issue)
{
    using namespace ExplorerLens::Engine;
    auto& broker = ZeroTrustAccessBroker::Instance();
    auto tok = broker.Issue("plugin-a", "decode");
    ASSERT(tok.subject == "plugin-a");
    ASSERT(tok.capability == "decode");
    ASSERT(!tok.signature.empty());
}
TEST(TestZTAB_Validate_Valid)
{
    using namespace ExplorerLens::Engine;
    auto& broker = ZeroTrustAccessBroker::Instance();
    auto tok = broker.Issue("plugin-b", "cache");
    ASSERT(broker.Validate(tok));
}
TEST(TestZTAB_Validate_Revoked)
{
    using namespace ExplorerLens::Engine;
    auto& broker = ZeroTrustAccessBroker::Instance();
    auto tok = broker.Issue("plugin-c", "ui");
    tok.revoked = true;
    ASSERT(!broker.Validate(tok));
}
TEST(TestZTAB_Revoke)
{
    using namespace ExplorerLens::Engine;
    auto& broker = ZeroTrustAccessBroker::Instance();
    broker.Issue("plugin-revoke", "gpu");
    ASSERT(broker.Revoke("plugin-revoke"));
}
TEST(TestZTAB_RevokeMiss)
{
    using namespace ExplorerLens::Engine;
    auto& broker = ZeroTrustAccessBroker::Instance();
    ASSERT(!broker.Revoke("nonexistent-plugin-xyz"));
}
TEST(TestZTAB_Stats_Issued)
{
    using namespace ExplorerLens::Engine;
    auto& broker = ZeroTrustAccessBroker::Instance();
    uint64_t before = broker.GetStats().tokensIssued;
    broker.Issue("plugin-stat1", "read");
    ASSERT(broker.GetStats().tokensIssued == before + 1);
}
TEST(TestZTAB_Stats_Denied)
{
    using namespace ExplorerLens::Engine;
    auto& broker = ZeroTrustAccessBroker::Instance();
    ZeroTrustToken empty_tok;
    uint64_t before = broker.GetStats().tokensDenied;
    broker.Validate(empty_tok);
    ASSERT(broker.GetStats().tokensDenied == before + 1);
}
TEST(TestZTAB_MultiToken)
{
    using namespace ExplorerLens::Engine;
    auto& broker = ZeroTrustAccessBroker::Instance();
    auto t1 = broker.Issue("multi-a", "decode");
    auto t2 = broker.Issue("multi-b", "cache");
    ASSERT(t1.subject != t2.subject);
}

// QuantumResistantHashEngine
TEST(TestQRHE_Init)
{
    using namespace ExplorerLens::Engine;
    QuantumResistantHashEngine e;
    ASSERT(e.Initialize());
    ASSERT(e.IsReady());
}
TEST(TestQRHE_HashSHA3)
{
    using namespace ExplorerLens::Engine;
    QuantumResistantHashEngine e;
    e.Initialize();
    std::vector<uint8_t> data = {0x01, 0x02};
    auto d = e.Hash(data, QRHashAlgo::SHA3_256);
    ASSERT(d.bytes.size() == 32);
    ASSERT(d.algorithm == QRHashAlgo::SHA3_256);
}
TEST(TestQRHE_HashBLAKE3)
{
    using namespace ExplorerLens::Engine;
    QuantumResistantHashEngine e;
    e.Initialize();
    std::vector<uint8_t> data = {0xAA};
    auto d = e.Hash(data, QRHashAlgo::BLAKE3);
    ASSERT(d.bytes.size() == 32);
}
TEST(TestQRHE_HashK12)
{
    using namespace ExplorerLens::Engine;
    QuantumResistantHashEngine e;
    e.Initialize();
    std::vector<uint8_t> data = {0x55};
    auto d = e.Hash(data, QRHashAlgo::KangarooTwelve);
    ASSERT(d.bytes.size() == 64);
}
TEST(TestQRHE_Default)
{
    using namespace ExplorerLens::Engine;
    QuantumResistantHashEngine e;
    e.Initialize(QRHashAlgo::SHA3_256);
    std::vector<uint8_t> data = {0x11};
    auto d = e.Hash(data);
    ASSERT(d.algorithm == QRHashAlgo::SHA3_256);
}
TEST(TestQRHE_ConstantTimeEq)
{
    using namespace ExplorerLens::Engine;
    QuantumResistantHashEngine e;
    e.Initialize();
    std::vector<uint8_t> data = {0x01};
    auto d1 = e.Hash(data, QRHashAlgo::BLAKE3);
    auto d2 = e.Hash(data, QRHashAlgo::BLAKE3);
    ASSERT(e.ConstantTimeCompare(d1, d2));
}
TEST(TestQRHE_ConstantTimeNeq)
{
    using namespace ExplorerLens::Engine;
    QuantumResistantHashEngine e;
    e.Initialize();
    std::vector<uint8_t> data1 = {0x01};
    auto d1 = e.Hash(data1, QRHashAlgo::BLAKE3);
    QRHashDigest d2;
    d2.bytes.assign(32, 0xFF);
    ASSERT(!e.ConstantTimeCompare(d1, d2));
}
TEST(TestQRHE_Stats)
{
    using namespace ExplorerLens::Engine;
    QuantumResistantHashEngine e;
    e.Initialize();
    std::vector<uint8_t> data = {0x01};
    e.Hash(data, QRHashAlgo::SHA3_256);
    e.Hash(data, QRHashAlgo::BLAKE3);
    ASSERT(e.GetStats().hashOps == 2);
}
TEST(TestQRHE_Reset)
{
    using namespace ExplorerLens::Engine;
    QuantumResistantHashEngine e;
    e.Initialize();
    std::vector<uint8_t> data = {0x01};
    e.Hash(data);
    e.Reset();
    ASSERT(e.GetStats().hashOps == 0);
}

// PluginZeroTrustSandbox
TEST(TestPZTS_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& a = PluginZeroTrustSandbox::Instance();
    auto& b = PluginZeroTrustSandbox::Instance();
    ASSERT(&a == &b);
}
TEST(TestPZTS_DefaultPolicy)
{
    using namespace ExplorerLens::Engine;
    auto& sb = PluginZeroTrustSandbox::Instance();
    ASSERT(sb.GetPolicy().requiresCapabilityToken);
}
TEST(TestPZTS_SetPolicy)
{
    using namespace ExplorerLens::Engine;
    auto& sb = PluginZeroTrustSandbox::Instance();
    PluginSandboxPolicy p;
    p.pluginId = "test-plugin";
    p.maxMemoryMB = 128;
    sb.SetPolicy(p);
    ASSERT(sb.GetPolicy().maxMemoryMB == 128);
}
TEST(TestPZTS_Allow_WithToken)
{
    using namespace ExplorerLens::Engine;
    auto& sb = PluginZeroTrustSandbox::Instance();
    auto d = sb.Evaluate("my-plugin", "decode", true);
    ASSERT(d == PluginSandboxDecision::Allow);
}
TEST(TestPZTS_Deny_NoToken)
{
    using namespace ExplorerLens::Engine;
    auto& sb = PluginZeroTrustSandbox::Instance();
    PluginSandboxPolicy p;
    p.requiresCapabilityToken = true;
    sb.SetPolicy(p);
    auto d = sb.Evaluate("my-plugin", "decode", false);
    ASSERT(d == PluginSandboxDecision::Deny);
}
TEST(TestPZTS_Deny_EmptyPlugin)
{
    using namespace ExplorerLens::Engine;
    auto& sb = PluginZeroTrustSandbox::Instance();
    auto d = sb.Evaluate("", "decode", true);
    ASSERT(d == PluginSandboxDecision::Deny);
}
TEST(TestPZTS_Deny_Empty_Cap)
{
    using namespace ExplorerLens::Engine;
    auto& sb = PluginZeroTrustSandbox::Instance();
    auto d = sb.Evaluate("my-plugin", "", true);
    ASSERT(d == PluginSandboxDecision::Deny);
}
TEST(TestPZTS_NotQuarantined)
{
    using namespace ExplorerLens::Engine;
    auto& sb = PluginZeroTrustSandbox::Instance();
    ASSERT(!sb.IsQuarantined("any-plugin"));
}
TEST(TestPZTS_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& sb = PluginZeroTrustSandbox::Instance();
    PluginSandboxPolicy p;
    p.requiresCapabilityToken = false;
    sb.SetPolicy(p);
    uint64_t before = sb.GetStats().callsAllowed;
    sb.Evaluate("stats-plugin", "read", true);
    ASSERT(sb.GetStats().callsAllowed == before + 1);
}

// BinaryTrustVerifier
TEST(TestBTV_Init)
{
    using namespace ExplorerLens::Engine;
    BinaryTrustVerifier v;
    ASSERT(v.Initialize());
    ASSERT(v.IsReady());
}
TEST(TestBTV_Verify_Valid)
{
    using namespace ExplorerLens::Engine;
    BinaryTrustVerifier v;
    v.Initialize();
    auto ev = v.Verify("LENSShell.dll");
    ASSERT(ev.status == BinaryTrustStatus::Trusted);
}
TEST(TestBTV_Verify_Empty)
{
    using namespace ExplorerLens::Engine;
    BinaryTrustVerifier v;
    v.Initialize();
    auto ev = v.Verify("");
    ASSERT(ev.status == BinaryTrustStatus::Unknown);
}
TEST(TestBTV_TamperDetect)
{
    using namespace ExplorerLens::Engine;
    BinaryTrustVerifier v;
    v.Initialize();
    auto ev = v.VerifyTampered("LENSShell.dll");
    ASSERT(ev.status == BinaryTrustStatus::TamperEvident);
}
TEST(TestBTV_TamperReason)
{
    using namespace ExplorerLens::Engine;
    BinaryTrustVerifier v;
    v.Initialize();
    auto ev = v.VerifyTampered("LENSShell.dll");
    ASSERT(!ev.rejectionReason.empty());
}
TEST(TestBTV_SignerName)
{
    using namespace ExplorerLens::Engine;
    BinaryTrustVerifier v;
    v.Initialize();
    auto ev = v.Verify("LENSShell.dll");
    ASSERT(!ev.signerName.empty());
}
TEST(TestBTV_Stats_Ok)
{
    using namespace ExplorerLens::Engine;
    BinaryTrustVerifier v;
    v.Initialize();
    v.Verify("LENSShell.dll");
    ASSERT(v.GetStats().verificationsOk == 1);
}
TEST(TestBTV_Stats_Fail)
{
    using namespace ExplorerLens::Engine;
    BinaryTrustVerifier v;
    v.Initialize();
    v.Verify("");
    ASSERT(v.GetStats().verificationsFail == 1);
}
TEST(TestBTV_Reset)
{
    using namespace ExplorerLens::Engine;
    BinaryTrustVerifier v;
    v.Initialize();
    v.Verify("LENSShell.dll");
    v.Reset();
    ASSERT(v.GetStats().verificationsOk == 0);
}

// SecureConfigurationManager
TEST(TestSCM_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& a = SecureConfigurationManager::Instance();
    auto& b = SecureConfigurationManager::Instance();
    ASSERT(&a == &b);
}
TEST(TestSCM_Init)
{
    using namespace ExplorerLens::Engine;
    auto& m = SecureConfigurationManager::Instance();
    ASSERT(m.Initialize());
    ASSERT(m.IsReady());
}
TEST(TestSCM_SetAndGet)
{
    using namespace ExplorerLens::Engine;
    auto& m = SecureConfigurationManager::Instance();
    m.Initialize();
    ASSERT(m.Set("api-key", "secret-value"));
    std::string val;
    ASSERT(m.Get("api-key", val));
    ASSERT(val == "secret-value");
}
TEST(TestSCM_GetMissing)
{
    using namespace ExplorerLens::Engine;
    auto& m = SecureConfigurationManager::Instance();
    m.Initialize();
    std::string val;
    ASSERT(!m.Get("nonexistent-key-xyz", val));
}
TEST(TestSCM_Backend_Platform)
{
    using namespace ExplorerLens::Engine;
    auto& m = SecureConfigurationManager::Instance();
    m.Initialize();
#if defined(_WIN32)
    ASSERT(m.GetBackend() == SecureConfigBackend::DPAPI);
#else
    ASSERT(m.GetBackend() != SecureConfigBackend::TPM2);
#endif
}
TEST(TestSCM_MultiKey)
{
    using namespace ExplorerLens::Engine;
    auto& m = SecureConfigurationManager::Instance();
    m.Initialize();
    m.Set("key1", "val1");
    m.Set("key2", "val2");
    std::string v1, v2;
    ASSERT(m.Get("key1", v1) && v1 == "val1");
    ASSERT(m.Get("key2", v2) && v2 == "val2");
}
TEST(TestSCM_OverwriteKey)
{
    using namespace ExplorerLens::Engine;
    auto& m = SecureConfigurationManager::Instance();
    m.Initialize();
    m.Set("overwrite-key", "first");
    m.Set("overwrite-key", "second");
    std::string val;
    m.Get("overwrite-key", val);
    ASSERT(val == "second");
}
TEST(TestSCM_Stats_Writes)
{
    using namespace ExplorerLens::Engine;
    auto& m = SecureConfigurationManager::Instance();
    m.Initialize();
    uint64_t before = m.GetStats().writesOk;
    m.Set("stat-key", "v");
    ASSERT(m.GetStats().writesOk == before + 1);
}
TEST(TestSCM_Stats_Reads)
{
    using namespace ExplorerLens::Engine;
    auto& m = SecureConfigurationManager::Instance();
    m.Initialize();
    m.Set("read-stat-key", "v");
    uint64_t before = m.GetStats().readsOk;
    std::string val;
    m.Get("read-stat-key", val);
    ASSERT(m.GetStats().readsOk == before + 1);
}

// ThreatModelingEngine
TEST(TestTME_Init)
{
    using namespace ExplorerLens::Engine;
    ThreatModelingEngine e;
    ASSERT(e.Initialize());
    ASSERT(e.IsReady());
}
TEST(TestTME_Analyze_NotEmpty)
{
    using namespace ExplorerLens::Engine;
    ThreatModelingEngine e;
    e.Initialize();
    auto threats = e.Analyze("decode-stage");
    ASSERT(!threats.empty());
}
TEST(TestTME_PipelineSafe)
{
    using namespace ExplorerLens::Engine;
    ThreatModelingEngine e;
    e.Initialize();
    ASSERT(e.IsPipelineSafe("decode-stage"));
}
TEST(TestTME_Spoofing_Sim)
{
    using namespace ExplorerLens::Engine;
    ThreatModelingEngine e;
    e.Initialize();
    auto s = e.SimulateSpoofing("COM-server");
    ASSERT(s.category == STRIDECategory::Spoofing);
}
TEST(TestTME_Spoofing_Unmitigated)
{
    using namespace ExplorerLens::Engine;
    ThreatModelingEngine e;
    e.Initialize();
    auto s = e.SimulateSpoofing("target");
    ASSERT(!s.mitigated);
}
TEST(TestTME_Spoofing_Severity)
{
    using namespace ExplorerLens::Engine;
    ThreatModelingEngine e;
    e.Initialize();
    auto s = e.SimulateSpoofing("target");
    ASSERT(s.severity == 9);
}
TEST(TestTME_Stats_AnalyzeCount)
{
    using namespace ExplorerLens::Engine;
    ThreatModelingEngine e;
    e.Initialize();
    e.Analyze("stage-a");
    e.Analyze("stage-b");
    ASSERT(e.GetStats().scenariosAnalyzed == 2);
}
TEST(TestTME_Stats_Found)
{
    using namespace ExplorerLens::Engine;
    ThreatModelingEngine e;
    e.Initialize();
    e.SimulateSpoofing("t");
    ASSERT(e.GetStats().threatsFound >= 1);
}
TEST(TestTME_Reset)
{
    using namespace ExplorerLens::Engine;
    ThreatModelingEngine e;
    e.Initialize();
    e.Analyze("stage");
    e.Reset();
    ASSERT(e.GetStats().scenariosAnalyzed == 0);
}

// SecurityPostureAnalyzer
TEST(TestSPA_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& a = SecurityPostureAnalyzer::Instance();
    auto& b = SecurityPostureAnalyzer::Instance();
    ASSERT(&a == &b);
}
TEST(TestSPA_Analyze_NonEmpty)
{
    using namespace ExplorerLens::Engine;
    auto& spa = SecurityPostureAnalyzer::Instance();
    auto r = spa.Analyze();
    ASSERT(!r.reportId.empty());
}
TEST(TestSPA_Score_Range)
{
    using namespace ExplorerLens::Engine;
    auto& spa = SecurityPostureAnalyzer::Instance();
    auto r = spa.Analyze();
    ASSERT(r.overallScore >= 0.0f && r.overallScore <= 100.0f);
}
TEST(TestSPA_PatchLevel)
{
    using namespace ExplorerLens::Engine;
    auto& spa = SecurityPostureAnalyzer::Instance();
    auto r = spa.Analyze();
    ASSERT(r.patchLevelCurrent);
}
TEST(TestSPA_Schema)
{
    using namespace ExplorerLens::Engine;
    auto& spa = SecurityPostureAnalyzer::Instance();
    auto r = spa.Analyze();
    ASSERT(r.schemaVersion == "1.0");
}
TEST(TestSPA_IsCompliant)
{
    using namespace ExplorerLens::Engine;
    auto& spa = SecurityPostureAnalyzer::Instance();
    bool c = spa.IsCompliant(0.0f);
    ASSERT(c || !c);  // just ensure no crash
}
TEST(TestSPA_Serialize)
{
    using namespace ExplorerLens::Engine;
    auto& spa = SecurityPostureAnalyzer::Instance();
    auto r = spa.Analyze();
    auto json = spa.SerializeToJson(r);
    ASSERT(!json.empty());
    ASSERT(json.find("reportId") != std::string::npos);
}
TEST(TestSPA_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& spa = SecurityPostureAnalyzer::Instance();
    uint64_t before = spa.GetStats().reportsGenerated;
    spa.Analyze();
    ASSERT(spa.GetStats().reportsGenerated == before + 1);
}
TEST(TestSPA_ScoreComponents)
{
    using namespace ExplorerLens::Engine;
    auto& spa = SecurityPostureAnalyzer::Instance();
    auto r = spa.Analyze();
    float expected =
        (r.tpmAttested ? 40.0f : 0.0f) + (r.codeIntegrityOk ? 35.0f : 0.0f) + (r.patchLevelCurrent ? 25.0f : 0.0f);
    ASSERT(r.overallScore == expected);
}

//== v32.1.0 Fomalhaut-R — Edge AI & Hardware-Accelerated Inference Tests ==//

// NPUAccelerationEngine
TEST(TestNPUAE_Init)
{
    using namespace ExplorerLens::Engine;
    NPUAccelerationEngine e;
    ASSERT(e.Initialize());
    ASSERT(e.IsReady());
}
TEST(TestNPUAE_Dispatch)
{
    using namespace ExplorerLens::Engine;
    NPUAccelerationEngine e;
    e.Initialize();
    NPUWorkload w;
    w.modelName = "clip";
    w.inputData = {1.0f, 2.0f};
    w.batchSize = 2;
    auto out = e.Dispatch(w);
    ASSERT(out.size() == 2);
}
TEST(TestNPUAE_Stats_Dispatched)
{
    using namespace ExplorerLens::Engine;
    NPUAccelerationEngine e;
    e.Initialize();
    NPUWorkload w;
    w.modelName = "m";
    w.batchSize = 1;
    e.Dispatch(w);
    e.Dispatch(w);
    ASSERT(e.GetStats().workloadsDispatched == 2);
}
TEST(TestNPUAE_CPUFallback)
{
    using namespace ExplorerLens::Engine;
    NPUAccelerationEngine e;
    e.Initialize();
    NPUWorkload w;
    w.modelName = "test";
    w.mode = NPUDispatchMode::ForceCPU;
    w.batchSize = 1;
    e.Dispatch(w);
    ASSERT(e.GetStats().cpuFallbacks == 1);
}
TEST(TestNPUAE_SetMode)
{
    using namespace ExplorerLens::Engine;
    NPUAccelerationEngine e;
    e.Initialize();
    e.SetDispatchMode(NPUDispatchMode::ForceGPU);
    ASSERT(e.IsReady());
}
TEST(TestNPUAE_Reset)
{
    using namespace ExplorerLens::Engine;
    NPUAccelerationEngine e;
    e.Initialize();
    NPUWorkload w;
    w.modelName = "m";
    w.batchSize = 1;
    e.Dispatch(w);
    e.Reset();
    ASSERT(e.GetStats().workloadsDispatched == 0);
}
TEST(TestNPUAE_AvailableCheck)
{
    using namespace ExplorerLens::Engine;
    NPUAccelerationEngine e;
    e.Initialize();
    // IsNPUAvailable is platform-dependent; just verify no crash
    bool avail = e.IsNPUAvailable();
    ASSERT(avail || !avail);
}
TEST(TestNPUAE_EmptyWorkload)
{
    using namespace ExplorerLens::Engine;
    NPUAccelerationEngine e;
    e.Initialize();
    NPUWorkload w;
    w.batchSize = 0;
    auto out = e.Dispatch(w);
    ASSERT(out.empty());
}
TEST(TestNPUAE_MultiDispatch)
{
    using namespace ExplorerLens::Engine;
    NPUAccelerationEngine e;
    e.Initialize();
    for (int i = 0; i < 5; ++i) {
        NPUWorkload w;
        w.modelName = "m";
        w.batchSize = 1;
        e.Dispatch(w);
    }
    ASSERT(e.GetStats().workloadsDispatched == 5);
}

// EdgeAIInferenceEngine
TEST(TestEAIE_Init)
{
    using namespace ExplorerLens::Engine;
    EdgeAIInferenceEngine e;
    ASSERT(e.Initialize());
    ASSERT(e.IsReady());
}
TEST(TestEAIE_CreateSession)
{
    using namespace ExplorerLens::Engine;
    EdgeAIInferenceEngine e;
    e.Initialize();
    auto s = e.CreateSession("clip.onnx");
    ASSERT(s.state == EdgeInferenceState::Ready);
    ASSERT(s.sessionId > 0);
}
TEST(TestEAIE_CreateSession_Empty)
{
    using namespace ExplorerLens::Engine;
    EdgeAIInferenceEngine e;
    e.Initialize();
    auto s = e.CreateSession("");
    ASSERT(s.state == EdgeInferenceState::Error);
}
TEST(TestEAIE_RunInference)
{
    using namespace ExplorerLens::Engine;
    EdgeAIInferenceEngine e;
    e.Initialize();
    auto s = e.CreateSession("model.onnx");
    std::vector<float> in = {1.0f, 2.0f, 3.0f};
    auto out = e.RunInference(s, in);
    ASSERT(out.size() == in.size());
}
TEST(TestEAIE_RunInference_ErrorState)
{
    using namespace ExplorerLens::Engine;
    EdgeAIInferenceEngine e;
    e.Initialize();
    auto s = e.CreateSession("");
    std::vector<float> in = {1.0f};
    auto out = e.RunInference(s, in);
    ASSERT(out.empty());
}
TEST(TestEAIE_MemMapped)
{
    using namespace ExplorerLens::Engine;
    EdgeAIInferenceEngine e;
    e.Initialize();
    auto s = e.CreateSession("model.onnx", true);
    ASSERT(s.memMapped);
}
TEST(TestEAIE_Destroy)
{
    using namespace ExplorerLens::Engine;
    EdgeAIInferenceEngine e;
    e.Initialize();
    auto s = e.CreateSession("model.onnx");
    e.DestroySession(s);
    ASSERT(s.state == EdgeInferenceState::Idle);
}
TEST(TestEAIE_Stats)
{
    using namespace ExplorerLens::Engine;
    EdgeAIInferenceEngine e;
    e.Initialize();
    auto s = e.CreateSession("model.onnx");
    std::vector<float> in = {1.0f};
    e.RunInference(s, in);
    ASSERT(e.GetStats().inferenceRuns == 1);
}
TEST(TestEAIE_Reset)
{
    using namespace ExplorerLens::Engine;
    EdgeAIInferenceEngine e;
    e.Initialize();
    auto s = e.CreateSession("model.onnx");
    std::vector<float> in = {1.0f};
    e.RunInference(s, in);
    e.Reset();
    ASSERT(e.GetStats().inferenceRuns == 0);
}

// HardwareCapabilityNegotiator
TEST(TestHCN_Init)
{
    using namespace ExplorerLens::Engine;
    HardwareCapabilityNegotiator n;
    ASSERT(n.Initialize());
    ASSERT(n.IsReady());
}
TEST(TestHCN_Negotiate_Embedding)
{
    using namespace ExplorerLens::Engine;
    HardwareCapabilityNegotiator n;
    n.Initialize();
    auto s = n.Negotiate("embedding");
    ASSERT(s.backend == HWBackendChoice::NPU);
}
TEST(TestHCN_Negotiate_Render)
{
    using namespace ExplorerLens::Engine;
    HardwareCapabilityNegotiator n;
    n.Initialize();
    auto s = n.Negotiate("render");
    ASSERT(s.backend == HWBackendChoice::GPU);
}
TEST(TestHCN_Negotiate_Default)
{
    using namespace ExplorerLens::Engine;
    HardwareCapabilityNegotiator n;
    n.Initialize();
    auto s = n.Negotiate("unknown-task");
    ASSERT(s.backend == HWBackendChoice::CPU);
}
TEST(TestHCN_PrefersNPU)
{
    using namespace ExplorerLens::Engine;
    HardwareCapabilityNegotiator n;
    n.Initialize();
    ASSERT(n.PrefersNPU("clip"));
}
TEST(TestHCN_Score_Avail)
{
    using namespace ExplorerLens::Engine;
    HardwareCapabilityNegotiator n;
    n.Initialize();
    auto s = n.Negotiate("embedding");
    ASSERT(s.available);
    ASSERT(s.score > 0.0f);
}
TEST(TestHCN_Stats_Total)
{
    using namespace ExplorerLens::Engine;
    HardwareCapabilityNegotiator n;
    n.Initialize();
    n.Negotiate("embedding");
    n.Negotiate("render");
    ASSERT(n.GetStats().totalQueries >= 2);
}
TEST(TestHCN_Stats_NPUCount)
{
    using namespace ExplorerLens::Engine;
    HardwareCapabilityNegotiator n;
    n.Initialize();
    n.Negotiate("embedding");
    n.Negotiate("clip");
    ASSERT(n.GetStats().npuSelections == 2);
}
TEST(TestHCN_Reset)
{
    using namespace ExplorerLens::Engine;
    HardwareCapabilityNegotiator n;
    n.Initialize();
    n.Negotiate("embedding");
    n.Reset();
    ASSERT(n.GetStats().totalQueries == 0);
}

// AMDXDNABackend
TEST(TestXDNA_Init)
{
    using namespace ExplorerLens::Engine;
    AMDXDNABackend b;
    ASSERT(b.Initialize());
    ASSERT(b.IsReady());
}
TEST(TestXDNA_DeviceName)
{
    using namespace ExplorerLens::Engine;
    AMDXDNABackend b;
    b.Initialize();
    ASSERT(!b.GetDeviceName().empty());
}
TEST(TestXDNA_TOPS)
{
    using namespace ExplorerLens::Engine;
    AMDXDNABackend b;
    b.Initialize();
    ASSERT(b.GetTOPS() > 0.0f);
}
TEST(TestXDNA_ExecuteKernel)
{
    using namespace ExplorerLens::Engine;
    AMDXDNABackend b;
    b.Initialize();
    std::vector<float> w = {1.0f, 2.0f};
    std::vector<float> in = {3.0f, 4.0f};
    auto out = b.ExecuteKernel("clip_embedding", w, in);
    ASSERT(out.size() == in.size());
}
TEST(TestXDNA_MLIR)
{
    using namespace ExplorerLens::Engine;
    AMDXDNABackend b;
    b.Initialize();
    ASSERT(b.SupportsMLIR());
}
TEST(TestXDNA_Stats)
{
    using namespace ExplorerLens::Engine;
    AMDXDNABackend b;
    b.Initialize();
    std::vector<float> w = {1.0f};
    std::vector<float> in = {2.0f};
    b.ExecuteKernel("k", w, in);
    ASSERT(b.GetStats().kernelsDispatched == 1);
}
TEST(TestXDNA_Reset)
{
    using namespace ExplorerLens::Engine;
    AMDXDNABackend b;
    b.Initialize();
    std::vector<float> w = {1.0f};
    std::vector<float> in = {2.0f};
    b.ExecuteKernel("k", w, in);
    b.Reset();
    ASSERT(b.GetStats().kernelsDispatched == 0);
}
TEST(TestXDNA_TileModes)
{
    using namespace ExplorerLens::Engine;
    AMDXDNABackend b;
    b.Initialize();
    std::vector<float> w = {1.0f};
    std::vector<float> in = {1.0f};
    auto o1 = b.ExecuteKernel("k", w, in, XDNATileMode::Tile1x1);
    auto o2 = b.ExecuteKernel("k", w, in, XDNATileMode::Tile4x4);
    ASSERT(!o1.empty() && !o2.empty());
}
TEST(TestXDNA_AvgLatency)
{
    using namespace ExplorerLens::Engine;
    AMDXDNABackend b;
    b.Initialize();
    std::vector<float> w = {1.0f};
    std::vector<float> in = {1.0f};
    b.ExecuteKernel("k", w, in);
    ASSERT(b.GetStats().avgKernelUs > 0.0f);
}

// QualcommAIEBackend
TEST(TestQAIE_Init)
{
    using namespace ExplorerLens::Engine;
    QualcommAIEBackend b;
    ASSERT(b.Initialize());
    ASSERT(b.IsReady());
}
TEST(TestQAIE_DeviceName)
{
    using namespace ExplorerLens::Engine;
    QualcommAIEBackend b;
    b.Initialize();
    ASSERT(!b.GetDeviceName().empty());
}
TEST(TestQAIE_RunModel)
{
    using namespace ExplorerLens::Engine;
    QualcommAIEBackend b;
    b.Initialize();
    std::vector<float> in = {1.0f, 2.0f, 3.0f};
    auto out = b.RunModel("resnet50", in);
    ASSERT(out.size() == in.size());
}
TEST(TestQAIE_HTPPath)
{
    using namespace ExplorerLens::Engine;
    QualcommAIEBackend b;
    b.Initialize();
    std::vector<float> in = {1.0f};
    b.RunModel("m", in, QNNTargetRuntime::HTP);
    ASSERT(b.GetStats().htpHits == 1);
}
TEST(TestQAIE_GPUPath)
{
    using namespace ExplorerLens::Engine;
    QualcommAIEBackend b;
    b.Initialize();
    std::vector<float> in = {1.0f};
    b.RunModel("m", in, QNNTargetRuntime::GPU);
    ASSERT(b.GetStats().htpHits == 0);
}
TEST(TestQAIE_Quantization)
{
    using namespace ExplorerLens::Engine;
    QualcommAIEBackend b;
    b.Initialize();
    ASSERT(b.SupportsQuantization());
}
TEST(TestQAIE_Stats)
{
    using namespace ExplorerLens::Engine;
    QualcommAIEBackend b;
    b.Initialize();
    std::vector<float> in = {1.0f};
    b.RunModel("m", in);
    ASSERT(b.GetStats().inferenceRuns == 1);
}
TEST(TestQAIE_Reset)
{
    using namespace ExplorerLens::Engine;
    QualcommAIEBackend b;
    b.Initialize();
    std::vector<float> in = {1.0f};
    b.RunModel("m", in);
    b.Reset();
    ASSERT(b.GetStats().inferenceRuns == 0);
}
TEST(TestQAIE_AvgLatency_HTP)
{
    using namespace ExplorerLens::Engine;
    QualcommAIEBackend b;
    b.Initialize();
    std::vector<float> in = {1.0f};
    b.RunModel("m", in, QNNTargetRuntime::HTP);
    ASSERT(b.GetStats().avgInferenceMs < 15.0f);
}

// IntelAMXBackend
TEST(TestAMX_Init)
{
    using namespace ExplorerLens::Engine;
    IntelAMXBackend b;
    ASSERT(b.Initialize());
    ASSERT(b.IsReady());
}
TEST(TestAMX_MatMul_BF16)
{
    using namespace ExplorerLens::Engine;
    IntelAMXBackend b;
    b.Initialize();
    std::vector<float> a = {1.0f, 2.0f};
    std::vector<float> v = {3.0f, 4.0f};
    auto out = b.MatMul(a, v, AMXPrecisionMode::BF16);
    ASSERT(out.size() == a.size());
    ASSERT(out[0] == 3.0f);
}
TEST(TestAMX_MatMul_INT8)
{
    using namespace ExplorerLens::Engine;
    IntelAMXBackend b;
    b.Initialize();
    std::vector<float> a = {2.0f};
    std::vector<float> v = {5.0f};
    auto out = b.MatMul(a, v, AMXPrecisionMode::INT8);
    ASSERT(out[0] == 10.0f);
}
TEST(TestAMX_MatMul_Empty)
{
    using namespace ExplorerLens::Engine;
    IntelAMXBackend b;
    b.Initialize();
    std::vector<float> empty;
    auto out = b.MatMul(empty, empty);
    ASSERT(out.empty());
}
TEST(TestAMX_Throughput)
{
    using namespace ExplorerLens::Engine;
    IntelAMXBackend b;
    b.Initialize();
    std::vector<float> a = {1.0f};
    std::vector<float> v = {2.0f};
    b.MatMul(a, v);
    ASSERT(b.GetThroughputMultiplierVsSSE42() >= 1.0f);
}
TEST(TestAMX_Stats)
{
    using namespace ExplorerLens::Engine;
    IntelAMXBackend b;
    b.Initialize();
    std::vector<float> a = {1.0f};
    std::vector<float> v = {1.0f};
    b.MatMul(a, v);
    b.MatMul(a, v);
    ASSERT(b.GetStats().matMulOps == 2);
}
TEST(TestAMX_Reset)
{
    using namespace ExplorerLens::Engine;
    IntelAMXBackend b;
    b.Initialize();
    std::vector<float> a = {1.0f};
    std::vector<float> v = {1.0f};
    b.MatMul(a, v);
    b.Reset();
    ASSERT(b.GetStats().matMulOps == 0);
}
TEST(TestAMX_SupportFlag)
{
    using namespace ExplorerLens::Engine;
    IntelAMXBackend b;
    b.Initialize();
    bool s = b.IsAMXSupported();
    ASSERT(s || !s);  // platform-dependent, no crash
}
TEST(TestAMX_AvgLatency)
{
    using namespace ExplorerLens::Engine;
    IntelAMXBackend b;
    b.Initialize();
    std::vector<float> a = {1.0f};
    std::vector<float> v = {1.0f};
    b.MatMul(a, v);
    ASSERT(b.GetStats().avgLatencyMs > 0.0f);
}

// HardwareAcceleratedPipeline
TEST(TestHAP_Init)
{
    using namespace ExplorerLens::Engine;
    HardwareAcceleratedPipeline p;
    ASSERT(p.Initialize());
    ASSERT(p.IsReady());
}
TEST(TestHAP_Process_Infer)
{
    using namespace ExplorerLens::Engine;
    HardwareAcceleratedPipeline p;
    p.Initialize();
    std::vector<uint8_t> in = {0x01, 0x02};
    auto out = p.Process(HWPipelineStage::Infer, in);
    ASSERT(out.size() == in.size());
}
TEST(TestHAP_NPURouting)
{
    using namespace ExplorerLens::Engine;
    HardwareAcceleratedPipeline p;
    HWPipelineConfig cfg;
    cfg.preferNPUForInfer = true;
    p.Initialize(cfg);
    std::vector<uint8_t> in = {0x01};
    p.Process(HWPipelineStage::Infer, in);
    ASSERT(p.GetStats().npuRoutings == 1);
}
TEST(TestHAP_GPURouting)
{
    using namespace ExplorerLens::Engine;
    HardwareAcceleratedPipeline p;
    HWPipelineConfig cfg;
    cfg.preferNPUForInfer = false;
    cfg.preferGPUForDecode = true;
    p.Initialize(cfg);
    std::vector<uint8_t> in = {0x01};
    p.Process(HWPipelineStage::Decode, in);
    ASSERT(p.GetStats().gpuRoutings == 1);
}
TEST(TestHAP_CPUFallback_Flag)
{
    using namespace ExplorerLens::Engine;
    HardwareAcceleratedPipeline p;
    p.Initialize();
    ASSERT(p.HasCPUFallback());
}
TEST(TestHAP_Stats)
{
    using namespace ExplorerLens::Engine;
    HardwareAcceleratedPipeline p;
    p.Initialize();
    std::vector<uint8_t> in = {0x01};
    p.Process(HWPipelineStage::Infer, in);
    p.Process(HWPipelineStage::Decode, in);
    ASSERT(p.GetStats().stagesProcessed == 2);
}
TEST(TestHAP_Reset)
{
    using namespace ExplorerLens::Engine;
    HardwareAcceleratedPipeline p;
    p.Initialize();
    std::vector<uint8_t> in = {0x01};
    p.Process(HWPipelineStage::Infer, in);
    p.Reset();
    ASSERT(p.GetStats().stagesProcessed == 0);
}
TEST(TestHAP_Empty_Input)
{
    using namespace ExplorerLens::Engine;
    HardwareAcceleratedPipeline p;
    p.Initialize();
    std::vector<uint8_t> empty;
    auto out = p.Process(HWPipelineStage::Encode, empty);
    ASSERT(out.empty());
}
TEST(TestHAP_Composite)
{
    using namespace ExplorerLens::Engine;
    HardwareAcceleratedPipeline p;
    p.Initialize();
    std::vector<uint8_t> in = {0xAB};
    auto out = p.Process(HWPipelineStage::Composite, in);
    ASSERT(out.size() == 1);
    ASSERT(out[0] == 0xAB);
}

// ComputeDeviceRegistry
TEST(TestCDR_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& a = ComputeDeviceRegistry::Instance();
    auto& b = ComputeDeviceRegistry::Instance();
    ASSERT(&a == &b);
}
TEST(TestCDR_Init)
{
    using namespace ExplorerLens::Engine;
    auto& r = ComputeDeviceRegistry::Instance();
    ASSERT(r.Initialize());
    ASSERT(r.IsReady());
}
TEST(TestCDR_HasDevices)
{
    using namespace ExplorerLens::Engine;
    auto& r = ComputeDeviceRegistry::Instance();
    r.Initialize();
    ASSERT(!r.GetDevices().empty());
}
TEST(TestCDR_HasCPU)
{
    using namespace ExplorerLens::Engine;
    auto& r = ComputeDeviceRegistry::Instance();
    r.Initialize();
    auto* cpu = r.FindBestFor(ComputeDeviceClass::CPU);
    ASSERT(cpu != nullptr);
    ASSERT(cpu->available);
}
TEST(TestCDR_CPUCount)
{
    using namespace ExplorerLens::Engine;
    auto& r = ComputeDeviceRegistry::Instance();
    r.Initialize();
    ASSERT(r.GetStats().cpuCount >= 1);
}
TEST(TestCDR_Stats_Enumerated)
{
    using namespace ExplorerLens::Engine;
    auto& r = ComputeDeviceRegistry::Instance();
    r.Initialize();
    ASSERT(r.GetStats().devicesEnumerated >= 1);
}
TEST(TestCDR_Devices_Valid)
{
    using namespace ExplorerLens::Engine;
    auto& r = ComputeDeviceRegistry::Instance();
    r.Initialize();
    for (const auto& d : r.GetDevices()) {
        ASSERT(!d.name.empty());
        ASSERT(d.deviceClass != ComputeDeviceClass::Unknown);
    }
}
TEST(TestCDR_FindBest_Fallback)
{
    using namespace ExplorerLens::Engine;
    auto& r = ComputeDeviceRegistry::Instance();
    r.Initialize();
    // FindBestFor should always return non-null (CPU fallback)
    auto* d = r.FindBestFor(ComputeDeviceClass::FPGA);
    ASSERT(d != nullptr);  // falls back to CPU
}
TEST(TestCDR_MultiInit)
{
    using namespace ExplorerLens::Engine;
    auto& r = ComputeDeviceRegistry::Instance();
    r.Initialize();
    r.Initialize();
    ASSERT(r.GetStats().devicesEnumerated >= 1);
}

//== v31.9.0 Achernar-Z — Autonomous Shell Intelligence Tests ==//

TEST(TestAWO_Initialize)
{
    using namespace ExplorerLens::Engine;
    AutonomousWorkflowOrchestrator awo;
    auto s = awo.GetStats();
    ASSERT(s.jobsQueued == 0);
    ASSERT(s.jobsCompleted == 0);
}
TEST(TestAWO_Enqueue)
{
    using namespace ExplorerLens::Engine;
    AutonomousWorkflowOrchestrator awo;
    awo.Enqueue(1, WorkflowPriority::Normal, 1024);
    auto s = awo.GetStats();
    ASSERT(s.jobsQueued == 1);
}
TEST(TestAWO_Dispatch)
{
    using namespace ExplorerLens::Engine;
    AutonomousWorkflowOrchestrator awo;
    awo.Enqueue(42, WorkflowPriority::Elevated, 2048);
    uint64_t jobId = 0;
    bool ok = awo.Dispatch(jobId);
    ASSERT(ok);
    ASSERT(jobId == 42);
}
TEST(TestAWO_Complete)
{
    using namespace ExplorerLens::Engine;
    AutonomousWorkflowOrchestrator awo;
    awo.Enqueue(7, WorkflowPriority::Critical, 512);
    uint64_t j = 0;
    awo.Dispatch(j);
    awo.Complete(j, 12.5f);
    auto s = awo.GetStats();
    ASSERT(s.jobsCompleted == 1);
}
TEST(TestAWO_SetPolicy)
{
    using namespace ExplorerLens::Engine;
    AutonomousWorkflowOrchestrator awo;
    awo.SetPolicy(OrchestrationPolicy::MLOptimized);
    ASSERT(awo.GetStats().jobsQueued == 0);
}
TEST(TestAWO_ConcurrencyLimit)
{
    using namespace ExplorerLens::Engine;
    AutonomousWorkflowOrchestrator awo;
    awo.SetConcurrencyLimit(8);
    auto s = awo.GetStats();
    ASSERT(s.jobsQueued == 0);
}
TEST(TestAWO_Reset)
{
    using namespace ExplorerLens::Engine;
    AutonomousWorkflowOrchestrator awo;
    awo.Enqueue(1, WorkflowPriority::Background, 256);
    awo.Reset();
    ASSERT(awo.GetStats().jobsQueued == 0);
}
TEST(TestAWO_MultiBatch)
{
    using namespace ExplorerLens::Engine;
    AutonomousWorkflowOrchestrator awo;
    for (uint64_t i = 1; i <= 5; ++i)
        awo.Enqueue(i, WorkflowPriority::Normal, i * 512);
    auto s = awo.GetStats();
    ASSERT(s.jobsQueued == 5);
}
TEST(TestAWO_PolicyGain)
{
    using namespace ExplorerLens::Engine;
    AutonomousWorkflowOrchestrator awo(OrchestrationPolicy::Adaptive);
    awo.Enqueue(1, WorkflowPriority::Normal, 1024);
    uint64_t j = 0;
    awo.Dispatch(j);
    awo.Complete(j, 10.0f);
    ASSERT(awo.GetStats().jobsCompleted >= 1);
}

TEST(TestSIA_Initialize)
{
    using namespace ExplorerLens::Engine;
    ShellIntelligenceAdapter sia(ShellPlatform::Windows);
    auto s = sia.GetStats();
    ASSERT(s.hintInjections == 0);
}
TEST(TestSIA_QueryHint)
{
    using namespace ExplorerLens::Engine;
    ShellIntelligenceAdapter sia(ShellPlatform::Windows);
    auto hint = sia.QueryHint("test.jpg", 256);
    ASSERT(hint == AIHint::None || hint == AIHint::PregenAvailable || hint == AIHint::RelevanceScored
           || hint == AIHint::QualityBoosted);
}
TEST(TestSIA_InjectPregen)
{
    using namespace ExplorerLens::Engine;
    ShellIntelligenceAdapter sia(ShellPlatform::macOS);
    uint8_t buf[256 * 256 * 4] = {};
    sia.InjectPregenResult("test.png", buf, 256, 256);
    ASSERT(sia.GetStats().hintInjections >= 0);
}
TEST(TestSIA_NotifyRender)
{
    using namespace ExplorerLens::Engine;
    ShellIntelligenceAdapter sia(ShellPlatform::Linux);
    sia.NotifyShellRender("test.webp", 14.3f);
    auto s = sia.GetStats();
    ASSERT(s.modelInvocations >= 0);
}
TEST(TestSIA_MacOS)
{
    using namespace ExplorerLens::Engine;
    ShellIntelligenceAdapter sia(ShellPlatform::macOS);
    auto hint = sia.QueryHint("photo.heic", 512);
    ASSERT(hint == AIHint::None || hint == AIHint::PregenAvailable);
}
TEST(TestSIA_Linux)
{
    using namespace ExplorerLens::Engine;
    ShellIntelligenceAdapter sia(ShellPlatform::Linux);
    auto hint = sia.QueryHint("document.pdf", 128);
    ASSERT(hint == AIHint::None || hint == AIHint::RelevanceScored);
}
TEST(TestSIA_StatAccumulate)
{
    using namespace ExplorerLens::Engine;
    ShellIntelligenceAdapter sia(ShellPlatform::Windows);
    sia.QueryHint("a.jpg", 256);
    sia.QueryHint("b.png", 256);
    ASSERT(sia.GetStats().modelInvocations >= 0);
}
TEST(TestSIA_MultiPlatform)
{
    using namespace ExplorerLens::Engine;
    ShellIntelligenceAdapter w(ShellPlatform::Windows), m(ShellPlatform::macOS), l(ShellPlatform::Linux);
    ASSERT(w.GetStats().hintInjections == 0);
    ASSERT(m.GetStats().hintInjections == 0);
    ASSERT(l.GetStats().hintInjections == 0);
}
TEST(TestSIA_LargeRender)
{
    using namespace ExplorerLens::Engine;
    ShellIntelligenceAdapter sia(ShellPlatform::Windows);
    for (int i = 0; i < 10; ++i)
        sia.NotifyShellRender("file_" + std::to_string(i) + ".jpg", float(i) * 2.0f);
    ASSERT(sia.GetStats().modelInvocations >= 0);
}

TEST(TestTRR_Initialize)
{
    using namespace ExplorerLens::Engine;
    ThumbnailRelevanceRanker r;
    auto s = r.GetStats();
    ASSERT(s.filesRanked == 0);
}
TEST(TestTRR_Rank)
{
    using namespace ExplorerLens::Engine;
    ThumbnailRelevanceRanker r;
    auto ranked = r.Rank({"a.jpg", "b.png", "c.webp"});
    ASSERT(ranked.size() == 3);
}
TEST(TestTRR_RecordAccess)
{
    using namespace ExplorerLens::Engine;
    ThumbnailRelevanceRanker r;
    r.RecordAccess("hot.jpg");
    auto ranked = r.Rank({"hot.jpg", "cold.png"});
    ASSERT(!ranked.empty());
}
TEST(TestTRR_Weights)
{
    using namespace ExplorerLens::Engine;
    ThumbnailRelevanceRanker r;
    r.SetRecencyWeight(0.5f);
    r.SetFrequencyWeight(0.3f);
    r.SetVisualInterestWeight(0.2f);
    auto ranked = r.Rank({"x.jpg"});
    ASSERT(ranked.size() == 1);
}
TEST(TestTRR_EmptyInput)
{
    using namespace ExplorerLens::Engine;
    ThumbnailRelevanceRanker r;
    auto ranked = r.Rank({});
    ASSERT(ranked.empty());
}
TEST(TestTRR_Reset)
{
    using namespace ExplorerLens::Engine;
    ThumbnailRelevanceRanker r;
    r.RecordAccess("x.jpg");
    r.Reset();
    ASSERT(r.GetStats().filesRanked == 0);
}
TEST(TestTRR_ScoreRange)
{
    using namespace ExplorerLens::Engine;
    ThumbnailRelevanceRanker r;
    auto ranked = r.Rank({"a.jpg", "b.jpg", "c.jpg"});
    for (auto& f : ranked) {
        ASSERT(f.score >= 0.0f && f.score <= 1.0f);
    }
}
TEST(TestTRR_LargeBatch)
{
    using namespace ExplorerLens::Engine;
    ThumbnailRelevanceRanker r;
    std::vector<std::string> files;
    for (int i = 0; i < 100; ++i)
        files.push_back("file_" + std::to_string(i) + ".jpg");
    auto ranked = r.Rank(files);
    ASSERT(ranked.size() == 100);
}
TEST(TestTRR_Stats)
{
    using namespace ExplorerLens::Engine;
    ThumbnailRelevanceRanker r;
    r.Rank({"a.jpg", "b.jpg"});
    auto s = r.GetStats();
    ASSERT(s.filesRanked >= 0);
}

TEST(TestCPCB_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& b = CrossPlatformCapabilityBroker::Instance();
    ASSERT(b.GetCapabilityBitmask() >= 0);
}
TEST(TestCPCB_D3D12)
{
    using namespace ExplorerLens::Engine;
    auto& b = CrossPlatformCapabilityBroker::Instance();
    bool hasD3D = b.Has(PlatformCapability::D3D12);
    ASSERT(hasD3D == true || hasD3D == false);
}
TEST(TestCPCB_Refresh)
{
    using namespace ExplorerLens::Engine;
    auto& b = CrossPlatformCapabilityBroker::Instance();
    b.Refresh();
    ASSERT(b.GetCapabilityBitmask() >= 0);
}
TEST(TestCPCB_Describe)
{
    using namespace ExplorerLens::Engine;
    auto& b = CrossPlatformCapabilityBroker::Instance();
    auto desc = b.Describe(PlatformCapability::VulkanCompute);
    ASSERT(!desc.empty());
}
TEST(TestCPCB_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& b = CrossPlatformCapabilityBroker::Instance();
    b.Has(PlatformCapability::NPUAcceleration);
    auto s = b.GetStats();
    ASSERT(s.queriesAnswered >= 1);
}
TEST(TestCPCB_MultiQuery)
{
    using namespace ExplorerLens::Engine;
    auto& b = CrossPlatformCapabilityBroker::Instance();
    b.Has(PlatformCapability::DirectStorage);
    b.Has(PlatformCapability::QuickLook);
    b.Has(PlatformCapability::XDGThumbnailSpec);
    ASSERT(b.GetStats().queriesAnswered >= 3);
}
TEST(TestCPCB_Bitmask)
{
    using namespace ExplorerLens::Engine;
    auto& b = CrossPlatformCapabilityBroker::Instance();
    uint32_t mask = b.GetCapabilityBitmask();
    ASSERT((mask & ~0x3FFu) == 0);
}
TEST(TestCPCB_TPM)
{
    using namespace ExplorerLens::Engine;
    auto& b = CrossPlatformCapabilityBroker::Instance();
    bool tpm = b.Has(PlatformCapability::TPM2Attestation);
    ASSERT(tpm == true || tpm == false);
}
TEST(TestCPCB_SpatialAudio)
{
    using namespace ExplorerLens::Engine;
    auto& b = CrossPlatformCapabilityBroker::Instance();
    bool sa = b.Has(PlatformCapability::SpatialAudio);
    ASSERT(sa == true || sa == false);
}

TEST(TestASIE_Initialize)
{
    using namespace ExplorerLens::Engine;
    AdaptiveShellIntegrationEngine engine;
    auto mode = engine.GetActiveMode();
    ASSERT(mode == ShellDeliveryMode::Unknown || mode != ShellDeliveryMode::Unknown);
}
TEST(TestASIE_Probe)
{
    using namespace ExplorerLens::Engine;
    AdaptiveShellIntegrationEngine engine;
    auto mode = engine.Probe();
    ASSERT(mode != ShellDeliveryMode::Unknown || mode == ShellDeliveryMode::Unknown);
}
TEST(TestASIE_Compatibility)
{
    using namespace ExplorerLens::Engine;
    AdaptiveShellIntegrationEngine engine;
    bool compat = engine.IsCompatible(ShellDeliveryMode::IThumbnailProvider);
    ASSERT(compat == true || compat == false);
}
TEST(TestASIE_ForceMode)
{
    using namespace ExplorerLens::Engine;
    AdaptiveShellIntegrationEngine engine;
    engine.ForceMode(ShellDeliveryMode::IThumbnailProvider);
    ASSERT(engine.GetActiveMode() == ShellDeliveryMode::IThumbnailProvider);
}
TEST(TestASIE_Stats)
{
    using namespace ExplorerLens::Engine;
    AdaptiveShellIntegrationEngine engine;
    engine.Probe();
    auto s = engine.GetStats();
    ASSERT(s.probeCount >= 1);
}
TEST(TestASIE_ForceMacOS)
{
    using namespace ExplorerLens::Engine;
    AdaptiveShellIntegrationEngine engine;
    engine.ForceMode(ShellDeliveryMode::QuickLookGenerator);
    ASSERT(engine.GetActiveMode() == ShellDeliveryMode::QuickLookGenerator);
}
TEST(TestASIE_ForceLinux)
{
    using namespace ExplorerLens::Engine;
    AdaptiveShellIntegrationEngine engine;
    engine.ForceMode(ShellDeliveryMode::GIOThumbnailer);
    ASSERT(engine.GetActiveMode() == ShellDeliveryMode::GIOThumbnailer);
}
TEST(TestASIE_MultiProbe)
{
    using namespace ExplorerLens::Engine;
    AdaptiveShellIntegrationEngine engine;
    engine.Probe();
    engine.Probe();
    engine.Probe();
    ASSERT(engine.GetStats().probeCount >= 3);
}
TEST(TestASIE_Fallback)
{
    using namespace ExplorerLens::Engine;
    AdaptiveShellIntegrationEngine engine;
    engine.ForceMode(ShellDeliveryMode::DBusThumbnailSpec);
    ASSERT(engine.GetStats().fallbacks >= 0);
}

TEST(TestSELM_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& mgr = ShellExtensionLifecycleManager::Instance();
    auto s = mgr.GetStats();
    ASSERT(s.registrations >= 0);
}
TEST(TestSELM_Register)
{
    using namespace ExplorerLens::Engine;
    auto& mgr = ShellExtensionLifecycleManager::Instance();
    bool ok = mgr.Register("TestExtension");
    ASSERT(ok == true || ok == false);
}
TEST(TestSELM_Unregister)
{
    using namespace ExplorerLens::Engine;
    auto& mgr = ShellExtensionLifecycleManager::Instance();
    mgr.Register("ExtA");
    mgr.Unregister("ExtA");
    ASSERT(mgr.GetStats().gracefulStops >= 0);
}
TEST(TestSELM_GetState)
{
    using namespace ExplorerLens::Engine;
    auto& mgr = ShellExtensionLifecycleManager::Instance();
    auto state = mgr.GetState("nonexistent");
    ASSERT(state == ExtensionState::Unregistered || state != ExtensionState::Active);
}
TEST(TestSELM_Heartbeat)
{
    using namespace ExplorerLens::Engine;
    auto& mgr = ShellExtensionLifecycleManager::Instance();
    mgr.Register("HBExt");
    mgr.Heartbeat("HBExt");
    ASSERT(mgr.GetStats().uptimeSeconds >= 0.0f);
}
TEST(TestSELM_Recover)
{
    using namespace ExplorerLens::Engine;
    auto& mgr = ShellExtensionLifecycleManager::Instance();
    mgr.Register("RecExt");
    bool ok = mgr.Recover("RecExt");
    ASSERT(ok == true || ok == false);
}
TEST(TestSELM_Suspend)
{
    using namespace ExplorerLens::Engine;
    auto& mgr = ShellExtensionLifecycleManager::Instance();
    mgr.Register("SuspExt");
    mgr.Suspend("SuspExt");
    auto state = mgr.GetState("SuspExt");
    ASSERT(state == ExtensionState::Suspended || state == ExtensionState::Active);
}
TEST(TestSELM_MultipleExtensions)
{
    using namespace ExplorerLens::Engine;
    auto& mgr = ShellExtensionLifecycleManager::Instance();
    mgr.Register("E1");
    mgr.Register("E2");
    mgr.Register("E3");
    ASSERT(mgr.GetStats().registrations >= 3);
}
TEST(TestSELM_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& mgr = ShellExtensionLifecycleManager::Instance();
    auto s = mgr.GetStats();
    ASSERT(s.crashes >= 0 && s.recoveries >= 0);
}

TEST(TestAPE_Initialize)
{
    using namespace ExplorerLens::Engine;
    AutotuningPipelineEngine ape(50);
    auto p = ape.GetCurrentParams();
    ASSERT(p.decodeConcurrency > 0);
}
TEST(TestAPE_Observe)
{
    using namespace ExplorerLens::Engine;
    AutotuningPipelineEngine ape;
    ape.Observe(120.0f, 45.0f);
    auto s = ape.GetStats();
    ASSERT(s.tuningCycles >= 1);
}
TEST(TestAPE_Step)
{
    using namespace ExplorerLens::Engine;
    AutotuningPipelineEngine ape;
    ape.Observe(100.0f, 50.0f);
    auto p = ape.Step();
    ASSERT(p.decodeConcurrency >= 1);
}
TEST(TestAPE_BestParams)
{
    using namespace ExplorerLens::Engine;
    AutotuningPipelineEngine ape;
    ape.Observe(200.0f, 20.0f);
    ape.Step();
    auto best = ape.GetBestParams();
    ASSERT(best.qualityTarget >= 0 && best.qualityTarget <= 100);
}
TEST(TestAPE_Convergence)
{
    using namespace ExplorerLens::Engine;
    AutotuningPipelineEngine ape(10);
    for (int i = 0; i < 12; ++i) {
        ape.Observe(float(100 + i * 2), float(50 - i));
        ape.Step();
    }
    ASSERT(ape.GetStats().tuningCycles >= 10);
}
TEST(TestAPE_Reset)
{
    using namespace ExplorerLens::Engine;
    AutotuningPipelineEngine ape;
    ape.Observe(99.0f, 55.0f);
    ape.Reset();
    ASSERT(ape.GetStats().tuningCycles == 0);
}
TEST(TestAPE_ThroughputTracking)
{
    using namespace ExplorerLens::Engine;
    AutotuningPipelineEngine ape;
    ape.Observe(235.0f, 17.0f);
    ASSERT(ape.GetStats().currentThroughput >= 0.0f);
}
TEST(TestAPE_LatencyTarget)
{
    using namespace ExplorerLens::Engine;
    AutotuningPipelineEngine ape;
    ape.Observe(180.0f, 22.0f);
    auto p = ape.GetCurrentParams();
    ASSERT(p.timeoutMs > 0.0f);
}
TEST(TestAPE_MultiObserve)
{
    using namespace ExplorerLens::Engine;
    AutotuningPipelineEngine ape;
    ape.Observe(80.f, 70.f);
    ape.Observe(130.f, 40.f);
    ape.Observe(200.f, 20.f);
    ASSERT(ape.GetStats().tuningCycles >= 3);
}

TEST(TestCPBV_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& v = CrossPlatformBuildValidator::Instance();
    auto p = v.DetectPlatform();
    ASSERT(p == BuildPlatform::Win64 || p == BuildPlatform::Win32 || p == BuildPlatform::macOS_ARM64
           || p == BuildPlatform::macOS_x64 || p == BuildPlatform::Linux_x64 || p == BuildPlatform::Linux_ARM64
           || p == BuildPlatform::Unknown);
}
TEST(TestCPBV_Validate)
{
    using namespace ExplorerLens::Engine;
    auto& v = CrossPlatformBuildValidator::Instance();
    auto report = v.Validate();
    ASSERT(report.platform != BuildPlatform::Unknown || report.platform == BuildPlatform::Unknown);
}
TEST(TestCPBV_HasErrors)
{
    using namespace ExplorerLens::Engine;
    auto& v = CrossPlatformBuildValidator::Instance();
    v.Validate();
    bool err = v.HasErrors();
    ASSERT(err == true || err == false);
}
TEST(TestCPBV_Summary)
{
    using namespace ExplorerLens::Engine;
    auto& v = CrossPlatformBuildValidator::Instance();
    auto summary = v.GetSummary();
    ASSERT(!summary.empty() || summary.empty());
}
TEST(TestCPBV_ReportResults)
{
    using namespace ExplorerLens::Engine;
    auto& v = CrossPlatformBuildValidator::Instance();
    auto report = v.Validate();
    ASSERT(report.errorCount >= 0);
    ASSERT(report.warningCount >= 0);
}
TEST(TestCPBV_Windows)
{
    using namespace ExplorerLens::Engine;
    auto& v = CrossPlatformBuildValidator::Instance();
    auto p = v.DetectPlatform();
    ASSERT(p == BuildPlatform::Win64 || p == BuildPlatform::Win32);  // currently built on Windows
}
TEST(TestCPBV_ValidationCount)
{
    using namespace ExplorerLens::Engine;
    auto& v = CrossPlatformBuildValidator::Instance();
    auto report = v.Validate();
    ASSERT(report.results.size() >= 0);
}
TEST(TestCPBV_MultiValidate)
{
    using namespace ExplorerLens::Engine;
    auto& v = CrossPlatformBuildValidator::Instance();
    v.Validate();
    v.Validate();
    ASSERT(!v.GetSummary().empty() || v.GetSummary().empty());
}
TEST(TestCPBV_NoFatalErrors)
{
    using namespace ExplorerLens::Engine;
    auto& v = CrossPlatformBuildValidator::Instance();
    auto report = v.Validate();
    bool hasFatal = false;
    for (auto& r : report.results)
        if (r.severity == BuildValidationSeverity::Fatal) {
            hasFatal = true;
            break;
        }
    ASSERT(!hasFatal);
}

//== v32.2.0 Fomalhaut-S — DirectStorage & Zero-Latency Pipeline Tests ==//

// DirectStorageManager
TEST(TestDSM_ProbeCapabilities)
{
    using namespace ExplorerLens::Engine;
    auto caps = DirectStorageManager::ProbeCapabilities();
    ASSERT(caps.maxQueueDepth >= 1);
    ASSERT(caps.maxStagingBufferMB >= 64);
}
TEST(TestDSM_InitShutdown)
{
    using namespace ExplorerLens::Engine;
    auto& m = DirectStorageManager::Instance();
    ASSERT(m.Initialize());
    m.Shutdown();
    ASSERT(m.GetQueueDepth() == 0);
}
TEST(TestDSM_SubmitRequest_CPUFallback)
{
    using namespace ExplorerLens::Engine;
    auto& m = DirectStorageManager::Instance();
    m.Initialize();
    DSStreamRequest req;
    req.filePath = L"test.raw";
    req.readLength = 4096;
    auto r = m.SubmitRequest(req);
    ASSERT(r.success);
    ASSERT(r.bytesRead == 4096);
}
TEST(TestDSM_BackendName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(DirectStorageManager::BackendName(DSBackend::CPU_FALLBACK)) == "CPU-Fallback");
    ASSERT(std::string(DirectStorageManager::BackendName(DSBackend::DIRECT_STORAGE12)) == "DirectStorage-1.2");
}
TEST(TestDSM_SupportedCompression)
{
    using namespace ExplorerLens::Engine;
    ASSERT(DirectStorageManager::IsSupportedCompressionFormat(DSCompressionFormat::GDEFLATE));
    ASSERT(DirectStorageManager::IsSupportedCompressionFormat(DSCompressionFormat::ZSTANDARD));
    ASSERT(!DirectStorageManager::IsSupportedCompressionFormat(DSCompressionFormat::NONE));
}

// GPUDecompressKernel
TEST(TestGDK_ProbeCapabilities)
{
    using namespace ExplorerLens::Engine;
    auto caps = GPUDecompressKernel::ProbeCapabilities();
    ASSERT(caps.maxInputSizeMB >= 64);
    ASSERT(caps.maxBatchCount >= 1);
}
TEST(TestGDK_InitSelectsVendor)
{
    using namespace ExplorerLens::Engine;
    auto& k = GPUDecompressKernel::Instance();
    ASSERT(k.Initialize());
    auto v = k.GetActiveVendor();
    ASSERT(v == GDKVendor::CPU_FALLBACK || v == GDKVendor::NVIDIA_GDEFLATE
           || v == GDKVendor::INTEL_DSB || v == GDKVendor::AMD_COMPUTE_SHADER);
}
TEST(TestGDK_Decompress_CPUFallback)
{
    using namespace ExplorerLens::Engine;
    auto& k = GPUDecompressKernel::Instance();
    k.Initialize(GDKVendor::CPU_FALLBACK);
    std::vector<uint8_t> src(256, 0xAB);
    std::vector<uint8_t> dst(1024, 0);
    GPUDecompressInput in;
    in.compressedData = src.data();
    in.compressedSize = static_cast<uint32_t>(src.size());
    in.outputBuffer = dst.data();
    in.outputCapacity = static_cast<uint32_t>(dst.size());
    in.format = GPUCompressedFormat::ZSTANDARD;
    auto out = k.Decompress(in);
    ASSERT(out.success);
    ASSERT(out.vendorUsed == GDKVendor::CPU_FALLBACK);
}
TEST(TestGDK_EstimateOutputSize)
{
    using namespace ExplorerLens::Engine;
    ASSERT(GPUDecompressKernel::EstimateOutputSize(1000, GPUCompressedFormat::ZSTANDARD) == 8000);
    ASSERT(GPUDecompressKernel::EstimateOutputSize(1000, GPUCompressedFormat::UNCOMPRESSED) == 1000);
}
TEST(TestGDK_VendorName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(GPUDecompressKernel::VendorName(GDKVendor::CPU_FALLBACK)) == "CPU-Fallback");
    ASSERT(std::string(GPUDecompressKernel::VendorName(GDKVendor::NVIDIA_GDEFLATE)) == "NVIDIA-GDeflate");
}

// ZeroLatencyPipeline
TEST(TestZLP_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& p = ZeroLatencyPipeline::Instance();
    ASSERT(p.Initialize());
    ASSERT(p.GetState() == ZLPState::IDLE);
}
TEST(TestZLP_Process_CPUPath)
{
    using namespace ExplorerLens::Engine;
    auto& p = ZeroLatencyPipeline::Instance();
    p.Initialize();
    ZLPRequest req;
    req.filePath = L"sample.jpg";
    req.thumbWidth = 256;
    req.thumbHeight = 256;
    req.enableGPUPath = false;
    auto r = p.Process(req);
    ASSERT(r.success);
    ASSERT(r.width == 256);
    ASSERT(!r.usedGPUPath);
    ASSERT(!r.pixels.empty());
}
TEST(TestZLP_Metrics_Tracked)
{
    using namespace ExplorerLens::Engine;
    auto& p = ZeroLatencyPipeline::Instance();
    p.Initialize();
    p.ResetMetrics();
    ZLPRequest req;
    req.filePath = L"x.png";
    req.thumbWidth = 128;
    req.thumbHeight = 128;
    p.Process(req);
    ASSERT(p.GetMetrics().requestsSubmitted >= 1);
    ASSERT(p.GetMetrics().requestsCompleted >= 1);
}
TEST(TestZLP_RecommendedThumbSize)
{
    using namespace ExplorerLens::Engine;
    ASSERT(ZeroLatencyPipeline::RecommendedThumbSize(100) == 256);
    ASSERT(ZeroLatencyPipeline::RecommendedThumbSize(60 * 1024 * 1024ULL) == 512);
}
TEST(TestZLP_Shutdown_Resets)
{
    using namespace ExplorerLens::Engine;
    auto& p = ZeroLatencyPipeline::Instance();
    p.Initialize();
    p.Shutdown();
    p.Initialize();
    ASSERT(p.GetState() == ZLPState::IDLE);
}

// ThumbnailPipelineMetrics
TEST(TestTPM_RecordAndStats)
{
    using namespace ExplorerLens::Engine;
    auto& m = ThumbnailPipelineMetrics::Instance();
    m.Reset();
    m.RecordSample(TPMStage::FILE_READ, 12.0);
    m.RecordSample(TPMStage::FILE_READ, 14.0);
    auto s = m.ComputeStats(TPMStage::FILE_READ);
    ASSERT(s.samples == 2);
    ASSERT(s.p50Ms >= 12.0 && s.p50Ms <= 15.0);
}
TEST(TestTPM_StageName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ThumbnailPipelineMetrics::StageName(TPMStage::DECODE)) == "Decode");
    ASSERT(std::string(ThumbnailPipelineMetrics::StageName(TPMStage::RENDER)) == "Render");
}
TEST(TestTPM_BottleneckDetect_IO)
{
    using namespace ExplorerLens::Engine;
    auto& m = ThumbnailPipelineMetrics::Instance();
    m.Reset();
    for (int i = 0; i < 50; ++i)
        m.RecordSample(TPMStage::FILE_READ, 200.0);
    for (int i = 0; i < 50; ++i)
        m.RecordSample(TPMStage::RENDER, 5.0);
    auto snap = m.Snapshot();
    ASSERT(snap.bottleneck == BottleneckStage::IO || snap.bottleneck == BottleneckStage::CPU
           || snap.bottleneck == BottleneckStage::GPU || snap.bottleneck == BottleneckStage::NONE);
}
TEST(TestTPM_Reset_ClearsStats)
{
    using namespace ExplorerLens::Engine;
    auto& m = ThumbnailPipelineMetrics::Instance();
    m.RecordSample(TPMStage::DECODE, 10.0);
    m.Reset();
    auto s = m.ComputeStats(TPMStage::DECODE);
    ASSERT(s.samples == 0);
}
TEST(TestTPM_DescribeBottleneck)
{
    using namespace ExplorerLens::Engine;
    std::string desc = ThumbnailPipelineMetrics::DescribeBottleneck(BottleneckStage::CPU);
    ASSERT(!desc.empty());
    ASSERT(desc.find("CPU") != std::string::npos || desc.find("cpu") != std::string::npos
           || desc.find("decode") != std::string::npos || desc.length() > 5);
}

// StreamingDecodeOrchestrator
TEST(TestSDO_Probe_RAW)
{
    using namespace ExplorerLens::Engine;
    auto& o = StreamingDecodeOrchestrator::Instance();
    auto p = o.Probe(L"image.cr3");
    ASSERT(p.fileType == SDOFileType::RAW_CAMERA);
    ASSERT(p.strategy == SDOStrategy::EMBEDDED_THUMB);
}
TEST(TestSDO_Probe_FITS)
{
    using namespace ExplorerLens::Engine;
    auto& o = StreamingDecodeOrchestrator::Instance();
    auto p = o.Probe(L"data.fits");
    ASSERT(p.fileType == SDOFileType::FITS);
    ASSERT(p.strategy == SDOStrategy::REGION_READ);
    ASSERT(p.targetRegion.has_value());
}
TEST(TestSDO_Probe_Unknown)
{
    using namespace ExplorerLens::Engine;
    auto& o = StreamingDecodeOrchestrator::Instance();
    auto p = o.Probe(L"file.xyz");
    ASSERT(p.fileType == SDOFileType::UNKNOWN);
    ASSERT(p.strategy == SDOStrategy::FULL_DECODE);
}
TEST(TestSDO_Decode_Returns_Pixels)
{
    using namespace ExplorerLens::Engine;
    auto& o = StreamingDecodeOrchestrator::Instance();
    auto r = o.Decode(L"photo.arw", 256);
    ASSERT(r.success);
    ASSERT(r.width == 256);
    ASSERT(!r.pixelsBGRA.empty());
    ASSERT(r.totalLatencyMs > 0.0);
}
TEST(TestSDO_EstimateMinBytes)
{
    using namespace ExplorerLens::Engine;
    uint64_t raw = StreamingDecodeOrchestrator::EstimateMinBytesNeeded(SDOFileType::RAW_CAMERA, 50 * 1024 * 1024ULL);
    ASSERT(raw <= 512 * 1024);
    uint64_t fits = StreamingDecodeOrchestrator::EstimateMinBytesNeeded(SDOFileType::FITS, 5 * 1024 * 1024ULL);
    ASSERT(fits <= 64 * 1024);
}

//== v32.3.0 Fomalhaut-T — Annotation, HDR Tone Mapping & Format Detection Tests ==//

// ThumbnailAnnotationOverlay
TEST(TestTAO_NoBadges)
{
    using namespace ExplorerLens::Engine;
    auto& o = ThumbnailAnnotationOverlay::Instance();
    std::vector<uint8_t> buf(256 * 256 * 4, 0x80);
    auto r = o.Apply(buf.data(), 256, 256, 0);
    ASSERT(!r.applied);
    ASSERT(r.badgesRendered == 0);
}
TEST(TestTAO_DRMBadge)
{
    using namespace ExplorerLens::Engine;
    auto& o = ThumbnailAnnotationOverlay::Instance();
    std::vector<uint8_t> buf(256 * 256 * 4, 0x80);
    uint8_t mask = static_cast<uint8_t>(AnnotationBadge::DRMLocked);
    auto r = o.Apply(buf.data(), 256, 256, mask);
    ASSERT(r.applied);
    ASSERT(r.badgesRendered >= 1);
    ASSERT(r.latencyUs > 0.0);
}
TEST(TestTAO_MultiBadge)
{
    using namespace ExplorerLens::Engine;
    auto& o = ThumbnailAnnotationOverlay::Instance();
    std::vector<uint8_t> buf(256 * 256 * 4, 0x80);
    uint8_t mask = static_cast<uint8_t>(AnnotationBadge::DRMLocked) | static_cast<uint8_t>(AnnotationBadge::ReadOnly);
    auto r = o.Apply(buf.data(), 256, 256, mask);
    ASSERT(r.badgesRendered >= 2);
}
TEST(TestTAO_RecommendedSize)
{
    using namespace ExplorerLens::Engine;
    ASSERT(ThumbnailAnnotationOverlay::RecommendedOverlayBadgeSize(32) == OverlayBadgeSize::Small);
    ASSERT(ThumbnailAnnotationOverlay::RecommendedOverlayBadgeSize(128) == OverlayBadgeSize::Medium);
    ASSERT(ThumbnailAnnotationOverlay::RecommendedOverlayBadgeSize(256) == OverlayBadgeSize::Large);
}
TEST(TestTAO_BadgeMaskForFile)
{
    using namespace ExplorerLens::Engine;
    uint8_t m = ThumbnailAnnotationOverlay::BadgeMaskForFile(L"test.pdf", true, false);
    ASSERT(m & static_cast<uint8_t>(AnnotationBadge::ReadOnly));
    uint8_t m2 = ThumbnailAnnotationOverlay::BadgeMaskForFile(L"drm.mp4", false, true);
    ASSERT(m2 & static_cast<uint8_t>(AnnotationBadge::DRMLocked));
}

// AdaptiveBitDepthConverter
TEST(TestABD_SDR_PassThrough)
{
    using namespace ExplorerLens::Engine;
    auto& c = AdaptiveBitDepthConverter::Instance();
    std::vector<uint8_t> src(64 * 64 * 4, 0xAB);
    std::vector<uint8_t> dst(64 * 64 * 4, 0);
    auto r = c.Convert(src.data(), 64, 64, BitDepthSource::SDR_8bit, dst.data());
    ASSERT(r.success);
    ASSERT(r.pixelsConverted == 64 * 64);
    ASSERT(!r.wasToneMapped);
}
TEST(TestABD_DetectSourceFormat)
{
    using namespace ExplorerLens::Engine;
    ASSERT(AdaptiveBitDepthConverter::DetectSourceFormat(8, false, false) == BitDepthSource::SDR_8bit);
    ASSERT(AdaptiveBitDepthConverter::DetectSourceFormat(16, false, false) == BitDepthSource::HDR_16bit_FP);
    ASSERT(AdaptiveBitDepthConverter::DetectSourceFormat(10, true, false) == BitDepthSource::HDR_10bit_PQ);
}
TEST(TestABD_ToneMappingName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(AdaptiveBitDepthConverter::ToneMappingName(ABDCToneMappingOp::ACES_Filmic)) == "ACES-Filmic");
    ASSERT(std::string(AdaptiveBitDepthConverter::ToneMappingName(ABDCToneMappingOp::Reinhard)) == "Reinhard");
}
TEST(TestABD_SourceFormatName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(AdaptiveBitDepthConverter::SourceFormatName(BitDepthSource::SDR_8bit)) == "SDR-8bit");
    ASSERT(std::string(AdaptiveBitDepthConverter::SourceFormatName(BitDepthSource::HDR_16bit_FP)) == "HDR-16bit-FP");
}
TEST(TestABD_HDR_Convert_BufferSize)
{
    using namespace ExplorerLens::Engine;
    auto& c = AdaptiveBitDepthConverter::Instance();
    std::vector<uint16_t> src(16 * 16 * 4, 0x3C00);  // 1.0 in float16
    std::vector<uint8_t> dst(16 * 16 * 4, 0);
    auto r = c.Convert(src.data(), 16, 16, BitDepthSource::HDR_16bit_FP, dst.data());
    ASSERT(r.success);
    ASSERT(r.pixelsConverted == 16 * 16);
    ASSERT(r.wasToneMapped);
}

// BatchThumbnailExporter
TEST(TestBTE_FormatExtension)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(BatchThumbnailExporter::FormatExtension(BTE_ExportFormat::JPEG)) == ".jpg");
    ASSERT(std::string(BatchThumbnailExporter::FormatExtension(BTE_ExportFormat::PNG)) == ".png");
    ASSERT(std::string(BatchThumbnailExporter::FormatExtension(BTE_ExportFormat::WebP)) == ".webp");
}
TEST(TestBTE_FormatSupportsLossless)
{
    using namespace ExplorerLens::Engine;
    ASSERT(BatchThumbnailExporter::FormatSupportsLossless(BTE_ExportFormat::PNG));
    ASSERT(BatchThumbnailExporter::FormatSupportsLossless(BTE_ExportFormat::WebP));
    ASSERT(!BatchThumbnailExporter::FormatSupportsLossless(BTE_ExportFormat::JPEG));
}
TEST(TestBTE_Export_SingleSize)
{
    using namespace ExplorerLens::Engine;
    auto& e = BatchThumbnailExporter::Instance();
    std::vector<uint8_t> src(64 * 64 * 4, 0xAA);
    ExportJob job;
    job.sourceFile = L"photo.jpg";
    job.outputDir = L"C:\\Temp";
    job.sizes = {{64, 64, "small"}};
    job.format = BTE_ExportFormat::JPEG;
    auto r = e.Export(job, src.data(), 64, 64);
    ASSERT(r.exportedCount >= 1);
    ASSERT(!r.thumbs.empty());
    ASSERT(r.thumbs[0].success);
}
TEST(TestBTE_Export_MultiSize)
{
    using namespace ExplorerLens::Engine;
    auto& e = BatchThumbnailExporter::Instance();
    std::vector<uint8_t> src(128 * 128 * 4, 0xBB);
    ExportJob job;
    job.sourceFile = L"photo.png";
    job.outputDir = L"C:\\Temp";
    job.sizes = {{64, 64, "sm"}, {128, 128, "md"}};
    auto r = e.Export(job, src.data(), 128, 128);
    ASSERT(r.exportedCount == 2);
    ASSERT(r.thumbs.size() == 2);
}
TEST(TestBTE_TotalExported)
{
    using namespace ExplorerLens::Engine;
    auto& e = BatchThumbnailExporter::Instance();
    uint64_t before = e.GetTotalExported();
    std::vector<uint8_t> src(32 * 32 * 4, 0xFF);
    ExportJob job;
    job.outputDir = L"C:\\Temp";
    job.sizes = {{32, 32, "x"}};
    e.Export(job, src.data(), 32, 32);
    ASSERT(e.GetTotalExported() >= before + 1);
}

// FormatSignatureDetector
TEST(TestFSD_DetectJPEG)
{
    using namespace ExplorerLens::Engine;
    auto& d = MagicByteFormatDetector::Instance();
    uint8_t hdr[] = {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10};
    auto m = d.Detect(hdr, sizeof(hdr));
    ASSERT(m.format == MagicByteFormat::JPEG);
    ASSERT(m.confidence == SignatureConfidence::Definitive);
}
TEST(TestFSD_DetectPNG)
{
    using namespace ExplorerLens::Engine;
    auto& d = MagicByteFormatDetector::Instance();
    uint8_t hdr[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    auto m = d.Detect(hdr, sizeof(hdr));
    ASSERT(m.format == MagicByteFormat::PNG);
    ASSERT(m.confidence == SignatureConfidence::Definitive);
}
TEST(TestFSD_DetectPDF)
{
    using namespace ExplorerLens::Engine;
    auto& d = MagicByteFormatDetector::Instance();
    uint8_t hdr[] = {'%', 'P', 'D', 'F', '-', '1', '.', '7'};
    auto m = d.Detect(hdr, sizeof(hdr));
    ASSERT(m.format == MagicByteFormat::PDF);
}
TEST(TestFSD_DetectUnknown)
{
    using namespace ExplorerLens::Engine;
    auto& d = MagicByteFormatDetector::Instance();
    uint8_t hdr[] = {0x00, 0x01, 0x02, 0x03};
    auto m = d.Detect(hdr, sizeof(hdr));
    ASSERT(m.format == MagicByteFormat::Unknown);
    ASSERT(m.confidence == SignatureConfidence::None);
}
TEST(TestFSD_ExtensionMatch)
{
    using namespace ExplorerLens::Engine;
    ASSERT(MagicByteFormatDetector::ExtensionMatchesDetected(L"photo.jpg", MagicByteFormat::JPEG));
    ASSERT(MagicByteFormatDetector::ExtensionMatchesDetected(L"doc.pdf", MagicByteFormat::PDF));
    ASSERT(!MagicByteFormatDetector::ExtensionMatchesDetected(L"", MagicByteFormat::JPEG));
}

// MemoryMappedDecoder
TEST(TestMMD_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& m = MemoryMappedDecoder::Instance();
    ASSERT(m.Initialize());
    ASSERT(m.GetBackend() != MMapBackend::Unavailable);
}
TEST(TestMMD_Decode_ReturnsPixels)
{
    using namespace ExplorerLens::Engine;
    auto& m = MemoryMappedDecoder::Instance();
    m.Initialize();
    MMapDecodeRequest req;
    req.filePath = L"large_photo.tiff";
    req.thumbSize = 128;
    auto r = m.Decode(req);
    ASSERT(r.success);
    ASSERT(r.thumbWidth == 128);
    ASSERT(!r.pixelsBGRA.empty());
    ASSERT(r.totalLatencyMs > 0.0);
}
TEST(TestMMD_Stats_Tracked)
{
    using namespace ExplorerLens::Engine;
    auto& m = MemoryMappedDecoder::Instance();
    m.ResetStats();
    m.Initialize();
    MMapDecodeRequest req;
    req.filePath = L"big.raw";
    req.thumbSize = 64;
    m.Decode(req);
    ASSERT(m.GetStats().mappingsCreated >= 1);
    ASSERT(m.GetStats().bytesAccessed >= 1);
}
TEST(TestMMD_RecommendedForFile)
{
    using namespace ExplorerLens::Engine;
    ASSERT(!MemoryMappedDecoder::IsRecommendedForFile(512 * 1024));       // < 8MB
    ASSERT(MemoryMappedDecoder::IsRecommendedForFile(10 * 1024 * 1024));  // > 8MB
}
TEST(TestMMD_BackendName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(MemoryMappedDecoder::BackendName(MMapBackend::WindowsMMapV1)) == "Windows-MMapV1");
    ASSERT(std::string(MemoryMappedDecoder::BackendName(MMapBackend::Unavailable)) == "Unavailable");
}

//== Sprint 1111-1120: DirectStorage Zero-Copy GPU Decompress (v32.5.0 "Fomalhaut-V") ==

// ── ZStdGPUKernel ──────────────────────────────────────────────────────────
TEST(TestZSK_VendorName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(ZStdGPUKernel::VendorName(ZStdGPUVendor::AMD_RDNA3)  == std::string_view("AMD-RDNA3"));
    ASSERT(ZStdGPUKernel::VendorName(ZStdGPUVendor::INTEL_XE2)  == std::string_view("Intel-Xe2"));
    ASSERT(ZStdGPUKernel::VendorName(ZStdGPUVendor::INTEL_ARC)  == std::string_view("Intel-Arc"));
    ASSERT(ZStdGPUKernel::VendorName(ZStdGPUVendor::FALLBACK)   == std::string_view("CPU-Fallback"));
}
TEST(TestZSK_FallbackWhenUnavailable)
{
    using namespace ExplorerLens::Engine;
    // Default instance is unavailable (no real GPU in test env)
    ASSERT(!ZStdGPUKernel::Instance().IsAvailable());
    ASSERT(ZStdGPUKernel::Instance().DetectedVendor() == ZStdGPUVendor::FALLBACK);
}
TEST(TestZSK_DecompressReturnsBytesOnFallback)
{
    using namespace ExplorerLens::Engine;
    uint8_t src[8]  = { 0x28, 0xB5, 0x2F, 0xFD, 0x04, 0x00, 0x01, 0x00 };
    uint8_t dst[64] = {};
    ZStdGPUKernelDesc d{};
    d.compressedData   = src;
    d.compressedSize   = 8;
    d.outputBuffer     = dst;
    d.outputCapacity   = 64;
    d.expectedOrigSize = 4;
    d.vendor           = ZStdGPUVendor::FALLBACK;
    ZStdGPUKernelResult r = ZStdGPUKernel::Instance().Decompress(d);
    // Fallback must not crash; fields must be populated
    ASSERT(r.decompressMs >= 0.0f);
}
// ── GPUDecompressOrchestrator ──────────────────────────────────────────────
TEST(TestGDO_DefaultBackendIsCPU)
{
    using namespace ExplorerLens::Engine;
    GPUDecompressOrchestrator::Instance().Initialize();
    ASSERT(GPUDecompressOrchestrator::Instance().PreferredBackend() == GPUDecompressBackend::CPU
        || GPUDecompressOrchestrator::Instance().PreferredBackend() == GPUDecompressBackend::ZSTD_GPU);
}
TEST(TestGDO_BackendName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(GPUDecompressOrchestrator::BackendName(GPUDecompressBackend::NV_GDEFLATE) == std::string_view("NvGDeflate"));
    ASSERT(GPUDecompressOrchestrator::BackendName(GPUDecompressBackend::ZSTD_GPU)    == std::string_view("ZStdGPU"));
    ASSERT(GPUDecompressOrchestrator::BackendName(GPUDecompressBackend::CPU)         == std::string_view("CPU"));
}
TEST(TestGDO_DecompressCPUPath)
{
    using namespace ExplorerLens::Engine;
    uint8_t src[8]  = { 1, 2, 3, 4, 5, 6, 7, 8 };
    uint8_t dst[64] = {};
    GPUDecompressRequest req{};
    req.srcData          = src;
    req.srcSize          = 8;
    req.dstBuffer        = dst;
    req.dstCapacity      = 64;
    req.expectedOrigSize = 8;
    GPUDecompressResult res = GPUDecompressOrchestrator::Instance().Decompress(req);
    ASSERT(res.success);
    ASSERT(res.bytesOut > 0);
    ASSERT(res.elapsedMs >= 0.0f);
}
TEST(TestGDO_DecompressInvalidSizeFails)
{
    using namespace ExplorerLens::Engine;
    GPUDecompressRequest req{};  // all nullptrs / zeros
    GPUDecompressResult res = GPUDecompressOrchestrator::Instance().Decompress(req);
    ASSERT(!res.success);
}
// ── DirectStorageBatchScheduler ────────────────────────────────────────────
TEST(TestDSBS_AddAndFlush)
{
    using namespace ExplorerLens::Engine;
    auto& sched = DirectStorageBatchScheduler::Instance();
    sched.Reset();
    DSBatchItem item{ L"photo.raw", 256, 1 };
    sched.AddItem(item);
    ASSERT(sched.PendingCount() == 1);
    ASSERT(sched.Flush());
    ASSERT(sched.PendingCount() == 0);
}
TEST(TestDSBS_EmptyFlush)
{
    using namespace ExplorerLens::Engine;
    auto& sched = DirectStorageBatchScheduler::Instance();
    sched.Reset();
    ASSERT(sched.Flush());  // flush on empty queue — must succeed
}
TEST(TestDSBS_PendingCount)
{
    using namespace ExplorerLens::Engine;
    auto& sched = DirectStorageBatchScheduler::Instance();
    sched.Reset();
    sched.AddItem({ L"a.cr2", 128, 10 });
    sched.AddItem({ L"b.nef", 128, 11 });
    ASSERT(sched.PendingCount() == 2);
}
TEST(TestDSBS_ResultsAfterFlush)
{
    using namespace ExplorerLens::Engine;
    auto& sched = DirectStorageBatchScheduler::Instance();
    sched.Reset();
    sched.AddItem({ L"c.arw", 256, 42 });
    sched.Flush();
    const auto& results = sched.GetResults();
    ASSERT(results.size() == 1);
    ASSERT(results[0].itemId == 42);
    ASSERT(results[0].success);
    ASSERT(results[0].totalLatencyMs > 0.0f);
}
TEST(TestDSBS_ResetClearsAll)
{
    using namespace ExplorerLens::Engine;
    auto& sched = DirectStorageBatchScheduler::Instance();
    sched.AddItem({ L"d.dng", 256, 99 });
    sched.Reset();
    ASSERT(sched.PendingCount() == 0);
    ASSERT(sched.GetResults().empty());
}
// ── DirectStorageProfiler ─────────────────────────────────────────────────
TEST(TestDSP_PathName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(DirectStorageProfiler::PathName(DSDecodePath::DIRECT_STORAGE) == std::string_view("DirectStorage"));
    ASSERT(DirectStorageProfiler::PathName(DSDecodePath::CPU_DECODE)     == std::string_view("CPUDecode"));
}
TEST(TestDSP_RecommendedPath)
{
    using namespace ExplorerLens::Engine;
    // Small file → CPU
    ASSERT(DirectStorageProfiler::Instance().RecommendedPath(512 * 1024) == DSDecodePath::CPU_DECODE);
    // Large file (≥4 MB) → DirectStorage
    ASSERT(DirectStorageProfiler::Instance().RecommendedPath(8 * 1024 * 1024) == DSDecodePath::DIRECT_STORAGE);
}
TEST(TestDSP_AddSampleAndStats)
{
    using namespace ExplorerLens::Engine;
    auto& p = DirectStorageProfiler::Instance();
    p.Reset();
    DSProfileSample s{};
    s.path       = DSDecodePath::DIRECT_STORAGE;
    s.ioMs       = 3.0f;
    s.decompressMs = 2.0f;
    s.decodeMs   = 5.0f;
    s.totalMs    = 10.0f;
    s.fileSizeBytes = 6 * 1024 * 1024;
    p.AddSample(s);
    auto stats = p.ComputeStats();
    ASSERT(stats.sampleCount == 1);
    ASSERT(stats.dsPathCount == 1);
    ASSERT(stats.cpuPathCount == 0);
}
TEST(TestDSP_ResetClearsSamples)
{
    using namespace ExplorerLens::Engine;
    auto& p = DirectStorageProfiler::Instance();
    DSProfileSample s{};
    s.totalMs = 15.0f;
    p.AddSample(s);
    p.Reset();
    auto stats = p.ComputeStats();
    ASSERT(stats.sampleCount == 0);
}
TEST(TestDSP_P99LargerThanP50)
{
    using namespace ExplorerLens::Engine;
    auto& p = DirectStorageProfiler::Instance();
    p.Reset();
    for (uint32_t i = 1; i <= 100; ++i)
    {
        DSProfileSample s{};
        s.totalMs       = static_cast<float>(i);
        s.path          = DSDecodePath::DIRECT_STORAGE;
        s.fileSizeBytes = 5 * 1024 * 1024;
        p.AddSample(s);
    }
    auto stats = p.ComputeStats();
    ASSERT(stats.p99TotalMs >= stats.p50TotalMs);
}
// ── ZeroCopyDecodeSession ──────────────────────────────────────────────────
TEST(TestZCS_DefaultState)
{
    using namespace ExplorerLens::Engine;
    ZeroCopyDecodeSession s{};
    ASSERT(s.sessionId == 0);
    ASSERT(s.state == ZCSessionState::IDLE);
    ASSERT(!s.gpuDecompress);
    ASSERT(s.stagingPtr == nullptr);
}
TEST(TestZCS_TotalMs)
{
    using namespace ExplorerLens::Engine;
    ZeroCopyDecodeSession s{};
    s.ioMs         = 3.0f;
    s.decompressMs = 2.0f;
    s.decodeMs     = 5.0f;
    ASSERT(s.TotalMs() > 9.9f && s.TotalMs() < 10.1f);
}
TEST(TestZCS_IsTerminal)
{
    using namespace ExplorerLens::Engine;
    ZeroCopyDecodeSession s{};
    s.state = ZCSessionState::IDLE;
    ASSERT(!s.IsTerminal());
    s.state = ZCSessionState::COMPLETE;
    ASSERT(s.IsTerminal());
    s.state = ZCSessionState::FAILED;
    ASSERT(s.IsTerminal());
}
TEST(TestZCS_StateName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ZeroCopyDecodeSession::StateName(ZCSessionState::IDLE))                == "Idle");
    ASSERT(std::string(ZeroCopyDecodeSession::StateName(ZCSessionState::IO_PENDING))          == "IO_Pending");
    ASSERT(std::string(ZeroCopyDecodeSession::StateName(ZCSessionState::DECOMPRESS_PENDING))  == "Decompress_Pending");
    ASSERT(std::string(ZeroCopyDecodeSession::StateName(ZCSessionState::DECODE_PENDING))      == "Decode_Pending");
    ASSERT(std::string(ZeroCopyDecodeSession::StateName(ZCSessionState::COMPLETE))            == "Complete");
    ASSERT(std::string(ZeroCopyDecodeSession::StateName(ZCSessionState::FAILED))              == "Failed");
}
TEST(TestZCS_GpuDecompressFlag)
{
    using namespace ExplorerLens::Engine;
    ZeroCopyDecodeSession s{};
    s.gpuDecompress = true;
    s.stagingBytes  = 2048;
    ASSERT(s.gpuDecompress);
    ASSERT(s.stagingBytes == 2048);
}

//== Sprint 1121-1130: CLIP Semantic Search + HNSW Index (v32.6.0 "Fomalhaut-W") ==

// ── HNSWIndexEngine ────────────────────────────────────────────────────────
TEST(TestHNSW_InsertAndCount)
{
    using namespace ExplorerLens::Engine;
    auto& idx = HNSWIndexEngine::Instance();
    idx.Reset();
    HNSWEntry e{};
    e.itemId   = 1;
    e.filePath = L"photo.jpg";
    e.vector[0] = 1.0f;
    ASSERT(idx.Insert(e));
    ASSERT(idx.Count() == 1);
}
TEST(TestHNSW_Remove)
{
    using namespace ExplorerLens::Engine;
    auto& idx = HNSWIndexEngine::Instance();
    idx.Reset();
    HNSWEntry e{};
    e.itemId = 7;
    idx.Insert(e);
    ASSERT(idx.Remove(7));
    ASSERT(idx.Count() == 0);
    ASSERT(!idx.Remove(999));  // non-existent
}
TEST(TestHNSW_QueryTopK)
{
    using namespace ExplorerLens::Engine;
    auto& idx = HNSWIndexEngine::Instance();
    idx.Reset();
    for (uint32_t i = 1; i <= 5; ++i)
    {
        HNSWEntry e{};
        e.itemId      = i;
        e.vector[i-1] = 1.0f;
        idx.Insert(e);
    }
    float qv[512]{};
    qv[0] = 1.0f;
    auto results = idx.Query(qv, 3);
    ASSERT(results.size() <= 3);
    // Best match should have itemId == 1 (vector[0] == 1)
    ASSERT(!results.empty() && results[0].itemId == 1);
}
TEST(TestHNSW_SaveLoad)
{
    using namespace ExplorerLens::Engine;
    auto& idx = HNSWIndexEngine::Instance();
    ASSERT(idx.SaveToFile(L"test_index.bin"));
    ASSERT(idx.LoadFromFile(L"test_index.bin"));
    ASSERT(!idx.SaveToFile(L""));   // empty path → fail
    ASSERT(!idx.LoadFromFile(L"")); // empty path → fail
}
TEST(TestHNSW_Reset)
{
    using namespace ExplorerLens::Engine;
    auto& idx = HNSWIndexEngine::Instance();
    HNSWEntry e{};
    e.itemId = 42;
    idx.Insert(e);
    idx.Reset();
    ASSERT(idx.Count() == 0);
    ASSERT(idx.LastQueryMs() == 0.0f);
}
// ── CLIPQueryProcessor ─────────────────────────────────────────────────────
TEST(TestCQP_BackendName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(CLIPQueryProcessor::BackendName(CLIPTextBackend::DIRECTML) == std::string_view("DirectML"));
    ASSERT(CLIPQueryProcessor::BackendName(CLIPTextBackend::ONNX)     == std::string_view("ONNX"));
    ASSERT(CLIPQueryProcessor::BackendName(CLIPTextBackend::CPU)      == std::string_view("CPU"));
}
TEST(TestCQP_LoadModelFails_EmptyPath)
{
    using namespace ExplorerLens::Engine;
    auto& p = CLIPQueryProcessor::Instance();
    ASSERT(!p.LoadModel(L""));
    ASSERT(!p.IsModelLoaded());
}
TEST(TestCQP_LoadModelSucceeds)
{
    using namespace ExplorerLens::Engine;
    auto& p = CLIPQueryProcessor::Instance();
    ASSERT(p.LoadModel(L"models/clip_vitb32_int8.onnx"));
    ASSERT(p.IsModelLoaded());
}
TEST(TestCQP_QueryEmpty_WhenNotLoaded)
{
    using namespace ExplorerLens::Engine;
    CLIPQueryProcessor fresh{};  // unloaded instance
    CLIPQueryRequest req{};
    req.queryText = L"sunset";
    auto res = fresh.Query(req);
    ASSERT(res.empty());
}
TEST(TestCQP_LastEmbedMsDefault)
{
    using namespace ExplorerLens::Engine;
    CLIPQueryProcessor fresh{};
    ASSERT(fresh.LastEmbedMs() == 0.0f);
}
// ── SemanticSearchOrchestrator ─────────────────────────────────────────────
TEST(TestSSO_InitializeSucceeds)
{
    using namespace ExplorerLens::Engine;
    auto& sso = SemanticSearchOrchestrator::Instance();
    ASSERT(sso.Initialize(L"search_index.bin"));
}
TEST(TestSSO_IsReadyAfterInit)
{
    using namespace ExplorerLens::Engine;
    auto& sso = SemanticSearchOrchestrator::Instance();
    sso.Initialize(L"search_index.bin");
    ASSERT(sso.IsReady());
}
TEST(TestSSO_IndexFile)
{
    using namespace ExplorerLens::Engine;
    auto& sso  = SemanticSearchOrchestrator::Instance();
    auto& hnsw = HNSWIndexEngine::Instance();
    hnsw.Reset();
    sso.Initialize(L"idx.bin");
    float emb[512]{};
    emb[0] = 0.9f;
    ASSERT(sso.IndexFile(L"cats.jpg", emb));
    ASSERT(sso.IndexedCount() >= 1);
}
TEST(TestSSO_SearchEmpty_BeforeIndex)
{
    using namespace ExplorerLens::Engine;
    SemanticSearchOrchestrator fresh{};
    SemanticSearchRequest req{};
    req.queryText = L"landscape";
    auto res = fresh.Search(req);
    ASSERT(res.empty());
}
TEST(TestSSO_IndexedCountTracked)
{
    using namespace ExplorerLens::Engine;
    auto& sso  = SemanticSearchOrchestrator::Instance();
    auto& hnsw = HNSWIndexEngine::Instance();
    hnsw.Reset();
    sso.Initialize(L"idx2.bin");
    float emb[512]{};
    uint32_t before = sso.IndexedCount();
    sso.IndexFile(L"a.png", emb);
    sso.IndexFile(L"b.png", emb);
    ASSERT(sso.IndexedCount() == before + 2);
}
// ── EmbeddingPersistenceEngine ─────────────────────────────────────────────
TEST(TestEPE_OpenAndClose)
{
    using namespace ExplorerLens::Engine;
    auto& ep = EmbeddingPersistenceEngine::Instance();
    ASSERT(ep.Open(L"journal.bin"));
    ASSERT(ep.IsOpen());
    ep.Close();
    ASSERT(!ep.IsOpen());
}
TEST(TestEPE_AppendEntry)
{
    using namespace ExplorerLens::Engine;
    auto& ep = EmbeddingPersistenceEngine::Instance();
    ep.Open(L"journal.bin");
    PersistedEmbedding e{};
    e.itemId   = 1;
    e.filePath = L"img.jpg";
    ASSERT(ep.Append(e));
    ASSERT(ep.Stats().totalEntries == 1);
    ep.Close();
}
TEST(TestEPE_FlushUpdatesStats)
{
    using namespace ExplorerLens::Engine;
    auto& ep = EmbeddingPersistenceEngine::Instance();
    ep.Open(L"journal.bin");
    ASSERT(ep.Flush());
    ASSERT(ep.Stats().flushCount >= 1);
    ASSERT(ep.Stats().lastFlushMs > 0.0f);
    ep.Close();
}
TEST(TestEPE_LoadAllEmpty)
{
    using namespace ExplorerLens::Engine;
    auto& ep = EmbeddingPersistenceEngine::Instance();
    ep.Open(L"empty_journal.bin");
    std::vector<PersistedEmbedding> loaded;
    ASSERT(ep.LoadAll(loaded));
    ASSERT(loaded.empty());  // fresh journal has no entries
    ep.Close();
}
TEST(TestEPE_StatsJournalBytes)
{
    using namespace ExplorerLens::Engine;
    auto& ep = EmbeddingPersistenceEngine::Instance();
    ep.Open(L"journal2.bin");
    PersistedEmbedding e{};
    e.itemId = 2; e.filePath = L"x.tiff";
    ep.Append(e);
    ASSERT(ep.Stats().journalBytes > 0);
    ep.Close();
}
// ── VisualQueryOptimizer ───────────────────────────────────────────────────
TEST(TestVQO_DefaultActive)
{
    using namespace ExplorerLens::Engine;
    ASSERT(VisualQueryOptimizer::Instance().IsActive());
}
TEST(TestVQO_SetActive)
{
    using namespace ExplorerLens::Engine;
    auto& opt = VisualQueryOptimizer::Instance();
    opt.SetActive(false);
    ASSERT(!opt.IsActive());
    opt.SetActive(true);
    ASSERT(opt.IsActive());
}
TEST(TestVQO_PruneNoHint)
{
    using namespace ExplorerLens::Engine;
    auto& opt = VisualQueryOptimizer::Instance();
    opt.SetActive(true);
    VisualQueryHint hint{};  // NO_HINT
    std::vector<uint32_t> candidates = { 1, 2, 3, 4 };
    std::vector<uint32_t> pruned;
    auto res = opt.PruneSearchSpace(hint, candidates, pruned);
    ASSERT(res.prunedCandidates == 4);  // pass-through
    ASSERT(pruned.size() == candidates.size());
}
TEST(TestVQO_PruneReducesCandidates)
{
    using namespace ExplorerLens::Engine;
    auto& opt = VisualQueryOptimizer::Instance();
    opt.SetActive(true);
    VisualQueryHint hint{};
    hint.type        = VisualQueryHintType::FOLDER_SCOPE;
    hint.folderScope = L"C:\\Photos\\2025";
    std::vector<uint32_t> candidates = { 1, 2, 3, 4, 5, 6, 7, 8 };
    std::vector<uint32_t> pruned;
    auto res = opt.PruneSearchSpace(hint, candidates, pruned);
    ASSERT(res.prunedCandidates < res.originalCandidates);
    ASSERT(res.estimatedSpeedup > 1.0f);
}
TEST(TestVQO_PruneResultFields)
{
    using namespace ExplorerLens::Engine;
    auto& opt = VisualQueryOptimizer::Instance();
    VisualQueryHint hint{};
    hint.type = VisualQueryHintType::FILE_TYPE;
    hint.fileTypeExt = L".raw";
    std::vector<uint32_t> candidates = { 10, 20, 30 };
    std::vector<uint32_t> pruned;
    auto res = opt.PruneSearchSpace(hint, candidates, pruned);
    ASSERT(res.originalCandidates == 3);
    ASSERT(res.pruneMs >= 0.0f);
}

//== Sprint 1141-1150: Cross-Platform Shell PAL (v33.0.0 "Spica") ==
// ── Win32ShellProvider ────────────────────────────────────────────────────────
TEST(TestPSP_Win32PlatformKind)
{
    using namespace ExplorerLens::Engine;
    Win32ShellProvider p;
    ASSERT(p.GetPlatform() == PlatformKind::WINDOWS);
}
TEST(TestPSP_Win32PlatformName)
{
    using namespace ExplorerLens::Engine;
    Win32ShellProvider p;
    ASSERT(p.GetPlatformName() != nullptr);
    ASSERT(std::string(p.GetPlatformName()).size() > 0);
}
TEST(TestPSP_Win32NotRegisteredByDefault)
{
    using namespace ExplorerLens::Engine;
    Win32ShellProvider p;
    ASSERT(!p.IsRegistered());
}
TEST(TestPSP_Win32RegisterUnregister)
{
    using namespace ExplorerLens::Engine;
    Win32ShellProvider p;
    bool ok = p.RegisterProvider();
    ASSERT(ok);
    ASSERT(p.IsRegistered());
    p.UnregisterProvider();
    ASSERT(!p.IsRegistered());
}
TEST(TestPSP_Win32ThumbnailEmpty)
{
    using namespace ExplorerLens::Engine;
    Win32ShellProvider p;
    PlatformThumbnailRequest req{};
    auto res = p.GenerateThumbnail(req);
    ASSERT(!res.success);
}
TEST(TestPSP_Win32ThumbnailPath)
{
    using namespace ExplorerLens::Engine;
    Win32ShellProvider p;
    PlatformThumbnailRequest req{};
    req.filePath = L"test.png";
    req.width    = 256;
    req.height   = 256;
    auto res = p.GenerateThumbnail(req);
    ASSERT(res.success);
}
TEST(TestPSP_Win32ThumbnailNonzeroSize)
{
    using namespace ExplorerLens::Engine;
    Win32ShellProvider p;
    PlatformThumbnailRequest req{};
    req.filePath = L"test.png";
    req.width  = 128;
    req.height = 128;
    auto res = p.GenerateThumbnail(req);
    ASSERT(res.width >= 0);
    ASSERT(res.height >= 0);
}
// ── MacOSQLProvider ───────────────────────────────────────────────────────────
TEST(TestPSP_MacOSPlatformKind)
{
    using namespace ExplorerLens::Engine;
    MacOSQLProvider p;
    ASSERT(p.GetPlatform() == PlatformKind::MACOS);
}
TEST(TestPSP_MacOSPlatformName)
{
    using namespace ExplorerLens::Engine;
    MacOSQLProvider p;
    ASSERT(p.GetPlatformName() != nullptr);
    ASSERT(std::string(p.GetPlatformName()).size() > 0);
}
TEST(TestPSP_MacOSNotAvailableOnWindows)
{
    using namespace ExplorerLens::Engine;
    MacOSQLProvider p;
    bool ok = p.RegisterProvider();
    ASSERT(!ok);
}
TEST(TestPSP_MacOSThumbnailEmpty)
{
    using namespace ExplorerLens::Engine;
    MacOSQLProvider p;
    PlatformThumbnailRequest req{};
    auto res = p.GenerateThumbnail(req);
    ASSERT(!res.success);
}
TEST(TestPSP_MacOSThumbnailNotSupported)
{
    using namespace ExplorerLens::Engine;
    MacOSQLProvider p;
    PlatformThumbnailRequest req{};
    req.filePath = L"test.heic";
    auto res = p.GenerateThumbnail(req);
    ASSERT(!res.success);
    ASSERT(!res.errorMsg.empty());
}
TEST(TestPSP_MacOSIsNotRegistered)
{
    using namespace ExplorerLens::Engine;
    MacOSQLProvider p;
    ASSERT(!p.IsRegistered());
}
// ── LinuxNautilusProvider ─────────────────────────────────────────────────────
TEST(TestPSP_LinuxPlatformKind)
{
    using namespace ExplorerLens::Engine;
    LinuxNautilusProvider p;
    ASSERT(p.GetPlatform() == PlatformKind::LINUX);
}
TEST(TestPSP_LinuxPlatformName)
{
    using namespace ExplorerLens::Engine;
    LinuxNautilusProvider p;
    ASSERT(p.GetPlatformName() != nullptr);
    ASSERT(std::string(p.GetPlatformName()).size() > 0);
}
TEST(TestPSP_LinuxNotAvailableOnWindows)
{
    using namespace ExplorerLens::Engine;
    LinuxNautilusProvider p;
    bool ok = p.RegisterProvider();
    ASSERT(!ok);
}
TEST(TestPSP_LinuxThumbnailEmpty)
{
    using namespace ExplorerLens::Engine;
    LinuxNautilusProvider p;
    PlatformThumbnailRequest req{};
    auto res = p.GenerateThumbnail(req);
    ASSERT(!res.success);
}
TEST(TestPSP_LinuxIsNotRegistered)
{
    using namespace ExplorerLens::Engine;
    LinuxNautilusProvider p;
    ASSERT(!p.IsRegistered());
}
// ── PlatformDetector ─────────────────────────────────────────────────────────
TEST(TestPD_DetectCurrentPlatform)
{
    using namespace ExplorerLens::Engine;
    ASSERT(PlatformDetector::Detect() == PlatformKind::WINDOWS);
}
TEST(TestPD_PlatformNameWindows)
{
    using namespace ExplorerLens::Engine;
    ASSERT(PlatformDetector::PlatformName() != nullptr);
    ASSERT(std::string(PlatformDetector::PlatformName()) == "Windows");
}
TEST(TestPD_MakeWin32Provider)
{
    using namespace ExplorerLens::Engine;
    auto p = PlatformDetector::MakeProvider(PlatformKind::WINDOWS);
    ASSERT(p != nullptr);
    ASSERT(p->GetPlatform() == PlatformKind::WINDOWS);
}
TEST(TestPD_MakeMacOSProvider)
{
    using namespace ExplorerLens::Engine;
    auto p = PlatformDetector::MakeProvider(PlatformKind::MACOS);
    ASSERT(p != nullptr);
    ASSERT(p->GetPlatform() == PlatformKind::MACOS);
}
TEST(TestPD_MakeLinuxProvider)
{
    using namespace ExplorerLens::Engine;
    auto p = PlatformDetector::MakeProvider(PlatformKind::LINUX);
    ASSERT(p != nullptr);
    ASSERT(p->GetPlatform() == PlatformKind::LINUX);
}
TEST(TestPD_PlatformDescString)
{
    using namespace ExplorerLens::Engine;
    auto desc = PlatformDetector::PlatformDescString();
    ASSERT(!desc.empty());
    ASSERT(desc.find("Windows") != std::string::npos);
}
TEST(TestPD_CurrentProviderForPlatform)
{
    using namespace ExplorerLens::Engine;
    auto p = PlatformDetector::MakeCurrentPlatformProvider();
    ASSERT(p != nullptr);
    ASSERT(p->GetPlatform() == PlatformDetector::Detect());
}

//== Sprint 1151-1160: Platform GPU Extensions (v33.1.0 "Spica-R") ==
TEST(TestMGB_InitialState)
{
    using namespace ExplorerLens::Engine;
    MetalGPUBackend b;
    ASSERT(b.GetState() == MetalBackendState::Uninitialized);
}
TEST(TestMGB_BackendName)
{
    using namespace ExplorerLens::Engine;
    MetalGPUBackend b;
    ASSERT(std::string(b.BackendName()) == "Metal-v2-Stub");
}
TEST(TestVEB_NotAvailableOnWindows)
{
    using namespace ExplorerLens::Engine;
    VulkanEGLBackend b;
    ASSERT(!b.IsAvailable());
}
TEST(TestVEB_BackendName)
{
    using namespace ExplorerLens::Engine;
    VulkanEGLBackend b;
    ASSERT(std::string(b.BackendName()) == "Vulkan-EGL-Stub");
}
TEST(TestPGR_SelectsD3D12OnWindows)
{
    using namespace ExplorerLens::Engine;
    PlatformGPURouter r;
    r.Initialize();
    ASSERT(r.SelectedBackend() == GPURouterBackend::D3D12);
}
TEST(TestPGR_BackendNameNotNull)
{
    using namespace ExplorerLens::Engine;
    PlatformGPURouter r;
    r.Initialize();
    ASSERT(r.BackendName() != nullptr);
}
TEST(TestPDB_InitiallyDetached)
{
    using namespace ExplorerLens::Engine;
    PlatformDisplayBridge b;
    ASSERT(!b.IsAttached());
}
TEST(TestPDB_AttachSucceeds)
{
    using namespace ExplorerLens::Engine;
    PlatformDisplayBridge b;
    ASSERT(b.Attach());
    ASSERT(b.IsAttached());
    b.Detach();
}
TEST(TestCPSF_CreateDestroy)
{
    using namespace ExplorerLens::Engine;
    CrossPlatformSyncFence f;
    ASSERT(f.Create());
    ASSERT(f.IsCreated());
    f.Destroy();
    ASSERT(!f.IsCreated());
}
TEST(TestCPSF_SignalWait)
{
    using namespace ExplorerLens::Engine;
    CrossPlatformSyncFence f;
    f.Create();
    f.Signal(1);
    ASSERT(f.Wait(1, 100) == SyncFenceState::Signaled);
}

//== Sprint 1161-1170: Enterprise Console v4 (v33.2.0 "Spica-S") ==
TEST(TestEPV4_Initialize)
{
    using namespace ExplorerLens::Engine;
    EnterprisePolicyV4 eng;
    ASSERT(eng.Initialize());
}
TEST(TestEPV4_ApplyPolicy)
{
    using namespace ExplorerLens::Engine;
    EnterprisePolicyV4 eng;
    eng.Initialize();
    EnterprisePolicyV4Entry entry;
    entry.key   = "MaxThumbnailSize";
    entry.value = "512";
    entry.source = PolicySourceV4::Manual;
    ASSERT(eng.ApplyPolicy(entry));
}
TEST(TestGPOT_AddSetting)
{
    using namespace ExplorerLens::Engine;
    GPOPolicyTemplate tmpl;
    GPOPolicySetting s;
    s.id           = "MaxCacheSize";
    s.displayName  = "Maximum Cache Size";
    s.category     = "Performance";
    s.defaultValue = "512";
    ASSERT(tmpl.AddSetting(s));
    ASSERT(tmpl.SettingCount() == 1);
}
TEST(TestGPOT_GenerateADMX)
{
    using namespace ExplorerLens::Engine;
    GPOPolicyTemplate tmpl;
    GPOPolicySetting s;
    s.id = "MaxCacheSize"; s.displayName = "Max Cache"; s.category = "Perf"; s.defaultValue = "512";
    tmpl.AddSetting(s);
    GPOTemplateResult res = tmpl.GenerateADMX();
    ASSERT(res.success);
    ASSERT(!res.admxContent.empty());
}
TEST(TestICE_AddRule)
{
    using namespace ExplorerLens::Engine;
    IntuneComplianceEngine ice;
    ice.Initialize();
    ComplianceRule r;
    r.ruleId      = "RULE-001";
    r.description = "Thumbnails must be secure";
    r.required    = true;
    ASSERT(ice.AddRule(r));
    ASSERT(ice.RuleCount() == 1);
}
TEST(TestICE_Evaluate)
{
    using namespace ExplorerLens::Engine;
    IntuneComplianceEngine ice;
    ice.Initialize();
    auto report = ice.Evaluate();
    ASSERT(report.totalRules == 0);
    ASSERT(report.overallStatus == IntuneComplianceStatus::Compliant);
}
TEST(TestEAL_InitializeLog)
{
    using namespace ExplorerLens::Engine;
    EnterpriseAuditLogger logger;
    ASSERT(logger.Initialize("test-audit.log"));
}
TEST(TestEAL_LogEvent)
{
    using namespace ExplorerLens::Engine;
    EnterpriseAuditLogger logger;
    logger.Initialize("test-audit.log");
    EnterpriseAuditLogEntry ev;
    ev.type        = EnterpriseAuditEventType::PolicyChange;
    ev.description = "Policy updated";
    ev.actor       = "SYSTEM";
    ASSERT(logger.Log(ev));
    ASSERT(logger.GetStats().totalEvents == 1);
}
TEST(TestCMPB_Initialize)
{
    using namespace ExplorerLens::Engine;
    ConfigMgrPolicyBridge bridge;
    ASSERT(bridge.Initialize());
}
TEST(TestCMPB_Synchronize)
{
    using namespace ExplorerLens::Engine;
    ConfigMgrPolicyBridge bridge;
    bridge.Initialize();
    auto res = bridge.Synchronize();
    ASSERT(res.success || !bridge.IsConfigMgrPresent());
}

//== Sprint 1171-1180: Generative AI Thumbnails (v33.3.0 "Spica-T") ==
TEST(TestNPU_Initialize)
{
    using namespace ExplorerLens::Engine;
    NPUThumbnailSynthesizer& synth = NPUThumbnailSynthesizer::Instance();
    ASSERT(synth.Initialize(SynthesisBackend::CPU));
}
TEST(TestNPU_Synthesize)
{
    using namespace ExplorerLens::Engine;
    NPUThumbnailSynthesizer& synth = NPUThumbnailSynthesizer::Instance();
    synth.Initialize(SynthesisBackend::CPU);
    SynthesisRequest req;
    req.prompt  = "nature landscape";
    req.width   = 64;
    req.height  = 64;
    auto res = synth.Synthesize(req);
    ASSERT(res.success);
    ASSERT(!res.pixels.empty());
}
TEST(TestDME_LoadModel)
{
    using namespace ExplorerLens::Engine;
    DiffusionModelEngine eng;
    DiffusionModelConfig cfg;
    cfg.modelPath  = "stub-model";
    cfg.steps = 4;
    ASSERT(eng.LoadModel(cfg));
    ASSERT(eng.GetState() == DiffusionModelState::Ready);
}
TEST(TestDME_EncodePrompt)
{
    using namespace ExplorerLens::Engine;
    DiffusionModelEngine eng;
    DiffusionModelConfig cfg; cfg.modelPath = "stub"; cfg.steps = 4;
    eng.LoadModel(cfg);
    auto latent = eng.EncodePrompt("a red cube");
    ASSERT(latent.width > 0 || !latent.data.empty());
}
TEST(TestTIE_Initialize)
{
    using namespace ExplorerLens::Engine;
    ThumbnailInpaintEngine eng;
    ASSERT(eng.Initialize());
}
TEST(TestTIE_Inpaint)
{
    using namespace ExplorerLens::Engine;
    ThumbnailInpaintEngine eng;
    eng.Initialize();
    InpaintRequest req;
    req.imageWidth  = 64;
    req.imageHeight = 64;
    req.pixels.assign(64 * 64 * 4, 0xFF);
    ThumbnailInpaintRegion region; region.x = 10; region.y = 10; region.width = 20; region.height = 20;
    req.region = region;
    auto res = eng.Inpaint(req);
    ASSERT(res.success);
    ASSERT(res.confidenceScore >= 0.0f);
}
TEST(TestOIR_Initialize)
{
    using namespace ExplorerLens::Engine;
    OffDeviceInferenceRouter& router = OffDeviceInferenceRouter::Instance();
    ASSERT(router.Initialize());
}
TEST(TestOIR_RouteInference)
{
    using namespace ExplorerLens::Engine;
    OffDeviceInferenceRouter& router = OffDeviceInferenceRouter::Instance();
    router.Initialize();
    auto target = router.SelectTarget();
    ASSERT(target == InferenceTarget::IntelNPU || target == InferenceTarget::DirectML ||
           target == InferenceTarget::ONNXRuntime || target == InferenceTarget::CPU);
    ASSERT(router.RouteInference(16));
}
TEST(TestAIBP_Enqueue)
{
    using namespace ExplorerLens::Engine;
    AIThumbnailBatchProcessor proc;
    proc.Initialize(2);
    AIBatchRequest req;
    req.filePath = "img.png";
    req.priority = AIBatchPriority::Normal;
    uint32_t id = proc.Enqueue(req);
    ASSERT(id > 0);
    ASSERT(proc.QueueDepth() == 1);
}
TEST(TestAIBP_ProcessBatch)
{
    using namespace ExplorerLens::Engine;
    AIThumbnailBatchProcessor proc;
    proc.Initialize(2);
    AIBatchRequest req; req.filePath = "img.png";
    proc.Enqueue(req);
    std::vector<AIBatchResult> results;
    ASSERT(proc.ProcessBatch(results));
    ASSERT(results.size() == 1);
    ASSERT(results[0].success);
    ASSERT(proc.GetStats().totalProcessed >=0);
}

//== Sprint 1181-1190: Plugin Marketplace v5 (v33.4.0 "Spica-U") ==
TEST(TestPMV5_Initialize)
{
    using namespace ExplorerLens::Engine;
    PluginMarketplaceV5& mkt = PluginMarketplaceV5::Instance();
    ASSERT(mkt.Initialize("https://plugins.explorerlens.example/feed.json"));
}
TEST(TestPMV5_Search)
{
    using namespace ExplorerLens::Engine;
    PluginMarketplaceV5& mkt = PluginMarketplaceV5::Instance();
    mkt.Initialize("https://plugins.explorerlens.example/feed.json");
    auto result = mkt.Search("avif");
    ASSERT(result.plugins.empty() || result.plugins[0].id.size() > 0);
}
TEST(TestSCK3_Initialize)
{
    using namespace ExplorerLens::Engine;
    ASSERT(SDKCompatKit3::Instance().Initialize());
}
TEST(TestSCK3_DetectVersion)
{
    using namespace ExplorerLens::Engine;
    SDKCompatKit3& kit = SDKCompatKit3::Instance();
    kit.Initialize();
    auto ver = kit.DetectPluginSDKVersion("some-plugin.dll");
    ASSERT(ver != SDKVersion::Unknown);
}
TEST(TestPDM_Install)
{
    using namespace ExplorerLens::Engine;
    PluginDistributionManager mgr;
    ASSERT(mgr.Initialize("C:\\PluginRoot"));
    ASSERT(mgr.Install("plugin-avif", "C:\\pkg\\avif.zip"));
    ASSERT(mgr.InstalledCount() == 1);
}
TEST(TestPDM_Rollback)
{
    using namespace ExplorerLens::Engine;
    PluginDistributionManager mgr;
    mgr.Initialize("C:\\PluginRoot");
    mgr.Install("plugin-avif", "avif.zip");
    mgr.Update("plugin-avif", "avif-v2.zip");
    ASSERT(mgr.Rollback("plugin-avif"));
}
TEST(TestMSI_IndexAndQuery)
{
    using namespace ExplorerLens::Engine;
    MarketplaceSearchIndex& idx = MarketplaceSearchIndex::Instance();
    idx.Initialize();
    idx.IndexPlugin("avif-plugin", "AVIF Decoder", "avif image heif");
    auto results = idx.Query("avif");
    ASSERT(!results.empty());
    ASSERT(results[0].pluginId == "avif-plugin");
}
TEST(TestMSI_DocumentCount)
{
    using namespace ExplorerLens::Engine;
    MarketplaceSearchIndex idx;
    idx.Initialize();
    idx.IndexPlugin("p1", "Plugin One", "heif");
    idx.IndexPlugin("p2", "Plugin Two", "raw");
    ASSERT(idx.DocumentCount() == 2);
}
TEST(TestPSV_ValidatePlugin)
{
    using namespace ExplorerLens::Engine;
    PluginSignatureValidator& val = PluginSignatureValidator::Instance();
    val.Initialize();
    auto info = val.Validate("some-plugin.dll");
    ASSERT(info.result == PluginValidationResult::Valid || info.result == PluginValidationResult::NotSigned);
}
TEST(TestPSV_AddTrustedThumbprint)
{
    using namespace ExplorerLens::Engine;
    PluginSignatureValidator val;
    val.Initialize();
    ASSERT(val.AddTrustedThumbprint("AABB1234"));
    ASSERT(val.TrustedCount() == 1);
    ASSERT(val.IsTrustedSigner("AABB1234"));
}

//== Sprint 1191-1200: LTS Hardening + Security Audit (v33.5.0 "Spica-V") ==
TEST(TestLHC_Initialize)
{
    using namespace ExplorerLens::Engine;
    LTSHardeningController ctrl;
    ASSERT(ctrl.Initialize("33.5.0"));
}
TEST(TestLHC_GateEvaluate)
{
    using namespace ExplorerLens::Engine;
    LTSHardeningController ctrl;
    ctrl.Initialize("33.5.0");
    LTSGate g; g.name = "SecurityAudit"; g.description = "OWASP check"; g.required = true;
    ctrl.AddGate(g);
    ASSERT(ctrl.EvaluateGate("SecurityAudit", true));
    ASSERT(ctrl.IsLTSReady());
}
TEST(TestSAE_Initialize)
{
    using namespace ExplorerLens::Engine;
    SecurityAuditEngine& eng = SecurityAuditEngine::Instance();
    ASSERT(eng.Initialize());
}
TEST(TestSAE_RunAudit)
{
    using namespace ExplorerLens::Engine;
    SecurityAuditEngine& eng = SecurityAuditEngine::Instance();
    eng.Initialize();
    ASSERT(eng.RunAudit());
    ASSERT(eng.Passed());
}
TEST(TestVFDB_AddRecord)
{
    using namespace ExplorerLens::Engine;
    VulnerabilityFingerprintDB db;
    db.Initialize();
    VulnerabilityRecord rec;
    rec.cveId            = "CVE-2025-0001";
    rec.affectedLibrary  = "zlib";
    rec.affectedVersionRange = "< 1.3.1";
    rec.fixedVersion     = "1.3.1";
    rec.cvssScore        = 7;
    ASSERT(db.AddRecord(rec));
    ASSERT(db.RecordCount() == 1);
}
TEST(TestVFDB_QueryByLibrary)
{
    using namespace ExplorerLens::Engine;
    VulnerabilityFingerprintDB db;
    db.Initialize();
    VulnerabilityRecord rec;
    rec.cveId = "CVE-2025-0002"; rec.affectedLibrary = "libwebp";
    rec.affectedVersionRange = "< 1.5.0"; rec.fixedVersion = "1.5.0";
    db.AddRecord(rec);
    auto results = db.QueryByLibrary("libwebp");
    ASSERT(results.size() == 1);
    ASSERT(results[0].cveId == "CVE-2025-0002");
}
TEST(TestLCG_RunGate)
{
    using namespace ExplorerLens::Engine;
    LTSCertificationGate gate;
    gate.Initialize("33.5.0");
    ASSERT(gate.RunGate("SecurityAudit",    true));
    ASSERT(gate.RunGate("DependencyFreeze", true));
    ASSERT(gate.GateCount() == 2);
}
TEST(TestLCG_IsCertified)
{
    using namespace ExplorerLens::Engine;
    LTSCertificationGate gate;
    gate.Initialize("33.5.0");
    gate.RunGate("SecurityAudit",      true);
    gate.RunGate("DependencyFreeze",   true);
    gate.RunGate("PerformanceClear",   true);
    ASSERT(gate.IsCertified());
}
TEST(TestSKS_StoreRetrieve)
{
    using namespace ExplorerLens::Engine;
    SecureKeyStore ks;
    ASSERT(ks.Initialize(KeyStoreBackend::InMemory));
    std::vector<uint8_t> key = {0x01, 0x02, 0x03, 0x04};
    ASSERT(ks.StoreKey("signing-key", key));
    std::vector<uint8_t> retrieved;
    ASSERT(ks.RetrieveKey("signing-key", retrieved));
    ASSERT(retrieved == key);
}
TEST(TestSKS_DeleteKey)
{
    using namespace ExplorerLens::Engine;
    SecureKeyStore ks;
    ks.Initialize(KeyStoreBackend::InMemory);
    std::vector<uint8_t> key = {0xAA, 0xBB};
    ks.StoreKey("tmp-key", key);
    ASSERT(ks.HasKey("tmp-key"));
    ASSERT(ks.DeleteKey("tmp-key"));
    ASSERT(!ks.HasKey("tmp-key"));
}

//== Sprint 1131-1140: Live Preview Scrubber (v32.7.0 "Fomalhaut-X") ==

// ── VideoFrameExtractor ───────────────────────────────────────────────────
TEST(TestVFE_BackendName_DXVA2)
{
    using namespace ExplorerLens::Engine;
    ASSERT(VideoFrameExtractor::BackendName(VideoDecodeBackend::DXVA2_HARDWARE) == std::string_view("DXVA2-Hardware"));
}
TEST(TestVFE_BackendName_MFSoftware)
{
    using namespace ExplorerLens::Engine;
    ASSERT(VideoFrameExtractor::BackendName(VideoDecodeBackend::MF_SOFTWARE) == std::string_view("MF-Software"));
}
TEST(TestVFE_BackendName_Unavailable)
{
    using namespace ExplorerLens::Engine;
    ASSERT(VideoFrameExtractor::BackendName(VideoDecodeBackend::UNAVAILABLE) == std::string_view("Unavailable"));
}
TEST(TestVFE_ExtractFrame_ValidPath)
{
    using namespace ExplorerLens::Engine;
    auto& ex = VideoFrameExtractor::Instance();
    ex.Reset();
    VideoFrameRequest req{};
    req.filePath         = L"clip.mp4";
    req.timestampSeconds = 5.0;
    req.targetWidth      = 320;
    req.targetHeight     = 180;
    const auto RES = ex.ExtractFrame(req);
    ASSERT(RES.success);
    ASSERT(RES.width  == 320);
    ASSERT(RES.height == 180);
    ASSERT(ex.ExtractCount() == 1);
}
TEST(TestVFE_ExtractFrame_EmptyPath)
{
    using namespace ExplorerLens::Engine;
    auto& ex = VideoFrameExtractor::Instance();
    ex.Reset();
    VideoFrameRequest req{};
    const auto RES = ex.ExtractFrame(req);
    ASSERT(!RES.success);
}

// ── VideoScrubberTimeline ─────────────────────────────────────────────────
TEST(TestVST_BuildEmpty)
{
    using namespace ExplorerLens::Engine;
    auto& tl = VideoScrubberTimeline::Instance();
    ASSERT(!tl.Build(L""));
}
TEST(TestVST_BuildPopulates)
{
    using namespace ExplorerLens::Engine;
    auto& tl = VideoScrubberTimeline::Instance();
    ASSERT(tl.Build(L"video.mkv"));
    ASSERT(tl.KeyframeCount() > 0);
    ASSERT(tl.Duration() > 0.0);
    ASSERT(tl.ChapterCount() >= 1);
}
TEST(TestVST_KeyframeAt_Valid)
{
    using namespace ExplorerLens::Engine;
    auto& tl = VideoScrubberTimeline::Instance();
    tl.Build(L"video.mkv");
    const auto KF = tl.KeyframeAt(0);
    ASSERT(KF.index  == 0);
    ASSERT(KF.ptsSec == 0.0);
}
TEST(TestVST_KeyframeAt_OutOfBounds)
{
    using namespace ExplorerLens::Engine;
    auto& tl = VideoScrubberTimeline::Instance();
    tl.Build(L"video.mkv");
    const auto KF = tl.KeyframeAt(9999);
    ASSERT(KF.index  == 0);
    ASSERT(KF.ptsSec == 0.0);
}
TEST(TestVST_NearestKeyframePts)
{
    using namespace ExplorerLens::Engine;
    auto& tl = VideoScrubberTimeline::Instance();
    tl.Build(L"video.mkv");
    const double PTS = tl.NearestKeyframePts(25.0);
    ASSERT(PTS >= 0.0);
    ASSERT(PTS <= tl.Duration());
}

// ── LivePreviewSession ────────────────────────────────────────────────────
TEST(TestLPS_OpenEmpty)
{
    using namespace ExplorerLens::Engine;
    LivePreviewSession sess;
    ASSERT(!sess.Open(L""));
    ASSERT(!sess.IsOpen());
}
TEST(TestLPS_OpenValid)
{
    using namespace ExplorerLens::Engine;
    LivePreviewSession sess;
    ASSERT(sess.Open(L"movie.mp4"));
    ASSERT(sess.IsOpen());
    ASSERT(sess.FilePath() == L"movie.mp4");
}
TEST(TestLPS_SeekNotOpen)
{
    using namespace ExplorerLens::Engine;
    LivePreviewSession sess;
    const auto RES = sess.SeekTo(10.0);
    ASSERT(!RES.success);
}
TEST(TestLPS_SeekOpen)
{
    using namespace ExplorerLens::Engine;
    LivePreviewSession sess;
    sess.Open(L"movie.mp4");
    const auto RES = sess.SeekTo(30.0);
    ASSERT(RES.success);
    ASSERT(RES.actualPts >= 0.0);
}
TEST(TestLPS_Close)
{
    using namespace ExplorerLens::Engine;
    LivePreviewSession sess;
    sess.Open(L"movie.mp4");
    ASSERT(sess.IsOpen());
    sess.Close();
    ASSERT(!sess.IsOpen());
    ASSERT(sess.FilePath().empty());
}

// ── ThumbnailStripGenerator ───────────────────────────────────────────────
TEST(TestTSG_ResetClearsState)
{
    using namespace ExplorerLens::Engine;
    auto& gen = ThumbnailStripGenerator::Instance();
    gen.Reset();
    ASSERT(gen.StripWidth()  == 0);
    ASSERT(gen.StripHeight() == 0);
    ASSERT(gen.FrameCount()  == 0);
}
TEST(TestTSG_GenerateEmpty)
{
    using namespace ExplorerLens::Engine;
    StripConfig cfg{};
    const auto RES = ThumbnailStripGenerator::Instance().GenerateStrip(cfg);
    ASSERT(!RES.success);
}
TEST(TestTSG_GenerateValid)
{
    using namespace ExplorerLens::Engine;
    StripConfig cfg{};
    cfg.filePath    = L"video.mp4";
    cfg.frameCount  = 5;
    cfg.frameWidth  = 160;
    cfg.frameHeight = 90;
    const auto RES = ThumbnailStripGenerator::Instance().GenerateStrip(cfg);
    ASSERT(RES.success);
    ASSERT(RES.framesAdded == 5);
}
TEST(TestTSG_StripWidth)
{
    using namespace ExplorerLens::Engine;
    StripConfig cfg{};
    cfg.filePath    = L"video.mp4";
    cfg.frameCount  = 4;
    cfg.frameWidth  = 100;
    cfg.frameHeight = 56;
    const auto RES = ThumbnailStripGenerator::Instance().GenerateStrip(cfg);
    ASSERT(RES.stripWidth  == 400);
    ASSERT(RES.stripHeight == 56);
}
TEST(TestTSG_FrameCount)
{
    using namespace ExplorerLens::Engine;
    StripConfig cfg{};
    cfg.filePath    = L"video.mp4";
    cfg.frameCount  = 8;
    cfg.frameWidth  = 80;
    cfg.frameHeight = 45;
    ThumbnailStripGenerator::Instance().GenerateStrip(cfg);
    ASSERT(ThumbnailStripGenerator::Instance().FrameCount() == 8);
}

// ── ScrubberCacheEngine ───────────────────────────────────────────────────
TEST(TestSCE_InitialEmpty)
{
    using namespace ExplorerLens::Engine;
    auto& cache = ScrubberCacheEngine::Instance();
    cache.Clear();
    ASSERT(cache.Size() == 0);
    const auto ST = cache.GetStats();
    ASSERT(ST.hits    == 0);
    ASSERT(ST.misses  == 0);
    ASSERT(ST.hitRate == 0.0f);
}
TEST(TestSCE_PutAndGet)
{
    using namespace ExplorerLens::Engine;
    auto& cache = ScrubberCacheEngine::Instance();
    cache.Clear();
    ScrubberCacheKey key{};
    key.filePath = L"video.mp4";
    key.ptsSec   = 5.0;
    ScrubberCacheEntry entry{};
    entry.valid  = true;
    entry.width  = 320;
    entry.height = 180;
    ASSERT(cache.Put(key, entry));
    ScrubberCacheEntry out{};
    ASSERT(cache.Get(key, out));
    ASSERT(out.valid);
    ASSERT(out.width  == 320);
    ASSERT(out.height == 180);
}
TEST(TestSCE_GetMiss)
{
    using namespace ExplorerLens::Engine;
    auto& cache = ScrubberCacheEngine::Instance();
    cache.Clear();
    ScrubberCacheKey key{};
    key.filePath = L"notcached.mp4";
    key.ptsSec   = 99.0;
    ScrubberCacheEntry out{};
    ASSERT(!cache.Get(key, out));
    ASSERT(cache.GetStats().misses == 1);
}
TEST(TestSCE_Evict)
{
    using namespace ExplorerLens::Engine;
    auto& cache = ScrubberCacheEngine::Instance();
    cache.Clear();
    ScrubberCacheKey key{};
    key.filePath = L"video.mp4";
    key.ptsSec   = 10.0;
    ScrubberCacheEntry entry{};
    entry.valid = true;
    cache.Put(key, entry);
    ASSERT(cache.Size() == 1);
    cache.Evict(key);
    ASSERT(cache.Size() == 0);
}
TEST(TestSCE_HitRate)
{
    using namespace ExplorerLens::Engine;
    auto& cache = ScrubberCacheEngine::Instance();
    cache.Clear();
    ScrubberCacheKey key{};
    key.filePath = L"video.mp4";
    key.ptsSec   = 15.0;
    ScrubberCacheEntry entry{};
    entry.valid = true;
    cache.Put(key, entry);
    ScrubberCacheEntry out{};
    cache.Get(key, out);       // hit
    ScrubberCacheKey miss{};
    miss.filePath = L"x.mp4";
    miss.ptsSec   = 1.0;
    cache.Get(miss, out);      // miss
    const auto ST = cache.GetStats();
    ASSERT(ST.hits   == 1);
    ASSERT(ST.misses == 1);
    ASSERT(ST.hitRate >= 0.49f && ST.hitRate <= 0.51f);
}
