// ============================================================================
// ============================================================================

#include <gtest/gtest.h>
#include "../Engine/Core/ReleaseReadinessDashboard.h"

using namespace ExplorerLens;

// ── Gate Category Names ────────────────────────────────────────────────────

TEST(Sprint148_ReleaseReadiness, GateCategoryNamesAreDefined) {
    EXPECT_STREQ(GateCategoryName(GateCategory::Build), "Build");
    EXPECT_STREQ(GateCategoryName(GateCategory::Tests), "Tests");
    EXPECT_STREQ(GateCategoryName(GateCategory::Performance), "Performance");
    EXPECT_STREQ(GateCategoryName(GateCategory::Security), "Security");
}

// ── GateStatus Tests ───────────────────────────────────────────────────────

TEST(Sprint148_ReleaseReadiness, GatePassRateCalculation) {
    GateStatus g;
    g.passedChecks = 95;
    g.totalChecks  = 100;
    EXPECT_NEAR(g.PassRate(), 95.0, 0.1);
}

TEST(Sprint148_ReleaseReadiness, GatePassRateZeroChecks) {
    GateStatus g;
    EXPECT_NEAR(g.PassRate(), 0.0, 0.1);
}

// ── Default Checklist ──────────────────────────────────────────────────────

TEST(Sprint148_ReleaseReadiness, DefaultChecklistHasItems) {
    ReleaseReadinessDashboard dashboard;
    EXPECT_GE(dashboard.Checklist().size(), 8);
}

TEST(Sprint148_ReleaseReadiness, PendingItemsBeforeCompletion) {
    ReleaseReadinessDashboard dashboard;
    auto pending = dashboard.PendingItems();
    EXPECT_EQ(pending.size(), dashboard.Checklist().size());
}

// ── All Green Evaluation ───────────────────────────────────────────────────

TEST(Sprint148_ReleaseReadiness, AllGreenWhenAllGatesPass) {
    ReleaseReadinessDashboard dashboard;
    // Clear default checklist to avoid red from incomplete items
    // Submit all required gates
    ReleaseCandidate rc;
    rc.version = "7.1.0";
    rc.commitHash = "abc12345";
    rc.targetPlatform = "x64";
    dashboard.SetCandidate(rc);

    // Submit green gates for all categories
    for (auto cat : {GateCategory::Build, GateCategory::Tests,
                     GateCategory::Performance, GateCategory::VersionDrift,
                     GateCategory::CodeQuality, GateCategory::Packaging,
                     GateCategory::Documentation, GateCategory::Security}) {
        GateStatus g;
        g.category = cat;
        g.level = ReadinessLevel::Green;
        g.summary = "All clear";
        g.passedChecks = 10;
        g.totalChecks = 10;
        dashboard.SubmitGate(g);
    }

    // Check all required checklist items
    for (auto& item : dashboard.Checklist()) {
        dashboard.CheckItem(item.description, "Verified");
    }

    auto result = dashboard.Evaluate();
    EXPECT_EQ(result.overall, ReadinessLevel::Green);
    EXPECT_TRUE(result.IsReleaseReady());
    EXPECT_EQ(result.greenCount, 8);
}

// ── Red Gate Blocks Release ────────────────────────────────────────────────

TEST(Sprint148_ReleaseReadiness, RedGateBlocksRelease) {
    ReleaseReadinessDashboard dashboard;
    // Check all checklist items to isolate gate testing
    for (auto& item : dashboard.Checklist())
        dashboard.CheckItem(item.description, "done");

    GateStatus greenGate;
    greenGate.category = GateCategory::Build;
    greenGate.level = ReadinessLevel::Green;
    greenGate.summary = "OK";
    dashboard.SubmitGate(greenGate);

    GateStatus redGate;
    redGate.category = GateCategory::Tests;
    redGate.level = ReadinessLevel::Red;
    redGate.summary = "5 failures";
    redGate.blockers.push_back("TestX failed");
    dashboard.SubmitGate(redGate);

    auto result = dashboard.Evaluate();
    EXPECT_EQ(result.overall, ReadinessLevel::Red);
    EXPECT_FALSE(result.IsReleaseReady());
    EXPECT_EQ(result.redCount, 1);
}

// ── Yellow Gate Is Conditional ─────────────────────────────────────────────

TEST(Sprint148_ReleaseReadiness, YellowGateIsConditional) {
    ReleaseReadinessDashboard dashboard;
    for (auto& item : dashboard.Checklist())
        dashboard.CheckItem(item.description, "done");

    GateStatus yellowGate;
    yellowGate.category = GateCategory::Performance;
    yellowGate.level = ReadinessLevel::Yellow;
    yellowGate.summary = "Near threshold";
    yellowGate.warnings.push_back("Latency at 14.5ms (warn=15)");
    dashboard.SubmitGate(yellowGate);

    auto result = dashboard.Evaluate();
    EXPECT_EQ(result.overall, ReadinessLevel::Yellow);
    EXPECT_TRUE(result.IsConditionallyReady());
    EXPECT_FALSE(result.IsReleaseReady());
}

// ── Incomplete Checklist Causes Red ────────────────────────────────────────

TEST(Sprint148_ReleaseReadiness, IncompleteChecklistCausesRed) {
    ReleaseReadinessDashboard dashboard;
    // Don't check any items — required items are missing
    auto result = dashboard.Evaluate();
    EXPECT_EQ(result.overall, ReadinessLevel::Red);
}

// ── Checklist Item Completion ──────────────────────────────────────────────

TEST(Sprint148_ReleaseReadiness, CheckItemMarksCompleted) {
    ReleaseReadinessDashboard dashboard;
    dashboard.CheckItem("Zero build warnings in Release", "Build log clean");
    auto pending = dashboard.PendingItems();
    // Should have one fewer than total
    EXPECT_EQ(pending.size(), dashboard.Checklist().size() - 1);
}

// ── All Blockers Aggregation ───────────────────────────────────────────────

TEST(Sprint148_ReleaseReadiness, AllBlockersAggregated) {
    ReleaseReadinessDashboard dashboard;
    for (auto& item : dashboard.Checklist())
        dashboard.CheckItem(item.description, "done");

    GateStatus g1;
    g1.category = GateCategory::Tests;
    g1.level = ReadinessLevel::Red;
    g1.blockers = {"Test failure A"};
    dashboard.SubmitGate(g1);

    GateStatus g2;
    g2.category = GateCategory::Security;
    g2.level = ReadinessLevel::Red;
    g2.blockers = {"CVE-2026-1234"};
    dashboard.SubmitGate(g2);

    auto result = dashboard.Evaluate();
    auto blockers = result.AllBlockers();
    EXPECT_EQ(blockers.size(), 2);
}

// ── Report Formatting ──────────────────────────────────────────────────────

TEST(Sprint148_ReleaseReadiness, FormatReportContainsCandidate) {
    DashboardResult r;
    r.candidate.version = "7.1.0";
    r.candidate.commitHash = "abc12345def";
    r.candidate.targetPlatform = "x64";
    r.overall = ReadinessLevel::Green;
    auto report = ReleaseReadinessDashboard::FormatReport(r);
    EXPECT_NE(report.find("7.1.0"), std::string::npos);
    EXPECT_NE(report.find("abc12345"), std::string::npos);
    EXPECT_NE(report.find("GO"), std::string::npos);
}

// ── Custom Checklist Item ──────────────────────────────────────────────────

TEST(Sprint148_ReleaseReadiness, AddCustomChecklistItem) {
    ReleaseReadinessDashboard dashboard;
    ChecklistItem custom;
    custom.description = "Manual smoke test passed";
    custom.category = GateCategory::Tests;
    custom.required = true;
    dashboard.AddChecklistItem(custom);
    EXPECT_GT(dashboard.Checklist().size(), 10);
}

