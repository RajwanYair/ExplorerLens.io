// PluginUsageTracker.h — Per-Plugin Usage Telemetry and Crash Reporting
// Copyright (c) 2026 ExplorerLens Project
//
// Counts thumbnail decode invocations per plugin, tracks latency percentiles,
// records plugin crashes, and emits structured telemetry events to the
// ObservabilityIntegration ETW provider (no PII collected).
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <atomic>

namespace ExplorerLens {
namespace Engine {

// ---- Per-Plugin Stats -------------------------------------------------------

struct PluginUsageStats {
    std::string pluginId;
    std::string version;
    uint64_t    totalInvocations  = 0;
    uint64_t    successCount      = 0;
    uint64_t    failureCount      = 0;
    uint64_t    crashCount        = 0;
    float       p50LatencyMs      = 0.0f;
    float       p95LatencyMs      = 0.0f;
    float       p99LatencyMs      = 0.0f;
    uint64_t    totalBytesDecoded = 0;
    std::string lastError;
    std::string lastCrashTimestamp;  // ISO 8601
};

// ---- Crash Record -----------------------------------------------------------

struct PluginUsageCrashRecord {
    std::string pluginId;
    std::string version;
    std::string timestamp;
    std::string exceptionCode;  // e.g. "0xC0000005" (access violation)
    std::string faultAddress;
    bool        autoDisabled = false;  // Plugin disabled after repeated crashes
};

// ---- PluginUsageTracker -----------------------------------------------------

class PluginUsageTracker {
public:
    PluginUsageTracker();
    ~PluginUsageTracker();

    // Record a successful decode invocation with latency.
    void RecordSuccess(const std::string& pluginId, float latencyMs, uint64_t bytesDecoded);

    // Record a decode failure (non-crash).
    void RecordFailure(const std::string& pluginId, const std::string& error);

    // Record a plugin crash (structured exception intercepted by sandbox).
    void RecordCrash(const PluginUsageCrashRecord& crash);

    // Get stats for a single plugin.
    PluginUsageStats GetStats(const std::string& pluginId) const;

    // Get stats for all tracked plugins.
    std::vector<PluginUsageStats> GetAllStats() const;

    // Get crash history (most recent first, up to maxRecords).
    std::vector<PluginUsageCrashRecord> GetCrashHistory(uint32_t maxRecords = 50) const;

    // Check if a plugin has been auto-disabled due to repeated crashes.
    bool IsAutoDisabled(const std::string& pluginId) const;

    // Clear counters for a plugin (e.g. after update).
    void Reset(const std::string& pluginId);
    void ResetAll();

    // Crash threshold — auto-disable after N crashes in a session.
    void SetCrashThreshold(uint32_t n);

    static PluginUsageTracker& Instance();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace Engine
} // namespace ExplorerLens
