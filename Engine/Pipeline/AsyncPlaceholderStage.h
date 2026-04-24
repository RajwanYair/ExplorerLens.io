// ============================================================================
// AsyncPlaceholderStage.h -- S254 / ROADMAP v6.0 H7
//
// Phase 2 async placeholder-thumbnail contract (H7 from macOS Quick Look).
// Returns a low-resolution placeholder within a hard latency budget, then
// upgrades to full resolution asynchronously when the real decoder finishes.
//
// Header-only.  The actual two-phase callback plumbing lands in Phase 2 once
// the std::jthread worker pool (S242 DecodeCancelToken) is live.
// ============================================================================
#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class PlaceholderKind : uint8_t
{
    FORMAT_ICON        = 0,   // generic file-format tile (fastest)
    EMBEDDED_THUMB     = 1,   // JPEG/RAW/HEIC embedded EXIF thumbnail
    DOWNSAMPLED_PREVIEW= 2,   // small-tile decode (TIFF pyramid, PSD)
    CACHED_PRIOR       = 3,   // previous cached thumb for this file
    NONE               = 4,   // no placeholder available -- wait for full decode
};

enum class PlaceholderStatus : uint8_t
{
    OK                 = 0,
    BUDGET_EXCEEDED    = 1,
    UNAVAILABLE        = 2,
    CANCELLED          = 3,
    DOWNGRADED         = 4,   // returned FORMAT_ICON instead of requested kind
};

struct PlaceholderRequest
{
    uint32_t   targetWidth        = 0;
    uint32_t   targetHeight       = 0;
    uint32_t   latencyBudgetMs    = 0;        // 0 -> use default
    bool       allowDowngrade     = true;     // accept FORMAT_ICON if needed
    bool       cacheOnly          = false;    // if true, never decode; CACHED_PRIOR only
};

struct PlaceholderResult
{
    PlaceholderKind    kind            = PlaceholderKind::NONE;
    PlaceholderStatus  status          = PlaceholderStatus::UNAVAILABLE;
    uint32_t           emittedWidth    = 0;
    uint32_t           emittedHeight   = 0;
    uint32_t           latencyMs       = 0;   // wall-clock actual
    uint64_t           cacheKeyHash    = 0;   // 0 if not yet cached
};

// Budgets: the Shell thumbnail call must return something within 33 ms or
// Explorer shows the generic format icon.  Placeholder budget is half of that.
inline constexpr uint32_t kPlaceholderDefaultLatencyMs = 16;
inline constexpr uint32_t kPlaceholderMaxLatencyMs     = 33;
inline constexpr uint32_t kPlaceholderMinSideDefault   = 16;
inline constexpr uint32_t kPlaceholderMaxSideDefault   = 256;

static_assert(std::is_trivially_copyable_v<PlaceholderRequest>,
              "PlaceholderRequest must be trivially copyable");
static_assert(std::is_trivially_copyable_v<PlaceholderResult>,
              "PlaceholderResult must be trivially copyable");
static_assert(kPlaceholderDefaultLatencyMs < kPlaceholderMaxLatencyMs,
              "Placeholder default must be under the hard max");

} // namespace ExplorerLens::Engine
