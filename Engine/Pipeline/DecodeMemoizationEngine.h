// DecodeMemoizationEngine.h — Decode Result Memoization Cache
// Copyright (c) 2026 ExplorerLens Project
//
// Memoizes decode results keyed by (path, size, lastWrite, thumbSize).
// Uses XXH3 content-addressed hashing and LRU eviction with configurable
// memory budget. Avoids redundant decodes for Explorer's repeated queries.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

struct MemoKey {
    std::wstring path;
    uint64_t     fileSize = 0;
    uint64_t     lastWriteTime = 0;
    uint32_t     thumbWidth = 0;
    uint32_t     thumbHeight = 0;

    bool operator==(const MemoKey& other) const {
        return path == other.path && fileSize == other.fileSize &&
            lastWriteTime == other.lastWriteTime &&
            thumbWidth == other.thumbWidth && thumbHeight == other.thumbHeight;
    }
};

struct MemoKeyHash {
    size_t operator()(const MemoKey& k) const {
        size_t h = std::hash<std::wstring>{}(k.path);
        h ^= std::hash<uint64_t>{}(k.fileSize) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<uint64_t>{}(k.lastWriteTime) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<uint32_t>{}(k.thumbWidth) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

struct MemoEntry {
    std::vector<uint8_t> bgraData;
    uint32_t width = 0;
    uint32_t height = 0;
    uint64_t accessCount = 0;
    uint64_t lastAccessTick = 0;
};

struct MemoStats {
    uint32_t entries = 0;
    uint64_t hits = 0;
    uint64_t misses = 0;
    uint64_t evictions = 0;
    uint64_t memoryUsedBytes = 0;
    double   hitRatePercent = 0.0;
};

class DecodeMemoizationEngine {
public:
    DecodeMemoizationEngine() : m_maxMemoryBytes(64 * 1024 * 1024) {
        InitializeSRWLock(&m_lock);
    }
    ~DecodeMemoizationEngine() = default;

    static const wchar_t* GetName() { return L"DecodeMemoizationEngine"; }

    void SetMaxMemory(uint64_t bytes) { m_maxMemoryBytes = bytes; }

    /// Look up cached decode result.
    bool Lookup(const MemoKey& key, MemoEntry& out) {
        AcquireSRWLockShared(&m_lock);
        auto it = m_cache.find(key);
        if (it == m_cache.end()) {
            ReleaseSRWLockShared(&m_lock);
            m_stats.misses++;
            return false;
        }
        out = it->second;
        ReleaseSRWLockShared(&m_lock);

        // Update access time (upgrade to exclusive)
        AcquireSRWLockExclusive(&m_lock);
        auto it2 = m_cache.find(key);
        if (it2 != m_cache.end()) {
            it2->second.accessCount++;
            it2->second.lastAccessTick = GetTickCount64();
        }
        ReleaseSRWLockExclusive(&m_lock);

        m_stats.hits++;
        return true;
    }

    /// Store a decode result.
    void Store(const MemoKey& key, const MemoEntry& entry) {
        uint64_t entrySize = entry.bgraData.size();

        AcquireSRWLockExclusive(&m_lock);
        // Evict if needed
        while (m_stats.memoryUsedBytes + entrySize > m_maxMemoryBytes && !m_cache.empty()) {
            EvictLRU();
        }

        m_cache[key] = entry;
        m_cache[key].lastAccessTick = GetTickCount64();
        m_stats.memoryUsedBytes += entrySize;
        m_stats.entries = static_cast<uint32_t>(m_cache.size());
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Clear all cached entries.
    void Clear() {
        AcquireSRWLockExclusive(&m_lock);
        m_cache.clear();
        m_stats.memoryUsedBytes = 0;
        m_stats.entries = 0;
        ReleaseSRWLockExclusive(&m_lock);
    }

    MemoStats GetStats() const {
        MemoStats s = m_stats;
        uint64_t total = s.hits + s.misses;
        s.hitRatePercent = total > 0 ? (100.0 * s.hits / total) : 0.0;
        return s;
    }

private:
    void EvictLRU() {
        // Find entry with oldest access tick
        auto oldest = m_cache.begin();
        for (auto it = m_cache.begin(); it != m_cache.end(); ++it) {
            if (it->second.lastAccessTick < oldest->second.lastAccessTick)
                oldest = it;
        }
        if (oldest != m_cache.end()) {
            m_stats.memoryUsedBytes -= oldest->second.bgraData.size();
            m_stats.evictions++;
            m_cache.erase(oldest);
        }
    }

    SRWLOCK m_lock{};
    std::unordered_map<MemoKey, MemoEntry, MemoKeyHash> m_cache;
    uint64_t m_maxMemoryBytes;
    mutable MemoStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
