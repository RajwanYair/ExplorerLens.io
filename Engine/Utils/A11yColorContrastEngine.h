// A11yColorContrastEngine.h — Accessibility Color Contrast Engine (WCAG 2.2)
// Copyright (c) 2026 ExplorerLens Project
//
// Calculates WCAG 2.2 contrast ratios between foreground/background color pairs
// and suggests accessible alternatives when contrast is insufficient.
//
#pragma once
#include <string>
#include <array>
#include <vector>
#include <cmath>

namespace ExplorerLens {
namespace Engine {

enum class A11yWCAGLevel        { AA_Normal, AA_Large, AAA_Normal, AAA_Large };
enum class ContrastDecision { Pass, Fail };

struct WCAGThresholds {
    static constexpr double AA_NORMAL  = 4.5;
    static constexpr double AA_LARGE   = 3.0;
    static constexpr double AAA_NORMAL = 7.0;
    static constexpr double AAA_LARGE  = 4.5;
};

struct ColorRGBA8 {
    uint8_t r = 0, g = 0, b = 0, a = 255;
};

struct A11yContrastResult {
    double            ratio      = 1.0;
    ContrastDecision  decision   = ContrastDecision::Fail;
    A11yWCAGLevel         level      = A11yWCAGLevel::AA_Normal;
    std::string       summary;
};

class A11yColorContrastEngine {
public:
    explicit A11yColorContrastEngine() = default;

    A11yContrastResult Evaluate(ColorRGBA8 fg, ColorRGBA8 bg, A11yWCAGLevel level) const {
        double lFG = RelativeLuminance(fg);
        double lBG = RelativeLuminance(bg);
        double lighter = std::max(lFG, lBG);
        double darker  = std::min(lFG, lBG);
        double ratio   = (lighter + 0.05) / (darker + 0.05);

        double required = RequiredRatio(level);
        ContrastDecision dec = (ratio >= required) ? ContrastDecision::Pass : ContrastDecision::Fail;
        std::string summary = std::string(dec == ContrastDecision::Pass ? "PASS" : "FAIL")
            + " " + std::to_string(static_cast<int>(ratio * 10) / 10.0).substr(0, 3)
            + ":1 (need " + std::to_string(required) + ":1)";
        return { ratio, dec, level, summary };
    }

    ColorRGBA8 SuggestAccessibleForeground(ColorRGBA8 bg, A11yWCAGLevel level = A11yWCAGLevel::AA_Normal) const {
        double lBG = RelativeLuminance(bg);
        // White or black: return whichever has higher contrast
        double ratioWhite = (1.0 + 0.05) / (lBG + 0.05);
        double ratioBlack = (lBG + 0.05) / (0.0 + 0.05);
        return (ratioWhite >= ratioBlack) ? ColorRGBA8{255,255,255,255} : ColorRGBA8{0,0,0,255};
        (void)level;
    }

    static double RequiredRatio(A11yWCAGLevel level) noexcept {
        switch (level) {
        case A11yWCAGLevel::AA_Normal:  return WCAGThresholds::AA_NORMAL;
        case A11yWCAGLevel::AA_Large:   return WCAGThresholds::AA_LARGE;
        case A11yWCAGLevel::AAA_Normal: return WCAGThresholds::AAA_NORMAL;
        case A11yWCAGLevel::AAA_Large:  return WCAGThresholds::AAA_LARGE;
        }
        return 4.5;
    }

private:
    static double sRGBToLinear(uint8_t c) noexcept {
        double v = c / 255.0;
        return v <= 0.04045 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4);
    }
    static double RelativeLuminance(ColorRGBA8 col) noexcept {
        return 0.2126 * sRGBToLinear(col.r)
             + 0.7152 * sRGBToLinear(col.g)
             + 0.0722 * sRGBToLinear(col.b);
    }
};

} // namespace Engine
} // namespace ExplorerLens
