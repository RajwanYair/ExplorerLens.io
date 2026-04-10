// CacheWarmupPreloader.h — Structured Cache Warm-Up Preloader
// Copyright (c) 2026 ExplorerLens Project
//
// Pre-populates the thumbnail sub-millisecond cache at startup by replaying
// the top-N most-recently-accessed paths from an on-disk MRU log file.
// Runs asynchronously; caller polls IsComplete() or waits on stats.
//
#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct WarmupStats
{
    uint32_t attempted;    // paths submitted for warm-up
    uint32_t fulfilled;    // paths whose thumbnail was already cached (hits)
    uint32_t generated;    // thumbnails generated during warm-up
    uint32_t failed;       // paths that failed to decode
    double   elapsedMs;    // total wall-clock time

    double HitRatePct() const noexcept
    {
        if (attempted == 0)
            return 0.0;
        return (fulfilled * 100.0) / attempted;
    }
};

class CacheWarmupPreloader
{
public:
    // Maximum paths to preload from the MRU log.
    static constexpr uint32_t DEFAULT_MAX_PATHS = 128;

    // Load the MRU log from disk and begin warm-up.
    // Returns false if the MRU log file could not be opened.
    bool Start(const std::wstring& mruLogPath,
               uint32_t maxPaths = DEFAULT_MAX_PATHS);

    // Abort any in-progress warm-up.
    void Stop() noexcept;

    // Returns true when warm-up has finished (or was never started).
    bool IsComplete() const noexcept { return m_complete; }

    // Returns stats accumulated so far (or final stats if complete).
    WarmupStats GetStats() const noexcept { return m_stats; }

    // Inject a test decoder function (path → success bool).
    // When set, the preloader calls this instead of the real cache.
    void SetDecoder(std::function<bool(const std::wstring&)> fn)
    {
        m_decoder = std::move(fn);
    }

private:
    bool        m_complete = true;
    WarmupStats m_stats    = {};
    std::function<bool(const std::wstring&)> m_decoder;
};

} // namespace Engine
} // namespace ExplorerLens
