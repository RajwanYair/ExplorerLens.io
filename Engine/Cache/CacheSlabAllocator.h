// CacheSlabAllocator.h — Slab-Based Cache Memory Allocation
// Copyright (c) 2026 ExplorerLens Project
//
// Provides slab-based memory allocation for cache entries to reduce
// fragmentation, improve locality, and enable fast allocation/deallocation.
//
#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class SlabSizeClass : uint8_t {
    Tiny,       // 64 bytes
    Small,      // 256 bytes
    Medium,     // 4096 bytes
    Large,      // 65536 bytes
    Huge        // 1048576 bytes
};

struct SlabInfo {
    SlabSizeClass sizeClass = SlabSizeClass::Medium;
    uint32_t objectSize = 0;
    uint32_t objectsPerSlab = 0;
    uint32_t freeObjects = 0;
    uint32_t totalObjects = 0;
    uint64_t totalBytesAllocated = 0;
};

struct SlabAllocatorMetrics {
    uint64_t totalAllocations = 0;
    uint64_t totalDeallocations = 0;
    uint64_t slabsCreated = 0;
    uint64_t slabsDestroyed = 0;
    uint64_t totalMemoryBytes = 0;
    float utilization = 0.0f;
};

class CacheSlabAllocator {
public:
    CacheSlabAllocator() { InitializeSlabs(); }

    SlabSizeClass SelectSizeClass(uint32_t requestedBytes) const {
        if (requestedBytes <= 64) return SlabSizeClass::Tiny;
        if (requestedBytes <= 256) return SlabSizeClass::Small;
        if (requestedBytes <= 4096) return SlabSizeClass::Medium;
        if (requestedBytes <= 65536) return SlabSizeClass::Large;
        return SlabSizeClass::Huge;
    }

    bool Allocate(uint32_t requestedBytes) {
        auto sizeClass = SelectSizeClass(requestedBytes);
        auto idx = static_cast<size_t>(sizeClass);
        if (idx >= m_slabs.size()) return false;
        if (m_slabs[idx].freeObjects == 0) {
            GrowSlab(sizeClass);
        }
        if (m_slabs[idx].freeObjects > 0) {
            m_slabs[idx].freeObjects--;
            m_slabs[idx].totalBytesAllocated += m_slabs[idx].objectSize;
            m_metrics.totalAllocations++;
            return true;
        }
        return false;
    }

    void Deallocate(uint32_t requestedBytes) {
        auto sizeClass = SelectSizeClass(requestedBytes);
        auto idx = static_cast<size_t>(sizeClass);
        if (idx >= m_slabs.size()) return;
        m_slabs[idx].freeObjects++;
        if (m_slabs[idx].totalBytesAllocated >= m_slabs[idx].objectSize)
            m_slabs[idx].totalBytesAllocated -= m_slabs[idx].objectSize;
        m_metrics.totalDeallocations++;
    }

    SlabInfo GetSlabInfo(SlabSizeClass sizeClass) const {
        auto idx = static_cast<size_t>(sizeClass);
        return idx < m_slabs.size() ? m_slabs[idx] : SlabInfo{};
    }

    SlabAllocatorMetrics GetMetrics() const { return m_metrics; }

    float GetUtilization() const {
        uint64_t totalFree = 0, totalObj = 0;
        for (const auto& slab : m_slabs) {
            totalFree += slab.freeObjects;
            totalObj += slab.totalObjects;
        }
        return totalObj > 0 ? 1.0f - (static_cast<float>(totalFree) / totalObj) : 0.0f;
    }

private:
    void InitializeSlabs() {
        m_slabs = {
            {SlabSizeClass::Tiny, 64, 256, 256, 256, 0},
            {SlabSizeClass::Small, 256, 128, 128, 128, 0},
            {SlabSizeClass::Medium, 4096, 64, 64, 64, 0},
            {SlabSizeClass::Large, 65536, 16, 16, 16, 0},
            {SlabSizeClass::Huge, 1048576, 4, 4, 4, 0}
        };
    }

    void GrowSlab(SlabSizeClass sizeClass) {
        auto idx = static_cast<size_t>(sizeClass);
        if (idx >= m_slabs.size()) return;
        uint32_t growth = m_slabs[idx].objectsPerSlab;
        m_slabs[idx].totalObjects += growth;
        m_slabs[idx].freeObjects += growth;
        m_metrics.slabsCreated++;
    }

    std::vector<SlabInfo> m_slabs;
    SlabAllocatorMetrics m_metrics;
};

} // namespace Engine
} // namespace ExplorerLens
