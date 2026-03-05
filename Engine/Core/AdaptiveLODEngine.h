// AdaptiveLODEngine.h — Level-of-Detail Adaptation
// Copyright (c) 2026 ExplorerLens Project
//
// Level-of-detail adaptation based on display DPI. Selects decode resolution
// based on screen DPI to avoid wasting compute on invisible detail.
//
#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>
#include <cmath>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class LODLevel : uint8_t {
    Thumbnail = 0,
    Low = 1,
    Medium = 2,
    High = 3,
    Ultra = 4,
    Full = 5
};

struct DisplayProfile {
    uint32_t screenWidthPx = 1920;
    uint32_t screenHeightPx = 1080;
    float dpi = 96.0f;
    float scaleFactor = 1.0f;
    bool isHDR = false;

    inline float GetEffectiveDPI() const {
        return dpi * scaleFactor;
    }
};

struct LODRequest {
    uint32_t sourceWidth = 0;
    uint32_t sourceHeight = 0;
    uint32_t thumbnailWidth = 256;
    uint32_t thumbnailHeight = 256;
    DisplayProfile display;
};

struct LODResult {
    uint32_t decodeWidth = 0;
    uint32_t decodeHeight = 0;
    LODLevel level = LODLevel::Medium;
    float qualityMultiplier = 1.0f;
    double memorySavings = 0.0;
};

class AdaptiveLODEngine {
public:
    static AdaptiveLODEngine& Instance() {
        static AdaptiveLODEngine instance;
        return instance;
    }

    inline LODResult ComputeOptimalLOD(const LODRequest& request) const {
        LODResult result;

        float effectiveDPI = request.display.GetEffectiveDPI();
        float dpiRatio = effectiveDPI / 96.0f;

        uint32_t effectiveThumbW = static_cast<uint32_t>(request.thumbnailWidth * dpiRatio);
        uint32_t effectiveThumbH = static_cast<uint32_t>(request.thumbnailHeight * dpiRatio);

        float scaleX = static_cast<float>(effectiveThumbW) / (std::max)(1u, request.sourceWidth);
        float scaleY = static_cast<float>(effectiveThumbH) / (std::max)(1u, request.sourceHeight);
        float scale = (std::max)(scaleX, scaleY);

        result.level = SelectLODLevel(scale);
        result.qualityMultiplier = GetQualityMultiplier(result.level);

        float decodeScale = GetDecodeScale(result.level);
        result.decodeWidth = (std::max)(1u, static_cast<uint32_t>(request.sourceWidth * decodeScale));
        result.decodeHeight = (std::max)(1u, static_cast<uint32_t>(request.sourceHeight * decodeScale));

        result.decodeWidth = (std::min)(result.decodeWidth, request.sourceWidth);
        result.decodeHeight = (std::min)(result.decodeHeight, request.sourceHeight);

        double fullPixels = static_cast<double>(request.sourceWidth) * request.sourceHeight;
        double decodedPixels = static_cast<double>(result.decodeWidth) * result.decodeHeight;
        result.memorySavings = fullPixels > 0 ? (1.0 - decodedPixels / fullPixels) * 100.0 : 0.0;

        return result;
    }

    inline LODLevel SelectLODLevel(float displayScale) const {
        if (displayScale < 0.03f) return LODLevel::Thumbnail;
        if (displayScale < 0.10f) return LODLevel::Low;
        if (displayScale < 0.30f) return LODLevel::Medium;
        if (displayScale < 0.60f) return LODLevel::High;
        if (displayScale < 0.90f) return LODLevel::Ultra;
        return LODLevel::Full;
    }

    inline std::string LODLevelToString(LODLevel level) const {
        switch (level) {
        case LODLevel::Thumbnail: return "Thumbnail";
        case LODLevel::Low:       return "Low";
        case LODLevel::Medium:    return "Medium";
        case LODLevel::High:      return "High";
        case LODLevel::Ultra:     return "Ultra";
        case LODLevel::Full:      return "Full";
        default:                  return "Unknown";
        }
    }

    inline float GetDecodeScale(LODLevel level) const {
        switch (level) {
        case LODLevel::Thumbnail: return 0.03125f;
        case LODLevel::Low:       return 0.125f;
        case LODLevel::Medium:    return 0.25f;
        case LODLevel::High:      return 0.5f;
        case LODLevel::Ultra:     return 0.75f;
        case LODLevel::Full:      return 1.0f;
        default:                  return 0.25f;
        }
    }

private:
    AdaptiveLODEngine() = default;

    inline float GetQualityMultiplier(LODLevel level) const {
        switch (level) {
        case LODLevel::Thumbnail: return 0.5f;
        case LODLevel::Low:       return 0.7f;
        case LODLevel::Medium:    return 0.85f;
        case LODLevel::High:      return 0.95f;
        case LODLevel::Ultra:     return 0.98f;
        case LODLevel::Full:      return 1.0f;
        default:                  return 0.85f;
        }
    }
};

}
} // namespace ExplorerLens::Engine
