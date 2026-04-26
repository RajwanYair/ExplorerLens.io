// ============================================================================
// AnimatedImagePlaybackContract.h -- S273 / ROADMAP v6.0 L14 animated images
//
// Phase 3 contract for animated thumbnails (APNG / animated WebP / GIF /
// AVIF-AIS).  Drives the IPreviewHandler pane when `allowAnimation` is set.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class AnimatedImageCodec : uint8_t
{
    NONE         = 0,
    GIF          = 1,
    APNG         = 2,
    ANIM_WEBP    = 3,
    ANIM_AVIF    = 4,
    ANIM_JXL     = 5,
    LOTTIE_JSON  = 6,   // dotLottie rasterisation
};

enum class AnimatedImageLoopMode : uint8_t
{
    LOOP_FROM_FILE = 0,
    LOOP_ONCE      = 1,
    LOOP_FOREVER   = 2,
    CLAMP_LAST     = 3,   // freeze on final frame
};

enum class AnimatedImagePlaybackStatus : uint8_t
{
    OK                      = 0,
    CODEC_UNSUPPORTED       = 1,
    TOO_MANY_FRAMES         = 2,
    FRAME_DECODE_FAILED     = 3,
    MEMORY_BUDGET_EXCEEDED  = 4,
    BUDGET_EXCEEDED         = 5,
};

struct AnimatedImagePlaybackOptions
{
    AnimatedImageLoopMode loopMode       = AnimatedImageLoopMode::LOOP_FROM_FILE;
    uint32_t              maxFrames      = 1024;
    uint32_t              minFrameMs     = 20;      // cap to 50 fps
    uint32_t              decodeBudgetMs = 16;      // per-frame budget
    uint32_t              memoryBudgetMb = 128;
    bool                  skipOnBattery  = true;    // honour power state
    bool                  preferHwDecode = true;
};

struct AnimatedImagePlaybackProbe
{
    AnimatedImagePlaybackStatus status     = AnimatedImagePlaybackStatus::OK;
    AnimatedImageCodec          codec      = AnimatedImageCodec::NONE;
    uint32_t                    frameCount = 0;
    uint32_t                    loopCountFromFile = 0;   // 0 = infinite
    uint32_t                    declaredFps = 0;
    uint32_t                    pixelBytes  = 0;
};

inline constexpr uint32_t kAnimatedImageMaxFrames          = 10000;
inline constexpr uint32_t kAnimatedImageDefaultBudgetMs    = 16;
inline constexpr uint32_t kAnimatedImageHardBudgetMs       = 66;   // 15 fps floor
inline constexpr uint32_t kAnimatedImageMaxMemoryMb        = 512;

static_assert(std::is_trivially_copyable_v<AnimatedImagePlaybackOptions>,
              "AnimatedImagePlaybackOptions must be trivially copyable");
static_assert(std::is_trivially_copyable_v<AnimatedImagePlaybackProbe>,
              "AnimatedImagePlaybackProbe must be trivially copyable");
static_assert(kAnimatedImageDefaultBudgetMs < kAnimatedImageHardBudgetMs,
              "animated default budget < hard budget");

} // namespace ExplorerLens::Engine
