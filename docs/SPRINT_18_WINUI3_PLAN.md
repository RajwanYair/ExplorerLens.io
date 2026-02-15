# Sprint 18: WinUI 3 Manager - Future Enhancement Plan

**Status:** Deferred (Optional Enhancement)  
**Priority:** P3  
**Estimated Effort:** 3-4 days  
**Current Progress:** 0% (Design phase only)

## Overview

Sprint 18 proposes creating a modern WinUI 3-based configuration manager as an **optional alternative** to the existing WTL-based CBXManager.exe. This would provide a contemporary UI experience while maintaining backward compatibility with the existing manager.

## Rationale for Deferral

1. **Current CBXManager is Functional** - The WTL-based manager (400.5 KB) provides full configuration capabilities
2. **Resource Constraints** - WinUI 3 requires significant development time (3-4 days)
3. **Deployment Complexity** - WinUI 3 apps require Windows App SDK runtime deployment
4. **Priority Alignment** - Plugin system (Sprint 19), GPU optimization (Sprint 20), and code signing (Sprint 24) deliver more immediate value

## Design Specification

### Architecture

```
DarkThumbsManager.WinUI/
├── App.xaml               # WinUI 3 application definition
├── App.xaml.cs           # Application lifecycle
├── MainWindow.xaml       # Main configuration window
├── Pages/
│   ├── HandlersPage.xaml        # Format handler toggles
│   ├── PerformancePage.xaml     # GPU/cache settings
│   ├── StatisticsPage.xaml      # Usage analytics
│   └── AboutPage.xaml           # System info
├── Controls/
│   ├── FormatToggleCard.xaml   # Reusable format control
│   └── PerformanceChart.xaml   # Live perf visualization
└── Package.appxmanifest         # MSIX packaging manifest
```

### Key Features (Sprint 18 Tasks)

#### 18.1-18.3: Project Setup
- Create WinUI 3 Desktop App (.NET 8)
- Configure Windows App SDK 1.5+
- Add Fluent Design icons and assets

#### 18.4: Performance Page
- Real-time decode time charts using WinUI DataChart
- GPU utilization visualization
- Cache hit/miss statistics
- Decoder performance breakdown (p50/p95/p99)

#### 18.5: About Page
- Hardware detection (CPU, GPU, RAM)
- Installed codec list (HEVC, AV1, VP9)
- DirectX feature level detection
- Windows version compatibility matrix

#### 18.6: Test Page
- File picker integration with drag-drop
- Live thumbnail preview at multiple sizes (96/256/512)
- Decode time measurement per format
- Export test reports to CSV

#### 18.7: Theme Switching
- Light/Dark/ System theme following Windows 11 themes
- Mica/Acrylic background materials
- Smooth transitions between themes

#### 18.8: Fluent Design
- Acrylic blur backgrounds
- Reveal hover effects on interactive elements
- Connected animations between pages
- Card-based layout with elevation shadows

#### 18.9: MSIX Packaging
- Package as MSIX for Microsoft Store submission
- Self-contained deployment (no admin required)
- Automatic updates via Store
- Sandboxed security model

#### 18.10: Side-by-Side Installation
- Install as `DarkThumbsManagerUI3.exe` (separate from WTL version)
- Share configuration with existing CBXManager.exe
- Allow users to choose which manager to use
- Migrate settings from registry to settings.json

## Technical Requirements

### Prerequisites
- Visual Studio 2022 17.8+
- Windows App SDK 1.5+
- .NET 8.0 SDK
- Windows 11 SDK (10.0.22621.0+)

### Dependencies
```xml
<PackageReference Include="Microsoft.WindowsAppSDK" Version="1.5.0" />
<PackageReference Include="Microsoft.Graphics.Win2D" Version="1.2.0" />
<PackageReference Include="CommunityToolkit.WinUI.UI.Controls" Version="7.1.2" />
<PackageReference Include="LiveCharts.WinUI" Version="2.0.0" /> <!-- For perf charts -->
```

### Configuration Schema (settings.json)
```json
{
  "version": "6.2.0",
  "enabledFormats": {
    "cbz": true,
    "cbr": true,
    "webp": true,
    "jxl": true,
    "heif": true
  },
  "performance": {
    "enableGPU": true,
    "cacheMaxSizeMB": 500,
    "preferredGPU": "integrated"
  },
  "appearance": {
    "theme": "system",
    "accentColor": "auto"
  }
}
```

## Implementation Timeline (When Activated)

| Phase | Duration | Tasks |
|-------|----------|-------|
| Setup | 0.5 days | Project creation, SDK config, solution integration |
| UI Design | 1.0 days | XAML pages, navigation, Fluent controls |
| Integration | 1.0 days | Registry/config bridge, handler status queries |
| Testing | 0.5 days | Multi-DPI testing, theme transitions, accessibility |
| Packaging | 0.5 days | MSIX manifest, Store assets, signing |
| **Total** | **3.5 days** | **Full WinUI 3 Manager** |

## Benefits When Implemented

1. **Modern UX** - Follows Windows 11 design language
2. **Better Accessibility** - Built-in narrator, high-contrast, scaling support
3. **Store Distribution** - Available via Microsoft Store for easy updates
4. **Rich Visualizations** - Live performance charts and statistics
5. **Touch-Friendly** - Optimized for tablets and touch screens

## Alternatives Considered

### Option A: Enhance Existing WTL Manager (Current Approach)
- ✅ No additional dependencies
- ✅ Small binary size (400 KB)
- ✅ Compatible with Windows 7+
- ❌ Limited UI modernization potential
- ❌ Manual DPI handling required

### Option B: WinUI 3 Manager (This Sprint)
- ✅ Modern, future-proof UI
- ✅ Automatic DPI scaling
- ✅ Rich control library
- ❌ Requires Windows App SDK runtime (80 MB)
- ❌ Windows 10 1809+ only
- ❌ 3-4 days development time

### Option C: Electron/Web-Based Manager
- ✅ Cross-platform potential
- ✅ Rich web ecosystem
- ❌ Large bundle size (150+ MB)
- ❌ High memory usage
- ❌ Poor Windows integration
- ⛔ **Rejected** - Poor fit for Windows shell integration tool

## Decision: Deferred to Post-6.2.0

**Rationale:**
- CBXManager (WTL) is fully functional and meets all P0/P1 requirements
- Sprint 19 (Plugin System), Sprint 20 (GPU), Sprint 21 (Cache), and Sprint 24 (Code Signing) provide more immediate value
- WinUI 3 manager can be developed as v6.3.0 feature after core functionality is complete
- Current user base has not requested modern UI (focus is on format support and performance)

**Recommendation:** Implement in v6.3.0 release (Q2 2026) after addressing:
- Sprint 19: Plugin system activation (P1)
- Sprint 20: GPU batch processing (P2)
- Sprint 21: Cache optimization (P2)
- Sprint 24: Code signing automation (P1)

## Status Update (February 15, 2026)

- **Sprint 18 Status:** Deferred (Design Complete, Implementation Pending)
- **Next Steps:** Document as future enhancement, proceed with P1 sprints
- **User Impact:** None - existing WTL manager continues to function fully
- **Technical Debt:** None - WinUI 3 is an additive enhancement, not a replacement

---

**Document Version:** 1.0  
**Last Updated:** February 15, 2026  
**Author:** DarkThumbs Development Team
