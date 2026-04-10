// AnimatedSequenceSampler.h — Multi-Format Animated Frame Key-Frame Sampler
// Copyright (c) 2026 ExplorerLens Project
//
// Unified front-end for extracting a fixed set of evenly-spaced key frames
// from any animated format (GIF, APNG, animated WebP, animated AVIF, animated
// JXL, MNG). Dispatches to the appropriate format decoder and returns a
// normalised set of BGRA32 key frames for hover-scrub thumbnail display.
// Target: < 8 ms total for 5 frames from a 100-frame clip.
//
#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace ExplorerLens { namespace Engine {

enum class SampledAnimFormat : uint8_t {
    Unknown      = 0,
    GIF          = 1,
    APNG         = 2,
    AnimatedWebP = 3,
    AnimatedAVIF = 4,
    AnimatedJXL  = 5,
    MNG          = 6,
    HEICLivePhoto = 7,
};

struct AnimatedKeyFrame {
    std::vector<uint8_t> pixelsBGRA;  // width × height × 4 bytes
    uint32_t width    = 0;
    uint32_t height   = 0;
    uint32_t frameIdx = 0;            // Original frame index in the source
    uint32_t delayMs  = 100;
};

struct AnimatedSampleResult {
    std::vector<AnimatedKeyFrame> keyFrames;
    uint32_t        totalFrames     = 0;
    SampledAnimFormat  format          = SampledAnimFormat::Unknown;
    bool            success         = false;
    float           processMs       = 0.0f;
};

struct AnimatedSampleOptions {
    uint32_t maxKeyFrames    = 5;   // How many key frames to extract
    uint32_t targetWidth     = 256; // Scale frames to this width (0 = native)
    uint32_t targetHeight    = 256;
    bool     evenDistribution = true; // Distribute key frame indices evenly
};

class AnimatedSequenceSampler {
public:
    AnimatedSequenceSampler()  = default;
    ~AnimatedSequenceSampler() = default;

    // Detect animated format from raw bytes.
    static SampledAnimFormat Detect(
        const uint8_t* data, size_t size) noexcept;

    // Extract key frames from raw animated format bytes.
    AnimatedSampleResult Sample(
        const uint8_t*              data,
        size_t                      size,
        const AnimatedSampleOptions& opts = {}) const noexcept;

    // Quick probe: return total frame count without full decode.
    static uint32_t ProbeFrameCount(
        const uint8_t* data, size_t size, SampledAnimFormat hint = SampledAnimFormat::Unknown) noexcept;
};

}} // namespace ExplorerLens::Engine
