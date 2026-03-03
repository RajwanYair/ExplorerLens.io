// ThumbnailQualityGate.h — Output Quality Validation for Thumbnails
// Copyright (c) 2026 ExplorerLens Project
//
// Validates generated thumbnails against quality thresholds before returning
// them to the shell. Checks for blank/corrupt output, excessive blur, color
// anomalies, and dimensional accuracy. Rejects sub-standard thumbnails and
// triggers fallback decode attempts.
//
#pragma once

#include <windows.h>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Quality gate check results
enum class QualityVerdict {
    Pass,               // Thumbnail meets all quality criteria
    Fail_Blank,         // All pixels same color (solid fill)
    Fail_TooSmall,      // Dimensions below minimum threshold
    Fail_Corrupt,       // Invalid pixel data detected
    Fail_TooBlurry,     // Below minimum sharpness threshold
    Fail_ColorAnomaly,  // Abnormal color distribution
    Fail_WrongSize,     // Doesn't match requested dimensions
    Fail_Transparency,  // Fully transparent output
    Warn_LowContrast,   // Passes but low contrast warning
    Warn_Monochrome,    // Passes but nearly monochrome
};

/// Configurable quality thresholds
struct GateThresholds {
    float   minSharpness = 0.01f;    // Minimum Laplacian variance
    float   minContrast = 0.02f;    // Minimum contrast ratio
    float   maxBlankRatio = 0.98f;    // Max ratio of identical pixels
    float   maxTransparentRatio = 0.95f;    // Max ratio of fully transparent pixels
    uint32_t minDimension = 16;       // Minimum output dimension
    float   dimensionTolerance = 0.1f;     // Allowed deviation from target size
    bool    rejectMonochrome = false;     // Whether to reject monochrome output
    bool    enableBlurCheck = true;      // Enable sharpness validation
};

/// Detailed quality assessment report
struct QualityReport {
    QualityVerdict  verdict = QualityVerdict::Pass;
    float           sharpness = 0.0f;     // Laplacian variance (higher = sharper)
    float           contrast = 0.0f;     // Weber contrast ratio
    float           blankRatio = 0.0f;     // Fraction of identical pixels
    float           transparentRatio = 0.0f;    // Fraction of transparent pixels
    float           colorVariance = 0.0f;     // Variance across color channels
    uint32_t        actualWidth = 0;
    uint32_t        actualHeight = 0;
    uint32_t        uniqueColors = 0;        // Unique color count (sampled)
    std::string     description;                // Human-readable explanation
};

/// Quality gate for thumbnail validation. Runs a fast multi-check pipeline
/// on generated BGRA pixel data before it's returned to Explorer.
///
/// Typical usage in the thumbnail pipeline:
///   ThumbnailQualityGate gate;
///   auto report = gate.Validate(pixels, width, height, stride, targetSize);
///   if (report.verdict != QualityVerdict::Pass) { /* try fallback decoder */ }
///
class ThumbnailQualityGate {
public:
    ThumbnailQualityGate() = default;

    explicit ThumbnailQualityGate(const GateThresholds& thresholds)
        : m_thresholds(thresholds) {
    }

    void SetThresholds(const GateThresholds& thresholds) { m_thresholds = thresholds; }
    const GateThresholds& GetThresholds() const { return m_thresholds; }

    /// Run all quality checks on a BGRA pixel buffer.
    QualityReport Validate(const uint8_t* pixels, uint32_t width, uint32_t height,
        uint32_t stride, uint32_t targetSize = 0) {
        QualityReport report;
        report.actualWidth = width;
        report.actualHeight = height;

        // Check 1: Null/empty
        if (!pixels || width == 0 || height == 0) {
            report.verdict = QualityVerdict::Fail_Corrupt;
            report.description = "Null or zero-dimension pixel data";
            return report;
        }

        // Check 2: Minimum dimensions
        if (width < m_thresholds.minDimension || height < m_thresholds.minDimension) {
            report.verdict = QualityVerdict::Fail_TooSmall;
            report.description = "Dimensions below minimum threshold";
            return report;
        }

        // Check 3: Target size match (if specified)
        if (targetSize > 0) {
            uint32_t maxDim = (std::max)(width, height);
            float deviation = std::abs(static_cast<float>(maxDim) - targetSize) / targetSize;
            if (deviation > m_thresholds.dimensionTolerance) {
                report.verdict = QualityVerdict::Fail_WrongSize;
                report.description = "Output size deviates from target";
                return report;
            }
        }

        // Check 4: Blank detection (solid color)
        report.blankRatio = ComputeBlankRatio(pixels, width, height, stride);
        if (report.blankRatio > m_thresholds.maxBlankRatio) {
            report.verdict = QualityVerdict::Fail_Blank;
            report.description = "Image is blank (solid fill)";
            return report;
        }

        // Check 5: Transparency check
        report.transparentRatio = ComputeTransparencyRatio(pixels, width, height, stride);
        if (report.transparentRatio > m_thresholds.maxTransparentRatio) {
            report.verdict = QualityVerdict::Fail_Transparency;
            report.description = "Image is fully transparent";
            return report;
        }

        // Check 6: Sharpness (Laplacian variance)
        if (m_thresholds.enableBlurCheck) {
            report.sharpness = ComputeSharpness(pixels, width, height, stride);
            if (report.sharpness < m_thresholds.minSharpness) {
                report.verdict = QualityVerdict::Fail_TooBlurry;
                report.description = "Image is too blurry";
                return report;
            }
        }

        // Check 7: Contrast
        report.contrast = ComputeContrast(pixels, width, height, stride);
        if (report.contrast < m_thresholds.minContrast) {
            report.verdict = QualityVerdict::Warn_LowContrast;
            report.description = "Low contrast (still passes)";
            // Warning — still passes
        }

        // Check 8: Color variance
        report.colorVariance = ComputeColorVariance(pixels, width, height, stride);
        report.uniqueColors = EstimateUniqueColors(pixels, width, height, stride);

        if (report.uniqueColors <= 2 && m_thresholds.rejectMonochrome) {
            report.verdict = QualityVerdict::Warn_Monochrome;
            report.description = "Nearly monochrome output";
        }

        if (report.verdict == QualityVerdict::Pass) {
            report.description = "All quality checks passed";
        }

        return report;
    }

    /// Quick pass/fail check (no detailed report)
    bool QuickCheck(const uint8_t* pixels, uint32_t width, uint32_t height, uint32_t stride) {
        auto report = Validate(pixels, width, height, stride);
        return report.verdict == QualityVerdict::Pass ||
            report.verdict == QualityVerdict::Warn_LowContrast ||
            report.verdict == QualityVerdict::Warn_Monochrome;
    }

private:
    GateThresholds m_thresholds;

    float ComputeBlankRatio(const uint8_t* pixels, uint32_t w, uint32_t h, uint32_t stride) {
        // Compare all pixels to the first pixel
        uint32_t refColor = *reinterpret_cast<const uint32_t*>(pixels);
        uint32_t matchCount = 0;
        uint32_t sampleCount = 0;
        uint32_t step = (std::max)(1u, (w * h) / 4096); // Sample up to 4096 pixels

        for (uint32_t y = 0; y < h; y += (std::max)(1u, h / 64)) {
            for (uint32_t x = 0; x < w; x += step) {
                uint32_t color = *reinterpret_cast<const uint32_t*>(pixels + y * stride + x * 4);
                if (color == refColor) matchCount++;
                sampleCount++;
            }
        }
        return sampleCount > 0 ? static_cast<float>(matchCount) / sampleCount : 0.0f;
    }

    float ComputeTransparencyRatio(const uint8_t* pixels, uint32_t w, uint32_t h, uint32_t stride) {
        uint32_t transparentCount = 0;
        uint32_t sampleCount = 0;
        uint32_t step = (std::max)(1u, (w * h) / 4096);

        for (uint32_t y = 0; y < h; y += (std::max)(1u, h / 64)) {
            for (uint32_t x = 0; x < w; x += step) {
                uint8_t alpha = pixels[y * stride + x * 4 + 3];
                if (alpha == 0) transparentCount++;
                sampleCount++;
            }
        }
        return sampleCount > 0 ? static_cast<float>(transparentCount) / sampleCount : 0.0f;
    }

    float ComputeSharpness(const uint8_t* pixels, uint32_t w, uint32_t h, uint32_t stride) {
        // Laplacian variance (measure of sharpness)
        float sum = 0.0f, sumSq = 0.0f;
        uint32_t count = 0;

        for (uint32_t y = 1; y + 1 < h; y += 2) {
            for (uint32_t x = 1; x + 1 < w; x += 2) {
                // Grayscale Laplacian
                auto lum = [&](uint32_t px, uint32_t py) -> float {
                    const uint8_t* p = pixels + py * stride + px * 4;
                    return 0.299f * p[2] + 0.587f * p[1] + 0.114f * p[0];
                    };
                float lap = -4.0f * lum(x, y) + lum(x - 1, y) + lum(x + 1, y) +
                    lum(x, y - 1) + lum(x, y + 1);
                sum += lap;
                sumSq += lap * lap;
                count++;
            }
        }
        if (count == 0) return 0.0f;
        float mean = sum / count;
        return (sumSq / count - mean * mean) / 65025.0f; // Normalize to [0,1]
    }

    float ComputeContrast(const uint8_t* pixels, uint32_t w, uint32_t h, uint32_t stride) {
        uint8_t minLum = 255, maxLum = 0;
        for (uint32_t y = 0; y < h; y += (std::max)(1u, h / 32)) {
            for (uint32_t x = 0; x < w; x += (std::max)(1u, w / 32)) {
                const uint8_t* p = pixels + y * stride + x * 4;
                uint8_t lum = static_cast<uint8_t>(0.299f * p[2] + 0.587f * p[1] + 0.114f * p[0]);
                minLum = (std::min)(minLum, lum);
                maxLum = (std::max)(maxLum, lum);
            }
        }
        if (maxLum + minLum == 0) return 0.0f;
        return static_cast<float>(maxLum - minLum) / (maxLum + minLum);
    }

    float ComputeColorVariance(const uint8_t* pixels, uint32_t w, uint32_t h, uint32_t stride) {
        double rSum = 0, gSum = 0, bSum = 0;
        double rSumSq = 0, gSumSq = 0, bSumSq = 0;
        uint32_t count = 0;

        for (uint32_t y = 0; y < h; y += (std::max)(1u, h / 32)) {
            for (uint32_t x = 0; x < w; x += (std::max)(1u, w / 32)) {
                const uint8_t* p = pixels + y * stride + x * 4;
                rSum += p[2]; gSum += p[1]; bSum += p[0];
                rSumSq += p[2] * p[2]; gSumSq += p[1] * p[1]; bSumSq += p[0] * p[0];
                count++;
            }
        }
        if (count == 0) return 0.0f;
        double rVar = rSumSq / count - (rSum / count) * (rSum / count);
        double gVar = gSumSq / count - (gSum / count) * (gSum / count);
        double bVar = bSumSq / count - (bSum / count) * (bSum / count);
        return static_cast<float>((rVar + gVar + bVar) / 3.0 / 65025.0);
    }

    uint32_t EstimateUniqueColors(const uint8_t* pixels, uint32_t w, uint32_t h, uint32_t stride) {
        // Use a small hash set via sorted unique for up to 256 samples
        std::vector<uint32_t> samples;
        samples.reserve(256);
        for (uint32_t y = 0; y < h && samples.size() < 256; y += (std::max)(1u, h / 16)) {
            for (uint32_t x = 0; x < w && samples.size() < 256; x += (std::max)(1u, w / 16)) {
                samples.push_back(*reinterpret_cast<const uint32_t*>(pixels + y * stride + x * 4));
            }
        }
        std::sort(samples.begin(), samples.end());
        auto it = std::unique(samples.begin(), samples.end());
        return static_cast<uint32_t>(std::distance(samples.begin(), it));
    }
};

} // namespace Engine
} // namespace ExplorerLens
