// DeploymentPreflightCheck.h — Pre-Deployment System Validation
// Copyright (c) 2026 ExplorerLens Project
//
// Validates system readiness for deploying the shell extension by running
// five checks: COM CLSID registration, critical system DLL presence
// (kernel32, user32, gdi32, ole32, shell32, d3d11), temp-directory write
// permissions, Windows 10+ version requirement (via RtlGetVersion), and
// sufficient disk space. The report's deployReady flag is true only when
// no checks fail.
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

namespace ExplorerLens {
namespace Engine {

enum class PreflightCheckStatus : uint32_t {
    Pass = 0,
    Warning = 1,
    Fail = 2,
    Skipped = 3
};

struct PreflightCheckResult {
    std::wstring          checkName;
    PreflightCheckStatus  status = PreflightCheckStatus::Skipped;
    std::wstring          message;
    uint32_t              durationMs = 0;
};

struct PreflightReport {
    uint64_t                          timestamp = 0;
    uint32_t                          totalChecks = 0;
    uint32_t                          passed = 0;
    uint32_t                          warnings = 0;
    uint32_t                          failed = 0;
    uint32_t                          skipped = 0;
    bool                              deployReady = false;
    std::vector<PreflightCheckResult> checks;
};

// ========================================================================
// DeploymentPreflightCheck — Validates system readiness for deployment
// ========================================================================
class DeploymentPreflightCheck {
public:
    static DeploymentPreflightCheck& Instance() {
        static DeploymentPreflightCheck instance;
        return instance;
    }

    void Initialize() {
        m_initialized = true;
        m_totalRuns = 0;
    }

    bool IsInitialized() const { return m_initialized; }

    // Run all preflight checks
    PreflightReport RunAllChecks() {
        PreflightReport report;
        report.timestamp = GetTickCount64();

        // Check 1: COM registration
        report.checks.push_back(CheckCOMRegistration());

        // Check 2: DLL dependencies
        report.checks.push_back(CheckDLLDependencies());

        // Check 3: Write permissions
        report.checks.push_back(CheckWritePermissions());

        // Check 4: Windows version
        report.checks.push_back(CheckWindowsVersion());

        // Check 5: Disk space
        report.checks.push_back(CheckDiskSpace());

        // Tally results
        report.totalChecks = static_cast<uint32_t>(report.checks.size());
        for (auto& check : report.checks) {
            switch (check.status) {
            case PreflightCheckStatus::Pass:    report.passed++;   break;
            case PreflightCheckStatus::Warning: report.warnings++; break;
            case PreflightCheckStatus::Fail:    report.failed++;   break;
            case PreflightCheckStatus::Skipped: report.skipped++;  break;
            }
        }

        report.deployReady = (report.failed == 0);
        m_totalRuns++;
        m_lastReport = report;
        return report;
    }

    // Get last report
    PreflightReport GetLastReport() const { return m_lastReport; }

    // Get total runs
    uint64_t GetTotalRuns() const { return m_totalRuns; }

private:
    DeploymentPreflightCheck() = default;

    PreflightCheckResult CheckCOMRegistration() {
        PreflightCheckResult result;
        result.checkName = L"COM Registration";
        DWORD start = GetTickCount();

        HKEY hKey = nullptr;
        LONG status = RegOpenKeyExW(
            HKEY_CLASSES_ROOT,
            L"CLSID\\{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}",
            0, KEY_READ, &hKey);

        if (status == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            result.status = PreflightCheckStatus::Pass;
            result.message = L"COM CLSID registered";
        }
        else {
            result.status = PreflightCheckStatus::Warning;
            result.message = L"COM CLSID not registered (will register on install)";
        }

        result.durationMs = GetTickCount() - start;
        return result;
    }

    PreflightCheckResult CheckDLLDependencies() {
        PreflightCheckResult result;
        result.checkName = L"DLL Dependencies";
        DWORD start = GetTickCount();

        // Check critical system DLLs
        const wchar_t* criticalDlls[] = { L"kernel32.dll", L"user32.dll", L"gdi32.dll",
                                           L"ole32.dll", L"shell32.dll", L"d3d11.dll" };
        bool allPresent = true;
        for (auto dll : criticalDlls) {
            HMODULE hMod = GetModuleHandleW(dll);
            if (!hMod) {
                hMod = LoadLibraryExW(dll, nullptr, LOAD_LIBRARY_AS_DATAFILE);
                if (hMod) FreeLibrary(hMod);
                else allPresent = false;
            }
        }

        result.status = allPresent ? PreflightCheckStatus::Pass : PreflightCheckStatus::Fail;
        result.message = allPresent ? L"All critical DLLs available" : L"Missing critical DLLs";
        result.durationMs = GetTickCount() - start;
        return result;
    }

    PreflightCheckResult CheckWritePermissions() {
        PreflightCheckResult result;
        result.checkName = L"Write Permissions";
        DWORD start = GetTickCount();

        wchar_t tempPath[MAX_PATH];
        GetTempPathW(MAX_PATH, tempPath);

        std::wstring testFile = std::wstring(tempPath) + L"explorlens_preflight_test.tmp";
        HANDLE hFile = CreateFileW(testFile.c_str(), GENERIC_WRITE, 0, nullptr,
            CREATE_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE, nullptr);
        if (hFile != INVALID_HANDLE_VALUE) {
            CloseHandle(hFile);
            result.status = PreflightCheckStatus::Pass;
            result.message = L"Write permissions confirmed";
        }
        else {
            result.status = PreflightCheckStatus::Fail;
            result.message = L"Cannot write to temp directory";
        }

        result.durationMs = GetTickCount() - start;
        return result;
    }

    PreflightCheckResult CheckWindowsVersion() {
        PreflightCheckResult result;
        result.checkName = L"Windows Version";
        DWORD start = GetTickCount();

        // Use RtlGetVersion to avoid WIN32_LEAN_AND_MEAN issues with versionhelpers.h
        using RtlGetVersionFn = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);
        HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
        if (ntdll) {
            auto pRtlGetVersion = reinterpret_cast<RtlGetVersionFn>(
                GetProcAddress(ntdll, "RtlGetVersion"));
            if (pRtlGetVersion) {
                RTL_OSVERSIONINFOW osvi = {};
                osvi.dwOSVersionInfoSize = sizeof(osvi);
                if (pRtlGetVersion(&osvi) == 0) {
                    if (osvi.dwMajorVersion >= 10) {
                        result.status = PreflightCheckStatus::Pass;
                        result.message = L"Windows 10+ detected";
                    }
                    else {
                        result.status = PreflightCheckStatus::Fail;
                        result.message = L"Windows 10+ required";
                    }
                }
            }
        }

        if (result.status == PreflightCheckStatus::Skipped) {
            result.status = PreflightCheckStatus::Warning;
            result.message = L"Could not determine Windows version";
        }

        result.durationMs = GetTickCount() - start;
        return result;
    }

    PreflightCheckResult CheckDiskSpace() {
        PreflightCheckResult result;
        result.checkName = L"Disk Space";
        DWORD start = GetTickCount();

        ULARGE_INTEGER freeBytes;
        if (GetDiskFreeSpaceExW(L"C:\\", &freeBytes, nullptr, nullptr)) {
            uint64_t freeMB = freeBytes.QuadPart / (1024 * 1024);
            if (freeMB >= 100) {
                result.status = PreflightCheckStatus::Pass;
                result.message = L"Sufficient disk space (>100MB)";
            }
            else {
                result.status = PreflightCheckStatus::Warning;
                result.message = L"Low disk space (<100MB)";
            }
        }
        else {
            result.status = PreflightCheckStatus::Skipped;
            result.message = L"Could not check disk space";
        }

        result.durationMs = GetTickCount() - start;
        return result;
    }

    PreflightReport m_lastReport;
    uint64_t m_totalRuns = 0;
    bool m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
