// SmartThumbnailRanker.h — Thumbnail Candidate Quality Ranking
// Copyright (c) 2026 ExplorerLens Project
//
// Ranks multiple thumbnail candidates (e.g., different video frames,
// PDF pages) to select the most visually appealing representative.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

enum class RankingCriteria : uint8_t {
    Sharpness,
    Colorfulness,
    Contrast,
    FacePresence,
    TextContent,
    Composition,
    Balanced
};

struct ThumbnailCandidate {
    uint32_t candidateId = 0;
    float sharpnessScore = 0.0f;
    float colorfulnessScore = 0.0f;
    float contrastScore = 0.0f;
    float compositionScore = 0.0f;
    float overallScore = 0.0f;
    uint32_t sourceIndex = 0;
    std::string sourceDescription;
};

struct RankingConfig {
    RankingCriteria primaryCriteria = RankingCriteria::Balanced;
    float sharpnessWeight = 0.3f;
    float colorfulnessWeight = 0.25f;
    float contrastWeight = 0.25f;
    float compositionWeight = 0.2f;
    uint32_t maxCandidates = 10;
};

class SmartThumbnailRanker {
public:
    explicit SmartThumbnailRanker(RankingConfig config = {})
        : m_config(config) {
    }

    void AddCandidate(ThumbnailCandidate candidate) {
        candidate.candidateId = m_nextId++;
        candidate.overallScore = ComputeOverallScore(candidate);
        m_candidates.push_back(candidate);
        if (m_candidates.size() > m_config.maxCandidates) {
            std::sort(m_candidates.begin(), m_candidates.end(),
                [](const auto& a, const auto& b) { return a.overallScore > b.overallScore; });
            m_candidates.resize(m_config.maxCandidates);
        }
    }

    const ThumbnailCandidate* GetBestCandidate() const {
        if (m_candidates.empty()) return nullptr;
        const ThumbnailCandidate* best = &m_candidates[0];
        for (const auto& c : m_candidates) {
            if (c.overallScore > best->overallScore) best = &c;
        }
        return best;
    }

    std::vector<ThumbnailCandidate> GetRankedCandidates() const {
        auto sorted = m_candidates;
        std::sort(sorted.begin(), sorted.end(),
            [](const auto& a, const auto& b) { return a.overallScore > b.overallScore; });
        return sorted;
    }

    size_t GetCandidateCount() const { return m_candidates.size(); }
    void SetConfig(const RankingConfig& config) { m_config = config; }
    RankingConfig GetConfig() const { return m_config; }
    void ClearCandidates() { m_candidates.clear(); }

private:
    float ComputeOverallScore(const ThumbnailCandidate& c) const {
        return c.sharpnessScore * m_config.sharpnessWeight +
            c.colorfulnessScore * m_config.colorfulnessWeight +
            c.contrastScore * m_config.contrastWeight +
            c.compositionScore * m_config.compositionWeight;
    }

    std::vector<ThumbnailCandidate> m_candidates;
    RankingConfig m_config;
    uint32_t m_nextId = 1;
};

} // namespace Engine
} // namespace ExplorerLens
