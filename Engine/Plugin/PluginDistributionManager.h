// PluginDistributionManager.h — Plugin install, update, and rollback manager
// Copyright (c) 2026 ExplorerLens Project
//
// Manages the full lifecycle of distributed plugin bundles: download, verify,
// install, version tracking, delta updates, and rollback on failure. Integrates
// with PluginSignatureValidator to enforce signing requirements before any plugin
// bytes are written to disk.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class DistributionOp : uint8_t
{
    Install  = 0,
    Update   = 1,
    Rollback = 2,
    Remove   = 3,
};

enum class DistributionStatus : uint8_t
{
    Idle       = 0,
    InProgress = 1,
    Success    = 2,
    Failed     = 3,
};

struct PluginDistributionRecord
{
    std::string        pluginId;
    std::string        currentVersion;
    std::string        previousVersion;
    DistributionStatus status = DistributionStatus::Idle;
};

struct DistributionManagerStats
{
    uint32_t  installCount  = 0;
    uint32_t  updateCount   = 0;
    uint32_t  rollbackCount = 0;
    uint32_t  failureCount  = 0;
};

class PluginDistributionManager
{
public:
    PluginDistributionManager();
    ~PluginDistributionManager();

    PluginDistributionManager(const PluginDistributionManager&)            = delete;
    PluginDistributionManager& operator=(const PluginDistributionManager&) = delete;

    bool                  Initialize(const std::string& installRoot);
    void                  Shutdown();
    bool                  Install(const std::string& pluginId,
                                   const std::string& packagePath);
    bool                  Update(const std::string& pluginId,
                                  const std::string& newPackagePath);
    bool                  Rollback(const std::string& pluginId);
    bool                  Remove(const std::string& pluginId);
    DistributionManagerStats GetStats() const noexcept { return m_stats; }
    uint32_t              InstalledCount() const noexcept { return static_cast<uint32_t>(m_records.size()); }

    static PluginDistributionManager& Instance() noexcept;

private:
    std::vector<PluginDistributionRecord> m_records;
    DistributionManagerStats              m_stats;
    std::string                           m_installRoot;
    bool                                  m_initialized = false;
    static PluginDistributionManager      s_instance;
};

}} // namespace ExplorerLens::Engine
