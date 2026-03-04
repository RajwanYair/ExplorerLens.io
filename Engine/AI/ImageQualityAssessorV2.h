// ImageQualityAssessorV2.h — Canonical Image Quality Assessor (V1 + V2 consolidated)
// Copyright (c) 2026 ExplorerLens Project
//
// Consolidated IQA: V1 metric-based assessor + V2 BRISQUE-inspired assessor.
//
#pragma once

#include <windows.h>
#include <vector>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <string>
#include <chrono>
#include <array>
#include <numeric>

namespace ExplorerLens {
namespace Engine {

// ── ImageQualityAssessor V1 types (consolidated from ImageQualityAssessor.h) ──

enum class IQAMetric : uint8_t {
    BRISQUE = 0,
    NIQE,
    CLIP_IQA,
    SSIM_Reference,
    PSNR = SSIM_Reference, // compat alias
    COUNT = SSIM_Reference + 1
};
enum class IQADefect : uint8_t {
    None = 0,
    Blur,
    Noise,
    Overexposed,
    Underexposed,
    Compression,
    COUNT
};
enum class IQAGrade : uint8_t {
    Excellent = 0,
    Good,
    Fair,
    Poor,
    Unacceptable,
    COUNT
};

struct IQAScore {
    IQAMetric metric = IQAMetric::BRISQUE;
    float value = 0.0f;
    IQAGrade grade = IQAGrade::Fair;
    IQADefect primaryDefect = IQADefect::None;
};

struct ImageQualityReport {
    IQAScore overallScore;
    float blurScore = 0.0f;
    float noiseScore = 0.0f;
    float exposureOk = 1.0f;
    bool worthEnhancing = false;
};

class ImageQualityAssessor {
public:
    static const wchar_t* MetricName(IQAMetric m) {
        switch (m) {
        case IQAMetric::BRISQUE:
            return L"BRISQUE";
        case IQAMetric::NIQE:
            return L"NIQE";
        case IQAMetric::CLIP_IQA:
            return L"CLIP-IQA";
        case IQAMetric::SSIM_Reference:
            return L"SSIM (Reference)";
        default:
            return L"Unknown";
        }
    }
    static const wchar_t* DefectName(IQADefect d) {
        switch (d) {
        case IQADefect::None:
            return L"None";
        case IQADefect::Blur:
            return L"Blur";
        case IQADefect::Noise:
            return L"Noise";
        case IQADefect::Overexposed:
            return L"Overexposed";
        case IQADefect::Underexposed:
            return L"Underexposed";
        case IQADefect::Compression:
            return L"Compression";
        default:
            return L"Unknown";
        }
    }
    static const wchar_t* GradeName(IQAGrade g) {
        switch (g) {
        case IQAGrade::Excellent:
            return L"Excellent";
        case IQAGrade::Good:
            return L"Good";
        case IQAGrade::Fair:
            return L"Fair";
        case IQAGrade::Poor:
            return L"Poor";
        case IQAGrade::Unacceptable:
            return L"Unacceptable";
        default:
            return L"Unknown";
        }
    }
    static constexpr size_t MetricCount() {
        return static_cast<size_t>(IQAMetric::COUNT);
    }
    static constexpr size_t DefectCount() {
        return static_cast<size_t>(IQADefect::COUNT);
    }
    static constexpr size_t GradeCount() {
        return static_cast<size_t>(IQAGrade::COUNT);
    }

    static double ComputeLaplacianVariance(const uint8_t* gray, uint32_t width,
        uint32_t height, uint32_t stride) {
        if (!gray || width < 3 || height < 3) return 0.0;
        double sum = 0.0, sumSq = 0.0;
        uint32_t count = 0;
        for (uint32_t y = 1; y < height - 1; ++y) {
            for (uint32_t x = 1; x < width - 1; ++x) {
                int lap = 4 * gray[y * stride + x]
                    - gray[(y - 1) * stride + x]
                    - gray[(y + 1) * stride + x]
                    - gray[y * stride + (x - 1)]
                    - gray[y * stride + (x + 1)];
                sum += lap;
                sumSq += static_cast<double>(lap) * lap;
                ++count;
            }
        }
        if (count == 0) return 0.0;
        double mean = sum / count;
        return (sumSq / count) - (mean * mean);
    }

    static double ComputeMeanBrightness(const uint8_t* gray, uint32_t width,
        uint32_t height, uint32_t stride) {
        if (!gray || width == 0 || height == 0) return 0.0;
        uint64_t sum = 0;
        for (uint32_t y = 0; y < height; ++y)
            for (uint32_t x = 0; x < width; ++x)
                sum += gray[y * stride + x];
        return static_cast<double>(sum) / (width * height);
    }

    static std::vector<IQADefect> DetectExposureDefects(const uint8_t* gray,
        uint32_t width, uint32_t height, uint32_t stride) {
        std::vector<IQADefect> defects;
        if (!gray || width == 0 || height == 0) return defects;
        uint32_t bright = 0, dark = 0;
        uint32_t total = width * height;
        for (uint32_t y = 0; y < height; ++y) {
            for (uint32_t x = 0; x < width; ++x) {
                uint8_t v = gray[y * stride + x];
                if (v > 240) ++bright;
                if (v < 15) ++dark;
            }
        }
        if (bright > total * 3 / 10) defects.push_back(IQADefect::Overexposed);
        if (dark > total * 3 / 10) defects.push_back(IQADefect::Underexposed);
        return defects;
    }

    static IQAGrade GradeBySharpness(double laplacianVariance) {
        if (laplacianVariance > 500.0) return IQAGrade::Excellent;
        if (laplacianVariance > 200.0) return IQAGrade::Good;
        if (laplacianVariance > 100.0) return IQAGrade::Fair;
        if (laplacianVariance > 30.0) return IQAGrade::Poor;
        return IQAGrade::Unacceptable;
    }

    static ImageQualityReport Assess(const uint8_t* gray, uint32_t width,
        uint32_t height, uint32_t stride) {
        ImageQualityReport report;
        double lapVar = ComputeLaplacianVariance(gray, width, height, stride);
        report.overallScore.metric = IQAMetric::BRISQUE;
        report.overallScore.value = static_cast<float>(lapVar);
        report.overallScore.grade = GradeBySharpness(lapVar);
        report.blurScore = (lapVar > 500.0f) ? 0.0f :
            (lapVar < 10.0f) ? 1.0f :
            1.0f - static_cast<float>(lapVar / 500.0);
        auto defects = DetectExposureDefects(gray, width, height, stride);
        report.exposureOk = defects.empty() ? 1.0f : 0.0f;
        if (lapVar < 100.0) report.overallScore.primaryDefect = IQADefect::Blur;
        else if (!defects.empty()) report.overallScore.primaryDefect = defects[0];
        report.worthEnhancing = (report.blurScore > 0.5f || report.exposureOk < 0.5f);
        return report;
    }
};

// ── ImageQualityAssessorV2 (BRISQUE-inspired, multi-axis) ──────────────────

/// Quality tier derived from the overall quality score.
enum class QualityTierV2 : uint8_t {
    Excellent = 0,   // > 0.8
    Good,            // > 0.6
    Fair,            // > 0.4
    Poor,            // > 0.2
    Bad              // <= 0.2
};

/// Per-axis quality sub-scores; all values in [0, 1].
struct QualityScoreV2 {
    float overall = 0.0f;
    float sharpness = 0.0f;
    float noise = 0.0f;   // 1.0 = low noise (good), 0.0 = very noisy
    float contrast = 0.0f;
    float colorfulness = 0.0f;
    float exposure = 0.0f;
};

/// Cumulative assessment statistics.
struct AssessmentStatsV2 {
    uint64_t imagesAssessed = 0;
    double   totalScore = 0.0;
    double   totalAssessTimeMs = 0.0;
    std::array<uint64_t, 5> tierDistribution{}; // [Excellent..Bad]
    double AvgScore()  const { return imagesAssessed ? totalScore / static_cast<double>(imagesAssessed) : 0.0; }
    double AvgTimeMs() const { return imagesAssessed ? totalAssessTimeMs / static_cast<double>(imagesAssessed) : 0.0; }
};

/// No-reference image quality assessor (BRISQUE-inspired).
class ImageQualityAssessorV2 {
public:
    ImageQualityAssessorV2() {
        InitializeSRWLock(&m_statsLock);
    }

    /// Full assessment returning all sub-metrics.
    inline QualityScoreV2 Assess(const uint8_t* rgbaData,
        uint32_t width,
        uint32_t height) {
        using Clock = std::chrono::high_resolution_clock;
        auto t0 = Clock::now();

        QualityScoreV2 qs{};
        if (!rgbaData || width < 3 || height < 3) return qs;

        const uint32_t pixelCount = width * height;

        // Convert to grayscale (luminance) buffer
        std::vector<float> lum(pixelCount);
        for (uint32_t i = 0; i < pixelCount; ++i) {
            uint32_t off = i * 4;
            lum[i] = (0.299f * rgbaData[off] + 0.587f * rgbaData[off + 1] + 0.114f * rgbaData[off + 2]) / 255.0f;
        }

        // 1. Sharpness — Laplacian variance
        qs.sharpness = ComputeSharpness(lum, width, height);

        // 2. Noise — median absolute deviation
        qs.noise = ComputeNoiseScore(lum, width, height);

        // 3. Contrast — Michelson contrast in 16×16 blocks
        qs.contrast = ComputeContrast(lum, width, height);

        // 4. Colorfulness — Hasler & Süsstrunk 2003
        qs.colorfulness = ComputeColorfulness(rgbaData, pixelCount);

        // 5. Exposure — deviation from target midpoint
        qs.exposure = ComputeExposure(lum, pixelCount);

        // Weighted overall
        qs.overall = 0.35f * qs.sharpness
            + 0.20f * qs.noise
            + 0.20f * qs.contrast
            + 0.15f * qs.colorfulness
            + 0.10f * qs.exposure;
        qs.overall = (std::max)(0.0f, (std::min)(qs.overall, 1.0f));

        auto t1 = Clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        QualityTierV2 tier = GetTier(qs.overall);

        AcquireSRWLockExclusive(&m_statsLock);
        m_stats.imagesAssessed++;
        m_stats.totalScore += qs.overall;
        m_stats.totalAssessTimeMs += ms;
        m_stats.tierDistribution[static_cast<size_t>(tier)]++;
        ReleaseSRWLockExclusive(&m_statsLock);

        return qs;
    }

    /// Map a score to a quality tier.
    static inline QualityTierV2 GetTier(float score) {
        if (score > 0.8f) return QualityTierV2::Excellent;
        if (score > 0.6f) return QualityTierV2::Good;
        if (score > 0.4f) return QualityTierV2::Fair;
        if (score > 0.2f) return QualityTierV2::Poor;
        return QualityTierV2::Bad;
    }

    /// Human-readable tier name.
    static inline const wchar_t* TierName(QualityTierV2 tier) {
        switch (tier) {
        case QualityTierV2::Excellent: return L"Excellent";
        case QualityTierV2::Good:      return L"Good";
        case QualityTierV2::Fair:      return L"Fair";
        case QualityTierV2::Poor:      return L"Poor";
        case QualityTierV2::Bad:       return L"Bad";
        default:                       return L"Unknown";
        }
    }

    /// Retrieve cumulative statistics (thread-safe).
    inline AssessmentStatsV2 GetStats() const {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_statsLock));
        AssessmentStatsV2 copy = m_stats;
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_statsLock));
        return copy;
    }

private:
    // ---- Sub-metric implementations ----------------------------------------

    /// Sharpness via Laplacian variance. Returns [0, 1].
    static inline float ComputeSharpness(const std::vector<float>& lum,
        uint32_t w, uint32_t h) {
        // Laplacian kernel: [0,-1,0; -1,4,-1; 0,-1,0]
        double sum = 0.0, sumSq = 0.0;
        uint32_t count = 0;
        for (uint32_t y = 1; y + 1 < h; ++y) {
            for (uint32_t x = 1; x + 1 < w; ++x) {
                float lap = 4.0f * lum[y * w + x]
                    - lum[(y - 1) * w + x]
                    - lum[(y + 1) * w + x]
                    - lum[y * w + (x - 1)]
                    - lum[y * w + (x + 1)];
                sum += lap;
                sumSq += static_cast<double>(lap) * lap;
                ++count;
            }
        }
        if (count == 0) return 0.0f;
        double mean = sum / count;
        double variance = sumSq / count - mean * mean;
        // Normalize: variance of ~0.02 on [0,1] luminance is sharp
        float normalized = static_cast<float>(variance / 0.025);
        return (std::max)(0.0f, (std::min)(normalized, 1.0f));
    }

    /// Noise estimation via median absolute deviation in 3x3 blocks.
    /// Returns [0, 1] where 1.0 = minimal noise (good).
    static inline float ComputeNoiseScore(const std::vector<float>& lum,
        uint32_t w, uint32_t h) {
        double madSum = 0.0;
        uint32_t blockCount = 0;
        std::array<float, 9> neighborhood{};

        for (uint32_t y = 1; y + 1 < h; y += 3) {
            for (uint32_t x = 1; x + 1 < w; x += 3) {
                uint32_t idx = 0;
                for (int ky = -1; ky <= 1; ++ky) {
                    for (int kx = -1; kx <= 1; ++kx) {
                        neighborhood[idx++] = lum[(y + ky) * w + (x + kx)];
                    }
                }
                // Find median
                std::sort(neighborhood.begin(), neighborhood.end());
                float median = neighborhood[4];
                // Average absolute deviation from median
                float ad = 0.0f;
                for (uint32_t i = 0; i < 9; ++i) {
                    ad += std::abs(neighborhood[i] - median);
                }
                ad /= 9.0f;
                madSum += ad;
                ++blockCount;
            }
        }
        if (blockCount == 0) return 1.0f;
        float avgMAD = static_cast<float>(madSum / blockCount);
        // Normalize: low MAD = low noise = score near 1.0
        // MAD > 0.05 is quite noisy on [0,1] luminance
        float noiseLevel = (std::min)(avgMAD / 0.06f, 1.0f);
        return 1.0f - noiseLevel;
    }

    /// Michelson contrast in 16x16 blocks, averaged. Returns [0, 1].
    static inline float ComputeContrast(const std::vector<float>& lum,
        uint32_t w, uint32_t h) {
        const uint32_t blockSize = 16;
        double contrastSum = 0.0;
        uint32_t blockCount = 0;

        for (uint32_t by = 0; by + blockSize <= h; by += blockSize) {
            for (uint32_t bx = 0; bx + blockSize <= w; bx += blockSize) {
                float bmin = 1.0f, bmax = 0.0f;
                for (uint32_t dy = 0; dy < blockSize; ++dy) {
                    for (uint32_t dx = 0; dx < blockSize; ++dx) {
                        float v = lum[(by + dy) * w + (bx + dx)];
                        bmin = (std::min)(bmin, v);
                        bmax = (std::max)(bmax, v);
                    }
                }
                float denom = bmax + bmin;
                float michelson = (denom > 1e-6f) ? (bmax - bmin) / denom : 0.0f;
                contrastSum += michelson;
                ++blockCount;
            }
        }
        if (blockCount == 0) return 0.0f;
        return static_cast<float>(contrastSum / blockCount);
    }

    /// Hasler & Süsstrunk 2003 colorfulness metric. Returns [0, 1].
    static inline float ComputeColorfulness(const uint8_t* rgba,
        uint32_t pixelCount) {
        if (pixelCount == 0) return 0.0f;

        double sumRG = 0.0, sumYB = 0.0;
        double sumRG2 = 0.0, sumYB2 = 0.0;

        for (uint32_t i = 0; i < pixelCount; ++i) {
            uint32_t off = i * 4;
            float r = rgba[off + 0] / 255.0f;
            float g = rgba[off + 1] / 255.0f;
            float b = rgba[off + 2] / 255.0f;
            float rg = r - g;
            float yb = 0.5f * (r + g) - b;
            sumRG += rg;
            sumYB += yb;
            sumRG2 += static_cast<double>(rg) * rg;
            sumYB2 += static_cast<double>(yb) * yb;
        }

        double n = static_cast<double>(pixelCount);
        double meanRG = sumRG / n;
        double meanYB = sumYB / n;
        double varRG = sumRG2 / n - meanRG * meanRG;
        double varYB = sumYB2 / n - meanYB * meanYB;
        double sigmaRG = std::sqrt((std::max)(varRG, 0.0));
        double sigmaYB = std::sqrt((std::max)(varYB, 0.0));

        double colorfulness = std::sqrt(sigmaRG * sigmaRG + sigmaYB * sigmaYB)
            + 0.3 * std::sqrt(meanRG * meanRG + meanYB * meanYB);

        // Typical colorfulness range: 0–0.8 for natural images
        float normalized = static_cast<float>(colorfulness / 0.75);
        return (std::max)(0.0f, (std::min)(normalized, 1.0f));
    }

    /// Exposure quality: penalize deviation from target 0.5 midpoint.
    /// Also penalize very dark (<0.1) or very bright (>0.9) areas.
    static inline float ComputeExposure(const std::vector<float>& lum,
        uint32_t pixelCount) {
        if (pixelCount == 0) return 0.0f;

        float sum = 0.0f;
        uint32_t extremeCount = 0;
        for (uint32_t i = 0; i < pixelCount; ++i) {
            sum += lum[i];
            if (lum[i] < 0.1f || lum[i] > 0.9f) ++extremeCount;
        }
        float meanLum = sum / static_cast<float>(pixelCount);

        // Distance from ideal mid-tone
        float midDev = std::abs(meanLum - 0.5f);

        // Fraction of extreme pixels
        float extremeFrac = static_cast<float>(extremeCount) / static_cast<float>(pixelCount);

        // Score: 1.0 when mean~0.5 and few extremes, 0.0 when far off
        float exposureScore = (1.0f - midDev * 2.0f) * (1.0f - extremeFrac);
        return (std::max)(0.0f, (std::min)(exposureScore, 1.0f));
    }

    mutable SRWLOCK       m_statsLock{};
    AssessmentStatsV2     m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
