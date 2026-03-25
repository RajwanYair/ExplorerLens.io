#pragma once
// InstallerManager.h — Consolidated Installation & Packaging
// Copyright (c) 2026 ExplorerLens Project
//
// Unified header for all installation and packaging concerns:
// - Installer UX: progress feedback, prerequisite checking, upgrade paths,
//   rollback
// - Post-install COM registration, upgrade detection, uninstall cleanup
// - MSIX + MSI packaging pipeline: silent install, staged rollout, delta
//   packages
// - MSIX manifest generation, signing, auto-update channels
// - MSIX package targeting (Desktop/Store/Sideload), capability declarations
// - Sparse package registration for Windows Shell integration
//
// Merged from: InstallerEnhancementsV2.h, InstallerLifecycleAutomation.h,
//              InstallerV2Manager.h, MSIXPackagingManager.h

#include <cstdint>
#include <string>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

// =============================================================================
// Section 1 — Installer Experience (ex InstallerEnhancementsV2.h)
// =============================================================================

namespace ExplorerLens {
namespace Engine {

/// Installer type
enum class InstallerType : uint8_t {
    MSI = 0, ///< WiX MSI (primary)
    InnoSetup = 1, ///< Inno Setup EXE
    MSIX = 2, ///< MSIX package (modern)
    Portable = 3, ///< No-install / xcopy deploy
    Scoop = 4 ///< Scoop package manager
};

/// Installation phase
enum class InstallPhase : uint8_t {
    PreCheck = 0, ///< Prerequisite validation
    Backup = 1, ///< Backup existing installation
    FileCopy = 2, ///< Copy binaries
    COMRegister = 3, ///< Register COM DLL
    ShellRefresh = 4, ///< SHChangeNotify
    ConfigMigrate = 5, ///< Migrate settings from old version
    Cleanup = 6, ///< Remove temp files
    Verify = 7, ///< Verify installation
    PhaseCount = 8
};

/// Prerequisite check result
struct PrerequisiteCheck {
    const char* name = nullptr; ///< e.g., "Visual C++ Runtime"
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
        { "Windows 10 1809+", "10.0.17763", true, false, nullptr,
        "ExplorerLens requires Windows 10 version 1809 or later" },
        { "Visual C++ Runtime", "14.40", true, false,
        "https://aka.ms/vs/17/release/vc_redist.x64.exe",
        "Microsoft Visual C++ 2022+ Redistributable required" },
        { "DirectX 11 Runtime", "11.0", false, false, nullptr,
        "GPU acceleration requires DirectX 11" },
        { "Administrator Rights", nullptr, true, false, nullptr,
        "COM registration requires administrator privileges" },
        { "Disk Space (50MB)", "50MB", true, false, nullptr,
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
        case InstallPhase::PreCheck: return "Checking prerequisites";
        case InstallPhase::Backup: return "Backing up current installation";
        case InstallPhase::FileCopy: return "Copying files";
        case InstallPhase::COMRegister: return "Registering COM components";
        case InstallPhase::ShellRefresh: return "Refreshing Windows Shell";
        case InstallPhase::ConfigMigrate: return "Migrating settings";
        case InstallPhase::Cleanup: return "Cleaning up";
        case InstallPhase::Verify: return "Verifying installation";
        default: return "Unknown";
        }
    }

    /// Get install progress percentage for a phase
    static float GetPhaseProgress(InstallPhase phase) {
        static const float progress[] = {
        5.0f, 15.0f, 50.0f, 70.0f, 80.0f, 90.0f, 95.0f, 100.0f
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

// =============================================================================
// Section 2 — Installer Lifecycle Automation (ex InstallerLifecycleAutomation.h)
// =============================================================================

namespace ExplorerLens::Utils {

// --- Install action ----------------------------------------------------------

enum class InstallAction : uint32_t {
    FreshInstall = 0,
    Upgrade = 1,
    Repair = 2,
    Uninstall = 3,
};

inline std::string ToString(InstallAction a) {
    switch (a) {
    case InstallAction::FreshInstall: return "FreshInstall";
    case InstallAction::Upgrade: return "Upgrade";
    case InstallAction::Repair: return "Repair";
    case InstallAction::Uninstall: return "Uninstall";
    default: return "Unknown";
    }
}

// --- Version info ------------------------------------------------------------

struct InstalledVersion {
    uint32_t major{ 0 };
    uint32_t minor{ 0 };
    uint32_t patch{ 0 };
    std::string buildTag; // e.g., "v15.4.0-Zenith-U"

    std::string ToString() const {
        return std::to_string(major) + "." + std::to_string(minor) +
            "." + std::to_string(patch);
    }

    bool IsNewerThan(const InstalledVersion& other) const {
        if (major != other.major) return major > other.major;
        if (minor != other.minor) return minor > other.minor;
        return patch > other.patch;
    }
};

// --- Registration record -----------------------------------------------------

struct COMRegistrationRecord {
    std::string clsid; // "{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}"
    std::string dllPath;
    bool isInprocServer{ false };
    bool approvedByShell{ false }; // HKCR\CLSID\...\Approved

    static COMRegistrationRecord Expected() {
        return { "{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}",
        "", true, true };
    }
};

// --- Install step result -----------------------------------------------------

struct InstallStepResult {
    std::string stepName;
    bool success{ false };
    std::string detail;
    double durationMs{ 0.0 };
};

// --- Lifecycle automation result ---------------------------------------------

struct LifecycleResult {
    InstallAction action{ InstallAction::FreshInstall };
    bool overall{ false };
    std::vector<InstallStepResult> steps;
    COMRegistrationRecord comRecord;
    InstalledVersion installedVersion;
    double totalMs{ 0.0 };

    uint32_t FailedStepCount() const {
        uint32_t n = 0;
        for (const auto& s : steps) if (!s.success) ++n;
        return n;
    }
};

// --- Installer lifecycle automation ------------------------------------------

class InstallerLifecycleAutomation {
public:
    static constexpr const char* kCLSID = "{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}";

    static InstalledVersion CurrentVersion() {
        return { 8, 3, 0, "v8.3.0" };
    }

    static LifecycleResult SimulateFreshInstall() {
        LifecycleResult result;
        result.action = InstallAction::FreshInstall;

        auto addStep = [&](const std::string& name, bool ok, double ms = 50.0) {
            result.steps.push_back({ name, ok, ok ? "OK" : "FAILED", ms });
            result.totalMs += ms;
            };

        addStep("CopyBinaries", true, 120.0);
        addStep("RegisterCOMServer", true, 30.0);
        addStep("AddShellExtApproval", true, 10.0);
        addStep("FlushShellCache", true, 5.0);
        addStep("ValidateCLSIDLive", true, 15.0);

        result.comRecord = COMRegistrationRecord::Expected();
        result.comRecord.dllPath = "C:\\Program Files\\ExplorerLens\\LENSShell.dll";
        result.installedVersion = CurrentVersion();
        result.overall = result.FailedStepCount() == 0;
        return result;
    }

    static LifecycleResult SimulateUninstall() {
        LifecycleResult result;
        result.action = InstallAction::Uninstall;

        auto addStep = [&](const std::string& name, bool ok, double ms = 30.0) {
            result.steps.push_back({ name, ok, ok ? "OK" : "FAILED", ms });
            result.totalMs += ms;
            };

        addStep("UnregisterCOMServer", true, 20.0);
        addStep("RemoveShellExtApproval", true, 10.0);
        addStep("FlushShellCache", true, 5.0);
        addStep("DeleteBinaries", true, 80.0);
        addStep("ValidateCLSIDGone", true, 15.0);

        result.overall = result.FailedStepCount() == 0;
        return result;
    }
};

} // namespace ExplorerLens::Utils

// =============================================================================
// Section 3 — Installer V2 Pipeline (ex InstallerV2Manager.h)
// =============================================================================

namespace ExplorerLens {
namespace Engine {

enum class InstallerFormat : uint8_t { MSI = 0, MSIX, MSIXBUNDLE, AppInstaller, WinGetManifest, COUNT };
enum class InstallScope : uint8_t { PerMachine = 0, PerUser, AdminRequired, COUNT };
enum class InstallerV2Phase : uint8_t { Download = 0, Stage, Verify, Apply, RegisterShell, Commit, COUNT };
enum class RollbackStrategy : uint8_t { None = 0, Snapshot, DeltaRevert, FullReinstall, COUNT };

struct InstallerV2Manifest {
    std::wstring productId;
    std::wstring version;
    InstallerFormat format = InstallerFormat::MSIX;
    InstallScope scope = InstallScope::PerMachine;
    RollbackStrategy rollback = RollbackStrategy::Snapshot;
    bool silentInstall = true;
    bool deltaEnabled = true;
    uint32_t rolloutPercent = 100; // 0-100 staged rollout
};

struct InstallerV2Status {
    InstallerV2Phase currentPhase = InstallerV2Phase::Download;
    uint8_t progressPercent = 0;
    bool success = false;
    bool rollbackActive = false;
    std::wstring errorMessage;
};

class InstallerV2Manager {
public:
    static const wchar_t* FormatName(InstallerFormat f) {
        switch (f) {
        case InstallerFormat::MSI: return L"MSI";
        case InstallerFormat::MSIX: return L"MSIX";
        case InstallerFormat::MSIXBUNDLE: return L"MSIX Bundle";
        case InstallerFormat::AppInstaller: return L"App Installer";
        case InstallerFormat::WinGetManifest: return L"WinGet Manifest";
        default: return L"Unknown";
        }
    }
    static const wchar_t* InstallScopeName(InstallScope s) {
        switch (s) {
        case InstallScope::PerMachine: return L"Per Machine";
        case InstallScope::PerUser: return L"Per User";
        case InstallScope::AdminRequired: return L"Admin Required";
        default: return L"Unknown";
        }
    }
    static const wchar_t* PhaseName(InstallerV2Phase p) {
        switch (p) {
        case InstallerV2Phase::Download: return L"Download";
        case InstallerV2Phase::Stage: return L"Stage";
        case InstallerV2Phase::Verify: return L"Verify";
        case InstallerV2Phase::Apply: return L"Apply";
        case InstallerV2Phase::RegisterShell: return L"Register Shell";
        case InstallerV2Phase::Commit: return L"Commit";
        default: return L"Unknown";
        }
    }
    static const wchar_t* RollbackStrategyName(RollbackStrategy r) {
        switch (r) {
        case RollbackStrategy::None: return L"None";
        case RollbackStrategy::Snapshot: return L"Snapshot";
        case RollbackStrategy::DeltaRevert: return L"Delta Revert";
        case RollbackStrategy::FullReinstall: return L"Full Reinstall";
        default: return L"Unknown";
        }
    }
    static constexpr size_t FormatCount() { return static_cast<size_t>(InstallerFormat::COUNT); }
    static constexpr size_t InstallScopeCount() { return static_cast<size_t>(InstallScope::COUNT); }
    static constexpr size_t PhaseCount() { return static_cast<size_t>(InstallerV2Phase::COUNT); }
    static constexpr size_t RollbackStrategyCount() { return static_cast<size_t>(RollbackStrategy::COUNT); }
};

}
} // namespace ExplorerLens::Engine

// =============================================================================
// Section 4 — MSIX Packaging (ex MSIXPackagingManager.h)
// =============================================================================

namespace ExplorerLens {
namespace Engine {

/// MSIX packaging target
enum class MSIXTarget : uint8_t {
    Desktop, // Win32 desktop bridge
    Store, // Microsoft Store submission
    Sideload, // Enterprise sideloading
    Development // Dev/test unsigned
};

/// MSIX capability declarations
enum class MSIXCapability : uint8_t {
    ShellExtension, // Shell thumbnail handler
    FileTypeAssociation, // File type registrations
    COMServer, // COM DLL registration
    RunFullTrust, // Full trust (required for shell ext)
    RestrictedFiles, // Access to all file types
    Removable, // Removable storage access
    COUNT
};

/// MSIX package info
struct MSIXPackageInfo {
    std::wstring identity; // Package identity name
    std::wstring publisher; // Publisher identity
    std::wstring version; // Package version (A.B.C.D)
    std::wstring displayName; // User-facing name
    std::wstring description;
    MSIXTarget target = MSIXTarget::Desktop;
    uint64_t estimatedSizeKB = 3200;
    bool isSigned = false;
    bool hasAutoUpdate = false;
};

/// MSIX packaging manager
class MSIXPackagingManager {
public:
    /// Target name
    static const wchar_t* TargetName(MSIXTarget t) {
        switch (t) {
        case MSIXTarget::Desktop: return L"Desktop Bridge";
        case MSIXTarget::Store: return L"Microsoft Store";
        case MSIXTarget::Sideload: return L"Enterprise Sideload";
        case MSIXTarget::Development: return L"Development";
        default: return L"Unknown";
        }
    }

    /// Capability name
    static const wchar_t* CapabilityName(MSIXCapability c) {
        switch (c) {
        case MSIXCapability::ShellExtension: return L"Shell Extension";
        case MSIXCapability::FileTypeAssociation: return L"File Type Association";
        case MSIXCapability::COMServer: return L"COM Server";
        case MSIXCapability::RunFullTrust: return L"Run Full Trust";
        case MSIXCapability::RestrictedFiles: return L"Restricted Files";
        case MSIXCapability::Removable: return L"Removable Storage";
        default: return L"Unknown";
        }
    }

    /// Target count
    static constexpr size_t TargetCount() { return 4; }

    /// Capability count
    static constexpr size_t CapabilityCount() { return static_cast<size_t>(MSIXCapability::COUNT); }

    /// Generate AppxManifest identity
    static std::wstring GenerateIdentity(const std::wstring& name, const std::wstring& version) {
        return name + L"_" + version;
    }

    /// Validate version format (A.B.C.D)
    static bool ValidateVersion(const std::wstring& version) {
        int dots = 0;
        for (auto c : version) {
            if (c == '.') dots++;
            else if (c < '0' || c > '9') return false;
        }
        return dots == 3;
    }
};

}
} // namespace ExplorerLens::Engine

// =============================================================================
// Section 5 — MSIX Package Manager (separate .h/.cpp pair — kept as include)
// =============================================================================
#include "MSIXPackageManager.h"
