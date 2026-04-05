// SecurityAuditEngine.cpp — Internal security audit and vulnerability scanner
// Copyright (c) 2026 ExplorerLens Project
//
#include "SecurityAuditEngine.h"

namespace ExplorerLens { namespace Engine {

SecurityAuditEngine SecurityAuditEngine::s_instance;

SecurityAuditEngine::SecurityAuditEngine()  = default;
SecurityAuditEngine::~SecurityAuditEngine() { Shutdown(); }

SecurityAuditEngine& SecurityAuditEngine::Instance() noexcept { return s_instance; }

bool SecurityAuditEngine::Initialize()
{
    m_findings.clear();
    m_initialized = true;
    return true;
}

void SecurityAuditEngine::Shutdown()
{
    m_findings.clear();
    m_initialized = false;
}

bool SecurityAuditEngine::RunAudit()
{
    if (!m_initialized)
        return false;
    // In production: scan plugin manifests, check dependency CVEs, validate config.
    return true;
}

bool SecurityAuditEngine::AddFinding(const AuditFinding& finding)
{
    if (finding.id.empty())
        return false;
    m_findings.push_back(finding);
    return true;
}

SecurityAuditReport SecurityAuditEngine::GetReport() const noexcept
{
    SecurityAuditReport report;
    report.findings      = m_findings;
    report.totalFindings = static_cast<uint32_t>(m_findings.size());
    for (const auto& f : m_findings)
    {
        if (f.severity == FindingSeverity::Critical) ++report.critical;
        if (f.severity == FindingSeverity::High)     ++report.high;
        if (f.resolved)                             ++report.resolved;
    }
    report.passed = (report.critical == 0 && report.high == 0);
    return report;
}

bool SecurityAuditEngine::Passed() const noexcept
{
    return GetReport().passed;
}

}} // namespace ExplorerLens::Engine
