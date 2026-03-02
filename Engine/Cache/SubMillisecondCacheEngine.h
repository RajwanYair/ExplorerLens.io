#pragma once
// ============================================================================
// SubMillisecondCacheEngine.h — Ultra-fast in-memory thumbnail cache (Sprint 544)
//
// Purpose:   Robin-hood open-addressing hash map for sub-millisecond
//            thumbnail cache lookups. Uses FNV-1a 64-bit hashing, SRWLOCK
//            for thread safety, and QueryPerformanceCounter for timing.
//
// Classes:   SubMillisecondCacheEngine
// Enums:     CacheHashAlgo
// Structs:   SubMsCacheStats, SubMsCacheEntry
//
// Inputs:    Thumbnail key (wstring), raw pixel data (uint8_t*), TTL in ms
// Outputs:   Cached thumbnail data, hit/miss statistics
//
// Threading: Thread-safe via SRWLOCK (slim reader-writer lock).
//            Reads acquire shared; writes acquire exclusive.
//
// Eviction:  TTL-based expiration + LRU eviction when capacity reached.
// ============================================================================

#include <windows.h>
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <cstring>

namespace ExplorerLens {
namespace Engine {

/// Hash algorithm selection for cache key hashing
enum class CacheHashAlgo : uint8_t {
    XXH3 = 0,  // Fast non-crypto hash (simulated via FNV variant)
    FNV1a = 1,  // Fowler-Noll-Vo 1a — 64-bit
    CityHash = 2   // Google CityHash-style mix
};

/// Cache performance statistics
struct SubMsCacheStats {
    uint64_t hitCount = 0;
    uint64_t missCount = 0;
    uint64_t evictionCount = 0;
    uint64_t entryCount = 0;
    uint64_t totalBytes = 0;
    uint64_t peakBytes = 0;
    double   hitRatePercent = 0.0;
    double   avgLookupUs = 0.0;   // average lookup time in microseconds
};

/// Internal cache entry stored in the robin-hood table
struct SubMsCacheEntry {
    std::wstring           key;
    std::vector<uint8_t>   data;
    uint64_t               hash = 0;
    uint64_t               insertTimeTick = 0;
    uint64_t               lastAccessTick = 0;
    uint32_t               ttlMs = 0;
    uint8_t                probeDistance = 0;
    bool                   occupied = false;
};

/// Ultra-fast in-memory thumbnail cache using robin-hood open-addressing.
/// Max probe distance of 16 keeps worst-case lookup bounded.
class SubMillisecondCacheEngine {
public:
    static constexpr uint32_t DEFAULT_CAPACITY = 10000;
    static constexpr uint8_t  MAX_PROBE = 16;

    explicit SubMillisecondCacheEngine(uint32_t capacity = DEFAULT_CAPACITY,
        CacheHashAlgo algo = CacheHashAlgo::FNV1a)
        : m_capacity(capacity < 64 ? 64 : capacity)
        , m_algo(algo) {
        InitializeSRWLock(&m_lock);
        QueryPerformanceFrequency(&m_freq);
        // Round capacity up to next power of two for mask-based indexing
        uint32_t pot = 1;
        while (pot < m_capacity) pot <<= 1;
        m_capacity = pot;
        m_mask = m_capacity - 1;
        m_table.resize(m_capacity);
    }

    ~SubMillisecondCacheEngine() = default;

    SubMillisecondCacheEngine(const SubMillisecondCacheEngine&) = delete;
    SubMillisecondCacheEngine& operator=(const SubMillisecondCacheEngine&) = delete;

    /// Retrieve a cached entry by key. Returns true if found and not expired.
    inline bool Get(const std::wstring& key, std::vector<uint8_t>& outData) {
        LARGE_INTEGER startTick;
        QueryPerformanceCounter(&startTick);

        AcquireSRWLockShared(&m_lock);
        const uint64_t h = HashKey(key);
        uint32_t idx = static_cast<uint32_t>(h & m_mask);
        bool found = false;

        for (uint8_t probe = 0; probe < MAX_PROBE; ++probe) {
            const uint32_t slot = (idx + probe) & m_mask;
            const SubMsCacheEntry& entry = m_table[slot];
            if (!entry.occupied) break;
            if (entry.hash == h && entry.key == key) {
                // Check TTL
                LARGE_INTEGER now;
                QueryPerformanceCounter(&now);
                double elapsedMs = static_cast<double>(now.QuadPart - entry.insertTimeTick)
                    * 1000.0 / static_cast<double>(m_freq.QuadPart);
                if (entry.ttlMs > 0 && elapsedMs > static_cast<double>(entry.ttlMs)) {
                    // Expired — treat as miss (will be cleaned up by EvictExpired)
                    break;
                }
                outData = entry.data;
                // const_cast to update access time under shared lock is safe
                // because lastAccessTick is only used for LRU heuristics
                const_cast<SubMsCacheEntry&>(entry).lastAccessTick = now.QuadPart;
                found = true;
                break;
            }
            if (entry.probeDistance < probe) break; // robin-hood invariant
        }

        ReleaseSRWLockShared(&m_lock);

        // Update stats (atomic-like under exclusive during actual stat bump)
        AcquireSRWLockExclusive(&m_lock);
        if (found) {
            m_stats.hitCount++;
        }
        else {
            m_stats.missCount++;
        }
        LARGE_INTEGER endTick;
        QueryPerformanceCounter(&endTick);
        double lookupUs = static_cast<double>(endTick.QuadPart - startTick.QuadPart)
            * 1000000.0 / static_cast<double>(m_freq.QuadPart);
        m_totalLookupUs += lookupUs;
        m_lookupCount++;
        ReleaseSRWLockExclusive(&m_lock);

        return found;
    }

    /// Store a thumbnail in the cache with a given TTL in milliseconds.
    /// TTL of 0 means the entry never expires (manual eviction only).
    inline void Put(const std::wstring& key, const uint8_t* data, size_t size, uint32_t ttlMs = 0) {
        if (!data || size == 0) return;

        AcquireSRWLockExclusive(&m_lock);

        // If at capacity, evict LRU entry
        if (m_entryCount >= m_capacity * 3 / 4) {
            EvictLRUInternal();
        }

        const uint64_t h = HashKey(key);
        uint32_t idx = static_cast<uint32_t>(h & m_mask);

        SubMsCacheEntry newEntry;
        newEntry.key = key;
        newEntry.data.assign(data, data + size);
        newEntry.hash = h;
        newEntry.ttlMs = ttlMs;
        newEntry.occupied = true;
        newEntry.probeDistance = 0;

        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        newEntry.insertTimeTick = now.QuadPart;
        newEntry.lastAccessTick = now.QuadPart;

        // Robin-hood insertion: swap with entries that have shorter probe distance
        for (uint8_t probe = 0; probe < MAX_PROBE; ++probe) {
            const uint32_t slot = (idx + probe) & m_mask;
            newEntry.probeDistance = probe;

            if (!m_table[slot].occupied) {
                m_table[slot] = std::move(newEntry);
                m_entryCount++;
                m_stats.totalBytes += size;
                if (m_stats.totalBytes > m_stats.peakBytes)
                    m_stats.peakBytes = m_stats.totalBytes;
                ReleaseSRWLockExclusive(&m_lock);
                return;
            }

            // If existing key matches, overwrite
            if (m_table[slot].hash == h && m_table[slot].key == key) {
                m_stats.totalBytes -= m_table[slot].data.size();
                m_stats.totalBytes += size;
                if (m_stats.totalBytes > m_stats.peakBytes)
                    m_stats.peakBytes = m_stats.totalBytes;
                m_table[slot] = std::move(newEntry);
                ReleaseSRWLockExclusive(&m_lock);
                return;
            }

            // Robin-hood swap: if the existing entry's probe distance is less,
            // it is "richer" — swap and continue inserting the displaced entry
            if (m_table[slot].probeDistance < probe) {
                std::swap(m_table[slot], newEntry);
                idx = slot + 1;
                probe = newEntry.probeDistance;
                // Continue loop with displaced entry
            }
        }

        // Max probe reached — force evict oldest in the probe chain and retry
        EvictLRUInternal();
        // Simple fallback: place at first available slot in next MAX_PROBE range
        for (uint8_t probe = 0; probe < MAX_PROBE; ++probe) {
            const uint32_t slot = (idx + probe) & m_mask;
            if (!m_table[slot].occupied) {
                newEntry.probeDistance = probe;
                m_table[slot] = std::move(newEntry);
                m_entryCount++;
                m_stats.totalBytes += size;
                if (m_stats.totalBytes > m_stats.peakBytes)
                    m_stats.peakBytes = m_stats.totalBytes;
                break;
            }
        }

        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Manually evict a specific key from the cache.
    inline void Evict(const std::wstring& key) {
        AcquireSRWLockExclusive(&m_lock);
        const uint64_t h = HashKey(key);
        uint32_t idx = static_cast<uint32_t>(h & m_mask);

        for (uint8_t probe = 0; probe < MAX_PROBE; ++probe) {
            const uint32_t slot = (idx + probe) & m_mask;
            if (!m_table[slot].occupied) break;
            if (m_table[slot].hash == h && m_table[slot].key == key) {
                RemoveAtInternal(slot);
                break;
            }
            if (m_table[slot].probeDistance < probe) break;
        }
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Sweep through the table and remove all expired entries.
    inline void EvictExpired() {
        AcquireSRWLockExclusive(&m_lock);
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);

        for (uint32_t i = 0; i < m_capacity; ++i) {
            if (!m_table[i].occupied) continue;
            if (m_table[i].ttlMs == 0) continue; // no expiry
            double elapsedMs = static_cast<double>(now.QuadPart - m_table[i].insertTimeTick)
                * 1000.0 / static_cast<double>(m_freq.QuadPart);
            if (elapsedMs > static_cast<double>(m_table[i].ttlMs)) {
                RemoveAtInternal(i);
                // Re-check this slot since backward shift may have moved an entry here
                --i;
            }
        }
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Return cache statistics. Hit rate is computed from hit/miss counters.
    inline SubMsCacheStats GetStats() {
        AcquireSRWLockShared(&m_lock);
        SubMsCacheStats s = m_stats;
        s.entryCount = m_entryCount;
        uint64_t total = s.hitCount + s.missCount;
        s.hitRatePercent = (total > 0) ? (100.0 * static_cast<double>(s.hitCount) / static_cast<double>(total)) : 0.0;
        s.avgLookupUs = (m_lookupCount > 0) ? (m_totalLookupUs / static_cast<double>(m_lookupCount)) : 0.0;
        ReleaseSRWLockShared(&m_lock);
        return s;
    }

    /// Return the current hash algorithm in use.
    inline CacheHashAlgo GetHashAlgorithm() const noexcept { return m_algo; }

    /// Return the table capacity (always a power of two).
    inline uint32_t GetCapacity() const noexcept { return m_capacity; }

private:
    /// FNV-1a 64-bit hash on the raw wchar_t data of the key.
    inline uint64_t FNV1a64(const std::wstring& key) const noexcept {
        constexpr uint64_t FNV_OFFSET = 14695981039346656037ULL;
        constexpr uint64_t FNV_PRIME = 1099511628211ULL;
        uint64_t hash = FNV_OFFSET;
        const auto* bytes = reinterpret_cast<const uint8_t*>(key.data());
        const size_t len = key.size() * sizeof(wchar_t);
        for (size_t i = 0; i < len; ++i) {
            hash ^= static_cast<uint64_t>(bytes[i]);
            hash *= FNV_PRIME;
        }
        return hash;
    }

    /// CityHash-style 64-bit finalizer mix.
    inline uint64_t CityMix(uint64_t h) const noexcept {
        h ^= h >> 33;
        h *= 0xFF51AFD7ED558CCDULL;
        h ^= h >> 33;
        h *= 0xC4CEB9FE1A85EC53ULL;
        h ^= h >> 33;
        return h;
    }

    /// Hash a key using the selected algorithm.
    inline uint64_t HashKey(const std::wstring& key) const noexcept {
        uint64_t h = FNV1a64(key);
        switch (m_algo) {
        case CacheHashAlgo::XXH3:
            // XXH3-style avalanche on top of FNV-1a
            h ^= h >> 29;
            h *= 0x1B03738712FAD5C9ULL;
            h ^= h >> 32;
            return h;
        case CacheHashAlgo::CityHash:
            return CityMix(h);
        case CacheHashAlgo::FNV1a:
        default:
            return h;
        }
    }

    /// Remove entry at slot and perform backward-shift deletion to maintain
    /// robin-hood invariants (no tombstones needed).
    inline void RemoveAtInternal(uint32_t slot) {
        m_stats.totalBytes -= m_table[slot].data.size();
        m_stats.evictionCount++;
        m_table[slot].occupied = false;
        m_table[slot].data.clear();
        m_table[slot].data.shrink_to_fit();
        m_table[slot].key.clear();
        m_entryCount--;

        // Backward-shift: move subsequent entries with probe > 0 backward
        uint32_t current = slot;
        for (uint8_t i = 1; i < MAX_PROBE; ++i) {
            uint32_t next = (current + 1) & m_mask;
            if (!m_table[next].occupied || m_table[next].probeDistance == 0) break;
            m_table[current] = std::move(m_table[next]);
            m_table[current].probeDistance--;
            m_table[next].occupied = false;
            current = next;
        }
    }

    /// Evict the least-recently-used entry from the table.
    /// Called internally under exclusive lock.
    inline void EvictLRUInternal() {
        uint32_t lruSlot = UINT32_MAX;
        uint64_t oldestTick = UINT64_MAX;

        for (uint32_t i = 0; i < m_capacity; ++i) {
            if (m_table[i].occupied && m_table[i].lastAccessTick < oldestTick) {
                oldestTick = m_table[i].lastAccessTick;
                lruSlot = i;
            }
        }

        if (lruSlot != UINT32_MAX) {
            RemoveAtInternal(lruSlot);
        }
    }

    SRWLOCK                  m_lock{};
    LARGE_INTEGER            m_freq{};
    std::vector<SubMsCacheEntry>  m_table;
    uint32_t                 m_capacity = DEFAULT_CAPACITY;
    uint32_t                 m_mask = 0;
    uint32_t                 m_entryCount = 0;
    CacheHashAlgo            m_algo = CacheHashAlgo::FNV1a;

    SubMsCacheStats               m_stats{};
    double                   m_totalLookupUs = 0.0;
    uint64_t                 m_lookupCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
