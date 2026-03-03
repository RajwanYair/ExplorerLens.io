#include <gtest/gtest.h>
#include "Utils/WindowsCompat.h"
using namespace ExplorerLens::Utils;

TEST(Compat, WindowsBuildName) {
    EXPECT_STREQ(WindowsBuildName(WindowsBuild::Win11_24H2), "Windows 11 24H2 (26100)");
}
TEST(Compat, IsWindows11) {
    EXPECT_TRUE(IsWindows11(WindowsBuild::Win11_22H2));
    EXPECT_FALSE(IsWindows11(WindowsBuild::Win10_22H2));
}
TEST(Compat, GPUVendorName) {
    EXPECT_STREQ(GPUVendorName(GPUVendor::Intel), "Intel");
}
TEST(Compat, DPIFromScale) {
    EXPECT_EQ(DPIFromScale(DPIScale::Scale_100), 96u);
    EXPECT_EQ(DPIFromScale(DPIScale::Scale_200), 192u);
}
TEST(Compat, Scenario_Description) {
    CompatTestScenario s;
    s.name = "Test1";
    s.osBuild = WindowsBuild::Win11_23H2;
    s.gpu = GPUVendor::NVIDIA;
    s.dpi = DPIScale::Scale_150;
    std::string desc = s.Description();
    EXPECT_NE(desc.find("NVIDIA"), std::string::npos);
    EXPECT_NE(desc.find("23H2"), std::string::npos);
}
TEST(Compat, CompatResultName) {
    EXPECT_STREQ(CompatResultName(CompatResult::Pass), "PASS");
    EXPECT_STREQ(CompatResultName(CompatResult::Fail), "FAIL");
}
TEST(Compat, Execution_IsFullPass) {
    CompatTestExecution exec;
    exec.result = CompatResult::Pass;
    exec.comRegistrationOk = true;
    exec.thumbnailRenderOk = true;
    exec.dpiScalingOk = true;
    exec.gpuAccelOk = true;
    exec.shellIntegrationOk = true;
    EXPECT_TRUE(exec.IsFullPass());
}
TEST(Compat, Execution_NotFullPass) {
    CompatTestExecution exec;
    exec.result = CompatResult::Pass;
    exec.comRegistrationOk = true;
    exec.thumbnailRenderOk = false;
    EXPECT_FALSE(exec.IsFullPass());
}
TEST(Compat, Matrix_DefaultSize) {
    auto matrix = CompatibilityMatrixExec::Create();
    // 4 builds × 3 GPUs × 3 DPI = 36 scenarios
    EXPECT_EQ(matrix.ScenarioCount(), 36u);
}
TEST(Compat, Matrix_StatsDefault) {
    auto matrix = CompatibilityMatrixExec::Create();
    auto stats = matrix.GetStats();
    EXPECT_EQ(stats.totalScenarios, 36u);
    EXPECT_EQ(stats.notTested, 36u);
}
TEST(Compat, Matrix_RecordResult) {
    auto matrix = CompatibilityMatrixExec::Create();
    matrix.RecordResult(0, CompatResult::Pass);
    auto stats = matrix.GetStats();
    EXPECT_EQ(stats.passed, 1u);
}
TEST(Compat, Stats_PassRate) {
    CompatMatrixStats stats;
    stats.passed = 9; stats.failed = 1;
    EXPECT_DOUBLE_EQ(stats.PassRate(), 0.9);
    EXPECT_FALSE(stats.MeetsMinimum(0.95));
    EXPECT_TRUE(stats.MeetsMinimum(0.85));
}
