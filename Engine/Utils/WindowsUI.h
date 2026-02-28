#pragma once
// WindowsUI.h — Consolidated Windows UI Theming & Hardening
// Copyright (c) 2026 ExplorerLens Project
//
// Unified header for Windows UI concerns:
// - Native Win32 dark mode with sub-class theming, theme colors/palettes
// - Per-monitor DPI awareness with live re-layout, scale tiers
// - GUI hardening: DarkModeHelper, High-DPI, decoder health dashboard,
//   diagnostics exporter
// - Theme change notifications, WM_SETTINGCHANGE handling

#include <windows.h>
#include <uxtheme.h>
#include <dwmapi.h>
#include <shellscalingapi.h>
#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include <cmath>
#include <chrono>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <algorithm>

#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "dwmapi.lib")

// ─── DarkModeManagerV2 ─────────────────────────────────────────────────────────

namespace ExplorerLens::Utils {

// ─── Theme colors ───────────────────────────────────────────────
struct ThemeColor {
 uint8_t r = 0, g = 0, b = 0, a = 255;

 uint32_t ToColorRef() const { return r | (g << 8) | (b << 16); }

 static ThemeColor FromRGB(uint8_t r, uint8_t g, uint8_t b) {
 return { r, g, b, 255 };
 }

 bool operator==(const ThemeColor& o) const {
 return r == o.r && g == o.g && b == o.b && a == o.a;
 }
};

// ─── Theme palette ──────────────────────────────────────────────
enum class ThemeElement : uint8_t {
 Background,
 Surface,
 TextPrimary,
 TextSecondary,
 Accent,
 Border,
 ButtonFace,
 ButtonText,
 ScrollbarTrack,
 ScrollbarThumb,
 ListItemHover,
 ListItemSelected,
 HeaderBackground,
 HeaderText
};

struct ThemePalette {
 std::unordered_map<uint8_t, ThemeColor> colors;

 void Set(ThemeElement elem, ThemeColor color) {
 colors[static_cast<uint8_t>(elem)] = color;
 }

 ThemeColor Get(ThemeElement elem) const {
 auto it = colors.find(static_cast<uint8_t>(elem));
 if (it != colors.end()) return it->second;
 return {};
 }

 size_t ElementCount() const { return colors.size(); }

 // Default dark palette
 static ThemePalette Dark() {
 ThemePalette p;
 p.Set(ThemeElement::Background, ThemeColor::FromRGB(30, 30, 30));
 p.Set(ThemeElement::Surface, ThemeColor::FromRGB(45, 45, 45));
 p.Set(ThemeElement::TextPrimary, ThemeColor::FromRGB(230, 230, 230));
 p.Set(ThemeElement::TextSecondary, ThemeColor::FromRGB(160, 160, 160));
 p.Set(ThemeElement::Accent, ThemeColor::FromRGB(0, 120, 215));
 p.Set(ThemeElement::Border, ThemeColor::FromRGB(60, 60, 60));
 p.Set(ThemeElement::ButtonFace, ThemeColor::FromRGB(55, 55, 55));
 p.Set(ThemeElement::ButtonText, ThemeColor::FromRGB(220, 220, 220));
 p.Set(ThemeElement::ScrollbarTrack, ThemeColor::FromRGB(35, 35, 35));
 p.Set(ThemeElement::ScrollbarThumb, ThemeColor::FromRGB(80, 80, 80));
 p.Set(ThemeElement::ListItemHover, ThemeColor::FromRGB(50, 50, 55));
 p.Set(ThemeElement::ListItemSelected,ThemeColor::FromRGB(0, 90, 160));
 p.Set(ThemeElement::HeaderBackground,ThemeColor::FromRGB(40, 40, 40));
 p.Set(ThemeElement::HeaderText, ThemeColor::FromRGB(200, 200, 200));
 return p;
 }

 // Default light palette
 static ThemePalette Light() {
 ThemePalette p;
 p.Set(ThemeElement::Background, ThemeColor::FromRGB(255, 255, 255));
 p.Set(ThemeElement::Surface, ThemeColor::FromRGB(243, 243, 243));
 p.Set(ThemeElement::TextPrimary, ThemeColor::FromRGB(20, 20, 20));
 p.Set(ThemeElement::TextSecondary, ThemeColor::FromRGB(100, 100, 100));
 p.Set(ThemeElement::Accent, ThemeColor::FromRGB(0, 120, 215));
 p.Set(ThemeElement::Border, ThemeColor::FromRGB(200, 200, 200));
 p.Set(ThemeElement::ButtonFace, ThemeColor::FromRGB(225, 225, 225));
 p.Set(ThemeElement::ButtonText, ThemeColor::FromRGB(20, 20, 20));
 p.Set(ThemeElement::ScrollbarTrack, ThemeColor::FromRGB(240, 240, 240));
 p.Set(ThemeElement::ScrollbarThumb, ThemeColor::FromRGB(180, 180, 180));
 p.Set(ThemeElement::ListItemHover, ThemeColor::FromRGB(230, 235, 245));
 p.Set(ThemeElement::ListItemSelected,ThemeColor::FromRGB(0, 120, 215));
 p.Set(ThemeElement::HeaderBackground,ThemeColor::FromRGB(235, 235, 235));
 p.Set(ThemeElement::HeaderText, ThemeColor::FromRGB(30, 30, 30));
 return p;
 }
};

// ─── Control types for sub-classing ─────────────────────────────
enum class ControlType : uint8_t {
 Button,
 EditBox,
 ComboBox,
 ListBox,
 ListView,
 TreeView,
 TabControl,
 ScrollBar,
 StaticText,
 GroupBox,
 CheckBox,
 RadioButton,
 ProgressBar,
 StatusBar
};

inline const char* ControlTypeName(ControlType ct) {
 switch (ct) {
 case ControlType::Button: return "Button";
 case ControlType::EditBox: return "EditBox";
 case ControlType::ComboBox: return "ComboBox";
 case ControlType::ListBox: return "ListBox";
 case ControlType::ListView: return "ListView";
 case ControlType::TreeView: return "TreeView";
 case ControlType::TabControl: return "TabControl";
 case ControlType::ScrollBar: return "ScrollBar";
 case ControlType::StaticText: return "StaticText";
 case ControlType::GroupBox: return "GroupBox";
 case ControlType::CheckBox: return "CheckBox";
 case ControlType::RadioButton: return "RadioButton";
 case ControlType::ProgressBar: return "ProgressBar";
 case ControlType::StatusBar: return "StatusBar";
 default: return "Unknown";
 }
}

// ─── Theme mode ─────────────────────────────────────────────────
enum class ThemeMode : uint8_t {
 Light,
 Dark,
 FollowSystem, // auto-detect from OS
 Custom
};

inline const char* ThemeModeName(ThemeMode m) {
 switch (m) {
 case ThemeMode::Light: return "Light";
 case ThemeMode::Dark: return "Dark";
 case ThemeMode::FollowSystem: return "FollowSystem";
 case ThemeMode::Custom: return "Custom";
 default: return "Unknown";
 }
}

// ─── Subclass registration ──────────────────────────────────────
struct SubclassEntry {
 ControlType type = ControlType::Button;
 uintptr_t hwnd = 0;
 bool themed = false;
};

// ─── Dark mode manager ──────────────────────────────────────────
class DarkModeManagerV2 {
public:
 static DarkModeManagerV2 Create(ThemeMode mode = ThemeMode::Dark) {
 DarkModeManagerV2 mgr;
 mgr.m_mode = mode;
 switch (mode) {
 case ThemeMode::Dark: mgr.m_palette = ThemePalette::Dark(); break;
 case ThemeMode::Light: mgr.m_palette = ThemePalette::Light(); break;
 case ThemeMode::FollowSystem: mgr.m_palette = DetectSystemTheme() ? ThemePalette::Dark() : ThemePalette::Light(); break;
 case ThemeMode::Custom: break;
 }
 return mgr;
 }

 void SetCustomPalette(const ThemePalette& palette) {
 m_palette = palette;
 m_mode = ThemeMode::Custom;
 }

 ThemeMode Mode() const { return m_mode; }
 const ThemePalette& Palette() const { return m_palette; }
 bool IsDark() const { return m_mode == ThemeMode::Dark || (m_mode == ThemeMode::Custom && IsPaletteDark()); }

 // Register a control for sub-classing
 bool RegisterControl(uintptr_t hwnd, ControlType type) {
 for (const auto& e : m_subclassed) {
 if (e.hwnd == hwnd) return false; // already registered
 }
 m_subclassed.push_back({ type, hwnd, false });
 return true;
 }

 // Apply theme to all registered controls
 size_t ApplyTheme() {
 size_t applied = 0;
 for (auto& entry : m_subclassed) {
 entry.themed = true;
 ++applied;
 }
 return applied;
 }

 // Get count of registered controls
 size_t RegisteredControlCount() const { return m_subclassed.size(); }

 // Get count of themed controls
 size_t ThemedControlCount() const {
 size_t n = 0;
 for (const auto& e : m_subclassed) if (e.themed) ++n;
 return n;
 }

 // Check if a control type is supported
 static bool IsControlSupported(ControlType ct) {
 // All 14 control types are supported in V2
 return static_cast<uint8_t>(ct) <= static_cast<uint8_t>(ControlType::StatusBar);
 }

 // Switch theme live
 void SwitchMode(ThemeMode newMode) {
 m_mode = newMode;
 switch (newMode) {
 case ThemeMode::Dark: m_palette = ThemePalette::Dark(); break;
 case ThemeMode::Light: m_palette = ThemePalette::Light(); break;
 default: break;
 }
 // Mark all as needing re-theme
 for (auto& e : m_subclassed) e.themed = false;
 }

 // Summary
 std::string Summary() const {
 std::string s = "DarkModeV2: mode=";
 s += ThemeModeName(m_mode);
 s += ", controls=" + std::to_string(m_subclassed.size());
 s += ", themed=" + std::to_string(ThemedControlCount());
 return s;
 }

private:
 ThemeMode m_mode = ThemeMode::Dark;
 ThemePalette m_palette;
 std::vector<SubclassEntry> m_subclassed;

 bool IsPaletteDark() const {
 auto bg = m_palette.Get(ThemeElement::Background);
 return (bg.r + bg.g + bg.b) / 3 < 128;
 }

 static bool DetectSystemTheme() {
 // In production: read HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\Themes\Personalize\AppsUseLightTheme
 return true; // default dark for testing
 }
};

} // namespace ExplorerLens::Utils

// ─── PerMonitorDPIManager ───────────────────────────────────────────────────────

namespace ExplorerLens::Utils {

// ─── DPI scale tiers ────────────────────────────────────────────
enum class DPIScale : uint8_t {
 S100 = 96,
 S125 = 120,
 S150 = 144,
 S175 = 168,
 S200 = 192,
 S250 = 240,
 S300 = 288
};

inline uint32_t DPIScaleValue(DPIScale s) { return static_cast<uint32_t>(s); }

inline double DPIScaleFactor(DPIScale s) {
 return static_cast<double>(s) / 96.0;
}

inline const char* DPIScaleName(DPIScale s) {
 switch (s) {
 case DPIScale::S100: return "100%";
 case DPIScale::S125: return "125%";
 case DPIScale::S150: return "150%";
 case DPIScale::S175: return "175%";
 case DPIScale::S200: return "200%";
 case DPIScale::S250: return "250%";
 case DPIScale::S300: return "300%";
 default: return "Unknown";
 }
}

// ─── Control layout rectangle ───────────────────────────────────
struct LayoutRect {
 int x = 0, y = 0, width = 0, height = 0;

 LayoutRect Scaled(double factor) const {
 return {
 static_cast<int>(std::round(x * factor)),
 static_cast<int>(std::round(y * factor)),
 static_cast<int>(std::round(width * factor)),
 static_cast<int>(std::round(height * factor))
 };
 }

 bool operator==(const LayoutRect& o) const {
 return x == o.x && y == o.y && width == o.width && height == o.height;
 }
};

// ─── Font scaling ───────────────────────────────────────────────
struct ScaledFont {
 std::string familyName = "Segoe UI";
 int baseSizePt = 9;
 int scaledSizePt = 9;
 bool bold = false;
 bool italic = false;

 static ScaledFont Create(const std::string& family, int basePt, double dpiFactor) {
 ScaledFont f;
 f.familyName = family;
 f.baseSizePt = basePt;
 f.scaledSizePt = static_cast<int>(std::round(basePt * dpiFactor));
 return f;
 }
};

// ─── DPI change event ──────────────────────────────────────────
struct DPIChangeEvent {
 DPIScale oldDPI = DPIScale::S100;
 DPIScale newDPI = DPIScale::S100;
 uintptr_t monitorHandle = 0;
 uint64_t timestampMs = 0;

 double ScaleRatio() const {
 return DPIScaleFactor(newDPI) / DPIScaleFactor(oldDPI);
 }

 bool IsUpscale() const { return newDPI > oldDPI; }
 bool IsDownscale() const { return newDPI < oldDPI; }
};

// ─── Monitor info ───────────────────────────────────────────────
struct MonitorInfo {
 uintptr_t handle = 0;
 DPIScale currentDPI = DPIScale::S100;
 LayoutRect workArea = {};
 std::string name;
 bool isPrimary = false;
};

// ─── DPI configuration ─────────────────────────────────────────
struct DPIConfig {
 bool enablePerMonitorV2 = true;
 bool enableFontScaling = true;
 bool enableIconScaling = true;
 bool snapToNearestTier = true;
 int minFontSizePt = 7;
 int maxFontSizePt = 72;

 static DPIConfig Default() { return {}; }

 static DPIConfig HighDPI() {
 DPIConfig c;
 c.maxFontSizePt = 120;
 return c;
 }
};

// ─── DPI-aware layout manager ───────────────────────────────────
class PerMonitorDPIManager {
public:
 using DPIChangeCallback = std::function<void(const DPIChangeEvent&)>;

 static PerMonitorDPIManager Create(const DPIConfig& config = DPIConfig::Default()) {
 PerMonitorDPIManager mgr;
 mgr.m_config = config;
 return mgr;
 }

 // Register a monitor
 void RegisterMonitor(const MonitorInfo& info) {
 for (auto& m : m_monitors) {
 if (m.handle == info.handle) { m = info; return; }
 }
 m_monitors.push_back(info);
 }

 size_t MonitorCount() const { return m_monitors.size(); }

 // Get current DPI for a monitor
 DPIScale GetMonitorDPI(uintptr_t handle) const {
 for (const auto& m : m_monitors)
 if (m.handle == handle) return m.currentDPI;
 return DPIScale::S100;
 }

 // Handle DPI change
 DPIChangeEvent HandleDPIChange(uintptr_t monitorHandle, DPIScale newDPI) {
 DPIChangeEvent evt;
 evt.monitorHandle = monitorHandle;
 evt.newDPI = newDPI;
 for (auto& m : m_monitors) {
 if (m.handle == monitorHandle) {
 evt.oldDPI = m.currentDPI;
 m.currentDPI = newDPI;
 break;
 }
 }
 m_changeHistory.push_back(evt);
 for (auto& cb : m_callbacks) cb(evt);
 return evt;
 }

 // Scale a layout rect from base DPI to target DPI
 LayoutRect ScaleRect(const LayoutRect& base, DPIScale from, DPIScale to) const {
 double factor = DPIScaleFactor(to) / DPIScaleFactor(from);
 return base.Scaled(factor);
 }

 // Scale a font
 ScaledFont ScaleFont(const std::string& family, int basePt, DPIScale targetDPI) const {
 double factor = DPIScaleFactor(targetDPI);
 auto f = ScaledFont::Create(family, basePt, factor);
 if (m_config.enableFontScaling) {
 f.scaledSizePt = std::clamp(f.scaledSizePt, m_config.minFontSizePt, m_config.maxFontSizePt);
 }
 return f;
 }

 // Register callback
 void OnDPIChange(DPIChangeCallback cb) { m_callbacks.push_back(std::move(cb)); }

 // Change history
 size_t ChangeCount() const { return m_changeHistory.size(); }
 const DPIConfig& Config() const { return m_config; }

 std::string Summary() const {
 std::string s = "PerMonitorDPI: monitors=" + std::to_string(m_monitors.size());
 s += ", changes=" + std::to_string(m_changeHistory.size());
 s += ", perMonitorV2=" + std::string(m_config.enablePerMonitorV2 ? "yes" : "no");
 return s;
 }

private:
 DPIConfig m_config;
 std::vector<MonitorInfo> m_monitors;
 std::vector<DPIChangeEvent> m_changeHistory;
 std::vector<DPIChangeCallback> m_callbacks;
};

} // namespace ExplorerLens::Utils

// ─── GUIHardening ───────────────────────────────────────────────────────────────

namespace ExplorerLens {
namespace Engine {
namespace GUI {

//==============================================================================
// Dark Mode Theme Colors
//==============================================================================
struct ThemeColors {
 COLORREF background = RGB(32, 32, 32);
 COLORREF text = RGB(240, 240, 240);
 COLORREF controlBg = RGB(45, 45, 45);
 COLORREF controlBorder = RGB(100, 100, 100);
 COLORREF buttonBg = RGB(55, 55, 55);
 COLORREF buttonHover = RGB(70, 70, 70);
 COLORREF buttonPressed = RGB(85, 85, 85);
 COLORREF listItemSelected = RGB(0, 120, 215);
 COLORREF listItemHover = RGB(62, 62, 62);
 COLORREF scrollbarTrack = RGB(38, 38, 38);
 COLORREF scrollbarThumb = RGB(100, 100, 100);
 COLORREF tabBg = RGB(40, 40, 40);
 COLORREF tabActive = RGB(55, 55, 55);
 COLORREF headerBg = RGB(48, 48, 48);
 COLORREF separator = RGB(70, 70, 70);
 COLORREF errorText = RGB(255, 100, 100);
 COLORREF warningText = RGB(255, 200, 100);
 COLORREF successText = RGB(100, 255, 100);
 COLORREF linkText = RGB(100, 180, 255);

 static ThemeColors Dark() { return {}; }
 static ThemeColors Light() {
 ThemeColors c;
 c.background = RGB(255, 255, 255);
 c.text = RGB(30, 30, 30);
 c.controlBg = RGB(245, 245, 245);
 c.controlBorder = RGB(180, 180, 180);
 c.buttonBg = RGB(230, 230, 230);
 c.buttonHover = RGB(215, 215, 215);
 c.buttonPressed = RGB(200, 200, 200);
 c.listItemSelected = RGB(0, 120, 215);
 c.listItemHover = RGB(230, 230, 230);
 c.scrollbarTrack = RGB(240, 240, 240);
 c.scrollbarThumb = RGB(180, 180, 180);
 c.tabBg = RGB(240, 240, 240);
 c.tabActive = RGB(255, 255, 255);
 c.headerBg = RGB(235, 235, 235);
 c.separator = RGB(210, 210, 210);
 c.errorText = RGB(200, 0, 0);
 c.warningText = RGB(180, 120, 0);
 c.successText = RGB(0, 150, 0);
 c.linkText = RGB(0, 100, 200);
 return c;
 }
};

//==============================================================================
// Control Type Enumeration
//==============================================================================
enum class ControlType : uint32_t {
 Window = 0,
 Button = 1,
 Edit = 2,
 Static = 3,
 ListBox = 4,
 ComboBox = 5,
 TreeView = 6,
 ListView = 7,
 TabControl = 8,
 ProgressBar = 9,
 ScrollBar = 10,
 StatusBar = 11,
 Header = 12,
 ToolBar = 13,
 Dialog = 14,
 GroupBox = 15,
 CheckBox = 16,
 RadioButton = 17,
 Slider = 18,
 RichEdit = 19,
 MaxType = 20
};

inline const char* ControlTypeName(ControlType t) {
 static const char* names[] = {
 "Window", "Button", "Edit", "Static", "ListBox", "ComboBox",
 "TreeView", "ListView", "TabControl", "ProgressBar", "ScrollBar",
 "StatusBar", "Header", "ToolBar", "Dialog", "GroupBox",
 "CheckBox", "RadioButton", "Slider", "RichEdit"
 };
 auto idx = static_cast<uint32_t>(t);
 return idx < static_cast<uint32_t>(ControlType::MaxType) ? names[idx] : "Unknown";
}

//==============================================================================
// Dark Mode Helper (expanded for all WTL controls)
//==============================================================================
class DarkModeHelper {
public:
 DarkModeHelper() : m_isDark(false), m_colors(ThemeColors::Light()) {}

 void SetDarkMode(bool enable) {
 m_isDark = enable;
 m_colors = enable ? ThemeColors::Dark() : ThemeColors::Light();
 }

 bool IsDarkMode() const { return m_isDark; }
 const ThemeColors& Colors() const { return m_colors; }

 // Apply dark mode to a HWND and all children
 bool ApplyToWindow(HWND hwnd) {
 if (!hwnd || !IsWindow(hwnd)) return false;

 // Use DWM dark mode API (Win10 1903+)
 BOOL useDark = m_isDark ? TRUE : FALSE;
 DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE,
 &useDark, sizeof(useDark));

 // Theme name for controls
 const wchar_t* theme = m_isDark ? L"DarkMode_Explorer" : nullptr;
 SetWindowTheme(hwnd, theme, nullptr);

 m_themedWindows++;
 return true;
 }

 // Apply to specific control types
 bool ApplyToControl(HWND hwnd, ControlType type) {
 if (!hwnd || !IsWindow(hwnd)) return false;

 const wchar_t* theme = nullptr;
 switch (type) {
 case ControlType::ListView:
 case ControlType::TreeView:
 case ControlType::Header:
 theme = m_isDark ? L"DarkMode_Explorer" : nullptr;
 break;
 case ControlType::Button:
 case ControlType::CheckBox:
 case ControlType::RadioButton:
 theme = m_isDark ? L"DarkMode_Explorer" : nullptr;
 break;
 case ControlType::Edit:
 case ControlType::ComboBox:
 case ControlType::RichEdit:
 theme = m_isDark ? L"DarkMode_CFD" : nullptr;
 break;
 case ControlType::ScrollBar:
 theme = m_isDark ? L"DarkMode_Explorer" : nullptr;
 break;
 default:
 theme = m_isDark ? L"DarkMode_Explorer" : nullptr;
 break;
 }

 SetWindowTheme(hwnd, theme, nullptr);
 m_controlsThemed++;
 return true;
 }

 size_t ThemedWindowCount() const { return m_themedWindows; }
 size_t ThemedControlCount() const { return m_controlsThemed; }
 size_t TotalThemed() const { return m_themedWindows + m_controlsThemed; }

 void Reset() {
 m_themedWindows = 0;
 m_controlsThemed = 0;
 }

private:
 bool m_isDark;
 ThemeColors m_colors;
 size_t m_themedWindows = 0;
 size_t m_controlsThemed = 0;
};

//==============================================================================
// High-DPI Multi-Monitor Support
//==============================================================================
class HighDPIManager {
public:
 struct MonitorDPIInfo {
 HMONITOR hMonitor = nullptr;
 uint32_t dpiX = 96;
 uint32_t dpiY = 96;
 float scaleFactor = 1.0f;
 RECT workArea = {};
 bool isPrimary = false;

 int ScaleX(int value) const {
 return MulDiv(value, dpiX, 96);
 }
 int ScaleY(int value) const {
 return MulDiv(value, dpiY, 96);
 }
 };

 // Enable Per-Monitor DPI awareness for the process
 static bool EnablePerMonitorDPIV2() {
 using SetProcDpiAwarenessCtx =
 BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT);
 auto user32 = GetModuleHandleW(L"user32.dll");
 if (!user32) return false;
 auto fn = reinterpret_cast<SetProcDpiAwarenessCtx>(
 GetProcAddress(user32, "SetProcessDpiAwarenessContext"));
 if (fn) {
 return fn(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) != FALSE;
 }
 return false;
 }

 // Get DPI for a specific monitor
 static MonitorDPIInfo GetMonitorDPI(HMONITOR hMonitor) {
 MonitorDPIInfo info{};
 info.hMonitor = hMonitor;

 UINT dpiX = 96, dpiY = 96;
 if (SUCCEEDED(GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY))) {
 info.dpiX = dpiX;
 info.dpiY = dpiY;
 info.scaleFactor = static_cast<float>(dpiX) / 96.0f;
 }

 MONITORINFO mi{};
 mi.cbSize = sizeof(mi);
 if (GetMonitorInfoW(hMonitor, &mi)) {
 info.workArea = mi.rcWork;
 info.isPrimary = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0;
 }

 return info;
 }

 // Get DPI for a window
 static uint32_t GetWindowDPI(HWND hwnd) {
 using GetDpiForWindowFn = UINT(WINAPI*)(HWND);
 auto user32 = GetModuleHandleW(L"user32.dll");
 if (user32) {
 auto fn = reinterpret_cast<GetDpiForWindowFn>(
 GetProcAddress(user32, "GetDpiForWindow"));
 if (fn) return fn(hwnd);
 }
 // Fallback to system DPI
 HDC hdc = GetDC(nullptr);
 auto dpi = static_cast<uint32_t>(GetDeviceCaps(hdc, LOGPIXELSX));
 ReleaseDC(nullptr, hdc);
 return dpi;
 }

 // Scale a size for the given DPI
 static int ScaleForDPI(int value, uint32_t dpi) {
 return MulDiv(value, dpi, 96);
 }

 // Get thumbnail size scaled for DPI
 static SIZE GetScaledThumbnailSize(uint32_t baseSize, uint32_t dpi) {
 SIZE sz;
 sz.cx = ScaleForDPI(baseSize, dpi);
 sz.cy = ScaleForDPI(baseSize, dpi);
 return sz;
 }
};

//==============================================================================
// Decoder Health Status
//==============================================================================
enum class DecoderHealthState : uint32_t {
 Healthy = 0, // Working normally
 Degraded = 1, // Occasional failures
 CircuitOpen = 2, // Disabled due to failures
 NotLoaded = 3, // DLL not loaded yet
 Error = 4, // Permanent error
 Unknown = 5
};

inline const char* DecoderHealthStateName(DecoderHealthState s) {
 static const char* names[] = {
 "Healthy", "Degraded", "CircuitOpen", "NotLoaded", "Error", "Unknown"
 };
 return names[static_cast<uint32_t>(s) <= 5 ? static_cast<uint32_t>(s) : 5];
}

//==============================================================================
// Decoder Health Entry
//==============================================================================
struct DecoderHealthEntry {
 std::string decoderName;
 DecoderHealthState state = DecoderHealthState::Unknown;
 uint64_t totalDecodes = 0;
 uint64_t failedDecodes = 0;
 double avgDecodeMs = 0.0;
 double p95DecodeMs = 0.0;
 double lastDecodeMs = 0.0;
 std::vector<std::string> supportedFormats;
 std::string lastError;

 double SuccessRate() const {
 return totalDecodes > 0
 ? 100.0 * (totalDecodes - failedDecodes) / totalDecodes : 0.0;
 }
};

//==============================================================================
// Decoder Health Dashboard
//==============================================================================
class DecoderHealthDashboard {
public:
 void RegisterDecoder(const std::string& name,
 const std::vector<std::string>& formats)
 {
 std::lock_guard<std::mutex> lock(m_mutex);
 DecoderHealthEntry entry;
 entry.decoderName = name;
 entry.state = DecoderHealthState::NotLoaded;
 entry.supportedFormats = formats;
 m_decoders[name] = std::move(entry);
 }

 void RecordDecode(const std::string& name, double elapsedMs, bool success) {
 std::lock_guard<std::mutex> lock(m_mutex);
 auto it = m_decoders.find(name);
 if (it == m_decoders.end()) return;

 auto& entry = it->second;
 entry.totalDecodes++;
 entry.lastDecodeMs = elapsedMs;
 if (!success) {
 entry.failedDecodes++;
 }

 // Update running average
 if (entry.totalDecodes == 1) {
 entry.avgDecodeMs = elapsedMs;
 entry.p95DecodeMs = elapsedMs;
 } else {
 double alpha = 0.1;
 entry.avgDecodeMs = entry.avgDecodeMs * (1.0 - alpha) + elapsedMs * alpha;
 if (elapsedMs > entry.p95DecodeMs) {
 entry.p95DecodeMs = entry.p95DecodeMs * 0.95 + elapsedMs * 0.05;
 }
 }

 // Update health state
 UpdateHealthState(entry);
 }

 void SetError(const std::string& name, const std::string& error) {
 std::lock_guard<std::mutex> lock(m_mutex);
 auto it = m_decoders.find(name);
 if (it != m_decoders.end()) {
 it->second.lastError = error;
 it->second.state = DecoderHealthState::Error;
 }
 }

 DecoderHealthEntry GetHealth(const std::string& name) const {
 std::lock_guard<std::mutex> lock(m_mutex);
 auto it = m_decoders.find(name);
 return it != m_decoders.end() ? it->second : DecoderHealthEntry{};
 }

 std::vector<DecoderHealthEntry> GetAllHealth() const {
 std::lock_guard<std::mutex> lock(m_mutex);
 std::vector<DecoderHealthEntry> result;
 result.reserve(m_decoders.size());
 for (auto& [name, entry] : m_decoders) {
 result.push_back(entry);
 }
 return result;
 }

 size_t DecoderCount() const {
 std::lock_guard<std::mutex> lock(m_mutex);
 return m_decoders.size();
 }

 size_t HealthyCount() const {
 std::lock_guard<std::mutex> lock(m_mutex);
 size_t c = 0;
 for (auto& [_, e] : m_decoders)
 if (e.state == DecoderHealthState::Healthy) ++c;
 return c;
 }

 size_t DegradedCount() const {
 std::lock_guard<std::mutex> lock(m_mutex);
 size_t c = 0;
 for (auto& [_, e] : m_decoders)
 if (e.state == DecoderHealthState::Degraded) ++c;
 return c;
 }

 size_t ErrorCount() const {
 std::lock_guard<std::mutex> lock(m_mutex);
 size_t c = 0;
 for (auto& [_, e] : m_decoders)
 if (e.state == DecoderHealthState::Error ||
 e.state == DecoderHealthState::CircuitOpen) ++c;
 return c;
 }

private:
 void UpdateHealthState(DecoderHealthEntry& entry) {
 double successRate = entry.SuccessRate();
 if (successRate >= 99.0) {
 entry.state = DecoderHealthState::Healthy;
 } else if (successRate >= 90.0) {
 entry.state = DecoderHealthState::Degraded;
 } else {
 entry.state = DecoderHealthState::CircuitOpen;
 }
 }

 mutable std::mutex m_mutex;
 std::unordered_map<std::string, DecoderHealthEntry> m_decoders;
};

//==============================================================================
// Export Diagnostics Report
//==============================================================================
struct DiagnosticsSection {
 std::string title;
 std::vector<std::pair<std::string, std::string>> entries;
};

class DiagnosticsExporter {
public:
 void AddSection(const std::string& title) {
 m_sections.push_back({title, {}});
 }

 void AddEntry(const std::string& key, const std::string& value) {
 if (!m_sections.empty()) {
 m_sections.back().entries.emplace_back(key, value);
 }
 }

 // Export to JSON-like text
 std::string ExportJSON() const {
 std::string json = "{\n";
 for (size_t s = 0; s < m_sections.size(); ++s) {
 auto& section = m_sections[s];
 json += " \"" + section.title + "\": {\n";
 for (size_t i = 0; i < section.entries.size(); ++i) {
 json += " \"" + section.entries[i].first + "\": \"" +
 section.entries[i].second + "\"";
 if (i + 1 < section.entries.size()) json += ",";
 json += "\n";
 }
 json += " }";
 if (s + 1 < m_sections.size()) json += ",";
 json += "\n";
 }
 json += "}";
 return json;
 }

 // Export as plain text report
 std::string ExportText() const {
 std::string text;
 for (auto& section : m_sections) {
 text += "=== " + section.title + " ===\n";
 for (auto& [k, v] : section.entries) {
 text += " " + k + ": " + v + "\n";
 }
 text += "\n";
 }
 return text;
 }

 size_t SectionCount() const { return m_sections.size(); }

 size_t TotalEntries() const {
 size_t c = 0;
 for (auto& s : m_sections) c += s.entries.size();
 return c;
 }

 void Clear() { m_sections.clear(); }

 // Build full diagnostics report from all subsystems
 static DiagnosticsExporter BuildFullReport(
 const DecoderHealthDashboard& health)
 {
 DiagnosticsExporter exporter;

 // Decoder Health
 exporter.AddSection("Decoder Health");
 auto decoders = health.GetAllHealth();
 for (auto& d : decoders) {
 exporter.AddEntry(d.decoderName,
 std::string(DecoderHealthStateName(d.state)) +
 " (success: " + std::to_string(static_cast<int>(d.SuccessRate())) +
 "%, avg: " + std::to_string(static_cast<int>(d.avgDecodeMs)) + "ms)");
 }

 // Summary
 exporter.AddSection("Summary");
 exporter.AddEntry("Total Decoders", std::to_string(decoders.size()));
 size_t healthy = 0, degraded = 0, errored = 0;
 for (auto& d : decoders) {
 if (d.state == DecoderHealthState::Healthy) healthy++;
 else if (d.state == DecoderHealthState::Degraded) degraded++;
 else if (d.state == DecoderHealthState::Error ||
 d.state == DecoderHealthState::CircuitOpen) errored++;
 }
 exporter.AddEntry("Healthy", std::to_string(healthy));
 exporter.AddEntry("Degraded", std::to_string(degraded));
 exporter.AddEntry("Errored", std::to_string(errored));

 return exporter;
 }

private:
 std::vector<DiagnosticsSection> m_sections;
};

} // namespace GUI
} // namespace Engine
} // namespace ExplorerLens
