// LTSHardeningController.cpp — LTS hardening gate controller
// Copyright (c) 2026 ExplorerLens Project
//
#include "LTSHardeningController.h"

namespace ExplorerLens { namespace Engine {

LTSHardeningController LTSHardeningController::s_instance;

LTSHardeningController::LTSHardeningController()  = default;
LTSHardeningController::~LTSHardeningController() { Shutdown(); }

LTSHardeningController& LTSHardeningController::Instance() noexcept { return s_instance; }

bool LTSHardeningController::Initialize(const std::string& versionLabel)
{
    if (versionLabel.empty())
        return false;
    m_gates.clear();
    m_versionLabel = versionLabel;
    return true;
}

void LTSHardeningController::Shutdown()
{
    m_gates.clear();
    m_versionLabel.clear();
}

bool LTSHardeningController::AddGate(const LTSGate& gate)
{
    if (gate.name.empty())
        return false;
    m_gates.push_back(gate);
    return true;
}

bool LTSHardeningController::EvaluateGate(const std::string& gateName, bool pass)
{
    for (auto& g : m_gates)
    {
        if (g.name == gateName)
        {
            g.status = pass ? LTSGateStatus::Passing : LTSGateStatus::Failing;
            return true;
        }
    }
    return false;
}

LTSHardeningReport LTSHardeningController::GenerateReport() const
{
    LTSHardeningReport report;
    report.totalGates    = static_cast<uint32_t>(m_gates.size());
    report.certification = "LTS-" + m_versionLabel;
    for (const auto& g : m_gates)
    {
        if (g.status == LTSGateStatus::Passing)
            ++report.passing;
        else if (g.status == LTSGateStatus::Failing || g.status == LTSGateStatus::Blocked)
            ++report.failing;
    }
    report.ltsReady = (report.failing == 0 && report.totalGates > 0);
    return report;
}

bool LTSHardeningController::IsLTSReady() const noexcept
{
    return GenerateReport().ltsReady;
}

}} // namespace ExplorerLens::Engine
