// ThemeEngineV3.h — Theme Engine V3
// Copyright (c) 2026 ExplorerLens Project
//
// Token-based theming system supporting dark/light/high-contrast/custom modes.
// Manages CSS-variable-style design tokens and validates WCAG contrast ratios.
//
#pragma once
#include <string>
#include <unordered_map>
#include <cmath>

namespace ExplorerLens { namespace Engine {

enum class ThemeMode { Light, Dark, HighContrast, Custom };

struct ColorToken {
    uint8_t r = 0, g = 0, b = 0, a = 255;
};

struct ThemeConfig {
    ThemeMode mode = ThemeMode::Dark;
    std::unordered_map<std::string, ColorToken> tokens;
};

class ThemeEngineV3 {
public:
    ThemeEngineV3() = default;

    bool Initialize() { BuildDefaultDark(); m_ready = true; return true; }
    bool IsReady() const { return m_ready; }

    void SetMode(ThemeMode mode) {
        m_config.mode = mode;
        if (mode == ThemeMode::Light) BuildDefaultLight();
        else if (mode == ThemeMode::Dark) BuildDefaultDark();
        else if (mode == ThemeMode::HighContrast) BuildHighContrast();
    }

    ThemeMode GetMode() const { return m_config.mode; }

    void SetToken(const std::string& key, const ColorToken& color) {
        m_config.tokens[key] = color;
    }

    ColorToken GetToken(const std::string& key, const ColorToken& fallback = {}) const {
        auto it = m_config.tokens.find(key);
        return it != m_config.tokens.end() ? it->second : fallback;
    }

    float GetContrastRatio(const ColorToken& fg, const ColorToken& bg) const {
        auto relLum = [](const ColorToken& c) -> double {
            auto chan = [](uint8_t v) -> double {
                double s = v / 255.0;
                return s <= 0.04045 ? s / 12.92 : std::pow((s + 0.055) / 1.055, 2.4);
            };
            return 0.2126 * chan(c.r) + 0.7152 * chan(c.g) + 0.0722 * chan(c.b);
        };
        double l1 = relLum(fg), l2 = relLum(bg);
        if (l1 < l2) { double tmp = l1; l1 = l2; l2 = tmp; }
        return static_cast<float>((l1 + 0.05) / (l2 + 0.05));
    }

    bool PassesWCAGAA(const ColorToken& fg, const ColorToken& bg) const {
        return GetContrastRatio(fg, bg) >= 4.5f;
    }

    void Shutdown() { m_ready = false; }

private:
    ThemeConfig m_config;
    bool        m_ready = false;

    void BuildDefaultDark() {
        m_config.tokens["--bg-primary"] = {0x1e, 0x1e, 0x2e, 0xff};
        m_config.tokens["--fg-primary"] = {0xcd, 0xd6, 0xf4, 0xff};
        m_config.tokens["--accent"]     = {0x89, 0xb4, 0xfa, 0xff};
    }
    void BuildDefaultLight() {
        m_config.tokens["--bg-primary"] = {0xff, 0xff, 0xff, 0xff};
        m_config.tokens["--fg-primary"] = {0x11, 0x11, 0x11, 0xff};
        m_config.tokens["--accent"]     = {0x1e, 0x66, 0xf5, 0xff};
    }
    void BuildHighContrast() {
        m_config.tokens["--bg-primary"] = {0x00, 0x00, 0x00, 0xff};
        m_config.tokens["--fg-primary"] = {0xff, 0xff, 0x00, 0xff};
        m_config.tokens["--accent"]     = {0xff, 0xff, 0xff, 0xff};
    }
};

}} // namespace ExplorerLens::Engine
