// Engine/Core/ThumbCacheWarmer.h
#pragma once

// ThumbCacheWarmer — thumbnail cache pre-warming on first install (S369)
//
// Implements H47: "On first install, pre-decode common file types in
// Desktop/Documents/Pictures." (ROADMAP v8.0 §5, Phase 5 preparation).
//
// The warmer runs as a low-priority background thread post-installation,
// discovers image files in well-known user folders, and pre-populates the
// L1/L2 thumbnail caches so first-open in Explorer is instant.
//
// Design constraints:
//   - Runs at THREAD_PRIORITY_IDLE to avoid competing with interactive use.
//   - Respects a byte/time budget; stops if system is under load.
//   - Skips files already present in L2 SQLite cache.
//   - Notifies caller via a completion callback.
//
// ROADMAP ref: H47 Phase 5 (Phase 3 header, implementation in Phase 5)

#ifndef EXPLORERLENS_ENGINE_THUMBCACHEWARMER_H
#define EXPLORERLENS_ENGINE_THUMBCACHEWARMER_H

#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace ExplorerLens::Engine {

/// Maximum files decoded in a single warm session.
inline constexpr std::uint32_t kCacheWarmMaxFiles      = 2048u;
/// Maximum wall-clock seconds allowed for a warm session.
inline constexpr std::uint32_t kCacheWarmMaxSeconds    = 300u;  // 5 minutes
/// Maximum MB of decoded bitmaps written to L2 cache in one session.
inline constexpr std::uint32_t kCacheWarmMaxCacheMb    = 128u;

// ---------------------------------------------------------------------------
// ThumbCacheWarmStatus
// ---------------------------------------------------------------------------
enum class ThumbCacheWarmStatus : std::uint8_t {
    OK              = 0,
    ALREADY_RUNNING = 1,  ///< A warm session is already in progress
    BUDGET_EXCEEDED = 2,  ///< Stopped because time/byte/file budget was exhausted
    CANCELLED       = 3,  ///< Caller called Cancel()
    IO_ERROR        = 4,  ///< Could not enumerate user folders
    CACHE_ERROR     = 5,  ///< L2 cache write failed
    NOT_WIN32       = 6,  ///< Non-Windows stub; not available
};

// ---------------------------------------------------------------------------
// CacheWarmConfig
// ---------------------------------------------------------------------------
struct CacheWarmConfig {
    std::uint32_t maxFiles       = kCacheWarmMaxFiles;
    std::uint32_t maxSeconds     = kCacheWarmMaxSeconds;
    std::uint32_t maxCacheMb     = kCacheWarmMaxCacheMb;
    std::uint32_t thumbnailSize  = 256u;   ///< Target thumbnail cx in pixels
    bool          warmDesktop    = true;
    bool          warmDocuments  = true;
    bool          warmPictures   = true;
    bool          warmDownloads  = false;  ///< Disabled by default — too large
    bool          runAtIdlePriority = true;

    [[nodiscard]] static CacheWarmConfig Default() noexcept {
        return CacheWarmConfig{};
    }

    [[nodiscard]] static CacheWarmConfig MinimalInstall() noexcept {
        CacheWarmConfig cfg{};
        cfg.maxFiles        = 256u;
        cfg.maxSeconds      = 60u;
        cfg.maxCacheMb      = 32u;
        cfg.warmDocuments   = false;
        cfg.warmDownloads   = false;
        return cfg;
    }
};

// ---------------------------------------------------------------------------
// CacheWarmProgress — snapshot of warm session progress
// ---------------------------------------------------------------------------
struct CacheWarmProgress {
    std::uint32_t filesDiscovered  = 0u;
    std::uint32_t filesDecoded     = 0u;
    std::uint32_t filesSkipped     = 0u;  ///< Already in cache
    std::uint32_t filesFailed      = 0u;
    std::uint32_t cacheKbWritten   = 0u;
    std::uint32_t elapsedSeconds   = 0u;
    bool          isRunning        = false;
};

/// Completion callback: called when the warm session finishes.
using CacheWarmCallback = std::function<void(ThumbCacheWarmStatus, const CacheWarmProgress&)>;

// ---------------------------------------------------------------------------
// ThumbCacheWarmer — singleton cache pre-warmer
// ---------------------------------------------------------------------------
class ThumbCacheWarmer final {
public:
    ThumbCacheWarmer(const ThumbCacheWarmer&)            = delete;
    ThumbCacheWarmer& operator=(const ThumbCacheWarmer&) = delete;

    /// Returns the process-wide singleton.
    [[nodiscard]] static ThumbCacheWarmer& Global() noexcept;

    /// Applies configuration before Start().
    void Configure(const CacheWarmConfig& cfg) noexcept;

    /// Starts the warm session in a background thread.
    /// callback is invoked on the warm thread when finished.
    [[nodiscard]] ThumbCacheWarmStatus Start(CacheWarmCallback callback = {}) noexcept;

    /// Requests cancellation of the running session.
    void Cancel() noexcept;

    /// Returns a snapshot of current warm progress.
    [[nodiscard]] CacheWarmProgress Progress() const noexcept;

    /// Returns true if a warm session is currently running.
    [[nodiscard]] bool IsRunning() const noexcept;

    /// Blocks until the current session finishes (or timeout_ms elapses).
    /// Returns OK if finished within the timeout.
    [[nodiscard]] ThumbCacheWarmStatus WaitForCompletion(
        std::uint32_t timeout_ms = 10000u) noexcept;

    /// Returns the number of complete warm sessions run since process start.
    [[nodiscard]] std::uint32_t SessionCount() const noexcept;

private:
    ThumbCacheWarmer() noexcept = default;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_THUMBCACHEWARMER_H
