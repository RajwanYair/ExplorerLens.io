// InstallerEnhancementsV2.h — Installer Experience Improvements
// ExplorerLens Engine v15.0.0 "Zenith" — Sprint 388
// Copyright (c) 2026 ExplorerLens Project
//
// Defines installer UX enhancements: progress feedback, prerequisite
// checking, upgrade path logic, rollback support, and MSIX sparse
// package integration for modern Windows deployments.

#pragma once

#include <cstdint>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/// Installer type
enum class InstallerType : uint8_t {
    MSI       = 0,   ///< WiX MSI (primary)
    InnoSetup = 1,   ///< Inno Setup EXE
    MSIX      = 2,   ///< MSIX package (modern)
    Portable  = 3,   ///< No-install / xcopy deploy
    Scoop     = 4    ///< Scoop package manager
};

/// Installation phase
enum class InstallPhase : uint8_t {
    PreCheck        = 0,   ///< Prerequisite validation
    Backup          = 1,   ///< Backup existing installation
    FileCopy        = 2,   ///< Copy binaries
    COMRegister     = 3,   ///< Register COM DLL
    ShellRefresh    = 4,   ///< SHChangeNotify
    ConfigMigrate   = 5,   ///< Migrate settings from old version
    Cleanup         = 6,   ///< Remove temp files
    Verify          = 7,   ///< Verify installation
    PhaseCount      = 8
};

/// Prerequisite check result
struct PrerequisiteCheck {
    const char* name = nullptr;       ///< e.g., "Visual C++ Runtime"
    const char* minVersion = nullptr; ///< e.g., "14.40"
    bool isRequired = true;
    bool isMet = false;
    const char* downloadUrl = nullptr;
    const char* message = nullptr;
};

/// Upgrade path definition
struct UpgradePath {
    const char* fromVersion = nullptr;
    const char* toVersion = nullptr;
    bool requiresReboot = false;
    bool requiresReregistration = true;
    bool migratesSettings = true;
    const char* notes = nullptr;
};

/// Installer enhancements manager
class InstallerEnhancementsV2 {
public:
    static InstallerEnhancementsV2& Instance() {
        static InstallerEnhancementsV2 inst;
        return inst;
    }

    /// Run prerequisite checks
    static constexpr uint32_t PREREQ_COUNT = 5;

    static PrerequisiteCheck CheckPrerequisite(uint32_t index) {
        static PrerequisiteCheck checks[] = {
            { "Windows 10 1809+",      "10.0.17763", true,  false, nullptr,
              "ExplorerLens requires Windows 10 version 1809 or later" },
            { "Visual C++ Runtime",    "14.40",      true,  false,
              "https://aka.ms/vs/17/release/vc_redist.x64.exe",
              "Microsoft Visual C++ 2022+ Redistributable required" },
            { "DirectX 11 Runtime",    "11.0",       false, false, nullptr,
              "GPU acceleration requires DirectX 11" },
            { "Administrator Rights",  nullptr,      true,  false, nullptr,
              "COM registration requires administrator privileges" },
            { "Disk Space (50MB)",     "50MB",       true,  false, nullptr,
              "At least 50MB of free disk space required" },
        };

        if (index >= PREREQ_COUNT) return {};

        // Perform actual checks
        auto& check = checks[index];
        switch (index) {
            case 0: // Windows version
                check.isMet = CheckWindowsVersion(17763);
                break;
            case 1: // VC++ runtime
                check.isMet = CheckVCRuntime();
                break;
            case 2: // DirectX
                check.isMet = CheckDirectX11();
                break;
            case 3: // Admin
                check.isMet = IsRunningAsAdmin();
                break;
            case 4: // Disk space
                check.isMet = CheckDiskSpace(50 * 1024 * 1024);
                break;
        }
        return check;
    }

    /// Run all prerequisite checks
    bool AllPrerequisitesMet() {
        for (uint32_t i = 0; i < PREREQ_COUNT; ++i) {
            auto check = CheckPrerequisite(i);
            if (check.isRequired && !check.isMet)
                return false;
        }
        return true;
    }

    /// Known upgrade paths
    static constexpr uint32_t UPGRADE_PATH_COUNT = 3;
    static const UpgradePath& GetUpgradePath(uint32_t index) {
        static const UpgradePath paths[] = {
            { "13.0.0", "15.0.0", false, true, true, "Major version upgrade - full re-registration" },
            { "14.0.0", "15.0.0", false, true, true, "Standard upgrade - settings preserved" },
            { "15.0.0-beta", "15.0.0", false, true, true, "Beta to release upgrade" },
        };
        static const UpgradePath empty{};
        return index < UPGRADE_PATH_COUNT ? paths[index] : empty;
    }

    /// Phase name
    static const char* PhaseName(InstallPhase p) {
        switch (p) {
            case InstallPhase::PreCheck:      return "Checking prerequisites";
            case InstallPhase::Backup:        return "Backing up current installation";
            case InstallPhase::FileCopy:       return "Copying files";
            case InstallPhase::COMRegister:   return "Registering COM components";
            case InstallPhase::ShellRefresh:   return "Refreshing Windows Shell";
            case InstallPhase::ConfigMigrate: return "Migrating settings";
            case InstallPhase::Cleanup:        return "Cleaning up";
            case InstallPhase::Verify:         return "Verifying installation";
            default:                           return "Unknown";
        }
    }

    /// Get install progress percentage for a phase
    static float GetPhaseProgress(InstallPhase phase) {
        static const float progress[] = {
            5.0f,  15.0f, 50.0f, 70.0f, 80.0f, 90.0f, 95.0f, 100.0f
        };
        auto idx = static_cast<uint32_t>(phase);
        return idx < static_cast<uint32_t>(InstallPhase::PhaseCount)
            ? progress[idx] : 0.0f;
    }

private:
    InstallerEnhancementsV2() = default;

    static bool CheckWindowsVersion(DWORD minBuild) {
        using RtlGetVersionFn = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);
        HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
        if (!hNtdll) return false;
        auto fn = reinterpret_cast<RtlGetVersionFn>(
            GetProcAddress(hNtdll, "RtlGetVersion"));
        if (!fn) return false;
        RTL_OSVERSIONINFOW osvi = {};
        osvi.dwOSVersionInfoSize = sizeof(osvi);
        fn(&osvi);
        return osvi.dwBuildNumber >= minBuild;
    }

    static bool CheckVCRuntime() {
        HMODULE hMod = LoadLibraryW(L"vcruntime140.dll");
        if (hMod) { FreeLibrary(hMod); return true; }
        return false;
    }

    static bool CheckDirectX11() {
        HMODULE hMod = LoadLibraryW(L"d3d11.dll");
        if (hMod) { FreeLibrary(hMod); return true; }
        return false;
    }

    static bool IsRunningAsAdmin() {
        BOOL isAdmin = FALSE;
        PSID adminGroup = nullptr;
        SID_IDENTIFIER_AUTHORITY ntAuth = SECURITY_NT_AUTHORITY;
        if (AllocateAndInitializeSid(&ntAuth, 2,
                SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0, &adminGroup)) {
            CheckTokenMembership(nullptr, adminGroup, &isAdmin);
            FreeSid(adminGroup);
        }
        return isAdmin != FALSE;
    }

    static bool CheckDiskSpace(ULONGLONG requiredBytes) {
        ULARGE_INTEGER freeBytesAvailable{};
        if (GetDiskFreeSpaceExW(L"C:\\", &freeBytesAvailable, nullptr, nullptr)) {
            return freeBytesAvailable.QuadPart >= requiredBytes;
        }
        return false;
    }
};

} // namespace Engine
} // namespace ExplorerLens
