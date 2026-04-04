// DarkModeEngine.h — Dark Mode / Theme Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Application-level theme management including Light, Dark, System, and
// High-Contrast modes with per-monitor DPI-aware color dispatch.
//
#pragma once
#include <cstddef>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

enum class AppTheme : uint8_t {
    Light,
    Dark,
    System,
    HighContrast,
    COUNT = 4
};

class DarkModeEngine
{
  public:
    static size_t ThemeCount() noexcept
    {
        return static_cast<size_t>(AppTheme::COUNT);
    }
    static const wchar_t* ThemeName(AppTheme t) noexcept
    {
        switch (t) {
            case AppTheme::Light:
                return L"Light";
            case AppTheme::Dark:
                return L"Dark";
            case AppTheme::System:
                return L"System";
            case AppTheme::HighContrast:
                return L"High Contrast";
            default:
                return L"Unknown";
        }
    }
};

// -- Dark-mode UI control types -----------------------------------------------

enum class DarkControlType : uint8_t {
    Checkbox = 0,
    RadioButton = 1,
    Button = 2,
    Toggle = 3,
    TextBox = 4,
    Combobox = 5,
    ListBox = 6,
    TabControl = 7,
    TreeView = 8,
    Slider = 9
};

struct DarkCheckState
{
    bool isChecked = false;
    bool isHovered = false;
    bool isPressed = false;
    bool isDisabled = false;
    bool isFocused = false;
};

class DarkModeControls
{
  public:
    static DarkModeControls& Instance() noexcept
    {
        static DarkModeControls s_inst;
        return s_inst;
    }
    void SetAccentColor(uint32_t color) noexcept
    {
        m_accentColor = color;
    }
    void SetBackgroundColor(uint32_t color) noexcept
    {
        m_bgColor = color;
    }
    uint32_t GetAccentColor() const noexcept
    {
        return m_accentColor;
    }
    uint32_t GetBackgroundColor() const noexcept
    {
        return m_bgColor;
    }

  private:
    DarkModeControls() = default;
    uint32_t m_accentColor = 0xFF0078D7;
    uint32_t m_bgColor = 0xFF1E1E1E;
};

}  // namespace Engine
}  // namespace ExplorerLens
