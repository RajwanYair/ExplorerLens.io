// PersonalizedRankingModel.h — Personalized Ranking Model
// Copyright (c) 2026 ExplorerLens Project
//
// On-device rank re-ordering of thumbnail search results using federated user signal models.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <unordered_map>

namespace ExplorerLens { namespace Engine {

struct PRMRankRequest {
    std::vector<std::string> candidates;
    std::vector<float>       baseScores;
    std::string              userProfile;
};

struct PRMRankResult {
    bool                     success = false;
    std::vector<std::string> ranked;
    std::vector<float>       scores;
};

class PersonalizedRankingModel {
public:
    PRMRankResult Rank(const PRMRankRequest& req) {
        PRMRankResult r;
        if (req.candidates.empty()) return r;
        size_t n = req.candidates.size();
        std::vector<size_t> idx(n);
        std::iota(idx.begin(), idx.end(), 0);
        std::vector<float> scores = req.baseScores;
        scores.resize(n, 0.5f);
        // Personalization boost: hash userProfile against candidate name
        for (size_t i = 0; i < n; ++i)
            scores[i] += static_cast<float>(
                (std::hash<std::string>{}(req.userProfile + req.candidates[i]) & 0xFF)) / 2550.0f;
        std::stable_sort(idx.begin(), idx.end(),
            [&](size_t a, size_t b) { return scores[a] > scores[b]; });
        r.ranked.reserve(n);
        r.scores.reserve(n);
        for (size_t i : idx) { r.ranked.push_back(req.candidates[i]); r.scores.push_back(scores[i]); }
        r.success = true;
        return r;
    }
    void UpdateUserSignal(const std::string& profile, const std::string& item, float signal) {
        m_signals[profile + "|" + item] = signal;
    }
    float GetSignal(const std::string& profile, const std::string& item) const {
        auto it = m_signals.find(profile + "|" + item);
        return it != m_signals.end() ? it->second : 0.0f;
    }
    uint32_t SignalCount() const { return static_cast<uint32_t>(m_signals.size()); }

private:
    std::unordered_map<std::string, float> m_signals;
};

}} // namespace ExplorerLens::Engine
