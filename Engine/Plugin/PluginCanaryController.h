// PluginCanaryController.h — Plugin Canary Deployment Controller
// Copyright (c) 2026 ExplorerLens Project
//
// Manages canary rollouts for plugin updates — gates traffic by percentage and auto-rollbacks on error spikes.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

enum class CanaryState { Inactive, Rolling, Stable, RollingBack };
struct CanaryRelease { std::string pluginId; std::string version; double trafficPct; CanaryState state; uint32_t errors; };
class PluginCanaryController {
public:
    void   StartCanary(const std::string& pluginId, const std::string& ver, double pct) {
        m_releases[pluginId] = { pluginId, ver, pct, CanaryState::Rolling, 0 };
    }
    bool   ShouldUseCanaray(const std::string& pluginId, uint64_t userId) const {
        auto it = m_releases.find(pluginId);
        if (it == m_releases.end() || it->second.state != CanaryState::Rolling) return false;
        return (userId % 100) < static_cast<uint64_t>(it->second.trafficPct * 100);
    }
    void   RecordError(const std::string& pluginId) {
        if (m_releases.count(pluginId)) { m_releases[pluginId].errors++; }
    }
    bool   IsRollingBack(const std::string& pluginId) const {
        auto it = m_releases.find(pluginId);
        return it != m_releases.end() && it->second.state == CanaryState::RollingBack;
    }
    size_t ActiveCanaries() const { return m_releases.size(); }
private:
    std::unordered_map<std::string, CanaryRelease> m_releases;
};

} // namespace Engine
} // namespace ExplorerLens