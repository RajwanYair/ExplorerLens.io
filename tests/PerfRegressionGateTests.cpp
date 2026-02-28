// ============================================================================
// ============================================================================

#include <gtest/gtest.h>
#include "../Engine/Core/PerfRegressionGate.h"

using namespace ExplorerLens;

// ── KPI Names ──────────────────────────────────────────────────────────────

TEST(Sprint146_PerfRegressionGate, KpiNamesAreDefined) {
    EXPECT_STREQ(KpiName(PerfKPI::SingleThumbnailMs), "SingleThumbnailMs");
    EXPECT_STREQ(KpiName(PerfKPI::BatchThroughputImgSec), "BatchThroughputImgSec");
    EXPECT_STREQ(KpiName(PerfKPI::CacheHitMs), "CacheHitMs");
}

// ── Default Thresholds ─────────────────────────────────────────────────────

TEST(Sprint146_PerfRegressionGate, DefaultThresholdsConfigured) {
    PerfRegressionGate gate;
    auto& th = gate.Thresholds();
    EXPECT_NE(th.find(PerfKPI::SingleThumbnailMs), th.end());
    EXPECT_NE(th.find(PerfKPI::BatchThroughputImgSec), th.end());
    EXPECT_EQ(th.size(), 8);
}

TEST(Sprint146_PerfRegressionGate, SingleThumbnailThreshold) {
    PerfRegressionGate gate;
    auto& th = gate.Thresholds().at(PerfKPI::SingleThumbnailMs);
    EXPECT_EQ(th.warnThreshold, 15.0);
    EXPECT_EQ(th.failThreshold, 25.0);
    EXPECT_EQ(th.direction, ThresholdDirection::LowerIsBetter);
}

// ── Evaluation Tests ───────────────────────────────────────────────────────

TEST(Sprint146_PerfRegressionGate, AllPassWhenWithinLimits) {
    PerfRegressionGate gate;
    std::map<PerfKPI, double> current = {
        {PerfKPI::SingleThumbnailMs, 10.0},
        {PerfKPI::BatchThroughputImgSec, 250.0},
        {PerfKPI::CacheHitMs, 2.0}
    };
    auto result = gate.Evaluate(current);
    EXPECT_EQ(result.overall, GateVerdict::Pass);
    EXPECT_TRUE(result.Passed());
    EXPECT_EQ(result.failCount, 0);
}

TEST(Sprint146_PerfRegressionGate, FailWhenOverThreshold) {
    PerfRegressionGate gate;
    std::map<PerfKPI, double> current = {
        {PerfKPI::SingleThumbnailMs, 30.0}   // fail threshold = 25.0
    };
    auto result = gate.Evaluate(current);
    EXPECT_EQ(result.overall, GateVerdict::Fail);
    EXPECT_FALSE(result.Passed());
}

TEST(Sprint146_PerfRegressionGate, WarnWhenNearThreshold) {
    PerfRegressionGate gate;
    std::map<PerfKPI, double> current = {
        {PerfKPI::SingleThumbnailMs, 18.0}   // warn=15, fail=25
    };
    auto result = gate.Evaluate(current);
    EXPECT_EQ(result.overall, GateVerdict::Warn);
    EXPECT_TRUE(result.Passed());  // warn still passes
}

TEST(Sprint146_PerfRegressionGate, ThroughputFailBelowMinimum) {
    PerfRegressionGate gate;
    std::map<PerfKPI, double> current = {
        {PerfKPI::BatchThroughputImgSec, 100.0}   // fail=150 (HigherIsBetter)
    };
    auto result = gate.Evaluate(current);
    EXPECT_EQ(result.overall, GateVerdict::Fail);
}

// ── Baseline Regression Tests ──────────────────────────────────────────────

TEST(Sprint146_PerfRegressionGate, BaselineRegressionDetected) {
    PerfRegressionGate gate;
    gate.SetBaseline(PerfKPI::SingleThumbnailMs, 10.0);
    std::map<PerfKPI, double> current = {
        {PerfKPI::SingleThumbnailMs, 14.0}   // 40% regression
    };
    auto result = gate.Evaluate(current);
    ASSERT_EQ(result.results.size(), 1);
    EXPECT_GT(result.results[0].regressionPct, 30.0);
}

// ── Trend Analysis ─────────────────────────────────────────────────────────

TEST(Sprint146_PerfRegressionGate, TrendAnalysisComputation) {
    PerfRegressionGate gate;
    for (int i = 0; i < 10; ++i) {
        gate.RecordSample({PerfKPI::SingleThumbnailMs, 10.0 + i * 0.5, "test", "abc123"});
    }
    auto trend = gate.ComputeTrend(PerfKPI::SingleThumbnailMs);
    EXPECT_EQ(trend.sampleCount, 10);
    EXPECT_NEAR(trend.min, 10.0, 0.01);
    EXPECT_NEAR(trend.max, 14.5, 0.01);
    EXPECT_GT(trend.trendSlope, 0.0);  // degrading (values increasing)
    EXPECT_TRUE(trend.IsDegrading(ThresholdDirection::LowerIsBetter));
}

TEST(Sprint146_PerfRegressionGate, TrendEmptyHistory) {
    PerfRegressionGate gate;
    auto trend = gate.ComputeTrend(PerfKPI::ColdStartMs);
    EXPECT_EQ(trend.sampleCount, 0);
}

TEST(Sprint146_PerfRegressionGate, TrendImprovingDirection) {
    PerfRegressionGate gate;
    for (int i = 10; i > 0; --i) {
        gate.RecordSample({PerfKPI::SingleThumbnailMs, 10.0 + i * 0.5, "test", "abc123"});
    }
    auto trend = gate.ComputeTrend(PerfKPI::SingleThumbnailMs);
    EXPECT_LT(trend.trendSlope, 0.0);  // improving (values decreasing)
    EXPECT_FALSE(trend.IsDegrading(ThresholdDirection::LowerIsBetter));
}

// ── Report Formatting ──────────────────────────────────────────────────────

TEST(Sprint146_PerfRegressionGate, FormatReportContainsVerdict) {
    GateResult r;
    r.overall = GateVerdict::Pass;
    r.passCount = 5;
    auto report = PerfRegressionGate::FormatReport(r);
    EXPECT_NE(report.find("PASS"), std::string::npos);
    EXPECT_NE(report.find("Pass: 5"), std::string::npos);
}

TEST(Sprint146_PerfRegressionGate, CustomThresholdOverride) {
    PerfRegressionGate gate;
    KpiThreshold custom;
    custom.kpi = PerfKPI::ColdStartMs;
    custom.warnThreshold = 50.0;
    custom.failThreshold = 80.0;
    custom.direction = ThresholdDirection::LowerIsBetter;
    gate.SetThreshold(custom);

    std::map<PerfKPI, double> current = {{PerfKPI::ColdStartMs, 90.0}};
    auto result = gate.Evaluate(current);
    EXPECT_EQ(result.overall, GateVerdict::Fail);
}

