// EnterprisePolicyV4.cpp — Enterprise Policy Engine v4
// Copyright (c) 2026 ExplorerLens Project
//
#include "EnterprisePolicyV4.h"
#include <algorithm>

namespace ExplorerLens { namespace Engine {

EnterprisePolicyV4 EnterprisePolicyV4::s_instance;

EnterprisePolicyV4::EnterprisePolicyV4()  = default;
EnterprisePolicyV4::~EnterprisePolicyV4() { Shutdown(); }

EnterprisePolicyV4& EnterprisePolicyV4::Instance() noexcept { return s_instance; }

bool EnterprisePolicyV4::Initialize()
{
    m_policies.clear();
    m_initialized = true;
    return true;
}

void EnterprisePolicyV4::Shutdown()
{
    m_policies.clear();
    m_initialized = false;
}

bool EnterprisePolicyV4::LoadFromGPO()
{
    // On Windows, would read HKLM\\SOFTWARE\\Policies\\ExplorerLens via RegOpenKeyEx.
    return m_initialized;
}

bool EnterprisePolicyV4::LoadFromIntune()
{
    // On Windows, would read Intune-delivered MDM policy via WMI or GPAPI.
    return m_initialized;
}

bool EnterprisePolicyV4::ApplyPolicy(const EnterprisePolicyEntry& entry)
{
    if (!m_initialized || entry.key.empty())
        return false;
    m_policies.push_back(entry);
    return true;
}

std::string EnterprisePolicyV4::GetPolicy(const std::string& key) const
{
    for (const auto& p : m_policies)
    {
        if (p.key == key)
            return p.value;
    }
    return {};
}

PolicyReport EnterprisePolicyV4::GenerateReport() const
{
    PolicyReport report;
    report.totalPolicies = static_cast<uint32_t>(m_policies.size());
    for (const auto& p : m_policies)
    {
        if (p.enforcementLevel >= PolicyEnforcementLevel::Enforced)
            ++report.enforced;
    }
    report.compliant = (report.violations == 0);
    return report;
}

bool EnterprisePolicyV4::IsCompliant() const noexcept
{
    return GenerateReport().compliant;
}

}} // namespace ExplorerLens::Engine
