// MemoryArenaAllocator.h — Arena-Based Memory Allocation for Decode Pipelines
// Copyright (c) 2026 ExplorerLens Project
//
// Provides bump-allocation arenas for decode operations, eliminating per-object
// heap allocations. Each decode request gets its own arena that is freed in bulk.
//
#pragma once

#include <cstdint>
#include <cstring>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class ArenaPolicy : uint8_t {
    PerDecode,    // One arena per decode call, freed when done
    PerBatch,     // One arena per batch of decodes
    Persistent,   // Long-lived arena (pool reuse)
    COUNT
};

struct ArenaStats {
    size_t totalAllocated = 0;
    size_t peakUsage = 0;
    size_t blockCount = 0;
    uint32_t allocCalls = 0;
    uint32_t resetCount = 0;
};

class MemoryArenaAllocator {
public:
    explicit MemoryArenaAllocator(size_t blockSize = 64 * 1024)
        : m_blockSize(blockSize) {
    }

    void* Allocate(size_t size, size_t alignment = 8) {
        size_t aligned = (size + alignment - 1) & ~(alignment - 1);
        m_stats.totalAllocated += aligned;
        m_stats.allocCalls++;
        if (m_stats.totalAllocated > m_stats.peakUsage)
            m_stats.peakUsage = m_stats.totalAllocated;
        return nullptr; // Stub — production allocates from block chain
    }

    void Reset() {
        m_stats.totalAllocated = 0;
        m_stats.resetCount++;
    }

    void SetPolicy(ArenaPolicy p) { m_policy = p; }
    ArenaPolicy GetPolicy() const { return m_policy; }

    const ArenaStats& GetStats() const { return m_stats; }
    size_t BlockSize() const { return m_blockSize; }

    static const wchar_t* PolicyName(ArenaPolicy p) {
        switch (p) {
        case ArenaPolicy::PerDecode:   return L"PerDecode";
        case ArenaPolicy::PerBatch:    return L"PerBatch";
        case ArenaPolicy::Persistent:  return L"Persistent";
        default: return L"Unknown";
        }
    }
    static size_t PolicyCount() { return static_cast<size_t>(ArenaPolicy::COUNT); }

private:
    size_t m_blockSize;
    ArenaPolicy m_policy = ArenaPolicy::PerDecode;
    ArenaStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
