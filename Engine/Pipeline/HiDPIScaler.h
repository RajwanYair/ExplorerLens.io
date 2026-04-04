// HiDPIScaler.h — High-Quality BGRA Thumbnail Upscaler
// Copyright (c) 2026 ExplorerLens Project
//
// Provides Lanczos-3, Mitchell-Netravali, and bilinear BGRA rescaling for
// upscaling thumbnails to high-DPI logical sizes with sub-pixel quality.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ---- Scaler Algorithm -------------------------------------------------------

enum class ScalerAlgorithm : uint8_t {
    Bilinear = 0,          // Fast; acceptable for downscale
    Bicubic = 1,           // Mitchell-Netravali B=1/3 C=1/3
    Lanczos3 = 2,          // Best for HiDPI upscale (default)
    AreaAverage = 3,       // Downscale only; optimal for minification
    NearestNeighbour = 4,  // Debug / pixel art
};

// ---- Scale Options ----------------------------------------------------------

struct ScaleOptions
{
    ScalerAlgorithm algorithm = ScalerAlgorithm::Lanczos3;
    bool premultiplyAlpha = false;  // Pre-multiply before filter, un-multiply after
    bool preserveAspectRatio = true;
    bool sharpenOnUpscale = false;  // Apply mild unsharp mask after upscale
    float sharpenAmount = 0.3f;     // Unsharp mask strength (0.0-1.0)
};

// ---- Result -----------------------------------------------------------------

struct ScaleResult
{
    bool success = false;
    std::vector<uint8_t> pixels;  // BGRA, row-major
    uint32_t width = 0;
    uint32_t height = 0;
    std::string note;
};

// ---- HiDPIScaler ------------------------------------------------------------

class HiDPIScaler
{
  public:
    HiDPIScaler() = default;
    ~HiDPIScaler() = default;

    // Scale BGRA pixel buffer from (srcW x srcH) to (dstW x dstH).
    ScaleResult Scale(const uint8_t* srcPixels, uint32_t srcWidth, uint32_t srcHeight, uint32_t dstWidth,
                      uint32_t dstHeight, const ScaleOptions& opts = {}) const;

    // Convenience: scale uniformly by a DPI ratio.
    ScaleResult ScaleByFactor(const uint8_t* srcPixels, uint32_t srcWidth, uint32_t srcHeight, float scaleFactor,
                              const ScaleOptions& opts = {}) const;

    // Recommended algorithm for a given (src -> dst) transition.
    static ScalerAlgorithm RecommendAlgorithm(uint32_t srcW, uint32_t srcH, uint32_t dstW, uint32_t dstH);

  private:
    ScaleResult ScaleLanczos3(const uint8_t* src, uint32_t sw, uint32_t sh, uint32_t dw, uint32_t dh) const;

    ScaleResult ScaleBicubic(const uint8_t* src, uint32_t sw, uint32_t sh, uint32_t dw, uint32_t dh) const;

    ScaleResult ScaleBilinear(const uint8_t* src, uint32_t sw, uint32_t sh, uint32_t dw, uint32_t dh) const;

    ScaleResult ScaleAreaAverage(const uint8_t* src, uint32_t sw, uint32_t sh, uint32_t dw, uint32_t dh) const;

    void ApplyUnsharpMask(std::vector<uint8_t>& pixels, uint32_t w, uint32_t h, float amount) const;

    // Lanczos kernel: sinc(x) * sinc(x/3), |x| < 3
    static float LanczosKernel(float x);
};

}  // namespace Engine
}  // namespace ExplorerLens
