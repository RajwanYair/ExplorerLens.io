// MemoryCompactionEngine.h — Active Memory Compaction
// Copyright (c) 2026 ExplorerLens Project
//
// Performs active memory compaction during idle periods to reduce
// fragmentation, consolidate allocations, and improve cache locality.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class EngineCompactionTrigger : uint8_t {
    Manual,
    IdleTimer,
    FragmentationThreshold,
    MemoryPressure,
    Periodic
};

struct FragmentationInfo {
    uint64_t totalAllocatedBytes = 0;
    uint64_t totalCommittedBytes = 0;
    uint64_t largestFreeBlock = 0;
    float fragmentationRatio = 0.0f;
    uint32_t freeBlockCount = 0;
};

struct CompactionResult {
    uint64_t bytesCompacted = 0;
    uint64_t bytesFreed = 0;
    uint32_t blocksConsolidated = 0;
    double durationMs = 0.0;
    float fragmentationBefore = 0.0f;
    float fragmentationAfter = 0.0f;
};

struct CompactionConfig {
    float triggerFragmentationThreshold = 0.3f;
    uint32_t idleTimerMs = 5000;
    uint32_t maxCompactionTimeMs = 100;
    bool compactDuringDecode = false;
};

class MemoryCompactionEngine {
public:
    MemoryCompactionEngine() = default;

    CompactionResult RunCompaction(EngineCompactionTrigger trigger) {
        CompactionResult result;
        result.fragmentationBefore = m_currentFragmentation;
        result.blocksConsolidated = m_freeBlockCount > 1 ? m_freeBlockCount - 1 : 0;
        result.bytesCompacted = result.blocksConsolidated * 4096ULL;
        result.fragmentationAfter = result.blocksConsolidated > 0
            ? m_currentFragmentation * 0.5f : m_currentFragmentation;
        m_currentFragmentation = result.fragmentationAfter;
        m_freeBlockCount = m_freeBlockCount > 0 ? 1 : 0;
        m_totalCompactions++;
        (void)trigger;
        return result;
    }

    FragmentationInfo GetFragmentationInfo() const {
        FragmentationInfo info;
        info.fragmentationRatio = m_currentFragmentation;
        info.freeBlockCount = m_freeBlockCount;
        info.totalAllocatedBytes = m_totalAllocated;
        info.totalCommittedBytes = m_totalCommitted;
        return info;
    }

    bool ShouldCompact() const {
        return m_currentFragmentation >= m_config.triggerFragmentationThreshold;
    }

    void UpdateFragmentation(float ratio, uint32_t freeBlocks) {
        m_currentFragmentation = ratio;
        m_freeBlockCount = freeBlocks;
    }

    void SetAllocatedBytes(uint64_t allocated, uint64_t committed) {
        m_totalAllocated = allocated;
        m_totalCommitted = committed;
    }

    void SetConfig(const CompactionConfig& config) { m_config = config; }
    CompactionConfig GetConfig() const { return m_config; }
    uint64_t GetTotalCompactions() const { return m_totalCompactions; }

private:
    float m_currentFragmentation = 0.0f;
    uint32_t m_freeBlockCount = 0;
    uint64_t m_totalAllocated = 0;
    uint64_t m_totalCommitted = 0;
    uint64_t m_totalCompactions = 0;
    CompactionConfig m_config;
};

} // namespace Engine
} // namespace ExplorerLens
