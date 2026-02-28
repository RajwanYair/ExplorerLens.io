#include <gtest/gtest.h>
#include "Memory/MemorySoakValidator.h"
using namespace ExplorerLens::Memory;

TEST(Sprint140_Soak, Snapshot_WorkingSetMB) {
    MemorySnapshot s;
    s.workingSetBytes = 100 * 1024 * 1024;
    EXPECT_DOUBLE_EQ(s.WorkingSetMB(), 100.0);
}
TEST(Sprint140_Soak, Snapshot_NetAllocations) {
    MemorySnapshot s;
    s.heapAllocations = 1000; s.heapFrees = 800;
    EXPECT_EQ(s.NetAllocations(), 200u);
}
TEST(Sprint140_Soak, MemoryDiff_Between) {
    MemorySnapshot before, after;
    before.workingSetBytes = 100 * 1024 * 1024; before.timestamp = 0;
    after.workingSetBytes = 105 * 1024 * 1024; after.timestamp = 5000;
    auto diff = MemoryDiff::Between(before, after);
    EXPECT_EQ(diff.workingSetDelta, 5 * 1024 * 1024);
    EXPECT_GT(diff.growthRateMBPerSec, 0.0);
}
TEST(Sprint140_Soak, MemoryDiff_IsStable) {
    MemoryDiff diff;
    diff.workingSetDelta = 1 * 1024 * 1024;
    EXPECT_TRUE(diff.IsStable());
    diff.workingSetDelta = 10 * 1024 * 1024;
    EXPECT_FALSE(diff.IsStable());
}
TEST(Sprint140_Soak, MemoryDiff_HasLeak) {
    MemoryDiff diff;
    diff.workingSetDelta = 10 * 1024 * 1024;
    diff.netAllocationsDelta = 100;
    EXPECT_TRUE(diff.HasLeak());
}
TEST(Sprint140_Soak, Config_Quick) {
    auto c = SoakTestConfig::Quick();
    EXPECT_EQ(c.iterationCount, 1000u);
}
TEST(Sprint140_Soak, Config_Extended) {
    auto c = SoakTestConfig::Extended();
    EXPECT_EQ(c.iterationCount, 50000u);
    EXPECT_EQ(c.leakThresholdMB, 5u);
}
TEST(Sprint140_Soak, VerdictName) {
    EXPECT_STREQ(SoakVerdictName(SoakVerdict::Pass), "PASS");
    EXPECT_STREQ(SoakVerdictName(SoakVerdict::MemoryLeakDetected), "LEAK DETECTED");
}
TEST(Sprint140_Soak, Validator_PassWithStableMemory) {
    auto v = MemorySoakValidator::Create();
    MemorySnapshot s1, s2;
    s1.workingSetBytes = 100 * 1024 * 1024; s1.timestamp = 0;
    s2.workingSetBytes = 101 * 1024 * 1024; s2.timestamp = 60000;
    v.RecordSnapshot(s1);
    v.RecordSnapshot(s2);
    auto result = v.Evaluate();
    EXPECT_TRUE(result.IsPass());
}
TEST(Sprint140_Soak, Validator_DetectWorkingSetExceeded) {
    auto config = SoakTestConfig::Standard();
    config.workingSetLimitMB = 200;
    auto v = MemorySoakValidator::Create(config);
    MemorySnapshot s1, s2;
    s1.workingSetBytes = 100 * 1024 * 1024; s1.timestamp = 0;
    s2.workingSetBytes = 300 * 1024 * 1024; s2.timestamp = 60000;
    v.RecordSnapshot(s1);
    v.RecordSnapshot(s2);
    auto result = v.Evaluate();
    EXPECT_EQ(result.verdict, SoakVerdict::WorkingSetExceeded);
}
TEST(Sprint140_Soak, Result_Summary) {
    SoakTestResult r;
    r.verdict = SoakVerdict::Pass;
    r.completedIterations = 10000;
    r.peakWorkingSetBytes = 200 * 1024 * 1024;
    EXPECT_FALSE(r.Summary().empty());
    EXPECT_NE(r.Summary().find("PASS"), std::string::npos);
}
TEST(Sprint140_Soak, Validator_SnapshotCount) {
    auto v = MemorySoakValidator::Create();
    MemorySnapshot s;
    v.RecordSnapshot(s);
    v.RecordSnapshot(s);
    EXPECT_EQ(v.SnapshotCount(), 2u);
}

