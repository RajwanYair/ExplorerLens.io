// PluginCrashIsolation.h — Crash Isolation and Recovery for Plugins
// Copyright (c) 2026 ExplorerLens Project
//
// Provides crash isolation without SEH (to avoid C++ destructor conflicts).
// Uses C++ exception handling to catch plugin failures and implements a
// circuit breaker pattern with per-plugin crash counters, health scores,
// and recovery strategies. Generates crash reports with diagnostic info.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// Recovery strategy when a plugin crash is detected
enum class CrashRecoveryStrategy : uint32_t {
    Retry    = 0,   // Retry the operation
    Disable  = 1,   // Disable the plugin
    Unload   = 2,   // Unload the plugin DLL
    Restart  = 3    // Restart the plugin in a new context
};

inline const wchar_t* CrashRecoveryStrategyName(CrashRecoveryStrategy s) {
    switch (s) {
        case CrashRecoveryStrategy::Retry:   return L"Retry";
        case CrashRecoveryStrategy::Disable: return L"Disable";
        case CrashRecoveryStrategy::Unload:  return L"Unload";
        case CrashRecoveryStrategy::Restart: return L"Restart";
        default:                             return L"Unknown";
    }
}

// Crash report with diagnostic information
struct CrashIsolationReport {
    std::wstring pluginId;
    std::chrono::system_clock::time_point timestamp;
    uint32_t exceptionCode = 0;
    uint64_t faultAddress = 0;
    std::wstring faultModule;
    std::wstring description;
    CrashRecoveryStrategy appliedStrategy = CrashRecoveryStrategy::Disable;
    uint32_t crashIndex = 0;  // Which crash number this is for the plugin
};

// Per-plugin crash tracking state
struct PluginCrashState {
    std::wstring pluginId;
    uint32_t crashCount = 0;
    uint32_t maxCrashesBeforeDisable = 3;
    bool disabled = false;
    double healthScore = 100.0;   // 0.0 to 100.0
    std::chrono::steady_clock::time_point lastCrashTime;
    std::chrono::steady_clock::time_point registrationTime;
    std::vector<CrashIsolationReport> crashHistory;

    // Circuit breaker: is the plugin allowed to execute?
    bool IsCircuitOpen() const {
        return disabled || (crashCount >= maxCrashesBeforeDisable);
    }

    // Recalculate health score based on crash frequency
    void UpdateHealthScore() {
        if (crashCount == 0) {
            healthScore = 100.0;
            return;
        }
        // Each crash reduces health by 25 points, floor at 0
        double penalty = static_cast<double>(crashCount) * 25.0;
        healthScore = (penalty >= 100.0) ? 0.0 : (100.0 - penalty);
    }
};

// Result of executing a plugin operation inside the isolation boundary
enum class IsolatedCallResult : uint32_t {
    Success       = 0,
    CppException  = 1,   // std::exception caught
    UnknownCrash  = 2,   // catch(...) triggered
    CircuitOpen   = 3    // Plugin is disabled (circuit breaker tripped)
};

inline const wchar_t* IsolatedCallResultName(IsolatedCallResult r) {
    switch (r) {
        case IsolatedCallResult::Success:      return L"Success";
        case IsolatedCallResult::CppException: return L"CppException";
        case IsolatedCallResult::UnknownCrash: return L"UnknownCrash";
        case IsolatedCallResult::CircuitOpen:  return L"CircuitOpen";
        default:                               return L"Unknown";
    }
}

// ========================================================================
// PluginCrashIsolation — per-plugin crash tracking + circuit breaker
// ========================================================================
class PluginCrashIsolation {
public:
    static PluginCrashIsolation& Instance() {
        static PluginCrashIsolation instance;
        return instance;
    }

    void Initialize(uint32_t defaultMaxCrashes = 3) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_defaultMaxCrashes = defaultMaxCrashes;
        m_pluginStates.clear();
        m_totalCrashes.store(0, std::memory_order_relaxed);
        m_totalCalls.store(0, std::memory_order_relaxed);
        m_initialized = true;
    }

    bool IsInitialized() const { return m_initialized; }

    // Register a plugin for crash tracking
    void RegisterPlugin(const std::wstring& pluginId, uint32_t maxCrashes = 0) {
        std::lock_guard<std::mutex> lock(m_mutex);
        PluginCrashState state;
        state.pluginId = pluginId;
        state.maxCrashesBeforeDisable = (maxCrashes > 0) ? maxCrashes : m_defaultMaxCrashes;
        state.registrationTime = std::chrono::steady_clock::now();
        m_pluginStates[pluginId] = std::move(state);
    }

    // Execute a plugin operation with crash isolation
    IsolatedCallResult ExecuteIsolated(const std::wstring& pluginId,
                                        std::function<void()> operation) {
        m_totalCalls.fetch_add(1, std::memory_order_relaxed);

        // Check circuit breaker
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_pluginStates.find(pluginId);
            if (it != m_pluginStates.end() && it->second.IsCircuitOpen()) {
                return IsolatedCallResult::CircuitOpen;
            }
        }

        try {
            operation();
            return IsolatedCallResult::Success;
        }
        catch (const std::exception& ex) {
            RecordCrash(pluginId, 0xE0000001, 0, L"CppException",
                        std::wstring(L"std::exception: ") +
                        std::wstring(ex.what(), ex.what() + strlen(ex.what())));
            return IsolatedCallResult::CppException;
        }
        catch (...) {
            RecordCrash(pluginId, 0xE0000002, 0, L"Unknown",
                        L"Unknown exception caught in plugin isolation boundary");
            return IsolatedCallResult::UnknownCrash;
        }
    }

    // Manually record a crash (e.g., from external SEH wrapper)
    void RecordCrash(const std::wstring& pluginId,
                     uint32_t exceptionCode,
                     uint64_t faultAddress,
                     const std::wstring& faultModule,
                     const std::wstring& description) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_totalCrashes.fetch_add(1, std::memory_order_relaxed);

        auto it = m_pluginStates.find(pluginId);
        if (it == m_pluginStates.end()) {
            // Auto-register
            RegisterPluginLocked(pluginId);
            it = m_pluginStates.find(pluginId);
        }

        auto& state = it->second;
        state.crashCount++;
        state.lastCrashTime = std::chrono::steady_clock::now();
        state.UpdateHealthScore();

        // Generate crash report
        CrashIsolationReport report;
        report.pluginId = pluginId;
        report.timestamp = std::chrono::system_clock::now();
        report.exceptionCode = exceptionCode;
        report.faultAddress = faultAddress;
        report.faultModule = faultModule;
        report.description = description;
        report.crashIndex = state.crashCount;

        // Determine recovery strategy
        if (state.crashCount >= state.maxCrashesBeforeDisable) {
            report.appliedStrategy = CrashRecoveryStrategy::Disable;
            state.disabled = true;
        }
        else if (state.crashCount >= 2) {
            report.appliedStrategy = CrashRecoveryStrategy::Unload;
        }
        else {
            report.appliedStrategy = CrashRecoveryStrategy::Retry;
        }

        state.crashHistory.push_back(std::move(report));
    }

    // Get crash state for a plugin
    PluginCrashState GetState(const std::wstring& pluginId) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_pluginStates.find(pluginId);
        return (it != m_pluginStates.end()) ? it->second : PluginCrashState{};
    }

    // Get health score for a plugin (0-100)
    double GetHealthScore(const std::wstring& pluginId) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_pluginStates.find(pluginId);
        return (it != m_pluginStates.end()) ? it->second.healthScore : 100.0;
    }

    // Check if plugin circuit breaker is open (disabled)
    bool IsDisabled(const std::wstring& pluginId) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_pluginStates.find(pluginId);
        return (it != m_pluginStates.end()) ? it->second.IsCircuitOpen() : false;
    }

    // Reset a plugin's crash state (manual recovery)
    void ResetPlugin(const std::wstring& pluginId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_pluginStates.find(pluginId);
        if (it != m_pluginStates.end()) {
            it->second.crashCount = 0;
            it->second.disabled = false;
            it->second.healthScore = 100.0;
            it->second.crashHistory.clear();
        }
    }

    // Get total crash count across all plugins
    uint64_t GetTotalCrashes() const { return m_totalCrashes.load(std::memory_order_relaxed); }
    uint64_t GetTotalCalls() const { return m_totalCalls.load(std::memory_order_relaxed); }

    uint32_t GetRegisteredPluginCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return static_cast<uint32_t>(m_pluginStates.size());
    }

private:
    PluginCrashIsolation() = default;

    void RegisterPluginLocked(const std::wstring& pluginId) {
        PluginCrashState state;
        state.pluginId = pluginId;
        state.maxCrashesBeforeDisable = m_defaultMaxCrashes;
        state.registrationTime = std::chrono::steady_clock::now();
        m_pluginStates[pluginId] = std::move(state);
    }

    mutable std::mutex m_mutex;
    std::unordered_map<std::wstring, PluginCrashState> m_pluginStates;
    uint32_t m_defaultMaxCrashes = 3;
    std::atomic<uint64_t> m_totalCrashes{0};
    std::atomic<uint64_t> m_totalCalls{0};
    bool m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
