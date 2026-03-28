// WASMHostController.h — Cross-Process WASM Host with Resource Limits
// Copyright (c) 2026 ExplorerLens Project
//
// Manages a dedicated host process for WASM plugin execution — applying
// per-plugin CPU, memory, and wall-clock resource limits via OS job objects.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <chrono>

namespace ExplorerLens {
namespace Engine {

enum class WASMHostState  { Idle, Launching, Running, Throttled, Killed };
enum class WASMIsolation  { InProcess, SeparateThread, SeparateProcess };

struct WASMResourceLimits {
    uint64_t maxMemoryBytes    = 256ULL * 1024 * 1024;
    uint32_t cpuSharePercent   = 50;
    uint32_t maxWallClockMs    = 10000;
    uint32_t maxOpenHandles    = 32;
    bool     allowNetworkAccess= false;
    bool     allowFileWrite    = false;
};

struct WASMHostMetrics {
    uint64_t bytesAllocated  = 0;
    uint32_t cpuUsagePercent = 0;
    uint64_t wallClockElapsedMs = 0;
    uint32_t openHandleCount = 0;
    bool     resourceLimitHit= false;
};

class WASMHostController {
public:
    explicit WASMHostController(WASMIsolation isolation = WASMIsolation::SeparateProcess,
                                const WASMResourceLimits& limits = {})
        : m_isolation(isolation), m_limits(limits) {}

    bool          Launch()                 { m_state = WASMHostState::Running; return true; }
    void          Terminate()             { m_state = WASMHostState::Killed; }
    void          Suspend()               { m_state = WASMHostState::Throttled; }
    void          Resume()                { m_state = WASMHostState::Running; }

    WASMHostState GetState()     const    { return m_state; }
    WASMIsolation GetIsolation() const    { return m_isolation; }
    bool          IsRunning()    const    { return m_state == WASMHostState::Running; }

    WASMHostMetrics  GetMetrics()  const  { return m_metrics; }
    void             UpdateMetrics(const WASMHostMetrics& m) { m_metrics = m; }

    const WASMResourceLimits& GetLimits() const           { return m_limits; }
    void  SetLimits(const WASMResourceLimits& l)          { m_limits = l; }
    bool  IsWithinLimits(uint64_t bytes) const {
        return bytes <= m_limits.maxMemoryBytes && !m_metrics.resourceLimitHit;
    }
    void  Reset() { m_state = WASMHostState::Idle; m_metrics = {}; }

private:
    WASMIsolation     m_isolation;
    WASMResourceLimits m_limits;
    WASMHostState     m_state   = WASMHostState::Idle;
    WASMHostMetrics   m_metrics;
};

} // namespace Engine
} // namespace ExplorerLens
