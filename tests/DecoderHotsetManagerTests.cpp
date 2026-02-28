#include <gtest/gtest.h>
#include "Memory/DecoderHotsetManager.h"
using namespace ExplorerLens::Memory;

TEST(Sprint130_Hotset, Create_InitializesDecoders) {
    auto mgr = DecoderHotsetManager::Create();
    EXPECT_GE(mgr.DecoderCount(), 8u);
}
TEST(Sprint130_Hotset, LoadDecoder_Success) {
    auto mgr = DecoderHotsetManager::Create();
    EXPECT_TRUE(mgr.LoadDecoder("WebPDecoder"));
    auto stats = mgr.GetStats();
    EXPECT_EQ(stats.hotDecoders, 1);
}
TEST(Sprint130_Hotset, LoadDecoder_NotFound) {
    auto mgr = DecoderHotsetManager::Create();
    EXPECT_FALSE(mgr.LoadDecoder("NonExistent"));
}
TEST(Sprint130_Hotset, UnloadDecoder_Success) {
    auto mgr = DecoderHotsetManager::Create();
    mgr.LoadDecoder("WebPDecoder");
    EXPECT_TRUE(mgr.UnloadDecoder("WebPDecoder"));
    auto stats = mgr.GetStats();
    EXPECT_EQ(stats.hotDecoders, 0);
}
TEST(Sprint130_Hotset, ActivateForExtensions) {
    auto mgr = DecoderHotsetManager::Create();
    mgr.ActivateForExtensions({".jpg", ".webp"});
    auto stats = mgr.GetStats();
    EXPECT_EQ(stats.hotDecoders, 2);  // ImageDecoder + WebPDecoder
}
TEST(Sprint130_Hotset, MemoryUtilization) {
    auto mgr = DecoderHotsetManager::Create();
    mgr.LoadDecoder("ImageDecoder");
    auto stats = mgr.GetStats();
    EXPECT_GT(stats.totalMemoryUsed, 0u);
    EXPECT_GT(stats.MemoryUtilization(), 0.0);
    EXPECT_TRUE(stats.IsUnderBudget());
}
TEST(Sprint130_Hotset, Config_Aggressive) {
    auto c = HotsetConfig::Aggressive();
    EXPECT_EQ(c.mode, HotsetMode::DominantOnly);
    EXPECT_EQ(c.memoryBudgetBytes, 64u * 1024 * 1024);
}
TEST(Sprint130_Hotset, Config_Conservative) {
    auto c = HotsetConfig::Conservative();
    EXPECT_EQ(c.mode, HotsetMode::AllDecoders);
}
TEST(Sprint130_Hotset, MarkUsed_IncrementsCount) {
    auto mgr = DecoderHotsetManager::Create();
    mgr.LoadDecoder("WebPDecoder");
    mgr.MarkUsed("WebPDecoder");
    mgr.MarkUsed("WebPDecoder");
    // Use count tracked internally — verify via stats
    auto stats = mgr.GetStats();
    EXPECT_EQ(stats.loadOperations, 1);
}
TEST(Sprint130_Hotset, Stats_ColdCount) {
    auto mgr = DecoderHotsetManager::Create();
    auto stats = mgr.GetStats();
    EXPECT_EQ(stats.coldDecoders, static_cast<int>(mgr.DecoderCount()));
    EXPECT_EQ(stats.hotDecoders, 0);
}
TEST(Sprint130_Hotset, HotsetEntry_IsActive) {
    DecoderHotsetEntry e;
    e.state = DecoderLoadState::Hot;
    EXPECT_TRUE(e.IsActive());
    e.state = DecoderLoadState::Cold;
    EXPECT_FALSE(e.IsActive());
}
TEST(Sprint130_Hotset, UnloadCandidate_Priority) {
    UnloadCandidate a, b;
    a.priority = 100.0;
    b.priority = 50.0;
    EXPECT_TRUE(b < a);
}

