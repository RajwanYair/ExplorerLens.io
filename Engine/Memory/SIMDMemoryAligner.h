// SIMDMemoryAligner.h — SIMD-Aligned Memory Allocation
// Copyright (c) 2026 ExplorerLens Project
//
// Provides aligned memory allocation for SIMD operations, ensuring buffers
// meet 16/32/64-byte alignment for SSE, AVX, and AVX-512 instructions.
//
#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <algorithm>

#ifdef _MSC_VER
#include <malloc.h>
#endif

namespace ExplorerLens {
namespace Engine {

enum class AlignmentBoundary : uint32_t {
    Align16 = 16,
    Align32 = 32,
    Align64 = 64
};

struct AlignedBlock {
    void* ptr = nullptr;
    size_t             size = 0;
    AlignmentBoundary  alignment = AlignmentBoundary::Align16;
    bool               inUse = false;
};

struct SIMDAllocStats {
    uint64_t totalAllocations = 0;
    uint64_t totalFrees = 0;
    uint64_t totalBytesAlloc = 0;
    uint64_t totalBytesFreed = 0;
    uint64_t currentBytes = 0;
    uint64_t peakBytes = 0;
    uint32_t activeBlocks = 0;
};

class SIMDMemoryAligner {
public:
    static SIMDMemoryAligner& Instance() { static SIMDMemoryAligner s; return s; }

    void* AllocAligned(size_t size, AlignmentBoundary alignment = AlignmentBoundary::Align32) {
        if (size == 0) return nullptr;
        uint32_t align = static_cast<uint32_t>(alignment);

#ifdef _MSC_VER
        void* ptr = _aligned_malloc(size, align);
#else
        void* ptr = nullptr;
        if (posix_memalign(&ptr, align, size) != 0) ptr = nullptr;
#endif
        if (!ptr) return nullptr;

        AlignedBlock block;
        block.ptr = ptr;
        block.size = size;
        block.alignment = alignment;
        block.inUse = true;

        m_blocks[reinterpret_cast<uintptr_t>(ptr)] = block;
        m_stats.totalAllocations++;
        m_stats.totalBytesAlloc += size;
        m_stats.currentBytes += size;
        m_stats.activeBlocks++;
        if (m_stats.currentBytes > m_stats.peakBytes)
            m_stats.peakBytes = m_stats.currentBytes;

        return ptr;
    }

    bool FreeAligned(void* ptr) {
        if (!ptr) return false;
        uintptr_t key = reinterpret_cast<uintptr_t>(ptr);
        auto it = m_blocks.find(key);
        if (it == m_blocks.end()) return false;

        m_stats.totalFrees++;
        m_stats.totalBytesFreed += it->second.size;
        m_stats.currentBytes -= it->second.size;
        m_stats.activeBlocks--;
        m_blocks.erase(it);

#ifdef _MSC_VER
        _aligned_free(ptr);
#else
        free(ptr);
#endif
        return true;
    }

    void* ReallocAligned(void* oldPtr, size_t newSize, AlignmentBoundary alignment = AlignmentBoundary::Align32) {
        if (!oldPtr) return AllocAligned(newSize, alignment);
        if (newSize == 0) { FreeAligned(oldPtr); return nullptr; }

        uintptr_t key = reinterpret_cast<uintptr_t>(oldPtr);
        auto it = m_blocks.find(key);
        size_t oldSize = (it != m_blocks.end()) ? it->second.size : 0;

        void* newPtr = AllocAligned(newSize, alignment);
        if (newPtr && oldPtr && oldSize > 0) {
            std::memcpy(newPtr, oldPtr, (std::min)(oldSize, newSize));
        }
        FreeAligned(oldPtr);
        return newPtr;
    }

    static bool IsAligned(const void* ptr, AlignmentBoundary alignment) {
        uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
        uint32_t align = static_cast<uint32_t>(alignment);
        return (addr & (align - 1)) == 0;
    }

    static AlignmentBoundary GetOptimalAlignment() {
#if defined(__AVX512F__) || defined(__AVX512BW__)
        return AlignmentBoundary::Align64;
#elif defined(__AVX2__) || defined(__AVX__)
        return AlignmentBoundary::Align32;
#else
        return AlignmentBoundary::Align16;
#endif
    }

    const SIMDAllocStats& GetStats() const { return m_stats; }

    const AlignedBlock* GetBlockInfo(void* ptr) const {
        uintptr_t key = reinterpret_cast<uintptr_t>(ptr);
        auto it = m_blocks.find(key);
        return (it != m_blocks.end()) ? &it->second : nullptr;
    }

    uint32_t ActiveBlockCount() const { return m_stats.activeBlocks; }

    bool Validate() const {
        if (m_stats.totalAllocations < m_stats.totalFrees) return false;
        if (m_stats.activeBlocks != static_cast<uint32_t>(m_blocks.size())) return false;
        uint64_t expectedCurrent = m_stats.totalBytesAlloc - m_stats.totalBytesFreed;
        if (m_stats.currentBytes != expectedCurrent) return false;
        // Verify all active blocks are actually aligned
        for (const auto& [key, block] : m_blocks) {
            if (!IsAligned(block.ptr, block.alignment)) return false;
            if (!block.inUse) return false;
        }
        return true;
    }

private:
    SIMDMemoryAligner() = default;
    ~SIMDMemoryAligner() {
        // Free all remaining blocks
        for (auto& [key, block] : m_blocks) {
            if (block.ptr) {
#ifdef _MSC_VER
                _aligned_free(block.ptr);
#else
                free(block.ptr);
#endif
            }
        }
        m_blocks.clear();
    }
    SIMDMemoryAligner(const SIMDMemoryAligner&) = delete;
    SIMDMemoryAligner& operator=(const SIMDMemoryAligner&) = delete;

    std::unordered_map<uintptr_t, AlignedBlock> m_blocks;
    SIMDAllocStats                               m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
