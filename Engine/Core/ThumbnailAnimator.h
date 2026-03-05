// ThumbnailAnimator.h — Animated Thumbnail Support
// Copyright (c) 2026 ExplorerLens Project
//
// Generates animated thumbnail previews for GIF, APNG, WebP-anim,
// and video formats by extracting key frames and compositing them
// into a representative thumbnail strip or animated preview.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class AnimationFormat : uint8_t {
    GIF, APNG, WebPAnimated, VideoKeyframes, SpriteSheet, COUNT
};

enum class AnimationMode : uint8_t {
    FirstFrame, KeyFrameStrip, AnimatedPreview, LoopPreview, COUNT
};

struct ThumbAnimFrame {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t delayMs = 100;
    uint32_t frameIndex = 0;
    bool isKeyFrame = false;
};

struct AnimationConfig {
    AnimationMode mode = AnimationMode::FirstFrame;
    uint32_t maxFrames = 8;
    uint32_t targetFPS = 10;
    uint32_t maxDurationMs = 3000;
    uint32_t stripColumns = 4;
    bool extractKeyFrames = true;
};

struct AnimationResult {
    AnimationFormat format = AnimationFormat::GIF;
    uint32_t totalFrames = 0;
    uint32_t extractedFrames = 0;
    uint32_t durationMs = 0;
    double extractionMs = 0.0;
    bool animated = false;
};

class ThumbnailAnimator {
public:
    void Configure(const AnimationConfig& cfg) { m_config = cfg; }
    const AnimationConfig& GetConfig() const { return m_config; }

    AnimationResult Analyze(AnimationFormat fmt, uint32_t totalFrames, uint32_t durationMs) {
        AnimationResult result;
        result.format = fmt;
        result.totalFrames = totalFrames;
        result.durationMs = durationMs;
        result.animated = (totalFrames > 1);
        result.extractedFrames = (totalFrames <= m_config.maxFrames)
            ? totalFrames : m_config.maxFrames;
        return result;
    }

    std::vector<uint32_t> SelectKeyFrameIndices(uint32_t totalFrames) const {
        std::vector<uint32_t> indices;
        if (totalFrames == 0) return indices;
        uint32_t count = (totalFrames <= m_config.maxFrames) ? totalFrames : m_config.maxFrames;
        float step = static_cast<float>(totalFrames) / static_cast<float>(count);
        for (uint32_t i = 0; i < count; ++i) {
            indices.push_back(static_cast<uint32_t>(i * step));
        }
        return indices;
    }

    AnimationMode GetMode() const { return m_config.mode; }

    static size_t FormatCount() { return static_cast<size_t>(AnimationFormat::COUNT); }
    static size_t ModeCount() { return static_cast<size_t>(AnimationMode::COUNT); }

private:
    AnimationConfig m_config;
};

} // namespace Engine
} // namespace ExplorerLens
