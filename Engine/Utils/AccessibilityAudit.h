// AccessibilityAudit.h — Accessibility Audit + UIA Compliance Module
// Copyright (c) 2026 ExplorerLens Project
//
// Provides compile-time and runtime accessibility validation for ExplorerLens
// UI components.  Checks UI Automation (UIA) provider registration, ARIA roles,
// keyboard navigation order, and contrast ratio compliance (WCAG 2.1 AA).
//
#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

// Accessibility violation severity.
enum class A11ySeverity : uint8_t {
    Info     = 0,
    Warning  = 1,
    Error    = 2,
    Critical = 3,
};

// A single accessibility finding from an audit pass.
struct A11yFinding {
    A11ySeverity severity;
    std::string  component;   // Component or control name
    std::string  rule;        // WCAG rule ID e.g. "1.4.3 Contrast Minimum"
    std::string  description; // Human-readable explanation
    std::string  suggestion;  // Actionable fix
};

// Summary result from AccessibilityAudit::RunAudit().
struct A11yAuditResult {
    std::vector<A11yFinding> findings;
    uint32_t                 criticalCount { 0 };
    uint32_t                 errorCount    { 0 };
    uint32_t                 warningCount  { 0 };
    bool                     passed        { false };

    std::string Summary() const;
};

// AccessibilityAudit — Runtime UIA and WCAG 2.1 AA compliance checker.
//
// Validates:
//   - UIA provider registration for all interactive controls
//   - Keyboard navigation (Tab order, focus visibility)
//   - Contrast ratios (WCAG 1.4.3 / 1.4.6)
//   - Accessible names for icon-only buttons
//   - Screen-reader announcements on dynamic content updates
class AccessibilityAudit {
public:
    AccessibilityAudit() noexcept  = default;
    ~AccessibilityAudit() noexcept = default;

    AccessibilityAudit(const AccessibilityAudit&)            = delete;
    AccessibilityAudit& operator=(const AccessibilityAudit&) = delete;

    // Run full audit pass.  hwnd = top-level window to inspect.
    // Set thoroughness 0 (quick) – 3 (exhaustive).
    A11yAuditResult RunAudit(void* hwnd, int thoroughness = 1) noexcept;

    // Check a single contrast ratio pair.  Returns true if ratio ≥ 4.5:1.
    static bool CheckContrastAA(uint32_t foregroundArgb,
                                 uint32_t backgroundArgb) noexcept;

    // Check enhanced contrast.  Returns true if ratio ≥ 7:1.
    static bool CheckContrastAAA(uint32_t foregroundArgb,
                                  uint32_t backgroundArgb) noexcept;

    // Compute relative luminance per WCAG 2.1 definition.
    static double RelativeLuminance(uint32_t argb) noexcept;

    // Compute contrast ratio between two luminance values.
    static double ContrastRatio(double l1, double l2) noexcept;
};

}} // namespace ExplorerLens::Engine
