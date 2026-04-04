//==============================================================================
// ExplorerLens Engine — HDR Display Pipeline
// High Dynamic Range display output with tone mapping and wide color gamut.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Tone mapping operator
enum class ToneMappingOp : uint8_t {
    Reinhard,    // Reinhard global operator
    ACES,        // Academy Color Encoding System
    Filmic,      // Uncharted 2 filmic curve
    AgX,         // AgX (cinematic)
    PBRNeutral,  // PBR Neutral (Khronos)
    Linear,      // No tone mapping (clamp only)
    COUNT
};

/// Color space / gamut
enum class ColorGamut : uint8_t {
    sRGB,      // IEC 61966-2-1
    AdobeRGB,  // Adobe RGB (1998)
    DCI_P3,    // DCI-P3 (Display P3)
    Rec2020,   // ITU-R BT.2020
    ACEScg,    // ACES computer graphics
    COUNT
};

/// HDR format
enum class HDRFormat : uint8_t {
    Radiance,     // .hdr / .rgbe
    OpenEXR,      // .exr
    HDR10,        // HDR10 (PQ + BT.2020)
    DolbyVision,  // Dolby Vision
    HLG,          // Hybrid Log-Gamma
    COUNT
};

/// HDR display config
struct HDRDisplayConfig
{
    ToneMappingOp toneMap = ToneMappingOp::ACES;
    ColorGamut outputGamut = ColorGamut::sRGB;
    float exposure = 1.0f;
    float gamma = 2.2f;
    float maxNits = 1000.0f;
    float paperWhiteNits = 200.0f;
    bool autoExposure = true;
};

/// HDR image metadata
struct HDRImageInfo
{
    HDRFormat format = HDRFormat::Radiance;
    ColorGamut sourceGamut = ColorGamut::sRGB;
    uint32_t width = 0;
    uint32_t height = 0;
    float maxLuminance = 0.0f;
    float avgLuminance = 0.0f;
    uint32_t bitsPerChannel = 32;
    bool hasAlpha = false;
};

/// HDR display pipeline
class HDRDisplayPipeline
{
  public:
    static const wchar_t* ToneMapName(ToneMappingOp op)
    {
        switch (op) {
            case ToneMappingOp::Reinhard:
                return L"Reinhard";
            case ToneMappingOp::ACES:
                return L"ACES";
            case ToneMappingOp::Filmic:
                return L"Filmic";
            case ToneMappingOp::AgX:
                return L"AgX";
            case ToneMappingOp::PBRNeutral:
                return L"PBR Neutral";
            case ToneMappingOp::Linear:
                return L"Linear";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* GamutName(ColorGamut g)
    {
        switch (g) {
            case ColorGamut::sRGB:
                return L"sRGB";
            case ColorGamut::AdobeRGB:
                return L"Adobe RGB";
            case ColorGamut::DCI_P3:
                return L"DCI-P3";
            case ColorGamut::Rec2020:
                return L"Rec.2020";
            case ColorGamut::ACEScg:
                return L"ACEScg";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* HDRFormatName(HDRFormat f)
    {
        switch (f) {
            case HDRFormat::Radiance:
                return L"Radiance HDR";
            case HDRFormat::OpenEXR:
                return L"OpenEXR";
            case HDRFormat::HDR10:
                return L"HDR10";
            case HDRFormat::DolbyVision:
                return L"Dolby Vision";
            case HDRFormat::HLG:
                return L"HLG";
            default:
                return L"Unknown";
        }
    }

    static constexpr size_t ToneMapCount()
    {
        return static_cast<size_t>(ToneMappingOp::COUNT);
    }
    static constexpr size_t GamutCount()
    {
        return static_cast<size_t>(ColorGamut::COUNT);
    }
    static constexpr size_t HDRFormatCount()
    {
        return static_cast<size_t>(HDRFormat::COUNT);
    }

    /// Validate exposure range
    static bool ValidateExposure(float exp)
    {
        return exp > 0.0f && exp <= 20.0f;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
