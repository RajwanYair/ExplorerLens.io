// FrameInterpolator.h — AI Frame Interpolation for Animated Thumbnails
// Copyright (c) 2026 ExplorerLens Project
//
// GPU-accelerated temporal frame interpolation pipeline for animated format
// thumbnails (GIF, WebP animation, AVIF sequence, APNG, video EAP frames).
// Generates smooth intermediate frames using RIFE (Real-Time Intermediate
// Flow Estimation) model via ONNX DirectML execution provider.
//
// Used by the animated thumbnail preview in the WinUI 3 manager and the
// shell extension hover preview mode.
//
#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <functional>

namespace ExplorerLens { namespace Engine {

// Interpolation quality/speed tradeoff.
enum class InterpolationMode : uint8_t {
    Fast    = 0,  // 2× frame count, RIFE-Lite (lower quality, higher throughput)
    Normal  = 1,  // 2× frame count, RIFE-v4 (balanced — default)
    Smooth  = 2,  // 4× frame count, RIFE-v4 (silky smooth, 4× GPU budget)
};

// A single decoded frame (owned BGRA buffer + duration).
struct AnimFrame {
    std::vector<uint8_t> bgra;         // BGRA raw pixels
    uint32_t             width  { 0 };
    uint32_t             height { 0 };
    uint32_t             delayMs { 0 }; // Original playback delay
};

// Result from the interpolation pipeline.
struct InterpolationResult {
    std::vector<AnimFrame> frames;      // Interpolated + original frames interleaved
    bool                   success  { false };
    std::string            errorMessage;
    double                 totalLatencyMs { 0.0 };
};

// Progress callback: (framesDone, framesTotal)
using InterpolationProgressFn = std::function<void(uint32_t, uint32_t)>;

// FrameInterpolator — AI frame interpolation for animated thumbnails.
//
// Input: decoded animation frames from GifDecoder/WebPDecoder/AvifDecoder.
// Output: InterpolationResult with 2× or 4× frame count.
// Requires: ONNX Runtime + DirectML EP + RIFE model file.
class FrameInterpolator {
public:
    FrameInterpolator() noexcept;
    ~FrameInterpolator() noexcept;

    FrameInterpolator(const FrameInterpolator&)            = delete;
    FrameInterpolator& operator=(const FrameInterpolator&) = delete;

    // Initialise with RIFE model path.  Returns false if ONNX/DirectML unavailable.
    bool Initialize(const std::string& rifeModelPath) noexcept;

    bool IsAvailable() const noexcept { return m_available; }

    // Interpolate frames.  onProgress called per interpolated pair.
    InterpolationResult Interpolate(const std::vector<AnimFrame>& inputFrames,
                                     InterpolationMode             mode       = InterpolationMode::Normal,
                                     InterpolationProgressFn       onProgress = nullptr) noexcept;

    // Convenience: how many output frames will be generated?
    static uint32_t OutputFrameCount(uint32_t inputCount,
                                      InterpolationMode mode) noexcept;

private:
    bool        m_available { false };
    std::string m_modelPath;

    struct Impl;
    Impl* m_impl { nullptr };
};

}} // namespace ExplorerLens::Engine
