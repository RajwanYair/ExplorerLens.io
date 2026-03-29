// ComplianceScoreEngineV2.h — Compliance Score Engine v2
// Copyright (c) 2026 ExplorerLens Project
//
// Computes compliance posture scores against SOC-2, ISO-27001, and NIST CSF
// frameworks with automated evidence mapping and gap analysis.
//
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class ComplianceFramework { SOC2, ISO27001, NISTCSF, GDPR, HIPAA };

struct ComplianceControl {
    std::string  id;
    std::string  description;
    bool         pass      = false;
    float        weight    = 1.0f;
    std::string  evidence;
};

struct ComplianceScore {
    ComplianceFramework framework    = ComplianceFramework::SOC2;
    float               score        = 0.0f; // 0-100
    uint32_t            passing      = 0;
    uint32_t            failing      = 0;
    uint32_t            notApplicable= 0;
    std::vector<std::string> gaps;
};

class ComplianceScoreEngineV2 {
public:
    ComplianceScoreEngineV2() = default;

    bool Initialize() { m_ready = true; return true; }
    bool IsReady() const { return m_ready; }

    void AddControl(ComplianceFramework fw, const ComplianceControl& ctrl) {
        m_controls[static_cast<int>(fw)].push_back(ctrl);
    }

    ComplianceScore Compute(ComplianceFramework fw) const {
        ComplianceScore score;
        score.framework = fw;
        const auto& ctrls = m_controls.count(static_cast<int>(fw))
            ? m_controls.at(static_cast<int>(fw))
            : std::vector<ComplianceControl>{};

        float total = 0.0f, passed = 0.0f;
        for (const auto& c : ctrls) {
            total += c.weight;
            if (c.pass) {
                passed += c.weight;
                ++score.passing;
            } else {
                ++score.failing;
                score.gaps.push_back(c.id + ": " + c.description);
            }
        }
        score.score = total > 0.0f ? (passed / total) * 100.0f : 100.0f;
        return score;
    }

    static std::string FrameworkName(ComplianceFramework f) {
        switch (f) {
            case ComplianceFramework::SOC2:     return "SOC-2";
            case ComplianceFramework::ISO27001: return "ISO-27001";
            case ComplianceFramework::NISTCSF:  return "NIST-CSF";
            case ComplianceFramework::GDPR:     return "GDPR";
            case ComplianceFramework::HIPAA:    return "HIPAA";
        }
        return "unknown";
    }

    void Shutdown() { m_ready = false; }

private:
    bool m_ready = false;
    std::unordered_map<int, std::vector<ComplianceControl>> m_controls;
};

}} // namespace ExplorerLens::Engine
