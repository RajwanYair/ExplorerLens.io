// PolicyComplianceValidator.h — Enterprise Policy Compliance Checking
// Copyright (c) 2026 ExplorerLens Project
//
// Validates that the current configuration complies with enterprise
// policies (GPO, Intune, SCCM). Reports violations and can enforce
// mandatory settings for managed deployments.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class PolicySource : uint8_t {
    GroupPolicy, Intune, SCCM, LocalAdmin, Default, COUNT
};

enum class PolicySeverity : uint8_t {
    Informational, Warning, Required, Mandatory, COUNT
};

enum class ComplianceStatus : uint8_t {
    Compliant, NonCompliant, Unknown, NotApplicable, COUNT
};

struct PolicyRule {
    std::wstring ruleName;
    std::wstring description;
    PolicySource source = PolicySource::Default;
    PolicySeverity severity = PolicySeverity::Informational;
    std::wstring expectedValue;
    std::wstring actualValue;
};

struct ComplianceResult {
    std::wstring ruleName;
    ComplianceStatus status = ComplianceStatus::Unknown;
    std::wstring detail;
    bool autoRemediated = false;
};

struct ComplianceReport {
    uint32_t totalRules = 0;
    uint32_t compliantCount = 0;
    uint32_t violationCount = 0;
    uint32_t remediatedCount = 0;
    bool overallCompliant = true;
};

class PolicyComplianceValidator {
public:
    void AddRule(const PolicyRule& rule) {
        m_rules.push_back(rule);
    }

    ComplianceResult Validate(const PolicyRule& rule) const {
        ComplianceResult result;
        result.ruleName = rule.ruleName;
        if (rule.expectedValue == rule.actualValue) {
            result.status = ComplianceStatus::Compliant;
        }
        else if (rule.severity == PolicySeverity::Mandatory) {
            result.status = ComplianceStatus::NonCompliant;
            result.detail = L"Mandatory policy violation: expected '" + rule.expectedValue + L"'";
        }
        else {
            result.status = ComplianceStatus::NonCompliant;
            result.detail = L"Policy mismatch";
        }
        return result;
    }

    ComplianceReport ValidateAll() const {
        ComplianceReport report;
        report.totalRules = static_cast<uint32_t>(m_rules.size());
        for (auto& rule : m_rules) {
            auto result = Validate(rule);
            if (result.status == ComplianceStatus::Compliant) {
                report.compliantCount++;
            }
            else {
                report.violationCount++;
                report.overallCompliant = false;
            }
        }
        return report;
    }

    size_t RuleCount() const { return m_rules.size(); }
    void Clear() { m_rules.clear(); }

    static size_t SourceCount() { return static_cast<size_t>(PolicySource::COUNT); }
    static size_t SeverityLevelCount() { return static_cast<size_t>(PolicySeverity::COUNT); }

private:
    std::vector<PolicyRule> m_rules;
};

} // namespace Engine
} // namespace ExplorerLens
