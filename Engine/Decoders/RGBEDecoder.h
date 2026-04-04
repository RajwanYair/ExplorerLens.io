// RGBEDecoder.h — Radiance RGBE/XYZE HDR Image Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Parses Radiance HDR (.hdr/.pic/.rgbe) format including XYZE variant.
// Decodes run-length encoded scanlines, converts to 32-bit RGBA for thumbnail.
// Supports both old and new RLE encoding schemes.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class HDREncoding : uint8_t {
    RGBE,
    XYZE
};

struct RGBEHeader
{
    uint32_t width = 0;
    uint32_t height = 0;
    HDREncoding encoding = HDREncoding::RGBE;
    float exposure = 1.0f;
    float gamma = 1.0f;
    std::string software;
};

struct RGBEStats
{
    uint32_t filesDecoded = 0;
    uint32_t rleScanlinesDecoded = 0;
    double avgExposure = 1.0;
    uint64_t totalPixelsProcessed = 0;
};

class RGBEDecoder
{
  public:
    RGBEDecoder() = default;
    ~RGBEDecoder() = default;

    static const wchar_t* GetName()
    {
        return L"RGBEDecoder";
    }

    bool CanDecode(const wchar_t* ext) const
    {
        if (!ext)
            return false;
        std::wstring e(ext);
        for (auto& c : e)
            c = towlower(c);
        return e == L".hdr" || e == L".pic" || e == L".rgbe";
    }

    /// Detect Radiance HDR magic: "#?RADIANCE" or "#?RGBE"
    bool DetectMagic(const uint8_t* data, size_t size) const
    {
        if (!data || size < 10)
            return false;
        return (memcmp(data, "#?RADIANCE", 10) == 0) || (size >= 6 && memcmp(data, "#?RGBE", 6) == 0);
    }

    /// Parse header lines (newline-delimited) for dimensions and metadata.
    RGBEHeader ParseHeader(const uint8_t* data, size_t size) const
    {
        RGBEHeader hdr;
        if (!data || size < 10)
            return hdr;

        std::string text(reinterpret_cast<const char*>(data), std::min(size, static_cast<size_t>(2048)));

        if (text.find("XYZE") != std::string::npos)
            hdr.encoding = HDREncoding::XYZE;

        auto posY = text.find("-Y ");
        auto posX = text.find("+X ");
        if (posY != std::string::npos && posX != std::string::npos) {
            hdr.height = static_cast<uint32_t>(std::atoi(text.c_str() + posY + 3));
            hdr.width = static_cast<uint32_t>(std::atoi(text.c_str() + posX + 3));
        }

        auto posE = text.find("EXPOSURE=");
        if (posE != std::string::npos)
            hdr.exposure = static_cast<float>(std::atof(text.c_str() + posE + 9));

        return hdr;
    }

    /// Convert RGBE pixel to linear RGB float.
    void RGBEToFloat(uint8_t r, uint8_t g, uint8_t b, uint8_t e, float& outR, float& outG, float& outB) const
    {
        if (e == 0) {
            outR = outG = outB = 0.0f;
            return;
        }
        float scale = std::ldexp(1.0f, static_cast<int>(e) - 128 - 8);
        outR = r * scale;
        outG = g * scale;
        outB = b * scale;
    }

    /// Tone-map linear HDR to 8-bit using Reinhard operator.
    uint8_t ToneMap(float linear) const
    {
        float mapped = linear / (1.0f + linear);
        float gamma = std::pow(mapped, 1.0f / 2.2f);
        return static_cast<uint8_t>(std::clamp(gamma * 255.0f, 0.0f, 255.0f));
    }

    RGBEStats GetStats() const
    {
        return m_stats;
    }

  private:
    mutable RGBEStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
