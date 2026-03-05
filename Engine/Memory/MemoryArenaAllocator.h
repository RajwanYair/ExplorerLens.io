// MemoryArenaAllocator.h — Arena-Based Memory Allocation for Decode Pipelines
// Copyright (c) 2026 ExplorerLens Project
//
// Provides bump-allocation arenas for decode operations, eliminating per-object
// heap allocations. Each decode request gets its own arena that is freed in bulk.
// Implements a linked-list of fixed-size blocks with bump-pointer allocation.
//
#pragma once

#include <cstdint>
#include <cstdlib>
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

    ~MemoryArenaAllocator() {
        FreeAllBlocks();
    }

    // Non-copyable, movable
    MemoryArenaAllocator(const MemoryArenaAllocator&) = delete;
    MemoryArenaAllocator& operator=(const MemoryArenaAllocator&) = delete;
    MemoryArenaAllocator(MemoryArenaAllocator&& other) noexcept
        : m_blockSize(other.m_blockSize), m_policy(other.m_policy),
          m_stats(other.m_stats), m_head(other.m_head),
          m_current(other.m_current), m_offset(other.m_offset) {
        other.m_head = nullptr;
        other.m_current = nullptr;
        other.m_offset = 0;
    }

    void* Allocate(size_t size, size_t alignment = 8) {
        if (size == 0) return nullptr;
        size_t aligned = (size + alignment - 1) & ~(alignment - 1);

        // Try to allocate from the current block
        if (m_current) {
            size_t padded = AlignOffset(m_offset, alignment);
            if (padded + aligned <= m_current->capacity) {
                void* ptr = m_current->data + padded;
                m_offset = padded + aligned;
                m_stats.totalAllocated += aligned;
                m_stats.allocCalls++;
                if (m_stats.totalAllocated > m_stats.peakUsage)
                    m_stats.peakUsage = m_stats.totalAllocated;
                return ptr;
            }
        }

        // Need a new block — size it to fit at least this allocation
        size_t newBlockCap = (aligned > m_blockSize) ? aligned : m_blockSize;
        Block* block = AllocateBlock(newBlockCap);
        if (!block) return nullptr;

        // Link into the chain
        block->next = nullptr;
        if (m_current) m_current->next = block;
        else m_head = block;
        m_current = block;
        m_offset = aligned;

        m_stats.totalAllocated += aligned;
        m_stats.allocCalls++;
        m_stats.blockCount++;
        if (m_stats.totalAllocated > m_stats.peakUsage)
            m_stats.peakUsage = m_stats.totalAllocated;

        return block->data;
    }

    void Reset() {
        FreeAllBlocks();
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
    struct Block {
        Block* next;
        size_t capacity;
        alignas(16) uint8_t data[1]; // Flexible tail (C-style, actual size = capacity)
    };

    static size_t AlignOffset(size_t offset, size_t alignment) {
        return (offset + alignment - 1) & ~(alignment - 1);
    }

    static Block* AllocateBlock(size_t capacity) {
        size_t headerSize = offsetof(Block, data);
        void* raw = std::malloc(headerSize + capacity);
        if (!raw) return nullptr;
        Block* b = static_cast<Block*>(raw);
        b->next = nullptr;
        b->capacity = capacity;
        return b;
    }

    void FreeAllBlocks() {
        Block* b = m_head;
        while (b) {
            Block* next = b->next;
            std::free(b);
            b = next;
        }
        m_head = nullptr;
        m_current = nullptr;
        m_offset = 0;
    }

    size_t m_blockSize;
    ArenaPolicy m_policy = ArenaPolicy::PerDecode;
    ArenaStats m_stats;
    Block* m_head = nullptr;
    Block* m_current = nullptr;
    size_t m_offset = 0;
};

} // namespace Engine
} // namespace ExplorerLens
