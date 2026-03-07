// ScopedMemoryBudget.h — RAII Memory Budget Enforcer
// Copyright (c) 2026 ExplorerLens Project
//
// Provides scoped memory budget enforcement that tracks cumulative
// allocations within a scope and asserts budget compliance on exit.
//
#pragma once

#include <cstdint>
#include <string>
#include <atomic>

namespace ExplorerLens {
namespace Engine {

struct BudgetViolation {
    std::string scopeName;
    uint64_t budgetBytes = 0;
    uint64_t actualBytes = 0;
    uint64_t overshootBytes = 0;
};

class ScopedMemoryBudget {
public:
    ScopedMemoryBudget(const std::string& name, uint64_t budgetBytes)
        : m_name(name), m_budget(budgetBytes), m_allocated(0), m_peakAllocated(0),
        m_allocationCount(0), m_violated(false) {
    }

    bool TryAllocate(uint64_t bytes) {
        uint64_t newTotal = m_allocated + bytes;
        if (newTotal > m_budget) {
            m_violated = true;
            return false;
        }
        m_allocated = newTotal;
        if (m_allocated > m_peakAllocated) m_peakAllocated = m_allocated;
        m_allocationCount++;
        return true;
    }

    void Release(uint64_t bytes) {
        if (bytes > m_allocated) bytes = m_allocated;
        m_allocated -= bytes;
    }

    bool IsViolated() const { return m_violated; }
    bool WouldExceed(uint64_t bytes) const { return m_allocated + bytes > m_budget; }
    uint64_t Remaining() const { return m_budget > m_allocated ? m_budget - m_allocated : 0; }
    uint64_t Used() const { return m_allocated; }
    uint64_t Budget() const { return m_budget; }
    uint64_t PeakUsage() const { return m_peakAllocated; }
    uint32_t AllocationCount() const { return m_allocationCount; }
    double Utilization() const { return m_budget > 0 ? 100.0 * m_allocated / m_budget : 0.0; }

    BudgetViolation GetViolation() const {
        BudgetViolation v;
        v.scopeName = m_name;
        v.budgetBytes = m_budget;
        v.actualBytes = m_peakAllocated;
        v.overshootBytes = m_peakAllocated > m_budget ? m_peakAllocated - m_budget : 0;
        return v;
    }

    const std::string& Name() const { return m_name; }

private:
    std::string m_name;
    uint64_t m_budget;
    uint64_t m_allocated;
    uint64_t m_peakAllocated;
    uint32_t m_allocationCount;
    bool m_violated;
};

} // namespace Engine
} // namespace ExplorerLens
