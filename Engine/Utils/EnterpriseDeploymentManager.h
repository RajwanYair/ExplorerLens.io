#pragma once
//==============================================================================
// EnterpriseDeploymentManager
// Enterprise deployment configuration for GPO, SCCM, Intune, and manual.
// Handles policy-driven settings, silent install, and compliance reporting.
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>
#include <map>

namespace ExplorerLens { namespace Engine {

enum class DeploymentMethod : uint8_t {
 Manual = 0,
 GPO = 1, // Group Policy Object
 SCCM = 2, // System Center Configuration Manager
 Intune = 3, // Microsoft Intune
 WSUS = 4, // Windows Server Update Services
 Custom = 5,
 MethodCount = 6
};

enum class PolicyType : uint8_t {
 MachinePol = 0,
 UserPol = 1,
 RegistryPol = 2,
 FilePol = 3
};

struct DeploymentPolicy {
 std::wstring name;
 PolicyType type = PolicyType::MachinePol;
 std::wstring key;
 std::wstring value;
 bool enforced = false;
};

struct DeploymentReport {
 bool success = false;
 DeploymentMethod method = DeploymentMethod::Manual;
 uint32_t policiesApplied = 0;
 uint32_t policiesFailed = 0;
 uint32_t machinesTargeted = 0;
 double deployTimeMs = 0.0;
 std::vector<std::wstring> errors;
};

class EnterpriseDeploymentManager {
public:
 EnterpriseDeploymentManager();

 void AddPolicy(const DeploymentPolicy& policy);
 std::vector<DeploymentPolicy> GetPolicies() const { return m_policies; }
 DeploymentReport Deploy(DeploymentMethod method);

 std::map<std::wstring, std::wstring> GenerateMSIProperties() const;
 std::wstring GenerateGPOTemplate() const;
 std::wstring GenerateIntuneConfig() const;

 bool ValidatePolicies() const;

 static const wchar_t* GetMethodName(DeploymentMethod method);
 static const wchar_t* GetPolicyTypeName(PolicyType type);
 static uint32_t GetMethodCount() { return static_cast<uint32_t>(DeploymentMethod::MethodCount); }

private:
 std::vector<DeploymentPolicy> m_policies;
};

}} // namespace ExplorerLens::Engine

