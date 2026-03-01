// ============================================================================
// SmartPointerPool.h — Intrusive Reference-Counted Object Pool
// ExplorerLens Engine v15.0.0
// Copyright (c) 2026 ExplorerLens Project
//
// High-performance object pool with intrusive reference counting for
// decoder and pipeline objects. Eliminates heap allocation overhead for
// frequently created/destroyed objects. Supports size-class bucketing,
// thread-safe acquire/release, and automatic compaction under pressure.
// ============================================================================

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <cassert>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Size class bucketing
// ============================================================================

enum class PoolSizeClass : uint8_t {
    Tiny = 0,  // 0 - 64 bytes
    Small = 1,  // 65 - 256 bytes
    Medium = 2,  // 257 - 1024 bytes
    Large = 3,  // 1025 - 4096 bytes
    Huge = 4,  // 4097 - 65536 bytes
    Custom = 5   // > 65536 bytes (fallback to heap)
};

inline const char* PoolSizeClassToString(PoolSizeClass cls) {
    static const char* names[] = {
        "Tiny", "Small", "Medium", "Large", "Huge", "Custom"
    };
    return names[static_cast<uint8_t>(cls)];
}

inline PoolSizeClass ClassifySize(size_t size) {
    if (size <= 64) return PoolSizeClass::Tiny;
    if (size <= 256) return PoolSizeClass::Small;
    if (size <= 1024) return PoolSizeClass::Medium;
    if (size <= 4096) return PoolSizeClass::Large;
    if (size <= 65536) return PoolSizeClass::Huge;
    return PoolSizeClass::Custom;
}

inline size_t GetBucketCapacity(PoolSizeClass cls) {
    static const size_t capacities[] = { 64, 256, 1024, 4096, 65536, 0 };
    return capacities[static_cast<uint8_t>(cls)];
}

// ============================================================================
// Pool block (internal allocation unit)
// ============================================================================

struct PoolBlock {
    void* data = nullptr;
    size_t          size = 0;
    PoolSizeClass   sizeClass = PoolSizeClass::Tiny;
    std::atomic<int32_t> refCount{ 0 };
    bool            inUse = false;
    uint64_t        allocationId = 0;
    uint64_t        lastAccessTime = 0;

    PoolBlock() = default;
    PoolBlock(PoolBlock&& other) noexcept
        : data(other.data), size(other.size), sizeClass(other.sizeClass),
        refCount(other.refCount.load()), inUse(other.inUse),
        allocationId(other.allocationId), lastAccessTime(other.lastAccessTime) {
        other.data = nullptr;
    }
    PoolBlock& operator=(PoolBlock&& other) noexcept {
        if (this != &other) {
            data = other.data; size = other.size; sizeClass = other.sizeClass;
            refCount.store(other.refCount.load()); inUse = other.inUse;
            allocationId = other.allocationId; lastAccessTime = other.lastAccessTime;
            other.data = nullptr;
        }
        return *this;
    }
    PoolBlock(const PoolBlock&) = delete;
    PoolBlock& operator=(const PoolBlock&) = delete;

    bool IsAvailable() const { return !inUse && refCount.load() == 0; }

    void AddRef() { refCount.fetch_add(1, std::memory_order_relaxed); }
    int32_t Release() { return refCount.fetch_sub(1, std::memory_order_acq_rel) - 1; }
    int32_t GetRefCount() const { return refCount.load(std::memory_order_relaxed); }
};

// ============================================================================
// Pool statistics
// ============================================================================

struct SmartPoolStats {
    uint64_t totalAllocations = 0;
    uint64_t totalDeallocations = 0;
    uint64_t poolHits = 0;          // Served from pool (no new alloc)
    uint64_t poolMisses = 0;        // Required new allocation
    uint64_t compactionCount = 0;
    uint64_t totalBytesAllocated = 0;
    uint64_t totalBytesInUse = 0;
    uint32_t blocksInPool = 0;
    uint32_t blocksInUse = 0;
    std::array<uint32_t, 6> perClassCount = {};

    double GetHitRate() const {
        uint64_t total = poolHits + poolMisses;
        return (total > 0) ? (static_cast<double>(poolHits) / total * 100.0) : 0.0;
    }

    double GetUtilization() const {
        return (totalBytesAllocated > 0)
            ? (static_cast<double>(totalBytesInUse) / totalBytesAllocated * 100.0)
            : 0.0;
    }
};

// ============================================================================
// SmartPointerPool — main class
// ============================================================================

class SmartPointerPool {
public:
    SmartPointerPool() = default;
    ~SmartPointerPool() { Shutdown(); }

    /// Initialize with maximum pool size
    bool Initialize(size_t maxPoolBytes = 16 * 1024 * 1024) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_maxPoolBytes = maxPoolBytes;
        m_initialized = true;
        return true;
    }

    bool IsInitialized() const { return m_initialized; }

    /// Acquire a block of at least `size` bytes
    PoolBlock* Acquire(size_t size) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.totalAllocations++;

        PoolSizeClass cls = ClassifySize(size);
        size_t bucketSize = GetBucketCapacity(cls);
        if (bucketSize == 0) bucketSize = size;  // Custom class

        // Search for available block in pool
        for (auto& block : m_blocks) {
            if (block.IsAvailable() && block.sizeClass == cls && block.size >= size) {
                block.inUse = true;
                block.AddRef();
                block.allocationId = ++m_nextId;
                m_stats.poolHits++;
                m_stats.blocksInUse++;
                m_stats.totalBytesInUse += block.size;
                m_stats.perClassCount[static_cast<uint8_t>(cls)]++;
                return &block;
            }
        }

        // No available block — allocate new
        m_stats.poolMisses++;

        PoolBlock block;
        block.data = ::operator new(bucketSize);
        block.size = bucketSize;
        block.sizeClass = cls;
        block.inUse = true;
        block.refCount.store(1);
        block.allocationId = ++m_nextId;

        m_blocks.push_back(std::move(block));
        m_stats.blocksInPool = static_cast<uint32_t>(m_blocks.size());
        m_stats.blocksInUse++;
        m_stats.totalBytesAllocated += bucketSize;
        m_stats.totalBytesInUse += bucketSize;
        m_stats.perClassCount[static_cast<uint8_t>(cls)]++;

        return &m_blocks.back();
    }

    /// Release a block back to the pool
    void Release(PoolBlock* block) {
        if (!block) return;
        std::lock_guard<std::mutex> lock(m_mutex);

        int32_t newRef = block->Release();
        if (newRef <= 0) {
            block->inUse = false;
            m_stats.totalDeallocations++;
            m_stats.blocksInUse--;
            m_stats.totalBytesInUse -= block->size;
            uint8_t cls = static_cast<uint8_t>(block->sizeClass);
            if (m_stats.perClassCount[cls] > 0) m_stats.perClassCount[cls]--;
        }
    }

    /// Compact the pool by freeing unused blocks
    uint32_t Compact() {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint32_t freed = 0;

        auto it = m_blocks.begin();
        while (it != m_blocks.end()) {
            if (it->IsAvailable()) {
                ::operator delete(it->data);
                m_stats.totalBytesAllocated -= it->size;
                it = m_blocks.erase(it);
                freed++;
            }
            else {
                ++it;
            }
        }

        m_stats.blocksInPool = static_cast<uint32_t>(m_blocks.size());
        m_stats.compactionCount++;
        return freed;
    }

    /// Get pool statistics
    const SmartPoolStats& GetStats() const { return m_stats; }

    /// Shutdown and free all memory
    void Shutdown() {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& block : m_blocks) {
            ::operator delete(block.data);
        }
        m_blocks.clear();
        m_stats = {};
        m_initialized = false;
    }

private:
    bool m_initialized = false;
    size_t m_maxPoolBytes = 16 * 1024 * 1024;
    uint64_t m_nextId = 0;
    std::mutex m_mutex;
    std::vector<PoolBlock> m_blocks;
    SmartPoolStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
