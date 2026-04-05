// IntuneComplianceEngine.h — Microsoft Intune compliance reporting
// Copyright (c) 2026 ExplorerLens Project
//
// Reports ExplorerLens configuration compliance status to Microsoft Intune via
// the Windows Management Instrumentation (WMI) bridge. Generates compliance
// reports for MDM enrollment check-ins and remediation workflows.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class ComplianceStatus : uint8_t
{
    Unknown    = 0,
    Compliant  = 1,
    NonCompliant = 2,
    Error      = 3,
};

struct ComplianceRule
{
    std::string      ruleId;
    std::string      description;
    bool             required     = true;
    ComplianceStatus status       = ComplianceStatus::Unknown;
    std::string      remediation;
};

struct IntuneComplianceReport
{
    ComplianceStatus      overallStatus = ComplianceStatus::Unknown;
    uint32_t              totalRules    = 0;
    uint32_t              passing       = 0;
    uint32_t              failing       = 0;
    std::vector<ComplianceRule> rules;
};

class IntuneComplianceEngine
{
public:
    IntuneComplianceEngine();
    ~IntuneComplianceEngine();

    IntuneComplianceEngine(const IntuneComplianceEngine&)            = delete;
    IntuneComplianceEngine& operator=(const IntuneComplianceEngine&) = delete;

    bool                   Initialize();
    void                   Shutdown();
    bool                   AddRule(const ComplianceRule& rule);
    IntuneComplianceReport Evaluate() const;
    bool                   ReportToWMI() const;
    uint32_t               RuleCount()  const noexcept { return static_cast<uint32_t>(m_rules.size()); }

    static IntuneComplianceEngine& Instance() noexcept;

private:
    std::vector<ComplianceRule>  m_rules;
    bool                         m_initialized = false;
    static IntuneComplianceEngine s_instance;
};

}} // namespace ExplorerLens::Engine
