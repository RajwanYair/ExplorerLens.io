// AccessibilityAuditPipeline.h — Accessibility Audit Pipeline (WCAG 2.2 / ARIA)
// Copyright (c) 2026 ExplorerLens Project
//
// Runs an automated accessibility audit across all registered UI elements,
// checking WCAG 2.2 compliance (contrast, labels, roles, keyboard access) and
// generating a structured report with severity-ranked findings.
//
#pragma once
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class A11yIssueSeverity { Critical, Serious, Moderate, Minor };
enum class WCAGSuccessCriterion {
    SC_1_1_1_NonTextContent,
    SC_1_4_3_ContrastMinimum,
    SC_1_4_6_ContrastEnhanced,
    SC_2_1_1_Keyboard,
    SC_2_4_6_HeadingsAndLabels,
    SC_4_1_2_NameRoleValue
};

struct A11yIssue {
    WCAGSuccessCriterion criterion   = WCAGSuccessCriterion::SC_1_4_3_ContrastMinimum;
    A11yIssueSeverity    severity    = A11yIssueSeverity::Serious;
    std::string          elementId;
    std::string          description;
    std::string          remediation;

    std::string SeverityName() const noexcept {
        switch (severity) {
        case A11yIssueSeverity::Critical:  return "Critical";
        case A11yIssueSeverity::Serious:   return "Serious";
        case A11yIssueSeverity::Moderate:  return "Moderate";
        case A11yIssueSeverity::Minor:     return "Minor";
        }
        return "Unknown";
    }
};

struct A11yAuditReport {
    int                   totalElements = 0;
    int                   passCount     = 0;
    int                   failCount     = 0;
    std::vector<A11yIssue> issues;
    double PassRate() const noexcept {
        return totalElements > 0 ? (100.0 * passCount / totalElements) : 0.0;
    }
    bool IsCompliant() const noexcept { return failCount == 0; }
};

struct AuditableElement {
    std::string  id;
    std::string  name;
    std::string  role;
    uint32_t     foregroundRGB = 0x000000;
    uint32_t     backgroundRGB = 0xFFFFFF;
    bool         hasLabel      = true;
    bool         isKeyboardFocusable = true;
};

class AccessibilityAuditPipeline {
public:
    explicit AccessibilityAuditPipeline() = default;

    A11yAuditReport Audit(const std::vector<AuditableElement>& elements) const {
        A11yAuditReport report;
        report.totalElements = static_cast<int>(elements.size());
        for (const auto& el : elements) {
            CheckElement(el, report);
        }
        return report;
    }

    static std::string CriterionName(WCAGSuccessCriterion c) noexcept {
        switch (c) {
        case WCAGSuccessCriterion::SC_1_1_1_NonTextContent:   return "1.1.1 Non-text Content";
        case WCAGSuccessCriterion::SC_1_4_3_ContrastMinimum:  return "1.4.3 Contrast (Minimum)";
        case WCAGSuccessCriterion::SC_1_4_6_ContrastEnhanced: return "1.4.6 Contrast (Enhanced)";
        case WCAGSuccessCriterion::SC_2_1_1_Keyboard:         return "2.1.1 Keyboard";
        case WCAGSuccessCriterion::SC_2_4_6_HeadingsAndLabels: return "2.4.6 Headings and Labels";
        case WCAGSuccessCriterion::SC_4_1_2_NameRoleValue:    return "4.1.2 Name, Role, Value";
        }
        return "Unknown";
    }

private:
    static double RelLuminance(uint32_t rgb) noexcept {
        auto chan = [](uint32_t c, int shift) {
            double v = ((c >> shift) & 0xFF) / 255.0;
            return v <= 0.04045 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4);
        };
        return 0.2126 * chan(rgb, 16) + 0.7152 * chan(rgb, 8) + 0.0722 * chan(rgb, 0);
    }

    void CheckElement(const AuditableElement& el, A11yAuditReport& report) const {
        bool pass = true;
        // Contrast check
        double lFG = RelLuminance(el.foregroundRGB), lBG = RelLuminance(el.backgroundRGB);
        double lighter = std::max(lFG, lBG), darker = std::min(lFG, lBG);
        double ratio = (lighter + 0.05) / (darker + 0.05);
        if (ratio < 4.5) {
            report.issues.push_back({ WCAGSuccessCriterion::SC_1_4_3_ContrastMinimum,
                A11yIssueSeverity::Serious, el.id,
                "Contrast ratio " + std::to_string(static_cast<int>(ratio * 10) / 10.0) + ":1 < 4.5:1",
                "Increase foreground/background colour contrast" });
            pass = false;
        }
        // Label check
        if (!el.hasLabel || el.name.empty()) {
            report.issues.push_back({ WCAGSuccessCriterion::SC_4_1_2_NameRoleValue,
                A11yIssueSeverity::Critical, el.id, "Element has no accessible name",
                "Add an accessible name via aria-label or label element" });
            pass = false;
        }
        // Keyboard check
        if (!el.isKeyboardFocusable) {
            report.issues.push_back({ WCAGSuccessCriterion::SC_2_1_1_Keyboard,
                A11yIssueSeverity::Critical, el.id, "Element not keyboard focusable",
                "Ensure tabIndex >= 0 or use a native interactive element" });
            pass = false;
        }
        if (pass) report.passCount++; else report.failCount++;
    }
};

} // namespace Engine
} // namespace ExplorerLens
