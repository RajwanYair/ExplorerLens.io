// CacheStatisticsExporter.h — Cache Metrics Export Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Aggregates cache performance metrics and exports them in structured
// formats (JSON, CSV, ETW) for monitoring and diagnostics.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <sstream>

namespace ExplorerLens {
namespace Engine {

enum class CacheExportFormat : uint8_t {
    JSON = 0,
    CSV = 1,
    PlainText = 2
};

struct CacheMetricSnapshot {
    uint64_t timestampMs = 0;
    uint64_t totalBytes = 0;
    uint64_t usedBytes = 0;
    uint32_t entryCount = 0;
    uint32_t hitCount = 0;
    uint32_t missCount = 0;
    uint32_t evictionCount = 0;
    double avgLookupUs = 0.0;
    double p99LookupUs = 0.0;

    double HitRate() const {
        uint32_t total = hitCount + missCount;
        return total > 0 ? 100.0 * hitCount / total : 0.0;
    }
    double Utilization() const {
        return totalBytes > 0 ? 100.0 * usedBytes / totalBytes : 0.0;
    }
};

class CacheStatisticsExporter {
public:
    void RecordSnapshot(const CacheMetricSnapshot& snapshot) {
        m_snapshots.push_back(snapshot);
        if (m_snapshots.size() > m_maxSnapshots)
            m_snapshots.erase(m_snapshots.begin());
    }

    std::string Export(CacheExportFormat format) const {
        switch (format) {
        case CacheExportFormat::JSON: return ExportJSON();
        case CacheExportFormat::CSV: return ExportCSV();
        case CacheExportFormat::PlainText: return ExportText();
        }
        return {};
    }

    CacheMetricSnapshot Latest() const {
        return m_snapshots.empty() ? CacheMetricSnapshot{} : m_snapshots.back();
    }

    size_t SnapshotCount() const { return m_snapshots.size(); }
    void SetMaxSnapshots(size_t max) { m_maxSnapshots = max; }

private:
    std::string ExportJSON() const {
        std::ostringstream ss;
        ss << "[";
        for (size_t i = 0; i < m_snapshots.size(); ++i) {
            const auto& s = m_snapshots[i];
            if (i > 0) ss << ",";
            ss << "{\"ts\":" << s.timestampMs
                << ",\"used\":" << s.usedBytes
                << ",\"entries\":" << s.entryCount
                << ",\"hitRate\":" << s.HitRate() << "}";
        }
        ss << "]";
        return ss.str();
    }

    std::string ExportCSV() const {
        std::ostringstream ss;
        ss << "timestamp,usedBytes,entries,hitRate,avgLookupUs\n";
        for (const auto& s : m_snapshots) {
            ss << s.timestampMs << "," << s.usedBytes << ","
                << s.entryCount << "," << s.HitRate() << ","
                << s.avgLookupUs << "\n";
        }
        return ss.str();
    }

    std::string ExportText() const {
        if (m_snapshots.empty()) return "No cache data available.\n";
        const auto& s = m_snapshots.back();
        std::ostringstream ss;
        ss << "Cache: " << s.usedBytes / (1024 * 1024) << "MB / "
            << s.totalBytes / (1024 * 1024) << "MB ("
            << s.Utilization() << "%) | "
            << s.entryCount << " entries | "
            << s.HitRate() << "% hit rate";
        return ss.str();
    }

    std::vector<CacheMetricSnapshot> m_snapshots;
    size_t m_maxSnapshots = 1000;
};

} // namespace Engine
} // namespace ExplorerLens
