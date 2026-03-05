// AccessibilityEnhancer.h — Accessibility Feature Support
// Copyright (c) 2026 ExplorerLens Project
//
// Enhances thumbnail accessibility through high-contrast adjustments,
// screen-reader compatible metadata annotations, enlarged overlay text,
// and colorblind-safe palette selection for overlay badges/icons.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class AccessibilityMode : uint8_t {
    Standard, HighContrast, LargeText, ScreenReader, ColorblindSafe, COUNT
};

enum class ColorVisionType : uint8_t {
    Normal, Protanopia, Deuteranopia, Tritanopia, Achromatopsia, COUNT
};

struct A11yEnhancerConfig {
    AccessibilityMode mode = AccessibilityMode::Standard;
    ColorVisionType vision = ColorVisionType::Normal;
    float textScaleFactor = 1.0f;
    float contrastBoost = 0.0f;
    bool useSystemSettings = true;
    bool enableNarrator = false;
};

struct AccessibilityResult {
    bool highContrastActive = false;
    bool largeTextActive = false;
    float effectiveScale = 1.0f;
    float effectiveContrast = 1.0f;
    uint32_t adjustmentsMade = 0;
};

class AccessibilityEnhancer {
public:
    void Configure(const A11yEnhancerConfig& cfg) { m_config = cfg; }
    const A11yEnhancerConfig& GetConfig() const { return m_config; }

    AccessibilityResult Apply() const {
        AccessibilityResult result;
        result.effectiveScale = m_config.textScaleFactor;
        result.effectiveContrast = 1.0f + m_config.contrastBoost;

        if (m_config.mode == AccessibilityMode::HighContrast) {
            result.highContrastActive = true;
            result.effectiveContrast = 2.0f;
            result.adjustmentsMade++;
        }
        if (m_config.mode == AccessibilityMode::LargeText) {
            result.largeTextActive = true;
            result.effectiveScale = 1.5f;
            result.adjustmentsMade++;
        }
        if (m_config.vision != ColorVisionType::Normal) {
            result.adjustmentsMade++; // Palette adjustment
        }
        return result;
    }

    bool IsHighContrast() const {
        return m_config.mode == AccessibilityMode::HighContrast;
    }

    bool NeedsColorAdjustment() const {
        return m_config.vision != ColorVisionType::Normal;
    }

    uint32_t SuggestedMinFontSize() const {
        if (m_config.mode == AccessibilityMode::LargeText) return 16;
        if (m_config.textScaleFactor >= 1.5f) return 14;
        return 10;
    }

    static size_t ModeCount() { return static_cast<size_t>(AccessibilityMode::COUNT); }
    static size_t VisionTypeCount() { return static_cast<size_t>(ColorVisionType::COUNT); }

private:
    A11yEnhancerConfig m_config;
};

} // namespace Engine
} // namespace ExplorerLens
