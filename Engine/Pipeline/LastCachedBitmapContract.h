// Engine/Pipeline/LastCachedBitmapContract.h
// ExplorerLens — Async placeholder thumbnail contract (H7 / ROADMAP v7.0 Phase 2)
// Sprint S315.
//
// Purpose:
//   Windows Explorer's IThumbnailProvider contract is synchronous — it blocks
//   the UI thread until GetThumbnail() returns.  For large or slow-to-decode
//   files (RAW, HEIF, PDF) this causes visible stuttering.
//
//   The "last-cached bitmap" pattern (Harvested from Apple Photos — H7) solves
//   this by:
//     1. On first request: spawn async decode, immediately return the last
//        known cached bitmap for this file (even from a previous session).
//     2. On completion: notify the cache; next Explorer refresh shows the
//        fresh decode.
//
//   This contract header defines the stable API surface.
//   Phase 2 wiring integrates with CacheManagerV2 and IThumbnailProvider.
//
// Expected performance improvement (H7 harvest):
//   - Cold path (no cache): return 256×256 blurred placeholder in ~0.1 ms
//   - Warm path (cache hit): return exact bitmap in ~0.5 ms
//   vs. current synchronous decode: 4–400 ms depending on format
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_LAST_CACHED_BITMAP_CONTRACT_H
#define EXPLORERLENS_ENGINE_LAST_CACHED_BITMAP_CONTRACT_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <optional>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// PlaceholderBitmapKind
// ---------------------------------------------------------------------------
enum class PlaceholderBitmapKind : std::uint8_t {
    NONE            = 0,   ///< No cached bitmap; caller must show spinner
    EXACT_CACHED    = 1,   ///< Last exact decode (may be stale)
    STALE_SCALED    = 2,   ///< Scaled-down version of older cached decode
    BLURRED_PROXY   = 3,   ///< Blurred 32×32 colour-proxy (fastest fallback)
    FORMAT_ICON     = 4,   ///< Format-specific icon (e.g. RAW camera icon)
};

// ---------------------------------------------------------------------------
// LastCachedBitmapResult
// ---------------------------------------------------------------------------
struct LastCachedBitmapResult final {
    PlaceholderBitmapKind kind{ PlaceholderBitmapKind::NONE };
    std::uint32_t         widthPixels{};
    std::uint32_t         heightPixels{};
    std::uint32_t         strideBytes{};
    std::vector<std::byte> pixels;          ///< BGRA-8 pixel data
    std::uint64_t         cacheAgeMs{};     ///< Age of cached bitmap in ms
    bool                  isPlaceholder{};  ///< True: stale/proxy; false: exact

    [[nodiscard]] bool HasPixels() const noexcept
    {
        return kind != PlaceholderBitmapKind::NONE && !pixels.empty();
    }
};

// ---------------------------------------------------------------------------
// LastCachedBitmapConfig
// ---------------------------------------------------------------------------
struct LastCachedBitmapConfig final {
    // Maximum age of a cached bitmap before it is considered too stale to
    // return (0 = always return, even years-old cached data).
    std::uint64_t maxCacheAgeMs = 7u * 24u * 3600u * 1000u;  // 7 days

    // Minimum placeholder size to return (don't return sub-32px bitmaps)
    std::uint32_t minReturnWidthPixels  = 32u;
    std::uint32_t minReturnHeightPixels = 32u;

    // Allow blurred colour-proxy when no exact cache entry exists
    bool allowBlurredProxy = true;

    // Allow format icon fallback
    bool allowFormatIcon = true;
};

// ---------------------------------------------------------------------------
// LastCachedBitmapContract
// ---------------------------------------------------------------------------
// Stateless fetch interface.  Phase 2 stub — Fetch() returns NONE until
// the CacheManagerV2 integration is wired.
//
class LastCachedBitmapContract final {
public:
    LastCachedBitmapContract() noexcept  = default;
    ~LastCachedBitmapContract() noexcept = default;

    LastCachedBitmapContract(const LastCachedBitmapContract&)            = delete;
    LastCachedBitmapContract& operator=(const LastCachedBitmapContract&) = delete;

    // ── Primary API ──────────────────────────────────────────────────────────

    /// Fetch the best available cached bitmap for the given file path hash.
    /// Returns quickly (< 1 ms target); never blocks for a full decode.
    ///
    /// @param filePathHash   CacheKeyV2 hash of the file path
    /// @param requestedSide  Larger of width/height requested by shell
    /// @param cfg            Staleness and fallback policy
    ///
    /// Phase 2 stub: always returns NONE.
    [[nodiscard]] LastCachedBitmapResult Fetch(
        std::uint64_t               filePathHash,
        std::uint32_t               requestedSide,
        const LastCachedBitmapConfig& cfg = LastCachedBitmapConfig{}) const noexcept
    {
        (void)filePathHash; (void)requestedSide; (void)cfg;
        return {};   // Phase 2 stub — returns NONE
    }

    /// Invalidate a cache entry (called after a successful fresh decode).
    void Invalidate([[maybe_unused]] std::uint64_t filePathHash) const noexcept {}

    // ── Capability ────────────────────────────────────────────────────────────

    /// True when the CacheManagerV2 backend is wired (Phase 2+).
    [[nodiscard]] static constexpr bool IsBackendAvailable() noexcept
    {
        return false;   // Phase 2 stub
    }

    /// Maximum bitmap side length we will store/return as a cached placeholder.
    static constexpr std::uint32_t kMaxCachedSidePixels = 2048u;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_LAST_CACHED_BITMAP_CONTRACT_H
