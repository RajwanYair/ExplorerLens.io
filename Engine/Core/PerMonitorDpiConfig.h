// Engine/Core/PerMonitorDpiConfig.h
// ExplorerLens Engine — S374
//
// Purpose:
//   Per-Monitor V2 DPI awareness configuration for LENSManager.
//   Complements DPIScalingManager (Utils/) which handles WM_DPICHANGED messages.
//   This header provides the configuration + validation layer that LENSManager
//   uses to request PMv2 awareness at startup and validate it was granted.
//
//   Phase 3 requirement: LENSManager high-DPI dynamic awareness.

#pragma once
#ifndef EXPLORERLENS_ENGINE_PERMONITORDPICONFIG_H
#define EXPLORERLENS_ENGINE_PERMONITORDPICONFIG_H

#include <cstdint>
#include <cstddef>

namespace ExplorerLens::Engine {

// ─── DPI awareness level ─────────────────────────────────────────────────────

enum class DpiAwarenessLevel : uint8_t {
    UNAWARE              = 0,   // 96 DPI always — legacy
    SYSTEM               = 1,   // single DPI for all monitors
    PER_MONITOR          = 2,   // per-monitor V1
    PER_MONITOR_V2       = 3,   // per-monitor V2 (Win10 1703+, preferred)
    PER_MONITOR_V2_GDISC = 4,   // PMv2 + GDI scaling (Win10 1803+)
};

// ─── DPI awareness status ────────────────────────────────────────────────────

enum class DpiConfigStatus : uint8_t {
    OK                   = 0,
    ALREADY_SET          = 1,
    LEVEL_DOWNGRADED     = 2,  // OS granted lower level than requested
    MANIFEST_OVERRIDES   = 3,  // application manifest already sets DPI awareness
    OS_TOO_OLD           = 4,  // PMv2 requires Win10 1703+
    API_FAILED           = 5,  // SetProcessDpiAwarenessContext failed
    NOT_WIN32            = 6,
};

// ─── Per-monitor DPI info ────────────────────────────────────────────────────

struct PerMonitorDpiInfo final {
    uint32_t dpiX           = 96;
    uint32_t dpiY           = 96;
    uint32_t scalePct       = 100;   // e.g. 150 = 150%
    bool     isHighDpi      = false; // dpiX > 96
    bool     v2Supported    = false; // OS supports PMv2

    static PerMonitorDpiInfo From96Dpi() noexcept {
        return PerMonitorDpiInfo{ 96, 96, 100, false, false };
    }

    static PerMonitorDpiInfo From120Dpi() noexcept {
        return PerMonitorDpiInfo{ 120, 120, 125, true, true };
    }

    static PerMonitorDpiInfo From144Dpi() noexcept {
        return PerMonitorDpiInfo{ 144, 144, 150, true, true };
    }

    static PerMonitorDpiInfo From192Dpi() noexcept {
        return PerMonitorDpiInfo{ 192, 192, 200, true, true };
    }
};

// ─── Config ──────────────────────────────────────────────────────────────────

struct PerMonitorDpiConfig final {
    DpiAwarenessLevel requestedLevel  = DpiAwarenessLevel::PER_MONITOR_V2;
    bool              fallbackToV1    = true;   // downgrade to PMv1 if PMv2 unavailable
    bool              fallbackSystem  = true;   // further downgrade to SYSTEM if V1 fails
    bool              validateOnInit  = true;   // assert achieved level on startup
    uint32_t          minDpi          = 96;
    uint32_t          maxDpi          = 288;   // 300% = 288 DPI

    static constexpr PerMonitorDpiConfig Default() noexcept {
        return PerMonitorDpiConfig{};
    }

    static constexpr PerMonitorDpiConfig ForLensManager() noexcept {
        PerMonitorDpiConfig c{};
        c.requestedLevel = DpiAwarenessLevel::PER_MONITOR_V2;
        c.fallbackToV1   = true;
        c.fallbackSystem = false;  // don't fall below PMv1 for LENSManager
        c.validateOnInit = true;
        return c;
    }

    static constexpr PerMonitorDpiConfig ForShellExtension() noexcept {
        // Shell extension inherits host process DPI context
        PerMonitorDpiConfig c{};
        c.requestedLevel = DpiAwarenessLevel::PER_MONITOR_V2;
        c.fallbackToV1   = true;
        c.fallbackSystem = true;
        c.validateOnInit = false;
        return c;
    }
};

// ─── Main class ──────────────────────────────────────────────────────────────

class PerMonitorDpiController final {
public:
    PerMonitorDpiController() = default;
    ~PerMonitorDpiController() = default;

    PerMonitorDpiController(const PerMonitorDpiController&) = delete;
    PerMonitorDpiController& operator=(const PerMonitorDpiController&) = delete;

    static PerMonitorDpiController& Global() noexcept {
        static PerMonitorDpiController s_instance;
        return s_instance;
    }

    void Configure(const PerMonitorDpiConfig& config) noexcept {
        m_config = config;
    }

    // Call once at process startup (before any window is created)
    DpiConfigStatus RequestAwareness() noexcept;

    // Query the current process DPI awareness level
    DpiAwarenessLevel CurrentLevel() const noexcept;

    // Query DPI for a specific monitor HWND (Windows only)
    PerMonitorDpiInfo QueryMonitorDpi(void* hmonitor) noexcept;

    // Scale a logical pixel count to physical pixels at current DPI
    uint32_t LogicalToPhysical(uint32_t logical, uint32_t dpi = 96) const noexcept;

    // Scale a physical pixel count back to logical units
    uint32_t PhysicalToLogical(uint32_t physical, uint32_t dpi = 96) const noexcept;

    bool IsHighDpiActive() const noexcept { return m_currentDpi > 96; }
    uint32_t CurrentDpi() const noexcept { return m_currentDpi; }

    const PerMonitorDpiConfig& Config() const noexcept { return m_config; }

private:
    PerMonitorDpiConfig  m_config{};
    DpiAwarenessLevel    m_achievedLevel = DpiAwarenessLevel::UNAWARE;
    uint32_t             m_currentDpi    = 96;
    bool                 m_initialized   = false;
};

// ─── Inline stubs ────────────────────────────────────────────────────────────

inline DpiConfigStatus PerMonitorDpiController::RequestAwareness() noexcept {
#ifndef _WIN32
    return DpiConfigStatus::NOT_WIN32;
#else
    m_initialized   = true;
    m_achievedLevel = m_config.requestedLevel;
    m_currentDpi    = 96;
    return DpiConfigStatus::OK;
#endif
}

inline DpiAwarenessLevel PerMonitorDpiController::CurrentLevel() const noexcept {
    return m_achievedLevel;
}

inline PerMonitorDpiInfo PerMonitorDpiController::QueryMonitorDpi(void* /*hmonitor*/) noexcept {
    return PerMonitorDpiInfo::From96Dpi();
}

inline uint32_t PerMonitorDpiController::LogicalToPhysical(uint32_t logical, uint32_t dpi) const noexcept {
    if (dpi == 0) return logical;
    return (logical * dpi + 48u) / 96u;
}

inline uint32_t PerMonitorDpiController::PhysicalToLogical(uint32_t physical, uint32_t dpi) const noexcept {
    if (dpi == 0) return physical;
    return (physical * 96u + dpi / 2u) / dpi;
}

// ─── Constants ───────────────────────────────────────────────────────────────

static constexpr uint32_t kDpiDefault           = 96u;
static constexpr uint32_t kDpiMedium            = 120u;   // 125%
static constexpr uint32_t kDpiHigh              = 144u;   // 150%
static constexpr uint32_t kDpiRetina            = 192u;   // 200%
static constexpr uint32_t kDpiMax               = 288u;   // 300%
static constexpr uint32_t kDpiScaleNominator    = 96u;

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_PERMONITORDPICONFIG_H
