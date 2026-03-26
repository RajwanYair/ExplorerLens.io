// MarketplaceClient.h — Plugin Registry Query and Download Client
// Copyright (c) 2026 ExplorerLens Project
//
// Queries the ExplorerLens Plugin Registry REST API to list, search, and
// download plugin packages (.lenspkg files) from the public marketplace.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace ExplorerLens {
namespace Engine {

// ---- Plugin Listing ---------------------------------------------------------

struct MarketplacePlugin {
    std::string  id;              // e.g. "com.example.myformatdecoder"
    std::string  displayName;
    std::string  description;
    std::string  version;         // SemVer e.g. "1.2.3"
    std::string  author;
    std::string  authorUrl;
    std::string  iconUrl;
    std::string  downloadUrl;
    uint64_t     sizeBytes    = 0;
    uint32_t     downloadCount = 0;
    float        rating       = 0.0f;  // 0.0 - 5.0
    std::string  minEngineVersion; // Minimum ExplorerLens engine version
    std::vector<std::string> supportedFormats;
    std::string  publishedDate;
    std::string  sha256;          // Expected SHA-256 of the .lenspkg
};

// ---- Query ------------------------------------------------------------------

struct MarketplaceQuery {
    std::string  searchText;
    std::string  format;          // Filter by file extension (e.g. ".xyz")
    uint32_t     pageIndex   = 0;
    uint32_t     pageSize    = 50;
    bool         sortByRating = false;
    bool         sortByDownloads = false;
};

struct MarketplaceQueryResult {
    bool   success      = false;
    uint32_t totalItems = 0;
    std::vector<MarketplacePlugin> plugins;
    std::string error;
};

// ---- Download ---------------------------------------------------------------

using DownloadProgressCallback = std::function<void(uint64_t received, uint64_t total)>;

struct DownloadResult {
    bool    success  = false;
    std::string localPath;        // Where the downloaded .lenspkg was saved
    std::string error;
    bool    sha256Verified = false;
};

// ---- MarketplaceClient ------------------------------------------------------

class MarketplaceClient {
public:
    explicit MarketplaceClient(
        std::string registryBaseUrl = "https://plugins.explorerlens.io/api/v1");
    ~MarketplaceClient();

    // List or search plugins from the registry.
    MarketplaceQueryResult Query(const MarketplaceQuery& query) const;

    // Fetch detailed info for a single plugin by ID.
    bool GetPluginInfo(const std::string& pluginId, MarketplacePlugin& outPlugin) const;

    // Download a .lenspkg to the given local directory.
    DownloadResult Download(
        const MarketplacePlugin&    plugin,
        const std::string&          destDir,
        DownloadProgressCallback    progress = nullptr) const;

    // Check if a newer version of an installed plugin is available.
    bool CheckForUpdate(
        const std::string& pluginId,
        const std::string& currentVersion,
        MarketplacePlugin& outLatest) const;

    void SetApiKey(const std::string& apiKey);  // Optional — for private registries
    void SetRegistryUrl(const std::string& url);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace Engine
} // namespace ExplorerLens
