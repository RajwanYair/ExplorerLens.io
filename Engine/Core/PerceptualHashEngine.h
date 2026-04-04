// PerceptualHashEngine.h — Perceptual Image Hashing for Duplicate Detection
// Copyright (c) 2026 ExplorerLens Project
//
// Implements average-hash (aHash), difference-hash (dHash), and Hamming
// distance comparison for perceptual image similarity.  Operates on raw
// pixel arrays — no image codec dependencies.
//
#pragma once

#include <windows.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// A 64-bit perceptual hash value.
using PHash = uint64_t;

/// Simple grayscale pixel buffer used as input.
struct GrayImage
{
    std::vector<uint8_t> pixels;  // row-major, 1 byte per pixel
    uint32_t width = 0;
    uint32_t height = 0;
};

class PerceptualHashEngine
{
  public:
    PerceptualHashEngine() = default;

    // ── Hash computation ─────────────────────────────────────────────

    /// Average Hash (aHash): resize to 8×8, compare each pixel against the
    /// mean luminance.  Returns a 64-bit hash.
    PHash ComputeAverageHash(const uint8_t* pixels, uint32_t w, uint32_t h) const
    {
        if (!pixels || w == 0 || h == 0)
            return 0;

        // Down-sample to 8×8 using nearest-neighbour
        uint8_t thumb8x8[64]{};
        Downsample8x8(pixels, w, h, thumb8x8);

        // Mean
        uint32_t sum = 0;
        for (int i = 0; i < 64; ++i)
            sum += thumb8x8[i];
        uint8_t mean = static_cast<uint8_t>(sum / 64);

        // Build hash: bit = 1 if pixel >= mean
        PHash hash = 0;
        for (int i = 0; i < 64; ++i) {
            if (thumb8x8[i] >= mean)
                hash |= (1ULL << i);
        }
        return hash;
    }

    /// Difference Hash (dHash): resize to 9×8, compare adjacent horizontal
    /// pixels.  Returns a 64-bit hash.
    PHash ComputeDifferenceHash(const uint8_t* pixels, uint32_t w, uint32_t h) const
    {
        if (!pixels || w == 0 || h == 0)
            return 0;

        // Down-sample to 9×8 (72 pixels)
        uint8_t thumb9x8[72]{};
        DownsampleNxM(pixels, w, h, 9, 8, thumb9x8);

        // Each row: compare pixel[col] < pixel[col+1] → bit
        PHash hash = 0;
        int bit = 0;
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                int idx = row * 9 + col;
                if (thumb9x8[idx] < thumb9x8[idx + 1])
                    hash |= (1ULL << bit);
                ++bit;
            }
        }
        return hash;
    }

    // ── Comparison ───────────────────────────────────────────────────

    /// Hamming distance between two 64-bit hashes (0 = identical, 64 = max).
    uint32_t HammingDistance(PHash hash1, PHash hash2) const
    {
        uint64_t diff = hash1 ^ hash2;
        return PopCount64(diff);
    }

    /// Returns true if two hashes are within the similarity threshold.
    /// Default threshold = 10 bits (out of 64).
    bool AreSimilar(PHash hash1, PHash hash2, uint32_t threshold = 10) const
    {
        return HammingDistance(hash1, hash2) <= threshold;
    }

    /// Normalised similarity score in [0.0, 1.0] where 1.0 = identical.
    float SimilarityScore(PHash hash1, PHash hash2) const
    {
        uint32_t dist = HammingDistance(hash1, hash2);
        return 1.0f - static_cast<float>(dist) / 64.0f;
    }

    // ── Utility ──────────────────────────────────────────────────────

    /// Convert a hash to a 16-char hex string.
    std::wstring HashToString(PHash hash) const
    {
        std::wstringstream ss;
        ss << std::hex << std::setfill(L'0') << std::setw(16) << hash;
        return ss.str();
    }

    /// Parse a 16-char hex string back to PHash.  Returns 0 on failure.
    PHash StringToHash(const std::wstring& str) const
    {
        PHash result = 0;
        std::wstringstream ss(str);
        ss >> std::hex >> result;
        return result;
    }

    /// Create a GrayImage from ARGB pixel data (uint32_t per pixel).
    static GrayImage ArgbToGray(const uint32_t* argb, uint32_t w, uint32_t h)
    {
        GrayImage img;
        img.width = w;
        img.height = h;
        img.pixels.resize(static_cast<size_t>(w) * h);
        for (size_t i = 0; i < img.pixels.size(); ++i) {
            uint32_t px = argb[i];
            uint8_t r = static_cast<uint8_t>((px >> 16) & 0xFF);
            uint8_t g = static_cast<uint8_t>((px >> 8) & 0xFF);
            uint8_t b = static_cast<uint8_t>(px & 0xFF);
            // ITU-R BT.601 luma
            img.pixels[i] = static_cast<uint8_t>(0.299f * r + 0.587f * g + 0.114f * b);
        }
        return img;
    }

    /// Rotate a hash 90° (transpose the 8×8 bit-grid).
    PHash RotateHash90(PHash hash) const
    {
        PHash rotated = 0;
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                int srcBit = row * 8 + col;
                int dstBit = col * 8 + (7 - row);
                if (hash & (1ULL << srcBit))
                    rotated |= (1ULL << dstBit);
            }
        }
        return rotated;
    }

  private:
    // ── Internal helpers ─────────────────────────────────────────────

    /// Nearest-neighbour downsample to exactly 8×8.
    static void Downsample8x8(const uint8_t* src, uint32_t srcW, uint32_t srcH, uint8_t dst[64])
    {
        DownsampleNxM(src, srcW, srcH, 8, 8, dst);
    }

    /// Nearest-neighbour downsample to dstW × dstH.
    static void DownsampleNxM(const uint8_t* src, uint32_t srcW, uint32_t srcH, uint32_t dstW, uint32_t dstH,
                              uint8_t* dst)
    {
        for (uint32_t y = 0; y < dstH; ++y) {
            uint32_t sy = y * srcH / dstH;
            sy = (std::min)(sy, srcH - 1);
            for (uint32_t x = 0; x < dstW; ++x) {
                uint32_t sx = x * srcW / dstW;
                sx = (std::min)(sx, srcW - 1);
                dst[y * dstW + x] = src[sy * srcW + sx];
            }
        }
    }

    /// Portable 64-bit popcount.
    static uint32_t PopCount64(uint64_t v)
    {
        // Kernighan's bit-counting
        uint32_t count = 0;
        while (v) {
            v &= (v - 1);
            ++count;
        }
        return count;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
