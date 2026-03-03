// IntelligentCachePruner.h — LRU/LFU Hybrid Cache Eviction Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Intelligent cache pruning using a hybrid LRU/LFU scoring model that
// combines access frequency, recency, and entry size to compute eviction
// priorities. Identifies optimal eviction candidates to fit within a
// target memory budget.
//
#pragma once

#include <cstdint>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

struct CacheAccessRecord {
    std::string key;
    uint32_t    accessCount = 0;
    uint64_t    lastAccessMs = 0;   // Timestamp in milliseconds
    uint64_t    firstAccessMs = 0;
    uint64_t    sizeBytes = 0;
    double      evictionScore = 0.0; // Higher = more likely to evict
};

struct EvictionCandidate {
    std::string key;
    double      score = 0.0;
    uint64_t    sizeBytes = 0;
};

class IntelligentCachePruner {
public:
    IntelligentCachePruner() = default;

    // ---------------------------------------------------------------
    // Weights for the scoring model (tunable)
    // ---------------------------------------------------------------
    void SetRecencyWeight(double w) noexcept { m_recencyWeight = w; }
    void SetFrequencyWeight(double w) noexcept { m_frequencyWeight = w; }
    void SetSizeWeight(double w) noexcept { m_sizeWeight = w; }

    double GetRecencyWeight() const noexcept { return m_recencyWeight; }
    double GetFrequencyWeight() const noexcept { return m_frequencyWeight; }
    double GetSizeWeight() const noexcept { return m_sizeWeight; }

    // ---------------------------------------------------------------
    // Record an access event for a cache key
    // If the key already exists, update its counters; otherwise, create it.
    // ---------------------------------------------------------------
    void AddAccessRecord(const std::string& key, uint64_t timestampMs,
        uint64_t sizeBytes) {
        auto it = m_entries.find(key);
        if (it != m_entries.end()) {
            it->second.accessCount++;
            if (timestampMs > it->second.lastAccessMs) {
                it->second.lastAccessMs = timestampMs;
            }
            // Size may change on re-cache
            m_totalSize -= it->second.sizeBytes;
            it->second.sizeBytes = sizeBytes;
            m_totalSize += sizeBytes;
        }
        else {
            CacheAccessRecord rec;
            rec.key = key;
            rec.accessCount = 1;
            rec.lastAccessMs = timestampMs;
            rec.firstAccessMs = timestampMs;
            rec.sizeBytes = sizeBytes;
            m_totalSize += sizeBytes;
            m_entries[key] = std::move(rec);
        }
    }

    // ---------------------------------------------------------------
    // Compute eviction score for given parameters
    // Higher score = better candidate for eviction
    //
    // Score = recencyWeight * (nowMs - lastAccessMs) / normMs
    //       - frequencyWeight * log2(1 + accessCount)
    //       + sizeWeight * (sizeBytes / normBytes)
    //
    // Stale, infrequent, large entries score highest.
    // ---------------------------------------------------------------
    double ComputeEvictionScore(uint32_t accessCount, uint64_t lastAccessMs,
        uint64_t sizeBytes) const noexcept {
        return ComputeEvictionScoreAt(accessCount, lastAccessMs, sizeBytes,
            m_nowMs);
    }

    double ComputeEvictionScoreAt(uint32_t accessCount, uint64_t lastAccessMs,
        uint64_t sizeBytes,
        uint64_t nowMs) const noexcept {
        // Recency: older = higher penalty
        const double ageMs = (nowMs > lastAccessMs)
            ? static_cast<double>(nowMs - lastAccessMs)
            : 0.0;
        constexpr double kNormMs = 3600000.0; // 1 hour normalization
        const double recencyTerm = m_recencyWeight * (ageMs / kNormMs);

        // Frequency: more accesses = less evictable
        const double freqTerm = m_frequencyWeight *
            std::log2(1.0 + static_cast<double>(accessCount));

        // Size: larger entries have higher eviction value (free more memory)
        constexpr double kNormBytes = 1048576.0; // 1 MB normalization
        const double sizeTerm = m_sizeWeight *
            (static_cast<double>(sizeBytes) / kNormBytes);

        return recencyTerm - freqTerm + sizeTerm;
    }

    // ---------------------------------------------------------------
    // Set the "now" reference time for scoring
    // ---------------------------------------------------------------
    void SetCurrentTime(uint64_t nowMs) noexcept { m_nowMs = nowMs; }
    uint64_t GetCurrentTime() const noexcept { return m_nowMs; }

    // ---------------------------------------------------------------
    // Get sorted eviction candidates that, when removed, bring total
    // cached size down to the target budget
    // ---------------------------------------------------------------
    std::vector<EvictionCandidate> GetEvictionCandidates(
        uint64_t budgetBytes) const {
        // Score all entries
        std::vector<EvictionCandidate> scored;
        scored.reserve(m_entries.size());

        for (const auto& [key, rec] : m_entries) {
            const double score = ComputeEvictionScoreAt(
                rec.accessCount, rec.lastAccessMs, rec.sizeBytes, m_nowMs);
            scored.push_back({ key, score, rec.sizeBytes });
        }

        // Sort descending by score (highest = evict first)
        std::sort(scored.begin(), scored.end(),
            [](const EvictionCandidate& a, const EvictionCandidate& b) {
                return a.score > b.score;
            });

        // Determine how many bytes we need to free
        if (m_totalSize <= budgetBytes) return {}; // Already within budget

        uint64_t bytesToFree = m_totalSize - budgetBytes;
        uint64_t freed = 0;

        std::vector<EvictionCandidate> result;
        for (const auto& c : scored) {
            if (freed >= bytesToFree) break;
            result.push_back(c);
            freed += c.sizeBytes;
        }

        return result;
    }

    // ---------------------------------------------------------------
    // Simple threshold-based eviction check
    // ---------------------------------------------------------------
    static bool ShouldEvict(double score, double threshold) noexcept {
        return score >= threshold;
    }

    // ---------------------------------------------------------------
    // Aggregate queries
    // ---------------------------------------------------------------
    uint64_t GetTotalCachedSize() const noexcept { return m_totalSize; }
    size_t   GetEntryCount() const noexcept { return m_entries.size(); }

    // ---------------------------------------------------------------
    // Remove a specific entry
    // ---------------------------------------------------------------
    bool RemoveEntry(const std::string& key) {
        auto it = m_entries.find(key);
        if (it == m_entries.end()) return false;
        m_totalSize -= it->second.sizeBytes;
        m_entries.erase(it);
        return true;
    }

    // ---------------------------------------------------------------
    // Clear all entries
    // ---------------------------------------------------------------
    void Clear() noexcept {
        m_entries.clear();
        m_totalSize = 0;
    }

private:
    std::unordered_map<std::string, CacheAccessRecord> m_entries;
    uint64_t m_totalSize = 0;
    uint64_t m_nowMs = 0;

    // Default weights (tuned for general cache behavior)
    double m_recencyWeight = 1.0;
    double m_frequencyWeight = 0.5;
    double m_sizeWeight = 0.3;
};

} // namespace Engine
} // namespace ExplorerLens
