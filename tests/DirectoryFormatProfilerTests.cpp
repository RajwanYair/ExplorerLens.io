#include <gtest/gtest.h>
#include "Memory/DirectoryFormatProfiler.h"
using namespace ExplorerLens::Memory;

TEST(DirProfiler, ClassifyExtension_JPEG) {
    auto p = DirectoryFormatProfiler::Create();
    EXPECT_EQ(p.ClassifyExtension(".jpg"), FormatFamily::LightweightImage);
    EXPECT_EQ(p.ClassifyExtension(".jpeg"), FormatFamily::LightweightImage);
}
TEST(DirProfiler, ClassifyExtension_HEIF) {
    auto p = DirectoryFormatProfiler::Create();
    EXPECT_EQ(p.ClassifyExtension(".heif"), FormatFamily::ModernImage);
    EXPECT_EQ(p.ClassifyExtension(".jxl"), FormatFamily::ModernImage);
}
TEST(DirProfiler, ClassifyExtension_RAW) {
    auto p = DirectoryFormatProfiler::Create();
    EXPECT_EQ(p.ClassifyExtension(".cr3"), FormatFamily::RawPhoto);
    EXPECT_EQ(p.ClassifyExtension(".arw"), FormatFamily::RawPhoto);
}
TEST(DirProfiler, ClassifyExtension_Unknown) {
    auto p = DirectoryFormatProfiler::Create();
    EXPECT_EQ(p.ClassifyExtension(".xyz"), FormatFamily::Unknown);
}
TEST(DirProfiler, FamilyMapSize) {
    auto p = DirectoryFormatProfiler::Create();
    EXPECT_GE(p.FamilyMapSize(), 60u);
}
TEST(DirProfiler, ProfileDirectory_SingleFormat) {
    auto p = DirectoryFormatProfiler::Create();
    std::vector<std::string> files = {"a.jpg","b.jpg","c.jpg","d.jpg","e.jpg"};
    auto profile = p.ProfileDirectory("C:\\Photos", files);
    EXPECT_EQ(profile.totalFiles, 5);
    EXPECT_TRUE(profile.isSingleFormatMode);
    EXPECT_EQ(profile.dominantFamily, FormatFamily::LightweightImage);
    EXPECT_GE(profile.dominantRatio, 0.8);
}
TEST(DirProfiler, ProfileDirectory_MixedFormat) {
    auto p = DirectoryFormatProfiler::Create();
    std::vector<std::string> files = {"a.jpg","b.png","c.heif","d.pdf","e.mp4"};
    auto profile = p.ProfileDirectory("C:\\Mixed", files);
    EXPECT_EQ(profile.totalFiles, 5);
    EXPECT_FALSE(profile.isSingleFormatMode);
    EXPECT_LT(profile.dominantRatio, 0.8);
}
TEST(DirProfiler, ProfileDirectory_DominantExtension) {
    auto p = DirectoryFormatProfiler::Create();
    std::vector<std::string> files = {"a.png","b.png","c.png","d.jpg"};
    auto profile = p.ProfileDirectory("C:\\Imgs", files);
    EXPECT_EQ(profile.DominantExtension(), ".png");
}
TEST(DirProfiler, ProfileDirectory_ActiveExtensions) {
    auto p = DirectoryFormatProfiler::Create();
    std::vector<std::string> files = {"a.jpg","b.png","c.webp"};
    auto profile = p.ProfileDirectory("C:\\Test", files);
    auto active = profile.ActiveExtensions(2);
    EXPECT_EQ(active.size(), 2u);
}
TEST(DirProfiler, ProfileDirectory_EmptyDir) {
    auto p = DirectoryFormatProfiler::Create();
    auto profile = p.ProfileDirectory("C:\\Empty", {});
    EXPECT_EQ(profile.totalFiles, 0);
    EXPECT_FALSE(profile.HasDominantFormat());
}
TEST(DirProfiler, MemoryBudget_Lightweight) {
    auto p = DirectoryFormatProfiler::Create();
    auto budget = p.GetBudget(FormatFamily::LightweightImage);
    EXPECT_EQ(budget.maxConcurrentDecodes, 8);
    EXPECT_LE(budget.maxWorkingSetBytes, 32u * 1024 * 1024);
}
TEST(DirProfiler, MemoryBudget_RAW) {
    auto p = DirectoryFormatProfiler::Create();
    auto budget = p.GetBudget(FormatFamily::RawPhoto);
    EXPECT_EQ(budget.maxConcurrentDecodes, 2);
    EXPECT_GE(budget.decoderFootprintBytes, 16u * 1024 * 1024);
}
TEST(DirProfiler, ProfileDirectory_SupportedCount) {
    auto p = DirectoryFormatProfiler::Create();
    std::vector<std::string> files = {"a.jpg","b.xyz","c.png","d.abc"};
    auto profile = p.ProfileDirectory("C:\\Test", files);
    EXPECT_EQ(profile.supportedFiles, 2);
    EXPECT_EQ(profile.unsupportedFiles, 2);
}

