// PerformanceReportGenerator.h — Performance Summary Report Builder
// Copyright (c) 2026 ExplorerLens Project
//
// Collects performance data from all subsystems and generates structured
// performance reports for diagnostics and benchmarking.
//
#pragma once

#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct PerfSection
{
    std::string name;
    double avgMs = 0.0;
    double p50Ms = 0.0;
    double p95Ms = 0.0;
    double p99Ms = 0.0;
    uint64_t invocations = 0;
    double throughputPerSec = 0.0;
};

struct SystemPerfSnapshot
{
    double cpuPercent = 0.0;
    uint64_t memoryUsedMB = 0;
    uint64_t gpuMemoryUsedMB = 0;
    uint32_t activeThreads = 0;
    double diskReadMBps = 0.0;
};

struct PerformanceReport
{
    std::string reportId;
    uint64_t timestampMs = 0;
    uint64_t durationMs = 0;
    std::vector<PerfSection> sections;
    SystemPerfSnapshot systemSnapshot;
    uint32_t totalDecodes = 0;
    uint32_t cacheHits = 0;
    double overallThroughput = 0.0;

    std::string ToSummaryString() const
    {
        std::ostringstream ss;
        ss << "=== Performance Report ===\n";
        ss << "Duration: " << durationMs << "ms | Decodes: " << totalDecodes << " | Cache Hits: " << cacheHits << "\n";
        ss << "CPU: " << systemSnapshot.cpuPercent << "% | RAM: " << systemSnapshot.memoryUsedMB << "MB\n";
        for (const auto& s : sections) {
            ss << "  " << s.name << ": avg=" << s.avgMs << "ms p95=" << s.p95Ms << "ms (" << s.invocations
               << " calls)\n";
        }
        return ss.str();
    }
};

class PerformanceReportGenerator
{
  public:
    void BeginReport(const std::string& id)
    {
        m_report = {};
        m_report.reportId = id;
    }

    void AddSection(const PerfSection& section)
    {
        m_report.sections.push_back(section);
    }

    void SetSystemSnapshot(const SystemPerfSnapshot& snap)
    {
        m_report.systemSnapshot = snap;
    }

    void SetDecodeStats(uint32_t totalDecodes, uint32_t cacheHits)
    {
        m_report.totalDecodes = totalDecodes;
        m_report.cacheHits = cacheHits;
    }

    PerformanceReport Finalize(uint64_t durationMs)
    {
        m_report.durationMs = durationMs;
        if (durationMs > 0) {
            m_report.overallThroughput = m_report.totalDecodes * 1000.0 / durationMs;
        }
        return m_report;
    }

    bool MeetsTargets(const PerformanceReport& report, double maxAvgMs, double minThroughput) const
    {
        for (const auto& s : report.sections) {
            if (s.avgMs > maxAvgMs)
                return false;
        }
        return report.overallThroughput >= minThroughput;
    }

  private:
    PerformanceReport m_report;
};

}  // namespace Engine
}  // namespace ExplorerLens
