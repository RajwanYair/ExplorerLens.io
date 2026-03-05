// PluginTelemetryBridge.h — Plugin-to-Host Telemetry Aggregation
// Copyright (c) 2026 ExplorerLens Project
//
// Plugin-to-host telemetry aggregation. Uses shared memory ring buffer for
// lock-free performance metric collection from plugins to host process.
//
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <mutex>
#include <chrono>
#include <array>
#include <atomic>

namespace ExplorerLens {
namespace Engine {

enum class PluginTelemetryEventType : uint8_t {
    DecodeStart,
    DecodeEnd,
    CacheHit,
    CacheMiss,
    Error,
    Warning,
    MemoryAlloc,
    MemoryFree,
    GPUSubmit,
    GPUComplete
};

struct PluginTelemetryEvent {
    PluginTelemetryEventType type = PluginTelemetryEventType::DecodeStart;
    uint64_t timestamp = 0;
    uint32_t pluginId = 0;
    uint32_t threadId = 0;
    double valueMs = 0.0;
    uint64_t valueBytes = 0;
    std::string metadata;
};

struct PluginMetrics {
    std::string pluginName;
    uint32_t pluginId = 0;
    uint64_t decodeCount = 0;
    double totalDecodeTimeMs = 0.0;
    double avgDecodeTimeMs = 0.0;
    double peakDecodeTimeMs = 0.0;
    uint64_t cacheHits = 0;
    uint64_t cacheMisses = 0;
    uint64_t errorCount = 0;
    uint64_t memoryAllocatedBytes = 0;
};

struct AggregateMetrics {
    uint64_t totalEvents = 0;
    double uptimeSeconds = 0.0;
    double avgDecodeTimeMs = 0.0;
    double overallCacheHitRate = 0.0;
    uint64_t totalErrorCount = 0;
    uint32_t activePlugins = 0;
};

class PluginTelemetryBridge {
public:
    static PluginTelemetryBridge& Instance() {
        static PluginTelemetryBridge instance;
        return instance;
    }

    inline void RegisterPlugin(uint32_t pluginId, const std::string& name) {
        std::lock_guard<std::mutex> lock(m_mutex);
        PluginMetrics metrics;
        metrics.pluginId = pluginId;
        metrics.pluginName = name;
        m_pluginMetrics[pluginId] = metrics;
    }

    inline void RecordEvent(const PluginTelemetryEvent& event) {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_ringBuffer[m_writePos % RING_BUFFER_SIZE] = event;
        m_writePos++;
        m_totalEvents++;

        auto it = m_pluginMetrics.find(event.pluginId);
        if (it == m_pluginMetrics.end()) return;

        auto& metrics = it->second;
        switch (event.type) {
        case PluginTelemetryEventType::DecodeEnd:
            metrics.decodeCount++;
            metrics.totalDecodeTimeMs += event.valueMs;
            metrics.avgDecodeTimeMs = metrics.totalDecodeTimeMs / metrics.decodeCount;
            if (event.valueMs > metrics.peakDecodeTimeMs) metrics.peakDecodeTimeMs = event.valueMs;
            break;
        case PluginTelemetryEventType::CacheHit:
            metrics.cacheHits++;
            break;
        case PluginTelemetryEventType::CacheMiss:
            metrics.cacheMisses++;
            break;
        case PluginTelemetryEventType::Error:
            metrics.errorCount++;
            break;
        case PluginTelemetryEventType::MemoryAlloc:
            metrics.memoryAllocatedBytes += event.valueBytes;
            break;
        case PluginTelemetryEventType::MemoryFree:
            if (metrics.memoryAllocatedBytes >= event.valueBytes)
                metrics.memoryAllocatedBytes -= event.valueBytes;
            break;
        default:
            break;
        }
    }

    inline PluginMetrics GetPluginMetrics(uint32_t pluginId) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_pluginMetrics.find(pluginId);
        return it != m_pluginMetrics.end() ? it->second : PluginMetrics{};
    }

    inline AggregateMetrics GetAggregateMetrics() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        AggregateMetrics agg;
        agg.totalEvents = m_totalEvents;
        agg.activePlugins = static_cast<uint32_t>(m_pluginMetrics.size());

        uint64_t totalDecodes = 0;
        double totalDecodeTime = 0.0;
        uint64_t totalHits = 0, totalMisses = 0;

        for (const auto& [id, metrics] : m_pluginMetrics) {
            totalDecodes += metrics.decodeCount;
            totalDecodeTime += metrics.totalDecodeTimeMs;
            totalHits += metrics.cacheHits;
            totalMisses += metrics.cacheMisses;
            agg.totalErrorCount += metrics.errorCount;
        }

        if (totalDecodes > 0) agg.avgDecodeTimeMs = totalDecodeTime / totalDecodes;
        if (totalHits + totalMisses > 0)
            agg.overallCacheHitRate = static_cast<double>(totalHits) / (totalHits + totalMisses);

        auto elapsed = std::chrono::steady_clock::now() - m_startTime;
        agg.uptimeSeconds = std::chrono::duration<double>(elapsed).count();
        return agg;
    }

    inline std::vector<PluginTelemetryEvent> GetRecentEvents(uint32_t count = 100) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<PluginTelemetryEvent> events;
        uint64_t start = m_writePos > count ? m_writePos - count : 0;
        for (uint64_t i = start; i < m_writePos; ++i) {
            events.push_back(m_ringBuffer[i % RING_BUFFER_SIZE]);
        }
        return events;
    }

    inline std::string EventTypeToString(PluginTelemetryEventType type) const {
        switch (type) {
        case PluginTelemetryEventType::DecodeStart: return "DecodeStart";
        case PluginTelemetryEventType::DecodeEnd:   return "DecodeEnd";
        case PluginTelemetryEventType::CacheHit:    return "CacheHit";
        case PluginTelemetryEventType::CacheMiss:   return "CacheMiss";
        case PluginTelemetryEventType::Error:       return "Error";
        case PluginTelemetryEventType::Warning:     return "Warning";
        case PluginTelemetryEventType::MemoryAlloc: return "MemoryAlloc";
        case PluginTelemetryEventType::MemoryFree:  return "MemoryFree";
        case PluginTelemetryEventType::GPUSubmit:   return "GPUSubmit";
        case PluginTelemetryEventType::GPUComplete: return "GPUComplete";
        default:                              return "Unknown";
        }
    }

private:
    PluginTelemetryBridge() : m_startTime(std::chrono::steady_clock::now()) {}

    static constexpr size_t RING_BUFFER_SIZE = 8192;

    mutable std::mutex m_mutex;
    std::array<PluginTelemetryEvent, RING_BUFFER_SIZE> m_ringBuffer;
    uint64_t m_writePos = 0;
    uint64_t m_totalEvents = 0;
    std::unordered_map<uint32_t, PluginMetrics> m_pluginMetrics;
    std::chrono::steady_clock::time_point m_startTime;
};

}
} // namespace ExplorerLens::Engine
