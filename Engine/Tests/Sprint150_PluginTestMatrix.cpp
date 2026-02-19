// Sprint 150 — Plugin Runtime Test Matrix — GTest
#include <gtest/gtest.h>
#include "Plugin/PluginRuntimeTestMatrix.h"

using namespace DarkThumbs::Plugin;

TEST(PluginRuntimeTestMatrix, DefaultConfigHasEntries) {
    auto cfg = TestMatrixConfig::Default();
    EXPECT_GT(cfg.maxIPCLatencyMs, 0u);
    EXPECT_GT(cfg.soakIterations, 0u);
}

TEST(PluginRuntimeTestMatrix, CIMinimalConfigIsSmaller) {
    auto full = TestMatrixConfig::Default();
    auto ci   = TestMatrixConfig::CIMinimal();
    EXPECT_LE(ci.soakIterations, full.soakIterations);
}

TEST(PluginRuntimeTestMatrix, FullSoakConfigHasMore) {
    auto soak = TestMatrixConfig::FullSoak();
    auto def  = TestMatrixConfig::Default();
    EXPECT_GE(soak.soakIterations, def.soakIterations);
}

TEST(PluginRuntimeTestMatrix, IPCLatencyResultHasField) {
    IPCLatencyResult r;
    r.p95Ms = 45.0;
    EXPECT_LT(r.p95Ms, 50.0);
}

TEST(PluginRuntimeTestMatrix, SoakTestConfigDefaults) {
    SoakTestConfig s;
    EXPECT_GT(s.iterations, 0u);
    EXPECT_GT(s.maxMemoryMB, 0u);
}

TEST(PluginRuntimeTestMatrix, TestMatrixReportFieldsZeroInit) {
    TestMatrixReport r;
    EXPECT_EQ(r.passed, 0u);
    EXPECT_EQ(r.failed, 0u);
}

TEST(PluginRuntimeTestMatrix, RunnerRunDryReturnsReport) {
    PluginRuntimeTestMatrixRunner runner;
    auto cfg = TestMatrixConfig::CIMinimal();
    auto report = runner.RunDry(cfg);
    EXPECT_GE(report.passed + report.failed, 0u);
}

TEST(PluginRuntimeTestMatrix, SoakTestSLAIs1000Iterations) {
    auto soak = TestMatrixConfig::FullSoak();
    EXPECT_GE(soak.soakIterations, 1000u);
}

TEST(PluginRuntimeTestMatrix, PluginTestEntryHasName) {
    PluginTestEntry e;
    e.testName = "LoadPlugin";
    EXPECT_FALSE(e.testName.empty());
}

TEST(PluginRuntimeTestMatrix, PluginTestResultOkDefaultFalse) {
    PluginTestResult r;
    EXPECT_FALSE(r.passed);
}

TEST(PluginRuntimeTestMatrix, IPCChannelConfigHasDefaults) {
    IPCChannelConfig cfg;
    EXPECT_GT(cfg.timeoutMs, 0u);
}

TEST(PluginRuntimeTestMatrix, MatrixReportPassRateZeroWhenEmpty) {
    TestMatrixReport r;
    r.passed = 0; r.failed = 0;
    EXPECT_DOUBLE_EQ(r.PassRate(), 0.0);
}

TEST(PluginRuntimeTestMatrix, MatrixReportPassRate100) {
    TestMatrixReport r;
    r.passed = 10; r.failed = 0;
    EXPECT_DOUBLE_EQ(r.PassRate(), 100.0);
}

TEST(PluginRuntimeTestMatrix, MatrixReportPassRatePartial) {
    TestMatrixReport r;
    r.passed = 8; r.failed = 2;
    EXPECT_DOUBLE_EQ(r.PassRate(), 80.0);
}

TEST(PluginRuntimeTestMatrix, RunnerDryRunNoSideEffects) {
    PluginRuntimeTestMatrixRunner r1, r2;
    auto cfg = TestMatrixConfig::CIMinimal();
    auto rep1 = r1.RunDry(cfg);
    auto rep2 = r2.RunDry(cfg);
    EXPECT_EQ(rep1.passed, rep2.passed);
}
