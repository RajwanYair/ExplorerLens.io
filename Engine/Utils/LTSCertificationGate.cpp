// LTSCertificationGate.cpp — LTS release certification gate
// Copyright (c) 2026 ExplorerLens Project
//
#include "LTSCertificationGate.h"

namespace ExplorerLens { namespace Engine {

LTSCertificationGate LTSCertificationGate::s_instance;

LTSCertificationGate::LTSCertificationGate()  = default;
LTSCertificationGate::~LTSCertificationGate() { Reset(); }

LTSCertificationGate& LTSCertificationGate::Instance() noexcept { return s_instance; }

bool LTSCertificationGate::Initialize(const std::string& buildVersion)
{
    if (buildVersion.empty())
        return false;
    m_gates.clear();
    m_buildVersion = buildVersion;
    return true;
}

void LTSCertificationGate::Reset()
{
    m_gates.clear();
    m_buildVersion.clear();
}

bool LTSCertificationGate::RunGate(const std::string& gateName, bool pass,
                                    const std::string& detail)
{
    if (gateName.empty())
        return false;
    CertGate gate;
    gate.name   = gateName;
    gate.result = pass ? CertGateResult::Pass : CertGateResult::Fail;
    gate.detail = detail;
    gate.blocking = true;
    m_gates.push_back(gate);
    return true;
}

CertificationSummary LTSCertificationGate::GetSummary() const noexcept
{
    CertificationSummary s;
    s.totalGates = static_cast<uint32_t>(m_gates.size());
    s.certLevel  = "LTS-" + m_buildVersion;
    for (const auto& g : m_gates)
    {
        if (g.result == CertGateResult::Pass)       ++s.passed;
        else if (g.result == CertGateResult::Fail)  ++s.failed;
    }
    s.certified = (s.failed == 0 && s.passed > 0);
    return s;
}

bool LTSCertificationGate::IsCertified() const noexcept
{
    return GetSummary().certified;
}

}} // namespace ExplorerLens::Engine
