// SSIMComparator.cpp — SSIM implementation
// Copyright (c) 2026 ExplorerLens Project
//
#include "SSIMComparator.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// SSIM stability constants (Wang et al. 2004)
static constexpr float K1 = 0.01f;
static constexpr float K2 = 0.03f;
// L = 255 for 8-bit channel
static constexpr float L  = 255.0f;
static const float C1 = (K1 * L) * (K1 * L);  // (0.01*255)^2
static const float C2 = (K2 * L) * (K2 * L);  // (0.03*255)^2

// ---------------------------------------------------------------------------
// GaussianKernel
// ---------------------------------------------------------------------------

std::vector<float> SSIMComparator::GaussianKernel(uint32_t n, float sigma) {
    assert(n % 2 == 1);
    std::vector<float> k(n);
    float sum = 0;
    int half = static_cast<int>(n) / 2;
    for (int i = -half; i <= half; ++i) {
        float val = std::exp(-(i * i) / (2 * sigma * sigma));
        k[i + half] = val;
        sum += val;
    }
    for (auto& v : k) v /= sum;
    return k;
}

// ---------------------------------------------------------------------------
// Internal: extract a single 8-bit channel into float plane
//   channel 0=B, 1=G, 2=R (BGRA layout)
// ---------------------------------------------------------------------------

static std::vector<float> ExtractChannel(std::span<const uint8_t> bgra,
                                          uint32_t w, uint32_t h,
                                          uint32_t ch) {
    std::vector<float> out(w * h);
    for (uint32_t i = 0; i < w * h; ++i) {
        out[i] = static_cast<float>(bgra[i * 4 + ch]);
    }
    return out;
}

// Apply separable 1-D Gaussian filter: convolve rows then columns
static std::vector<float> GaussianBlur(const std::vector<float>& src,
                                        uint32_t w, uint32_t h,
                                        std::span<const float> kernel) {
    int kHalf = static_cast<int>(kernel.size()) / 2;
    std::vector<float> tmp(w * h, 0.f);

    // Horizontal pass
    for (uint32_t oy = 0; oy < h; ++oy) {
        for (uint32_t ox = 0; ox < w; ++ox) {
            float acc = 0;
            for (int k = -kHalf; k <= kHalf; ++k) {
                uint32_t sx = static_cast<uint32_t>(
                    (std::clamp)(static_cast<int>(ox) + k, 0, static_cast<int>(w) - 1));
                acc += src[oy * w + sx] * kernel[k + kHalf];
            }
            tmp[oy * w + ox] = acc;
        }
    }

    // Vertical pass
    std::vector<float> out(w * h, 0.f);
    for (uint32_t oy = 0; oy < h; ++oy) {
        for (uint32_t ox = 0; ox < w; ++ox) {
            float acc = 0;
            for (int k = -kHalf; k <= kHalf; ++k) {
                uint32_t sy = static_cast<uint32_t>(
                    (std::clamp)(static_cast<int>(oy) + k, 0, static_cast<int>(h) - 1));
                acc += tmp[sy * w + ox] * kernel[k + kHalf];
            }
            out[oy * w + ox] = acc;
        }
    }
    return out;
}

// ---------------------------------------------------------------------------
// SSIMSingleChannel
// ---------------------------------------------------------------------------

float SSIMComparator::SSIMSingleChannel(const std::vector<float>& p1,
                                         const std::vector<float>& p2,
                                         uint32_t w, uint32_t h,
                                         std::span<const float> kernel,
                                         uint32_t /*kSize*/) {
    auto mu1 = GaussianBlur(p1, w, h, kernel);
    auto mu2 = GaussianBlur(p2, w, h, kernel);

    // Compute mu^2, mu1*mu2, sigma^2, sigma12
    size_t N = w * h;
    std::vector<float> p1sq(N), p2sq(N), p1p2(N);
    for (size_t i = 0; i < N; ++i) {
        p1sq[i] = p1[i] * p1[i];
        p2sq[i] = p2[i] * p2[i];
        p1p2[i] = p1[i] * p2[i];
    }

    auto sigma1sq = GaussianBlur(p1sq, w, h, kernel);
    auto sigma2sq = GaussianBlur(p2sq, w, h, kernel);
    auto sigma12  = GaussianBlur(p1p2, w, h, kernel);

    float ssimSum = 0;
    for (size_t i = 0; i < N; ++i) {
        float u1 = mu1[i], u2 = mu2[i];
        float s1 = sigma1sq[i] - u1 * u1;
        float s2 = sigma2sq[i] - u2 * u2;
        float s12 = sigma12[i]  - u1 * u2;
        float numerator   = (2 * u1 * u2 + C1) * (2 * s12 + C2);
        float denominator = (u1*u1 + u2*u2 + C1) * (s1 + s2 + C2);
        ssimSum += numerator / denominator;
    }
    return ssimSum / static_cast<float>(N);
}

// ---------------------------------------------------------------------------
// SSIMComparator::Compare
// ---------------------------------------------------------------------------

SSIMResult SSIMComparator::Compare(std::span<const uint8_t> a,
                                    std::span<const uint8_t> b,
                                    uint32_t width, uint32_t height,
                                    uint32_t windowSize) {
    size_t required = static_cast<size_t>(width) * height * 4;
    if (a.size() < required || b.size() < required)
        throw std::invalid_argument("SSIMComparator::Compare: pixel buffer too small");
    if (width == 0 || height == 0)
        throw std::invalid_argument("SSIMComparator::Compare: zero dimensions");

    auto kernel = GaussianKernel(windowSize);

    // Extract luma plane Y = 0.299R + 0.587G + 0.114B  (BGRA order)
    std::vector<float> lumaA(width * height), lumaB(width * height);
    for (size_t i = 0; i < width * height; ++i) {
        lumaA[i] = 0.114f * a[i*4+0] + 0.587f * a[i*4+1] + 0.299f * a[i*4+2];
        lumaB[i] = 0.114f * b[i*4+0] + 0.587f * b[i*4+1] + 0.299f * b[i*4+2];
    }

    float s = SSIMSingleChannel(lumaA, lumaB, width, height, kernel, windowSize);

    // Decompose SSIM into L×C×S components (approximate: use full SSIM for structure)
    SSIMResult res;
    res.ssim       = s;
    res.structure  = s;     // full structural term
    res.luminance  = s;
    res.contrast   = s;
    res.isIdentical = (s >= SSIM_IDENTICAL_THRESHOLD);
    return res;
}

// ---------------------------------------------------------------------------
// SSIMComparator::CompareChannels
// ---------------------------------------------------------------------------

SSIMComparator::ChannelSSIM SSIMComparator::CompareChannels(
    std::span<const uint8_t> a, std::span<const uint8_t> b,
    uint32_t w, uint32_t h, uint32_t windowSize) {
    size_t req = static_cast<size_t>(w) * h * 4;
    if (a.size() < req || b.size() < req)
        throw std::invalid_argument("SSIMComparator::CompareChannels: buffer too small");

    auto kernel = GaussianKernel(windowSize);
    ChannelSSIM out;
    out.b = SSIMSingleChannel(ExtractChannel(a, w, h, 0), ExtractChannel(b, w, h, 0), w, h, kernel, windowSize);
    out.g = SSIMSingleChannel(ExtractChannel(a, w, h, 1), ExtractChannel(b, w, h, 1), w, h, kernel, windowSize);
    out.r = SSIMSingleChannel(ExtractChannel(a, w, h, 2), ExtractChannel(b, w, h, 2), w, h, kernel, windowSize);
    out.luminance = (out.r + out.g + out.b) / 3.0f;
    return out;
}

// ---------------------------------------------------------------------------
// SSIMComparator::PassesThreshold
// ---------------------------------------------------------------------------

bool SSIMComparator::PassesThreshold(std::span<const uint8_t> a,
                                      std::span<const uint8_t> b,
                                      uint32_t w, uint32_t h,
                                      float threshold) {
    return Compare(a, b, w, h).ssim >= threshold;
}

} // namespace Engine
} // namespace ExplorerLens
