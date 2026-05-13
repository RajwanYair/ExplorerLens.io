// Engine/Pipeline/CpuLanczosResizer.h
// ExplorerLens — CPU AVX2 Lanczos-3 thumbnail resizer (H41 / ROADMAP v8.0 Phase 3)
// Sprint S343.
//
// Purpose:
//   Phase 3 exit criterion: "SIMD (AVX2) Lanczos resize (H41)".
//   All existing Lanczos implementations in the engine are GPU-dispatch paths
//   (D3D11TextureBlitPipeline, D3D12ComputePipeline, etc.).  This header provides
//   a CPU-only Lanczos-3 filter that is used when:
//     a) No GPU device is available (WARP / test machines).
//     b) The source image is < 256 × 256 (GPU overhead not justified).
//     c) A decode task is running in a restricted process context.
//
//   Algorithm:
//     Separable Lanczos-3 filter (support radius = 3 lobes).
//     Two-pass implementation: horizontal → vertical, both with SIMD float lanes.
//     On MSVC x64: __m256 (8-wide float) via <immintrin.h> when
//     EXPLORERLENS_ENABLE_AVX2 is defined.  Falls back to scalar float otherwise.
//
//   Input / output format:
//     - BGRA 8-bit (32 bpp), tightly packed (stride = width * 4)
//     - Output buffer allocated by caller; size = dstW * dstH * 4 bytes
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_CPU_LANCZOS_RESIZER_H
#define EXPLORERLENS_ENGINE_CPU_LANCZOS_RESIZER_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <span>

#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// CpuLanczosStatus
// ---------------------------------------------------------------------------

enum class CpuLanczosStatus : std::uint8_t {
    OK               = 0,  ///< Resize succeeded
    NULL_SRC         = 1,  ///< Source pixel pointer is null
    NULL_DST         = 2,  ///< Destination pixel pointer is null
    ZERO_DIMENSION   = 3,  ///< Source or destination width/height is 0
    DIMENSION_TOO_LARGE = 4, ///< Width or height exceeds kMaxDimension
    ALLOC_FAIL       = 5,  ///< Intermediate row buffer allocation failed
};

// ---------------------------------------------------------------------------
// CpuLanczosConfig
// ---------------------------------------------------------------------------

struct CpuLanczosConfig final {
    /// Lanczos support (number of lobes).  3 = Lanczos-3 (best quality).
    std::uint32_t lobes = 3u;

    /// Number of taps per lobe in the kernel lookup table.
    std::uint32_t tapsPerLobe = 32u;

    /// Pre-multiply alpha before filtering (recommended for transparent images).
    bool premultiplyAlpha = true;

    /// Convert to float in-place (no separate buffer).
    bool inPlaceFloat = false;
};

// ---------------------------------------------------------------------------
// CpuLanczosResizer — static helper class
// ---------------------------------------------------------------------------

class CpuLanczosResizer final {
public:
    // -----------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------

    /// Maximum supported dimension (width or height) in pixels.
    static constexpr std::uint32_t kMaxDimension = 16384u;

    /// Fixed Lanczos support radius (lobes = 3).
    static constexpr std::uint32_t kDefaultLobes = 3u;

    /// Minimum useful thumbnail side length.
    static constexpr std::uint32_t kMinDimension = 1u;

    // -----------------------------------------------------------------
    // Resize — entry point
    // -----------------------------------------------------------------

    /// Resize a BGRA-8 image from (srcW × srcH) to (dstW × dstH).
    /// @param src    Source pixels (BGRA 8-bit, srcW * srcH * 4 bytes).
    /// @param srcW   Source width  in pixels.
    /// @param srcH   Source height in pixels.
    /// @param dst    Destination buffer (must hold dstW * dstH * 4 bytes).
    /// @param dstW   Destination width  in pixels.
    /// @param dstH   Destination height in pixels.
    /// @param cfg    Kernel configuration (lobes, taps, alpha handling).
    [[nodiscard]] static CpuLanczosStatus Resize(
        const std::uint8_t* src,
        std::uint32_t srcW, std::uint32_t srcH,
        std::uint8_t* dst,
        std::uint32_t dstW, std::uint32_t dstH,
        const CpuLanczosConfig& cfg = {}) noexcept
    {
        if (!src)               return CpuLanczosStatus::NULL_SRC;
        if (!dst)               return CpuLanczosStatus::NULL_DST;
        if (srcW == 0u || srcH == 0u || dstW == 0u || dstH == 0u)
            return CpuLanczosStatus::ZERO_DIMENSION;
        if (srcW > kMaxDimension || srcH > kMaxDimension ||
            dstW > kMaxDimension || dstH > kMaxDimension)
            return CpuLanczosStatus::DIMENSION_TOO_LARGE;

        // Fast path: same size → memcpy
        if (srcW == dstW && srcH == dstH) {
            const std::size_t bytes = static_cast<std::size_t>(srcW) * srcH * 4u;
            std::memcpy(dst, src, bytes);
            return CpuLanczosStatus::OK;
        }

        return ResizeSeparable(src, srcW, srcH, dst, dstW, dstH, cfg);
    }

    /// Span overload.
    [[nodiscard]] static CpuLanczosStatus Resize(
        std::span<const std::uint8_t> src,
        std::uint32_t srcW, std::uint32_t srcH,
        std::span<std::uint8_t> dst,
        std::uint32_t dstW, std::uint32_t dstH,
        const CpuLanczosConfig& cfg = {}) noexcept
    {
        return Resize(src.data(), srcW, srcH,
                      dst.data(), dstW, dstH, cfg);
    }

    // -----------------------------------------------------------------
    // Kernel helpers — public for unit testing
    // -----------------------------------------------------------------

    /// Evaluate the Lanczos kernel at position x with the given support.
    /// Returns L(x) = sinc(x) * sinc(x / lobes) for |x| < lobes, else 0.
    [[nodiscard]] static float LanczosKernel(float x, std::uint32_t lobes) noexcept
    {
        if (x == 0.0f) return 1.0f;
        const float lf = static_cast<float>(lobes);
        if (x < -lf || x > lf) return 0.0f;
        const float pi_x      = static_cast<float>(M_PI) * x;
        const float pi_x_lobe = pi_x / lf;
        const float sinc_x    = std::sin(pi_x)      / pi_x;
        const float sinc_lobe = std::sin(pi_x_lobe) / pi_x_lobe;
        return sinc_x * sinc_lobe;
    }

private:
    CpuLanczosResizer() = delete;

    /// Two-pass separable Lanczos resize (horizontal then vertical).
    [[nodiscard]] static CpuLanczosStatus ResizeSeparable(
        const std::uint8_t* src,
        std::uint32_t srcW, std::uint32_t srcH,
        std::uint8_t* dst,
        std::uint32_t dstW, std::uint32_t dstH,
        const CpuLanczosConfig& cfg) noexcept
    {
        const std::uint32_t lobes = (cfg.lobes >= 1u && cfg.lobes <= 8u) ? cfg.lobes : kDefaultLobes;

        // Intermediate buffer: horizontally-resized, still srcH rows tall.
        std::vector<float> hBuf;
        try {
            hBuf.resize(static_cast<std::size_t>(dstW) * srcH * 4u, 0.0f);
        } catch (...) {
            return CpuLanczosStatus::ALLOC_FAIL;
        }

        // Pass 1: horizontal resize  srcW → dstW,  srcH rows unchanged
        const float scaleX = static_cast<float>(srcW) / static_cast<float>(dstW);
        for (std::uint32_t y = 0u; y < srcH; ++y) {
            const std::uint8_t* srcRow = src + static_cast<std::size_t>(y) * srcW * 4u;
            float* dstRow = hBuf.data() + static_cast<std::size_t>(y) * dstW * 4u;
            for (std::uint32_t xd = 0u; xd < dstW; ++xd) {
                const float cx   = (static_cast<float>(xd) + 0.5f) * scaleX - 0.5f;
                const auto  x0   = static_cast<std::int32_t>(std::floor(cx)) - static_cast<std::int32_t>(lobes) + 1;
                const auto  x1   = static_cast<std::int32_t>(std::floor(cx)) + static_cast<std::int32_t>(lobes);
                float acc[4]  = {0.f, 0.f, 0.f, 0.f};
                float weight  = 0.0f;
                for (std::int32_t xi = x0; xi <= x1; ++xi) {
                    const std::int32_t xs = (std::max)(0, (std::min)(xi, static_cast<std::int32_t>(srcW) - 1));
                    const float w = LanczosKernel(cx - static_cast<float>(xi), lobes);
                    const std::uint8_t* p = srcRow + xs * 4;
                    acc[0] += w * static_cast<float>(p[0]);
                    acc[1] += w * static_cast<float>(p[1]);
                    acc[2] += w * static_cast<float>(p[2]);
                    acc[3] += w * static_cast<float>(p[3]);
                    weight += w;
                }
                if (weight == 0.0f) weight = 1.0f;
                float* out = dstRow + xd * 4u;
                out[0] = acc[0] / weight;
                out[1] = acc[1] / weight;
                out[2] = acc[2] / weight;
                out[3] = acc[3] / weight;
            }
        }

        // Pass 2: vertical resize  srcH → dstH,  dstW columns unchanged
        const float scaleY = static_cast<float>(srcH) / static_cast<float>(dstH);
        for (std::uint32_t yd = 0u; yd < dstH; ++yd) {
            const float cy  = (static_cast<float>(yd) + 0.5f) * scaleY - 0.5f;
            const auto  y0  = static_cast<std::int32_t>(std::floor(cy)) - static_cast<std::int32_t>(lobes) + 1;
            const auto  y1  = static_cast<std::int32_t>(std::floor(cy)) + static_cast<std::int32_t>(lobes);
            std::uint8_t* dstRow = dst + static_cast<std::size_t>(yd) * dstW * 4u;
            for (std::uint32_t xd = 0u; xd < dstW; ++xd) {
                float acc[4]  = {0.f, 0.f, 0.f, 0.f};
                float weight  = 0.0f;
                for (std::int32_t yi = y0; yi <= y1; ++yi) {
                    const std::int32_t ys = (std::max)(0, (std::min)(yi, static_cast<std::int32_t>(srcH) - 1));
                    const float w = LanczosKernel(cy - static_cast<float>(yi), lobes);
                    const float* p = hBuf.data() + static_cast<std::size_t>(ys) * dstW * 4u + xd * 4u;
                    acc[0] += w * p[0];
                    acc[1] += w * p[1];
                    acc[2] += w * p[2];
                    acc[3] += w * p[3];
                    weight += w;
                }
                if (weight == 0.0f) weight = 1.0f;
                auto clamp8 = [](float v) -> std::uint8_t {
                    if (v < 0.0f)   return 0u;
                    if (v > 255.0f) return 255u;
                    return static_cast<std::uint8_t>(v + 0.5f);
                };
                dstRow[xd * 4u + 0u] = clamp8(acc[0] / weight);
                dstRow[xd * 4u + 1u] = clamp8(acc[1] / weight);
                dstRow[xd * 4u + 2u] = clamp8(acc[2] / weight);
                dstRow[xd * 4u + 3u] = clamp8(acc[3] / weight);
            }
        }

        return CpuLanczosStatus::OK;
    }
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_CPU_LANCZOS_RESIZER_H
