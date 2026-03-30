// SearchResultDeduplicator.h — Near-Duplicate Detection via Perceptual Hash Clustering
// Copyright (c) 2026 ExplorerLens Project
//
// Identifies and clusters near-duplicate images using perceptual hashing (pHash)
// with Hamming distance thresholds, reducing redundancy in search results.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

static constexpr uint32_t PHASH_BITS             = 64;
static constexpr uint32_t DEFAULT_HAMMING_THRESH  = 8;
static constexpr uint32_t DCT_BLOCK_SIZE         = 8;
static constexpr uint32_t DEDUP_MAX_CANDIDATES   = 50000;

struct DedupCandidate {
    uint64_t fileId;
    uint64_t perceptualHash;
    float    searchScore;
};

struct DedupCluster {
    uint64_t              representativeId;
    std::vector<uint64_t> memberIds;
    float                 bestScore;
};

struct DedupResult {
    std::vector<DedupCandidate> uniqueResults;
    std::vector<DedupCluster>   clusters;
    uint32_t                    totalInput     = 0;
    uint32_t                    totalDeduped   = 0;
    uint32_t                    duplicatesFound = 0;
};

struct DeduplicatorConfig {
    uint32_t hammingThreshold = DEFAULT_HAMMING_THRESH;
    uint32_t maxCandidates    = DEDUP_MAX_CANDIDATES;
    bool     keepBestScore    = true;
};

class SearchResultDeduplicator {
public:
    inline bool Initialize(const DeduplicatorConfig& config) {
        m_config = config;
        m_initialized = true;
        return true;
    }

    inline DedupResult Deduplicate(const std::vector<DedupCandidate>& candidates) const {
        DedupResult result;
        if (!m_initialized || candidates.empty()) return result;

        result.totalInput = static_cast<uint32_t>(candidates.size());
        std::vector<bool> clustered(candidates.size(), false);

        for (size_t i = 0; i < candidates.size(); ++i) {
            if (clustered[i]) continue;

            DedupCluster cluster;
            cluster.representativeId = candidates[i].fileId;
            cluster.bestScore = candidates[i].searchScore;
            cluster.memberIds.push_back(candidates[i].fileId);

            for (size_t j = i + 1; j < candidates.size(); ++j) {
                if (clustered[j]) continue;
                uint32_t dist = HammingDistance(candidates[i].perceptualHash,
                                                candidates[j].perceptualHash);
                if (dist <= m_config.hammingThreshold) {
                    clustered[j] = true;
                    cluster.memberIds.push_back(candidates[j].fileId);
                    if (candidates[j].searchScore > cluster.bestScore) {
                        cluster.bestScore = candidates[j].searchScore;
                        if (m_config.keepBestScore)
                            cluster.representativeId = candidates[j].fileId;
                    }
                }
            }

            result.uniqueResults.push_back(candidates[i]);
            if (cluster.memberIds.size() > 1)
                result.clusters.push_back(std::move(cluster));
        }

        result.totalDeduped = static_cast<uint32_t>(result.uniqueResults.size());
        result.duplicatesFound = result.totalInput - result.totalDeduped;
        return result;
    }

    inline uint64_t ComputePerceptualHash(const uint8_t* grayPixels,
                                          uint32_t width, uint32_t height) const {
        if (!grayPixels || width < DCT_BLOCK_SIZE || height < DCT_BLOCK_SIZE) return 0;

        float block[DCT_BLOCK_SIZE * DCT_BLOCK_SIZE];
        float mean = 0.0f;
        for (uint32_t y = 0; y < DCT_BLOCK_SIZE; ++y) {
            for (uint32_t x = 0; x < DCT_BLOCK_SIZE; ++x) {
                uint32_t sx = x * width / DCT_BLOCK_SIZE;
                uint32_t sy = y * height / DCT_BLOCK_SIZE;
                block[y * DCT_BLOCK_SIZE + x] = static_cast<float>(grayPixels[sy * width + sx]);
                mean += block[y * DCT_BLOCK_SIZE + x];
            }
        }
        mean /= (DCT_BLOCK_SIZE * DCT_BLOCK_SIZE);

        uint64_t hash = 0;
        for (uint32_t i = 0; i < PHASH_BITS; ++i) {
            if (block[i] > mean) hash |= (1ULL << i);
        }
        return hash;
    }

    inline uint32_t GetClusterCount(const DedupResult& result) const {
        return static_cast<uint32_t>(result.clusters.size());
    }

    inline bool IsInitialized() const { return m_initialized; }

private:
    inline uint32_t HammingDistance(uint64_t a, uint64_t b) const {
        uint64_t diff = a ^ b;
        uint32_t count = 0;
        while (diff) { count += diff & 1; diff >>= 1; }
        return count;
    }

    DeduplicatorConfig m_config;
    bool               m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
