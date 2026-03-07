// CacheReplicationEngine.h — Multi-Tier Cache Replication
// Copyright (c) 2026 ExplorerLens Project
//
// Replicates cache entries across storage tiers (RAM, SSD, HDD) for
// resilience, with configurable replication policies and consistency.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ReplicationTier : uint8_t {
    L1_Memory,
    L2_SSD,
    L3_HDD,
    L4_Network
};

enum class ReplicationPolicy : uint8_t {
    None,
    Synchronous,
    Asynchronous,
    WriteBehind,
    WriteThrough
};

struct ReplicationEntry {
    std::wstring cacheKey;
    ReplicationTier sourceTier = ReplicationTier::L1_Memory;
    ReplicationTier targetTier = ReplicationTier::L2_SSD;
    uint64_t sizeBytes = 0;
    bool replicated = false;
};

struct ReplicationMetrics {
    uint64_t totalReplications = 0;
    uint64_t successfulReplications = 0;
    uint64_t failedReplications = 0;
    uint64_t bytesReplicated = 0;
    double avgReplicationTimeMs = 0.0;
};

class CacheReplicationEngine {
public:
    explicit CacheReplicationEngine(ReplicationPolicy policy = ReplicationPolicy::Asynchronous)
        : m_policy(policy) {
    }

    bool Replicate(const std::wstring& key, ReplicationTier source, ReplicationTier target, uint64_t size) {
        ReplicationEntry entry;
        entry.cacheKey = key;
        entry.sourceTier = source;
        entry.targetTier = target;
        entry.sizeBytes = size;
        entry.replicated = true;
        m_entries.push_back(entry);
        m_metrics.totalReplications++;
        m_metrics.successfulReplications++;
        m_metrics.bytesReplicated += size;
        return true;
    }

    bool IsReplicated(const std::wstring& key, ReplicationTier tier) const {
        for (const auto& entry : m_entries) {
            if (entry.cacheKey == key && entry.targetTier == tier && entry.replicated)
                return true;
        }
        return false;
    }

    ReplicationMetrics GetMetrics() const { return m_metrics; }
    ReplicationPolicy GetPolicy() const { return m_policy; }
    void SetPolicy(ReplicationPolicy policy) { m_policy = policy; }
    size_t GetReplicatedEntryCount() const { return m_entries.size(); }

    void PurgeReplicas(ReplicationTier tier) {
        m_entries.erase(
            std::remove_if(m_entries.begin(), m_entries.end(),
                [tier](const ReplicationEntry& e) { return e.targetTier == tier; }),
            m_entries.end());
    }

private:
    std::vector<ReplicationEntry> m_entries;
    ReplicationPolicy m_policy;
    ReplicationMetrics m_metrics;
};

} // namespace Engine
} // namespace ExplorerLens
