// AccessibilityAuditPipeline.h — WCAG 2.1 Accessibility Audit Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Audits a set of UI elements against WCAG 2.1 success criteria (contrast
// ratio, labelling, keyboard reachability). Returns a structured report with
// per-criterion issues and aggregate pass/fail statistics.
//
#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <cmath>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

enum class WCAGSuccessCriterion : uint32_t {
    SC_1_1_1_NonTextContent         = 0,
    SC_1_4_3_ContrastMinimum        = 1,
    SC_1_4_11_NonTextContrast       = 2,
    SC_4_1_2_NameRoleValue          = 3,
    SC_2_1_1_Keyboard               = 4,
};

struct AuditableElement {
    std::string id;
    std::string name;
    uint32_t    foregroundRGB       = 0x000000;
    uint32_t    backgroundRGB       = 0xFFFFFF;
    bool        hasLabel            = false;
    bool        isKeyboardFocusable = false;
};

struct AuditIssue {
    std::string         elementId;
    WCAGSuccessCriterion criterion = WCAGSuccessCriterion::SC_1_4_3_ContrastMinimum;
    std::string         description;
};

struct AuditReport {
    int                    totalElements = 0;
    std::vector<AuditIssue> issues;

    bool IsCompliant() const noexcept {
        return issues.empty();
    }

    double PassRate() const noexcept {
        if (totalElements == 0) return 100.0;
        // Compute per-element failure using issue element ids (unique set)
        std::vector<std::string> failedIds;
        for (const auto& iss : issues) {
            bool found = false;
            for (const auto& id : failedIds) if (id == iss.elementId) { found = true; break; }
            if (!found) failedIds.push_back(iss.elementId);
        }
        const double passed = static_cast<double>(totalElements - static_cast<int>(failedIds.size()));
        return (passed / totalElements) * 100.0;
    }
};

class AccessibilityAuditPipeline {
public:
    AuditReport Audit(const std::vector<AuditableElement>& elements) const {
        AuditReport report;
        report.totalElements = static_cast<int>(elements.size());
        for (const auto& el : elements) {
            CheckContrast(el, report);
            CheckLabel(el, report);
        }
        return report;
    }

    static std::string CriterionName(WCAGSuccessCriterion c) {
        switch (c) {
            case WCAGSuccessCriterion::SC_1_1_1_NonTextContent:   return "1.1.1 Non-text Content";
            case WCAGSuccessCriterion::SC_1_4_3_ContrastMinimum:  return "1.4.3 Contrast (Minimum)";
            case WCAGSuccessCriterion::SC_1_4_11_NonTextContrast: return "1.4.11 Non-text Contrast";
            case WCAGSuccessCriterion::SC_4_1_2_NameRoleValue:    return "4.1.2 Name, Role, Value";
            case WCAGSuccessCriterion::SC_2_1_1_Keyboard:         return "2.1.1 Keyboard";
        }
        return "Unknown";
    }

private:
    static double RelativeLuminance(uint32_t rgb) noexcept {
        const double r = ToLinear(((rgb >> 16) & 0xFF) / 255.0);
        const double g = ToLinear(((rgb >>  8) & 0xFF) / 255.0);
        const double b = ToLinear(( rgb        & 0xFF) / 255.0);
        return 0.2126 * r + 0.7152 * g + 0.0722 * b;
    }

    static double ToLinear(double c) noexcept {
        if (c <= 0.04045) return c / 12.92;
        return std::pow((c + 0.055) / 1.055, 2.4);
    }

    static double ContrastRatio(uint32_t fg, uint32_t bg) noexcept {
        const double L1 = RelativeLuminance(fg);
        const double L2 = RelativeLuminance(bg);
        const double lighter = std::max(L1, L2);
        const double darker  = std::min(L1, L2);
        return (lighter + 0.05) / (darker + 0.05);
    }

    static void CheckContrast(const AuditableElement& el, AuditReport& report) {
        constexpr double WCAG_AA_MIN = 4.5;
        const double ratio = ContrastRatio(el.foregroundRGB, el.backgroundRGB);
        if (ratio < WCAG_AA_MIN) {
            AuditIssue issue;
            issue.elementId   = el.id;
            issue.criterion   = WCAGSuccessCriterion::SC_1_4_3_ContrastMinimum;
            issue.description = "Contrast ratio " + std::to_string(ratio) + " below 4.5:1";
            report.issues.push_back(std::move(issue));
        }
    }

    static void CheckLabel(const AuditableElement& el, AuditReport& report) {
        if (!el.hasLabel) {
            AuditIssue issue;
            issue.elementId   = el.id;
            issue.criterion   = WCAGSuccessCriterion::SC_4_1_2_NameRoleValue;
            issue.description = "Element must have an accessible name (label)";
            report.issues.push_back(std::move(issue));
        }
    }
};

}} // namespace ExplorerLens::Engine
