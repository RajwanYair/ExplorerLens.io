// ============================================================================
// TaskbarPreviewManager.h — Windows Taskbar Thumbnail Preview Helper
// ExplorerLens Engine v15.0.0
// Copyright (c) 2026 ExplorerLens Project
//
// Manages taskbar thumbnail integration for the ExplorerLens Manager app.
// Provides DWM thumbnail registration, custom drawing for the taskbar
// preview, and live/static thumbnail switching based on app state.
// Uses DwmSetIconicThumbnail and DwmSetIconicLivePreviewBitmap APIs.
// ============================================================================

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <algorithm>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <dwmapi.h>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Taskbar thumbnail mode
// ============================================================================

enum class TaskbarThumbnailMode : uint8_t {
    Default = 0,  // Let DWM handle thumbnails
    IconicStatic = 1, // Custom static thumbnail (DwmSetIconicThumbnail)
    IconicLive = 2,  // Live preview bitmap (DwmSetIconicLivePreviewBitmap)
    Custom = 3   // Fully custom rendering
};

inline const char* TaskbarThumbnailModeToString(TaskbarThumbnailMode mode) {
    static const char* names[] = {
        "Default", "IconicStatic", "IconicLive", "Custom"
    };
    return names[static_cast<uint8_t>(mode)];
}

// ============================================================================
// Tab information for taskbar grouping
// ============================================================================

struct TaskbarTab {
    HWND     hwnd = nullptr;
    uint32_t tabId = 0;
    std::wstring title;
    std::wstring tooltip;
    uint32_t iconIndex = 0;
    bool     isActive = false;
    bool     hasProgress = false;
    float    progressPercent = 0.0f;
};

// ============================================================================
// Preview statistics
// ============================================================================

struct TaskbarPreviewStats {
    uint32_t thumbnailUpdates = 0;
    uint32_t livePreviewUpdates = 0;
    uint32_t tabRegistrations = 0;
    uint32_t dwmMessages = 0;
    double   avgRenderTimeMs = 0.0;
    bool     dwmCompositionEnabled = false;
};

// ============================================================================
// TaskbarPreviewManager — main class
// ============================================================================

class TaskbarPreviewManager {
public:
    TaskbarPreviewManager() = default;
    ~TaskbarPreviewManager() { Shutdown(); }

    /// Initialize taskbar preview for a window
    bool Initialize(HWND hwnd) {
        if (!hwnd || !::IsWindow(hwnd)) return false;
        m_hwnd = hwnd;

        // Check DWM composition
        BOOL compositionEnabled = FALSE;
        HRESULT hr = ::DwmIsCompositionEnabled(&compositionEnabled);
        m_stats.dwmCompositionEnabled = SUCCEEDED(hr) && compositionEnabled;

        // Enable iconic thumbnail mode
        if (m_stats.dwmCompositionEnabled) {
            BOOL fForceIconic = TRUE;
            BOOL fHasIconicBitmap = TRUE;
            ::DwmSetWindowAttribute(hwnd, DWMWA_FORCE_ICONIC_REPRESENTATION,
                &fForceIconic, sizeof(fForceIconic));
            ::DwmSetWindowAttribute(hwnd, DWMWA_HAS_ICONIC_BITMAP,
                &fHasIconicBitmap, sizeof(fHasIconicBitmap));
        }

        m_initialized = true;
        return true;
    }

    bool IsInitialized() const { return m_initialized; }
    HWND GetWindow() const { return m_hwnd; }

    /// Set the thumbnail mode
    void SetMode(TaskbarThumbnailMode mode) { m_mode = mode; }
    TaskbarThumbnailMode GetMode() const { return m_mode; }

    /// Register a tab for tabbed thumbnail
    bool RegisterTab(uint32_t tabId, const std::wstring& title) {
        TaskbarTab tab;
        tab.tabId = tabId;
        tab.title = title;
        m_tabs.push_back(tab);
        m_stats.tabRegistrations++;
        return true;
    }

    /// Set the iconic thumbnail bitmap (responds to WM_DWMSENDICONICTHUMBNAIL)
    bool SetIconicThumbnail(const uint8_t* bitmapData, uint32_t width, uint32_t height) {
        if (!m_initialized || !bitmapData || width == 0 || height == 0) return false;

        HBITMAP hBitmap = CreateThumbnailBitmap(bitmapData, width, height);
        if (!hBitmap) return false;

        HRESULT hr = ::DwmSetIconicThumbnail(m_hwnd, hBitmap, 0);
        ::DeleteObject(hBitmap);

        if (SUCCEEDED(hr)) {
            m_stats.thumbnailUpdates++;
            return true;
        }
        return false;
    }

    /// Set live preview bitmap (responds to WM_DWMSENDICONICLIVEPREVIEWBITMAP)
    bool SetLivePreview(const uint8_t* bitmapData, uint32_t width, uint32_t height) {
        if (!m_initialized || !bitmapData || width == 0 || height == 0) return false;

        HBITMAP hBitmap = CreateThumbnailBitmap(bitmapData, width, height);
        if (!hBitmap) return false;

        POINT offset = { 0, 0 };
        HRESULT hr = ::DwmSetIconicLivePreviewBitmap(m_hwnd, hBitmap, &offset, 0);
        ::DeleteObject(hBitmap);

        if (SUCCEEDED(hr)) {
            m_stats.livePreviewUpdates++;
            return true;
        }
        return false;
    }

    /// Handle DWM-related window messages
    bool HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
        (void)wParam;
        (void)lParam;
        switch (msg) {
        case WM_DWMSENDICONICTHUMBNAIL:
            m_stats.dwmMessages++;
            return true;  // Caller should provide thumbnail
        case WM_DWMSENDICONICLIVEPREVIEWBITMAP:
            m_stats.dwmMessages++;
            return true;  // Caller should provide preview
        default:
            return false;
        }
    }

    /// Get active tab count
    uint32_t GetTabCount() const { return static_cast<uint32_t>(m_tabs.size()); }

    /// Get statistics
    const TaskbarPreviewStats& GetStats() const { return m_stats; }

    /// Shutdown and clean up
    void Shutdown() {
        m_tabs.clear();
        m_initialized = false;
        m_hwnd = nullptr;
    }

private:
    HBITMAP CreateThumbnailBitmap(const uint8_t* data, uint32_t width, uint32_t height) {
        BITMAPINFO bmi{};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = static_cast<LONG>(width);
        bmi.bmiHeader.biHeight = -static_cast<LONG>(height);  // Top-down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        void* bits = nullptr;
        HDC hdc = ::GetDC(nullptr);
        HBITMAP hBitmap = ::CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
        ::ReleaseDC(nullptr, hdc);

        if (hBitmap && bits) {
            memcpy(bits, data, width * height * 4);
        }
        return hBitmap;
    }

    HWND m_hwnd = nullptr;
    TaskbarThumbnailMode m_mode = TaskbarThumbnailMode::Default;
    bool m_initialized = false;
    std::vector<TaskbarTab> m_tabs;
    TaskbarPreviewStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
