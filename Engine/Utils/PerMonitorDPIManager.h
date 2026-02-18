#pragma once
// Sprint 142 — Per-Monitor DPI V2
// Handle DPI changes mid-session, rescale controls in real time.
// Provides per-monitor DPI awareness with live re-layout.

#include <cstdint>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <functional>

namespace DarkThumbs::Utils {

// ─── DPI scale tiers ────────────────────────────────────────────
enum class DPIScale : uint8_t {
    S100  = 96,
    S125  = 120,
    S150  = 144,
    S175  = 168,
    S200  = 192,
    S250  = 240,
    S300  = 288
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

} // namespace DarkThumbs::Utils
