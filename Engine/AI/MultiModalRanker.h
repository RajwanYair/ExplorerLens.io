// MultiModalRanker.h — Multi-Modal Ranking Fusion
// Copyright (c) 2026 ExplorerLens Project
//
// Fuses CLIP visual similarity, BM25 text relevance, file recency, and file
// size signals into a single ranked result list with configurable weights.
//
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

static constexpr float DEFAULT_WEIGHT_CLIP    = 0.50f;
static constexpr float DEFAULT_WEIGHT_BM25    = 0.25f;
static constexpr float DEFAULT_WEIGHT_RECENCY = 0.15f;
static constexpr float DEFAULT_WEIGHT_SIZE    = 0.10f;
static constexpr uint32_t MAX_RANK_RESULTS    = 10000;

enum class RankSignal : uint8_t {
    CLIPSimilarity = 0,
    BM25Text       = 1,
    Recency        = 2,
    FileSize       = 3,
    COUNT          = 4
};

struct RankWeights {
    float clip    = DEFAULT_WEIGHT_CLIP;
    float bm25    = DEFAULT_WEIGHT_BM25;
    float recency = DEFAULT_WEIGHT_RECENCY;
    float size    = DEFAULT_WEIGHT_SIZE;
};

struct RankCandidate {
    uint64_t    fileId;
    std::string fileName;
    float       clipScore   = 0.0f;
    float       bm25Score   = 0.0f;
    float       recencyScore = 0.0f;
    float       sizeScore   = 0.0f;
};

struct RankedResult {
    uint64_t    fileId;
    std::string fileName;
    float       finalScore  = 0.0f;
    float       clipScore   = 0.0f;
    float       bm25Score   = 0.0f;
};

struct RankerConfig {
    RankWeights weights;
    uint32_t    maxResults     = 100;
    float       minScore       = 0.05f;
    bool        normalizeInput = true;
};

class MultiModalRanker {
public:
    inline bool Initialize(const RankerConfig& config) {
        m_config = config;
        m_weights = config.weights;
        NormalizeWeights();
        m_initialized = true;
        return true;
    }

    inline std::vector<RankedResult> Rank(const std::vector<RankCandidate>& candidates) const {
        if (!m_initialized) return {};

        std::vector<RankedResult> results;
        results.reserve(candidates.size());

        for (const auto& c : candidates) {
            float score = m_weights.clip    * c.clipScore
                        + m_weights.bm25    * c.bm25Score
                        + m_weights.recency * c.recencyScore
                        + m_weights.size    * c.sizeScore;

            if (score < m_config.minScore) continue;

            results.push_back({c.fileId, c.fileName, score, c.clipScore, c.bm25Score});
        }

        std::sort(results.begin(), results.end(),
                  [](const RankedResult& a, const RankedResult& b) { return a.finalScore > b.finalScore; });

        if (results.size() > m_config.maxResults)
            results.resize(m_config.maxResults);

        return results;
    }

    inline void SetWeights(const RankWeights& weights) {
        m_weights = weights;
        NormalizeWeights();
    }

    inline std::vector<RankedResult> GetTopK(const std::vector<RankCandidate>& candidates,
                                             uint32_t k) const {
        auto ranked = Rank(candidates);
        if (ranked.size() > k) ranked.resize(k);
        return ranked;
    }

    inline RankWeights GetWeights() const { return m_weights; }
    inline bool IsInitialized() const { return m_initialized; }

private:
    inline void NormalizeWeights() {
        float sum = m_weights.clip + m_weights.bm25 + m_weights.recency + m_weights.size;
        if (sum > 1e-6f) {
            m_weights.clip    /= sum;
            m_weights.bm25    /= sum;
            m_weights.recency /= sum;
            m_weights.size    /= sum;
        }
    }

    RankerConfig m_config;
    RankWeights  m_weights;
    bool         m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
