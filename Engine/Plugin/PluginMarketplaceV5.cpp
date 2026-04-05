// PluginMarketplaceV5.cpp — Plugin Marketplace v5 catalog and discovery engine
// Copyright (c) 2026 ExplorerLens Project
//
#include "PluginMarketplaceV5.h"
#include <algorithm>

namespace ExplorerLens { namespace Engine {

PluginMarketplaceV5 PluginMarketplaceV5::s_instance;

PluginMarketplaceV5::PluginMarketplaceV5()  = default;
PluginMarketplaceV5::~PluginMarketplaceV5() { Shutdown(); }

PluginMarketplaceV5& PluginMarketplaceV5::Instance() noexcept { return s_instance; }

bool PluginMarketplaceV5::Initialize(const std::string& feedUrl)
{
    if (feedUrl.empty())
        return false;
    m_catalog.clear();
    m_initialized = true;
    return true;
}

void PluginMarketplaceV5::Shutdown()
{
    m_catalog.clear();
    m_initialized = false;
}

MarketplaceSearchResult PluginMarketplaceV5::Search(const std::string& query,
                                                     PluginTier /*tier*/) const
{
    MarketplaceSearchResult result;
    if (!m_initialized)
        return result;
    for (const auto& p : m_catalog)
    {
        if (query.empty() || p.name.find(query) != std::string::npos)
            result.plugins.push_back(p);
    }
    result.totalCount = static_cast<uint32_t>(result.plugins.size());
    result.success    = true;
    return result;
}

bool PluginMarketplaceV5::InstallPlugin(const std::string& pluginId)
{
    return m_initialized && !pluginId.empty();
}

bool PluginMarketplaceV5::UninstallPlugin(const std::string& pluginId)
{
    return m_initialized && !pluginId.empty();
}

}} // namespace ExplorerLens::Engine
