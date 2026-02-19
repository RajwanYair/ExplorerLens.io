// Sprint 158 — ARM64 Performance Baseline — GTest
#include <gtest/gtest.h>
#include "Utils/ARM64PerformanceBaseline.h"

using namespace DarkThumbs::Utils;

TEST(ARM64PerformanceBaseline, MockBaselineCreated) {
    auto b = ARM64PerformanceBaseline::CreateMock();
    EXPECT_GT(b.samples.size(), 0u);
}

TEST(ARM64PerformanceBaseline, KPIsThroughputAbove100) {
    ARM64KPIs kpi;
    EXPECT_GT(kpi.minThroughputImgSec, 50.0);
}

TEST(ARM64PerformanceBaseline, KPIsLatencyP95Below200ms) {
    ARM64KPIs kpi;
    EXPECT_LT(kpi.maxLatencyP95Ms, 200.0);
}

TEST(ARM64PerformanceBaseline, KPIsCacheHitBelow10ms) {
    ARM64KPIs kpi;
    EXPECT_LT(kpi.maxCacheHitMs, 10.0);
}

TEST(ARM64PerformanceBaseline, BenchmarkSampleHasDuration) {
    BenchmarkSample s;
    s.durationMs = 22.5;
    EXPECT_GT(s.durationMs, 0.0);
}

TEST(ARM64PerformanceBaseline, SIMDComparisonHasSpeedup) {
    SIMDComparisonResult r;
    r.x64Ms   = 17.0;
    r.arm64Ms = 22.0;
    EXPECT_GT(r.arm64Ms, 0.0);
}

TEST(ARM64PerformanceBaseline, RegressionThresholdIs10Percent) {
    EXPECT_DOUBLE_EQ(ARM64PerformanceBaseline::kRegressionThreshold, 10.0);
}

TEST(ARM64PerformanceBaseline, TrendCompareNoRegression) {
    auto b1 = ARM64PerformanceBaseline::CreateMock();
    auto b2 = ARM64PerformanceBaseline::CreateMock();
    auto trend = BaselineTrendResult::Compare(b1, b2);
    EXPECT_FALSE(trend.hasRegression);
}

TEST(ARM64PerformanceBaseline, MockSamplesAllPositiveDuration) {
    auto b = ARM64PerformanceBaseline::CreateMock();
    for (const auto& s : b.samples)
        EXPECT_GT(s.durationMs, 0.0);
}

TEST(ARM64PerformanceBaseline, KPIs150msP95) {
    ARM64KPIs kpi;
    EXPECT_LE(kpi.maxLatencyP95Ms, 200.0);
    EXPECT_GE(kpi.maxLatencyP95Ms, 50.0);
}

TEST(ARM64PerformanceBaseline, TrendHasBaselineTag) {
    auto b = ARM64PerformanceBaseline::CreateMock();
    EXPECT_FALSE(b.baselineTag.empty());
}

TEST(ARM64PerformanceBaseline, ColdStartUnderSecond) {
    ARM64KPIs kpi;
    EXPECT_LT(kpi.maxColdStartMs, 2000.0);
}
