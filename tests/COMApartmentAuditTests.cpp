#include <gtest/gtest.h>
#include "Shell/COMApartmentAudit.h"
using namespace ExplorerLens::COM;

TEST(COMApartment, AuditEntry_Compliant) {
    InterfaceAuditEntry e;
    e.declaredModel = ApartmentType::STA;
    e.actualModel = ApartmentType::STA;
    e.usesGlobalState = false;
    e.hasReentrancyGuard = true;
    EXPECT_TRUE(e.IsCompliant());
}
TEST(COMApartment, AuditEntry_MismatchNotCompliant) {
    InterfaceAuditEntry e;
    e.declaredModel = ApartmentType::STA;
    e.actualModel = ApartmentType::MTA;
    e.hasReentrancyGuard = true;
    EXPECT_FALSE(e.IsCompliant());
}
TEST(COMApartment, AuditEntry_GlobalStateNotCompliant) {
    InterfaceAuditEntry e;
    e.declaredModel = ApartmentType::STA;
    e.actualModel = ApartmentType::STA;
    e.usesGlobalState = true;
    e.hasReentrancyGuard = true;
    EXPECT_FALSE(e.IsCompliant());
}
TEST(COMApartment, Auditor_CreateWithKnownInterfaces) {
    auto auditor = COMApartmentAuditor::Create();
    EXPECT_GE(auditor.InterfaceCount(), 3u);
}
TEST(COMApartment, Auditor_RunAudit_AllCompliant) {
    auto auditor = COMApartmentAuditor::Create();
    auto result = auditor.RunAudit();
    EXPECT_EQ(result.totalInterfaces, 3);
    EXPECT_TRUE(result.IsFullyCompliant());
    EXPECT_DOUBLE_EQ(result.CompliancePercent(), 100.0);
}
TEST(COMApartment, Auditor_RunAudit_ViolationDetected) {
    auto auditor = COMApartmentAuditor::Create();
    InterfaceAuditEntry bad;
    bad.interfaceName = "ITestBad";
    bad.declaredModel = ApartmentType::STA;
    bad.actualModel = ApartmentType::MTA;
    bad.hasReentrancyGuard = false;
    auditor.RegisterInterface(bad);
    auto result = auditor.RunAudit();
    EXPECT_FALSE(result.IsFullyCompliant());
    EXPECT_GE(result.recommendations.size(), 1u);
}
TEST(COMApartment, TestMatrix_GeneratesScenarios) {
    auto auditor = COMApartmentAuditor::Create();
    auto matrix = auditor.GenerateTestMatrix();
    EXPECT_GE(matrix.size(), 5u);
    EXPECT_EQ(matrix[0].sourceApartment, ApartmentType::STA);
}
TEST(COMApartment, StabilityImprovements_Exist) {
    auto auditor = COMApartmentAuditor::Create();
    auto fixes = auditor.GetImprovements();
    EXPECT_GE(fixes.size(), 5u);
}
TEST(COMApartment, ThreadSafetyValidator_SingleThread) {
    ThreadSafetyValidator v;
    v.RecordAccess(1, "IThumbnailProvider");
    v.RecordAccess(1, "IThumbnailProvider");
    EXPECT_FALSE(v.HasCrossThreadAccess("IThumbnailProvider"));
    EXPECT_EQ(v.UniqueThreadCount("IThumbnailProvider"), 1u);
}
TEST(COMApartment, ThreadSafetyValidator_CrossThread) {
    ThreadSafetyValidator v;
    v.RecordAccess(1, "ITest");
    v.RecordAccess(2, "ITest");
    EXPECT_TRUE(v.HasCrossThreadAccess("ITest"));
    EXPECT_EQ(v.UniqueThreadCount("ITest"), 2u);
}
TEST(COMApartment, ThreadSafetyValidator_Reset) {
    ThreadSafetyValidator v;
    v.RecordAccess(1, "ITest");
    v.Reset();
    EXPECT_EQ(v.UniqueThreadCount("ITest"), 0u);
}
TEST(COMApartment, AuditResult_ComplianceZeroInterfaces) {
    ApartmentAuditResult r;
    EXPECT_DOUBLE_EQ(r.CompliancePercent(), 100.0);
}

