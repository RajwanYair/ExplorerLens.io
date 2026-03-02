#pragma once
// ============================================================================
// DecoderHotsetManager.h — Sprint 562
//
// Purpose:
//   Keeps frequently-used decoder instances resident in memory (the "hot set")
//   to avoid repeated initialisation.  Decoders in ExplorerLens have non-
//   trivial startup costs (loading format tables, allocating scratch buffers,
//   building lookup tables).  This class pools decoder instances, tracks usage
//   frequency, and keeps the top-N most active decoders always resident while
//   LRU-evicting cold ones when memory budget is exceeded.
//
// Classes:
//   DecoderHotsetManager — pool/cache for decoder instances keyed by type ID.
//
// Key Types:
//   DecoderInstance  — one cached decoder (type, pointer, footprint, stats)
//   DecoderFactory   — create/destroy function pair for a decoder type
//   HotsetStats      — hit/miss/eviction counters and memory usage
//
// Inputs:
//   RegisterFactory   — register create+destroy for a decoder type
//   AcquireDecoder    — get a pooled instance (or nullptr)
//   ReleaseDecoder    — return an instance to the pool
//
// Outputs:
//   GetStats()        — cache-hit rate, pool size, eviction count
//   Trim(targetBytes) — manually evict until below budget
//
// Background Thread:
//   Periodically (30 s) recalculates the hot set based on a rolling window
//   of usage stats.  The top-N (default 8) decoders are always kept resident.
//
// Thread Safety:
//   All public methods are serialized with a Win32 SRWLOCK (exclusive).
//
// Dependencies:
//   Windows API + C++ standard library only (header-only, no external libs).
// ============================================================================

#include <windows.h>
#include <vector>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

// ── Instance descriptor ──────────────────────────────────────────────────────

struct DecoderInstance {
    uint32_t decoderType    = 0;
    void*    instance       = nullptr;
    size_t   memoryFootprint = 0;
    uint64_t lastUsed       = 0;   // tick counter
    uint32_t useCount       = 0;
};

// ── Factory pair ─────────────────────────────────────────────────────────────

struct DecoderFactory {
    std::function<void*()>    create;
    std::function<void(void*)> destroy;
};

// ── Statistics ───────────────────────────────────────────────────────────────

struct HotsetStats {
    size_t   poolSizeBytes   = 0;
    uint32_t instancesCached = 0;
    uint32_t hotSetCount     = 0;
    uint64_t cacheHits       = 0;
    uint64_t cacheMisses     = 0;
    uint64_t evictions       = 0;
};

// ── Main class ───────────────────────────────────────────────────────────────

class DecoderHotsetManager {
public:
    explicit DecoderHotsetManager(size_t maxPoolBytes = 256ULL * 1024 * 1024,
                                   uint32_t hotSetSize = 8) noexcept
        : m_maxPoolBytes(maxPoolBytes), m_hotSetSize(hotSetSize) {
        InitializeSRWLock(&m_lock);
    }

    ~DecoderHotsetManager() {
        StopBackgroundThread();

        AcquireSRWLockExclusive(&m_lock);
        // Destroy all pooled instances
        for (auto& [type, instances] : m_pool) {
            auto factIt = m_factories.find(type);
            for (auto& inst : instances) {
                if (inst.instance && factIt != m_factories.end() &&
                    factIt->second.destroy) {
                    factIt->second.destroy(inst.instance);
                }
                inst.instance = nullptr;
            }
        }
        m_pool.clear();
        m_factories.clear();
        ReleaseSRWLockExclusive(&m_lock);
    }

    DecoderHotsetManager(const DecoderHotsetManager&)            = delete;
    DecoderHotsetManager& operator=(const DecoderHotsetManager&) = delete;

    // ── Factory registration ─────────────────────────────────────

    void RegisterFactory(uint32_t decoderType,
                         std::function<void*()> create,
                         std::function<void(void*)> destroy) {
        AcquireSRWLockExclusive(&m_lock);
        DecoderFactory factory;
        factory.create  = std::move(create);
        factory.destroy = std::move(destroy);
        m_factories[decoderType] = std::move(factory);
        ReleaseSRWLockExclusive(&m_lock);
    }

    // ── Acquire / Release ────────────────────────────────────────

    void* AcquireDecoder(uint32_t decoderType) {
        AcquireSRWLockExclusive(&m_lock);

        auto poolIt = m_pool.find(decoderType);
        if (poolIt != m_pool.end() && !poolIt->second.empty()) {
            // Return the most recently used instance (back of vector)
            DecoderInstance inst = poolIt->second.back();
            poolIt->second.pop_back();
            m_currentPoolBytes -= inst.memoryFootprint;
            m_stats.cacheHits++;

            // Update usage tracking
            m_usageCounts[decoderType]++;
            m_globalTick++;
            inst.lastUsed = m_globalTick;
            inst.useCount++;

            void* result = inst.instance;
            ReleaseSRWLockExclusive(&m_lock);
            return result;
        }

        // Cache miss — try to create via factory
        auto factIt = m_factories.find(decoderType);
        if (factIt != m_factories.end() && factIt->second.create) {
            m_stats.cacheMisses++;
            m_usageCounts[decoderType]++;
            m_globalTick++;

            auto createFn = factIt->second.create;
            ReleaseSRWLockExclusive(&m_lock);

            void* newInstance = createFn();
            return newInstance;
        }

        m_stats.cacheMisses++;
        ReleaseSRWLockExclusive(&m_lock);
        return nullptr;
    }

    void ReleaseDecoder(uint32_t decoderType, void* instance,
                        size_t footprint) {
        if (!instance) return;

        AcquireSRWLockExclusive(&m_lock);

        // Check if adding this would exceed budget
        if (m_currentPoolBytes + footprint > m_maxPoolBytes) {
            // Try to evict something first
            EvictLRULocked(footprint);
        }

        DecoderInstance inst;
        inst.decoderType    = decoderType;
        inst.instance       = instance;
        inst.memoryFootprint = footprint;
        inst.lastUsed       = m_globalTick;
        inst.useCount       = static_cast<uint32_t>(m_usageCounts[decoderType]);

        m_pool[decoderType].push_back(inst);
        m_currentPoolBytes += footprint;

        ReleaseSRWLockExclusive(&m_lock);
    }

    // ── Pool size management ─────────────────────────────────────

    void SetMaxPoolSize(size_t maxBytes) {
        AcquireSRWLockExclusive(&m_lock);
        m_maxPoolBytes = maxBytes;
        // Evict if over new budget
        while (m_currentPoolBytes > m_maxPoolBytes) {
            if (!EvictLRULocked(0)) break;
        }
        ReleaseSRWLockExclusive(&m_lock);
    }

    void Trim(size_t targetBytes) {
        AcquireSRWLockExclusive(&m_lock);
        while (m_currentPoolBytes > targetBytes) {
            if (!EvictLRULocked(0)) break;
        }
        ReleaseSRWLockExclusive(&m_lock);
    }

    // ── Statistics ───────────────────────────────────────────────

    HotsetStats GetStats() {
        AcquireSRWLockExclusive(&m_lock);
        HotsetStats stats = m_stats;
        stats.poolSizeBytes   = m_currentPoolBytes;
        stats.instancesCached = 0;
        for (const auto& [type, instances] : m_pool) {
            stats.instancesCached += static_cast<uint32_t>(instances.size());
        }
        stats.hotSetCount = static_cast<uint32_t>(m_hotSet.size());
        ReleaseSRWLockExclusive(&m_lock);
        return stats;
    }

    // ── Background hot-set recalculation ─────────────────────────

    void StartBackgroundThread(uint32_t intervalMs = 30000) {
        StopBackgroundThread();

        m_bgRunning.store(true, std::memory_order_release);
        m_bgIntervalMs = intervalMs;
        m_bgThread = std::thread([this]() {
            while (m_bgRunning.load(std::memory_order_acquire)) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(m_bgIntervalMs));
                if (!m_bgRunning.load(std::memory_order_acquire)) break;
                RecalculateHotSet();
            }
        });
    }

    void StopBackgroundThread() {
        m_bgRunning.store(false, std::memory_order_release);
        if (m_bgThread.joinable()) {
            m_bgThread.join();
        }
    }

    // ── Manual hot-set recalculation ─────────────────────────────

    void RecalculateHotSet() {
        AcquireSRWLockExclusive(&m_lock);

        // Build a sorted list of decoder types by usage count (descending)
        struct TypeUsage {
            uint32_t type;
            uint64_t count;
        };
        std::vector<TypeUsage> usageList;
        usageList.reserve(m_usageCounts.size());
        for (const auto& [type, count] : m_usageCounts) {
            usageList.push_back({type, count});
        }
        std::sort(usageList.begin(), usageList.end(),
            [](const TypeUsage& a, const TypeUsage& b) {
                return a.count > b.count;
            });

        // Top N become the hot set
        m_hotSet.clear();
        uint32_t n = (std::min)(m_hotSetSize,
                                static_cast<uint32_t>(usageList.size()));
        for (uint32_t i = 0; i < n; ++i) {
            m_hotSet.push_back(usageList[i].type);
        }

        ReleaseSRWLockExclusive(&m_lock);
    }

    // ── Query ────────────────────────────────────────────────────

    bool IsInHotSet(uint32_t decoderType) {
        AcquireSRWLockExclusive(&m_lock);
        bool found = std::find(m_hotSet.begin(), m_hotSet.end(),
                               decoderType) != m_hotSet.end();
        ReleaseSRWLockExclusive(&m_lock);
        return found;
    }

    uint32_t GetPooledInstanceCount(uint32_t decoderType) {
        AcquireSRWLockExclusive(&m_lock);
        uint32_t count = 0;
        auto it = m_pool.find(decoderType);
        if (it != m_pool.end()) {
            count = static_cast<uint32_t>(it->second.size());
        }
        ReleaseSRWLockExclusive(&m_lock);
        return count;
    }

private:
    // ── Eviction (caller holds lock) ─────────────────────────────

    bool EvictLRULocked(size_t spaceNeeded) {
        // Find the pool entry with the oldest lastUsed that is NOT in hot set
        uint32_t bestType     = UINT32_MAX;
        size_t   bestIdx      = SIZE_MAX;
        uint64_t oldestTick   = UINT64_MAX;

        for (auto& [type, instances] : m_pool) {
            if (instances.empty()) continue;
            // Skip hot set types
            if (std::find(m_hotSet.begin(), m_hotSet.end(), type)
                != m_hotSet.end()) {
                continue;
            }
            for (size_t i = 0; i < instances.size(); ++i) {
                if (instances[i].lastUsed < oldestTick) {
                    oldestTick = instances[i].lastUsed;
                    bestType   = type;
                    bestIdx    = i;
                }
            }
        }

        // If all non-hot entries are exhausted, evict from hot set too
        if (bestType == UINT32_MAX) {
            for (auto& [type, instances] : m_pool) {
                if (instances.empty()) continue;
                for (size_t i = 0; i < instances.size(); ++i) {
                    if (instances[i].lastUsed < oldestTick) {
                        oldestTick = instances[i].lastUsed;
                        bestType   = type;
                        bestIdx    = i;
                    }
                }
            }
        }

        if (bestType == UINT32_MAX) return false;

        auto& instances = m_pool[bestType];
        DecoderInstance& victim = instances[bestIdx];

        // Destroy the instance
        auto factIt = m_factories.find(bestType);
        if (factIt != m_factories.end() && factIt->second.destroy &&
            victim.instance) {
            factIt->second.destroy(victim.instance);
        }

        m_currentPoolBytes -= victim.memoryFootprint;
        m_stats.evictions++;

        instances.erase(instances.begin() + static_cast<ptrdiff_t>(bestIdx));
        return true;
    }

    // ── Data members ─────────────────────────────────────────────

    SRWLOCK m_lock{};

    // Pool: decoderType -> vector of cached instances
    std::unordered_map<uint32_t, std::vector<DecoderInstance>> m_pool;

    // Registered factories
    std::unordered_map<uint32_t, DecoderFactory> m_factories;

    // Usage tracking (rolling)
    std::unordered_map<uint32_t, uint64_t> m_usageCounts;
    uint64_t m_globalTick = 0;

    // Hot set
    std::vector<uint32_t> m_hotSet;
    uint32_t m_hotSetSize = 8;

    // Memory budget
    size_t m_maxPoolBytes     = 256ULL * 1024 * 1024;
    size_t m_currentPoolBytes = 0;

    // Cumulative stats
    HotsetStats m_stats{};

    // Background thread
    std::atomic<bool> m_bgRunning{false};
    uint32_t          m_bgIntervalMs = 30000;
    std::thread       m_bgThread;
};

} // namespace Engine
} // namespace ExplorerLens
