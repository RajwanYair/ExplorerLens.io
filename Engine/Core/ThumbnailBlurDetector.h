// ThumbnailBlurDetector.h — Image Blur Detection for Thumbnails
// Copyright (c) 2026 ExplorerLens Project
//
// Detects blurry images to flag low-quality thumbnails using Laplacian
// variance and directional gradient analysis. Pure math on pixel arrays
// with no external dependencies — works on grayscale uint8 buffers.
//
#pragma once

#include <windows.h>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// Sharpness classification levels
enum class SharpnessLevel : int {
    VeryBlurry = 0,   // variance < 50
    Blurry = 1,   // variance < 200
    Acceptable = 2,   // variance < 500
    Sharp = 3,   // variance < 1000
    VerySharp = 4    // variance >= 1000
};

// Result of a blur detection analysis
struct BlurAnalysisResult {
    double laplacianVariance = 0.0;
    double horizontalEnergy = 0.0;
    double verticalEnergy = 0.0;
    SharpnessLevel level = SharpnessLevel::VeryBlurry;
    bool isBlurry = true;
    bool motionBlurDetected = false;
};

class ThumbnailBlurDetector {
public:
    ThumbnailBlurDetector() = default;
    ~ThumbnailBlurDetector() = default;

    // Computes Laplacian variance on a grayscale image buffer.
    // pixels: pointer to row-major uint8 grayscale data
    // w, h: image dimensions; stride: bytes per row (>= w)
    // Returns variance value (higher = sharper). Returns -1.0 on invalid input.
    double ComputeLaplacianVariance(const uint8_t* pixels, int w, int h, int stride) const {
        if (!pixels || w < 3 || h < 3 || stride < w)
            return -1.0;

        // Laplacian kernel: [0, 1, 0; 1, -4, 1; 0, 1, 0]
        double sum = 0.0;
        double sumSq = 0.0;
        const int count = (w - 2) * (h - 2);

        for (int y = 1; y < h - 1; ++y) {
            for (int x = 1; x < w - 1; ++x) {
                const int center = pixels[y * stride + x];
                const int top = pixels[(y - 1) * stride + x];
                const int bottom = pixels[(y + 1) * stride + x];
                const int left = pixels[y * stride + (x - 1)];
                const int right = pixels[y * stride + (x + 1)];

                const double lap = static_cast<double>(top + bottom + left + right - 4 * center);
                sum += lap;
                sumSq += lap * lap;
            }
        }

        if (count <= 0) return 0.0;
        const double mean = sum / count;
        return (sumSq / count) - (mean * mean);
    }

    // Returns true if the image is considered blurry at the given threshold.
    // Default threshold of 200.0 is suitable for typical thumbnails.
    bool IsBlurry(const uint8_t* pixels, int w, int h, double threshold = 200.0) const {
        if (threshold < 0.0) threshold = 200.0;
        const double variance = ComputeLaplacianVariance(pixels, w, h, w);
        if (variance < 0.0) return true; // invalid input treated as blurry
        return variance < threshold;
    }

    // Maps a Laplacian variance value to a sharpness level.
    static SharpnessLevel GetSharpnessScore(double variance) {
        if (variance < 50.0)   return SharpnessLevel::VeryBlurry;
        if (variance < 200.0)  return SharpnessLevel::Blurry;
        if (variance < 500.0)  return SharpnessLevel::Acceptable;
        if (variance < 1000.0) return SharpnessLevel::Sharp;
        return SharpnessLevel::VerySharp;
    }

    // Detects motion blur by comparing horizontal vs vertical gradient energy.
    // A strong directional imbalance suggests motion blur.
    // Returns true if motion blur is detected. Returns false on invalid input.
    bool DetectMotionBlur(const uint8_t* pixels, int w, int h) const {
        if (!pixels || w < 3 || h < 3)
            return false;

        double hEnergy = 0.0;
        double vEnergy = 0.0;
        const int stride = w;

        for (int y = 1; y < h - 1; ++y) {
            for (int x = 1; x < w - 1; ++x) {
                const double gx = static_cast<double>(
                    pixels[y * stride + (x + 1)]) - pixels[y * stride + (x - 1)];
                const double gy = static_cast<double>(
                    pixels[(y + 1) * stride + x]) - pixels[(y - 1) * stride + x];
                hEnergy += gx * gx;
                vEnergy += gy * gy;
            }
        }

        // Motion blur typically shows much stronger energy in one direction
        const double total = hEnergy + vEnergy;
        if (total < 1.0) return false; // flat image

        const double ratio = (hEnergy > vEnergy)
            ? (hEnergy / (std::max)(1.0, vEnergy))
            : (vEnergy / (std::max)(1.0, hEnergy));

        return ratio > m_motionBlurRatioThreshold;
    }

    // Performs full blur analysis on a grayscale buffer.
    BlurAnalysisResult Analyze(const uint8_t* pixels, int w, int h, double blurThreshold = 200.0) const {
        BlurAnalysisResult result;
        result.laplacianVariance = ComputeLaplacianVariance(pixels, w, h, w);
        result.isBlurry = (result.laplacianVariance < 0.0) || (result.laplacianVariance < blurThreshold);
        result.level = GetSharpnessScore(result.laplacianVariance);
        result.motionBlurDetected = DetectMotionBlur(pixels, w, h);

        // Compute directional energies
        if (pixels && w >= 3 && h >= 3) {
            for (int y = 1; y < h - 1; ++y) {
                for (int x = 1; x < w - 1; ++x) {
                    const double gx = static_cast<double>(
                        pixels[y * w + (x + 1)]) - pixels[y * w + (x - 1)];
                    const double gy = static_cast<double>(
                        pixels[(y + 1) * w + x]) - pixels[(y - 1) * w + x];
                    result.horizontalEnergy += gx * gx;
                    result.verticalEnergy += gy * gy;
                }
            }
        }

        return result;
    }

    // Sets the motion blur direction ratio threshold. Returns false if <= 1.0.
    bool SetMotionBlurThreshold(double ratio) {
        if (ratio <= 1.0) return false;
        m_motionBlurRatioThreshold = ratio;
        return true;
    }

private:
    double m_motionBlurRatioThreshold = 3.0;
};

} // namespace Engine
} // namespace ExplorerLens
