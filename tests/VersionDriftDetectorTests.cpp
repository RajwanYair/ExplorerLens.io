// ============================================================================
// ============================================================================

#include <gtest/gtest.h>
#include "../Engine/Core/VersionDriftDetector.h"

using namespace ExplorerLens;

// ── SemanticVersion Tests ──────────────────────────────────────────────────

TEST(Sprint145_VersionDriftDetector, ParseBasicVersion) {
    auto v = SemanticVersion::Parse("7.1.0");
    EXPECT_EQ(v.major, 7);
    EXPECT_EQ(v.minor, 1);
    EXPECT_EQ(v.patch, 0);
    EXPECT_TRUE(v.preRelease.empty());
}

TEST(Sprint145_VersionDriftDetector, ParseVersionWithPreRelease) {
    auto v = SemanticVersion::Parse("8.0.0-beta.1");
    EXPECT_EQ(v.major, 8);
    EXPECT_EQ(v.minor, 0);
    EXPECT_EQ(v.patch, 0);
    EXPECT_EQ(v.preRelease, "beta.1");
}

TEST(Sprint145_VersionDriftDetector, ParseVersionWithMeta) {
    auto v = SemanticVersion::Parse("7.1.0+20260218");
    EXPECT_EQ(v.buildMeta, "20260218");
}

TEST(Sprint145_VersionDriftDetector, VersionComparison) {
    auto v710 = SemanticVersion::Parse("7.1.0");
    auto v700 = SemanticVersion::Parse("7.0.0");
    auto v800 = SemanticVersion::Parse("8.0.0");
    EXPECT_TRUE(v700 < v710);
    EXPECT_TRUE(v710 < v800);
    EXPECT_FALSE(v710 < v700);
}

TEST(Sprint145_VersionDriftDetector, VersionToString) {
    SemanticVersion v{7, 1, 0, "rc.1", "build42"};
    EXPECT_EQ(v.ToString(), "7.1.0-rc.1+build42");
}

// ── DriftScanPolicy Tests ──────────────────────────────────────────────────

TEST(Sprint145_VersionDriftDetector, DefaultPolicyCanonicalVersion) {
    auto policy = DefaultPolicy();
    EXPECT_EQ(policy.canonicalVersion.major, 7);
    EXPECT_EQ(policy.canonicalVersion.minor, 1);
    EXPECT_TRUE(policy.allowPatchDrift);
}

// ── Content Scanning Tests ─────────────────────────────────────────────────

TEST(Sprint145_VersionDriftDetector, ScanContentNoVersion) {
    VersionDriftDetector det;
    auto results = det.ScanContent("test.md", "This file has no version info.", ArtifactKind::Documentation);
    EXPECT_TRUE(results.empty());
}

TEST(Sprint145_VersionDriftDetector, ScanContentMatchingVersion) {
    VersionDriftDetector det;
    auto results = det.ScanContent("test.md", "Version: v7.1.0", ArtifactKind::Documentation);
    EXPECT_TRUE(results.empty());  // matches canonical — no drift
}

TEST(Sprint145_VersionDriftDetector, ScanContentStaleVersion) {
    VersionDriftDetector det;
    auto results = det.ScanContent("old.md", "Version: v6.2.0\nSome text.", ArtifactKind::Documentation);
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].foundVersion, "6.2.0");
    EXPECT_EQ(results[0].lineNumber, 1);
    EXPECT_EQ(results[0].severity, DriftSeverity::Error);
}

TEST(Sprint145_VersionDriftDetector, ScanContentCriticalMajorDrift) {
    VersionDriftDetector det;
    auto results = det.ScanContent("ancient.md", "v5.0.0 legacy", ArtifactKind::Documentation);
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].severity, DriftSeverity::Critical);
}

TEST(Sprint145_VersionDriftDetector, ScanContentPatchDriftWarning) {
    DriftScanPolicy pol;
    pol.canonicalVersion = SemanticVersion{7, 1, 2};
    pol.allowPatchDrift = true;
    pol.maxAcceptableDrift = 1;
    VersionDriftDetector det(pol);
    auto results = det.ScanContent("near.md", "v7.1.1", ArtifactKind::Documentation);
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].severity, DriftSeverity::Warning);
}

// ── Full Scan Tests ────────────────────────────────────────────────────────

TEST(Sprint145_VersionDriftDetector, FullScanClean) {
    VersionDriftDetector det;
    det.AddArtifact("README.md", ArtifactKind::Documentation);
    det.AddArtifact("config.h", ArtifactKind::Header);
    auto result = det.Scan([](const std::string&) { return "ExplorerLens v7.1.0"; });
    EXPECT_TRUE(result.IsClean());
    EXPECT_EQ(result.totalFilesScanned, 2);
    EXPECT_EQ(result.Score(), 100);
}

TEST(Sprint145_VersionDriftDetector, FullScanWithDrift) {
    VersionDriftDetector det;
    det.AddArtifact("old.md", ArtifactKind::Documentation);
    auto result = det.Scan([](const std::string&) {
        return "Version 6.2.0\nAlso 5.0.0 legacy";
    });
    EXPECT_FALSE(result.IsClean());
    EXPECT_EQ(result.totalDriftEntries, 2);
    EXPECT_GE(result.errorCount + result.criticalCount, 1);
}

TEST(Sprint145_VersionDriftDetector, ExcludePatternsSkipFiles) {
    DriftScanPolicy pol = DefaultPolicy();
    pol.excludePatterns = {"external/"};
    VersionDriftDetector det(pol);
    det.AddArtifact("external/lib/readme.md", ArtifactKind::Documentation);
    auto result = det.Scan([](const std::string&) { return "v5.0.0"; });
    EXPECT_EQ(result.totalFilesScanned, 0);
}

// ── Report Formatting ──────────────────────────────────────────────────────

TEST(Sprint145_VersionDriftDetector, FormatReportContainsStatus) {
    DriftScanResult r;
    r.totalFilesScanned = 10;
    r.totalDriftEntries = 0;
    auto report = VersionDriftDetector::FormatReport(r);
    EXPECT_NE(report.find("PASS"), std::string::npos);
    EXPECT_NE(report.find("Files scanned: 10"), std::string::npos);
}

