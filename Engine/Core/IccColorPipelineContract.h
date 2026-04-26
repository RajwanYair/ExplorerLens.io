// ============================================================================
// IccColorPipelineContract.h -- S272 / ROADMAP v6.0 H20 color management
//
// Phase 3 ICC pipeline contract.  Header-only.  Declares how embedded ICC
// profiles are extracted, cached, and applied between decode and display.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class IccColorSourceProfile : uint8_t
{
    SRGB              = 0,   // implicit if no ICC tag
    ADOBE_RGB         = 1,
    DISPLAY_P3        = 2,
    PROPHOTO_RGB      = 3,
    REC2020           = 4,
    REC709            = 5,
    EMBEDDED          = 6,   // read from file tag
    UNTAGGED_ASSUMED  = 7,
};

enum class IccColorIntent : uint8_t
{
    PERCEPTUAL              = 0,
    RELATIVE_COLORIMETRIC   = 1,
    SATURATION              = 2,
    ABSOLUTE_COLORIMETRIC   = 3,
};

enum class IccColorPipelineStatus : uint8_t
{
    OK                   = 0,
    NO_PROFILE           = 1,
    PROFILE_CORRUPT      = 2,
    DISPLAY_UNKNOWN      = 3,
    UNSUPPORTED_TRC      = 4,   // non-parametric tone curve
    BUDGET_EXCEEDED      = 5,
};

struct IccColorPipelinePolicy
{
    IccColorIntent         intent            = IccColorIntent::PERCEPTUAL;
    bool                   applyBpc          = true;   // black-point compensation
    bool                   useDisplayProfile = true;   // honour monitor EDID
    bool                   fastSrgbShortcut  = true;   // sRGB-in / sRGB-out skip
    uint32_t               lutSize           = 33;     // 3D LUT edge
    uint32_t               budgetMs          = 8;
};

struct IccColorPipelineProbe
{
    IccColorPipelineStatus status         = IccColorPipelineStatus::OK;
    IccColorSourceProfile  detectedSource = IccColorSourceProfile::SRGB;
    uint32_t               profileBytes   = 0;
    bool                   profileIsV4    = false;
    bool                   hasParametricTrc = true;
    bool                   isWideGamut    = false;
};

inline constexpr uint32_t kIccColorPipelineMaxProfileBytes = 2u * 1024u * 1024u; // 2 MiB
inline constexpr uint32_t kIccColorPipelineDefaultBudgetMs = 8;
inline constexpr uint32_t kIccColorPipelineHardBudgetMs    = 25;
inline constexpr uint32_t kIccColorPipelineLutCacheEntries = 32;

static_assert(std::is_trivially_copyable_v<IccColorPipelinePolicy>,
              "IccColorPipelinePolicy must be trivially copyable");
static_assert(std::is_trivially_copyable_v<IccColorPipelineProbe>,
              "IccColorPipelineProbe must be trivially copyable");

} // namespace ExplorerLens::Engine
