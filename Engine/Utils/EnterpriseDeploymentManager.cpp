//==============================================================================
// EnterpriseDeploymentManager — Sprint 223
//==============================================================================

#include "EnterpriseDeploymentManager.h"
#include <chrono>
#include <sstream>

namespace DarkThumbs { namespace Engine {

EnterpriseDeploymentManager::EnterpriseDeploymentManager() {}

void EnterpriseDeploymentManager::AddPolicy(const DeploymentPolicy& policy) {
    m_policies.push_back(policy);
}

DeploymentReport EnterpriseDeploymentManager::Deploy(DeploymentMethod method) {
    DeploymentReport report;
    auto start = std::chrono::high_resolution_clock::now();

    report.method = method;

    for (const auto& policy : m_policies) {
        // In production: apply via GPO/registry/MDM API
        if (!policy.name.empty() && !policy.key.empty()) {
            report.policiesApplied++;
        } else {
            report.policiesFailed++;
            report.errors.push_back(L"Invalid policy: " + policy.name);
        }
    }

    report.success = (report.policiesFailed == 0);

    auto end = std::chrono::high_resolution_clock::now();
    report.deployTimeMs = std::chrono::duration<double, std::milli>(end - start).count();
    return report;
}

std::map<std::wstring, std::wstring> EnterpriseDeploymentManager::GenerateMSIProperties() const {
    std::map<std::wstring, std::wstring> props;
    props[L"INSTALLLEVEL"] = L"100";
    props[L"REBOOT"] = L"ReallySuppress";
    props[L"ALLUSERS"] = L"1";

    for (const auto& policy : m_policies) {
        if (!policy.key.empty()) {
            props[policy.key] = policy.value;
        }
    }
    return props;
}

std::wstring EnterpriseDeploymentManager::GenerateGPOTemplate() const {
    std::wstringstream ss;
    ss << L"CLASS MACHINE\n";
    ss << L"CATEGORY \"DarkThumbs\"\n";
    for (const auto& policy : m_policies) {
        if (policy.type == PolicyType::MachinePol || policy.type == PolicyType::RegistryPol) {
            ss << L"  POLICY \"" << policy.name << L"\"\n";
            ss << L"    KEYNAME \"Software\\DarkThumbs\"\n";
            ss << L"    VALUENAME \"" << policy.key << L"\"\n";
            ss << L"    VALUEON \"" << policy.value << L"\"\n";
            ss << L"  END POLICY\n";
        }
    }
    ss << L"END CATEGORY\n";
    return ss.str();
}

std::wstring EnterpriseDeploymentManager::GenerateIntuneConfig() const {
    std::wstringstream ss;
    ss << L"{\n  \"name\": \"DarkThumbs Configuration\",\n";
    ss << L"  \"settings\": [\n";
    bool first = true;
    for (const auto& policy : m_policies) {
        if (!first) ss << L",\n";
        ss << L"    {\"key\": \"" << policy.key << L"\", \"value\": \"" << policy.value << L"\"}";
        first = false;
    }
    ss << L"\n  ]\n}\n";
    return ss.str();
}

bool EnterpriseDeploymentManager::ValidatePolicies() const {
    for (const auto& policy : m_policies) {
        if (policy.name.empty() || policy.key.empty()) return false;
    }
    return !m_policies.empty();
}

const wchar_t* EnterpriseDeploymentManager::GetMethodName(DeploymentMethod method) {
    switch (method) {
        case DeploymentMethod::Manual: return L"Manual";
        case DeploymentMethod::GPO:    return L"Group Policy";
        case DeploymentMethod::SCCM:   return L"SCCM";
        case DeploymentMethod::Intune: return L"Intune";
        case DeploymentMethod::WSUS:   return L"WSUS";
        case DeploymentMethod::Custom: return L"Custom";
        default: return L"Unknown";
    }
}

const wchar_t* EnterpriseDeploymentManager::GetPolicyTypeName(PolicyType type) {
    switch (type) {
        case PolicyType::MachinePol:  return L"Machine Policy";
        case PolicyType::UserPol:     return L"User Policy";
        case PolicyType::RegistryPol: return L"Registry Policy";
        case PolicyType::FilePol:     return L"File Policy";
        default: return L"Unknown";
    }
}

}} // namespace DarkThumbs::Engine
