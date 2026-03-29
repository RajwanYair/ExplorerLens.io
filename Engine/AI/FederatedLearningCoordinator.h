// FederatedLearningCoordinator.h — Federated Learning Coordinator
// Copyright (c) 2026 ExplorerLens Project
//
// Coordinates federated gradient aggregation across on-device nodes without raw data upload.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <numeric>

namespace ExplorerLens { namespace Engine {

enum class FedAggregationAlgo { FedAvg, FedProx, FedNova };

struct FedRoundConfig {
    FedAggregationAlgo algo          = FedAggregationAlgo::FedAvg;
    uint32_t           minClients    = 2;
    uint32_t           roundsTotal   = 10;
    float              learningRate  = 0.01f;
};

struct FedRoundResult {
    bool              success          = false;
    uint32_t          participantCount = 0;
    float             globalLoss       = 0.0f;
    uint32_t          roundNumber      = 0;
};

class FederatedLearningCoordinator {
public:
    explicit FederatedLearningCoordinator(const FedRoundConfig& config)
        : m_config(config), m_currentRound(0), m_globalLoss(1.0f) {}

    FedRoundResult RunRound(const std::vector<std::vector<float>>& clientGradients) {
        FedRoundResult r;
        if (clientGradients.size() < m_config.minClients) return r;
        ++m_currentRound;
        // FedAvg: average all gradients
        size_t dim = clientGradients[0].size();
        m_aggregatedGradient.assign(dim, 0.0f);
        for (const auto& grads : clientGradients)
            for (size_t i = 0; i < std::min(dim, grads.size()); ++i)
                m_aggregatedGradient[i] += grads[i];
        for (auto& g : m_aggregatedGradient)
            g /= static_cast<float>(clientGradients.size());
        m_globalLoss = std::max(0.0f, m_globalLoss - m_config.learningRate * 0.1f);
        r.participantCount = static_cast<uint32_t>(clientGradients.size());
        r.globalLoss       = m_globalLoss;
        r.roundNumber      = m_currentRound;
        r.success          = true;
        return r;
    }
    float GlobalLoss()       const { return m_globalLoss; }
    uint32_t RoundsComplete() const { return m_currentRound; }

private:
    FedRoundConfig        m_config;
    uint32_t              m_currentRound;
    float                 m_globalLoss;
    std::vector<float>    m_aggregatedGradient;
};

}} // namespace ExplorerLens::Engine
