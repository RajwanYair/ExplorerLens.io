// Sprint 126 — ETW Sink Completion Tests
// Validates ETW provider wiring, retention policies, event emission, and sink management

#include <gtest/gtest.h>
#include "Core/ETWSinkComplete.h"

using namespace DarkThumbs::ETW;

// ── Schema Version ───────────────────────────────────────────────

TEST(Sprint126_ETWSink, SchemaVersion_Current) {
    EXPECT_EQ(SchemaVersion::Major, 2);
    EXPECT_EQ(SchemaVersion::Minor, 0);
    EXPECT_STREQ(SchemaVersion::VersionString, "2.0");
}

// ── Retention Policy ─────────────────────────────────────────────

TEST(Sprint126_ETWSink, RetentionPolicy_Production) {
    auto p = RetentionPolicy::Production();
    EXPECT_EQ(p.maxLogFiles, 10);
    EXPECT_EQ(p.maxRetentionDays, 30);
    EXPECT_EQ(p.strategy, RotationStrategy::Hybrid);
    EXPECT_TRUE(p.compressRotated);
}

TEST(Sprint126_ETWSink, RetentionPolicy_Development) {
    auto p = RetentionPolicy::Development();
    EXPECT_EQ(p.maxLogFiles, 5);
    EXPECT_EQ(p.maxRetentionDays, 7);
    EXPECT_FALSE(p.compressRotated);
}

TEST(Sprint126_ETWSink, RetentionPolicy_Enterprise) {
    auto p = RetentionPolicy::Enterprise();
    EXPECT_EQ(p.maxLogFiles, 50);
    EXPECT_EQ(p.maxRetentionDays, 90);
    EXPECT_TRUE(p.compressRotated);
}

// ── ETW Event Structure ──────────────────────────────────────────

TEST(Sprint126_ETWSink, ETWEvent_AddFields) {
    ETWEvent e;
    e.eventId = EventIds::DecodeComplete;
    e.AddField("decoder", "WebPDecoder");
    e.AddField("elapsedMs", 12.5);
    e.AddField("size", int64_t(1024));

    EXPECT_EQ(e.payload.size(), 3u);
    EXPECT_EQ(e.payload["decoder"], "WebPDecoder");
    EXPECT_EQ(e.eventId, EventIds::DecodeComplete);
}

TEST(Sprint126_ETWSink, EventIds_WellKnown) {
    EXPECT_EQ(EventIds::RequestStart, 100);
    EXPECT_EQ(EventIds::CacheHit, 200);
    EXPECT_EQ(EventIds::DecodeStart, 300);
    EXPECT_EQ(EventIds::GPUSubmit, 400);
    EXPECT_EQ(EventIds::PluginLoad, 500);
    EXPECT_EQ(EventIds::MemoryPressure, 600);
    EXPECT_EQ(EventIds::ConfigChange, 700);
    EXPECT_EQ(EventIds::HealthCheck, 800);
}

TEST(Sprint126_ETWSink, Keywords_BitMasks) {
    EXPECT_EQ(Keywords::Pipeline, 0x0001u);
    EXPECT_EQ(Keywords::Cache, 0x0002u);
    EXPECT_EQ(Keywords::Decoder, 0x0004u);
    EXPECT_NE(Keywords::Pipeline & Keywords::Cache, Keywords::Pipeline);
    EXPECT_EQ(Keywords::All & Keywords::Decoder, Keywords::Decoder);
}

// ── Sink Configuration ───────────────────────────────────────────

TEST(Sprint126_ETWSink, ETWSinkConfig_Production) {
    auto c = ETWSinkConfig::Production();
    EXPECT_TRUE(c.enableETW);
    EXPECT_TRUE(c.enableFileLog);
    EXPECT_FALSE(c.enableConsole);
    EXPECT_EQ(c.minLevel, 4);
}

TEST(Sprint126_ETWSink, ETWSinkConfig_Development) {
    auto c = ETWSinkConfig::Development();
    EXPECT_TRUE(c.enableConsole);
    EXPECT_EQ(c.minLevel, 5);
}

TEST(Sprint126_ETWSink, ETWSinkConfig_Enterprise) {
    auto c = ETWSinkConfig::Enterprise();
    EXPECT_EQ(c.retention.maxRetentionDays, 90);
    EXPECT_EQ(c.retention.maxLogFiles, 50);
}

// ── Sink Manager ─────────────────────────────────────────────────

TEST(Sprint126_ETWSink, SinkManager_Configure) {
    auto& mgr = ETWSinkManager::Get();
    mgr.Configure(ETWSinkConfig::Production());
    EXPECT_TRUE(mgr.IsConfigured());
    EXPECT_EQ(mgr.Config().providerName, "DarkThumbs-Engine-Core");
}

TEST(Sprint126_ETWSink, SinkManager_EmitDecodeEvent) {
    auto& mgr = ETWSinkManager::Get();
    mgr.Configure(ETWSinkConfig::Development());
    mgr.ResetStats();

    int received = 0;
    mgr.ClearHandlers();
    mgr.AddHandler([&](const ETWEvent& e) { received++; });

    mgr.EmitDecodeEvent(EventIds::DecodeComplete, "WebPDecoder", ".webp", 5.2, true);
    EXPECT_EQ(received, 1);
    EXPECT_EQ(mgr.Stats().eventsEmitted.load(), 1u);
}

TEST(Sprint126_ETWSink, SinkManager_EmitCacheEvent) {
    auto& mgr = ETWSinkManager::Get();
    mgr.Configure(ETWSinkConfig::Development());
    mgr.ResetStats();
    mgr.ClearHandlers();

    int hits = 0;
    mgr.AddHandler([&](const ETWEvent& e) {
        if (e.eventId == EventIds::CacheHit) hits++;
    });

    mgr.EmitCacheEvent(EventIds::CacheHit, "hash123", true);
    EXPECT_EQ(hits, 1);
}

TEST(Sprint126_ETWSink, SinkManager_DropRate) {
    SinkStatistics stats;
    stats.eventsEmitted = 90;
    stats.eventsDropped = 10;
    EXPECT_DOUBLE_EQ(stats.DropRate(), 10.0);
}

// ── File Log Entry ───────────────────────────────────────────────

TEST(Sprint126_ETWSink, FileLogEntry_ToJsonLine) {
    FileLogEntry entry;
    entry.timestamp = "2026-02-18T12:00:00Z";
    entry.level = "Info";
    entry.eventName = "DecodeComplete";
    entry.eventId = 301;
    entry.fields["decoder"] = "HEIFDecoder";
    auto json = entry.ToJsonLine();
    EXPECT_NE(json.find("DecodeComplete"), std::string::npos);
    EXPECT_NE(json.find("HEIFDecoder"), std::string::npos);
    EXPECT_NE(json.find("301"), std::string::npos);
}

// ── Rotation Logic ───────────────────────────────────────────────

TEST(Sprint126_ETWSink, ShouldRotate_SizeBased) {
    auto& mgr = ETWSinkManager::Get();
    ETWSinkConfig c = ETWSinkConfig::Production();
    c.retention.strategy = RotationStrategy::SizeBased;
    c.retention.maxFileSizeBytes = 1024;
    mgr.Configure(c);
    EXPECT_TRUE(mgr.ShouldRotate(2048));
    EXPECT_FALSE(mgr.ShouldRotate(512));
}

TEST(Sprint126_ETWSink, ShouldRotate_Hybrid) {
    auto& mgr = ETWSinkManager::Get();
    ETWSinkConfig c = ETWSinkConfig::Production();
    c.retention.strategy = RotationStrategy::Hybrid;
    c.retention.maxFileSizeBytes = 1000;
    mgr.Configure(c);
    EXPECT_TRUE(mgr.ShouldRotate(1500));
}

TEST(Sprint126_ETWSink, RetentionDays_Config) {
    auto& mgr = ETWSinkManager::Get();
    mgr.Configure(ETWSinkConfig::Enterprise());
    EXPECT_EQ(mgr.RetentionDays(), 90);
    EXPECT_EQ(mgr.MaxLogFiles(), 50);
}
