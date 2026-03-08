// CachePredictiveLoader.h — ML-inspired predictive thumbnail prefetch
// Copyright (c) 2026 ExplorerLens Project
//
// Uses access frequency and recency patterns to predict which thumbnails
// will be requested next and pre-loads them into cache.
//
#pragma once
#include <string>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct CachePredictiveLoaderConfig {
    bool enabled = true;
    uint32_t maxTrackedItems = 1024;
    uint32_t predictionWindow = 16;
    std::string label = "CachePredictiveLoader";
};

class CachePredictiveLoader {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    CachePredictiveLoaderConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    void RecordAccess(const std::string& key) {
        m_accessHistory.push_back(key);
        m_accessFrequency[key]++;
        while (m_accessHistory.size() > m_config.maxTrackedItems)
            m_accessHistory.erase(m_accessHistory.begin());
    }

    std::vector<std::string> PredictNext(uint32_t count = 4) const {
        std::vector<std::pair<std::string, uint32_t>> scored(
            m_accessFrequency.begin(), m_accessFrequency.end());
        std::sort(scored.begin(), scored.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });
        std::vector<std::string> result;
        for (uint32_t i = 0; i < count && i < scored.size(); ++i)
            result.push_back(scored[i].first);
        return result;
    }

    uint32_t GetTrackedCount() const { return static_cast<uint32_t>(m_accessFrequency.size()); }

private:
    bool m_initialized = false;
    CachePredictiveLoaderConfig m_config;
    std::vector<std::string> m_accessHistory;
    std::unordered_map<std::string, uint32_t> m_accessFrequency;
};

}
} // namespace ExplorerLens::Engine
