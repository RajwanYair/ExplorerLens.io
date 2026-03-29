// RBACEngineV2.h — Multi-Tier RBAC Engine v2
// Copyright (c) 2026 ExplorerLens Project
//
// Enforces role-based and attribute-based access control for ExplorerLens
// enterprise resources with dynamic policy evaluation.
//
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace ExplorerLens { namespace Engine {

enum class RBACRole { Viewer, Editor, Admin, SuperAdmin, ServiceAccount };

struct RBACPrincipal {
    std::string             principalId;
    RBACRole                role = RBACRole::Viewer;
    std::vector<std::string> groups;
    std::unordered_map<std::string, std::string> attributes;
};

struct RBACPolicy {
    std::string              resource;
    std::unordered_set<RBACRole> allowedRoles;
    std::vector<std::string> requiredAttributes;
};

class RBACEngineV2 {
public:
    RBACEngineV2() = default;

    bool Initialize() { m_ready = true; return true; }
    bool IsReady() const { return m_ready; }

    void AddPrincipal(const RBACPrincipal& p) { m_principals[p.principalId] = p; }
    void AddPolicy(const RBACPolicy& policy)   { m_policies[policy.resource] = policy; }

    bool IsAuthorized(const std::string& principalId, const std::string& resource) const {
        if (!m_ready) return false;
        auto pit = m_principals.find(principalId);
        if (pit == m_principals.end()) return false;
        auto policyIt = m_policies.find(resource);
        if (policyIt == m_policies.end()) return true; // no policy = open
        const auto& policy = policyIt->second;
        const auto& principal = pit->second;
        if (policy.allowedRoles.count(principal.role) == 0) return false;
        for (const auto& attr : policy.requiredAttributes) {
            if (principal.attributes.count(attr) == 0) return false;
        }
        return true;
    }

    void RevokePrincipal(const std::string& id) { m_principals.erase(id); }

    static std::string RoleName(RBACRole r) {
        switch (r) {
            case RBACRole::Viewer:         return "viewer";
            case RBACRole::Editor:         return "editor";
            case RBACRole::Admin:          return "admin";
            case RBACRole::SuperAdmin:     return "superadmin";
            case RBACRole::ServiceAccount: return "service";
        }
        return "unknown";
    }

    void Shutdown() { m_ready = false; }

private:
    bool m_ready = false;
    std::unordered_map<std::string, RBACPrincipal> m_principals;
    std::unordered_map<std::string, RBACPolicy>    m_policies;
};

}} // namespace ExplorerLens::Engine
