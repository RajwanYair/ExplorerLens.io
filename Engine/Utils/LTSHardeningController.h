// LTSHardeningController.h — LTS (Long-Term Support) hardening gate controller
// Copyright (c) 2026 ExplorerLens Project
//
// Controls feature gating, dependency freezing, and hardening policies for
// Long-Term Support releases. Enforces conservative code paths, disables
// experimental features, and manages the LTS certification checklist.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class LTSGateStatus : uint8_t
{
    Open     = 0,
    Passing  = 1,
    Failing  = 2,
    Blocked  = 3,
};

struct LTSGate
{
    std::string   name;
    std::string   description;
    LTSGateStatus status  = LTSGateStatus::Open;
    bool          required = true;
};

struct LTSHardeningReport
{
    uint32_t  totalGates   = 0;
    uint32_t  passing      = 0;
    uint32_t  failing      = 0;
    bool      ltsReady     = false;
    std::string certification;
};

class LTSHardeningController
{
public:
    LTSHardeningController();
    ~LTSHardeningController();

    LTSHardeningController(const LTSHardeningController&)            = delete;
    LTSHardeningController& operator=(const LTSHardeningController&) = delete;

    bool                Initialize(const std::string& versionLabel);
    void                Shutdown();
    bool                AddGate(const LTSGate& gate);
    bool                EvaluateGate(const std::string& gateName, bool pass);
    LTSHardeningReport  GenerateReport() const;
    bool                IsLTSReady()     const noexcept;
    uint32_t            GateCount()      const noexcept { return static_cast<uint32_t>(m_gates.size()); }

    static LTSHardeningController& Instance() noexcept;

private:
    std::vector<LTSGate>  m_gates;
    std::string           m_versionLabel;
    static LTSHardeningController s_instance;
};

}} // namespace ExplorerLens::Engine
