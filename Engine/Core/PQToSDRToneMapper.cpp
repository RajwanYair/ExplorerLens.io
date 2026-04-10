// PQToSDRToneMapper.cpp — SMPTE ST.2084 (PQ) to sRGB Tone Mapper
// Copyright (c) 2026 ExplorerLens Project
//
#include "Core/PQToSDRToneMapper.h"
#include <chrono>
#include <cmath>
#include <cstring>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

// Half-float (fp16) to float helper.
static float FP16ToFloat(uint16_t h) noexcept
{
    const uint32_t sign     = (h >> 15u) & 1u;
    const uint32_t exp      = (h >> 10u) & 0x1Fu;
    const uint32_t mantissa =  h         & 0x3FFu;
    uint32_t f = 0;
    if (exp == 0) {
        if (mantissa == 0) { f = sign << 31u; }
        else {
            uint32_t e2 = 127u - 14u;
            uint32_t m2 = mantissa;
            while (!(m2 & 0x400u)) { m2 <<= 1u; --e2; }
            f = (sign << 31u) | (e2 << 23u) | ((m2 & 0x3FFu) << 13u);
        }
    } else if (exp == 31u) {
        f = (sign << 31u) | 0x7F800000u | (mantissa << 13u);
    } else {
        f = (sign << 31u) | ((exp + 112u) << 23u) | (mantissa << 13u);
    }
    float result;
    memcpy(&result, &f, sizeof(result));
    return result;
}

// PQ (SMPTE ST.2084) inverse EOTF: signal → normalised linear [0,1].
float PQToSDRToneMapper::PQToLinear(float pq) noexcept
{
    constexpr float m1  = 0.1593017578125f;
    constexpr float m2  = 78.84375f;
    constexpr float c1  = 0.8359375f;
    constexpr float c2  = 18.8515625f;
    constexpr float c3  = 18.6875f;
    const float p = std::pow(std::max(pq, 0.0f), 1.0f / m2);
    const float num = std::max(p - c1, 0.0f);
    const float den = c2 - c3 * p;
    if (den <= 0.0f) return 10000.0f;
    return std::pow(num / den, 1.0f / m1) * 10000.0f;  // in nits
}

// Hable filmic curve (Uncharted 2).
float PQToSDRToneMapper::HableCurve(float x) noexcept
{
    constexpr float A = 0.15f, B = 0.50f, C = 0.10f, D = 0.20f, E = 0.02f, F = 0.30f;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

// ACES RRT approximation (Narkowicz 2015).
float PQToSDRToneMapper::ACESCurve(float x) noexcept
{
    constexpr float a = 2.51f, b = 0.03f, c = 2.43f, d = 0.59f, e = 0.14f;
    x = std::max(x, 0.0f);
    return std::clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0f, 1.0f);
}

// Linear → sRGB gamma.
static float LinearToSRGB(float v) noexcept
{
    if (v <= 0.0031308f) return v * 12.92f;
    return 1.055f * std::pow(v, 1.0f / 2.4f) - 0.055f;
}

PQToSDRToneMapper::PQToSDRToneMapper()  = default;
PQToSDRToneMapper::~PQToSDRToneMapper() = default;

std::array<uint8_t, 1024> PQToSDRToneMapper::BuildPQToSRGBLUT(
    const PQToneMapParams& params) noexcept
{
    std::array<uint8_t, 1024> lut{};
    for (int i = 0; i < 1024; ++i) {
        float pq      = static_cast<float>(i) / 1023.0f;
        float nits    = PQToLinear(pq);
        float linear  = nits / params.peakNits;
        float exposed = linear * std::powf(2.0f, params.exposure);
        float mapped  = (params.op == PQToneMapOp::Hable)
                        ? HableCurve(exposed) / HableCurve(11.2f)
                        : ACESCurve(exposed);
        float srgb    = LinearToSRGB(std::clamp(mapped, 0.0f, 1.0f));
        lut[i] = static_cast<uint8_t>(std::clamp(srgb * 255.0f + 0.5f, 0.0f, 255.0f));
    }
    return lut;
}

PQToneMapResult PQToSDRToneMapper::ToneMap(
    const uint16_t* srcFP16, uint32_t width, uint32_t height,
    const PQToneMapParams& params) const noexcept
{
    PQToneMapResult result{};
    if (!srcFP16 || width == 0 || height == 0) return result;

    const auto t0 = std::chrono::high_resolution_clock::now();
    const size_t bufSize = static_cast<size_t>(width) * height * 4u;
    result.pixelsBGRA = new (std::nothrow) uint8_t[bufSize];
    if (!result.pixelsBGRA) return result;

    const float exposeMult = std::powf(2.0f, params.exposure);
    const float peakNorm   = params.peakNits;

    for (uint32_t i = 0; i < width * height; ++i) {
        const float r_fp = FP16ToFloat(srcFP16[i * 4 + 0]);
        const float g_fp = FP16ToFloat(srcFP16[i * 4 + 1]);
        const float b_fp = FP16ToFloat(srcFP16[i * 4 + 2]);

        auto mapChannel = [&](float pq) -> uint8_t {
            float nits    = PQToLinear(pq);
            float lin     = nits / peakNorm * exposeMult;
            float mapped  = (params.op == PQToneMapOp::Hable)
                            ? HableCurve(lin) / HableCurve(11.2f)
                            : ACESCurve(lin);
            float srgb = LinearToSRGB(std::clamp(mapped, 0.0f, 1.0f));
            return static_cast<uint8_t>(std::clamp(srgb * 255.0f + 0.5f, 0.0f, 255.0f));
        };

        result.pixelsBGRA[i * 4 + 0] = mapChannel(b_fp);  // B
        result.pixelsBGRA[i * 4 + 1] = mapChannel(g_fp);  // G
        result.pixelsBGRA[i * 4 + 2] = mapChannel(r_fp);  // R
        result.pixelsBGRA[i * 4 + 3] = 0xFF;
    }

    result.width   = width;
    result.height  = height;
    result.success = true;
    const auto t1  = std::chrono::high_resolution_clock::now();
    result.processMs = std::chrono::duration<float, std::milli>(t1 - t0).count();
    return result;
}

PQToneMapResult PQToSDRToneMapper::ToneMapBGRA10(
    const uint32_t* srcBGRA10, uint32_t width, uint32_t height,
    const PQToneMapParams& params) const noexcept
{
    PQToneMapResult result{};
    if (!srcBGRA10 || width == 0 || height == 0) return result;

    const auto t0 = std::chrono::high_resolution_clock::now();
    const size_t bufSize = static_cast<size_t>(width) * height * 4u;
    result.pixelsBGRA = new (std::nothrow) uint8_t[bufSize];
    if (!result.pixelsBGRA) return result;

    const float exposeMult = std::powf(2.0f, params.exposure);
    const float peakNorm   = params.peakNits;

    for (uint32_t i = 0; i < width * height; ++i) {
        const uint32_t packed = srcBGRA10[i];
        const float b_pq = ((packed >>  0u) & 0x3FFu) / 1023.0f;
        const float g_pq = ((packed >> 10u) & 0x3FFu) / 1023.0f;
        const float r_pq = ((packed >> 20u) & 0x3FFu) / 1023.0f;

        auto mapCh = [&](float pq) -> uint8_t {
            float nits   = PQToLinear(pq);
            float lin    = nits / peakNorm * exposeMult;
            float mapped = (params.op == PQToneMapOp::Hable)
                           ? HableCurve(lin) / HableCurve(11.2f)
                           : ACESCurve(lin);
            float srgb   = LinearToSRGB(std::clamp(mapped, 0.0f, 1.0f));
            return static_cast<uint8_t>(std::clamp(srgb * 255.0f + 0.5f, 0.0f, 255.0f));
        };

        result.pixelsBGRA[i * 4 + 0] = mapCh(b_pq);
        result.pixelsBGRA[i * 4 + 1] = mapCh(g_pq);
        result.pixelsBGRA[i * 4 + 2] = mapCh(r_pq);
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
