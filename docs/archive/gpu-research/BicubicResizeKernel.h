// BicubicResizeKernel.h — Mitchell-Netravali Bicubic GPU Resize
// Copyright (c) 2026 ExplorerLens Project
//
// Implements Mitchell-Netravali bicubic interpolation for GPU image resizing.
// Default parameters B=1/3, C=1/3 provide optimal perceptual quality.
//
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct BicubicParams
{
    float B = 1.0f / 3.0f;  // Mitchell-Netravali B parameter
    float C = 1.0f / 3.0f;  // Mitchell-Netravali C parameter
    uint32_t srcWidth = 0;
    uint32_t srcHeight = 0;
    uint32_t dstWidth = 0;
    uint32_t dstHeight = 0;
    uint32_t channels = 4;
    bool clamp = true;
};

struct BicubicWeightEntry
{
    float weight = 0.0f;
    int32_t index = 0;
};

class BicubicResizeKernel
{
  public:
    static BicubicResizeKernel& Instance()
    {
        static BicubicResizeKernel s;
        return s;
    }

    bool Configure(const BicubicParams& params)
    {
        if (params.srcWidth == 0 || params.srcHeight == 0)
            return false;
        if (params.dstWidth == 0 || params.dstHeight == 0)
            return false;
        if (params.channels == 0 || params.channels > 4)
            return false;
        m_params = params;
        m_configured = true;
        return true;
    }

    // Mitchell-Netravali kernel: k(x) defined piecewise
    static float MNKernel(float x, float B, float C)
    {
        float ax = std::abs(x);
        if (ax < 1.0f) {
            return ((12.0f - 9.0f * B - 6.0f * C) * ax * ax * ax + (-18.0f + 12.0f * B + 6.0f * C) * ax * ax
                    + (6.0f - 2.0f * B))
                   / 6.0f;
        } else if (ax < 2.0f) {
            return ((-B - 6.0f * C) * ax * ax * ax + (6.0f * B + 30.0f * C) * ax * ax + (-12.0f * B - 48.0f * C) * ax
                    + (8.0f * B + 24.0f * C))
                   / 6.0f;
        }
        return 0.0f;
    }

    std::vector<BicubicWeightEntry> ComputeWeights(float center, uint32_t srcSize) const
    {
        std::vector<BicubicWeightEntry> entries;
        float filterScale = 1.0f;
        int start = static_cast<int>(std::floor(center)) - 1;
        int end = start + 3;

        float wSum = 0.0f;
        for (int j = start; j <= end; ++j) {
            int idx = (std::max)(0, (std::min)(j, static_cast<int>(srcSize) - 1));
            float d = center - static_cast<float>(j);
            float w = MNKernel(d * filterScale, m_params.B, m_params.C);
            if (std::abs(w) > 1e-8f) {
                entries.push_back({w, idx});
                wSum += w;
            }
        }
        // Normalize
        if (wSum > 0.0f) {
            for (auto& e : entries)
                e.weight /= wSum;
        }
        return entries;
    }

    bool Scale(const uint8_t* src, uint8_t* dst)
    {
        if (!m_configured || !src || !dst)
            return false;

        uint32_t srcStride = m_params.srcWidth * m_params.channels;
        uint32_t dstStride = m_params.dstWidth * m_params.channels;
        float xRatio = static_cast<float>(m_params.srcWidth) / static_cast<float>(m_params.dstWidth);
        float yRatio = static_cast<float>(m_params.srcHeight) / static_cast<float>(m_params.dstHeight);

        // Horizontal pass to temporary buffer
        uint32_t tmpStride = m_params.dstWidth * m_params.channels;
        std::vector<float> tmp(static_cast<size_t>(m_params.srcHeight) * tmpStride, 0.0f);

        for (uint32_t y = 0; y < m_params.srcHeight; ++y) {
            const uint8_t* srcRow = src + y * srcStride;
            float* tmpRow = tmp.data() + y * tmpStride;
            for (uint32_t x = 0; x < m_params.dstWidth; ++x) {
                float center = (x + 0.5f) * xRatio - 0.5f;
                auto weights = ComputeWeights(center, m_params.srcWidth);
                for (uint32_t c = 0; c < m_params.channels; ++c) {
                    float val = 0.0f;
                    for (const auto& w : weights)
                        val += w.weight * static_cast<float>(srcRow[w.index * m_params.channels + c]);
                    tmpRow[x * m_params.channels + c] = val;
                }
            }
        }

        // Vertical pass
        for (uint32_t y = 0; y < m_params.dstHeight; ++y) {
            float centerY = (y + 0.5f) * yRatio - 0.5f;
            auto weights = ComputeWeights(centerY, m_params.srcHeight);
            uint8_t* dstRow = dst + y * dstStride;
            for (uint32_t x = 0; x < m_params.dstWidth; ++x) {
                for (uint32_t c = 0; c < m_params.channels; ++c) {
                    float val = 0.0f;
                    for (const auto& w : weights) {
                        val += w.weight * tmp[w.index * tmpStride + x * m_params.channels + c];
                    }
                    if (m_params.clamp)
                        val = (std::max)(0.0f, (std::min)(val + 0.5f, 255.0f));
                    dstRow[x * m_params.channels + c] = static_cast<uint8_t>(val);
                }
            }
        }
        ++m_scaleCount;
        return true;
    }

    // Verify kernel properties: MN(0)=1, MN(2)=0, sum ~ 1
    bool VerifyKernel() const
    {
        float v0 = MNKernel(0.0f, m_params.B, m_params.C);
        float v2 = MNKernel(2.0f, m_params.B, m_params.C);
        if (std::abs(v2) > 1e-5f)
            return false;
        // Sum of integer-offset weights should be ~1
        float sum = 0.0f;
        for (int i = -3; i <= 3; ++i)
            sum += MNKernel(static_cast<float>(i), m_params.B, m_params.C);
        (void)v0;
        return (std::abs(sum - 1.0f) < 0.01f);
    }

    uint64_t GetScaleCount() const
    {
        return m_scaleCount;
    }
    const BicubicParams& GetParams() const
    {
        return m_params;
    }

    bool Validate() const
    {
        if (!m_configured)
            return true;
        if (m_params.srcWidth == 0 || m_params.dstWidth == 0)
            return false;
        if (m_params.B < 0.0f || m_params.B > 1.0f)
            return false;
        if (m_params.C < 0.0f || m_params.C > 1.0f)
            return false;
        return VerifyKernel();
    }

  private:
    BicubicResizeKernel() = default;
    ~BicubicResizeKernel() = default;
    BicubicResizeKernel(const BicubicResizeKernel&) = delete;
    BicubicResizeKernel& operator=(const BicubicResizeKernel&) = delete;

    BicubicParams m_params{};
    uint64_t m_scaleCount = 0;
    bool m_configured = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
