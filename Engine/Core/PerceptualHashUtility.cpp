// PerceptualHashUtility.cpp — Perceptual image hashing implementation
// Copyright (c) 2026 ExplorerLens Project
//
// dHash, pHash, aHash implementations operating on BGRA32 pixel buffers.
//
#include "PerceptualHashUtility.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

namespace {

// Convert BGRA32 pixel at (x,y) to luminance in [0,1]
inline float Luminance(std::span<const uint8_t> px, uint32_t x, uint32_t y, uint32_t stride) {
    const uint8_t* p = px.data() + (y * stride + x) * 4;
    // ITU-R BT.601 luma (note: BGRA order)
    return (0.114f * p[0] + 0.587f * p[1] + 0.299f * p[2]) / 255.0f;
}

// Bilinear sample into float grayscale grid of dstW × dstH
std::vector<float> DownsampleGrayscaleImpl(std::span<const uint8_t> pixels,
                                           uint32_t srcW, uint32_t srcH,
                                           uint32_t dstW, uint32_t dstH) {
    std::vector<float> grid(dstW * dstH);
    const float sx = static_cast<float>(srcW) / dstW;
    const float sy = static_cast<float>(srcH) / dstH;
    for (uint32_t oy = 0; oy < dstH; ++oy) {
        for (uint32_t ox = 0; ox < dstW; ++ox) {
            float fx = (ox + 0.5f) * sx - 0.5f;
            float fy = (oy + 0.5f) * sy - 0.5f;
            float left = std::floor(fx), top = std::floor(fy);
            float fracX = fx - left, fracY = fy - top;
            uint32_t x0 = (uint32_t)(std::max)(0.f, left);
            uint32_t y0 = (uint32_t)(std::max)(0.f, top);
            uint32_t x1 = (std::min)(x0 + 1, srcW - 1);
            uint32_t y1 = (std::min)(y0 + 1, srcH - 1);

            float tl = Luminance(pixels, x0, y0, srcW);
            float tr = Luminance(pixels, x1, y0, srcW);
            float bl = Luminance(pixels, x0, y1, srcW);
            float br = Luminance(pixels, x1, y1, srcW);

            float val = tl * (1-fracX)*(1-fracY) + tr * fracX*(1-fracY)
                      + bl * (1-fracX)*fracY      + br * fracX*fracY;
            grid[oy * dstW + ox] = val;
        }
    }
    return grid;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// PerceptualHashUtility::DownsampleGrayscale (public helper → calls impl)
// ---------------------------------------------------------------------------

std::vector<float> PerceptualHashUtility::DownsampleGrayscale(
    std::span<const uint8_t> pixels, uint32_t srcW, uint32_t srcH,
    uint32_t dstW, uint32_t dstH) {
    return DownsampleGrayscaleImpl(pixels, srcW, srcH, dstW, dstH);
}

// ---------------------------------------------------------------------------
// dHash
// ---------------------------------------------------------------------------

PerceptualHash PerceptualHashUtility::DHash(std::span<const uint8_t> pixels,
                                             uint32_t srcW, uint32_t srcH) {
    if (pixels.size() < static_cast<size_t>(srcW) * srcH * 4)
        throw std::invalid_argument("PerceptualHashUtility::DHash: pixel buffer too small");

    // 9 columns × 8 rows for horizontal adjacency comparisons
    auto grid = DownsampleGrayscaleImpl(pixels, srcW, srcH, 9, 8);
    uint64_t hash = 0;
    for (uint32_t row = 0; row < 8; ++row) {
        for (uint32_t col = 0; col < 8; ++col) {
            float left  = grid[row * 9 + col];
            float right = grid[row * 9 + col + 1];
            if (left < right) hash |= (1ULL << (row * 8 + col));
        }
    }
    return hash;
}

// ---------------------------------------------------------------------------
// aHash
// ---------------------------------------------------------------------------

PerceptualHash PerceptualHashUtility::AHash(std::span<const uint8_t> pixels,
                                             uint32_t srcW, uint32_t srcH) {
    if (pixels.size() < static_cast<size_t>(srcW) * srcH * 4)
        throw std::invalid_argument("PerceptualHashUtility::AHash: pixel buffer too small");

    auto grid = DownsampleGrayscaleImpl(pixels, srcW, srcH, 8, 8);
    float mean = std::accumulate(grid.begin(), grid.end(), 0.f) / 64.f;
    uint64_t hash = 0;
    for (int i = 0; i < 64; ++i) {
        if (grid[i] >= mean) hash |= (1ULL << i);
    }
    return hash;
}

// ---------------------------------------------------------------------------
// pHash — DCT-based
// ---------------------------------------------------------------------------

void PerceptualHashUtility::DCT1D(std::vector<double>& v, uint32_t N) {
    // Type-II DCT using simple O(N^2) formula — sufficient for N ≤ 32
    std::vector<double> out(N);
    for (uint32_t k = 0; k < N; ++k) {
        double sum = 0;
        for (uint32_t n = 0; n < N; ++n) {
            sum += v[n] * std::cos(M_PI * k * (2.0 * n + 1.0) / (2.0 * N));
        }
        out[k] = sum;
    }
    v = std::move(out);
}

PerceptualHash PerceptualHashUtility::PHash(std::span<const uint8_t> pixels,
                                             uint32_t srcW, uint32_t srcH) {
    if (pixels.size() < static_cast<size_t>(srcW) * srcH * 4)
        throw std::invalid_argument("PerceptualHashUtility::PHash: pixel buffer too small");

    constexpr uint32_t N = 32;
    auto grid = DownsampleGrayscaleImpl(pixels, srcW, srcH, N, N);

    // 2-D DCT: row-major.  Apply 1-D DCT to each row, then each column.
    std::vector<std::vector<double>> dct(N, std::vector<double>(N));
    for (uint32_t r = 0; r < N; ++r) {
        std::vector<double> row(N);
        for (uint32_t c = 0; c < N; ++c) row[c] = grid[r * N + c];
        DCT1D(row, N);
        for (uint32_t c = 0; c < N; ++c) dct[r][c] = row[c];
    }
    for (uint32_t c = 0; c < N; ++c) {
        std::vector<double> col(N);
        for (uint32_t r = 0; r < N; ++r) col[r] = dct[r][c];
        DCT1D(col, N);
        for (uint32_t r = 0; r < N; ++r) dct[r][c] = col[r];
    }

    // Extract 8×8 low-frequency block (skip DC at [0][0])
    constexpr uint32_t LF = 8;
    double sum = 0;
    std::array<double, 64> block{};
    int idx = 0;
    for (uint32_t r = 0; r < LF; ++r) {
        for (uint32_t c = 0; c < LF; ++c) {
            if (r == 0 && c == 0) continue; // skip DC
            block[idx++] = dct[r][c];
            sum += dct[r][c];
        }
    }
    double mean = sum / idx;

    uint64_t hash = 0;
    for (int i = 0; i < idx; ++i) {
        if (block[i] > mean) hash |= (1ULL << i);
    }
    return hash;
}

// ---------------------------------------------------------------------------
// String conversion
// ---------------------------------------------------------------------------

std::string PerceptualHashUtility::ToHexString(PerceptualHash h) {
    char buf[17];
    snprintf(buf, sizeof(buf), "%016llx", static_cast<unsigned long long>(h));
    return buf;
}

PerceptualHash PerceptualHashUtility::FromHexString(std::string_view hex) {
    if (hex.size() != 16)
        throw std::invalid_argument("PerceptualHash hex string must be exactly 16 characters");
    uint64_t val = 0;
    for (char c : hex) {
        val <<= 4;
        if      (c >= '0' && c <= '9') val |= static_cast<uint64_t>(c - '0');
        else if (c >= 'a' && c <= 'f') val |= static_cast<uint64_t>(c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') val |= static_cast<uint64_t>(c - 'A' + 10);
        else throw std::invalid_argument(std::string("Invalid hex character: ") + c);
    }
    return val;
}

} // namespace Engine
} // namespace ExplorerLens
