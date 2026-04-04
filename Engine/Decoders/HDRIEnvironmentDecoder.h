// HDRIEnvironmentDecoder.h — HDRI Environment Map Spherical Preview
// Copyright (c) 2026 ExplorerLens Project
//
// HDRI environment map spherical preview decoder. Loads .hdr/.exr environment maps
// and generates equirectangular-to-sphere projection thumbnails.
//
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class EnvironmentMapFormat : uint8_t {
    Equirectangular,
    CubeMap,
    AngularMap,
    Unknown
};

struct EnvironmentMapInfo
{
    uint32_t width = 0;
    uint32_t height = 0;
    EnvironmentMapFormat format = EnvironmentMapFormat::Unknown;
    float averageLuminance = 0.0f;
    float peakLuminance = 0.0f;
    uint32_t channels = 3;
};

class HDRIEnvironmentDecoder
{
  public:
    static constexpr double PI = 3.14159265358979323846;

    static HDRIEnvironmentDecoder& Instance()
    {
        static HDRIEnvironmentDecoder instance;
        return instance;
    }

    inline bool IsHDRFile(const uint8_t* data, size_t size) const
    {
        if (!data || size < 10)
            return false;
        return (data[0] == '#' && data[1] == '?');
    }

    inline EnvironmentMapInfo AnalyzeEnvironment(const float* hdrData, uint32_t width, uint32_t height,
                                                 uint32_t channels) const
    {
        EnvironmentMapInfo info;
        info.width = width;
        info.height = height;
        info.channels = channels;

        if (!hdrData || width == 0 || height == 0)
            return info;

        if (width == height * 2)
            info.format = EnvironmentMapFormat::Equirectangular;
        else if (width == height)
            info.format = EnvironmentMapFormat::AngularMap;

        float totalLum = 0.0f;
        float maxLum = 0.0f;
        size_t pixelCount = static_cast<size_t>(width) * height;
        for (size_t i = 0; i < pixelCount; ++i) {
            float lum = 0.2126f * hdrData[i * channels];
            if (channels > 1)
                lum += 0.7152f * hdrData[i * channels + 1];
            if (channels > 2)
                lum += 0.0722f * hdrData[i * channels + 2];
            totalLum += lum;
            if (lum > maxLum)
                maxLum = lum;
        }
        info.averageLuminance = totalLum / static_cast<float>(pixelCount);
        info.peakLuminance = maxLum;
        return info;
    }

    inline std::vector<uint8_t> GenerateSpherePreview(const float* hdrData, uint32_t envWidth, uint32_t envHeight,
                                                      uint32_t channels, uint32_t thumbWidth, uint32_t thumbHeight,
                                                      float rotationDeg = 0.0f) const
    {
        std::vector<uint8_t> output(static_cast<size_t>(thumbWidth) * thumbHeight * 3, 30);
        if (!hdrData || envWidth == 0 || envHeight == 0 || thumbWidth == 0 || thumbHeight == 0)
            return output;

        float cx = thumbWidth / 2.0f, cy = thumbHeight / 2.0f;
        float radius = (std::min)(thumbWidth, thumbHeight) * 0.45f;
        float rotRad = static_cast<float>(rotationDeg * PI / 180.0);

        float exposure = ComputeAutoExposure(hdrData, envWidth * envHeight, channels);

        for (uint32_t y = 0; y < thumbHeight; ++y) {
            for (uint32_t x = 0; x < thumbWidth; ++x) {
                float dx = (x - cx) / radius;
                float dy = (y - cy) / radius;
                float distSq = dx * dx + dy * dy;

                size_t outIdx = (static_cast<size_t>(y) * thumbWidth + x) * 3;

                if (distSq <= 1.0f) {
                    float nz = std::sqrt(1.0f - distSq);
                    float nx = dx, ny = -dy;

                    float theta = std::atan2(nx, nz) + rotRad;
                    float phi = std::asin((std::max)(-1.0f, (std::min)(1.0f, ny)));

                    float u = static_cast<float>(theta / (2.0 * PI) + 0.5);
                    float v = static_cast<float>(0.5 - phi / PI);
                    u = u - std::floor(u);
                    v = (std::max)(0.0f, (std::min)(1.0f, v));

                    uint32_t sx = static_cast<uint32_t>(u * (envWidth - 1));
                    uint32_t sy = static_cast<uint32_t>(v * (envHeight - 1));
                    size_t srcIdx = (static_cast<size_t>(sy) * envWidth + sx) * channels;

                    float r = hdrData[srcIdx] * exposure;
                    float g = channels > 1 ? hdrData[srcIdx + 1] * exposure : r;
                    float b = channels > 2 ? hdrData[srcIdx + 2] * exposure : r;

                    r = r / (1.0f + r);
                    g = g / (1.0f + g);
                    b = b / (1.0f + b);

                    output[outIdx + 0] = GammaEncode(r);
                    output[outIdx + 1] = GammaEncode(g);
                    output[outIdx + 2] = GammaEncode(b);
                } else {
                    float edgeDist = std::sqrt(distSq) - 1.0f;
                    if (edgeDist < 0.02f) {
                        output[outIdx + 0] = 80;
                        output[outIdx + 1] = 80;
                        output[outIdx + 2] = 80;
                    }
                }
            }
        }
        return output;
    }

    inline std::string FormatToString(EnvironmentMapFormat fmt) const
    {
        switch (fmt) {
            case EnvironmentMapFormat::Equirectangular:
                return "Equirectangular";
            case EnvironmentMapFormat::CubeMap:
                return "Cube Map";
            case EnvironmentMapFormat::AngularMap:
                return "Angular Map";
            default:
                return "Unknown";
        }
    }

  private:
    HDRIEnvironmentDecoder() = default;

    inline float ComputeAutoExposure(const float* data, size_t pixelCount, uint32_t channels) const
    {
        double logSum = 0.0;
        size_t count = 0;
        size_t step = (std::max)(static_cast<size_t>(1), pixelCount / 10000);
        for (size_t i = 0; i < pixelCount; i += step) {
            float lum = 0.2126f * data[i * channels];
            if (channels > 1)
                lum += 0.7152f * data[i * channels + 1];
            if (channels > 2)
                lum += 0.0722f * data[i * channels + 2];
            if (lum > 0.001f) {
                logSum += std::log(static_cast<double>(lum));
                ++count;
            }
        }
        if (count == 0)
            return 1.0f;
        float logAvg = static_cast<float>(std::exp(logSum / count));
        return 0.18f / (logAvg + 0.001f);
    }

    inline uint8_t GammaEncode(float linear) const
    {
        float v = (std::max)(0.0f, (std::min)(1.0f, linear));
        return static_cast<uint8_t>(std::pow(v, 1.0f / 2.2f) * 255.0f + 0.5f);
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
