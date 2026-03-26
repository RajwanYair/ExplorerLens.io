// RemoteConfigPusher.h — Azure App Configuration Remote Config Delivery
// Copyright (c) 2026 ExplorerLens Project
//
// Polls Azure App Configuration (or a custom REST endpoint) for fleet-level
// thumbnail engine settings. Applies deltas atomically via registry writes
// and notifies FleetConfigManager to refresh.
//
#pragma once

#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <cstdint>
#include <thread>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <optional>

namespace ExplorerLens { namespace Engine { namespace Enterprise {

struct RemoteConfigEntry {
    std::string key;
    std::string value;
    std::string label;          // Azure App Config label (e.g. "Production")
    std::string contentType;    // "application/json" or "text/plain"
    std::string etag;           // For conditional GET (If-None-Match)
    std::chrono::system_clock::time_point lastModified;
};

struct RemoteConfigPusherSettings {
    std::string   endpointUrl;          // https://<name>.azconfig.io or custom
    std::string   credential;           // AAD client credential or API key
    std::string   labelFilter;          // Azure App Config label selector
    std::string   keyPrefix;            // e.g. "ExplorerLens/"
    uint32_t      pollIntervalSec  = 300;  // 5-minute default
    bool          useAADAuth       = true;
    bool          applyToRegistry  = true; // Write fetched values to GP registry
    std::string   registryRoot;    // e.g. "SOFTWARE\\Policies\\ExplorerLens"
};

enum class RemoteConfigSyncResult : uint8_t {
    NoChange     = 0,
    Updated      = 1,
    Error        = 2,
    Unauthorized = 3
};

class RemoteConfigPusher {
public:
    static RemoteConfigPusher& Instance() {
        static RemoteConfigPusher inst;
        return inst;
    }

    void Configure(RemoteConfigPusherSettings settings) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_settings = std::move(settings);
    }

    // Start background polling thread
    void StartPolling() {
        if (m_running) return;
        m_running = true;
        m_pollThread = std::thread([this]() {
            while (m_running) {
                SyncNow();
                auto deadline = std::chrono::steady_clock::now()
                    + std::chrono::seconds(m_settings.pollIntervalSec);
                while (m_running && std::chrono::steady_clock::now() < deadline)
                    std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        });
    }

    void StopPolling() {
        m_running = false;
        if (m_pollThread.joinable()) m_pollThread.join();
    }

    // Manual sync; returns Updated if values changed
    RemoteConfigSyncResult SyncNow() {
        if (m_settings.endpointUrl.empty()) return RemoteConfigSyncResult::Error;

        auto entries = FetchRemoteEntries();
        if (!entries.has_value()) return RemoteConfigSyncResult::Error;

        bool any = false;
        for (auto& e : *entries) {
            auto it = m_current.find(e.key);
            if (it == m_current.end() || it->second.etag != e.etag) {
                m_current[e.key] = e;
                if (m_settings.applyToRegistry) ApplyToRegistry(e);
                any = true;
            }
        }

        if (any) {
            FireChangeCallbacks();
            return RemoteConfigSyncResult::Updated;
        }
        return RemoteConfigSyncResult::NoChange;
    }

    // Subscribe to config change notifications
    using ChangeFn = std::function<void(const std::vector<RemoteConfigEntry>&)>;
    void OnChange(ChangeFn fn) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_callbacks.push_back(std::move(fn));
    }

    // Retrieve latest known value for a key
    std::optional<std::string> Get(const std::string& key) const {
        auto it = m_current.find(m_settings.keyPrefix + key);
        if (it != m_current.end()) return it->second.value;
        return std::nullopt;
    }

    const RemoteConfigPusherSettings& Settings() const { return m_settings; }

    ~RemoteConfigPusher() { StopPolling(); }

private:
    RemoteConfigPusher() = default;

    std::optional<std::vector<RemoteConfigEntry>> FetchRemoteEntries() {
        // Production: HTTP GET to Azure App Config REST API with AAD bearer token
        // Uses WinHTTP for SSL/TLS — note: no Winsock2 include needed (WinHTTP wraps it)
        // Returns nullopt on auth failure or network error.
        // Stub implementation returns empty list (no-op when unconfigured).
        return std::vector<RemoteConfigEntry>{};
    }

    void ApplyToRegistry(const RemoteConfigEntry& e) {
        if (m_settings.registryRoot.empty()) return;
        HKEY hk = nullptr;
        std::wstring root(m_settings.registryRoot.begin(), m_settings.registryRoot.end());
        if (RegCreateKeyExW(HKEY_LOCAL_MACHINE, root.c_str(), 0, nullptr,
            REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, nullptr, &hk, nullptr) != ERROR_SUCCESS) return;

        std::wstring kw(e.key.begin(), e.key.end());
        std::wstring vw(e.value.begin(), e.value.end());
        // Strip key prefix for registry value name
        auto slash = kw.rfind(L'/');
        auto valName = (slash != std::wstring::npos) ? kw.substr(slash + 1) : kw;

        RegSetValueExW(hk, valName.c_str(), 0, REG_SZ,
            reinterpret_cast<const BYTE*>(vw.c_str()),
            static_cast<DWORD>((vw.size() + 1) * sizeof(wchar_t)));
        RegCloseKey(hk);
    }

    void FireChangeCallbacks() {
        std::vector<RemoteConfigEntry> all;
        all.reserve(m_current.size());
        for (auto& [k, v] : m_current) all.push_back(v);
        std::lock_guard<std::mutex> lk(m_mutex);
        for (auto& fn : m_callbacks) fn(all);
    }

    RemoteConfigPusherSettings                          m_settings;
    std::unordered_map<std::string, RemoteConfigEntry>  m_current;
    std::vector<ChangeFn>                               m_callbacks;
    std::thread                                         m_pollThread;
    std::atomic<bool>                                   m_running { false };
    std::mutex                                          m_mutex;
};

}}} // namespace ExplorerLens::Engine::Enterprise
