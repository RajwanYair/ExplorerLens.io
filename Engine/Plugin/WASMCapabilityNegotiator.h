// WASMCapabilityNegotiator.h — WASM Plugin Capability Negotiation Protocol
// Copyright (c) 2026 ExplorerLens Project
//
// Implements a handshake protocol between host and WASM plugin to negotiate
// the exact set of WASI capabilities granted at load time — reject/revoke workflow.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

enum class CapabilityDecision { Granted, Denied, Deferred };

struct CapabilityRequest {
    std::string capabilityName;
    std::string reason;
    bool        isRequired  = false;
};

struct CapabilityGrant {
    std::string       capabilityName;
    CapabilityDecision decision  = CapabilityDecision::Denied;
    std::string        denyReason;
    bool               IsGranted() const { return decision == CapabilityDecision::Granted; }
};

struct NegotiationResult {
    std::vector<CapabilityGrant> grants;
    bool allRequiredGranted = false;
    uint32_t GrantedCount() const {
        uint32_t n = 0;
        for (const auto& g : grants) if (g.IsGranted()) ++n;
        return n;
    }
};

class WASMCapabilityNegotiator {
public:
    WASMCapabilityNegotiator() = default;

    void AllowCapability(const std::string& name)  { m_allowed.push_back(name); }
    void DenyCapability(const std::string& name)   { m_denied.push_back(name); }

    NegotiationResult Negotiate(const std::vector<CapabilityRequest>& requests) {
        NegotiationResult result;
        bool allRequired = true;
        for (const auto& req : requests) {
            CapabilityGrant grant;
            grant.capabilityName = req.capabilityName;
            bool allowed = std::find(m_allowed.begin(), m_allowed.end(), req.capabilityName) != m_allowed.end();
            bool denied  = std::find(m_denied.begin(), m_denied.end(),  req.capabilityName) != m_denied.end();
            if (allowed && !denied) {
                grant.decision = CapabilityDecision::Granted;
            } else {
                grant.decision  = CapabilityDecision::Denied;
                grant.denyReason= "Not in allow-list";
                if (req.isRequired) allRequired = false;
            }
            result.grants.push_back(grant);
        }
        result.allRequiredGranted = allRequired;
        return result;
    }

    void  Reset() { m_allowed.clear(); m_denied.clear(); }
    size_t AllowedCount() const { return m_allowed.size(); }
    size_t DeniedCount()  const { return m_denied.size(); }

private:
    std::vector<std::string> m_allowed;
    std::vector<std::string> m_denied;
};

} // namespace Engine
} // namespace ExplorerLens
