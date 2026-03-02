// ColorPaletteExtractor.h — Dominant Color Palette Extraction
// Copyright (c) 2026 ExplorerLens Project
//
// Extracts dominant color palette from images using a median-cut
// quantization algorithm. Outputs 3-8 dominant colors sorted by
// coverage area. Used for color-coded file previews and search indexing.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct ColorRGB {
    uint8_t r = 0, g = 0, b = 0;
    float coverage = 0.0f; // Proportion of pixels (0-1)

    uint32_t ToARGB() const {
        return (0xFF << 24) | (r << 16) | (g << 8) | b;
    }
};

struct Palette {
    std::vector<ColorRGB> colors; // Sorted by coverage (descending)
    uint32_t pixelCount = 0;
    float colorfulness = 0.0f;    // 0-1: How colorful the image is
    bool isMonochrome = false;
};

struct PaletteStats {
    uint32_t imagesProcessed = 0;
    uint32_t monochromeCount = 0;
    double   avgColorfulness = 0.0;
};

class ColorPaletteExtractor {
public:
    ColorPaletteExtractor() = default;
    ~ColorPaletteExtractor() = default;

    static const wchar_t* GetName() { return L"ColorPaletteExtractor"; }

    /// Extract dominant palette using median-cut quantization.
    Palette Extract(const uint8_t* bgra, uint32_t width, uint32_t height,
        uint32_t maxColors = 5) const {
        Palette result;
        if (!bgra || width == 0 || height == 0) return result;

        uint32_t pixelCount = width * height;
        result.pixelCount = pixelCount;

        // Subsample for performance (max 4096 pixels)
        uint32_t step = std::max(1u, pixelCount / 4096);
        std::vector<std::array<uint8_t, 3>> samples;
        samples.reserve(pixelCount / step);
        for (uint32_t i = 0; i < pixelCount; i += step) {
            samples.push_back({ bgra[i * 4 + 2], bgra[i * 4 + 1], bgra[i * 4] }); // BGR->RGB
        }

        // Median-cut: recursively split along axis with largest range
        std::vector<std::vector<std::array<uint8_t, 3>>> buckets;
        buckets.push_back(samples);

        while (buckets.size() < maxColors) {
            // Find bucket with largest range to split
            size_t bestBucket = 0;
            int bestRange = 0;
            int bestAxis = 0;

            for (size_t b = 0; b < buckets.size(); ++b) {
                for (int ch = 0; ch < 3; ++ch) {
                    uint8_t lo = 255, hi = 0;
                    for (const auto& px : buckets[b]) {
                        lo = std::min(lo, px[ch]);
                        hi = std::max(hi, px[ch]);
                    }
                    int range = hi - lo;
                    if (range > bestRange) {
                        bestRange = range;
                        bestBucket = b;
                        bestAxis = ch;
                    }
                }
            }
            if (bestRange == 0) break;

            // Sort along best axis and split at median
            auto& bucket = buckets[bestBucket];
            std::sort(bucket.begin(), bucket.end(),
                [bestAxis](const auto& a, const auto& b) { return a[bestAxis] < b[bestAxis]; });
            size_t mid = bucket.size() / 2;
            std::vector<std::array<uint8_t, 3>> secondHalf(bucket.begin() + mid, bucket.end());
            bucket.resize(mid);
            buckets.push_back(std::move(secondHalf));
        }

        // Average each bucket to get representative color
        for (const auto& bucket : buckets) {
            if (bucket.empty()) continue;
            uint32_t rSum = 0, gSum = 0, bSum = 0;
            for (const auto& px : bucket) {
                rSum += px[0]; gSum += px[1]; bSum += px[2];
            }
            ColorRGB c;
            auto n = static_cast<uint32_t>(bucket.size());
            c.r = static_cast<uint8_t>(rSum / n);
            c.g = static_cast<uint8_t>(gSum / n);
            c.b = static_cast<uint8_t>(bSum / n);
            c.coverage = static_cast<float>(bucket.size()) / samples.size();
            result.colors.push_back(c);
        }

        // Sort by coverage
        std::sort(result.colors.begin(), result.colors.end(),
            [](const ColorRGB& a, const ColorRGB& b) { return a.coverage > b.coverage; });

        // Colorfulness and monochrome detection
        result.colorfulness = ComputeColorfulness(bgra, pixelCount, step);
        result.isMonochrome = result.colorfulness < 0.05f;

        m_stats.imagesProcessed++;
        if (result.isMonochrome) m_stats.monochromeCount++;

        return result;
    }

    PaletteStats GetStats() const { return m_stats; }

private:
    float ComputeColorfulness(const uint8_t* bgra, uint32_t pixelCount, uint32_t step) const {
        double avgSat = 0.0;
        uint32_t count = 0;
        for (uint32_t i = 0; i < pixelCount; i += step) {
            uint8_t r = bgra[i * 4 + 2], g = bgra[i * 4 + 1], b = bgra[i * 4];
            uint8_t maxC = std::max({ r, g, b });
            uint8_t minC = std::min({ r, g, b });
            if (maxC > 0) avgSat += static_cast<double>(maxC - minC) / maxC;
            count++;
        }
        return count > 0 ? static_cast<float>(avgSat / count) : 0.0f;
    }

    mutable PaletteStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
