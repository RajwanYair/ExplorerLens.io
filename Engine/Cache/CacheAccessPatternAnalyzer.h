// CacheAccessPatternAnalyzer.h — Cache Access Pattern Intelligence
// Copyright (c) 2026 ExplorerLens Project
//
// Analyzes cache access patterns (temporal, spatial, frequency) to
// predict future accesses and optimize eviction/prefetch strategies.
//
#pragma once

#include <cstdint>
#include <string>
#include <deque>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

enum class CacheAccessPattern : uint8_t {
    Unknown,
    Sequential,
    Random,
    Temporal,
    FrequencyBased,
    Clustered
};

struct PatternAccessRecord {
    std::wstring key;
    uint64_t timestamp = 0;
    uint32_t hitCount = 0;
    double latencyMs = 0.0;
};

struct PatternAnalysis {
    CacheAccessPattern dominantPattern = CacheAccessPattern::Unknown;
    float sequentialScore = 0.0f;
    float temporalScore = 0.0f;
    float frequencyScore = 0.0f;
    uint64_t totalAccesses = 0;
    float hitRate = 0.0f;
};

class CacheAccessPatternAnalyzer {
public:
    explicit CacheAccessPatternAnalyzer(size_t windowSize = 1000)
        : m_windowSize(windowSize) {
    }

    void RecordAccess(const std::wstring& key, bool isHit) {
        m_accessWindow.push_back(PatternAccessRecord{ key, 0, 0, 0.0 });
        if (m_accessWindow.size() > m_windowSize)
            m_accessWindow.pop_front();
        m_keyFrequency[key]++;
        m_totalAccesses++;
        if (isHit) m_totalHits++;
    }

    PatternAnalysis Analyze() const {
        PatternAnalysis analysis;
        analysis.totalAccesses = m_totalAccesses;
        analysis.hitRate = m_totalAccesses > 0
            ? static_cast<float>(m_totalHits) / m_totalAccesses : 0.0f;

        // Determine dominant pattern from access window
        if (m_accessWindow.size() < 10) {
            analysis.dominantPattern = CacheAccessPattern::Unknown;
            return analysis;
        }

        // Simple sequential detection
        uint32_t sequentialRuns = 0;
        for (size_t i = 1; i < m_accessWindow.size(); i++) {
            if (m_accessWindow[i].key == m_accessWindow[i - 1].key)
                sequentialRuns++;
        }
        analysis.sequentialScore = static_cast<float>(sequentialRuns) / m_accessWindow.size();

        // Frequency-based scoring
        size_t uniqueKeys = m_keyFrequency.size();
        analysis.frequencyScore = uniqueKeys > 0
            ? 1.0f - (static_cast<float>(uniqueKeys) / m_totalAccesses) : 0.0f;

        if (analysis.sequentialScore > 0.5f)
            analysis.dominantPattern = CacheAccessPattern::Sequential;
        else if (analysis.frequencyScore > 0.7f)
            analysis.dominantPattern = CacheAccessPattern::FrequencyBased;
        else
            analysis.dominantPattern = CacheAccessPattern::Random;

        return analysis;
    }

    float GetHitRate() const {
        return m_totalAccesses > 0
            ? static_cast<float>(m_totalHits) / m_totalAccesses : 0.0f;
    }

    uint64_t GetTotalAccesses() const { return m_totalAccesses; }
    void Reset() {
        m_accessWindow.clear();
        m_keyFrequency.clear();
        m_totalAccesses = 0;
        m_totalHits = 0;
    }

private:
    std::deque<PatternAccessRecord> m_accessWindow;
    std::unordered_map<std::wstring, uint64_t> m_keyFrequency;
    size_t m_windowSize;
    uint64_t m_totalAccesses = 0;
    uint64_t m_totalHits = 0;
};

} // namespace Engine
} // namespace ExplorerLens
