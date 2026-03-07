// MemoryWatermarkTracker.h — Memory Usage Threshold Monitor
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks memory consumption against configurable high/low watermarks and
// fires callbacks when thresholds are crossed, enabling proactive eviction.
//
#pragma once

#include <cstdint>
#include <functional>
#include <vector>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class WatermarkLevel : uint8_t {
    BelowLow = 0,    // Comfortable — free to allocate
    Normal = 1,       // Between low and high
    AboveHigh = 2,    // Should start freeing
    Critical = 3      // Emergency — stop allocating
};

struct MemWatermarkConfig {
    uint64_t lowWatermarkBytes = 256ULL * 1024 * 1024;
    uint64_t highWatermarkBytes = 384ULL * 1024 * 1024;
    uint64_t criticalBytes = 480ULL * 1024 * 1024;
    uint64_t maxBytes = 512ULL * 1024 * 1024;
    uint32_t checkIntervalMs = 100;
};

struct WatermarkSnapshot {
    uint64_t currentBytes = 0;
    WatermarkLevel level = WatermarkLevel::BelowLow;
    uint64_t headroom = 0;
    double utilizationPercent = 0.0;
    uint32_t crossingCount = 0;
};

class MemoryWatermarkTracker {
public:
    using ThresholdCallback = std::function<void(WatermarkLevel, uint64_t)>;

    void Configure(const MemWatermarkConfig& config) { m_config = config; }

    WatermarkLevel Evaluate(uint64_t currentBytes) {
        WatermarkLevel newLevel;
        if (currentBytes >= m_config.criticalBytes)
            newLevel = WatermarkLevel::Critical;
        else if (currentBytes >= m_config.highWatermarkBytes)
            newLevel = WatermarkLevel::AboveHigh;
        else if (currentBytes >= m_config.lowWatermarkBytes)
            newLevel = WatermarkLevel::Normal;
        else
            newLevel = WatermarkLevel::BelowLow;

        if (newLevel != m_lastLevel) {
            m_crossings++;
            if (m_callback) m_callback(newLevel, currentBytes);
        }
        m_lastLevel = newLevel;
        m_lastBytes = currentBytes;
        return newLevel;
    }

    void OnThresholdCrossing(ThresholdCallback cb) {
        m_callback = std::move(cb);
    }

    WatermarkSnapshot GetSnapshot() const {
        WatermarkSnapshot s;
        s.currentBytes = m_lastBytes;
        s.level = m_lastLevel;
        s.headroom = m_config.maxBytes > m_lastBytes ? m_config.maxBytes - m_lastBytes : 0;
        s.utilizationPercent = m_config.maxBytes > 0
            ? 100.0 * m_lastBytes / m_config.maxBytes : 0.0;
        s.crossingCount = m_crossings;
        return s;
    }

    bool CanAllocate(uint64_t bytes) const {
        return m_lastBytes + bytes < m_config.criticalBytes;
    }

private:
    MemWatermarkConfig m_config;
    ThresholdCallback m_callback;
    WatermarkLevel m_lastLevel = WatermarkLevel::BelowLow;
    uint64_t m_lastBytes = 0;
    uint32_t m_crossings = 0;
};

} // namespace Engine
} // namespace ExplorerLens
