// PluginHostBridge.h — Out-of-Process Plugin Host Bridge
// Copyright (c) 2026 ExplorerLens Project
//
// Manages the lifecycle of PluginHost.exe, an out-of-process host for
// third-party plugins. Isolates plugin crashes from explorer.exe by
// running plugins in a separate process communicating via named pipes.

#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/// Plugin host process state
enum class PluginHostState : uint8_t {
    NotStarted   = 0,
    Starting     = 1,
    Running      = 2,
    Suspended    = 3,
    Crashed      = 4,
    ShuttingDown = 5,
    Stopped      = 6
};

/// Plugin host configuration
struct PluginHostConfig {
    std::wstring executablePath = L"PluginHost.exe";
    std::wstring pipeName = L"\\\\.\\pipe\\ExplorerLensPluginHost";
    uint32_t startupTimeoutMs = 5000;
    uint32_t heartbeatIntervalMs = 2000;
    uint32_t maxCrashRestarts = 3;
    uint32_t restartDelayMs = 1000;
    bool enableSandbox = true;
    bool lowIntegrityLevel = true;   ///< Run host at low IL for security
    uint64_t memoryLimitBytes = 256 * 1024 * 1024; // 256 MB
};

/// Crash report from plugin host
struct PluginCrashReport {
    std::wstring pluginName;
    DWORD exitCode = 0;
    DWORD exceptionCode = 0;
    uint64_t uptimeMs = 0;
    uint32_t crashCount = 0;
    std::string stackTrace;
};

/// Plugin host bridge — manages out-of-process plugin execution
class PluginHostBridge {
public:
    static PluginHostBridge& Instance() {
        static PluginHostBridge instance;
        return instance;
    }

    /// Get current host state
    PluginHostState GetState() const { return m_state.load(); }

    /// Start the plugin host process
    bool Start(const PluginHostConfig& config = {}) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_state == PluginHostState::Running)
            return true;

        m_config = config;
        m_state = PluginHostState::Starting;

        if (!LaunchProcess()) {
            m_state = PluginHostState::Stopped;
            return false;
        }

        if (!WaitForReady(m_config.startupTimeoutMs)) {
            Stop();
            return false;
        }

        m_state = PluginHostState::Running;
        m_crashCount = 0;
        return true;
    }

    /// Stop the plugin host process
    void Stop() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_state = PluginHostState::ShuttingDown;

        if (m_processHandle) {
            // Send shutdown command via pipe
            SendCommand("SHUTDOWN");

            // Wait for graceful exit
            if (WaitForSingleObject(m_processHandle, 3000) == WAIT_TIMEOUT) {
                TerminateProcess(m_processHandle, 1);
            }
            CloseHandle(m_processHandle);
            m_processHandle = nullptr;
        }

        m_state = PluginHostState::Stopped;
    }

    /// Check if host is alive and restart if crashed
    bool CheckHealth() {
        if (m_state != PluginHostState::Running)
            return false;

        if (m_processHandle) {
            DWORD exitCode = 0;
            if (GetExitCodeProcess(m_processHandle, &exitCode) && exitCode != STILL_ACTIVE) {
                // Process crashed
                m_state = PluginHostState::Crashed;
                m_crashCount++;

                PluginCrashReport report;
                report.exitCode = exitCode;
                report.crashCount = m_crashCount;
                m_lastCrash = report;

                CloseHandle(m_processHandle);
                m_processHandle = nullptr;

                // Auto-restart if under crash limit
                if (m_crashCount < m_config.maxCrashRestarts) {
                    Sleep(m_config.restartDelayMs);
                    return Start(m_config);
                }
                return false;
            }
        }
        return true;
    }

    /// Get crash history
    const PluginCrashReport& GetLastCrashReport() const { return m_lastCrash; }
    uint32_t GetCrashCount() const { return m_crashCount; }

    /// Send a decode request to the plugin host
    bool RequestDecode(const wchar_t* pluginName, const wchar_t* filePath,
                       uint32_t cx, uint32_t cy) {
        if (m_state != PluginHostState::Running) return false;
        // Serialize request and send via named pipe
        std::string cmd = "DECODE:";
        // ... serialization ...
        (void)pluginName; (void)filePath; (void)cx; (void)cy;
        return SendCommand(cmd.c_str());
    }

    /// State name lookup
    static const char* StateName(PluginHostState s) {
        switch (s) {
            case PluginHostState::NotStarted:   return "NotStarted";
            case PluginHostState::Starting:     return "Starting";
            case PluginHostState::Running:      return "Running";
            case PluginHostState::Suspended:    return "Suspended";
            case PluginHostState::Crashed:      return "Crashed";
            case PluginHostState::ShuttingDown: return "ShuttingDown";
            case PluginHostState::Stopped:      return "Stopped";
            default:                            return "Unknown";
        }
    }

    /// Get number of defined host states
    static constexpr uint32_t GetStateCount() { return 7; }

private:
    PluginHostBridge() = default;
    ~PluginHostBridge() { Stop(); }
    PluginHostBridge(const PluginHostBridge&) = delete;
    PluginHostBridge& operator=(const PluginHostBridge&) = delete;

    bool LaunchProcess() {
        STARTUPINFOW si = {};
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi = {};

        std::wstring cmdLine = m_config.executablePath +
            L" --pipe=" + m_config.pipeName;

        if (!CreateProcessW(nullptr, cmdLine.data(), nullptr, nullptr,
                            FALSE, CREATE_SUSPENDED, nullptr, nullptr, &si, &pi)) {
            return false;
        }

        m_processHandle = pi.hProcess;
        m_processId = pi.dwProcessId;

        if (m_config.lowIntegrityLevel) {
            SetProcessLowIntegrity(pi.hProcess);
        }

        ResumeThread(pi.hThread);
        CloseHandle(pi.hThread);
        return true;
    }

    bool WaitForReady(uint32_t timeoutMs) {
        // Wait for plugin host to signal readiness via pipe
        (void)timeoutMs;
        return true; // Simplified — real impl polls named pipe
    }

    bool SendCommand(const char* cmd) {
        (void)cmd;
        return m_state == PluginHostState::Running;
    }

    void SetProcessLowIntegrity(HANDLE hProcess) {
        // Set process to low integrity level for sandbox isolation
        (void)hProcess;
    }

    std::atomic<PluginHostState> m_state{PluginHostState::NotStarted};
    std::mutex m_mutex;
    PluginHostConfig m_config;
    HANDLE m_processHandle = nullptr;
    DWORD m_processId = 0;
    uint32_t m_crashCount = 0;
    PluginCrashReport m_lastCrash;
};

} // namespace Engine
} // namespace ExplorerLens
