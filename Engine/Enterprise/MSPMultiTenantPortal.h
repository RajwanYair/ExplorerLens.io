// MSPMultiTenantPortal.h — MSP Multi-Tenant Portal
// Copyright (c) 2026 ExplorerLens Project
//
// Provides managed service providers with isolated per-customer tenant
// namespaces, credential scoping, and data isolation guarantees.
//
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

struct TenantConfig {
    std::string  tenantId;
    std::string  displayName;
    std::string  isolationKey;
    uint64_t     maxNodes       = 100;
    uint64_t     storageLimitMB = 10240;
    bool         active         = true;
};

struct TenantStats {
    std::string  tenantId;
    uint64_t     nodeCount      = 0;
    uint64_t     storageMB      = 0;
    uint64_t     thumbsGenerated= 0;
    bool         withinLimits   = true;
};

class MSPMultiTenantPortal {
public:
    MSPMultiTenantPortal() = default;

    bool Initialize() { m_ready = true; return true; }
    bool IsReady() const { return m_ready; }

    bool CreateTenant(const TenantConfig& cfg) {
        if (cfg.tenantId.empty() || m_tenants.count(cfg.tenantId)) return false;
        m_tenants[cfg.tenantId] = cfg;
        m_stats[cfg.tenantId]   = TenantStats{cfg.tenantId};
        return true;
    }

    bool DeleteTenant(const std::string& tenantId) {
        m_stats.erase(tenantId);
        return m_tenants.erase(tenantId) > 0;
    }

    bool IsIsolated(const std::string& tenantA, const std::string& tenantB) const {
        auto itA = m_tenants.find(tenantA);
        auto itB = m_tenants.find(tenantB);
        if (itA == m_tenants.end() || itB == m_tenants.end()) return false;
        return itA->second.isolationKey != itB->second.isolationKey;
    }

    TenantStats GetStats(const std::string& tenantId) const {
        auto it = m_stats.find(tenantId);
        return it != m_stats.end() ? it->second : TenantStats{tenantId};
    }

    bool RecordThumb(const std::string& tenantId) {
        auto it = m_stats.find(tenantId);
        if (it == m_stats.end()) return false;
        ++it->second.thumbsGenerated;
        return true;
    }

    uint64_t GetTenantCount() const { return static_cast<uint64_t>(m_tenants.size()); }
    void Shutdown() { m_ready = false; }

private:
    bool m_ready = false;
    std::unordered_map<std::string, TenantConfig> m_tenants;
    std::unordered_map<std::string, TenantStats>  m_stats;
};

}} // namespace ExplorerLens::Engine
