// Validates semantic versioning, drift classification, gate policies, and compliance reporting

#include <gtest/gtest.h>
#include "Core/VersionDriftGate.h"

using namespace ExplorerLens::VersionDrift;

// ── SemanticVersion Parsing ──────────────────────────────────────

TEST(Sprint125_VersionDrift, SemanticVersion_Parse_Standard) {
    auto v = SemanticVersion::Parse("7.1.0");
    EXPECT_EQ(v.major, 7);
    EXPECT_EQ(v.minor, 1);
    EXPECT_EQ(v.patch, 0);
    EXPECT_TRUE(v.preRelease.empty());
    EXPECT_TRUE(v.buildMeta.empty());
}

TEST(Sprint125_VersionDrift, SemanticVersion_Parse_PreRelease) {
    auto v = SemanticVersion::Parse("8.0.0-beta.1");
    EXPECT_EQ(v.major, 8);
    EXPECT_EQ(v.minor, 0);
    EXPECT_EQ(v.patch, 0);
    EXPECT_EQ(v.preRelease, "beta.1");
}

TEST(Sprint125_VersionDrift, SemanticVersion_Parse_BuildMeta) {
    auto v = SemanticVersion::Parse("7.1.0+20260218");
    EXPECT_EQ(v.major, 7);
    EXPECT_EQ(v.minor, 1);
    EXPECT_EQ(v.patch, 0);
    EXPECT_EQ(v.buildMeta, "20260218");
}

TEST(Sprint125_VersionDrift, SemanticVersion_ToString) {
    auto v = SemanticVersion::Parse("8.2.0-rc.1+build.42");
    EXPECT_EQ(v.ToString(), "8.2.0-rc.1+build.42");
}

TEST(Sprint125_VersionDrift, SemanticVersion_Comparison) {
    auto v710 = SemanticVersion::Parse("7.1.0");
    auto v700 = SemanticVersion::Parse("7.0.0");
    auto v800 = SemanticVersion::Parse("8.0.0");
    auto v711 = SemanticVersion::Parse("7.1.1");

    EXPECT_TRUE(v710 > v700);
    EXPECT_TRUE(v800 > v710);
    EXPECT_TRUE(v711 > v710);
    EXPECT_TRUE(v700 < v710);
    EXPECT_EQ(v710, v710);
    EXPECT_NE(v710, v700);
}

TEST(Sprint125_VersionDrift, SemanticVersion_PreReleaseLessThanRelease) {
    auto release = SemanticVersion::Parse("8.0.0");
    auto beta = SemanticVersion::Parse("8.0.0-beta.1");
    EXPECT_TRUE(beta < release);
    EXPECT_TRUE(release > beta);
}

// ── Drift Classification ─────────────────────────────────────────

TEST(Sprint125_VersionDrift, ClassifyDrift_ExactMatch) {
    auto gate = VersionDriftGate::Create("7.1.0");
    EXPECT_EQ(gate.ClassifyDrift("7.1.0"), DriftSeverity::None);
}

TEST(Sprint125_VersionDrift, ClassifyDrift_PatchBehind) {
    auto gate = VersionDriftGate::Create("7.1.1");
    EXPECT_EQ(gate.ClassifyDrift("7.1.0"), DriftSeverity::Minor);
}

TEST(Sprint125_VersionDrift, ClassifyDrift_MinorBehind) {
    auto gate = VersionDriftGate::Create("7.1.0");
    EXPECT_EQ(gate.ClassifyDrift("7.0.0"), DriftSeverity::Moderate);
}

TEST(Sprint125_VersionDrift, ClassifyDrift_MajorBehind) {
    auto gate = VersionDriftGate::Create("7.1.0");
    EXPECT_EQ(gate.ClassifyDrift("6.2.0"), DriftSeverity::Major);
}

TEST(Sprint125_VersionDrift, ClassifyDrift_Critical) {
    auto gate = VersionDriftGate::Create("7.1.0");
    EXPECT_EQ(gate.ClassifyDrift("5.4.0"), DriftSeverity::Critical);
}

// ── Gate Creation ─────────────────────────────────────────────────

TEST(Sprint125_VersionDrift, GateCreate_Canonical) {
    auto gate = VersionDriftGate::Create("7.1.0");
    EXPECT_EQ(gate.CanonicalString(), "7.1.0");
    EXPECT_EQ(gate.CanonicalVersion().major, 7);
    EXPECT_EQ(gate.CanonicalVersion().minor, 1);
}

TEST(Sprint125_VersionDrift, GateCreate_InitializesStalePatterns) {
    auto gate = VersionDriftGate::Create("7.1.0");
    EXPECT_GE(gate.StalePatterns().size(), 3u);
}

// ── Source Registration & Validation ──────────────────────────────

TEST(Sprint125_VersionDrift, RegisterSource_SourceCount) {
    auto gate = VersionDriftGate::Create("7.1.0");
    gate.RegisterSource("README.md", "7.1.0", "doc");
    gate.RegisterSource("CHANGELOG.md", "7.1.0", "doc");
    gate.RegisterSource("Engine/CMakeLists.txt", "7.1.0", "code");
    EXPECT_EQ(gate.SourceCount(), 3u);
}

TEST(Sprint125_VersionDrift, Validate_AllClean) {
    auto gate = VersionDriftGate::Create("7.1.0");
    gate.RegisterSource("README.md", "7.1.0");
    gate.RegisterSource("CHANGELOG.md", "7.1.0");
    auto report = gate.Validate();
    EXPECT_TRUE(report.IsClean());
    EXPECT_EQ(report.driftingSources, 0);
    EXPECT_EQ(report.worstSeverity, DriftSeverity::None);
    EXPECT_EQ(report.SeverityText(), "CLEAN");
}

TEST(Sprint125_VersionDrift, Validate_DetectsDrift) {
    auto gate = VersionDriftGate::Create("7.1.0");
    gate.RegisterSource("DECODER_STATUS.md", "5.4.0", "doc");
    gate.AddReference("DECODER_STATUS.md", 1, "5.4.0", "Version: 5.4.0");
    auto report = gate.Validate();
    EXPECT_FALSE(report.IsClean());
    EXPECT_EQ(report.driftingSources, 1);
    EXPECT_EQ(report.staleReferences, 1);
    EXPECT_GE(report.worstSeverity, DriftSeverity::Critical);
}

TEST(Sprint125_VersionDrift, Validate_Compliance) {
    auto gate = VersionDriftGate::Create("7.1.0");
    gate.RegisterSource("file1.md", "7.1.0");
    gate.AddReference("file1.md", 1, "7.1.0");
    gate.AddReference("file1.md", 10, "7.1.0");
    gate.RegisterSource("file2.md", "6.2.0");
    gate.AddReference("file2.md", 1, "6.2.0");
    auto report = gate.Validate();
    EXPECT_DOUBLE_EQ(report.CompliancePercent(), 200.0 / 3.0 * 100.0 / 100.0);
}

// ── Gate Policy Enforcement ──────────────────────────────────────

TEST(Sprint125_VersionDrift, GateCheck_PassesStrict) {
    auto gate = VersionDriftGate::Create("7.1.0");
    gate.RegisterSource("README.md", "7.1.0");
    auto result = gate.CheckGate(GatePolicies::Strict());
    EXPECT_TRUE(result.passed);
}

TEST(Sprint125_VersionDrift, GateCheck_FailsOnMajorDrift) {
    auto gate = VersionDriftGate::Create("7.1.0");
    gate.RegisterSource("old_doc.md", "6.0.0");
    gate.AddReference("old_doc.md", 5, "6.0.0");
    auto result = gate.CheckGate(GatePolicies::CI());
    EXPECT_FALSE(result.passed);
}

TEST(Sprint125_VersionDrift, GateCheck_PermissiveAllowsModerate) {
    auto gate = VersionDriftGate::Create("7.1.0");
    gate.RegisterSource("guide.md", "7.0.0");
    gate.AddReference("guide.md", 1, "7.0.0");
    auto result = gate.CheckGate(GatePolicies::Permissive());
    EXPECT_TRUE(result.passed);
}

// ── Preset Policies ──────────────────────────────────────────────

TEST(Sprint125_VersionDrift, GatePolicies_StrictConfig) {
    auto p = GatePolicies::Strict();
    EXPECT_EQ(p.maxAllowed, DriftSeverity::None);
    EXPECT_DOUBLE_EQ(p.minCompliancePercent, 100.0);
    EXPECT_TRUE(p.failOnAnyMajorDrift);
}

TEST(Sprint125_VersionDrift, GatePolicies_CIConfig) {
    auto p = GatePolicies::CI();
    EXPECT_EQ(p.maxAllowed, DriftSeverity::Minor);
    EXPECT_DOUBLE_EQ(p.minCompliancePercent, 95.0);
}

TEST(Sprint125_VersionDrift, GatePolicies_PermissiveConfig) {
    auto p = GatePolicies::Permissive();
    EXPECT_EQ(p.maxAllowed, DriftSeverity::Moderate);
    EXPECT_FALSE(p.failOnAnyMajorDrift);
}

TEST(Sprint125_VersionDrift, GatePolicy_ExemptFiles) {
    auto p = GatePolicies::CI();
    p.exemptFiles.push_back("CHANGELOG.md");
    EXPECT_TRUE(p.IsExempt("CHANGELOG.md"));
    EXPECT_FALSE(p.IsExempt("README.md"));
}

// ── Report Formatting ────────────────────────────────────────────

TEST(Sprint125_VersionDrift, DriftReport_SeverityText) {
    DriftReport r;
    r.worstSeverity = DriftSeverity::None;
    EXPECT_EQ(r.SeverityText(), "CLEAN");
    r.worstSeverity = DriftSeverity::Critical;
    EXPECT_EQ(r.SeverityText(), "CRITICAL_DRIFT");
}

TEST(Sprint125_VersionDrift, DriftReport_CompliancePercent_AllClean) {
    DriftReport r;
    r.totalReferences = 10;
    r.staleReferences = 0;
    EXPECT_DOUBLE_EQ(r.CompliancePercent(), 100.0);
}

TEST(Sprint125_VersionDrift, DriftReport_CompliancePercent_NoRefs) {
    DriftReport r;
    r.totalReferences = 0;
    r.staleReferences = 0;
    EXPECT_DOUBLE_EQ(r.CompliancePercent(), 100.0);
}

