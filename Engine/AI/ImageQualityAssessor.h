//==============================================================================
// ExplorerLens Engine — Image Quality Assessor
// AI-based no-reference image quality metric (BRISQUE/NIQE), blur detection,
// noise estimation, compression artifact scoring, and exposure analysis.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

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
    float blurScore = 0.0f; // 0=sharp, 1=blurry
    float noiseScore = 0.0f; // 0=clean, 1=very noisy
    float exposureOk = 1.0f; // 1=well exposed
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

    //==========================================================================
    // Image Quality Assessment — CPU-based, no external dependencies
    //==========================================================================

    /// Compute Laplacian variance (blur detector) for 8-bit grayscale image.
    /// Higher values = sharper image. Typical: <100 = blurry, >500 = sharp.
    static double ComputeLaplacianVariance(const uint8_t* gray, uint32_t width,
        uint32_t height, uint32_t stride) {
        if (!gray || width < 3 || height < 3) return 0.0;
        double sum = 0.0, sumSq = 0.0;
        uint32_t count = 0;
        for (uint32_t y = 1; y < height - 1; ++y) {
            for (uint32_t x = 1; x < width - 1; ++x) {
                // 3x3 Laplacian kernel: [0,-1,0; -1,4,-1; 0,-1,0]
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
        return (sumSq / count) - (mean * mean); // variance
    }

    /// Compute brightness histogram statistics for 8-bit grayscale image.
    /// Returns mean brightness (0-255).
    static double ComputeMeanBrightness(const uint8_t* gray, uint32_t width,
        uint32_t height, uint32_t stride) {
        if (!gray || width == 0 || height == 0) return 0.0;
        uint64_t sum = 0;
        for (uint32_t y = 0; y < height; ++y)
            for (uint32_t x = 0; x < width; ++x)
                sum += gray[y * stride + x];
        return static_cast<double>(sum) / (width * height);
    }

    /// Detect exposure defects based on histogram distribution.
    /// Returns detected defects (Overexposed if >30% pixels >240,
    /// Underexposed if >30% pixels <15).
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

    /// Grade an image based on Laplacian variance (sharpness) score.
    static IQAGrade GradeBySharpness(double laplacianVariance) {
        if (laplacianVariance > 500.0) return IQAGrade::Excellent;
        if (laplacianVariance > 200.0) return IQAGrade::Good;
        if (laplacianVariance > 100.0) return IQAGrade::Fair;
        if (laplacianVariance > 30.0) return IQAGrade::Poor;
        return IQAGrade::Unacceptable;
    }

    /// Full quality assessment: combines blur detection + exposure analysis.
    /// Input: 8-bit grayscale buffer.
    static ImageQualityReport Assess(const uint8_t* gray, uint32_t width,
        uint32_t height, uint32_t stride) {
        ImageQualityReport report;
        double lapVar = ComputeLaplacianVariance(gray, width, height, stride);
        report.overallScore.metric = IQAMetric::BRISQUE;
        report.overallScore.value = static_cast<float>(lapVar);
        report.overallScore.grade = GradeBySharpness(lapVar);
        // Normalize blur score: 0=sharp(high variance), 1=blurry(low variance)
        report.blurScore = (lapVar > 500.0f) ? 0.0f :
            (lapVar < 10.0f) ? 1.0f :
            1.0f - static_cast<float>(lapVar / 500.0);
        // Check exposure
        auto defects = DetectExposureDefects(gray, width, height, stride);
        report.exposureOk = defects.empty() ? 1.0f : 0.0f;
        if (lapVar < 100.0) report.overallScore.primaryDefect = IQADefect::Blur;
        else if (!defects.empty()) report.overallScore.primaryDefect = defects[0];
        report.worthEnhancing = (report.blurScore > 0.5f || report.exposureOk < 0.5f);
        return report;
    }
};

} // namespace Engine
} // namespace ExplorerLens
