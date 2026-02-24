#pragma once
// Plugin Sandbox Policy Hardening
// Job Object limits, timeout kill chain, memory quota, and handle leak detection.

#include <string>
#include <vector>
#include <cstdint>
#include <optional>

namespace ExplorerLens::Plugin {

// ─── Resource limits ─────────────────────────────────────────────────────────

struct JobObjectLimits {
    uint64_t    maxMemoryBytes          { 256ULL * 1024 * 1024 };   // 256 MB
    uint32_t    maxCPUPercent           { 25 };                      // % of one core
    uint32_t    maxHandles              { 256 };
    uint32_t    maxProcesses            { 4 };
    uint32_t    maxThreadsPerProcess    { 32 };
    bool        killOnJobClose          { true };
    bool        dieOnUnhandledException { true };
    bool        allowUIAccess           { false };

    static constexpr uint64_t kDefaultMemoryBudget = 256ULL * 1024 * 1024;
};

// ─── Policy presets ───────────────────────────────────────────────────────────

enum class SandboxPolicyPreset : uint32_t {
    Strict      = 0,   // tightest limits, for untrusted plugins
    Standard    = 1,   // production defaults
    Developer   = 2,   // relaxed for debug/dev builds
    Disabled    = 3,   // no sandbox (dangerous — testing only)
};

inline std::string ToString(SandboxPolicyPreset p) {
    switch (p) {
        case SandboxPolicyPreset::Strict:    return "Strict";
        case SandboxPolicyPreset::Standard:  return "Standard";
        case SandboxPolicyPreset::Developer: return "Developer";
        case SandboxPolicyPreset::Disabled:  return "Disabled";
        default: return "Unknown";
    }
}

struct SandboxPolicy {
    SandboxPolicyPreset preset      { SandboxPolicyPreset::Standard };
    JobObjectLimits     limits;
    bool                enablePageFaultNotification { false };

    static SandboxPolicy Strict() {
        SandboxPolicy p;
        p.preset = SandboxPolicyPreset::Strict;
        p.limits.maxMemoryBytes  = 64ULL * 1024 * 1024;   // 64 MB
        p.limits.maxCPUPercent   = 10;
        p.limits.maxHandles      = 64;
        p.limits.maxProcesses    = 1;
        p.limits.allowUIAccess   = false;
        return p;
    }

    static SandboxPolicy Standard() { return {}; }

    static SandboxPolicy Developer() {
        SandboxPolicy p;
        p.preset = SandboxPolicyPreset::Developer;
        p.limits.maxMemoryBytes  = 1024ULL * 1024 * 1024;  // 1 GB
        p.limits.maxCPUPercent   = 100;
        p.limits.maxHandles      = 2048;
        p.limits.allowUIAccess   = true;
        p.limits.killOnJobClose  = false;
        return p;
    }
};

// ─── Timeout kill chain ───────────────────────────────────────────────────────

struct TimeoutKillChain {
    uint32_t    softSignalMs        { 500 };    // send CTRL_BREAK_EVENT first
    uint32_t    drainWindowMs       { 1000 };   // allow orderly shutdown
    uint32_t    forceKillMs         { 2000 };   // TerminateProcess after drain

    static constexpr uint32_t kMaxKillTimeMs = 5000;  // total budget

    uint32_t TotalBudgetMs() const {
        return softSignalMs + drainWindowMs + forceKillMs;
    }

    bool IsValid() const {
        return TotalBudgetMs() <= kMaxKillTimeMs && softSignalMs < drainWindowMs;
    }
};

// ─── Sandbox teardown result ──────────────────────────────────────────────────

enum class TeardownReason : uint32_t {
    NormalExit      = 0,
    TimeoutKill     = 1,
    MemoryQuota     = 2,
    CrashDetected   = 3,
    HostShutdown    = 4,
    PolicyViolation = 5,
};

inline std::string ToString(TeardownReason r) {
    switch (r) {
        case TeardownReason::NormalExit:       return "NormalExit";
        case TeardownReason::TimeoutKill:      return "TimeoutKill";
        case TeardownReason::MemoryQuota:      return "MemoryQuota";
        case TeardownReason::CrashDetected:    return "CrashDetected";
        case TeardownReason::HostShutdown:     return "HostShutdown";
        case TeardownReason::PolicyViolation:  return "PolicyViolation";
        default: return "Unknown";
    }
}

struct HandleLeakReport {
    uint32_t    handleCountAtStart  { 0 };
    uint32_t    handleCountAtEnd    { 0 };
    uint32_t    leakCount           { 0 };
    bool        cleanClose          { true };

    bool HasLeak() const { return leakCount > 0; }
    std::string Summary() const {
        return "Handles: start=" + std::to_string(handleCountAtStart) +
               " end=" + std::to_string(handleCountAtEnd) +
               " leaked=" + std::to_string(leakCount);
    }
};

struct SandboxTeardownResult {
    TeardownReason      reason          { TeardownReason::NormalExit };
    uint32_t            exitCode        { 0 };
    double              durationMs      { 0.0 };
    HandleLeakReport    handleReport;
    bool                orphanDetected  { false };

    bool WasClean() const {
        return reason == TeardownReason::NormalExit &&
               !handleReport.HasLeak() &&
               !orphanDetected;
    }
};

// ─── Policy validator ─────────────────────────────────────────────────────────

struct PolicyViolation {
    std::string     rule;
    std::string     detail;
    bool            isFatal { true };
};

class SandboxPolicyValidator {
public:
    explicit SandboxPolicyValidator(const SandboxPolicy& policy) : m_policy(policy) {}

    std::vector<PolicyViolation> Validate() const {
        std::vector<PolicyViolation> violations;

        if (m_policy.limits.maxMemoryBytes == 0)
            violations.push_back({ "no-memory-limit", "maxMemoryBytes is 0", true });

        if (m_policy.limits.maxHandles > 4096)
            violations.push_back({ "excessive-handles",
                "maxHandles > 4096 — likely misconfigured", false });

        if (m_policy.limits.allowUIAccess &&
            m_policy.preset == SandboxPolicyPreset::Strict)
            violations.push_back({ "ui-access-in-strict",
                "allowUIAccess=true conflicts with Strict preset", true });

        if (!m_policy.limits.killOnJobClose &&
            m_policy.preset != SandboxPolicyPreset::Developer)
            violations.push_back({ "kill-on-close-disabled",
                "killOnJobClose=false only allowed in Developer preset", true });

        return violations;
    }

    bool IsValid() const {
        auto v = Validate();
        for (const auto& viol : v)
            if (viol.isFatal) return false;
        return true;
    }

private:
    SandboxPolicy m_policy;
};

} // namespace ExplorerLens::Plugin

