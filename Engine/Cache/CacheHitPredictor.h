// CacheHitPredictor.h — ML Cache Hit Prediction
// Copyright (c) 2026 ExplorerLens Project
//
// ML-based cache hit prediction. Analyzes access pattern history to predict
// which entries will be accessed next, enabling proactive preloading.
//
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <mutex>
#include <cmath>
#include <deque>

namespace ExplorerLens {
namespace Engine {

struct AccessRecord {
    std::string key;
    uint64_t timestamp = 0;
    uint32_t accessCount = 0;
    double recencyScore = 0.0;
    double frequencyScore = 0.0;
};

struct HitPrediction {
    std::string key;
    double probability = 0.0;
    double confidence = 0.0;
    bool shouldPreload = false;
};

struct PredictorConfig {
    uint32_t historyWindowSize = 1000;
    double preloadThreshold = 0.6;
    double decayFactor = 0.95;
    uint32_t minAccessCount = 2;
    uint32_t maxPredictions = 10;
};

struct PredictorStats {
    uint64_t totalPredictions = 0;
    uint64_t correctPredictions = 0;
    double accuracy = 0.0;
    double avgConfidence = 0.0;
    uint32_t trackedKeys = 0;
};

class CacheHitPredictor {
public:
    static CacheHitPredictor& Instance() {
        static CacheHitPredictor instance;
        return instance;
    }

    inline void Configure(const PredictorConfig& config) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_config = config;
    }

    inline void RecordAccess(const std::string& key, uint64_t timestamp) {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto& record = m_accessRecords[key];
        record.key = key;
        record.timestamp = timestamp;
        record.accessCount++;

        m_accessLog.push_back({ key, timestamp, 0, 0.0, 0.0 });
        if (m_accessLog.size() > m_config.historyWindowSize) {
            m_accessLog.pop_front();
        }

        if (!m_lastKey.empty() && m_lastKey != key) {
            m_sequentialPairs[m_lastKey][key]++;
        }
        m_lastKey = key;
    }

    inline std::vector<HitPrediction> PredictNextHits(uint64_t currentTimestamp) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<HitPrediction> predictions;

        for (const auto& [key, record] : m_accessRecords) {
            double recency = ComputeRecencyScore(record.timestamp, currentTimestamp);
            double frequency = ComputeFrequencyScore(record.accessCount);
            double sequential = ComputeSequentialScore(key);

            HitPrediction pred;
            pred.key = key;
            pred.probability = 0.4 * recency + 0.3 * frequency + 0.3 * sequential;
            pred.confidence = (std::min)(1.0, static_cast<double>(record.accessCount) / 10.0);
            pred.shouldPreload = pred.probability >= m_config.preloadThreshold;
            predictions.push_back(pred);
        }

        std::sort(predictions.begin(), predictions.end(),
            [](const HitPrediction& a, const HitPrediction& b) {
                return a.probability > b.probability;
            });

        if (predictions.size() > m_config.maxPredictions) {
            predictions.resize(m_config.maxPredictions);
        }
        return predictions;
    }

    inline void RecordOutcome(const std::string& key, bool wasHit) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.totalPredictions++;
        if (wasHit) m_stats.correctPredictions++;
        if (m_stats.totalPredictions > 0) {
            m_stats.accuracy = static_cast<double>(m_stats.correctPredictions) / m_stats.totalPredictions;
        }
    }

    inline PredictorStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto stats = m_stats;
        stats.trackedKeys = static_cast<uint32_t>(m_accessRecords.size());
        return stats;
    }

    inline void Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_accessRecords.clear();
        m_accessLog.clear();
        m_sequentialPairs.clear();
        m_lastKey.clear();
        m_stats = {};
    }

private:
    CacheHitPredictor() = default;

    inline double ComputeRecencyScore(uint64_t lastAccess, uint64_t now) const {
        if (now <= lastAccess) return 1.0;
        double ageSec = static_cast<double>(now - lastAccess) / 1000.0;
        return std::exp(-ageSec * 0.01);
    }

    inline double ComputeFrequencyScore(uint32_t accessCount) const {
        return 1.0 - 1.0 / (1.0 + std::log(1.0 + accessCount));
    }

    inline double ComputeSequentialScore(const std::string& key) const {
        if (m_lastKey.empty()) return 0.0;
        auto pairIt = m_sequentialPairs.find(m_lastKey);
        if (pairIt == m_sequentialPairs.end()) return 0.0;
        auto keyIt = pairIt->second.find(key);
        if (keyIt == pairIt->second.end()) return 0.0;

        uint64_t total = 0;
        for (const auto& [k, c] : pairIt->second) total += c;
        return total > 0 ? static_cast<double>(keyIt->second) / total : 0.0;
    }

    mutable std::mutex m_mutex;
    PredictorConfig m_config;
    std::unordered_map<std::string, AccessRecord> m_accessRecords;
    std::deque<AccessRecord> m_accessLog;
    std::unordered_map<std::string, std::unordered_map<std::string, uint64_t>> m_sequentialPairs;
    std::string m_lastKey;
    PredictorStats m_stats;
};

}
} // namespace ExplorerLens::Engine
