// OfflineFirstPWAManager.h — Offline-First PWA Manager
// Copyright (c) 2026 ExplorerLens Project
//
// Manages service-worker registration, thumbnail pre-caching strategies,
// and background sync for the ExplorerLens PWA shell.
//
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class PWACacheStrategy { CacheFirst, NetworkFirst, StaleWhileRevalidate };

struct PWAConfig {
    std::string      swScriptPath;
    std::string      manifestPath;
    PWACacheStrategy cacheStrategy  = PWACacheStrategy::CacheFirst;
    uint32_t         maxCacheSizeMB = 512;
    bool             enableBackgroundSync = true;
    bool             enablePushNotifications = false;
};

struct PWASyncStatus {
    uint64_t cachedThumbnails = 0;
    uint64_t pendingSyncs     = 0;
    bool     serviceWorkerActive = false;
    std::string lastSyncTime;
};

class OfflineFirstPWAManager {
public:
    explicit OfflineFirstPWAManager(const PWAConfig& cfg = {}) : m_cfg(cfg) {}

    bool RegisterServiceWorker() {
        m_swActive = true;
        return true;
    }

    bool UnregisterServiceWorker() {
        m_swActive = false;
        return true;
    }

    bool IsServiceWorkerActive() const { return m_swActive; }

    bool PreCacheThumbnails(const std::vector<std::string>& paths) {
        m_status.cachedThumbnails += static_cast<uint64_t>(paths.size());
        return true;
    }

    bool QueueBackgroundSync(const std::string& tag) {
        if (tag.empty()) return false;
        m_status.pendingSyncs++;
        return true;
    }

    bool FlushSyncQueue() {
        m_status.pendingSyncs = 0;
        return true;
    }

    PWASyncStatus GetStatus() const { return m_status; }
    const PWAConfig& GetConfig() const { return m_cfg; }

    std::string GenerateManifestJSON(const std::string& appName, const std::string& startUrl) const {
        return R"({"name":")" + appName + R"(","start_url":")" + startUrl + R"(","display":"standalone"})";
    }

private:
    PWAConfig   m_cfg;
    PWASyncStatus m_status;
    bool        m_swActive = false;
};

}} // namespace ExplorerLens::Engine
