// Sprint 171 — Installer Lifecycle Automation — GTest
#include <gtest/gtest.h>
#include "Utils/InstallerLifecycleAutomation.h"

using namespace DarkThumbs::Utils;

TEST(InstallerLifecycleAutomation, CurrentVersionIs8_3_0) {
    auto v = InstallerLifecycleAutomation::CurrentVersion();
    EXPECT_EQ(v.major, 8u);
    EXPECT_EQ(v.minor, 3u);
    EXPECT_EQ(v.patch, 0u);
}

TEST(InstallerLifecycleAutomation, VersionToStringFormat) {
    InstalledVersion v{ 8, 3, 0 };
    EXPECT_EQ(v.ToString(), std::string("8.3.0"));
}

TEST(InstallerLifecycleAutomation, NewerVersionDetected) {
    InstalledVersion v1{ 8, 3, 0 };
    InstalledVersion v2{ 7, 1, 0 };
    EXPECT_TRUE(v1.IsNewerThan(v2));
}

TEST(InstallerLifecycleAutomation, OlderVersionNotNewer) {
    InstalledVersion v1{ 7, 1, 0 };
    InstalledVersion v2{ 8, 3, 0 };
    EXPECT_FALSE(v1.IsNewerThan(v2));
}

TEST(InstallerLifecycleAutomation, FreshInstallSuccess) {
    auto result = InstallerLifecycleAutomation::SimulateFreshInstall();
    EXPECT_TRUE(result.overall);
}

TEST(InstallerLifecycleAutomation, FreshInstallHasSteps) {
    auto result = InstallerLifecycleAutomation::SimulateFreshInstall();
    EXPECT_GE(result.steps.size(), 4u);
}

TEST(InstallerLifecycleAutomation, FreshInstallAllStepsPass) {
    auto result = InstallerLifecycleAutomation::SimulateFreshInstall();
    EXPECT_EQ(result.FailedStepCount(), 0u);
}

TEST(InstallerLifecycleAutomation, UninstallSuccess) {
    auto result = InstallerLifecycleAutomation::SimulateUninstall();
    EXPECT_TRUE(result.overall);
}

TEST(InstallerLifecycleAutomation, COMRegistrationCLSIDContainsGUID) {
    auto rec = COMRegistrationRecord::Expected();
    EXPECT_NE(rec.clsid.find("9E6ECB90"), std::string::npos);
}

TEST(InstallerLifecycleAutomation, COMRegistrationInprocServer) {
    auto rec = COMRegistrationRecord::Expected();
    EXPECT_TRUE(rec.isInprocServer);
}

TEST(InstallerLifecycleAutomation, COMRegistrationApproved) {
    auto rec = COMRegistrationRecord::Expected();
    EXPECT_TRUE(rec.approvedByShell);
}

TEST(InstallerLifecycleAutomation, FreshInstallCOMRecordSet) {
    auto result = InstallerLifecycleAutomation::SimulateFreshInstall();
    EXPECT_NE(result.comRecord.clsid.find("9E6ECB90"), std::string::npos);
}

TEST(InstallerLifecycleAutomation, ActionEnumCoverage) {
    EXPECT_EQ(static_cast<uint32_t>(InstallAction::FreshInstall), 0u);
    EXPECT_EQ(static_cast<uint32_t>(InstallAction::Uninstall),    3u);
}

TEST(InstallerLifecycleAutomation, FreshInstallVersionTag) {
    auto result = InstallerLifecycleAutomation::SimulateFreshInstall();
    EXPECT_FALSE(result.installedVersion.buildTag.empty());
}

TEST(InstallerLifecycleAutomation, UninstallHas4PlusSteps) {
    auto result = InstallerLifecycleAutomation::SimulateUninstall();
    EXPECT_GE(result.steps.size(), 4u);
}
