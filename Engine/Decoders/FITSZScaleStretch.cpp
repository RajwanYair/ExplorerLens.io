// FITSZScaleStretch.cpp — FITS ZScale Auto-Contrast Algorithm
// Copyright (c) 2026 ExplorerLens Project
//
#include "Decoders/FITSZScaleStretch.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cstring>

namespace ExplorerLens { namespace Engine {

// IRAF ZScale: fit a line to the sorted sub-sample and derive z1/z2.
ZScaleLimits FITSZScaleStretch::ComputeLimits(
    const float* pixels, uint32_t width, uint32_t height,
    const ZScaleOptions& opts) const noexcept
{
    ZScaleLimits lim{};
    if (!pixels || width == 0 || height == 0) return lim;

    const uint32_t total = width * height;
    const uint32_t step  = std::max(1u, total / std::max(1u, opts.nsamples));

    std::vector<float> sample;
    sample.reserve(opts.nsamples);
    for (uint32_t i = 0; i < total && sample.size() < opts.nsamples; i += step) {
        const float v = pixels[i];
        if (std::isfinite(v)) sample.push_back(v);
    }
    if (sample.size() < 5) {
        lim.z1 = *std::min_element(pixels, pixels + total);
        lim.z2 = *std::max_element(pixels, pixels + total);
        lim.valid = true;
        return lim;
    }

    std::sort(sample.begin(), sample.end());
    const uint32_t n = static_cast<uint32_t>(sample.size());

    // Fit a linear trend to [median region], derive z1/z2 from slope.
    const float vmin = sample.front();
    const float vmax = sample.back();
    const float vmid = sample[n / 2];
    const float slope = (n > 1) ? (vmax - vmin) / static_cast<float>(n - 1u) : 1.0f;

    const float halfRange = (opts.contrast > 0.0f)
        ? slope * static_cast<float>(n) * 0.5f / static_cast<float>(opts.contrast)
        : (vmax - vmin) * 0.5f;

    lim.z1    = std::max(static_cast<double>(vmin),
                         static_cast<double>(vmid - halfRange));
    lim.z2    = std::min(static_cast<double>(vmax),
                         static_cast<double>(vmid + halfRange));
    if (lim.z2 <= lim.z1) { lim.z1 = vmin; lim.z2 = vmax; }
    lim.valid = true;
    return lim;
}

uint32_t FITSZScaleStretch::HeatMapBGRA(float t) noexcept
{
    t = std::clamp(t, 0.0f, 1.0f);
    // Black→Blue→Cyan→Green→Yellow→Red→White
    struct Ctrl { float t; uint8_t r, g, b; };
    static const Ctrl ctrl[] = {
        {0.00f, 0,  0,  0},
        {0.20f, 0,  0,255},
        {0.40f, 0,255,255},
        {0.60f, 0,255,  0},
        {0.80f,255,255,  0},
        {0.90f,255,  0,  0},
        {1.00f,255,255,255},
    };
    const int n = static_cast<int>(sizeof(ctrl)/sizeof(ctrl[0]));
    int lo = 0;
    for (int i = 0; i < n-1; ++i) { if (t <= ctrl[i+1].t) { lo = i; break; } }
    const float span = ctrl[lo+1].t - ctrl[lo].t;
    const float frac = (span > 0) ? (t - ctrl[lo].t) / span : 0.0f;
    const uint8_t r = static_cast<uint8_t>(ctrl[lo].r + frac * (ctrl[lo+1].r - ctrl[lo].r));
    const uint8_t g = static_cast<uint8_t>(ctrl[lo].g + frac * (ctrl[lo+1].g - ctrl[lo].g));
    const uint8_t b = static_cast<uint8_t>(ctrl[lo].b + frac * (ctrl[lo+1].b - ctrl[lo].b));
    return b | (static_cast<uint32_t>(g) << 8) | (static_cast<uint32_t>(r) << 16) | 0xFF000000u;
}

FITSStretchResult FITSZScaleStretch::Stretch(
    const float* pixels, uint32_t width, uint32_t height,
    const ZScaleOptions& opts) const noexcept
{
    FITSStretchResult result{};
    if (!pixels || width == 0 || height == 0) return result;

    result.limits = ComputeLimits(pixels, width, height, opts);
    result.width  = width;
    result.height = height;
    result.pixelsBGRA.resize(static_cast<size_t>(width) * height * 4u);

    const float z1 = static_cast<float>(result.limits.z1);
    const float z2 = static_cast<float>(result.limits.z2);
    const float range = (z2 > z1) ? (z2 - z1) : 1.0f;

    uint8_t* dst = result.pixelsBGRA.data();
    for (uint32_t i = 0; i < width * height; ++i) {
        const float t = std::clamp((pixels[i] - z1) / range, 0.0f, 1.0f);
        if (opts.pseudoColor) {
            const uint32_t rgba = HeatMapBGRA(t);
            std::memcpy(dst + i * 4, &rgba, 4);
        } else {
            const uint8_t g = static_cast<uint8_t>(t * 255.0f + 0.5f);
            dst[i*4+0] = g; dst[i*4+1] = g; dst[i*4+2] = g; dst[i*4+3] = 0xFF;
        }
    }
    result.success = true;
    return result;
}

FITSStretchResult FITSZScaleStretch::StretchInt16(
    const int16_t* pixels, uint32_t width, uint32_t height,
    float bscale, float bzero,
    const ZScaleOptions& opts) const noexcept
{
    if (!pixels || width == 0 || height == 0) return {};
    const uint32_t total = width * height;
    std::vector<float> fp(total);
    for (uint32_t i = 0; i < total; ++i)
        fp[i] = static_cast<float>(pixels[i]) * bscale + bzero;
    return Stretch(fp.data(), width, height, opts);
}

}} // namespace ExplorerLens::Engine
