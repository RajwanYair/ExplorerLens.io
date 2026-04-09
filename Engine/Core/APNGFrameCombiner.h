// APNGFrameCombiner.h — Animated PNG Frame Compositor
// Copyright (c) 2026 ExplorerLens Project
//
// Composites APNG animation frames according to fnctl/fdatl dispose and
// blend operations (APNG spec Section 4). Produces a flat sequence of
// BGRA32 frames from decoded APNG chunk data, ready for thumbnail scrub
// or key-frame extraction. Reuses existing APNGDecoder for raw frame decode.
//
#pragma once
#include <cstdint>
#include <vector>
#include <memory>

namespace ExplorerLens { namespace Engine {

enum class APNGBlend : uint8_t {
    Source = 0,   // Replace — fcTL blend_op 0
    Over   = 1,   // Alpha composite — fcTL blend_op 1
};

enum class APNGDispose : uint8_t {
    None       = 0,  // No disposal before next frame
    Background = 1,  // Clear region to background colour
    Previous   = 2,  // Restore region to previous frame
};

struct APNGRawFrame {
    const uint8_t* rgba    = nullptr;  // Decoded RGBA8 pixels
    uint32_t       width   = 0;
    uint32_t       height  = 0;
    uint32_t       xOffset = 0;
    uint32_t       yOffset = 0;
    APNGBlend      blend   = APNGBlend::Source;
    APNGDispose    dispose = APNGDispose::None;
    uint32_t       delayMs = 100;
};

struct APNGCompositeResult {
    std::vector<std::vector<uint8_t>> framesRGBA;  // Each: width × height × 4 bytes
    uint32_t width      = 0;
    uint32_t height     = 0;
    uint32_t frameCount = 0;
    bool     success    = false;
};

class APNGFrameCombiner {
public:
    APNGFrameCombiner()  = default;
    ~APNGFrameCombiner() = default;

    // Composite a sequence of raw APNG frames into full-canvas BGRA32 frames.
    APNGCompositeResult Composite(
        const std::vector<APNGRawFrame>& rawFrames,
        uint32_t canvasWidth,
        uint32_t canvasHeight) const noexcept;

    // Extract up to `maxFrames` evenly-spaced representative key frames from
    // the composite result (used for thumbnail strip generation).
    static std::vector<uint32_t> SelectKeyFrameIndices(
        uint32_t totalFrames, uint32_t maxFrames) noexcept;

    // Quick probe: count animation frames in a raw PNG/APNG byte buffer.
    static uint32_t ProbeFrameCount(
        const uint8_t* pngData, size_t pngSize) noexcept;
};

}} // namespace ExplorerLens::Engine
