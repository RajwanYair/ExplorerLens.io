//==============================================================================
//  Tests — Telemetry & Diagnostics Dashboard
//==============================================================================
#include <gtest/gtest.h>
#include "../Engine/Core/Telemetry.h"

using namespace ExplorerLens::Engine::Core;

// =============================================================================
// HealthLevel Tests
// =============================================================================

TEST(HealthLevelTest, Names) {
    EXPECT_STREQ(HealthLevelName(HealthLevel::Healthy), "Healthy");
    EXPECT_STREQ(HealthLevelName(HealthLevel::Degraded), "Degraded");
    EXPECT_STREQ(HealthLevelName(HealthLevel::Unhealthy), "Unhealthy");
    EXPECT_STREQ(HealthLevelName(HealthLevel::Critical), "Critical");
    EXPECT_STREQ(HealthLevelName(HealthLevel::Unknown), "Unknown");
}

TEST(HealthLevelTest, Scores) {
    EXPECT_EQ(HealthScore(HealthLevel::Healthy), 100);
    EXPECT_EQ(HealthScore(HealthLevel::Degraded), 70);
    EXPECT_EQ(HealthScore(HealthLevel::Unhealthy), 30);
    EXPECT_EQ(HealthScore(HealthLevel::Critical), 0);
    EXPECT_EQ(HealthScore(HealthLevel::Unknown), -1);
}

// =============================================================================
// MetricType Tests
// =============================================================================

TEST(MetricTypeTest, Names) {
    EXPECT_STREQ(MetricTypeName(MetricType::Counter), "Counter");
    EXPECT_STREQ(MetricTypeName(MetricType::Gauge), "Gauge");
    EXPECT_STREQ(MetricTypeName(MetricType::Histogram), "Histogram");
    EXPECT_STREQ(MetricTypeName(MetricType::Timer), "Timer");
}

// =============================================================================
// MetricSample Tests
// =============================================================================

TEST(MetricSampleTest, FormattedTimer) {
    MetricSample s;
    s.name = "decode_time";
    s.value = 12.345;
    s.type = MetricType::Timer;
    EXPECT_EQ(s.FormattedValue(), "12.35 ms");
}

TEST(MetricSampleTest, FormattedPercent) {
    MetricSample s;
    s.value = 85.7;
    s.unit = "%";
    EXPECT_EQ(s.FormattedValue(), "85.7%");
}

TEST(MetricSampleTest, FormattedBytesMB) {
    MetricSample s;
    s.value = 2.5 * 1024.0 * 1024.0;
    s.unit = "bytes";
    EXPECT_NE(s.FormattedValue().find("MB"), std::string::npos);
}

TEST(MetricSampleTest, FormattedBytesKB) {
    MetricSample s;
    s.value = 512.0 * 1024.0;
    s.unit = "bytes";
    EXPECT_NE(s.FormattedValue().find("KB"), std::string::npos);
}

TEST(MetricSampleTest, FormattedBytesB) {
    MetricSample s;
    s.value = 42.0;
    s.unit = "bytes";
    EXPECT_NE(s.FormattedValue().find("B"), std::string::npos);
}

TEST(MetricSampleTest, FormattedCountWithUnit) {
    MetricSample s;
    s.value = 100.0;
    s.unit = "count";
    EXPECT_EQ(s.FormattedValue(), "100 count");
}

TEST(MetricSampleTest, FormattedGaugeNoUnit) {
    MetricSample s;
    s.value = 42.0;
    EXPECT_EQ(s.FormattedValue(), "42");
}

// =============================================================================
// Statistics Tests
// =============================================================================

TEST(StatisticsTest, EmptyVector) {
    auto stats = Statistics::Compute({});
    EXPECT_TRUE(stats.IsEmpty());
    EXPECT_EQ(stats.count, 0u);
}

TEST(StatisticsTest, SingleValue) {
    auto stats = Statistics::Compute({ 5.0 });
    EXPECT_FALSE(stats.IsEmpty());
    EXPECT_EQ(stats.count, 1u);
    EXPECT_DOUBLE_EQ(stats.min, 5.0);
    EXPECT_DOUBLE_EQ(stats.max, 5.0);
    EXPECT_DOUBLE_EQ(stats.mean, 5.0);
}

TEST(StatisticsTest, MultipleValues) {
    auto stats = Statistics::Compute({ 10.0, 20.0, 30.0, 40.0, 50.0 });
    EXPECT_EQ(stats.count, 5u);
    EXPECT_DOUBLE_EQ(stats.min, 10.0);
    EXPECT_DOUBLE_EQ(stats.max, 50.0);
    EXPECT_DOUBLE_EQ(stats.mean, 30.0);
    EXPECT_DOUBLE_EQ(stats.median, 30.0);
}

TEST(StatisticsTest, Percentiles) {
    std::vector<double> vals;
    for (int i = 1; i <= 100; ++i) vals.push_back(static_cast<double>(i));
    auto stats = Statistics::Compute(vals);
    EXPECT_EQ(stats.count, 100u);
    EXPECT_GE(stats.p95, 95.0);
    EXPECT_GE(stats.p99, 99.0);
}

TEST(StatisticsTest, StandardDeviation) {
    auto stats = Statistics::Compute({ 2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0 });
    EXPECT_GT(stats.stddev, 0.0);
    EXPECT_LT(stats.stddev, 5.0);
}

TEST(StatisticsTest, Summary) {
    auto stats = Statistics::Compute({ 1.0, 2.0, 3.0 });
    auto summary = stats.Summary();
    EXPECT_NE(summary.find("n=3"), std::string::npos);
    EXPECT_NE(summary.find("min="), std::string::npos);
    EXPECT_NE(summary.find("mean="), std::string::npos);
}

// =============================================================================
// DecoderTelemetry Tests
// =============================================================================

TEST(DecoderTelemetryTest, SuccessRate) {
    DecoderTelemetry d;
    d.decoderName = "JPEG";
    d.totalDecodes = 100;
    d.successCount = 95;
    d.failureCount = 5;
    EXPECT_DOUBLE_EQ(d.SuccessRate(), 95.0);
}

TEST(DecoderTelemetryTest, FailureRate) {
    DecoderTelemetry d;
    d.totalDecodes = 100;
    d.failureCount = 20;
    EXPECT_DOUBLE_EQ(d.FailureRate(), 20.0);
}

TEST(DecoderTelemetryTest, ZeroDecodes) {
    DecoderTelemetry d;
    EXPECT_DOUBLE_EQ(d.SuccessRate(), 0.0);
    EXPECT_DOUBLE_EQ(d.FailureRate(), 0.0);
}

TEST(DecoderTelemetryTest, HealthHealthy) {
    DecoderTelemetry d;
    d.totalDecodes = 100;
    d.successCount = 98;
    d.failureCount = 2;
    EXPECT_EQ(d.Health(), HealthLevel::Healthy);
}

TEST(DecoderTelemetryTest, HealthDegraded) {
    DecoderTelemetry d;
    d.totalDecodes = 100;
    d.successCount = 90;
    d.failureCount = 10;
    EXPECT_EQ(d.Health(), HealthLevel::Degraded);
}

TEST(DecoderTelemetryTest, HealthUnhealthy) {
    DecoderTelemetry d;
    d.totalDecodes = 100;
    d.successCount = 70;
    d.failureCount = 30;
    EXPECT_EQ(d.Health(), HealthLevel::Unhealthy);
}

TEST(DecoderTelemetryTest, HealthCritical) {
    DecoderTelemetry d;
    d.totalDecodes = 100;
    d.successCount = 40;
    d.failureCount = 60;
    EXPECT_EQ(d.Health(), HealthLevel::Critical);
}

TEST(DecoderTelemetryTest, HealthUnknown) {
    DecoderTelemetry d;
    EXPECT_EQ(d.Health(), HealthLevel::Unknown);
}

// =============================================================================
// CacheTelemetry Tests
// =============================================================================

TEST(CacheTelemetryTest, HitRate) {
    CacheTelemetry c;
    c.totalRequests = 200;
    c.hits = 160;
    c.misses = 40;
    EXPECT_DOUBLE_EQ(c.HitRate(), 80.0);
}

TEST(CacheTelemetryTest, MissRate) {
    CacheTelemetry c;
    c.totalRequests = 200;
    c.hits = 160;
    EXPECT_DOUBLE_EQ(c.MissRate(), 20.0);
}

TEST(CacheTelemetryTest, Utilization) {
    CacheTelemetry c;
    c.currentSizeBytes = 512 * 1024 * 1024ULL;
    c.maxSizeBytes = 1024 * 1024 * 1024ULL;
    EXPECT_DOUBLE_EQ(c.Utilization(), 50.0);
}

TEST(CacheTelemetryTest, HealthHealthy) {
    CacheTelemetry c;
    c.totalRequests = 100;
    c.hits = 90;
    EXPECT_EQ(c.Health(), HealthLevel::Healthy);
}

TEST(CacheTelemetryTest, HealthDegraded) {
    CacheTelemetry c;
    c.totalRequests = 100;
    c.hits = 60;
    EXPECT_EQ(c.Health(), HealthLevel::Degraded);
}

TEST(CacheTelemetryTest, HealthUnhealthy) {
    CacheTelemetry c;
    c.totalRequests = 100;
    c.hits = 30;
    EXPECT_EQ(c.Health(), HealthLevel::Unhealthy);
}

TEST(CacheTelemetryTest, HealthUnknown) {
    CacheTelemetry c;
    EXPECT_EQ(c.Health(), HealthLevel::Unknown);
}

TEST(CacheTelemetryTest, ZeroRequests) {
    CacheTelemetry c;
    EXPECT_DOUBLE_EQ(c.HitRate(), 0.0);
    EXPECT_DOUBLE_EQ(c.Utilization(), 0.0);
}

// =============================================================================
// SystemMetrics Tests
// =============================================================================

TEST(SystemMetricsTest, MemoryUsagePercent) {
    SystemMetrics s;
    s.memoryUsedBytes = 8ULL * 1024 * 1024 * 1024;
    s.memoryTotalBytes = 16ULL * 1024 * 1024 * 1024;
    EXPECT_DOUBLE_EQ(s.MemoryUsagePercent(), 50.0);
}

TEST(SystemMetricsTest, DiskUsagePercent) {
    SystemMetrics s;
    s.diskUsedBytes = 400ULL * 1024 * 1024 * 1024;
    s.diskTotalBytes = 1000ULL * 1024 * 1024 * 1024;
    EXPECT_DOUBLE_EQ(s.DiskUsagePercent(), 40.0);
}

TEST(SystemMetricsTest, HealthHealthy) {
    SystemMetrics s;
    s.cpuUsagePercent = 30.0;
    s.memoryUsedBytes = 4ULL * 1024 * 1024 * 1024;
    s.memoryTotalBytes = 16ULL * 1024 * 1024 * 1024;
    EXPECT_EQ(s.OverallHealth(), HealthLevel::Healthy);
}

TEST(SystemMetricsTest, HealthDegraded) {
    SystemMetrics s;
    s.cpuUsagePercent = 80.0;
    s.memoryUsedBytes = 4ULL * 1024 * 1024 * 1024;
    s.memoryTotalBytes = 16ULL * 1024 * 1024 * 1024;
    EXPECT_EQ(s.OverallHealth(), HealthLevel::Degraded);
}

TEST(SystemMetricsTest, HealthCriticalCPU) {
    SystemMetrics s;
    s.cpuUsagePercent = 95.0;
    s.memoryUsedBytes = 4ULL * 1024 * 1024 * 1024;
    s.memoryTotalBytes = 16ULL * 1024 * 1024 * 1024;
    EXPECT_EQ(s.OverallHealth(), HealthLevel::Critical);
}

TEST(SystemMetricsTest, HealthCriticalMemory) {
    SystemMetrics s;
    s.cpuUsagePercent = 30.0;
    s.memoryUsedBytes = 15872ULL * 1024 * 1024;
    s.memoryTotalBytes = 16384ULL * 1024 * 1024;
    EXPECT_EQ(s.OverallHealth(), HealthLevel::Critical);
}

TEST(SystemMetricsTest, ZeroTotals) {
    SystemMetrics s;
    EXPECT_DOUBLE_EQ(s.MemoryUsagePercent(), 0.0);
    EXPECT_DOUBLE_EQ(s.DiskUsagePercent(), 0.0);
}

// =============================================================================
// DashboardData Tests
// =============================================================================

TEST(DashboardDataTest, UptimeHuman) {
    DashboardData d;
    d.uptimeMs = 3661000; // 1h 1m 1s
    auto result = d.UptimeHuman();
    EXPECT_NE(result.find("1h"), std::string::npos);
    EXPECT_NE(result.find("1m"), std::string::npos);
    EXPECT_NE(result.find("1s"), std::string::npos);
}

TEST(DashboardDataTest, UptimeSecondsOnly) {
    DashboardData d;
    d.uptimeMs = 45000;
    EXPECT_EQ(d.UptimeHuman(), "45s");
}

TEST(DashboardDataTest, HealthyDecoderCount) {
    DashboardData d;
    DecoderTelemetry healthy;
    healthy.totalDecodes = 100;
    healthy.successCount = 99;
    healthy.failureCount = 1;
    DecoderTelemetry unhealthy;
    unhealthy.totalDecodes = 100;
    unhealthy.successCount = 40;
    unhealthy.failureCount = 60;
    d.decoders = { healthy, unhealthy, healthy };
    EXPECT_EQ(d.HealthyDecoderCount(), 2u);
}

TEST(DashboardDataTest, OverallHealthCriticalErrors) {
    DashboardData d;
    d.totalErrors = 200;
    EXPECT_EQ(d.OverallHealth(), HealthLevel::Critical);
}

TEST(DashboardDataTest, OverallHealthCriticalSystem) {
    DashboardData d;
    d.system.cpuUsagePercent = 95.0;
    d.system.memoryUsedBytes = 4ULL * 1024 * 1024 * 1024;
    d.system.memoryTotalBytes = 16ULL * 1024 * 1024 * 1024;
    EXPECT_EQ(d.OverallHealth(), HealthLevel::Critical);
}

TEST(DashboardDataTest, OverallHealthDegradedCache) {
    DashboardData d;
    d.cache.totalRequests = 100;
    d.cache.hits = 30;
    EXPECT_EQ(d.OverallHealth(), HealthLevel::Degraded);
}

TEST(DashboardDataTest, OverallHealthHealthy) {
    DashboardData d;
    d.totalErrors = 5;
    d.cache.totalRequests = 100;
    d.cache.hits = 90;
    d.system.cpuUsagePercent = 30.0;
    EXPECT_EQ(d.OverallHealth(), HealthLevel::Healthy);
}

// =============================================================================
// DiagnosticExport Tests
// =============================================================================

TEST(DiagnosticExportTest, ToTextBasic) {
    DashboardData d;
    d.version = "7.0.0";
    d.uptimeMs = 60000;
    d.totalThumbnailsGenerated = 500;
    d.totalErrors = 3;
    auto text = DiagnosticExport::ToText(d);
    EXPECT_NE(text.find("ExplorerLens Diagnostics"), std::string::npos);
    EXPECT_NE(text.find("7.0.0"), std::string::npos);
    EXPECT_NE(text.find("500"), std::string::npos);
}

TEST(DiagnosticExportTest, ToTextWithDecoders) {
    DashboardData d;
    d.version = "7.0.0";
    DecoderTelemetry dec;
    dec.decoderName = "HEIF";
    dec.totalDecodes = 50;
    dec.successCount = 48;
    dec.failureCount = 2;
    dec.avgDecodeMs = 15.5;
    d.decoders.push_back(dec);
    auto text = DiagnosticExport::ToText(d);
    EXPECT_NE(text.find("HEIF"), std::string::npos);
    EXPECT_NE(text.find("48/50"), std::string::npos);
}

TEST(DiagnosticExportTest, ToJSONBasic) {
    DashboardData d;
    d.version = "7.0.0";
    d.uptimeMs = 120000;
    d.totalThumbnailsGenerated = 100;
    auto json = DiagnosticExport::ToJSON(d);
    EXPECT_NE(json.find("\"version\": \"7.0.0\""), std::string::npos);
    EXPECT_NE(json.find("\"uptime_ms\": 120000"), std::string::npos);
    EXPECT_NE(json.find("\"thumbnails_generated\": 100"), std::string::npos);
}

TEST(DiagnosticExportTest, ToJSONCacheSection) {
    DashboardData d;
    d.version = "7.0.0";
    d.cache.totalRequests = 100;
    d.cache.hits = 85;
    auto json = DiagnosticExport::ToJSON(d);
    EXPECT_NE(json.find("\"cache\""), std::string::npos);
    EXPECT_NE(json.find("\"hit_rate\""), std::string::npos);
}

// =============================================================================
// DiagnosticsConfig Tests
// =============================================================================

TEST(DiagnosticsConfigTest, Default) {
    auto cfg = DiagnosticsConfig::Default();
    EXPECT_TRUE(cfg.enabled);
    EXPECT_TRUE(cfg.collectTimings);
    EXPECT_TRUE(cfg.collectMemory);
    EXPECT_FALSE(cfg.collectGPU);
    EXPECT_EQ(cfg.sampleIntervalMs, 5000u);
    EXPECT_EQ(cfg.retentionMinutes, 60u);
}

TEST(DiagnosticsConfigTest, Detailed) {
    auto cfg = DiagnosticsConfig::Detailed();
    EXPECT_TRUE(cfg.enabled);
    EXPECT_TRUE(cfg.collectGPU);
    EXPECT_EQ(cfg.sampleIntervalMs, 1000u);
    EXPECT_EQ(cfg.retentionMinutes, 1440u);
}

TEST(DiagnosticsConfigTest, Minimal) {
    auto cfg = DiagnosticsConfig::Minimal();
    EXPECT_TRUE(cfg.enabled);
    EXPECT_FALSE(cfg.collectTimings);
    EXPECT_FALSE(cfg.collectMemory);
    EXPECT_EQ(cfg.sampleIntervalMs, 30000u);
}

TEST(DiagnosticsConfigTest, Disabled) {
    auto cfg = DiagnosticsConfig::Disabled();
    EXPECT_FALSE(cfg.enabled);
}
