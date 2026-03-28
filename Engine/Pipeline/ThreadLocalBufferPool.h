// ThreadLocalBufferPool.h — Thread-Local Decode Buffer Pool
// Copyright (c) 2026 ExplorerLens Project
//
// Provides per-thread buffer pools for decode operations, eliminating
// heap allocation contention in multi-threaded thumbnail generation.
// Each thread gets its own recycling buffer pool with size-class binning.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <vector>
#include <array>

namespace ExplorerLens {
namespace Engine {

struct ThreadLocalPoolStats {
    uint64_t allocations = 0;
    uint64_t recycledHits = 0;
    uint64_t totalBytesAllocated = 0;
    uint64_t peakBytesUsed = 0;
    double   hitRatePercent = 0.0;
};

class ThreadLocalBufferPool {
public:
    static constexpr uint32_t NUM_SIZE_CLASSES = 8;
    static constexpr uint32_t MAX_BUFFERS_PER_CLASS = 4;
    // Size classes: 4KB, 16KB, 64KB, 256KB, 1MB, 4MB, 16MB, 64MB
    static constexpr uint32_t SizeClasses[NUM_SIZE_CLASSES] = {
        4096, 16384, 65536, 262144, 1048576, 4194304, 16777216, 67108864
    };

    ThreadLocalBufferPool() { InitializeSRWLock(&m_lock); }
    ~ThreadLocalBufferPool() { Reset(); }

    static const wchar_t* GetName() { return L"ThreadLocalBufferPool"; }

    /// Find the size class index for a given size.
    uint32_t FindSizeClass(uint32_t size) const {
        for (uint32_t i = 0; i < NUM_SIZE_CLASSES; ++i)
            if (size <= SizeClasses[i]) return i;
        return NUM_SIZE_CLASSES; // Too large for pool
    }

    /// Acquire a buffer of at least the requested size.
    uint8_t* Acquire(uint32_t size) {
        uint32_t classIdx = FindSizeClass(size);
        if (classIdx >= NUM_SIZE_CLASSES) {
            // Fallback to heap for oversized requests
            m_stats.allocations++;
            m_stats.totalBytesAllocated += size;
            return new uint8_t[size];
        }

        AcquireSRWLockExclusive(&m_lock);
        auto& pool = m_pools[classIdx];
        uint8_t* buf = nullptr;
        if (!pool.empty()) {
            buf = pool.back();
            pool.pop_back();
            m_stats.recycledHits++;
        }
        ReleaseSRWLockExclusive(&m_lock);

        if (!buf) {
            buf = new uint8_t[SizeClasses[classIdx]];
            m_stats.totalBytesAllocated += SizeClasses[classIdx];
        }
        m_stats.allocations++;
        return buf;
    }

    /// Return a buffer to the pool for recycling.
    void Release(uint8_t* buf, uint32_t size) {
        if (!buf) return;
        uint32_t classIdx = FindSizeClass(size);
        if (classIdx >= NUM_SIZE_CLASSES) {
            delete[] buf;
            return;
        }

        AcquireSRWLockExclusive(&m_lock);
        auto& pool = m_pools[classIdx];
        if (pool.size() < MAX_BUFFERS_PER_CLASS) {
            pool.push_back(buf);
            buf = nullptr;
        }
        ReleaseSRWLockExclusive(&m_lock);

        if (buf) delete[] buf;
    }

    /// Reset all pools, freeing all buffers.
    void Reset() {
        AcquireSRWLockExclusive(&m_lock);
        for (auto& pool : m_pools) {
            for (auto* buf : pool) delete[] buf;
            pool.clear();
        }
        ReleaseSRWLockExclusive(&m_lock);
    }

    ThreadLocalPoolStats GetStats() const {
        ThreadLocalPoolStats s = m_stats;
        if (s.allocations > 0)
            s.hitRatePercent = 100.0 * s.recycledHits / s.allocations;
        return s;
    }

private:
    SRWLOCK m_lock{};
    std::array<std::vector<uint8_t*>, NUM_SIZE_CLASSES> m_pools;
    mutable ThreadLocalPoolStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
