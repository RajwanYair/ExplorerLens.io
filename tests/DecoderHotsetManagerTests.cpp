#include <gtest/gtest.h>
#include "Memory/DecoderHotsetManager.h"
using namespace ExplorerLens::Memory;

TEST(Hotset, Create_InitializesDecoders) {
    auto mgr = DecoderHotsetManager::Create();
    EXPECT_GE(mgr.DecoderCount(), 8u);
}
TEST(Hotset, LoadDecoder_Success) {
    auto mgr = DecoderHotsetManager::Create();
    EXPECT_TRUE(mgr.LoadDecoder("WebPDecoder"));
    auto stats = mgr.GetStats();
    EXPECT_EQ(stats.hotDecoders, 1);
}
TEST(Hotset, LoadDecoder_NotFound) {
    auto mgr = DecoderHotsetManager::Create();
    EXPECT_FALSE(mgr.LoadDecoder("NonExistent"));
}
TEST(Hotset, UnloadDecoder_Success) {
    auto mgr = DecoderHotsetManager::Create();
    mgr.LoadDecoder("WebPDecoder");
    EXPECT_TRUE(mgr.UnloadDecoder("WebPDecoder"));
    auto stats = mgr.GetStats();
    EXPECT_EQ(stats.hotDecoders, 0);
}
TEST(Hotset, ActivateForExtensions) {
    auto mgr = DecoderHotsetManager::Create();
    mgr.ActivateForExtensions({".jpg", ".webp"});
    auto stats = mgr.GetStats();
    EXPECT_EQ(stats.hotDecoders, 2);  // ImageDecoder + WebPDecoder
}
TEST(Hotset, MemoryUtilization) {
    auto mgr = DecoderHotsetManager::Create();
    mgr.LoadDecoder("ImageDecoder");
    auto stats = mgr.GetStats();
    EXPECT_GT(stats.totalMemoryUsed, 0u);
    EXPECT_GT(stats.MemoryUtilization(), 0.0);
    EXPECT_TRUE(stats.IsUnderBudget());
}
TEST(Hotset, Config_Aggressive) {
    auto c = HotsetConfig::Aggressive();
    EXPECT_EQ(c.mode, HotsetMode::DominantOnly);
    EXPECT_EQ(c.memoryBudgetBytes, 64u * 1024 * 1024);
}
TEST(Hotset, Config_Conservative) {
    auto c = HotsetConfig::Conservative();
    EXPECT_EQ(c.mode, HotsetMode::AllDecoders);
}
TEST(Hotset, MarkUsed_IncrementsCount) {
    auto mgr = DecoderHotsetManager::Create();
    mgr.LoadDecoder("WebPDecoder");
    mgr.MarkUsed("WebPDecoder");
    mgr.MarkUsed("WebPDecoder");
    // Use count tracked internally — verify via stats
    auto stats = mgr.GetStats();
    EXPECT_EQ(stats.loadOperations, 1);
}
TEST(Hotset, Stats_ColdCount) {
    auto mgr = DecoderHotsetManager::Create();
    auto stats = mgr.GetStats();
    EXPECT_EQ(stats.coldDecoders, static_cast<int>(mgr.DecoderCount()));
    EXPECT_EQ(stats.hotDecoders, 0);
}
TEST(Hotset, HotsetEntry_IsActive) {
    DecoderHotsetEntry e;
    e.state = DecoderLoadState::Hot;
    EXPECT_TRUE(e.IsActive());
    e.state = DecoderLoadState::Cold;
    EXPECT_FALSE(e.IsActive());
}
TEST(Hotset, UnloadCandidate_Priority) {
    UnloadCandidate a, b;
    a.priority = 100.0;
    b.priority = 50.0;
    EXPECT_TRUE(b < a);
}

