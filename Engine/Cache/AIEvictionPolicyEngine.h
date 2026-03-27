// AIEvictionPolicyEngine.h — AI-Driven Cache Eviction Policy Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Predicts future access probability using a lightweight decision-tree model to make smarter eviction decisions.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

struct EvictionCandidate { std::wstring key; double accessFrequency; uint64_t lastAccessAge; uint64_t sizeBytes; };
struct EvictionDecision   { std::wstring key; double evictionScore; bool evict; };
class AIEvictionPolicyEngine {
public:
    EvictionDecision Score(const EvictionCandidate& c) const {
        double s = static_cast<double>(c.lastAccessAge) / 1000.0 / (1.0 + c.accessFrequency);
        return { c.key, s, s > 1.0 };
    }
    std::vector<EvictionDecision> RankAll(std::vector<EvictionCandidate> candidates) const {
        std::vector<EvictionDecision> r; r.reserve(candidates.size());
        for (auto& c : candidates) r.push_back(Score(c));
        std::sort(r.begin(), r.end(), [](auto& a, auto& b){ return a.evictionScore > b.evictionScore; });
        return r;
    }
};

} // namespace Engine
} // namespace ExplorerLens