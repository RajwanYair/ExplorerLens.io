#pragma once
//==============================================================================
// AccessibilityEngine — Sprint 220
// Accessibility support: screen reader integration, high contrast detection,
// keyboard navigation, reduced motion, and WCAG compliance checking.
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace DarkThumbs { namespace Engine {

enum class A11yFeature : uint8_t {
    ScreenReader     = 0,
    HighContrast     = 1,
    ReducedMotion    = 2,
    LargeText        = 3,
    KeyboardNav      = 4,
    ColorBlindMode   = 5,
    NarratorSupport  = 6,
    FeatureCount     = 7
};

enum class ContrastMode : uint8_t {
    Normal     = 0,
    HighWhite  = 1,
    HighBlack  = 2,
    Custom     = 3
};

struct A11yStatus {
    bool screenReaderActive = false;
    bool highContrastEnabled = false;
    ContrastMode contrastMode = ContrastMode::Normal;
    bool reducedMotion = false;
    uint32_t textScalePercent = 100;
    uint32_t featuresEnabled = 0;
};

struct A11yAuditResult {
    bool compliant = false;
    uint32_t checksRun = 0;
    uint32_t checksPassed = 0;
    double auditTimeMs = 0.0;
    std::vector<std::wstring> issues;
};

class AccessibilityEngine {
public:
    AccessibilityEngine();

    A11yStatus DetectSettings() const;
    A11yAuditResult RunComplianceAudit() const;

    bool IsFeatureEnabled(A11yFeature feature) const;
    void EnableFeature(A11yFeature feature);
    void DisableFeature(A11yFeature feature);

    uint32_t GetEnabledFeatureCount() const;

    static const wchar_t* GetFeatureName(A11yFeature feature);
    static const wchar_t* GetContrastModeName(ContrastMode mode);
    static uint32_t GetFeatureCount() { return static_cast<uint32_t>(A11yFeature::FeatureCount); }

private:
    uint32_t m_enabledFeatures = 0;
};

}} // namespace DarkThumbs::Engine
