// ============================================================================
// ============================================================================

#include <gtest/gtest.h>
#include "../Engine/Core/ReproducibleBuildVerifier.h"

using namespace ExplorerLens;

// ── BuildHash Tests ────────────────────────────────────────────────────────

TEST(ReproducibleBuild, HashFromHexRoundTrip) {
    auto h = BuildHash::FromHex("abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789");
    EXPECT_FALSE(h.IsEmpty());
    EXPECT_EQ(h.hexString.size(), 64);
}

TEST(ReproducibleBuild, EmptyHashDetection) {
    BuildHash h;
    EXPECT_TRUE(h.IsEmpty());
}

TEST(ReproducibleBuild, HashEquality) {
    auto h1 = BuildHash::FromHex("aa");
    auto h2 = BuildHash::FromHex("aa");
    auto h3 = BuildHash::FromHex("bb");
    EXPECT_EQ(h1, h2);
    EXPECT_NE(h1, h3);
}

// ── ArtifactType Tests ─────────────────────────────────────────────────────

TEST(ReproducibleBuild, ArtifactTypeNames) {
    EXPECT_STREQ(ArtifactTypeName(ArtifactType::DLL), "DLL");
    EXPECT_STREQ(ArtifactTypeName(ArtifactType::EXE), "EXE");
    EXPECT_STREQ(ArtifactTypeName(ArtifactType::LIB), "LIB");
    EXPECT_STREQ(ArtifactTypeName(ArtifactType::PDB), "PDB");
}

// ── Policy Tests ───────────────────────────────────────────────────────────

TEST(ReproducibleBuild, StrictPolicyDefaults) {
    auto pol = StrictPolicy();
    EXPECT_TRUE(pol.stripTimestamps);
    EXPECT_TRUE(pol.stripPDBPaths);
    EXPECT_EQ(pol.maxSizeDriftPct, 0.1);
}

TEST(ReproducibleBuild, RelaxedPolicyDefaults) {
    auto pol = RelaxedPolicy();
    EXPECT_TRUE(pol.stripBuildMetadata);
    EXPECT_EQ(pol.maxSizeDriftPct, 5.0);
}

// ── Verification Tests ─────────────────────────────────────────────────────

TEST(ReproducibleBuild, IdenticalBuildsAreReproducible) {
    auto hash = BuildHash::FromHex("aabbccdd");
    BuildArtifact art;
    art.path = "LENSShell.dll";
    art.type = ArtifactType::DLL;
    art.sizeBytes = 2940 * 1024;
    art.contentHash = hash;
    art.strippedHash = hash;

    auto buildA = ReproducibleBuildVerifier::CreateManifest("abc123", "main", "Release", {art});
    auto buildB = ReproducibleBuildVerifier::CreateManifest("abc123", "main", "Release", {art});

    ReproducibleBuildVerifier verifier;
    auto result = verifier.Verify(buildA, buildB);
    EXPECT_TRUE(result.IsFullyReproducible());
    EXPECT_EQ(result.reproducibleCount, 1);
    EXPECT_NEAR(result.ReproducibilityScore(), 100.0, 0.1);
}

TEST(ReproducibleBuild, DifferentHashesAreNonReproducible) {
    BuildArtifact artA, artB;
    artA.path = artB.path = "LENSShell.dll";
    artA.sizeBytes = artB.sizeBytes = 2940 * 1024;
    artA.contentHash = artA.strippedHash = BuildHash::FromHex("aa");
    artB.contentHash = artB.strippedHash = BuildHash::FromHex("bb");

    auto buildA = ReproducibleBuildVerifier::CreateManifest("abc", "main", "Release", {artA});
    auto buildB = ReproducibleBuildVerifier::CreateManifest("abc", "main", "Release", {artB});

    ReproducibleBuildVerifier verifier;
    auto result = verifier.Verify(buildA, buildB);
    EXPECT_FALSE(result.IsFullyReproducible());
    EXPECT_EQ(result.nonReproducibleCount, 1);
}

TEST(ReproducibleBuild, TimestampDriftDetected) {
    auto stripped = BuildHash::FromHex("aa");
    BuildArtifact artA, artB;
    artA.path = artB.path = "LENSShell.dll";
    artA.sizeBytes = artB.sizeBytes = 1000;
    artA.contentHash = BuildHash::FromHex("cc");   // raw differs
    artB.contentHash = BuildHash::FromHex("dd");
    artA.strippedHash = stripped;                    // stripped matches
    artB.strippedHash = stripped;

    auto buildA = ReproducibleBuildVerifier::CreateManifest("abc", "main", "Release", {artA});
    auto buildB = ReproducibleBuildVerifier::CreateManifest("abc", "main", "Release", {artB});

    ReproducibleBuildVerifier verifier;
    auto result = verifier.Verify(buildA, buildB);
    EXPECT_EQ(result.timestampDriftCount, 1);
    EXPECT_TRUE(result.IsFullyReproducible());
}

TEST(ReproducibleBuild, MissingArtifactDetected) {
    BuildArtifact art;
    art.path = "LENSShell.dll";
    art.sizeBytes = 1000;
    art.contentHash = art.strippedHash = BuildHash::FromHex("aa");

    auto buildA = ReproducibleBuildVerifier::CreateManifest("abc", "main", "Release", {art});
    BuildManifest buildB;
    buildB.commitHash = "abc";

    ReproducibleBuildVerifier verifier;
    auto result = verifier.Verify(buildA, buildB);
    EXPECT_EQ(result.missingCount, 1);
    EXPECT_FALSE(result.IsFullyReproducible());
}

TEST(ReproducibleBuild, ExcludedPathsSkipped) {
    ReproducibilityPolicy pol = StrictPolicy();
    pol.excludePaths = {"debug/"};
    ReproducibleBuildVerifier verifier(pol);

    BuildArtifact art;
    art.path = "debug/test.pdb";
    art.contentHash = art.strippedHash = BuildHash::FromHex("aa");

    auto buildA = ReproducibleBuildVerifier::CreateManifest("abc", "main", "Release", {art});
    auto buildB = ReproducibleBuildVerifier::CreateManifest("abc", "main", "Release", {art});

    auto result = verifier.Verify(buildA, buildB);
    EXPECT_EQ(result.skippedCount, 1);
    EXPECT_EQ(result.totalArtifacts, 0);
}

// ── Report Formatting ──────────────────────────────────────────────────────

TEST(ReproducibleBuild, FormatReportContainsStatus) {
    VerificationResult r;
    r.totalArtifacts = 5;
    r.reproducibleCount = 5;
    auto report = ReproducibleBuildVerifier::FormatReport(r);
    EXPECT_NE(report.find("REPRODUCIBLE"), std::string::npos);
    EXPECT_NE(report.find("100.0%"), std::string::npos);
}

// ── Manifest Tests ─────────────────────────────────────────────────────────

TEST(ReproducibleBuild, ManifestFindArtifact) {
    BuildArtifact art;
    art.path = "LENSShell.dll";
    auto manifest = ReproducibleBuildVerifier::CreateManifest("abc", "main", "Release", {art});
    EXPECT_NE(manifest.FindArtifact("LENSShell.dll"), nullptr);
    EXPECT_EQ(manifest.FindArtifact("missing.dll"), nullptr);
}

