// DominantColorExtractor.h — Dominant Color Extraction from Pixel Data
// Copyright (c) 2026 ExplorerLens Project
//
// Extracts dominant colours from thumbnail pixel buffers using a simplified
// k-means clustering algorithm.  All operations are pure math on raw ARGB
// pixel arrays — no image codec dependencies.
//
#pragma once

#include <windows.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// ARGB colour packed as 0xAARRGGBB.
struct RGBColor
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;

    uint32_t ToARGB() const
    {
        return (0xFFu << 24) | (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8)
               | static_cast<uint32_t>(b);
    }

    static RGBColor FromARGB(uint32_t argb)
    {
        return {static_cast<uint8_t>((argb >> 16) & 0xFF), static_cast<uint8_t>((argb >> 8) & 0xFF),
                static_cast<uint8_t>(argb & 0xFF)};
    }

    bool operator==(const RGBColor& o) const
    {
        return r == o.r && g == o.g && b == o.b;
    }
};

/// A dominant colour together with its weight (pixel fraction).
struct DominantColor
{
    RGBColor color{};
    float weight = 0.0f;  // fraction of total pixels [0, 1]
};

class DominantColorExtractor
{
  public:
    DominantColorExtractor() = default;

    // ── Core extraction ──────────────────────────────────────────────

    /// Extract the top `count` dominant colours from an ARGB pixel buffer
    /// using simplified k-means.  `pixels` is row-major, one uint32_t per
    /// pixel (0xAARRGGBB).
    std::vector<DominantColor> ExtractTopColors(const uint32_t* pixels, uint32_t w, uint32_t h, uint32_t count) const
    {
        if (!pixels || w == 0 || h == 0 || count == 0)
            return {};

        const size_t totalPixels = static_cast<size_t>(w) * h;
        count = (std::min)(count, 16u);  // cap

        // Sub-sample for performance (max ~4096 samples)
        std::vector<RGBColor> samples = SubSample(pixels, totalPixels, 4096);
        if (samples.empty())
            return {};

        // Initialise centroids with evenly-spaced samples
        std::vector<RGBColor> centroids(count);
        for (uint32_t i = 0; i < count; ++i) {
            size_t idx = i * samples.size() / count;
            centroids[i] = samples[idx];
        }

        // K-means iterations
        std::vector<uint32_t> assignments(samples.size(), 0);
        for (int iter = 0; iter < m_maxIterations; ++iter) {
            bool changed = false;

            // Assign each sample to nearest centroid
            for (size_t s = 0; s < samples.size(); ++s) {
                uint32_t best = 0;
                float bestDist = ColorDistanceSq(samples[s], centroids[0]);
                for (uint32_t c = 1; c < count; ++c) {
                    float d = ColorDistanceSq(samples[s], centroids[c]);
                    if (d < bestDist) {
                        bestDist = d;
                        best = c;
                    }
                }
                if (assignments[s] != best) {
                    assignments[s] = best;
                    changed = true;
                }
            }

            if (!changed)
                break;

            // Recompute centroids
            std::vector<double> sumR(count, 0), sumG(count, 0), sumB(count, 0);
            std::vector<uint32_t> clusterSize(count, 0);
            for (size_t s = 0; s < samples.size(); ++s) {
                uint32_t c = assignments[s];
                sumR[c] += samples[s].r;
                sumG[c] += samples[s].g;
                sumB[c] += samples[s].b;
                ++clusterSize[c];
            }
            for (uint32_t c = 0; c < count; ++c) {
                if (clusterSize[c] > 0) {
                    centroids[c].r = static_cast<uint8_t>(sumR[c] / clusterSize[c]);
                    centroids[c].g = static_cast<uint8_t>(sumG[c] / clusterSize[c]);
                    centroids[c].b = static_cast<uint8_t>(sumB[c] / clusterSize[c]);
                }
            }
        }

        // Build result sorted by cluster weight descending
        std::vector<uint32_t> clusterSize(count, 0);
        for (uint32_t a : assignments)
            ++clusterSize[a];

        std::vector<DominantColor> result(count);
        float invTotal = 1.0f / static_cast<float>(samples.size());
        for (uint32_t c = 0; c < count; ++c) {
            result[c].color = centroids[c];
            result[c].weight = static_cast<float>(clusterSize[c]) * invTotal;
        }

        std::sort(result.begin(), result.end(),
                  [](const DominantColor& a, const DominantColor& b) { return a.weight > b.weight; });
        return result;
    }

    // ── Single-pass helpers ──────────────────────────────────────────

    /// Average colour across all pixels.
    RGBColor GetAverageColor(const uint32_t* pixels, uint32_t w, uint32_t h) const
    {
        if (!pixels || w == 0 || h == 0)
            return {};
        const size_t n = static_cast<size_t>(w) * h;
        double sr = 0, sg = 0, sb = 0;
        for (size_t i = 0; i < n; ++i) {
            RGBColor c = RGBColor::FromARGB(pixels[i]);
            sr += c.r;
            sg += c.g;
            sb += c.b;
        }
        return {static_cast<uint8_t>(sr / n), static_cast<uint8_t>(sg / n), static_cast<uint8_t>(sb / n)};
    }

    /// Euclidean distance in RGB space between two colours.
    float GetColorDistance(RGBColor c1, RGBColor c2) const
    {
        return std::sqrt(ColorDistanceSq(c1, c2));
    }

    /// Quantise a colour to the given number of levels per channel.
    /// E.g. levels=4 gives 64 possible colours (4^3).
    RGBColor QuantizeColor(RGBColor color, uint32_t levels) const
    {
        if (levels < 2)
            return color;
        float step = 255.0f / static_cast<float>(levels - 1);
        auto q = [step](uint8_t v) -> uint8_t {
            float fv = static_cast<float>(v);
            int bin = static_cast<int>(fv / step + 0.5f);
            float qv = static_cast<float>(bin) * step;
            return static_cast<uint8_t>((std::max)(0.0f, (std::min)(255.0f, qv)));
        };
        return {q(color.r), q(color.g), q(color.b)};
    }

    /// Compute perceived luminance (BT.601) for a colour.
    float GetLuminance(RGBColor color) const
    {
        return 0.299f * color.r + 0.587f * color.g + 0.114f * color.b;
    }

    /// Check if a colour is approximately grayscale (R ≈ G ≈ B).
    bool IsGrayscale(RGBColor color, uint8_t tolerance = 12) const
    {
        int maxC = (std::max)({static_cast<int>(color.r), static_cast<int>(color.g), static_cast<int>(color.b)});
        int minC = (std::min)({static_cast<int>(color.r), static_cast<int>(color.g), static_cast<int>(color.b)});
        return (maxC - minC) <= tolerance;
    }

    // ── Configuration ────────────────────────────────────────────────

    void SetMaxIterations(int iters)
    {
        m_maxIterations = (std::max)(1, iters);
    }
    int GetMaxIterations() const
    {
        return m_maxIterations;
    }

  private:
    int m_maxIterations = 20;

    /// Squared Euclidean distance in RGB space.
    static float ColorDistanceSq(RGBColor a, RGBColor b)
    {
        float dr = static_cast<float>(a.r) - static_cast<float>(b.r);
        float dg = static_cast<float>(a.g) - static_cast<float>(b.g);
        float db = static_cast<float>(a.b) - static_cast<float>(b.b);
        return dr * dr + dg * dg + db * db;
    }

    /// Deterministic sub-sampling: pick evenly-spaced pixels.
    static std::vector<RGBColor> SubSample(const uint32_t* pixels, size_t totalPixels, size_t maxSamples)
    {
        size_t n = (std::min)(totalPixels, maxSamples);
        std::vector<RGBColor> out(n);
        for (size_t i = 0; i < n; ++i) {
            size_t idx = i * totalPixels / n;
            out[i] = RGBColor::FromARGB(pixels[idx]);
        }
        return out;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
