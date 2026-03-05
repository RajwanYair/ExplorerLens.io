// OwnerDrawThemeEngine.h — Custom Owner-Draw for WTL Dark Mode Controls
// Copyright (c) 2026 ExplorerLens Project
//
// Provides owner-draw rendering for WTL controls in dark mode,
// including checkboxes, radio buttons, group boxes, and buttons.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <cstdint>
#include <string>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

enum class ThemeControlType : uint8_t {
    Checkbox = 0,
    Radio = 1,
    GroupBox = 2,
    Button = 3,
    Label = 4,
    ComboBox = 5
};

struct OwnerDrawMetrics {
    uint32_t checkboxSize = 16;
    uint32_t radioSize = 16;
    uint32_t buttonPadding = 8;
    uint32_t groupBoxRadius = 4;
    uint32_t borderWidth = 1;
    uint32_t fontSize = 14;
};

struct OwnerDrawColors {
    COLORREF bgNormal = RGB(30, 30, 30);
    COLORREF bgHover = RGB(50, 50, 50);
    COLORREF bgPressed = RGB(60, 60, 60);
    COLORREF bgDisabled = RGB(40, 40, 40);
    COLORREF fgNormal = RGB(220, 220, 220);
    COLORREF fgDisabled = RGB(100, 100, 100);
    COLORREF accent = RGB(0, 120, 212);
    COLORREF border = RGB(80, 80, 80);
    COLORREF checkMark = RGB(255, 255, 255);
};

class OwnerDrawThemeEngine {
public:
    static OwnerDrawThemeEngine& Instance() { static OwnerDrawThemeEngine s; return s; }

    void SetMetrics(const OwnerDrawMetrics& m) { m_metrics = m; }
    void SetColors(const OwnerDrawColors& c) { m_colors = c; }

    bool DrawCheckbox(HDC hdc, const RECT& rc, bool checked, bool enabled, bool hover) {
        if (!hdc) return false;
        COLORREF bg = enabled ? (hover ? m_colors.bgHover : m_colors.bgNormal) : m_colors.bgDisabled;
        COLORREF fg = enabled ? m_colors.fgNormal : m_colors.fgDisabled;

        RECT box = { rc.left, rc.top,
                     rc.left + static_cast<LONG>(m_metrics.checkboxSize),
                     rc.top + static_cast<LONG>(m_metrics.checkboxSize) };
        HBRUSH bgBrush = ::CreateSolidBrush(bg);
        ::FillRect(hdc, &box, bgBrush);
        ::DeleteObject(bgBrush);

        HPEN pen = ::CreatePen(PS_SOLID, static_cast<int>(m_metrics.borderWidth),
            checked ? m_colors.accent : m_colors.border);
        HPEN oldPen = static_cast<HPEN>(::SelectObject(hdc, pen));
        ::MoveToEx(hdc, box.left, box.top, nullptr);
        ::LineTo(hdc, box.right - 1, box.top);
        ::LineTo(hdc, box.right - 1, box.bottom - 1);
        ::LineTo(hdc, box.left, box.bottom - 1);
        ::LineTo(hdc, box.left, box.top);
        ::SelectObject(hdc, oldPen);
        ::DeleteObject(pen);

        if (checked) {
            HBRUSH accentBrush = ::CreateSolidBrush(m_colors.accent);
            RECT inner = { box.left + 2, box.top + 2, box.right - 2, box.bottom - 2 };
            ::FillRect(hdc, &inner, accentBrush);
            ::DeleteObject(accentBrush);
        }
        (void)fg;
        return true;
    }

    bool DrawRadio(HDC hdc, const RECT& rc, bool selected, bool enabled, bool hover) {
        if (!hdc) return false;
        int cx = (rc.left + rc.right) / 2;
        int cy = (rc.top + rc.bottom) / 2;
        int radius = static_cast<int>(m_metrics.radioSize / 2);

        COLORREF bg = enabled ? (hover ? m_colors.bgHover : m_colors.bgNormal) : m_colors.bgDisabled;
        HBRUSH bgBrush = ::CreateSolidBrush(bg);
        HPEN pen = ::CreatePen(PS_SOLID, static_cast<int>(m_metrics.borderWidth), m_colors.border);
        HBRUSH oldBrush = static_cast<HBRUSH>(::SelectObject(hdc, bgBrush));
        HPEN oldPen = static_cast<HPEN>(::SelectObject(hdc, pen));
        ::Ellipse(hdc, cx - radius, cy - radius, cx + radius, cy + radius);
        ::SelectObject(hdc, oldBrush);
        ::SelectObject(hdc, oldPen);
        ::DeleteObject(bgBrush);
        ::DeleteObject(pen);

        if (selected) {
            HBRUSH accentBrush = ::CreateSolidBrush(m_colors.accent);
            HBRUSH ob = static_cast<HBRUSH>(::SelectObject(hdc, accentBrush));
            int ir = radius / 2;
            ::Ellipse(hdc, cx - ir, cy - ir, cx + ir, cy + ir);
            ::SelectObject(hdc, ob);
            ::DeleteObject(accentBrush);
        }
        return true;
    }

    bool DrawGroupBox(HDC hdc, const RECT& rc, const wchar_t* title, bool enabled) {
        if (!hdc) return false;
        HPEN pen = ::CreatePen(PS_SOLID, static_cast<int>(m_metrics.borderWidth), m_colors.border);
        HPEN oldPen = static_cast<HPEN>(::SelectObject(hdc, pen));
        HBRUSH hollow = static_cast<HBRUSH>(::GetStockObject(HOLLOW_BRUSH));
        HBRUSH oldBrush = static_cast<HBRUSH>(::SelectObject(hdc, hollow));
        ::RoundRect(hdc, rc.left, rc.top + 8, rc.right, rc.bottom,
            static_cast<int>(m_metrics.groupBoxRadius * 2),
            static_cast<int>(m_metrics.groupBoxRadius * 2));
        ::SelectObject(hdc, oldBrush);
        ::SelectObject(hdc, oldPen);
        ::DeleteObject(pen);

        if (title) {
            ::SetBkMode(hdc, TRANSPARENT);
            ::SetTextColor(hdc, enabled ? m_colors.fgNormal : m_colors.fgDisabled);
            RECT textRc = { rc.left + 10, rc.top, rc.right - 10, rc.top + 16 };
            ::DrawTextW(hdc, title, -1, &textRc, DT_LEFT | DT_SINGLELINE);
        }
        return true;
    }

    bool DrawButton(HDC hdc, const RECT& rc, const wchar_t* text, bool enabled, bool hover, bool pressed) {
        if (!hdc) return false;
        COLORREF bg = pressed ? m_colors.bgPressed : (hover ? m_colors.bgHover : m_colors.bgNormal);
        if (!enabled) bg = m_colors.bgDisabled;
        HBRUSH bgBrush = ::CreateSolidBrush(bg);
        ::FillRect(hdc, &rc, bgBrush);
        ::DeleteObject(bgBrush);

        HPEN pen = ::CreatePen(PS_SOLID, static_cast<int>(m_metrics.borderWidth),
            hover ? m_colors.accent : m_colors.border);
        HPEN oldPen = static_cast<HPEN>(::SelectObject(hdc, pen));
        ::MoveToEx(hdc, rc.left, rc.top, nullptr);
        ::LineTo(hdc, rc.right - 1, rc.top);
        ::LineTo(hdc, rc.right - 1, rc.bottom - 1);
        ::LineTo(hdc, rc.left, rc.bottom - 1);
        ::LineTo(hdc, rc.left, rc.top);
        ::SelectObject(hdc, oldPen);
        ::DeleteObject(pen);

        if (text) {
            ::SetBkMode(hdc, TRANSPARENT);
            ::SetTextColor(hdc, enabled ? m_colors.fgNormal : m_colors.fgDisabled);
            RECT textRc = rc;
            ::DrawTextW(hdc, text, -1, &textRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        return true;
    }

    const OwnerDrawMetrics& GetMetrics() const { return m_metrics; }
    const OwnerDrawColors& GetColors() const { return m_colors; }

    bool Validate() const {
        if (m_metrics.checkboxSize < 8 || m_metrics.checkboxSize > 64) return false;
        if (m_metrics.radioSize < 8 || m_metrics.radioSize > 64) return false;
        if (m_metrics.borderWidth == 0 || m_metrics.borderWidth > 4) return false;
        return true;
    }

private:
    OwnerDrawThemeEngine() = default;
    ~OwnerDrawThemeEngine() = default;
    OwnerDrawThemeEngine(const OwnerDrawThemeEngine&) = delete;
    OwnerDrawThemeEngine& operator=(const OwnerDrawThemeEngine&) = delete;

    OwnerDrawMetrics m_metrics{};
    OwnerDrawColors  m_colors{};
};

} // namespace Engine
} // namespace ExplorerLens
