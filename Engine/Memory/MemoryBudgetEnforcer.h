// MemoryBudgetEnforcer.h — Process Working-Set Budget Enforcement
// Copyright (c) 2026 ExplorerLens Project
//
// Monitors the process working set (via GetProcessMemoryInfo) against a
// configurable memory budget and enforces a four-tier response ladder:
// Permissive (warnings only), Moderate (reject new allocations), Strict
// (trigger cache eviction), Emergency (compact + evict). CanAllocate()
// is the primary gate, intended to be called before large allocations.
//
// Thread-safe singleton.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <psapi.h>
#include <windows.h>
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class BudgetEnforcementLevel : uint32_t {
    Permissive = 0,  // Log warnings only
    Moderate = 1,    // Reject new allocations, don't free existing
    Strict = 2,      // Reject + trigger cache eviction
    Emergency = 3    // Reject + evict + compact
};

struct MemoryBudgetStatus
{
    uint64_t budgetBytes = 0;
    uint64_t currentWorkingSet = 0;
    uint64_t peakWorkingSet = 0;
    double utilizationPct = 0.0;
    BudgetEnforcementLevel level = BudgetEnforcementLevel::Permissive;
    uint64_t rejectedAllocations = 0;
    uint64_t evictionRequests = 0;
    bool overBudget = false;
};

struct BudgetEnforcerConfig
{
    uint64_t budgetBytes = 512ULL * 1024 * 1024;  // 512 MB
    double warningThreshold = 0.70;
    double moderateThreshold = 0.85;
    double strictThreshold = 0.95;
    double emergencyThreshold = 1.00;
    bool enableAutoEviction = true;
    uint32_t pollingIntervalMs = 1000;
};

// ========================================================================
// MemoryBudgetEnforcer — Working-set budget enforcement
// ========================================================================
class MemoryBudgetEnforcer
{
  public:
    static MemoryBudgetEnforcer& Instance()
    {
        static MemoryBudgetEnforcer instance;
        return instance;
    }

    void Initialize(const BudgetEnforcerConfig& config = {})
    {
        m_config = config;
        m_rejectedCount.store(0, std::memory_order_relaxed);
        m_evictionRequests.store(0, std::memory_order_relaxed);
        m_peakWorkingSet = 0;
        m_initialized = true;
    }

    bool IsInitialized() const
    {
        return m_initialized;
    }

    // Check if an allocation of given size should be allowed
    bool CanAllocate(uint64_t requestedBytes)
    {
        uint64_t currentWS = GetCurrentWorkingSet();
        BudgetEnforcementLevel level = ComputeLevel(currentWS);

        if (level == BudgetEnforcementLevel::Permissive) {
            return true;
        }

        if (currentWS + requestedBytes > m_config.budgetBytes) {
            m_rejectedCount.fetch_add(1, std::memory_order_relaxed);
            return false;
        }

        return (level < BudgetEnforcementLevel::Strict);
    }

    // Get current enforcement level
    BudgetEnforcementLevel GetEnforcementLevel()
    {
        uint64_t ws = GetCurrentWorkingSet();
        return ComputeLevel(ws);
    }

    // Check if eviction is needed
    bool ShouldEvict()
    {
        uint64_t ws = GetCurrentWorkingSet();
        BudgetEnforcementLevel level = ComputeLevel(ws);
        if (level >= BudgetEnforcementLevel::Strict && m_config.enableAutoEviction) {
            m_evictionRequests.fetch_add(1, std::memory_order_relaxed);
            return true;
        }
        return false;
    }

    // Get current working set
    uint64_t GetCurrentWorkingSet()
    {
        PROCESS_MEMORY_COUNTERS_EX pmc = {};
        pmc.cb = sizeof(pmc);
        if (GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
            uint64_t ws = pmc.WorkingSetSize;
            if (ws > m_peakWorkingSet)
                m_peakWorkingSet = ws;
            return ws;
        }
        return 0;
    }

    // Get status
    MemoryBudgetStatus GetStatus()
    {
        MemoryBudgetStatus status;
        status.budgetBytes = m_config.budgetBytes;
        status.currentWorkingSet = GetCurrentWorkingSet();
        status.peakWorkingSet = m_peakWorkingSet;
        status.utilizationPct =
            (m_config.budgetBytes > 0)
                ? (static_cast<double>(status.currentWorkingSet) / static_cast<double>(m_config.budgetBytes) * 100.0)
                : 0.0;
        status.level = ComputeLevel(status.currentWorkingSet);
        status.rejectedAllocations = m_rejectedCount.load(std::memory_order_relaxed);
        status.evictionRequests = m_evictionRequests.load(std::memory_order_relaxed);
        status.overBudget = (status.currentWorkingSet > m_config.budgetBytes);
        return status;
    }

    // Update budget
    void SetBudget(uint64_t bytes)
    {
        m_config.budgetBytes = bytes;
    }

    // Get budget
    uint64_t GetBudget() const
    {
        return m_config.budgetBytes;
    }

  private:
    MemoryBudgetEnforcer() = default;

    BudgetEnforcementLevel ComputeLevel(uint64_t workingSet) const
    {
        double ratio = (m_config.budgetBytes > 0)
                           ? (static_cast<double>(workingSet) / static_cast<double>(m_config.budgetBytes))
                           : 0.0;

        if (ratio >= m_config.emergencyThreshold)
            return BudgetEnforcementLevel::Emergency;
        if (ratio >= m_config.strictThreshold)
            return BudgetEnforcementLevel::Strict;
        if (ratio >= m_config.moderateThreshold)
            return BudgetEnforcementLevel::Moderate;
        return BudgetEnforcementLevel::Permissive;
    }

    BudgetEnforcerConfig m_config;
    std::atomic<uint64_t> m_rejectedCount{0};
    std::atomic<uint64_t> m_evictionRequests{0};
    uint64_t m_peakWorkingSet = 0;
    bool m_initialized = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
