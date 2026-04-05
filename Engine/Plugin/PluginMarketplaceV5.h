// PluginMarketplaceV5.h — Plugin Marketplace v5 catalog and discovery engine
// Copyright (c) 2026 ExplorerLens Project
//
// Manages plugin discovery, featured listings, ratings, and install orchestration
// for the ExplorerLens Plugin Marketplace v5. Supports curated collections,
// organization-private feeds, and signed plugin bundles.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class PluginTier : uint8_t
{
    Free       = 0,
    Commercial = 1,
    Enterprise = 2,
    Internal   = 3,
};

struct MarketplacePlugin
{
    std::string id;
    std::string name;
    std::string version;
    std::string author;
    PluginTier  tier        = PluginTier::Free;
    float       rating      = 0.0f;
    uint64_t    installCount = 0;
    bool        verified    = false;
};

struct MarketplaceSearchResult
{
    std::vector<MarketplacePlugin> plugins;
    uint32_t                       totalCount = 0;
    bool                           success    = false;
};

class PluginMarketplaceV5
{
public:
    PluginMarketplaceV5();
    ~PluginMarketplaceV5();

    PluginMarketplaceV5(const PluginMarketplaceV5&)            = delete;
    PluginMarketplaceV5& operator=(const PluginMarketplaceV5&) = delete;

    bool                    Initialize(const std::string& feedUrl);
    void                    Shutdown();
    MarketplaceSearchResult Search(const std::string& query, PluginTier tier = PluginTier::Free) const;
    bool                    InstallPlugin(const std::string& pluginId);
    bool                    UninstallPlugin(const std::string& pluginId);
    uint32_t                CatalogSize()  const noexcept { return static_cast<uint32_t>(m_catalog.size()); }
    bool                    IsReady()      const noexcept { return m_initialized; }

    static PluginMarketplaceV5& Instance() noexcept;

private:
    std::vector<MarketplacePlugin> m_catalog;
    bool                           m_initialized = false;
    static PluginMarketplaceV5     s_instance;
};

}} // namespace ExplorerLens::Engine
