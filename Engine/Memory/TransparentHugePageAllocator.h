// TransparentHugePageAllocator.h — THP/Large Page Optimization
// Copyright (c) 2026 ExplorerLens Project
//
// Transparent huge page / large page optimization. Uses VirtualAlloc MEM_LARGE_PAGES
// for hot-path allocations, reducing TLB misses on large decode buffers.
//
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <mutex>
#include <algorithm>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

enum class HugePageSize : uint8_t {
    Standard4K,
    Large2M,
    Huge1G
};

enum class AllocationHint : uint8_t {
    Default,
    HotPath,
    DecodeBuffer,
    CacheEntry,
    TempScratch
};

struct HugePageAllocation {
    uintptr_t address = 0;
    size_t size = 0;
    size_t alignedSize = 0;
    HugePageSize HugePageSize = HugePageSize::Standard4K;
    AllocationHint hint = AllocationHint::Default;
    uint64_t allocationId = 0;
    bool isActive = false;
};

struct AllocatorStats {
    uint64_t totalAllocations = 0;
    uint64_t totalDeallocations = 0;
    uint64_t activeAllocations = 0;
    size_t totalBytesAllocated = 0;
    size_t totalBytesFreed = 0;
    size_t currentUsageBytes = 0;
    size_t peakUsageBytes = 0;
    uint64_t largePageAllocations = 0;
    uint64_t hugePageAllocations = 0;
    uint64_t fallbackToStandard = 0;
};

class TransparentHugePageAllocator {
public:
    static TransparentHugePageAllocator& Instance() {
        static TransparentHugePageAllocator instance;
        return instance;
    }

    inline size_t GetPageSizeBytes(HugePageSize ps) const {
        switch (ps) {
        case HugePageSize::Standard4K: return 4096;
        case HugePageSize::Large2M:    return 2 * 1024 * 1024;
        case HugePageSize::Huge1G:     return 1024ULL * 1024 * 1024;
        default:                   return 4096;
        }
    }

    inline HugePageSize RecommendPageSize(size_t allocationSize, AllocationHint hint) const {
        if (hint == AllocationHint::TempScratch) return HugePageSize::Standard4K;

        if (allocationSize >= 256 * 1024 * 1024) return HugePageSize::Huge1G;
        if (allocationSize >= 2 * 1024 * 1024 || hint == AllocationHint::HotPath ||
            hint == AllocationHint::DecodeBuffer) {
            return HugePageSize::Large2M;
        }
        return HugePageSize::Standard4K;
    }

    inline size_t AlignToPageSize(size_t size, HugePageSize ps) const {
        size_t HugePageSize = GetPageSizeBytes(ps);
        return (size + HugePageSize - 1) & ~(HugePageSize - 1);
    }

    inline uint64_t RecordAllocation(size_t size, AllocationHint hint = AllocationHint::Default) {
        std::lock_guard<std::mutex> lock(m_mutex);
        HugePageSize ps = RecommendPageSize(size, hint);
        size_t alignedSize = AlignToPageSize(size, ps);

        HugePageAllocation record;
        record.allocationId = m_nextId++;
        record.size = size;
        record.alignedSize = alignedSize;
        record.HugePageSize = ps;
        record.hint = hint;
        record.isActive = true;

        m_records[record.allocationId] = record;

        m_stats.totalAllocations++;
        m_stats.activeAllocations++;
        m_stats.totalBytesAllocated += alignedSize;
        m_stats.currentUsageBytes += alignedSize;
        if (m_stats.currentUsageBytes > m_stats.peakUsageBytes) {
            m_stats.peakUsageBytes = m_stats.currentUsageBytes;
        }

        if (ps == HugePageSize::Large2M) m_stats.largePageAllocations++;
        else if (ps == HugePageSize::Huge1G) m_stats.hugePageAllocations++;

        return record.allocationId;
    }

    inline bool RecordDeallocation(uint64_t allocationId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_records.find(allocationId);
        if (it == m_records.end() || !it->second.isActive) return false;

        it->second.isActive = false;
        m_stats.totalDeallocations++;
        m_stats.activeAllocations--;
        m_stats.totalBytesFreed += it->second.alignedSize;
        m_stats.currentUsageBytes -= it->second.alignedSize;
        return true;
    }

    inline double ComputeWasteRatio() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        size_t requestedTotal = 0;
        size_t alignedTotal = 0;
        for (const auto& [id, rec] : m_records) {
            if (rec.isActive) {
                requestedTotal += rec.size;
                alignedTotal += rec.alignedSize;
            }
        }
        return alignedTotal > 0 ? 1.0 - static_cast<double>(requestedTotal) / alignedTotal : 0.0;
    }

    inline AllocatorStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    inline std::string PageSizeToString(HugePageSize ps) const {
        switch (ps) {
        case HugePageSize::Standard4K: return "4 KB (Standard)";
        case HugePageSize::Large2M:    return "2 MB (Large)";
        case HugePageSize::Huge1G:     return "1 GB (Huge)";
        default:                   return "Unknown";
        }
    }

private:
    TransparentHugePageAllocator() = default;

    mutable std::mutex m_mutex;
    std::unordered_map<uint64_t, HugePageAllocation> m_records;
    AllocatorStats m_stats;
    uint64_t m_nextId = 1;
};

}
} // namespace ExplorerLens::Engine
