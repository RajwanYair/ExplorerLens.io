//==============================================================================
// ExplorerLens — Sprint 9 Tests: Version Normalization & v7.0 Release Notes
// Tests version scanning, stale reference detection, decoder status registry,
// release notes generation, and documentation integrity reporting.
//==============================================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>

// Header under test
#include "../Engine/Utils/VersionNormalization.h"

using namespace ExplorerLens::Engine::Docs;

//==============================================================================
// VersionInfo Tests
//==============================================================================

TEST(VersionInfo, CurrentVersion)
{
    auto v = VersionInfo::Current();
    EXPECT_EQ(v.major, 7u);
    EXPECT_EQ(v.minor, 0u);
    EXPECT_EQ(v.patch, 0u);
    EXPECT_TRUE(v.preRelease.empty());
}

TEST(VersionInfo, ToString)
{
    auto v = VersionInfo::Current();
    EXPECT_EQ(v.ToString(), "v7.0.0");
}

TEST(VersionInfo, ToShort)
{
    auto v = VersionInfo::Current();
    EXPECT_EQ(v.ToShort(), "v7.0");
}

TEST(VersionInfo, PreReleaseVersion)
{
    VersionInfo v{7, 0, 0, "rc1", ""};
    EXPECT_EQ(v.ToString(), "v7.0.0-rc1");
}

TEST(VersionInfo, BuildMetadata)
{
    VersionInfo v{7, 0, 0, "", "build.1234"};
    EXPECT_EQ(v.ToString(), "v7.0.0+build.1234");
}

TEST(VersionInfo, Equality)
{
    VersionInfo a{7, 0, 0};
    VersionInfo b{7, 0, 0};
    EXPECT_EQ(a, b);
}

TEST(VersionInfo, Inequality)
{
    VersionInfo a{7, 0, 0};
    VersionInfo b{6, 2, 0};
    EXPECT_NE(a, b);
}

//==============================================================================
// Version Scanner Tests
//==============================================================================

TEST(VersionScanner, DetectStaleVersion)
{
    VersionScanner scanner;
    std::string content = "ExplorerLens v5.4.0 - Released 2024\n"
                          "Current version: v7.0.0\n"
                          "Previously v6.2 was the last release.\n";
    auto refs = scanner.ScanContent("test.md", content);
    // Should find v5.4 and v6.2 as stale
    EXPECT_GE(refs.size(), 2u);
    for (auto& ref : refs) {
        EXPECT_TRUE(ref.isStale);
    }
}

TEST(VersionScanner, NoStaleInCleanContent)
{
    VersionScanner scanner;
    std::string content = "ExplorerLens v7.0.0\n"
                          "All decoders updated to latest.\n";
    auto refs = scanner.ScanContent("clean.md", content);
    EXPECT_EQ(refs.size(), 0u);
}

TEST(VersionScanner, ExcludedPath)
{
    VersionScanner scanner;
    std::string content = "Old version v5.0\n";
    auto refs = scanner.ScanContent("CHANGELOG.md", content);
    EXPECT_EQ(refs.size(), 0u);
}

TEST(VersionScanner, ExcludedBuildDir)
{
    VersionScanner scanner;
    auto refs = scanner.ScanContent("build/output.log", "v5.4.0\n");
    EXPECT_EQ(refs.size(), 0u);
}

TEST(VersionScanner, IsCanonical)
{
    VersionScanner scanner;
    EXPECT_TRUE(scanner.IsCanonical("v7.0.0"));
    EXPECT_TRUE(scanner.IsCanonical("v7.0"));
    EXPECT_TRUE(scanner.IsCanonical("7.0.0"));
    EXPECT_FALSE(scanner.IsCanonical("v6.2.0"));
    EXPECT_FALSE(scanner.IsCanonical("v5.4.0"));
}

TEST(VersionScanner, CountStaleReferences)
{
    VersionScanner scanner;
    std::string content = "v5.0 and v5.4 and v6.0 are old versions.";
    auto count = scanner.CountStaleReferences(content);
    EXPECT_GE(count, 3u);
}

TEST(VersionScanner, StalePatternCount)
{
    VersionScanner scanner;
    EXPECT_GE(scanner.Config().staleVersionPatterns.size(), 8u);
}

TEST(VersionScanner, LineNumberTracking)
{
    VersionScanner scanner;
    std::string content = "Line 1\nLine 2 v5.4\nLine 3\n";
    auto refs = scanner.ScanContent("test.md", content);
    ASSERT_GE(refs.size(), 1u);
    EXPECT_EQ(refs[0].lineNumber, 2u);
}

//==============================================================================
// Decoder Status Tests
//==============================================================================

TEST(DecoderStatus, StatusNames)
{
    EXPECT_STREQ(DecoderStatusName(DecoderStatus::Stable), "Stable");
    EXPECT_STREQ(DecoderStatusName(DecoderStatus::Beta), "Beta");
    EXPECT_STREQ(DecoderStatusName(DecoderStatus::Experimental), "Experimental");
    EXPECT_STREQ(DecoderStatusName(DecoderStatus::External), "External");
}

TEST(DecoderStatus, Badges)
{
    DecoderDocEntry entry;
    entry.status = DecoderStatus::Stable;
    EXPECT_EQ(entry.StatusBadge(), "[STABLE]");
    entry.status = DecoderStatus::Beta;
    EXPECT_EQ(entry.StatusBadge(), "[BETA]");
    entry.status = DecoderStatus::Planned;
    EXPECT_EQ(entry.StatusBadge(), "[PLANNED]");
}

//==============================================================================
// Decoder Status Registry Tests
//==============================================================================

TEST(DecoderRegistry, Populated)
{
    DecoderStatusRegistry registry;
    EXPECT_GE(registry.TotalCount(), 20u);
}

TEST(DecoderRegistry, AllStable)
{
    DecoderStatusRegistry registry;
    auto stable = registry.GetByStatus(DecoderStatus::Stable);
    EXPECT_GE(stable.size(), 20u);
}

TEST(DecoderRegistry, FormatCoverage)
{
    DecoderStatusRegistry registry;
    EXPECT_GE(registry.FormatCount(), 40u);
}

TEST(DecoderRegistry, MarkdownTable)
{
    DecoderStatusRegistry registry;
    auto md = registry.GenerateMarkdownTable();
    EXPECT_NE(md.find("| Decoder | Status |"), std::string::npos);
    EXPECT_NE(md.find("JPEG"), std::string::npos);
    EXPECT_NE(md.find("WebP"), std::string::npos);
    EXPECT_NE(md.find("HEIF"), std::string::npos);
    EXPECT_NE(md.find("[STABLE]"), std::string::npos);
}

TEST(DecoderRegistry, ContainsJXL)
{
    DecoderStatusRegistry registry;
    bool found = false;
    for (auto& d : registry.AllDecoders()) {
        if (d.name.find("JPEG XL") != std::string::npos) {
            found = true;
            EXPECT_EQ(d.status, DecoderStatus::Stable);
            EXPECT_NE(d.library.find("libjxl"), std::string::npos);
        }
    }
    EXPECT_TRUE(found);
}

TEST(DecoderRegistry, ContainsAVIF)
{
    DecoderStatusRegistry registry;
    bool found = false;
    for (auto& d : registry.AllDecoders()) {
        if (d.name.find("AVIF") != std::string::npos) {
            found = true;
            EXPECT_EQ(d.status, DecoderStatus::Stable);
        }
    }
    EXPECT_TRUE(found);
}

//==============================================================================
// Release Notes Generator Tests
//==============================================================================

TEST(ReleaseNotes, BasicGeneration)
{
    ReleaseNotesGenerator gen;
    gen.AddFeature("HEIF/HEIC support via libheif 1.19.5");
    gen.AddFeature("JPEG XL support via libjxl 0.11.1");
    gen.AddBugFix("Fixed RAW decoder memory leak");

    auto md = gen.Generate();
    EXPECT_NE(md.find("v7.0.0"), std::string::npos);
    EXPECT_NE(md.find("New Features"), std::string::npos);
    EXPECT_NE(md.find("Bug Fixes"), std::string::npos);
    EXPECT_NE(md.find("HEIF"), std::string::npos);
}

TEST(ReleaseNotes, NoteCount)
{
    ReleaseNotesGenerator gen;
    gen.AddFeature("Feature 1");
    gen.AddFeature("Feature 2");
    gen.AddBugFix("Fix 1");
    gen.AddImprovement("Improvement 1");
    EXPECT_EQ(gen.NoteCount(), 4u);
}

TEST(ReleaseNotes, CategoryCount)
{
    ReleaseNotesGenerator gen;
    gen.AddFeature("F1");
    gen.AddBugFix("B1");
    gen.AddImprovement("I1");
    gen.AddBreakingChange("BC1");
    EXPECT_EQ(gen.CategoryCount(), 4u);
}

TEST(ReleaseNotes, Version)
{
    ReleaseNotesGenerator gen;
    EXPECT_EQ(gen.Version().ToString(), "v7.0.0");
}

TEST(ReleaseNotes, EmptyGenerate)
{
    ReleaseNotesGenerator gen;
    auto md = gen.Generate();
    EXPECT_NE(md.find("v7.0.0"), std::string::npos);
    EXPECT_EQ(gen.NoteCount(), 0u);
}

//==============================================================================
// Documentation Integrity Report Tests
//==============================================================================

TEST(DocIntegrity, CleanReport)
{
    DocIntegrityReport report;
    report.filesScanned = 10;
    report.cleanDocs = 10;
    report.staleReferences = 0;
    EXPECT_TRUE(report.IsClean());
    EXPECT_DOUBLE_EQ(report.IntegrityPercent(), 100.0);
}

TEST(DocIntegrity, DirtyReport)
{
    DocIntegrityReport report;
    report.filesScanned = 10;
    report.cleanDocs = 7;
    report.staleDocs = 3;
    report.staleReferences = 5;
    EXPECT_FALSE(report.IsClean());
    EXPECT_DOUBLE_EQ(report.IntegrityPercent(), 70.0);
}

TEST(DocIntegrity, EmptyReport)
{
    DocIntegrityReport report;
    EXPECT_DOUBLE_EQ(report.IntegrityPercent(), 100.0);
}

//==============================================================================
// Stale Doc Tracker Tests
//==============================================================================

TEST(StaleTracker, RegisterKnownDocs)
{
    StaleDocTracker tracker;
    tracker.RegisterKnownStaleDocs();
    EXPECT_EQ(tracker.TotalStale(), 12u);
}

TEST(StaleTracker, FixDocs)
{
    StaleDocTracker tracker;
    tracker.RegisterKnownStaleDocs();
    tracker.MarkFixed("README.md");
    tracker.MarkFixed("DECODER_STATUS.md");
    EXPECT_EQ(tracker.FixedCount(), 2u);
    EXPECT_EQ(tracker.RemainingCount(), 10u);
    EXPECT_TRUE(tracker.IsFixed("README.md"));
    EXPECT_FALSE(tracker.IsFixed("USER_GUIDE.md"));
}

TEST(StaleTracker, AllFixed)
{
    StaleDocTracker tracker;
    tracker.RegisterKnownStaleDocs();
    auto remaining = tracker.GetRemaining();
    for (auto& doc : remaining) tracker.MarkFixed(doc);
    EXPECT_EQ(tracker.RemainingCount(), 0u);
    EXPECT_DOUBLE_EQ(tracker.ProgressPercent(), 100.0);
}

TEST(StaleTracker, ProgressPercent)
{
    StaleDocTracker tracker;
    tracker.RegisterKnownStaleDocs();
    tracker.MarkFixed("README.md");
    tracker.MarkFixed("USER_GUIDE.md");
    tracker.MarkFixed("KNOWN_ISSUES.md");
    // 3/12 = 25%
    EXPECT_DOUBLE_EQ(tracker.ProgressPercent(), 25.0);
}

