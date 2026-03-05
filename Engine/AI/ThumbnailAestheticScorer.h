// ThumbnailAestheticScorer.h — Aesthetic Quality Scoring for Frame Selection
// Copyright (c) 2026 ExplorerLens Project
//
// Lightweight scorer that evaluates thumbnail aesthetic appeal using
// compositional rules, edge density, color variance, and luminance
// distribution.  Used for optimal frame selection from multi-frame
// sources (GIF, video, TIFF).
//
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <mutex>
#include <numeric>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Aesthetic quality breakdown
struct AestheticBreakdown {
    float composition = 0.0f;  // Rule-of-thirds adherence (0-1)
    float sharpness = 0.0f;  // Edge density (0-1)
    float colorfulness = 0.0f;  // Color variance (0-1)
    float luminance = 0.0f;  // Balanced brightness (0-1)
    float contrast = 0.0f;  // Dynamic range utilization (0-1)
    float noise = 0.0f;  // Inverse noise level (0-1, higher = less noise)
};

/// Complete aesthetic score result
struct AestheticScore {
    float           overall = 0.0f;     // Weighted composite (0-100)
    AestheticBreakdown breakdown;
    uint32_t        width = 0;
    uint32_t        height = 0;
    double          computeMs = 0.0;
};

/// Scorer configuration
struct AestheticConfig {
    float wComposition = 0.25f;
    float wSharpness = 0.20f;
    float wColorfulness = 0.20f;
    float wLuminance = 0.15f;
    float wContrast = 0.10f;
    float wNoise = 0.10f;
};

/// Scoring statistics
struct AestheticStats {
    uint64_t imagesScored = 0;
    double   avgScore = 0.0;
    float    bestScore = 0.0f;
    float    worstScore = 100.0f;
};

/// Lightweight aesthetic quality scorer for thumbnails
class ThumbnailAestheticScorer {
public:
    explicit ThumbnailAestheticScorer(const AestheticConfig& cfg = {})
        : m_config(cfg) {
    }

    /// Score a grayscale thumbnail (8-bit per pixel)
    AestheticScore ScoreGrayscale(const uint8_t* pixels, uint32_t w, uint32_t h) {
        AestheticScore result;
        result.width = w; result.height = h;
        if (!pixels || w < 4 || h < 4) return result;

        auto start = std::chrono::high_resolution_clock::now();

        result.breakdown.composition = ScoreComposition(pixels, w, h);
        result.breakdown.sharpness = ScoreSharpness(pixels, w, h);
        result.breakdown.luminance = ScoreLuminance(pixels, w, h);
        result.breakdown.contrast = ScoreContrast(pixels, w, h);
        result.breakdown.noise = ScoreNoise(pixels, w, h);
        result.breakdown.colorfulness = 0.5f;  // Neutral for grayscale

        result.overall = ComputeOverall(result.breakdown) * 100.0f;

        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        result.computeMs = std::chrono::duration<double, std::milli>(elapsed).count();

        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.imagesScored++;
        m_stats.avgScore = m_stats.avgScore * 0.95 + result.overall * 0.05;
        m_stats.bestScore = (std::max)(m_stats.bestScore, result.overall);
        m_stats.worstScore = (std::min)(m_stats.worstScore, result.overall);
        return result;
    }

    /// Score ARGB thumbnail (32-bit per pixel)
    AestheticScore ScoreARGB(const uint32_t* pixels, uint32_t w, uint32_t h) {
        if (!pixels || w < 4 || h < 4) return {};

        // Extract grayscale and color channels
        std::vector<uint8_t> gray(w * h);
        float rVar = 0, gVar = 0, bVar = 0;
        float rMean = 0, gMean = 0, bMean = 0;
        float count = static_cast<float>(w * h);

        for (uint32_t i = 0; i < w * h; ++i) {
            uint32_t px = pixels[i];
            uint8_t r = static_cast<uint8_t>((px >> 16) & 0xFF);
            uint8_t g = static_cast<uint8_t>((px >> 8) & 0xFF);
            uint8_t b = static_cast<uint8_t>(px & 0xFF);
            gray[i] = static_cast<uint8_t>(0.299f * r + 0.587f * g + 0.114f * b);
            rMean += r; gMean += g; bMean += b;
        }
        rMean /= count; gMean /= count; bMean /= count;

        for (uint32_t i = 0; i < w * h; ++i) {
            uint32_t px = pixels[i];
            float r = static_cast<float>((px >> 16) & 0xFF) - rMean;
            float g = static_cast<float>((px >> 8) & 0xFF) - gMean;
            float b = static_cast<float>(px & 0xFF) - bMean;
            rVar += r * r; gVar += g * g; bVar += b * b;
        }
        rVar /= count; gVar /= count; bVar /= count;

        auto result = ScoreGrayscale(gray.data(), w, h);

        // Colorfulness: Hasler-Süsstrunk formula simplified
        float sigma = std::sqrt(rVar + gVar + bVar);
        result.breakdown.colorfulness = (std::min)(1.0f, sigma / 128.0f);
        result.overall = ComputeOverall(result.breakdown) * 100.0f;

        return result;
    }

    /// Pick the best frame from a set of candidates
    uint32_t SelectBestFrame(const std::vector<AestheticScore>& scores) const {
        if (scores.empty()) return 0;
        auto it = std::max_element(scores.begin(), scores.end(),
            [](const AestheticScore& a, const AestheticScore& b) {
                return a.overall < b.overall;
            });
        return static_cast<uint32_t>(std::distance(scores.begin(), it));
    }

    AestheticStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

private:
    float ComputeOverall(const AestheticBreakdown& bd) const {
        return bd.composition * m_config.wComposition +
            bd.sharpness * m_config.wSharpness +
            bd.colorfulness * m_config.wColorfulness +
            bd.luminance * m_config.wLuminance +
            bd.contrast * m_config.wContrast +
            bd.noise * m_config.wNoise;
    }

    /// Rule-of-thirds: measure edge energy at 1/3 and 2/3 lines
    static float ScoreComposition(const uint8_t* px, uint32_t w, uint32_t h) {
        float thirdEnergy = 0, totalEnergy = 0;
        uint32_t y3a = h / 3, y3b = 2 * h / 3;
        uint32_t x3a = w / 3, x3b = 2 * w / 3;

        for (uint32_t y = 1; y < h - 1; ++y) {
            for (uint32_t x = 1; x < w - 1; ++x) {
                int gx = px[(y)*w + (x + 1)] - px[(y)*w + (x - 1)];
                int gy = px[(y + 1) * w + x] - px[(y - 1) * w + x];
                float e = std::sqrt(static_cast<float>(gx * gx + gy * gy));
                totalEnergy += e;
                bool nearThird = (std::abs(static_cast<int>(y) - static_cast<int>(y3a)) < 3 ||
                    std::abs(static_cast<int>(y) - static_cast<int>(y3b)) < 3 ||
                    std::abs(static_cast<int>(x) - static_cast<int>(x3a)) < 3 ||
                    std::abs(static_cast<int>(x) - static_cast<int>(x3b)) < 3);
                if (nearThird) thirdEnergy += e;
            }
        }
        if (totalEnergy < 1.0f) return 0.5f;
        return (std::min)(1.0f, thirdEnergy / (totalEnergy * 0.02f));
    }

    /// Sharpness: average Laplacian magnitude
    static float ScoreSharpness(const uint8_t* px, uint32_t w, uint32_t h) {
        double sum = 0;
        uint64_t count = 0;
        for (uint32_t y = 1; y < h - 1; ++y) {
            for (uint32_t x = 1; x < w - 1; ++x) {
                int lap = 4 * px[y * w + x]
                    - px[(y - 1) * w + x] - px[(y + 1) * w + x]
                    - px[y * w + (x - 1)] - px[y * w + (x + 1)];
                sum += std::abs(lap);
                count++;
            }
        }
        double avg = (count > 0) ? sum / count : 0;
        return (std::min)(1.0f, static_cast<float>(avg / 40.0));
    }

    /// Luminance: penalize very dark or very bright
    static float ScoreLuminance(const uint8_t* px, uint32_t w, uint32_t h) {
        double sum = 0;
        uint32_t count = w * h;
        for (uint32_t i = 0; i < count; ++i) sum += px[i];
        double mean = sum / (std::max)(1u, count);
        // Ideal mean ~128; penalize deviation
        double deviation = std::abs(mean - 128.0) / 128.0;
        return static_cast<float>(1.0 - deviation);
    }

    /// Contrast: standard deviation of pixel intensity
    static float ScoreContrast(const uint8_t* px, uint32_t w, uint32_t h) {
        uint32_t count = w * h;
        double sum = 0, sumSq = 0;
        for (uint32_t i = 0; i < count; ++i) {
            sum += px[i];
            sumSq += static_cast<double>(px[i]) * px[i];
        }
        double mean = sum / (std::max)(1u, count);
        double variance = sumSq / (std::max)(1u, count) - mean * mean;
        double stddev = std::sqrt((std::max)(0.0, variance));
        return (std::min)(1.0f, static_cast<float>(stddev / 64.0));
    }

    /// Noise: inverse of high-frequency energy in 3×3 neighborhoods
    static float ScoreNoise(const uint8_t* px, uint32_t w, uint32_t h) {
        double noiseSum = 0;
        uint64_t count = 0;
        for (uint32_t y = 1; y < h - 1; y += 2) {
            for (uint32_t x = 1; x < w - 1; x += 2) {
                int center = px[y * w + x];
                int neighborSum = px[(y - 1) * w + x] + px[(y + 1) * w + x] +
                    px[y * w + (x - 1)] + px[y * w + (x + 1)];
                double diff = std::abs(center * 4 - neighborSum);
                noiseSum += diff;
                count++;
            }
        }
        double avgNoise = (count > 0) ? noiseSum / count : 0;
        return (std::min)(1.0f, static_cast<float>(1.0 - avgNoise / 100.0));
    }

    AestheticConfig    m_config;
    mutable std::mutex m_mutex;
    AestheticStats     m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
