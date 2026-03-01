#pragma once
// ============================================================================
// ThumbnailDiffEngine.h — Structural similarity comparison between thumbnails
//
// Purpose:   Structural similarity comparison between thumbnail versions
// Provides:  DiffAlgorithm, DiffSeverity enums, DiffResult,
//            DiffThresholdConfig structs, and ThumbnailDiffEngine class
// Used by:   Cache validation and quality comparison
// ============================================================================

#include <string>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// ThumbnailDiffEngine — Visual diff between cached and current thumbnails
// ============================================================================

enum class DiffAlgorithm {
    PixelWise,
    SSIM,
    Perceptual,
    Histogram,
    EdgeDetect
};

inline const char* DiffAlgorithmName(DiffAlgorithm value) {
    switch (value) {
    case DiffAlgorithm::PixelWise:   return "PixelWise";
    case DiffAlgorithm::SSIM:        return "SSIM";
    case DiffAlgorithm::Perceptual:  return "Perceptual";
    case DiffAlgorithm::Histogram:   return "Histogram";
    case DiffAlgorithm::EdgeDetect:  return "EdgeDetect";
    default:                         return "Unknown";
    }
}

enum class DiffSeverity {
    Identical,
    Minor,
    Moderate,
    Major,
    Complete
};

inline const char* DiffSeverityName(DiffSeverity value) {
    switch (value) {
    case DiffSeverity::Identical: return "Identical";
    case DiffSeverity::Minor:     return "Minor";
    case DiffSeverity::Moderate:  return "Moderate";
    case DiffSeverity::Major:     return "Major";
    case DiffSeverity::Complete:  return "Complete";
    default:                      return "Unknown";
    }
}

struct DiffResult {
    float         similarity = 1.0f;   // 0.0 = completely different, 1.0 = identical
    DiffSeverity  severity = DiffSeverity::Identical;
    uint32_t      changedPixelCount = 0;
    DiffAlgorithm algorithm = DiffAlgorithm::PixelWise;
    uint32_t      totalPixels = 0;
    double        computeTimeMs = 0.0;

    float GetChangePercent() const {
        if (totalPixels == 0) return 0.0f;
        return (static_cast<float>(changedPixelCount) / static_cast<float>(totalPixels)) * 100.0f;
    }
};

struct DiffThresholdConfig {
    float minorThreshold = 0.95f;  // similarity >= 0.95 -> Minor
    float moderateThreshold = 0.80f;  // similarity >= 0.80 -> Moderate
    float majorThreshold = 0.50f;  // similarity >= 0.50 -> Major
    // Below majorThreshold -> Complete
};

class ThumbnailDiffEngine {
public:
    static constexpr float DEFAULT_SIGNIFICANCE_THRESHOLD = 0.02f;
    static constexpr uint32_t MAX_THUMBNAIL_DIMENSION = 4096;

    ThumbnailDiffEngine() = default;
    ~ThumbnailDiffEngine() = default;

    DiffResult ComputeDiff(const uint8_t* imageA, const uint8_t* imageB,
        uint32_t width, uint32_t height, DiffAlgorithm algo) {
        DiffResult result;
        result.algorithm = algo;
        result.totalPixels = width * height;

        if (!imageA || !imageB || width == 0 || height == 0) {
            result.similarity = 0.0f;
            result.severity = DiffSeverity::Complete;
            return result;
        }

        if (width > MAX_THUMBNAIL_DIMENSION || height > MAX_THUMBNAIL_DIMENSION) {
            result.similarity = 0.0f;
            result.severity = DiffSeverity::Complete;
            return result;
        }

        uint32_t pixelCount = width * height;
        uint32_t diffCount = 0;
        double totalDiff = 0.0;

        for (uint32_t i = 0; i < pixelCount * 4; i += 4) {
            int dr = static_cast<int>(imageA[i]) - static_cast<int>(imageB[i]);
            int dg = static_cast<int>(imageA[i + 1]) - static_cast<int>(imageB[i + 1]);
            int db = static_cast<int>(imageA[i + 2]) - static_cast<int>(imageB[i + 2]);
            double pixelDist = std::sqrt(static_cast<double>(dr * dr + dg * dg + db * db));
            totalDiff += pixelDist;
            if (pixelDist > 10.0) {
                diffCount++;
            }
        }

        double maxDiff = pixelCount * 441.67; // sqrt(255^2 * 3)
        result.similarity = static_cast<float>(1.0 - (totalDiff / maxDiff));
        result.similarity = (std::max)(0.0f, (std::min)(1.0f, result.similarity));
        result.changedPixelCount = diffCount;
        result.severity = ClassifySeverity(result.similarity);

        m_totalDiffs++;
        return result;
    }

    DiffSeverity ClassifySeverity(float similarity) const {
        if (similarity >= 1.0f)                           return DiffSeverity::Identical;
        if (similarity >= m_thresholds.minorThreshold)    return DiffSeverity::Minor;
        if (similarity >= m_thresholds.moderateThreshold) return DiffSeverity::Moderate;
        if (similarity >= m_thresholds.majorThreshold)    return DiffSeverity::Major;
        return DiffSeverity::Complete;
    }

    float GetThreshold() const { return m_significanceThreshold; }
    void SetThreshold(float t) { m_significanceThreshold = t; }

    bool IsSignificantChange(const DiffResult& result) const {
        return (1.0f - result.similarity) > m_significanceThreshold;
    }

    void SetThresholdConfig(const DiffThresholdConfig& config) { m_thresholds = config; }
    const DiffThresholdConfig& GetThresholdConfig() const { return m_thresholds; }

    uint64_t GetTotalDiffs() const { return m_totalDiffs; }

private:
    float               m_significanceThreshold = DEFAULT_SIGNIFICANCE_THRESHOLD;
    DiffThresholdConfig m_thresholds;
    uint64_t            m_totalDiffs = 0;
};

} // namespace Engine
} // namespace ExplorerLens
