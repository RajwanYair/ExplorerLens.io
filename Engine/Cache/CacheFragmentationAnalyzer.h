// CacheFragmentationAnalyzer.h — Cache Fragmentation Analysis
// Copyright (c) 2026 ExplorerLens Project
//
// Analyzes persistent cache file fragmentation and internal waste ratio.
// Triggers background compaction when efficiency drops below the configured
// threshold.  Tracks hole sizes and allocation patterns.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <mutex>
#include <numeric>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Fragmentation severity levels
enum class FragmentationLevel : uint8_t {
    None = 0,   // < 5% waste
    Low = 1,   // 5-15% waste
    Moderate = 2,   // 15-30% waste
    High = 3,   // 30-50% waste
    Critical = 4    // > 50% waste
};

/// Configuration for fragmentation analysis
struct FragmentationConfig {
    double   compactionThreshold = 0.30;   // Trigger compaction at 30% waste
    uint32_t minHoleSizeBytes = 64;     // Ignore holes smaller than this
    uint32_t analysisIntervalSec = 300;    // Analyze every 5 minutes
    bool     autoCompact = true;
};

/// A contiguous free region in the cache file
struct CacheHole {
    uint64_t offset = 0;
    uint64_t sizeBytes = 0;
};

/// Result of a fragmentation analysis pass
struct FragmentationReport {
    FragmentationLevel level = FragmentationLevel::None;
    uint64_t totalSizeBytes = 0;
    uint64_t usedBytes = 0;
    uint64_t wastedBytes = 0;
    double   wasteRatio = 0.0;
    uint32_t holeCount = 0;
    uint64_t largestHoleBytes = 0;
    uint64_t smallestHoleBytes = 0;
    double   avgHoleSizeBytes = 0.0;
    bool     compactionNeeded = false;
    uint32_t analysisTimeMs = 0;
};

/// Statistics over time
struct FragmentationStats {
    uint64_t totalAnalyses = 0;
    uint64_t compactionsTriggered = 0;
    uint64_t bytesReclaimed = 0;
    double   avgWasteRatio = 0.0;
};

/// Cache fragmentation analyzer with hole tracking
class CacheFragmentationAnalyzer {
public:
    explicit CacheFragmentationAnalyzer(const FragmentationConfig& config = {})
        : m_config(config) {
    }

    /// Register an allocation in the cache
    void RecordAllocation(uint64_t offset, uint64_t size) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_allocations.push_back({ offset, size });
        m_totalAllocated += size;
    }

    /// Register a deallocation (creates a hole)
    void RecordDeallocation(uint64_t offset, uint64_t size) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_holes.push_back({ offset, size });
        if (m_totalAllocated >= size) m_totalAllocated -= size;
        m_totalFreed += size;
    }

    /// Run fragmentation analysis on current state
    FragmentationReport Analyze(uint64_t totalCacheSizeBytes) {
        std::lock_guard<std::mutex> lock(m_mutex);
        FragmentationReport report;
        report.totalSizeBytes = totalCacheSizeBytes;

        // Merge adjacent holes
        MergeAdjacentHoles();

        // Filter small holes
        std::vector<CacheHole> significantHoles;
        for (const auto& hole : m_holes) {
            if (hole.sizeBytes >= m_config.minHoleSizeBytes)
                significantHoles.push_back(hole);
        }

        // Calculate waste
        uint64_t wastedBytes = 0;
        for (const auto& h : significantHoles)
            wastedBytes += h.sizeBytes;

        report.usedBytes = totalCacheSizeBytes - wastedBytes;
        report.wastedBytes = wastedBytes;
        report.wasteRatio = (totalCacheSizeBytes > 0) ?
            static_cast<double>(wastedBytes) / static_cast<double>(totalCacheSizeBytes) : 0.0;

        report.holeCount = static_cast<uint32_t>(significantHoles.size());

        if (!significantHoles.empty()) {
            auto minmax = std::minmax_element(significantHoles.begin(), significantHoles.end(),
                [](const CacheHole& a, const CacheHole& b) { return a.sizeBytes < b.sizeBytes; });
            report.smallestHoleBytes = minmax.first->sizeBytes;
            report.largestHoleBytes = minmax.second->sizeBytes;
            report.avgHoleSizeBytes = static_cast<double>(wastedBytes) / significantHoles.size();
        }

        // Classify severity
        if (report.wasteRatio < 0.05)       report.level = FragmentationLevel::None;
        else if (report.wasteRatio < 0.15)  report.level = FragmentationLevel::Low;
        else if (report.wasteRatio < 0.30)  report.level = FragmentationLevel::Moderate;
        else if (report.wasteRatio < 0.50)  report.level = FragmentationLevel::High;
        else                                report.level = FragmentationLevel::Critical;

        report.compactionNeeded = (report.wasteRatio >= m_config.compactionThreshold);

        // Update running stats
        m_stats.totalAnalyses++;
        m_stats.avgWasteRatio = m_stats.avgWasteRatio * 0.9 + report.wasteRatio * 0.1;

        return report;
    }

    /// Simulate compaction (returns bytes that would be reclaimed)
    uint64_t EstimateCompactionSavings() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint64_t savings = 0;
        for (const auto& h : m_holes)
            if (h.sizeBytes >= m_config.minHoleSizeBytes)
                savings += h.sizeBytes;
        return savings;
    }

    /// Clear all tracking after compaction
    void NotifyCompactionComplete(uint64_t bytesReclaimed) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_holes.clear();
        m_stats.compactionsTriggered++;
        m_stats.bytesReclaimed += bytesReclaimed;
    }

    FragmentationStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    const FragmentationConfig& GetConfig() const { return m_config; }

private:
    void MergeAdjacentHoles() {
        if (m_holes.size() < 2) return;
        std::sort(m_holes.begin(), m_holes.end(),
            [](const CacheHole& a, const CacheHole& b) { return a.offset < b.offset; });

        std::vector<CacheHole> merged;
        merged.push_back(m_holes[0]);
        for (size_t i = 1; i < m_holes.size(); ++i) {
            auto& last = merged.back();
            if (m_holes[i].offset <= last.offset + last.sizeBytes) {
                uint64_t end = (std::max)(last.offset + last.sizeBytes,
                    m_holes[i].offset + m_holes[i].sizeBytes);
                last.sizeBytes = end - last.offset;
            }
            else {
                merged.push_back(m_holes[i]);
            }
        }
        m_holes = std::move(merged);
    }

    FragmentationConfig         m_config;
    mutable std::mutex          m_mutex;
    std::vector<CacheHole>      m_holes;
    std::vector<CacheHole>      m_allocations;
    uint64_t                    m_totalAllocated = 0;
    uint64_t                    m_totalFreed = 0;
    FragmentationStats          m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
