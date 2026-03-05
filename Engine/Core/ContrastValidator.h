// ContrastValidator.h — WCAG Contrast Ratio Validation Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Validates text-on-background contrast ratios against WCAG 2.1 AA/AAA
// standards. Used by the theme engine to ensure all rendered text is
// readable regardless of system theme.
//
#pragma once

#include <cstdint>
#include <cmath>
#include <string>

namespace ExplorerLens {
namespace Engine {

/// WCAG conformance level
enum class ValidatorWCAGLevel : uint8_t {
    Fail = 0,   // Below 3:1
    A = 1,   // 3:1+ (large text minimum)
    AA = 2,   // 4.5:1+ (normal text minimum)
    AAA = 3    // 7:1+ (enhanced)
};

/// Contrast validation result
struct ContrastResult {
    float ratio = 0.0f;
    ValidatorWCAGLevel level = ValidatorWCAGLevel::Fail;
    bool passesAA = false;
    bool passesAAA = false;
};

/// WCAG contrast ratio calculator and validator
class ContrastValidator {
public:
    /// Calculate relative luminance of an sRGB color (0xRRGGBB)
    static float RelativeLuminance(uint32_t rgb) {
        auto linearize = [](float c) -> float {
            return (c <= 0.03928f) ? c / 12.92f
                : std::pow((c + 0.055f) / 1.055f, 2.4f);
            };
        float r = linearize(static_cast<float>((rgb >> 16) & 0xFF) / 255.0f);
        float g = linearize(static_cast<float>((rgb >> 8) & 0xFF) / 255.0f);
        float b = linearize(static_cast<float>(rgb & 0xFF) / 255.0f);
        return 0.2126f * r + 0.7152f * g + 0.0722f * b;
    }

    /// Calculate contrast ratio between two colors
    static float ContrastRatio(uint32_t fg, uint32_t bg) {
        float l1 = RelativeLuminance(fg);
        float l2 = RelativeLuminance(bg);
        float lighter = (l1 > l2) ? l1 : l2;
        float darker = (l1 > l2) ? l2 : l1;
        return (lighter + 0.05f) / (darker + 0.05f);
    }

    /// Validate contrast and return detailed result
    static ContrastResult Validate(uint32_t fgColor, uint32_t bgColor) {
        ContrastResult r;
        r.ratio = ContrastRatio(fgColor, bgColor);
        if (r.ratio >= 7.0f)      r.level = ValidatorWCAGLevel::AAA;
        else if (r.ratio >= 4.5f) r.level = ValidatorWCAGLevel::AA;
        else if (r.ratio >= 3.0f) r.level = ValidatorWCAGLevel::A;
        else                       r.level = ValidatorWCAGLevel::Fail;
        r.passesAA = r.ratio >= 4.5f;
        r.passesAAA = r.ratio >= 7.0f;
        return r;
    }

    /// Suggest a lighter or darker foreground to meet target ratio
    static uint32_t SuggestForeground(uint32_t bgColor, float targetRatio = 4.5f) {
        float bgLum = RelativeLuminance(bgColor);
        // If bg is dark, suggest light fg; if bg is light, suggest dark fg
        if (bgLum < 0.5f) {
            // Need a light foreground
            float neededLum = targetRatio * (bgLum + 0.05f) - 0.05f;
            uint8_t v = static_cast<uint8_t>(
                (neededLum < 1.0f ? neededLum : 1.0f) * 255.0f);
            return (static_cast<uint32_t>(v) << 16) |
                (static_cast<uint32_t>(v) << 8) | v;
        }
        // Need a dark foreground
        float neededLum = (bgLum + 0.05f) / targetRatio - 0.05f;
        uint8_t v = static_cast<uint8_t>(
            (neededLum > 0.0f ? neededLum : 0.0f) * 255.0f);
        return (static_cast<uint32_t>(v) << 16) |
            (static_cast<uint32_t>(v) << 8) | v;
    }

    static const wchar_t* LevelName(ValidatorWCAGLevel l) {
        switch (l) {
        case ValidatorWCAGLevel::Fail: return L"Fail";
        case ValidatorWCAGLevel::A:    return L"A";
        case ValidatorWCAGLevel::AA:   return L"AA";
        case ValidatorWCAGLevel::AAA:  return L"AAA";
        default:              return L"Unknown";
        }
    }

    static size_t LevelCount() { return 4; }
};

} // namespace Engine
} // namespace ExplorerLens
