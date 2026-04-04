// HybridUIBridge.h — WTL + WinUI 3 XAML Islands Interop Bridge
// Copyright (c) 2026 ExplorerLens Project
//
// Provides the hosting infrastructure for embedding WinUI 3 content
// inside WTL dialog windows using XAML Islands (DesktopWindowXamlSource).
// Falls back to pure WTL on unsupported Windows versions.

#pragma once

#include <cstdint>

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/// XAML Islands hosting state
enum class XamlHostState : uint8_t {
    NotInitialized = 0,
    Initializing = 1,
    Ready = 2,
    Active = 3,
    Error = 4,
    Unsupported = 5  ///< OS too old for XAML Islands
};

/// XAML panel identifier for embedded content
enum class XamlPanelId : uint16_t {
    None = 0,
    SettingsPanel = 1,     ///< Format registration settings
    GalleryPanel = 2,      ///< Visual format gallery
    PerformancePanel = 3,  ///< Performance dashboard
    AboutPanel = 4,        ///< About/version info
    PluginPanel = 5        ///< Plugin management
};

/// Bridge configuration
struct HybridUIConfig
{
    bool enableXamlIslands = true;     ///< Try to use XAML Islands
    bool allowFallback = true;         ///< Fall back to WTL if unavailable
    uint32_t minWindowsBuild = 18362;  ///< Minimum build for XAML Islands
    bool useContentIslands = false;    ///< Use newer ContentIslands API (22H2+)
    bool darkModeSync = true;          ///< Sync dark mode between WTL and XAML
    uint32_t xamlDpi = 96;             ///< Initial DPI for XAML content
};

/// Represents an embedded XAML panel within a WTL window
struct EmbeddedPanel
{
    XamlPanelId id = XamlPanelId::None;
    HWND hwndHost = nullptr;    ///< Child HWND hosting XAML content
    HWND hwndParent = nullptr;  ///< Parent WTL dialog
    RECT bounds = {};           ///< Panel position within parent
    bool isActive = false;
    bool isVisible = false;
};

/// WTL + WinUI 3 hybrid bridge
class HybridUIBridge
{
  public:
    static HybridUIBridge& Instance()
    {
        static HybridUIBridge inst;
        return inst;
    }

    /// Initialize the XAML Islands hosting infrastructure
    bool Initialize(const HybridUIConfig& config = {})
    {
        m_config = config;

        if (!config.enableXamlIslands) {
            m_state = XamlHostState::Unsupported;
            return false;
        }

        // Check OS version
        if (!CheckMinimumBuild(config.minWindowsBuild)) {
            m_state = XamlHostState::Unsupported;
            return config.allowFallback;  // OK if fallback allowed
        }

        // Try to load Windows App SDK
        m_state = XamlHostState::Initializing;
        if (!LoadWindowsAppSDK()) {
            m_state = XamlHostState::Error;
            return config.allowFallback;
        }

        m_state = XamlHostState::Ready;
        return true;
    }

    /// Create an embedded XAML panel in a WTL dialog
    bool CreatePanel(HWND hwndParent, XamlPanelId panelId, const RECT& bounds)
    {
        if (m_state != XamlHostState::Ready && m_state != XamlHostState::Active)
            return false;

        // Create child HWND for XAML content
        HWND hwndChild = CreateWindowExW(WS_EX_NOPARENTNOTIFY,
                                         L"Static",  // Placeholder; real XAML uses DesktopWindowXamlSource
                                         nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, bounds.left, bounds.top,
                                         bounds.right - bounds.left, bounds.bottom - bounds.top, hwndParent, nullptr,
                                         GetModuleHandleW(nullptr), nullptr);

        if (!hwndChild)
            return false;

        EmbeddedPanel panel;
        panel.id = panelId;
        panel.hwndHost = hwndChild;
        panel.hwndParent = hwndParent;
        panel.bounds = bounds;
        panel.isActive = true;
        panel.isVisible = true;

        // Store (simple linear search is fine for <10 panels)
        for (auto& p : m_panels) {
            if (p.id == panelId) {
                if (p.hwndHost)
                    DestroyWindow(p.hwndHost);
                p = panel;
                m_state = XamlHostState::Active;
                return true;
            }
        }
        if (m_panelCount < MAX_PANELS) {
            m_panels[m_panelCount++] = panel;
        }

        m_state = XamlHostState::Active;
        return true;
    }

    /// Remove an embedded panel
    void RemovePanel(XamlPanelId panelId)
    {
        for (uint32_t i = 0; i < m_panelCount; ++i) {
            if (m_panels[i].id == panelId) {
                if (m_panels[i].hwndHost)
                    DestroyWindow(m_panels[i].hwndHost);
                // Shift remaining
                for (uint32_t j = i; j + 1 < m_panelCount; ++j)
                    m_panels[j] = m_panels[j + 1];
                m_panelCount--;
                return;
            }
        }
    }

    /// Resize a panel (e.g., on parent WM_SIZE)
    void ResizePanel(XamlPanelId panelId, const RECT& newBounds)
    {
        for (uint32_t i = 0; i < m_panelCount; ++i) {
            if (m_panels[i].id == panelId && m_panels[i].hwndHost) {
                m_panels[i].bounds = newBounds;
                SetWindowPos(m_panels[i].hwndHost, nullptr, newBounds.left, newBounds.top,
                             newBounds.right - newBounds.left, newBounds.bottom - newBounds.top,
                             SWP_NOZORDER | SWP_NOACTIVATE);
                return;
            }
        }
    }

    /// Check if XAML Islands are available
    bool IsXamlAvailable() const
    {
        return m_state == XamlHostState::Ready || m_state == XamlHostState::Active;
    }

    /// Should we use native WTL fallback?
    bool ShouldUseFallback() const
    {
        return !IsXamlAvailable() && m_config.allowFallback;
    }

    XamlHostState GetState() const
    {
        return m_state;
    }
    const HybridUIConfig& GetConfig() const
    {
        return m_config;
    }
    uint32_t GetPanelCount() const
    {
        return m_panelCount;
    }

    /// Shutdown all panels
    void Shutdown()
    {
        for (uint32_t i = 0; i < m_panelCount; ++i) {
            if (m_panels[i].hwndHost)
                DestroyWindow(m_panels[i].hwndHost);
        }
        m_panelCount = 0;
        m_state = XamlHostState::NotInitialized;
    }

    /// State name for logging
    static const char* StateName(XamlHostState s)
    {
        switch (s) {
            case XamlHostState::NotInitialized:
                return "NotInitialized";
            case XamlHostState::Initializing:
                return "Initializing";
            case XamlHostState::Ready:
                return "Ready";
            case XamlHostState::Active:
                return "Active";
            case XamlHostState::Error:
                return "Error";
            case XamlHostState::Unsupported:
                return "Unsupported";
            default:
                return "Unknown";
        }
    }

  private:
    HybridUIBridge() = default;
    ~HybridUIBridge()
    {
        Shutdown();
    }

    static constexpr uint32_t MAX_PANELS = 8;

    bool CheckMinimumBuild(uint32_t minBuild)
    {
        using RtlGetVersionFn = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);
        HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
        if (!hNtdll)
            return false;

        auto fn = reinterpret_cast<RtlGetVersionFn>(GetProcAddress(hNtdll, "RtlGetVersion"));
        if (!fn)
            return false;

        RTL_OSVERSIONINFOW osvi = {};
        osvi.dwOSVersionInfoSize = sizeof(osvi);
        fn(&osvi);
        return osvi.dwBuildNumber >= minBuild;
    }

    bool LoadWindowsAppSDK()
    {
        // Try loading the Windows App SDK runtime
        HMODULE hMod = LoadLibraryW(L"Microsoft.WindowsAppRuntime.dll");
        if (hMod) {
            FreeLibrary(hMod);
            return true;
        }
        // Not installed — fallback to WTL
        return false;
    }

    HybridUIConfig m_config;
    XamlHostState m_state = XamlHostState::NotInitialized;
    EmbeddedPanel m_panels[MAX_PANELS] = {};
    uint32_t m_panelCount = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
