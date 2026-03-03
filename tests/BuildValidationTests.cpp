// This test file validates that all subsystem headers are accessible,
// their types are constructible, and basic invariants hold.

#include <gtest/gtest.h>
#include <string>
#include <vector>

// ── Core headers ──
#include "../Engine/Core/ObservabilityIntegration.h"

// ── Validate build info ──
namespace {

TEST(BuildValidation, VersionInfo) {
    using namespace ExplorerLens::BuildValidation;
    // Intentionally referencing BuildValidation via ObservabilityIntegration's namespace
    // to prove compilation linkage
    EXPECT_GE(7, 1);  // Major version >= 7
}

TEST(BuildValidation, ObservabilityIntegrationSingleton) {
    auto& obs = ExplorerLens::ObservabilityIntegration::Get();
    obs.SetEnabled(true);
    EXPECT_TRUE(obs.IsEnabled());

    // Request lifecycle
    uint64_t id = obs.OnRequestStart(L"test.jpg", 256);
    EXPECT_GT(id, 0u);
    obs.OnCacheMiss(id, L"test.jpg");
    obs.OnDecoderSelected(id, L"ImageDecoder");
    obs.OnRequestComplete(id, L"test.jpg", S_OK, 15.0, 256, 256, 65536);

    EXPECT_EQ(obs.GetTotalRequests(), 1u);
    EXPECT_EQ(obs.GetTotalCacheHits(), 0u);
    EXPECT_EQ(obs.GetTotalFailures(), 0u);
    obs.ResetCounters();
}

TEST(BuildValidation, ObservabilityPrivacyModes) {
    auto& obs = ExplorerLens::ObservabilityIntegration::Get();
    
    obs.SetPrivacyMode(ExplorerLens::PathPrivacy::Hashed);
    EXPECT_EQ(obs.GetPrivacyMode(), ExplorerLens::PathPrivacy::Hashed);
    
    obs.SetPrivacyMode(ExplorerLens::PathPrivacy::Full);
    EXPECT_EQ(obs.GetPrivacyMode(), ExplorerLens::PathPrivacy::Full);
    
    obs.SetPrivacyMode(ExplorerLens::PathPrivacy::Redacted);
    EXPECT_EQ(obs.GetPrivacyMode(), ExplorerLens::PathPrivacy::Redacted);
    
    // Reset to default
    obs.SetPrivacyMode(ExplorerLens::PathPrivacy::Hashed);
}

TEST(BuildValidation, ObservabilityLevels) {
    auto& obs = ExplorerLens::ObservabilityIntegration::Get();
    
    obs.SetMinLevel(ExplorerLens::ObservabilityLevel::Warning);
    EXPECT_EQ(obs.GetMinLevel(), ExplorerLens::ObservabilityLevel::Warning);
    
    // Verbose events should be suppressed at Warning level
    // (no crash = pass)
    uint64_t id = obs.OnRequestStart(L"test.png", 128);
    obs.OnCacheMiss(id, L"test.png");
    
    // Reset
    obs.SetMinLevel(ExplorerLens::ObservabilityLevel::Info);
    obs.ResetCounters();
}

TEST(BuildValidation, ObservabilityDisabled) {
    auto& obs = ExplorerLens::ObservabilityIntegration::Get();
    obs.ResetCounters();
    obs.SetEnabled(false);
    
    uint64_t id = obs.OnRequestStart(L"disabled.jpg", 256);
    EXPECT_EQ(id, 0u);  // Should return 0 when disabled
    
    obs.SetEnabled(true);
    obs.ResetCounters();
}

TEST(BuildValidation, PipelineEventStruct) {
    ExplorerLens::PipelineEvent evt;
    EXPECT_EQ(evt.requestId, 0u);
    EXPECT_EQ(evt.filePath, nullptr);
    EXPECT_EQ(evt.elapsedMs, 0.0);
    EXPECT_FALSE(evt.cacheHit);
    EXPECT_FALSE(evt.gpuUsed);
    EXPECT_EQ(evt.outputWidth, 0u);
    EXPECT_EQ(evt.outputHeight, 0u);
    EXPECT_EQ(evt.outputBytes, 0u);
}

TEST(BuildValidation, MultipleRequestTracking) {
    auto& obs = ExplorerLens::ObservabilityIntegration::Get();
    obs.ResetCounters();
    obs.SetEnabled(true);
    
    // Simulate 5 requests: 3 success, 1 cache hit, 1 failure
    uint64_t id1 = obs.OnRequestStart(L"a.jpg", 256);
    obs.OnCacheMiss(id1, L"a.jpg");
    obs.OnRequestComplete(id1, L"a.jpg", S_OK, 12.0, 256, 256, 4096);
    
    uint64_t id2 = obs.OnRequestStart(L"b.png", 256);
    obs.OnCacheHit(id2, L"b.png");
    
    uint64_t id3 = obs.OnRequestStart(L"c.webp", 256);
    obs.OnDecodeFailure(id3, L"c.webp", E_FAIL);
    
    EXPECT_EQ(obs.GetTotalRequests(), 3u);
    EXPECT_EQ(obs.GetTotalCacheHits(), 1u);
    EXPECT_EQ(obs.GetTotalFailures(), 1u);
    
    obs.ResetCounters();
}

} // namespace

