// CacheDiagnostics.h — Cache Health Dashboard
// Copyright (c) 2026 ExplorerLens Project
//
// Real-time health dashboard for the thumbnail cache. Hit rate, eviction
// pressure, and memory utilization are combined to compute a five-tier
// health classification (Excellent -> Critical). Atomic counters allow
// thread-safe recording of cache events, and GenerateReport() produces
// a timestamped snapshot of all diagnostic metrics.
//
// Thread-safe singleton.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <atomic>
#include <cmath>

namespace ExplorerLens {
namespace Engine {

enum class CacheHealthTier : uint32_t {
    Excellent = 0,  // Hit rate > 90%, no pressure
    Good = 1,  // Hit rate > 70%, low pressure
    Fair = 2,  // Hit rate > 50%, moderate pressure
    Poor = 3,  // Hit rate > 30%, high pressure
    Critical = 4   // Hit rate < 30% or OOM imminent
};

static const wchar_t* CacheHealthTierName(CacheHealthTier t) {
    static const wchar_t* names[] = {
        L"Excellent", L"Good", L"Fair", L"Poor", L"Critical"
    };
    return names[static_cast<uint32_t>(t)];
}

struct CachePartitionInfo {
    std::wstring name;
    uint64_t     capacityBytes = 0;
    uint64_t     usedBytes = 0;
    uint32_t     entryCount = 0;
    uint32_t     evictionCount = 0;
    double       hitRate = 0.0;

    double GetUtilization() const {
        return (capacityBytes > 0) ? (static_cast<double>(usedBytes) / static_cast<double>(capacityBytes)) : 0.0;
    }
};

struct CacheDiagnosticReport {
    CacheHealthTier  healthTier = CacheHealthTier::Good;
    double           overallHitRate = 0.0;
    double           overallMissRate = 0.0;
    uint64_t         totalHits = 0;
    uint64_t         totalMisses = 0;
    uint64_t         totalEvictions = 0;
    uint64_t         totalInsertions = 0;
    double           evictionPressure = 0.0; // 0.0 - 1.0
    uint64_t         totalMemoryBytes = 0;
    uint64_t         usedMemoryBytes = 0;
    double           memoryUtilization = 0.0;
    uint32_t         staleEntryCount = 0;
    double           avgEntryAgeMs = 0.0;
    uint64_t         reportTimestamp = 0;
    std::vector<CachePartitionInfo> partitions;
};

// ========================================================================
// CacheDiagnostics — Cache health monitoring and analysis
// ========================================================================
class CacheDiagnostics {
public:
    static CacheDiagnostics& Instance() {
        static CacheDiagnostics instance;
        return instance;
    }

    void Initialize(uint64_t totalCacheMemory = 256ULL * 1024 * 1024) {
        m_totalMemory = totalCacheMemory;
        Reset();
        m_initialized = true;
    }

    bool IsInitialized() const { return m_initialized; }

    // Record cache events
    void RecordHit() { m_hits.fetch_add(1, std::memory_order_relaxed); }
    void RecordMiss() { m_misses.fetch_add(1, std::memory_order_relaxed); }
    void RecordEviction() { m_evictions.fetch_add(1, std::memory_order_relaxed); }
    void RecordInsertion(uint64_t sizeBytes) {
        m_insertions.fetch_add(1, std::memory_order_relaxed);
        m_usedMemory.fetch_add(sizeBytes, std::memory_order_relaxed);
    }
    void RecordRemoval(uint64_t sizeBytes) {
        uint64_t used = m_usedMemory.load(std::memory_order_relaxed);
        m_usedMemory.store((sizeBytes <= used) ? (used - sizeBytes) : 0, std::memory_order_relaxed);
    }

    // Get current hit rate
    double GetHitRate() const {
        uint64_t h = m_hits.load(std::memory_order_relaxed);
        uint64_t m = m_misses.load(std::memory_order_relaxed);
        uint64_t total = h + m;
        return (total > 0) ? (static_cast<double>(h) / static_cast<double>(total)) : 0.0;
    }

    // Get eviction pressure (evictions relative to insertions)
    double GetEvictionPressure() const {
        uint64_t e = m_evictions.load(std::memory_order_relaxed);
        uint64_t i = m_insertions.load(std::memory_order_relaxed);
        return (i > 0) ? (static_cast<double>(e) / static_cast<double>(i)) : 0.0;
    }

    // Compute health tier
    CacheHealthTier ComputeHealthTier() const {
        double hitRate = GetHitRate();
        double pressure = GetEvictionPressure();
        double memUtil = GetMemoryUtilization();

        if (hitRate > 0.90 && pressure < 0.1 && memUtil < 0.85)
            return CacheHealthTier::Excellent;
        if (hitRate > 0.70 && pressure < 0.3)
            return CacheHealthTier::Good;
        if (hitRate > 0.50 && pressure < 0.6)
            return CacheHealthTier::Fair;
        if (hitRate > 0.30)
            return CacheHealthTier::Poor;
        return CacheHealthTier::Critical;
    }

    // Get memory utilization
    double GetMemoryUtilization() const {
        return (m_totalMemory > 0)
            ? (static_cast<double>(m_usedMemory.load(std::memory_order_relaxed)) / static_cast<double>(m_totalMemory))
            : 0.0;
    }

    // Generate full diagnostic report
    CacheDiagnosticReport GenerateReport() const {
        CacheDiagnosticReport report;
        report.totalHits = m_hits.load(std::memory_order_relaxed);
        report.totalMisses = m_misses.load(std::memory_order_relaxed);
        report.totalEvictions = m_evictions.load(std::memory_order_relaxed);
        report.totalInsertions = m_insertions.load(std::memory_order_relaxed);

        uint64_t totalAccess = report.totalHits + report.totalMisses;
        report.overallHitRate = (totalAccess > 0) ? (static_cast<double>(report.totalHits) / static_cast<double>(totalAccess)) : 0.0;
        report.overallMissRate = 1.0 - report.overallHitRate;

        report.evictionPressure = GetEvictionPressure();
        report.totalMemoryBytes = m_totalMemory;
        report.usedMemoryBytes = m_usedMemory.load(std::memory_order_relaxed);
        report.memoryUtilization = GetMemoryUtilization();
        report.healthTier = ComputeHealthTier();

        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        report.reportTimestamp = (static_cast<uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;

        return report;
    }

    // Reset all counters
    void Reset() {
        m_hits.store(0, std::memory_order_relaxed);
        m_misses.store(0, std::memory_order_relaxed);
        m_evictions.store(0, std::memory_order_relaxed);
        m_insertions.store(0, std::memory_order_relaxed);
        m_usedMemory.store(0, std::memory_order_relaxed);
    }

private:
    CacheDiagnostics() = default;

    std::atomic<uint64_t> m_hits{ 0 };
    std::atomic<uint64_t> m_misses{ 0 };
    std::atomic<uint64_t> m_evictions{ 0 };
    std::atomic<uint64_t> m_insertions{ 0 };
    std::atomic<uint64_t> m_usedMemory{ 0 };
    uint64_t m_totalMemory = 256ULL * 1024 * 1024;
    bool m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
