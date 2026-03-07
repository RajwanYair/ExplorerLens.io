// ColorPaletteGenerator.h — Dominant Color Palette Extraction
// Copyright (c) 2026 ExplorerLens Project
//
// Extracts dominant colors from decoded thumbnails for use in cache keys,
// visual search, and adaptive UI theming.
//
#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>
#include <cmath>

namespace ExplorerLens {
namespace Engine {

struct PaletteColorRGB {
    uint8_t r = 0, g = 0, b = 0;

    uint32_t ToPackedRGB() const { return (r << 16) | (g << 8) | b; }

    double DistanceTo(const PaletteColorRGB& other) const {
        double dr = static_cast<double>(r) - other.r;
        double dg = static_cast<double>(g) - other.g;
        double db = static_cast<double>(b) - other.b;
        return std::sqrt(dr * dr + dg * dg + db * db);
    }
};

struct PaletteEntry {
    PaletteColorRGB color;
    float proportion = 0.0f; // 0..1
    uint32_t pixelCount = 0;
};

struct PaletteConfig {
    uint32_t maxColors = 5;
    uint32_t sampleStride = 4;    // Sample every Nth pixel
    double mergeDistance = 30.0;   // Colors closer than this merge
    uint32_t minPixelCount = 10;
};

struct PaletteResult {
    bool valid = false;
    std::vector<PaletteEntry> colors;
    PaletteColorRGB averageColor;
    bool isDark = false;
    bool isMonochrome = false;
};

class ColorPaletteGenerator {
public:
    void Configure(const PaletteConfig& config) { m_config = config; }

    PaletteResult Generate(const uint8_t* rgbaPixels, uint32_t width,
        uint32_t height) const {
        PaletteResult result;
        if (!rgbaPixels || width == 0 || height == 0) return result;

        // Quantize to reduced color space and count
        struct Bucket {
            uint64_t sumR = 0, sumG = 0, sumB = 0;
            uint32_t count = 0;
        };
        // Simple 4x4x4 = 64 bucket quantization
        std::vector<Bucket> buckets(64);

        uint32_t totalSampled = 0;
        uint64_t globalR = 0, globalG = 0, globalB = 0;

        for (uint32_t y = 0; y < height; y += m_config.sampleStride) {
            for (uint32_t x = 0; x < width; x += m_config.sampleStride) {
                size_t idx = (static_cast<size_t>(y) * width + x) * 4;
                uint8_t r = rgbaPixels[idx], g = rgbaPixels[idx + 1], b = rgbaPixels[idx + 2];
                uint32_t bi = (r / 64) * 16 + (g / 64) * 4 + (b / 64);
                if (bi >= 64) bi = 63;
                buckets[bi].sumR += r;
                buckets[bi].sumG += g;
                buckets[bi].sumB += b;
                buckets[bi].count++;
                globalR += r; globalG += g; globalB += b;
                totalSampled++;
            }
        }
        if (totalSampled == 0) return result;

        result.averageColor.r = static_cast<uint8_t>(globalR / totalSampled);
        result.averageColor.g = static_cast<uint8_t>(globalG / totalSampled);
        result.averageColor.b = static_cast<uint8_t>(globalB / totalSampled);

        // Sort buckets by count
        std::vector<size_t> indices(64);
        for (size_t i = 0; i < 64; ++i) indices[i] = i;
        std::sort(indices.begin(), indices.end(),
            [&](size_t a, size_t b) { return buckets[a].count > buckets[b].count; });

        for (size_t i = 0; i < 64 && result.colors.size() < m_config.maxColors; ++i) {
            const auto& bk = buckets[indices[i]];
            if (bk.count < m_config.minPixelCount) continue;
            PaletteEntry entry;
            entry.color.r = static_cast<uint8_t>(bk.sumR / bk.count);
            entry.color.g = static_cast<uint8_t>(bk.sumG / bk.count);
            entry.color.b = static_cast<uint8_t>(bk.sumB / bk.count);
            entry.pixelCount = bk.count;
            entry.proportion = static_cast<float>(bk.count) / totalSampled;
            result.colors.push_back(entry);
        }

        double avgLum = 0.299 * result.averageColor.r + 0.587 * result.averageColor.g +
            0.114 * result.averageColor.b;
        result.isDark = avgLum < 80.0;
        result.isMonochrome = result.colors.size() <= 2;
        result.valid = true;
        return result;
    }

private:
    PaletteConfig m_config;
};

} // namespace Engine
} // namespace ExplorerLens
