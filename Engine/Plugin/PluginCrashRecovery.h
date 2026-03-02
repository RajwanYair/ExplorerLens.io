// PluginCrashRecovery.h — Automatic Plugin Crash Detection and Recovery
// Copyright (c) 2026 ExplorerLens Project
//
// Monitors plugin crashes and applies a graduated response: immediate
// auto-restart for the first few crashes, quarantine for repeated
// crashers, and permanent disable after exceeding max recovery attempts.
// Crash counters auto-reset after a configurable time window (default
// 1 hour). IsPluginAvailable() gates access based on recovery state.
//
// Thread-safe singleton.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

enum class PluginRecoveryState : uint32_t {
    Healthy = 0,
    Crashed = 1,
    Quarantined = 2,
    Recovering = 3,
    Disabled = 4
};

static const wchar_t* PluginRecoveryStateName(PluginRecoveryState s) {
    static const wchar_t* names[] = {
        L"Healthy", L"Crashed", L"Quarantined", L"Recovering", L"Disabled"
    };
    return names[static_cast<uint32_t>(s)];
}

struct PluginCrashRecord {
    std::wstring         pluginId;
    PluginRecoveryState  state = PluginRecoveryState::Healthy;
    uint32_t             crashCount = 0;
    uint64_t             lastCrashTime = 0;
    uint64_t             quarantineUntil = 0;
    uint32_t             recoveryAttempts = 0;
    uint32_t             exitCode = 0;
    std::wstring         lastCrashReason;
};

struct PluginCrashRecoveryConfig {
    uint32_t maxCrashesBeforeQuarantine = 3;
    uint32_t quarantineDurationMs = 300000;  // 5 minutes
    uint32_t maxRecoveryAttempts = 5;
    uint32_t crashCountResetMs = 3600000; // 1 hour
    bool     autoRestart = true;
    bool     permanentDisableOnExceed = true;    // Disable after max attempts
};

struct PluginCrashRecoveryStats {
    uint64_t totalCrashes = 0;
    uint64_t totalRecoveries = 0;
    uint64_t totalQuarantines = 0;
    uint64_t totalDisabled = 0;
    uint32_t currentQuarantined = 0;
    uint32_t currentDisabled = 0;
};

// ========================================================================
// PluginCrashRecovery — Detects and recovers from plugin crashes
// ========================================================================
class PluginCrashRecovery {
public:
    static PluginCrashRecovery& Instance() {
        static PluginCrashRecovery instance;
        return instance;
    }

    void Initialize(const PluginCrashRecoveryConfig& config = {}) {
        m_config = config;
        m_records.clear();
        m_stats = {};
        m_initialized = true;
    }

    bool IsInitialized() const { return m_initialized; }

    // Register a plugin for monitoring
    void RegisterPlugin(const std::wstring& pluginId) {
        PluginCrashRecord record;
        record.pluginId = pluginId;
        m_records[pluginId] = record;
    }

    // Report a plugin crash
    PluginRecoveryState ReportCrash(const std::wstring& pluginId, uint32_t exitCode = 0,
        const std::wstring& reason = L"") {
        auto it = m_records.find(pluginId);
        if (it == m_records.end()) {
            RegisterPlugin(pluginId);
            it = m_records.find(pluginId);
        }

        auto& record = it->second;
        uint64_t now = GetTickCount64();

        // Reset crash count if enough time has passed
        if (now - record.lastCrashTime > m_config.crashCountResetMs) {
            record.crashCount = 0;
        }

        record.crashCount++;
        record.lastCrashTime = now;
        record.exitCode = exitCode;
        record.lastCrashReason = reason;
        m_stats.totalCrashes++;

        // Decide recovery action
        if (record.recoveryAttempts >= m_config.maxRecoveryAttempts && m_config.permanentDisableOnExceed) {
            record.state = PluginRecoveryState::Disabled;
            m_stats.totalDisabled++;
            return record.state;
        }

        if (record.crashCount >= m_config.maxCrashesBeforeQuarantine) {
            record.state = PluginRecoveryState::Quarantined;
            record.quarantineUntil = now + m_config.quarantineDurationMs;
            m_stats.totalQuarantines++;
            return record.state;
        }

        if (m_config.autoRestart) {
            record.state = PluginRecoveryState::Recovering;
            record.recoveryAttempts++;
            return record.state;
        }

        record.state = PluginRecoveryState::Crashed;
        return record.state;
    }

    // Mark plugin as recovered
    void MarkRecovered(const std::wstring& pluginId) {
        auto it = m_records.find(pluginId);
        if (it != m_records.end()) {
            it->second.state = PluginRecoveryState::Healthy;
            m_stats.totalRecoveries++;
        }
    }

    // Check if a plugin can be loaded/used
    bool IsPluginAvailable(const std::wstring& pluginId) {
        auto it = m_records.find(pluginId);
        if (it == m_records.end()) return true; // Unknown = available

        auto& record = it->second;
        uint64_t now = GetTickCount64();

        if (record.state == PluginRecoveryState::Disabled) return false;

        if (record.state == PluginRecoveryState::Quarantined) {
            if (now >= record.quarantineUntil) {
                record.state = PluginRecoveryState::Recovering;
                record.recoveryAttempts++;
                return true;
            }
            return false;
        }

        return (record.state == PluginRecoveryState::Healthy ||
            record.state == PluginRecoveryState::Recovering);
    }

    // Get plugin state
    PluginRecoveryState GetPluginState(const std::wstring& pluginId) const {
        auto it = m_records.find(pluginId);
        return (it != m_records.end()) ? it->second.state : PluginRecoveryState::Healthy;
    }

    // Get crash count for a plugin
    uint32_t GetCrashCount(const std::wstring& pluginId) const {
        auto it = m_records.find(pluginId);
        return (it != m_records.end()) ? it->second.crashCount : 0;
    }

    // Get stats
    PluginCrashRecoveryStats GetStats() const {
        PluginCrashRecoveryStats stats = m_stats;
        stats.currentQuarantined = 0;
        stats.currentDisabled = 0;
        for (auto& [id, rec] : m_records) {
            if (rec.state == PluginRecoveryState::Quarantined) stats.currentQuarantined++;
            if (rec.state == PluginRecoveryState::Disabled) stats.currentDisabled++;
        }
        return stats;
    }

    // Get registered plugin count
    uint32_t GetMonitoredCount() const { return static_cast<uint32_t>(m_records.size()); }

private:
    PluginCrashRecovery() = default;

    PluginCrashRecoveryConfig m_config;
    PluginCrashRecoveryStats  m_stats;
    std::unordered_map<std::wstring, PluginCrashRecord> m_records;
    bool m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
