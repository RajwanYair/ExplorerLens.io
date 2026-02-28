// ============================================================================
// PredictiveCacheEngine.h — ML-Based Predictive Cache Prefetch
// ExplorerLens Engine v15.0.0
// Copyright (c) 2026 ExplorerLens Project
//
// Predicts which thumbnails will be needed based on user navigation patterns,
// directory browsing history, and file access frequency. Proactively populates
// cache during idle time, targeting sub-2ms cache hit latency for 95%+ of
// navigations.
// ============================================================================

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <mutex>
#include <atomic>
#include <functional>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Navigation event for pattern learning
// ============================================================================

struct NavigationEvent {
    std::wstring directoryPath;
    uint32_t     fileCount = 0;    // Files in directory
    uint32_t     imageCount = 0;    // Image files specifically
    uint64_t     timestamp = 0;    // Unix epoch ms
    uint32_t     dwellTimeMs = 0;    // How long user stayed
    bool         scrolledToEnd = false;// User saw all files
};

// ============================================================================
// Prediction result
// ============================================================================

struct CachePrediction {
    std::wstring directoryPath;
    float        confidence = 0.0f; // 0.0 to 1.0
    uint32_t     estimatedFiles = 0;
    uint32_t     priority = 0;    // Lower = higher priority

    bool operator>(const CachePrediction& other) const {
        return confidence > other.confidence;
    }
};

// ============================================================================
// Directory access profile
// ============================================================================

struct DirectoryProfile {
    std::wstring path;
    uint32_t     accessCount = 0;
    uint32_t     totalDwellMs = 0;
    uint64_t     lastAccessTime = 0;
    float        accessFrequency = 0.0f; // Accesses per hour
    std::vector<std::wstring> nextDirectories;  // Where user went after
    bool         isPinned = false;

    float GetRecencyScore(uint64_t nowMs) const {
        if (lastAccessTime == 0) return 0.0f;
        double ageHours = static_cast<double>(nowMs - lastAccessTime) / 3600000.0;
        return static_cast<float>(1.0 / (1.0 + ageHours));  // Exponential decay
    }
};

// ============================================================================
// Prediction model configuration
// ============================================================================

struct PredictionConfig {
    uint32_t maxHistorySize = 1000;  // Max navigation events to keep
    uint32_t maxProfiles = 500;   // Max directory profiles
    uint32_t maxPredictions = 10;    // Max predictions per query
    float    minConfidence = 0.1f;  // Minimum prediction confidence
    float    recencyWeight = 0.4f;  // Weight for recency in scoring
    float    frequencyWeight = 0.3f;  // Weight for access frequency
    float    sequenceWeight = 0.3f;  // Weight for sequential patterns
    uint32_t idlePrefetchDelayMs = 2000;  // Wait for idle before prefetching
    uint32_t maxPrefetchBatch = 50;    // Max files to prefetch at once
};

// ============================================================================
// Cache prediction statistics
// ============================================================================

struct PredictionStats {
    uint64_t totalPredictions = 0;
    uint64_t correctPredictions = 0;  // User actually visited
    uint64_t wastedPrefetches = 0;  // Prefetched but never visited
    uint64_t totalPrefetched = 0;
    uint64_t cacheHitsDueToPrefetch = 0;
    double   avgPredictionTimeUs = 0.0;

    double GetAccuracy() const {
        return (totalPredictions > 0)
            ? (static_cast<double>(correctPredictions) / totalPredictions * 100.0)
            : 0.0;
    }

    double GetWasteRate() const {
        return (totalPrefetched > 0)
            ? (static_cast<double>(wastedPrefetches) / totalPrefetched * 100.0)
            : 0.0;
    }
};

// ============================================================================
// PredictiveCacheEngine
// ============================================================================

class PredictiveCacheEngine {
public:
    PredictiveCacheEngine() = default;
    explicit PredictiveCacheEngine(const PredictionConfig& config)
        : m_config(config) {
    }

    // ========================================================================
    // Navigation tracking
    // ========================================================================

    /// Record a directory navigation event
    void RecordNavigation(const NavigationEvent& event) {
        std::lock_guard<std::mutex> lock(m_mutex);

        // Add to history
        m_history.push_back(event);
        if (m_history.size() > m_config.maxHistorySize) {
            m_history.pop_front();
        }

        // Update directory profile
        auto& profile = m_profiles[event.directoryPath];
        profile.path = event.directoryPath;
        profile.accessCount++;
        profile.totalDwellMs += event.dwellTimeMs;
        profile.lastAccessTime = event.timestamp;

        // Track sequential navigation patterns
        if (!m_history.empty() && m_history.size() >= 2) {
            auto prevIt = m_history.end();
            --prevIt; --prevIt;
            auto& prevProfile = m_profiles[prevIt->directoryPath];
            // Record "after visiting X, user went to Y"
            auto& nextDirs = prevProfile.nextDirectories;
            if (std::find(nextDirs.begin(), nextDirs.end(), event.directoryPath) == nextDirs.end()) {
                nextDirs.push_back(event.directoryPath);
            }
        }

        // Recompute frequencies
        UpdateFrequencies();
    }

    // ========================================================================
    // Prediction
    // ========================================================================

    /// Predict next directories the user is likely to visit
    std::vector<CachePrediction> PredictNext(const std::wstring& currentDir) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto startTime = std::chrono::steady_clock::now();

        std::vector<CachePrediction> predictions;
        uint64_t nowMs = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
            );

        // Score all known directories
        for (const auto& [path, profile] : m_profiles) {
            if (path == currentDir) continue;  // Skip current

            float score = ComputePredictionScore(profile, currentDir, nowMs);
            if (score >= m_config.minConfidence) {
                CachePrediction pred;
                pred.directoryPath = path;
                pred.confidence = score;
                pred.estimatedFiles = EstimateFileCount(profile);
                predictions.push_back(std::move(pred));
            }
        }

        // Sort by confidence (descending)
        std::sort(predictions.begin(), predictions.end(),
            [](const CachePrediction& a, const CachePrediction& b) {
                return a.confidence > b.confidence;
            });

        // Limit results
        if (predictions.size() > m_config.maxPredictions) {
            predictions.resize(m_config.maxPredictions);
        }

        // Assign priorities
        for (uint32_t i = 0; i < predictions.size(); i++) {
            predictions[i].priority = i;
        }

        auto elapsed = std::chrono::steady_clock::now() - startTime;
        double elapsedUs = std::chrono::duration<double, std::micro>(elapsed).count();
        m_stats.totalPredictions++;
        m_stats.avgPredictionTimeUs =
            (m_stats.avgPredictionTimeUs * (m_stats.totalPredictions - 1) + elapsedUs)
            / m_stats.totalPredictions;

        return predictions;
    }

    /// Validate a prediction (user actually visited the directory)
    void ValidatePrediction(const std::wstring& actualDir) {
        m_stats.correctPredictions++;
        m_stats.cacheHitsDueToPrefetch++;
    }

    // ========================================================================
    // Configuration
    // ========================================================================

    const PredictionConfig& GetConfig() const { return m_config; }
    void SetConfig(const PredictionConfig& config) { m_config = config; }

    // ========================================================================
    // Statistics
    // ========================================================================

    PredictionStats GetStats() const { return m_stats; }

    uint32_t GetProfileCount() const {
        return static_cast<uint32_t>(m_profiles.size());
    }

    uint32_t GetHistorySize() const {
        return static_cast<uint32_t>(m_history.size());
    }

    /// Pin a directory for high-priority prefetch
    void PinDirectory(const std::wstring& path) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_profiles[path].isPinned = true;
    }

    void UnpinDirectory(const std::wstring& path) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_profiles.find(path);
        if (it != m_profiles.end()) it->second.isPinned = false;
    }

    /// Clear all learned patterns
    void Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_history.clear();
        m_profiles.clear();
        m_stats = {};
    }

private:
    // ========================================================================
    // Scoring
    // ========================================================================

    float ComputePredictionScore(const DirectoryProfile& profile,
        const std::wstring& currentDir,
        uint64_t nowMs) const {
        float score = 0.0f;

        // Pinned directories always score high
        if (profile.isPinned) return 0.95f;

        // Recency score (exponential decay over hours)
        float recency = profile.GetRecencyScore(nowMs);
        score += recency * m_config.recencyWeight;

        // Frequency score (normalized)
        float maxFreq = 0.0f;
        for (const auto& [p, prof] : m_profiles) {
            if (prof.accessFrequency > maxFreq) maxFreq = prof.accessFrequency;
        }
        float freqNorm = (maxFreq > 0) ? (profile.accessFrequency / maxFreq) : 0.0f;
        score += freqNorm * m_config.frequencyWeight;

        // Sequential pattern score (did user go here after currentDir before?)
        auto currentIt = m_profiles.find(currentDir);
        if (currentIt != m_profiles.end()) {
            const auto& nextDirs = currentIt->second.nextDirectories;
            for (const auto& next : nextDirs) {
                if (next == profile.path) {
                    score += m_config.sequenceWeight;
                    break;
                }
            }
        }

        return (std::min)(score, 1.0f);
    }

    uint32_t EstimateFileCount(const DirectoryProfile& profile) const {
        // Simple estimate based on access history
        for (auto it = m_history.rbegin(); it != m_history.rend(); ++it) {
            if (it->directoryPath == profile.path) return it->fileCount;
        }
        return 50;  // Default estimate
    }

    void UpdateFrequencies() {
        if (m_history.size() < 2) return;

        uint64_t firstTime = m_history.front().timestamp;
        uint64_t lastTime = m_history.back().timestamp;
        double spanHours = (std::max)(1.0, static_cast<double>(lastTime - firstTime) / 3600000.0);

        for (auto& [path, profile] : m_profiles) {
            profile.accessFrequency =
                static_cast<float>(profile.accessCount / spanHours);
        }
    }

    PredictionConfig m_config;
    mutable std::mutex m_mutex;
    std::deque<NavigationEvent> m_history;
    std::unordered_map<std::wstring, DirectoryProfile> m_profiles;
    PredictionStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
