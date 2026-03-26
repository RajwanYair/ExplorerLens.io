// TenantIsolationPolicy.h — Tenant-Scoped Resource Isolation
// Copyright (c) 2026 ExplorerLens Project
//
// Enforces cache, model, and config isolation between Azure AD tenants on
// shared devices (BYOD/kiosk scenarios). Each tenantId gets its own
// cache partition, AI model namespace, and audit trail.
//
#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <functional>
#include <cstdint>

namespace ExplorerLens { namespace Engine { namespace Enterprise {

struct TenantResourceLimits {
    uint32_t maxCacheMB         = 256;
    uint32_t maxModelVRAMMB     = 512;
    uint32_t maxDecodeQueueDepth= 8;
    uint32_t maxConcurrentOps   = 4;
    bool     isolateAIModels    = true;   // each tenant loads its own model copy
    bool     isolateCache       = true;   // separate LRU partition per tenant
    bool     isolateAuditLog    = true;   // separate CEF stream per tenant
};

enum class TenantAuthMode : uint8_t {
    Anonymous    = 0,   // No identity check (non-enterprise)
    AzureAD      = 1,   // Entra ID (Azure AD) token validation
    ADFS         = 2,   // On-premises ADFS federation
    LocalDomain  = 3    // Windows domain SID-based tenant
};

struct TenantDescriptor {
    std::string           tenantId;        // GUID string
    std::wstring          tenantName;      // Display name (e.g. "Contoso")
    std::wstring          cachePath;       // Absolute path to this tenant cache root
    TenantAuthMode        authMode        = TenantAuthMode::AzureAD;
    TenantResourceLimits  limits;
    bool                  active          = true;
};

struct TenantIsolationStats {
    std::string tenantId;
    uint32_t    cacheUsedMB      = 0;
    uint32_t    vramUsedMB       = 0;
    uint64_t    decodesTotal     = 0;
    uint32_t    activeOps        = 0;
    bool        limitsExceeded   = false;
};

class TenantIsolationPolicy {
public:
    static TenantIsolationPolicy& Instance() {
        static TenantIsolationPolicy inst;
        return inst;
    }

    // Register a tenant (call at startup from FleetConfigManager)
    void RegisterTenant(const TenantDescriptor& desc) {
        m_tenants[desc.tenantId] = desc;
        m_stats[desc.tenantId]   = { desc.tenantId };
    }

    void UnregisterTenant(const std::string& tenantId) {
        m_tenants.erase(tenantId);
        m_stats.erase(tenantId);
    }

    // Look up the tenant for the currently active session
    std::optional<TenantDescriptor> ResolveTenant(const std::string& tenantId) const {
        auto it = m_tenants.find(tenantId);
        if (it != m_tenants.end() && it->second.active) return it->second;
        return std::nullopt;
    }

    // Enforce limits before decode operations
    bool CheckDecodeAllowed(const std::string& tenantId) const {
        auto it = m_stats.find(tenantId);
        auto td = m_tenants.find(tenantId);
        if (it == m_stats.end() || td == m_tenants.end()) return true;
        return it->second.activeOps < td->second.limits.maxConcurrentOps;
    }

    void ReportDecodeStart(const std::string& tenantId) {
        m_stats[tenantId].activeOps++;
        m_stats[tenantId].decodesTotal++;
    }

    void ReportDecodeEnd(const std::string& tenantId, uint32_t cacheDeltaMB) {
        auto& st = m_stats[tenantId];
        if (st.activeOps > 0) st.activeOps--;
        st.cacheUsedMB += cacheDeltaMB;

        auto td = m_tenants.find(tenantId);
        if (td != m_tenants.end())
            st.limitsExceeded = (st.cacheUsedMB > td->second.limits.maxCacheMB);
    }

    // Cache root path for an isolated tenant partition
    std::wstring TenantCachePath(const std::string& tenantId) const {
        auto it = m_tenants.find(tenantId);
        if (it != m_tenants.end()) return it->second.cachePath;

        // Fallback: %LOCALAPPDATA%\ExplorerLens\tenants\<id>\
        wchar_t la[MAX_PATH] = {};
        ExpandEnvironmentStringsW(L"%LOCALAPPDATA%\\ExplorerLens\\tenants\\", la, MAX_PATH);
        std::wstring base = la;
        base.insert(base.end(), tenantId.begin(), tenantId.end());
        return base + L"\\";
    }

    // Enumerate all registered tenants
    std::vector<TenantDescriptor> AllTenants() const {
        std::vector<TenantDescriptor> out;
        out.reserve(m_tenants.size());
        for (auto& [k, v] : m_tenants) out.push_back(v);
        return out;
    }

    const TenantIsolationStats& Stats(const std::string& tenantId) {
        return m_stats[tenantId];
    }

    // Subscribe to limit-exceeded events
    using ViolationFn = std::function<void(const std::string& tenantId, const std::string& rule)>;
    void OnLimitExceeded(ViolationFn fn) { m_violationCb = std::move(fn); }

private:
    TenantIsolationPolicy() = default;

    std::unordered_map<std::string, TenantDescriptor>     m_tenants;
    std::unordered_map<std::string, TenantIsolationStats> m_stats;
    ViolationFn                                            m_violationCb;
};

}}} // namespace ExplorerLens::Engine::Enterprise
