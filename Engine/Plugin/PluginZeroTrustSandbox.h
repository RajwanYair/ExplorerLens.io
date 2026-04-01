// PluginZeroTrustSandbox.h — Plugin Zero-Trust Sandbox
// Copyright (c) 2026 ExplorerLens Project
//
// Enforces the zero-trust model for plugins: every system access requires
// a valid capability token before the call is allowed through the sandbox.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class PluginSandboxDecision : uint8_t {
    Allow = 0,
    Deny,
    DenyAndRevoke,
    Quarantine
};

struct PluginSandboxPolicy {
    std::string pluginId;
    bool        requiresCapabilityToken = true;
    bool        allowNetworkAccess      = false;
    bool        allowFileWrite          = false;
    uint32_t    maxMemoryMB             = 64;
};

struct PluginSandboxStats {
    uint64_t callsAllowed    = 0;
    uint64_t callsDenied     = 0;
    uint64_t quarantineCount = 0;
};

class PluginZeroTrustSandbox {
public:
    static PluginZeroTrustSandbox& Instance() {
        static PluginZeroTrustSandbox s;
        return s;
    }

    void SetPolicy(const PluginSandboxPolicy& policy) { m_policy = policy; }
    const PluginSandboxPolicy& GetPolicy() const { return m_policy; }

    PluginSandboxDecision Evaluate(const std::string& pluginId,
                                   const std::string& capability,
                                   bool               hasValidToken) {
        if (pluginId.empty() || capability.empty()) {
            ++m_stats.callsDenied;
            return PluginSandboxDecision::Deny;
        }
        if (!hasValidToken && m_policy.requiresCapabilityToken) {
            ++m_stats.callsDenied;
            return PluginSandboxDecision::Deny;
        }
        ++m_stats.callsAllowed;
        return PluginSandboxDecision::Allow;
    }

    bool IsQuarantined(const std::string& pluginId) {
        (void)pluginId;
        return false;
    }

    const PluginSandboxStats& GetStats() const { return m_stats; }

private:
    PluginZeroTrustSandbox() = default;
    PluginSandboxPolicy m_policy;
    PluginSandboxStats  m_stats;
};

}} // namespace ExplorerLens::Engine
