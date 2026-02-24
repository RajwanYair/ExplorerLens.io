// ARM64 CI Integration — Tests
#include "../Utils/ARM64CIIntegration.h"
#include "GTestShim.h"

using namespace ExplorerLens::Platform;

TEST(ARM64CIIntegration, DefaultIntegrationHasMatrixEntries) {
  auto ci = ARM64CIIntegration::CreateDefault();
  EXPECT_GE(ci.workflow.matrix.size(), 1u);
}

TEST(ARM64CIIntegration, CIStageEnumValues) {
  EXPECT_EQ(static_cast<uint32_t>(CIStage::Checkout), 0u);
  EXPECT_EQ(static_cast<uint32_t>(CIStage::Build), 2u);
  EXPECT_EQ(static_cast<uint32_t>(CIStage::UnitTest), 3u);
}

TEST(ARM64CIIntegration, WorkflowJobDefaultNotEmpty) {
  auto job = WorkflowJobDescriptor::Default();
  EXPECT_FALSE(job.jobName.empty());
}

TEST(ARM64CIIntegration, DocsManifestRequiredNotEmpty) {
  auto manifest = ARM64DocsManifest::Required();
  EXPECT_GE(manifest.entries.size(), 1u);
}

TEST(ARM64CIIntegration, MatrixEntryHasArchitecture) {
  auto entry = CIMatrixEntry::ARM64BuildOnly();
  EXPECT_FALSE(entry.architecture.empty());
}

TEST(ARM64CIIntegration, CIIntegrationHasBuildStage) {
  auto ci = ARM64CIIntegration::CreateDefault();
  bool hasBuild = false;
  for (const auto &s : ci.workflow.stages)
    if (s == CIStage::Build) {
      hasBuild = true;
      break;
    }
  EXPECT_TRUE(hasBuild);
}

TEST(ARM64CIIntegration, WorkflowJobDefaultHasMatrix) {
  auto job = WorkflowJobDescriptor::Default();
  EXPECT_GE(job.matrix.size(), 1u);
}

TEST(ARM64CIIntegration, CIIntegrationDefaultHasUnitTestStage) {
  auto ci = ARM64CIIntegration::CreateDefault();
  bool hasTest = false;
  for (const auto &s : ci.workflow.stages)
    if (s == CIStage::UnitTest) {
      hasTest = true;
      break;
    }
  EXPECT_TRUE(hasTest);
}

TEST(ARM64CIIntegration, DocsManifestIncludesARM64SupportDoc) {
  auto manifest = ARM64DocsManifest::Required();
  bool found = false;
  for (const auto &d : manifest.entries)
    if (d.filePath.find("ARM64") != std::string::npos) {
      found = true;
      break;
    }
  EXPECT_TRUE(found);
}

TEST(ARM64CIIntegration, CIMatrixEntryARM64FullRunsTests) {
  auto entry = CIMatrixEntry::ARM64Full();
  EXPECT_TRUE(entry.runTests);
}
