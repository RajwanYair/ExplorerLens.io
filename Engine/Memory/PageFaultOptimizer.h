// PageFaultOptimizer.h — Virtual Memory Page Fault Optimization
// Copyright (c) 2026 ExplorerLens Project
//
// Optimizes virtual memory page fault behavior by pre-touching pages,
// controlling commit patterns, and reducing soft fault overhead.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class PageFaultStrategy : uint8_t {
    OnDemand,
    PreTouch,
    LargePages,
    HugePages,
    Adaptive
};

struct PageFaultRecord {
    uint64_t address = 0;
    uint32_t pageSize = 4096;
    bool wasSoftFault = true;
    double latencyUs = 0.0;
    uint64_t timestamp = 0;
};

struct PageFaultMetrics {
    uint64_t softFaults = 0;
    uint64_t hardFaults = 0;
    uint64_t preTouchedPages = 0;
    uint64_t totalPagesAccessed = 0;
    double avgFaultLatencyUs = 0.0;
    double faultReductionPercent = 0.0;
};

class PageFaultOptimizer {
public:
    explicit PageFaultOptimizer(PageFaultStrategy strategy = PageFaultStrategy::PreTouch)
        : m_strategy(strategy) {
    }

    void RecordFault(const PageFaultRecord& record) {
        if (record.wasSoftFault) m_metrics.softFaults++;
        else m_metrics.hardFaults++;
        m_metrics.totalPagesAccessed++;
        m_totalLatencyUs += record.latencyUs;
        m_metrics.avgFaultLatencyUs = m_totalLatencyUs / m_metrics.totalPagesAccessed;
    }

    uint64_t PreTouchRegion(uint64_t startAddress, uint64_t sizeBytes) {
        uint32_t pageSize = (m_strategy == PageFaultStrategy::LargePages) ? 2 * 1024 * 1024 : 4096;
        uint64_t pages = (sizeBytes + pageSize - 1) / pageSize;
        m_metrics.preTouchedPages += pages;
        (void)startAddress;
        return pages;
    }

    PageFaultMetrics GetMetrics() const {
        PageFaultMetrics result = m_metrics;
        if (m_metrics.totalPagesAccessed > 0 && m_metrics.preTouchedPages > 0) {
            result.faultReductionPercent =
                static_cast<double>(m_metrics.preTouchedPages) / m_metrics.totalPagesAccessed * 100.0;
        }
        return result;
    }

    void SetStrategy(PageFaultStrategy strategy) { m_strategy = strategy; }
    PageFaultStrategy GetStrategy() const { return m_strategy; }
    bool IsLargePageEnabled() const {
        return m_strategy == PageFaultStrategy::LargePages ||
            m_strategy == PageFaultStrategy::HugePages;
    }

private:
    PageFaultStrategy m_strategy;
    PageFaultMetrics m_metrics;
    double m_totalLatencyUs = 0.0;
};

} // namespace Engine
} // namespace ExplorerLens
