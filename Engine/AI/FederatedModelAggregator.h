// FederatedModelAggregator.h — Federated Model Aggregator
// Copyright (c) 2026 ExplorerLens Project
//
// Aggregates local model weight deltas from multiple nodes into a unified global model.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <numeric>
#include <cmath>

namespace ExplorerLens { namespace Engine {

struct FedAggConfig {
    uint32_t minParticipants = 2;
    float    clipNorm        = 1.0f;
};

struct FedAggResult {
    bool               success    = false;
    std::vector<float> weights;
    float              totalNorm  = 0.0f;
    uint32_t           participants = 0;
};

class FederatedModelAggregator {
public:
    explicit FederatedModelAggregator(const FedAggConfig& config) : m_config(config) {}

    FedAggResult Aggregate(const std::vector<std::vector<float>>& deltas) {
        FedAggResult r;
        if (deltas.size() < m_config.minParticipants) return r;
        size_t dim = deltas[0].size();
        r.weights.assign(dim, 0.0f);
        for (const auto& d : deltas)
            for (size_t i = 0; i < std::min(dim, d.size()); ++i)
                r.weights[i] += d[i];
        r.participants = static_cast<uint32_t>(deltas.size());
        float norm = 0.0f;
        for (auto& w : r.weights) {
            w /= static_cast<float>(r.participants);
            norm += w * w;
        }
        r.totalNorm = std::sqrt(norm);
        // Gradient clipping
        if (r.totalNorm > m_config.clipNorm && r.totalNorm > 0.0f) {
            float scale = m_config.clipNorm / r.totalNorm;
            for (auto& w : r.weights) w *= scale;
        }
        r.success = true;
        return r;
    }
    bool HasSufficientParticipants(uint32_t count) const { return count >= m_config.minParticipants; }

private:
    FedAggConfig m_config;
};

}} // namespace ExplorerLens::Engine
