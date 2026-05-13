// =============================================================================
// ExplorerLens Engine — MiniDumpCapture.h
// Sprint S353 | ROADMAP v8.0 Phase 3 (Crash Telemetry)
// Structured Windows MiniDump writer for shell extension crash reporting.
//
// Writes a MiniDump (DbgHelp MiniDumpWriteDump) to a temp file when the
// COM thumbnail provider encounters an unhandled exception or fatal OOM.
// The dump path is reported back to the caller for opt-in telemetry upload.
//
// Windows-only. Non-Windows builds compile to stubs.
// Avoids naming conflict with existing CrashDumpType (CrashRecoveryEngine.h)
// by using MiniDumpVariant and MiniDumpCaptureStatus names.
// =============================================================================
#pragma once

#include <cstdint>
#include <string>

#ifndef EXPLORERLENS_ENGINE_MINIDUMPCAPTURE_H
#define EXPLORERLENS_ENGINE_MINIDUMPCAPTURE_H

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// MiniDumpVariant — controls MiniDump detail level (maps to MINIDUMP_TYPE)
// ---------------------------------------------------------------------------
enum class MiniDumpVariant : uint32_t {
    MINI          = 0x00000000u, ///< MiniDumpNormal — thread stacks + loaded modules
    WITH_DATA_SEGS = 0x00000001u, ///< MiniDumpWithDataSegs — include data segments
    WITH_FULL_MEMORY = 0x00000002u, ///< Full heap — very large, dev only
    WITH_HANDLES  = 0x00000004u, ///< MiniDumpWithHandleData
    TRIAGE        = 0x00000040u, ///< MiniDumpFilterTriage — minimal privacy exposure
};

// ---------------------------------------------------------------------------
// MiniDumpCaptureStatus — result of one WriteDump call
// ---------------------------------------------------------------------------
enum class MiniDumpCaptureStatus : uint8_t {
    OK               = 0, ///< Dump written successfully
    NOT_WIN32        = 1, ///< Non-Windows build
    DBGHELP_MISSING  = 2, ///< DbgHelp.dll not loadable
    WRITE_FAIL       = 3, ///< MiniDumpWriteDump returned FALSE
    CREATE_FILE_FAIL = 4, ///< Could not create output file
    NULL_PATH        = 5, ///< Output directory path is null/empty
    EXCEPTION_DURING = 6, ///< SEH exception during dump write
    ALREADY_WRITING  = 7, ///< Re-entrant call detected (guard active)
};

// ---------------------------------------------------------------------------
// MiniDumpConfig — capture policy
// ---------------------------------------------------------------------------
struct MiniDumpConfig final {
    MiniDumpVariant variant{MiniDumpVariant::TRIAGE}; ///< Default to triage (privacy-safe)
    std::wstring    outputDirectory{L""};              ///< Empty = use %TEMP%/ExplorerLens/dumps/
    bool            includeExceptionInfo{true};        ///< Attach EXCEPTION_POINTERS if available
    bool            timestampFilename{true};           ///< Append ISO-8601 timestamp to filename
    bool            overwriteExisting{false};          ///< Overwrite if file already exists
    uint32_t        maxDumpsInDirectory{10u};          ///< Auto-prune oldest when exceeded

    [[nodiscard]] static MiniDumpConfig Default() noexcept {
        MiniDumpConfig c;
        c.variant               = MiniDumpVariant::TRIAGE;
        c.includeExceptionInfo  = true;
        c.timestampFilename     = true;
        c.overwriteExisting     = false;
        c.maxDumpsInDirectory   = 10u;
        return c;
    }
};

// ---------------------------------------------------------------------------
// MiniDumpCaptureResult — outcome of one capture operation
// ---------------------------------------------------------------------------
struct MiniDumpCaptureResult final {
    MiniDumpCaptureStatus status{MiniDumpCaptureStatus::NOT_WIN32};
    std::wstring          dumpFilePath;  ///< Full path of written .dmp file
    uint64_t              dumpSizeBytes{0};
    uint32_t              lastError{0};  ///< GetLastError() on failure

    [[nodiscard]] bool Success()  const noexcept { return status == MiniDumpCaptureStatus::OK; }
    [[nodiscard]] bool HasDump()  const noexcept { return Success() && !dumpFilePath.empty(); }
};

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
static constexpr uint32_t kMiniDumpDefaultMaxDumps   = 10u;
static constexpr uint32_t kMiniDumpMaxFullMemoryBytes = 500u * 1024u * 1024u; ///< 500 MB guard
static constexpr wchar_t  kMiniDumpFileExtension[]    = L".dmp";
static constexpr wchar_t  kMiniDumpSubDirectory[]     = L"ExplorerLens\\dumps";

// ---------------------------------------------------------------------------
// MiniDumpCapture — single-call dump writer (static, Windows-only)
// ---------------------------------------------------------------------------
class MiniDumpCapture final {
public:
    MiniDumpCapture() = delete;

    /// Write a MiniDump for the current process.
    /// @param cfg   Capture configuration
    /// @return      MiniDumpCaptureResult with path and status
    [[nodiscard]] static MiniDumpCaptureResult Write(
        const MiniDumpConfig& cfg = MiniDumpConfig::Default()) noexcept;

    /// Write a MiniDump with an attached exception context.
    /// Typically called from an SEH __except handler.
    /// @param exceptionPointers  EXCEPTION_POINTERS* from GetExceptionInformation()
    /// @param cfg                Capture configuration
    [[nodiscard]] static MiniDumpCaptureResult WriteFromException(
        void* exceptionPointers,
        const MiniDumpConfig& cfg = MiniDumpConfig::Default()) noexcept;

    /// Delete dump files in the output directory older than maxAge days.
    /// Returns number of files deleted.
    static uint32_t PruneOldDumps(
        const std::wstring& directory,
        uint32_t maxAge = 30u) noexcept;

    /// Returns the default dump output directory path.
    [[nodiscard]] static std::wstring DefaultDumpDirectory() noexcept;

    /// Returns true if DbgHelp.dll is loadable on this machine.
    [[nodiscard]] static bool IsDbgHelpAvailable() noexcept;
};

// ---------------------------------------------------------------------------
// Inline stub bodies for non-Windows
// ---------------------------------------------------------------------------
#ifndef _WIN32

inline MiniDumpCaptureResult MiniDumpCapture::Write(
    const MiniDumpConfig& /*cfg*/) noexcept
{
    MiniDumpCaptureResult r;
    r.status = MiniDumpCaptureStatus::NOT_WIN32;
    return r;
}

inline MiniDumpCaptureResult MiniDumpCapture::WriteFromException(
    void* /*exceptionPointers*/,
    const MiniDumpConfig& /*cfg*/) noexcept
{
    MiniDumpCaptureResult r;
    r.status = MiniDumpCaptureStatus::NOT_WIN32;
    return r;
}

inline uint32_t MiniDumpCapture::PruneOldDumps(
    const std::wstring& /*directory*/,
    uint32_t /*maxAge*/) noexcept
{
    return 0u;
}

inline std::wstring MiniDumpCapture::DefaultDumpDirectory() noexcept
{
    return L"";
}

inline bool MiniDumpCapture::IsDbgHelpAvailable() noexcept
{
    return false;
}

#endif // !_WIN32

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_MINIDUMPCAPTURE_H
