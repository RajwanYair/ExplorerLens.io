// HighContrastAdapter.h — Windows High Contrast Theme Support
// Copyright (c) 2026 ExplorerLens Project
//
// Detects Windows high contrast mode and provides adapted color values
// for UI rendering in accessibility scenarios.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <cstdint>
#include <string>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

enum class HCTheme : uint32_t {
    Standard = 0,
    WhiteOnBlack = 1,
    BlackOnWhite = 2,
    Custom = 3
};

struct HighContrastColors {
    COLORREF background = RGB(0, 0, 0);
    COLORREF foreground = RGB(255, 255, 255);
    COLORREF highlight = RGB(0, 120, 215);
    COLORREF highlightText = RGB(255, 255, 255);
    COLORREF buttonFace = RGB(128, 128, 128);
    COLORREF buttonText = RGB(255, 255, 255);
    COLORREF grayText = RGB(128, 128, 128);
    COLORREF hotTrack = RGB(0, 102, 204);

    bool IsDarkBackground() const {
        int r = GetRValue(background);
        int g = GetGValue(background);
        int b = GetBValue(background);
        return (r * 299 + g * 587 + b * 114) / 1000 < 128;
    }

    double ContrastRatio(COLORREF fg, COLORREF bg) const {
        auto luminance = [](COLORREF c) -> double {
            auto sRGB = [](double v) -> double {
                v /= 255.0;
                return v <= 0.03928 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4);
                };
            return 0.2126 * sRGB(GetRValue(c)) +
                0.7152 * sRGB(GetGValue(c)) +
                0.0722 * sRGB(GetBValue(c));
            };
        double l1 = luminance(fg);
        double l2 = luminance(bg);
        if (l1 < l2) std::swap(l1, l2);
        return (l1 + 0.05) / (l2 + 0.05);
    }
};

class HighContrastAdapter {
public:
    static HighContrastAdapter& Instance() {
        static HighContrastAdapter s;
        return s;
    }

    bool IsHighContrast() const {
        HIGHCONTRASTW hc{};
        hc.cbSize = sizeof(hc);
        if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0)) {
            return (hc.dwFlags & HCF_HIGHCONTRASTON) != 0;
        }
        return false;
    }

    HCTheme DetectTheme() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!IsHighContrast()) return HCTheme::Standard;

        HIGHCONTRASTW hc{};
        hc.cbSize = sizeof(hc);
        SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0);

        if (hc.lpszDefaultScheme) {
            std::wstring scheme(hc.lpszDefaultScheme);
            if (scheme.find(L"White") != std::wstring::npos &&
                scheme.find(L"Black") != std::wstring::npos) {
                // Check background to determine which variant
                COLORREF bg = GetSysColor(COLOR_WINDOW);
                if (GetRValue(bg) < 50 && GetGValue(bg) < 50 && GetBValue(bg) < 50)
                    return HCTheme::WhiteOnBlack;
                else
                    return HCTheme::BlackOnWhite;
            }
        }
        return HCTheme::Custom;
    }

    HighContrastColors GetColors() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        HighContrastColors colors;
        colors.background = GetSysColor(COLOR_WINDOW);
        colors.foreground = GetSysColor(COLOR_WINDOWTEXT);
        colors.highlight = GetSysColor(COLOR_HIGHLIGHT);
        colors.highlightText = GetSysColor(COLOR_HIGHLIGHTTEXT);
        colors.buttonFace = GetSysColor(COLOR_BTNFACE);
        colors.buttonText = GetSysColor(COLOR_BTNTEXT);
        colors.grayText = GetSysColor(COLOR_GRAYTEXT);
        colors.hotTrack = GetSysColor(COLOR_HOTLIGHT);
        return colors;
    }

    COLORREF AdaptColor(COLORREF original) const {
        if (!IsHighContrast()) return original;
        auto colors = GetColors();
        // Map to nearest HC color based on luminance
        int lum = (GetRValue(original) * 299 + GetGValue(original) * 587 +
            GetBValue(original) * 114) / 1000;
        return lum > 128 ? colors.foreground : colors.background;
    }

    bool AdaptLayout(uint32_t& borderWidth, uint32_t& padding) const {
        if (!IsHighContrast()) return false;
        // Increase visual boundaries for accessibility
        borderWidth = (borderWidth < 2) ? 2 : borderWidth;
        padding = (padding < 4) ? 4 : padding;
        return true;
    }

    bool MeetsWCAG_AA(COLORREF fg, COLORREF bg) const {
        HighContrastColors colors;
        return colors.ContrastRatio(fg, bg) >= 4.5;
    }

    bool Validate() const {
        auto colors = GetColors();
        double ratio = colors.ContrastRatio(colors.foreground, colors.background);
        // Standard mode doesn't need high contrast ratio
        if (!IsHighContrast()) return true;
        return ratio >= 3.0; // Minimum usable contrast
    }

private:
    HighContrastAdapter() = default;
    ~HighContrastAdapter() = default;
    HighContrastAdapter(const HighContrastAdapter&) = delete;
    HighContrastAdapter& operator=(const HighContrastAdapter&) = delete;

    mutable std::mutex m_mutex;
};

}
} // namespace ExplorerLens::Engine
