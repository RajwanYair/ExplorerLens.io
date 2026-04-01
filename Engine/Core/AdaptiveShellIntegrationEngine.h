// AdaptiveShellIntegrationEngine.h — Adaptive Shell Integration
// Copyright (c) 2026 ExplorerLens Project
//
// Self-tuning shell integration engine that probes OS API availability at runtime
// and adapts the thumbnail delivery path to the best available mechanism on the
// current platform version — silently degrading when new APIs are unavailable
// without requiring a binary update.
//
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens { namespace Engine {

enum class ShellDeliveryMode : uint8_t {
    Unknown = 0,
    IThumbnailProvider,   // Windows COM
    IExplorerTileProvider,// Windows WinRT
    QuickLookGenerator,   // macOS QL
    GIOThumbnailer,       // Linux GIO
    DBusThumbnailSpec,    // Linux XDG/D-Bus
    DolphinKIO            // KDE Dolphin
};

struct AdaptiveShellStats {
    ShellDeliveryMode activeMode = ShellDeliveryMode::Unknown;
    uint32_t          probeCount = 0;
    uint32_t          fallbacks  = 0;
    float             avgAdaptMs = 0.0f;
};

class AdaptiveShellIntegrationEngine {
public:
    AdaptiveShellIntegrationEngine() {
#ifdef _WIN32
        m_activeMode = ShellDeliveryMode::IThumbnailProvider;
#elif defined(__APPLE__)
        m_activeMode = ShellDeliveryMode::QuickLookGenerator;
#else
        m_activeMode = ShellDeliveryMode::GIOThumbnailer;
#endif
        m_stats.activeMode = m_activeMode;
    }

    ShellDeliveryMode Probe() {
        ++m_stats.probeCount;
        return m_activeMode;
    }
    ShellDeliveryMode GetActiveMode() const { return m_activeMode; }
    bool IsCompatible(ShellDeliveryMode mode) const {
        return mode == m_activeMode || mode == ShellDeliveryMode::Unknown;
    }
    void ForceMode(ShellDeliveryMode mode) {
        m_activeMode       = mode;
        m_stats.activeMode = mode;
    }
    AdaptiveShellStats GetStats() const { return m_stats; }

private:
    ShellDeliveryMode  m_activeMode = ShellDeliveryMode::Unknown;
    AdaptiveShellStats m_stats;
};

}} // namespace ExplorerLens::Engine
