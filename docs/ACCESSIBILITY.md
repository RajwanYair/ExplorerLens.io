# ExplorerLens Accessibility Guide

> Version 23.5.0 "Vega-V" | Standard: WCAG 2.2 Level AA

---

## Summary

ExplorerLens targets **WCAG 2.1 Level AA** compliance across all user interfaces:
- `LENSManager.WinUI.exe` — WinUI 3 configuration app
- `LENSManager.exe` — WTL legacy config tool
- In-context thumbnail overlays within Windows Explorer

| Category | Status | Notes |
|----------|--------|-------|
| 1.1 Text Alternatives | ✅ Pass | All icons have AutomationProperties.Name |
| 1.3 Adaptable | ✅ Pass | NavigationView hierarchy exposed to UI Automation |
| 1.4 Distinguishable | ✅ Pass | All text passes 4.5:1 contrast ratio |
| 1.4.3 Contrast | ✅ Pass | Verified via AccessibilityAudit + ColorBlindFilter |
| 1.4.11 Non-text Contrast | ✅ Pass | UI components ≥ 3:1 vs background |
| 2.1 Keyboard Accessible | ✅ Pass | Full keyboard navigation via KeyboardNavigationMap |
| 2.4 Navigable | ✅ Pass | Focus indicators visible, skip-links present |
| 3.2 Predictable | ✅ Pass | No unexpected focus changes |
| 4.1 Compatible | ✅ Pass | Narrator + JAWS + NVDA tested |

---

## Color Blind Simulation Results

Tested with `ColorBlindFilter` (Brettel 1997 matrices) across all UI states:

| Condition | Deuteranopia | Protanopia | Tritanopia | Achromatopsia |
|-----------|-------------|-----------|-----------|---------------|
| Status: Ready (green) | ✅ | ✅ | ✅ | ⚠️ |
| Status: Error (red) | ✅ | ✅ | ✅ | ⚠️ |
| Active nav item | ✅ | ✅ | ✅ | ✅ |
| Plugin trust badge | ✅ | ✅ | ✅ | ✅ |

> ⚠️ Achromatopsia (full monochromacy, <0.01% of population): status states rely on
> icons in addition to color, so the information is still conveyed non-chromatically.

---

## High Contrast Support

`HighContrastAdapter` detects the active high-contrast theme and remaps:
- Thumbnail overlay background to system `WINDOW` color
- Badge text to system `WINDOW_TEXT` color
- Navigation focus indicator to system `HIGHLIGHT` color

All adaptive colors pass 4.5:1 contrast ratio in both HC Black and HC White themes.

---

## Keyboard Navigation Map

All interactive elements are reachable via keyboard. See `KeyboardNavigationMap` for
the full shortcut registry. Highlights:

| Shortcut | Action |
|----------|--------|
| Tab / Shift+Tab | Navigate between controls |
| Enter / Space | Activate button or toggle |
| Alt+F4 | Close window |
| Ctrl+, | Open settings |
| F5 | Refresh thumbnail cache |
| Ctrl+Shift+R | Re-register shell extension |

---

## Locale & RTL Support

`LocalizationValidator` runs on CI to check:
- Translated strings ≤ 130% of English source length (prevents UI overflow)
- Arabic / Hebrew strings include correct BiDi marks
- No missing `{0}` / `%s` placeholders vs source

Supported locales: EN, FR, DE, JA, ZH-CN, KO, AR, HE

---

## Accessibility Tools Tested

- **Narrator** (Windows 11 22H2+): Full read-through of all pages, no silent elements
- **JAWS 2025**: Navigation and button activation tested
- **NVDA 2025.1**: Navigation and combo box tested
- **Accessibility Insights for Windows**: Zero critical, zero serious issues
- **AccessibilityAudit** (internal): Automated scan via `Engine/Utils/AccessibilityAudit.h`
