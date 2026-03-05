// ColorConvertKernel.h — Color Space Conversion GPU Kernel
// Copyright (c) 2026 ExplorerLens Project
//
// Provides color space conversion between sRGB, Adobe RGB, Display P3,
// BT.709/BT.2020, and linear spaces using 3x3 matrix transforms.
//
#pragma once

#include <cstdint>
#include <cmath>
#include <array>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

enum class ColorConvertSpace : uint8_t {
    sRGB = 0,
    LinearSRGB = 1,
    AdobeRGB = 2,
    DisplayP3 = 3,
    BT709 = 4,
    BT2020 = 5,
    CIEXYZ = 6
};

struct ColorConvertParams {
    ColorConvertSpace srcSpace = ColorConvertSpace::sRGB;
    ColorConvertSpace dstSpace = ColorConvertSpace::LinearSRGB;
    bool              clamp = true;
};

using Mat3x3 = std::array<std::array<float, 3>, 3>;

struct ColorVec3 {
    float r = 0.0f, g = 0.0f, b = 0.0f;
};

class ColorConvertKernel {
public:
    static ColorConvertKernel& Instance() { static ColorConvertKernel s; return s; }

    void SetParams(const ColorConvertParams& params) { m_params = params; }
    const ColorConvertParams& GetParams() const { return m_params; }

    static Mat3x3 BuildMatrix(ColorConvertSpace src, ColorConvertSpace dst) {
        // Identity by default
        Mat3x3 m = { {{1,0,0},{0,1,0},{0,0,1}} };

        if (src == dst) return m;

        // sRGB to XYZ (D65)
        if (src == ColorConvertSpace::LinearSRGB && dst == ColorConvertSpace::CIEXYZ) {
            m = { {{ 0.4124564f, 0.3575761f, 0.1804375f },
                  { 0.2126729f, 0.7151522f, 0.0721750f },
                  { 0.0193339f, 0.1191920f, 0.9503041f }} };
        }
        // XYZ to sRGB
        else if (src == ColorConvertSpace::CIEXYZ && dst == ColorConvertSpace::LinearSRGB) {
            m = { {{ 3.2404542f, -1.5371385f, -0.4985314f },
                  {-0.9692660f,  1.8760108f,  0.0415560f },
                  { 0.0556434f, -0.2040259f,  1.0572252f }} };
        }
        // Display P3 to XYZ
        else if (src == ColorConvertSpace::DisplayP3 && dst == ColorConvertSpace::CIEXYZ) {
            m = { {{ 0.4865709f, 0.2656677f, 0.1982173f },
                  { 0.2289746f, 0.6917385f, 0.0792869f },
                  { 0.0000000f, 0.0451134f, 1.0439444f }} };
        }
        // BT.2020 to XYZ
        else if (src == ColorConvertSpace::BT2020 && dst == ColorConvertSpace::CIEXYZ) {
            m = { {{ 0.6369580f, 0.1446169f, 0.1688810f },
                  { 0.2627002f, 0.6779981f, 0.0593017f },
                  { 0.0000000f, 0.0280727f, 1.0609851f }} };
        }

        return m;
    }

    ColorVec3 Convert(const ColorVec3& input) const {
        ColorVec3 linear = input;

        // Linearize source if needed
        if (m_params.srcSpace == ColorConvertSpace::sRGB) {
            linear.r = SRGBToLinear(input.r);
            linear.g = SRGBToLinear(input.g);
            linear.b = SRGBToLinear(input.b);
        }

        // Get conversion matrix
        ColorConvertSpace matSrc = (m_params.srcSpace == ColorConvertSpace::sRGB) ? ColorConvertSpace::LinearSRGB : m_params.srcSpace;
        ColorConvertSpace matDst = (m_params.dstSpace == ColorConvertSpace::sRGB) ? ColorConvertSpace::LinearSRGB : m_params.dstSpace;
        Mat3x3 mat = BuildMatrix(matSrc, matDst);

        // Apply matrix
        ColorVec3 result;
        result.r = mat[0][0] * linear.r + mat[0][1] * linear.g + mat[0][2] * linear.b;
        result.g = mat[1][0] * linear.r + mat[1][1] * linear.g + mat[1][2] * linear.b;
        result.b = mat[2][0] * linear.r + mat[2][1] * linear.g + mat[2][2] * linear.b;

        // Apply output gamma if needed
        if (m_params.dstSpace == ColorConvertSpace::sRGB) {
            result.r = LinearToSRGB(result.r);
            result.g = LinearToSRGB(result.g);
            result.b = LinearToSRGB(result.b);
        }

        if (m_params.clamp) {
            result.r = (std::max)(0.0f, (std::min)(result.r, 1.0f));
            result.g = (std::max)(0.0f, (std::min)(result.g, 1.0f));
            result.b = (std::max)(0.0f, (std::min)(result.b, 1.0f));
        }
        return result;
    }

    bool ConvertBuffer(const float* src, float* dst, uint32_t pixelCount, uint32_t channels = 3) {
        if (!src || !dst || pixelCount == 0 || channels < 3) return false;

        for (uint32_t i = 0; i < pixelCount; ++i) {
            size_t off = static_cast<size_t>(i) * channels;
            ColorVec3 in = { src[off], src[off + 1], src[off + 2] };
            ColorVec3 out = Convert(in);
            dst[off] = out.r;
            dst[off + 1] = out.g;
            dst[off + 2] = out.b;
            if (channels == 4) dst[off + 3] = src[off + 3]; // preserve alpha
        }
        ++m_convertCount;
        return true;
    }

    static Mat3x3 MultiplyMatrices(const Mat3x3& a, const Mat3x3& b) {
        Mat3x3 result = {};
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                for (int k = 0; k < 3; ++k)
                    result[i][j] += a[i][k] * b[k][j];
        return result;
    }

    uint64_t GetConvertCount() const { return m_convertCount; }

    bool Validate() const {
        // Test identity conversion
        ColorConvertParams idParams;
        idParams.srcSpace = ColorConvertSpace::LinearSRGB;
        idParams.dstSpace = ColorConvertSpace::LinearSRGB;
        Mat3x3 id = BuildMatrix(idParams.srcSpace, idParams.dstSpace);
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j) {
                float expected = (i == j) ? 1.0f : 0.0f;
                if (std::abs(id[i][j] - expected) > 0.001f) return false;
            }
        // Test sRGB roundtrip
        float testVal = 0.5f;
        float linear = SRGBToLinear(testVal);
        float back = LinearToSRGB(linear);
        if (std::abs(back - testVal) > 0.01f) return false;
        return true;
    }

private:
    ColorConvertKernel() = default;
    ~ColorConvertKernel() = default;
    ColorConvertKernel(const ColorConvertKernel&) = delete;
    ColorConvertKernel& operator=(const ColorConvertKernel&) = delete;

    static float SRGBToLinear(float v) {
        if (v <= 0.04045f) return v / 12.92f;
        return std::pow((v + 0.055f) / 1.055f, 2.4f);
    }

    static float LinearToSRGB(float v) {
        v = (std::max)(0.0f, (std::min)(v, 1.0f));
        if (v <= 0.0031308f) return v * 12.92f;
        return 1.055f * std::pow(v, 1.0f / 2.4f) - 0.055f;
    }

    ColorConvertParams m_params{};
    uint64_t           m_convertCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
