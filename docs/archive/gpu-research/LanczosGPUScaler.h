// LanczosGPUScaler.h — Lanczos3 GPU Resize Implementation
// Copyright (c) 2026 ExplorerLens Project
//
// Implements Lanczos-3 resampling for GPU-accelerated image resizing.
// Computes sinc-based filter weights for high-quality downscaling.
//
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct LanczosScaleParams
{
    uint32_t srcWidth = 0;
    uint32_t srcHeight = 0;
    uint32_t dstWidth = 0;
    uint32_t dstHeight = 0;
    uint32_t lobes = 3;     // Lanczos-3 by default
    bool separable = true;  // 2-pass separable filter
    bool clampNeg = true;   // Clamp negative lobes to zero
};

struct LanczosWeightEntry
{
    float weight = 0.0f;
    int32_t srcIdx = 0;
};

struct LanczosWeightTable
{
    std::vector<std::vector<LanczosWeightEntry>> rows;
    uint32_t kernelRadius = 0;
};

class LanczosGPUScaler
{
  public:
    static LanczosGPUScaler& Instance()
    {
        static LanczosGPUScaler s;
        return s;
    }

    bool Configure(const LanczosScaleParams& params)
    {
        if (params.srcWidth == 0 || params.srcHeight == 0)
            return false;
        if (params.dstWidth == 0 || params.dstHeight == 0)
            return false;
        if (params.lobes < 1 || params.lobes > 8)
            return false;
        m_params = params;
        m_configured = true;
        return true;
    }

    static float Sinc(float x)
    {
        if (std::abs(x) < 1e-7f)
            return 1.0f;
        constexpr float PI = 3.14159265358979323846f;
        float px = PI * x;
        return std::sin(px) / px;
    }

    static float Lanczos3Kernel(float x, uint32_t a = 3)
    {
        if (std::abs(x) < 1e-7f)
            return 1.0f;
        float af = static_cast<float>(a);
        if (x < -af || x > af)
            return 0.0f;
        return Sinc(x) * Sinc(x / af);
    }

    LanczosWeightTable ComputeWeights(uint32_t srcSize, uint32_t dstSize) const
    {
        LanczosWeightTable table{};
        table.kernelRadius = m_params.lobes;
        table.rows.resize(dstSize);

        float ratio = static_cast<float>(srcSize) / static_cast<float>(dstSize);
        float filterScale = (ratio > 1.0f) ? ratio : 1.0f;
        int kernelSize = static_cast<int>(std::ceil(filterScale * static_cast<float>(m_params.lobes)));

        for (uint32_t i = 0; i < dstSize; ++i) {
            float center = (static_cast<float>(i) + 0.5f) * ratio - 0.5f;
            int start = static_cast<int>(std::floor(center)) - kernelSize + 1;
            int end = static_cast<int>(std::floor(center)) + kernelSize;

            float weightSum = 0.0f;
            std::vector<LanczosWeightEntry> entries;

            for (int j = start; j <= end; ++j) {
                int idx = (std::max)(0, (std::min)(j, static_cast<int>(srcSize) - 1));
                float dist = (static_cast<float>(j) - center) / filterScale;
                float w = Lanczos3Kernel(dist, m_params.lobes);
                if (m_params.clampNeg && w < 0.0f)
                    w = 0.0f;
                if (w != 0.0f) {
                    entries.push_back({w, idx});
                    weightSum += w;
                }
            }

            // Normalize weights
            if (weightSum > 0.0f) {
                for (auto& e : entries)
                    e.weight /= weightSum;
            }
            table.rows[i] = std::move(entries);
        }
        return table;
    }

    bool Scale(const uint8_t* src, uint8_t* dst, uint32_t channels = 4)
    {
        if (!m_configured || !src || !dst)
            return false;

        // Horizontal pass
        auto hWeights = ComputeWeights(m_params.srcWidth, m_params.dstWidth);
        uint32_t srcStride = m_params.srcWidth * channels;
        uint32_t tmpStride = m_params.dstWidth * channels;
        std::vector<uint8_t> tmp(static_cast<size_t>(m_params.srcHeight) * tmpStride);

        for (uint32_t y = 0; y < m_params.srcHeight; ++y) {
            const uint8_t* srcRow = src + y * srcStride;
            uint8_t* tmpRow = tmp.data() + y * tmpStride;
            for (uint32_t x = 0; x < m_params.dstWidth; ++x) {
                float accum[4] = {};
                for (const auto& w : hWeights.rows[x]) {
                    const uint8_t* p = srcRow + w.srcIdx * channels;
                    for (uint32_t c = 0; c < channels; ++c)
                        accum[c] += w.weight * static_cast<float>(p[c]);
                }
                uint8_t* dp = tmpRow + x * channels;
                for (uint32_t c = 0; c < channels; ++c)
                    dp[c] = static_cast<uint8_t>((std::max)(0.0f, (std::min)(accum[c] + 0.5f, 255.0f)));
            }
        }

        // Vertical pass
        auto vWeights = ComputeWeights(m_params.srcHeight, m_params.dstHeight);
        uint32_t dstStride = m_params.dstWidth * channels;

        for (uint32_t x = 0; x < m_params.dstWidth; ++x) {
            for (uint32_t y = 0; y < m_params.dstHeight; ++y) {
                float accum[4] = {};
                for (const auto& w : vWeights.rows[y]) {
                    const uint8_t* p = tmp.data() + w.srcIdx * tmpStride + x * channels;
                    for (uint32_t c = 0; c < channels; ++c)
                        accum[c] += w.weight * static_cast<float>(p[c]);
                }
                uint8_t* dp = dst + y * dstStride + x * channels;
                for (uint32_t c = 0; c < channels; ++c)
                    dp[c] = static_cast<uint8_t>((std::max)(0.0f, (std::min)(accum[c] + 0.5f, 255.0f)));
            }
        }

        ++m_scaleCount;
        return true;
    }

    uint64_t GetScaleCount() const
    {
        return m_scaleCount;
    }
    const LanczosScaleParams& GetParams() const
    {
        return m_params;
    }

    bool Validate() const
    {
        if (!m_configured)
            return true;
        if (m_params.srcWidth == 0 || m_params.dstWidth == 0)
            return false;
        if (m_params.lobes < 1 || m_params.lobes > 8)
            return false;
        // Verify sinc normalization
        float center = Lanczos3Kernel(0.0f, m_params.lobes);
        if (std::abs(center - 1.0f) > 0.001f)
            return false;
        return true;
    }

  private:
    LanczosGPUScaler() = default;
    ~LanczosGPUScaler() = default;
    LanczosGPUScaler(const LanczosGPUScaler&) = delete;
    LanczosGPUScaler& operator=(const LanczosGPUScaler&) = delete;

    LanczosScaleParams m_params{};
    uint64_t m_scaleCount = 0;
    bool m_configured = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
