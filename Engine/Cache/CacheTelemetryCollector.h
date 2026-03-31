// CacheTelemetryCollector.h — Cache telemetry and metrics collector
// Copyright (c) 2026 ExplorerLens Project
//
// Collects cache hit/miss/eviction events for telemetry reporting.
//
#pragma once
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

enum class CacheTelemetryEvent : uint8_t { CacheHit = 0, CacheMiss = 1, Eviction = 2, Corruption = 3 };

inline const char* CacheTelemetryEventName(CacheTelemetryEvent e) noexcept {
    switch (e) {
    case CacheTelemetryEvent::CacheHit:    return "CacheHit";
    case CacheTelemetryEvent::CacheMiss:   return "CacheMiss";
    case CacheTelemetryEvent::Eviction:    return "Eviction";
    case CacheTelemetryEvent::Corruption:  return "Corruption";
    default:                               return "Unknown";
    }
}

enum class CacheTelemetryInterval : uint8_t { Realtime = 0, Session = 1, Daily = 2 };

inline const char* CacheTelemetryIntervalName(CacheTelemetryInterval i) noexcept {
    switch (i) {
    case CacheTelemetryInterval::Realtime: return "Realtime";
    case CacheTelemetryInterval::Session:  return "Session";
    case CacheTelemetryInterval::Daily:    return "Daily";
    default:                               return "Unknown";
    }
}

struct TelemetrySnapshot {
    uint32_t hits       = 0;
    uint32_t misses     = 0;
    uint32_t evictions  = 0;
    float    hitRate      = 0.0f;
    float    evictionRate = 0.0f;
};

class CacheTelemetryCollector {
public:
    void Record(CacheTelemetryEvent event) {
        ++m_total;
        switch (event) {
        case CacheTelemetryEvent::CacheHit:  ++m_hits;      break;
        case CacheTelemetryEvent::CacheMiss: ++m_misses;    break;
        case CacheTelemetryEvent::Eviction:  ++m_evictions; break;
        default: break;
        }
    }

    float GetHitRate() const noexcept {
        if (m_total == 0) return 0.0f;
        return static_cast<float>(m_hits) / static_cast<float>(m_total);
    }

    uint32_t GetTotalEvents() const noexcept { return m_total; }

    TelemetrySnapshot Export() const {
        TelemetrySnapshot snap;
        snap.hits       = m_hits;
        snap.misses     = m_misses;
        snap.evictions  = m_evictions;
        snap.hitRate      = GetHitRate();
        snap.evictionRate = m_total > 0
            ? static_cast<float>(m_evictions) / static_cast<float>(m_total)
            : 0.0f;
        return snap;
    }

private:
    uint32_t m_hits      = 0;
    uint32_t m_misses    = 0;
    uint32_t m_evictions = 0;
    uint32_t m_total     = 0;
};

} // namespace Engine
} // namespace ExplorerLens
