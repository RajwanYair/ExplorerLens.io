// GroupPolicyBridge.h — Windows Group Policy & MDM Config Reader
// Copyright (c) 2026 ExplorerLens Project
//
// Reads HKLM/HKCU Group Policy registry keys and MDM (Intune) CSP paths,
// surfacing enterprise thumbnail policy values with a priority hierarchy.
//
#pragma once

#include <windows.h>
#include <string>
#include <optional>
#include <unordered_map>
#include <vector>
#include <functional>

namespace ExplorerLens { namespace Engine { namespace Enterprise {

// Registry root for ExplorerLens GP policies
static constexpr wchar_t GP_POLICY_ROOT[]   = L"SOFTWARE\\Policies\\ExplorerLens";
static constexpr wchar_t MDM_POLICY_ROOT[]  = L"SOFTWARE\\Microsoft\\PolicyManager\\current\\device\\ExplorerLens";

enum class BridgePolicySource : uint8_t {
    None       = 0,
    MachineGPO = 1,   // HKLM\SOFTWARE\Policies\...
    UserGPO    = 2,   // HKCU\SOFTWARE\Policies\...
    MDMIntune  = 3,   // MDM CSP via PolicyManager
    ConfigMgr  = 4,   // SCCM/MECM pushed registry
    Manual     = 5    // Direct registry (no GP)
};

struct PolicyValue {
    std::wstring    key;
    std::wstring    rawValue;
    DWORD           dwValue   = 0;
    bool            boolValue = false;
    BridgePolicySource    source    = BridgePolicySource::None;
    bool            isSet     = false;
};

class GroupPolicyBridge {
public:
    static GroupPolicyBridge& Instance() {
        static GroupPolicyBridge inst;
        return inst;
    }

    // Refresh all cached policy values from registry
    void Refresh() {
        m_cache.clear();
        LoadHive(HKEY_LOCAL_MACHINE, GP_POLICY_ROOT,  BridgePolicySource::MachineGPO);
        LoadHive(HKEY_CURRENT_USER,  GP_POLICY_ROOT,  BridgePolicySource::UserGPO);
        LoadHive(HKEY_LOCAL_MACHINE, MDM_POLICY_ROOT, BridgePolicySource::MDMIntune);
    }

    // Returns the effective policy string value, in source priority order
    std::optional<std::wstring> GetString(const wchar_t* valueName) const {
        auto it = m_cache.find(valueName);
        if (it != m_cache.end() && it->second.isSet)
            return it->second.rawValue;
        return std::nullopt;
    }

    std::optional<DWORD> GetDWord(const wchar_t* valueName) const {
        auto it = m_cache.find(valueName);
        if (it != m_cache.end() && it->second.isSet)
            return it->second.dwValue;
        return std::nullopt;
    }

    bool GetBool(const wchar_t* valueName, bool defaultVal = false) const {
        auto dw = GetDWord(valueName);
        return dw.has_value() ? (dw.value() != 0) : defaultVal;
    }

    BridgePolicySource GetSource(const wchar_t* valueName) const {
        auto it = m_cache.find(valueName);
        return (it != m_cache.end()) ? it->second.source : BridgePolicySource::None;
    }

    // Known policy keys
    bool ThumbnailsEnabled()          const { return GetBool(L"EnableThumbnails", true); }
    bool AIFeaturesAllowed()          const { return GetBool(L"AllowAIProcessing", true); }
    bool CloudSyncAllowed()           const { return GetBool(L"AllowCloudSync", true); }
    bool NSFWGuardEnabled()           const { return GetBool(L"EnableNSFWGuard", false); }
    DWORD MaxCacheSizeMB()            const { return GetDWord(L"MaxCacheSizeMB").value_or(512); }
    bool TelemetryEnabled()           const { return GetBool(L"AllowTelemetry", true); }
    bool PluginMarketplaceAllowed()   const { return GetBool(L"AllowPluginMarketplace", true); }

    // Enumerate all loaded policy values for audit
    std::vector<PolicyValue> EnumerateAll() const {
        std::vector<PolicyValue> out;
        out.reserve(m_cache.size());
        for (auto& [k, v] : m_cache) out.push_back(v);
        return out;
    }

    // Register a callback for policy change events (via RegNotifyChangeKeyValue)
    void SetChangeCallback(std::function<void()> cb) { m_changeCallback = std::move(cb); }

private:
    GroupPolicyBridge() { Refresh(); }

    void LoadHive(HKEY root, const wchar_t* path, BridgePolicySource src) {
        HKEY hk = nullptr;
        if (RegOpenKeyExW(root, path, 0, KEY_READ, &hk) != ERROR_SUCCESS) return;

        DWORD idx = 0;
        wchar_t name[256];
        DWORD   nameLen = 256;
        DWORD   type    = 0;
        BYTE    data[512];
        DWORD   dataLen = sizeof(data);

        while (RegEnumValueW(hk, idx++, name, &nameLen, nullptr, &type, data, &dataLen) == ERROR_SUCCESS) {
            PolicyValue pv;
            pv.key    = name;
            pv.source = src;
            pv.isSet  = true;

            if (type == REG_DWORD && dataLen == 4) {
                pv.dwValue   = *reinterpret_cast<DWORD*>(data);
                pv.boolValue = (pv.dwValue != 0);
                pv.rawValue  = std::to_wstring(pv.dwValue);
            } else if (type == REG_SZ) {
                pv.rawValue = reinterpret_cast<wchar_t*>(data);
            }

            // Higher-priority source wins (lower enum value = higher priority)
            auto existing = m_cache.find(name);
            if (existing == m_cache.end() || src < existing->second.source)
                m_cache[name] = pv;

            nameLen = 256;
            dataLen = sizeof(data);
        }
        RegCloseKey(hk);
    }

    std::unordered_map<std::wstring, PolicyValue> m_cache;
    std::function<void()> m_changeCallback;
};

}}} // namespace ExplorerLens::Engine::Enterprise
