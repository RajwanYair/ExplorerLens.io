/******************************************************************************
 * DarkThumbs Crash Handler
 * Copyright (c) 2026 - DarkThumbs Project
 * 
 * Detects and recovers from plugin crashes. Automatically disables crashed
 * plugins and provides crash reporting.
 *****************************************************************************/

#pragma once

#include <Windows.h>
#include <string>
#include <functional>
#include <chrono>
#include <mutex>
#include <unordered_set>
#include <unordered_map>

namespace DarkThumbs {

//============================================================================
// Crash Information
//============================================================================

struct CrashInfo {
    std::wstring plugin_id;
    std::wstring plugin_path;
    DWORD exit_code;
    uint64_t correlation_id;
    std::wstring file_path;
    std::chrono::system_clock::time_point timestamp;
    
    // Crash type categorization
    bool IsAccessViolation() const {
        return exit_code == STATUS_ACCESS_VIOLATION;
    }
    
    bool IsStackOverflow() const {
        return exit_code == STATUS_STACK_OVERFLOW;
    }
    
    bool IsDivideByZero() const {
        return exit_code == STATUS_INTEGER_DIVIDE_BY_ZERO;
    }
    
    bool IsIllegalInstruction() const {
        return exit_code == STATUS_ILLEGAL_INSTRUCTION;
    }
    
    bool IsCrash() const {
        return IsAccessViolation() || IsStackOverflow() || 
               IsDivideByZero() || IsIllegalInstruction();
    }
    
    const wchar_t* GetCrashTypeName() const {
        if (IsAccessViolation()) return L"Access Violation";
        if (IsStackOverflow()) return L"Stack Overflow";
        if (IsDivideByZero()) return L"Divide by Zero";
        if (IsIllegalInstruction()) return L"Illegal Instruction";
        return L"Unknown";
    }
};

//============================================================================
// Crash Handler
//============================================================================

class CrashHandler {
public:
    using CrashCallback = std::function<void(const CrashInfo&)>;
    
    static CrashHandler& Instance() {
        static CrashHandler instance;
        return instance;
    }
    
    // Non-copyable, non-movable
    CrashHandler(const CrashHandler&) = delete;
    CrashHandler& operator=(const CrashHandler&) = delete;
    
    // Detect if a process crashed
    bool DetectCrash(HANDLE process_handle,
                    const std::wstring& plugin_id,
                    const std::wstring& plugin_path,
                    uint64_t correlation_id = 0,
                    const std::wstring& file_path = L"");
    
    // Check if a plugin is disabled due to crashes
    bool IsPluginDisabled(const std::wstring& plugin_id) const;
    
    // Disable a plugin
    void DisablePlugin(const std::wstring& plugin_id, const CrashInfo& crash_info);
    
    // Re-enable a plugin (admin/user action)
    void EnablePlugin(const std::wstring& plugin_id);
    
    // Get crash count for a plugin
    uint32_t GetCrashCount(const std::wstring& plugin_id) const;
    
    // Get last crash info for a plugin
    bool GetLastCrash(const std::wstring& plugin_id, CrashInfo& info) const;
    
    // Set crash callback (for logging, telemetry, notifications)
    void SetCrashCallback(CrashCallback callback) {
        crash_callback_ = callback;
    }
    
    // Clear crash history (testing/development)
    void ClearHistory();
    
    // Load/Save disabled plugins from registry
    void LoadDisabledPlugins();
    void SaveDisabledPlugins();

private:
    CrashHandler();
    ~CrashHandler();
    
    void RecordCrash(const CrashInfo& crash_info);
    void NotifyCrash(const CrashInfo& crash_info);
    
    // Disabled plugins (plugin_id -> crash info)
    std::unordered_map<std::wstring, CrashInfo> disabled_plugins_;
    
    // Crash history (plugin_id -> count)
    std::unordered_map<std::wstring, uint32_t> crash_counts_;
    
    // Last crash info (plugin_id -> crash info)
    std::unordered_map<std::wstring, CrashInfo> last_crashes_;
    
    // Callback
    CrashCallback crash_callback_;
    
    // Thread safety
    mutable std::mutex mutex_;
};

//============================================================================
// Helper Functions
//============================================================================

// Get crash exit code name
const wchar_t* GetExitCodeName(DWORD exit_code);

// Check if exit code indicates a crash
bool IsCrashExitCode(DWORD exit_code);

// Format crash info for logging
std::wstring FormatCrashInfo(const CrashInfo& crash_info);

} // namespace DarkThumbs
