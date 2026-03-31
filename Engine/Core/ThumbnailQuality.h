// ThumbnailQuality.h — Unified Thumbnail Quality Assessment & Validation
// Copyright (c) 2026 ExplorerLens Project
//
// Unified header consolidating: ThumbnailQualityAnalyzer.h, ThumbnailQualityGate.h,
// ThumbnailQualityValidator.h, ThumbnailBlurDetector.h, ThumbnailColorCorrector.h,
// ThumbnailSignatureVerifier.h, ThumbnailColorSpace.h.
// Part of v31.2.0 consolidation pass.
//
#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// -- Quality metric types (canonical — superset of Analyzer + Gate metrics) ---

enum class QualityMetric : uint8_t {
    SSIM, PSNR, Sharpness, Clarity, ColorAccuracy, Saturation,
    EdgePreservation, ContrastRatio, NoiseLevel, Artifacts,
    CompressionArtifact, COUNT
};

// -- Quality grade (from ThumbnailQualityAnalyzer) ----------------------------

enum class QualityGrade : uint8_t {
    Rejected, Poor, Acceptable, Good, Excellent
};

// -- Quality gate verdicts (from QualityGate) ---------------------------------

enum class QualityVerdict {
    Pass, Fail_Blank, Fail_TooSmall, Fail_Corrupt, Fail_TooBlurry,
    Fail_ColorAnomaly, Fail_WrongSize, Fail_Transparency
};

// -- Quality check bitmask flags (from QualityValidator) ----------------------

enum class QualityCheckFlag : uint32_t {
    None         = 0,
    IsBlank      = 1 << 0,
    IsSolidColor = 1 << 1,
    IsTooSmall   = 1 << 2,
    HasColorCast = 1 << 3,
    IsCorrupt    = 1 << 4,
    IsTooBlurry  = 1 << 5,
    HasArtifacts = 1 << 6
};

inline QualityCheckFlag operator|(QualityCheckFlag a, QualityCheckFlag b) {
    return static_cast<QualityCheckFlag>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline bool HasFlag(QualityCheckFlag set, QualityCheckFlag flag) {
    return (static_cast<uint32_t>(set) & static_cast<uint32_t>(flag)) != 0;
}
inline bool HasQualityFlag(QualityCheckFlag set, QualityCheckFlag flag) { return HasFlag(set, flag); }

// -- Sharpness levels (from BlurDetector) -------------------------------------

enum class SharpnessLevel : int {
    VeryBlurry = 0, Blurry = 1, Acceptable = 2, Sharp = 3, VerySharp = 4
};

// -- Color correction profiles (from ColorCorrector) --------------------------

enum class ColorCorrectionProfile : uint8_t {
    None, sRGBNormalize, DarkModeBoost, LightModeDim, HighContrastMap, COUNT
};

// -- Color space types (from ColorSpace) --------------------------------------

enum class ColorSpaceType : uint8_t {
    SRGB, AdobeRGB, DisplayP3, Rec2020, ProPhotoRGB
};

// -- Result structs -----------------------------------------------------------

struct QualityScore {
    float ssim         = 0.0f;
    float psnr         = 0.0f;
    float sharpness    = 0.0f;
    float colorDeltaE  = 0.0f;
    float confidence   = 0.0f;
    QualityVerdict verdict = QualityVerdict::Pass;
    QualityCheckFlag flags = QualityCheckFlag::None;
};

struct BlurAnalysisResult {
    double laplacianVariance = 0.0;
    SharpnessLevel level     = SharpnessLevel::Acceptable;
    bool isBlurry            = false;
};

struct ColorCorrectionResult {
    bool  applied       = false;
    float avgLuminance  = 0.0f;
    float contrastRatio = 0.0f;
};

// -- Unified quality engine ---------------------------------------------------

class ThumbnailQualityEngine {
public:
    QualityScore Analyze(const uint8_t* rgba, uint32_t w, uint32_t h) const {
        QualityScore s;
        if (!rgba || w == 0 || h == 0) { s.verdict = QualityVerdict::Fail_Corrupt; return s; }
        s.sharpness = ComputeLaplacianVariance(rgba, w, h);
        s.confidence = (s.sharpness > 200.0f) ? 1.0f : s.sharpness / 200.0f;
        if (IsBlank(rgba, w * h * 4)) s.verdict = QualityVerdict::Fail_Blank;
        else if (s.sharpness < 50.0f) s.verdict = QualityVerdict::Fail_TooBlurry;
        return s;
    }

    BlurAnalysisResult DetectBlur(const uint8_t* gray, uint32_t w, uint32_t h) const {
        BlurAnalysisResult r;
        if (!gray || w < 3 || h < 3) return r;
        r.laplacianVariance = ComputeLaplacianVariance(gray, w, h);
        r.level = (r.laplacianVariance < 50) ? SharpnessLevel::VeryBlurry
                : (r.laplacianVariance < 200) ? SharpnessLevel::Blurry
                : (r.laplacianVariance < 500) ? SharpnessLevel::Acceptable
                : (r.laplacianVariance < 1000) ? SharpnessLevel::Sharp : SharpnessLevel::VerySharp;
        r.isBlurry = (r.level <= SharpnessLevel::Blurry);
        return r;
    }

    uint32_t ComputeCRC32(const uint8_t* data, size_t size) const {
        if (!data || size == 0) return 0;
        uint32_t crc = 0xFFFFFFFF;
        for (size_t i = 0; i < size; ++i) {
            crc ^= data[i];
            for (int b = 0; b < 8; ++b) crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
        }
        return ~crc;
    }

    bool VerifySignature(const uint8_t* data, size_t size, uint32_t expected) const {
        return ComputeCRC32(data, size) == expected;
    }

private:
    static double ComputeLaplacianVariance(const uint8_t* px, uint32_t w, uint32_t h) {
        if (w < 3 || h < 3) return 0.0;
        double sum = 0.0, sumSq = 0.0;
        uint32_t count = 0;
        for (uint32_t y = 1; y < h - 1; ++y) {
            for (uint32_t x = 1; x < w - 1; ++x) {
                int lap = -4 * px[y * w + x] + px[(y-1)*w+x] + px[(y+1)*w+x]
                          + px[y*w+x-1] + px[y*w+x+1];
                sum += lap; sumSq += static_cast<double>(lap) * lap; ++count;
            }
        }
        if (count == 0) return 0.0;
        double mean = sum / count;
        return (sumSq / count) - (mean * mean);
    }

    static bool IsBlank(const uint8_t* rgba, size_t bytes) {
        if (bytes < 4) return true;
        uint8_t r0 = rgba[0], g0 = rgba[1], b0 = rgba[2];
        for (size_t i = 4; i < bytes; i += 4) {
            if (rgba[i] != r0 || rgba[i+1] != g0 || rgba[i+2] != b0) return false;
        }
        return true;
    }
};

// -- ThumbnailQualityAnalyzer — from ThumbnailQualityAnalyzer.h ---------------

struct ThumbnailQualityReport {
    QualityGrade grade        = QualityGrade::Rejected;
    double       overallScore = 0.0;
    bool IsAcceptable() const noexcept { return grade >= QualityGrade::Acceptable; }
};

struct QualityThresholds {
    double minSSIM   = 0.90;
    double minPSNR   = 30.0;
    double maxDeltaE = 3.0;
};

class ThumbnailQualityAnalyzer {
public:
    static size_t MetricCount() noexcept { return 8; }  // canonical 8-metric count
    static const wchar_t* MetricName(QualityMetric m) noexcept {
        switch (m) {
        case QualityMetric::SSIM:                return L"Structural Similarity (SSIM)";
        case QualityMetric::PSNR:                return L"Peak SNR (dB)";
        case QualityMetric::Sharpness:           return L"Sharpness (Laplacian)";
        case QualityMetric::Clarity:             return L"Clarity";
        case QualityMetric::ColorAccuracy:       return L"Color Accuracy";
        case QualityMetric::Saturation:          return L"Saturation";
        case QualityMetric::NoiseLevel:          return L"Noise Level";
        case QualityMetric::CompressionArtifact: return L"Compression Artifact";
        default: return L"Unknown";
        }
    }
    static QualityGrade GradeFromSSIM(double ssim) noexcept {
        if (ssim >= 0.95) return QualityGrade::Excellent;
        if (ssim >= 0.90) return QualityGrade::Good;
        if (ssim >= 0.83) return QualityGrade::Acceptable;
        if (ssim >= 0.70) return QualityGrade::Poor;
        return QualityGrade::Rejected;
    }
    static const wchar_t* GradeName(QualityGrade g) noexcept {
        switch (g) {
        case QualityGrade::Excellent:  return L"Excellent";
        case QualityGrade::Good:       return L"Good";
        case QualityGrade::Acceptable: return L"Acceptable";
        case QualityGrade::Poor:       return L"Poor";
        case QualityGrade::Rejected:   return L"Rejected";
        default: return L"Unknown";
        }
    }
};

// -- Validator thresholds (from ThumbnailQualityValidator.h) ------------------

struct QualityValidatorThresholds {
    float minBrightness    = 0.05f;
    float maxBrightness    = 0.95f;
    float minSharpness     = 20.0f;
    float minOverallScore  = 0.3f;
    float maxColorCast     = 0.4f;
};

struct QualityValidationResult {
    bool  passed       = true;
    float overallScore = 1.0f;
    QualityCheckFlag flags = QualityCheckFlag::None;
};

class ThumbnailQualityValidator {
public:
    void SetThresholds(const QualityValidatorThresholds& t) noexcept { m_thresholds = t; }
    const QualityValidatorThresholds& GetThresholds() const noexcept { return m_thresholds; }

    QualityValidationResult Validate(const uint8_t* rgba, uint32_t w, uint32_t h,
                                     uint32_t /*stride*/) const {
        QualityValidationResult r;
        if (!rgba || w == 0 || h == 0) { r.passed = false; r.overallScore = 0.0f; return r; }
        size_t bytes = static_cast<size_t>(w) * h * 4;
        // Blank / solid-color check
        uint8_t r0 = rgba[0], g0 = rgba[1], b0 = rgba[2];
        bool blank = true;
        for (size_t i = 4; i < bytes && blank; i += 4) {
            if (rgba[i] != r0 || rgba[i + 1] != g0 || rgba[i + 2] != b0) blank = false;
        }
        if (blank) {
            r.flags = r.flags | QualityCheckFlag::IsBlank;
            float lum = (0.2126f * r0 + 0.7152f * g0 + 0.0722f * b0) / 255.0f;
            if (lum < m_thresholds.minBrightness || lum > m_thresholds.maxBrightness)
                r.passed = false;
            r.overallScore = lum;
        }
        return r;
    }

private:
    QualityValidatorThresholds m_thresholds;
};

} // namespace Engine
} // namespace ExplorerLens
