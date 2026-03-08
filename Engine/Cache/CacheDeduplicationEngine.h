// CacheDeduplicationEngine.h — Duplicate thumbnail detection and elimination
// Copyright (c) 2026 ExplorerLens Project
//
// Detects and eliminates duplicate cached thumbnails using content hashing,
// replacing duplicates with references to save cache space.
//
#pragma once
#include <string>
#include <cstdint>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

struct CacheDeduplicationEngineConfig {
    bool enabled = true;
    uint32_t hashBuckets = 4096;
    std::string label = "CacheDeduplicationEngine";
};

class CacheDeduplicationEngine {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    CacheDeduplicationEngineConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    bool IsDuplicate(uint64_t contentHash) const {
        return m_hashToCanonical.find(contentHash) != m_hashToCanonical.end();
    }

    std::string GetCanonical(uint64_t contentHash) const {
        auto it = m_hashToCanonical.find(contentHash);
        return it != m_hashToCanonical.end() ? it->second : "";
    }

    void Register(uint64_t contentHash, const std::string& cacheKey) {
        if (m_hashToCanonical.find(contentHash) == m_hashToCanonical.end())
            m_hashToCanonical[contentHash] = cacheKey;
        else
            m_deduplicatedCount++;
    }

    uint64_t GetDeduplicatedCount() const { return m_deduplicatedCount; }
    uint32_t GetUniqueCount() const { return static_cast<uint32_t>(m_hashToCanonical.size()); }

private:
    bool m_initialized = false;
    CacheDeduplicationEngineConfig m_config;
    std::unordered_map<uint64_t, std::string> m_hashToCanonical;
    uint64_t m_deduplicatedCount = 0;
};

}
} // namespace ExplorerLens::Engine
