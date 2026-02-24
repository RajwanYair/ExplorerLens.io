#pragma once
// Theme Engine — dark/light/custom theme management for shell extension UI
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

/// Built-in theme types
enum class ThemeType : uint32_t {
    System     = 0,    ///< Follow Windows system setting
    Light      = 1,
    Dark       = 2,
    HighContrast = 3,
    Custom     = 4,
    COUNT      = 5
};

/// Color channel (RGBA)
struct ThemeColor {
    uint8_t r = 0, g = 0, b = 0, a = 255;

    bool operator==(const ThemeColor& o) const {
        return r == o.r && g == o.g && b == o.b && a == o.a;
    }
};

/// Named color slot in a theme
struct ThemeSlot {
    std::wstring name;
    ThemeColor   color;
};

/// Complete theme definition
struct ThemeDefinition {
    std::wstring name;
    ThemeType    type = ThemeType::Dark;
    ThemeColor   background  {30, 30, 30, 255};
    ThemeColor   foreground  {220, 220, 220, 255};
    ThemeColor   accent      {0, 120, 215, 255};
    ThemeColor   border      {60, 60, 60, 255};
    std::vector<ThemeSlot> customSlots;
};

/// Manages theme selection and custom theme storage
class ThemeEngine {
public:
    ThemeEngine();

    static const wchar_t* GetThemeTypeName(ThemeType type);
    static uint32_t GetThemeTypeCount() { return static_cast<uint32_t>(ThemeType::COUNT); }

    /// Get the active theme
    const ThemeDefinition& GetActiveTheme() const { return m_activeTheme; }
    /// Set the active theme type (uses built-in defaults)
    void SetThemeType(ThemeType type);
    /// Register a custom theme
    void RegisterCustomTheme(const ThemeDefinition& theme);
    /// Get all registered themes
    const std::vector<ThemeDefinition>& GetRegisteredThemes() const { return m_themes; }

    /// Create default dark theme
    static ThemeDefinition CreateDarkTheme();
    /// Create default light theme
    static ThemeDefinition CreateLightTheme();
    /// Create high-contrast theme
    static ThemeDefinition CreateHighContrastTheme();

private:
    ThemeDefinition m_activeTheme;
    std::vector<ThemeDefinition> m_themes;
};

}} // namespace ExplorerLens::Engine

