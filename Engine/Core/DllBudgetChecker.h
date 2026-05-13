// Engine/Core/DllBudgetChecker.h
// ExplorerLens — DLL binary size budget enforcement (H18 / ROADMAP v8.0 Phase 3)
// Sprint S344.
//
// Purpose:
//   Phase 3 exit criterion: "DLL size < 2,500 KB" (H18: < 5 MB shell extension
//   footprint — LENSShell.dll < 3 MB, total MSI < 8 MB).
//
//   DllBudgetChecker provides:
//     1. Compile-time budget constants for all tracked binaries.
//     2. A runtime file-size query (GetDllSizeBytes) via Win32 GetFileAttributesEx.
//     3. A BudgetCheckResult summarising pass/warn/fail for CI.
//
//   Intended callers:
//     - EngineTests_Platform.cpp: static assert + runtime check in Test S344.
//     - build-scripts/Check-Build-Status.ps1 (PowerShell reads a JSON report).
//     - CI post-build step that calls DllBudgetChecker::ReportJson().
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_DLL_BUDGET_CHECKER_H
#define EXPLORERLENS_ENGINE_DLL_BUDGET_CHECKER_H

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string>

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#endif

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// Binary size budget constants — all values in bytes
// ---------------------------------------------------------------------------

/// Hard budget for LENSShell.dll (Phase 3 target < 2,500 KB).
inline constexpr std::uint64_t kShellDllHardBudgetBytes  = 2'500u * 1'024u;

/// Warn threshold for LENSShell.dll — alert before hitting hard limit.
inline constexpr std::uint64_t kShellDllWarnBudgetBytes  = 2'200u * 1'024u;

/// Hard budget for LENSManager.exe (Phase 3 target < 800 KB).
inline constexpr std::uint64_t kManagerExeHardBudgetBytes = 800u * 1'024u;

/// Warn threshold for LENSManager.exe.
inline constexpr std::uint64_t kManagerExeWarnBudgetBytes = 650u * 1'024u;

/// Hard budget for ExplorerLensEngine.lib (< 6 MB).
inline constexpr std::uint64_t kEnginelibHardBudgetBytes  = 6u * 1'024u * 1'024u;

/// Total MSI hard budget (Phase 3 target < 8 MB).
inline constexpr std::uint64_t kMsiHardBudgetBytes        = 8u * 1'024u * 1'024u;

// ---------------------------------------------------------------------------
// BinaryBudgetId — enum for identifying each tracked binary
// ---------------------------------------------------------------------------

enum class BinaryBudgetId : std::uint8_t {
    LENS_SHELL_DLL    = 0,
    LENS_MANAGER_EXE  = 1,
    ENGINE_LIB        = 2,
    MSI_PACKAGE       = 3,
    COUNT             = 4,
};

// ---------------------------------------------------------------------------
// BudgetCheckOutcome
// ---------------------------------------------------------------------------

enum class BudgetCheckOutcome : std::uint8_t {
    PASS              = 0,  ///< File size is below warn threshold
    WARN              = 1,  ///< File size is between warn and hard threshold
    FAIL              = 2,  ///< File size exceeds hard budget
    FILE_NOT_FOUND    = 3,  ///< Binary not present (build not complete)
    QUERY_ERROR       = 4,  ///< Win32 GetFileAttributesEx / stat failed
};

// ---------------------------------------------------------------------------
// BudgetCheckResult
// ---------------------------------------------------------------------------

struct BudgetCheckResult final {
    BinaryBudgetId    id             = BinaryBudgetId::LENS_SHELL_DLL;
    BudgetCheckOutcome outcome       = BudgetCheckOutcome::FILE_NOT_FOUND;
    std::uint64_t     actualBytes    = 0u;
    std::uint64_t     warnThreshold  = 0u;
    std::uint64_t     hardThreshold  = 0u;

    [[nodiscard]] bool Passed()  const noexcept { return outcome == BudgetCheckOutcome::PASS; }
    [[nodiscard]] bool Warning() const noexcept { return outcome == BudgetCheckOutcome::WARN; }
    [[nodiscard]] bool Failed()  const noexcept { return outcome == BudgetCheckOutcome::FAIL; }
};

// ---------------------------------------------------------------------------
// DllBudgetChecker — static utility class
// ---------------------------------------------------------------------------

class DllBudgetChecker final {
public:
    // -----------------------------------------------------------------
    // Constants — surfaced for unit tests
    // -----------------------------------------------------------------

    static constexpr std::uint64_t kShellDllHard    = kShellDllHardBudgetBytes;
    static constexpr std::uint64_t kShellDllWarn    = kShellDllWarnBudgetBytes;
    static constexpr std::uint64_t kManagerHard     = kManagerExeHardBudgetBytes;
    static constexpr std::uint64_t kManagerWarn     = kManagerExeWarnBudgetBytes;
    static constexpr std::uint64_t kEngineLibHard   = kEnginelibHardBudgetBytes;
    static constexpr std::uint64_t kMsiHard         = kMsiHardBudgetBytes;

    // -----------------------------------------------------------------
    // GetFileSizeBytes — portable Win32 / stub
    // -----------------------------------------------------------------

    /// Query the on-disk size of a binary file.
    /// Returns 0 on error or if the file does not exist.
    [[nodiscard]] static std::uint64_t GetFileSizeBytes(const wchar_t* path) noexcept
    {
#ifdef _WIN32
        WIN32_FILE_ATTRIBUTE_DATA fa{};
        if (!GetFileAttributesExW(path, GetFileExInfoStandard, &fa))
            return 0u;
        return (static_cast<std::uint64_t>(fa.nFileSizeHigh) << 32u) |
                static_cast<std::uint64_t>(fa.nFileSizeLow);
#else
        (void)path;
        return 0u;
#endif
    }

    // -----------------------------------------------------------------
    // CheckBinary — evaluate a single binary against its budget
    // -----------------------------------------------------------------

    [[nodiscard]] static BudgetCheckResult CheckBinary(
        BinaryBudgetId    id,
        const wchar_t*    path,
        std::uint64_t     warnThreshold,
        std::uint64_t     hardThreshold) noexcept
    {
        BudgetCheckResult r;
        r.id            = id;
        r.warnThreshold = warnThreshold;
        r.hardThreshold = hardThreshold;

        const std::uint64_t sz = GetFileSizeBytes(path);
        if (sz == 0u) {
            r.outcome     = BudgetCheckOutcome::FILE_NOT_FOUND;
            r.actualBytes = 0u;
            return r;
        }

        r.actualBytes = sz;
        if (sz > hardThreshold)
            r.outcome = BudgetCheckOutcome::FAIL;
        else if (sz > warnThreshold)
            r.outcome = BudgetCheckOutcome::WARN;
        else
            r.outcome = BudgetCheckOutcome::PASS;
        return r;
    }

    // -----------------------------------------------------------------
    // CheckShellDll — convenience wrapper
    // -----------------------------------------------------------------

    [[nodiscard]] static BudgetCheckResult CheckShellDll(
        const wchar_t* path = L"x64\\Release\\LENSShell.dll") noexcept
    {
        return CheckBinary(BinaryBudgetId::LENS_SHELL_DLL, path,
                           kShellDllWarn, kShellDllHard);
    }

    [[nodiscard]] static BudgetCheckResult CheckManagerExe(
        const wchar_t* path = L"x64\\Release\\LENSManager.exe") noexcept
    {
        return CheckBinary(BinaryBudgetId::LENS_MANAGER_EXE, path,
                           kManagerWarn, kManagerHard);
    }

    // -----------------------------------------------------------------
    // Budget headroom (positive = bytes remaining; negative = overage)
    // -----------------------------------------------------------------

    [[nodiscard]] static std::int64_t Headroom(
        const BudgetCheckResult& r) noexcept
    {
        return static_cast<std::int64_t>(r.hardThreshold) -
               static_cast<std::int64_t>(r.actualBytes);
    }

private:
    DllBudgetChecker() = delete;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_DLL_BUDGET_CHECKER_H
