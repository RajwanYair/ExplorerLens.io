// CacheGarbageCollector.h — Expired Cache Entry Reclamation
// Copyright (c) 2026 ExplorerLens Project
//
// Background garbage collector that reclaims cache entries based on TTL,
// reference count, file modification time, and memory pressure signals.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace ExplorerLens {
namespace Engine {

enum class GCTrigger : uint8_t {
    Timer = 0,          // Periodic interval
    MemoryPressure = 1, // System memory threshold hit
    CacheFull = 2,      // Cache budget exceeded
    Manual = 3          // Explicit invocation
};

enum class GCStrategy : uint8_t {
    LRU = 0,        // Least recently used
    LFU = 1,        // Least frequently used
    TTL = 2,        // Time-to-live expired first
    Size = 3,       // Largest entries first
    Hybrid = 4      // Weighted combination
};

struct GCConfig {
    GCStrategy strategy = GCStrategy::Hybrid;
    uint32_t intervalSeconds = 60;
    uint64_t targetFreeBytes = 64ULL * 1024 * 1024;
    uint32_t maxEntriesPerPass = 100;
    uint32_t ttlSeconds = 3600;
    bool compactAfterGC = true;
};

struct GCResult {
    GCTrigger trigger = GCTrigger::Timer;
    uint32_t entriesScanned = 0;
    uint32_t entriesReclaimed = 0;
    uint64_t bytesReclaimed = 0;
    double durationMs = 0.0;
    bool reachedTarget = false;
};

struct GCStats {
    uint32_t totalPasses = 0;
    uint64_t totalBytesReclaimed = 0;
    uint32_t totalEntriesReclaimed = 0;
    double avgPassDurationMs = 0.0;
    double totalGCTimeMs = 0.0;
};

class CacheGarbageCollector {
public:
    void Configure(const GCConfig& config) { m_config = config; }

    bool ShouldCollect(uint64_t freeBytes) const {
        return freeBytes < m_config.targetFreeBytes;
    }

    double ScoreEntry(uint64_t lastAccessMs, uint32_t accessCount,
        uint64_t sizeBytes, uint64_t nowMs) const {
        double ageScore = static_cast<double>(nowMs - lastAccessMs) / 1000.0;
        double freqScore = accessCount > 0 ? 1.0 / accessCount : 100.0;
        double sizeScore = static_cast<double>(sizeBytes) / (1024.0 * 1024.0);

        switch (m_config.strategy) {
        case GCStrategy::LRU:  return ageScore;
        case GCStrategy::LFU:  return freqScore;
        case GCStrategy::TTL:  return ageScore;
        case GCStrategy::Size: return sizeScore;
        case GCStrategy::Hybrid:
            return ageScore * 0.4 + freqScore * 0.3 + sizeScore * 0.3;
        }
        return ageScore;
    }

    void RecordPass(const GCResult& result) {
        m_stats.totalPasses++;
        m_stats.totalBytesReclaimed += result.bytesReclaimed;
        m_stats.totalEntriesReclaimed += result.entriesReclaimed;
        m_stats.totalGCTimeMs += result.durationMs;
        m_stats.avgPassDurationMs = m_stats.totalGCTimeMs / m_stats.totalPasses;
    }

    GCStats GetStats() const { return m_stats; }

private:
    GCConfig m_config;
    GCStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
