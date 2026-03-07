// ResourceThrottlePolicy.h — System Resource Consumption Limiter
// Copyright (c) 2026 ExplorerLens Project
//
// Enforces policies on CPU, memory, and I/O usage to prevent the
// thumbnail generator from degrading system responsiveness.
//
#pragma once

#include <cstdint>
#include <string>
#include <algorithm>
#include "SystemResourceMonitor.h"  // for ThrottleLevel

namespace ExplorerLens {
namespace Engine {

struct ResourceThrottleCfg {
    double cpuCeilingPercent = 50.0;
    uint64_t memoryBudgetBytes = 512ULL * 1024 * 1024;
    uint32_t ioBandwidthMBps = 100;
    uint32_t maxConcurrentDecodes = 4;
    bool respectBatteryMode = true;
    bool respectGameMode = false;
};

struct ThrottleSnapshot {
    ThrottleLevel currentLevel = ThrottleLevel::None;
    double cpuUsagePercent = 0.0;
    uint64_t memoryUsedBytes = 0;
    uint32_t activeDecodes = 0;
    uint32_t throttledRequests = 0;
    uint32_t pausedMs = 0;
};

class ResourceThrottlePolicy {
public:
    void Configure(const ResourceThrottleCfg& config) { m_config = config; }

    ThrottleLevel Evaluate(double cpuPercent, uint64_t memBytes) const {
        if (cpuPercent > m_config.cpuCeilingPercent * 1.5 ||
            memBytes > m_config.memoryBudgetBytes * 2)
            return ThrottleLevel::Paused;
        if (cpuPercent > m_config.cpuCeilingPercent * 1.2 ||
            memBytes > m_config.memoryBudgetBytes * 1.5)
            return ThrottleLevel::Heavy;
        if (cpuPercent > m_config.cpuCeilingPercent ||
            memBytes > m_config.memoryBudgetBytes)
            return ThrottleLevel::Moderate;
        if (cpuPercent > m_config.cpuCeilingPercent * 0.8)
            return ThrottleLevel::Light;
        return ThrottleLevel::None;
    }

    uint32_t AllowedConcurrency(ThrottleLevel level) const {
        switch (level) {
        case ThrottleLevel::None:     return m_config.maxConcurrentDecodes;
        case ThrottleLevel::Light:    return std::max(1u, m_config.maxConcurrentDecodes * 3 / 4);
        case ThrottleLevel::Moderate: return std::max(1u, m_config.maxConcurrentDecodes / 2);
        case ThrottleLevel::Heavy:    return 1;
        case ThrottleLevel::Paused:   return 0;
        }
        return 1;
    }

    ThrottleSnapshot GetSnapshot() const { return m_snapshot; }

private:
    ResourceThrottleCfg m_config;
    ThrottleSnapshot m_snapshot;
};

} // namespace Engine
} // namespace ExplorerLens
