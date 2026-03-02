// ThumbnailRelevanceScorer.h — Thumbnail Relevance Scoring Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Assigns relevance scores to generated thumbnails based on visual
// information content: edge density, color variance, spatial entropy,
// and contrast ratio. Low-scoring thumbnails trigger re-decode with
// alternative strategies (different frame, different crop, etc.)

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct RelevanceFactors {
    double edgeDensity = 0.0;        // 0-1: Proportion of edge pixels
    double colorVariance = 0.0;      // 0-1: Color channel variance
    double spatialEntropy = 0.0;     // 0-8+ bits: Shannon entropy
    double contrastRatio = 0.0;      // 0-1: Min/max luminance ratio
    double informationDensity = 0.0; // Combined score
};

struct RelevanceScore {
    double     score = 0.0;      // 0-100
    bool       isUseful = true;  // Score > threshold
    bool       needsRetry = false;
    RelevanceFactors factors;
};

struct RelevanceStats {
    uint32_t thumbnailsScored = 0;
    uint32_t retryTriggered = 0;
    double   avgScore = 0.0;
    double   minScore = 100.0;
};

class ThumbnailRelevanceScorer {
public:
    ThumbnailRelevanceScorer() = default;
    ~ThumbnailRelevanceScorer() = default;

    static const wchar_t* GetName() { return L"ThumbnailRelevanceScorer"; }

    void SetMinScoreThreshold(double threshold) { m_minThreshold = threshold; }

    /// Compute edge density using simple Sobel-like gradient magnitude.
    double ComputeEdgeDensity(const uint8_t* gray, uint32_t w, uint32_t h) const {
        if (!gray || w < 3 || h < 3) return 0.0;
        uint32_t edgePixels = 0;
        for (uint32_t y = 1; y < h - 1; ++y) {
            for (uint32_t x = 1; x < w - 1; ++x) {
                int gx = -gray[(y - 1) * w + (x - 1)] + gray[(y - 1) * w + (x + 1)]
                    - 2 * gray[y * w + (x - 1)] + 2 * gray[y * w + (x + 1)]
                    - gray[(y + 1) * w + (x - 1)] + gray[(y + 1) * w + (x + 1)];
                int gy = -gray[(y - 1) * w + (x - 1)] - 2 * gray[(y - 1) * w + x] - gray[(y - 1) * w + (x + 1)]
                    + gray[(y + 1) * w + (x - 1)] + 2 * gray[(y + 1) * w + x] + gray[(y + 1) * w + (x + 1)];
                int mag = std::abs(gx) + std::abs(gy);
                if (mag > 40) edgePixels++;
            }
        }
        return static_cast<double>(edgePixels) / ((w - 2) * (h - 2));
    }

    /// Compute spatial entropy from grayscale histogram.
    double ComputeEntropy(const uint8_t* gray, uint32_t pixelCount) const {
        if (!gray || pixelCount == 0) return 0.0;
        uint32_t hist[256] = {};
        for (uint32_t i = 0; i < pixelCount; ++i) hist[gray[i]]++;
        double entropy = 0.0;
        for (int i = 0; i < 256; ++i) {
            if (hist[i] == 0) continue;
            double p = static_cast<double>(hist[i]) / pixelCount;
            entropy -= p * std::log2(p);
        }
        return entropy;
    }

    /// Score a thumbnail for visual relevance (0-100).
    RelevanceScore Score(const uint8_t* bgra, uint32_t width, uint32_t height) const {
        RelevanceScore result;
        if (!bgra || width == 0 || height == 0) {
            result.score = 0;
            result.isUseful = false;
            result.needsRetry = true;
            return result;
        }

        uint32_t pixelCount = width * height;
        // Convert to grayscale
        std::vector<uint8_t> gray(pixelCount);
        for (uint32_t i = 0; i < pixelCount; ++i) {
            gray[i] = static_cast<uint8_t>(
                bgra[i * 4] * 0.114 + bgra[i * 4 + 1] * 0.587 + bgra[i * 4 + 2] * 0.299);
        }

        result.factors.edgeDensity = ComputeEdgeDensity(gray.data(), width, height);
        result.factors.spatialEntropy = ComputeEntropy(gray.data(), pixelCount);

        // Compute contrast
        uint8_t minLum = 255, maxLum = 0;
        for (auto v : gray) { minLum = std::min(minLum, v); maxLum = std::max(maxLum, v); }
        result.factors.contrastRatio = maxLum > 0 ? static_cast<double>(maxLum - minLum) / 255.0 : 0.0;

        // Weighted combination
        result.score = std::clamp(
            result.factors.edgeDensity * 30.0 +
            (result.factors.spatialEntropy / 8.0) * 40.0 +
            result.factors.contrastRatio * 30.0,
            0.0, 100.0);

        result.isUseful = result.score >= m_minThreshold;
        result.needsRetry = !result.isUseful;

        m_stats.thumbnailsScored++;
        m_stats.avgScore = (m_stats.avgScore * (m_stats.thumbnailsScored - 1) + result.score)
            / m_stats.thumbnailsScored;
        if (result.score < m_stats.minScore) m_stats.minScore = result.score;
        if (result.needsRetry) m_stats.retryTriggered++;

        return result;
    }

    RelevanceStats GetStats() const { return m_stats; }

private:
    double m_minThreshold = 15.0;
    mutable RelevanceStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
