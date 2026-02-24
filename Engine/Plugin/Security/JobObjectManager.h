/******************************************************************************
 * ExplorerLens Job Object Manager
 * Copyright (c) 2026 - ExplorerLens Project
 * 
 * Manages Job Objects for enforcing resource limits on plugin processes.
 *****************************************************************************/

#pragma once

#include <Windows.h>
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Security {

//============================================================================
// Job Object Configuration
//============================================================================

struct JobObjectLimits {
    // Memory limits
    uint64_t process_memory_limit = 512 * 1024 * 1024;  // 512 MB
    uint64_t job_memory_limit = 512 * 1024 * 1024;       // 512 MB
    
    // CPU limits
    uint64_t per_process_user_time_limit_ms = 60000;     // 60 seconds
    
    // Process limits
    uint32_t active_process_limit = 1;
    
    // Flags
    bool kill_on_job_close = true;
    bool die_on_unhandled_exception = true;
    bool breakaway_ok = false;
    bool silent_breakaway_ok = false;
    
    // I/O limits (optional, may not be supported on all Windows versions)
    bool enable_io_rate_control = false;
    uint64_t max_bandwidth_bytes_per_sec = 10 * 1024 * 1024;  // 10 MB/s
};

//============================================================================
// Job Object Manager
//============================================================================

class JobObjectManager {
public:
    JobObjectManager();
    ~JobObjectManager();
    
    // Non-copyable
    JobObjectManager(const JobObjectManager&) = delete;
    JobObjectManager& operator=(const JobObjectManager&) = delete;
    
    // Create a job object with specified limits
    bool Create(const std::wstring& name, const JobObjectLimits& limits);
    
    // Assign a process to the job
    bool AssignProcess(HANDLE process_handle);
    
    // Query job information
    bool GetBasicAccountingInfo(JOBOBJECT_BASIC_ACCOUNTING_INFORMATION& info);
    bool GetBasicLimitInfo(JOBOBJECT_BASIC_LIMIT_INFORMATION& info);
    bool GetExtendedLimitInfo(JOBOBJECT_EXTENDED_LIMIT_INFORMATION& info);
    
    // Check if limits are being exceeded
    bool IsMemoryLimitExceeded();
    bool IsCPUTimeLimitExceeded();
    
    // Terminate all processes in the job
    bool TerminateAllProcesses(uint32_t exit_code);
    
    // Close the job handle
    void Close();
    
    // Check if job is valid
    bool IsValid() const { return job_handle_ != nullptr; }
    
    // Get handle
    HANDLE GetHandle() const { return job_handle_; }

private:
    bool SetBasicLimits(const JobObjectLimits& limits);
    bool SetExtendedLimits(const JobObjectLimits& limits);
    bool SetNotificationLimits(const JobObjectLimits& limits);
    
    HANDLE job_handle_ = nullptr;
    JobObjectLimits limits_;
};

//============================================================================
// Helper Functions
//============================================================================

// Create a job object with default plugin limits
JobObjectManager* CreatePluginJobObject(const std::wstring& plugin_id);

// Convert milliseconds to 100-nanosecond intervals (Windows time)
inline LARGE_INTEGER MillisecondsToFileTime(uint64_t milliseconds) {
    LARGE_INTEGER ft;
    ft.QuadPart = milliseconds * 10000LL;  // 1 ms = 10,000 * 100ns
    return ft;
}

} // namespace Security
} // namespace ExplorerLens

