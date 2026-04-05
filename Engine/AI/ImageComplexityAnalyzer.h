// ImageComplexityAnalyzer.h — Image Complexity & Decode Cost Estimation
// Copyright (c) 2026 ExplorerLens Project
//
// Estimates image decode complexity before full decoding. Analyzes file
// headers to predict decode time, memory requirements, and optimal
// strategy. Used by the scheduler to prioritize and allocate resources.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ComplexityLevel : uint8_t {
    Trivial,
    Low,
    Medium,
    High,
    Extreme
};

struct ComplexityEstimate
{
    ComplexityLevel level = ComplexityLevel::Medium;
    double score = 5.0;  // 0-10
    double estimatedDecodeMs = 10.0;
    uint64_t estimatedMemoryBytes = 0;
    bool needsGPU = false;
    bool needsMultiThread = false;
    uint32_t recommendedThreads = 1;
    std::string rationale;
};

struct ComplexityStats
{
    uint32_t filesAnalyzed = 0;
    uint32_t trivialCount = 0;
    uint32_t extremeCount = 0;
    double avgScore = 0.0;
    double estimateAccuracy = 0.0;  // How close estimates are vs actual
};

class ImageComplexityAnalyzer
{
public:
    ImageComplexityAnalyzer() = default;
    ~ImageComplexityAnalyzer() = default;

    static const wchar_t* GetName() { return L"ImageComplexityAnalyzer"; }

    /// Estimate complexity from image dimensions and format.
    ComplexityEstimate Estimate(uint32_t width, uint32_t height, uint32_t bitsPerPixel, const std::string& format) const
    {
        ComplexityEstimate est;
        uint64_t pixelCount = static_cast<uint64_t>(width) * height;
        uint64_t rawBytes = pixelCount * bitsPerPixel / 8;

        // Memory estimate: raw + working buffer
        est.estimatedMemoryBytes = rawBytes * 2;

        // Base decode complexity from pixel count
        double pixelComplexity = std::log2(static_cast<double>(pixelCount + 1)) / 24.0;

        // Format-specific multipliers
        double formatMultiplier = 1.0;
        if (format == "JPEG-XL" || format == "JXL")
            formatMultiplier = 3.0;
        else if (format == "HEIF" || format == "HEIC")
            formatMultiplier = 2.5;
        else if (format == "AVIF")
            formatMultiplier = 2.5;
        else if (format == "RAW" || format == "CR2" || format == "NEF")
            formatMultiplier = 4.0;
        else if (format == "TIFF")
            formatMultiplier = 1.5;
        else if (format == "BMP" || format == "PNG")
            formatMultiplier = 1.0;
        else if (format == "JPEG" || format == "JPG")
            formatMultiplier = 0.8;
        else if (format == "GIF" || format == "ICO")
            formatMultiplier = 0.5;
        else if (format == "PDF")
            formatMultiplier = 5.0;
        else if (format == "glTF" || format == "FBX" || format == "OBJ")
            formatMultiplier = 8.0;

        est.score = std::clamp(pixelComplexity * formatMultiplier * 10.0, 0.0, 10.0);

        // Decode time estimation (empirical model)
        est.estimatedDecodeMs = 0.5 + (pixelCount / 1000000.0) * formatMultiplier * 2.0;

        // Resource recommendations
        if (pixelCount > 4000000 || est.score > 6.0) {
            est.needsMultiThread = true;
            est.recommendedThreads = std::min(4u, std::max(2u, static_cast<uint32_t>(pixelCount / 4000000) + 1));
        }
        if (est.score > 7.0 || pixelCount > 16000000) {
            est.needsGPU = true;
        }

        // Classify
        if (est.score < 1.0)
            est.level = ComplexityLevel::Trivial;
        else if (est.score < 3.0)
            est.level = ComplexityLevel::Low;
        else if (est.score < 6.0)
            est.level = ComplexityLevel::Medium;
        else if (est.score < 8.5)
            est.level = ComplexityLevel::High;
        else
            est.level = ComplexityLevel::Extreme;

        m_stats.filesAnalyzed++;
        m_stats.avgScore = (m_stats.avgScore * (m_stats.filesAnalyzed - 1) + est.score) / m_stats.filesAnalyzed;
        if (est.level == ComplexityLevel::Trivial)
            m_stats.trivialCount++;
        if (est.level == ComplexityLevel::Extreme)
            m_stats.extremeCount++;

        return est;
    }

    static const char* LevelName(ComplexityLevel level)
    {
        switch (level) {
            case ComplexityLevel::Trivial:
                return "Trivial";
            case ComplexityLevel::Low:
                return "Low";
            case ComplexityLevel::Medium:
                return "Medium";
            case ComplexityLevel::High:
                return "High";
            case ComplexityLevel::Extreme:
                return "Extreme";
            default:
                return "Unknown";
        }
    }

    ComplexityStats GetStats() const { return m_stats; }

private:
    mutable ComplexityStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
