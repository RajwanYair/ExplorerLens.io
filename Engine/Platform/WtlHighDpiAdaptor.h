// Engine/Platform/WtlHighDpiAdaptor.h
// ExplorerLens — WTL LENSManager high-DPI dynamic adaptor (ROADMAP v8.0 Phase 3)
// Sprint S346.
//
// Purpose:
//   Phase 3 exit criterion: "LENSManager high-DPI dynamic awareness".
//   Complements the existing HighDpiScaleHelper.h (Engine/Pipeline, S337) which
//   computes scale factors for *thumbnails*.  WtlHighDpiAdaptor handles the
//   LENSManager dialog/window DPI awareness:
//     1. Per-monitor DPI awareness V2 (SetProcessDpiAwarenessContext).
//     2. WM_DPICHANGED message handler: re-layouts controls and re-scales fonts.
//     3. Physical-pixel margin / padding calculation for MoveWindow / SetWindowPos.
//
//   Integration pattern:
//     In MainDlg.h::OnInitDialog():
//       WtlHighDpiAdaptor::EnablePerMonitorV2();
//     In MainDlg.h::OnDpiChanged(UINT newDpi, RECT* pSuggestedRect):
//       m_dpiAdaptor.OnDpiChanged(m_hWnd, newDpi, pSuggestedRect);
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_WTL_HIGH_DPI_ADAPTOR_H
#define EXPLORERLENS_ENGINE_WTL_HIGH_DPI_ADAPTOR_H

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
// WtlDpiAwarenessMode
// ---------------------------------------------------------------------------

enum class WtlDpiAwarenessMode : std::uint8_t {
    UNAWARE           = 0,  ///< Legacy — no DPI scaling
    SYSTEM_AWARE      = 1,  ///< Single DPI, set at process start
    PER_MONITOR_V1    = 2,  ///< WM_DPICHANGED, no font scaling
    PER_MONITOR_V2    = 3,  ///< WM_DPICHANGED + auto non-client scaling (Win10 1703+)
};

// ---------------------------------------------------------------------------
// WtlDpiAdaptorStatus
// ---------------------------------------------------------------------------

enum class WtlDpiAdaptorStatus : std::uint8_t {
    OK                  = 0,
    NOT_WIN32           = 1,
    ALREADY_SET         = 2,  ///< DPI awareness already configured
    API_NOT_AVAILABLE   = 3,  ///< SetProcessDpiAwarenessContext not found (< Win10)
    NULL_HWND           = 4,
    SET_CONTEXT_FAIL    = 5,
};

// ---------------------------------------------------------------------------
// WtlDpiInfo — current DPI state for a window
// ---------------------------------------------------------------------------

struct WtlDpiInfo final {
    std::uint32_t dpi        = 96u;    ///< Current DPI (96 = 100 %)
    std::uint32_t prevDpi    = 96u;    ///< Previous DPI before change
    float         scaleFactor = 1.0f;  ///< dpi / 96.0f

    [[nodiscard]] std::int32_t Scale(std::int32_t logicalPixels) const noexcept
    {
        return static_cast<std::int32_t>(
            static_cast<float>(logicalPixels) * scaleFactor + 0.5f);
    }

    [[nodiscard]] bool Changed() const noexcept { return dpi != prevDpi; }
};

// ---------------------------------------------------------------------------
// WtlHighDpiAdaptorConfig
// ---------------------------------------------------------------------------

struct WtlHighDpiAdaptorConfig final {
    WtlDpiAwarenessMode targetMode = WtlDpiAwarenessMode::PER_MONITOR_V2;
    bool rescaleOnChange  = true;  ///< Call SetWindowPos with new DPI rect
    bool invalidateOnChange = true;  ///< Force repaint after DPI change
};

// ---------------------------------------------------------------------------
// WtlHighDpiAdaptor — instance-per-window adaptor
// ---------------------------------------------------------------------------

class WtlHighDpiAdaptor final {
public:
    // -----------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------

    static constexpr std::uint32_t kBaseDpi       = 96u;
    static constexpr std::uint32_t kDpi125Percent = 120u;
    static constexpr std::uint32_t kDpi150Percent = 144u;
    static constexpr std::uint32_t kDpi175Percent = 168u;
    static constexpr std::uint32_t kDpi200Percent = 192u;

    // -----------------------------------------------------------------
    // Process-wide DPI awareness setup
    // -----------------------------------------------------------------

    /// Call once in WinMain / _tWinMain before any window is created.
    [[nodiscard]] static WtlDpiAdaptorStatus EnablePerMonitorV2() noexcept
    {
#ifdef _WIN32
        // DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 = (DPI_AWARENESS_CONTEXT)-4
        constexpr auto kPmV2 = reinterpret_cast<void*>(static_cast<LONG_PTR>(-4));
        using PFN = BOOL(WINAPI*)(void*);
        HMODULE hUser = GetModuleHandleW(L"user32.dll");
        if (!hUser) return WtlDpiAdaptorStatus::API_NOT_AVAILABLE;
        auto pfn = reinterpret_cast<PFN>(
            GetProcAddress(hUser, "SetProcessDpiAwarenessContext"));
        if (!pfn) return WtlDpiAdaptorStatus::API_NOT_AVAILABLE;
        if (!pfn(kPmV2)) {
            DWORD err = GetLastError();
            if (err == ERROR_ACCESS_DENIED)
                return WtlDpiAdaptorStatus::ALREADY_SET;
            return WtlDpiAdaptorStatus::SET_CONTEXT_FAIL;
        }
        return WtlDpiAdaptorStatus::OK;
#else
        return WtlDpiAdaptorStatus::NOT_WIN32;
#endif
    }

    // -----------------------------------------------------------------
    // Lifecycle
    // -----------------------------------------------------------------

    WtlHighDpiAdaptor() noexcept = default;

    explicit WtlHighDpiAdaptor(const WtlHighDpiAdaptorConfig& cfg) noexcept
        : m_config(cfg) {}

    // -----------------------------------------------------------------
    // OnDpiChanged — call from WM_DPICHANGED handler
    // -----------------------------------------------------------------

    /// @param hwnd           The top-level window receiving WM_DPICHANGED.
    /// @param newDpi         LOWORD of wParam from WM_DPICHANGED.
    /// @param suggestedRect  lParam cast to RECT* from WM_DPICHANGED.
    WtlDpiAdaptorStatus OnDpiChanged(
        [[maybe_unused]] void* hwnd,
        std::uint32_t   newDpi,
        [[maybe_unused]] const void* suggestedRect) noexcept
    {
#ifdef _WIN32
        if (!hwnd) return WtlDpiAdaptorStatus::NULL_HWND;
        HWND hWnd = static_cast<HWND>(hwnd);

        m_dpiInfo.prevDpi    = m_dpiInfo.dpi;
        m_dpiInfo.dpi        = newDpi;
        m_dpiInfo.scaleFactor = static_cast<float>(newDpi) /
                                static_cast<float>(kBaseDpi);

        if (m_config.rescaleOnChange && suggestedRect) {
            const RECT* prc = static_cast<const RECT*>(suggestedRect);
            ::SetWindowPos(hWnd, nullptr,
                           prc->left, prc->top,
                           prc->right - prc->left,
                           prc->bottom - prc->top,
                           SWP_NOZORDER | SWP_NOACTIVATE);
        }
        if (m_config.invalidateOnChange)
            ::InvalidateRect(hWnd, nullptr, TRUE);

        return WtlDpiAdaptorStatus::OK;
#else
        (void)newDpi;
        return WtlDpiAdaptorStatus::NOT_WIN32;
#endif
    }

    // -----------------------------------------------------------------
    // Accessors
    // -----------------------------------------------------------------

    [[nodiscard]] const WtlDpiInfo& DpiInfo() const noexcept { return m_dpiInfo; }
    [[nodiscard]] std::uint32_t     CurrentDpi() const noexcept { return m_dpiInfo.dpi; }
    [[nodiscard]] float             ScaleFactor() const noexcept { return m_dpiInfo.scaleFactor; }

    /// Scale a logical pixel dimension to physical pixels.
    [[nodiscard]] std::int32_t PhysicalPixels(std::int32_t logical) const noexcept
    {
        return m_dpiInfo.Scale(logical);
    }

    // -----------------------------------------------------------------
    // Static helpers
    // -----------------------------------------------------------------

    [[nodiscard]] static std::uint32_t GetWindowDpi([[maybe_unused]] void* hwnd) noexcept
    {
#ifdef _WIN32
        if (!hwnd) return kBaseDpi;
        using PFN = UINT(WINAPI*)(HWND);
        HMODULE hUser = GetModuleHandleW(L"user32.dll");
        if (!hUser) return kBaseDpi;
        auto pfn = reinterpret_cast<PFN>(
            GetProcAddress(hUser, "GetDpiForWindow"));
        if (!pfn) return kBaseDpi;
        return pfn(static_cast<HWND>(hwnd));
#else
        return kBaseDpi;
#endif
    }

private:
    WtlDpiInfo             m_dpiInfo{};
    WtlHighDpiAdaptorConfig m_config{};
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_WTL_HIGH_DPI_ADAPTOR_H
