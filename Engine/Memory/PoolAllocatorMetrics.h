// PoolAllocatorMetrics.h — Pool Allocator Performance Instrumentation
// Copyright (c) 2026 ExplorerLens Project
//
// Collects detailed metrics from pool allocators including allocation
// patterns, fragmentation ratios, and pool utilization histograms.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

struct PoolBucketMetrics {
    uint32_t blockSize = 0;
    uint32_t totalBlocks = 0;
    uint32_t freeBlocks = 0;
    uint32_t peakUsed = 0;
    uint64_t allocationCount = 0;
    uint64_t deallocationCount = 0;

    double Utilization() const {
        return totalBlocks > 0 ? 100.0 * (totalBlocks - freeBlocks) / totalBlocks : 0.0;
    }
    uint32_t UsedBlocks() const { return totalBlocks - freeBlocks; }
};

struct PoolAllocatorSummary {
    std::string poolName;
    uint32_t bucketCount = 0;
    uint64_t totalCapacityBytes = 0;
    uint64_t usedBytes = 0;
    uint64_t peakUsedBytes = 0;
    double fragmentationRatio = 0.0;
    uint64_t totalAllocations = 0;
    uint64_t totalDeallocations = 0;
    double avgAllocationTimeNs = 0.0;
    std::vector<PoolBucketMetrics> buckets;

    double Utilization() const {
        return totalCapacityBytes > 0 ? 100.0 * usedBytes / totalCapacityBytes : 0.0;
    }
};

class PoolAllocatorMetrics {
public:
    void SetPoolName(const std::string& name) { m_summary.poolName = name; }

    void RecordAllocation(uint32_t blockSize, double timeNs) {
        m_summary.totalAllocations++;
        m_totalAllocTimeNs += timeNs;
        m_summary.avgAllocationTimeNs = m_totalAllocTimeNs / m_summary.totalAllocations;
        UpdateBucket(blockSize, true);
    }

    void RecordDeallocation(uint32_t blockSize) {
        m_summary.totalDeallocations++;
        UpdateBucket(blockSize, false);
    }

    void UpdateCapacity(uint64_t totalBytes, uint64_t usedBytes) {
        m_summary.totalCapacityBytes = totalBytes;
        m_summary.usedBytes = usedBytes;
        if (usedBytes > m_summary.peakUsedBytes) m_summary.peakUsedBytes = usedBytes;
        m_summary.fragmentationRatio = totalBytes > 0
            ? 1.0 - (static_cast<double>(usedBytes) / totalBytes) : 0.0;
    }

    PoolAllocatorSummary GetSummary() const {
        auto s = m_summary;
        s.bucketCount = static_cast<uint32_t>(s.buckets.size());
        return s;
    }

    void Reset() {
        auto name = m_summary.poolName;
        m_summary = {};
        m_summary.poolName = name;
        m_totalAllocTimeNs = 0.0;
    }

private:
    void UpdateBucket(uint32_t blockSize, bool isAlloc) {
        for (auto& b : m_summary.buckets) {
            if (b.blockSize == blockSize) {
                if (isAlloc) {
                    b.allocationCount++;
                    if (b.freeBlocks > 0) b.freeBlocks--;
                    if (b.UsedBlocks() > b.peakUsed) b.peakUsed = b.UsedBlocks();
                }
                else {
                    b.deallocationCount++;
                    b.freeBlocks++;
                }
                return;
            }
        }
        if (isAlloc) {
            PoolBucketMetrics b;
            b.blockSize = blockSize;
            b.totalBlocks = 1;
            b.freeBlocks = 0;
            b.allocationCount = 1;
            b.peakUsed = 1;
            m_summary.buckets.push_back(b);
        }
    }

    PoolAllocatorSummary m_summary;
    double m_totalAllocTimeNs = 0.0;
};

} // namespace Engine
} // namespace ExplorerLens
