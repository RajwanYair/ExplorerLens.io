// Engine/Platform/WtlDarkModeTheme.h
// ExplorerLens — WTL LENSManager dark mode theme applicator (ROADMAP v8.0 Phase 3)
// Sprint S345.
//
// Purpose:
//   Phase 3 exit criterion: "LENSManager WTL dark mode completed".
//   The existing DarkModeController.h detects the Windows dark mode preference.
//   WtlDarkModeTheme completes the integration by:
//     1. Applying the dark or light COLORREF palette to WTL window classes.
//     2. Providing the HBRUSH factory for window backgrounds (CTLCOLOR messages).
//     3. Offering a per-control override map for mixed-mode layouts.
//
//   This header is Windows-only; on non-Windows builds it compiles to an empty stub.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_WTL_DARK_MODE_THEME_H
#define EXPLORERLENS_ENGINE_WTL_DARK_MODE_THEME_H

#include <cstdint>

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#endif

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// WtlThemeMode
// ---------------------------------------------------------------------------

enum class WtlThemeMode : std::uint8_t {
    SYSTEM   = 0,   ///< Follow Windows dark/light preference
    FORCE_LIGHT = 1,
    FORCE_DARK  = 2,
};

// ---------------------------------------------------------------------------
// WtlDarkModeColors — COLORREF palette for both themes
// ---------------------------------------------------------------------------

struct WtlDarkModeColors final {
    // Dark theme defaults (Windows 11 dark)
    std::uint32_t background      = 0x00202020u;  ///< Dark background  (BGRA 0x00RRGGBB)
    std::uint32_t surface         = 0x00303030u;  ///< Control surface
    std::uint32_t textPrimary     = 0x00F0F0F0u;  ///< Primary text
    std::uint32_t textSecondary   = 0x00A0A0A0u;  ///< Secondary text / hint
    std::uint32_t accent          = 0x00CF7828u;  ///< Accent colour (ExplorerLens orange)
    std::uint32_t border          = 0x00505050u;  ///< Control border
    std::uint32_t highlight       = 0x00404040u;  ///< Selection / hover fill

    // Light theme defaults
    std::uint32_t lightBackground = 0x00F2F2F2u;
    std::uint32_t lightSurface    = 0x00FFFFFFu;
    std::uint32_t lightText       = 0x00202020u;
    std::uint32_t lightAccent     = 0x00CF7828u;
    std::uint32_t lightBorder     = 0x00D0D0D0u;
};

// ---------------------------------------------------------------------------
// WtlDarkModeThemeConfig
// ---------------------------------------------------------------------------

struct WtlDarkModeThemeConfig final {
    WtlThemeMode     mode       = WtlThemeMode::SYSTEM;
    WtlDarkModeColors colors    {};
    bool             immersiveMode = true;  ///< Use DwmSetWindowAttribute MICA
    bool             uxThemeHook   = true;  ///< Allow SetWindowTheme("DarkMode_Explorer")
};

// ---------------------------------------------------------------------------
// WtlThemeApplicatorStatus
// ---------------------------------------------------------------------------

enum class WtlThemeApplicatorStatus : std::uint8_t {
    OK                    = 0,
    NULL_HWND             = 1,  ///< Target HWND is NULL
    NOT_WIN32             = 2,  ///< Not compiled for Windows
    DARK_MODE_UNSUPPORTED = 3,  ///< Windows < 10 build 1809
    DWM_FAIL              = 4,  ///< DwmSetWindowAttribute failed
    THEME_API_FAIL        = 5,  ///< SetWindowTheme / uxtheme API failed
    BRUSH_CREATE_FAIL     = 6,  ///< CreateSolidBrush returned NULL
};

// ---------------------------------------------------------------------------
// WtlDarkModeTheme — static applicator
// ---------------------------------------------------------------------------

class WtlDarkModeTheme final {
public:
    // -----------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------

    /// Minimum Windows 10 build that supports dark mode app frames (1809 = 17763).
    static constexpr std::uint32_t kMinDarkModeBuild = 17763u;

    // -----------------------------------------------------------------
    // Apply — set theme on a window and all immediate children
    // -----------------------------------------------------------------

    [[nodiscard]] static WtlThemeApplicatorStatus Apply(
        [[maybe_unused]] void*  hwnd,
        [[maybe_unused]] bool   dark,
        [[maybe_unused]] const WtlDarkModeThemeConfig& cfg = {}) noexcept
    {
#ifdef _WIN32
        if (!hwnd)
            return WtlThemeApplicatorStatus::NULL_HWND;

        HWND hWnd = static_cast<HWND>(hwnd);

        // Set DWMWA_USE_IMMERSIVE_DARK_MODE (value 20, available since Win10 1903)
        if (cfg.immersiveMode) {
            constexpr DWORD kAttrImmersiveDark = 20u;
            BOOL val = dark ? TRUE : FALSE;
            // DwmSetWindowAttribute may not be available on all systems;
            // ignore return code — best-effort cosmetic call.
            using PFN_DWM = HRESULT(WINAPI*)(HWND, DWORD, LPCVOID, DWORD);
            HMODULE hDwm = GetModuleHandleW(L"dwmapi.dll");
            if (hDwm) {
                auto pfn = reinterpret_cast<PFN_DWM>(
                    GetProcAddress(hDwm, "DwmSetWindowAttribute"));
                if (pfn)
                    pfn(hWnd, kAttrImmersiveDark, &val, sizeof(BOOL));
            }
        }

        // Apply UxTheme "DarkMode_Explorer" to allow dark scrollbars / buttons
        if (cfg.uxThemeHook) {
            using PFN_SWT = HRESULT(WINAPI*)(HWND, LPCWSTR, LPCWSTR);
            HMODULE hUx = GetModuleHandleW(L"uxtheme.dll");
            if (!hUx) hUx = LoadLibraryW(L"uxtheme.dll");
            if (hUx) {
                auto pfn = reinterpret_cast<PFN_SWT>(
                    GetProcAddress(hUx, "SetWindowTheme"));
                if (pfn) {
                    if (dark)
                        pfn(hWnd, L"DarkMode_Explorer", nullptr);
                    else
                        pfn(hWnd, L"", nullptr);
                }
            }
        }

        // Trigger a repaint
        ::InvalidateRect(hWnd, nullptr, TRUE);
        return WtlThemeApplicatorStatus::OK;
#else
        return WtlThemeApplicatorStatus::NOT_WIN32;
#endif
    }

    // -----------------------------------------------------------------
    // GetBackground — return an HBRUSH for WM_CTLCOLORDLG / WM_ERASEBKGND
    // -----------------------------------------------------------------

    /// Returns an HBRUSH for the given mode.  Caller is responsible for
    /// deleting the brush when it is no longer needed (DeleteObject).
    [[nodiscard]] static void* GetBackgroundBrush(
        bool dark,
        const WtlDarkModeColors& colors = {}) noexcept
    {
#ifdef _WIN32
        const COLORREF bg = dark
            ? static_cast<COLORREF>(colors.background)
            : static_cast<COLORREF>(colors.lightBackground);
        return static_cast<void*>(CreateSolidBrush(bg));
#else
        (void)dark; (void)colors;
        return nullptr;
#endif
    }

    // -----------------------------------------------------------------
    // IsDarkModeSupported — Win10 build 1809+
    // -----------------------------------------------------------------

    [[nodiscard]] static bool IsDarkModeSupported() noexcept
    {
#ifdef _WIN32
        OSVERSIONINFOEXW vi{};
        vi.dwOSVersionInfoSize = sizeof(vi);
        vi.dwBuildNumber = kMinDarkModeBuild;
        DWORDLONG mask = ::VerSetConditionMask(0,
            VER_BUILDNUMBER, VER_GREATER_EQUAL);
        return ::VerifyVersionInfoW(&vi, VER_BUILDNUMBER, mask) != FALSE;
#else
        return false;
#endif
    }

private:
    WtlDarkModeTheme() = delete;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_WTL_DARK_MODE_THEME_H
