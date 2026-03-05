// PredictiveCacheLoader.h — ML-Based Predictive Cache Loading
// Copyright (c) 2026 ExplorerLens Project
//
// Predicts which files the user is likely to access next based on historical
// access patterns, using a lightweight frequency/recency model.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <string>
#include <mutex>
#include <algorithm>
#include <unordered_map>
#include <cmath>

namespace ExplorerLens {
namespace Engine {

struct FileAccessFeatures {
    std::wstring filePath;
    uint32_t     accessCount = 0;
    uint64_t     lastAccessTimeMs = 0;
    uint64_t     avgIntervalMs = 0;
    double       recencyScore = 0.0;
    double       frequencyScore = 0.0;
    std::string  extension;
};

struct PredictedCacheEntry {
    std::wstring filePath;
    double       confidence = 0.0;  // 0.0 - 1.0
    uint32_t     rank = 0;

    bool operator<(const PredictedCacheEntry& other) const {
        return confidence > other.confidence; // Higher confidence = lower rank
    }
};

class PredictiveCacheLoader {
public:
    static PredictiveCacheLoader& Instance() {
        static PredictiveCacheLoader s;
        return s;
    }

    void RecordAccess(const std::wstring& filePath, const std::string& ext) {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint64_t now = GetTickCount64();
        auto& features = m_accessHistory[filePath];
        features.filePath = filePath;
        features.extension = ext;

        if (features.accessCount > 0) {
            uint64_t interval = now - features.lastAccessTimeMs;
            features.avgIntervalMs = (features.avgIntervalMs * features.accessCount + interval) /
                (features.accessCount + 1);
        }

        features.accessCount++;
        features.lastAccessTimeMs = now;
        m_totalAccesses++;
    }

    std::vector<PredictedCacheEntry> Predict(size_t topK = 10) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint64_t now = GetTickCount64();
        std::vector<PredictedCacheEntry> predictions;

        for (const auto& [path, features] : m_accessHistory) {
            PredictedCacheEntry pred;
            pred.filePath = path;

            // Recency: exponential decay over 60 seconds
            double recencyMs = static_cast<double>(now - features.lastAccessTimeMs);
            double recencyScore = std::exp(-recencyMs / 60000.0);

            // Frequency: normalized by total accesses
            double freqScore = m_totalAccesses > 0 ?
                static_cast<double>(features.accessCount) / m_totalAccesses : 0.0;

            // Combined score with weights
            pred.confidence = m_recencyWeight * recencyScore + m_frequencyWeight * freqScore;
            pred.confidence = (std::min)(pred.confidence, 1.0);
            pred.confidence = (std::max)(pred.confidence, 0.0);

            predictions.push_back(pred);
        }

        std::sort(predictions.begin(), predictions.end());

        if (predictions.size() > topK)
            predictions.resize(topK);

        for (uint32_t i = 0; i < predictions.size(); ++i)
            predictions[i].rank = i + 1;

        return predictions;
    }

    void Train() {
        std::lock_guard<std::mutex> lock(m_mutex);
        // Adaptive weight adjustment based on prediction accuracy
        if (m_hitCount + m_missCount > 100) {
            double hitRate = static_cast<double>(m_hitCount) / (m_hitCount + m_missCount);
            if (hitRate < 0.5) {
                m_recencyWeight = (std::min)(m_recencyWeight + 0.05, 0.9);
                m_frequencyWeight = 1.0 - m_recencyWeight;
            }
        }

        // Prune stale entries older than 5 minutes
        uint64_t now = GetTickCount64();
        for (auto it = m_accessHistory.begin(); it != m_accessHistory.end(); ) {
            if (now - it->second.lastAccessTimeMs > 300000) {
                it = m_accessHistory.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    void RecordPredictionResult(bool hit) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (hit) m_hitCount++;
        else m_missCount++;
    }

    double GetPredictionAccuracy() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint64_t total = m_hitCount + m_missCount;
        return total > 0 ? static_cast<double>(m_hitCount) / total : 0.0;
    }

    size_t GetHistorySize() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_accessHistory.size();
    }

    void Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_accessHistory.clear();
        m_totalAccesses = 0;
        m_hitCount = 0;
        m_missCount = 0;
        m_recencyWeight = 0.6;
        m_frequencyWeight = 0.4;
    }

    bool Validate() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_recencyWeight + m_frequencyWeight < 0.99 ||
            m_recencyWeight + m_frequencyWeight > 1.01) return false;
        for (const auto& [path, f] : m_accessHistory) {
            if (f.filePath.empty()) return false;
            if (f.accessCount == 0) return false;
        }
        return true;
    }

private:
    PredictiveCacheLoader() = default;
    ~PredictiveCacheLoader() = default;
    PredictiveCacheLoader(const PredictiveCacheLoader&) = delete;
    PredictiveCacheLoader& operator=(const PredictiveCacheLoader&) = delete;

    mutable std::mutex m_mutex;
    std::unordered_map<std::wstring, FileAccessFeatures> m_accessHistory;
    uint64_t m_totalAccesses = 0;
    uint64_t m_hitCount = 0;
    uint64_t m_missCount = 0;
    double   m_recencyWeight = 0.6;
    double   m_frequencyWeight = 0.4;
};

}
} // namespace ExplorerLens::Engine
