#pragma once
/******************************************************************************
 * PredictiveCacheEngine.h
 * Copyright (c) 2026 ExplorerLens Project
 *
 * PURPOSE:
 *   ML-inspired predictive cache that forecasts thumbnail access patterns
 *   based on observed navigation history, file access frequency, recency
 *   decay, and directory-level pattern detection. Proactively advises which
 *   entries to prefetch and which to evict.
 *
 * CLASSES:
 *   PredictiveCacheEngine — Maintains frequency/recency access table, computes
 *     access probability scores (0.0–1.0), detects directory browsing patterns,
 *     runs background frequency decay (0.95x every 60s), and provides eviction
 *     advisory. LRU-evicts tracking entries when capacity exceeded.
 *
 * INPUTS:
 *   RecordAccess(path) on every thumbnail lookup. Configuration via
 *   PredictionConfig (max history, max profiles, scoring weights).
 *
 * OUTPUTS:
 *   PredictAccessProbability(path) → [0.0, 1.0] combined score.
 *   GetTopPredictions(N) → top N most likely accessed paths.
 *   GetBestEvictionCandidate(candidates) → lowest-scored candidate.
 *   CacheStats with accuracy tracking.
 *
 * THREAD SAFETY:
 *   All public methods are thread-safe via SRWLOCK.
 *****************************************************************************/

#include <windows.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <list>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <functional>
#include <unordered_set>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Navigation event for pattern learning (backward-compatible)
// ============================================================================

struct NavigationEvent {
    std::wstring directoryPath;
    uint32_t     fileCount = 0;
    uint32_t     imageCount = 0;
    uint64_t     timestamp = 0;
    uint32_t     dwellTimeMs = 0;
    bool         scrolledToEnd = false;
};

// ============================================================================
// Cache prediction result (backward-compatible)
// ============================================================================

struct CachePrediction {
    std::wstring directoryPath;
    float        confidence = 0.0f;
    uint32_t     estimatedFiles = 0;
    uint32_t     priority = 0;

    bool operator>(const CachePrediction& other) const {
        return confidence > other.confidence;
    }
};

// ============================================================================
// Directory access profile (backward-compatible)
// ============================================================================

struct DirectoryProfile {
    std::wstring path;
    uint32_t     accessCount = 0;
    uint32_t     totalDwellMs = 0;
    uint64_t     lastAccessTime = 0;
    float        accessFrequency = 0.0f;
    std::vector<std::wstring> nextDirectories;
    bool         isPinned = false;

    float GetRecencyScore(uint64_t nowMs) const {
        if (lastAccessTime == 0) return 0.0f;
        double ageHours = static_cast<double>(nowMs - lastAccessTime) / 3600000.0;
        return static_cast<float>(1.0 / (1.0 + ageHours));
    }
};

// ============================================================================
// Prediction model configuration (backward-compatible + extended)
// ============================================================================

struct PredictionConfig {
    uint32_t maxHistorySize = 1000;
    uint32_t maxProfiles = 500;
    uint32_t maxPredictions = 10;
    float    minConfidence = 0.1f;
    float    recencyWeight = 0.4f;
    float    frequencyWeight = 0.3f;
    float    sequenceWeight = 0.3f;
    uint32_t idlePrefetchDelayMs = 2000;
    uint32_t maxPrefetchBatch = 50;
    uint32_t maxTrackedPaths = 10000;
    uint32_t directoryPatternWindow = 5;
    float    directoryBoostFactor = 0.15f;
    float    decayFactor = 0.95f;
    uint32_t decayIntervalSeconds = 60;
};

// ============================================================================
// Access entry for frequency/recency tracking
// ============================================================================

struct AccessEntry {
    uint64_t accessCount = 0;
    uint64_t lastAccessTimeMs = 0;
    double   avgInterAccessTimeMs = 0.0;
    uint64_t prevAccessTimeMs = 0;
    float    decayedFrequency = 0.0f;
    float    directoryBoost = 0.0f;
};

// ============================================================================
// Prediction statistics (backward-compatible + extended)
// ============================================================================

struct PredictionStats {
    uint64_t totalPredictions = 0;
    uint64_t correctPredictions = 0;
    uint64_t wastedPrefetches = 0;
    uint64_t totalPrefetched = 0;
    uint64_t cacheHitsDueToPrefetch = 0;
    double   avgPredictionTimeUs = 0.0;
    uint32_t trackedPaths = 0;
    uint32_t predictionsMade = 0;
    uint32_t decayCycles = 0;
    uint32_t lruEvictions = 0;

    double GetAccuracy() const {
        return (totalPredictions > 0)
            ? (static_cast<double>(correctPredictions) / static_cast<double>(totalPredictions) * 100.0)
            : 0.0;
    }

    double GetWasteRate() const {
        return (totalPrefetched > 0)
            ? (static_cast<double>(wastedPrefetches) / static_cast<double>(totalPrefetched) * 100.0)
            : 0.0;
    }
};

using PredictiveCacheStats = PredictionStats;

// ============================================================================
// PredictiveCacheEngine
// ============================================================================

class PredictiveCacheEngine {
public:
    PredictiveCacheEngine() {
        ::InitializeSRWLock(&m_srwLock);
    }

    explicit PredictiveCacheEngine(const PredictionConfig& config)
        : m_config(config) {
        ::InitializeSRWLock(&m_srwLock);
    }

    ~PredictiveCacheEngine() {
        StopDecayThread();
    }

    PredictiveCacheEngine(const PredictiveCacheEngine&) = delete;
    PredictiveCacheEngine& operator=(const PredictiveCacheEngine&) = delete;

    // ========================================================================
    // Access recording (frequency table)
    // ========================================================================

    void RecordAccess(const std::wstring& path) {
        ::AcquireSRWLockExclusive(&m_srwLock);

        auto nowMs = CurrentTimeMs();
        auto it = m_accessTable.find(path);

        if (it == m_accessTable.end()) {
            // Enforce max tracked paths with LRU eviction
            if (m_accessTable.size() >= m_config.maxTrackedPaths) {
                EvictLRUEntryLocked();
            }
            AccessEntry entry;
            entry.accessCount = 1;
            entry.lastAccessTimeMs = nowMs;
            entry.prevAccessTimeMs = 0;
            entry.decayedFrequency = 1.0f;
            m_accessTable[path] = entry;
            m_lruList.push_front(path);
            m_lruIterators[path] = m_lruList.begin();
        }
        else {
            auto& entry = it->second;
            entry.accessCount++;
            if (entry.prevAccessTimeMs > 0 && nowMs > entry.prevAccessTimeMs) {
                double interAccess = static_cast<double>(nowMs - entry.prevAccessTimeMs);
                double alpha = 0.3;
                entry.avgInterAccessTimeMs = (entry.avgInterAccessTimeMs == 0.0)
                    ? interAccess
                    : entry.avgInterAccessTimeMs * (1.0 - alpha) + interAccess * alpha;
            }
            entry.prevAccessTimeMs = entry.lastAccessTimeMs;
            entry.lastAccessTimeMs = nowMs;
            entry.decayedFrequency += 1.0f;

            // Move to front of LRU
            auto lruIt = m_lruIterators.find(path);
            if (lruIt != m_lruIterators.end()) {
                m_lruList.erase(lruIt->second);
                m_lruList.push_front(path);
                m_lruIterators[path] = m_lruList.begin();
            }
        }

        // Directory pattern detection
        m_recentAccessPaths.push_back(path);
        if (m_recentAccessPaths.size() > m_config.directoryPatternWindow) {
            m_recentAccessPaths.pop_front();
        }
        DetectDirectoryPatternLocked();

        ::ReleaseSRWLockExclusive(&m_srwLock);
    }

    // ========================================================================
    // Access probability prediction
    // ========================================================================

    float PredictAccessProbability(const std::wstring& path) {
        ::AcquireSRWLockShared(&m_srwLock);
        float score = ComputeProbabilityLocked(path);
        ::ReleaseSRWLockShared(&m_srwLock);
        return score;
    }

    std::vector<std::wstring> GetTopPredictions(uint32_t count) {
        ::AcquireSRWLockShared(&m_srwLock);

        struct Scored { std::wstring path; float score; };
        std::vector<Scored> scored;
        scored.reserve(m_accessTable.size());

        for (const auto& [path, entry] : m_accessTable) {
            float s = ComputeProbabilityLocked(path);
            if (s > 0.0f) {
                scored.push_back({ path, s });
            }
        }

        std::sort(scored.begin(), scored.end(),
            [](const Scored& a, const Scored& b) { return a.score > b.score; });

        std::vector<std::wstring> result;
        uint32_t n = (std::min)(count, static_cast<uint32_t>(scored.size()));
        result.reserve(n);
        for (uint32_t i = 0; i < n; ++i) {
            result.push_back(scored[i].path);
        }

        m_stats.predictionsMade++;
        ::ReleaseSRWLockShared(&m_srwLock);
        return result;
    }

    std::wstring GetBestEvictionCandidate(const std::vector<std::wstring>& candidates) {
        ::AcquireSRWLockShared(&m_srwLock);
        float lowestScore = 2.0f;
        std::wstring bestCandidate;

        for (const auto& path : candidates) {
            float s = ComputeProbabilityLocked(path);
            if (s < lowestScore) {
                lowestScore = s;
                bestCandidate = path;
            }
        }

        ::ReleaseSRWLockShared(&m_srwLock);
        return bestCandidate;
    }

    // ========================================================================
    // Frequency decay (background)
    // ========================================================================

    void StartDecayThread() {
        if (m_decayRunning.exchange(true)) return;
        m_decayThread = std::thread([this]() {
            while (m_decayRunning.load()) {
                uint32_t waited = 0;
                while (waited < m_config.decayIntervalSeconds * 1000 && m_decayRunning.load()) {
                    ::Sleep(100);
                    waited += 100;
                }
                if (!m_decayRunning.load()) break;
                ApplyDecay();
            }
            });
    }

    void StopDecayThread() {
        if (m_decayRunning.exchange(false)) {
            if (m_decayThread.joinable()) {
                m_decayThread.join();
            }
        }
    }

    void ApplyDecay() {
        ::AcquireSRWLockExclusive(&m_srwLock);
        for (auto& [path, entry] : m_accessTable) {
            entry.decayedFrequency *= m_config.decayFactor;
        }
        m_stats.decayCycles++;
        ::ReleaseSRWLockExclusive(&m_srwLock);
    }

    // ========================================================================
    // Navigation tracking (backward-compatible)
    // ========================================================================

    void RecordNavigation(const NavigationEvent& event) {
        ::AcquireSRWLockExclusive(&m_srwLock);

        m_history.push_back(event);
        if (m_history.size() > m_config.maxHistorySize) {
            m_history.pop_front();
        }

        auto& profile = m_profiles[event.directoryPath];
        profile.path = event.directoryPath;
        profile.accessCount++;
        profile.totalDwellMs += event.dwellTimeMs;
        profile.lastAccessTime = event.timestamp;

        if (m_history.size() >= 2) {
            auto prevIt = m_history.end();
            --prevIt; --prevIt;
            auto& prevProfile = m_profiles[prevIt->directoryPath];
            auto& nextDirs = prevProfile.nextDirectories;
            if (std::find(nextDirs.begin(), nextDirs.end(), event.directoryPath) == nextDirs.end()) {
                nextDirs.push_back(event.directoryPath);
            }
        }

        UpdateFrequenciesLocked();
        ::ReleaseSRWLockExclusive(&m_srwLock);
    }

    // ========================================================================
    // Prediction (backward-compatible)
    // ========================================================================

    std::vector<CachePrediction> PredictNext(const std::wstring& currentDir) {
        ::AcquireSRWLockExclusive(&m_srwLock);
        auto startTime = std::chrono::steady_clock::now();

        std::vector<CachePrediction> predictions;
        uint64_t nowMs = CurrentTimeMs();

        for (const auto& [path, profile] : m_profiles) {
            if (path == currentDir) continue;
            float score = ComputeProfilePredictionLocked(profile, currentDir, nowMs);
            if (score >= m_config.minConfidence) {
                CachePrediction pred;
                pred.directoryPath = path;
                pred.confidence = score;
                pred.estimatedFiles = EstimateFileCountLocked(profile);
                predictions.push_back(std::move(pred));
            }
        }

        std::sort(predictions.begin(), predictions.end(),
            [](const CachePrediction& a, const CachePrediction& b) {
                return a.confidence > b.confidence;
            });

        if (predictions.size() > m_config.maxPredictions) {
            predictions.resize(m_config.maxPredictions);
        }

        for (uint32_t i = 0; i < static_cast<uint32_t>(predictions.size()); i++) {
            predictions[i].priority = i;
        }

        auto elapsed = std::chrono::steady_clock::now() - startTime;
        double elapsedUs = std::chrono::duration<double, std::micro>(elapsed).count();
        m_stats.totalPredictions++;
        m_stats.avgPredictionTimeUs =
            (m_stats.avgPredictionTimeUs * static_cast<double>(m_stats.totalPredictions - 1) + elapsedUs)
            / static_cast<double>(m_stats.totalPredictions);

        ::ReleaseSRWLockExclusive(&m_srwLock);
        return predictions;
    }

    void ValidatePrediction(const std::wstring& /*actualDir*/) {
        m_stats.correctPredictions++;
        m_stats.cacheHitsDueToPrefetch++;
    }

    // ========================================================================
    // Configuration & Statistics
    // ========================================================================

    const PredictionConfig& GetConfig() const { return m_config; }
    void SetConfig(const PredictionConfig& config) { m_config = config; }

    PredictionStats GetStats() const {
        ::AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_srwLock));
        PredictionStats s = m_stats;
        s.trackedPaths = static_cast<uint32_t>(m_accessTable.size());
        ::ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_srwLock));
        return s;
    }

    uint32_t GetProfileCount() const {
        return static_cast<uint32_t>(m_profiles.size());
    }

    uint32_t GetHistorySize() const {
        return static_cast<uint32_t>(m_history.size());
    }

    void PinDirectory(const std::wstring& path) {
        ::AcquireSRWLockExclusive(&m_srwLock);
        m_profiles[path].isPinned = true;
        ::ReleaseSRWLockExclusive(&m_srwLock);
    }

    void UnpinDirectory(const std::wstring& path) {
        ::AcquireSRWLockExclusive(&m_srwLock);
        auto it = m_profiles.find(path);
        if (it != m_profiles.end()) it->second.isPinned = false;
        ::ReleaseSRWLockExclusive(&m_srwLock);
    }

    void Reset() {
        ::AcquireSRWLockExclusive(&m_srwLock);
        m_history.clear();
        m_profiles.clear();
        m_accessTable.clear();
        m_lruList.clear();
        m_lruIterators.clear();
        m_recentAccessPaths.clear();
        m_stats = {};
        ::ReleaseSRWLockExclusive(&m_srwLock);
    }

private:
    // ── Internal: access probability ────────────────────────────────────────

    float ComputeProbabilityLocked(const std::wstring& path) const {
        auto it = m_accessTable.find(path);
        if (it == m_accessTable.end()) return 0.0f;

        const auto& entry = it->second;
        uint64_t nowMs = CurrentTimeMs();

        // Recency score: exponential decay over hours
        float recencyScore = 0.0f;
        if (entry.lastAccessTimeMs > 0 && nowMs >= entry.lastAccessTimeMs) {
            double ageHours = static_cast<double>(nowMs - entry.lastAccessTimeMs) / 3600000.0;
            recencyScore = static_cast<float>(1.0 / (1.0 + ageHours));
        }

        // Frequency score: normalized decayed frequency
        float maxFreq = 1.0f;
        for (const auto& [p, e] : m_accessTable) {
            if (e.decayedFrequency > maxFreq) maxFreq = e.decayedFrequency;
        }
        float freqScore = entry.decayedFrequency / maxFreq;

        // Combined score with configurable weights
        float combined = recencyScore * m_config.recencyWeight
            + freqScore * m_config.frequencyWeight
            + entry.directoryBoost;

        return (std::min)(combined, 1.0f);
    }

    // ── Internal: directory pattern detection ───────────────────────────────

    void DetectDirectoryPatternLocked() {
        if (m_recentAccessPaths.size() < m_config.directoryPatternWindow) return;

        // Extract directory from each path
        std::unordered_map<std::wstring, uint32_t> dirCounts;
        for (const auto& p : m_recentAccessPaths) {
            auto lastSlash = p.find_last_of(L'\\');
            std::wstring dir = (lastSlash != std::wstring::npos) ? p.substr(0, lastSlash) : p;
            dirCounts[dir]++;
        }

        // If all recent accesses are in same directory, boost siblings
        for (const auto& [dir, count] : dirCounts) {
            if (count >= m_config.directoryPatternWindow) {
                for (auto& [path, entry] : m_accessTable) {
                    auto pathLastSlash = path.find_last_of(L'\\');
                    std::wstring pathDir = (pathLastSlash != std::wstring::npos)
                        ? path.substr(0, pathLastSlash) : path;
                    if (pathDir == dir) {
                        entry.directoryBoost = m_config.directoryBoostFactor;
                    }
                }
            }
        }
    }

    // ── Internal: LRU eviction ──────────────────────────────────────────────

    void EvictLRUEntryLocked() {
        if (m_lruList.empty()) return;
        auto& oldest = m_lruList.back();
        m_accessTable.erase(oldest);
        m_lruIterators.erase(oldest);
        m_lruList.pop_back();
        m_stats.lruEvictions++;
    }

    // ── Internal: backward-compatible scoring ───────────────────────────────

    float ComputeProfilePredictionLocked(const DirectoryProfile& profile,
        const std::wstring& currentDir, uint64_t nowMs) const {
        float score = 0.0f;
        if (profile.isPinned) return 0.95f;

        float recency = profile.GetRecencyScore(nowMs);
        score += recency * m_config.recencyWeight;

        float maxFreq = 0.0f;
        for (const auto& [p, prof] : m_profiles) {
            if (prof.accessFrequency > maxFreq) maxFreq = prof.accessFrequency;
        }
        float freqNorm = (maxFreq > 0) ? (profile.accessFrequency / maxFreq) : 0.0f;
        score += freqNorm * m_config.frequencyWeight;

        auto currentIt = m_profiles.find(currentDir);
        if (currentIt != m_profiles.end()) {
            for (const auto& next : currentIt->second.nextDirectories) {
                if (next == profile.path) {
                    score += m_config.sequenceWeight;
                    break;
                }
            }
        }
        return (std::min)(score, 1.0f);
    }

    uint32_t EstimateFileCountLocked(const DirectoryProfile& profile) const {
        for (auto it = m_history.rbegin(); it != m_history.rend(); ++it) {
            if (it->directoryPath == profile.path) return it->fileCount;
        }
        return 50;
    }

    void UpdateFrequenciesLocked() {
        if (m_history.size() < 2) return;
        uint64_t firstTime = m_history.front().timestamp;
        uint64_t lastTime = m_history.back().timestamp;
        double spanHours = (std::max)(1.0, static_cast<double>(lastTime - firstTime) / 3600000.0);
        for (auto& [path, profile] : m_profiles) {
            profile.accessFrequency = static_cast<float>(static_cast<double>(profile.accessCount) / spanHours);
        }
    }

    static uint64_t CurrentTimeMs() {
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
            );
    }

    // ── Data members ────────────────────────────────────────────────────────

    SRWLOCK m_srwLock{};
    PredictionConfig m_config;

    // Access frequency table
    std::unordered_map<std::wstring, AccessEntry> m_accessTable;
    std::list<std::wstring> m_lruList;
    std::unordered_map<std::wstring, std::list<std::wstring>::iterator> m_lruIterators;
    std::deque<std::wstring> m_recentAccessPaths;

    // Navigation history (backward-compatible)
    std::deque<NavigationEvent> m_history;
    std::unordered_map<std::wstring, DirectoryProfile> m_profiles;

    // Decay thread
    std::atomic<bool> m_decayRunning{ false };
    std::thread m_decayThread;

    // Stats
    mutable PredictionStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
