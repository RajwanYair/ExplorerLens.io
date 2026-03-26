// PluginsPageViewModel.h — Plugins Management Page ViewModel
// Copyright (c) 2026 ExplorerLens Project
//
// Manages the Plugins page in the WinUI 3 Manager: enumerates installed plugins,
// shows version/compat status, enables/disables, triggers update checks,
// and opens the plugin marketplace in the default browser.
//
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <functional>
#include <cstdint>
#include "../Engine/SDK/SDKVersionGuard.h"

namespace ExplorerLens { namespace Engine { namespace WinUI {

// Plugin installation state
enum class PluginState {
    Enabled,
    Disabled,
    UpdateAvailable,
    Incompatible,   // Failed SDKVersionGuard check
    Corrupt,        // DLL missing or won't load
};

struct InstalledPlugin {
    std::wstring   id;          // Unique plugin identifier
    std::wstring   name;
    std::wstring   vendor;
    std::wstring   version;     // "1.2.3"
    std::wstring   dllPath;
    PluginState    state = PluginState::Enabled;
    SDKCompat      sdkCompat;
    std::vector<std::wstring> extensions;
    std::wstring   latestVersionAvailable;   // empty if no update
    std::wstring   marketplace;  // URL to plugin's marketplace page
};

class PluginsPageViewModel {
public:
    PluginsPageViewModel() { Refresh(); }

    const std::vector<InstalledPlugin>& Plugins() const { return m_plugins; }

    // Refresh plugin list from registry
    void Refresh();

    // Enable / disable a plugin by ID
    bool EnablePlugin(const std::wstring& id);
    bool DisablePlugin(const std::wstring& id);

    // Uninstall plugin (removes DLL + registry entry)
    bool UninstallPlugin(const std::wstring& id);

    // Open marketplace in browser
    static void OpenMarketplace() {
        ShellExecuteW(nullptr, L"open",
            L"https://marketplace.explorerlens.io/plugins",
            nullptr, nullptr, SW_SHOWNORMAL);
    }

    // Open individual plugin page
    static void OpenPluginPage(const std::wstring& marketplace) {
        if (!marketplace.empty())
            ShellExecuteW(nullptr, L"open", marketplace.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    }

    // Check for plugin updates (async — not implemented here; fires callback when done)
    void CheckUpdatesAsync(std::function<void()> onComplete);

    uint32_t UpdateAvailableCount() const {
        uint32_t n = 0;
        for (auto& p : m_plugins) if (p.state == PluginState::UpdateAvailable) ++n;
        return n;
    }

private:
    std::vector<InstalledPlugin> m_plugins;

    static constexpr wchar_t REGISTRY_KEY[] =
        L"Software\\ExplorerLens\\Plugins";

    InstalledPlugin LoadPlugin(HKEY hPluginKey, const std::wstring& id) const;
};

inline void PluginsPageViewModel::Refresh() {
    m_plugins.clear();
    HKEY hKey = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, REGISTRY_KEY, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return;

    wchar_t subkey[256] = {}; DWORD idx = 0, nameSz = 256;
    while (RegEnumKeyExW(hKey, idx++, subkey, &nameSz, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
        HKEY hSub = nullptr;
        if (RegOpenKeyExW(hKey, subkey, 0, KEY_READ, &hSub) == ERROR_SUCCESS) {
            m_plugins.push_back(LoadPlugin(hSub, subkey));
            RegCloseKey(hSub);
        }
        nameSz = 256;
    }
    RegCloseKey(hKey);
}

inline InstalledPlugin PluginsPageViewModel::LoadPlugin(HKEY hKey, const std::wstring& id) const {
    InstalledPlugin p;
    p.id = id;

    auto readStr = [&](const wchar_t* val, std::wstring& out) {
        wchar_t buf[512] = {}; DWORD sz = sizeof(buf);
        if (RegGetValueW(hKey, nullptr, val, RRF_RT_REG_SZ, nullptr, buf, &sz) == ERROR_SUCCESS)
            out = buf;
    };
    readStr(L"Name",       p.name);
    readStr(L"Vendor",     p.vendor);
    readStr(L"Version",    p.version);
    readStr(L"DllPath",    p.dllPath);
    readStr(L"Marketplace",p.marketplace);

    DWORD disabled = 0, sz = sizeof(disabled);
    RegGetValueW(hKey, nullptr, L"Disabled", RRF_RT_REG_DWORD, nullptr, &disabled, &sz);

    SDKVersionInfo info;
    p.sdkCompat = SDKVersionGuard::ValidateDLL(p.dllPath, &info);

    if (disabled)                               p.state = PluginState::Disabled;
    else if (p.sdkCompat == SDKCompat::MajorMismatch ||
             p.sdkCompat == SDKCompat::TooOld) p.state = PluginState::Incompatible;
    else if (GetFileAttributesW(p.dllPath.c_str()) == INVALID_FILE_ATTRIBUTES)
                                                p.state = PluginState::Corrupt;
    else                                        p.state = PluginState::Enabled;
    return p;
}

}}} // namespace ExplorerLens::Engine::WinUI
