// FleetConfigManager.h — Centralized Fleet Thumbnail Policy Manager
// Copyright (c) 2026 ExplorerLens Project
//
// Aggregates policy from GroupPolicyBridge, Intune, ConfigMgr, and manual settings.
// Provides a single authoritative policy view for all engine subsystems.
//
#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <chrono>
#include <functional>
#include <optional>
#include <cstdint>

namespace ExplorerLens { namespace Engine { namespace Enterprise {

enum class FleetTier : uint8_t {
    Unknown    = 0,
    Developer  = 1,
    Standard   = 2,
    Regulated  = 3,   // e.g. HIPAA, FedRAMP — stricter defaults
    Classified = 4    // Air-gapped, all external comms disabled
};

enum class UpdateChannel : uint8_t {
    Stable   = 0,
    Preview  = 1,
    Insider  = 2,
    Disabled = 3    // MDM-managed, no auto-update
};

struct FleetThumbnailPolicy {
    bool          enableThumbnails      = true;
    bool          enableAI              = true;
    bool          enableCloudSync       = true;
    bool          enablePlugins         = true;
    bool          enableTelemetry       = true;
    bool          enableNSFWGuard       = false;
    uint32_t      maxCacheSizeMB        = 512;
    uint32_t      maxFileSizeMBPerDecode = 256;
    uint32_t      thumbnailTimeoutMs    = 5000;
    FleetTier     tier                  = FleetTier::Standard;
    UpdateChannel updateChannel         = UpdateChannel::Stable;
    std::wstring  allowedPluginPattern;   // regex filter for plugin IDs
    std::wstring  logDestinationPath;     // empty = EventLog only
    bool          forceLocalCacheOnly    = false;
    bool          disableGPUDecode       = false;
};

struct FleetConfigSnapshot {
    FleetThumbnailPolicy policy;
    std::wstring         machineId;
    std::wstring         tenantId;
    std::wstring         entraObjectId;
    std::chrono::system_clock::time_point fetchedAt;
    bool                 isPolicyManaged = false;   // true if GP/MDM controls it
};

class FleetConfigManager {
public:
    static FleetConfigManager& Instance() {
        static FleetConfigManager inst;
        return inst;
    }

    // Reload from all policy sources; should be called on GP change notifications
    void Refresh();

    // Most-recently-loaded policy snapshot
    const FleetConfigSnapshot& CurrentSnapshot() const { return m_snapshot; }
    const FleetThumbnailPolicy& Policy()         const { return m_snapshot.policy; }

    // Subscribe to policy change events
    using ChangeFn = std::function<void(const FleetThumbnailPolicy&)>;
    void OnPolicyChange(ChangeFn fn) { m_changeCallbacks.push_back(std::move(fn)); }

    // Per-machine enrollment
    bool IsManagedDevice() const { return m_snapshot.isPolicyManaged; }
    FleetTier Tier()        const { return m_snapshot.policy.tier; }

    // Export current effective policy to JSON for audit
    std::string SerializeToJson() const;

    // Validates that engine config matches fleet policy; returns violation messages
    std::vector<std::wstring> ValidateCompliance() const;

private:
    FleetConfigManager();

    void ReadGroupPolicy(FleetThumbnailPolicy& pol);
    void ReadMachineIdentity(FleetConfigSnapshot& snap);
    void FireChangeCallbacks();

    FleetConfigSnapshot              m_snapshot;
    std::vector<ChangeFn>            m_changeCallbacks;
};

// Implementation

inline FleetConfigManager::FleetConfigManager() { Refresh(); }

inline void FleetConfigManager::Refresh() {
    FleetConfigSnapshot snap;
    snap.fetchedAt = std::chrono::system_clock::now();

    ReadGroupPolicy(snap.policy);
    ReadMachineIdentity(snap);

    m_snapshot = snap;
    FireChangeCallbacks();
}

inline void FleetConfigManager::ReadGroupPolicy(FleetThumbnailPolicy& pol) {
    HKEY hk = nullptr;
    const wchar_t* root = L"SOFTWARE\\Policies\\ExplorerLens";
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, root, 0, KEY_READ, &hk) != ERROR_SUCCESS) return;

    auto readDW = [&](const wchar_t* name, DWORD def) -> DWORD {
        DWORD val = def, sz = sizeof(DWORD), type = 0;
        RegQueryValueExW(hk, name, nullptr, &type, reinterpret_cast<BYTE*>(&val), &sz);
        return val;
    };

    pol.enableThumbnails       = readDW(L"EnableThumbnails", 1) != 0;
    pol.enableAI               = readDW(L"AllowAI", 1) != 0;
    pol.enableCloudSync        = readDW(L"AllowCloudSync", 1) != 0;
    pol.enablePlugins          = readDW(L"AllowPlugins", 1) != 0;
    pol.enableTelemetry        = readDW(L"AllowTelemetry", 1) != 0;
    pol.enableNSFWGuard        = readDW(L"EnableNSFWGuard", 0) != 0;
    pol.maxCacheSizeMB         = readDW(L"MaxCacheSizeMB", 512);
    pol.thumbnailTimeoutMs     = readDW(L"ThumbnailTimeoutMs", 5000);
    pol.disableGPUDecode       = readDW(L"DisableGPUDecode", 0) != 0;
    pol.forceLocalCacheOnly    = readDW(L"ForceLocalCacheOnly", 0) != 0;
    pol.tier                   = static_cast<FleetTier>(readDW(L"FleetTier", 2));

    RegCloseKey(hk);
    m_snapshot.isPolicyManaged = true;
}

inline void FleetConfigManager::ReadMachineIdentity(FleetConfigSnapshot& snap) {
    wchar_t buf[MAX_COMPUTERNAME_LENGTH + 1] = {};
    DWORD   sz = MAX_COMPUTERNAME_LENGTH + 1;
    GetComputerNameExW(ComputerNameDnsHostname, buf, &sz);
    snap.machineId = buf;
}

inline void FleetConfigManager::FireChangeCallbacks() {
    for (auto& fn : m_changeCallbacks) fn(m_snapshot.policy);
}

inline std::vector<std::wstring> FleetConfigManager::ValidateCompliance() const {
    std::vector<std::wstring> violations;
    const auto& pol = m_snapshot.policy;

    if (pol.tier == FleetTier::Classified && pol.enableCloudSync)
        violations.push_back(L"CloudSync must be disabled on Classified tier devices");
    if (pol.tier == FleetTier::Regulated && !pol.enableNSFWGuard)
        violations.push_back(L"NSFWGuard is required on Regulated tier devices");
    if (pol.maxCacheSizeMB > 8192)
        violations.push_back(L"CacheSize exceeds 8 GB policy maximum");

    return violations;
}

inline std::string FleetConfigManager::SerializeToJson() const {
    const auto& pol = m_snapshot.policy;
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "{\"enableThumbnails\":%s,\"enableAI\":%s,\"enableCloudSync\":%s,"
        "\"enablePlugins\":%s,\"enableTelemetry\":%s,\"enableNSFWGuard\":%s,"
        "\"maxCacheSizeMB\":%u,\"thumbnailTimeoutMs\":%u,\"tier\":%d,"
        "\"disableGPUDecode\":%s,\"forceLocalCacheOnly\":%s}",
        pol.enableThumbnails    ? "true"  : "false",
        pol.enableAI            ? "true"  : "false",
        pol.enableCloudSync     ? "true"  : "false",
        pol.enablePlugins       ? "true"  : "false",
        pol.enableTelemetry     ? "true"  : "false",
        pol.enableNSFWGuard     ? "true"  : "false",
        pol.maxCacheSizeMB,
        pol.thumbnailTimeoutMs,
        static_cast<int>(pol.tier),
        pol.disableGPUDecode    ? "true"  : "false",
        pol.forceLocalCacheOnly ? "true"  : "false"
    );
    return buf;
}

}}} // namespace ExplorerLens::Engine::Enterprise
