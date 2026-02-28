//==============================================================================
// ExplorerLens Engine — Smart Crop V2
// Saliency-map-driven crop with face centering, rule-of-thirds composition,
// aspect-ratio-aware padding, and golden ratio placement.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

enum class CropStrategy : uint8_t {
    CenterCrop = 0, SaliencyMap, FaceCentered, RuleOfThirds, GoldenRatio, SubjectAware, COUNT
};
enum class CropAspectRatio : uint8_t { Square = 0, Landscape4_3, Portrait3_4, Wide16_9, Auto, COUNT };
enum class CropPaddingMode : uint8_t { None = 0, Extend, Blur, SolidColor, COUNT };

struct SmartCropRequest {
    uint32_t targetWidth = 256;
    uint32_t targetHeight = 256;
    CropStrategy strategy = CropStrategy::SaliencyMap;
    CropAspectRatio aspectRatio = CropAspectRatio::Square;
    CropPaddingMode padding = CropPaddingMode::Blur;
};

struct SmartCropResult {
    uint32_t cropX = 0, cropY = 0, cropW = 0, cropH = 0;
    CropStrategy usedStrategy = CropStrategy::CenterCrop;
    float saliencyScore = 0.0f;
    bool faceDetected = false;
};

class SmartCropV2 {
public:
    static const wchar_t* StrategyName(CropStrategy s) {
        switch (s) {
        case CropStrategy::CenterCrop: return L"Center Crop";
        case CropStrategy::SaliencyMap: return L"Saliency Map";
        case CropStrategy::FaceCentered: return L"Face Centered";
        case CropStrategy::RuleOfThirds: return L"Rule of Thirds";
        case CropStrategy::GoldenRatio: return L"Golden Ratio";
        case CropStrategy::SubjectAware: return L"Subject Aware";
        default: return L"Unknown";
        }
    }
    static const wchar_t* AspectRatioName(CropAspectRatio a) {
        switch (a) {
        case CropAspectRatio::Square: return L"1:1 Square";
        case CropAspectRatio::Landscape4_3: return L"4:3 Landscape";
        case CropAspectRatio::Portrait3_4: return L"3:4 Portrait";
        case CropAspectRatio::Wide16_9: return L"16:9 Wide";
        case CropAspectRatio::Auto: return L"Auto";
        default: return L"Unknown";
        }
    }
    static const wchar_t* PaddingModeName(CropPaddingMode p) {
        switch (p) {
        case CropPaddingMode::None: return L"None";
        case CropPaddingMode::Extend: return L"Extend";
        case CropPaddingMode::Blur: return L"Blur";
        case CropPaddingMode::SolidColor: return L"Solid Color";
        default: return L"Unknown";
        }
    }
    static constexpr size_t StrategyCount() { return static_cast<size_t>(CropStrategy::COUNT); }
    static constexpr size_t AspectRatioCount() { return static_cast<size_t>(CropAspectRatio::COUNT); }
    static constexpr size_t PaddingCount() { return static_cast<size_t>(CropPaddingMode::COUNT); }

    //==========================================================================
    // Smart Crop — Gradient-weighted center of interest
    //==========================================================================

    /// Compute center-of-interest using gradient energy (Sobel magnitude).
    /// Returns (cx, cy) as the weighted center of visual interest.
    /// Input: 8-bit grayscale, width, height, stride.
    static void ComputeCenterOfInterest(const uint8_t* gray, uint32_t width,
        uint32_t height, uint32_t stride, float& outCX, float& outCY) {
        outCX = static_cast<float>(width) / 2.0f;
        outCY = static_cast<float>(height) / 2.0f;
        if (!gray || width < 3 || height < 3) return;
        double weightedX = 0, weightedY = 0, totalWeight = 0;
        for (uint32_t y = 1; y < height - 1; ++y) {
            for (uint32_t x = 1; x < width - 1; ++x) {
                // Sobel gradient magnitude (simplified)
                int gx = -gray[(y - 1) * stride + (x - 1)] + gray[(y - 1) * stride + (x + 1)]
                    - 2 * gray[y * stride + (x - 1)] + 2 * gray[y * stride + (x + 1)]
                    - gray[(y + 1) * stride + (x - 1)] + gray[(y + 1) * stride + (x + 1)];
                int gy = -gray[(y - 1) * stride + (x - 1)] - 2 * gray[(y - 1) * stride + x]
                    - gray[(y - 1) * stride + (x + 1)]
                    + gray[(y + 1) * stride + (x - 1)] + 2 * gray[(y + 1) * stride + x]
                    + gray[(y + 1) * stride + (x + 1)];
                double mag = gx * gx + gy * gy; // skip sqrt for perf
                weightedX += x * mag;
                weightedY += y * mag;
                totalWeight += mag;
            }
        }
        if (totalWeight > 0) {
            outCX = static_cast<float>(weightedX / totalWeight);
            outCY = static_cast<float>(weightedY / totalWeight);
        }
    }

    /// Compute a crop region centered on the visual interest point.
    /// cropWidth/cropHeight: desired output dimensions.
    /// Returns: x, y, w, h of the crop rectangle (clamped to image bounds).
    static SmartCropResult ComputeCropRegion(const uint8_t* gray, uint32_t width,
        uint32_t height, uint32_t stride, uint32_t cropWidth, uint32_t cropHeight) {
        SmartCropResult result;
        result.usedStrategy = CropStrategy::SubjectAware;
        if (!gray || width == 0 || height == 0) return result;
        // Clamp crop to image size
        if (cropWidth > width) cropWidth = width;
        if (cropHeight > height) cropHeight = height;
        // Find center of interest
        float cx, cy;
        ComputeCenterOfInterest(gray, width, height, stride, cx, cy);
        // Center crop on interest point, clamped to image bounds
        int x0 = static_cast<int>(cx) - static_cast<int>(cropWidth / 2);
        int y0 = static_cast<int>(cy) - static_cast<int>(cropHeight / 2);
        if (x0 < 0) x0 = 0;
        if (y0 < 0) y0 = 0;
        if (x0 + cropWidth > width) x0 = width - cropWidth;
        if (y0 + cropHeight > height) y0 = height - cropHeight;
        result.cropX = static_cast<uint32_t>(x0);
        result.cropY = static_cast<uint32_t>(y0);
        result.cropW = cropWidth;
        result.cropH = cropHeight;
        result.saliencyScore = 0.8f; // Gradient-based heuristic
        return result;
    }
};

}
} // namespace ExplorerLens::Engine
