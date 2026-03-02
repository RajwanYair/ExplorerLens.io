// GPUMemoryTracker.h — Per-Category VRAM Tracking with Budget Enforcement
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks GPU VRAM consumption across six categories (textures, render
// targets, staging buffers, shader resources, pipeline states, general)
// with atomic counters for thread safety. A configurable budget with
// three threshold tiers (Elevated / High / Critical) triggers enforcement
// actions. Allocations can be hard-rejected when the budget is exceeded.
// Snapshots provide a per-category breakdown for diagnostics.
//
// Thread-safe singleton.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <string>
#include <atomic>
#include <vector>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

enum class VRAMCategory : uint32_t {
    Texture = 0,
    RenderTarget = 1,
    StagingBuffer = 2,
    ShaderResource = 3,
    PipelineState = 4,
    General = 5,
    Count = 6
};

static const wchar_t* VRAMCategoryName(VRAMCategory c) {
    static const wchar_t* names[] = {
        L"Texture", L"RenderTarget", L"StagingBuffer",
        L"ShaderResource", L"PipelineState", L"General"
    };
    auto idx = static_cast<uint32_t>(c);
    return (idx < static_cast<uint32_t>(VRAMCategory::Count)) ? names[idx] : L"Unknown";
}

enum class VRAMBudgetLevel : uint32_t {
    Normal = 0,   // Under 50% budget
    Elevated = 1,   // 50-75% budget
    High = 2,   // 75-90% budget
    Critical = 3    // >90% or over budget
};

struct VRAMCategoryUsage {
    std::atomic<uint64_t> allocated{ 0 };
    std::atomic<uint64_t> peakAllocated{ 0 };
    std::atomic<uint32_t> allocationCount{ 0 };

    void Add(uint64_t bytes) {
        uint64_t current = allocated.fetch_add(bytes, std::memory_order_relaxed) + bytes;
        uint64_t peak = peakAllocated.load(std::memory_order_relaxed);
        while (current > peak) {
            if (peakAllocated.compare_exchange_weak(peak, current, std::memory_order_relaxed))
                break;
        }
        allocationCount.fetch_add(1, std::memory_order_relaxed);
    }

    void Remove(uint64_t bytes) {
        uint64_t cur = allocated.load(std::memory_order_relaxed);
        allocated.store((bytes <= cur) ? (cur - bytes) : 0, std::memory_order_relaxed);
    }
};

struct VRAMSnapshot {
    uint64_t budgetBytes = 0;
    uint64_t totalAllocated = 0;
    uint64_t peakAllocated = 0;
    VRAMBudgetLevel level = VRAMBudgetLevel::Normal;
    double   utilizationPct = 0.0;
    uint64_t categoryBytes[static_cast<size_t>(VRAMCategory::Count)] = {};
    uint32_t categoryAllocCounts[static_cast<size_t>(VRAMCategory::Count)] = {};
};

struct GPUMemoryTrackerConfig {
    uint64_t vramBudgetBytes = 512ULL * 1024 * 1024;  // 512 MB default
    double   elevatedThreshold = 0.50;
    double   highThreshold = 0.75;
    double   criticalThreshold = 0.90;
    bool     enforceHardLimit = true;
};

// ========================================================================
// GPUMemoryTracker — VRAM allocation tracking and budget enforcement
// ========================================================================
class GPUMemoryTracker {
public:
    static GPUMemoryTracker& Instance() {
        static GPUMemoryTracker instance;
        return instance;
    }

    void Initialize(const GPUMemoryTrackerConfig& config = {}) {
        m_config = config;
        Reset();
        m_initialized = true;
    }

    bool IsInitialized() const { return m_initialized; }

    // Track an allocation
    bool TrackAllocation(VRAMCategory category, uint64_t bytes) {
        // Check budget before allocation
        if (m_config.enforceHardLimit) {
            uint64_t current = m_totalAllocated.load(std::memory_order_relaxed);
            if (current + bytes > m_config.vramBudgetBytes) {
                m_rejectedAllocations.fetch_add(1, std::memory_order_relaxed);
                return false; // Over budget
            }
        }

        auto idx = static_cast<uint32_t>(category);
        if (idx < static_cast<uint32_t>(VRAMCategory::Count)) {
            m_categories[idx].Add(bytes);
        }

        uint64_t newTotal = m_totalAllocated.fetch_add(bytes, std::memory_order_relaxed) + bytes;
        uint64_t peak = m_peakAllocated.load(std::memory_order_relaxed);
        while (newTotal > peak) {
            if (m_peakAllocated.compare_exchange_weak(peak, newTotal, std::memory_order_relaxed))
                break;
        }

        return true;
    }

    // Track a deallocation
    void TrackDeallocation(VRAMCategory category, uint64_t bytes) {
        auto idx = static_cast<uint32_t>(category);
        if (idx < static_cast<uint32_t>(VRAMCategory::Count)) {
            m_categories[idx].Remove(bytes);
        }

        uint64_t cur = m_totalAllocated.load(std::memory_order_relaxed);
        m_totalAllocated.store((bytes <= cur) ? (cur - bytes) : 0, std::memory_order_relaxed);
    }

    // Get current budget level
    VRAMBudgetLevel GetBudgetLevel() const {
        double util = GetUtilization();
        if (util >= m_config.criticalThreshold) return VRAMBudgetLevel::Critical;
        if (util >= m_config.highThreshold)     return VRAMBudgetLevel::High;
        if (util >= m_config.elevatedThreshold) return VRAMBudgetLevel::Elevated;
        return VRAMBudgetLevel::Normal;
    }

    // Get utilization ratio
    double GetUtilization() const {
        return (m_config.vramBudgetBytes > 0)
            ? (static_cast<double>(m_totalAllocated.load(std::memory_order_relaxed))
                / static_cast<double>(m_config.vramBudgetBytes))
            : 0.0;
    }

    // Get total allocated
    uint64_t GetTotalAllocated() const { return m_totalAllocated.load(std::memory_order_relaxed); }

    // Get remaining budget
    uint64_t GetRemainingBudget() const {
        uint64_t used = m_totalAllocated.load(std::memory_order_relaxed);
        return (used < m_config.vramBudgetBytes) ? (m_config.vramBudgetBytes - used) : 0;
    }

    // Capture snapshot
    VRAMSnapshot CaptureSnapshot() const {
        VRAMSnapshot snap;
        snap.budgetBytes = m_config.vramBudgetBytes;
        snap.totalAllocated = m_totalAllocated.load(std::memory_order_relaxed);
        snap.peakAllocated = m_peakAllocated.load(std::memory_order_relaxed);
        snap.level = GetBudgetLevel();
        snap.utilizationPct = GetUtilization() * 100.0;

        for (size_t i = 0; i < static_cast<size_t>(VRAMCategory::Count); ++i) {
            snap.categoryBytes[i] = m_categories[i].allocated.load(std::memory_order_relaxed);
            snap.categoryAllocCounts[i] = m_categories[i].allocationCount.load(std::memory_order_relaxed);
        }

        return snap;
    }

    // Update budget dynamically
    void SetBudget(uint64_t bytes) { m_config.vramBudgetBytes = bytes; }

    // Reset
    void Reset() {
        m_totalAllocated.store(0, std::memory_order_relaxed);
        m_peakAllocated.store(0, std::memory_order_relaxed);
        m_rejectedAllocations.store(0, std::memory_order_relaxed);
        for (auto& c : m_categories) {
            c.allocated.store(0, std::memory_order_relaxed);
            c.peakAllocated.store(0, std::memory_order_relaxed);
            c.allocationCount.store(0, std::memory_order_relaxed);
        }
    }

private:
    GPUMemoryTracker() = default;

    GPUMemoryTrackerConfig m_config;
    std::atomic<uint64_t> m_totalAllocated{ 0 };
    std::atomic<uint64_t> m_peakAllocated{ 0 };
    std::atomic<uint64_t> m_rejectedAllocations{ 0 };
    VRAMCategoryUsage m_categories[static_cast<size_t>(VRAMCategory::Count)];
    bool m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
