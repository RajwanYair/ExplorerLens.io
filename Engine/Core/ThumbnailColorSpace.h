// ThumbnailColorSpace.h — Color space management for thumbnails
// Copyright (c) 2026 ExplorerLens Project
//
// Provides color space type handling and gamma mapping for thumbnail rendering.
//
#pragma once
#include <cstdint>
#include <algorithm>
#include "ThumbnailQuality.h"

namespace ExplorerLens {
namespace Engine {

inline const char* ColorSpaceTypeName(ColorSpaceType t) noexcept {
    switch (t) {
    case ColorSpaceType::SRGB:        return "sRGB";
    case ColorSpaceType::AdobeRGB:    return "AdobeRGB";
    case ColorSpaceType::DisplayP3:   return "DisplayP3";
    case ColorSpaceType::Rec2020:     return "Rec2020";
    case ColorSpaceType::ProPhotoRGB: return "ProPhotoRGB";
    default:                          return "Unknown";
    }
}

enum class GammaMapping : uint8_t { Linear = 0, Gamma22 = 1, PQ = 2, HLG = 3 };

inline const char* GammaMappingName(GammaMapping g) noexcept {
    switch (g) {
    case GammaMapping::Linear:  return "Linear";
    case GammaMapping::Gamma22: return "Gamma2.2";
    case GammaMapping::PQ:      return "PQ";
    case GammaMapping::HLG:     return "HLG";
    default:                    return "Unknown";
    }
}

struct ColorProfile {
    ColorSpaceType colorSpace = ColorSpaceType::SRGB;
    GammaMapping   gamma      = GammaMapping::Gamma22;
};

class ThumbnailColorSpace {
public:
    void SetWorkingSpace(ColorSpaceType type) noexcept { m_profile.colorSpace = type; }
    const ColorProfile& GetProfile() const noexcept { return m_profile; }

    bool Convert(float& r, float& g, float& b) noexcept {
        r = (std::max)(0.0f, (std::min)(1.0f, r));
        g = (std::max)(0.0f, (std::min)(1.0f, g));
        b = (std::max)(0.0f, (std::min)(1.0f, b));
        return true;
    }

private:
    ColorProfile m_profile;
};

} // namespace Engine
} // namespace ExplorerLens
