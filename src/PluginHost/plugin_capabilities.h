// plugin_capabilities.h - ExplorerLens Plugin Capability Gate System v1.0
//
// PURPOSE:
//   Runtime capability enforcement and permission checking for plugins.
//   Prevents plugins from accessing resources they're not authorized to use.
//   Integrates with OS-level security (tokens, ACLs, Job Objects) and
//   application-level gates (network, GPU, file system).
//
// CAPABILITY MODEL:
//   - Capabilities are declared in plugin manifest
//   - Verified against signature and trust level
//   - Enforced at runtime through interception hooks
//   - Policy can further restrict capabilities
//   - Violations logged and result in plugin termination
//
// ENFORCEMENT LAYERS:
//   1. Manifest declaration (compile-time)
//   2. Signature verification (load-time)
//   3. Policy check (load-time)
//   4. Runtime gate checks (execution-time)
//   5. OS-level security (AppContainer, restricted token)
//
// Created: January 6, 2026
// Version: 1.0.0

#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <atomic>
#include <mutex>
#include <map>
#include <set>

namespace ExplorerLens {
namespace PluginCapabilities {

// Version
constexpr uint32_t CAPABILITY_SYSTEM_VERSION = 1;

//=============================================================================
// CAPABILITY DEFINITIONS
//=============================================================================

// Individual capabilities (bit flags)
enum Capability : uint64_t {
    // File system access
    CAP_FILE_READ_LOCAL = 0x0000000000000001ULL,        // Read local files
    CAP_FILE_READ_NETWORK = 0x0000000000000002ULL,      // Read network shares
    CAP_FILE_WRITE_LOCAL = 0x0000000000000004ULL,       // Write local files
    CAP_FILE_WRITE_NETWORK = 0x0000000000000008ULL,     // Write network shares
    CAP_FILE_DELETE = 0x0000000000000010ULL,            // Delete files
    CAP_FILE_EXECUTE = 0x0000000000000020ULL,           // Execute files
    
    // Network access
    CAP_NETWORK_HTTP = 0x0000000000000100ULL,           // HTTP/HTTPS requests
    CAP_NETWORK_SOCKET = 0x0000000000000200ULL,         // Raw socket access
    CAP_NETWORK_SERVER = 0x0000000000000400ULL,         // Listen on ports
    CAP_NETWORK_DNS = 0x0000000000000800ULL,            // DNS queries
    
    // GPU access
    CAP_GPU_COMPUTE = 0x0000000000001000ULL,            // Compute shaders
    CAP_GPU_RENDER = 0x0000000000002000ULL,             // Rendering
    CAP_GPU_MEMORY = 0x0000000000004000ULL,             // GPU memory allocation
    
    // System access
    CAP_REGISTRY_READ = 0x0000000000010000ULL,          // Read registry
    CAP_REGISTRY_WRITE = 0x0000000000020000ULL,         // Write registry
    CAP_PROCESS_CREATE = 0x0000000000040000ULL,         // Spawn processes
    CAP_PROCESS_QUERY = 0x0000000000080000ULL,          // Query process info
    CAP_CLIPBOARD = 0x0000000000100000ULL,              // Clipboard access
    CAP_SHELL_INTEGRATION = 0x0000000000200000ULL,      // Shell API access
    
    // Advanced features
    CAP_CRYPTO = 0x0000000001000000ULL,                 // Cryptography APIs
    CAP_COM = 0x0000000002000000ULL,                    // COM automation
    CAP_WMI = 0x0000000004000000ULL,                    // WMI queries
    CAP_PERFORMANCE_COUNTERS = 0x0000000008000000ULL,   // Perf counters
    
    // Special capabilities
    CAP_UNRESTRICTED = 0x8000000000000000ULL,           // No restrictions (dangerous)
    
    // Convenience combinations
    CAP_FILE_READ_ALL = CAP_FILE_READ_LOCAL | CAP_FILE_READ_NETWORK,
    CAP_FILE_WRITE_ALL = CAP_FILE_WRITE_LOCAL | CAP_FILE_WRITE_NETWORK,
    CAP_FILE_ALL = CAP_FILE_READ_ALL | CAP_FILE_WRITE_ALL | CAP_FILE_DELETE,
    
    CAP_NETWORK_CLIENT = CAP_NETWORK_HTTP | CAP_NETWORK_SOCKET | CAP_NETWORK_DNS,
    CAP_NETWORK_ALL = CAP_NETWORK_CLIENT | CAP_NETWORK_SERVER,
    
    CAP_GPU_ALL = CAP_GPU_COMPUTE | CAP_GPU_RENDER | CAP_GPU_MEMORY,
    
    CAP_REGISTRY_ALL = CAP_REGISTRY_READ | CAP_REGISTRY_WRITE,
    
    CAP_PROCESS_ALL = CAP_PROCESS_CREATE | CAP_PROCESS_QUERY,
    
    // Standard profiles
    CAP_MINIMAL = CAP_FILE_READ_LOCAL,  // Absolute minimum for decoder
    
    CAP_STANDARD = CAP_FILE_READ_LOCAL | CAP_GPU_COMPUTE | CAP_GPU_RENDER,
    
    CAP_TRUSTED = CAP_STANDARD | CAP_FILE_WRITE_LOCAL | CAP_NETWORK_HTTP | CAP_REGISTRY_READ,
    
    CAP_NONE = 0ULL
};

//=============================================================================
// CAPABILITY SET
//=============================================================================

// Represents a set of capabilities
class CapabilitySet {
public:
    CapabilitySet() : caps_(0) {}
    explicit CapabilitySet(uint64_t caps) : caps_(caps) {}
    
    // Add/remove capabilities
    void Add(Capability cap) { caps_ |= static_cast<uint64_t>(cap); }
    void Remove(Capability cap) { caps_ &= ~static_cast<uint64_t>(cap); }
    void Clear() { caps_ = 0; }
    
    // Query
    bool Has(Capability cap) const { return (caps_ & static_cast<uint64_t>(cap)) != 0; }
    bool HasAll(uint64_t required) const { return (caps_ & required) == required; }
    bool HasAny(uint64_t options) const { return (caps_ & options) != 0; }
    bool IsEmpty() const { return caps_ == 0; }
    
    // Operations
    CapabilitySet Intersect(const CapabilitySet& other) const {
        return CapabilitySet(caps_ & other.caps_);
    }
    
    CapabilitySet Union(const CapabilitySet& other) const {
        return CapabilitySet(caps_ | other.caps_);
    }
    
    CapabilitySet Difference(const CapabilitySet& other) const {
        return CapabilitySet(caps_ & ~other.caps_);
    }
    
    // Conversion
    uint64_t ToUInt64() const { return caps_; }
    std::wstring ToString() const;
    std::vector<std::wstring> ToStringList() const;
    
    static CapabilitySet FromString(const std::wstring& str);
    
private:
    uint64_t caps_;
};

//=============================================================================
// CAPABILITY MANIFEST
//=============================================================================

// Plugin capability manifest (embedded in plugin or sidecar file)
struct CapabilityManifest {
    uint32_t version = 1;
    std::wstring pluginName;
    std::wstring pluginVersion;
    
    CapabilitySet requestedCapabilities;
    CapabilitySet optionalCapabilities;  // Nice to have, but can work without
    
    // Justification for each capability (for user display)
    std::map<Capability, std::wstring> justifications;
    
    // Restrictions
    std::vector<std::wstring> allowedFilePaths;      // Whitelisted paths
    std::vector<std::wstring> allowedNetworkHosts;   // Whitelisted domains
    std::vector<std::wstring> allowedRegistryKeys;   // Whitelisted registry keys
    
    CapabilityManifest() = default;
    
    bool IsValid() const;
    static CapabilityManifest Load(const std::wstring& manifestPath);
    bool Save(const std::wstring& manifestPath) const;
};

//=============================================================================
// CAPABILITY CONTEXT
//=============================================================================

// Runtime capability context for a plugin instance
struct CapabilityContext {
    std::wstring pluginPath;
    CapabilitySet grantedCapabilities;      // Capabilities actually granted
    CapabilitySet requestedCapabilities;    // Capabilities requested
    CapabilitySet deniedCapabilities;       // Capabilities denied by policy
    
    // Runtime state
    std::atomic<uint64_t> violationCount{0};
    std::chrono::system_clock::time_point creationTime;
    
    // Policy restrictions
    std::vector<std::wstring> allowedFilePaths;
    std::vector<std::wstring> allowedNetworkHosts;
    std::vector<std::wstring> allowedRegistryKeys;
    
    CapabilityContext() : creationTime(std::chrono::system_clock::now()) {}
};

//=============================================================================
// CAPABILITY CHECKER
//=============================================================================

// Callback for capability violations
using ViolationCallback = std::function<void(
    const std::wstring& pluginPath,
    Capability violatedCap,
    const std::wstring& details
)>;

// Runtime capability checker and enforcer
class CapabilityChecker {
public:
    explicit CapabilityChecker(const CapabilityContext& context);
    ~CapabilityChecker() = default;
    
    // Primary check methods
    bool CheckFileAccess(const std::wstring& path, Capability requiredCap);
    bool CheckNetworkAccess(const std::wstring& host, uint16_t port, Capability requiredCap);
    bool CheckGPUAccess(Capability requiredCap);
    bool CheckRegistryAccess(const std::wstring& keyPath, Capability requiredCap);
    bool CheckProcessAccess(Capability requiredCap);
    bool CheckCapability(Capability cap);
    
    // Path/host validation
    bool IsFilePathAllowed(const std::wstring& path) const;
    bool IsNetworkHostAllowed(const std::wstring& host) const;
    bool IsRegistryKeyAllowed(const std::wstring& keyPath) const;
    
    // Statistics
    uint64_t GetViolationCount() const { return context_.violationCount.load(); }
    void ResetViolations() { context_.violationCount = 0; }
    
    // Callbacks
    void SetViolationCallback(ViolationCallback callback) { violationCallback_ = callback; }
    
    // Context
    const CapabilityContext& GetContext() const { return context_; }
    
private:
    void RecordViolation(Capability cap, const std::wstring& details);
    bool MatchPathPattern(const std::wstring& path, const std::wstring& pattern) const;
    
    CapabilityContext context_;
    ViolationCallback violationCallback_;
    mutable std::mutex checkerMutex_;
};

//=============================================================================
// CAPABILITY ENFORCER
//=============================================================================

// Global capability enforcement coordinator
class CapabilityEnforcer {
public:
    static CapabilityEnforcer& Instance();
    
    // Plugin registration
    bool RegisterPlugin(const std::wstring& pluginPath, const CapabilityManifest& manifest);
    bool UnregisterPlugin(const std::wstring& pluginPath);
    
    // Get checker for plugin
    std::shared_ptr<CapabilityChecker> GetChecker(const std::wstring& pluginPath);
    
    // Policy application
    CapabilitySet ApplyPolicy(const CapabilitySet& requested, const std::wstring& pluginPath);
    
    // Violation handling
    void SetGlobalViolationCallback(ViolationCallback callback);
    
    // Statistics
    struct EnforcerStats {
        uint32_t registeredPlugins = 0;
        uint64_t totalViolations = 0;
        uint64_t fileAccessViolations = 0;
        uint64_t networkViolations = 0;
        uint64_t gpuViolations = 0;
        uint64_t registryViolations = 0;
        uint64_t processViolations = 0;
    };
    
    EnforcerStats GetStats() const;
    void ResetStats();
    
private:
    CapabilityEnforcer() = default;
    ~CapabilityEnforcer() = default;
    
    mutable std::mutex enforcerMutex_;
    std::map<std::wstring, std::shared_ptr<CapabilityContext>> contexts_;
    std::map<std::wstring, std::shared_ptr<CapabilityChecker>> checkers_;
    ViolationCallback globalCallback_;
    EnforcerStats stats_;
};

//=============================================================================
// CAPABILITY GATES
//=============================================================================

// File system gate
class FileSystemGate {
public:
    static bool CheckAccess(
        const CapabilityChecker& checker,
        const std::wstring& path,
        DWORD desiredAccess
    );
    
    static bool InterceptCreateFile(
        const CapabilityChecker& checker,
        const std::wstring& path,
        DWORD desiredAccess,
        HANDLE& hFile
    );
    
    static bool InterceptDeleteFile(
        const CapabilityChecker& checker,
        const std::wstring& path
    );
};

// Network gate
class NetworkGate {
public:
    static bool CheckAccess(
        const CapabilityChecker& checker,
        const std::wstring& host,
        uint16_t port
    );
    
    static bool InterceptConnect(
        const CapabilityChecker& checker,
        const std::wstring& host,
        uint16_t port,
        SOCKET& sock
    );
    
    static bool InterceptBind(
        const CapabilityChecker& checker,
        uint16_t port,
        SOCKET& sock
    );
};

// GPU gate
class GPUGate {
public:
    static bool CheckAccess(
        const CapabilityChecker& checker,
        Capability requiredCap
    );
    
    static bool InterceptDeviceCreation(
        const CapabilityChecker& checker,
        void** ppDevice
    );
    
    static bool InterceptResourceAllocation(
        const CapabilityChecker& checker,
        size_t sizeBytes
    );
};

// Registry gate
class RegistryGate {
public:
    static bool CheckAccess(
        const CapabilityChecker& checker,
        const std::wstring& keyPath,
        bool isWrite
    );
    
    static bool InterceptRegOpenKey(
        const CapabilityChecker& checker,
        HKEY hKey,
        const std::wstring& subKey,
        REGSAM samDesired,
        HKEY& hResult
    );
    
    static bool InterceptRegSetValue(
        const CapabilityChecker& checker,
        HKEY hKey,
        const std::wstring& valueName
    );
};

// Process gate
class ProcessGate {
public:
    static bool CheckAccess(
        const CapabilityChecker& checker,
        Capability requiredCap
    );
    
    static bool InterceptCreateProcess(
        const CapabilityChecker& checker,
        const std::wstring& applicationName,
        PROCESS_INFORMATION& procInfo
    );
    
    static bool InterceptOpenProcess(
        const CapabilityChecker& checker,
        DWORD desiredAccess,
        DWORD processId,
        HANDLE& hProcess
    );
};

//=============================================================================
// POLICY INTEGRATION
//=============================================================================

// Capability policy (integrates with Group Policy)
class CapabilityPolicy {
public:
    static CapabilityPolicy& Instance();
    
    struct PolicyConfig {
        // Global restrictions
        CapabilitySet blockedCapabilities;      // Never allow these
        CapabilitySet requiresUserConsent;      // Require user approval
        
        // Trust level mappings
        CapabilitySet untrustedAllowed;         // Allowed for untrusted plugins
        CapabilitySet codeSignedAllowed;        // Allowed for code-signed
        CapabilitySet verifiedAllowed;          // Allowed for verified publishers
        
        // Feature flags
        bool enforcePathRestrictions = true;
        bool enforceNetworkRestrictions = true;
        bool allowUnrestrictedCap = false;      // Allow CAP_UNRESTRICTED (never in production)
        
        PolicyConfig();
    };
    
    void LoadPolicy();  // Load from registry/GP
    void SavePolicy();
    
    const PolicyConfig& GetPolicy() const { return policy_; }
    void SetPolicy(const PolicyConfig& policy);
    
    // Apply policy to capability set
    CapabilitySet ApplyPolicy(const CapabilitySet& requested, uint32_t trustLevel) const;
    
private:
    CapabilityPolicy() = default;
    ~CapabilityPolicy() = default;
    
    PolicyConfig policy_;
    mutable std::mutex policyMutex_;
};

//=============================================================================
// HELPER FUNCTIONS
//=============================================================================

// Get human-readable name for capability
std::wstring GetCapabilityName(Capability cap);

// Get description for capability
std::wstring GetCapabilityDescription(Capability cap);

// Parse capability from string
Capability ParseCapability(const std::wstring& str);

// Validate capability manifest
bool ValidateManifest(const CapabilityManifest& manifest, std::vector<std::wstring>& errors);

// Get recommended capabilities for format type
CapabilitySet GetRecommendedCapabilities(const std::wstring& formatType);

} // namespace PluginCapabilities
} // namespace ExplorerLens

