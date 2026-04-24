// ============================================================================
// MediaFoundationKeyframeStub.h -- S265 / ROADMAP v6.0 H10 video thumbnail
//
// Evaluation contract for extracting a representative keyframe from video
// containers (MP4/MKV/MOV/WEBM/AVI) via Windows Media Foundation.  Replaces
// the brittle ad-hoc FFmpeg path used in early v39 experiments.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class MediaFoundationSeekPolicy : uint8_t
{
    FIRST_KEYFRAME       = 0,   // cheapest, often black frame
    NEAREST_TO_TIMECODE  = 1,   // seek to user-supplied offset
    FIXED_PERCENT_10     = 2,   // 10% into duration (good default)
    FIXED_PERCENT_25     = 3,
    LARGEST_I_FRAME      = 4,   // scan first N seconds, pick max coded size
};

enum class MediaFoundationKeyframeStatus : uint8_t
{
    OK                        = 0,
    CONTAINER_UNSUPPORTED     = 1,
    CODEC_UNSUPPORTED         = 2,
    DRM_PROTECTED             = 3,
    NO_VIDEO_STREAM           = 4,
    SEEK_FAILED               = 5,
    DECODE_FAILED             = 6,
    BUDGET_EXCEEDED           = 7,
    OUT_OF_MEMORY             = 8,
};

struct MediaFoundationKeyframeOptions
{
    MediaFoundationSeekPolicy seekPolicy   = MediaFoundationSeekPolicy::FIXED_PERCENT_10;
    uint64_t                  timecodeMs   = 0;    // only for NEAREST_TO_TIMECODE
    uint32_t                  maxScanMs    = 2000; // cap for LARGEST_I_FRAME search
    uint32_t                  targetWidth  = 0;
    uint32_t                  targetHeight = 0;
    bool                      allowHardwareDecode = true;
    uint32_t                  budgetMs     = 250;
};

struct MediaFoundationKeyframeResult
{
    MediaFoundationKeyframeStatus status      = MediaFoundationKeyframeStatus::OK;
    uint32_t                      emittedWidth  = 0;
    uint32_t                      emittedHeight = 0;
    uint64_t                      durationMs    = 0;
    uint64_t                      pickedTimecodeMs = 0;
    uint32_t                      latencyMs     = 0;
    bool                          usedHardware  = false;
};

inline constexpr uint32_t kMediaFoundationDefaultBudgetMs = 250;
inline constexpr uint32_t kMediaFoundationHardBudgetMs    = 1000;
inline constexpr uint32_t kMediaFoundationMaxScanMs       = 5000;

static_assert(std::is_trivially_copyable_v<MediaFoundationKeyframeOptions>,
              "MediaFoundationKeyframeOptions must be trivially copyable");
static_assert(std::is_trivially_copyable_v<MediaFoundationKeyframeResult>,
              "MediaFoundationKeyframeResult must be trivially copyable");

} // namespace ExplorerLens::Engine
