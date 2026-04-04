//==============================================================================
// ExplorerLens Engine — Scene Understanding Engine
// Deep-learning scene classification with object detection, dominant scene
// category labeling, and content-aware thumbnail crop region suggestion.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class SceneCategory : uint8_t {
    Nature = 0,
    Architecture,
    People,
    Food,
    Animals,
    Vehicle,
    Text,
    Abstract,
    Technology,
    Sports,
    Indoor = Architecture,  // compat alias
    COUNT = Sports + 1
};
enum class SceneMLBackend : uint8_t {
    DirectML = 0,
    ONNX,
    OpenVINO,
    CPU,
    COUNT
};
enum class SceneConfidence : uint8_t {
    VeryLow = 0,
    Low,
    Medium,
    High,
    VeryHigh,
    COUNT
};

struct SceneClassification
{
    SceneCategory category = SceneCategory::Abstract;
    SceneConfidence confidence = SceneConfidence::Low;
    float score = 0.0f;
    std::wstring label;
};

struct SceneUnderstandingResult
{
    SceneClassification primaryScene;
    std::vector<SceneClassification> alternateScenes;
    SceneMLBackend backend = SceneMLBackend::DirectML;
    uint32_t inferenceMs = 0;
    bool hasFaces = false;
    bool hasText = false;
};

class SceneUnderstandingEngine
{
  public:
    static const wchar_t* CategoryName(SceneCategory c)
    {
        switch (c) {
            case SceneCategory::Nature:
                return L"Nature";
            case SceneCategory::Architecture:
                return L"Architecture";
            case SceneCategory::People:
                return L"People";
            case SceneCategory::Food:
                return L"Food";
            case SceneCategory::Animals:
                return L"Animals";
            case SceneCategory::Vehicle:
                return L"Vehicle";
            case SceneCategory::Text:
                return L"Text/Document";
            case SceneCategory::Abstract:
                return L"Abstract";
            case SceneCategory::Technology:
                return L"Technology";
            case SceneCategory::Sports:
                return L"Sports";
            default:
                return L"Unknown";
        }
    }
    static const wchar_t* BackendName(SceneMLBackend b)
    {
        switch (b) {
            case SceneMLBackend::DirectML:
                return L"DirectML";
            case SceneMLBackend::ONNX:
                return L"ONNX Runtime";
            case SceneMLBackend::OpenVINO:
                return L"OpenVINO";
            case SceneMLBackend::CPU:
                return L"CPU";
            default:
                return L"Unknown";
        }
    }
    static const wchar_t* ConfidenceName(SceneConfidence c)
    {
        switch (c) {
            case SceneConfidence::VeryLow:
                return L"Very Low (<20%)";
            case SceneConfidence::Low:
                return L"Low (20-40%)";
            case SceneConfidence::Medium:
                return L"Medium (40-60%)";
            case SceneConfidence::High:
                return L"High (60-80%)";
            case SceneConfidence::VeryHigh:
                return L"Very High (>80%)";
            default:
                return L"Unknown";
        }
    }
    static constexpr size_t CategoryCount()
    {
        return static_cast<size_t>(SceneCategory::COUNT);
    }
    static constexpr size_t BackendCount()
    {
        return static_cast<size_t>(SceneMLBackend::COUNT);
    }
    static constexpr size_t ConfidenceCount()
    {
        return static_cast<size_t>(SceneConfidence::COUNT);
    }

    //==========================================================================
    // Scene Classification — Color histogram heuristics (CPU, no ML required)
    //==========================================================================

    /// Classify a scene based on color distribution heuristics.
    /// Input: RGB pixel buffer (3 bytes per pixel), width, height, stride.
    /// Uses color channel statistics and edge density as features.
    static SceneClassification ClassifyByHeuristics(const uint8_t* rgb, uint32_t width, uint32_t height, uint32_t stride)
    {
        SceneClassification result;
        result.category = SceneCategory::Abstract;
        result.confidence = SceneConfidence::Low;
        result.score = 0.3f;
        if (!rgb || width < 4 || height < 4)
            return result;

        // Compute channel means and green ratio
        uint64_t sumR = 0, sumG = 0, sumB = 0;
        uint32_t pixelCount = width * height;
        for (uint32_t y = 0; y < height; ++y) {
            const uint8_t* row = rgb + y * stride;
            for (uint32_t x = 0; x < width; ++x) {
                sumR += row[x * 3 + 0];
                sumG += row[x * 3 + 1];
                sumB += row[x * 3 + 2];
            }
        }
        double meanR = static_cast<double>(sumR) / pixelCount;
        double meanG = static_cast<double>(sumG) / pixelCount;
        double meanB = static_cast<double>(sumB) / pixelCount;
        double brightness = (meanR + meanG + meanB) / 3.0;
        double greenRatio = meanG / (brightness + 1.0);
        double blueRatio = meanB / (brightness + 1.0);
        double saturation = 0;
        {
            double maxC = meanR > meanG ? (meanR > meanB ? meanR : meanB) : (meanG > meanB ? meanG : meanB);
            double minC = meanR < meanG ? (meanR < meanB ? meanR : meanB) : (meanG < meanB ? meanG : meanB);
            if (maxC > 0)
                saturation = (maxC - minC) / maxC;
        }

        // Compute edge density (simplified Sobel on grayscale)
        uint32_t edgePixels = 0;
        for (uint32_t y = 1; y < height - 1; ++y) {
            for (uint32_t x = 1; x < width - 1; ++x) {
                auto luma = [&](uint32_t px, uint32_t py) -> int {
                    const uint8_t* p = rgb + py * stride + px * 3;
                    return (p[0] * 77 + p[1] * 150 + p[2] * 29) >> 8;
                };
                int gx = luma(x + 1, y) - luma(x - 1, y);
                int gy = luma(x, y + 1) - luma(x, y - 1);
                if (gx * gx + gy * gy > 2500)
                    ++edgePixels;
            }
        }
        double edgeDensity = static_cast<double>(edgePixels) / ((width - 2) * (height - 2));

        // Heuristic classification rules
        if (greenRatio > 1.2 && saturation > 0.2) {
            result.category = SceneCategory::Nature;
            result.label = L"Nature (high green)";
            result.score = static_cast<float>(greenRatio * 0.5);
        } else if (blueRatio > 1.3 && saturation > 0.15 && edgeDensity < 0.15) {
            result.category = SceneCategory::Nature;
            result.label = L"Sky/Water";
            result.score = static_cast<float>(blueRatio * 0.4);
        } else if (edgeDensity > 0.4 && saturation < 0.15) {
            result.category = SceneCategory::Text;
            result.label = L"Text/Document";
            result.score = static_cast<float>(edgeDensity);
        } else if (edgeDensity > 0.3 && saturation < 0.25) {
            result.category = SceneCategory::Architecture;
            result.label = L"Architecture (many edges)";
            result.score = static_cast<float>(edgeDensity * 0.8);
        } else if (saturation > 0.5 && brightness > 100 && brightness < 200) {
            result.category = SceneCategory::Food;
            result.label = L"Food (high saturation)";
            result.score = static_cast<float>(saturation);
        } else if (saturation < 0.1 && brightness < 50) {
            result.category = SceneCategory::Technology;
            result.label = L"Dark/Tech (low sat, dark)";
            result.score = 0.4f;
        } else {
            result.category = SceneCategory::Abstract;
            result.label = L"Abstract/Generic";
            result.score = 0.3f;
        }

        // Map score to confidence
        if (result.score > 0.7f)
            result.confidence = SceneConfidence::High;
        else if (result.score > 0.5f)
            result.confidence = SceneConfidence::Medium;
        else
            result.confidence = SceneConfidence::Low;

        return result;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
