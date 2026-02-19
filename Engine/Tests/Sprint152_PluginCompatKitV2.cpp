// Sprint 152 — Plugin Compatibility Kit V2 — GTest
#include <gtest/gtest.h>
#include "Plugin/PluginCompatibilityKitV2.h"

using namespace DarkThumbs::Plugin;

TEST(PluginCompatibilityKitV2, ABIVersionMajorOnly) {
    ABIVersion v{ 2, 1 };
    EXPECT_TRUE(v.IsBackwardCompatible(ABIVersion{ 1, 99 }));
    EXPECT_FALSE(v.IsBackwardCompatible(ABIVersion{ 3, 0 }));
}

TEST(PluginCompatibilityKitV2, V1BaselineHasMandatoryExports) {
    auto surface = ABIStableSurface::V1Baseline();
    EXPECT_GE(surface.exports.size(), 5u);
}

TEST(PluginCompatibilityKitV2, ConformanceCheckResultDefaultFail) {
    ConformanceCheckResult r;
    EXPECT_FALSE(r.passed);
}

TEST(PluginCompatibilityKitV2, PerfGateDecodeThreshold) {
    PluginPerfGate g;
    EXPECT_GT(g.maxDecodeMs, 0.0);
    EXPECT_LE(g.maxDecodeMs, 200.0);
}

TEST(PluginCompatibilityKitV2, MemoryGatePeakThreshold) {
    PluginMemoryGate g;
    EXPECT_GT(g.maxPeakMB, 0u);
    EXPECT_LE(g.maxPeakMB, 200u);
}

TEST(PluginCompatibilityKitV2, ConformanceReportNoFailuresDefault) {
    PluginConformanceReport r;
    EXPECT_EQ(r.failureCount, 0u);
}

TEST(PluginCompatibilityKitV2, ABIDriftDetectorNoDriftOnSameSurface) {
    ABIDriftDetector det;
    auto s1 = ABIStableSurface::V1Baseline();
    auto s2 = ABIStableSurface::V1Baseline();
    EXPECT_FALSE(det.HasDrift(s1, s2));
}

TEST(PluginCompatibilityKitV2, ABIDriftDetectorDriftOnDifferentSurface) {
    ABIDriftDetector det;
    auto s1 = ABIStableSurface::V1Baseline();
    ABIStableSurface s2;
    s2.exports.push_back({ "extra_export", false });
    EXPECT_TRUE(det.HasDrift(s1, s2));
}

TEST(PluginCompatibilityKitV2, ExportedSymbolRequired) {
    ExportedSymbol sym{ "DT_CreateDecoder", true };
    EXPECT_TRUE(sym.required);
}

TEST(PluginCompatibilityKitV2, ConformanceCheckIdEnumCoverage) {
    auto id = ConformanceCheckId::PerfGate;
    EXPECT_EQ(static_cast<uint32_t>(id), 2u);
}

TEST(PluginCompatibilityKitV2, PerfGateDefaultThreshold100ms) {
    PluginPerfGate g;
    EXPECT_DOUBLE_EQ(g.maxDecodeMs, 100.0);
}

TEST(PluginCompatibilityKitV2, MemGateDefault50MB) {
    PluginMemoryGate g;
    EXPECT_EQ(g.maxPeakMB, 50u);
}

TEST(PluginCompatibilityKitV2, ConformanceReportPassRateWhenAllPass) {
    PluginConformanceReport r;
    r.totalChecks = 5; r.failureCount = 0;
    EXPECT_DOUBLE_EQ(r.PassRate(), 100.0);
}

TEST(PluginCompatibilityKitV2, ConformanceReportPassRatePartial) {
    PluginConformanceReport r;
    r.totalChecks = 10; r.failureCount = 2;
    EXPECT_DOUBLE_EQ(r.PassRate(), 80.0);
}
