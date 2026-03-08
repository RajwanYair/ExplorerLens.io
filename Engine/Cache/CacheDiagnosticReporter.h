// CacheDiagnosticReporter.h — Cache health and performance reports
// Copyright (c) 2026 ExplorerLens Project
//
// Generates detailed cache diagnostic reports including hit rates,
// eviction patterns, memory usage, and performance bottlenecks.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct CacheDiagnosticReporterConfig {
    bool enabled = true;
    uint32_t reportIntervalSec = 60;
    std::string label = "CacheDiagnosticReporter";
};

class CacheDiagnosticReporter {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    CacheDiagnosticReporterConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct DiagnosticReport {
        uint64_t totalHits = 0;
        uint64_t totalMisses = 0;
        uint64_t evictions = 0;
        uint64_t memoryUsedBytes = 0;
        double hitRate = 0.0;
        std::string healthStatus;
    };

    void RecordHit() { m_hits++; }
    void RecordMiss() { m_misses++; }
    void RecordEviction() { m_evictions++; }

    DiagnosticReport GenerateReport() const {
        DiagnosticReport r;
        r.totalHits = m_hits;
        r.totalMisses = m_misses;
        r.evictions = m_evictions;
        uint64_t total = m_hits + m_misses;
        r.hitRate = total > 0 ? static_cast<double>(m_hits) / total : 0.0;
        r.healthStatus = r.hitRate > 0.8 ? "healthy" : (r.hitRate > 0.5 ? "degraded" : "poor");
        return r;
    }

private:
    bool m_initialized = false;
    CacheDiagnosticReporterConfig m_config;
    uint64_t m_hits = 0;
    uint64_t m_misses = 0;
    uint64_t m_evictions = 0;
};

}
} // namespace ExplorerLens::Engine
