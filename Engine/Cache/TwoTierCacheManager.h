// TwoTierCacheManager.h — L1 (in-memory) + L2 (disk-mapped) cache with transparent promotion
// Copyright (c) 2026 ExplorerLens Project
//
// Coordinates access across two cache tiers:
//   L1 — in-memory LRU (SubMillisecondCacheEngine, 32–128 MB, < 1 ms access)
//   L2 — disk-persistent store (DiskCacheStore, 256 MB–2 GB, < 5 ms access)
//
// Hit path:  L1 hit → return immediately
//            L2 hit → promote to L1, return
//            Miss   → caller decodes; insert into both tiers
//
#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ---------------------------------------------------------------------------
// CacheEntry — a payload stored in either cache tier
// ---------------------------------------------------------------------------

struct CacheEntry {
    std::vector<uint8_t> pixels;   // BGRA32 or BGRA64 pixel data
    uint32_t             width  = 0;
    uint32_t             height = 0;
    uint32_t             stride = 0;
    uint32_t             bpp    = 32;  // bits per pixel
    bool                 valid  = false;

    size_t SizeBytes() const noexcept { return pixels.size(); }
};

// ---------------------------------------------------------------------------
// CacheTier — selects which tier was used for a lookup
// ---------------------------------------------------------------------------

enum class CacheTier : uint8_t { Miss = 0, L1 = 1, L2 = 2 };

// ---------------------------------------------------------------------------
// TwoTierCacheConfig — tunable parameters
// ---------------------------------------------------------------------------

struct TwoTierCacheConfig {
    size_t      l1MaxBytes    = 64 * 1024 * 1024;    // 64 MB default
    size_t      l2MaxBytes    = 512 * 1024 * 1024;   // 512 MB default
    std::string l2CachePath;                           // Empty = %LOCALAPPDATA%\ExplorerLens\Cache
    bool        enableL2      = true;
    bool        writeThrough  = false;  // Write L2 synchronously on insert (default: async)
};

// ---------------------------------------------------------------------------
// TwoTierCacheManager
// ---------------------------------------------------------------------------

class TwoTierCacheManager {
public:
    explicit TwoTierCacheManager(TwoTierCacheConfig config = {});
    ~TwoTierCacheManager();

    // Non-copyable, moveable
    TwoTierCacheManager(const TwoTierCacheManager&) = delete;
    TwoTierCacheManager& operator=(const TwoTierCacheManager&) = delete;
    TwoTierCacheManager(TwoTierCacheManager&&) noexcept;
    TwoTierCacheManager& operator=(TwoTierCacheManager&&) noexcept;

    // Lookup a thumbnail by key.
    // Returns the entry and the tier that served it (Miss / L1 / L2).
    std::pair<CacheEntry, CacheTier> Lookup(std::string_view key) const;

    // Insert a thumbnail into both tiers.
    void Insert(std::string_view key, CacheEntry entry);

    // Invalidate a key from all tiers.
    void Invalidate(std::string_view key);

    // Invalidate all entries associated with a file path prefix.
    void InvalidatePrefix(std::string_view pathPrefix);

    // Evict entries until both tiers are under budget.
    void TrimTobudget();

    // Flush pending async L2 writes. Call before process exit.
    void FlushL2();

    // Return current memory usage of L1 (bytes).
    size_t L1CurrentBytes() const noexcept;

    // Return current disk usage of L2 (bytes).
    size_t L2CurrentBytes() const noexcept;

    // Return combined cache statistics as a JSON string.
    std::string StatsJson() const;

    // True if L2 is initialized and functioning.
    bool IsL2Available() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace Engine
} // namespace ExplorerLens
