// CacheReplicationStrategy.h — Defines cache entry replication policy
// Copyright (c) 2026 ExplorerLens Project
//
// Controls how frequently-accessed cache entries are replicated across
// cache tiers (L1 in-memory, L2 disk, L3 shared) for resilience.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct CacheReplicationStrategyConfig {
    bool enabled = true;
    uint32_t minAccessCountForReplication = 5;
    std::string label = "CacheReplicationStrategy";
};

class CacheReplicationStrategy {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    CacheReplicationStrategyConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    enum class Tier : uint8_t { L1_Memory, L2_Disk, L3_Shared };

    bool ShouldReplicate(uint32_t accessCount) const {
        return accessCount >= m_config.minAccessCountForReplication;
    }

    Tier GetTargetTier(uint32_t accessCount) const {
        if (accessCount >= 50) return Tier::L1_Memory;
        if (accessCount >= 10) return Tier::L2_Disk;
        return Tier::L3_Shared;
    }

private:
    bool m_initialized = false;
    CacheReplicationStrategyConfig m_config;
};

}
} // namespace ExplorerLens::Engine
