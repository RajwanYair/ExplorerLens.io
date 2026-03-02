#pragma once
// Buffer Pool & Slab Allocator
// Decode buffer reuse by dimension/format class to reduce heap churn
// during Explorer thumbnail batch operations.

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <chrono>
#include <mutex>
#include <algorithm>
#include <unordered_map>

namespace ExplorerLens::Memory {

// ─── Slab size classes ──────────────────────────────────────────────
enum class SlabClass : uint8_t {
    Tiny = 0, // ≤ 64×64 (16 KB)
    Small = 1, // ≤ 128×128 (64 KB)
    Medium = 2, // ≤ 256×256 (256 KB)
    Large = 3, // ≤ 512×512 (1 MB)
    XLarge = 4, // ≤ 1024×1024 (4 MB)
    Huge = 5, // > 1024×1024 (16 MB cap)
    COUNT = 6
};

inline const char* SlabClassName(SlabClass c) {
    switch (c) {
    case SlabClass::Tiny: return "Tiny (≤64x64)";
    case SlabClass::Small: return "Small (≤128x128)";
    case SlabClass::Medium: return "Medium (≤256x256)";
    case SlabClass::Large: return "Large (≤512x512)";
    case SlabClass::XLarge: return "XLarge (≤1024x1024)";
    case SlabClass::Huge: return "Huge (>1024x1024)";
    default: return "Unknown";
    }
}

inline size_t SlabClassBufferSize(SlabClass c) {
    switch (c) {
    case SlabClass::Tiny: return 64 * 64 * 4; // 16 KB
    case SlabClass::Small: return 128 * 128 * 4; // 64 KB
    case SlabClass::Medium: return 256 * 256 * 4; // 256 KB
    case SlabClass::Large: return 512 * 512 * 4; // 1 MB
    case SlabClass::XLarge: return 1024 * 1024 * 4; // 4 MB
    case SlabClass::Huge: return 2048 * 2048 * 4; // 16 MB
    default: return 0;
    }
}

inline SlabClass ClassifyDimension(uint32_t width, uint32_t height) {
    uint32_t maxDim = (width > height) ? width : height;
    if (maxDim <= 64) return SlabClass::Tiny;
    if (maxDim <= 128) return SlabClass::Small;
    if (maxDim <= 256) return SlabClass::Medium;
    if (maxDim <= 512) return SlabClass::Large;
    if (maxDim <= 1024) return SlabClass::XLarge;
    return SlabClass::Huge;
}

// ─── Buffer handle ─────────────────────────────────────────────────
struct PooledBuffer {
    uint8_t* data = nullptr;
    size_t capacity = 0;
    SlabClass slabClass = SlabClass::Tiny;
    uint64_t poolId = 0; // identifies originating pool
    bool fromPool = false;

    bool IsValid() const { return data != nullptr && capacity > 0; }
    void Clear() { if (data) std::fill(data, data + capacity, static_cast<uint8_t>(0)); }
};

// ─── Per-class slab pool ──────────────────────────────────────────
struct SlabPoolStats {
    size_t totalAllocated = 0;
    size_t totalReused = 0;
    size_t currentFree = 0;
    size_t currentInUse = 0;
    size_t peakInUse = 0;
    size_t totalBytes = 0;

    double ReuseRate() const {
        size_t total = totalAllocated + totalReused;
        return total > 0 ? static_cast<double>(totalReused) / total : 0.0;
    }
};

class SlabPool {
public:
    explicit SlabPool(SlabClass sc, size_t maxFreeBuffers = 16)
        : m_class(sc)
        , m_bufferSize(SlabClassBufferSize(sc))
        , m_maxFree(maxFreeBuffers) {
    }

    PooledBuffer Acquire() {
        std::lock_guard<std::mutex> lock(m_mutex);

        PooledBuffer buf;
        buf.slabClass = m_class;
        buf.poolId = reinterpret_cast<uint64_t>(this);

        if (!m_freeList.empty()) {
            buf.data = m_freeList.back();
            buf.capacity = m_bufferSize;
            buf.fromPool = true;
            m_freeList.pop_back();
            m_stats.totalReused++;
        }
        else {
            buf.data = new uint8_t[m_bufferSize];
            buf.capacity = m_bufferSize;
            buf.fromPool = true;
            m_stats.totalAllocated++;
            m_stats.totalBytes += m_bufferSize;
        }

        m_stats.currentInUse++;
        m_stats.currentFree = m_freeList.size();
        if (m_stats.currentInUse > m_stats.peakInUse)
            m_stats.peakInUse = m_stats.currentInUse;

        return buf;
    }

    bool Release(PooledBuffer& buf) {
        if (buf.slabClass != m_class || buf.poolId != reinterpret_cast<uint64_t>(this))
            return false;

        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_freeList.size() < m_maxFree) {
            m_freeList.push_back(buf.data);
        }
        else {
            delete[] buf.data;
            m_stats.totalBytes -= m_bufferSize;
        }

        m_stats.currentInUse--;
        m_stats.currentFree = m_freeList.size();
        buf.data = nullptr;
        buf.capacity = 0;
        return true;
    }

    void DrainFreeList() {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto* p : m_freeList) delete[] p;
        m_stats.totalBytes -= m_freeList.size() * m_bufferSize;
        m_freeList.clear();
        m_stats.currentFree = 0;
    }

    SlabPoolStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    SlabClass GetClass() const { return m_class; }
    size_t BufferSize() const { return m_bufferSize; }

    ~SlabPool() {
        for (auto* p : m_freeList) delete[] p;
        m_freeList.clear();
    }

    // Non-copyable (has mutex)
    SlabPool(const SlabPool&) = delete;
    SlabPool& operator=(const SlabPool&) = delete;

    // Moveable (safe only during construction before any thread uses the pool)
    SlabPool(SlabPool&& other) noexcept
        : m_class(other.m_class)
        , m_bufferSize(other.m_bufferSize)
        , m_maxFree(other.m_maxFree)
        , m_freeList(std::move(other.m_freeList))
        , m_stats(other.m_stats) {
    }
    SlabPool& operator=(SlabPool&&) = delete;

private:
    SlabClass m_class;
    size_t m_bufferSize;
    size_t m_maxFree;
    std::vector<uint8_t*> m_freeList;
    SlabPoolStats m_stats;
    mutable std::mutex m_mutex;
};

// ─── Aggregate pool statistics ────────────────────────────────────
struct BufferPoolStats {
    size_t totalAcquires = 0;
    size_t totalReleases = 0;
    size_t totalReuses = 0;
    size_t totalFreshAllocs = 0;
    size_t totalMemoryBytes = 0;
    size_t peakMemoryBytes = 0;

    double OverallReuseRate() const {
        size_t total = totalReuses + totalFreshAllocs;
        return total > 0 ? static_cast<double>(totalReuses) / total : 0.0;
    }

    std::string Summary() const {
        return "Acquires=" + std::to_string(totalAcquires) +
            " Releases=" + std::to_string(totalReleases) +
            " ReuseRate=" + std::to_string(static_cast<int>(OverallReuseRate() * 100)) + "%";
    }
};

// ─── Buffer Pool Configuration ────────────────────────────────────
struct BufferPoolConfig {
    size_t maxFreePerClass = 16;
    size_t globalMemoryLimitBytes = 64 * 1024 * 1024; // 64 MB
    bool zeroOnRelease = false;

    static BufferPoolConfig Default() { return {}; }
    static BufferPoolConfig LowMemory() {
        BufferPoolConfig c;
        c.maxFreePerClass = 4;
        c.globalMemoryLimitBytes = 16 * 1024 * 1024;
        return c;
    }
    static BufferPoolConfig HighThroughput() {
        BufferPoolConfig c;
        c.maxFreePerClass = 32;
        c.globalMemoryLimitBytes = 128 * 1024 * 1024;
        return c;
    }
};

// ─── Buffer Pool (main interface) ─────────────────────────────────
class BufferPool {
public:
    explicit BufferPool(BufferPoolConfig config = BufferPoolConfig::Default())
        : m_config(config) {
        for (int i = 0; i < static_cast<int>(SlabClass::COUNT); i++) {
            m_pools.emplace_back(static_cast<SlabClass>(i), config.maxFreePerClass);
        }
    }

    PooledBuffer Acquire(uint32_t width, uint32_t height) {
        SlabClass sc = ClassifyDimension(width, height);
        return AcquireByClass(sc);
    }

    PooledBuffer AcquireByClass(SlabClass sc) {
        auto& pool = m_pools[static_cast<int>(sc)];
        auto buf = pool.Acquire();
        m_stats.totalAcquires++;

        auto poolStats = pool.GetStats();
        if (poolStats.totalReused > m_lastReused[static_cast<int>(sc)]) {
            m_stats.totalReuses++;
        }
        else {
            m_stats.totalFreshAllocs++;
        }
        m_lastReused[static_cast<int>(sc)] = poolStats.totalReused;

        UpdateMemoryStats();
        return buf;
    }

    bool Release(PooledBuffer& buf) {
        if (m_config.zeroOnRelease && buf.data) {
            buf.Clear();
        }

        auto& pool = m_pools[static_cast<int>(buf.slabClass)];
        bool ok = pool.Release(buf);
        if (ok) m_stats.totalReleases++;
        UpdateMemoryStats();
        return ok;
    }

    void DrainAll() {
        for (auto& pool : m_pools) {
            pool.DrainFreeList();
        }
        UpdateMemoryStats();
    }

    void DrainClass(SlabClass sc) {
        m_pools[static_cast<int>(sc)].DrainFreeList();
        UpdateMemoryStats();
    }

    BufferPoolStats GetStats() const { return m_stats; }
    BufferPoolConfig GetConfig() const { return m_config; }

    SlabPoolStats GetClassStats(SlabClass sc) const {
        return m_pools[static_cast<int>(sc)].GetStats();
    }

    size_t PoolCount() const { return m_pools.size(); }

    static BufferPool Create(BufferPoolConfig config = BufferPoolConfig::Default()) {
        return BufferPool(config);
    }

private:
    void UpdateMemoryStats() {
        size_t total = 0;
        for (auto& pool : m_pools) {
            total += pool.GetStats().totalBytes;
        }
        m_stats.totalMemoryBytes = total;
        if (total > m_stats.peakMemoryBytes) m_stats.peakMemoryBytes = total;
    }

    BufferPoolConfig m_config;
    std::vector<SlabPool> m_pools;
    BufferPoolStats m_stats;
    size_t m_lastReused[static_cast<int>(SlabClass::COUNT)] = {};
};

} // namespace ExplorerLens::Memory
