# WinUI 3 Migration Plan

## ExplorerLens Manager v20.5.0 "Quasar-V" — Sprint 259 Status

**Status:** Phase 2 Complete — WinUI 3 App Shell live in `LENSManager.WinUI/`

### v20.5.0 Deliverables (Sprints 251-259)

| Component | File | Status |
|-----------|------|--------|
| **WinUIAppHost** | `LENSManager.WinUI/WinUIAppHost.h` | ✅ Windows App SDK bootstrap, AppWindow, Mica |
| **NavigationViewModel** | `LENSManager.WinUI/NavigationViewModel.h` | ✅ Back-stack, deep-link, breadcrumb |
| **ThemeManager** | `LENSManager.WinUI/ThemeManager.h` | ✅ Dark/Light/HighContrast, AccentColor, WM_WININICHANGE |
| **SettingsViewModel** | `LENSManager.WinUI/SettingsViewModel.h` | ✅ Registry-backed, typed bool/int/string settings |
| **DashboardViewModel** | `LENSManager.WinUI/DashboardViewModel.h` | ✅ StatCards, registration check, DXGI GPU name |
| **PluginsPageViewModel** | `LENSManager.WinUI/PluginsPageViewModel.h` | ✅ Installed plugins, SDKVersionGuard compat check |
| **TrayIconController** | `LENSManager.WinUI/TrayIconController.h` | ✅ Shell_NotifyIcon, context menu, balloon toast |
| **UpdateNotifier** | `LENSManager.WinUI/UpdateNotifier.h` | ✅ WinHTTP update check, 24-hr cooldown, async |

### Architecture: WinUI 3 Manager Shell

```
Manager.WinUI.exe
  ├── WinUIAppHost         — Windows App SDK bootstrap + AppWindow
  ├── NavigationViewModel  — NavigationView back-stack + deep-link
  ├── ThemeManager         — System dark/light/HC + AccentColor
  │
  ├── Pages/
  │   ├── Dashboard       ← DashboardViewModel (StatCards, reg status)
  │   ├── Formats         ← LensIsFormatSupported enumeration
  │   ├── Performance     ← cache/GPU stats
  │   ├── Cache           ← LensClearCache / LensGetCacheStats
  │   ├── Plugins         ← PluginsPageViewModel + SDKVersionGuard
  │   └── Settings        ← SettingsViewModel (HKCU registry)
  │
  ├── TrayIconController   — Shell_NotifyIcon + context menu
  └── UpdateNotifier       — WinHTTP update.explorerlens.io check
```

---

## Previous Status (v15.0.0 "Zenith")

### Executive Summary

This document outlines the phased migration from WTL (Windows Template Library) to
WinUI 3 for the ExplorerLens Manager GUI. The migration uses **XAML Islands** as a
bridge to enable incremental modernization without a full rewrite.

---

## Step 1: XAML Islands Prototype 

### Goal
Embed a single WinUI 3 control inside the existing WTL dialog to validate feasibility.

### Approach
1. **WindowsAppSDK NuGet** — Add `Microsoft.WindowsAppSDK` (1.6+) to LENSManager.vcxproj
2. **DesktopWindowXamlSource** — Create a XAML island host inside a WTL tab page
3. **Prototype Target** — Settings page "General" tab as a XAML UserControl

### Technical Requirements
- Windows App SDK 1.6+ (packaged or unpackaged)
- C++/WinRT projection headers
- MSIX identity (optional, for unpackaged use `<uap:SupportedUsers>`)
- Minimum: Windows 10 1903 (build 18362)

### Key APIs
```cpp
#include <winrt/Microsoft.UI.Xaml.Hosting.h>
winrt::Microsoft::UI::Xaml::Hosting::DesktopWindowXamlSource xamlSource;
```

### Risks
- **COM apartment threading** — WTL uses STA, WinUI 3 requires ASTA
- **DPI handling** — WinUI 3 handles DPI natively vs manual WTL DPI
- **Event routing** — Keyboard/mouse events need proper forwarding
- **Unpackaged deployment** — Requires bootstrapper API initialization

---

## Step 2: MSIX Packaging 

### Goal
Add MSIX package support for Windows Store distribution alongside MSI/Inno.

### Approach
1. Create `packaging/msix/AppxManifest.xml` with required capabilities
2. Add COM server registration in manifest (replace .rgs files)
3. Use sparse package for unpackaged COM registration fallback
4. CI/CD pipeline produces both MSI and MSIX

### Manifest Template
```xml
<?xml version="1.0" encoding="utf-8"?>
<Package xmlns="http://schemas.microsoft.com/appx/manifest/foundation/windows10"
 xmlns:com="http://schemas.microsoft.com/appx/manifest/com/windows10"
 xmlns:uap="http://schemas.microsoft.com/appx/manifest/uap/windows10"
 xmlns:rescap="http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities">

 <Identity Name="ExplorerLens" Publisher="CN=ExplorerLens" Version="15.0.0.0" />
 
 <Properties>
 <DisplayName>ExplorerLens</DisplayName>
 <PublisherDisplayName>ExplorerLens Project</PublisherDisplayName>
 <Logo>Assets\StoreLogo.png</Logo>
 </Properties>
 
 <Applications>
 <Application Id="Manager" Executable="LENSManager.exe" EntryPoint="windows.fullTrustApplication">
 <Extensions>
 <com:Extension Category="windows.comServer">
 <com:ComServer>
 <com:SurrogateServer DisplayName="ExplorerLens Shell Extension">
 <com:Class Id="9E6ECB90-5A61-42BD-B851-D3297D9C7F39"
 Path="LENSShell.dll"
 ThreadingModel="Apartment" />
 </com:SurrogateServer>
 </com:ComServer>
 </com:Extension>
 </Extensions>
 </Application>
 </Applications>

 <Capabilities>
 <rescap:Capability Name="runFullTrust" />
 </Capabilities>
</Package>
```

### Build Integration
```powershell
# Add to Build-All-And-Package.ps1
makemsix pack /d packaging/msix/output /p ExplorerLens.msix
```

---

## Step 3: Prototype Settings Page 

### Goal
Create a WinUI 3 XAML settings page that can run standalone or embedded via XAML Islands.

### UI Design
```
┌─────────────────────────────────────────────────────┐
│ ExplorerLens Settings [×] │
├─────────────────────────────────────────────────────┤
│ ┌──────────┐ ┌─────────────────────────────────────┐│
│ │ General │ │ [NavigationView with sections] ││
│ │ Formats │ │ ││
│ │ Advanced │ │ Thumbnail Quality: [Slider] ││
│ │ GPU │ │ Cache Size: [NumberBox] MB ││
│ │ Perf │ │ GPU Acceleration: [ToggleSwitch] ││
│ │ About │ │ Dark Mode: [ToggleSwitch] ││
│ └──────────┘ │ ││
│ │ [InfoBar: Status messages] ││
│ └─────────────────────────────────────┘│
└─────────────────────────────────────────────────────┘
```

### XAML Snippet (SettingsPage.xaml)
```xml
<Page xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation"
 xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml"
 xmlns:muxc="using:Microsoft.UI.Xaml.Controls">
 
 <NavigationView PaneDisplayMode="Left" IsBackButtonVisible="Collapsed">
 <NavigationView.MenuItems>
 <NavigationViewItem Content="General" Icon="Setting" Tag="general"/>
 <NavigationViewItem Content="Formats" Icon="List" Tag="formats"/>
 <NavigationViewItem Content="Performance" Icon="Clock" Tag="perf"/>
 </NavigationView.MenuItems>
 
 <ScrollViewer>
 <StackPanel Spacing="8" Padding="20">
 <muxc:Expander Header="Thumbnail Settings" IsExpanded="True">
 <StackPanel Spacing="4">
 <Slider Header="Quality" Minimum="50" Maximum="100" Value="90"/>
 <NumberBox Header="Cache Size (MB)" Value="256" Minimum="64" Maximum="2048"/>
 </StackPanel>
 </muxc:Expander>
 
 <muxc:Expander Header="GPU Acceleration">
 <StackPanel Spacing="4">
 <ToggleSwitch Header="Enable GPU Decode" IsOn="True"/>
 <ToggleSwitch Header="DirectX 12 (if available)" IsOn="False"/>
 </StackPanel>
 </muxc:Expander>
 </StackPanel>
 </ScrollViewer>
 </NavigationView>
</Page>
```

---

## Migration Timeline

| Step | Target | Scope | Risk |
|------|--------|-------|------|
| 1 — XAML Islands POC | v15.0+ | Embed single control | Medium |
| 2 — MSIX Packaging | v15.0+ | Store-ready package | Low |
| 3 — Settings Prototype | v15.0+ | Full settings page | Medium |
| 4 — Full Migration | v16.0+ | Replace all WTL UI | High |

## Decision: Hybrid Approach

For v15.0.0, we use a **hybrid architecture**:
- WTL remains the primary UI framework (proven, lightweight, no runtime dependency)
- WinUI 3 XAML Islands available for new pages (opt-in, requires Windows App SDK)
- MSIX packaging as an additional distribution channel alongside MSI
- Full WinUI 3 migration planned for v16.0.0

## Dependencies

| Component | Version | Purpose |
|-----------|---------|---------|
| Windows App SDK | 1.6+ | WinUI 3 runtime |
| C++/WinRT | 2.0.240405.15+ | Projection headers |
| MSIX Packaging Tool | 1.2024+ | Package creation |
| Windows 10 SDK | 10.0.26100.0 | Already present |

## Files to Create (Future)

- `LENSManager/xaml/SettingsPage.xaml` — XAML settings UI
- `LENSManager/xaml/SettingsPage.xaml.h` — Code-behind
- `packaging/msix/AppxManifest.xml` — MSIX manifest
- `packaging/msix/Assets/` — Store logos and icons
