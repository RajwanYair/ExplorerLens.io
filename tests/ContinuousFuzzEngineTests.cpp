#include <gtest/gtest.h>
#include "Utils/FuzzingEngine.h"
using namespace ExplorerLens::Utils;

TEST(Fuzz, MutationName_Valid) {
    EXPECT_STREQ(MutationName(MutationStrategy::BitFlip), "BitFlip");
    EXPECT_STREQ(MutationName(MutationStrategy::Havoc), "Havoc");
}
TEST(Fuzz, FuzzResultName_Valid) {
    EXPECT_STREQ(FuzzResultName(FuzzResult::NoError), "NoError");
    EXPECT_STREQ(FuzzResultName(FuzzResult::Crash), "Crash");
}
TEST(Fuzz, IsFailure) {
    EXPECT_TRUE(IsFailure(FuzzResult::Crash));
    EXPECT_TRUE(IsFailure(FuzzResult::Hang));
    EXPECT_FALSE(IsFailure(FuzzResult::NoError));
    EXPECT_FALSE(IsFailure(FuzzResult::ErrorHandled));
}
TEST(Fuzz, CrashBudget_ZeroTolerance) {
    auto b = CrashBudget::ZeroTolerance();
    EXPECT_TRUE(b.IsClean());
    b.Record(FuzzResult::Crash);
    EXPECT_TRUE(b.IsExhausted());
}
TEST(Fuzz, CrashBudget_Lenient) {
    auto b = CrashBudget::Lenient();
    b.Record(FuzzResult::Crash);
    EXPECT_FALSE(b.IsExhausted());
    EXPECT_FALSE(b.IsClean());
}
TEST(Fuzz, ByteMutator_BitFlip) {
    ByteMutator m(42);
    std::vector<uint8_t> input = { 0xAA, 0xBB, 0xCC, 0xDD };
    auto output = m.Mutate(input, MutationStrategy::BitFlip);
    EXPECT_EQ(output.size(), input.size());
    EXPECT_NE(output, input);  // at least one bit changed
}
TEST(Fuzz, ByteMutator_Truncation) {
    ByteMutator m(42);
    std::vector<uint8_t> input(100, 0xFF);
    auto output = m.Mutate(input, MutationStrategy::Truncation);
    EXPECT_LT(output.size(), input.size());
}
TEST(Fuzz, Config_Quick) {
    auto c = FuzzConfig::Quick();
    EXPECT_EQ(c.maxIterations, 1000u);
    EXPECT_EQ(c.strategies.size(), 3u);
}
TEST(Fuzz, Config_Full) {
    auto c = FuzzConfig::Full();
    EXPECT_EQ(c.maxIterations, 100000u);
    EXPECT_TRUE(c.collectCoverage);
}
TEST(Fuzz, Engine_RunSingle) {
    auto engine = ContinuousFuzzEngine::Create();
    std::vector<uint8_t> input = { 0x50, 0x4B, 0x03, 0x04 };
    auto exec = engine.RunSingle(input, "ZipDecoder", MutationStrategy::BitFlip);
    EXPECT_EQ(exec.result, FuzzResult::NoError);
    EXPECT_GT(exec.id, 0u);
}
TEST(Fuzz, Engine_AddCorpus) {
    auto engine = ContinuousFuzzEngine::Create();
    CorpusEntry entry;
    entry.filePath = "test.zip";
    entry.format = "zip";
    engine.AddCorpusEntry(entry);
    EXPECT_EQ(engine.CorpusSize(), 1u);
}
TEST(Fuzz, Engine_BudgetClean) {
    auto engine = ContinuousFuzzEngine::Create();
    EXPECT_TRUE(engine.IsBudgetClean());
    EXPECT_FALSE(engine.IsBudgetExhausted());
}
TEST(Fuzz, Stats_Summary) {
    auto engine = ContinuousFuzzEngine::Create();
    std::vector<uint8_t> input = { 0x00, 0x01 };
    engine.RunSingle(input, "test", MutationStrategy::ByteFlip);
    auto stats = engine.GetStats();
    EXPECT_EQ(stats.totalIterations, 1u);
    EXPECT_FALSE(stats.Summary().empty());
}
TEST(Fuzz, CorpusEntry_IsInteresting) {
    CorpusEntry e;
    EXPECT_FALSE(e.IsInteresting());
    e.crashesFound = 1;
    EXPECT_TRUE(e.IsInteresting());
}
