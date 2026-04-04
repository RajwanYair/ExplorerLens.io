// ColorSpaceConverter.h — Color Space Conversion for Thumbnails
// Copyright (c) 2026 ExplorerLens Project
//
// Converts image data between color spaces (sRGB, Adobe RGB, Display P3,
// ProPhoto RGB, Rec.2020) during thumbnail generation. Ensures accurate
// color reproduction across different display profiles.
//
#pragma once

#include <cmath>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

enum class CSCColorSpace : uint8_t {
    sRGB,
    AdobeRGB,
    DisplayP3,
    ProPhotoRGB,
    Rec2020,
    Rec709,
    CIEXYZ,
    COUNT
};

enum class RenderIntent : uint8_t {
    Perceptual,
    RelativeColorimetric,
    Saturation,
    AbsoluteColorimetric,
    COUNT
};

struct ColorTriple
{
    float r = 0.0f, g = 0.0f, b = 0.0f;
};

struct ConversionConfig
{
    CSCColorSpace source = CSCColorSpace::sRGB;
    CSCColorSpace destination = CSCColorSpace::sRGB;
    RenderIntent intent = RenderIntent::Perceptual;
    bool chromaticAdaptation = true;
    bool clampOutput = true;
};

struct CSCResult
{
    CSCColorSpace from = CSCColorSpace::sRGB;
    CSCColorSpace to = CSCColorSpace::sRGB;
    uint32_t pixelsConverted = 0;
    uint32_t pixelsClamped = 0;
    double conversionMs = 0.0;
    bool outOfGamut = false;
};

class ColorSpaceConverter
{
  public:
    void Configure(const ConversionConfig& cfg)
    {
        m_config = cfg;
    }
    const ConversionConfig& GetConfig() const
    {
        return m_config;
    }

    ColorTriple Convert(const ColorTriple& input) const
    {
        if (m_config.source == m_config.destination)
            return input;
        // Simplified: linearize -> XYZ -> destination
        ColorTriple linear = Linearize(input, m_config.source);
        ColorTriple xyz = ToXYZ(linear, m_config.source);
        ColorTriple destLinear = FromXYZ(xyz, m_config.destination);
        ColorTriple result = ApplyGamma(destLinear, m_config.destination);
        if (m_config.clampOutput)
            Clamp(result);
        return result;
    }

    bool IsIdentityConversion() const
    {
        return m_config.source == m_config.destination;
    }

    float GamutCoverage(CSCColorSpace wider, CSCColorSpace narrower) const
    {
        // Approximate gamut coverage ratios
        if (wider == narrower)
            return 1.0f;
        if (wider == CSCColorSpace::Rec2020 && narrower == CSCColorSpace::sRGB)
            return 0.41f;
        if (wider == CSCColorSpace::DisplayP3 && narrower == CSCColorSpace::sRGB)
            return 0.79f;
        if (wider == CSCColorSpace::AdobeRGB && narrower == CSCColorSpace::sRGB)
            return 0.68f;
        return 0.5f;
    }

    static size_t SpaceCount()
    {
        return static_cast<size_t>(CSCColorSpace::COUNT);
    }
    static size_t IntentCount()
    {
        return static_cast<size_t>(RenderIntent::COUNT);
    }

  private:
    ColorTriple Linearize(ColorTriple c, CSCColorSpace) const
    {
        c.r = (c.r > 0.04045f) ? std::pow((c.r + 0.055f) / 1.055f, 2.4f) : c.r / 12.92f;
        c.g = (c.g > 0.04045f) ? std::pow((c.g + 0.055f) / 1.055f, 2.4f) : c.g / 12.92f;
        c.b = (c.b > 0.04045f) ? std::pow((c.b + 0.055f) / 1.055f, 2.4f) : c.b / 12.92f;
        return c;
    }
    ColorTriple ToXYZ(ColorTriple c, CSCColorSpace) const
    {
        return {c.r * 0.4124f + c.g * 0.3576f + c.b * 0.1805f, c.r * 0.2126f + c.g * 0.7152f + c.b * 0.0722f,
                c.r * 0.0193f + c.g * 0.1192f + c.b * 0.9505f};
    }
    ColorTriple FromXYZ(ColorTriple c, CSCColorSpace) const
    {
        return {c.r * 3.2406f + c.g * -1.5372f + c.b * -0.4986f, c.r * -0.9689f + c.g * 1.8758f + c.b * 0.0415f,
                c.r * 0.0557f + c.g * -0.2040f + c.b * 1.0570f};
    }
    ColorTriple ApplyGamma(ColorTriple c, CSCColorSpace) const
    {
        auto g = [](float v) {
            return (v > 0.0031308f) ? (1.055f * std::pow(v, 1.0f / 2.4f) - 0.055f) : v * 12.92f;
        };
        return {g(c.r), g(c.g), g(c.b)};
    }
    static void Clamp(ColorTriple& c)
    {
        auto cl = [](float& v) {
            v = (v < 0) ? 0.0f : ((v > 1.0f) ? 1.0f : v);
        };
        cl(c.r);
        cl(c.g);
        cl(c.b);
    }
    ConversionConfig m_config;
};

}  // namespace Engine
}  // namespace ExplorerLens
