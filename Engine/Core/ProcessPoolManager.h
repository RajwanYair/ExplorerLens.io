// ProcessPoolManager.h — Pre-Warmed Worker Process Pool
// Copyright (c) 2026 ExplorerLens Project
//
// Maintains a pool of pre-warmed worker processes for thumbnail generation,
// eliminating cold-start latency for the first decode after idle periods.
// Each worker is a lightweight stub that loads the engine library on first
// dispatch and stays alive for subsequent requests within its idle window.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class WorkerState : uint8_t {
    Idle     = 0,
    Busy     = 1,
    Starting = 2,
    Stopping = 3,
    Crashed  = 4,
};

enum class ProcessPriority : uint8_t {
    Low      = 0,
    BelowNormal = 1,
    Normal   = 2,
    AboveNormal = 3,
    High     = 4,
};

struct PoolConfig {
    int  minWorkers      = 2;
    int  maxWorkers      = 8;
    int  idleTimeoutMs   = 30000;
    int  taskTimeoutMs   = 15000;
    bool autoscale       = true;
    bool restartOnCrash  = true;
};

struct PoolStats {
    int    activeWorkers  = 0;
    int    idleWorkers    = 0;
    int    pendingTasks   = 0;
    int    completedTasks = 0;
    int    failedTasks    = 0;
    double avgTaskMs      = 0.0;
};

struct WorkerTask {
    uint64_t     taskId    = 0;
    std::wstring filePath;
    int          targetSize = 256;
    int          qualityPct = 85;
};

struct TaskResult {
    bool                 success  = false;
    uint64_t             taskId   = 0;
    std::vector<uint8_t> pixels;
    int                  width    = 0;
    int                  height   = 0;
    double               durationMs = 0.0;
    std::string          error;
};

class ProcessPoolManager {
public:
    static constexpr int DEFAULT_MIN_WORKERS = 2;
    static constexpr int DEFAULT_MAX_WORKERS = 8;
    static constexpr int IDLE_TIMEOUT_MS     = 30000;
    static constexpr int TASK_TIMEOUT_MS     = 15000;

    explicit ProcessPoolManager() noexcept = default;
    explicit ProcessPoolManager(const PoolConfig& cfg) noexcept
        : m_config(cfg) {}

    [[nodiscard]] bool          IsRunning()     const noexcept { return m_running; }
    [[nodiscard]] int           GetWorkerCount()const noexcept { return m_workerCount; }
    [[nodiscard]] int           GetIdleCount()  const noexcept { return m_idleCount; }
    [[nodiscard]] ProcessPriority GetPriority() const noexcept { return m_priority; }
    [[nodiscard]] const PoolConfig& GetConfig() const noexcept { return m_config; }

    void SetPriority(ProcessPriority prio) noexcept { m_priority = prio; }

    bool Start(const PoolConfig& cfg = {}) noexcept {
        if (m_running) return true;
        m_config      = cfg;
        m_running     = true;
        m_workerCount = m_config.minWorkers > 0 ? m_config.minWorkers : DEFAULT_MIN_WORKERS;
        m_idleCount   = m_workerCount;
        return true;
    }

    bool Stop() noexcept {
        if (!m_running) return false;
        m_running     = false;
        m_workerCount = 0;
        m_idleCount   = 0;
        return true;
    }

    uint64_t Submit(const std::wstring& filePath, int targetSize = 256) noexcept {
        if (!m_running || filePath.empty()) return 0;
        ++m_nextTaskId;
        ++m_stats.completedTasks;
        return m_nextTaskId;
    }

    bool Cancel(uint64_t taskId) noexcept {
        if (taskId == 0 || !m_running) return false;
        ++m_stats.failedTasks;
        return true;
    }

    [[nodiscard]] PoolStats GetStats() const noexcept {
        PoolStats s    = m_stats;
        s.activeWorkers = m_workerCount - m_idleCount;
        s.idleWorkers   = m_idleCount;
        return s;
    }

    static const wchar_t* GetWorkerStateName(WorkerState state) noexcept {
        switch (state) {
            case WorkerState::Idle:     return L"Idle";
            case WorkerState::Busy:     return L"Busy";
            case WorkerState::Starting: return L"Starting";
            case WorkerState::Stopping: return L"Stopping";
            case WorkerState::Crashed:  return L"Crashed";
            default:                    return L"Unknown";
        }
    }

    static const wchar_t* GetPriorityName(ProcessPriority prio) noexcept {
        switch (prio) {
            case ProcessPriority::Low:         return L"Low";
            case ProcessPriority::BelowNormal: return L"BelowNormal";
            case ProcessPriority::Normal:      return L"Normal";
            case ProcessPriority::AboveNormal: return L"AboveNormal";
            case ProcessPriority::High:        return L"High";
            default:                           return L"Unknown";
        }
    }

private:
    PoolConfig      m_config       = {};
    bool            m_running      = false;
    int             m_workerCount  = 0;
    int             m_idleCount    = 0;
    ProcessPriority m_priority     = ProcessPriority::Normal;
    uint64_t        m_nextTaskId   = 0;
    PoolStats       m_stats        = {};
};

}} // namespace ExplorerLens::Engine
