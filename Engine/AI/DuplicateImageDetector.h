// DuplicateImageDetector.h — Perceptual hash-based duplicate detection
// Copyright (c) 2026 ExplorerLens Project
//
// Detects near-duplicate images using perceptual hashing (pHash),
// enabling cache deduplication and duplicate file identification.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct DuplicateImageDetectorConfig {
    bool enabled = true;
    uint32_t hashSize = 64; // bits
    uint32_t similarityThreshold = 8; // max hamming distance
    std::string label = "DuplicateImageDetector";
};

class DuplicateImageDetector {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    DuplicateImageDetectorConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    uint32_t HammingDistance(uint64_t hash1, uint64_t hash2) const {
        uint64_t xorVal = hash1 ^ hash2;
        uint32_t distance = 0;
        while (xorVal) { distance += xorVal & 1; xorVal >>= 1; }
        return distance;
    }

    bool IsDuplicate(uint64_t hash1, uint64_t hash2) const {
        return HammingDistance(hash1, hash2) <= m_config.similarityThreshold;
    }

    bool IsExactMatch(uint64_t hash1, uint64_t hash2) const {
        return hash1 == hash2;
    }

    float SimilarityScore(uint64_t hash1, uint64_t hash2) const {
        uint32_t dist = HammingDistance(hash1, hash2);
        return 1.0f - static_cast<float>(dist) / m_config.hashSize;
    }

private:
    bool m_initialized = false;
    DuplicateImageDetectorConfig m_config;
};

}
} // namespace ExplorerLens::Engine
