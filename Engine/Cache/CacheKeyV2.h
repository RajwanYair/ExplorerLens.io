//==============================================================================
// ExplorerLens Engine — CacheKey v2 (Sprint S235)
// Copyright (c) 2026 — ExplorerLens Project
// ROADMAP v6.0 ADR-013: decoder_version embedded in cache key
//==============================================================================
//
// v1 key = hash(path) + size + mtime + thumbSize
// v2 key = v1 + decoderVersion
//
// Why: a decoder upgrade (e.g. libjpeg-turbo 3.0.4 → 3.1.0) can subtly change
// pixel output for the same input. Stale cached thumbnails would be served
// after the upgrade. v2 forces a cache miss whenever the decoder bumps its
// reported version, automatically re-decoding stale entries.
//==============================================================================
#pragma once

#include <cstdint>
#include <cstring>

namespace ExplorerLens {
namespace Engine {
namespace Cache {

/// <summary>
/// POD cache key — safe across COM / DLL boundaries.
/// </summary>
struct CacheKeyV2
{
    std::uint64_t pathHash       = 0;  // Blake3/FNV of normalized path
    std::uint64_t fileSize       = 0;
    std::uint64_t mtime100ns     = 0;
    std::uint32_t decoderVersion = 0;  // NEW in v2 — forces miss on upgrade
    std::uint32_t thumbSize      = 0;  // requested thumbnail edge length

    friend bool operator==(const CacheKeyV2& a, const CacheKeyV2& b) noexcept
    {
        return std::memcmp(&a, &b, sizeof(CacheKeyV2)) == 0;
    }
    friend bool operator!=(const CacheKeyV2& a, const CacheKeyV2& b) noexcept
    {
        return !(a == b);
    }
};

static_assert(sizeof(CacheKeyV2) == 32,
    "CacheKeyV2 must be exactly 32 bytes for stable on-disk format.");
static_assert(std::is_trivially_copyable_v<CacheKeyV2>,
    "CacheKeyV2 crosses COM / DLL boundaries — must be trivially copyable.");

/// <summary>
/// Schema version byte written to the head of every on-disk key.
/// Bump this whenever CacheKeyV2 layout changes.
/// </summary>
inline constexpr std::uint8_t kCacheKeySchemaVersion = 2;

/// <summary>
/// Compute a stable 64-bit hash of a CacheKeyV2 for use in
/// std::unordered_map and similar containers.
/// </summary>
[[nodiscard]] inline std::uint64_t HashCacheKeyV2(const CacheKeyV2& k) noexcept
{
    // FNV-1a over the raw bytes
    std::uint64_t h = 1469598103934665603ULL;
    const auto* p = reinterpret_cast<const std::uint8_t*>(&k);
    for (std::size_t i = 0; i < sizeof(CacheKeyV2); ++i) {
        h ^= p[i];
        h *= 1099511628211ULL;
    }
    return h;
}

} // namespace Cache
} // namespace Engine
} // namespace ExplorerLens
