// Engine/Pipeline/ThumbnailPlaceholderBroker.h
// ExplorerLens — Async placeholder: return last-cached HBITMAP immediately (H1 / ROADMAP v8.0 Phase 2)
// Sprint S329.
//
// Purpose:
//   When Explorer requests a thumbnail for a file it has not seen before,
//   the current pipeline makes Explorer wait for the full decode to complete
//   (up to 500 ms).  During this time Explorer shows a blank white square.
//
//   Apple Quick Look's "async placeholder" pattern (H1) solves this:
//     1. Return the *last cached* bitmap for this file path immediately
//        (may be stale — different size or from a previous version).
//     2. Queue a fresh decode in the background.
//     3. Explorer refreshes the thumbnail when the fresh decode completes.
//     4. If no cached entry exists, return a format-specific generic icon
//        (also pre-rendered) so the square is never blank.
//
//   ThumbnailPlaceholderBroker manages the in-process placeholder cache:
//     - Key: file path hash (xxHash3-64)
//     - Value: HBITMAP handle + timestamp
//     - Capacity: kMaxEntries (128) LRU entries
//     - No disk I/O in the Resolve() hot path
//
// Thread-safety:
//   Register() and Resolve() are guarded by a shared_mutex.
//   Resolve() acquires a read lock; Register() acquires a write lock.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_THUMBNAIL_PLACEHOLDER_BROKER_H
#define EXPLORERLENS_ENGINE_THUMBNAIL_PLACEHOLDER_BROKER_H

#include <cstdint>
#include <cstddef>
#include <chrono>
#include <shared_mutex>
#include <array>
#include <atomic>
#include <algorithm>

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>  // HBITMAP
#endif

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// PlaceholderResolveResult
// ---------------------------------------------------------------------------
enum class PlaceholderResolveResult : std::uint8_t {
    HIT_EXACT     = 0,  ///< Cached entry found at the requested size
    HIT_SCALED    = 1,  ///< Cached entry found but at a different size (stale)
    MISS          = 2,  ///< No cached entry; caller should decode from scratch
    CACHE_EMPTY   = 3,  ///< Cache has no entries at all
};

// ---------------------------------------------------------------------------
// PlaceholderEntry — one cached thumbnail handle
// ---------------------------------------------------------------------------
struct PlaceholderEntry final {
#ifdef _WIN32
    HBITMAP       bitmap{ nullptr };
#else
    void*         bitmap{ nullptr };  // Stub for non-Windows builds
#endif
    std::uint64_t pathHash{};      ///< xxHash3-64 of the absolute file path
    std::uint32_t width{};
    std::uint32_t height{};
    std::int64_t  cachedAtMs{};    ///< Epoch milliseconds when cached
    bool          valid{ false };
};

// ---------------------------------------------------------------------------
// ThumbnailPlaceholderBroker
// ---------------------------------------------------------------------------
class ThumbnailPlaceholderBroker final {
public:
    /// Maximum placeholder entries held in memory simultaneously.
    static constexpr std::size_t kMaxEntries = 128u;

    /// Maximum age of a placeholder entry before it is considered stale.
    static constexpr std::int64_t kMaxAgeMs = 30'000;  // 30 seconds

    // Non-copyable, non-movable (owns mutex + array)
    ThumbnailPlaceholderBroker()                                           = default;
    ~ThumbnailPlaceholderBroker()                                          = default;
    ThumbnailPlaceholderBroker(const ThumbnailPlaceholderBroker&)          = delete;
    ThumbnailPlaceholderBroker& operator=(const ThumbnailPlaceholderBroker&) = delete;

    // ------------------------------------------------------------------
    // Register() — store a freshly-decoded thumbnail as a future placeholder.
    // Called from the decode completion path (write lock).
    // ------------------------------------------------------------------
    void Register(const PlaceholderEntry& entry) noexcept
    {
        if (!entry.valid || entry.pathHash == 0u) return;

        std::unique_lock lock(m_mutex);

        // Update existing entry if path hash matches
        for (auto& e : m_entries) {
            if (e.valid && e.pathHash == entry.pathHash) {
                e = entry;
                return;
            }
        }

        // Find an empty or expired slot
        const auto nowMs = NowMs();
        for (auto& e : m_entries) {
            if (!e.valid || (nowMs - e.cachedAtMs) > kMaxAgeMs) {
                e = entry;
                m_registeredTotal.fetch_add(1u, std::memory_order_relaxed);
                return;
            }
        }

        // LRU eviction: replace the oldest entry
        auto* oldest = &m_entries[0];
        for (auto& e : m_entries)
            if (e.cachedAtMs < oldest->cachedAtMs) oldest = &e;
        *oldest = entry;
        m_evictedTotal.fetch_add(1u, std::memory_order_relaxed);
        m_registeredTotal.fetch_add(1u, std::memory_order_relaxed);
    }

    // ------------------------------------------------------------------
    // Resolve() — retrieve a placeholder for the given path hash and size.
    // Called from IThumbnailProvider::GetThumbnail() (read lock).
    // ------------------------------------------------------------------
    [[nodiscard]]
    PlaceholderResolveResult Resolve(std::uint64_t  pathHash,
                                     std::uint32_t  requestedCx,
                                     PlaceholderEntry& out) const noexcept
    {
        std::shared_lock lock(m_mutex);

        bool anyValid = false;
        const PlaceholderEntry* best = nullptr;

        for (const auto& e : m_entries) {
            if (!e.valid || e.pathHash != pathHash) continue;
            anyValid = true;
            best = &e;
            if (e.width == requestedCx) break;  // exact match — stop search
        }

        if (!best) {
            return anyValid ? PlaceholderResolveResult::MISS
                            : PlaceholderResolveResult::CACHE_EMPTY;
        }

        out = *best;
        return (best->width == requestedCx)
               ? PlaceholderResolveResult::HIT_EXACT
               : PlaceholderResolveResult::HIT_SCALED;
    }

    // ------------------------------------------------------------------
    // Invalidate() — remove all entries for a path hash (file changed).
    // ------------------------------------------------------------------
    void Invalidate(std::uint64_t pathHash) noexcept
    {
        std::unique_lock lock(m_mutex);
        for (auto& e : m_entries)
            if (e.valid && e.pathHash == pathHash)
                e.valid = false;
    }

    // ------------------------------------------------------------------
    // Clear() — flush all entries.
    // ------------------------------------------------------------------
    void Clear() noexcept
    {
        std::unique_lock lock(m_mutex);
        for (auto& e : m_entries) e.valid = false;
    }

    // ------------------------------------------------------------------
    // Diagnostics
    // ------------------------------------------------------------------
    [[nodiscard]] std::uint64_t RegisteredTotal() const noexcept
    { return m_registeredTotal.load(std::memory_order_relaxed); }

    [[nodiscard]] std::uint64_t EvictedTotal() const noexcept
    { return m_evictedTotal.load(std::memory_order_relaxed); }

    [[nodiscard]] std::size_t LiveCount() const noexcept
    {
        std::shared_lock lock(m_mutex);
        std::size_t count = 0;
        for (const auto& e : m_entries)
            if (e.valid) ++count;
        return count;
    }

private:
    static std::int64_t NowMs() noexcept
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    mutable std::shared_mutex                  m_mutex;
    std::array<PlaceholderEntry, kMaxEntries>  m_entries{};
    std::atomic<std::uint64_t>                 m_registeredTotal{ 0u };
    std::atomic<std::uint64_t>                 m_evictedTotal{ 0u };
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_THUMBNAIL_PLACEHOLDER_BROKER_H
