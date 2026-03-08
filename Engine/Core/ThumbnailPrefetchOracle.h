// ThumbnailPrefetchOracle.h — Predictive thumbnail prefetch engine
// Copyright (c) 2026 ExplorerLens Project
//
// Analyzes Explorer navigation patterns (back/forward, folder drill-down)
// to predict which directories will be visited next and prefetch thumbnails.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>
#include <deque>

namespace ExplorerLens {
namespace Engine {

struct ThumbnailPrefetchOracleConfig {
    bool enabled = true;
    uint32_t historyDepth = 32;
    uint32_t prefetchAhead = 2;
    std::string label = "ThumbnailPrefetchOracle";
};

class ThumbnailPrefetchOracle {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    ThumbnailPrefetchOracleConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    void RecordNavigation(const std::string& dirPath) {
        m_history.push_back(dirPath);
        while (m_history.size() > m_config.historyDepth)
            m_history.pop_front();
    }

    std::vector<std::string> PredictNext() const {
        std::vector<std::string> predictions;
        if (m_history.size() < 2) return predictions;
        // Simple sibling-directory prediction
        const auto& last = m_history.back();
        auto sep = last.find_last_of("/\\");
        if (sep != std::string::npos) {
            predictions.push_back(last.substr(0, sep));
        }
        return predictions;
    }

    uint32_t GetHistorySize() const { return static_cast<uint32_t>(m_history.size()); }

private:
    bool m_initialized = false;
    ThumbnailPrefetchOracleConfig m_config;
    std::deque<std::string> m_history;
};

}
} // namespace ExplorerLens::Engine
