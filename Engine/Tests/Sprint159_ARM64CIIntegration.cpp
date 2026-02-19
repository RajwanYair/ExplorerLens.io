// Sprint 159 — ARM64 CI Integration — GTest
#include <gtest/gtest.h>
#include "Utils/ARM64CIIntegration.h"

using namespace DarkThumbs::Utils;

TEST(ARM64CIIntegration, DefaultIntegrationHasEntries) {
    auto ci = ARM64CIIntegration::CreateDefault();
    EXPECT_GE(ci.matrixEntries.size(), 1u);
}

TEST(ARM64CIIntegration, CIStageEnumCoverage) {
    EXPECT_EQ(static_cast<uint32_t>(CIStage::Build), 0u);
    EXPECT_EQ(static_cast<uint32_t>(CIStage::Test),  1u);
}

TEST(ARM64CIIntegration, WorkflowJobDefaultNotEmpty) {
    auto job = WorkflowJobDescriptor::Default();
    EXPECT_FALSE(job.jobName.empty());
}

TEST(ARM64CIIntegration, DocsManifestRequiredNotEmpty) {
    auto manifest = ARM64DocsManifest::Required();
    EXPECT_GE(manifest.requiredDocs.size(), 1u);
}

TEST(ARM64CIIntegration, MatrixEntryHasLabel) {
    CIMatrixEntry e;
    e.label = "ARM64-BuildOnly";
    EXPECT_FALSE(e.label.empty());
}

TEST(ARM64CIIntegration, CIIntegrationHasBuildStage) {
    auto ci = ARM64CIIntegration::CreateDefault();
    bool hasBuild = false;
    for (const auto& e : ci.matrixEntries)
        if (e.stage == CIStage::Build) { hasBuild = true; break; }
    EXPECT_TRUE(hasBuild);
}

TEST(ARM64CIIntegration, WorkflowJobDefaultHasRunsOn) {
    auto job = WorkflowJobDescriptor::Default();
    EXPECT_FALSE(job.runsOn.empty());
}

TEST(ARM64CIIntegration, CIIntegrationDefaultHasTestStage) {
    auto ci = ARM64CIIntegration::CreateDefault();
    bool hasTest = false;
    for (const auto& e : ci.matrixEntries)
        if (e.stage == CIStage::Test) { hasTest = true; break; }
    EXPECT_TRUE(hasTest);
}

TEST(ARM64CIIntegration, DocsManifestIncludesARM64SupportDoc) {
    auto manifest = ARM64DocsManifest::Required();
    bool found = false;
    for (const auto& d : manifest.requiredDocs)
        if (d.find("ARM64") != std::string::npos) { found = true; break; }
    EXPECT_TRUE(found);
}

TEST(ARM64CIIntegration, CIMatrixEntryEnumCoverage) {
    CIMatrixEntry e;
    e.stage = CIStage::Package;
    EXPECT_EQ(static_cast<uint32_t>(e.stage), 2u);
}
