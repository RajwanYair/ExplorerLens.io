//==============================================================================
// DarkThumbs Engine — Sprint 345: Accessibility Suite V2
// WCAG 2.2 AA/AAA thumbnail colour contrast enforcement, UI Automation (UIA)
// V2 provider for thumbnail grid, enhanced high-contrast mode theming,
// live region announcements, and colour-blind simulation palette V2.
//==============================================================================
#pragma once
#include <string>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

enum class WCAGLevel           : uint8_t { A=0,AA,AAA,COUNT };
enum class ColorBlindMode      : uint8_t { None=0,Deuteranopia,Protanopia,Tritanopia,Achromatopsia,COUNT };
enum class UIAControlType      : uint8_t { Image=0,List,ListItem,Group,Button,COUNT };
enum class AccessibilityFeature: uint8_t {
    ContrastEnforcement=0, ColorBlindSim, UIA_V2, HighContrast, LiveRegion, FocusIndicator, COUNT
};

struct AccessibilityConfig {
    WCAGLevel           targetLevel      = WCAGLevel::AA;
    ColorBlindMode      colorBlindSim    = ColorBlindMode::None;
    bool                highContrast     = false;
    bool                uiaV2            = true;
    bool                liveRegions      = true;
    float               minContrastRatio = 4.5f; // WCAG AA normal text
};

struct AccessibilityAuditResult {
    WCAGLevel   achievedLevel    = WCAGLevel::A;
    uint32_t    violationCount   = 0;
    float       avgContrastRatio = 0.0f;
    bool        uiaTreeValid     = false;
};

class AccessibilitySuiteV2 {
public:
    static const wchar_t* WCAGLevelName(WCAGLevel l) {
        switch(l) {
            case WCAGLevel::A:   return L"WCAG 2.2 Level A";
            case WCAGLevel::AA:  return L"WCAG 2.2 Level AA";
            case WCAGLevel::AAA: return L"WCAG 2.2 Level AAA";
            default: return L"Unknown";
        }
    }
    static const wchar_t* ColorBlindModeName(ColorBlindMode m) {
        switch(m) {
            case ColorBlindMode::None:          return L"Normal Vision";
            case ColorBlindMode::Deuteranopia:  return L"Deuteranopia (Red-Green)";
            case ColorBlindMode::Protanopia:    return L"Protanopia (Red-Green)";
            case ColorBlindMode::Tritanopia:    return L"Tritanopia (Blue-Yellow)";
            case ColorBlindMode::Achromatopsia: return L"Achromatopsia (Monochrome)";
            default: return L"Unknown";
        }
    }
    static const wchar_t* FeatureName(AccessibilityFeature f) {
        switch(f) {
            case AccessibilityFeature::ContrastEnforcement: return L"Contrast Enforcement";
            case AccessibilityFeature::ColorBlindSim:       return L"Color Blind Simulation";
            case AccessibilityFeature::UIA_V2:              return L"UI Automation V2";
            case AccessibilityFeature::HighContrast:        return L"High Contrast Mode";
            case AccessibilityFeature::LiveRegion:          return L"Live Region Announcements";
            case AccessibilityFeature::FocusIndicator:      return L"Focus Indicator";
            default: return L"Unknown";
        }
    }
    static constexpr size_t WCAGLevelCount()    { return static_cast<size_t>(WCAGLevel::COUNT); }
    static constexpr size_t ColorBlindCount()   { return static_cast<size_t>(ColorBlindMode::COUNT); }
    static constexpr size_t FeatureCount()      { return static_cast<size_t>(AccessibilityFeature::COUNT); }
};

}} // namespace DarkThumbs::Engine
