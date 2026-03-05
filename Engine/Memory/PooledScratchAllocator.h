// PooledScratchAllocator.h — Per-Thread Scratch Memory Allocator
// Copyright (c) 2026 ExplorerLens Project
//
// Per-thread scratch memory allocator. Thread-local bump allocator with zero
// fragmentation, ideal for decode pipeline temporary buffers.
//
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include <array>

namespace ExplorerLens {
namespace Engine {

struct ScratchBlock {
    std::vector<uint8_t> memory;
    size_t used = 0;
    size_t capacity = 0;
    uint32_t allocationCount = 0;
};

struct ScratchAllocation {
    uint8_t* ptr = nullptr;
    size_t size = 0;
    size_t alignedSize = 0;
    uint32_t blockIndex = 0;
};

struct ScratchAllocatorStats {
    size_t totalCapacity = 0;
    size_t totalUsed = 0;
    uint32_t activeBlocks = 0;
    uint32_t totalAllocations = 0;
    uint32_t totalResets = 0;
    double fragmentationRatio = 0.0;
    size_t peakUsage = 0;
};

class PooledScratchAllocator {
public:
    static constexpr size_t DEFAULT_BLOCK_SIZE = 1024 * 1024;
    static constexpr size_t DEFAULT_ALIGNMENT = 16;

    inline explicit PooledScratchAllocator(size_t blockSize = DEFAULT_BLOCK_SIZE)
        : m_blockSize(blockSize) {
        AddBlock(blockSize);
    }

    inline uint8_t* Allocate(size_t size, size_t alignment = DEFAULT_ALIGNMENT) {
        size_t alignedSize = AlignUp(size, alignment);

        for (auto& block : m_blocks) {
            size_t alignedOffset = AlignUp(block.used, alignment);
            if (alignedOffset + alignedSize <= block.capacity) {
                uint8_t* ptr = block.memory.data() + alignedOffset;
                block.used = alignedOffset + alignedSize;
                block.allocationCount++;
                m_stats.totalAllocations++;
                m_stats.totalUsed += alignedSize;
                if (m_stats.totalUsed > m_stats.peakUsage) m_stats.peakUsage = m_stats.totalUsed;
                return ptr;
            }
        }

        size_t newBlockSize = (std::max)(m_blockSize, alignedSize + alignment);
        AddBlock(newBlockSize);
        auto& block = m_blocks.back();
        uint8_t* ptr = block.memory.data();
        block.used = alignedSize;
        block.allocationCount++;
        m_stats.totalAllocations++;
        m_stats.totalUsed += alignedSize;
        if (m_stats.totalUsed > m_stats.peakUsage) m_stats.peakUsage = m_stats.totalUsed;
        return ptr;
    }

    template<typename T>
    inline T* AllocateTyped(size_t count = 1) {
        return reinterpret_cast<T*>(Allocate(sizeof(T) * count, alignof(T)));
    }

    inline void Reset() {
        for (auto& block : m_blocks) {
            block.used = 0;
            block.allocationCount = 0;
        }
        m_stats.totalUsed = 0;
        m_stats.totalAllocations = 0;
        m_stats.totalResets++;
    }

    inline void ReleaseUnusedBlocks() {
        while (m_blocks.size() > 1 && m_blocks.back().allocationCount == 0) {
            m_stats.totalCapacity -= m_blocks.back().capacity;
            m_blocks.pop_back();
        }
        m_stats.activeBlocks = static_cast<uint32_t>(m_blocks.size());
    }

    inline size_t GetAvailableSpace() const {
        size_t available = 0;
        for (const auto& block : m_blocks) {
            available += block.capacity - block.used;
        }
        return available;
    }

    inline ScratchAllocatorStats GetStats() const {
        auto stats = m_stats;
        stats.activeBlocks = static_cast<uint32_t>(m_blocks.size());
        if (stats.totalCapacity > 0) {
            stats.fragmentationRatio = 1.0 - static_cast<double>(stats.totalUsed) / stats.totalCapacity;
        }
        return stats;
    }

    inline uint8_t* AllocateZeroed(size_t size, size_t alignment = DEFAULT_ALIGNMENT) {
        uint8_t* ptr = Allocate(size, alignment);
        if (ptr) std::memset(ptr, 0, size);
        return ptr;
    }

private:
    inline void AddBlock(size_t capacity) {
        ScratchBlock block;
        block.memory.resize(capacity);
        block.capacity = capacity;
        block.used = 0;
        m_stats.totalCapacity += capacity;
        m_blocks.push_back(std::move(block));
        m_stats.activeBlocks = static_cast<uint32_t>(m_blocks.size());
    }

    inline size_t AlignUp(size_t value, size_t alignment) const {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    size_t m_blockSize;
    std::vector<ScratchBlock> m_blocks;
    ScratchAllocatorStats m_stats = {};
};

}
} // namespace ExplorerLens::Engine
