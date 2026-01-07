// =============================================================================
// worker_process.h - Worker Process Architecture
// =============================================================================
// Sprint 19: Out-of-Process Worker
// Worker process hosts thumbnail generation pipeline isolated from Explorer
// =============================================================================

#pragma once

#include "ipc_protocol.h"
#include "ipc_transport.h"
#include <memory>
#include <thread>
#include <queue>
#include <atomic>
#include <chrono>

namespace DarkThumbs {
namespace Worker {

// =============================================================================
// Worker Configuration
// =============================================================================

struct WorkerConfig {
    // Process Configuration
    std::wstring workerExePath = L"DarkThumbsWorker.exe";
    std::wstring workingDirectory;
    uint32_t maxMemoryMB = 512;         // Memory limit
    uint32_t maxCpuPercent = 80;        // CPU throttle
    
    // Lifecycle
    uint32_t startupTimeoutMs = 10000;  // 10 seconds
    uint32_t idleTimeoutMs = 300000;    // 5 minutes
    uint32_t shutdownTimeoutMs = 5000;  // 5 seconds
    bool restartOnCrash = true;
    uint32_t maxRestarts = 5;
    uint32_t restartBackoffMs = 1000;   // 1 second
    
    // Performance
    uint32_t workerThreads = 4;
    uint32_t maxQueueSize = 100;
    bool enableGPU = true;
    bool enablePlugins = true;
    
    // Warm Pool
    bool useWarmPool = true;
    uint32_t warmPoolSize = 2;
    uint32_t warmPoolTimeoutMs = 60000; // 1 minute keep-alive
    
    // IPC
    IPC::TransportConfig ipcConfig;
};

// =============================================================================
// Worker Process States
// =============================================================================

enum class WorkerState {
    STOPPED,
    STARTING,
    READY,
    BUSY,
    IDLE,
    STOPPING,
    CRASHED,
    UNRESPONSIVE,
};

// =============================================================================
// Worker Statistics
// =============================================================================

struct WorkerStats {
    uint64_t startTimeUs;
    uint64_t uptimeSeconds;
    uint32_t requestsProcessed;
    uint32_t requestsFailed;
    uint32_t requestsCancelled;
    uint32_t cacheHits;
    uint32_t cacheMisses;
    uint32_t gpuAccelerations;
    uint64_t totalProcessingTimeUs;
    uint32_t averageProcessingTimeMs;
    uint32_t restartCount;
    uint32_t crashCount;
    uint32_t currentQueueDepth;
    uint32_t peakQueueDepth;
    WorkerState currentState;
};

// =============================================================================
// Worker Process Manager
// =============================================================================

class WorkerProcess {
public:
    explicit WorkerProcess(const WorkerConfig& config);
    ~WorkerProcess();
    
    // Lifecycle Management
    bool Start();
    void Stop();
    bool Restart();
    bool IsRunning() const;
    WorkerState GetState() const { return m_state.load(); }
    
    // Request Processing
    bool SubmitRequest(const IPC::ThumbnailRequest& request, 
                       IPC::ThumbnailResponse& response,
                       uint32_t timeoutMs = 0);
    bool CancelRequest(uint32_t requestId);
    
    // Health Monitoring
    bool IsHealthy() const;
    bool Ping(uint32_t timeoutMs = 1000);
    const WorkerStats& GetStats() const { return m_stats; }
    
    // Process Information
    DWORD GetProcessId() const { return m_processId; }
    HANDLE GetProcessHandle() const { return m_processHandle; }
    
    // Configuration
    const WorkerConfig& GetConfig() const { return m_config; }
    void UpdateConfig(const WorkerConfig& config);
    
private:
    WorkerConfig m_config;
    std::atomic<WorkerState> m_state{WorkerState::STOPPED};
    WorkerStats m_stats{};
    
    // Process Management
    HANDLE m_processHandle = nullptr;
    DWORD m_processId = 0;
    HANDLE m_jobObject = nullptr;  // For resource limits
    
    // IPC
    std::unique_ptr<IPC::IPCClient> m_ipcClient;
    
    // Threading
    std::unique_ptr<std::thread> m_watchdogThread;
    std::atomic<bool> m_running{false};
    
    // Watchdog
    std::chrono::steady_clock::time_point m_lastHeartbeat;
    uint32_t m_missedHeartbeats = 0;
    
    // Restart Logic
    uint32_t m_currentRestartCount = 0;
    std::chrono::steady_clock::time_point m_lastRestart;
    
    // Private Methods
    bool LaunchWorkerProcess();
    void TerminateWorkerProcess();
    bool SetupJobObject();
    void WatchdogLoop();
    void UpdateHeartbeat();
    bool RecoverFromCrash();
};

// =============================================================================
// Worker Pool (Manages multiple workers)
// =============================================================================

class WorkerPool {
public:
    explicit WorkerPool(const WorkerConfig& config, uint32_t poolSize);
    ~WorkerPool();
    
    // Pool Management
    bool Start();
    void Stop();
    bool Prewarm();
    
    // Request Routing
    WorkerProcess* GetAvailableWorker();
    bool SubmitRequest(const IPC::ThumbnailRequest& request,
                       IPC::ThumbnailResponse& response,
                       uint32_t timeoutMs = 0);
    
    // Pool Statistics
    uint32_t GetActiveWorkers() const;
    uint32_t GetTotalCapacity() const { return m_workers.size(); }
    uint32_t GetAggregateQueueDepth() const;
    
    // Health & Maintenance
    void HealthCheck();
    void RestartUnhealthyWorkers();
    
private:
    WorkerConfig m_config;
    uint32_t m_poolSize;
    std::vector<std::unique_ptr<WorkerProcess>> m_workers;
    std::atomic<uint32_t> m_nextWorkerIndex{0};
    mutable std::mutex m_mutex;
    
    // Round-robin selection
    WorkerProcess* SelectWorkerRoundRobin();
    
    // Least-loaded selection
    WorkerProcess* SelectWorkerLeastLoaded();
};

// =============================================================================
// Restart Policy
// =============================================================================

class RestartPolicy {
public:
    virtual ~RestartPolicy() = default;
    virtual bool ShouldRestart(const WorkerProcess& worker) const = 0;
    virtual uint32_t GetBackoffMs(uint32_t restartCount) const = 0;
};

class ExponentialBackoffPolicy : public RestartPolicy {
public:
    explicit ExponentialBackoffPolicy(uint32_t maxRestarts = 5, uint32_t baseBackoffMs = 1000)
        : m_maxRestarts(maxRestarts), m_baseBackoffMs(baseBackoffMs) {}
    
    bool ShouldRestart(const WorkerProcess& worker) const override {
        return worker.GetStats().restartCount < m_maxRestarts;
    }
    
    uint32_t GetBackoffMs(uint32_t restartCount) const override {
        return m_baseBackoffMs * (1 << std::min(restartCount, 5u)); // Cap at 32x
    }
    
private:
    uint32_t m_maxRestarts;
    uint32_t m_baseBackoffMs;
};

// =============================================================================
// Worker Watchdog (Monitors worker health)
// =============================================================================

class WorkerWatchdog {
public:
    explicit WorkerWatchdog(WorkerPool& pool);
    ~WorkerWatchdog();
    
    // Start/Stop Monitoring
    void Start();
    void Stop();
    bool IsRunning() const { return m_running; }
    
    // Configuration
    void SetHeartbeatInterval(uint32_t intervalMs) { m_heartbeatIntervalMs = intervalMs; }
    void SetMaxMissedHeartbeats(uint32_t max) { m_maxMissedHeartbeats = max; }
    
private:
    WorkerPool& m_pool;
    std::unique_ptr<std::thread> m_thread;
    std::atomic<bool> m_running{false};
    uint32_t m_heartbeatIntervalMs = 5000;
    uint32_t m_maxMissedHeartbeats = 3;
    
    void MonitorLoop();
};

// =============================================================================
// Worker Builder (Fluent API for configuration)
// =============================================================================

class WorkerBuilder {
public:
    WorkerBuilder() = default;
    
    WorkerBuilder& WithExePath(const std::wstring& path) {
        m_config.workerExePath = path;
        return *this;
    }
    
    WorkerBuilder& WithMemoryLimit(uint32_t mb) {
        m_config.maxMemoryMB = mb;
        return *this;
    }
    
    WorkerBuilder& WithThreads(uint32_t threads) {
        m_config.workerThreads = threads;
        return *this;
    }
    
    WorkerBuilder& EnableGPU(bool enable = true) {
        m_config.enableGPU = enable;
        return *this;
    }
    
    WorkerBuilder& EnablePlugins(bool enable = true) {
        m_config.enablePlugins = enable;
        return *this;
    }
    
    WorkerBuilder& WithWarmPool(uint32_t size) {
        m_config.useWarmPool = true;
        m_config.warmPoolSize = size;
        return *this;
    }
    
    WorkerBuilder& WithRestartPolicy(uint32_t maxRestarts, uint32_t backoffMs) {
        m_config.maxRestarts = maxRestarts;
        m_config.restartBackoffMs = backoffMs;
        return *this;
    }
    
    std::unique_ptr<WorkerProcess> Build() {
        return std::make_unique<WorkerProcess>(m_config);
    }
    
    std::unique_ptr<WorkerPool> BuildPool(uint32_t poolSize) {
        return std::make_unique<WorkerPool>(m_config, poolSize);
    }
    
private:
    WorkerConfig m_config;
};

} // namespace Worker
} // namespace DarkThumbs
