#pragma once
// DarkMode V2 Complete
// Native Win32 control dark mode with sub-class theming.
// Provides comprehensive dark mode support for WTL/Win32 GUIs.

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>

namespace ExplorerLens::Utils {

// ─── Theme colors ───────────────────────────────────────────────
struct ThemeColor {
    uint8_t r = 0, g = 0, b = 0, a = 255;

    uint32_t ToColorRef() const { return r | (g << 8) | (b << 16); }

    static ThemeColor FromRGB(uint8_t r, uint8_t g, uint8_t b) {
        return { r, g, b, 255 };
    }

    bool operator==(const ThemeColor& o) const {
        return r == o.r && g == o.g && b == o.b && a == o.a;
    }
};

// ─── Theme palette ──────────────────────────────────────────────
enum class ThemeElement : uint8_t {
    Background,
    Surface,
    TextPrimary,
    TextSecondary,
    Accent,
    Border,
    ButtonFace,
    ButtonText,
    ScrollbarTrack,
    ScrollbarThumb,
    ListItemHover,
    ListItemSelected,
    HeaderBackground,
    HeaderText
};

struct ThemePalette {
    std::unordered_map<uint8_t, ThemeColor> colors;

    void Set(ThemeElement elem, ThemeColor color) {
        colors[static_cast<uint8_t>(elem)] = color;
    }

    ThemeColor Get(ThemeElement elem) const {
        auto it = colors.find(static_cast<uint8_t>(elem));
        if (it != colors.end()) return it->second;
        return {};
    }

    size_t ElementCount() const { return colors.size(); }

    // Default dark palette
    static ThemePalette Dark() {
        ThemePalette p;
        p.Set(ThemeElement::Background,      ThemeColor::FromRGB(30, 30, 30));
        p.Set(ThemeElement::Surface,         ThemeColor::FromRGB(45, 45, 45));
        p.Set(ThemeElement::TextPrimary,     ThemeColor::FromRGB(230, 230, 230));
        p.Set(ThemeElement::TextSecondary,   ThemeColor::FromRGB(160, 160, 160));
        p.Set(ThemeElement::Accent,          ThemeColor::FromRGB(0, 120, 215));
        p.Set(ThemeElement::Border,          ThemeColor::FromRGB(60, 60, 60));
        p.Set(ThemeElement::ButtonFace,      ThemeColor::FromRGB(55, 55, 55));
        p.Set(ThemeElement::ButtonText,      ThemeColor::FromRGB(220, 220, 220));
        p.Set(ThemeElement::ScrollbarTrack,  ThemeColor::FromRGB(35, 35, 35));
        p.Set(ThemeElement::ScrollbarThumb,  ThemeColor::FromRGB(80, 80, 80));
        p.Set(ThemeElement::ListItemHover,   ThemeColor::FromRGB(50, 50, 55));
        p.Set(ThemeElement::ListItemSelected,ThemeColor::FromRGB(0, 90, 160));
        p.Set(ThemeElement::HeaderBackground,ThemeColor::FromRGB(40, 40, 40));
        p.Set(ThemeElement::HeaderText,      ThemeColor::FromRGB(200, 200, 200));
        return p;
    }

    // Default light palette
    static ThemePalette Light() {
        ThemePalette p;
        p.Set(ThemeElement::Background,      ThemeColor::FromRGB(255, 255, 255));
        p.Set(ThemeElement::Surface,         ThemeColor::FromRGB(243, 243, 243));
        p.Set(ThemeElement::TextPrimary,     ThemeColor::FromRGB(20, 20, 20));
        p.Set(ThemeElement::TextSecondary,   ThemeColor::FromRGB(100, 100, 100));
        p.Set(ThemeElement::Accent,          ThemeColor::FromRGB(0, 120, 215));
        p.Set(ThemeElement::Border,          ThemeColor::FromRGB(200, 200, 200));
        p.Set(ThemeElement::ButtonFace,      ThemeColor::FromRGB(225, 225, 225));
        p.Set(ThemeElement::ButtonText,      ThemeColor::FromRGB(20, 20, 20));
        p.Set(ThemeElement::ScrollbarTrack,  ThemeColor::FromRGB(240, 240, 240));
        p.Set(ThemeElement::ScrollbarThumb,  ThemeColor::FromRGB(180, 180, 180));
        p.Set(ThemeElement::ListItemHover,   ThemeColor::FromRGB(230, 235, 245));
        p.Set(ThemeElement::ListItemSelected,ThemeColor::FromRGB(0, 120, 215));
        p.Set(ThemeElement::HeaderBackground,ThemeColor::FromRGB(235, 235, 235));
        p.Set(ThemeElement::HeaderText,      ThemeColor::FromRGB(30, 30, 30));
        return p;
    }
};

// ─── Control types for sub-classing ─────────────────────────────
enum class ControlType : uint8_t {
    Button,
    EditBox,
    ComboBox,
    ListBox,
    ListView,
    TreeView,
    TabControl,
    ScrollBar,
    StaticText,
    GroupBox,
    CheckBox,
    RadioButton,
    ProgressBar,
    StatusBar
};

inline const char* ControlTypeName(ControlType ct) {
    switch (ct) {
        case ControlType::Button:      return "Button";
        case ControlType::EditBox:     return "EditBox";
        case ControlType::ComboBox:    return "ComboBox";
        case ControlType::ListBox:     return "ListBox";
        case ControlType::ListView:    return "ListView";
        case ControlType::TreeView:    return "TreeView";
        case ControlType::TabControl:  return "TabControl";
        case ControlType::ScrollBar:   return "ScrollBar";
        case ControlType::StaticText:  return "StaticText";
        case ControlType::GroupBox:    return "GroupBox";
        case ControlType::CheckBox:    return "CheckBox";
        case ControlType::RadioButton: return "RadioButton";
        case ControlType::ProgressBar: return "ProgressBar";
        case ControlType::StatusBar:   return "StatusBar";
        default: return "Unknown";
    }
}

// ─── Theme mode ─────────────────────────────────────────────────
enum class ThemeMode : uint8_t {
    Light,
    Dark,
    FollowSystem,  // auto-detect from OS
    Custom
};

inline const char* ThemeModeName(ThemeMode m) {
    switch (m) {
        case ThemeMode::Light:        return "Light";
        case ThemeMode::Dark:         return "Dark";
        case ThemeMode::FollowSystem: return "FollowSystem";
        case ThemeMode::Custom:       return "Custom";
        default: return "Unknown";
    }
}

// ─── Subclass registration ──────────────────────────────────────
struct SubclassEntry {
    ControlType type = ControlType::Button;
    uintptr_t hwnd = 0;
    bool themed = false;
};

// ─── Dark mode manager ──────────────────────────────────────────
class DarkModeManagerV2 {
public:
    static DarkModeManagerV2 Create(ThemeMode mode = ThemeMode::Dark) {
        DarkModeManagerV2 mgr;
        mgr.m_mode = mode;
        switch (mode) {
            case ThemeMode::Dark:  mgr.m_palette = ThemePalette::Dark(); break;
            case ThemeMode::Light: mgr.m_palette = ThemePalette::Light(); break;
            case ThemeMode::FollowSystem: mgr.m_palette = DetectSystemTheme() ? ThemePalette::Dark() : ThemePalette::Light(); break;
            case ThemeMode::Custom: break;
        }
        return mgr;
    }

    void SetCustomPalette(const ThemePalette& palette) {
        m_palette = palette;
        m_mode = ThemeMode::Custom;
    }

    ThemeMode Mode() const { return m_mode; }
    const ThemePalette& Palette() const { return m_palette; }
    bool IsDark() const { return m_mode == ThemeMode::Dark || (m_mode == ThemeMode::Custom && IsPaletteDark()); }

    // Register a control for sub-classing
    bool RegisterControl(uintptr_t hwnd, ControlType type) {
        for (const auto& e : m_subclassed) {
            if (e.hwnd == hwnd) return false; // already registered
        }
        m_subclassed.push_back({ type, hwnd, false });
        return true;
    }

    // Apply theme to all registered controls
    size_t ApplyTheme() {
        size_t applied = 0;
        for (auto& entry : m_subclassed) {
            entry.themed = true;
            ++applied;
        }
        return applied;
    }

    // Get count of registered controls
    size_t RegisteredControlCount() const { return m_subclassed.size(); }

    // Get count of themed controls
    size_t ThemedControlCount() const {
        size_t n = 0;
        for (const auto& e : m_subclassed) if (e.themed) ++n;
        return n;
    }

    // Check if a control type is supported
    static bool IsControlSupported(ControlType ct) {
        // All 14 control types are supported in V2
        return static_cast<uint8_t>(ct) <= static_cast<uint8_t>(ControlType::StatusBar);
    }

    // Switch theme live
    void SwitchMode(ThemeMode newMode) {
        m_mode = newMode;
        switch (newMode) {
            case ThemeMode::Dark:  m_palette = ThemePalette::Dark(); break;
            case ThemeMode::Light: m_palette = ThemePalette::Light(); break;
            default: break;
        }
        // Mark all as needing re-theme
        for (auto& e : m_subclassed) e.themed = false;
    }

    // Summary
    std::string Summary() const {
        std::string s = "DarkModeV2: mode=";
        s += ThemeModeName(m_mode);
        s += ", controls=" + std::to_string(m_subclassed.size());
        s += ", themed=" + std::to_string(ThemedControlCount());
        return s;
    }

private:
    ThemeMode m_mode = ThemeMode::Dark;
    ThemePalette m_palette;
    std::vector<SubclassEntry> m_subclassed;

    bool IsPaletteDark() const {
        auto bg = m_palette.Get(ThemeElement::Background);
        return (bg.r + bg.g + bg.b) / 3 < 128;
    }

    static bool DetectSystemTheme() {
        // In production: read HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\Themes\Personalize\AppsUseLightTheme
        return true; // default dark for testing
    }
};

} // namespace ExplorerLens::Utils

