// SettingsViewModel.h — Settings Page ViewModel for Manager.WinUI
// Copyright (c) 2026 ExplorerLens Project
//
// Manages all user-facing settings for the WinUI 3 Manager. Reads/writes
// settings from the registry (HKCU) and exposes observable properties
// for data-binding. Validates all inputs before persisting.
//
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace ExplorerLens { namespace Engine { namespace WinUI {

// Setting category identifiers
enum class SettingCategory {
    General,
    Performance,
    Cache,
    Privacy,
    Plugins,
    Advanced,
};

// Base for all setting entries
struct SettingEntry {
    std::wstring     key;
    std::wstring     displayName;
    std::wstring     description;
    SettingCategory  category    = SettingCategory::General;
    bool             requiresAdmin = false;
    bool             requiresRestart = false;
};

// Typed setting values
struct BoolSetting   : SettingEntry { bool    value = false; bool    defaultVal = false; };
struct IntSetting    : SettingEntry { int32_t value = 0;     int32_t defaultVal = 0;
                                      int32_t minVal = 0;    int32_t maxVal = 100; };
struct StringSetting : SettingEntry { std::wstring value; std::wstring defaultVal; };

// PropertyChanged callback: void(key)
using PropertyChangedCallback = std::function<void(const std::wstring&)>;

class SettingsViewModel {
public:
    SettingsViewModel() { Load(); }

    // Observable bool settings
    bool GetBool(const std::wstring& key) const;
    void SetBool(const std::wstring& key, bool value);

    int32_t      GetInt(const std::wstring& key) const;
    void         SetInt(const std::wstring& key, int32_t value);

    std::wstring GetString(const std::wstring& key) const;
    void         SetString(const std::wstring& key, const std::wstring& value);

    // Persist all modified settings to registry
    bool Save();

    // Reload from registry (discards unsaved changes)
    void Load();

    // Reset a single setting to default
    void ResetToDefault(const std::wstring& key);

    // Reset all settings to defaults
    void ResetAll();

    // Subscribe to property changes
    void AddPropertyChangedCallback(PropertyChangedCallback cb) {
        m_callbacks.push_back(std::move(cb));
    }

    // Get all settings in a category for UI rendering
    std::vector<std::wstring> GetKeysForCategory(SettingCategory cat) const;

private:
    std::vector<BoolSetting>   m_bools;
    std::vector<IntSetting>    m_ints;
    std::vector<StringSetting> m_strings;
    std::vector<PropertyChangedCallback> m_callbacks;

    static constexpr wchar_t REGISTRY_KEY[] =
        L"Software\\ExplorerLens\\Manager\\Settings";

    void Notify(const std::wstring& key) {
        for (auto& cb : m_callbacks) cb(key);
    }

    void RegisterDefaults() {
        m_bools = {
            { {L"EnableThumbnails",    L"Enable Thumbnails",     L"Generate thumbnails for all supported formats", SettingCategory::General} },
            { {L"GPUAcceleration",     L"GPU Acceleration",      L"Use GPU for fast thumbnail decode",             SettingCategory::Performance} },
            { {L"HighQuality",         L"High Quality Decode",   L"Use best quality at cost of speed",             SettingCategory::Performance} },
            { {L"EnableCache",         L"Enable Disk Cache",     L"Cache thumbnails to speed up repeat loads",     SettingCategory::Cache} },
            { {L"TelemetryOptIn",      L"Usage Telemetry",       L"Send anonymous usage statistics",                SettingCategory::Privacy} },
            { {L"PluginAutoUpdate",    L"Plugin Auto-Update",    L"Automatically update installed plugins",         SettingCategory::Plugins} },
        };
        for (auto& b : m_bools) b.value = b.defaultVal;

        m_ints = {
            { {L"CacheSizeMB",   L"Cache Size (MB)",      L"Max disk cache size",  SettingCategory::Cache,   false, false}, 512, 512, 64, 4096 },
            { {L"ThumbnailWidth",L"Thumbnail Width (px)", L"Default output width", SettingCategory::General, false, false}, 256, 256, 64, 1024 },
            { {L"WorkerThreads", L"Worker Threads",       L"Decode parallelism",   SettingCategory::Performance, false, true}, 0, 0, 0, 32 },
        };

        m_strings = {
            { {L"CacheDir", L"Cache Directory", L"Path for thumbnail cache files", SettingCategory::Cache} },
        };
    }
};

inline void SettingsViewModel::Load() {
    RegisterDefaults();
    HKEY hKey = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, REGISTRY_KEY, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return;
    for (auto& b : m_bools) {
        DWORD val = 0, sz = sizeof(val);
        if (RegGetValueW(hKey, nullptr, b.key.c_str(), RRF_RT_REG_DWORD, nullptr, &val, &sz) == ERROR_SUCCESS)
            b.value = val != 0;
    }
    for (auto& i : m_ints) {
        DWORD val = 0, sz = sizeof(val);
        if (RegGetValueW(hKey, nullptr, i.key.c_str(), RRF_RT_REG_DWORD, nullptr, &val, &sz) == ERROR_SUCCESS)
            i.value = (int32_t)val;
    }
    RegCloseKey(hKey);
}

inline bool SettingsViewModel::Save() {
    HKEY hKey = nullptr;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, REGISTRY_KEY, 0, nullptr,
            REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS)
        return false;
    for (auto& b : m_bools) {
        DWORD val = b.value ? 1 : 0;
        RegSetValueExW(hKey, b.key.c_str(), 0, REG_DWORD, (BYTE*)&val, sizeof(val));
    }
    for (auto& i : m_ints) {
        DWORD val = (DWORD)i.value;
        RegSetValueExW(hKey, i.key.c_str(), 0, REG_DWORD, (BYTE*)&val, sizeof(val));
    }
    RegCloseKey(hKey);
    return true;
}

}}} // namespace ExplorerLens::Engine::WinUI
