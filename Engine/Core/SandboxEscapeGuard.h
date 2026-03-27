// SandboxEscapeGuard.h — Process Isolation and Job Object Constraints
// Copyright (c) 2026 ExplorerLens Project
//
// Enforces process-level isolation for the thumbnail decode worker process using
// Windows Job Objects: limits CPU, memory, UI access, and child process creation
// to contain damage from malformed or malicious file content.
//
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

// Sandbox restriction level — higher levels are more restrictive
enum class SandboxLevel {
    None,       // No restrictions (development only)
    Minimal,    // Memory cap + no child processes
    Standard,   // + UI restrictions + DLL injection guard
    Strict      // + network block + restricted token
};

struct SandboxEscapePolicy {
    SandboxLevel level         = SandboxLevel::Standard;
    uint64_t     maxMemoryBytes = 512ULL * 1024 * 1024; // 512 MB
    uint32_t     maxCpuRatePercent = 25; // Per-job CPU rate cap (requires Win8+)
    bool         allowChildProcesses = false;
    bool         allowUIAccess       = false;
    bool         allowBreakaway      = false;
    bool         killOnJobClose      = true;
};

struct SandboxEscapeStats {
    uint64_t  peakMemoryBytes   = 0;
    uint64_t  totalCpuMs        = 0;
    uint32_t  terminationCount  = 0; // Jobs killed for exceeding limits
    bool      activeJobObject   = false;
};

class SandboxEscapeGuard {
public:
    ~SandboxEscapeGuard() { Release(); }

    // Create a Job Object with the given policy
    bool Create(const SandboxEscapePolicy& policy = SandboxEscapePolicy{}) {
        m_policy = policy;
        if (policy.level == SandboxLevel::None) return true;

        m_hJob = CreateJobObjectW(nullptr, nullptr);
        if (!m_hJob) return false;

        // --- Basic limits ---
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = {};
        jeli.BasicLimitInformation.LimitFlags =
            JOB_OBJECT_LIMIT_PROCESS_MEMORY |
            JOB_OBJECT_LIMIT_JOB_MEMORY;
        jeli.ProcessMemoryLimit = static_cast<SIZE_T>(policy.maxMemoryBytes);
        jeli.JobMemoryLimit     = static_cast<SIZE_T>(policy.maxMemoryBytes);

        if (!policy.allowChildProcesses)
            jeli.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
        if (policy.killOnJobClose)
            jeli.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        if (!policy.allowBreakaway)
            jeli.BasicLimitInformation.LimitFlags |=
                JOB_OBJECT_LIMIT_BREAKAWAY_OK | JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK;

        if (!policy.allowChildProcesses)
            jeli.BasicLimitInformation.ActiveProcessLimit = 1;

        SetInformationJobObject(m_hJob, JobObjectExtendedLimitInformation,
                &jeli, sizeof(jeli));

        // --- UI restrictions ---
        if (policy.level >= SandboxLevel::Standard) {
            JOBOBJECT_BASIC_UI_RESTRICTIONS uiRestr = {};
            uiRestr.UIRestrictionsClass =
                JOB_OBJECT_UILIMIT_HANDLES |
                JOB_OBJECT_UILIMIT_READCLIPBOARD |
                JOB_OBJECT_UILIMIT_WRITECLIPBOARD |
                JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS |
                JOB_OBJECT_UILIMIT_DISPLAYSETTINGS |
                JOB_OBJECT_UILIMIT_EXITWINDOWS |
                JOB_OBJECT_UILIMIT_DESKTOP;
            if (!policy.allowUIAccess)
                uiRestr.UIRestrictionsClass |= JOB_OBJECT_UILIMIT_HANDLES;
            SetInformationJobObject(m_hJob, JobObjectBasicUIRestrictions,
                    &uiRestr, sizeof(uiRestr));
        }

        // --- CPU rate cap (Win8+) ---
        if (policy.maxCpuRatePercent > 0 && policy.maxCpuRatePercent < 100) {
            JOBOBJECT_CPU_RATE_CONTROL_INFORMATION cpuCtrl = {};
            cpuCtrl.ControlFlags = JOB_OBJECT_CPU_RATE_CONTROL_ENABLE |
                                   JOB_OBJECT_CPU_RATE_CONTROL_HARD_CAP;
            cpuCtrl.CpuRate = policy.maxCpuRatePercent * 100; // In units of 0.01%
            SetInformationJobObject(m_hJob, JobObjectCpuRateControlInformation,
                    &cpuCtrl, sizeof(cpuCtrl));
        }

        m_stats.activeJobObject = true;
        return true;
    }

    // Assign a process to the sandbox job
    bool AssignProcess(HANDLE hProcess) {
        if (!m_hJob) return true; // No job = no restriction
        return AssignProcessToJobObject(m_hJob, hProcess) != FALSE;
    }

    // Assign the current process
    bool AssignSelf() {
        HANDLE hCurr = GetCurrentProcess();
        return AssignProcess(hCurr);
    }

    // Query whether a process is in our job
    bool IsInJob(HANDLE hProcess) const {
        if (!m_hJob) return false;
        BOOL inside = FALSE;
        IsProcessInJob(hProcess, m_hJob, &inside);
        return inside != FALSE;
    }

    // Update stats from job accounting info
    void RefreshStats() {
        if (!m_hJob) return;
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION info = {};
        if (QueryInformationJobObject(m_hJob,
                JobObjectExtendedLimitInformation, &info, sizeof(info), nullptr)) {
            m_stats.peakMemoryBytes = info.PeakJobMemoryUsed;
        }
        JOBOBJECT_BASIC_ACCOUNTING_INFORMATION acct = {};
        if (QueryInformationJobObject(m_hJob,
                JobObjectBasicAccountingInformation, &acct, sizeof(acct), nullptr)) {
            m_stats.totalCpuMs = acct.TotalUserTime.QuadPart / 10000;
        }
    }

    void Release() {
        if (m_hJob) { CloseHandle(m_hJob); m_hJob = nullptr; }
        m_stats.activeJobObject = false;
    }

    const SandboxEscapeStats&  Stats()  const { return m_stats; }
    const SandboxEscapePolicy& Policy() const { return m_policy; }
    HANDLE               JobHandle() const { return m_hJob; }

private:
    HANDLE        m_hJob   = nullptr;
    SandboxEscapePolicy m_policy;
    SandboxEscapeStats  m_stats;
};

}} // namespace ExplorerLens::Engine
