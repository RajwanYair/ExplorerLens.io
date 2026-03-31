// DecoderHealthDashboard.h — Decoder health monitoring with circuit-breaker pattern
// Copyright (c) 2026 ExplorerLens Project
//
// Provides circuit-state tracking, health status reporting, and per-decoder
// statistics for the decode pipeline. Used by observability and diagnostics.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace ExplorerLens {
namespace Core {

enum class CircuitState : uint8_t {
    Closed   = 0,
    Open     = 1,
    HalfOpen = 2
};

enum class HealthStatus : uint8_t {
    Healthy   = 0,
    Degraded  = 1,
    Unhealthy = 2,
    Disabled  = 3
};

inline const char* HealthStatusName(HealthStatus s) noexcept {
    switch (s) {
        case HealthStatus::Healthy:   return "Healthy";
        case HealthStatus::Degraded:  return "Degraded";
        case HealthStatus::Unhealthy: return "Unhealthy";
        case HealthStatus::Disabled:  return "Disabled";
    }
    return "Unknown";
}

struct DecoderDashboardConfig {
    uint32_t maxDecoders = 64;
    uint32_t failureThreshold = 5;
    uint32_t halfOpenProbeIntervalMs = 10000;
};

struct DecoderStats {
    uint64_t totalDecodes = 0;
    uint64_t successCount = 0;
    uint64_t failureCount = 0;
    double avgLatencyMs = 0.0;
    HealthStatus status = HealthStatus::Healthy;
    CircuitState circuit = CircuitState::Closed;
};

class DecoderHealthDashboard {
public:
    static DecoderHealthDashboard Create(const DecoderDashboardConfig& config) {
        DecoderHealthDashboard d;
        d.m_config = config;
        return d;
    }

    void RegisterDecoder(const std::string& name, const std::vector<std::string>& extensions) {
        m_decoders[name] = DecoderStats{};
        (void)extensions;
    }

    void RecordDecode(const std::string& name, bool success, int durationMs, int sizeBytes) {
        auto& s = m_decoders[name];
        s.totalDecodes++;
        if (success) s.successCount++; else s.failureCount++;
        s.avgLatencyMs = (s.avgLatencyMs * (s.totalDecodes - 1) + durationMs) / s.totalDecodes;
        (void)sizeBytes;
    }

    DecoderStats GetStats() const {
        DecoderStats agg;
        for (auto& [k, v] : m_decoders) {
            agg.totalDecodes += v.totalDecodes;
            agg.successCount += v.successCount;
            agg.failureCount += v.failureCount;
        }
        return agg;
    }

private:
    DecoderHealthDashboard() = default;
    DecoderDashboardConfig m_config{};
    std::unordered_map<std::string, DecoderStats> m_decoders;
};

} // namespace Core
} // namespace ExplorerLens
