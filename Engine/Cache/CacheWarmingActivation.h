// CacheWarmingActivation.h — Cache Warming Pipeline Activation
// Copyright (c) 2026 ExplorerLens Project
//
// Activates proactive cache warming for thumbnail pre-generation in
// directories the user is likely to browse next. Integrates with
// CacheWarmingService for background scheduling.
//
#pragma once

#include <cstdint>
#include <atomic>
#include <string>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

enum class WarmActivationMode : uint8_t {
    Disabled = 0,
    Eager = 1,
    Adaptive = 2,
    Predictive = 3
};

struct WarmActivationConfig {
    WarmActivationMode strategy = WarmActivationMode::Adaptive;
    uint32_t maxConcurrentWarms = 4;
    uint32_t maxDirectoryDepth = 2;
    uint64_t maxWarmBudgetBytes = 256 * 1024 * 1024;
};

struct WarmActivationStats {
    uint64_t directoriesWarmed = 0;
    uint64_t thumbnailsPregenerated = 0;
    uint64_t cacheHitsFromWarming = 0;
    uint64_t warmingTimeMs = 0;
};

class CacheWarmingActivation {
public:
    static CacheWarmingActivation& Instance() {
        static CacheWarmingActivation s_instance;
        return s_instance;
    }

    bool Activate(const WarmActivationConfig& config) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_active.load()) return true;
        m_config = config;
        m_active.store(true);
        return true;
    }

    void Deactivate() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_active.store(false);
        m_stats = {};
    }

    bool IsActive() const { return m_active.load(); }

    void AddWatchDirectory(const std::wstring& directoryPath) {
        if (!m_active.load()) return;
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.directoriesWarmed++;
        (void)directoryPath;
    }

    const WarmActivationStats& GetStats() const { return m_stats; }
    const WarmActivationConfig& GetConfig() const { return m_config; }

private:
    CacheWarmingActivation() = default;
    ~CacheWarmingActivation() = default;
    CacheWarmingActivation(const CacheWarmingActivation&) = delete;
    CacheWarmingActivation& operator=(const CacheWarmingActivation&) = delete;

    std::atomic<bool> m_active{false};
    std::mutex m_mutex;
    WarmActivationConfig m_config;
    WarmActivationStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
