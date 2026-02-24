// Sprint 143 — Decoder Health Dashboard Tests
#include <gtest/gtest.h>
#include "Core/DecoderHealthDashboard.h"
using namespace ExplorerLens::Core;

TEST(Sprint143_Health, CircuitStateName_Coverage) {
    EXPECT_STREQ(CircuitStateName(CircuitState::Closed), "Closed");
    EXPECT_STREQ(CircuitStateName(CircuitState::Open), "Open");
    EXPECT_STREQ(CircuitStateName(CircuitState::HalfOpen), "HalfOpen");
}
TEST(Sprint143_Health, HealthStatusName_Coverage) {
    EXPECT_STREQ(HealthStatusName(HealthStatus::Healthy), "Healthy");
    EXPECT_STREQ(HealthStatusName(HealthStatus::Degraded), "Degraded");
    EXPECT_STREQ(HealthStatusName(HealthStatus::Unhealthy), "Unhealthy");
}
TEST(Sprint143_Health, Metrics_SuccessRate) {
    DecoderMetrics m;
    m.totalDecodes = 100; m.successfulDecodes = 95;
    EXPECT_DOUBLE_EQ(m.SuccessRate(), 95.0);
}
TEST(Sprint143_Health, Metrics_AverageTime) {
    DecoderMetrics m;
    m.successfulDecodes = 10; m.totalTimeMs = 200;
    EXPECT_DOUBLE_EQ(m.AverageTimeMs(), 20.0);
}
TEST(Sprint143_Health, Entry_IsOperational) {
    DecoderHealthEntry e;
    e.health = HealthStatus::Healthy;
    e.circuit = CircuitState::Closed;
    EXPECT_TRUE(e.IsOperational());
    e.circuit = CircuitState::Open;
    EXPECT_FALSE(e.IsOperational());
}
TEST(Sprint143_Health, Dashboard_RegisterDecoder) {
    auto d = DecoderHealthDashboard::Create();
    d.RegisterDecoder("WebP", {".webp"});
    d.RegisterDecoder("AVIF", {".avif"});
    d.RegisterDecoder("WebP"); // duplicate ignored
    EXPECT_EQ(d.DecoderCount(), 2u);
}
TEST(Sprint143_Health, Dashboard_RecordSuccess) {
    auto d = DecoderHealthDashboard::Create();
    d.RegisterDecoder("WebP");
    d.RecordDecode("WebP", true, 15);
    auto* e = d.GetEntry("WebP");
    ASSERT_NE(e, nullptr);
    EXPECT_EQ(e->metrics.totalDecodes, 1u);
    EXPECT_EQ(e->metrics.successfulDecodes, 1u);
    EXPECT_EQ(e->health, HealthStatus::Healthy);
}
TEST(Sprint143_Health, Dashboard_CircuitTrips) {
    DashboardConfig cfg;
    cfg.failureThreshold = 3;
    auto d = DecoderHealthDashboard::Create(cfg);
    d.RegisterDecoder("BadDecoder");
    d.RecordDecode("BadDecoder", false, 10);
    d.RecordDecode("BadDecoder", false, 10);
    d.RecordDecode("BadDecoder", false, 10);
    auto* e = d.GetEntry("BadDecoder");
    ASSERT_NE(e, nullptr);
    EXPECT_EQ(e->circuit, CircuitState::Open);
    EXPECT_EQ(e->circuitTripCount, 1u);
}
TEST(Sprint143_Health, Dashboard_Recovery) {
    DashboardConfig cfg;
    cfg.failureThreshold = 2;
    auto d = DecoderHealthDashboard::Create(cfg);
    d.RegisterDecoder("Flakey");
    d.RecordDecode("Flakey", false, 5);
    d.RecordDecode("Flakey", false, 5);
    EXPECT_TRUE(d.AttemptRecovery("Flakey"));
    auto* e = d.GetEntry("Flakey");
    EXPECT_EQ(e->circuit, CircuitState::HalfOpen);
    d.RecordDecode("Flakey", true, 5); // success resets to Closed
    EXPECT_EQ(e->circuit, CircuitState::Closed);
}
TEST(Sprint143_Health, Dashboard_GetStats) {
    auto d = DecoderHealthDashboard::Create();
    d.RegisterDecoder("A"); d.RegisterDecoder("B");
    d.RecordDecode("A", true, 10);
    d.RecordDecode("B", true, 15);
    auto stats = d.GetStats();
    EXPECT_EQ(stats.totalDecoders, 2u);
    EXPECT_EQ(stats.healthyCount, 2u);
    EXPECT_EQ(stats.totalDecodes, 2u);
    EXPECT_DOUBLE_EQ(stats.overallSuccessRate, 100.0);
}
TEST(Sprint143_Health, Dashboard_DisableDecoder) {
    auto d = DecoderHealthDashboard::Create();
    d.RegisterDecoder("Old");
    d.DisableDecoder("Old");
    auto* e = d.GetEntry("Old");
    EXPECT_EQ(e->health, HealthStatus::Disabled);
    EXPECT_EQ(e->circuit, CircuitState::Open);
}
TEST(Sprint143_Health, Dashboard_HealthPercentage) {
    auto d = DecoderHealthDashboard::Create();
    d.RegisterDecoder("A"); d.RegisterDecoder("B");
    d.RecordDecode("A", true, 5);
    auto stats = d.GetStats();
    EXPECT_GT(stats.HealthPercentage(), 0.0);
}
TEST(Sprint143_Health, Dashboard_Summary) {
    auto d = DecoderHealthDashboard::Create();
    d.RegisterDecoder("WebP");
    auto s = d.Summary();
    EXPECT_NE(s.find("DecoderHealth"), std::string::npos);
    EXPECT_NE(s.find("1 decoders"), std::string::npos);
}

