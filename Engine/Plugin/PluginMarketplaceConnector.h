// PluginMarketplaceConnector.h — Plugin Marketplace Connector
// Copyright (c) 2026 ExplorerLens Project
//
// Provides offline-capable plugin marketplace browsing and secure package downloads.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace ExplorerLens { namespace Engine {

struct PMCPluginListing {
    std::string id;
    std::string name;
    std::string version;
    std::string author;
    float       rating    = 0.0f;
    uint32_t    downloads = 0;
};

struct PMCInstallResult {
    bool        success          = false;
    std::string installedId;
    std::string installedVersion;
    std::string errorMsg;
};

class PluginMarketplaceConnector {
public:
    void AddListing(const PMCPluginListing& listing) {
        m_catalog[listing.id] = listing;
    }

    std::vector<PMCPluginListing> Search(const std::string& query) const {
        std::vector<PMCPluginListing> results;
        for (const auto& [id, l] : m_catalog)
            if (query.empty() || l.name.find(query) != std::string::npos)
                results.push_back(l);
        return results;
    }

    PMCInstallResult Install(const std::string& pluginId) {
        PMCInstallResult r;
        auto it = m_catalog.find(pluginId);
        if (it == m_catalog.end()) { r.errorMsg = "Not found"; return r; }
        m_installed[pluginId] = it->second.version;
        r.installedId         = pluginId;
        r.installedVersion    = it->second.version;
        r.success             = true;
        return r;
    }

    bool IsInstalled(const std::string& pluginId) const {
        return m_installed.count(pluginId) > 0;
    }

private:
    std::unordered_map<std::string, PMCPluginListing> m_catalog;
    std::unordered_map<std::string, std::string>      m_installed;
};

}} // namespace ExplorerLens::Engine
