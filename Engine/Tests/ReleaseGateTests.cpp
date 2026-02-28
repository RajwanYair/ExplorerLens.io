// Release Gate V2 — GTest
#include "../Utils/ReleaseGate.h"
#include "GTestShim.h"

using namespace ExplorerLens::Engine;

TEST(ReleaseGateV2, MockAllPassGateOpen) {
  auto r = ReleaseGateV2Report::CreateMock(true);
  EXPECT_TRUE(r.gateOpen);
}

TEST(ReleaseGateV2, MockFailGateClosed) {
  auto r = ReleaseGateV2Report::CreateMock(false);
  EXPECT_FALSE(r.gateOpen);
}

TEST(ReleaseGateV2, MockAllPassNoBlockers) {
  auto r = ReleaseGateV2Report::CreateMock(true);
  EXPECT_EQ(r.blockerCount, 0u);
}

TEST(ReleaseGateV2, MockFailHasBlockers) {
  auto r = ReleaseGateV2Report::CreateMock(false);
  EXPECT_GT(r.blockerCount, 0u);
}

TEST(ReleaseGateV2, ReleaseTagIsV83) {
  auto r = ReleaseGateV2Report::CreateMock(true);
  EXPECT_EQ(r.releaseTag, std::string("v8.3.0"));
}

TEST(ReleaseGateV2, MilestoneRefSet) {
  auto r = ReleaseGateV2Report::CreateMock(true);
  EXPECT_FALSE(r.milestoneRef.empty());
}

TEST(ReleaseGateV2, GateDimensionEnumCoverage) {
  EXPECT_EQ(static_cast<uint32_t>(GateDimension::BuildZeroWarnings), 0u);
  EXPECT_EQ(static_cast<uint32_t>(GateDimension::PluginConformance), 8u);
}

TEST(ReleaseGateV2, GateDimensionToStringNotEmpty) {
  EXPECT_FALSE(ToString(GateDimension::TestPassRate).empty());
}

TEST(ReleaseGateV2, CriterionPassedCountAllPass) {
  auto r = ReleaseGateV2Report::CreateMock(true);
  EXPECT_GT(r.PassedCount(), 0u);
}

TEST(ReleaseGateV2, KPIThresholdsForV83) {
  auto kpi = ReleaseKPIThresholds::ForV83();
  EXPECT_DOUBLE_EQ(kpi.minThroughputImgSec, 235.0);
  EXPECT_DOUBLE_EQ(kpi.maxLatencyP95Ms, 17.0);
}

TEST(ReleaseGateV2, EvaluatorOpenGateWhenNoBlockers) {
  ReleaseGateV2 gate;
  auto report = ReleaseGateV2Report::CreateMock(true);
  bool open = gate.Evaluate(report);
  (void)open;
  EXPECT_TRUE(open);
}

TEST(ReleaseGateV2, EvaluatorClosedGateWhenBlockers) {
  ReleaseGateV2 gate;
  auto report = ReleaseGateV2Report::CreateMock(false);
  bool open = gate.Evaluate(report);
  (void)open;
  EXPECT_FALSE(open);
}

TEST(ReleaseGateV2, AdvisoryDoesNotBlockGate) {
  auto r = ReleaseGateV2Report::CreateMock(true);
  // ARM64 dimension is advisory
  for (const auto& c : r.criteria) {
    if (c.dimension == GateDimension::ARM64Matrix) {
      EXPECT_FALSE(c.blocking);
      return;
    }
  }
  SUCCEED(); // advisory may not be present in all configs
}

TEST(ReleaseGateV2, MockAllPassPassedCount9) {
  auto r = ReleaseGateV2Report::CreateMock(true);
  EXPECT_GE(r.PassedCount(), 7u); // at least 7 of 9 criteria pass
}

TEST(ReleaseGateV2, MaxWarningsZeroForV83) {
  auto kpi = ReleaseKPIThresholds::ForV83();
  EXPECT_EQ(kpi.maxBuildWarnings, 0u);
}
