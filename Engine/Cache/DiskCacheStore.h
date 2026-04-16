// DiskCacheStore.h — SHA256-keyed persistent disk cache for thumbnail blobs
// Copyright (c) 2026 ExplorerLens Project
//
// Stores thumbnail pixel blobs on disk in a flat-file structure:
//   <cache_root>/index.bin   — fixed-size slot index (key hash → slot offset)
//   <cache_root>/blobs/      — per-entry binary files named by key hash
//
// Thread-safe for concurrent readers and one writer (shared_mutex per slot).
//
#pragma once

#include "TwoTierCacheManager.h"   // CacheEntry

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace ExplorerLens {
namespace Engine {

// ---------------------------------------------------------------------------
// DiskCacheStore
// ---------------------------------------------------------------------------

class DiskCacheStore {
public:
    // cachePath: directory where index.bin + blobs/ live
    // maxBytes:  evict LRU entries when total disk usage exceeds this
    explicit DiskCacheStore(std::string cachePath, size_t maxBytes = 512ULL * 1024 * 1024);
    ~DiskCacheStore();

    DiskCacheStore(const DiskCacheStore&) = delete;
    DiskCacheStore& operator=(const DiskCacheStore&) = delete;
    DiskCacheStore(DiskCacheStore&&) noexcept;
    DiskCacheStore& operator=(DiskCacheStore&&) noexcept;

    // Synchronously load an entry. Returns std::nullopt on miss or I/O error.
    std::optional<CacheEntry> Load(std::string_view key) const;

    // Synchronously persist an entry, updating the index.
    bool Store(std::string_view key, const CacheEntry& entry);

    // Queue a write for background I/O (worker thread drains the queue).
    void StoreAsync(std::string_view key, const CacheEntry& entry);

    // Remove a single key.
    void Invalidate(std::string_view key);

    // Remove all keys whose textual key starts with pathPrefix.
    void InvalidatePrefix(std::string_view pathPrefix);

    // Evict oldest entries until disk usage <= maxBytes.
    void TrimTobudget();

    // Flush all pending async writes. Blocks until queue is drained.
    void Flush();

    // Current total disk usage in bytes (blobs only, not index).
    size_t CurrentBytes() const noexcept;

    // True if the cache directory is writable and the index is coherent.
    bool IsHealthy() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace Engine
} // namespace ExplorerLens
