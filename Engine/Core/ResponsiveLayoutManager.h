// ResponsiveLayoutManager.h — DPI-Aware Responsive Layout for Manager UI
// Copyright (c) 2026 ExplorerLens Project
//
// Computes layout breakpoints, spacing, and control sizes for the WTL Manager
// window at any DPI scale. Listens to WM_DPICHANGED and returns updated geometry
// so dialogs reflow without hard-coded pixel positions.
//
#pragma once

#include <windows.h>
#include <cstdint>
#include <functional>
#include <vector>

namespace ExplorerLens {
namespace Engine {
namespace Core {

enum class LayoutBreakpoint : uint8_t {
    Compact = 0,   // < 800 effective px wide
    Normal = 1,    // 800–1280 effective px
    Wide = 2,      // > 1280 effective px
    UltraWide = 3  // > 1920 effective px
};

struct LayoutMetrics
{
    uint32_t dpi = 96;
    float scaleFactor = 1.0f;  // dpi / 96.0
    LayoutBreakpoint breakpoint = LayoutBreakpoint::Normal;
    int margin = 8;          // outer margin dp
    int padding = 4;         // inner padding dp
    int controlHeight = 24;  // standard button/input height dp
    int iconSize = 16;       // toolbar icon dp
    int listRowHeight = 28;  // list view row dp
    int sidebarWidth = 220;  // settings sidebar dp
    int windowMinWidth = 640;
    int windowMinHeight = 480;

    // Convert density-independent pixels to physical pixels
    int ToPhysical(int dp) const
    {
        return static_cast<int>(dp * scaleFactor + 0.5f);
    }

    // Scale a dimension relative to 96 DPI baseline
    int Scale(int baseline) const
    {
        return ToPhysical(baseline);
    }
};

class ResponsiveLayoutManager
{
  public:
    static ResponsiveLayoutManager& Instance()
    {
        static ResponsiveLayoutManager inst;
        return inst;
    }

    // Call when window is created or WM_DPICHANGED fires
    void Update(HWND hwnd, uint32_t newDpi = 0)
    {
        if (newDpi == 0) {
            // Query per-monitor DPI
            HDC hdc = GetDC(hwnd);
            newDpi = static_cast<uint32_t>(GetDeviceCaps(hdc, LOGPIXELSX));
            ReleaseDC(hwnd, hdc);
        }
        m_metrics.dpi = newDpi;
        m_metrics.scaleFactor = newDpi / 96.0f;
        m_metrics.breakpoint = ComputeBreakpoint(hwnd);
        RefreshDerivedMetrics();
        FireCallbacks();
    }

    const LayoutMetrics& Metrics() const
    {
        return m_metrics;
    }

    // Subscribe to DPI/layout change events
    using ChangeFn = std::function<void(const LayoutMetrics&)>;
    void OnChange(ChangeFn fn)
    {
        m_cbs.push_back(std::move(fn));
    }

    // Get a scaled system font height for UI labels
    int ScaledFontHeight(int pointSize) const
    {
        return -MulDiv(pointSize, static_cast<int>(m_metrics.dpi), 72);
    }

    // Compute RECT for a control in a dialog given its dp-spec layout
    RECT ControlRect(int xDp, int yDp, int wDp, int hDp) const
    {
        return RECT{m_metrics.ToPhysical(xDp), m_metrics.ToPhysical(yDp), m_metrics.ToPhysical(xDp + wDp),
                    m_metrics.ToPhysical(yDp + hDp)};
    }

    // Whether to use compact (single-column) layout
    bool IsCompact() const
    {
        return m_metrics.breakpoint == LayoutBreakpoint::Compact;
    }

  private:
    ResponsiveLayoutManager()
    {
        m_metrics.scaleFactor = 1.f;
    }

    LayoutBreakpoint ComputeBreakpoint(HWND hwnd) const
    {
        RECT r;
        GetClientRect(hwnd, &r);
        int effectivePx = static_cast<int>((r.right - r.left) / m_metrics.scaleFactor);
        if (effectivePx < 800)
            return LayoutBreakpoint::Compact;
        if (effectivePx < 1280)
            return LayoutBreakpoint::Normal;
        if (effectivePx < 1920)
            return LayoutBreakpoint::Wide;
        return LayoutBreakpoint::UltraWide;
    }

    void RefreshDerivedMetrics()
    {
        float s = m_metrics.scaleFactor;
        m_metrics.margin = static_cast<int>(8 * s);
        m_metrics.padding = static_cast<int>(4 * s);
        m_metrics.controlHeight = static_cast<int>(24 * s);
        m_metrics.iconSize = static_cast<int>((m_metrics.dpi >= 144 ? 24 : 16));
        m_metrics.listRowHeight = static_cast<int>(28 * s);
        m_metrics.sidebarWidth = m_metrics.breakpoint >= LayoutBreakpoint::Wide ? 280 : 220;
    }

    void FireCallbacks()
    {
        for (auto& fn : m_cbs)
            fn(m_metrics);
    }

    LayoutMetrics m_metrics;
    std::vector<ChangeFn> m_cbs;
};

}  // namespace Core
}  // namespace Engine
}  // namespace ExplorerLens
