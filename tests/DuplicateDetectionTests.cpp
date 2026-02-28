//==============================================================================
// ExplorerLens — Duplicate Detection & Perceptual Hashing
// Tests pHash/dHash computation, Hamming distance, similarity search,
// duplicate grouping, scan results, CSV/JSON export.
//==============================================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "../Engine/Utils/PerceptualHashing.h"

using namespace ExplorerLens::Engine::Utils;

//==============================================================================
// Hash Algorithm Tests
//==============================================================================

TEST(HashAlgorithm, Names)
{
    EXPECT_STREQ(HashAlgorithmName(HashAlgorithm::pHash), "pHash (Perceptual)");
    EXPECT_STREQ(HashAlgorithmName(HashAlgorithm::dHash), "dHash (Difference)");
    EXPECT_STREQ(HashAlgorithmName(HashAlgorithm::aHash), "aHash (Average)");
    EXPECT_STREQ(HashAlgorithmName(HashAlgorithm::wHash), "wHash (Wavelet)");
}

//==============================================================================
// PerceptualHash Tests
//==============================================================================

TEST(PerceptualHash, DefaultInvalid)
{
    PerceptualHash h;
    EXPECT_FALSE(h.IsValid());
}

TEST(PerceptualHash, ToHex)
{
    PerceptualHash h;
    h.value = 0xDEADBEEF12345678ULL;
    EXPECT_EQ(h.ToHex(), "deadbeef12345678");
}

TEST(PerceptualHash, HammingDistanceSame)
{
    PerceptualHash a, b;
    a.value = 0xFFFF;
    b.value = 0xFFFF;
    EXPECT_EQ(a.HammingDistance(b), 0u);
}

TEST(PerceptualHash, HammingDistanceOneBit)
{
    PerceptualHash a, b;
    a.value = 0b1000;
    b.value = 0b0000;
    EXPECT_EQ(a.HammingDistance(b), 1u);
}

TEST(PerceptualHash, HammingDistanceAllBits)
{
    PerceptualHash a, b;
    a.value = 0x0;
    b.value = 0xFFFFFFFFFFFFFFFFULL;
    EXPECT_EQ(a.HammingDistance(b), 64u);
}

TEST(PerceptualHash, SimilarityExact)
{
    PerceptualHash a, b;
    a.value = 0xABCD;
    b.value = 0xABCD;
    EXPECT_DOUBLE_EQ(a.Similarity(b), 100.0);
}

TEST(PerceptualHash, SimilarityZero)
{
    PerceptualHash a, b;
    a.value = 0x0;
    b.value = 0xFFFFFFFFFFFFFFFFULL;
    EXPECT_DOUBLE_EQ(a.Similarity(b), 0.0);
}

TEST(PerceptualHash, IsExactMatch)
{
    PerceptualHash a, b;
    a.value = 12345;
    b.value = 12345;
    EXPECT_TRUE(a.IsExactMatch(b));
    b.value = 12346;
    EXPECT_FALSE(a.IsExactMatch(b));
}

TEST(PerceptualHash, IsSimilar)
{
    PerceptualHash a, b;
    a.value = 0xFF00;
    b.value = 0xFF01; // 1 bit different
    EXPECT_TRUE(a.IsSimilar(b, 5));
    b.value = 0x00FF; // many bits different
    EXPECT_FALSE(a.IsSimilar(b, 5));
}

//==============================================================================
// Hash Compute Params Tests
//==============================================================================

TEST(HashParams, PHashDefaults)
{
    auto p = HashComputeParams::ForPHash();
    EXPECT_EQ(p.algorithm, HashAlgorithm::pHash);
    EXPECT_EQ(p.resizeWidth, 32u);
    EXPECT_TRUE(p.convertToGrayscale);
}

TEST(HashParams, DHashDefaults)
{
    auto p = HashComputeParams::ForDHash();
    EXPECT_EQ(p.algorithm, HashAlgorithm::dHash);
    EXPECT_EQ(p.resizeWidth, 9u);
    EXPECT_EQ(p.resizeHeight, 8u);
}

TEST(HashParams, AHashDefaults)
{
    auto p = HashComputeParams::ForAHash();
    EXPECT_EQ(p.algorithm, HashAlgorithm::aHash);
    EXPECT_EQ(p.resizeWidth, 8u);
}

//==============================================================================
// Hash Entry Tests
//==============================================================================

TEST(HashEntry, NoPHash)
{
    HashEntry e;
    EXPECT_FALSE(e.HasPHash());
    EXPECT_FALSE(e.HasDHash());
    EXPECT_FALSE(e.HasBothHashes());
}

TEST(HashEntry, WithBothHashes)
{
    HashEntry e;
    e.pHash.value = 0xABCD;
    e.dHash.value = 0x1234;
    EXPECT_TRUE(e.HasBothHashes());
}

//==============================================================================
// Duplicate Group Tests
//==============================================================================

TEST(DuplicateGroup, EmptyHasNoDuplicates)
{
    DuplicateGroup g;
    EXPECT_FALSE(g.HasDuplicates());
    EXPECT_EQ(g.Size(), 0u);
    EXPECT_EQ(g.WastedSpace(), 0u);
}

TEST(DuplicateGroup, TwoMembersIsDuplicate)
{
    DuplicateGroup g;
    g.members.push_back({"a.jpg", {}, 0, 100, 1000});
    g.members.push_back({"b.jpg", {}, 2, 97, 800});
    EXPECT_TRUE(g.HasDuplicates());
    EXPECT_EQ(g.Size(), 2u);
}

TEST(DuplicateGroup, WastedSpace)
{
    DuplicateGroup g;
    g.members.push_back({"a.jpg", {}, 0, 100, 5000});
    g.members.push_back({"b.jpg", {}, 2, 97, 3000});
    g.members.push_back({"c.jpg", {}, 3, 95, 4000});
    EXPECT_EQ(g.TotalFileSize(), 12000u);
    EXPECT_EQ(g.WastedSpace(), 7000u); // 12000 - 5000 (largest)
}

//==============================================================================
// Similarity Search Engine Tests
//==============================================================================

TEST(SearchEngine, EmptyEngine)
{
    SimilaritySearchEngine engine;
    EXPECT_EQ(engine.EntryCount(), 0u);
}

TEST(SearchEngine, AddAndFind)
{
    SimilaritySearchEngine engine;
    PerceptualHash h;
    h.value = 0xAAAA;
    engine.AddHash("test.jpg", h, 1000);
    EXPECT_EQ(engine.EntryCount(), 1u);

    auto results = engine.FindSimilar(h, 0);
    EXPECT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0].filePath, "test.jpg");
}

TEST(SearchEngine, FindExact)
{
    SimilaritySearchEngine engine;
    PerceptualHash h1, h2;
    h1.value = 0xAAAA;
    h2.value = 0xBBBB;
    engine.AddHash("a.jpg", h1, 1000);
    engine.AddHash("b.jpg", h2, 2000);

    auto results = engine.FindExact(h1);
    EXPECT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0].filePath, "a.jpg");
}

TEST(SearchEngine, FindSimilarWithThreshold)
{
    SimilaritySearchEngine engine;
    PerceptualHash h1, h2, h3;
    h1.value = 0xFF00;
    h2.value = 0xFF01;  // 1 bit diff
    h3.value = 0x00FF;  // many bits diff
    engine.AddHash("a.jpg", h1, 1000);
    engine.AddHash("b.jpg", h2, 1000);
    engine.AddHash("c.jpg", h3, 1000);

    auto results = engine.FindSimilar(h1, 5);
    EXPECT_EQ(results.size(), 2u); // a.jpg (0) and b.jpg (1)
}

TEST(SearchEngine, FindAllDuplicates)
{
    SimilaritySearchEngine engine;
    PerceptualHash h1, h2, h3;
    h1.value = 0xAAAA;
    h2.value = 0xAAAB;  // 1 bit diff from h1
    h3.value = 0x5555;  // very different
    engine.AddHash("a.jpg", h1, 1000);
    engine.AddHash("b.jpg", h2, 800);
    engine.AddHash("c.jpg", h3, 2000);

    auto groups = engine.FindAllDuplicates(5);
    EXPECT_EQ(groups.size(), 1u); // Only a+b group
    EXPECT_EQ(groups[0].Size(), 2u);
}

TEST(SearchEngine, Clear)
{
    SimilaritySearchEngine engine;
    PerceptualHash h;
    h.value = 1;
    engine.AddHash("test.jpg", h);
    engine.Clear();
    EXPECT_EQ(engine.EntryCount(), 0u);
}

//==============================================================================
// Scan Result Tests
//==============================================================================

TEST(ScanResult, DuplicateRate)
{
    DuplicateScanResult r;
    r.totalFiles = 100;
    r.filesHashed = 100;
    r.totalDuplicates = 25;
    EXPECT_DOUBLE_EQ(r.DuplicateRate(), 25.0);
}

TEST(ScanResult, WastedSizeHuman)
{
    DuplicateScanResult r;
    r.wastedBytes = 500;
    EXPECT_EQ(r.WastedSizeHuman(), "500 B");
    r.wastedBytes = 5000;
    EXPECT_EQ(r.WastedSizeHuman(), "4 KB");
    r.wastedBytes = 5242880;
    EXPECT_EQ(r.WastedSizeHuman(), "5 MB");
    r.wastedBytes = 2147483648ULL;
    EXPECT_EQ(r.WastedSizeHuman(), "2 GB");
}

TEST(ScanResult, Summary)
{
    DuplicateScanResult r;
    r.totalDuplicates = 10;
    r.duplicateGroups = 3;
    r.wastedBytes = 5242880;
    r.filesHashed = 100;
    auto s = r.Summary();
    EXPECT_NE(s.find("10 duplicates"), std::string::npos);
    EXPECT_NE(s.find("3 groups"), std::string::npos);
    EXPECT_NE(s.find("5 MB"), std::string::npos);
}

//==============================================================================
// Export Tests
//==============================================================================

TEST(Export, FormatNames)
{
    EXPECT_STREQ(ExportFormatName(ExportFormat::CSV), "CSV");
    EXPECT_STREQ(ExportFormatName(ExportFormat::JSON), "JSON");
    EXPECT_STREQ(ExportFormatName(ExportFormat::Text), "Text");
}

TEST(Export, CSVHeader)
{
    std::vector<DuplicateGroup> groups;
    auto csv = ExportDuplicatesCSV(groups);
    EXPECT_NE(csv.find("GroupID"), std::string::npos);
    EXPECT_NE(csv.find("FilePath"), std::string::npos);
    EXPECT_NE(csv.find("Hash"), std::string::npos);
}

TEST(Export, CSVWithData)
{
    DuplicateGroup g;
    g.groupId = 1;
    PerceptualHash h;
    h.value = 0xDEAD;
    g.members.push_back({"photo1.jpg", h, 0, 100.0, 5000});
    g.members.push_back({"photo2.jpg", h, 2, 97.0, 4000});

    auto csv = ExportDuplicatesCSV({g});
    EXPECT_NE(csv.find("photo1.jpg"), std::string::npos);
    EXPECT_NE(csv.find("photo2.jpg"), std::string::npos);
}

TEST(Export, JSONStructure)
{
    DuplicateGroup g;
    g.groupId = 1;
    PerceptualHash h;
    h.value = 0xBEEF;
    g.members.push_back({"a.png", h, 0, 100.0, 1000});

    auto json = ExportDuplicatesJSON({g});
    EXPECT_NE(json.find("\"groups\""), std::string::npos);
    EXPECT_NE(json.find("\"groupId\""), std::string::npos);
    EXPECT_NE(json.find("a.png"), std::string::npos);
}

//==============================================================================
// Config Tests
//==============================================================================

TEST(DupConfig, Default)
{
    auto cfg = DuplicateDetectionConfig::Default();
    EXPECT_EQ(cfg.primaryAlgorithm, HashAlgorithm::pHash);
    EXPECT_TRUE(cfg.computeDHash);
    EXPECT_EQ(cfg.similarityThreshold, 10u);
}

TEST(DupConfig, FastScan)
{
    auto cfg = DuplicateDetectionConfig::FastScan();
    EXPECT_EQ(cfg.primaryAlgorithm, HashAlgorithm::dHash);
    EXPECT_FALSE(cfg.computeDHash);
    EXPECT_EQ(cfg.similarityThreshold, 5u);
}

TEST(DupConfig, Strict)
{
    auto cfg = DuplicateDetectionConfig::Strict();
    EXPECT_EQ(cfg.similarityThreshold, 3u);
}

