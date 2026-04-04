// HighContrastThemeAdapter.h — High Contrast Theme Adapter
// Copyright (c) 2026 ExplorerLens Project
//
// Adapts ExplorerLens thumbnail UI to Windows High Contrast / forced-color
// modes (prefers-color-scheme: forced-colors). Remaps palette, border, and
// focus ring tokens to system colors.
//
#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

enum class HighContrastMode {
    Off,
    Black,
    White,
    Custom
};

struct SystemColorTokens
{
    uint32_t WindowText = 0xFF000000;
    uint32_t Window = 0xFFFFFFFF;
    uint32_t ButtonText = 0xFF000000;
    uint32_t ButtonFace = 0xFFEEEEEE;
    uint32_t Highlight = 0xFF0078D4;
    uint32_t HighlightText = 0xFFFFFFFF;
    uint32_t GrayText = 0xFF767676;
    uint32_t LinkText = 0xFF0066CC;
};

struct HCAdaptedPalette
{
    uint32_t thumbBackground = 0;
    uint32_t thumbBorder = 0;
    uint32_t thumbFocusRing = 0;
    uint32_t labelText = 0;
    uint32_t selectedBg = 0;
    HighContrastMode activeMode = HighContrastMode::Off;
};

class HighContrastThemeAdapter
{
  public:
    HighContrastThemeAdapter() = default;

    bool Initialize()
    {
        m_ready = true;
        return true;
    }
    bool IsReady() const
    {
        return m_ready;
    }

    HighContrastMode DetectSystemMode() const
    {
#if defined(_WIN32)
        return HighContrastMode::Black;  // Real impl uses SystemParametersInfo
#else
        return HighContrastMode::Off;
#endif
    }

    HCAdaptedPalette AdaptPalette(HighContrastMode mode, const SystemColorTokens& sys = {}) const
    {
        HCAdaptedPalette p;
        p.activeMode = mode;
        if (mode == HighContrastMode::Off) {
            p.thumbBackground = 0xFF1E1E2E;
            p.thumbBorder = 0xFF575268;
            p.thumbFocusRing = 0xFF89B4FA;
            p.labelText = 0xFFCDD6F4;
            p.selectedBg = 0xFF313244;
        } else {
            p.thumbBackground = sys.Window;
            p.thumbBorder = sys.WindowText;
            p.thumbFocusRing = sys.Highlight;
            p.labelText = sys.WindowText;
            p.selectedBg = sys.Highlight;
        }
        return p;
    }

    bool IsHighContrastActive() const
    {
        return DetectSystemMode() != HighContrastMode::Off;
    }

    void Shutdown()
    {
        m_ready = false;
    }

  private:
    bool m_ready = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
