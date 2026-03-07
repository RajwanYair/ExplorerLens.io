// ImageSimilarityMatcher.h — Perceptual Image Hashing and Matching
// Copyright (c) 2026 ExplorerLens Project
//
// Computes perceptual hashes (pHash/dHash) for thumbnail deduplication
// and near-duplicate detection in cached results.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <cmath>

namespace ExplorerLens {
namespace Engine {

enum class ImageHashAlgorithm : uint8_t {
    DifferenceHash = 0,   // dHash — gradient-based, fast
    AverageHash = 1,      // aHash — average luminance comparison
    PerceptualHash = 2    // pHash — DCT-based, most robust
};

struct ImageHash {
    uint64_t value = 0;
    HashAlgorithm algorithm = HashAlgorithm::DifferenceHash;
    bool valid = false;
};

struct ImageSimilarityResult {
    double similarity = 0.0; // 0..1 (1 = identical)
    uint32_t hammingDistance = 0;
    bool isNearDuplicate = false;
};

struct SimilarityConfig {
    ImageHashAlgorithm algorithm = ImageHashAlgorithm::DifferenceHash;
    uint32_t hashGridSize = 8;           // 8x8 = 64-bit hash
    double nearDuplicateThreshold = 0.9; // 90% similarity
};

class ImageSimilarityMatcher {
public:
    void Configure(const SimilarityConfig& config) { m_config = config; }

    ImageHash ComputeHash(const uint8_t* rgbaPixels, uint32_t width,
        uint32_t height) const {
        ImageHash hash;
        hash.algorithm = m_config.algorithm;
        if (!rgbaPixels || width == 0 || height == 0) return hash;

        // Downsample to grid
        uint32_t gs = m_config.hashGridSize;
        std::vector<float> grid(gs * gs, 0.0f);
        for (uint32_t gy = 0; gy < gs; ++gy) {
            for (uint32_t gx = 0; gx < gs; ++gx) {
                uint32_t srcX = gx * width / gs;
                uint32_t srcY = gy * height / gs;
                size_t idx = (static_cast<size_t>(srcY) * width + srcX) * 4;
                float lum = 0.299f * rgbaPixels[idx] + 0.587f * rgbaPixels[idx + 1] +
                    0.114f * rgbaPixels[idx + 2];
                grid[gy * gs + gx] = lum;
            }
        }

        switch (m_config.algorithm) {
        case ImageHashAlgorithm::AverageHash: {
            float avgLum = 0.0f;
            for (float l : grid) avgLum += l;
            avgLum /= grid.size();
            for (size_t i = 0; i < 64 && i < grid.size(); ++i) {
                if (grid[i] > avgLum) hash.value |= (1ULL << i);
            }
            break;
        }
        case ImageHashAlgorithm::DifferenceHash:
        default: {
            uint32_t bit = 0;
            for (uint32_t y = 0; y < gs && bit < 64; ++y) {
                for (uint32_t x = 0; x + 1 < gs && bit < 64; ++x) {
                    if (grid[y * gs + x] > grid[y * gs + x + 1])
                        hash.value |= (1ULL << bit);
                    bit++;
                }
            }
            break;
        }
        }
        hash.valid = true;
        return hash;
    }

    ImageSimilarityResult Compare(ImageHash a, ImageHash b) const {
        ImageSimilarityResult result;
        if (!a.valid || !b.valid) return result;
        uint64_t xorVal = a.value ^ b.value;
        // Count differing bits (Hamming distance)
        uint32_t dist = 0;
        while (xorVal) { dist += xorVal & 1; xorVal >>= 1; }
        result.hammingDistance = dist;
        result.similarity = 1.0 - static_cast<double>(dist) / 64.0;
        result.isNearDuplicate = result.similarity >= m_config.nearDuplicateThreshold;
        return result;
    }

private:
    SimilarityConfig m_config;
};

} // namespace Engine
} // namespace ExplorerLens
