// AdaptiveContrastEnhancer.h — Adaptive Contrast Enhancement for Thumbnails
// Copyright (c) 2026 ExplorerLens Project
//
// Provides adaptive contrast enhancement for dark or low-contrast thumbnails.
// Uses histogram analysis and linear contrast stretching with pure math on
// pixel values (no GDI/Direct2D dependency).
//
#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

// Parameters for linear contrast stretch mapping
struct ContrastStretchParams
{
    double scale;    // Multiply factor
    double offset;   // Additive offset after scale
    uint8_t inMin;   // Input range minimum
    uint8_t inMax;   // Input range maximum
    uint8_t outMin;  // Output range minimum
    uint8_t outMax;  // Output range maximum
};

// Result of histogram analysis
struct HistogramAnalysis
{
    double avgBrightness;  // 0.0 - 255.0
    double contrast;       // Standard deviation of luminance
    uint8_t minLuminance;
    uint8_t maxLuminance;
    bool valid;
};

// Adaptive contrast enhancement for thumbnail images.
// Analyzes pixel luminance distribution and applies linear contrast
// stretching to improve visibility of dark or washed-out thumbnails.
class AdaptiveContrastEnhancer
{
  public:
    AdaptiveContrastEnhancer() : m_brightnessThreshold(80.0), m_contrastThreshold(40.0) {}

    // Analyze the luminance histogram of a pixel buffer (BGRA format, 4 bytes/pixel).
    // Returns brightness and contrast statistics.
    HistogramAnalysis AnalyzeHistogram(const uint8_t* pixels, uint32_t w, uint32_t h) const
    {
        HistogramAnalysis result = {};
        result.valid = false;

        if (!pixels || w == 0 || h == 0)
            return result;

        const size_t pixelCount = static_cast<size_t>(w) * h;
        double sumLum = 0.0;
        uint8_t minLum = 255;
        uint8_t maxLum = 0;

        for (size_t i = 0; i < pixelCount; ++i) {
            const uint8_t b = pixels[i * 4 + 0];
            const uint8_t g = pixels[i * 4 + 1];
            const uint8_t r = pixels[i * 4 + 2];
            // ITU-R BT.601 luminance
            uint8_t lum = static_cast<uint8_t>((std::min)(255.0, 0.299 * r + 0.587 * g + 0.114 * b));
            sumLum += lum;
            minLum = (std::min)(minLum, lum);
            maxLum = (std::max)(maxLum, lum);
        }

        result.avgBrightness = sumLum / static_cast<double>(pixelCount);
        result.minLuminance = minLum;
        result.maxLuminance = maxLum;

        // Compute standard deviation as contrast measure
        double sumSqDiff = 0.0;
        for (size_t i = 0; i < pixelCount; ++i) {
            const uint8_t b = pixels[i * 4 + 0];
            const uint8_t g = pixels[i * 4 + 1];
            const uint8_t r = pixels[i * 4 + 2];
            double lum = (std::min)(255.0, 0.299 * r + 0.587 * g + 0.114 * b);
            double diff = lum - result.avgBrightness;
            sumSqDiff += diff * diff;
        }
        result.contrast = std::sqrt(sumSqDiff / static_cast<double>(pixelCount));
        result.valid = true;
        return result;
    }

    // Compute linear contrast stretch parameters to map [inMin, inMax] -> [targetMin, targetMax]
    static ContrastStretchParams ComputeStretchParams(uint8_t minVal, uint8_t maxVal, uint8_t targetMin,
                                                      uint8_t targetMax)
    {
        ContrastStretchParams params = {};
        params.inMin = minVal;
        params.inMax = maxVal;
        params.outMin = targetMin;
        params.outMax = targetMax;

        double inRange = static_cast<double>(maxVal) - static_cast<double>(minVal);
        double outRange = static_cast<double>(targetMax) - static_cast<double>(targetMin);

        if (inRange < 1.0) {
            // Degenerate case: all pixels same value
            params.scale = 1.0;
            params.offset = static_cast<double>(targetMin) - static_cast<double>(minVal);
        } else {
            params.scale = outRange / inRange;
            params.offset = static_cast<double>(targetMin) - params.scale * static_cast<double>(minVal);
        }
        return params;
    }

    // Apply contrast stretch to a single channel value
    static uint8_t ApplyContrastStretch(uint8_t pixel, const ContrastStretchParams& params)
    {
        double mapped = params.scale * static_cast<double>(pixel) + params.offset;
        mapped = (std::max)(0.0, (std::min)(255.0, mapped));
        return static_cast<uint8_t>(mapped + 0.5);
    }

    // Determine if the image needs contrast enhancement based on brightness and contrast
    bool NeedsEnhancement(double avgBrightness, double contrast) const
    {
        // Enhance if too dark or too low contrast
        if (avgBrightness < m_brightnessThreshold)
            return true;
        if (contrast < m_contrastThreshold)
            return true;
        return false;
    }

    // Compute a simple brightness score (0.0 - 1.0) from a BGRA pixel buffer
    static double GetBrightnessScore(const uint8_t* pixels, uint32_t w, uint32_t h)
    {
        if (!pixels || w == 0 || h == 0)
            return 0.0;

        const size_t pixelCount = static_cast<size_t>(w) * h;
        double sumLum = 0.0;

        for (size_t i = 0; i < pixelCount; ++i) {
            const uint8_t b = pixels[i * 4 + 0];
            const uint8_t g = pixels[i * 4 + 1];
            const uint8_t r = pixels[i * 4 + 2];
            sumLum += 0.299 * r + 0.587 * g + 0.114 * b;
        }

        double avg = sumLum / static_cast<double>(pixelCount);
        return (std::max)(0.0, (std::min)(1.0, avg / 255.0));
    }

    // Configuration
    void SetBrightnessThreshold(double t)
    {
        m_brightnessThreshold = t;
    }
    void SetContrastThreshold(double t)
    {
        m_contrastThreshold = t;
    }

  private:
    double m_brightnessThreshold;
    double m_contrastThreshold;
};

}  // namespace Engine
}  // namespace ExplorerLens
