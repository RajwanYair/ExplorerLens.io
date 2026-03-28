// HiDPIThumbnailCache.h — Per-DPI Thumbnail Cache with Bucket Eviction
// Copyright (c) 2026 ExplorerLens Project
//
// Extends the sub-ms cache with per-DPI bucket partitioning and automatic
// eviction of stale density variants when DPI changes are detected by
// MonitorConfigWatcher.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "Core/DPIScalingPolicy.h"

namespace ExplorerLens {
namespace Engine {

// ---- Cache Entry ------------------------------------------------------------

struct HiDPICacheEntry {
    std::string          key;           // "<hash>@<scaleSuffix>_<sizeN>"
    DPIBucket            bucket      = DPIBucket::DPI_96;
    uint32_t             physicalPx  = 0;
    std::vector<uint8_t> pixels;        // BGRA
    uint32_t             width       = 0;
    uint32_t             height      = 0;
    uint64_t             insertedAt  = 0;  // epoch ms
    uint64_t             lastUsed    = 0;  // epoch ms
};

// ---- Stats ------------------------------------------------------------------

struct HiDPICacheStats {
    uint64_t hits           = 0;
    uint64_t misses         = 0;
    uint64_t evictions      = 0;
    uint64_t dpiInvalidations = 0;  // Entries removed due to DPI change
    size_t   entriesCount   = 0;
    size_t   totalBytes     = 0;
};

// ---- HiDPIThumbnailCache ----------------------------------------------------

class HiDPIThumbnailCache {
public:
    explicit HiDPIThumbnailCache(
        size_t maxBytes  = 256 * 1024 * 1024,  // 256 MB default
        size_t maxEntries = 4096);
    ~HiDPIThumbnailCache();

    // Lookup entry by DPI-aware cache key. Returns null if not found.
    const HiDPICacheEntry* Get(const std::string& key) const;

    // Insert or update a cache entry.
    void Put(HiDPICacheEntry entry);

    // Remove all entries for a given file hash (all DPI variants).
    void InvalidateFile(const std::string& fileHash);

    // Remove all entries for a specific DPI bucket (e.g. on monitor disconnect).
    void InvalidateBucket(DPIBucket bucket);

    // Remove all entries with physical size outside [minPx, maxPx].
    void InvalidateOutOfRange(uint32_t minPx, uint32_t maxPx);

    // Evict entries down to (targetFraction * maxBytes) — default 0.75.
    void EvictLRU(float targetFraction = 0.75f);

    // Clear entire cache.
    void Clear();

    HiDPICacheStats Stats() const;

    // Singleton accessor.
    static HiDPIThumbnailCache& Instance();

    // Wire up to MonitorConfigWatcher to auto-invalidate on DPI changes.
    void HookMonitorWatcher();
    void UnhookMonitorWatcher();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    HiDPIThumbnailCache(const HiDPIThumbnailCache&) = delete;
    HiDPIThumbnailCache& operator=(const HiDPIThumbnailCache&) = delete;
};

} // namespace Engine
} // namespace ExplorerLens
