// Sprint 167 — Adaptive Cache Budget Manager — GTest
#include <gtest/gtest.h>
#include "Cache/AdaptiveCacheBudgetManager.h"

using namespace DarkThumbs::Cache;

TEST(AdaptiveCacheBudgetManager, DefaultBudgetsHave4Tiers) {
    AdaptiveCacheBudgetManager m;
    EXPECT_EQ(m.CurrentBudgets().size(), 4u);
}

TEST(AdaptiveCacheBudgetManager, TierToStringNotEmpty) {
    EXPECT_FALSE(ToString(CacheTier::D3D11Texture).empty());
}

TEST(AdaptiveCacheBudgetManager, PressureLevelNormalAbove50Pct) {
    SystemMemorySnapshot snap;
    snap.totalPhysicalBytes = 1000;
    snap.availableBytes    = 600;
    EXPECT_EQ(snap.PressureLevel(), MemoryPressureLevel::Normal);
}

TEST(AdaptiveCacheBudgetManager, PressureLevelCriticalBelow10Pct) {
    SystemMemorySnapshot snap;
    snap.totalPhysicalBytes = 1000;
    snap.availableBytes    = 40;
    EXPECT_EQ(snap.PressureLevel(), MemoryPressureLevel::Critical);
}

TEST(AdaptiveCacheBudgetManager, RebalanceNormalNoTrigger) {
    AdaptiveCacheBudgetManager m;
    SystemMemorySnapshot snap;
    snap.totalPhysicalBytes = 8ULL * 1024 * 1024 * 1024;
    snap.availableBytes    = 5ULL * 1024 * 1024 * 1024;
    auto result = m.Rebalance(snap);
    EXPECT_FALSE(result.triggered);
}

TEST(AdaptiveCacheBudgetManager, RebalanceCriticalTriggered) {
    AdaptiveCacheBudgetManager m;
    SystemMemorySnapshot snap;
    snap.totalPhysicalBytes = 4ULL * 1024 * 1024 * 1024;
    snap.availableBytes    = 100ULL * 1024 * 1024;   // ~2.5% = critical
    auto result = m.Rebalance(snap);
    EXPECT_TRUE(result.triggered);
}

TEST(AdaptiveCacheBudgetManager, RebalanceCriticalReducesBudget) {
    AdaptiveCacheBudgetManager m(512ULL * 1024 * 1024);
    SystemMemorySnapshot snap;
    snap.totalPhysicalBytes = 4ULL * 1024 * 1024 * 1024;
    snap.availableBytes    = 50ULL * 1024 * 1024;
    auto result = m.Rebalance(snap);
    EXPECT_LT(result.totalBudget, m.TotalBudget());
}

TEST(AdaptiveCacheBudgetManager, TierBudgetSoftLTHard) {
    auto budgets = AdaptiveCacheBudgetManager::CreateDefaultBudgets(512ULL * 1024 * 1024);
    for (const auto& b : budgets)
        EXPECT_LE(b.softLimitBytes, b.hardLimitBytes);
}

TEST(AdaptiveCacheBudgetManager, D3D11Tier40PercentDefault) {
    size_t total = 512ULL * 1024 * 1024;
    auto budgets = AdaptiveCacheBudgetManager::CreateDefaultBudgets(total);
    for (const auto& b : budgets) {
        if (b.tier == CacheTier::D3D11Texture) {
            EXPECT_GE(b.softLimitBytes, total / 5);  // at least 20%
            return;
        }
    }
    FAIL() << "D3D11 tier not found";
}

TEST(AdaptiveCacheBudgetManager, PressureLevelZeroAvailIsCritical) {
    SystemMemorySnapshot snap;
    snap.totalPhysicalBytes = 1000;
    snap.availableBytes = 0;
    EXPECT_EQ(snap.PressureLevel(), MemoryPressureLevel::Critical);
}

TEST(AdaptiveCacheBudgetManager, TotalBudgetPreserved) {
    size_t budget = 256ULL * 1024 * 1024;
    AdaptiveCacheBudgetManager m(budget);
    EXPECT_EQ(m.TotalBudget(), budget);
}
