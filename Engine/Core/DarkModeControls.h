// DarkModeControls.h — Dark Mode Owner-Draw Control Library
// Copyright (c) 2026 ExplorerLens Project
//
// Custom owner-draw implementations for common Windows controls in dark mode:
// checkbox, radio button, group box, combo box, list view, tab control,
// progress bar, and button. Designed for WTL integration.

#pragma once

#include <cstdint>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/// Control type for dark mode rendering
enum class DarkControlType : uint8_t {
    Checkbox   = 0,
    RadioButton = 1,
    GroupBox   = 2,
    Button     = 3,
    ComboBox   = 4,
    ListView   = 5,
    TabControl = 6,
    ProgressBar = 7,
    EditBox    = 8,
    Slider     = 9
};

/// Dark mode checkbox/radio drawing state
struct DarkCheckState {
    bool isChecked = false;
    bool isHovered = false;
    bool isPressed = false;
    bool isDisabled = false;
    bool isFocused = false;
};

/// Dark mode control renderer
class DarkModeControls {
public:
    static DarkModeControls& Instance() {
        static DarkModeControls inst;
        return inst;
    }

    /// Draw a dark mode checkbox (owner-draw)
    void DrawCheckbox(HDC hdc, const RECT& rc, const wchar_t* text,
                      const DarkCheckState& state) const {
        // Background
        HBRUSH bgBrush = CreateSolidBrush(m_bgColor);
        FillRect(hdc, &rc, bgBrush);
        DeleteObject(bgBrush);

        // Check box area (16x16 square)
        int boxSize = 16;
        int boxY = rc.top + (rc.bottom - rc.top - boxSize) / 2;
        RECT boxRect = { rc.left + 2, boxY, rc.left + 2 + boxSize, boxY + boxSize };

        // Box background
        COLORREF boxBg = state.isDisabled ? RGB(60, 60, 60) :
                         state.isHovered ? RGB(70, 70, 70) : RGB(55, 55, 55);
        HBRUSH boxBrush = CreateSolidBrush(boxBg);
        FillRect(hdc, &boxRect, boxBrush);
        DeleteObject(boxBrush);

        // Box border
        COLORREF borderColor = state.isFocused ? m_accentColor :
                               state.isHovered ? RGB(140, 140, 140) : RGB(100, 100, 100);
        HPEN pen = CreatePen(PS_SOLID, 1, borderColor);
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, boxRect.left, boxRect.top, boxRect.right, boxRect.bottom);
        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);

        // Checkmark
        if (state.isChecked) {
            HPEN checkPen = CreatePen(PS_SOLID, 2, m_accentColor);
            HPEN oldCP = (HPEN)SelectObject(hdc, checkPen);
            // Draw checkmark path
            int cx = boxRect.left + 4, cy = boxRect.top + 8;
            MoveToEx(hdc, cx, cy, nullptr);
            LineTo(hdc, cx + 3, cy + 3);
            LineTo(hdc, cx + 8, cy - 3);
            SelectObject(hdc, oldCP);
            DeleteObject(checkPen);
        }

        // Text
        if (text) {
            RECT textRect = { boxRect.right + 6, rc.top, rc.right, rc.bottom };
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, state.isDisabled ? RGB(100, 100, 100) : m_textColor);
            ::DrawTextW(hdc, text, -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        }
    }

    /// Draw a dark mode radio button (owner-draw)
    void DrawRadioButton(HDC hdc, const RECT& rc, const wchar_t* text,
                         const DarkCheckState& state) const {
        // Background
        HBRUSH bgBrush = CreateSolidBrush(m_bgColor);
        FillRect(hdc, &rc, bgBrush);
        DeleteObject(bgBrush);

        // Radio circle (16px diameter)
        int circSize = 16;
        int cy = rc.top + (rc.bottom - rc.top) / 2;
        int cx = rc.left + 2 + circSize / 2;

        // Outer circle
        COLORREF borderColor = state.isFocused ? m_accentColor : RGB(100, 100, 100);
        HPEN pen = CreatePen(PS_SOLID, 1, borderColor);
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        COLORREF fillColor = state.isHovered ? RGB(70, 70, 70) : RGB(55, 55, 55);
        HBRUSH fillBrush = CreateSolidBrush(fillColor);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, fillBrush);
        Ellipse(hdc, cx - circSize/2, cy - circSize/2,
                cx + circSize/2, cy + circSize/2);
        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
        DeleteObject(fillBrush);
        DeleteObject(pen);

        // Inner dot when selected
        if (state.isChecked) {
            HBRUSH dotBrush = CreateSolidBrush(m_accentColor);
            HBRUSH odb = (HBRUSH)SelectObject(hdc, dotBrush);
            HPEN dotPen = CreatePen(PS_SOLID, 1, m_accentColor);
            HPEN odp = (HPEN)SelectObject(hdc, dotPen);
            Ellipse(hdc, cx - 4, cy - 4, cx + 4, cy + 4);
            SelectObject(hdc, odp);
            SelectObject(hdc, odb);
            DeleteObject(dotPen);
            DeleteObject(dotBrush);
        }

        // Text
        if (text) {
            RECT textRect = { rc.left + 2 + circSize + 6, rc.top, rc.right, rc.bottom };
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, state.isDisabled ? RGB(100, 100, 100) : m_textColor);
            ::DrawTextW(hdc, text, -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        }
    }

    /// Draw a dark mode group box
    void DrawGroupBox(HDC hdc, const RECT& rc, const wchar_t* text) const {
        // Background
        HBRUSH bgBrush = CreateSolidBrush(m_bgColor);
        FillRect(hdc, &rc, bgBrush);
        DeleteObject(bgBrush);

        // Calculate text width
        SIZE textSize = {};
        if (text) {
            GetTextExtentPoint32W(hdc, text, static_cast<int>(wcslen(text)), &textSize);
        }

        // Border with gap for text
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(80, 80, 80));
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

        int textY = rc.top + 8;
        // Top line (with gap for text)
        MoveToEx(hdc, rc.left, textY, nullptr);
        LineTo(hdc, rc.left + 8, textY);
        MoveToEx(hdc, rc.left + 14 + textSize.cx, textY, nullptr);
        LineTo(hdc, rc.right, textY);
        // Sides and bottom
        MoveToEx(hdc, rc.left, textY, nullptr);
        LineTo(hdc, rc.left, rc.bottom);
        LineTo(hdc, rc.right, rc.bottom);
        LineTo(hdc, rc.right, textY);

        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);

        // Text
        if (text) {
            RECT textRect = { rc.left + 10, rc.top, rc.left + 12 + textSize.cx, rc.top + 16 };
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, m_textColor);
            ::DrawTextW(hdc, text, -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        }
    }

    /// Draw a dark mode flat button
    void DrawButton(HDC hdc, const RECT& rc, const wchar_t* text,
                    bool isDefault, const DarkCheckState& state) const {
        COLORREF bg = state.isPressed ? m_accentPressedColor :
                      state.isHovered ? m_accentHoverColor :
                      isDefault ? m_accentColor : m_surfaceColor;
        HBRUSH brush = CreateSolidBrush(bg);
        FillRect(hdc, &rc, brush);
        DeleteObject(brush);

        // Border
        HPEN pen = CreatePen(PS_SOLID, 1,
                             state.isFocused ? m_accentColor : RGB(80, 80, 80));
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 4, 4);
        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);

        // Text
        if (text) {
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, isDefault || state.isPressed ? RGB(255, 255, 255) : m_textColor);
            RECT textRect = rc;
            ::DrawTextW(hdc, text, -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
    }

    /// Draw a dark mode progress bar
    void DrawProgressBar(HDC hdc, const RECT& rc, float progress,
                         COLORREF barColor = 0) const {
        if (barColor == 0) barColor = m_accentColor;

        // Track
        HBRUSH trackBrush = CreateSolidBrush(RGB(55, 55, 55));
        FillRect(hdc, &rc, trackBrush);
        DeleteObject(trackBrush);

        // Bar
        float clampedProgress = (progress < 0.0f) ? 0.0f : (progress > 1.0f ? 1.0f : progress);
        int barWidth = static_cast<int>(static_cast<float>(rc.right - rc.left) * clampedProgress);
        if (barWidth > 0) {
            RECT barRect = { rc.left, rc.top, rc.left + barWidth, rc.bottom };
            HBRUSH barBrush = CreateSolidBrush(barColor);
            FillRect(hdc, &barRect, barBrush);
            DeleteObject(barBrush);
        }

        // Border
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(80, 80, 80));
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }

    /// Set custom colors
    void SetAccentColor(COLORREF c) { m_accentColor = c; }
    void SetBackgroundColor(COLORREF c) { m_bgColor = c; }

private:
    DarkModeControls() = default;

    COLORREF m_bgColor = RGB(32, 32, 32);
    COLORREF m_surfaceColor = RGB(44, 44, 44);
    COLORREF m_textColor = RGB(230, 230, 230);
    COLORREF m_accentColor = RGB(0, 120, 215);
    COLORREF m_accentHoverColor = RGB(30, 150, 245);
    COLORREF m_accentPressedColor = RGB(0, 90, 170);
};

} // namespace Engine
} // namespace ExplorerLens
