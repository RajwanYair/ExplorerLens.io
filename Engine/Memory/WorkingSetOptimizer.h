// WorkingSetOptimizer.h — Process Working Set Optimization
// Copyright (c) 2026 ExplorerLens Project
//
// Optimizes the process working set size by trimming unused pages,
// locking critical pages, and monitoring working set growth patterns.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class WorkingSetAction : uint8_t {
    None,
    Trim,
    Lock,
    Unlock,
    EmptyStandby,
    SetMinMax
};

struct OptimizerWSSnapshot
{
    uint64_t currentSizeBytes = 0;
    uint64_t peakSizeBytes = 0;
    uint64_t minSizeBytes = 0;
    uint64_t maxSizeBytes = 0;
    uint32_t pageFaultCount = 0;
    uint64_t timestamp = 0;
};

struct WorkingSetMetrics
{
    uint64_t trimCount = 0;
    uint64_t bytesTotalTrimmed = 0;
    uint64_t lockCount = 0;
    uint64_t snapshotCount = 0;
    double avgWorkingSetMB = 0.0;
    double peakWorkingSetMB = 0.0;
};

class WorkingSetOptimizer
{
  public:
    WorkingSetOptimizer() = default;

    void RecordSnapshot(const OptimizerWSSnapshot& snapshot)
    {
        m_snapshots.push_back(snapshot);
        m_metrics.snapshotCount++;
        m_totalBytes += snapshot.currentSizeBytes;
        m_metrics.avgWorkingSetMB = static_cast<double>(m_totalBytes) / m_metrics.snapshotCount / (1024.0 * 1024.0);
        double peakMB = snapshot.peakSizeBytes / (1024.0 * 1024.0);
        if (peakMB > m_metrics.peakWorkingSetMB)
            m_metrics.peakWorkingSetMB = peakMB;
    }

    bool ShouldTrim(uint64_t currentSizeBytes, uint64_t targetMaxBytes) const
    {
        return currentSizeBytes > targetMaxBytes;
    }

    uint64_t CalculateTrimAmount(uint64_t currentBytes, uint64_t targetBytes) const
    {
        if (currentBytes <= targetBytes)
            return 0;
        return currentBytes - targetBytes;
    }

    void RecordTrim(uint64_t bytesTrimed)
    {
        m_metrics.trimCount++;
        m_metrics.bytesTotalTrimmed += bytesTrimed;
    }

    void RecordLock()
    {
        m_metrics.lockCount++;
    }

    WorkingSetMetrics GetMetrics() const
    {
        return m_metrics;
    }

    OptimizerWSSnapshot GetLatestSnapshot() const
    {
        return m_snapshots.empty() ? OptimizerWSSnapshot{} : m_snapshots.back();
    }

    bool IsGrowthTrending() const
    {
        if (m_snapshots.size() < 5)
            return false;
        size_t n = m_snapshots.size();
        return m_snapshots[n - 1].currentSizeBytes > m_snapshots[n - 5].currentSizeBytes * 11 / 10;
    }

  private:
    std::vector<OptimizerWSSnapshot> m_snapshots;
    WorkingSetMetrics m_metrics;
    uint64_t m_totalBytes = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
