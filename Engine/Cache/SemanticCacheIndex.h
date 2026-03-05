// SemanticCacheIndex.h — Content-Based Semantic Cache Indexing
// Copyright (c) 2026 ExplorerLens Project
//
// Content-based semantic cache indexing. Uses visual similarity fingerprints
// for cache deduplication, avoiding redundant storage of near-identical thumbnails.
//
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <mutex>
#include <cmath>
#include <array>
#include <numeric>

namespace ExplorerLens {
namespace Engine {

struct SemanticFingerprint {
    std::array<uint64_t, 4> hash = {};
    std::array<float, 16> featureVector = {};
    uint32_t width = 0;
    uint32_t height = 0;
    float avgLuminance = 0.0f;
    float edgeDensity = 0.0f;
};

struct SimilarityResult {
    std::string matchKey;
    double similarity = 0.0;
    bool isDuplicate = false;
    double hammingDistance = 0.0;
    double cosineSimilarity = 0.0;
};

struct SemanticIndexStats {
    uint64_t totalEntries = 0;
    uint64_t duplicatesFound = 0;
    uint64_t bytesSaved = 0;
    double avgSimilarityThreshold = 0.95;
    uint64_t lookupCount = 0;
};

class SemanticCacheIndex {
public:
    static SemanticCacheIndex& Instance() {
        static SemanticCacheIndex instance;
        return instance;
    }

    inline SemanticFingerprint ComputeFingerprint(const uint8_t* pixels, uint32_t width, uint32_t height,
        uint32_t channels = 4) const {
        SemanticFingerprint fp;
        fp.width = width;
        fp.height = height;
        if (!pixels || width == 0 || height == 0) return fp;

        fp.avgLuminance = ComputeAvgLuminance(pixels, width, height, channels);

        uint32_t blockW = (std::max)(1u, width / 4);
        uint32_t blockH = (std::max)(1u, height / 4);
        for (uint32_t by = 0; by < 4; ++by) {
            for (uint32_t bx = 0; bx < 4; ++bx) {
                float blockAvg = 0.0f;
                uint32_t count = 0;
                for (uint32_t y = by * blockH; y < (std::min)((by + 1) * blockH, height); ++y) {
                    for (uint32_t x = bx * blockW; x < (std::min)((bx + 1)* blockW, width); ++x) {
                        size_t idx = (static_cast<size_t>(y) * width + x) * channels;
                        float lum = 0.299f * pixels[idx] + 0.587f * pixels[idx + (std::min)(1u, channels - 1)] +
                            0.114f * pixels[idx + (std::min)(2u, channels - 1)];
                        blockAvg += lum;
                        ++count;
                    }
                }
                fp.featureVector[by * 4 + bx] = count > 0 ? blockAvg / count : 0.0f;
            }
        }

        for (uint32_t i = 0; i < 4; ++i) {
            uint64_t bits = 0;
            for (uint32_t b = 0; b < 4; ++b) {
                uint32_t fidx = i * 4 + b;
                if (fp.featureVector[fidx] > fp.avgLuminance) {
                    bits |= (1ULL << b);
                }
            }
            fp.hash[i] = bits;
        }

        fp.edgeDensity = ComputeEdgeDensity(pixels, width, height, channels);
        return fp;
    }

    inline void Insert(const std::string& key, const SemanticFingerprint& fingerprint) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_index[key] = fingerprint;
        m_stats.totalEntries = m_index.size();
    }

    inline SimilarityResult FindMostSimilar(const SemanticFingerprint& query,
        double threshold = 0.95) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.lookupCount++;

        SimilarityResult best;
        best.similarity = 0.0;

        for (const auto& [key, fp] : m_index) {
            double cosine = ComputeCosineSimilarity(query.featureVector, fp.featureVector);
            double hamming = ComputeHammingDistance(query.hash, fp.hash);
            double combined = 0.7 * cosine + 0.3 * (1.0 - hamming / 256.0);

            if (combined > best.similarity) {
                best.matchKey = key;
                best.similarity = combined;
                best.cosineSimilarity = cosine;
                best.hammingDistance = hamming;
            }
        }

        best.isDuplicate = best.similarity >= threshold;
        return best;
    }

    inline bool Remove(const std::string& key) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto removed = m_index.erase(key) > 0;
        if (removed) m_stats.totalEntries = m_index.size();
        return removed;
    }

    inline SemanticIndexStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

private:
    SemanticCacheIndex() = default;

    inline float ComputeAvgLuminance(const uint8_t* pixels, uint32_t w, uint32_t h, uint32_t c) const {
        double sum = 0.0;
        size_t count = static_cast<size_t>(w) * h;
        size_t step = (std::max)(static_cast<size_t>(1), count / 1024);
        size_t sampled = 0;
        for (size_t i = 0; i < count; i += step) {
            size_t idx = i * c;
            sum += 0.299 * pixels[idx] + 0.587 * pixels[idx + (std::min)(1u, c - 1)] +
                0.114 * pixels[idx + (std::min)(2u, c - 1)];
            ++sampled;
        }
        return sampled > 0 ? static_cast<float>(sum / sampled) : 0.0f;
    }

    inline float ComputeEdgeDensity(const uint8_t* pixels, uint32_t w, uint32_t h, uint32_t c) const {
        if (w < 3 || h < 3) return 0.0f;
        uint32_t edges = 0;
        uint32_t total = 0;
        uint32_t step = (std::max)(1u, (std::min)(w, h) / 16);
        for (uint32_t y = 1; y < h - 1; y += step) {
            for (uint32_t x = 1; x < w - 1; x += step) {
                size_t idx = (static_cast<size_t>(y) * w + x) * c;
                size_t idxR = (static_cast<size_t>(y) * w + x + 1) * c;
                size_t idxD = (static_cast<size_t>((y + 1)) * w + x) * c;
                int dx = std::abs(static_cast<int>(pixels[idx]) - static_cast<int>(pixels[idxR]));
                int dy = std::abs(static_cast<int>(pixels[idx]) - static_cast<int>(pixels[idxD]));
                if (dx + dy > 30) ++edges;
                ++total;
            }
        }
        return total > 0 ? static_cast<float>(edges) / total : 0.0f;
    }

    inline double ComputeCosineSimilarity(const std::array<float, 16>& a, const std::array<float, 16>& b) const {
        double dot = 0.0, normA = 0.0, normB = 0.0;
        for (size_t i = 0; i < 16; ++i) {
            dot += a[i] * b[i];
            normA += a[i] * a[i];
            normB += b[i] * b[i];
        }
        double denom = std::sqrt(normA) * std::sqrt(normB);
        return denom > 1e-10 ? dot / denom : 0.0;
    }

    inline double ComputeHammingDistance(const std::array<uint64_t, 4>& a, const std::array<uint64_t, 4>& b) const {
        uint32_t dist = 0;
        for (size_t i = 0; i < 4; ++i) {
            uint64_t x = a[i] ^ b[i];
            while (x) { dist += x & 1; x >>= 1; }
        }
        return static_cast<double>(dist);
    }

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, SemanticFingerprint> m_index;
    mutable SemanticIndexStats m_stats;
};

}
} // namespace ExplorerLens::Engine
