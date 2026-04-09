// HLGToSDRConverter.cpp — ITU-R BT.2100 HLG to sRGB Converter
// Copyright (c) 2026 ExplorerLens Project
//
#include "Core/HLGToSDRConverter.h"
#include <chrono>
#include <cmath>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

// HLG inverse OETF constants (ITU-R BT.2100-2, Table 5).
static constexpr float HLG_A  = 0.17883277f;
static constexpr float HLG_B  = 0.28466892f;
static constexpr float HLG_C  = 0.55991073f;
static constexpr float HLG_R  = 0.5f;

// Half-float to float (same as PQToSDRToneMapper.cpp, kept local to avoid coupling).
static float H2F(uint16_t h) noexcept
{
    const uint32_t sign     = (h >> 15u) & 1u;
    const uint32_t exp      = (h >> 10u) & 0x1Fu;
    const uint32_t mantissa =  h         & 0x3FFu;
    uint32_t f = 0;
    if (exp == 0 && mantissa == 0) { f = sign << 31u; }
    else if (exp == 31u) { f = (sign << 31u) | 0x7F800000u | (mantissa << 13u); }
    else { f = (sign << 31u) | ((exp + 112u) << 23u) | (mantissa << 13u); }
    float r;
    __builtin_memcpy(&r, &f, sizeof(r));
    return r;
}

// HLG inverse OETF: signal ∈ [0,1] → normalised scene-linear ∈ [0,1].
float HLGToSDRConverter::HLGtoLinear(float signal) noexcept
{
    signal = std::max(signal, 0.0f);
    if (signal <= HLG_R) {
        return (signal * signal) / 3.0f;
    }
    return (std::exp((signal - HLG_C) / HLG_A) + HLG_B) / 12.0f;
}

// HLG OOTF: scene-linear → display-linear.
float HLGToSDRConverter::HLGOOTF(float sceneLinear, float systemGamma,
                                   float peakLuminance) noexcept
{
    if (sceneLinear <= 0.0f) return 0.0f;
    // Nominal display luminance for gamma derivation: 1000 cd/m².
    const float alpha = peakLuminance * std::pow(0.1f, systemGamma);
    return alpha * std::pow(sceneLinear, systemGamma);
}

// BT.2020 → BT.709 colour primaries matrix (D65).
void HLGToSDRConverter::BT2020ToBT709(float& r, float& g, float& b) noexcept
{
    const float r2 =  1.6605f * r - 0.5876f * g - 0.0728f * b;
    const float g2 = -0.1246f * r + 1.1329f * g - 0.0083f * b;
    const float b2 = -0.0182f * r - 0.1006f * g + 1.1187f * b;
    r = std::clamp(r2, 0.0f, 1.0f);
    g = std::clamp(g2, 0.0f, 1.0f);
    b = std::clamp(b2, 0.0f, 1.0f);
}

// Linear → sRGB gamma.
static float LSrgb(float v) noexcept {
    if (v <= 0.0031308f) return v * 12.92f;
    return 1.055f * std::pow(std::max(v, 0.0f), 1.0f / 2.4f) - 0.055f;
}

HLGConvertResult HLGToSDRConverter::Convert(
    const uint16_t* srcBGRA16F, uint32_t width, uint32_t height,
    const HLGConvertParams& params) const noexcept
{
    HLGConvertResult result{};
    if (!srcBGRA16F || width == 0 || height == 0) return result;

    const auto t0 = std::chrono::high_resolution_clock::now();
    const size_t bufSize = static_cast<size_t>(width) * height * 4u;
    result.pixelsBGRA = new (std::nothrow) uint8_t[bufSize];
    if (!result.pixelsBGRA) return result;

    // Scene luminance reference: normalise display peak to [0, 1].
    const float peakNorm   = params.peakLuminance / 1000.0f;
    const float sysGamma   = params.systemGamma;

    for (uint32_t i = 0; i < width * height; ++i) {
        float r = H2F(srcBGRA16F[i * 4 + 2]);
        float g = H2F(srcBGRA16F[i * 4 + 1]);
        float b = H2F(srcBGRA16F[i * 4 + 0]);

        // Inverse OETF.
        r = HLGtoLinear(r);
        g = HLGtoLinear(g);
        b = HLGtoLinear(b);

        // OOTF (scene-referred → display).
        if (params.sceneAdaptive) {
            const float luma = 0.2627f * r + 0.6780f * g + 0.0593f * b;
            if (luma > 0.0f) {
                const float scale = HLGOOTF(luma, sysGamma, params.peakLuminance) / luma / peakNorm;
                r *= scale; g *= scale; b *= scale;
            }
        }

        // BT.2020 → BT.709 primaries.
        BT2020ToBT709(r, g, b);

        auto to8 = [](float v) -> uint8_t {
            return static_cast<uint8_t>(std::clamp(LSrgb(v) * 255.0f + 0.5f, 0.0f, 255.0f));
        };

        result.pixelsBGRA[i * 4 + 0] = to8(b);
        result.pixelsBGRA[i * 4 + 1] = to8(g);
        result.pixelsBGRA[i * 4 + 2] = to8(r);
        result.pixelsBGRA[i * 4 + 3] = 0xFF;
    }

    result.width   = width;
    result.height  = height;
    result.success = true;
    const auto t1  = std::chrono::high_resolution_clock::now();
    result.processMs = std::chrono::duration<float, std::milli>(t1 - t0).count();
    return result;
}

HLGConvertResult HLGToSDRConverter::ConvertBGRA10(
    const uint32_t* srcBGRA10, uint32_t width, uint32_t height,
    const HLGConvertParams& params) const noexcept
{
    HLGConvertResult result{};
    if (!srcBGRA10 || width == 0 || height == 0) return result;

    const auto t0 = std::chrono::high_resolution_clock::now();
    const size_t bufSize = static_cast<size_t>(width) * height * 4u;
    result.pixelsBGRA = new (std::nothrow) uint8_t[bufSize];
    if (!result.pixelsBGRA) return result;

    const float peakNorm = params.peakLuminance / 1000.0f;

    for (uint32_t i = 0; i < width * height; ++i) {
        const uint32_t p = srcBGRA10[i];
        float b = ((p >>  0u) & 0x3FFu) / 1023.0f;
        float g = ((p >> 10u) & 0x3FFu) / 1023.0f;
        float r = ((p >> 20u) & 0x3FFu) / 1023.0f;

        r = HLGtoLinear(r); g = HLGtoLinear(g); b = HLGtoLinear(b);
        if (params.sceneAdaptive) {
            const float luma = 0.2627f * r + 0.6780f * g + 0.0593f * b;
            if (luma > 0.0f) {
                const float scale = HLGOOTF(luma, params.systemGamma, params.peakLuminance) / luma / peakNorm;
                r *= scale; g *= scale; b *= scale;
            }
        }
        BT2020ToBT709(r, g, b);

        auto to8 = [](float v) -> uint8_t {
            return static_cast<uint8_t>(std::clamp(LSrgb(v) * 255.0f + 0.5f, 0.0f, 255.0f));
        };
        result.pixelsBGRA[i * 4 + 0] = to8(b);
        result.pixelsBGRA[i * 4 + 1] = to8(g);
        result.pixelsBGRA[i * 4 + 2] = to8(r);
        result.pixelsBGRA[i * 4 + 3] = 0xFF;
    }

    result.width   = width;
    result.height  = height;
    result.success = true;
    const auto t1  = std::chrono::high_resolution_clock::now();
    result.processMs = std::chrono::duration<float, std::milli>(t1 - t0).count();
    return result;
}

}} // namespace ExplorerLens::Engine
