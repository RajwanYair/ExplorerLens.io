#pragma once
// ============================================================================
// IntelligentPrefetchV2.h — Predictive file prefetching via access patterns
//
// Purpose:   Predictive file prefetching based on access pattern analysis
// Provides:  PrefetchStrategyV2, AccessPatternV2 enums, PrefetchRequest
//            struct, and IntelligentPrefetchV2 class
// Used by:   Pipeline I/O layer
// ============================================================================

#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// IntelligentPrefetchV2 — ML-based folder access prediction
// ============================================================================

enum class PrefetchStrategyV2 {
    None,
    Sequential,
    Predictive,
    Adaptive,
    Aggressive
};

inline const char* PrefetchStrategyV2Name(PrefetchStrategyV2 value)
{
    switch (value) {
        case PrefetchStrategyV2::None:
            return "None";
        case PrefetchStrategyV2::Sequential:
            return "Sequential";
        case PrefetchStrategyV2::Predictive:
            return "Predictive";
        case PrefetchStrategyV2::Adaptive:
            return "Adaptive";
        case PrefetchStrategyV2::Aggressive:
            return "Aggressive";
        default:
            return "Unknown";
    }
}

enum class AccessPatternV2 {
    Random,
    Sequential,
    Temporal,
    Spatial,
    Hybrid
};

inline const char* AccessPatternV2Name(AccessPatternV2 value)
{
    switch (value) {
        case AccessPatternV2::Random:
            return "Random";
        case AccessPatternV2::Sequential:
            return "Sequential";
        case AccessPatternV2::Temporal:
            return "Temporal";
        case AccessPatternV2::Spatial:
            return "Spatial";
        case AccessPatternV2::Hybrid:
            return "Hybrid";
        default:
            return "Unknown";
    }
}

struct PrefetchPrediction
{
    std::wstring folderPath;
    float confidence = 0.0f;
    uint32_t suggestedCount = 0;
    AccessPatternV2 pattern = AccessPatternV2::Random;
    PrefetchStrategyV2 strategy = PrefetchStrategyV2::None;

    bool IsViable(float threshold) const
    {
        return confidence >= threshold && suggestedCount > 0;
    }
};

struct FolderAccessEntry
{
    std::wstring folderPath;
    uint32_t accessCount = 0;
    uint64_t lastAccessMs = 0;
    uint64_t firstAccessMs = 0;
    AccessPatternV2 detectedPattern = AccessPatternV2::Random;
};

class IntelligentPrefetchV2
{
  public:
    static constexpr float CONFIDENCE_THRESHOLD = 0.7f;
    static constexpr uint32_t MAX_HISTORY_ENTRIES = 10000;
    static constexpr uint32_t MAX_PREDICTIONS = 32;
    static constexpr uint32_t MIN_ACCESS_FOR_PREDICT = 3;

    IntelligentPrefetchV2() = default;
    ~IntelligentPrefetchV2() = default;

    IntelligentPrefetchV2(const IntelligentPrefetchV2&) = delete;
    IntelligentPrefetchV2& operator=(const IntelligentPrefetchV2&) = delete;

    PrefetchPrediction Predict(const std::wstring& currentFolder)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        PrefetchPrediction prediction;
        prediction.folderPath = currentFolder;

        auto it = m_accessHistory.find(currentFolder);
        if (it == m_accessHistory.end() || it->second.accessCount < MIN_ACCESS_FOR_PREDICT) {
            prediction.confidence = 0.0f;
            prediction.strategy = PrefetchStrategyV2::None;
            return prediction;
        }

        const FolderAccessEntry& entry = it->second;
        prediction.pattern = entry.detectedPattern;

        // Confidence based on access frequency
        float frequencyScore = static_cast<float>(entry.accessCount) / 100.0f;
        frequencyScore = (frequencyScore > 1.0f) ? 1.0f : frequencyScore;

        // Recency bonus
        uint64_t now = GetCurrentTimeMs();
        uint64_t ageMs = now - entry.lastAccessMs;
        float recencyScore = (ageMs < 60000) ? 1.0f : (ageMs < 300000) ? 0.7f : 0.3f;

        prediction.confidence = frequencyScore * 0.6f + recencyScore * 0.4f;
        prediction.suggestedCount = (entry.accessCount > 20) ? 16 : 8;

        if (prediction.confidence >= CONFIDENCE_THRESHOLD) {
            prediction.strategy = (entry.detectedPattern == AccessPatternV2::Sequential)
                                      ? PrefetchStrategyV2::Sequential
                                      : PrefetchStrategyV2::Adaptive;
        } else {
            prediction.strategy = PrefetchStrategyV2::None;
        }

        m_totalPredictions++;
        if (prediction.confidence >= CONFIDENCE_THRESHOLD) {
            m_successfulPredictions++;
        }

        return prediction;
    }

    void RecordAccess(const std::wstring& folderPath)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint64_t now = GetCurrentTimeMs();

        auto& entry = m_accessHistory[folderPath];
        if (entry.accessCount == 0) {
            entry.folderPath = folderPath;
            entry.firstAccessMs = now;
        }
        entry.accessCount++;
        entry.lastAccessMs = now;

        // Simple pattern detection
        if (entry.accessCount > 5) {
            uint64_t avgInterval = (now - entry.firstAccessMs) / entry.accessCount;
            if (avgInterval < 2000) {
                entry.detectedPattern = AccessPatternV2::Sequential;
            } else if (avgInterval < 30000) {
                entry.detectedPattern = AccessPatternV2::Temporal;
            } else {
                entry.detectedPattern = AccessPatternV2::Random;
            }
        }

        m_totalAccesses++;
    }

    float GetAccuracyPercent() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_totalPredictions == 0)
            return 0.0f;
        return (static_cast<float>(m_successfulPredictions) / static_cast<float>(m_totalPredictions)) * 100.0f;
    }

    size_t GetHistorySize() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_accessHistory.size();
    }

    uint64_t GetTotalAccesses() const
    {
        return m_totalAccesses;
    }
    uint64_t GetTotalPredictions() const
    {
        return m_totalPredictions;
    }

    void ClearHistory()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_accessHistory.clear();
        m_totalAccesses = 0;
        m_totalPredictions = 0;
        m_successfulPredictions = 0;
    }

  private:
    uint64_t GetCurrentTimeMs() const
    {
        auto now = std::chrono::steady_clock::now();
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
    }

    mutable std::mutex m_mutex;
    std::unordered_map<std::wstring, FolderAccessEntry> m_accessHistory;
    uint64_t m_totalAccesses = 0;
    uint64_t m_totalPredictions = 0;
    uint64_t m_successfulPredictions = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
