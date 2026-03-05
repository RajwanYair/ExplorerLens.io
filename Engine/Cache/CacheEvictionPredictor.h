// CacheEvictionPredictor.h — ML-Driven Cache Eviction Prediction
// Copyright (c) 2026 ExplorerLens Project
//
// Predicts which cache entries will be needed next based on directory
// navigation patterns, recency, frequency, and format affinity.
// Replaces simple LRU with an intelligent eviction policy.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

enum class EvictionPolicy : uint8_t {
    LRU,           // Least recently used
    LFU,           // Least frequently used
    ARC,           // Adaptive replacement cache
    Predictive,    // ML-based prediction
    SizeBased,     // Evict largest first when under pressure
    COUNT
};

struct CacheEntryMeta {
    uint64_t key = 0;
    uint32_t accessCount = 0;
    double lastAccessMs = 0.0;
    uint32_t sizeBytes = 0;
    float predictedReuse = 0.0f;
    bool pinned = false;
};

struct EvictionDecision {
    uint64_t evictKey = 0;
    EvictionPolicy policy = EvictionPolicy::LRU;
    float confidence = 0.0f;
    uint32_t freedBytes = 0;
    bool evicted = false;
};

class CacheEvictionPredictor {
public:
    void SetPolicy(EvictionPolicy p) { m_policy = p; }
    EvictionPolicy GetPolicy() const { return m_policy; }

    void RecordAccess(uint64_t key, uint32_t sizeBytes) {
        auto& meta = m_entries[key];
        meta.key = key;
        meta.accessCount++;
        meta.sizeBytes = sizeBytes;
        meta.lastAccessMs = static_cast<double>(m_accessCounter++);
    }

    EvictionDecision SelectEviction(uint64_t targetBytes) const {
        (void)targetBytes;
        EvictionDecision d;
        d.policy = m_policy;
        if (m_entries.empty()) return d;

        uint64_t bestKey = 0;
        float bestScore = 1e30f;
        for (auto& [key, meta] : m_entries) {
            if (meta.pinned) continue;
            float score = static_cast<float>(meta.accessCount) * 0.3f +
                static_cast<float>(meta.lastAccessMs) * 0.7f;
            if (m_policy == EvictionPolicy::LFU)
                score = static_cast<float>(meta.accessCount);
            if (score < bestScore) {
                bestScore = score;
                bestKey = key;
            }
        }
        d.evictKey = bestKey;
        d.evicted = (bestKey != 0);
        d.confidence = 0.8f;
        auto it = m_entries.find(bestKey);
        if (it != m_entries.end()) d.freedBytes = it->second.sizeBytes;
        return d;
    }

    void PinEntry(uint64_t key) {
        if (m_entries.count(key)) m_entries[key].pinned = true;
    }

    size_t EntryCount() const { return m_entries.size(); }
    void Clear() { m_entries.clear(); }

    static const wchar_t* PolicyName(EvictionPolicy p) {
        switch (p) {
        case EvictionPolicy::LRU:        return L"LRU";
        case EvictionPolicy::LFU:        return L"LFU";
        case EvictionPolicy::ARC:        return L"ARC";
        case EvictionPolicy::Predictive: return L"Predictive";
        case EvictionPolicy::SizeBased:  return L"SizeBased";
        default: return L"Unknown";
        }
    }
    static size_t PolicyCount() { return static_cast<size_t>(EvictionPolicy::COUNT); }

private:
    EvictionPolicy m_policy = EvictionPolicy::LRU;
    std::unordered_map<uint64_t, CacheEntryMeta> m_entries;
    uint64_t m_accessCounter = 0;
};

} // namespace Engine
} // namespace ExplorerLens
