// IntuneComplianceEngine.cpp — Microsoft Intune compliance reporting
// Copyright (c) 2026 ExplorerLens Project
//
#include "IntuneComplianceEngine.h"

namespace ExplorerLens { namespace Engine {

IntuneComplianceEngine IntuneComplianceEngine::s_instance;

IntuneComplianceEngine::IntuneComplianceEngine()  = default;
IntuneComplianceEngine::~IntuneComplianceEngine() { Shutdown(); }

IntuneComplianceEngine& IntuneComplianceEngine::Instance() noexcept { return s_instance; }

bool IntuneComplianceEngine::Initialize()
{
    m_rules.clear();
    m_initialized = true;
    return true;
}

void IntuneComplianceEngine::Shutdown()
{
    m_rules.clear();
    m_initialized = false;
}

bool IntuneComplianceEngine::AddRule(const ComplianceRule& rule)
{
    if (rule.ruleId.empty())
        return false;
    m_rules.push_back(rule);
    return true;
}

IntuneComplianceReport IntuneComplianceEngine::Evaluate() const
{
    IntuneComplianceReport report;
    report.rules      = m_rules;
    report.totalRules = static_cast<uint32_t>(m_rules.size());
    for (auto& r : report.rules)
    {
        r.status = ComplianceStatus::Compliant;
        ++report.passing;
    }
    report.overallStatus = (report.failing == 0) ? ComplianceStatus::Compliant
                                                  : ComplianceStatus::NonCompliant;
    return report;
}

bool IntuneComplianceEngine::ReportToWMI() const
{
    // On Windows, would use WMI to write to MDM_Reporting namespace.
    return m_initialized;
}

}} // namespace ExplorerLens::Engine
