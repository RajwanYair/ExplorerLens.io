/******************************************************************************
 * ExplorerLens Job Object Manager Implementation
 * Copyright (c) 2026 - ExplorerLens Project
 *****************************************************************************/

#include "JobObjectManager.h"
#include <sstream>

namespace ExplorerLens {
namespace Security {

//============================================================================
// JobObjectManager Implementation
//============================================================================

JobObjectManager::JobObjectManager() = default;

JobObjectManager::~JobObjectManager()
{
    Close();
}

bool JobObjectManager::Create(const std::wstring& name, const JobObjectLimits& limits)
{
    if (IsValid()) {
        return false;  // Already created
    }

    limits_ = limits;

    // Create the job object
    job_handle_ = CreateJobObjectW(nullptr, name.empty() ? nullptr : name.c_str());
    if (!job_handle_) {
        return false;
    }

    // Set limits
    if (!SetBasicLimits(limits)) {
        Close();
        return false;
    }

    if (!SetExtendedLimits(limits)) {
        Close();
        return false;
    }

    if (!SetNotificationLimits(limits)) {
        Close();
        return false;
    }

    return true;
}

bool JobObjectManager::SetBasicLimits(const JobObjectLimits& limits)
{
    JOBOBJECT_BASIC_LIMIT_INFORMATION basic_limits = {};

    // Set limit flags
    basic_limits.LimitFlags = 0;

    if (limits.kill_on_job_close) {
        basic_limits.LimitFlags |= JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    }

    if (limits.die_on_unhandled_exception) {
        basic_limits.LimitFlags |= JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION;
    }

    if (!limits.breakaway_ok) {
        basic_limits.LimitFlags |= JOB_OBJECT_LIMIT_BREAKAWAY_OK;
    }

    if (!limits.silent_breakaway_ok) {
        basic_limits.LimitFlags |= JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK;
    }

    // Active process limit
    basic_limits.LimitFlags |= JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
    basic_limits.ActiveProcessLimit = limits.active_process_limit;

    // CPU time limit
    if (limits.per_process_user_time_limit_ms > 0) {
        basic_limits.LimitFlags |= JOB_OBJECT_LIMIT_PROCESS_TIME;
        basic_limits.PerProcessUserTimeLimit = MillisecondsToFileTime(limits.per_process_user_time_limit_ms);
    }

    return SetInformationJobObject(job_handle_, JobObjectBasicLimitInformation, &basic_limits, sizeof(basic_limits))
           != 0;
}

bool JobObjectManager::SetExtendedLimits(const JobObjectLimits& limits)
{
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION extended_limits = {};

    // Copy basic limits
    GetBasicLimitInfo(extended_limits.BasicLimitInformation);

    // Add memory limits
    if (limits.process_memory_limit > 0) {
        extended_limits.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_PROCESS_MEMORY;
        extended_limits.ProcessMemoryLimit = limits.process_memory_limit;
    }

    if (limits.job_memory_limit > 0) {
        extended_limits.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_JOB_MEMORY;
        extended_limits.JobMemoryLimit = limits.job_memory_limit;
    }

    return SetInformationJobObject(job_handle_, JobObjectExtendedLimitInformation, &extended_limits,
                                   sizeof(extended_limits))
           != 0;
}

bool JobObjectManager::SetNotificationLimits(const JobObjectLimits& limits)
{
    // Set notification limits (triggers when limits are reached)
    JOBOBJECT_NOTIFICATION_LIMIT_INFORMATION notification_limits = {};

    // Memory notification at 90% of limit
    if (limits.process_memory_limit > 0) {
        notification_limits.JobMemoryLimit = static_cast<uint64_t>(limits.job_memory_limit * 0.9);
        notification_limits.IoWriteBytesLimit = 0;
        notification_limits.IoReadBytesLimit = 0;
        notification_limits.PerJobUserTimeLimit.QuadPart = 0;

        // Set flags
        notification_limits.LimitFlags = JOB_OBJECT_LIMIT_JOB_MEMORY;
    }

    return SetInformationJobObject(job_handle_, JobObjectNotificationLimitInformation, &notification_limits,
                                   sizeof(notification_limits))
           != 0;
}

bool JobObjectManager::AssignProcess(HANDLE process_handle)
{
    if (!IsValid() || !process_handle) {
        return false;
    }

    return AssignProcessToJobObject(job_handle_, process_handle) != 0;
}

bool JobObjectManager::GetBasicAccountingInfo(JOBOBJECT_BASIC_ACCOUNTING_INFORMATION& info)
{
    if (!IsValid()) {
        return false;
    }

    return QueryInformationJobObject(job_handle_, JobObjectBasicAccountingInformation, &info, sizeof(info), nullptr)
           != 0;
}

bool JobObjectManager::GetBasicLimitInfo(JOBOBJECT_BASIC_LIMIT_INFORMATION& info)
{
    if (!IsValid()) {
        return false;
    }

    return QueryInformationJobObject(job_handle_, JobObjectBasicLimitInformation, &info, sizeof(info), nullptr) != 0;
}

bool JobObjectManager::GetExtendedLimitInfo(JOBOBJECT_EXTENDED_LIMIT_INFORMATION& info)
{
    if (!IsValid()) {
        return false;
    }

    return QueryInformationJobObject(job_handle_, JobObjectExtendedLimitInformation, &info, sizeof(info), nullptr) != 0;
}

bool JobObjectManager::IsMemoryLimitExceeded()
{
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION info = {};
    if (!GetExtendedLimitInfo(info)) {
        return false;
    }

    // Check if process memory limit flag is set
    if (info.BasicLimitInformation.LimitFlags & JOB_OBJECT_LIMIT_PROCESS_MEMORY) {
        JOBOBJECT_BASIC_ACCOUNTING_INFORMATION accounting = {};
        if (GetBasicAccountingInfo(accounting)) {
            // Note: We'd need process-specific memory info, this is a simplified check
            return false;
        }
    }

    return false;
}

bool JobObjectManager::IsCPUTimeLimitExceeded()
{
    JOBOBJECT_BASIC_ACCOUNTING_INFORMATION info = {};
    if (!GetBasicAccountingInfo(info)) {
        return false;
    }

    // Check if total user time exceeds limit
    uint64_t total_time_ms = info.TotalUserTime.QuadPart / 10000;
    return total_time_ms >= limits_.per_process_user_time_limit_ms;
}

bool JobObjectManager::TerminateAllProcesses(uint32_t exit_code)
{
    if (!IsValid()) {
        return false;
    }

    return TerminateJobObject(job_handle_, exit_code) != 0;
}

void JobObjectManager::Close()
{
    if (job_handle_) {
        CloseHandle(job_handle_);
        job_handle_ = nullptr;
    }
}

//============================================================================
// Helper Functions
//============================================================================

JobObjectManager* CreatePluginJobObject(const std::wstring& plugin_id)
{
    auto manager = new JobObjectManager();

    JobObjectLimits limits;
    std::wstring job_name = L"ExplorerLens-Plugin-" + plugin_id;

    if (!manager->Create(job_name, limits)) {
        delete manager;
        return nullptr;
    }

    return manager;
}

}  // namespace Security
}  // namespace ExplorerLens
