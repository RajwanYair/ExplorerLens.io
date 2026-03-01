#pragma once
// ============================================================================
// CacheTelemetryCollector.h — Cache performance metrics collection
//
// Purpose:   Cache performance metrics collection and aggregation
// Provides:  CacheTelemetryEvent, CacheTelemetryInterval enums, and
//            CacheTelemetryCollector class
// Used by:   Telemetry pipeline for hit-rate and efficiency monitoring
// ============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Types of cache telemetry events
enum class CacheTelemetryEvent : uint8_t {
    CacheHit = 0,  // Successful cache lookup
    CacheMiss = 1,  // Cache miss (decode required)
    Eviction = 2,  // Entry evicted from cache
    Resize = 3,  // Cache budget resized
    Corruption = 4   // Cache entry corruption detected
};

inline const char* CacheTelemetryEventName(CacheTelemetryEvent e) noexcept {
    switch (e) {
    case CacheTelemetryEvent::CacheHit:   return "CacheHit";
    case CacheTelemetryEvent::CacheMiss:  return "CacheMiss";
    case CacheTelemetryEvent::Eviction:   return "Eviction";
    case CacheTelemetryEvent::Resize:     return "Resize";
    case CacheTelemetryEvent::Corruption: return "Corruption";
    default:                         return "Unknown";
    }
}

/// Time interval for telemetry aggregation
enum class CacheTelemetryInterval : uint8_t {
    Realtime = 0,  // No aggregation, raw events
    Minute = 1,  // Per-minute buckets
    Hour = 2,  // Per-hour buckets
    Day = 3,  // Per-day buckets
    Session = 4   // Per-session aggregate
};

inline const char* CacheTelemetryIntervalName(CacheTelemetryInterval i) noexcept {
    switch (i) {
    case CacheTelemetryInterval::Realtime: return "Realtime";
    case CacheTelemetryInterval::Minute:   return "Minute";
    case CacheTelemetryInterval::Hour:     return "Hour";
    case CacheTelemetryInterval::Day:      return "Day";
    case CacheTelemetryInterval::Session:  return "Session";
    default:                          return "Unknown";
    }
}

/// A single recorded telemetry event
struct TelemetryRecord {
    CacheTelemetryEvent event = CacheTelemetryEvent::CacheHit;
    uint64_t       timestamp = 0;   // Milliseconds since session start
    uint64_t       sizeBytes = 0;   // Related size (entry, resize delta)
    std::string    key;              // Cache key if applicable
};

/// Snapshot of cache performance metrics
struct CacheTelemetrySnapshot {
    uint64_t hits = 0;
    uint64_t misses = 0;
    uint64_t evictions = 0;
    uint64_t resizes = 0;
    uint64_t corruptions = 0;
    float    hitRate = 0.0f;
    float    evictionRate = 0.0f;
};

/// Configuration for telemetry collection
struct CacheTelemetryConfig {
    CacheTelemetryInterval interval = CacheTelemetryInterval::Session;
    uint32_t          maxRecords = 100000;
    bool              includeKeys = false;  // Include cache keys in records
    bool              exportEnabled = true;
};

/// Collects and reports cache performance telemetry,
/// tracking hit/miss rates, eviction patterns, and
/// corruption events for the thumbnail cache system.
class CacheTelemetryCollector {
public:
    CacheTelemetryCollector() = default;
    ~CacheTelemetryCollector() = default;

    CacheTelemetryCollector(const CacheTelemetryCollector&) = delete;
    CacheTelemetryCollector& operator=(const CacheTelemetryCollector&) = delete;
    CacheTelemetryCollector(CacheTelemetryCollector&&) noexcept = default;
    CacheTelemetryCollector& operator=(CacheTelemetryCollector&&) noexcept = default;

    /// Record a telemetry event
    void Record(CacheTelemetryEvent event, uint64_t sizeBytes = 0) {
        (void)sizeBytes;
        switch (event) {
        case CacheTelemetryEvent::CacheHit:   m_hits++; break;
        case CacheTelemetryEvent::CacheMiss:  m_misses++; break;
        case CacheTelemetryEvent::Eviction:   m_evictions++; break;
        case CacheTelemetryEvent::Resize:     m_resizes++; break;
        case CacheTelemetryEvent::Corruption: m_corruptions++; break;
        }
        m_totalEvents++;
    }

    /// Get cache hit rate (0.0 - 1.0)
    float GetHitRate() const noexcept {
        uint64_t total = m_hits + m_misses;
        if (total == 0) return 0.0f;
        return static_cast<float>(m_hits) / static_cast<float>(total);
    }

    /// Get eviction rate relative to total operations
    float GetEvictionRate() const noexcept {
        if (m_totalEvents == 0) return 0.0f;
        return static_cast<float>(m_evictions) / static_cast<float>(m_totalEvents);
    }

    /// Export a snapshot of current metrics
    CacheTelemetrySnapshot Export() const {
        CacheTelemetrySnapshot snap;
        snap.hits = m_hits;
        snap.misses = m_misses;
        snap.evictions = m_evictions;
        snap.resizes = m_resizes;
        snap.corruptions = m_corruptions;
        snap.hitRate = GetHitRate();
        snap.evictionRate = GetEvictionRate();
        return snap;
    }

    /// Get total event count
    uint64_t GetTotalEvents() const noexcept { return m_totalEvents; }

    /// Reset all counters
    void Reset() noexcept {
        m_hits = m_misses = m_evictions = m_resizes = m_corruptions = 0;
        m_totalEvents = 0;
    }

    /// Apply configuration
    void SetConfig(const CacheTelemetryConfig& cfg) noexcept { m_config = cfg; }

private:
    CacheTelemetryConfig m_config;
    uint64_t m_hits = 0;
    uint64_t m_misses = 0;
    uint64_t m_evictions = 0;
    uint64_t m_resizes = 0;
    uint64_t m_corruptions = 0;
    uint64_t m_totalEvents = 0;
};

} // namespace Engine
} // namespace ExplorerLens
