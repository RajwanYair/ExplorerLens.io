// FolderWatchPredictor.h — Predict Folder Access for Cache Warming
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks folder access patterns to predict which folders the user will browse
// next, enabling proactive cache warming for faster thumbnail display.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <chrono>

namespace ExplorerLens {
namespace Engine {

struct FolderAccessPattern {
    std::wstring   folderPath;
    uint64_t       lastAccessMs = 0;
    uint32_t       accessCount = 0;
    uint32_t       fileCount = 0;
    float          avgDwellTimeMs = 0.0f;
};

struct PredictedPath {
    std::wstring   path;
    float          confidence = 0.0f;   // 0.0 to 1.0
    uint32_t       expectedFiles = 0;
};

struct FolderPredictorConfig {
    uint32_t maxHistoryEntries = 1000;
    uint32_t maxPredictions = 5;
    float    decayFactor = 0.95f;   // score decay per access cycle
    uint64_t hotThresholdMs = 60000;   // 1 minute recency threshold
    uint32_t minAccessCount = 2;       // minimum accesses to predict
};

class FolderWatchPredictor {
public:
    static FolderWatchPredictor& Instance() { static FolderWatchPredictor s; return s; }

    void SetConfig(const FolderPredictorConfig& config) { m_config = config; }
    const FolderPredictorConfig& GetConfig() const { return m_config; }

    void RecordAccess(const std::wstring& folderPath, uint32_t fileCount = 0) {
        uint64_t now = GetCurrentTimeMs();
        auto it = m_patterns.find(folderPath);
        if (it != m_patterns.end()) {
            auto& p = it->second;
            if (p.accessCount > 0 && p.lastAccessMs > 0) {
                float dwell = static_cast<float>(now - p.lastAccessMs);
                p.avgDwellTimeMs = (p.avgDwellTimeMs * p.accessCount + dwell) / (p.accessCount + 1);
            }
            p.accessCount++;
            p.lastAccessMs = now;
            if (fileCount > 0) p.fileCount = fileCount;
        }
        else {
            if (m_patterns.size() >= m_config.maxHistoryEntries) {
                EvictOldest();
            }
            FolderAccessPattern p;
            p.folderPath = folderPath;
            p.lastAccessMs = now;
            p.accessCount = 1;
            p.fileCount = fileCount;
            m_patterns[folderPath] = p;
        }

        // Track transition for sequence prediction
        if (!m_lastFolder.empty() && m_lastFolder != folderPath) {
            m_transitions[m_lastFolder][folderPath]++;
        }
        m_lastFolder = folderPath;
    }

    std::vector<PredictedPath> PredictNext(const std::wstring& currentFolder) const {
        std::vector<PredictedPath> predictions;

        // Strategy 1: Markov transition probability
        auto transIt = m_transitions.find(currentFolder);
        if (transIt != m_transitions.end()) {
            uint32_t totalTransitions = 0;
            for (const auto& [path, count] : transIt->second)
                totalTransitions += count;

            for (const auto& [path, count] : transIt->second) {
                PredictedPath pred;
                pred.path = path;
                pred.confidence = static_cast<float>(count) / static_cast<float>(totalTransitions);
                auto patIt = m_patterns.find(path);
                if (patIt != m_patterns.end())
                    pred.expectedFiles = patIt->second.fileCount;
                predictions.push_back(pred);
            }
        }

        // Strategy 2: Hot folders by recency + frequency
        uint64_t now = GetCurrentTimeMs();
        for (const auto& [path, pattern] : m_patterns) {
            if (path == currentFolder) continue;
            if (pattern.accessCount < m_config.minAccessCount) continue;
            uint64_t age = now - pattern.lastAccessMs;
            if (age < m_config.hotThresholdMs) {
                bool alreadyPredicted = false;
                for (const auto& p : predictions) {
                    if (p.path == path) { alreadyPredicted = true; break; }
                }
                if (!alreadyPredicted) {
                    float recencyScore = 1.0f - static_cast<float>(age) / static_cast<float>(m_config.hotThresholdMs);
                    float freqScore = static_cast<float>(pattern.accessCount) / 100.0f;
                    freqScore = (std::min)(freqScore, 1.0f);
                    PredictedPath pred;
                    pred.path = path;
                    pred.confidence = recencyScore * 0.6f + freqScore * 0.4f;
                    pred.expectedFiles = pattern.fileCount;
                    predictions.push_back(pred);
                }
            }
        }

        // Sort by confidence and limit
        std::sort(predictions.begin(), predictions.end(),
            [](const PredictedPath& a, const PredictedPath& b) { return a.confidence > b.confidence; });
        if (predictions.size() > m_config.maxPredictions)
            predictions.resize(m_config.maxPredictions);

        return predictions;
    }

    std::vector<std::wstring> GetHotFolders(uint32_t maxCount = 10) const {
        uint64_t now = GetCurrentTimeMs();
        std::vector<std::pair<float, std::wstring>> scored;
        for (const auto& [path, pattern] : m_patterns) {
            uint64_t age = now - pattern.lastAccessMs;
            float score = static_cast<float>(pattern.accessCount) *
                std::pow(m_config.decayFactor, static_cast<float>(age) / 60000.0f);
            scored.push_back({ score, path });
        }
        std::sort(scored.begin(), scored.end(),
            [](const auto& a, const auto& b) { return a.first > b.first; });

        std::vector<std::wstring> result;
        for (uint32_t i = 0; i < maxCount && i < scored.size(); ++i)
            result.push_back(scored[i].second);
        return result;
    }

    size_t PatternCount() const { return m_patterns.size(); }

    void Clear() {
        m_patterns.clear();
        m_transitions.clear();
        m_lastFolder.clear();
    }

    bool Validate() const {
        if (m_config.maxHistoryEntries == 0) return false;
        if (m_config.maxPredictions == 0) return false;
        if (m_config.decayFactor <= 0.0f || m_config.decayFactor > 1.0f) return false;
        if (m_patterns.size() > m_config.maxHistoryEntries + 1) return false;
        return true;
    }

private:
    FolderWatchPredictor() = default;
    ~FolderWatchPredictor() = default;
    FolderWatchPredictor(const FolderWatchPredictor&) = delete;
    FolderWatchPredictor& operator=(const FolderWatchPredictor&) = delete;

    static uint64_t GetCurrentTimeMs() {
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
    }

    void EvictOldest() {
        if (m_patterns.empty()) return;
        auto oldest = m_patterns.begin();
        for (auto it = m_patterns.begin(); it != m_patterns.end(); ++it) {
            if (it->second.lastAccessMs < oldest->second.lastAccessMs)
                oldest = it;
        }
        m_patterns.erase(oldest);
    }

    FolderPredictorConfig m_config{};
    std::unordered_map<std::wstring, FolderAccessPattern> m_patterns;
    std::unordered_map<std::wstring, std::unordered_map<std::wstring, uint32_t>> m_transitions;
    std::wstring m_lastFolder;
};

} // namespace Engine
} // namespace ExplorerLens
