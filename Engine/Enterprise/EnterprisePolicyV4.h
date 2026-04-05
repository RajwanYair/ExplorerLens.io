// EnterprisePolicyV4.h — Enterprise Policy Engine v4
// Copyright (c) 2026 ExplorerLens Project
//
// Enhanced enterprise policy engine supporting GPO (Group Policy Objects),
// Intune Configuration Profiles, ConfigMgr baselines, and manual overrides.
// Policy source hierarchy: GPO > Intune > ConfigMgr > Manual.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class PolicySourceV4 : uint8_t
{
    None     = 0,
    GPO      = 1,
    Intune   = 2,
    ConfigMgr = 3,
    Manual   = 4,
};

enum class PolicyEnforcementLevel : uint8_t
{
    Advisory  = 0,
    Warning   = 1,
    Enforced  = 2,
    Locked    = 3,
};

struct EnterprisePolicyV4Entry
{
    std::string            key;
    std::string            value;
    PolicySourceV4           source           = PolicySourceV4::None;
    PolicyEnforcementLevel enforcementLevel = PolicyEnforcementLevel::Advisory;
};

struct PolicyReport
{
    uint32_t totalPolicies   = 0;
    uint32_t enforced        = 0;
    uint32_t violations      = 0;
    bool     compliant       = false;
};

class EnterprisePolicyV4
{
public:
    EnterprisePolicyV4();
    ~EnterprisePolicyV4();

    EnterprisePolicyV4(const EnterprisePolicyV4&)            = delete;
    EnterprisePolicyV4& operator=(const EnterprisePolicyV4&) = delete;

    bool         Initialize();
    void         Shutdown();
    bool         LoadFromGPO();
    bool         LoadFromIntune();
    bool         ApplyPolicy(const EnterprisePolicyV4Entry& entry);
    std::string  GetPolicy(const std::string& key) const;
    PolicyReport GenerateReport() const;
    uint32_t     PolicyCount()  const noexcept { return static_cast<uint32_t>(m_policies.size()); }
    bool         IsCompliant()  const noexcept;

    static EnterprisePolicyV4& Instance() noexcept;

private:
    std::vector<EnterprisePolicyV4Entry>  m_policies;
    bool                      m_initialized = false;
    static EnterprisePolicyV4 s_instance;
};

}} // namespace ExplorerLens::Engine
