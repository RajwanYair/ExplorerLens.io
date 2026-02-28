#include <gtest/gtest.h>
#include "Plugin/PluginRuntimeValidation.h"
using namespace ExplorerLens::Plugin;

TEST(Sprint128_PluginE2E, LifecycleTransition_Valid) {
    auto v = PluginRuntimeValidator::Create();
    EXPECT_TRUE(v.IsValidTransition(PluginState::Unloaded, PluginState::Discovering));
    EXPECT_TRUE(v.IsValidTransition(PluginState::Ready, PluginState::Decoding));
    EXPECT_TRUE(v.IsValidTransition(PluginState::Faulted, PluginState::Unloading));
}
TEST(Sprint128_PluginE2E, LifecycleTransition_Invalid) {
    auto v = PluginRuntimeValidator::Create();
    EXPECT_FALSE(v.IsValidTransition(PluginState::Unloaded, PluginState::Decoding));
    EXPECT_FALSE(v.IsValidTransition(PluginState::Ready, PluginState::Discovering));
}
TEST(Sprint128_PluginE2E, RecordTransition_ValidEvent) {
    auto v = PluginRuntimeValidator::Create();
    auto e = v.RecordTransition("sample-plugin", PluginState::Unloaded, PluginState::Discovering);
    EXPECT_TRUE(e.isValid);
    EXPECT_EQ(v.EventCount(), 1u);
}
TEST(Sprint128_PluginE2E, RecordTransition_InvalidEvent) {
    auto v = PluginRuntimeValidator::Create();
    auto e = v.RecordTransition("bad-plugin", PluginState::Unloaded, PluginState::Decoding);
    EXPECT_FALSE(e.isValid);
    EXPECT_EQ(v.InvalidTransitionCount(), 1);
}
TEST(Sprint128_PluginE2E, TestMatrix_Generated) {
    auto v = PluginRuntimeValidator::Create();
    auto matrix = v.GenerateTestMatrix();
    EXPECT_GE(matrix.size(), 5u);
}
TEST(Sprint128_PluginE2E, Scenario_NormalDecode) {
    auto s = PluginTestScenario::NormalDecode(".psd");
    EXPECT_TRUE(s.expectSuccess);
    EXPECT_FALSE(s.injectFault);
    EXPECT_EQ(s.iterations, 100);
}
TEST(Sprint128_PluginE2E, Scenario_CrashInjection) {
    auto s = PluginTestScenario::CrashInjection();
    EXPECT_TRUE(s.injectFault);
    EXPECT_FALSE(s.expectSuccess);
}
TEST(Sprint128_PluginE2E, Scenario_TimeoutInjection) {
    auto s = PluginTestScenario::TimeoutInjection();
    EXPECT_TRUE(s.injectTimeout);
    EXPECT_FALSE(s.expectSuccess);
}
TEST(Sprint128_PluginE2E, SoakConfig_Quick) {
    auto c = SoakTestConfig::Quick();
    EXPECT_EQ(c.totalIterations, 500);
}
TEST(Sprint128_PluginE2E, SoakConfig_Exhaustive) {
    auto c = SoakTestConfig::Exhaustive();
    EXPECT_EQ(c.totalIterations, 50000);
    EXPECT_EQ(c.maxConcurrent, 8);
}
TEST(Sprint128_PluginE2E, SoakResult_SuccessRate) {
    SoakTestResult r;
    r.totalRequests = 1000;
    r.successCount = 950;
    EXPECT_DOUBLE_EQ(r.SuccessRate(), 95.0);
}
TEST(Sprint128_PluginE2E, SoakResult_CrashBudget) {
    SoakTestResult r;
    r.crashCount = 0;
    EXPECT_TRUE(r.PassedCrashBudget(0));
    r.crashCount = 1;
    EXPECT_FALSE(r.PassedCrashBudget(0));
}
TEST(Sprint128_PluginE2E, SoakTest_Run) {
    auto v = PluginRuntimeValidator::Create();
    auto result = v.RunSoakTest(SoakTestConfig::Quick());
    EXPECT_EQ(result.totalRequests, 500);
    EXPECT_EQ(result.crashCount, 0);
    EXPECT_GT(result.SuccessRate(), 99.0);
}
TEST(Sprint128_PluginE2E, IPCMessage_Latency) {
    IPCMessage msg;
    msg.sentAt = std::chrono::steady_clock::now();
    msg.receivedAt = msg.sentAt + std::chrono::milliseconds(5);
    EXPECT_GE(msg.LatencyMs(), 4.0);
    EXPECT_LE(msg.LatencyMs(), 6.0);
}
TEST(Sprint128_PluginE2E, PluginCapability_Defaults) {
    PluginCapability cap;
    cap.pluginId = "test-plugin";
    cap.supportedExtensions = {".psd", ".ai"};
    EXPECT_EQ(cap.maxMemoryBytes, 64u * 1024 * 1024);
    EXPECT_FALSE(cap.supportsGPU);
    EXPECT_TRUE(cap.requiresSandbox);
}

