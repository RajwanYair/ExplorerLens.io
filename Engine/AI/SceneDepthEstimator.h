// SceneDepthEstimator.h — Monocular Depth Estimation
// Copyright (c) 2026 ExplorerLens Project
//
// Monocular depth estimation from a single image. Generates depth maps using
// gradient-based heuristics for parallax-ready thumbnail generation.
//
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <array>

namespace ExplorerLens {
namespace Engine {

struct MonocularDepthMap {
    std::vector<float> depth;
    uint32_t width = 0;
    uint32_t height = 0;
    float minDepth = 0.0f;
    float maxDepth = 1.0f;
    float avgDepth = 0.5f;
};

enum class DepthMethod : uint8_t {
    GradientBased,
    FocusBased,
    PositionBased,
    Combined
};

struct DepthConfig {
    DepthMethod method = DepthMethod::Combined;
    float nearPlane = 0.0f;
    float farPlane = 1.0f;
    float focusWeight = 0.4f;
    float positionWeight = 0.3f;
    float gradientWeight = 0.3f;
    uint32_t blurRadius = 3;
};

class SceneDepthEstimator {
public:
    static SceneDepthEstimator& Instance() {
        static SceneDepthEstimator instance;
        return instance;
    }

    inline MonocularDepthMap EstimateDepth(const uint8_t* pixels, uint32_t width, uint32_t height,
        uint32_t channels, const DepthConfig& config = {}) const {
        MonocularDepthMap result;
        result.width = width;
        result.height = height;
        size_t count = static_cast<size_t>(width) * height;
        result.depth.resize(count, 0.5f);
        if (!pixels || count == 0) return result;

        std::vector<float> gradientDepth, focusDepth, positionDepth;

        if (config.method == DepthMethod::GradientBased || config.method == DepthMethod::Combined) {
            gradientDepth = EstimateFromGradient(pixels, width, height, channels);
        }
        if (config.method == DepthMethod::FocusBased || config.method == DepthMethod::Combined) {
            focusDepth = EstimateFromFocus(pixels, width, height, channels, config.blurRadius);
        }
        if (config.method == DepthMethod::PositionBased || config.method == DepthMethod::Combined) {
            positionDepth = EstimateFromPosition(width, height);
        }

        if (config.method == DepthMethod::Combined) {
            for (size_t i = 0; i < count; ++i) {
                result.depth[i] = config.gradientWeight * gradientDepth[i] +
                    config.focusWeight * focusDepth[i] +
                    config.positionWeight * positionDepth[i];
            }
        }
        else if (config.method == DepthMethod::GradientBased) {
            result.depth = std::move(gradientDepth);
        }
        else if (config.method == DepthMethod::FocusBased) {
            result.depth = std::move(focusDepth);
        }
        else {
            result.depth = std::move(positionDepth);
        }

        NormalizeDepth(result);
        return result;
    }

    inline std::vector<uint8_t> RenderDepthVisualization(const MonocularDepthMap& depth) const {
        std::vector<uint8_t> vis(static_cast<size_t>(depth.width) * depth.height * 3);
        for (size_t i = 0; i < depth.depth.size(); ++i) {
            float d = (std::max)(0.0f, (std::min)(1.0f, depth.depth[i]));
            auto color = DepthToColor(d);
            vis[i * 3] = color[0];
            vis[i * 3 + 1] = color[1];
            vis[i * 3 + 2] = color[2];
        }
        return vis;
    }

    inline std::vector<uint8_t> GenerateParallaxFrame(const uint8_t* pixels, const MonocularDepthMap& depth,
        uint32_t channels, float shiftX, float shiftY) const {
        uint32_t w = depth.width, h = depth.height;
        std::vector<uint8_t> output(static_cast<size_t>(w) * h * 3, 0);
        if (!pixels) return output;

        for (uint32_t y = 0; y < h; ++y) {
            for (uint32_t x = 0; x < w; ++x) {
                float d = depth.depth[y * w + x];
                float offsetX = shiftX * d * 10.0f;
                float offsetY = shiftY * d * 10.0f;

                int srcX = static_cast<int>(x - offsetX);
                int srcY = static_cast<int>(y - offsetY);
                srcX = (std::max)(0, (std::min)(static_cast<int>(w) - 1, srcX));
                srcY = (std::max)(0, (std::min)(static_cast<int>(h) - 1, srcY));

                size_t srcIdx = (srcY * w + srcX) * channels;
                size_t dstIdx = (static_cast<size_t>(y) * w + x) * 3;
                for (uint32_t c = 0; c < 3 && c < channels; ++c) {
                    output[dstIdx + c] = pixels[srcIdx + c];
                }
            }
        }
        return output;
    }

    inline std::string MethodToString(DepthMethod method) const {
        switch (method) {
        case DepthMethod::GradientBased: return "Gradient-Based";
        case DepthMethod::FocusBased:    return "Focus-Based";
        case DepthMethod::PositionBased: return "Position-Based";
        case DepthMethod::Combined:      return "Combined";
        default:                         return "Unknown";
        }
    }

private:
    SceneDepthEstimator() = default;

    inline std::vector<float> EstimateFromGradient(const uint8_t* pixels, uint32_t w, uint32_t h, uint32_t c) const {
        size_t count = static_cast<size_t>(w) * h;
        std::vector<float> depth(count, 0.5f);
        for (uint32_t y = 1; y < h - 1; ++y) {
            for (uint32_t x = 1; x < w - 1; ++x) {
                auto lum = [&](uint32_t px, uint32_t py) {
                    size_t idx = (py * w + px) * c;
                    return 0.299f * pixels[idx] + 0.587f * pixels[idx + (std::min)(1u, c - 1)] +
                        0.114f * pixels[idx + (std::min)(2u, c - 1)];
                    };
                float gx = lum(x + 1, y) - lum(x - 1, y);
                float gy = lum(x, y + 1) - lum(x, y - 1);
                float gradient = std::sqrt(gx * gx + gy * gy) / 360.0f;
                depth[y * w + x] = 1.0f - (std::min)(1.0f, gradient);
            }
        }
        return depth;
    }

    inline std::vector<float> EstimateFromFocus(const uint8_t* pixels, uint32_t w, uint32_t h,
        uint32_t c, uint32_t blurRadius) const {
        size_t count = static_cast<size_t>(w) * h;
        std::vector<float> depth(count, 0.5f);
        int r = static_cast<int>(blurRadius);

        for (uint32_t y = r; y < h - r; ++y) {
            for (uint32_t x = r; x < w - r; ++x) {
                float variance = 0.0f;
                float mean = 0.0f;
                int samples = 0;
                for (int dy = -r; dy <= r; ++dy) {
                    for (int dx = -r; dx <= r; ++dx) {
                        size_t idx = ((y + dy) * w + (x + dx)) * c;
                        float lum = 0.299f * pixels[idx] + 0.587f * pixels[idx + (std::min)(1u, c - 1)] +
                            0.114f * pixels[idx + (std::min)(2u, c - 1)];
                        mean += lum;
                        ++samples;
                    }
                }
                mean /= samples;
                for (int dy = -r; dy <= r; ++dy) {
                    for (int dx = -r; dx <= r; ++dx) {
                        size_t idx = ((y + dy) * w + (x + dx)) * c;
                        float lum = 0.299f * pixels[idx] + 0.587f * pixels[idx + (std::min)(1u, c - 1)] +
                            0.114f * pixels[idx + (std::min)(2u, c - 1)];
                        variance += (lum - mean) * (lum - mean);
                    }
                }
                variance /= samples;
                depth[y * w + x] = (std::min)(1.0f, variance / 2000.0f);
            }
        }
        return depth;
    }

    inline std::vector<float> EstimateFromPosition(uint32_t w, uint32_t h) const {
        size_t count = static_cast<size_t>(w) * h;
        std::vector<float> depth(count);
        for (uint32_t y = 0; y < h; ++y) {
            float t = static_cast<float>(y) / (std::max)(1u, h - 1);
            for (uint32_t x = 0; x < w; ++x) {
                float cx = std::abs(static_cast<float>(x) / w - 0.5f) * 2.0f;
                depth[y * w + x] = t * 0.8f + cx * 0.2f;
            }
        }
        return depth;
    }

    inline void NormalizeDepth(MonocularDepthMap& dm) const {
        if (dm.depth.empty()) return;
        float minD = dm.depth[0], maxD = dm.depth[0];
        double sum = 0.0;
        for (float d : dm.depth) {
            if (d < minD) minD = d;
            if (d > maxD) maxD = d;
            sum += d;
        }
        dm.minDepth = minD;
        dm.maxDepth = maxD;
        dm.avgDepth = static_cast<float>(sum / dm.depth.size());
        float range = maxD - minD;
        if (range > 1e-6f) {
            for (auto& d : dm.depth) d = (d - minD) / range;
        }
    }

    inline std::array<uint8_t, 3> DepthToColor(float d) const {
        if (d < 0.5f) {
            float t = d * 2.0f;
            return { 0, static_cast<uint8_t>(t * 255), static_cast<uint8_t>((1.0f - t) * 255) };
        }
        float t = (d - 0.5f) * 2.0f;
        return { static_cast<uint8_t>(t * 255), static_cast<uint8_t>((1.0f - t) * 255), 0 };
    }
};

}
} // namespace ExplorerLens::Engine
