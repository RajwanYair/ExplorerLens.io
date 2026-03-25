// DoctorCommand.h — lens doctor: System Health Check & Diagnostic Report
// Copyright (c) 2026 ExplorerLens Project
//
// Sprint 22 (v15.4.0 "Zenith-U"): Runs a comprehensive health check that
// validates GPU availability, COM registration status, decoder DLL presence,
// cache directory accessibility, and required Windows features.
// Outputs a pass/fail table with remediation hints for any failures.
//
#pragma once
#include "CommandRouter.h"
#include <vector>

namespace ExplorerLens {
namespace CLI {

enum class CheckStatus { Pass, Warn, Fail };

struct DiagnosticCheck {
    std::wstring  name;
    CheckStatus   status    = CheckStatus::Pass;
    std::wstring  detail;
    std::wstring  fix;
};

class DoctorCommand final : public ISubCommand {
public:
    int Execute(const ParsedArgs& args) override;
    std::wstring_view Name()      const noexcept override { return L"doctor"; }
    std::wstring_view ShortDesc() const noexcept override {
        return L"Run system health checks for ExplorerLens";
    }
    std::wstring_view Usage() const noexcept override {
        return L"lens doctor [--json] [--verbose]";
    }

private:
    std::vector<DiagnosticCheck> RunChecks();

    DiagnosticCheck CheckRegistration();
    DiagnosticCheck CheckGPUAvailability();
    DiagnosticCheck CheckCacheDirectory();
    DiagnosticCheck CheckWindowsVersion();
    DiagnosticCheck CheckDllPresence();
    DiagnosticCheck CheckThumbnailServiceEnabled();

    void PrintTextReport(const std::vector<DiagnosticCheck>& checks) const;
    void PrintJsonReport(const std::vector<DiagnosticCheck>& checks) const;
};

} // namespace CLI
} // namespace ExplorerLens
