// ModelVersioningController.h — Model Versioning Controller
// Copyright (c) 2026 ExplorerLens Project
//
// Manages multiple model checkpoints with A/B split and one-click rollback.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace ExplorerLens { namespace Engine {

struct MVCModelVersion {
    std::string id;
    uint32_t    buildNumber   = 0;
    float       validationAcc = 0.0f;
    bool        isActive      = false;
};

struct MVCExperiment {
    std::string name;
    std::string versionA;
    std::string versionB;
    float       splitRatio = 0.5f;
};

struct MVCRollbackResult {
    bool        success       = false;
    std::string rolledBackTo;
    std::string errorMsg;
};

class ModelVersioningController {
public:
    void RegisterVersion(const MVCModelVersion& v)  { m_versions[v.id] = v; }

    bool ActivateVersion(const std::string& id) {
        auto it = m_versions.find(id);
        if (it == m_versions.end()) return false;
        for (auto& [k, ver] : m_versions) ver.isActive = false;
        it->second.isActive = true;
        m_activeId          = id;
        m_history.push_back(id);
        return true;
    }
    MVCRollbackResult Rollback() {
        MVCRollbackResult r;
        if (m_history.size() < 2) { r.errorMsg = "No previous version"; return r; }
        m_history.pop_back();
        r.rolledBackTo = m_history.back();
        ActivateVersion(r.rolledBackTo);
        r.success = true;
        return r;
    }
    std::string ActiveVersionId() const { return m_activeId; }

private:
    std::unordered_map<std::string, MVCModelVersion> m_versions;
    std::vector<std::string>                         m_history;
    std::string                                      m_activeId;
};

}} // namespace ExplorerLens::Engine
