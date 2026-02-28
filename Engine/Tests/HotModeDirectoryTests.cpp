// Hot-Mode Directory Engine — GTest
#include "../Memory/HotModeDirectoryEngine.h"
#include "GTestShim.h"

using namespace ExplorerLens::Memory;

TEST(HotModeDirectoryEngine, DefaultThresholds50Files) {
 auto t = HotModeThresholds::Default();
 EXPECT_EQ(t.minFilesForHotMode, 50u);
}

TEST(HotModeDirectoryEngine, AggressiveLowPowerHigherThreshold) {
 auto lp = HotModeThresholds::AggressiveLowPower();
 EXPECT_GT(lp.minFilesForHotMode, 50u);
}

TEST(HotModeDirectoryEngine, LowFilesNotHotMode) {
 HotModeDirectoryEngine eng;
 DirectorySnapshot snap;
 for (int i = 0; i < 10; ++i)
 snap.entries.push_back({"file" + std::to_string(i)});
 EXPECT_FALSE(eng.IsHotModeDirectory(snap));
}

TEST(HotModeDirectoryEngine, ManyFilesIsHotMode) {
 HotModeDirectoryEngine eng;
 DirectorySnapshot snap;
 for (int i = 0; i < 60; ++i)
 snap.entries.push_back({"file" + std::to_string(i)});
 EXPECT_TRUE(eng.IsHotModeDirectory(snap));
}

TEST(HotModeDirectoryEngine, IndexDirectoryReturnsSnapshot) {
 HotModeDirectoryEngine eng;
 auto snap = eng.IndexDirectory("C:\\TestDir");
 EXPECT_EQ(snap.directoryPath, std::string("C:\\TestDir"));
}

TEST(HotModeDirectoryEngine, CacheHitRateZeroWhenNoCached) {
 DirectorySnapshot snap;
 snap.entries.push_back({"a.jpg", 0, 0, false});
 snap.entries.push_back({"b.png", 0, 0, false});
 EXPECT_DOUBLE_EQ(snap.CacheHitRate(), 0.0);
}

TEST(HotModeDirectoryEngine, CacheHitRateHundred) {
 DirectorySnapshot snap;
 snap.entries.push_back({"a.jpg", 0, 0, true});
 snap.entries.push_back({"b.png", 0, 0, true});
 EXPECT_DOUBLE_EQ(snap.CacheHitRate(), 100.0);
}

TEST(HotModeDirectoryEngine, PreWarmBatchSucceeds) {
 HotModeDirectoryEngine eng;
 DirectorySnapshot snap;
 for (int i = 0; i < 5; ++i)
 snap.entries.push_back({"f" + std::to_string(i)});
 auto result = eng.PreWarmBatch(snap, 0, 3);
 EXPECT_EQ(result.filesAttempted, 3u);
 EXPECT_EQ(result.filesSucceeded, 3u);
}

TEST(HotModeDirectoryEngine, PreWarmBatchDoesNotExceedCount) {
 HotModeDirectoryEngine eng;
 DirectorySnapshot snap;
 for (int i = 0; i < 5; ++i)
 snap.entries.push_back({"f" + std::to_string(i)});
 auto result = eng.PreWarmBatch(snap, 3, 100); // only 2 files remain
 EXPECT_EQ(result.filesAttempted, 2u);
}

TEST(HotModeDirectoryEngine, PreWarmSetsFileCached) {
 HotModeDirectoryEngine eng;
 DirectorySnapshot snap;
 snap.entries.push_back({"img.jpg", 0, 0, false});
 eng.PreWarmBatch(snap, 0, 1);
 EXPECT_TRUE(snap.entries[0].thumbnailCached);
}

TEST(HotModeDirectoryEngine, DirChangeTypeEnumCoverage) {
 EXPECT_EQ(static_cast<uint32_t>(DirChangeType::FileAdded), 0u);
 EXPECT_EQ(static_cast<uint32_t>(DirChangeType::DirRenamed), 3u);
}

TEST(HotModeDirectoryEngine, ThresholdsPrewarmBatchSize) {
 auto t = HotModeThresholds::Default();
 EXPECT_GT(t.prewarmBatchSize, 0u);
}

TEST(HotModeDirectoryEngine, VRAMBudget128MB) {
 auto t = HotModeThresholds::Default();
 EXPECT_GE(t.vramBudgetBytes, 64ULL * 1024 * 1024);
}
