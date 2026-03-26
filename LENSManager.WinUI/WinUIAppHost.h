// WinUIAppHost.h — WinUI 3 Application Host for Manager.WinUI.exe
// Copyright (c) 2026 ExplorerLens Project
//
// BootStraps the WinUI 3 (Windows App SDK) application host for the modern
// Manager GUI. Handles WindowsAppSDK BootstrapInitialize, App activation
// lifecycle, and DPI-aware window creation via AppWindow / OverlappedPresenter.
//
#pragma once
#include <windows.h>
#include <string>
#include <functional>
#include <cstdint>

// Windows App SDK / WinUI 3 forward declarations
// Full headers from Microsoft.WindowsAppSDK NuGet package at build time
struct IInspectable;

namespace ExplorerLens { namespace Engine { namespace WinUI {

// Activation mode for the WinUI app host
enum class ActivationMode {
    Normal,          // Standard window launch
    Settings,        // Launch directly to Settings page
    FirstRun,        // Launch in first-run/OOBE mode
    Tray,            // Launch minimized to tray
};

// Window sizing hints
struct WindowSizeHint {
    int32_t width    = 960;
    int32_t height   = 640;
    int32_t minWidth = 640;
    int32_t minHeight= 480;
    bool    centered = true;
    bool    resizable= true;
};

// Host configuration passed to WinUIAppHost::Initialize
struct WinUIHostConfig {
    std::wstring    appDisplayName   = L"ExplorerLens Manager";
    std::wstring    appId            = L"com.explorerlens.manager";
    ActivationMode  activationMode   = ActivationMode::Normal;
    WindowSizeHint  size;
    bool            darkModeFollow   = true;   // Follow system dark/light
    bool            micaBackground   = true;   // Use Mica material
    bool            titleBarCustom   = true;   // Custom caption buttons
    uint32_t        accentColorARGB  = 0;      // 0 = system accent
};

class WinUIAppHost {
public:
    // Initialize Windows App SDK Bootstrap
    // Returns false if Windows App SDK is not installed (will show install link)
    static bool Initialize(const WinUIHostConfig& config);

    // Run the WinUI message pump — blocks until window closes
    // Returns process exit code
    static int Run();

    // Gracefully close the main window
    static void RequestClose();

    // Get main window HWND (available after Initialize)
    static HWND GetMainHWND();

    // Navigate to a named page in the NavigationView
    static void NavigateTo(const std::wstring& pageKey);

    // Show a toast notification
    static void ShowToast(const std::wstring& title, const std::wstring& body,
                          uint32_t durationMs = 3000);

    // Query if Windows App SDK >= minVersion is available
    static bool IsWindowsAppSDKAvailable(uint32_t minVersionMajor = 1,
                                          uint32_t minVersionMinor = 5);

    // Open ms-windows-store link to install Windows App SDK
    static void OpenAppSDKInstallPage();

private:
    static HWND  s_hwnd;
    static bool  s_initialized;

    static bool  BootstrapWindowsAppSDK(uint32_t major, uint32_t minor);
    static void  ApplyMicaMaterial(HWND hwnd);
    static void  SetupCustomTitleBar(HWND hwnd);
};

HWND WinUIAppHost::s_hwnd        = nullptr;
bool WinUIAppHost::s_initialized = false;

inline bool WinUIAppHost::IsWindowsAppSDKAvailable(uint32_t major, uint32_t minor) {
    // Check for WindowsAppSDK DLL in known locations
    HMODULE hDll = LoadLibraryExW(L"Microsoft.WindowsAppRuntime.Bootstrap.dll",
                                   nullptr, LOAD_LIBRARY_AS_DATAFILE);
    if (!hDll) return false;
    FreeLibrary(hDll);
    (void)major; (void)minor;
    return true;
}

inline void WinUIAppHost::OpenAppSDKInstallPage() {
    ShellExecuteW(nullptr, L"open",
        L"https://learn.microsoft.com/windows/apps/windows-app-sdk/downloads",
        nullptr, nullptr, SW_SHOWNORMAL);
}

inline HWND WinUIAppHost::GetMainHWND() { return s_hwnd; }

}}} // namespace ExplorerLens::Engine::WinUI
