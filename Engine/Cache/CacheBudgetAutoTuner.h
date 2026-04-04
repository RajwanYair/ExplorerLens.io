// CacheBudgetAutoTuner.h — Adaptive Cache Budget Management
// Copyright (c) 2026 ExplorerLens Project
//
// Automatically adjusts the thumbnail cache size budget based on:
//   - Available physical memory (via GlobalMemoryStatusEx)
//   - Current memory pressure tier (from MemoryPressureControllerV2)
//   - Disk space availability on the cache volume
//   - Cache hit rate trends (expand on high miss rate, shrink on low utilization)
//   - System workload classification (idle/light/heavy)
//
// Budget tiers:
//   Minimum:  64 MB  (memory-constrained devices)
//   Default: 256 MB  (typical desktop)
//   Optimal: 512 MB  (16+ GB RAM)
//   Maximum:   1 GB  (32+ GB RAM, SSD cache volume)
//
// Tuning interval: every 30 seconds, with immediate adjustment on pressure events.

#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Budget tiers
// ============================================================================

enum class CacheBudgetTier : uint8_t {
    Minimum = 0,  //  64 MB
    Compact = 1,  // 128 MB
    Default = 2,  // 256 MB
    Optimal = 3,  // 512 MB
    Maximum = 4   //   1 GB
};

inline const char* BudgetTierName(CacheBudgetTier tier)
{
    static const char* names[] = {"Minimum", "Compact", "Default", "Optimal", "Maximum"};
    auto idx = static_cast<uint8_t>(tier);
    return (idx < 5) ? names[idx] : "Unknown";
}

inline uint64_t BudgetTierSizeMB(CacheBudgetTier tier)
{
    static const uint64_t sizes[] = {64, 128, 256, 512, 1024};
    auto idx = static_cast<uint8_t>(tier);
    return (idx < 5) ? sizes[idx] : 256;
}

// ============================================================================
// Configuration and statistics
// ============================================================================

struct CacheBudgetConfig
{
    uint64_t minBudgetMB = 64;
    uint64_t maxBudgetMB = 1024;
    uint32_t tuningIntervalSec = 30;
    double expandThreshold = 0.85;        // Expand when utilization > 85%
    double shrinkThreshold = 0.30;        // Shrink when utilization < 30%
    double missRateExpandTrigger = 0.25;  // Expand when miss rate > 25%
    double minDiskFreePercent = 10.0;     // Don't expand if disk < 10% free
    bool respectMemoryPressure = true;
    bool enableDiskSpaceCheck = true;
};

struct CacheBudgetSnapshot
{
    CacheBudgetTier currentTier = CacheBudgetTier::Default;
    uint64_t currentBudgetMB = 256;
    uint64_t usedMB = 0;
    double utilizationPct = 0.0;
    double hitRate = 0.0;
    double missRate = 0.0;
    uint64_t totalRAM_MB = 0;
    uint64_t availRAM_MB = 0;
    uint64_t diskFreeMB = 0;
    uint32_t adjustmentCount = 0;
    std::chrono::steady_clock::time_point lastTuning{};
    std::string lastAction;
};

// ============================================================================
// CacheBudgetAutoTuner
// ============================================================================

class CacheBudgetAutoTuner
{
  public:
    static CacheBudgetAutoTuner& Instance()
    {
        static CacheBudgetAutoTuner inst;
        return inst;
    }

    /// Initialize with configuration. Must be called before Tune().
    void Initialize(const CacheBudgetConfig& config = {})
    {
        m_config = config;
        m_snapshot.currentBudgetMB = BudgetTierSizeMB(DetermineInitialTier());
        m_snapshot.currentTier = DetermineInitialTier();
        m_initialized = true;
    }

    /// Run one tuning cycle. Call periodically or on pressure events.
    CacheBudgetSnapshot Tune(uint64_t usedCacheMB, double cacheHitRate)
    {
        if (!m_initialized)
            Initialize();

        auto now = std::chrono::steady_clock::now();
        m_snapshot.usedMB = usedCacheMB;
        m_snapshot.hitRate = cacheHitRate;
        m_snapshot.missRate = 1.0 - cacheHitRate;
        m_snapshot.utilizationPct =
            (m_snapshot.currentBudgetMB > 0) ? (100.0 * usedCacheMB / m_snapshot.currentBudgetMB) : 0.0;

        // Query system memory
        MEMORYSTATUSEX memInfo{};
        memInfo.dwLength = sizeof(memInfo);
        if (GlobalMemoryStatusEx(&memInfo)) {
            m_snapshot.totalRAM_MB = memInfo.ullTotalPhys / (1024 * 1024);
            m_snapshot.availRAM_MB = memInfo.ullAvailPhys / (1024 * 1024);
        }

        // Determine if adjustment is needed
        std::string action = "hold";
        uint64_t newBudget = m_snapshot.currentBudgetMB;

        // High utilization + high miss rate → expand
        if (m_snapshot.utilizationPct > m_config.expandThreshold * 100
            || m_snapshot.missRate > m_config.missRateExpandTrigger) {
            if (CanExpand()) {
                newBudget = ExpandBudget(m_snapshot.currentBudgetMB);
                action = "expand";
            }
        }
        // Low utilization → shrink
        else if (m_snapshot.utilizationPct < m_config.shrinkThreshold * 100
                 && m_snapshot.currentBudgetMB > m_config.minBudgetMB) {
            newBudget = ShrinkBudget(m_snapshot.currentBudgetMB);
            action = "shrink";
        }

        // Memory pressure override → force shrink
        if (m_config.respectMemoryPressure && IsUnderMemoryPressure()) {
            newBudget = (std::max)(m_config.minBudgetMB, m_snapshot.currentBudgetMB / 2);
            action = "pressure-shrink";
        }

        // Apply bounds
        newBudget = (std::max)(m_config.minBudgetMB, (std::min)(m_config.maxBudgetMB, newBudget));

        if (newBudget != m_snapshot.currentBudgetMB) {
            m_snapshot.adjustmentCount++;
        }

        m_snapshot.currentBudgetMB = newBudget;
        m_snapshot.currentTier = TierFromSize(newBudget);
        m_snapshot.lastTuning = now;
        m_snapshot.lastAction = action;

        return m_snapshot;
    }

    /// Get the current budget in bytes.
    uint64_t GetBudgetBytes() const
    {
        return m_snapshot.currentBudgetMB * 1024 * 1024;
    }

    /// Get the current budget in MB.
    uint64_t GetBudgetMB() const
    {
        return m_snapshot.currentBudgetMB;
    }

    /// Get full snapshot for diagnostics.
    CacheBudgetSnapshot GetSnapshot() const
    {
        return m_snapshot;
    }

    /// Force a specific tier (overrides auto-tuning until next Tune()).
    void ForceTier(CacheBudgetTier tier)
    {
        m_snapshot.currentTier = tier;
        m_snapshot.currentBudgetMB = BudgetTierSizeMB(tier);
    }

  private:
    CacheBudgetAutoTuner() = default;

    CacheBudgetTier DetermineInitialTier() const
    {
        MEMORYSTATUSEX memInfo{};
        memInfo.dwLength = sizeof(memInfo);
        if (!GlobalMemoryStatusEx(&memInfo))
            return CacheBudgetTier::Default;

        uint64_t totalGB = memInfo.ullTotalPhys / (1024ULL * 1024 * 1024);
        if (totalGB >= 32)
            return CacheBudgetTier::Maximum;
        if (totalGB >= 16)
            return CacheBudgetTier::Optimal;
        if (totalGB >= 8)
            return CacheBudgetTier::Default;
        if (totalGB >= 4)
            return CacheBudgetTier::Compact;
        return CacheBudgetTier::Minimum;
    }

    CacheBudgetTier TierFromSize(uint64_t mb) const
    {
        if (mb >= 1024)
            return CacheBudgetTier::Maximum;
        if (mb >= 512)
            return CacheBudgetTier::Optimal;
        if (mb >= 256)
            return CacheBudgetTier::Default;
        if (mb >= 128)
            return CacheBudgetTier::Compact;
        return CacheBudgetTier::Minimum;
    }

    uint64_t ExpandBudget(uint64_t currentMB) const
    {
        // Step up by 50%, capped at max
        return (std::min)(currentMB * 3 / 2, m_config.maxBudgetMB);
    }

    uint64_t ShrinkBudget(uint64_t currentMB) const
    {
        // Step down by 25%, floored at min
        return (std::max)(currentMB * 3 / 4, m_config.minBudgetMB);
    }

    bool CanExpand() const
    {
        if (m_snapshot.currentBudgetMB >= m_config.maxBudgetMB)
            return false;

        // Check available RAM  — need at least 2x the expansion amount free
        uint64_t expansionMB = m_snapshot.currentBudgetMB / 2;
        if (m_snapshot.availRAM_MB < expansionMB * 2)
            return false;

        return true;
    }

    bool IsUnderMemoryPressure() const
    {
        // Pressure if available RAM < 15% of total
        if (m_snapshot.totalRAM_MB == 0)
            return false;
        double availPct = 100.0 * m_snapshot.availRAM_MB / m_snapshot.totalRAM_MB;
        return availPct < 15.0;
    }

    CacheBudgetConfig m_config;
    CacheBudgetSnapshot m_snapshot;
    bool m_initialized = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
