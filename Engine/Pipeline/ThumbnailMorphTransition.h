// ThumbnailMorphTransition.h — Smooth Morph Transitions
// Copyright (c) 2026 ExplorerLens Project
//
// Smooth morph transitions between thumbnail states. Generates interpolated
// frames for loading-to-loaded transitions with easing functions.
//
#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>
#include <cmath>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class EasingFunction : uint8_t {
    Linear,
    EaseInQuad,
    EaseOutQuad,
    EaseInOutQuad,
    EaseInCubic,
    EaseOutCubic,
    EaseInOutCubic,
    EaseOutElastic,
    EaseOutBounce
};

enum class TransitionType : uint8_t {
    CrossFade,
    SlideIn,
    ZoomIn,
    Dissolve,
    Wipe
};

struct TransitionConfig {
    TransitionType type = TransitionType::CrossFade;
    EasingFunction easing = EasingFunction::EaseOutCubic;
    double durationMs = 200.0;
    uint32_t frameCount = 8;
};

struct TransitionFrame {
    std::vector<uint8_t> pixels;
    uint32_t width = 0;
    uint32_t height = 0;
    double timestamp = 0.0;
    float progress = 0.0f;
};

class ThumbnailMorphTransition {
public:
    static ThumbnailMorphTransition& Instance() {
        static ThumbnailMorphTransition instance;
        return instance;
    }

    inline std::vector<TransitionFrame> GenerateTransition(
        const uint8_t* fromPixels, const uint8_t* toPixels,
        uint32_t width, uint32_t height, uint32_t channels,
        const TransitionConfig& config) const {

        std::vector<TransitionFrame> frames;
        frames.reserve(config.frameCount);
        size_t pixelCount = static_cast<size_t>(width) * height * channels;
        double timeStep = config.durationMs / (std::max)(1u, config.frameCount - 1);

        for (uint32_t i = 0; i < config.frameCount; ++i) {
            float linearT = static_cast<float>(i) / (std::max)(1u, config.frameCount - 1);
            float easedT = ApplyEasing(linearT, config.easing);

            TransitionFrame frame;
            frame.width = width;
            frame.height = height;
            frame.timestamp = i * timeStep;
            frame.progress = linearT;
            frame.pixels.resize(pixelCount);

            switch (config.type) {
            case TransitionType::CrossFade:
                BlendCrossFade(fromPixels, toPixels, frame.pixels.data(), pixelCount, easedT);
                break;
            case TransitionType::ZoomIn:
                BlendZoomIn(fromPixels, toPixels, frame.pixels.data(), width, height, channels, easedT);
                break;
            case TransitionType::Dissolve:
                BlendDissolve(fromPixels, toPixels, frame.pixels.data(), width, height, channels, easedT);
                break;
            case TransitionType::Wipe:
                BlendWipe(fromPixels, toPixels, frame.pixels.data(), width, height, channels, easedT);
                break;
            default:
                BlendCrossFade(fromPixels, toPixels, frame.pixels.data(), pixelCount, easedT);
                break;
            }
            frames.push_back(std::move(frame));
        }
        return frames;
    }

    inline float ApplyEasing(float t, EasingFunction easing) const {
        t = (std::max)(0.0f, (std::min)(1.0f, t));
        switch (easing) {
        case EasingFunction::Linear:        return t;
        case EasingFunction::EaseInQuad:    return t * t;
        case EasingFunction::EaseOutQuad:   return t * (2.0f - t);
        case EasingFunction::EaseInOutQuad:
            return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
        case EasingFunction::EaseInCubic:   return t * t * t;
        case EasingFunction::EaseOutCubic: {
            float u = t - 1.0f;
            return u * u * u + 1.0f;
        }
        case EasingFunction::EaseInOutCubic:
            return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
        case EasingFunction::EaseOutElastic: {
            if (t <= 0.0f || t >= 1.0f) return t;
            float p = 0.3f;
            return std::pow(2.0f, -10.0f * t) * std::sin((t - p / 4.0f) * (2.0f * 3.14159f) / p) + 1.0f;
        }
        case EasingFunction::EaseOutBounce: {
            if (t < 1.0f / 2.75f) return 7.5625f * t * t;
            if (t < 2.0f / 2.75f) { float u = t - 1.5f / 2.75f; return 7.5625f * u * u + 0.75f; }
            if (t < 2.5f / 2.75f) { float u = t - 2.25f / 2.75f; return 7.5625f * u * u + 0.9375f; }
            float u = t - 2.625f / 2.75f;
            return 7.5625f * u * u + 0.984375f;
        }
        default: return t;
        }
    }

    inline std::string EasingToString(EasingFunction easing) const {
        switch (easing) {
        case EasingFunction::Linear:         return "Linear";
        case EasingFunction::EaseInQuad:     return "EaseInQuad";
        case EasingFunction::EaseOutQuad:    return "EaseOutQuad";
        case EasingFunction::EaseInOutQuad:  return "EaseInOutQuad";
        case EasingFunction::EaseInCubic:    return "EaseInCubic";
        case EasingFunction::EaseOutCubic:   return "EaseOutCubic";
        case EasingFunction::EaseInOutCubic: return "EaseInOutCubic";
        case EasingFunction::EaseOutElastic: return "EaseOutElastic";
        case EasingFunction::EaseOutBounce:  return "EaseOutBounce";
        default:                             return "Unknown";
        }
    }

private:
    ThumbnailMorphTransition() = default;

    inline void BlendCrossFade(const uint8_t* from, const uint8_t* to, uint8_t* out,
        size_t count, float t) const {
        for (size_t i = 0; i < count; ++i) {
            float a = from ? from[i] : 0.0f;
            float b = to ? to[i] : 0.0f;
            out[i] = static_cast<uint8_t>((std::max)(0.0f, (std::min)(255.0f, a * (1.0f - t) + b * t)));
        }
    }

    inline void BlendZoomIn(const uint8_t* from, const uint8_t* to, uint8_t* out,
        uint32_t w, uint32_t h, uint32_t c, float t) const {
        float scale = 1.0f + (1.0f - t) * 0.3f;
        float cx = w * 0.5f, cy = h * 0.5f;
        for (uint32_t y = 0; y < h; ++y) {
            for (uint32_t x = 0; x < w; ++x) {
                int sx = static_cast<int>((x - cx) * scale + cx);
                int sy = static_cast<int>((y - cy) * scale + cy);
                size_t dstIdx = (static_cast<size_t>(y) * w + x) * c;
                if (sx >= 0 && sx < static_cast<int>(w) && sy >= 0 && sy < static_cast<int>(h)) {
                    size_t srcIdx = (static_cast<size_t>(sy) * w + sx) * c;
                    for (uint32_t ch = 0; ch < c; ++ch) {
                        float fromVal = from ? from[srcIdx + ch] : 0.0f;
                        float toVal = to ? to[dstIdx + ch] : 0.0f;
                        out[dstIdx + ch] = static_cast<uint8_t>(fromVal * (1.0f - t) + toVal * t);
                    }
                }
                else {
                    for (uint32_t ch = 0; ch < c; ++ch) {
                        out[dstIdx + ch] = to ? to[dstIdx + ch] : 0;
                    }
                }
            }
        }
    }

    inline void BlendDissolve(const uint8_t* from, const uint8_t* to, uint8_t* out,
        uint32_t w, uint32_t h, uint32_t c, float t) const {
        uint32_t threshold = static_cast<uint32_t>(t * 997);
        for (uint32_t y = 0; y < h; ++y) {
            for (uint32_t x = 0; x < w; ++x) {
                uint32_t hash = (x * 73856093u ^ y * 19349663u) % 997u;
                bool useNew = hash < threshold;
                size_t idx = (static_cast<size_t>(y) * w + x) * c;
                for (uint32_t ch = 0; ch < c; ++ch) {
                    out[idx + ch] = useNew ? (to ? to[idx + ch] : 0) : (from ? from[idx + ch] : 0);
                }
            }
        }
    }

    inline void BlendWipe(const uint8_t* from, const uint8_t* to, uint8_t* out,
        uint32_t w, uint32_t h, uint32_t c, float t) const {
        uint32_t boundary = static_cast<uint32_t>(t * w);
        for (uint32_t y = 0; y < h; ++y) {
            for (uint32_t x = 0; x < w; ++x) {
                size_t idx = (static_cast<size_t>(y) * w + x) * c;
                const uint8_t* src = x < boundary ? to : from;
                for (uint32_t ch = 0; ch < c; ++ch) {
                    out[idx + ch] = src ? src[idx + ch] : 0;
                }
            }
        }
    }
};

}
} // namespace ExplorerLens::Engine
