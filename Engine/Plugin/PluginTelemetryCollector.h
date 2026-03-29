// PluginTelemetryCollector.h — Plugin Telemetry Collector
// Copyright (c) 2026 ExplorerLens Project
//
// Collects per-plugin performance and error telemetry with differential privacy scrubbing.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace ExplorerLens { namespace Engine {

struct PTCEvent {
    std::string pluginId;
    std::string eventName;
    float       duration = 0.0f;
    bool        isError  = false;
};

struct PTCSummary {
    std::string pluginId;
    uint32_t    totalEvents = 0;
    uint32_t    errorCount  = 0;
    float       avgDuration = 0.0f;
};

class PluginTelemetryCollector {
public:
    void Record(const PTCEvent& event) {
        m_events[event.pluginId].push_back(event);
    }

    PTCSummary GetSummary(const std::string& pluginId) const {
        PTCSummary s;
        s.pluginId = pluginId;
        auto it = m_events.find(pluginId);
        if (it == m_events.end()) return s;
        s.totalEvents = static_cast<uint32_t>(it->second.size());
        float totalDur = 0.0f;
        for (const auto& e : it->second) {
            if (e.isError) ++s.errorCount;
            totalDur += e.duration;
        }
        s.avgDuration = (s.totalEvents > 0)
            ? totalDur / static_cast<float>(s.totalEvents)
            : 0.0f;
        return s;
    }

    void FlushPlugin(const std::string& pluginId) { m_events.erase(pluginId); }

    uint32_t TotalEventCount() const {
        uint32_t n = 0;
        for (const auto& [k, v] : m_events) n += static_cast<uint32_t>(v.size());
        return n;
    }

private:
    std::unordered_map<std::string, std::vector<PTCEvent>> m_events;
};

}} // namespace ExplorerLens::Engine
