// AdaptiveCropEngine.h — Intelligent Thumbnail Cropping
// Copyright (c) 2026 ExplorerLens Project
//
// Determines optimal crop rectangles using aspect ratio analysis, saliency
// hints, and content-aware heuristics to maximize thumbnail informativeness.
//
#pragma once

#include <cstdint>
#include <algorithm>
#include <cmath>

namespace ExplorerLens {
namespace Engine {

enum class AdaptiveCropStrategy : uint8_t {
    Center = 0,       // Simple center crop
    ThirdsRule = 1,   // Rule-of-thirds intersection
    SaliencyGuided = 2, // Crop around salient region
    FaceFocused = 3,  // Center on detected face
    ContentAware = 4  // Combined heuristic
};

struct AdaptiveCropRect {
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

struct CropConfig {
    AdaptiveCropStrategy strategy = AdaptiveCropStrategy::ContentAware;
    float targetAspectRatio = 1.0f; // width / height
    float maxCropFraction = 0.4f;   // Max 40% of image can be cropped
    float salientX = 0.5f;         // Saliency center hint (0..1)
    float salientY = 0.5f;
    uint32_t minDimensionPx = 32;
};

struct AdaptiveCropResult {
    AdaptiveCropRect rect;
    AdaptiveCropStrategy usedStrategy = AdaptiveCropStrategy::Center;
    float croppedFraction = 0.0f;
    float qualityScore = 0.0f;
};

class AdaptiveCropEngine {
public:
    void Configure(const CropConfig& config) { m_config = config; }

    AdaptiveCropResult ComputeCrop(uint32_t srcWidth, uint32_t srcHeight) const {
        AdaptiveCropResult result;
        if (srcWidth == 0 || srcHeight == 0) return result;

        float srcAspect = static_cast<float>(srcWidth) / srcHeight;
        float tgtAspect = m_config.targetAspectRatio;

        uint32_t cropW, cropH;
        if (srcAspect > tgtAspect) {
            // Image is wider — crop horizontally
            cropH = srcHeight;
            cropW = static_cast<uint32_t>(srcHeight * tgtAspect);
        }
        else {
            // Image is taller — crop vertically
            cropW = srcWidth;
            cropH = static_cast<uint32_t>(srcWidth / tgtAspect);
        }
        cropW = std::min(cropW, srcWidth);
        cropH = std::min(cropH, srcHeight);
        cropW = std::max(cropW, m_config.minDimensionPx);
        cropH = std::max(cropH, m_config.minDimensionPx);

        // Check max crop fraction
        float fraction = 1.0f - (static_cast<float>(cropW) * cropH) / (srcWidth * srcHeight);
        if (fraction > m_config.maxCropFraction) {
            // Fall back to center with less aggressive crop
            cropW = srcWidth;
            cropH = srcHeight;
            fraction = 0.0f;
        }

        // Position based on strategy
        uint32_t cx, cy;
        if (m_config.strategy == AdaptiveCropStrategy::SaliencyGuided ||
            m_config.strategy == AdaptiveCropStrategy::ContentAware) {
            cx = static_cast<uint32_t>(m_config.salientX * srcWidth);
            cy = static_cast<uint32_t>(m_config.salientY * srcHeight);
            // Clamp so crop doesn't go out of bounds
            cx = std::clamp(cx, cropW / 2, srcWidth - cropW / 2);
            cy = std::clamp(cy, cropH / 2, srcHeight - cropH / 2);
            result.rect.x = cx - cropW / 2;
            result.rect.y = cy - cropH / 2;
        }
        else {
            result.rect.x = (srcWidth - cropW) / 2;
            result.rect.y = (srcHeight - cropH) / 2;
        }

        result.rect.width = cropW;
        result.rect.height = cropH;
        result.usedStrategy = m_config.strategy;
        result.croppedFraction = fraction;
        result.qualityScore = 1.0f - fraction * 0.5f;
        return result;
    }

private:
    CropConfig m_config;
};

} // namespace Engine
} // namespace ExplorerLens
