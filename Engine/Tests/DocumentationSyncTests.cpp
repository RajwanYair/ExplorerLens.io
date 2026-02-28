// Documentation Sync Audit — GTest
#include "../Core/DocumentationSyncAudit.h"
#include "GTestShim.h"

using namespace ExplorerLens::Core;

TEST(DocumentationSyncAudit, MockAllPassAuditPassed) {
 auto r = DocSyncAuditResult::CreateMock(true);
 EXPECT_TRUE(r.auditPassed);
}

TEST(DocumentationSyncAudit, MockFailAuditFailed) {
 auto r = DocSyncAuditResult::CreateMock(false);
 EXPECT_FALSE(r.auditPassed);
}

TEST(DocumentationSyncAudit, MockAllPassNoFailures) {
 auto r = DocSyncAuditResult::CreateMock(true);
 EXPECT_EQ(r.failedCount, 0u);
}

TEST(DocumentationSyncAudit, MockAllPassHasChecks) {
 auto r = DocSyncAuditResult::CreateMock(true);
 EXPECT_GE(r.checks.size(), 5u);
}

TEST(DocumentationSyncAudit, VersionRefIsV83) {
 auto r = DocSyncAuditResult::CreateMock(true);
 EXPECT_EQ(r.versionRef, std::string("v8.3.0"));
}

TEST(DocumentationSyncAudit, DocArtifactToStringNotEmpty) {
 EXPECT_FALSE(ToString(DocArtifact::MasterPlan).empty());
 EXPECT_FALSE(ToString(DocArtifact::Changelog).empty());
}

TEST(DocumentationSyncAudit, SyncCheckCopilotInstructionsMatch) {
 auto r = DocSyncAuditResult::CreateMock(true);
 for (const auto &c : r.checks) {
 if (c.id == SyncCheckId::CopilotInstructionsSync) {
 EXPECT_TRUE(c.passed);
 return;
 }
 }
 FAIL(); // CopilotInstructionsSync check not found
}

TEST(DocumentationSyncAudit, FailReasonEmptyWhenPass) {
 DocSyncCheck c;
 c.passed = true;
 EXPECT_TRUE(c.FailReason().empty());
}

TEST(DocumentationSyncAudit, FailReasonNotEmptyWhenFail) {
 DocSyncCheck c;
 c.passed = false;
 c.expected = "v8.3.0";
 c.actual = "v7.1.0";
 EXPECT_FALSE(c.FailReason().empty());
}

TEST(DocumentationSyncAudit, RunAuditReturnsReport) {
 DocumentationSyncAudit aud;
 auto r = aud.RunAudit("v8.3.0");
 EXPECT_EQ(r.versionRef, std::string("v8.3.0"));
}

TEST(DocumentationSyncAudit, MinPassForGate5) {
 EXPECT_GE(DocSyncAuditResult::kMinPassForGate, 5u);
}

TEST(DocumentationSyncAudit, ARM64DocCheckPresent) {
 auto r = DocSyncAuditResult::CreateMock(true);
 bool found = false;
 for (const auto &c : r.checks)
 if (c.id == SyncCheckId::ARM64DocExists)
 found = true;
 EXPECT_TRUE(found);
}
