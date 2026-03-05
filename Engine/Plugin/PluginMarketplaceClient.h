// PluginMarketplaceClient.h — Online Plugin Marketplace Browser
// Copyright (c) 2026 ExplorerLens Project
//
// Online plugin marketplace client. Handles discovery, download, and verification
// of plugins from a remote marketplace with integrity checking.
//
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <mutex>
#include <chrono>

namespace ExplorerLens {
namespace Engine {

enum class MarketplacePluginCategory : uint8_t {
    Decoder,
    Renderer,
    Filter,
    Archive,
    ThreeD,
    Scientific,
    Utility,
    Theme
};

enum class DownloadStatus : uint8_t {
    Idle,
    Downloading,
    Verifying,
    Installing,
    Completed,
    Failed
};

struct MarketplaceClientEntry {
    std::string pluginId;
    std::string name;
    std::string author;
    std::string version;
    std::string description;
    MarketplacePluginCategory category = MarketplacePluginCategory::Utility;
    uint64_t sizeBytes = 0;
    uint32_t downloadCount = 0;
    float rating = 0.0f;
    uint32_t ratingCount = 0;
    std::string sha256Hash;
    std::string downloadUrl;
    bool isVerified = false;
};

struct MarketplaceDownloadProgress {
    std::string pluginId;
    DownloadStatus status = DownloadStatus::Idle;
    uint64_t bytesDownloaded = 0;
    uint64_t totalBytes = 0;
    float progressPercent = 0.0f;
    std::string errorMessage;
};

struct SearchFilter {
    std::string query;
    MarketplacePluginCategory category = MarketplacePluginCategory::Utility;
    bool filterByCategory = false;
    bool verifiedOnly = true;
    float minRating = 0.0f;
    uint32_t maxResults = 20;
    std::string sortBy = "downloads";
};

class PluginMarketplaceClient {
public:
    static PluginMarketplaceClient& Instance() {
        static PluginMarketplaceClient instance;
        return instance;
    }

    inline void AddEntry(const MarketplaceClientEntry& entry) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_catalog[entry.pluginId] = entry;
    }

    inline std::vector<MarketplaceClientEntry> Search(const SearchFilter& filter) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<MarketplaceClientEntry> results;

        for (const auto& [id, entry] : m_catalog) {
            if (filter.verifiedOnly && !entry.isVerified) continue;
            if (filter.filterByCategory && entry.category != filter.category) continue;
            if (entry.rating < filter.minRating) continue;

            if (!filter.query.empty()) {
                bool found = ContainsCaseInsensitive(entry.name, filter.query) ||
                    ContainsCaseInsensitive(entry.description, filter.query) ||
                    ContainsCaseInsensitive(entry.author, filter.query);
                if (!found) continue;
            }

            results.push_back(entry);
        }

        if (filter.sortBy == "downloads") {
            std::sort(results.begin(), results.end(),
                [](const MarketplaceClientEntry& a, const MarketplaceClientEntry& b) {
                    return a.downloadCount > b.downloadCount;
                });
        }
        else if (filter.sortBy == "rating") {
            std::sort(results.begin(), results.end(),
                [](const MarketplaceClientEntry& a, const MarketplaceClientEntry& b) {
                    return a.rating > b.rating;
                });
        }
        else if (filter.sortBy == "name") {
            std::sort(results.begin(), results.end(),
                [](const MarketplaceClientEntry& a, const MarketplaceClientEntry& b) {
                    return a.name < b.name;
                });
        }

        if (results.size() > filter.maxResults) results.resize(filter.maxResults);
        return results;
    }

    inline MarketplaceDownloadProgress StartDownload(const std::string& pluginId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        MarketplaceDownloadProgress progress;
        progress.pluginId = pluginId;

        auto it = m_catalog.find(pluginId);
        if (it == m_catalog.end()) {
            progress.status = DownloadStatus::Failed;
            progress.errorMessage = "Plugin not found in catalog";
            return progress;
        }

        progress.totalBytes = it->second.sizeBytes;
        progress.status = DownloadStatus::Downloading;
        m_downloads[pluginId] = progress;
        return progress;
    }

    inline bool VerifyIntegrity(const std::string& pluginId, const std::string& computedHash) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_catalog.find(pluginId);
        if (it == m_catalog.end()) return false;
        return CompareCaseInsensitive(it->second.sha256Hash, computedHash);
    }

    inline MarketplaceDownloadProgress GetDownloadProgress(const std::string& pluginId) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_downloads.find(pluginId);
        return it != m_downloads.end() ? it->second : MarketplaceDownloadProgress{};
    }

    inline std::string CategoryToString(MarketplacePluginCategory cat) const {
        switch (cat) {
        case MarketplacePluginCategory::Decoder:    return "Decoder";
        case MarketplacePluginCategory::Renderer:   return "Renderer";
        case MarketplacePluginCategory::Filter:     return "Filter";
        case MarketplacePluginCategory::Archive:    return "Archive";
        case MarketplacePluginCategory::ThreeD:     return "3D";
        case MarketplacePluginCategory::Scientific: return "Scientific";
        case MarketplacePluginCategory::Utility:    return "Utility";
        case MarketplacePluginCategory::Theme:      return "Theme";
        default:                         return "Unknown";
        }
    }

    inline size_t GetCatalogSize() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_catalog.size();
    }

private:
    PluginMarketplaceClient() = default;

    inline bool ContainsCaseInsensitive(const std::string& haystack, const std::string& needle) const {
        if (needle.size() > haystack.size()) return false;
        auto it = std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(),
            [](char a, char b) {
                return std::tolower(static_cast<unsigned char>(a)) ==
                    std::tolower(static_cast<unsigned char>(b));
            });
        return it != haystack.end();
    }

    inline bool CompareCaseInsensitive(const std::string& a, const std::string& b) const {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (std::tolower(static_cast<unsigned char>(a[i])) !=
                std::tolower(static_cast<unsigned char>(b[i]))) return false;
        }
        return true;
    }

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, MarketplaceClientEntry> m_catalog;
    std::unordered_map<std::string, MarketplaceDownloadProgress> m_downloads;
};

}
} // namespace ExplorerLens::Engine
