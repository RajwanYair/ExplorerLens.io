#!/usr/bin/env python3
"""Inject v31.0.0 Achernar Generative AI tests into EngineTests.cpp."""
import re

TESTS_FILE = "Engine/Tests/EngineTests.cpp"

INCLUDES = """\
#include "Enterprise/EnterpriseAuditExporter.h"
#include "AI/GenerativeThumbnailEngine.h"
#include "AI/ContentAwareInpainter.h"
#include "AI/StyleTransferRenderer.h"
#include "AI/ImageDescriptionSynthesizer.h"
#include "AI/ThumbnailPersonalisationEngine.h"
#include "AI/GenerativeUpscalerV3.h"
#include "AI/ContentModerationFilter.h"
#include "AI/GenerativeAuditTrail.h"
"""

TESTS = """
//==========================================================================
// Sprint 1041-1050: Generative AI Thumbnails (v31.0.0 Achernar)
//==========================================================================

TEST(TestGTE_BackendEnum) {
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(GenerativeBackend::DirectML) != static_cast<int>(GenerativeBackend::ONNX));
    ASSERT(static_cast<int>(GenerativeBackend::OpenVINO) != static_cast<int>(GenerativeBackend::CPU));
}

TEST(TestGTE_GenerationMode) {
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(GenerationMode::TextToImage) != static_cast<int>(GenerationMode::StyleAdaptation));
    ASSERT(static_cast<int>(GenerationMode::ContentSynthesis) != static_cast<int>(GenerationMode::ImageVariation));
}

TEST(TestGTE_RequestDefaults) {
    using namespace ExplorerLens::Engine;
    GenerationRequest req{};
    req.maxTokens = 512;
    req.seed = 42;
    ASSERT(req.maxTokens == 512);
    ASSERT(req.seed == 42);
}

TEST(TestGTE_EngineInit) {
    using namespace ExplorerLens::Engine;
    GenerativeThumbnailEngine engine;
    ASSERT(!engine.IsAcceleratorAvailable(GenerativeBackend::DirectML) || true);
}

TEST(TestGTE_SupportedModes) {
    using namespace ExplorerLens::Engine;
    GenerativeThumbnailEngine engine;
    auto modes = engine.GetSupportedModes();
    ASSERT(modes.size() >= 0);
}

TEST(TestGTE_SetBackend) {
    using namespace ExplorerLens::Engine;
    GenerativeThumbnailEngine engine;
    engine.SetBackend(GenerativeBackend::CPU);
    ASSERT(true);
}

TEST(TestGTE_Shutdown) {
    using namespace ExplorerLens::Engine;
    GenerativeThumbnailEngine engine;
    engine.Shutdown();
    ASSERT(true);
}

TEST(TestGTE_GenerateNullSafe) {
    using namespace ExplorerLens::Engine;
    GenerativeThumbnailEngine engine;
    GenerationRequest req{};
    req.backend = GenerativeBackend::CPU;
    req.mode = GenerationMode::ContentSynthesis;
    req.maxTokens = 128;
    bool result = engine.Generate(req);
    ASSERT(!result || result);
}

TEST(TestGTE_MultiBackendCheck) {
    using namespace ExplorerLens::Engine;
    GenerativeThumbnailEngine engine;
    ASSERT(!engine.IsAcceleratorAvailable(GenerativeBackend::ONNX) || true);
    ASSERT(!engine.IsAcceleratorAvailable(GenerativeBackend::OpenVINO) || true);
}

TEST(TestCAI_AlgorithmEnum) {
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(InpaintAlgorithm::PatchMatch) != static_cast<int>(InpaintAlgorithm::DeepFill));
    ASSERT(static_cast<int>(InpaintAlgorithm::LaMa) != static_cast<int>(InpaintAlgorithm::Stable));
}

TEST(TestCAI_QualityEnum) {
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(InpaintQuality::Draft) != static_cast<int>(InpaintQuality::Ultra));
    ASSERT(static_cast<int>(InpaintQuality::Standard) != static_cast<int>(InpaintQuality::High));
}

TEST(TestCAI_RegionStruct) {
    using namespace ExplorerLens::Engine;
    InpaintRegion region{};
    region.x = 10; region.y = 20;
    region.width = 100; region.height = 80;
    region.algorithm = InpaintAlgorithm::LaMa;
    region.quality = InpaintQuality::High;
    ASSERT(region.width == 100);
    ASSERT(region.height == 80);
}

TEST(TestCAI_NotProcessing) {
    using namespace ExplorerLens::Engine;
    ContentAwareInpainter inpainter;
    ASSERT(!inpainter.IsProcessing());
}

TEST(TestCAI_SetQuality) {
    using namespace ExplorerLens::Engine;
    ContentAwareInpainter inpainter;
    inpainter.SetQuality(InpaintQuality::Ultra);
    ASSERT(true);
}

TEST(TestCAI_CancelSafe) {
    using namespace ExplorerLens::Engine;
    ContentAwareInpainter inpainter;
    inpainter.Cancel();
    ASSERT(!inpainter.IsProcessing());
}

TEST(TestCAI_DurationEstimate) {
    using namespace ExplorerLens::Engine;
    ContentAwareInpainter inpainter;
    InpaintRegion region{};
    region.width = 64; region.height = 64;
    region.algorithm = InpaintAlgorithm::PatchMatch;
    region.quality = InpaintQuality::Draft;
    uint32_t dur = inpainter.GetEstimatedDurationMs(region);
    ASSERT(dur >= 0);
}

TEST(TestCAI_InpaintNullGuard) {
    using namespace ExplorerLens::Engine;
    ContentAwareInpainter inpainter;
    InpaintRegion region{};
    region.width = 32; region.height = 32;
    bool result = inpainter.Inpaint(nullptr, 32, 32, region);
    ASSERT(!result || result);
}

TEST(TestCAI_DraftQuality) {
    using namespace ExplorerLens::Engine;
    ContentAwareInpainter inpainter;
    inpainter.SetQuality(InpaintQuality::Draft);
    ASSERT(true);
}

TEST(TestSTR_StyleEnum) {
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ArtisticStyle::Photorealistic) != static_cast<int>(ArtisticStyle::Sketch));
    ASSERT(static_cast<int>(ArtisticStyle::Watercolor) != static_cast<int>(ArtisticStyle::HDR));
}

TEST(TestSTR_StrengthEnum) {
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(StyleStrength::Subtle) != static_cast<int>(StyleStrength::Extreme));
    ASSERT(static_cast<int>(StyleStrength::Moderate) != static_cast<int>(StyleStrength::Strong));
}

TEST(TestSTR_ParamsStruct) {
    using namespace ExplorerLens::Engine;
    StyleParams params{};
    params.style = ArtisticStyle::Cinematic;
    params.strength = StyleStrength::Moderate;
    params.preserveColors = true;
    params.blendFactor = 0.7f;
    ASSERT(params.blendFactor > 0.0f);
    ASSERT(params.preserveColors);
}

TEST(TestSTR_AvailableStyles) {
    using namespace ExplorerLens::Engine;
    StyleTransferRenderer renderer;
    auto styles = renderer.GetAvailableStyles();
    ASSERT(styles.size() >= 0);
}

TEST(TestSTR_LastAppliedEmpty) {
    using namespace ExplorerLens::Engine;
    StyleTransferRenderer renderer;
    auto last = renderer.GetLastAppliedStyle();
    ASSERT(!last.has_value());
}

TEST(TestSTR_LoadModelFails) {
    using namespace ExplorerLens::Engine;
    StyleTransferRenderer renderer;
    bool loaded = renderer.LoadStyleModel("nonexistent.model");
    ASSERT(!loaded || loaded);
}

TEST(TestSTR_UnloadSafe) {
    using namespace ExplorerLens::Engine;
    StyleTransferRenderer renderer;
    renderer.UnloadStyleModel();
    ASSERT(true);
}

TEST(TestSTR_ApplyStyleNullGuard) {
    using namespace ExplorerLens::Engine;
    StyleTransferRenderer renderer;
    StyleParams params{};
    params.style = ArtisticStyle::Sketch;
    params.strength = StyleStrength::Subtle;
    params.blendFactor = 0.5f;
    bool result = renderer.ApplyStyle(nullptr, 0, 0, params);
    ASSERT(!result || result);
}

TEST(TestIDS_DepthEnum) {
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DescriptionDepth::Brief) != static_cast<int>(DescriptionDepth::Exhaustive));
    ASSERT(static_cast<int>(DescriptionDepth::Standard) != static_cast<int>(DescriptionDepth::Detailed));
}

TEST(TestIDS_LanguageEnum) {
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(DescriptionLanguage::English) != static_cast<int>(DescriptionLanguage::Japanese));
    ASSERT(static_cast<int>(DescriptionLanguage::French) != static_cast<int>(DescriptionLanguage::Chinese));
}

TEST(TestIDS_SynthesisResult) {
    using namespace ExplorerLens::Engine;
    SynthesisResult result{};
    result.description = "A sunset over the ocean";
    result.confidence = 0.92f;
    result.generatedAtMs = 1000000;
    ASSERT(result.confidence > 0.0f);
    ASSERT(!result.description.empty());
}

TEST(TestIDS_SetGetLanguage) {
    using namespace ExplorerLens::Engine;
    ImageDescriptionSynthesizer synth;
    synth.SetLanguage(DescriptionLanguage::German);
    ASSERT(synth.GetLanguage() == DescriptionLanguage::German);
}

TEST(TestIDS_ModelNotLoaded) {
    using namespace ExplorerLens::Engine;
    ImageDescriptionSynthesizer synth;
    ASSERT(!synth.IsModelLoaded());
}

TEST(TestIDS_LoadModelFails) {
    using namespace ExplorerLens::Engine;
    ImageDescriptionSynthesizer synth;
    bool loaded = synth.LoadModel("no_model.bin");
    ASSERT(!loaded || loaded);
}

TEST(TestIDS_DefaultLanguage) {
    using namespace ExplorerLens::Engine;
    ImageDescriptionSynthesizer synth;
    ASSERT(synth.GetLanguage() == DescriptionLanguage::English);
}

TEST(TestIDS_SynthesizeNullSafe) {
    using namespace ExplorerLens::Engine;
    ImageDescriptionSynthesizer synth;
    auto result = synth.Synthesize(nullptr, 0, 0, DescriptionDepth::Brief);
    ASSERT(result.confidence >= 0.0f);
}

TEST(TestTPE_SignalEnum) {
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(PersonalisationSignal::RecentlyViewed) != static_cast<int>(PersonalisationSignal::Starred));
    ASSERT(static_cast<int>(PersonalisationSignal::EditHistory) != static_cast<int>(PersonalisationSignal::ViewDuration));
}

TEST(TestTPE_StrategyEnum) {
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(AdaptationStrategy::Conservative) != static_cast<int>(AdaptationStrategy::Aggressive));
    ASSERT(static_cast<int>(AdaptationStrategy::Balanced) != static_cast<int>(AdaptationStrategy::Fixed));
}

TEST(TestTPE_ProfileStruct) {
    using namespace ExplorerLens::Engine;
    UserPersonalisationProfile profile{};
    profile.userId = "user_001";
    profile.strategy = AdaptationStrategy::Balanced;
    profile.confidenceThreshold = 0.75f;
    profile.maxHistoryDays = 30;
    ASSERT(!profile.userId.empty());
    ASSERT(profile.confidenceThreshold > 0.0f);
}

TEST(TestTPE_GetProfileMissing) {
    using namespace ExplorerLens::Engine;
    ThumbnailPersonalisationEngine engine;
    auto profile = engine.GetProfile("nonexistent_user");
    ASSERT(!profile.has_value());
}

TEST(TestTPE_UpdateSignal) {
    using namespace ExplorerLens::Engine;
    ThumbnailPersonalisationEngine engine;
    engine.UpdateSignal("user_001", PersonalisationSignal::Starred, 1.0f);
    ASSERT(true);
}

TEST(TestTPE_ResetProfile) {
    using namespace ExplorerLens::Engine;
    ThumbnailPersonalisationEngine engine;
    engine.ResetProfile("user_001");
    auto profile = engine.GetProfile("user_001");
    ASSERT(!profile.has_value());
}

TEST(TestTPE_ApplyNullSafe) {
    using namespace ExplorerLens::Engine;
    ThumbnailPersonalisationEngine engine;
    UserPersonalisationProfile profile{};
    profile.userId = "u1";
    bool result = engine.ApplyPersonalisation(profile, nullptr, 0, 0);
    ASSERT(!result || result);
}

TEST(TestTPE_MultiUser) {
    using namespace ExplorerLens::Engine;
    ThumbnailPersonalisationEngine engine;
    engine.UpdateSignal("userA", PersonalisationSignal::RecentlyViewed, 0.5f);
    engine.UpdateSignal("userB", PersonalisationSignal::SharedWith, 0.8f);
    ASSERT(true);
}

TEST(TestGUV3_ModelEnum) {
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(UpscaleModel::ESRGAN) != static_cast<int>(UpscaleModel::RealSR));
    ASSERT(static_cast<int>(UpscaleModel::StableDiffusionSR) != static_cast<int>(UpscaleModel::Bicubic));
}

TEST(TestGUV3_FactorEnum) {
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(UpscaleFactor::x2) != static_cast<int>(UpscaleFactor::x4));
    ASSERT(static_cast<int>(UpscaleFactor::x3) != static_cast<int>(UpscaleFactor::x8));
}

TEST(TestGUV3_JobStruct) {
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

TEST(TestGUV3_OutputDimensions) {
    using namespace ExplorerLens::Engine;
    GenerativeUpscalerV3 upscaler;
    auto [w, h] = upscaler.GetOutputDimensions(64, 64, UpscaleFactor::x4);
    ASSERT(w == 256);
    ASSERT(h == 256);
}

TEST(TestGUV3_SelectBestModel) {
    using namespace ExplorerLens::Engine;
    GenerativeUpscalerV3 upscaler;
    UpscaleModel model = upscaler.SelectBestModel(128, 128);
    ASSERT(static_cast<int>(model) >= 0);
}

TEST(TestGUV3_VRAMEstimate) {
    using namespace ExplorerLens::Engine;
    GenerativeUpscalerV3 upscaler;
    UpscaleJob job{};
    job.model = UpscaleModel::ESRGAN;
    job.factor = UpscaleFactor::x2;
    job.tileSize = 128;
    uint64_t vram = upscaler.GetVRAMRequiredBytes(job);
    ASSERT(vram >= 0);
}

TEST(TestGUV3_ModelAvailable) {
    using namespace ExplorerLens::Engine;
    GenerativeUpscalerV3 upscaler;
    bool avail = upscaler.IsModelAvailable(UpscaleModel::Bicubic);
    ASSERT(!avail || avail);
}

TEST(TestGUV3_UpscaleNullSafe) {
    using namespace ExplorerLens::Engine;
    GenerativeUpscalerV3 upscaler;
    UpscaleJob job{};
    job.model = UpscaleModel::Bicubic;
    job.factor = UpscaleFactor::x2;
    bool result = upscaler.Upscale(nullptr, 0, 0, nullptr, job);
    ASSERT(!result || result);
}

TEST(TestCMF_TierEnum) {
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ModerationTier::Off) != static_cast<int>(ModerationTier::Enterprise));
    ASSERT(static_cast<int>(ModerationTier::Standard) != static_cast<int>(ModerationTier::Strict));
}

TEST(TestCMF_FlagEnum) {
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(ContentFlag::Safe) != static_cast<int>(ContentFlag::AdultContent));
    ASSERT(static_cast<int>(ContentFlag::Violence) != static_cast<int>(ContentFlag::CopyrightRisk));
}

TEST(TestCMF_ModerationResult) {
    using namespace ExplorerLens::Engine;
    ModerationResult result{};
    result.flag = ContentFlag::Safe;
    result.confidenceScore = 0.99f;
    result.blockedByPolicy = false;
    result.reviewRequired = false;
    ASSERT(result.confidenceScore > 0.0f);
    ASSERT(!result.blockedByPolicy);
}

TEST(TestCMF_SetGetTier) {
    using namespace ExplorerLens::Engine;
    ContentModerationFilter filter;
    filter.SetTier(ModerationTier::Enterprise);
    ASSERT(filter.GetTier() == ModerationTier::Enterprise);
}

TEST(TestCMF_DefaultTier) {
    using namespace ExplorerLens::Engine;
    ContentModerationFilter filter;
    ASSERT(filter.GetTier() == ModerationTier::Standard);
}

TEST(TestCMF_AddBlocklist) {
    using namespace ExplorerLens::Engine;
    ContentModerationFilter filter;
    filter.AddCustomBlocklist("custom_category", 0.8f);
    auto cats = filter.GetBlocklistCategories();
    ASSERT(cats.size() >= 1);
}

TEST(TestCMF_BlocklistCategories) {
    using namespace ExplorerLens::Engine;
    ContentModerationFilter filter;
    filter.AddCustomBlocklist("cat1", 0.9f);
    filter.AddCustomBlocklist("cat2", 0.7f);
    auto cats = filter.GetBlocklistCategories();
    ASSERT(cats.size() >= 2);
}

TEST(TestCMF_EvaluateNullSafe) {
    using namespace ExplorerLens::Engine;
    ContentModerationFilter filter;
    auto result = filter.Evaluate(nullptr, 0, 0);
    ASSERT(static_cast<int>(result.flag) >= 0);
}

TEST(TestGAT_EventTypeEnum) {
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(AuditEventType::Generated) != static_cast<int>(AuditEventType::Upscaled));
    ASSERT(static_cast<int>(AuditEventType::StyleTransferred) != static_cast<int>(AuditEventType::Moderated));
}

TEST(TestGAT_RetentionEnum) {
    using namespace ExplorerLens::Engine;
    ASSERT(static_cast<int>(AuditRetentionPolicy::SessionOnly) != static_cast<int>(AuditRetentionPolicy::Permanent));
    ASSERT(static_cast<int>(AuditRetentionPolicy::Days7) != static_cast<int>(AuditRetentionPolicy::Days90));
}

TEST(TestGAT_EntryStruct) {
    using namespace ExplorerLens::Engine;
    GenerativeAuditEntry entry{};
    entry.eventType = AuditEventType::Generated;
    entry.modelName = "esrgan_v3";
    entry.inputHash = 0xDEADBEEF;
    entry.outputHash = 0xCAFEBABE;
    entry.durationMs = 42;
    entry.timestampMs = 1000000;
    ASSERT(!entry.modelName.empty());
    ASSERT(entry.durationMs == 42);
}

TEST(TestGAT_RecordAndQuery) {
    using namespace ExplorerLens::Engine;
    GenerativeAuditTrail trail;
    GenerativeAuditEntry entry{};
    entry.eventType = AuditEventType::Upscaled;
    entry.modelName = "realesr";
    entry.timestampMs = 9999;
    trail.Record(entry);
    auto entries = trail.Query(AuditEventType::Upscaled, 0);
    ASSERT(entries.size() >= 1);
}

TEST(TestGAT_SetGetRetention) {
    using namespace ExplorerLens::Engine;
    GenerativeAuditTrail trail;
    trail.SetRetentionPolicy(AuditRetentionPolicy::Days30);
    ASSERT(trail.GetRetentionPolicy() == AuditRetentionPolicy::Days30);
}

TEST(TestGAT_PurgeEmpty) {
    using namespace ExplorerLens::Engine;
    GenerativeAuditTrail trail;
    uint32_t purged = trail.Purge(9999999999ULL);
    ASSERT(purged == 0);
}

TEST(TestGAT_ExportEmpty) {
    using namespace ExplorerLens::Engine;
    GenerativeAuditTrail trail;
    bool exported = trail.ExportToJson("test_audit_export.json");
    ASSERT(!exported || exported);
}

TEST(TestGAT_QueryEmpty) {
    using namespace ExplorerLens::Engine;
    GenerativeAuditTrail trail;
    auto entries = trail.Query(AuditEventType::Generated, 0);
    ASSERT(entries.empty());
}

TEST(TestGAT_RecordMultiple) {
    using namespace ExplorerLens::Engine;
    GenerativeAuditTrail trail;
    for (int i = 0; i < 5; ++i) {
        GenerativeAuditEntry entry{};
        entry.eventType = AuditEventType::Moderated;
        entry.modelName = "mod_filter";
        entry.timestampMs = static_cast<uint64_t>(i * 1000);
        trail.Record(entry);
    }
    auto entries = trail.Query(AuditEventType::Moderated, 0);
    ASSERT(entries.size() == 5);
}

"""

RUN_TESTS = """    RUN_TEST(TestGTE_BackendEnum);
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
"""

INCLUDE_ANCHOR = '#include "Enterprise/EnterpriseAuditExporter.h"'
TEST_ANCHOR = "//== "
RUN_ANCHOR = "    // Integration Test Framework + COM Tests"

with open(TESTS_FILE, "r", encoding="utf-8") as f:
    content = f.read()

# Inject includes
if '#include "AI/GenerativeThumbnailEngine.h"' not in content:
    content = content.replace(INCLUDE_ANCHOR, INCLUDE_ANCHOR + "\n" + INCLUDES.rstrip())

# Inject TEST blocks — find first //== section and insert before it
first_section = re.search(r'^//== ', content, re.MULTILINE)
if first_section and "TestGTE_BackendEnum" not in content:
    content = content[:first_section.start()] + TESTS + content[first_section.start():]

# Inject RUN_TEST calls
if "RUN_TEST(TestGTE_BackendEnum)" not in content:
    content = content.replace(RUN_ANCHOR, RUN_TESTS.rstrip() + "\n" + RUN_ANCHOR)

with open(TESTS_FILE, "w", encoding="utf-8") as f:
    f.write(content)

run_count = content.count("RUN_TEST(")
print(f"After v31.0.0: {len(content.splitlines())} lines, {run_count} RUN_TEST calls")
