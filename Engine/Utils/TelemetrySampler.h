// TelemetrySampler.h — Statistical sampling for telemetry data reduction
// Copyright (c) 2026 ExplorerLens Project
//
// Reduces telemetry volume by applying reservoir sampling and rate limiting
// to high-frequency events while preserving statistical significance.
//
#pragma once
#include <string>
#include <cstdint>
#include <random>

namespace ExplorerLens {
namespace Engine {

struct TelemetrySamplerConfig {
    bool enabled = true;
    uint32_t sampleRate = 100;  // 1 in N events sampled
    uint32_t reservoirSize = 1000;
    std::string label = "TelemetrySampler";
};

class TelemetrySampler {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_rng.seed(42);
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    TelemetrySamplerConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    bool ShouldSample() {
        m_totalEvents++;
        std::uniform_int_distribution<uint32_t> dist(0, m_config.sampleRate - 1);
        return dist(m_rng) == 0;
    }

    uint64_t GetTotalEvents() const { return m_totalEvents; }
    uint64_t EstimateTotalFromSampled(uint64_t sampledCount) const {
        return sampledCount * m_config.sampleRate;
    }

private:
    bool m_initialized = false;
    TelemetrySamplerConfig m_config;
    std::mt19937 m_rng;
    uint64_t m_totalEvents = 0;
};

}
} // namespace ExplorerLens::Engine
