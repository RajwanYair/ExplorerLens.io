// ScrubberCacheEngine.h — LRU frame cache for video scrubber seek acceleration
// Copyright (c) 2026 ExplorerLens Project
//
// Caches decoded video frames keyed by (filePath, ptsSec) to avoid re-decoding
// when the user seeks back to a recently visited timestamp. Maintains per-session
// hit/miss counters for performance observability.
//
#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

namespace ExplorerLens { namespace Engine {

struct ScrubberCacheKey
{
    std::wstring filePath;
    double       ptsSec = 0.0;

    bool operator==(const ScrubberCacheKey& o) const noexcept
    {
        return filePath == o.filePath && ptsSec == o.ptsSec;
    }
};

struct ScrubberCacheEntry
{
    bool     valid     = false;
    uint32_t width     = 0;
    uint32_t height    = 0;
    uint32_t sizeBytes = 0;
};

struct ScrubberCacheStats
{
    uint32_t entries = 0;
    uint32_t hits    = 0;
    uint32_t misses  = 0;
    float    hitRate = 0.0f;
};

class ScrubberCacheEngine
{
public:
    static ScrubberCacheEngine& Instance() noexcept;

    bool Put(const ScrubberCacheKey& key, const ScrubberCacheEntry& entry) noexcept;
    bool Get(const ScrubberCacheKey& key, ScrubberCacheEntry& out)         noexcept;
    void Evict(const ScrubberCacheKey& key)                                noexcept;
    void Clear()                                                            noexcept;

    uint32_t           Size()     const noexcept { return static_cast<uint32_t>(m_map.size()); }
    ScrubberCacheStats GetStats() const noexcept;

private:
    struct KeyHash
    {
        std::size_t operator()(const ScrubberCacheKey& k) const noexcept
        {
            const std::hash<std::wstring> wh;
            const std::hash<double>       dh;
            return wh(k.filePath) ^ (dh(k.ptsSec) << 1U);
        }
    };

    std::unordered_map<ScrubberCacheKey, ScrubberCacheEntry, KeyHash> m_map;
    uint32_t m_hits   = 0;
    uint32_t m_misses = 0;
};

}} // namespace ExplorerLens::Engine
