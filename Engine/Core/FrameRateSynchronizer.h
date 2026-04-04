// FrameRateSynchronizer.h — Thumbnail Frame Rate Synchronizer (VSync / Adaptive Refresh)
// Copyright (c) 2026 ExplorerLens Project
//
// Synchronises thumbnail decode completion with display refresh cycles to eliminate
// tearing and reduce jitter in animated previews and live annotation overlays.
//
#pragma once
#include <atomic>
#include <chrono>
#include <functional>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class SyncMode {
    Disabled,
    VSync,
    AdaptiveSync,
    LowLatency
};
enum class RefreshRate {
    Hz30 = 30,
    Hz60 = 60,
    Hz90 = 90,
    Hz120 = 120,
    Hz144 = 144
};

struct FrameSyncConfig
{
    SyncMode mode = SyncMode::VSync;
    RefreshRate targetHz = RefreshRate::Hz60;
    bool allowTearing = false;
    int msaaSamples = 1;
};

struct FrameSyncResult
{
    bool presented = false;
    double presentTimeMs = 0.0;
    int64_t frameNumber = 0;
    double frameTimeMs = 16.67;
    bool Ok() const noexcept
    {
        return presented;
    }
};

using FramePresentCallback = std::function<void(const FrameSyncResult&)>;

class FrameRateSynchronizer
{
  public:
    explicit FrameRateSynchronizer(FrameSyncConfig config = {}) : m_config(std::move(config))
    {
        m_frameTimeMs = 1000.0 / static_cast<double>(static_cast<int>(m_config.targetHz));
    }

    void SetPresentCallback(FramePresentCallback cb)
    {
        m_callback = std::move(cb);
    }

    FrameSyncResult PresentFrame()
    {
        double now = CurrentMs();
        double elapsed = now - m_lastPresentMs;
        if (m_config.mode != SyncMode::Disabled && elapsed < m_frameTimeMs) {
            // Would sleep here; stub just records
        }
        m_lastPresentMs = now;
        FrameSyncResult r{true, now, ++m_frameNumber, elapsed};
        if (m_callback)
            m_callback(r);
        return r;
    }

    double TargetFrameTimeMs() const noexcept
    {
        return m_frameTimeMs;
    }
    int64_t FrameNumber() const noexcept
    {
        return m_frameNumber.load();
    }
    const FrameSyncConfig& Config() const noexcept
    {
        return m_config;
    }
    void SetMode(SyncMode mode) noexcept
    {
        m_config.mode = mode;
    }

    static std::string ModeName(SyncMode m) noexcept
    {
        switch (m) {
            case SyncMode::Disabled:
                return "Disabled";
            case SyncMode::VSync:
                return "VSync";
            case SyncMode::AdaptiveSync:
                return "AdaptiveSync";
            case SyncMode::LowLatency:
                return "LowLatency";
        }
        return "Unknown";
    }

  private:
    static double CurrentMs() noexcept
    {
        return static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(
                                       std::chrono::steady_clock::now().time_since_epoch())
                                       .count())
               / 1000.0;
    }

    FrameSyncConfig m_config;
    double m_frameTimeMs = 16.67;
    double m_lastPresentMs = 0.0;
    std::atomic<int64_t> m_frameNumber{0};
    FramePresentCallback m_callback;
};

}  // namespace Engine
}  // namespace ExplorerLens
