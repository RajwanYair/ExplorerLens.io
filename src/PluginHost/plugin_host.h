// plugin_host.h - ExplorerLens Plugin Host Process Architecture v1.0
//
// PURPOSE:
//   PluginHost is a restricted sandboxed process that loads and executes
//   third-party decoder plugins in isolation from Worker and Explorer.
//   Provides security boundaries, resource limits, and capability enforcement.
//
// SECURITY MODEL:
//   - Runs with restricted token (low integrity level)
//   - Job Object enforces memory/CPU/handle limits
//   - AppContainer isolation (optional, Windows 8+)
//   - Capability gates control network/GPU/file access
//   - Signature verification before plugin load
//
// ARCHITECTURE:
//   Explorer → ShellHost (COM) → Worker (IPC) → PluginHost (IPC) → Plugin.dll
//
// Created: January 6, 2026
// Version: 1.0.0

#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <atomic>
#include <mutex>
#include <map>

namespace ExplorerLens {
namespace PluginHost {

// Version
constexpr uint32_t PLUGIN_HOST_VERSION = 1;
constexpr uint32_t PLUGIN_HOST_PROTOCOL_VERSION = 1;

//=============================================================================
// SECURITY CONFIGURATION
//=============================================================================

// Security level for plugin execution
enum class SecurityLevel : uint32_t {
    UNRESTRICTED = 0,       // Full access (testing only, never in production)
    RESTRICTED_TOKEN = 1,   // Run with restricted token (default)
    LOW_INTEGRITY = 2,      // Low integrity level + restricted token
    APP_CONTAINER = 3       // AppContainer isolation (Windows 8+, recommended)
};

// Capability flags - what the plugin is allowed to access
enum PluginCapability : uint32_t {
    CAP_NONE = 0,
    CAP_FILE_READ = 0x0001,         // Read local files
    CAP_FILE_WRITE = 0x0002,        // Write local files (rare)
    CAP_NETWORK_CLIENT = 0x0004,    // Make outbound network requests
    CAP_NETWORK_SERVER = 0x0008,    // Listen on network (usually blocked)
    CAP_GPU_COMPUTE = 0x0010,       // Use GPU for decode/processing
    CAP_GPU_RENDER = 0x0020,        // Use GPU for rendering
    CAP_REGISTRY_READ = 0x0040,     // Read registry
    CAP_REGISTRY_WRITE = 0x0080,    // Write registry (usually blocked)
    CAP_CLIPBOARD = 0x0100,         // Access clipboard (usually blocked)
    CAP_PROCESS_CREATE = 0x0200,    // Spawn child processes (blocked)
    
    // Convenience combinations
    CAP_FILE_ALL = CAP_FILE_READ | CAP_FILE_WRITE,
    CAP_NETWORK_ALL = CAP_NETWORK_CLIENT | CAP_NETWORK_SERVER,
    CAP_GPU_ALL = CAP_GPU_COMPUTE | CAP_GPU_RENDER,
    CAP_REGISTRY_ALL = CAP_REGISTRY_READ | CAP_REGISTRY_WRITE,
    
    // Default for untrusted plugins
    CAP_MINIMAL = CAP_FILE_READ,
    
    // Default for verified plugins
    CAP_STANDARD = CAP_FILE_READ | CAP_GPU_COMPUTE | CAP_GPU_RENDER,
    
    // Full access (signed by trusted vendor)
    CAP_TRUSTED = 0xFFFFFFFF
};

// Security configuration for plugin execution
struct SecurityConfig {
    SecurityLevel securityLevel = SecurityLevel::LOW_INTEGRITY;
    uint32_t capabilities = CAP_STANDARD;
    bool enforceSignature = true;           // Require valid signature
    bool blockUnsigned = false;             // Reject unsigned plugins (policy controlled)
    bool enableAppContainer = false;        // Use AppContainer (Windows 8+)
    std::wstring appContainerName;          // AppContainer name if enabled
    
    // Resource limits (enforced via Job Object)
    uint64_t maxMemoryBytes = 512 * 1024 * 1024;    // 512 MB default
    uint32_t maxCpuPercent = 80;                     // 80% CPU throttle
    uint32_t maxHandles = 1000;                      // Handle limit
    uint32_t maxThreads = 16;                        // Thread limit
    
    // Timeouts
    std::chrono::milliseconds startupTimeout{10000};     // 10 seconds
    std::chrono::milliseconds operationTimeout{30000};   // 30 seconds per decode
    std::chrono::milliseconds idleTimeout{300000};       // 5 minutes idle shutdown
    
    SecurityConfig() = default;
};

//=============================================================================
// PLUGIN HOST STATE
//=============================================================================

enum class PluginHostState : uint32_t {
    STOPPED = 0,            // Not running
    STARTING = 1,           // Spawning process
    LOADING_PLUGIN = 2,     // Loading plugin DLL
    READY = 3,              // Ready to accept requests
    BUSY = 4,               // Processing request
    IDLE = 5,               // Idle (eligible for shutdown)
    STOPPING = 6,           // Graceful shutdown in progress
    CRASHED = 7,            // Process crashed
    UNRESPONSIVE = 8,       // Not responding to pings
    TERMINATED = 9,         // Forcibly terminated
    SECURITY_VIOLATION = 10 // Attempted forbidden operation
};

inline const wchar_t* ToString(PluginHostState state) {
    switch (state) {
        case PluginHostState::STOPPED: return L"STOPPED";
        case PluginHostState::STARTING: return L"STARTING";
        case PluginHostState::LOADING_PLUGIN: return L"LOADING_PLUGIN";
        case PluginHostState::READY: return L"READY";
        case PluginHostState::BUSY: return L"BUSY";
        case PluginHostState::IDLE: return L"IDLE";
        case PluginHostState::STOPPING: return L"STOPPING";
        case PluginHostState::CRASHED: return L"CRASHED";
        case PluginHostState::UNRESPONSIVE: return L"UNRESPONSIVE";
        case PluginHostState::TERMINATED: return L"TERMINATED";
        case PluginHostState::SECURITY_VIOLATION: return L"SECURITY_VIOLATION";
        default: return L"UNKNOWN";
    }
}

// Statistics for monitoring
struct PluginHostStats {
    std::chrono::system_clock::time_point startTime;
    std::chrono::milliseconds uptime{0};
    
    uint64_t requestsProcessed = 0;
    uint64_t requestsSucceeded = 0;
    uint64_t requestsFailed = 0;
    uint64_t requestsTimedOut = 0;
    uint64_t securityViolations = 0;
    
    uint64_t memoryUsageBytes = 0;
    uint32_t cpuUsagePercent = 0;
    uint32_t handleCount = 0;
    uint32_t threadCount = 0;
    
    uint32_t restartCount = 0;
    std::chrono::system_clock::time_point lastRestartTime;
    
    PluginHostStats() : startTime(std::chrono::system_clock::now()) {}
};

//=============================================================================
// PLUGIN HOST PROCESS
//=============================================================================

// Manages a single PluginHost process instance
class PluginHostProcess {
public:
    PluginHostProcess(const std::wstring& pluginPath, const SecurityConfig& config);
    ~PluginHostProcess();
    
    // Lifecycle
    bool Start();
    bool Stop(std::chrono::milliseconds timeout = std::chrono::milliseconds{5000});
    bool Restart();
    bool Terminate();  // Force kill
    
    // State
    PluginHostState GetState() const { return state_; }
    bool IsRunning() const;
    bool IsHealthy() const;
    bool IsIdle() const;
    
    // Health monitoring
    bool Ping(std::chrono::milliseconds timeout = std::chrono::milliseconds{1000});
    std::chrono::milliseconds GetLastPingLatency() const { return lastPingLatency_; }
    
    // Plugin operations (via IPC to the host process)
    bool LoadPlugin();
    bool UnloadPlugin();
    bool ValidatePlugin();  // Check signature, capabilities
    
    // Request processing
    struct DecodeRequest {
        uint64_t requestId;
        std::wstring filePath;
        uint32_t targetWidth;
        uint32_t targetHeight;
        uint32_t flags;
        std::chrono::milliseconds timeout;
    };
    
    struct DecodeResult {
        uint64_t requestId;
        bool success;
        std::vector<uint8_t> pixelData;
        uint32_t width;
        uint32_t height;
        uint32_t format;
        std::wstring errorMessage;
        std::chrono::milliseconds processingTime;
    };
    
    DecodeResult ProcessRequest(const DecodeRequest& request);
    
    // Statistics
    PluginHostStats GetStats() const;
    void ResetStats();
    
    // Configuration
    const SecurityConfig& GetConfig() const { return config_; }
    const std::wstring& GetPluginPath() const { return pluginPath_; }
    DWORD GetProcessId() const { return processId_; }
    
private:
    // Process management
    bool SpawnProcess();
    bool CreateRestrictedToken(HANDLE& hToken);
    bool CreateJobObject();
    bool SetJobLimits();
    bool CreateAppContainer();
    bool AssignToJobObject();
    
    // IPC (communication with PluginHost.exe)
    bool ConnectIPC();
    bool DisconnectIPC();
    bool SendMessage(const std::vector<uint8_t>& message);
    bool ReceiveMessage(std::vector<uint8_t>& message, std::chrono::milliseconds timeout);
    
    // Monitoring
    void UpdateStats();
    void MonitorThread();
    bool CheckResourceLimits();
    bool CheckCapabilityViolation();
    
    // State management
    void SetState(PluginHostState newState);
    bool TransitionState(PluginHostState from, PluginHostState to);
    
    std::wstring pluginPath_;
    SecurityConfig config_;
    
    std::atomic<PluginHostState> state_{PluginHostState::STOPPED};
    DWORD processId_ = 0;
    HANDLE hProcess_ = nullptr;
    HANDLE hToken_ = nullptr;
    HANDLE hJob_ = nullptr;
    void* pAppContainer_ = nullptr;  // PSECURITY_CAPABILITIES
    
    // IPC handles (named pipe to PluginHost.exe)
    HANDLE hPipeRead_ = nullptr;
    HANDLE hPipeWrite_ = nullptr;
    std::wstring pipeName_;
    
    // Monitoring
    std::unique_ptr<std::thread> monitorThread_;
    std::atomic<bool> stopMonitoring_{false};
    mutable std::mutex statsMutex_;
    PluginHostStats stats_;
    std::chrono::system_clock::time_point lastPingTime_;
    std::chrono::milliseconds lastPingLatency_{0};
    
    // Prevent copy/move
    PluginHostProcess(const PluginHostProcess&) = delete;
    PluginHostProcess& operator=(const PluginHostProcess&) = delete;
};

//=============================================================================
// PLUGIN HOST POOL
//=============================================================================

// Pool management strategy
enum class PoolStrategy : uint32_t {
    ON_DEMAND = 0,      // Create on first request, destroy when idle
    WARM_POOL = 1,      // Keep N processes warm
    DEDICATED = 2       // One process per plugin type
};

struct PoolConfig {
    PoolStrategy strategy = PoolStrategy::ON_DEMAND;
    uint32_t minInstances = 0;      // Minimum warm instances
    uint32_t maxInstances = 8;      // Maximum concurrent instances
    std::chrono::milliseconds idleTimeout{300000};  // 5 minutes
    bool enableAutoRestart = true;
    uint32_t maxRestarts = 5;       // Max restarts before giving up
};

// Manages a pool of PluginHost processes
class PluginHostPool {
public:
    explicit PluginHostPool(const PoolConfig& config);
    ~PluginHostPool();
    
    // Lifecycle
    bool Start();
    bool Stop();
    
    // Plugin host acquisition
    std::shared_ptr<PluginHostProcess> AcquireHost(
        const std::wstring& pluginPath,
        const SecurityConfig& securityConfig
    );
    
    void ReleaseHost(std::shared_ptr<PluginHostProcess> host);
    
    // Pool management
    uint32_t GetActiveCount() const;
    uint32_t GetIdleCount() const;
    uint32_t GetTotalCount() const;
    
    // Statistics
    struct PoolStats {
        uint32_t totalHosts = 0;
        uint32_t activeHosts = 0;
        uint32_t idleHosts = 0;
        uint32_t failedStarts = 0;
        uint32_t restarts = 0;
        uint64_t totalRequestsProcessed = 0;
    };
    
    PoolStats GetStats() const;
    
private:
    struct HostEntry {
        std::shared_ptr<PluginHostProcess> process;
        std::wstring pluginPath;
        bool inUse = false;
        std::chrono::system_clock::time_point lastUsed;
    };
    
    void MaintainPool();
    void CleanupIdleHosts();
    void WarmPool();
    
    PoolConfig config_;
    mutable std::mutex poolMutex_;
    std::vector<HostEntry> hosts_;
    std::unique_ptr<std::thread> maintenanceThread_;
    std::atomic<bool> running_{false};
};

//=============================================================================
// PLUGIN HOST FACTORY
//=============================================================================

// Factory for creating and managing PluginHost instances
class PluginHostFactory {
public:
    static PluginHostFactory& Instance();
    
    // Configuration
    void SetDefaultSecurityConfig(const SecurityConfig& config);
    void SetDefaultPoolConfig(const PoolConfig& config);
    
    // Plugin host creation
    std::shared_ptr<PluginHostProcess> CreateHost(
        const std::wstring& pluginPath,
        const SecurityConfig* config = nullptr
    );
    
    // Pool management
    std::shared_ptr<PluginHostPool> GetPool();
    
    // Capability checking
    static bool HasCapability(uint32_t capabilities, PluginCapability cap);
    static std::wstring CapabilitiesToString(uint32_t capabilities);
    
private:
    PluginHostFactory() = default;
    ~PluginHostFactory() = default;
    
    SecurityConfig defaultSecurityConfig_;
    PoolConfig defaultPoolConfig_;
    std::shared_ptr<PluginHostPool> pool_;
    mutable std::mutex factoryMutex_;
};

//=============================================================================
// HELPER FUNCTIONS
//=============================================================================

// Validate that plugin capabilities match policy
bool ValidateCapabilities(uint32_t requestedCaps, uint32_t allowedCaps);

// Get security level from policy
SecurityLevel GetSecurityLevelFromPolicy(const std::wstring& pluginPath);

// Get capabilities from plugin manifest
uint32_t GetPluginCapabilities(const std::wstring& pluginPath);

} // namespace PluginHost
} // namespace ExplorerLens

