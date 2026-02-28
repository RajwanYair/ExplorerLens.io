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

// Sandbox hardening: Job Object, timeout kill chain, memory quotas
#include "PluginSandboxPolicy.h"

// Runtime validation: state machine, soak tests, IPC message types
#include "PluginRuntimeValidation.h"

// Certificate trust chain verification
#include "PluginTrustChain.h"

namespace ExplorerLens {
namespace Plugin {

/// Security assessment level for a plugin
enum class PluginSecurityLevel : uint32_t {
    Untrusted = 0,  ///< Unknown publisher, no signature
    Basic = 1,  ///< Signed but not vetted
    Verified = 2,  ///< Marketplace-verified, sandbox-compliant
    Trusted = 3,  ///< Microsoft Store / enterprise-signed
    BuiltIn = 4   ///< Ships with ExplorerLens
};

/// Quick check: can plugin run without elevated sandbox?
inline bool RequiresSandbox(PluginSecurityLevel level) {
    return level < PluginSecurityLevel::Trusted;
}

/// Quick check: is plugin allowed to execute at all?
inline bool IsAllowedToExecute(PluginSecurityLevel /*level*/) {
    // All levels can execute; Untrusted runs in strict sandbox
    return true;
}

/// Returns recommended SandboxPolicyPreset for a given security level
inline SandboxPolicyPreset RecommendedSandboxPreset(PluginSecurityLevel level) {
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

} // namespace Plugin
} // namespace ExplorerLens
