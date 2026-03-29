// AccessibilityAuditEngine.h — Accessibility Audit Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Runs automated WCAG 2.2 / EN 301 549 accessibility audits on thumbnail
// UI state. Reports violations with WCAG success criterion references.
//
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class WCAGCriteria {
    SC_1_1_1_NonTextContent,
    SC_1_4_1_UseOfColor,
    SC_1_4_3_ContrastMinimum,
    SC_1_4_11_NonTextContrast,
    SC_2_1_1_Keyboard,
    SC_2_4_3_FocusOrder,
    SC_4_1_2_NameRoleValue
};

enum class A11yViolationLevel { Error, Warning, Info };

struct A11yViolation {
    WCAGCriteria        criterion;
    A11yViolationLevel  level = A11yViolationLevel::Error;
    std::string         message;
    std::string         element;
    std::string         suggestion;
};

struct A11yAuditResult {
    bool                       passed      = false;
    std::vector<A11yViolation> violations;
    uint32_t                   errorCount  = 0;
    uint32_t                   warnCount   = 0;
    float                      score       = 100.0f;  // 0-100
};

class AccessibilityAuditEngine {
public:
    AccessibilityAuditEngine() = default;

    bool Initialize() { m_ready = true; return true; }
    bool IsReady() const { return m_ready; }

    A11yAuditResult Audit(const std::string& element,
                           const std::string& ariaLabel,
                           float contrastRatio,
                           bool  hasKeyboardFocus) const {
        A11yAuditResult r;
        r.passed = true;

        if (ariaLabel.empty()) {
            A11yViolation v;
            v.criterion  = WCAGCriteria::SC_1_1_1_NonTextContent;
            v.level      = A11yViolationLevel::Error;
            v.element    = element;
            v.message    = "Image missing accessible name (alt text or aria-label).";
            v.suggestion = "Add aria-label or alt attribute.";
            r.violations.push_back(v);
            ++r.errorCount;
            r.passed = false;
        }

        if (contrastRatio < 4.5f) {
            A11yViolation v;
            v.criterion  = WCAGCriteria::SC_1_4_3_ContrastMinimum;
            v.level      = A11yViolationLevel::Error;
            v.element    = element;
            v.message    = "Contrast ratio " + std::to_string(contrastRatio)
                         + ":1 below WCAG AA minimum of 4.5:1.";
            v.suggestion = "Increase foreground/background contrast.";
            r.violations.push_back(v);
            ++r.errorCount;
            r.passed = false;
        }

        if (!hasKeyboardFocus) {
            A11yViolation v;
            v.criterion  = WCAGCriteria::SC_2_1_1_Keyboard;
            v.level      = A11yViolationLevel::Warning;
            v.element    = element;
            v.message    = "Element may not be keyboard-focusable.";
            v.suggestion = "Ensure tabindex=0 and keypress handlers.";
            r.violations.push_back(v);
            ++r.warnCount;
        }

        r.score = 100.0f - (r.errorCount * 20.0f) - (r.warnCount * 5.0f);
        if (r.score < 0.0f) r.score = 0.0f;
        return r;
    }

    void Shutdown() { m_ready = false; }

private:
    bool m_ready = false;
};

}} // namespace ExplorerLens::Engine
