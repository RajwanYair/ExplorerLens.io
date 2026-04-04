// ContentAwareCompositor.h — Smart Cropping and Composition
// Copyright (c) 2026 ExplorerLens Project
//
// Smart cropping/composition engine. Detects subject region using edge density
// and auto-crops to the most interesting region for thumbnails.
//
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct CropRegion
{
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    double interestScore = 0.0;
};

enum class CompositorAspectRatio : uint8_t {
    Square,
    FourByThree,
    SixteenByNine,
    FreeForm
};

class ContentAwareCompositor
{
  public:
    static ContentAwareCompositor& Instance()
    {
        static ContentAwareCompositor instance;
        return instance;
    }

    inline CropRegion FindInterestRegion(const uint8_t* pixelData, uint32_t width, uint32_t height, uint32_t channels,
                                         CompositorAspectRatio aspect = CompositorAspectRatio::Square) const
    {
        CropRegion best;
        if (!pixelData || width == 0 || height == 0 || channels == 0)
            return best;

        auto edgeMap = ComputeEdgeDensityMap(pixelData, width, height, channels);
        auto integralMap = ComputeIntegralImage(edgeMap, width, height);

        double aspectW = 1.0, aspectH = 1.0;
        switch (aspect) {
            case CompositorAspectRatio::Square:
                aspectW = 1.0;
                aspectH = 1.0;
                break;
            case CompositorAspectRatio::FourByThree:
                aspectW = 4.0;
                aspectH = 3.0;
                break;
            case CompositorAspectRatio::SixteenByNine:
                aspectW = 16.0;
                aspectH = 9.0;
                break;
            case CompositorAspectRatio::FreeForm:
                aspectW = 1.0;
                aspectH = 1.0;
                break;
        }

        double bestScore = -1.0;
        uint32_t minDim = (std::min)(width, height);
        uint32_t stepSize = (std::max)(1u, minDim / 20);

        for (double scale = 0.4; scale <= 0.9; scale += 0.1) {
            uint32_t cropW = static_cast<uint32_t>(width * scale);
            uint32_t cropH = static_cast<uint32_t>(cropW * aspectH / aspectW);
            if (cropH > height) {
                cropH = static_cast<uint32_t>(height * scale);
                cropW = static_cast<uint32_t>(cropH * aspectW / aspectH);
            }
            cropW = (std::min)(cropW, width);
            cropH = (std::min)(cropH, height);

            for (uint32_t y = 0; y + cropH <= height; y += stepSize) {
                for (uint32_t x = 0; x + cropW <= width; x += stepSize) {
                    double score = SumRegion(integralMap, width, x, y, cropW, cropH);
                    double area = static_cast<double>(cropW) * cropH;
                    double normalized = score / area;

                    double centerX = (x + cropW / 2.0) / width;
                    double centerY = (y + cropH / 2.0) / height;
                    double centerBonus = 1.0 - 0.3 * (std::abs(centerX - 0.5) + std::abs(centerY - 0.5));
                    normalized *= centerBonus;

                    if (normalized > bestScore) {
                        bestScore = normalized;
                        best.x = x;
                        best.y = y;
                        best.width = cropW;
                        best.height = cropH;
                        best.interestScore = normalized;
                    }
                }
            }
        }
        return best;
    }

    inline std::vector<double> ComputeEdgeDensityMap(const uint8_t* pixelData, uint32_t width, uint32_t height,
                                                     uint32_t channels) const
    {
        std::vector<double> edgeMap(static_cast<size_t>(width) * height, 0.0);
        if (!pixelData || width < 3 || height < 3)
            return edgeMap;

        for (uint32_t y = 1; y + 1 < height; ++y) {
            for (uint32_t x = 1; x + 1 < width; ++x) {
                double gx = GetLuminance(pixelData, width, x + 1, y, channels)
                            - GetLuminance(pixelData, width, x - 1, y, channels);
                double gy = GetLuminance(pixelData, width, x, y + 1, channels)
                            - GetLuminance(pixelData, width, x, y - 1, channels);
                edgeMap[static_cast<size_t>(y) * width + x] = std::sqrt(gx * gx + gy * gy);
            }
        }
        return edgeMap;
    }

    inline double ComputeImageComplexity(const uint8_t* pixelData, uint32_t width, uint32_t height,
                                         uint32_t channels) const
    {
        auto edgeMap = ComputeEdgeDensityMap(pixelData, width, height, channels);
        if (edgeMap.empty())
            return 0.0;
        double total = 0.0;
        for (auto v : edgeMap)
            total += v;
        return total / static_cast<double>(edgeMap.size());
    }

  private:
    ContentAwareCompositor() = default;

    inline double GetLuminance(const uint8_t* data, uint32_t width, uint32_t x, uint32_t y, uint32_t channels) const
    {
        size_t idx = (static_cast<size_t>(y) * width + x) * channels;
        if (channels >= 3) {
            return 0.299 * data[idx] + 0.587 * data[idx + 1] + 0.114 * data[idx + 2];
        }
        return data[idx];
    }

    inline std::vector<double> ComputeIntegralImage(const std::vector<double>& map, uint32_t width,
                                                    uint32_t height) const
    {
        std::vector<double> integral(static_cast<size_t>(width) * height, 0.0);
        for (uint32_t y = 0; y < height; ++y) {
            double rowSum = 0.0;
            for (uint32_t x = 0; x < width; ++x) {
                size_t idx = static_cast<size_t>(y) * width + x;
                rowSum += map[idx];
                integral[idx] = rowSum + (y > 0 ? integral[(static_cast<size_t>(y) - 1) * width + x] : 0.0);
            }
        }
        return integral;
    }

    inline double SumRegion(const std::vector<double>& integral, uint32_t stride, uint32_t x, uint32_t y, uint32_t w,
                            uint32_t h) const
    {
        uint32_t x2 = x + w - 1;
        uint32_t y2 = y + h - 1;
        double total = integral[static_cast<size_t>(y2) * stride + x2];
        if (x > 0)
            total -= integral[static_cast<size_t>(y2) * stride + (x - 1)];
        if (y > 0)
            total -= integral[(static_cast<size_t>(y) - 1) * stride + x2];
        if (x > 0 && y > 0)
            total += integral[(static_cast<size_t>(y) - 1) * stride + (x - 1)];
        return total;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
