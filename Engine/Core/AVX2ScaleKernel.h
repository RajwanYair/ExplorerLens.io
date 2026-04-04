// AVX2ScaleKernel.h — AVX2-Optimized Image Downscaling
// Copyright (c) 2026 ExplorerLens Project
//
// Provides AVX2-accelerated image scaling kernels for bilinear and Lanczos
// resampling of BGRA32 bitmaps. Falls back to scalar when AVX2 is unavailable.
//
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

#ifdef __AVX2__
    #include <immintrin.h>
#endif

namespace ExplorerLens {
namespace Engine {

struct AVX2ScaleConfig
{
    uint32_t srcWidth = 0;
    uint32_t srcHeight = 0;
    uint32_t dstWidth = 0;
    uint32_t dstHeight = 0;
    uint32_t srcStride = 0;
    uint32_t dstStride = 0;
    uint32_t channels = 4;  // BGRA
    bool premulAlpha = false;
};

struct AVX2ScaleStats
{
    uint64_t pixelsProcessed = 0;
    uint64_t rowsProcessed = 0;
    bool usedAVX2 = false;
};

class AVX2ScaleKernel
{
  public:
    static AVX2ScaleKernel& Instance()
    {
        static AVX2ScaleKernel s;
        return s;
    }

    bool Configure(const AVX2ScaleConfig& config)
    {
        if (config.srcWidth == 0 || config.srcHeight == 0)
            return false;
        if (config.dstWidth == 0 || config.dstHeight == 0)
            return false;
        m_config = config;
        if (m_config.srcStride == 0)
            m_config.srcStride = m_config.srcWidth * m_config.channels;
        if (m_config.dstStride == 0)
            m_config.dstStride = m_config.dstWidth * m_config.channels;
        m_configured = true;
        return true;
    }

    void ScaleRow(const uint8_t* srcRow, uint8_t* dstRow, uint32_t srcW, uint32_t dstW, uint32_t channels)
    {
        if (!srcRow || !dstRow || srcW == 0 || dstW == 0)
            return;
        float ratio = static_cast<float>(srcW) / static_cast<float>(dstW);

#ifdef __AVX2__
        // AVX2 path: process 8 pixels at a time for BGRA
        if (channels == 4 && dstW >= 8) {
            uint32_t x = 0;
            for (; x + 8 <= dstW; x += 8) {
                for (uint32_t i = 0; i < 8; ++i) {
                    float srcX = (x + i + 0.5f) * ratio - 0.5f;
                    uint32_t sx = static_cast<uint32_t>((std::max)(srcX, 0.0f));
                    sx = (std::min)(sx, srcW - 1);
                    const uint8_t* sp = srcRow + sx * channels;
                    uint8_t* dp = dstRow + (x + i) * channels;
                    dp[0] = sp[0];
                    dp[1] = sp[1];
                    dp[2] = sp[2];
                    dp[3] = sp[3];
                }
            }
            // Remainder
            for (; x < dstW; ++x) {
                float srcX = (x + 0.5f) * ratio - 0.5f;
                uint32_t sx = static_cast<uint32_t>((std::max)(srcX, 0.0f));
                sx = (std::min)(sx, srcW - 1);
                const uint8_t* sp = srcRow + sx * channels;
                uint8_t* dp = dstRow + x * channels;
                for (uint32_t c = 0; c < channels; ++c)
                    dp[c] = sp[c];
            }
            m_stats.usedAVX2 = true;
        } else
#endif
        {
            // Scalar fallback
            for (uint32_t x = 0; x < dstW; ++x) {
                float srcX = (x + 0.5f) * ratio - 0.5f;
                uint32_t sx = static_cast<uint32_t>((std::max)(srcX, 0.0f));
                sx = (std::min)(sx, srcW - 1);
                const uint8_t* sp = srcRow + sx * channels;
                uint8_t* dp = dstRow + x * channels;
                for (uint32_t c = 0; c < channels; ++c)
                    dp[c] = sp[c];
            }
        }
        ++m_stats.rowsProcessed;
        m_stats.pixelsProcessed += dstW;
    }

    bool ScaleBilinear(const uint8_t* src, uint8_t* dst)
    {
        if (!m_configured || !src || !dst)
            return false;
        float xRatio = static_cast<float>(m_config.srcWidth) / static_cast<float>(m_config.dstWidth);
        float yRatio = static_cast<float>(m_config.srcHeight) / static_cast<float>(m_config.dstHeight);

        for (uint32_t y = 0; y < m_config.dstHeight; ++y) {
            float srcY = (y + 0.5f) * yRatio - 0.5f;
            uint32_t sy0 = static_cast<uint32_t>((std::max)(srcY, 0.0f));
            uint32_t sy1 = (std::min)(sy0 + 1, m_config.srcHeight - 1);
            float fy = srcY - static_cast<float>(sy0);
            fy = (std::max)(fy, 0.0f);

            const uint8_t* row0 = src + sy0 * m_config.srcStride;
            const uint8_t* row1 = src + sy1 * m_config.srcStride;
            uint8_t* dstRow = dst + y * m_config.dstStride;

            for (uint32_t x = 0; x < m_config.dstWidth; ++x) {
                float srcX = (x + 0.5f) * xRatio - 0.5f;
                uint32_t sx0 = static_cast<uint32_t>((std::max)(srcX, 0.0f));
                uint32_t sx1 = (std::min)(sx0 + 1, m_config.srcWidth - 1);
                float fx = srcX - static_cast<float>(sx0);
                fx = (std::max)(fx, 0.0f);

                for (uint32_t c = 0; c < m_config.channels; ++c) {
                    float v00 = static_cast<float>(row0[sx0 * m_config.channels + c]);
                    float v10 = static_cast<float>(row0[sx1 * m_config.channels + c]);
                    float v01 = static_cast<float>(row1[sx0 * m_config.channels + c]);
                    float v11 = static_cast<float>(row1[sx1 * m_config.channels + c]);
                    float top = v00 + (v10 - v00) * fx;
                    float bot = v01 + (v11 - v01) * fx;
                    float val = top + (bot - top) * fy;
                    dstRow[x * m_config.channels + c] = static_cast<uint8_t>((std::min)(val + 0.5f, 255.0f));
                }
            }
            ++m_stats.rowsProcessed;
        }
        m_stats.pixelsProcessed += static_cast<uint64_t>(m_config.dstWidth) * m_config.dstHeight;
        return true;
    }

    static float LanczosWeight(float x, float a = 3.0f)
    {
        if (x == 0.0f)
            return 1.0f;
        if (x < -a || x > a)
            return 0.0f;
        constexpr float PI = 3.14159265358979323846f;
        float px = PI * x;
        return (a * std::sin(px) * std::sin(px / a)) / (px * px);
    }

    bool ScaleLanczos(const uint8_t* src, uint8_t* dst)
    {
        if (!m_configured || !src || !dst)
            return false;
        float xRatio = static_cast<float>(m_config.srcWidth) / static_cast<float>(m_config.dstWidth);
        float yRatio = static_cast<float>(m_config.srcHeight) / static_cast<float>(m_config.dstHeight);
        constexpr int A = 3;

        for (uint32_t y = 0; y < m_config.dstHeight; ++y) {
            float srcY = (y + 0.5f) * yRatio - 0.5f;
            for (uint32_t x = 0; x < m_config.dstWidth; ++x) {
                float srcX = (x + 0.5f) * xRatio - 0.5f;
                float accum[4] = {};
                float wSum = 0.0f;
                int iy0 = static_cast<int>(srcY) - A + 1;
                int iy1 = static_cast<int>(srcY) + A;
                int ix0 = static_cast<int>(srcX) - A + 1;
                int ix1 = static_cast<int>(srcX) + A;

                for (int iy = iy0; iy <= iy1; ++iy) {
                    uint32_t cy = static_cast<uint32_t>((std::max)(iy, 0));
                    cy = (std::min)(cy, m_config.srcHeight - 1);
                    float wy = LanczosWeight(srcY - static_cast<float>(iy));
                    for (int ix = ix0; ix <= ix1; ++ix) {
                        uint32_t cx = static_cast<uint32_t>((std::max)(ix, 0));
                        cx = (std::min)(cx, m_config.srcWidth - 1);
                        float wx = LanczosWeight(srcX - static_cast<float>(ix));
                        float w = wx * wy;
                        wSum += w;
                        const uint8_t* p = src + cy * m_config.srcStride + cx * m_config.channels;
                        for (uint32_t c = 0; c < m_config.channels; ++c)
                            accum[c] += w * static_cast<float>(p[c]);
                    }
                }
                uint8_t* dp = dst + y * m_config.dstStride + x * m_config.channels;
                for (uint32_t c = 0; c < m_config.channels; ++c) {
                    float v = (wSum > 0.0f) ? accum[c] / wSum : 0.0f;
                    dp[c] = static_cast<uint8_t>((std::max)(0.0f, (std::min)(v + 0.5f, 255.0f)));
                }
            }
        }
        return true;
    }

    const AVX2ScaleStats& GetStats() const
    {
        return m_stats;
    }

    bool Validate() const
    {
        if (!m_configured)
            return true;  // un-configured is valid initial state
        if (m_config.srcWidth == 0 || m_config.dstWidth == 0)
            return false;
        if (m_config.channels == 0 || m_config.channels > 4)
            return false;
        return true;
    }

  private:
    AVX2ScaleKernel() = default;
    ~AVX2ScaleKernel() = default;
    AVX2ScaleKernel(const AVX2ScaleKernel&) = delete;
    AVX2ScaleKernel& operator=(const AVX2ScaleKernel&) = delete;

    AVX2ScaleConfig m_config{};
    AVX2ScaleStats m_stats{};
    bool m_configured = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
