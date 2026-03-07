// ThumbnailSaliencyMap.h — Visual Saliency Detection for Thumbnails
// Copyright (c) 2026 ExplorerLens Project
//
// Computes visual saliency maps to identify the most visually important
// regions of an image, guiding smart cropping and focus decisions.
//
#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

struct SaliencyPoint {
    uint32_t x = 0;
    uint32_t y = 0;
    float score = 0.0f;  // 0.0 = not salient, 1.0 = maximally salient
};

struct SaliencyMapRegion {
    uint32_t left = 0;
    uint32_t top = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    float avgSaliency = 0.0f;
};

struct SaliencyMapConfig {
    uint32_t gridSize = 8;       // NxN grid for analysis
    float threshold = 0.3f;      // Minimum saliency to consider
    bool useColorContrast = true;
    bool useLuminanceContrast = true;
    bool useEdgeDetection = true;
};

struct SaliencyResult {
    bool valid = false;
    SaliencyMapRegion primaryRegion;
    float overallSaliency = 0.0f;
    uint32_t salientCellCount = 0;
    uint32_t totalCells = 0;
};

class ThumbnailSaliencyMap {
public:
    void Configure(const SaliencyMapConfig& config) { m_config = config; }

    SaliencyResult AnalyzeFromPixels(const uint8_t* rgbaPixels, uint32_t width,
        uint32_t height) const {
        SaliencyResult result;
        if (!rgbaPixels || width == 0 || height == 0) return result;

        uint32_t cellW = width / m_config.gridSize;
        uint32_t cellH = height / m_config.gridSize;
        if (cellW == 0 || cellH == 0) return result;

        result.totalCells = m_config.gridSize * m_config.gridSize;
        result.valid = true;

        // Compute average luminance per cell
        std::vector<float> cellLum(result.totalCells, 0.0f);
        for (uint32_t cy = 0; cy < m_config.gridSize; ++cy) {
            for (uint32_t cx = 0; cx < m_config.gridSize; ++cx) {
                float sum = 0.0f;
                uint32_t count = 0;
                for (uint32_t py = cy * cellH; py < (cy + 1) * cellH && py < height; ++py) {
                    for (uint32_t px = cx * cellW; px < (cx + 1) * cellW && px < width; ++px) {
                        size_t idx = (static_cast<size_t>(py) * width + px) * 4;
                        float lum = 0.299f * rgbaPixels[idx] + 0.587f * rgbaPixels[idx + 1] +
                            0.114f * rgbaPixels[idx + 2];
                        sum += lum;
                        count++;
                    }
                }
                cellLum[cy * m_config.gridSize + cx] = count > 0 ? sum / count : 0.0f;
            }
        }

        // Saliency = deviation from mean luminance
        float meanLum = 0.0f;
        for (float l : cellLum) meanLum += l;
        meanLum /= cellLum.size();

        float bestSaliency = 0.0f;
        uint32_t bestIdx = 0;
        for (uint32_t i = 0; i < cellLum.size(); ++i) {
            float sal = std::abs(cellLum[i] - meanLum) / 255.0f;
            if (sal > m_config.threshold) result.salientCellCount++;
            result.overallSaliency += sal;
            if (sal > bestSaliency) { bestSaliency = sal; bestIdx = i; }
        }
        result.overallSaliency /= cellLum.size();

        result.primaryRegion.left = (bestIdx % m_config.gridSize) * cellW;
        result.primaryRegion.top = (bestIdx / m_config.gridSize) * cellH;
        result.primaryRegion.width = cellW;
        result.primaryRegion.height = cellH;
        result.primaryRegion.avgSaliency = bestSaliency;
        return result;
    }

private:
    SaliencyMapConfig m_config;
};

} // namespace Engine
} // namespace ExplorerLens
