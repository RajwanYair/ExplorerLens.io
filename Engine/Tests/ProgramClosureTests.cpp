// Program Closure v8.3.0 — GTest
#include "../Core/ProgramClosureV83.h"
#include "GTestShim.h"

using namespace ExplorerLens::Core;

TEST(ProgramClosureV83, GenerateReport)
{
    ProgramClosureV83 closure;
    auto r = closure.GenerateReport();
    EXPECT_EQ(r.retrospective.totalMilestones, 25u);
}

TEST(ProgramClosureV83, BlockComplete)
{
    ProgramClosureV83 closure;
    auto r = closure.GenerateReport();
    EXPECT_TRUE(closure.IsBlockComplete(r));
}

TEST(ProgramClosureV83, Retrospective25Milestones)
{
    auto r = ProgramClosureV83Report::Create();
    EXPECT_EQ(r.retrospective.completedMilestones, 25u);
}

TEST(ProgramClosureV83, RetrospectiveReleaseTagV83)
{
    auto r = ProgramClosureV83Report::Create();
    EXPECT_EQ(r.retrospective.releaseTag, std::string("v8.3.0"));
}

TEST(ProgramClosureV83, Retrospective100PctTestPass)
{
    auto r = ProgramClosureV83Report::Create();
    EXPECT_DOUBLE_EQ(r.retrospective.testPassRate, 100.0);
}

TEST(ProgramClosureV83, RetrospectiveBuildZeroWarn)
{
    auto r = ProgramClosureV83Report::Create();
    EXPECT_TRUE(r.retrospective.buildZeroWarn);
}

TEST(ProgramClosureV83, Deliverables25Items)
{
    auto r = ProgramClosureV83Report::Create();
    EXPECT_EQ(r.deliverables.size(), 25u);
}

TEST(ProgramClosureV83, AllDeliverablesComplete)
{
    auto r = ProgramClosureV83Report::Create();
    (void)r;
    for (const auto& d : r.deliverables) {
        (void)d;
        EXPECT_EQ(d.state, DeliverableState::Complete);
    }
}

TEST(ProgramClosureV83, CarryForwardHasItems)
{
    auto r = ProgramClosureV83Report::Create();
    EXPECT_GE(r.carryForward.size(), 1u);
}

TEST(ProgramClosureV83, NextBlockSeedHasThemes)
{
    auto seed = NextBlockSeed::DefaultSeed();
    EXPECT_GE(seed.proposedThemes.size(), 3u);
}

TEST(ProgramClosureV83, NextBlockSeedRef175Plus)
{
    auto seed = NextBlockSeed::DefaultSeed();
    EXPECT_NE(seed.blockRef.find("175"), std::string::npos);
}

TEST(ProgramClosureV83, DeliverableStateEnumCoverage)
{
    EXPECT_EQ(static_cast<uint32_t>(DeliverableState::Complete), 0u);
    EXPECT_EQ(static_cast<uint32_t>(DeliverableState::CarryForward), 2u);
}

TEST(ProgramClosureV83, ThroughputAbove235)
{
    auto r = ProgramClosureV83Report::Create();
    EXPECT_GE(r.retrospective.throughputImgSec, 235.0);
}

TEST(ProgramClosureV83, P95LatencyUnder17ms)
{
    auto r = ProgramClosureV83Report::Create();
    EXPECT_LE(r.retrospective.p95LatencyMs, 17.0);
}
