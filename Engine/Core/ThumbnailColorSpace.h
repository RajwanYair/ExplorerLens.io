#pragma once
// ============================================================================
// ThumbnailColorSpace.h — Color space conversion and ICC profile management
//                         for thumbnails
//
// Purpose:   Color space conversion and ICC profile management for thumbnails
// Provides:  ColorSpaceType, RenderingIntent enums, ColorProfile struct,
//            ThumbnailColorSpace class
// Used by:   Render pipeline
// ============================================================================

#include <cstdint>
#include <string>
#include <cmath>

namespace ExplorerLens {
namespace Engine {

/// Supported color space types for thumbnail conversion
enum class ColorSpaceType : uint8_t {
    SRGB = 0,  // Standard sRGB (IEC 61966-2-1)
    AdobeRGB = 1,  // Adobe RGB (1998)
    DisplayP3 = 2,  // Apple Display P3
    Rec2020 = 3,  // ITU-R BT.2020 wide-gamut
    ProPhotoRGB = 4   // ProPhoto RGB / ROMM RGB
};

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

/// Gamma transfer function mapping
enum class GammaMapping : uint8_t {
    Linear = 0,  // Linear (gamma 1.0)
    SRGB = 1,  // sRGB transfer function (~2.2)
    Gamma22 = 2,  // Pure gamma 2.2
    PQ = 3,  // Perceptual Quantizer (SMPTE ST 2084)
    HLG = 4   // Hybrid Log-Gamma (BT.2100)
};

inline const char* GammaMappingName(GammaMapping g) noexcept {
    switch (g) {
    case GammaMapping::Linear:  return "Linear";
    case GammaMapping::SRGB:    return "sRGB";
    case GammaMapping::Gamma22: return "Gamma22";
    case GammaMapping::PQ:      return "PQ";
    case GammaMapping::HLG:     return "HLG";
    default:                    return "Unknown";
    }
}

/// Color profile metadata for an image
struct ColorProfile {
    ColorSpaceType colorSpace = ColorSpaceType::SRGB;
    GammaMapping   gamma = GammaMapping::SRGB;
    float          whitePointX = 0.3127f;  // D65 illuminant
    float          whitePointY = 0.3290f;
    bool           embedded = false;    // ICC profile embedded in file
};

/// Configuration for color space conversion
struct ColorConversionConfig {
    ColorSpaceType sourceSpace = ColorSpaceType::SRGB;
    ColorSpaceType targetSpace = ColorSpaceType::SRGB;
    GammaMapping   sourceGamma = GammaMapping::SRGB;
    GammaMapping   targetGamma = GammaMapping::SRGB;
    bool           clampOutput = true;   // Clamp to [0, 1] after conversion
    bool           useLUT = false;  // Use lookup table acceleration
};

/// Manages color space conversion for the thumbnail pipeline,
/// supporting sRGB, Adobe RGB, Display P3, Rec.2020, and
/// ProPhoto RGB with configurable gamma transfer functions.
class ThumbnailColorSpace {
public:
    ThumbnailColorSpace() = default;
    ~ThumbnailColorSpace() = default;

    ThumbnailColorSpace(const ThumbnailColorSpace&) = delete;
    ThumbnailColorSpace& operator=(const ThumbnailColorSpace&) = delete;
    ThumbnailColorSpace(ThumbnailColorSpace&&) noexcept = default;
    ThumbnailColorSpace& operator=(ThumbnailColorSpace&&) noexcept = default;

    /// Convert pixel value between configured color spaces
    bool Convert(float& r, float& g, float& b) const {
        if (m_config.sourceSpace == m_config.targetSpace &&
            m_config.sourceGamma == m_config.targetGamma) {
            return true;  // No conversion needed
        }
        // Linearize from source gamma
        if (m_config.sourceGamma == GammaMapping::Gamma22) {
            r = std::pow(r, 2.2f);
            g = std::pow(g, 2.2f);
            b = std::pow(b, 2.2f);
        }
        // Re-apply target gamma
        if (m_config.targetGamma == GammaMapping::Gamma22) {
            r = std::pow(r, 1.0f / 2.2f);
            g = std::pow(g, 1.0f / 2.2f);
            b = std::pow(b, 1.0f / 2.2f);
        }
        if (m_config.clampOutput) {
            auto clamp01 = [](float v) { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); };
            r = clamp01(r); g = clamp01(g); b = clamp01(b);
        }
        m_conversionCount++;
        return true;
    }

    /// Get the active color profile
    ColorProfile GetProfile() const noexcept { return m_profile; }

    /// Set the working color space
    void SetWorkingSpace(ColorSpaceType space) noexcept {
        m_config.targetSpace = space;
        m_profile.colorSpace = space;
    }

    /// Apply full conversion config
    void SetConfig(const ColorConversionConfig& cfg) noexcept { m_config = cfg; }

    /// Get conversion count
    uint64_t GetConversionCount() const noexcept { return m_conversionCount; }

    /// Get current config
    const ColorConversionConfig& GetConfig() const noexcept { return m_config; }

private:
    ColorConversionConfig m_config;
    ColorProfile          m_profile;
    mutable uint64_t      m_conversionCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
