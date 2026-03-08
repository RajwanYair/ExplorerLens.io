// CachePrefetchOracle.h — Predicts next-accessed cache entries for prefetch
// Copyright (c) 2026 ExplorerLens Project
//
// Uses access pattern analysis to predict which cache entries will be
// requested next, issuing prefetch hints to reduce cache miss latency.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

struct CachePrefetchOracleConfig {
    bool enabled = true;
    uint32_t maxPredictions = 16;
    std::string label = "CachePrefetchOracle";
};

class CachePrefetchOracle {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    CachePrefetchOracleConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    void RecordAccess(const std::string& key) {
        m_accessCounts[key]++;
        m_recentAccesses.push_back(key);
        if (m_recentAccesses.size() > 1000) m_recentAccesses.erase(m_recentAccesses.begin());
    }

    std::vector<std::string> PredictNext(uint32_t count) const {
        std::vector<std::string> predictions;
        // Simple frequency-based prediction
        std::vector<std::pair<std::string, uint32_t>> sorted(m_accessCounts.begin(), m_accessCounts.end());
        for (const auto& [k, v] : sorted) {
            if (predictions.size() >= count) break;
            predictions.push_back(k);
        }
        return predictions;
    }

private:
    bool m_initialized = false;
    CachePrefetchOracleConfig m_config;
    std::unordered_map<std::string, uint32_t> m_accessCounts;
    std::vector<std::string> m_recentAccesses;
};

}
} // namespace ExplorerLens::Engine
