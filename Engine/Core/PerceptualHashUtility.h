// PerceptualHashUtility.h — Perceptual image hashing for decoder regression testing
// Copyright (c) 2026 ExplorerLens Project
//
// Provides dHash (difference hash) and pHash (perceptual DCT hash) for
// detecting visual regressions in decoder output between versions.
// All hash operations are header-only and operate on raw BGRA pixel buffers.
//
#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <span>
#include <string>
#include <stdexcept>

namespace ExplorerLens {
namespace Engine {

// 64-bit dHash or pHash packed as uint64_t.
using PerceptualHash = uint64_t;

// Hamming distance between two hash values (number of differing bits, 0–64).
inline int HammingDistance(PerceptualHash a, PerceptualHash b) noexcept {
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
    return static_cast<int>(__popcnt64(a ^ b));
#else
    uint64_t x = a ^ b;
    int count = 0;
    while (x) { count += (x & 1); x >>= 1; }
    return count;
#endif
}

// Threshold for "visually identical" thumbnails (empirically calibrated):
//   0       = identical
//   <= 6    = very likely the same image, minor encoding differences
//   <= 10   = similar, possible color/scale shift
//   > 10    = different images
constexpr int PHASH_NEAR_IDENTICAL_THRESHOLD = 6;

// PerceptualHashUtility
//
// Works exclusively on 32-bit BGRA pixel buffers (the engine's native format).
// The caller allocates and owns the pixel data.
class PerceptualHashUtility {
public:
    // dHash (Difference Hash)
    //
    // Resize src (BGRA32, any size) to 9×8 grayscale, compare adjacent
    // horizontal pixels, produce a 64-bit hash.  Very fast; tolerates
    // slight JPEG re-encoding differences.
    //
    // Throws std::invalid_argument if pixels.size() < srcWidth * srcHeight * 4
    static PerceptualHash DHash(std::span<const uint8_t> pixels,
                                uint32_t srcWidth,
                                uint32_t srcHeight);

    // pHash (Perceptual DCT Hash)
    //
    // Resize to 32×32 grayscale, apply 2-D DCT, take the 8×8 low-frequency
    // block (skipping DC), threshold against the block mean.  Returns a 64-bit
    // hash.  More robust than dHash for rotated / slightly-cropped images.
    //
    // Throws std::invalid_argument if pixels.size() < srcWidth * srcHeight * 4
    static PerceptualHash PHash(std::span<const uint8_t> pixels,
                                uint32_t srcWidth,
                                uint32_t srcHeight);

    // aHash (Average Hash) — very fast, lower quality
    static PerceptualHash AHash(std::span<const uint8_t> pixels,
                                uint32_t srcWidth,
                                uint32_t srcHeight);

    // Hex string representation of a hash (16 lowercase hex chars)
    static std::string ToHexString(PerceptualHash h);

    // Parse a hash from a hex string (throws std::invalid_argument if malformed)
    static PerceptualHash FromHexString(std::string_view hex);

    // Returns true if two hashes are "visually identical" within threshold
    static bool AreVisuallyIdentical(PerceptualHash a, PerceptualHash b,
                                      int threshold = PHASH_NEAR_IDENTICAL_THRESHOLD) noexcept {
        return HammingDistance(a, b) <= threshold;
    }

private:
    // Bilinear downsample BGRA32 → grayscale float grid of wOut×hOut.
    static std::vector<float> DownsampleGrayscale(std::span<const uint8_t> pixels,
                                                   uint32_t srcW, uint32_t srcH,
                                                   uint32_t dstW, uint32_t dstH);
    // 1-D discrete cosine transform (in-place, double precision, N must be power-of-2 friendly)
    static void DCT1D(std::vector<double>& v, uint32_t N);
};

} // namespace Engine
} // namespace ExplorerLens
