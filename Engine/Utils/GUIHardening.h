//==============================================================================
// ExplorerLens — GUI Hardening
//
// DarkModeHelper expansion for all WTL controls, high-DPI multi-monitor
// support, export diagnostics, and decoder health dashboard data model.
//==============================================================================

#pragma once

#include <windows.h>
#include <uxtheme.h>
#include <dwmapi.h>
#include <shellscalingapi.h>
#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include <chrono>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <algorithm>

#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "dwmapi.lib")

namespace ExplorerLens {
namespace Engine {
namespace GUI {

//==============================================================================
// Dark Mode Theme Colors
//==============================================================================
struct ThemeColors {
    COLORREF background      = RGB(32, 32, 32);
    COLORREF text             = RGB(240, 240, 240);
    COLORREF controlBg        = RGB(45, 45, 45);
    COLORREF controlBorder    = RGB(100, 100, 100);
    COLORREF buttonBg         = RGB(55, 55, 55);
    COLORREF buttonHover      = RGB(70, 70, 70);
    COLORREF buttonPressed    = RGB(85, 85, 85);
    COLORREF listItemSelected = RGB(0, 120, 215);
    COLORREF listItemHover    = RGB(62, 62, 62);
    COLORREF scrollbarTrack   = RGB(38, 38, 38);
    COLORREF scrollbarThumb   = RGB(100, 100, 100);
    COLORREF tabBg            = RGB(40, 40, 40);
    COLORREF tabActive        = RGB(55, 55, 55);
    COLORREF headerBg         = RGB(48, 48, 48);
    COLORREF separator        = RGB(70, 70, 70);
    COLORREF errorText        = RGB(255, 100, 100);
    COLORREF warningText      = RGB(255, 200, 100);
    COLORREF successText      = RGB(100, 255, 100);
    COLORREF linkText         = RGB(100, 180, 255);

    static ThemeColors Dark()  { return {}; }
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
    Window          = 0,
    Button          = 1,
    Edit            = 2,
    Static          = 3,
    ListBox         = 4,
    ComboBox        = 5,
    TreeView        = 6,
    ListView        = 7,
    TabControl      = 8,
    ProgressBar     = 9,
    ScrollBar       = 10,
    StatusBar       = 11,
    Header          = 12,
    ToolBar         = 13,
    Dialog          = 14,
    GroupBox        = 15,
    CheckBox        = 16,
    RadioButton     = 17,
    Slider          = 18,
    RichEdit        = 19,
    MaxType         = 20
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
        HMONITOR hMonitor     = nullptr;
        uint32_t dpiX         = 96;
        uint32_t dpiY         = 96;
        float    scaleFactor  = 1.0f;
        RECT     workArea     = {};
        bool     isPrimary    = false;

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
    Healthy     = 0,    // Working normally
    Degraded    = 1,    // Occasional failures
    CircuitOpen = 2,    // Disabled due to failures
    NotLoaded   = 3,    // DLL not loaded yet
    Error       = 4,    // Permanent error
    Unknown     = 5
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
    uint64_t totalDecodes    = 0;
    uint64_t failedDecodes   = 0;
    double   avgDecodeMs     = 0.0;
    double   p95DecodeMs     = 0.0;
    double   lastDecodeMs    = 0.0;
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
            json += "  \"" + section.title + "\": {\n";
            for (size_t i = 0; i < section.entries.size(); ++i) {
                json += "    \"" + section.entries[i].first + "\": \"" +
                    section.entries[i].second + "\"";
                if (i + 1 < section.entries.size()) json += ",";
                json += "\n";
            }
            json += "  }";
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
                text += "  " + k + ": " + v + "\n";
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

