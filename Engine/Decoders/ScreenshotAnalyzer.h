// ScreenshotAnalyzer.h — Screenshot Quality Analysis
// Copyright (c) 2026 ExplorerLens Project
//
// Analyzes screenshot images to detect UI elements, text regions,
// and optimize thumbnail generation for screen capture content.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ScreenshotType : uint8_t {
    Unknown,
    FullScreen,
    WindowCapture,
    RegionCapture,
    ScrollingCapture,
    GameCapture
};

struct UIRegion
{
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    bool isTextRegion = false;
    bool isToolbar = false;
    bool isContent = false;
    float importance = 0.0f;
};

struct ScreenshotAnalysis
{
    ScreenshotType type = ScreenshotType::Unknown;
    float textDensity = 0.0f;
    float contentRegionCoverage = 0.0f;
    std::vector<UIRegion> regions;
    bool hasTaskbar = false;
    bool hasTitleBar = false;
    bool isDarkMode = false;
    float uniformityScore = 0.0f;
};

struct ScreenshotThumbnailHint
{
    float cropX = 0.0f;
    float cropY = 0.0f;
    float cropWidth = 1.0f;
    float cropHeight = 1.0f;
    bool shouldSharpen = false;
    float contrastBoost = 0.0f;
};

class ScreenshotAnalyzer
{
  public:
    ScreenshotAnalyzer() = default;

    ScreenshotAnalysis Analyze(const uint8_t* pixels, uint32_t width, uint32_t height)
    {
        ScreenshotAnalysis analysis;
        if (!pixels || width == 0 || height == 0)
            return analysis;

        // Detect screenshot type from edge patterns
        uint32_t topEdgeVariance = CalculateEdgeVariance(pixels, width, true);
        uint32_t bottomEdgeVariance = CalculateEdgeVariance(pixels + (height - 1) * width * 3, width, true);

        if (topEdgeVariance < 10 && bottomEdgeVariance < 10) {
            analysis.type = ScreenshotType::FullScreen;
        } else if (topEdgeVariance < 10) {
            analysis.type = ScreenshotType::WindowCapture;
            analysis.hasTitleBar = true;
        } else {
            analysis.type = ScreenshotType::RegionCapture;
        }

        // Detect dark mode
        uint64_t totalBrightness = 0;
        uint32_t pixelCount = width * height;
        for (uint32_t i = 0; i < pixelCount && i < 10000; i++) {
            totalBrightness += pixels[i * 3] + pixels[i * 3 + 1] + pixels[i * 3 + 2];
        }
        uint32_t sampledCount = pixelCount < 10000 ? pixelCount : 10000;
        float avgBrightness = static_cast<float>(totalBrightness) / (sampledCount * 3);
        analysis.isDarkMode = avgBrightness < 80.0f;

        m_totalAnalyzed++;
        return analysis;
    }

    ScreenshotThumbnailHint GetThumbnailHint(const ScreenshotAnalysis& analysis) const
    {
        ScreenshotThumbnailHint hint;
        if (analysis.hasTitleBar) {
            hint.cropY = 0.05f;
            hint.cropHeight = 0.9f;
        }
        if (analysis.type == ScreenshotType::FullScreen) {
            hint.shouldSharpen = true;
        }
        return hint;
    }

    uint64_t GetTotalAnalyzed() const
    {
        return m_totalAnalyzed;
    }

  private:
    uint32_t CalculateEdgeVariance(const uint8_t* row, uint32_t width, bool isHorizontal) const
    {
        if (!row || width < 2)
            return 0;
        uint32_t variance = 0;
        uint32_t samples = width < 100 ? width : 100;
        for (uint32_t i = 1; i < samples; i++) {
            uint32_t idx = isHorizontal ? i * 3 : i;
            int diff = static_cast<int>(row[idx]) - static_cast<int>(row[idx - 3]);
            variance += static_cast<uint32_t>(diff > 0 ? diff : -diff);
        }
        return variance / samples;
    }

    uint64_t m_totalAnalyzed = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
