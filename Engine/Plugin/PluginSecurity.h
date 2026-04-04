#pragma once
//=============================================================================
// PluginSecurity.h — Unified Plugin Security Umbrella
//
// Consolidates plugin security/validation components:
//   - PluginSandboxPolicy.h       (Job Object limits, timeout kill chain,
//                                   memory quotas, handle leak detection)
//   - PluginRuntimeValidation.h   (Lifecycle state machine, soak testing,
//                                   IPC message types, capability checks)
//   - PluginTrustChain.h          (Certificate trust chain verification)
//
// Usage: #include "PluginSecurity.h" for all security-related plugin APIs.
//
// Namespace note:
//   SandboxPolicy and RuntimeValidation use ExplorerLens::Plugin
//   TrustChain uses ExplorerLens::Engine
//   All coexist without collision.
//=============================================================================

// Sandbox hardening (V3): Job Object, timeout kill chain, memory quotas
#include "PluginSandboxV3.h"

// Certificate trust chain verification
#include "PluginTrustChainValidator.h"

namespace ExplorerLens {
namespace Plugin {

/// Sandbox policy preset (consolidated from PluginSandboxPolicy.h)
enum class SandboxPolicyPreset : uint8_t {
    Developer = 0,
    Standard,
    Strict
};

/// Security assessment level for a plugin
enum class PluginSecurityLevel : uint32_t {
    Untrusted = 0,  ///< Unknown publisher, no signature
    Basic = 1,      ///< Signed but not vetted
    Verified = 2,   ///< Marketplace-verified, sandbox-compliant
    Trusted = 3,    ///< Microsoft Store / enterprise-signed
    BuiltIn = 4     ///< Ships with ExplorerLens
};

/// Quick check: can plugin run without elevated sandbox?
inline bool RequiresSandbox(PluginSecurityLevel level)
{
    return level < PluginSecurityLevel::Trusted;
}

/// Quick check: is plugin allowed to execute at all?
inline bool IsAllowedToExecute(PluginSecurityLevel /*level*/)
{
    // All levels can execute; Untrusted runs in strict sandbox
    return true;
}

/// Returns recommended SandboxPolicyPreset for a given security level
inline SandboxPolicyPreset RecommendedSandboxPreset(PluginSecurityLevel level)
{
    switch (level) {
        case PluginSecurityLevel::Untrusted:
            return SandboxPolicyPreset::Strict;
        case PluginSecurityLevel::Basic:
            return SandboxPolicyPreset::Standard;
        case PluginSecurityLevel::Verified:
            return SandboxPolicyPreset::Standard;
        case PluginSecurityLevel::Trusted:
        case PluginSecurityLevel::BuiltIn:
            return SandboxPolicyPreset::Developer;
        default:
            return SandboxPolicyPreset::Strict;
    }
}

// Concrete sandbox policy specification with static factory presets
struct SandboxJobLimits
{
    uint64_t maxMemoryBytes = 256ULL * 1024 * 1024;
    uint32_t maxCPUPercent = 25;
    uint32_t maxHandles = 256;
    bool killOnJobClose = true;
};

struct SandboxPolicySpec
{
    SandboxPolicyPreset preset = SandboxPolicyPreset::Developer;
    SandboxJobLimits limits;
    static SandboxPolicySpec Strict()
    {
        SandboxPolicySpec s;
        s.preset = SandboxPolicyPreset::Strict;
        s.limits = {128ULL * 1024 * 1024, 10, 128, true};
        return s;
    }
    static SandboxPolicySpec Standard()
    {
        SandboxPolicySpec s;
        s.preset = SandboxPolicyPreset::Standard;
        s.limits = {256ULL * 1024 * 1024, 25, 256, true};
        return s;
    }
    static SandboxPolicySpec Developer()
    {
        SandboxPolicySpec s;
        s.preset = SandboxPolicyPreset::Developer;
        s.limits = {512ULL * 1024 * 1024, 50, 512, true};
        return s;
    }
};

enum class TeardownReason : uint8_t {
    NormalExit,
    TimeoutKill,
    CrashTerminate
};

inline std::string ToString(TeardownReason r)
{
    switch (r) {
        case TeardownReason::NormalExit:
            return "NormalExit";
        case TeardownReason::TimeoutKill:
            return "TimeoutKill";
        case TeardownReason::CrashTerminate:
            return "CrashTerminate";
    }
    return "Unknown";
}

struct SandboxTeardownResult
{
    TeardownReason reason = TeardownReason::NormalExit;
    int32_t exitCode = 0;
    bool WasClean() const noexcept
    {
        return reason == TeardownReason::NormalExit && exitCode == 0;
    }
};

struct HandleLeakReport
{
    uint32_t leaked = 0;
    bool HasLeak() const noexcept
    {
        return leaked > 0;
    }
    std::string Summary() const
    {
        return "leaked=" + std::to_string(leaked);
    }
};

class SandboxPolicyValidator
{
  public:
    explicit SandboxPolicyValidator(const SandboxPolicySpec& policy) : m_policy(policy) {}
    bool IsValid() const noexcept
    {
        return m_policy.limits.maxMemoryBytes > 0 && m_policy.limits.maxCPUPercent <= 100;
    }

  private:
    SandboxPolicySpec m_policy;
};

// Runtime validator for plugin state machine transitions
class PluginRuntimeValidator
{
  public:
    static PluginRuntimeValidator Create()
    {
        return {};
    }
    uint32_t InvalidTransitionCount() const noexcept
    {
        return 0;
    }

    // Validate lifecycle state machine transitions (see PluginRuntimeValidation.h for states)
    template <typename StateEnum>
    bool IsValidTransition(StateEnum from, StateEnum to) const noexcept
    {
        int f = static_cast<int>(from);
        int t = static_cast<int>(to);
        if (t == 7)
            return true;  // -> Faulted is always valid
        if (t == f + 1)
            return true;
        if (f == 6 && t == 0)
            return true;  // Stopping -> Unloaded
        return false;
    }
};

}  // namespace Plugin
}  // namespace ExplorerLens
