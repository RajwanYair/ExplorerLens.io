// RTLTextAdapter.h — Right-to-Left Layout Mirroring for Manager UI
// Copyright (c) 2026 ExplorerLens Project
//
// Detects RTL locales (Arabic, Hebrew) and provides mirroring helpers for
// WTL/Win32 dialog layouts, text rendering, and thumbnail overlay coordinates.
//
#pragma once

#include <windows.h>
#include <string>
#include <cstdint>

namespace ExplorerLens { namespace Engine { namespace i18n {

enum class LayoutDirection : uint8_t {
    LTR = 0,
    RTL = 1
};

struct MirroredRect {
    int left, top, right, bottom;
};

class RTLTextAdapter {
public:
    static RTLTextAdapter& Instance() {
        static RTLTextAdapter inst;
        return inst;
    }

    LayoutDirection Direction() const { return m_dir; }
    bool IsRTL() const { return m_dir == LayoutDirection::RTL; }

    // Mirror a RECT within a parent container width
    RECT MirrorRect(const RECT& r, int containerWidth) const {
        if (!IsRTL()) return r;
        return RECT{
            containerWidth - r.right,
            r.top,
            containerWidth - r.left,
            r.bottom
        };
    }

    // Apply RTL extended style to a Win32 window
    void ApplyRTLStyle(HWND hwnd) const {
        if (!IsRTL()) return;
        LONG_PTR ex = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
        SetWindowLongPtrW(hwnd, GWL_EXSTYLE,
            ex | WS_EX_RTLREADING | WS_EX_LAYOUTRTL);
    }

    // Wrap a string with Unicode RLE/LRE directional marks for mixed text
    std::wstring WrapBidi(const std::wstring& text) const {
        if (!IsRTL()) return text;
        // U+202B RIGHT-TO-LEFT EMBEDDING + U+202C POP DIRECTIONAL FORMATTING
        return std::wstring(1, L'\u202B') + text + std::wstring(1, L'\u202C');
    }

    // Align text flags for DrawText
    DWORD DrawTextFlags(DWORD baseFlags) const {
        if (!IsRTL()) return baseFlags;
        // Replace DT_LEFT with DT_RIGHT and add DT_RTLREADING
        baseFlags &= ~DT_LEFT;
        return baseFlags | DT_RIGHT | DT_RTLREADING;
    }

    // Mirror thumbnail overlay X coordinate within thumbnail width
    int MirrorX(int x, int overlayWidth, int thumbWidth) const {
        return IsRTL() ? (thumbWidth - x - overlayWidth) : x;
    }

    // Detect whether the system or font is RTL
    void DetectFromSystemLocale() {
        wchar_t localeName[LOCALE_NAME_MAX_LENGTH] = {};
        GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH);
        // Arabic and Hebrew are the primary RTL locales supported
        std::wstring n = localeName;
        m_dir = (n.find(L"ar-") != std::wstring::npos ||
                 n.find(L"he-") != std::wstring::npos ||
                 n.find(L"fa-") != std::wstring::npos ||
                 n.find(L"ur-") != std::wstring::npos)
                ? LayoutDirection::RTL
                : LayoutDirection::LTR;
    }

    void SetDirection(LayoutDirection dir) { m_dir = dir; }

private:
    RTLTextAdapter() { DetectFromSystemLocale(); }
    LayoutDirection m_dir = LayoutDirection::LTR;
};

}}} // namespace ExplorerLens::Engine::i18n
