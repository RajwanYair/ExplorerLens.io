#include "ThemeEngine.h"

namespace DarkThumbs { namespace Engine {

ThemeEngine::ThemeEngine() {
    m_activeTheme = CreateDarkTheme();
    m_themes.push_back(CreateDarkTheme());
    m_themes.push_back(CreateLightTheme());
    m_themes.push_back(CreateHighContrastTheme());
}

const wchar_t* ThemeEngine::GetThemeTypeName(ThemeType type) {
    switch (type) {
        case ThemeType::System:       return L"System";
        case ThemeType::Light:        return L"Light";
        case ThemeType::Dark:         return L"Dark";
        case ThemeType::HighContrast: return L"High Contrast";
        case ThemeType::Custom:       return L"Custom";
        default:                      return L"Unknown";
    }
}

void ThemeEngine::SetThemeType(ThemeType type) {
    switch (type) {
        case ThemeType::Light:        m_activeTheme = CreateLightTheme(); break;
        case ThemeType::Dark:         m_activeTheme = CreateDarkTheme(); break;
        case ThemeType::HighContrast: m_activeTheme = CreateHighContrastTheme(); break;
        default:                      m_activeTheme = CreateDarkTheme(); break;
    }
}

void ThemeEngine::RegisterCustomTheme(const ThemeDefinition& theme) {
    m_themes.push_back(theme);
}

ThemeDefinition ThemeEngine::CreateDarkTheme() {
    ThemeDefinition t;
    t.name = L"Dark";
    t.type = ThemeType::Dark;
    t.background = {30, 30, 30, 255};
    t.foreground = {220, 220, 220, 255};
    t.accent     = {0, 120, 215, 255};
    t.border     = {60, 60, 60, 255};
    return t;
}

ThemeDefinition ThemeEngine::CreateLightTheme() {
    ThemeDefinition t;
    t.name = L"Light";
    t.type = ThemeType::Light;
    t.background = {255, 255, 255, 255};
    t.foreground = {30, 30, 30, 255};
    t.accent     = {0, 99, 177, 255};
    t.border     = {200, 200, 200, 255};
    return t;
}

ThemeDefinition ThemeEngine::CreateHighContrastTheme() {
    ThemeDefinition t;
    t.name = L"High Contrast";
    t.type = ThemeType::HighContrast;
    t.background = {0, 0, 0, 255};
    t.foreground = {255, 255, 255, 255};
    t.accent     = {255, 255, 0, 255};
    t.border     = {255, 255, 255, 255};
    return t;
}

}} // namespace DarkThumbs::Engine
