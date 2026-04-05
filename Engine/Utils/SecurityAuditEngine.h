// SecurityAuditEngine.h — Internal security audit and vulnerability scanner
// Copyright (c) 2026 ExplorerLens Project
//
// Runs automated security audits against the Engine configuration, plugin manifests,
// and dependency inventory. Checks for known vulnerability patterns, outdated
// dependencies, insecure configurations, and OWASP Top-10 risk categories.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class FindingSeverity : uint8_t
{
    Info     = 0,
    Low      = 1,
    Medium   = 2,
    High     = 3,
    Critical = 4,
};

struct AuditFinding
{
    std::string     id;
    std::string     description;
    FindingSeverity severity    = FindingSeverity::Info;
    std::string     remediation;
    bool            resolved    = false;
};

struct SecurityAuditReport
{
    uint32_t                 totalFindings    = 0;
    uint32_t                 critical         = 0;  // FindingSeverity::Critical count
    uint32_t                 high             = 0;  // FindingSeverity::High count
    uint32_t                 resolved         = 0;
    std::vector<AuditFinding> findings;
    bool                     passed           = false;
};

class SecurityAuditEngine
{
public:
    SecurityAuditEngine();
    ~SecurityAuditEngine();

    SecurityAuditEngine(const SecurityAuditEngine&)            = delete;
    SecurityAuditEngine& operator=(const SecurityAuditEngine&) = delete;

    bool                Initialize();
    void                Shutdown();
    bool                RunAudit();
    bool                AddFinding(const AuditFinding& finding);
    SecurityAuditReport GetReport()    const noexcept;
    bool                Passed()       const noexcept;
    uint32_t            FindingCount() const noexcept { return static_cast<uint32_t>(m_findings.size()); }

    static SecurityAuditEngine& Instance() noexcept;

private:
    std::vector<AuditFinding> m_findings;
    bool                      m_initialized = false;
    static SecurityAuditEngine s_instance;
};

}} // namespace ExplorerLens::Engine
