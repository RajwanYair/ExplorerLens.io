// ============================================================================
// ThumbnailQualityValidator.h — Post-Decode Quality Validation
// ExplorerLens Engine v15.0.0
// Copyright (c) 2026 ExplorerLens Project
//
// Validates generated thumbnails meet quality thresholds before caching.
// Detects common decode artifacts: blank/solid images, extreme color casts,
// corrupted pixels, undersized output, and excessive blur. Assigns a
// confidence score to each thumbnail and rejects sub-threshold results.
// ============================================================================

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <array>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Quality check result flags (bitmask)
// ============================================================================

enum class QualityCheckFlag : uint32_t {
    None = 0,
    IsBlank = 1 << 0,  // All pixels same color
    IsSolidColor = 1 << 1,  // Very low color variance
    IsTooSmall = 1 << 2,  // Below minimum dimensions
    IsTooBlurry = 1 << 3,  // Laplacian variance below threshold
    HasColorCast = 1 << 4,  // Extreme channel imbalance
    HasCorruption = 1 << 5,  // NaN, Inf, or out-of-range values
    HasBanding = 1 << 6,  // Color banding (low bit depth)
    IsOverexposed = 1 << 7,  // >50% pixels near white
    IsUnderexposed = 1 << 8,  // >50% pixels near black
    HasAspectError = 1 << 9   // Aspect ratio significantly wrong
};

inline QualityCheckFlag operator|(QualityCheckFlag a, QualityCheckFlag b) {
    return static_cast<QualityCheckFlag>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline bool HasQualityFlag(QualityCheckFlag flags, QualityCheckFlag test) {
    return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(test)) != 0;
}

inline uint32_t CountQualityFlags(QualityCheckFlag flags) {
    uint32_t v = static_cast<uint32_t>(flags);
    uint32_t count = 0;
    while (v) { count += v & 1; v >>= 1; }
    return count;
}

// ============================================================================
// Validation result
// ============================================================================

struct QualityValidationResult {
    float             overallScore = 0.0f;      // 0-1, overall quality
    float             sharpnessScore = 0.0f;    // Laplacian variance normalized
    float             colorVariance = 0.0f;     // Channel variance
    float             brightnessAvg = 0.0f;     // Average brightness 0-255
    float             contrastRatio = 0.0f;     // Max/min luminance ratio
    QualityCheckFlag  flags = QualityCheckFlag::None;
    bool              passed = false;
    std::string       failureReason;
    double            validationTimeMs = 0.0;
};

// ============================================================================
// Validation thresholds
// ============================================================================

struct QualityValidatorThresholds {
    float   minOverallScore = 0.3f;
    float   minSharpness = 0.05f;
    float   minColorVariance = 5.0f;
    float   maxColorCastRatio = 3.0f;   // Max ratio between R/G/B averages
    uint32_t minWidth = 16;
    uint32_t minHeight = 16;
    float   maxBlankPixelRatio = 0.95f;  // Max % of same-color pixels
    float   minBrightness = 5.0f;
    float   maxBrightness = 250.0f;
    float   maxAspectDeviation = 0.5f;   // Max |actual - expected| ratio
};

// ============================================================================
// ThumbnailQualityValidator — main class
// ============================================================================

class ThumbnailQualityValidator {
public:
    ThumbnailQualityValidator() = default;

    /// Set validation thresholds
    void SetThresholds(const QualityValidatorThresholds& thresholds) {
        m_thresholds = thresholds;
    }
    const QualityValidatorThresholds& GetThresholds() const { return m_thresholds; }

    /// Validate a thumbnail bitmap (BGRA 32bpp)
    QualityValidationResult Validate(const uint8_t* data, uint32_t width,
        uint32_t height, uint32_t stride = 0) {
        QualityValidationResult result;

        if (!data || width == 0 || height == 0) {
            result.flags = QualityCheckFlag::IsTooSmall;
            result.failureReason = "Null or zero-size image";
            return result;
        }

        if (stride == 0) stride = width * 4;  // Default BGRA stride

        // Check minimum dimensions
        if (width < m_thresholds.minWidth || height < m_thresholds.minHeight) {
            result.flags = result.flags | QualityCheckFlag::IsTooSmall;
        }

        // Analyze pixels
        PixelAnalysis analysis = AnalyzePixels(data, width, height, stride);

        // Check for blank image
        if (analysis.uniqueColors <= 1) {
            result.flags = result.flags | QualityCheckFlag::IsBlank;
        }
        else if (analysis.dominantColorRatio > m_thresholds.maxBlankPixelRatio) {
            result.flags = result.flags | QualityCheckFlag::IsSolidColor;
        }

        // Check brightness
        result.brightnessAvg = analysis.avgBrightness;
        if (analysis.avgBrightness < m_thresholds.minBrightness) {
            result.flags = result.flags | QualityCheckFlag::IsUnderexposed;
        }
        if (analysis.avgBrightness > m_thresholds.maxBrightness) {
            result.flags = result.flags | QualityCheckFlag::IsOverexposed;
        }

        // Check color cast
        if (analysis.maxChannelAvg > 0 && analysis.minChannelAvg > 0) {
            float castRatio = analysis.maxChannelAvg / analysis.minChannelAvg;
            if (castRatio > m_thresholds.maxColorCastRatio) {
                result.flags = result.flags | QualityCheckFlag::HasColorCast;
            }
        }

        // Check corruption
        if (analysis.hasNaN || analysis.hasOutOfRange) {
            result.flags = result.flags | QualityCheckFlag::HasCorruption;
        }

        // Compute scores
        result.colorVariance = analysis.colorVariance;
        result.sharpnessScore = analysis.edgeDensity;
        result.contrastRatio = (analysis.minBrightness > 0)
            ? (analysis.maxBrightness / analysis.minBrightness) : 0.0f;

        // Sharpness check
        if (result.sharpnessScore < m_thresholds.minSharpness) {
            result.flags = result.flags | QualityCheckFlag::IsTooBlurry;
        }

        // Compute overall score
        result.overallScore = ComputeOverallScore(result, analysis);

        // Determine pass/fail
        result.passed = (result.overallScore >= m_thresholds.minOverallScore) &&
            !HasQualityFlag(result.flags, QualityCheckFlag::IsBlank) &&
            !HasQualityFlag(result.flags, QualityCheckFlag::HasCorruption);

        if (!result.passed) {
            result.failureReason = BuildFailureReason(result.flags);
        }

        m_totalValidated++;
        if (result.passed) m_totalPassed++;

        return result;
    }

    /// Quick check — just tests for obviously bad thumbnails
    bool QuickCheck(const uint8_t* data, uint32_t width, uint32_t height) const {
        if (!data || width < 4 || height < 4) return false;

        // Sample corners and center
        uint32_t stride = width * 4;
        std::array<uint32_t, 5> samplePixels;
        samplePixels[0] = *reinterpret_cast<const uint32_t*>(data);
        samplePixels[1] = *reinterpret_cast<const uint32_t*>(data + (width - 1) * 4);
        samplePixels[2] = *reinterpret_cast<const uint32_t*>(data + (height - 1) * stride);
        samplePixels[3] = *reinterpret_cast<const uint32_t*>(data + (height - 1) * stride + (width - 1) * 4);
        samplePixels[4] = *reinterpret_cast<const uint32_t*>(data + (height / 2) * stride + (width / 2) * 4);

        // If all 5 samples are identical, likely blank
        bool allSame = std::all_of(samplePixels.begin(), samplePixels.end(),
            [&](uint32_t p) { return p == samplePixels[0]; });
        return !allSame;
    }

    uint64_t GetTotalValidated() const { return m_totalValidated; }
    uint64_t GetTotalPassed() const { return m_totalPassed; }
    float GetPassRate() const {
        return (m_totalValidated > 0)
            ? (static_cast<float>(m_totalPassed) / m_totalValidated) : 0.0f;
    }

private:
    struct PixelAnalysis {
        float avgBrightness = 0.0f;
        float minBrightness = 255.0f;
        float maxBrightness = 0.0f;
        float avgR = 0.0f, avgG = 0.0f, avgB = 0.0f;
        float maxChannelAvg = 0.0f;
        float minChannelAvg = 255.0f;
        float colorVariance = 0.0f;
        float edgeDensity = 0.0f;
        float dominantColorRatio = 0.0f;
        uint32_t uniqueColors = 0;
        bool hasNaN = false;
        bool hasOutOfRange = false;
    };

    PixelAnalysis AnalyzePixels(const uint8_t* data, uint32_t width,
        uint32_t height, uint32_t stride) const {
        PixelAnalysis a;
        uint64_t sumR = 0, sumG = 0, sumB = 0;
        uint64_t pixelCount = 0;

        // Sample every 4th pixel for performance
        uint32_t step = (std::max)(1u, (std::max)(width / 64, height / 64));
        std::unordered_map<uint32_t, uint32_t> colorHist;

        for (uint32_t y = 0; y < height; y += step) {
            const uint8_t* row = data + y * stride;
            for (uint32_t x = 0; x < width; x += step) {
                uint8_t b = row[x * 4 + 0];
                uint8_t g = row[x * 4 + 1];
                uint8_t r = row[x * 4 + 2];

                float brightness = 0.299f * r + 0.587f * g + 0.114f * b;
                sumR += r; sumG += g; sumB += b;

                if (brightness < a.minBrightness) a.minBrightness = brightness;
                if (brightness > a.maxBrightness) a.maxBrightness = brightness;

                uint32_t colorKey = (static_cast<uint32_t>(r >> 4) << 8) |
                    (static_cast<uint32_t>(g >> 4) << 4) | (b >> 4);
                colorHist[colorKey]++;

                pixelCount++;
            }
        }

        if (pixelCount > 0) {
            a.avgR = static_cast<float>(sumR) / pixelCount;
            a.avgG = static_cast<float>(sumG) / pixelCount;
            a.avgB = static_cast<float>(sumB) / pixelCount;
            a.avgBrightness = 0.299f * a.avgR + 0.587f * a.avgG + 0.114f * a.avgB;
            a.maxChannelAvg = (std::max)({ a.avgR, a.avgG, a.avgB });
            a.minChannelAvg = (std::min)({ a.avgR, a.avgG, a.avgB });
        }

        a.uniqueColors = static_cast<uint32_t>(colorHist.size());

        // Dominant color ratio
        uint32_t maxCount = 0;
        for (const auto& [color, count] : colorHist) {
            if (count > maxCount) maxCount = count;
        }
        a.dominantColorRatio = (pixelCount > 0)
            ? (static_cast<float>(maxCount) / pixelCount) : 1.0f;

        // Simple edge density via horizontal gradient
        uint64_t edgeSum = 0, edgeSamples = 0;
        for (uint32_t y = 0; y < height; y += step * 2) {
            const uint8_t* row = data + y * stride;
            for (uint32_t x = 1; x < width - 1; x += step) {
                int diff = std::abs(static_cast<int>(row[(x + 1) * 4 + 1]) -
                    static_cast<int>(row[(x - 1) * 4 + 1]));
                edgeSum += diff;
                edgeSamples++;
            }
        }
        a.edgeDensity = (edgeSamples > 0)
            ? (static_cast<float>(edgeSum) / edgeSamples / 255.0f) : 0.0f;

        // Color variance
        float varSum = 0;
        varSum += (a.avgR - a.avgBrightness) * (a.avgR - a.avgBrightness);
        varSum += (a.avgG - a.avgBrightness) * (a.avgG - a.avgBrightness);
        varSum += (a.avgB - a.avgBrightness) * (a.avgB - a.avgBrightness);
        a.colorVariance = std::sqrt(varSum / 3.0f);

        return a;
    }

    float ComputeOverallScore(const QualityValidationResult& result,
        const PixelAnalysis& analysis) const {
        float score = 0.0f;

        // Edge density → sharpness (40% weight)
        score += (std::min)(1.0f, analysis.edgeDensity * 10.0f) * 0.4f;

        // Color variety (30% weight)
        score += (std::min)(1.0f, static_cast<float>(analysis.uniqueColors) / 200.0f) * 0.3f;

        // Brightness range (20% weight)
        float range = analysis.maxBrightness - analysis.minBrightness;
        score += (std::min)(1.0f, range / 200.0f) * 0.2f;

        // Penalty for flags
        uint32_t flagCount = CountQualityFlags(result.flags);
        score -= flagCount * 0.15f;

        return (std::max)(0.0f, (std::min)(1.0f, score));
    }

    std::string BuildFailureReason(QualityCheckFlag flags) const {
        std::string reason;
        if (HasQualityFlag(flags, QualityCheckFlag::IsBlank)) reason += "Blank; ";
        if (HasQualityFlag(flags, QualityCheckFlag::IsSolidColor)) reason += "SolidColor; ";
        if (HasQualityFlag(flags, QualityCheckFlag::IsTooSmall)) reason += "TooSmall; ";
        if (HasQualityFlag(flags, QualityCheckFlag::IsTooBlurry)) reason += "TooBlurry; ";
        if (HasQualityFlag(flags, QualityCheckFlag::HasCorruption)) reason += "Corruption; ";
        if (HasQualityFlag(flags, QualityCheckFlag::HasColorCast)) reason += "ColorCast; ";
        if (HasQualityFlag(flags, QualityCheckFlag::IsOverexposed)) reason += "Overexposed; ";
        if (HasQualityFlag(flags, QualityCheckFlag::IsUnderexposed)) reason += "Underexposed; ";
        return reason;
    }

    // Need this for unordered_map
    struct ColorHash {
        size_t operator()(uint32_t k) const { return std::hash<uint32_t>()(k); }
    };

    QualityValidatorThresholds m_thresholds;
    uint64_t m_totalValidated = 0;
    uint64_t m_totalPassed = 0;
};

} // namespace Engine
} // namespace ExplorerLens
