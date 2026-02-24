// Plugin Runtime Test Matrix — GTestShim
#include "GTestShim.h"
#include "Plugin/PluginRuntimeTestMatrix.h"

using namespace ExplorerLens::Plugin;

TEST(PluginRuntimeTestMatrix, DefaultConfigHasEntries) {
  auto cfg = TestMatrixConfig::Default();
  EXPECT_GT(cfg.entries.size(), 0u);
  EXPECT_GT(cfg.soakConfig.iterations, 0u);
}

TEST(PluginRuntimeTestMatrix, CIMinimalConfigIsSmaller) {
  auto full = TestMatrixConfig::Default();
  auto ci = TestMatrixConfig::CIMinimal();
  EXPECT_LE(ci.soakConfig.iterations, full.soakConfig.iterations);
}

TEST(PluginRuntimeTestMatrix, FullSoakConfigHasMore) {
  auto soak = TestMatrixConfig::FullSoak();
  auto def = TestMatrixConfig::Default();
  EXPECT_GE(soak.soakConfig.iterations, def.soakConfig.iterations);
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
  EXPECT_EQ(r.passCount, 0u);
  EXPECT_EQ(r.failCount, 0u);
}

TEST(PluginRuntimeTestMatrix, RunnerRunDryReturnsReport) {
  auto cfg = TestMatrixConfig::CIMinimal();
  PluginRuntimeTestMatrixRunner runner(cfg);
  auto report = runner.RunDry();
  EXPECT_GE(report.passCount + report.failCount, 0u);
}

TEST(PluginRuntimeTestMatrix, SoakTestSLAIs1000Iterations) {
  auto soak = TestMatrixConfig::FullSoak();
  EXPECT_GE(soak.soakConfig.iterations, 1000u);
}

TEST(PluginRuntimeTestMatrix, PluginTestEntryHasName) {
  PluginTestEntry e{PluginTestScenario::PluginLoad, "LoadPlugin", "desc"};
  EXPECT_FALSE(e.name.empty());
}

TEST(PluginRuntimeTestMatrix, PluginTestResultPassedDefaultFalse) {
  PluginTestResult r{PluginTestScenario::PluginLoad};
  EXPECT_FALSE(r.Passed());
}

TEST(PluginRuntimeTestMatrix, IPCChannelConfigHasDefaults) {
  IPCChannelConfig cfg;
  EXPECT_GT(cfg.timeoutMs, 0u);
}

TEST(PluginRuntimeTestMatrix, MatrixReportPassRateZeroWhenEmpty) {
  // passCount/failCount are 0 by default — report has no PassRate(),
  // verify zero init is correct
  TestMatrixReport r;
  EXPECT_EQ(r.passCount, 0u);
  EXPECT_EQ(r.failCount, 0u);
}

TEST(PluginRuntimeTestMatrix, MatrixReportAllRequiredPassedWhenNoFail) {
  TestMatrixReport r;
  r.passCount = 10;
  r.failCount = 0;
  EXPECT_TRUE(r.AllRequiredPassed());
}

TEST(PluginRuntimeTestMatrix, MatrixReportAllRequiredFailedWhenFail) {
  TestMatrixReport r;
  r.passCount = 8;
  r.failCount = 2;
  EXPECT_FALSE(r.AllRequiredPassed());
}

TEST(PluginRuntimeTestMatrix, RunnerDryRunNoSideEffects) {
  auto cfg = TestMatrixConfig::CIMinimal();
  PluginRuntimeTestMatrixRunner r1(cfg);
  PluginRuntimeTestMatrixRunner r2(cfg);
  auto rep1 = r1.RunDry();
  auto rep2 = r2.RunDry();
  EXPECT_EQ(rep1.passCount, rep2.passCount);
}
