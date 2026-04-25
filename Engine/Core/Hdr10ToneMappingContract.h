// ============================================================================
// Hdr10ToneMappingContract.h -- S277 / ROADMAP v6.0 H21 HDR10 thumbnails
//
// Phase 3 HDR10 / HLG tone-mapping contract.  Header-only.  Declares how a
// BT.2020 PQ or HLG source is mapped down to an SDR 8-bit sRGB thumbnail
// when the Explorer pane/icon is not HDR-capable.  Kept distinct from the
// pre-existing `HDRToneMappingOp` and `HDRToneMappingPipeline` which cover
// the runtime execution side.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class Hdr10SourceTransfer : uint8_t
{
    SDR_SRGB    = 0,   // no HDR handling needed
    PQ_BT2020   = 1,   // SMPTE ST 2084 - Perceptual Quantizer
    HLG_BT2020  = 2,   // ARIB STD-B67 - Hybrid Log-Gamma
    DOLBY_VISION = 3,  // profile 5/8 fallback only
    LINEAR_SCRGB = 4,
};

enum class Hdr10ToneCurve : uint8_t
{
    REINHARD    = 0,
    ACES_FILMIC = 1,   // ACES Narkowicz fit
    HABLE       = 2,   // Uncharted 2
    BT2390      = 3,   // ITU-R BT.2390-4
    PASS_THROUGH_HDR = 4,   // HDR-capable destination (renamed vs wingdi PASSTHROUGH macro)
};

enum class Hdr10ToneMappingStatus : uint8_t
{
    OK                       = 0,
    NOT_HDR_SOURCE           = 1,   // pipeline skipped
    UNSUPPORTED_DOVI_PROFILE = 2,
    METADATA_MISSING         = 3,
    BUDGET_EXCEEDED          = 4,
};

struct Hdr10ToneMappingPolicy
{
    Hdr10ToneCurve curve        = Hdr10ToneCurve::BT2390;
    float          targetNits   = 203.0f;   // SDR reference white
    float          maxContentLightNits = 1000.0f;   // from MaxCLL if present
    bool           useFrameMetadata = true;
    bool           preserveHueRotation = true;
    uint32_t       budgetMs     = 4;        // tone-map is tiny at thumb size
};

struct Hdr10ToneMappingProbe
{
    Hdr10ToneMappingStatus status         = Hdr10ToneMappingStatus::OK;
    Hdr10SourceTransfer    sourceTransfer = Hdr10SourceTransfer::SDR_SRGB;
    float                  detectedMaxCll = 0.0f;
    float                  detectedMaxFall = 0.0f;
    bool                   hasHdr10PlusMetadata = false;
    bool                   hasDolbyVisionMetadata = false;
};

inline constexpr float    kHdr10ToneMappingReferenceWhiteNits = 203.0f;
inline constexpr float    kHdr10ToneMappingMaxContentLightCap = 10000.0f;
inline constexpr uint32_t kHdr10ToneMappingDefaultBudgetMs    = 4;
inline constexpr uint32_t kHdr10ToneMappingHardBudgetMs       = 20;

static_assert(std::is_trivially_copyable_v<Hdr10ToneMappingPolicy>,
              "Hdr10ToneMappingPolicy must be trivially copyable");
static_assert(std::is_trivially_copyable_v<Hdr10ToneMappingProbe>,
              "Hdr10ToneMappingProbe must be trivially copyable");

} // namespace ExplorerLens::Engine
