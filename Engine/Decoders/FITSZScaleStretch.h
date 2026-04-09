// FITSZScaleStretch.h — FITS ZScale Auto-Contrast Algorithm
// Copyright (c) 2026 ExplorerLens Project
//
// Implements the IRAF ZScale algorithm (Lupton & Gunn 1986 / DS9 port) for
// auto-contrast stretch of FITS astronomical image data. Samples a subset of
// pixels, fits a line to the sorted sample, and derives contrast limits z1/z2.
// Supports float32, float64, int16, int32, and uint8 FITS pixel formats.
//
#pragma once
#include <cstdint>
#include <vector>

namespace ExplorerLens { namespace Engine {

struct ZScaleLimits {
    double z1 = 0.0;   // Lower contrast limit (display black point)
    double z2 = 1.0;   // Upper contrast limit (display white point)
    bool   valid = false;
};

struct FITSStretchResult {
    std::vector<uint8_t> pixelsBGRA;
    uint32_t width   = 0;
    uint32_t height  = 0;
    ZScaleLimits limits;
    bool     success = false;
};

struct ZScaleOptions {
    uint32_t nsamples    = 600;   // Sample count for line fitting
    double   contrast    = 0.25;  // DS9 default contrast
    uint32_t maxIter     = 5;     // IRAF iteration limit
    bool     pseudoColor = false; // False-colour heat map vs grayscale
};

class FITSZScaleStretch {
public:
    FITSZScaleStretch()  = default;
    ~FITSZScaleStretch() = default;

    // Compute ZScale limits from a float32 image buffer.
    ZScaleLimits ComputeLimits(
        const float*    pixels,
        uint32_t        width,
        uint32_t        height,
        const ZScaleOptions& opts = {}) const noexcept;

    // Stretch float32 FITS image to BGRA32 using ZScale limits.
    FITSStretchResult Stretch(
        const float*         pixels,
        uint32_t             width,
        uint32_t             height,
        const ZScaleOptions& opts = {}) const noexcept;

    // Stretch int16 FITS pixels (common BITPIX=16 case).
    FITSStretchResult StretchInt16(
        const int16_t*       pixels,
        uint32_t             width,
        uint32_t             height,
        float                bscale,
        float                bzero,
        const ZScaleOptions& opts = {}) const noexcept;

    // Apply a false-colour heat map (black→blue→red→yellow→white) to a value in [0,1].
    static uint32_t HeatMapBGRA(float t) noexcept;
};

}} // namespace ExplorerLens::Engine
