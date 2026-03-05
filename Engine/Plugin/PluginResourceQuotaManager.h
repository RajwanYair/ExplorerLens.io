// PluginResourceQuotaManager.h — Resource Quota Enforcement for Plugins
// Copyright (c) 2026 ExplorerLens Project
//
// Enforces resource quotas (memory, CPU, GDI handles, file handles) for
// loaded plugins to prevent any single plugin from exhausting system resources.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <string>
#include <mutex>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

enum class PluginQuotaRes : uint32_t {
    Memory = 0,
    CPU = 1,
    GDIHandles = 2,
    FileHandles = 3
};

struct QuotaLimit {
    PluginQuotaRes resource = PluginQuotaRes::Memory;
    uint64_t      limit = 0;
    uint64_t      current = 0;
    uint64_t      peak = 0;
    bool          enforced = true;
    uint32_t      violations = 0;

    double Utilization() const {
        return limit > 0 ? static_cast<double>(current) / limit : 0.0;
    }

    bool IsExceeded() const {
        return enforced && current > limit && limit > 0;
    }
};

struct PluginQuotaProfile {
    std::string pluginId;
    std::unordered_map<uint32_t, QuotaLimit> quotas; // Key = PluginQuotaRes enum

    void SetQuota(PluginQuotaRes res, uint64_t limit) {
        auto& q = quotas[static_cast<uint32_t>(res)];
        q.resource = res;
        q.limit = limit;
    }

    QuotaLimit GetQuota(PluginQuotaRes res) const {
        auto it = quotas.find(static_cast<uint32_t>(res));
        return it != quotas.end() ? it->second : QuotaLimit{};
    }
};

class PluginResourceQuotaManager {
public:
    static PluginResourceQuotaManager& Instance() {
        static PluginResourceQuotaManager s;
        return s;
    }

    void SetQuota(const std::string& pluginId, PluginQuotaRes resource, uint64_t limit) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto& profile = m_profiles[pluginId];
        profile.pluginId = pluginId;
        profile.SetQuota(resource, limit);
    }

    void SetDefaultQuotas(const std::string& pluginId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto& profile = m_profiles[pluginId];
        profile.pluginId = pluginId;

        QuotaLimit memQuota;
        memQuota.resource = PluginQuotaRes::Memory;
        memQuota.limit = 256 * 1024 * 1024; // 256 MB
        profile.quotas[0] = memQuota;

        QuotaLimit cpuQuota;
        cpuQuota.resource = PluginQuotaRes::CPU;
        cpuQuota.limit = 50; // 50% max CPU
        profile.quotas[1] = cpuQuota;

        QuotaLimit gdiQuota;
        gdiQuota.resource = PluginQuotaRes::GDIHandles;
        gdiQuota.limit = 500;
        profile.quotas[2] = gdiQuota;

        QuotaLimit fileQuota;
        fileQuota.resource = PluginQuotaRes::FileHandles;
        fileQuota.limit = 100;
        profile.quotas[3] = fileQuota;
    }

    bool CheckUsage(const std::string& pluginId, PluginQuotaRes resource,
        uint64_t requestedAmount) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto pit = m_profiles.find(pluginId);
        if (pit == m_profiles.end()) return true; // No profile = unrestricted

        uint32_t resKey = static_cast<uint32_t>(resource);
        auto qit = pit->second.quotas.find(resKey);
        if (qit == pit->second.quotas.end()) return true;

        return !qit->second.enforced ||
            (qit->second.current + requestedAmount <= qit->second.limit);
    }

    void RecordUsage(const std::string& pluginId, PluginQuotaRes resource, uint64_t amount) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto pit = m_profiles.find(pluginId);
        if (pit == m_profiles.end()) return;

        uint32_t resKey = static_cast<uint32_t>(resource);
        auto& q = pit->second.quotas[resKey];
        q.current = amount;
        if (amount > q.peak) q.peak = amount;
    }

    bool Enforce(const std::string& pluginId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto pit = m_profiles.find(pluginId);
        if (pit == m_profiles.end()) return true;

        bool allOk = true;
        for (auto& [resKey, quota] : pit->second.quotas) {
            if (quota.IsExceeded()) {
                quota.violations++;
                allOk = false;
                m_totalViolations++;
            }
        }
        return allOk;
    }

    PluginQuotaProfile GetProfile(const std::string& pluginId) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_profiles.find(pluginId);
        return it != m_profiles.end() ? it->second : PluginQuotaProfile{};
    }

    uint32_t GetTotalViolations() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_totalViolations;
    }

    std::vector<std::string> GetPluginsExceedingQuota() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<std::string> result;
        for (const auto& [id, profile] : m_profiles) {
            for (const auto& [resKey, quota] : profile.quotas) {
                if (quota.IsExceeded()) {
                    result.push_back(id);
                    break;
                }
            }
        }
        return result;
    }

    void Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_profiles.clear();
        m_totalViolations = 0;
    }

    bool Validate() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& [id, profile] : m_profiles) {
            if (profile.pluginId.empty()) return false;
            if (profile.pluginId != id) return false;
            for (const auto& [resKey, quota] : profile.quotas) {
                if (quota.Utilization() < 0.0) return false;
            }
        }
        return true;
    }

private:
    PluginResourceQuotaManager() = default;
    ~PluginResourceQuotaManager() = default;
    PluginResourceQuotaManager(const PluginResourceQuotaManager&) = delete;
    PluginResourceQuotaManager& operator=(const PluginResourceQuotaManager&) = delete;

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, PluginQuotaProfile> m_profiles;
    uint32_t m_totalViolations = 0;
};

}
} // namespace ExplorerLens::Engine
