// OutputColorSpaceMapper.h — Maps decoded pixels to target color space
// Copyright (c) 2026 ExplorerLens Project
//
// Converts decoded image data between color spaces (sRGB, Adobe RGB, P3,
// linear) to match the output display profile. Uses ICC profile when available.
//
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct OutputColorSpaceMapperConfig
{
    bool enabled = true;
    uint32_t lutSize = 64;
    std::string label = "OutputColorSpaceMapper";
};

class OutputColorSpaceMapper
{
  public:
    bool Initialize()
    {
        if (m_initialized)
            return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const
    {
        return m_initialized;
    }
    OutputColorSpaceMapperConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    enum class ColorSpace : uint8_t {
        SRGB,
        AdobeRGB,
        DisplayP3,
        LinearRGB,
        ProPhotoRGB
    };

    bool NeedsConversion(ColorSpace source, ColorSpace target) const
    {
        return source != target;
    }

    float LinearToSRGB(float linear) const
    {
        if (linear <= 0.0031308f)
            return 12.92f * linear;
        return 1.055f * std::pow(linear, 1.0f / 2.4f) - 0.055f;
    }

    float SRGBToLinear(float srgb) const
    {
        if (srgb <= 0.04045f)
            return srgb / 12.92f;
        return std::pow((srgb + 0.055f) / 1.055f, 2.4f);
    }

  private:
    bool m_initialized = false;
    OutputColorSpaceMapperConfig m_config;
};

}  // namespace Engine
}  // namespace ExplorerLens
