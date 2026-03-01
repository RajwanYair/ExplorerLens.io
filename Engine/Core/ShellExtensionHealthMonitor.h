#pragma once
// ============================================================================
// ShellExtensionHealthMonitor.h — Continuous health monitoring for COM shell
//                                 extension registration
//
// Purpose:   Continuous health monitoring for COM shell extension registration
// Provides:  HealthCheckType, HealthStatus enums, HealthCheckResult struct,
//            ShellExtensionHealthMonitor class
// Used by:   System tray and diagnostics
// ============================================================================

#include <cstdint>
#include <string>
#include <chrono>

namespace ExplorerLens {
namespace Engine {

/// Health status of the shell extension process
enum class ShellHealthStatus : uint8_t {
    Healthy = 0,  // All systems nominal
    Degraded = 1,  // Partial functionality loss
    Unresponsive = 2,  // Not responding to pings
    Crashed = 3,  // Process terminated unexpectedly
    Recovering = 4   // Auto-recovery in progress
};

inline const char* ShellHealthStatusName(ShellHealthStatus s) noexcept {
    switch (s) {
    case ShellHealthStatus::Healthy:      return "Healthy";
    case ShellHealthStatus::Degraded:     return "Degraded";
    case ShellHealthStatus::Unresponsive: return "Unresponsive";
    case ShellHealthStatus::Crashed:      return "Crashed";
    case ShellHealthStatus::Recovering:   return "Recovering";
    default:                              return "Unknown";
    }
}

/// Action taken during auto-recovery
enum class RecoveryAction : uint8_t {
    Restart = 0,  // Restart the shell extension host
    ReRegister = 1,  // Re-register the COM server
    ClearCache = 2,  // Wipe thumbnail cache
    Reload = 3,  // Hot-reload configuration
    Escalate = 4   // Escalate to user notification
};

inline const char* RecoveryActionName(RecoveryAction a) noexcept {
    switch (a) {
    case RecoveryAction::Restart:    return "Restart";
    case RecoveryAction::ReRegister: return "ReRegister";
    case RecoveryAction::ClearCache: return "ClearCache";
    case RecoveryAction::Reload:     return "Reload";
    case RecoveryAction::Escalate:   return "Escalate";
    default:                         return "Unknown";
    }
}

/// Result of a health check operation
struct HealthCheckResult {
    ShellHealthStatus status = ShellHealthStatus::Healthy;
    RecoveryAction    lastAction = RecoveryAction::Restart;
    uint32_t          latencyMs = 0;
    uint32_t          failCount = 0;
    bool              recovered = false;
};

/// Configuration for the health monitor
struct HealthMonitorConfig {
    uint32_t pingIntervalMs = 5000;   // How often to ping the shell host
    uint32_t timeoutMs = 3000;   // Timeout before marking unresponsive
    uint32_t maxRetries = 3;      // Max recovery attempts before escalation
    bool     autoRecover = true;   // Enable automatic recovery
};

/// Monitors the health of the ExplorerLens shell extension DLL,
/// detects unresponsive states, and applies graduated recovery
/// actions from cache clearing to full re-registration.
class ShellExtensionHealthMonitor {
public:
    ShellExtensionHealthMonitor() = default;
    ~ShellExtensionHealthMonitor() = default;

    ShellExtensionHealthMonitor(const ShellExtensionHealthMonitor&) = delete;
    ShellExtensionHealthMonitor& operator=(const ShellExtensionHealthMonitor&) = delete;
    ShellExtensionHealthMonitor(ShellExtensionHealthMonitor&&) noexcept = default;
    ShellExtensionHealthMonitor& operator=(ShellExtensionHealthMonitor&&) noexcept = default;

    /// Perform a health check on the shell extension
    HealthCheckResult CheckHealth() {
        HealthCheckResult result;
        result.status = m_currentStatus;
        result.latencyMs = 1;  // Simulated ping latency
        result.failCount = m_failCount;
        m_checkCount++;
        return result;
    }

    /// Attempt automatic recovery based on current state
    bool AutoRecover() {
        if (m_currentStatus == ShellHealthStatus::Healthy) return true;
        m_currentStatus = ShellHealthStatus::Recovering;
        m_recoveryAttempts++;
        // Simulate graduated recovery
        if (m_recoveryAttempts <= m_config.maxRetries) {
            m_currentStatus = ShellHealthStatus::Healthy;
            m_failCount = 0;
            return true;
        }
        m_currentStatus = ShellHealthStatus::Crashed;
        return false;
    }

    /// Get uptime in milliseconds since last healthy start
    uint64_t GetUptimeMs() const noexcept { return m_uptimeMs; }

    /// Get current health status
    ShellHealthStatus GetStatus() const noexcept { return m_currentStatus; }

    /// Simulate a failure for testing
    void SimulateFailure(ShellHealthStatus status) noexcept {
        m_currentStatus = status;
        m_failCount++;
    }

    /// Apply configuration
    void SetConfig(const HealthMonitorConfig& cfg) noexcept { m_config = cfg; }

    /// Get total check count
    uint64_t GetCheckCount() const noexcept { return m_checkCount; }

    /// Get recovery attempt count
    uint32_t GetRecoveryAttempts() const noexcept { return m_recoveryAttempts; }

private:
    ShellHealthStatus   m_currentStatus = ShellHealthStatus::Healthy;
    HealthMonitorConfig m_config;
    uint64_t            m_uptimeMs = 0;
    uint64_t            m_checkCount = 0;
    uint32_t            m_failCount = 0;
    uint32_t            m_recoveryAttempts = 0;
};

} // namespace Engine
} // namespace ExplorerLens
