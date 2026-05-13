// Engine/Cache/ThumbnailEvictionPolicy.h
// ExplorerLens — SSIM-weighted thumbnail cache eviction policy (ROADMAP v8.0 Phase 3)
// Sprint S348.
//
// Purpose:
//   Phase 3 exit criterion: "SSIM gate raised to 0.97".
//   Traditional cache eviction uses LRU or LFU.  ExplorerLens Phase 3 introduces
//   quality-aware eviction: thumbnails with SSIM < kSsimEvictThreshold are evicted
//   first (even if recently accessed) so that high-quality entries survive longer.
//
//   Policy algorithm (two-tier):
//     Tier A  (SSIM ≥ 0.97) — standard LRU within tier
//     Tier B  (SSIM  < 0.97) — evicted before Tier A regardless of access time
//     Special: thumbnails that failed SSIM measurement (status UNKNOWN) sort to
//              the end of Tier B (evicted last among low-quality entries, to avoid
//              re-decoding unknown-quality content unnecessarily).
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_THUMBNAIL_EVICTION_POLICY_H
#define EXPLORERLENS_ENGINE_THUMBNAIL_EVICTION_POLICY_H

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <algorithm>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// EvictionTier — quality-tiered eviction bucket
// ---------------------------------------------------------------------------

enum class EvictionTier : std::uint8_t {
    TIER_A       = 0,   ///< SSIM ≥ kSsimHighQualityThreshold — survive LRU pressure
    TIER_B       = 1,   ///< SSIM  < kSsimHighQualityThreshold — evict first
    TIER_UNKNOWN = 2,   ///< SSIM not yet measured — end of Tier B
};

// ---------------------------------------------------------------------------
// EvictionRecord — per-entry cache metadata
// ---------------------------------------------------------------------------

struct EvictionRecord final {
    std::uint64_t  cacheKey       = 0u;     ///< Opaque 64-bit cache key (from L2SqliteCacheIndex)
    float          ssimScore      = 0.0f;   ///< Last measured SSIM (0 if unknown)
    bool           ssimMeasured   = false;  ///< True when ssimScore is valid
    std::uint64_t  lastAccessMs   = 0u;     ///< Epoch ms of last cache hit
    std::uint64_t  sizeBytes      = 0u;     ///< Uncompressed thumbnail size in bytes
    EvictionTier   tier           = EvictionTier::TIER_UNKNOWN;

    // Compute tier from ssimScore.
    void UpdateTier(float ssimHighThreshold) noexcept
    {
        if (!ssimMeasured) {
            tier = EvictionTier::TIER_UNKNOWN;
        } else if (ssimScore >= ssimHighThreshold) {
            tier = EvictionTier::TIER_A;
        } else {
            tier = EvictionTier::TIER_B;
        }
    }
};

// ---------------------------------------------------------------------------
// ThumbnailEvictionConfig
// ---------------------------------------------------------------------------

struct ThumbnailEvictionConfig final {
    /// SSIM threshold for Tier A ("high quality" — Phase 3 target = 0.97).
    float    ssimHighThreshold    = 0.97f;

    /// Maximum cache size in bytes before eviction is triggered.
    std::uint64_t maxCacheBytes   = 512u * 1'024u * 1'024u;  // 512 MiB

    /// Maximum number of entries in the cache index before eviction.
    std::uint32_t maxEntryCount   = 50'000u;

    /// Fraction of Tier B entries to evict per cycle (0.0–1.0).
    float    evictFraction        = 0.25f;

    /// Minimum entries to evict per cycle regardless of fraction.
    std::uint32_t minEvictCount   = 4u;
};

// ---------------------------------------------------------------------------
// EvictionCandidateSet — sorted list of entries to evict
// ---------------------------------------------------------------------------

struct EvictionCandidateSet final {
    std::vector<std::uint64_t> keys;      ///< Ordered: evict keys[0] first
    std::uint64_t              totalBytes = 0u;

    [[nodiscard]] bool Empty() const noexcept { return keys.empty(); }
    [[nodiscard]] std::size_t Count() const noexcept { return keys.size(); }
};

// ---------------------------------------------------------------------------
// ThumbnailEvictionPolicy — static policy engine
// ---------------------------------------------------------------------------

class ThumbnailEvictionPolicy final {
public:
    // -----------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------

    /// Phase 3 SSIM quality target (ROADMAP exit criterion).
    static constexpr float kSsimPhase3Gate     = 0.97f;

    /// Minimum SSIM considered acceptable for Phase 2 (existing threshold).
    static constexpr float kSsimPhase2Gate     = 0.95f;

    /// SSIM value that always triggers immediate re-decode on next access.
    static constexpr float kSsimTriggerEvict   = 0.80f;

    // -----------------------------------------------------------------
    // SelectEvictionCandidates
    // -----------------------------------------------------------------

    /// From a vector of EvictionRecord, select entries to evict so that
    /// the cache shrinks to fit within cfg.maxCacheBytes / cfg.maxEntryCount.
    /// Returns an EvictionCandidateSet ordered from lowest-priority to highest.
    [[nodiscard]] static EvictionCandidateSet SelectEvictionCandidates(
        std::vector<EvictionRecord> entries,
        const ThumbnailEvictionConfig& cfg = {}) noexcept
    {
        // Update tier for each entry
        for (auto& e : entries)
            e.UpdateTier(cfg.ssimHighThreshold);

        // Sort: Tier B (low quality) first, within tier sort oldest access first
        std::sort(entries.begin(), entries.end(),
            [](const EvictionRecord& a, const EvictionRecord& b) {
                if (a.tier != b.tier)
                    return static_cast<std::uint8_t>(a.tier) >
                           static_cast<std::uint8_t>(b.tier); // B=1 > A=0
                return a.lastAccessMs < b.lastAccessMs;
            });

        EvictionCandidateSet result;
        // Determine how many entries to evict
        std::uint64_t totalSz  = 0u;
        for (auto& e : entries) totalSz += e.sizeBytes;

        const std::uint64_t budgetBytes  = cfg.maxCacheBytes;
        const std::uint32_t budgetCount  = cfg.maxEntryCount;
        const auto          entryCount   = static_cast<std::uint32_t>(entries.size());

        // Always evict at minimum minEvictCount from Tier B / UNKNOWN
        for (std::uint32_t i = 0u; i < entryCount; ++i) {
            const auto& e = entries[i];
            const bool aboveBudget = totalSz > budgetBytes ||
                                     entryCount - i > budgetCount;
            const bool lowQuality  = e.tier != EvictionTier::TIER_A;

            if (!aboveBudget && !lowQuality &&
                result.Count() >= cfg.minEvictCount)
                break;

            result.keys.push_back(e.cacheKey);
            result.totalBytes += e.sizeBytes;
            totalSz            = (totalSz > e.sizeBytes) ? totalSz - e.sizeBytes : 0u;
        }

        return result;
    }

    // -----------------------------------------------------------------
    // ClassifyEntry
    // -----------------------------------------------------------------

    [[nodiscard]] static EvictionTier ClassifyEntry(
        float ssim,
        bool  measured,
        float threshold = kSsimPhase3Gate) noexcept
    {
        if (!measured)           return EvictionTier::TIER_UNKNOWN;
        if (ssim >= threshold)   return EvictionTier::TIER_A;
        return EvictionTier::TIER_B;
    }

    // -----------------------------------------------------------------
    // MeetsPhase3Gate
    // -----------------------------------------------------------------

    [[nodiscard]] static bool MeetsPhase3Gate(float ssim, bool measured) noexcept
    {
        return measured && ssim >= kSsimPhase3Gate;
    }

private:
    ThumbnailEvictionPolicy() = delete;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_THUMBNAIL_EVICTION_POLICY_H
