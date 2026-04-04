// ObjectSaliencyMapper.h — Visual Saliency Map Generation
// Copyright (c) 2026 ExplorerLens Project
//
// Visual saliency map generation using Itti-Koch model. Computes color, intensity,
// and orientation feature channels to identify regions of visual interest.
//
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct ObjectSaliencyMap
{
    std::vector<float> data;
    uint32_t width = 0;
    uint32_t height = 0;
    float maxSaliency = 0.0f;
    float avgSaliency = 0.0f;
};

struct ObjectSaliencyRegion
{
    int32_t x = 0;
    int32_t y = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    float saliency = 0.0f;
};

struct SaliencyConfig
{
    float intensityWeight = 0.33f;
    float colorWeight = 0.33f;
    float orientationWeight = 0.34f;
    uint32_t gaussianLevels = 4;
    float normalizationPower = 2.0f;
};

class ObjectSaliencyMapper
{
  public:
    static ObjectSaliencyMapper& Instance()
    {
        static ObjectSaliencyMapper instance;
        return instance;
    }

    inline ObjectSaliencyMap ComputeSaliency(const uint8_t* pixels, uint32_t width, uint32_t height,
                                             uint32_t channels = 4, const SaliencyConfig& config = {}) const
    {
        ObjectSaliencyMap result;
        result.width = width;
        result.height = height;
        size_t pixelCount = static_cast<size_t>(width) * height;
        result.data.resize(pixelCount, 0.0f);
        if (!pixels || pixelCount == 0)
            return result;

        auto intensityMap = ComputeIntensityChannel(pixels, width, height, channels);
        auto colorMap = ComputeColorChannel(pixels, width, height, channels);
        auto orientationMap = ComputeOrientationChannel(intensityMap, width, height);

        float maxVal = 0.0f;
        double sumVal = 0.0;
        for (size_t i = 0; i < pixelCount; ++i) {
            result.data[i] = config.intensityWeight * intensityMap[i] + config.colorWeight * colorMap[i]
                             + config.orientationWeight * orientationMap[i];
            if (result.data[i] > maxVal)
                maxVal = result.data[i];
            sumVal += result.data[i];
        }

        if (maxVal > 0.0f) {
            for (auto& v : result.data)
                v /= maxVal;
        }
        result.maxSaliency = 1.0f;
        result.avgSaliency = static_cast<float>(sumVal / pixelCount / (std::max)(0.001f, maxVal));
        return result;
    }

    inline ObjectSaliencyRegion FindMostSalientRegion(const ObjectSaliencyMap& map, uint32_t regionW,
                                                      uint32_t regionH) const
    {
        ObjectSaliencyRegion best;
        if (map.data.empty())
            return best;

        uint32_t stepX = (std::max)(1u, regionW / 4);
        uint32_t stepY = (std::max)(1u, regionH / 4);
        float bestScore = -1.0f;

        for (uint32_t y = 0; y + regionH <= map.height; y += stepY) {
            for (uint32_t x = 0; x + regionW <= map.width; x += stepX) {
                float score = 0.0f;
                for (uint32_t ry = 0; ry < regionH; ++ry) {
                    for (uint32_t rx = 0; rx < regionW; ++rx) {
                        score += map.data[(y + ry) * map.width + (x + rx)];
                    }
                }
                if (score > bestScore) {
                    bestScore = score;
                    best.x = static_cast<int32_t>(x);
                    best.y = static_cast<int32_t>(y);
                    best.width = regionW;
                    best.height = regionH;
                    best.saliency = score / (regionW * regionH);
                }
            }
        }
        return best;
    }

    inline std::vector<uint8_t> RenderHeatmap(const ObjectSaliencyMap& map) const
    {
        std::vector<uint8_t> heatmap(static_cast<size_t>(map.width) * map.height * 3);
        for (size_t i = 0; i < map.data.size(); ++i) {
            float v = (std::max)(0.0f, (std::min)(1.0f, map.data[i]));
            size_t idx = i * 3;
            if (v < 0.25f) {
                heatmap[idx] = 0;
                heatmap[idx + 1] = static_cast<uint8_t>(v * 4 * 255);
                heatmap[idx + 2] = 255;
            } else if (v < 0.5f) {
                heatmap[idx] = 0;
                heatmap[idx + 1] = 255;
                heatmap[idx + 2] = static_cast<uint8_t>((1.0f - (v - 0.25f) * 4) * 255);
            } else if (v < 0.75f) {
                heatmap[idx] = static_cast<uint8_t>((v - 0.5f) * 4 * 255);
                heatmap[idx + 1] = 255;
                heatmap[idx + 2] = 0;
            } else {
                heatmap[idx] = 255;
                heatmap[idx + 1] = static_cast<uint8_t>((1.0f - (v - 0.75f) * 4) * 255);
                heatmap[idx + 2] = 0;
            }
        }
        return heatmap;
    }

  private:
    ObjectSaliencyMapper() = default;

    inline std::vector<float> ComputeIntensityChannel(const uint8_t* pixels, uint32_t w, uint32_t h, uint32_t c) const
    {
        size_t count = static_cast<size_t>(w) * h;
        std::vector<float> intensity(count);
        for (size_t i = 0; i < count; ++i) {
            size_t idx = i * c;
            intensity[i] = (0.299f * pixels[idx] + 0.587f * pixels[idx + (std::min)(1u, c - 1)]
                            + 0.114f * pixels[idx + (std::min)(2u, c - 1)])
                           / 255.0f;
        }

        float mean = 0.0f;
        for (auto v : intensity)
            mean += v;
        mean /= count;

        for (auto& v : intensity) {
            v = std::abs(v - mean);
        }
        return intensity;
    }

    inline std::vector<float> ComputeColorChannel(const uint8_t* pixels, uint32_t w, uint32_t h, uint32_t c) const
    {
        size_t count = static_cast<size_t>(w) * h;
        std::vector<float> colorSaliency(count, 0.0f);
        if (c < 3)
            return colorSaliency;

        float meanR = 0.0f, meanG = 0.0f, meanB = 0.0f;
        for (size_t i = 0; i < count; ++i) {
            size_t idx = i * c;
            meanR += pixels[idx];
            meanG += pixels[idx + 1];
            meanB += pixels[idx + 2];
        }
        meanR /= count;
        meanG /= count;
        meanB /= count;

        for (size_t i = 0; i < count; ++i) {
            size_t idx = i * c;
            float dr = pixels[idx] - meanR;
            float dg = pixels[idx + 1] - meanG;
            float db = pixels[idx + 2] - meanB;
            colorSaliency[i] = std::sqrt(dr * dr + dg * dg + db * db) / 441.0f;
        }
        return colorSaliency;
    }

    inline std::vector<float> ComputeOrientationChannel(const std::vector<float>& intensity, uint32_t w,
                                                        uint32_t h) const
    {
        size_t count = static_cast<size_t>(w) * h;
        std::vector<float> orientation(count, 0.0f);
        if (w < 3 || h < 3)
            return orientation;

        for (uint32_t y = 1; y < h - 1; ++y) {
            for (uint32_t x = 1; x < w - 1; ++x) {
                float gx = intensity[y * w + x + 1] - intensity[y * w + x - 1];
                float gy = intensity[(y + 1) * w + x] - intensity[(y - 1) * w + x];
                orientation[y * w + x] = std::sqrt(gx * gx + gy * gy);
            }
        }
        return orientation;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
