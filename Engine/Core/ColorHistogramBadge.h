// ColorHistogramBadge.h — Color Histogram Analysis for Thumbnails
// Copyright (c) 2026 ExplorerLens Project
//
// Generates color histogram analysis from pixel data, computing dominant bins,
// contrast ratios, monochromaticity detection, and colorfulness metrics.
//
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// Histogram bin holding normalized count and representative luminance
struct ColorHistBin
{
    uint32_t count = 0;
    double normalized = 0.0;   // count / totalPixels
    uint8_t luminanceMid = 0;  // midpoint luminance of this bin
};

// Result of a full histogram computation
struct ColorHistResult
{
    std::vector<ColorHistBin> bins;
    uint32_t totalPixels = 0;
    double meanLuminance = 0.0;
    double stdDevLuminance = 0.0;
    bool valid = false;
};

class ColorHistogramBadge
{
  public:
    // Compute a luminance histogram from BGRA pixel data.
    // binCount is clamped to [2, 256].
    static ColorHistResult ComputeHistogram(const uint8_t* pixels, uint32_t width, uint32_t height,
                                            uint32_t binCount) noexcept
    {
        ColorHistResult result{};
        if (!pixels || width == 0 || height == 0)
            return result;

        binCount = (std::max)(2u, (std::min)(binCount, 256u));
        result.bins.resize(binCount);
        result.totalPixels = width * height;

        const double binWidth = 256.0 / static_cast<double>(binCount);

        // Initialize midpoints
        for (uint32_t i = 0; i < binCount; ++i) {
            result.bins[i].luminanceMid = static_cast<uint8_t>((std::min)(255.0, (i + 0.5) * binWidth));
        }

        // Accumulate luminance values (BT.601 weights on BGRA layout)
        double sumLum = 0.0;
        double sumLum2 = 0.0;

        for (uint32_t i = 0; i < result.totalPixels; ++i) {
            const uint32_t offset = i * 4;
            const uint8_t b = pixels[offset + 0];
            const uint8_t g = pixels[offset + 1];
            const uint8_t r = pixels[offset + 2];
            // alpha at offset+3 is ignored

            const double lum = 0.299 * r + 0.587 * g + 0.114 * b;
            sumLum += lum;
            sumLum2 += lum * lum;

            uint32_t binIdx = static_cast<uint32_t>(lum / binWidth);
            binIdx = (std::min)(binIdx, binCount - 1);
            result.bins[binIdx].count++;
        }

        // Normalize
        const double total = static_cast<double>(result.totalPixels);
        for (auto& bin : result.bins)
            bin.normalized = static_cast<double>(bin.count) / total;

        result.meanLuminance = sumLum / total;
        const double variance = (sumLum2 / total) - (result.meanLuminance * result.meanLuminance);
        result.stdDevLuminance = (variance > 0.0) ? std::sqrt(variance) : 0.0;
        result.valid = true;
        return result;
    }

    // Return the index of the bin with the highest count.
    static uint32_t GetDominantBin(const ColorHistResult& hist) noexcept
    {
        if (!hist.valid || hist.bins.empty())
            return 0;
        uint32_t best = 0;
        uint32_t bestCount = 0;
        for (uint32_t i = 0; i < static_cast<uint32_t>(hist.bins.size()); ++i) {
            if (hist.bins[i].count > bestCount) {
                bestCount = hist.bins[i].count;
                best = i;
            }
        }
        return best;
    }

    // Contrast ratio based on min/max occupied luminance bins,
    // following simplified (L1+0.05)/(L2+0.05) formula.
    static double GetContrastRatio(const ColorHistResult& hist) noexcept
    {
        if (!hist.valid || hist.bins.empty())
            return 1.0;

        double minL = 256.0;
        double maxL = -1.0;
        for (const auto& bin : hist.bins) {
            if (bin.count > 0) {
                const double mid = static_cast<double>(bin.luminanceMid) / 255.0;
                if (mid < minL)
                    minL = mid;
                if (mid > maxL)
                    maxL = mid;
            }
        }
        if (maxL < 0.0)
            return 1.0;

        const double lighter = (std::max)(minL, maxL) + 0.05;
        const double darker = (std::min)(minL, maxL) + 0.05;
        return lighter / darker;
    }

    // Returns true if the standard deviation of luminance is below threshold.
    // threshold is in [0, 128] luminance units.
    static bool IsMonochrome(const ColorHistResult& hist, double threshold = 10.0) noexcept
    {
        if (!hist.valid)
            return false;
        return hist.stdDevLuminance < (std::max)(0.0, threshold);
    }

    // Colorfulness metric (Hasler & Suesstrunk simplified) on BGRA data.
    // Returns 0.0 for perfectly grey, higher = more colorful.
    static double GetColorfulness(const uint8_t* pixels, uint32_t width, uint32_t height) noexcept
    {
        if (!pixels || width == 0 || height == 0)
            return 0.0;

        const uint32_t total = width * height;
        double sumRG = 0.0, sumRG2 = 0.0;
        double sumYB = 0.0, sumYB2 = 0.0;

        for (uint32_t i = 0; i < total; ++i) {
            const uint32_t off = i * 4;
            const double b = pixels[off + 0];
            const double g = pixels[off + 1];
            const double r = pixels[off + 2];

            const double rg = r - g;
            const double yb = 0.5 * (r + g) - b;

            sumRG += rg;
            sumRG2 += rg * rg;
            sumYB += yb;
            sumYB2 += yb * yb;
        }

        const double n = static_cast<double>(total);
        const double meanRG = sumRG / n;
        const double meanYB = sumYB / n;
        const double stdRG = std::sqrt((std::max)(0.0, sumRG2 / n - meanRG * meanRG));
        const double stdYB = std::sqrt((std::max)(0.0, sumYB2 / n - meanYB * meanYB));

        const double stdRoot = std::sqrt(stdRG * stdRG + stdYB * stdYB);
        const double meanRoot = std::sqrt(meanRG * meanRG + meanYB * meanYB);

        return stdRoot + 0.3 * meanRoot;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
