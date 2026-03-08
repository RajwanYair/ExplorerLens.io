// PipelineLatencyTracker.h — End-to-end pipeline latency measurement
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks end-to-end latency from thumbnail request to delivery,
// computing percentile distributions for SLA monitoring.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

struct PipelineLatencyTrackerConfig {
    bool enabled = true;
    uint32_t maxSamples = 10000;
    std::string label = "PipelineLatencyTracker";
};

class PipelineLatencyTracker {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_samples.reserve(m_config.maxSamples);
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    PipelineLatencyTrackerConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    void RecordLatency(uint64_t latencyUs) {
        if (m_samples.size() >= m_config.maxSamples)
            m_samples.erase(m_samples.begin());
        m_samples.push_back(latencyUs);
        m_sorted = false;
    }

    uint64_t GetPercentile(double pct) {
        if (m_samples.empty()) return 0;
        if (!m_sorted) { std::sort(m_samples.begin(), m_samples.end()); m_sorted = true; }
        size_t idx = static_cast<size_t>(pct / 100.0 * (m_samples.size() - 1));
        return m_samples[idx];
    }

    uint64_t GetP50() { return GetPercentile(50.0); }
    uint64_t GetP95() { return GetPercentile(95.0); }
    uint64_t GetP99() { return GetPercentile(99.0); }
    uint32_t GetSampleCount() const { return static_cast<uint32_t>(m_samples.size()); }

private:
    bool m_initialized = false;
    PipelineLatencyTrackerConfig m_config;
    std::vector<uint64_t> m_samples;
    bool m_sorted = false;
};

}
} // namespace ExplorerLens::Engine
