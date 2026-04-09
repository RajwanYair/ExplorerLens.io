// IdleTimePreGenerator.h — CPU/GPU Idle-Time Thumbnail Pre-Generator
// Copyright (c) 2026 ExplorerLens Project
//
// Monitors system CPU and GPU utilisation at LOW_PRIORITY_CLASS. When the
// system is idle (CPU < 5%, GPU < 10%) the pre-generator consumes pending
// entries from the DirectoryPreScanQueue. On battery or thermal pressure
// it backs off automatically. This fills the cache opportunistically
// without competing with foreground work.
//
#pragma once
#include <cstdint>
#include <functional>

namespace ExplorerLens { namespace Engine {

struct IdlePreGenConfig {
    float    cpuIdleThreshold  = 5.0f;   // % CPU below which idle gen is allowed
    float    gpuIdleThreshold  = 10.0f;  // % GPU below which idle gen is allowed
    float    thermalThreshold  = 85.0f;  // °C CPU package temp above which we stop
    uint32_t pollIntervalMs    = 500;    // How often to check CPU/GPU/temp
    uint32_t batchSize         = 8;      // Items to pre-generate per idle window
    bool     disableOnBattery  = true;   // Stop when not plugged in
};

struct IdlePreGenStats {
    uint64_t totalPreGenerated  = 0;
    uint64_t idleWindowsUsed    = 0;
    float    lastCpuPercent     = 0.0f;
    float    lastGpuPercent     = 0.0f;
    float    lastThermalCelsius = 0.0f;
    bool     isActive           = false;
};

class IdleTimePreGenerator {
public:
    using WorkCallback = std::function<bool()>;  // Returns false when queue empty

    explicit IdleTimePreGenerator(
        const IdlePreGenConfig& config = {}) noexcept;
    ~IdleTimePreGenerator();

    // Register the callback that does one unit of pre-generation work.
    // Returning false means "queue is empty — nothing more to do".
    void SetWorkCallback(WorkCallback cb) noexcept;

    // Start the idle-monitoring background thread.
    void Start() noexcept;
    void Stop()  noexcept;

    IdlePreGenStats GetStats() const noexcept;
    bool IsRunning() const noexcept;

    // Sample current system CPU usage (0–100%).
    static float SampleCpuPercent() noexcept;

    // Sample current GPU usage (0–100%) via DXGI / PDH.
    static float SampleGpuPercent() noexcept;

    // Sample CPU package thermal reading via WMI stub (returns 0.0 if unavailable).
    static float SampleThermalCelsius() noexcept;

    // Check if the system is currently running on battery.
    static bool IsOnBattery() noexcept;

private:
    struct Impl;
    Impl* m_impl = nullptr;
    IdlePreGenConfig m_config;
    WorkCallback     m_workCb;
    bool             m_running = false;
};

}} // namespace ExplorerLens::Engine
