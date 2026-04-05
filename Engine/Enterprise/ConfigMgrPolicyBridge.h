// ConfigMgrPolicyBridge.h — Microsoft ConfigMgr policy synchronization bridge
// Copyright (c) 2026 ExplorerLens Project
//
// Bridges ExplorerLens configuration policy with Microsoft Configuration
// Manager (SCCM/ConfigMgr) compliance baselines. Reads policy from ConfigMgr
// WMI namespace root\ccm\policy and applies to Engine settings.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

struct ConfigMgrBaseline
{
    std::string id;
    std::string name;
    std::string version;
    bool        assigned = false;
};

struct ConfigMgrSyncResult
{
    bool        success       = false;
    uint32_t    baselinesRead = 0;
    uint32_t    policiesApplied = 0;
    std::string lastError;
};

class ConfigMgrPolicyBridge
{
public:
    ConfigMgrPolicyBridge();
    ~ConfigMgrPolicyBridge();

    ConfigMgrPolicyBridge(const ConfigMgrPolicyBridge&)            = delete;
    ConfigMgrPolicyBridge& operator=(const ConfigMgrPolicyBridge&) = delete;

    bool                Initialize();
    void                Shutdown();
    ConfigMgrSyncResult Synchronize();
    bool                IsConfigMgrPresent() const noexcept;
    uint32_t            BaselineCount()      const noexcept { return static_cast<uint32_t>(m_baselines.size()); }

    static ConfigMgrPolicyBridge& Instance() noexcept;

private:
    std::vector<ConfigMgrBaseline> m_baselines;
    bool                           m_initialized = false;
    static ConfigMgrPolicyBridge   s_instance;
};

}} // namespace ExplorerLens::Engine
