// =============================================================================
// shellhost_client.h - ShellHost IPC Client with Circuit Breaker
// =============================================================================
// Sprint 19: Out-of-Process Worker
// Thin client in ShellHost (Explorer) that communicates with Worker process
// =============================================================================

#pragma once

#include "../Worker/ipc_protocol.h"
#include "../Worker/ipc_transport.h"
#include "../Worker/worker_process.h"
#include <memory>
#include <atomic>
#include <chrono>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>
#include <string>

namespace ExplorerLens {
namespace ShellHost {

// =============================================================================
// Circuit Breaker States
// =============================================================================

enum class CircuitState {
    CLOSED,      // Normal operation
    OPEN,        // Too many failures, rejecting requests
    HALF_OPEN,   // Testing if service recovered
};

// =============================================================================
// Circuit Breaker Configuration
// =============================================================================

struct CircuitBreakerConfig {
    uint32_t failureThreshold = 5;         // Failures before opening
    uint32_t successThreshold = 2;         // Successes to close from half-open
    uint32_t timeoutMs = 30000;            // Request timeout (30 seconds)
    uint32_t openDurationMs = 60000;       // Time circuit stays open (1 minute)
    uint32_t halfOpenMaxAttempts = 3;      // Max attempts in half-open state
};

// =============================================================================
// ShellHost Client (Main interface for Explorer integration)
// =============================================================================

class ShellHostClient {
public:
    explicit ShellHostClient(const Worker::WorkerConfig& workerConfig);
    ~ShellHostClient();
    
    // Initialization
    bool Initialize();
    void Shutdown();
    
    // Thumbnail Generation (Primary API)
    HRESULT GetThumbnail(
        const wchar_t* filePath,
        uint32_t sizePx,
        HBITMAP* phBitmap,
        uint32_t timeoutMs = 30000
    );
    
    // Fallback to Legacy Path
    HRESULT GetThumbnailLegacy(
        const wchar_t* filePath,
        uint32_t sizePx,
        HBITMAP* phBitmap
    );
    
    // Health & Status
    bool IsWorkerHealthy() const;
    bool IsCircuitOpen() const { return m_circuitState.load() == CircuitState::OPEN; }
    CircuitState GetCircuitState() const { return m_circuitState.load(); }
    
    // Statistics
    uint64_t GetTotalRequests() const { return m_totalRequests; }
    uint64_t GetSuccessfulRequests() const { return m_successfulRequests; }
    uint64_t GetFailedRequests() const { return m_failedRequests; }
    uint64_t GetFallbackRequests() const { return m_fallbackRequests; }
    uint64_t GetCircuitOpenCount() const { return m_circuitOpenCount; }
    
    // Configuration
    void SetCircuitBreakerConfig(const CircuitBreakerConfig& config);
    void EnableFallback(bool enable) { m_fallbackEnabled = enable; }
    
private:
    Worker::WorkerConfig m_workerConfig;
    CircuitBreakerConfig m_circuitConfig;
    std::unique_ptr<Worker::WorkerProcess> m_worker;
    std::unique_ptr<IPC::IPCClient> m_ipcClient;
    
    // Circuit Breaker State
    std::atomic<CircuitState> m_circuitState{CircuitState::CLOSED};
    std::atomic<uint32_t> m_consecutiveFailures{0};
    std::atomic<uint32_t> m_consecutiveSuccesses{0};
    std::chrono::steady_clock::time_point m_circuitOpenTime;
    mutable std::mutex m_circuitMutex;
    
    // Statistics
    std::atomic<uint64_t> m_totalRequests{0};
    std::atomic<uint64_t> m_successfulRequests{0};
    std::atomic<uint64_t> m_failedRequests{0};
    std::atomic<uint64_t> m_fallbackRequests{0};
    std::atomic<uint64_t> m_circuitOpenCount{0};
    
    // Fallback
    bool m_fallbackEnabled = true;
    
    // Circuit Breaker Logic
    bool ShouldAllowRequest();
    void RecordSuccess();
    void RecordFailure();
    void OpenCircuit();
    void CloseCircuit();
    void TransitionToHalfOpen();
    
    // Worker Management
    bool EnsureWorkerRunning();
    bool RestartWorker();
    
    // IPC Helpers
    HRESULT SendThumbnailRequest(const wchar_t* filePath, uint32_t sizePx,
                                  IPC::ThumbnailResponse& response, uint32_t timeoutMs);
    HBITMAP ConvertResponseToBitmap(const IPC::ThumbnailResponse& response,
                                     const std::vector<uint8_t>& pixelData);
};

// =============================================================================
// Timeout Guard (RAII timeout enforcement)
// =============================================================================

class TimeoutGuard {
public:
    explicit TimeoutGuard(uint32_t timeoutMs);
    ~TimeoutGuard();
    
    bool IsExpired() const;
    uint32_t RemainingMs() const;
    
private:
    std::chrono::steady_clock::time_point m_startTime;
    std::chrono::milliseconds m_timeout;
};

// =============================================================================
// Request Context (Tracks individual requests)
// =============================================================================

struct RequestContext {
    uint64_t correlationId;
    std::wstring filePath;
    uint32_t sizePx;
    uint32_t requestId;
    std::chrono::steady_clock::time_point startTime;
    uint32_t timeoutMs;
    
    bool IsTimedOut() const {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime
        );
        return elapsed.count() >= timeoutMs;
    }
    
    uint32_t ElapsedMs() const {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime
        );
        return static_cast<uint32_t>(elapsed.count());
    }
};

// =============================================================================
// Request Queue Manager (For throttling)
// =============================================================================

class RequestQueueManager {
public:
    explicit RequestQueueManager(uint32_t maxQueueSize = 100);
    ~RequestQueueManager() = default;
    
    // Queue Operations
    bool Enqueue(const RequestContext& context);
    bool Dequeue(RequestContext& context, uint32_t timeoutMs = 0);
    bool Cancel(uint64_t correlationId);
    
    // Queue Status
    uint32_t GetQueueDepth() const { return m_queueDepth; }
    uint32_t GetMaxQueueSize() const { return m_maxQueueSize; }
    bool IsFull() const { return m_queueDepth >= m_maxQueueSize; }
    
    // Cleanup
    void RemoveExpiredRequests();
    void Clear();
    
private:
    uint32_t m_maxQueueSize;
    std::atomic<uint32_t> m_queueDepth{0};
    std::queue<RequestContext> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
};

// =============================================================================
// Retry Policy (For transient failures)
// =============================================================================

class RetryPolicy {
public:
    struct RetryConfig {
        uint32_t maxAttempts = 3;
        uint32_t initialBackoffMs = 100;
        uint32_t maxBackoffMs = 5000;
        double backoffMultiplier = 2.0;
        bool retryOnTimeout = true;
        bool retryOnConnectionError = true;
    };
    
    explicit RetryPolicy(const RetryConfig& config);
    
    bool ShouldRetry(IPC::ErrorCode errorCode, uint32_t attemptCount) const;
    uint32_t GetBackoffMs(uint32_t attemptCount) const;
    
private:
    RetryConfig m_config;
};

// =============================================================================
// Performance Metrics Collector
// =============================================================================

struct PerformanceMetrics {
    // Timing
    uint64_t totalRequestTimeUs;
    uint64_t ipcOverheadUs;
    uint64_t workerProcessingTimeUs;
    
    // Success/Failure
    bool success;
    IPC::ErrorCode errorCode;
    
    // Circuit Breaker
    CircuitState circuitStateAtRequest;
    bool fallbackUsed;
    
    // Retry
    uint32_t retryAttempts;
    
    // Queue
    uint32_t queueDepthAtRequest;
    uint32_t queueWaitTimeMs;
};

class MetricsCollector {
public:
    void RecordRequest(const PerformanceMetrics& metrics);
    
    // Aggregated Metrics
    double GetAverageRequestTimeMs() const;
    double GetAverageIPCOverheadMs() const;
    double GetSuccessRate() const;
    double GetFallbackRate() const;
    uint32_t GetP50LatencyMs() const;
    uint32_t GetP95LatencyMs() const;
    uint32_t GetP99LatencyMs() const;
    
    // Export
    std::string ExportJSON() const;
    void Reset();
    
private:
    std::vector<PerformanceMetrics> m_metrics;
    mutable std::mutex m_mutex;
    
    static const size_t MAX_METRICS = 10000; // Keep last 10k requests
};

// =============================================================================
// ShellHost Factory (Singleton pattern for Explorer)
// =============================================================================

class ShellHostFactory {
public:
    static ShellHostClient& GetInstance();
    static void Shutdown();
    
    // Configuration
    static void Configure(const Worker::WorkerConfig& workerConfig,
                         const CircuitBreakerConfig& circuitConfig);
    
private:
    ShellHostFactory() = default;
    ~ShellHostFactory() = default;
    
    static std::unique_ptr<ShellHostClient> s_instance;
    static std::once_flag s_initFlag;
    static Worker::WorkerConfig s_workerConfig;
    static CircuitBreakerConfig s_circuitConfig;
};

// =============================================================================
// Legacy Fallback Interface
// =============================================================================

namespace Legacy {

// These functions provide fallback to in-process thumbnail generation
// when worker is unavailable or circuit is open

HRESULT GenerateThumbnailInProcess(
    const wchar_t* filePath,
    uint32_t sizePx,
    HBITMAP* phBitmap
);

bool IsLegacyPathAvailable();

} // namespace Legacy

} // namespace ShellHost
} // namespace ExplorerLens

