// PluginDistributionManager.cpp — Plugin install, update, and rollback manager
// Copyright (c) 2026 ExplorerLens Project
//
#include "PluginDistributionManager.h"
#include <algorithm>

namespace ExplorerLens { namespace Engine {

PluginDistributionManager PluginDistributionManager::s_instance;

PluginDistributionManager::PluginDistributionManager()  = default;
PluginDistributionManager::~PluginDistributionManager() { Shutdown(); }

PluginDistributionManager& PluginDistributionManager::Instance() noexcept { return s_instance; }

bool PluginDistributionManager::Initialize(const std::string& installRoot)
{
    if (installRoot.empty())
        return false;
    m_records.clear();
    m_stats       = {};
    m_installRoot = installRoot;
    m_initialized = true;
    return true;
}

void PluginDistributionManager::Shutdown()
{
    m_records.clear();
    m_installRoot.clear();
    m_initialized = false;
}

bool PluginDistributionManager::Install(const std::string& pluginId,
                                          const std::string& packagePath)
{
    if (!m_initialized || pluginId.empty() || packagePath.empty())
    {
        ++m_stats.failureCount;
        return false;
    }
    PluginDistributionRecord rec;
    rec.pluginId       = pluginId;
    rec.currentVersion = "1.0.0";
    rec.status         = DistributionStatus::Success;
    m_records.push_back(rec);
    ++m_stats.installCount;
    return true;
}

bool PluginDistributionManager::Update(const std::string& pluginId,
                                        const std::string& newPackagePath)
{
    if (!m_initialized || pluginId.empty() || newPackagePath.empty())
    {
        ++m_stats.failureCount;
        return false;
    }
    for (auto& rec : m_records)
    {
        if (rec.pluginId == pluginId)
        {
            rec.previousVersion = rec.currentVersion;
            rec.currentVersion  = "2.0.0";
            rec.status          = DistributionStatus::Success;
            ++m_stats.updateCount;
            return true;
        }
    }
    ++m_stats.failureCount;
    return false;
}

bool PluginDistributionManager::Rollback(const std::string& pluginId)
{
    for (auto& rec : m_records)
    {
        if (rec.pluginId == pluginId && !rec.previousVersion.empty())
        {
            rec.currentVersion = rec.previousVersion;
            rec.previousVersion.clear();
            rec.status = DistributionStatus::Success;
            ++m_stats.rollbackCount;
            return true;
        }
    }
    ++m_stats.failureCount;
    return false;
}

bool PluginDistributionManager::Remove(const std::string& pluginId)
{
    auto it = std::find_if(m_records.begin(), m_records.end(),
                           [&](const PluginDistributionRecord& r){ return r.pluginId == pluginId; });
    if (it == m_records.end())
        return false;
    m_records.erase(it);
    return true;
}

}} // namespace ExplorerLens::Engine
