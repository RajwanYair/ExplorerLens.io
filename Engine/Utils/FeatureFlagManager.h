// FeatureFlagManager.h — Runtime Feature Flag System via Registry and Cloud
// Copyright (c) 2026 ExplorerLens Project
//
// Resolves feature flags from a priority-ordered chain: Group Policy (HKLM) →
// Cloud config (polled every 15 min) → User override (HKCU) → Compiled-in defaults.
// Provides type-safe accessors and change notification callbacks.
//
#pragma once
#include <windows.h>
#include <string>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <vector>
#include <variant>

namespace ExplorerLens { namespace Engine {

// Feature flag value: bool, int, or string
using FlagValue = std::variant<bool, int32_t, std::wstring>;

enum class FlagSource {
    Default,    // Compiled-in default
    UserHKCU,   // HKCU registry override
    CloudConfig,// Polled remote config
    GroupPolicy // HKLM GP policy (highest priority)
};

struct FlagRecord {
    std::wstring name;
    FlagValue    value;
    FlagSource   source = FlagSource::Default;
};

// Registry layout:
//   HKLM\Software\Policies\ExplorerLens\Flags\<name>  (GP / highest)
//   HKCU\Software\ExplorerLens\Flags\<name>            (user override)

class FeatureFlagManager {
public:
    static FeatureFlagManager& Get() {
        static FeatureFlagManager s_inst;
        return s_inst;
    }

    // Register a flag with a compiled-in default
    void Declare(const std::wstring& name, FlagValue defaultVal) {
        std::lock_guard<std::mutex> lk(m_mtx);
        if (m_flags.find(name) == m_flags.end()) {
            m_flags[name] = { name, defaultVal, FlagSource::Default };
        }
    }

    // Get bool flag (returns defaultValue if not declared)
    bool GetBool(const std::wstring& name, bool fallback = false) const {
        auto rec = Resolve(name);
        if (rec && std::holds_alternative<bool>(rec->value))
            return std::get<bool>(rec->value);
        return fallback;
    }

    int32_t GetInt(const std::wstring& name, int32_t fallback = 0) const {
        auto rec = Resolve(name);
        if (rec && std::holds_alternative<int32_t>(rec->value))
            return std::get<int32_t>(rec->value);
        return fallback;
    }

    std::wstring GetString(const std::wstring& name,
                           const std::wstring& fallback = L"") const {
        auto rec = Resolve(name);
        if (rec && std::holds_alternative<std::wstring>(rec->value))
            return std::get<std::wstring>(rec->value);
        return fallback;
    }

    // Reload all flags from registry (call on startup + WM_SETTINGCHANGE)
    void Reload() {
        std::lock_guard<std::mutex> lk(m_mtx);
        for (auto& [name, rec] : m_flags) {
            // Priority: GP / HKLM → User / HKCU → Default
            if (LoadFromRegistry(HKEY_LOCAL_MACHINE,
                    L"Software\\Policies\\ExplorerLens\\Flags", name, rec)) {
                rec.source = FlagSource::GroupPolicy;
            } else if (LoadFromRegistry(HKEY_CURRENT_USER,
                    L"Software\\ExplorerLens\\Flags", name, rec)) {
                rec.source = FlagSource::UserHKCU;
            } else {
                // Keep default
                rec.source = FlagSource::Default;
            }
        }
        for (auto& cb : m_onChange) cb();
    }

    // Apply a cloud payload (JSON-style key:value flat map)
    void ApplyCloudPayload(
            const std::unordered_map<std::wstring, std::wstring>& payload) {
        std::lock_guard<std::mutex> lk(m_mtx);
        for (const auto& [key, val] : payload) {
            auto it = m_flags.find(key);
            if (it == m_flags.end()) continue;
            // Only apply cloud if not overridden by GP
            if (it->second.source == FlagSource::GroupPolicy) continue;
            it->second.source = FlagSource::CloudConfig;
            // Parse value type from existing default type
            if (std::holds_alternative<bool>(it->second.value))
                it->second.value = (val == L"true" || val == L"1");
            else if (std::holds_alternative<int32_t>(it->second.value))
                it->second.value = static_cast<int32_t>(_wtoi(val.c_str()));
            else
                it->second.value = val;
        }
        for (auto& cb : m_onChange) cb();
    }

    // Set user override (HKCU)
    void SetUserOverride(const std::wstring& name, bool val) {
        WriteHKCU(name, val ? L"1" : L"0", REG_DWORD);
        Reload();
    }

    FlagSource GetSource(const std::wstring& name) const {
        auto rec = Resolve(name);
        return rec ? rec->source : FlagSource::Default;
    }

    void OnChange(std::function<void()> cb) {
        std::lock_guard<std::mutex> lk(m_mtx);
        m_onChange.push_back(std::move(cb));
    }

    // Dump all flags (for diagnostics)
    std::wstring DumpAll() const {
        std::lock_guard<std::mutex> lk(m_mtx);
        std::wstring out;
        for (const auto& [name, rec] : m_flags) {
            out += name + L" = ";
            std::visit([&](const auto& v) {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<T, bool>)
                    out += v ? L"true" : L"false";
                else if constexpr (std::is_same_v<T, int32_t>)
                    out += std::to_wstring(v);
                else
                    out += v;
            }, rec.value);
            out += L" [";
            switch (rec.source) {
            case FlagSource::Default:     out += L"default"; break;
            case FlagSource::UserHKCU:    out += L"user"; break;
            case FlagSource::CloudConfig: out += L"cloud"; break;
            case FlagSource::GroupPolicy: out += L"gpo"; break;
            }
            out += L"]\n";
        }
        return out;
    }

private:
    FeatureFlagManager() { RegisterBuiltins(); Reload(); }

    void RegisterBuiltins() {
        Declare(L"gpu.enabled",           true);
        Declare(L"gpu.d3d12",             true);
        Declare(L"cache.persist",         true);
        Declare(L"cache.maxMb",           int32_t(512));
        Declare(L"ai.smartCrop",          false);
        Declare(L"ai.sceneUnderstanding", false);
        Declare(L"plugins.enabled",       true);
        Declare(L"telemetry.enabled",     false);
        Declare(L"updates.auto",          true);
        Declare(L"ui.animations",         true);
        Declare(L"ui.rtl",                false);
        Declare(L"store.channel",         std::wstring(L"stable"));
    }

    const FlagRecord* Resolve(const std::wstring& name) const {
        auto it = m_flags.find(name);
        return (it != m_flags.end()) ? &it->second : nullptr;
    }

    bool LoadFromRegistry(HKEY root, const wchar_t* subKey,
                          const std::wstring& name, FlagRecord& rec) {
        HKEY hk = nullptr;
        if (RegOpenKeyExW(root, subKey, 0, KEY_READ, &hk) != ERROR_SUCCESS)
            return false;
        bool found = false;
        if (std::holds_alternative<bool>(rec.value) ||
            std::holds_alternative<int32_t>(rec.value)) {
            DWORD val = 0, sz = sizeof(val), type = 0;
            if (RegQueryValueExW(hk, name.c_str(), nullptr, &type,
                    reinterpret_cast<BYTE*>(&val), &sz) == ERROR_SUCCESS) {
                if (std::holds_alternative<bool>(rec.value))
                    rec.value = val != 0;
                else
                    rec.value = static_cast<int32_t>(val);
                found = true;
            }
        } else {
            wchar_t buf[512] = {}; DWORD sz = sizeof(buf);
            if (RegQueryValueExW(hk, name.c_str(), nullptr, nullptr,
                    reinterpret_cast<BYTE*>(buf), &sz) == ERROR_SUCCESS) {
                rec.value = std::wstring(buf);
                found = true;
            }
        }
        RegCloseKey(hk);
        return found;
    }

    void WriteHKCU(const std::wstring& name, const std::wstring& val, DWORD type) {
        HKEY hk = nullptr;
        RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\ExplorerLens\\Flags",
                0, nullptr, 0, KEY_WRITE, nullptr, &hk, nullptr);
        if (!hk) return;
        if (type == REG_DWORD) {
            DWORD d = _wtoi(val.c_str());
            RegSetValueExW(hk, name.c_str(), 0, REG_DWORD,
                    reinterpret_cast<const BYTE*>(&d), sizeof(DWORD));
        } else {
            RegSetValueExW(hk, name.c_str(), 0, REG_SZ,
                reinterpret_cast<const BYTE*>(val.c_str()),
                static_cast<DWORD>((val.size()+1)*sizeof(wchar_t)));
        }
        RegCloseKey(hk);
    }

    mutable std::mutex m_mtx;
    std::unordered_map<std::wstring, FlagRecord> m_flags;
    std::vector<std::function<void()>>           m_onChange;
};

}} // namespace ExplorerLens::Engine
