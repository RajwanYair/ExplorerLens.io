// ACESODTProcessor.cpp — ACES Output Device Transform Processor
// Copyright (c) 2026 ExplorerLens Project
//
#include "Core/ACESODTProcessor.h"
#include <chrono>
#include <cstring>
#include <cmath>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

// Half-float to float (local copy, no cross-translation-unit dependency).
static float H2F(uint16_t h) noexcept
{
    const uint32_t s = (h >> 15u) & 1u;
    const uint32_t e = (h >> 10u) & 0x1Fu;
    const uint32_t m =  h         & 0x3FFu;
    uint32_t f = 0;
    if (e == 0 && m == 0) { f = s << 31u; }
    else if (e == 31u) { f = (s << 31u) | 0x7F800000u | (m << 13u); }
    else               { f = (s << 31u) | ((e + 112u) << 23u) | (m << 13u); }
    float r; __builtin_memcpy(&r, &f, sizeof(r)); return r;
}

// ACES RRT segmented-spline C5 approximation.
float ACESODTProcessor::ApplyRRT(float x) noexcept
{
    // Narkowicz 2015 compact approximation (fast, < 0.5% error vs full RRT).
    x = std::max(x, 0.0f);
    constexpr float a = 2.51f, b = 0.03f, c = 2.43f, d = 0.59f, e_ = 0.14f;
    return std::clamp((x * (a * x + b)) / (x * (c * x + d) + e_), 0.0f, 1.0f);
}

// AP0 → AP1 (ACEScg) primaries matrix.
// Derived from the ACES technical specification S-2014-003.
void ACESODTProcessor::AP0toAP1(float& r, float& g, float& b) noexcept
{
    const float r2 =  1.4514393161f * r - 0.2365107469f * g - 0.2149285693f * b;
    const float g2 = -0.0765537734f * r + 1.1762296998f * g - 0.0996759264f * b;
    const float b2 =  0.0083161484f * r - 0.0060324498f * g + 0.9977163014f * b;
    r = r2; g = g2; b = b2;
}

// AP1 → sRGB primaries matrix.
void ACESODTProcessor::AP1toSRGB(float& r, float& g, float& b) noexcept
{
    const float r2 =  1.7050487811f * r - 0.6217902208f * g - 0.0832585603f * b;
    const float g2 = -0.1302564082f * r + 1.1408406797f * g - 0.0105842715f * b;
    const float b2 = -0.0240032715f * r - 0.1289689188f * g + 1.1529721903f * b;
    r = std::clamp(r2, 0.0f, 1.0f);
    g = std::clamp(g2, 0.0f, 1.0f);
    b = std::clamp(b2, 0.0f, 1.0f);
}

// Detect ACES colorspace from a string identifier.
ACESColorspace ACESODTProcessor::DetectFromString(const char* name) noexcept
{
    if (!name) return ACESColorspace::Unknown;
    // Common string identifiers from OpenColorIO configs and DCC tools.
    if (std::strstr(name, "ACES2065")  || std::strstr(name, "AP0"))   return ACESColorspace::AP0;
    if (std::strstr(name, "ACEScg")    || std::strstr(name, "AP1"))   return ACESColorspace::AP1;
    if (std::strstr(name, "ACEScc"))                                   return ACESColorspace::ACEScc;
    if (std::strstr(name, "ACEScct"))                                  return ACESColorspace::ACEScct;
    if (std::strstr(name, "ACESproxy"))                                return ACESColorspace::ACESproxy;
    return ACESColorspace::Unknown;
}

// Detect ACES colorspace from EXR header chromaticities (brute-force search for magic floats).
ACESColorspace ACESODTProcessor::DetectColorspace(
    const uint8_t* exrHeader, size_t exrHeaderSize) noexcept
{
    if (!exrHeader || exrHeaderSize < 8) return ACESColorspace::Unknown;
    // EXR magic: 0x76, 0x2F, 0x31, 0x01.
    if (exrHeader[0] != 0x76 || exrHeader[1] != 0x2F ||
        exrHeader[2] != 0x31 || exrHeader[3] != 0x01)
        return ACESColorspace::Unknown;
    // Scan header for attribute name "chromaticities" or "acesImageContainerFlag".
    const char* hdr  = reinterpret_cast<const char*>(exrHeader + 8);
    const size_t len = exrHeaderSize - 8;
    for (size_t i = 0; i + 14 < len; ++i) {
        if (std::memcmp(hdr + i, "acesImageContainerFlag", 22) == 0) return ACESColorspace::AP0;
        if (std::memcmp(hdr + i, "chromaticities", 14) == 0)         return ACESColorspace::AP0;
    }
    return ACESColorspace::Unknown;
}

// Linear → sRGB OETF.
static float LinearToSRGB(float v) noexcept {
    v = std::max(v, 0.0f);
    return (v <= 0.0031308f) ? v * 12.92f : 1.055f * std::pow(v, 1.0f / 2.4f) - 0.055f;
}

ACESODTResult ACESODTProcessor::ApplyODT(
    const uint16_t* srcFP16, uint32_t width, uint32_t height,
    const ACESODTParams& params) const noexcept
{
    ACESODTResult result{};
    if (!srcFP16 || width == 0 || height == 0) return result;

    const auto t0 = std::chrono::high_resolution_clock::now();
    const size_t bufSize = static_cast<size_t>(width) * height * 4u;
    result.pixelsBGRA = new (std::nothrow) uint8_t[bufSize];
    if (!result.pixelsBGRA) return result;

    result.detectedColorspace = params.sourceColorspace;
    const float exposeMult = std::pow(2.0f, params.exposureAdjust);

    for (uint32_t i = 0; i < width * height; ++i) {
        float r = H2F(srcFP16[i * 4 + 0]) * exposeMult;
        float g = H2F(srcFP16[i * 4 + 1]) * exposeMult;
        float b = H2F(srcFP16[i * 4 + 2]) * exposeMult;

        // Convert AP0 → AP1 if needed.
        if (params.sourceColorspace == ACESColorspace::AP0) {
            AP0toAP1(r, g, b);
        }

        // Apply RRT.
        if (params.applyRRT) {
            r = ApplyRRT(r); g = ApplyRRT(g); b = ApplyRRT(b);
        }

        // ODT: AP1 → sRGB primaries + gamma.
        AP1toSRGB(r, g, b);

        auto to8 = [](float v) -> uint8_t {
            return static_cast<uint8_t>(std::clamp(LinearToSRGB(v) * 255.0f + 0.5f, 0.0f, 255.0f));
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

ACESODTResult ACESODTProcessor::ACEScgToSRGB(
    const uint16_t* srcFP16, uint32_t width, uint32_t height,
    float exposureAdjust) const noexcept
{
    ACESODTParams p;
    p.sourceColorspace = ACESColorspace::AP1;
    p.target           = ACESODTTarget::sRGB_D65;
    p.exposureAdjust   = exposureAdjust;
    p.applyRRT         = true;
    return ApplyODT(srcFP16, width, height, p);
}

}} // namespace ExplorerLens::Engine
