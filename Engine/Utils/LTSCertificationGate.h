// LTSCertificationGate.h — LTS release certification gate
// Copyright (c) 2026 ExplorerLens Project
//
// Enforces a multi-stage certification gate before a build can be promoted to
// LTS status. Gates include: security audit pass, dependency freeze verification,
// performance regression clear, API stability confirmation, and SBOM completeness.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class CertGateResult : uint8_t
{
    NotRun  = 0,
    Pass    = 1,
    Fail    = 2,
    Skipped = 3,
};

struct CertGate
{
    std::string    name;
    CertGateResult result   = CertGateResult::NotRun;
    bool           blocking = true;
    std::string    detail;
};

struct CertificationSummary
{
    bool        certified      = false;
    uint32_t    totalGates     = 0;
    uint32_t    passed         = 0;
    uint32_t    failed         = 0;
    std::string certLevel;
};

class LTSCertificationGate
{
public:
    LTSCertificationGate();
    ~LTSCertificationGate();

    LTSCertificationGate(const LTSCertificationGate&)            = delete;
    LTSCertificationGate& operator=(const LTSCertificationGate&) = delete;

    bool                 Initialize(const std::string& buildVersion);
    void                 Reset();
    bool                 RunGate(const std::string& gateName, bool pass,
                                  const std::string& detail = {});
    CertificationSummary GetSummary() const noexcept;
    bool                 IsCertified() const noexcept;
    uint32_t             GateCount()   const noexcept { return static_cast<uint32_t>(m_gates.size()); }

    static LTSCertificationGate& Instance() noexcept;

private:
    std::vector<CertGate> m_gates;
    std::string           m_buildVersion;
    static LTSCertificationGate s_instance;
};

}} // namespace ExplorerLens::Engine
