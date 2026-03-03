#include <gtest/gtest.h>
#include "Release/MSILifecycleRunner.h"
using namespace ExplorerLens::Release;

TEST(MSI, OperationName) {
    EXPECT_STREQ(OperationName(InstallerOperation::Install), "Install");
    EXPECT_STREQ(OperationName(InstallerOperation::Uninstall), "Uninstall");
}
TEST(MSI, PackageTypeName) {
    EXPECT_STREQ(PackageTypeName(PackageType::WiX), "WiX MSI");
}
TEST(MSI, VerificationChecks_AllPassed) {
    VerificationChecks v;
    v.filesDeployed = true; v.registryKeysCreated = true;
    v.comRegistered = true; v.uninstallEntry = true;
    EXPECT_TRUE(v.AllPassed());
}
TEST(MSI, VerificationChecks_NotAllPassed) {
    VerificationChecks v;
    v.filesDeployed = true;
    EXPECT_FALSE(v.AllPassed());
}
TEST(MSI, VerificationChecks_PassedCount) {
    VerificationChecks v;
    v.filesDeployed = true; v.comRegistered = true;
    EXPECT_EQ(v.PassedCount(), 2u);
}
TEST(MSI, UninstallVerification_IsClean) {
    UninstallVerification u;
    u.filesRemoved = true; u.registryKeysCleaned = true;
    u.comUnregistered = true; u.uninstallEntryRemoved = true;
    u.noOrphanedFiles = true; u.noOrphanedRegistry = true;
    EXPECT_TRUE(u.IsClean());
}
TEST(MSI, Scenario_Description) {
    MSITestScenario s;
    s.name = "Fresh"; s.operation = InstallerOperation::Install;
    s.packageType = PackageType::WiX; s.targetVersion = "7.1.0";
    EXPECT_NE(s.Description().find("Install"), std::string::npos);
}
TEST(MSI, Runner_DefaultScenarios) {
    auto runner = MSILifecycleRunner::Create();
    EXPECT_EQ(runner.ScenarioCount(), 6u);
}
TEST(MSI, Runner_RunInstall) {
    auto runner = MSILifecycleRunner::Create();
    auto exec = runner.RunScenario(0);  // Fresh Install
    EXPECT_TRUE(exec.IsPass());
    EXPECT_EQ(exec.msiExitCode, 0);
}
TEST(MSI, Runner_RunUninstall) {
    auto runner = MSILifecycleRunner::Create();
    auto exec = runner.RunScenario(4);  // Uninstall
    EXPECT_TRUE(exec.IsPass());
    EXPECT_TRUE(exec.uninstallChecks.IsClean());
}
TEST(MSI, Stats_ReleaseReady) {
    MSILifecycleStats stats;
    stats.passed = 6;
    EXPECT_TRUE(stats.IsReleaseReady());
    stats.failed = 1;
    EXPECT_FALSE(stats.IsReleaseReady());
}
TEST(MSI, MSIResultName) {
    EXPECT_STREQ(MSIResultName(MSITestResult::Pass), "PASS");
    EXPECT_STREQ(MSIResultName(MSITestResult::SkippedNoAdmin), "SKIPPED (No Admin)");
}

