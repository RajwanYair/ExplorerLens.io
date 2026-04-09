// AnimatedThumbnailCache.h — Per-Frame Thumbnail Cache for Animated Formats
// Copyright (c) 2026 ExplorerLens Project
//
// Caches decoded BGRA32 key frames keyed by (filePath, frameIndex) to avoid
// repeated decode on hover-scrub. Uses a bounded LRU store with a configurable
// per-entry byte limit. Thread-safe via shared mutex (SWMR) for multi-reader
// pipeline access.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <list>

namespace ExplorerLens { namespace Engine {

struct AnimatedCacheKey {
    std::wstring path;
    uint32_t     frameIndex = 0;
    bool operator==(const AnimatedCacheKey& o) const noexcept {
        return frameIndex == o.frameIndex && path == o.path;
    }
};

struct AnimatedCacheKeyHash {
    size_t operator()(const AnimatedCacheKey& k) const noexcept {
        size_t h = std::hash<std::wstring>{}(k.path);
        h ^= std::hash<uint32_t>{}(k.frameIndex) + 0x9e3779b9u + (h << 6) + (h >> 2);
        return h;
    }
};

struct AnimatedCacheEntry {
    std::vector<uint8_t> pixelsBGRA;
    uint32_t width   = 0;
    uint32_t height  = 0;
    uint64_t accessSeq = 0;  // Monotonic access counter for LRU ordering
};

struct AnimatedThumbnailCacheStats {
    uint64_t hits       = 0;
    uint64_t misses     = 0;
    uint64_t evictions  = 0;
    uint64_t bytesUsed  = 0;
    uint32_t entryCount = 0;
};

struct AnimatedCacheConfig {
    uint64_t maxBytes        = 64u * 1024u * 1024u;  // 64 MB default
    uint32_t maxEntries      = 512;
    uint32_t maxFramesPerFile = 5;
};

class AnimatedThumbnailCache {
public:
    explicit AnimatedThumbnailCache(const AnimatedCacheConfig& cfg = {}) noexcept;
    ~AnimatedThumbnailCache() = default;

    // Attempt to retrieve a cached frame. Returns nullptr if not cached.
    const AnimatedCacheEntry* Get(const AnimatedCacheKey& key) noexcept;

    // Insert or update a cached frame. Evicts LRU entries if budget exceeded.
    void Put(const AnimatedCacheKey& key, AnimatedCacheEntry entry) noexcept;

    // Evict all frames for a given file path (e.g. on file change notification).
    void Invalidate(const std::wstring& path) noexcept;

    // Flush entire cache.
    void Clear() noexcept;

    AnimatedThumbnailCacheStats GetStats() const noexcept;

private:
    void EvictLRU() noexcept;  // Must be called with m_mutex held exclusive

    AnimatedCacheConfig m_cfg;
    mutable std::shared_mutex m_mutex;
    uint64_t m_accessSeq = 0;
    uint64_t m_bytesUsed = 0;

    using Map = std::unordered_map<AnimatedCacheKey, AnimatedCacheEntry,
                                   AnimatedCacheKeyHash>;
    Map m_cache;
    AnimatedThumbnailCacheStats m_stats;
};

}} // namespace ExplorerLens::Engine
