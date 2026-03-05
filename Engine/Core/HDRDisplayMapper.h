// HDRDisplayMapper.h — HDR-Aware Thumbnail Rendering
// Copyright (c) 2026 ExplorerLens Project
//
// HDR-aware thumbnail rendering for HDR displays. Tonemaps HDR content using
// Reinhard/ACES operators, outputs SDR or HDR10 depending on display capabilities.
//
#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>
#include <cmath>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class DisplayToneMapOp : uint8_t {
    Reinhard,
    ReinhardExtended,
    ACES,
    Uncharted2,
    Linear
};

enum class DisplayCapability : uint8_t {
    SDR,
    HDR10,
    DolbyVision,
    HLG
};

struct HDRDisplayMappingMetadata {
    float maxContentLightLevel = 1000.0f;
    float maxFrameAverageLightLevel = 400.0f;
    float masteringDisplayMaxLuminance = 1000.0f;
    float masteringDisplayMinLuminance = 0.001f;
};

struct ColorPixel {
    float r = 0.0f, g = 0.0f, b = 0.0f;
};

class HDRDisplayMapper {
public:
    static HDRDisplayMapper& Instance() {
        static HDRDisplayMapper instance;
        return instance;
    }

    inline std::vector<uint8_t> ToneMapToSDR(const float* hdrPixels, uint32_t width, uint32_t height,
        uint32_t channels, DisplayToneMapOp op = DisplayToneMapOp::ACES) const {
        size_t pixelCount = static_cast<size_t>(width) * height;
        std::vector<uint8_t> sdrOutput(pixelCount * 3);
        if (!hdrPixels || pixelCount == 0 || channels == 0) return sdrOutput;

        for (size_t i = 0; i < pixelCount; ++i) {
            size_t srcIdx = i * channels;
            float r = hdrPixels[srcIdx];
            float g = channels > 1 ? hdrPixels[srcIdx + 1] : r;
            float b = channels > 2 ? hdrPixels[srcIdx + 2] : r;

            ColorPixel mapped = ApplyToneMap({ r, g, b }, op);

            size_t dstIdx = i * 3;
            sdrOutput[dstIdx + 0] = FloatToByte(GammaCorrect(mapped.r, 2.2f));
            sdrOutput[dstIdx + 1] = FloatToByte(GammaCorrect(mapped.g, 2.2f));
            sdrOutput[dstIdx + 2] = FloatToByte(GammaCorrect(mapped.b, 2.2f));
        }
        return sdrOutput;
    }

    inline ColorPixel ApplyToneMap(ColorPixel pixel, DisplayToneMapOp op) const {
        switch (op) {
        case DisplayToneMapOp::Reinhard:         return ReinhardToneMap(pixel);
        case DisplayToneMapOp::ReinhardExtended:  return ReinhardExtendedToneMap(pixel, 4.0f);
        case DisplayToneMapOp::ACES:             return ACESToneMap(pixel);
        case DisplayToneMapOp::Uncharted2:       return Uncharted2ToneMap(pixel);
        case DisplayToneMapOp::Linear:           return pixel;
        default:                                return ACESToneMap(pixel);
        }
    }

    inline float ComputeExposure(const float* hdrPixels, uint32_t width, uint32_t height, uint32_t channels) const {
        if (!hdrPixels || width == 0 || height == 0 || channels == 0) return 1.0f;
        size_t pixelCount = static_cast<size_t>(width) * height;

        double logSum = 0.0;
        size_t validCount = 0;
        for (size_t i = 0; i < pixelCount; ++i) {
            size_t idx = i * channels;
            float luminance = 0.2126f * hdrPixels[idx];
            if (channels > 1) luminance += 0.7152f * hdrPixels[idx + 1];
            if (channels > 2) luminance += 0.0722f * hdrPixels[idx + 2];
            if (luminance > 0.0001f) {
                logSum += std::log(luminance + 0.0001);
                ++validCount;
            }
        }
        if (validCount == 0) return 1.0f;
        float logAvg = static_cast<float>(std::exp(logSum / validCount));
        return 0.18f / (logAvg + 0.001f);
    }

    inline std::string GetOperatorName(DisplayToneMapOp op) const {
        switch (op) {
        case DisplayToneMapOp::Reinhard:        return "Reinhard";
        case DisplayToneMapOp::ReinhardExtended: return "Reinhard Extended";
        case DisplayToneMapOp::ACES:            return "ACES Filmic";
        case DisplayToneMapOp::Uncharted2:      return "Uncharted 2";
        case DisplayToneMapOp::Linear:          return "Linear (No Tonemap)";
        default:                               return "Unknown";
        }
    }

private:
    HDRDisplayMapper() = default;

    inline ColorPixel ReinhardToneMap(ColorPixel p) const {
        return { p.r / (1.0f + p.r), p.g / (1.0f + p.g), p.b / (1.0f + p.b) };
    }

    inline ColorPixel ReinhardExtendedToneMap(ColorPixel p, float whitePoint) const {
        float wp2 = whitePoint * whitePoint;
        return {
            p.r * (1.0f + p.r / wp2) / (1.0f + p.r),
            p.g * (1.0f + p.g / wp2) / (1.0f + p.g),
            p.b * (1.0f + p.b / wp2) / (1.0f + p.b)
        };
    }

    inline ColorPixel ACESToneMap(ColorPixel p) const {
        auto aces = [](float x) -> float {
            float a = 2.51f, b = 0.03f, c = 2.43f, d = 0.59f, e = 0.14f;
            float result = (x * (a * x + b)) / (x * (c * x + d) + e);
            return (std::max)(0.0f, (std::min)(1.0f, result));
            };
        return { aces(p.r), aces(p.g), aces(p.b) };
    }

    inline ColorPixel Uncharted2ToneMap(ColorPixel p) const {
        auto uc2 = [this](float x) -> float { return Uncharted2Func(x); };
        float exposureBias = 2.0f;
        float whiteScale = 1.0f / Uncharted2Func(11.2f);
        return {
            uc2(exposureBias * p.r) * whiteScale,
            uc2(exposureBias * p.g) * whiteScale,
            uc2(exposureBias * p.b) * whiteScale
        };
    }

    inline float Uncharted2Func(float x) const {
        float A = 0.15f, B = 0.50f, C = 0.10f, D = 0.20f, E = 0.02f, F = 0.30f;
        return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
    }

    inline float GammaCorrect(float linear, float gamma) const {
        if (linear <= 0.0f) return 0.0f;
        return std::pow(linear, 1.0f / gamma);
    }

    inline uint8_t FloatToByte(float value) const {
        return static_cast<uint8_t>((std::max)(0.0f, (std::min)(1.0f, value)) * 255.0f + 0.5f);
    }
};

}
} // namespace ExplorerLens::Engine
