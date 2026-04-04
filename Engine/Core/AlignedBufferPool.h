// AlignedBufferPool.h — Cache-line aligned buffer pool for decode operations
// Copyright (c) 2026 ExplorerLens Project
//
// Lock-free, thread-safe buffer pool with pre-allocated pools of common sizes
// (64KB, 256KB, 1MB, 4MB). Uses compare_exchange for lock-free free-list.
// Includes PooledBuffer RAII wrapper and hit/miss statistics tracking.
//
#pragma once

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4324)  // structure was padded due to alignment specifier
#endif

#include <array>
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <new>

namespace ExplorerLens {
namespace Engine {

/// Pool size tiers matching common decode buffer sizes
enum class BufferTier : uint8_t {
    Small = 0,   // 64 KB
    Medium = 1,  // 256 KB
    Large = 2,   // 1 MB
    Huge = 3,    // 4 MB
    COUNT = 4
};

/// Statistics for pool monitoring
struct AlignedPoolStats
{
    std::atomic<uint64_t> hits{0};
    std::atomic<uint64_t> misses{0};
    std::atomic<uint64_t> allocations{0};
    std::atomic<uint64_t> deallocations{0};
    std::atomic<uint64_t> activeBuffers{0};

    double HitRate() const
    {
        uint64_t total = hits.load(std::memory_order_relaxed) + misses.load(std::memory_order_relaxed);
        if (total == 0)
            return 0.0;
        return static_cast<double>(hits.load(std::memory_order_relaxed)) / static_cast<double>(total);
    }

    void Reset()
    {
        hits.store(0, std::memory_order_relaxed);
        misses.store(0, std::memory_order_relaxed);
        allocations.store(0, std::memory_order_relaxed);
        deallocations.store(0, std::memory_order_relaxed);
        activeBuffers.store(0, std::memory_order_relaxed);
    }
};

/// Cache-line alignment constant for aligned buffer pool
static constexpr size_t ALIGNED_POOL_CACHE_LINE = 64;

/// Tier sizes in bytes
static constexpr size_t ALIGNED_POOL_TIER_SIZES[] = {
    64 * 1024,        // 64 KB
    256 * 1024,       // 256 KB
    1 * 1024 * 1024,  // 1 MB
    4 * 1024 * 1024   // 4 MB
};

/// Maximum buffers per tier in the free-list
static constexpr size_t ALIGNED_POOL_MAX_DEPTH = 16;

/// Returns the name of a buffer tier
inline const char* TierName(BufferTier tier)
{
    switch (tier) {
        case BufferTier::Small:
            return "64KB";
        case BufferTier::Medium:
            return "256KB";
        case BufferTier::Large:
            return "1MB";
        case BufferTier::Huge:
            return "4MB";
        default:
            return "Unknown";
    }
}

/// Returns the appropriate tier for a requested size
inline BufferTier TierForSize(size_t requestedBytes)
{
    if (requestedBytes <= ALIGNED_POOL_TIER_SIZES[0])
        return BufferTier::Small;
    if (requestedBytes <= ALIGNED_POOL_TIER_SIZES[1])
        return BufferTier::Medium;
    if (requestedBytes <= ALIGNED_POOL_TIER_SIZES[2])
        return BufferTier::Large;
    if (requestedBytes <= ALIGNED_POOL_TIER_SIZES[3])
        return BufferTier::Huge;
    return BufferTier::COUNT;  // Too large for pool
}

/// Lock-free node for the free-list
struct alignas(ALIGNED_POOL_CACHE_LINE) FreeNode
{
    FreeNode* next = nullptr;
    void* buffer = nullptr;
    size_t capacity = 0;
};

/// Lock-free free-list using compare_exchange
class LockFreeFreeList
{
  public:
    LockFreeFreeList() : m_head(nullptr), m_count(0) {}

    ~LockFreeFreeList()
    {
        // Drain and free all nodes
        FreeNode* node = m_head.load(std::memory_order_relaxed);
        while (node) {
            FreeNode* nodeNext = node->next;
            if (node->buffer) {
                _aligned_free(node->buffer);
            }
            delete node;
            node = nodeNext;
        }
    }

    /// Try to pop a buffer from the free-list (lock-free)
    FreeNode* Pop()
    {
        FreeNode* oldHead = m_head.load(std::memory_order_acquire);
        while (oldHead) {
            if (m_head.compare_exchange_weak(oldHead, oldHead->next, std::memory_order_release,
                                             std::memory_order_relaxed)) {
                m_count.fetch_sub(1, std::memory_order_relaxed);
                oldHead->next = nullptr;
                return oldHead;
            }
        }
        return nullptr;
    }

    /// Push a buffer back to the free-list (lock-free)
    bool Push(FreeNode* node, size_t maxDepth)
    {
        if (m_count.load(std::memory_order_relaxed) >= maxDepth) {
            return false;  // Pool is full
        }
        FreeNode* oldHead = m_head.load(std::memory_order_relaxed);
        do {
            node->next = oldHead;
        } while (!m_head.compare_exchange_weak(oldHead, node, std::memory_order_release, std::memory_order_relaxed));
        m_count.fetch_add(1, std::memory_order_relaxed);
        return true;
    }

    size_t Count() const
    {
        return m_count.load(std::memory_order_relaxed);
    }

  private:
    std::atomic<FreeNode*> m_head;
    std::atomic<size_t> m_count;
};

// Forward declare the pool for RAII wrapper
class AlignedBufferPool;

/// RAII wrapper for pooled buffers — automatically returns to pool on destruction
class AlignedPooledBuffer
{
  public:
    AlignedPooledBuffer() = default;

    AlignedPooledBuffer(void* data, size_t capacity, BufferTier tier, AlignedBufferPool* pool, FreeNode* node)
        : m_data(data)
        , m_capacity(capacity)
        , m_size(0)
        , m_tier(tier)
        , m_pool(pool)
        , m_node(node)
    {}

    ~AlignedPooledBuffer();

    // Move-only
    AlignedPooledBuffer(AlignedPooledBuffer&& other) noexcept
        : m_data(other.m_data)
        , m_capacity(other.m_capacity)
        , m_size(other.m_size)
        , m_tier(other.m_tier)
        , m_pool(other.m_pool)
        , m_node(other.m_node)
    {
        other.m_data = nullptr;
        other.m_pool = nullptr;
        other.m_node = nullptr;
    }

    AlignedPooledBuffer& operator=(AlignedPooledBuffer&& other) noexcept
    {
        if (this != &other) {
            Release();
            m_data = other.m_data;
            m_capacity = other.m_capacity;
            m_size = other.m_size;
            m_tier = other.m_tier;
            m_pool = other.m_pool;
            m_node = other.m_node;
            other.m_data = nullptr;
            other.m_pool = nullptr;
            other.m_node = nullptr;
        }
        return *this;
    }

    AlignedPooledBuffer(const AlignedPooledBuffer&) = delete;
    AlignedPooledBuffer& operator=(const AlignedPooledBuffer&) = delete;

    void* Data() const
    {
        return m_data;
    }
    size_t Capacity() const
    {
        return m_capacity;
    }
    size_t Size() const
    {
        return m_size;
    }
    bool Valid() const
    {
        return m_data != nullptr;
    }
    BufferTier Tier() const
    {
        return m_tier;
    }

    void SetSize(size_t sz)
    {
        m_size = (sz <= m_capacity) ? sz : m_capacity;
    }

    /// Typed access to the buffer
    template <typename T>
    T* As()
    {
        return static_cast<T*>(m_data);
    }

    template <typename T>
    const T* As() const
    {
        return static_cast<const T*>(m_data);
    }

    /// Release back to pool explicitly
    void Release();

  private:
    void* m_data = nullptr;
    size_t m_capacity = 0;
    size_t m_size = 0;
    BufferTier m_tier = BufferTier::Small;
    AlignedBufferPool* m_pool = nullptr;
    FreeNode* m_node = nullptr;
};

/// Cache-line aligned buffer pool for decode operations
class AlignedBufferPool
{
  public:
    static AlignedBufferPool& Instance()
    {
        static AlignedBufferPool instance;
        return instance;
    }

    /// Acquire a buffer of at least `minBytes` size, cache-line aligned
    AlignedPooledBuffer Acquire(size_t minBytes)
    {
        BufferTier tier = TierForSize(minBytes);

        // Try pool first
        if (tier < BufferTier::COUNT) {
            size_t idx = static_cast<size_t>(tier);
            FreeNode* node = m_freeLists[idx].Pop();
            if (node) {
                m_stats.hits.fetch_add(1, std::memory_order_relaxed);
                m_stats.activeBuffers.fetch_add(1, std::memory_order_relaxed);
                return AlignedPooledBuffer(node->buffer, node->capacity, tier, this, node);
            }
            m_stats.misses.fetch_add(1, std::memory_order_relaxed);
        }

        // Allocate a new aligned buffer
        size_t allocSize = (tier < BufferTier::COUNT) ? ALIGNED_POOL_TIER_SIZES[static_cast<size_t>(tier)] : minBytes;
        void* buf = _aligned_malloc(allocSize, ALIGNED_POOL_CACHE_LINE);
        if (!buf) {
            return AlignedPooledBuffer();
        }

        auto* node = new (std::nothrow) FreeNode();
        if (!node) {
            _aligned_free(buf);
            return AlignedPooledBuffer();
        }
        node->buffer = buf;
        node->capacity = allocSize;

        m_stats.allocations.fetch_add(1, std::memory_order_relaxed);
        m_stats.activeBuffers.fetch_add(1, std::memory_order_relaxed);
        return AlignedPooledBuffer(buf, allocSize, tier, this, node);
    }

    /// Return a buffer to the pool
    void Return(FreeNode* node, BufferTier tier)
    {
        if (!node)
            return;
        m_stats.activeBuffers.fetch_sub(1, std::memory_order_relaxed);

        if (tier < BufferTier::COUNT) {
            size_t idx = static_cast<size_t>(tier);
            if (m_freeLists[idx].Push(node, ALIGNED_POOL_MAX_DEPTH)) {
                m_stats.deallocations.fetch_add(1, std::memory_order_relaxed);
                return;
            }
        }

        // Pool full or oversized — free directly
        if (node->buffer) {
            _aligned_free(node->buffer);
        }
        delete node;
        m_stats.deallocations.fetch_add(1, std::memory_order_relaxed);
    }

    /// Get pool statistics
    const AlignedPoolStats& GetStats() const
    {
        return m_stats;
    }

    /// Get free-list depth for a tier
    size_t FreeCount(BufferTier tier) const
    {
        if (tier >= BufferTier::COUNT)
            return 0;
        return m_freeLists[static_cast<size_t>(tier)].Count();
    }

    /// Reset statistics counters
    void ResetStats()
    {
        m_stats.Reset();
    }

    /// Check if a pointer is cache-line aligned
    static bool IsAligned(const void* ptr)
    {
        return (reinterpret_cast<uintptr_t>(ptr) % ALIGNED_POOL_CACHE_LINE) == 0;
    }

  private:
    AlignedBufferPool() = default;
    ~AlignedBufferPool() = default;
    AlignedBufferPool(const AlignedBufferPool&) = delete;
    AlignedBufferPool& operator=(const AlignedBufferPool&) = delete;

    std::array<LockFreeFreeList, static_cast<size_t>(BufferTier::COUNT)> m_freeLists;
    AlignedPoolStats m_stats;
};

// AlignedPooledBuffer implementation (must be after AlignedBufferPool definition)
inline AlignedPooledBuffer::~AlignedPooledBuffer()
{
    Release();
}

inline void AlignedPooledBuffer::Release()
{
    if (m_pool && m_node) {
        m_pool->Return(m_node, m_tier);
        m_data = nullptr;
        m_pool = nullptr;
        m_node = nullptr;
    }
}

}  // namespace Engine
}  // namespace ExplorerLens

#ifdef _MSC_VER
    #pragma warning(pop)
#endif
