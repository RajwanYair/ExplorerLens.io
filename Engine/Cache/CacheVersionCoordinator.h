// CacheVersionCoordinator.h — Versioned cache consistency coordination
// Copyright (c) 2026 ExplorerLens Project
//
// Coordinates cache version consistency across concurrent threads using
// atomic versioning and optimistic compare-and-swap protocols.
//
#pragma once
#include <string>
#include <cstdint>
#include <atomic>

namespace ExplorerLens {
namespace Engine {

struct CacheVersionCoordinatorConfig {
    bool enabled = true;
    uint32_t maxRetries = 3;
    bool strictOrdering = false;
    std::string label = "CacheVersionCoordinator";
};

class CacheVersionCoordinator {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    CacheVersionCoordinatorConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    uint64_t GetCurrentVersion() const { return m_version.load(); }

    bool TryUpdate(uint64_t expectedVersion) {
        uint64_t expected = expectedVersion;
        if (m_version.compare_exchange_strong(expected, expectedVersion + 1)) {
            m_successfulUpdates++;
            return true;
        }
        m_conflictCount++;
        return false;
    }

    void ForceVersion(uint64_t version) { m_version.store(version); }
    uint64_t GetConflictCount() const { return m_conflictCount; }
    uint64_t GetSuccessfulUpdates() const { return m_successfulUpdates; }

private:
    bool m_initialized = false;
    CacheVersionCoordinatorConfig m_config;
    std::atomic<uint64_t> m_version{ 0 };
    uint64_t m_conflictCount = 0;
    uint64_t m_successfulUpdates = 0;
};

}
} // namespace ExplorerLens::Engine
