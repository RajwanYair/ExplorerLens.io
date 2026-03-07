// PluginRateLimiter.h — Plugin API Call Rate Limiting
// Copyright (c) 2026 ExplorerLens Project
//
// Enforces rate limits on plugin API calls to prevent resource exhaustion,
// using token bucket algorithm with configurable per-plugin quotas.
//
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

enum class RateLimitResult : uint8_t {
    Allowed,
    Throttled,
    Blocked,
    QuotaExhausted
};

struct PluginQuota {
    std::string pluginId;
    uint32_t maxCallsPerSecond = 100;
    uint32_t maxCallsPerMinute = 5000;
    uint32_t burstSize = 20;
    uint64_t currentTokens = 0;
    uint64_t lastRefillTimestamp = 0;
};

struct RateLimitMetrics {
    uint64_t totalRequests = 0;
    uint64_t allowedRequests = 0;
    uint64_t throttledRequests = 0;
    uint64_t blockedRequests = 0;
    float throttleRate = 0.0f;
};

class PluginRateLimiter {
public:
    PluginRateLimiter() = default;

    void ConfigurePlugin(const std::string& pluginId, uint32_t maxPerSecond, uint32_t burstSize = 20) {
        std::lock_guard<std::mutex> lock(m_mutex);
        PluginQuota quota;
        quota.pluginId = pluginId;
        quota.maxCallsPerSecond = maxPerSecond;
        quota.burstSize = burstSize;
        quota.currentTokens = burstSize;
        m_quotas[pluginId] = quota;
    }

    RateLimitResult CheckLimit(const std::string& pluginId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_quotas.find(pluginId);
        if (it == m_quotas.end()) return RateLimitResult::Allowed;

        m_metrics.totalRequests++;
        auto& quota = it->second;

        if (quota.currentTokens > 0) {
            quota.currentTokens--;
            m_metrics.allowedRequests++;
            return RateLimitResult::Allowed;
        }

        m_metrics.throttledRequests++;
        m_metrics.throttleRate = static_cast<float>(m_metrics.throttledRequests) /
            m_metrics.totalRequests;
        return RateLimitResult::Throttled;
    }

    void RefillTokens(const std::string& pluginId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_quotas.find(pluginId);
        if (it != m_quotas.end()) {
            it->second.currentTokens = it->second.burstSize;
        }
    }

    RateLimitMetrics GetMetrics() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_metrics;
    }

    size_t GetConfiguredPluginCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_quotas.size();
    }

private:
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, PluginQuota> m_quotas;
    RateLimitMetrics m_metrics;
};

} // namespace Engine
} // namespace ExplorerLens
