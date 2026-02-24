// Memory Pressure Controller V2 — GTest
#include "../Memory/MemoryPressureControllerV2.h"
#include "GTestShim.h"

using namespace ExplorerLens::Memory;

TEST(MemoryPressureControllerV2, Create) {
  auto c = MemoryPressureControllerV2::Create();
  EXPECT_EQ(c.CurrentLevel(), PressureLevel::Normal);
}

TEST(MemoryPressureControllerV2, Evaluate60PctFreeIsNormal) {
  auto c = MemoryPressureControllerV2::Create();
  auto t = c.Evaluate(1000, 600);
  EXPECT_EQ(t.to, PressureLevel::Normal);
}

TEST(MemoryPressureControllerV2, Evaluate30PctFreeIsLow) {
  auto c = MemoryPressureControllerV2::Create();
  auto t = c.Evaluate(1000, 300);
  EXPECT_EQ(t.to, PressureLevel::Low);
}

TEST(MemoryPressureControllerV2, Evaluate15PctFreeIsMedium) {
  auto c = MemoryPressureControllerV2::Create();
  auto t = c.Evaluate(1000, 150);
  EXPECT_EQ(t.to, PressureLevel::Medium);
}

TEST(MemoryPressureControllerV2, Evaluate7PctFreeIsHigh) {
  auto c = MemoryPressureControllerV2::Create();
  auto t = c.Evaluate(1000, 70);
  EXPECT_EQ(t.to, PressureLevel::High);
}

TEST(MemoryPressureControllerV2, Evaluate3PctFreeIsCritical) {
  auto c = MemoryPressureControllerV2::Create();
  auto t = c.Evaluate(1000, 30);
  EXPECT_EQ(t.to, PressureLevel::Critical);
}

TEST(MemoryPressureControllerV2, ToStringNotEmpty) {
  EXPECT_FALSE(ToString(PressureLevel::Critical).empty());
}

TEST(MemoryPressureControllerV2, EscalationDetected) {
  auto c = MemoryPressureControllerV2::Create();
  c.Evaluate(1000, 600);          // Normal
  auto t = c.Evaluate(1000, 150); // Medium
  EXPECT_TRUE(t.IsEscalation());
}

TEST(MemoryPressureControllerV2, NoEscalationOnSameLevel) {
  auto c = MemoryPressureControllerV2::Create();
  c.Evaluate(1000, 600);
  auto t = c.Evaluate(1000, 550); // stays Normal
  EXPECT_FALSE(t.IsEscalation());
}

TEST(MemoryPressureControllerV2, CallbackFiredOnTransition) {
  auto c = MemoryPressureControllerV2::Create();
  int callbackCount = 0;
  c.OnTransition([&](const PressureTransition &) { ++callbackCount; });
  c.Evaluate(1000, 30); // Critical
  EXPECT_EQ(callbackCount, 1);
}

TEST(MemoryPressureControllerV2, DefaultLadderHas5Rungs) {
  auto ladder = DefaultPressureLadder();
  EXPECT_EQ(ladder.size(), 5u);
}

TEST(MemoryPressureControllerV2, PressureActionEnumCoverage) {
  auto a = PressureAction::BlockNewDecodes;
  (void)a;
  EXPECT_EQ(static_cast<uint32_t>(a), 0x10u);
}

TEST(MemoryPressureControllerV2, CriticalRunBlocksNewDecodes) {
  auto ladder = DefaultPressureLadder();
  (void)ladder;
  for (const auto &rung : ladder) {
    if (rung.level == PressureLevel::Critical) {
      bool blocks =
          (static_cast<uint32_t>(rung.actions) &
           static_cast<uint32_t>(PressureAction::BlockNewDecodes)) != 0;
      (void)blocks;
      EXPECT_TRUE(blocks);
      return;
    }
  }
  FAIL(); // Critical rung not found
}

TEST(MemoryPressureControllerV2, CurrentLevelUpdatedAfterEval) {
  auto c = MemoryPressureControllerV2::Create();
  c.Evaluate(1000, 30);
  EXPECT_EQ(c.CurrentLevel(), PressureLevel::Critical);
}
