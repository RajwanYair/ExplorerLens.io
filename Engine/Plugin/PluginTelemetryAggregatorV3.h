// PluginTelemetryAggregatorV3.h — Plugin Telemetry Aggregator v3
// Copyright (c) 2026 ExplorerLens Project
//
// Aggregates per-plugin decode metrics, error rates, and user interactions into time-windowed reports.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

struct PluginTelemetryV3 {
    std::string pluginId;
    uint64_t    decodes      = 0;
    uint64_t    errors       = 0;
    double      avgLatencyMs = 0.0;
    double      p99LatencyMs = 0.0;
    uint64_t    windowMs     = 1000;
};
class PluginTelemetryAggregatorV3 {
public:
    void Record(const std::string& id, double latencyMs, bool error) {
        auto& t = m_data[id];
        t.pluginId = id; t.decodes++;
        if (error) t.errors++;
        t.avgLatencyMs = (t.avgLatencyMs * (t.decodes - 1) + latencyMs) / t.decodes;
    }
    PluginTelemetryV3 Get(const std::string& id) const {
        auto it = m_data.find(id);
        return it != m_data.end() ? it->second : PluginTelemetryV3{};
    }
    size_t PluginCount() const { return m_data.size(); }
private:
    std::unordered_map<std::string, PluginTelemetryV3> m_data;
};

} // namespace Engine
} // namespace ExplorerLens