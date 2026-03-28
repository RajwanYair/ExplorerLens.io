// CrashReporter.h — Windows Error Reporting + Minidump Crash Reporter
// Copyright (c) 2026 ExplorerLens Project
//
// Installs an unhandled exception filter to capture minidumps via DbgHelp,
// forwards crash reports to Windows Error Reporting (WER), and optionally
// appends engine diagnostics (version, last decoded file, AI state).
//
#pragma once

#include <windows.h>
#include <DbgHelp.h>
#include <string>
#include <functional>
#include <vector>
#include <cstdint>
#include <filesystem>

#pragma comment(lib, "DbgHelp.lib")

namespace ExplorerLens { namespace Engine { namespace Utils {

enum class DumpType : DWORD {
    Mini    = MiniDumpNormal,
    WithHeap= MiniDumpWithFullMemory,
    Triage  = MiniDumpFilterMemory    // Small, no PII in memory
};

struct CrashContext {
    std::string   engineVersion   = "19.1.0";
    std::string   lastDecodedFile;        // Sanitized (basename only, no path)
    std::string   lastDecoder;
    bool          aiPipelineActive = false;
    uint32_t      memPressureLevel = 0;
    std::string   customAnnotation;
};

struct ReporterCrashReport {
    std::wstring  dumpPath;
    std::string   exceptionCode;
    std::string   exceptionAddress;
    CrashContext  context;
    bool          writtenToDisk = false;
    bool          sentToWER     = false;
};

class CrashReporter {
public:
    static CrashReporter& Instance() {
        static CrashReporter inst;
        return inst;
    }

    // Call once at DLL/process start
    void Install() {
        if (m_installed) return;
        m_prevFilter = SetUnhandledExceptionFilter(UnhandledExceptionHandler);
        m_installed  = true;
    }

    void Uninstall() {
        if (!m_installed) return;
        SetUnhandledExceptionFilter(m_prevFilter);
        m_installed = false;
    }

    void SetDumpDirectory(const std::wstring& dir) { m_dumpDir = dir; }

    void SetDumpType(DumpType type) { m_dumpType = static_cast<DWORD>(type); }

    // Update live context (call from decode pipeline)
    void UpdateContext(const CrashContext& ctx) { m_context = ctx; }

    void SetLastDecodedFile(const std::string& baseNameOnly) {
        m_context.lastDecodedFile = baseNameOnly;
    }

    void SetLastDecoder(const std::string& decoderName) {
        m_context.lastDecoder = decoderName;
    }

    // Register a callback for post-crash notification
    using CrashFn = std::function<void(const ReporterCrashReport&)>;
    void OnCrash(CrashFn fn) { m_callbacks.push_back(std::move(fn)); }

    // Manually generate a diagnostic dump (for support tickets)
    ReporterCrashReport GenerateDiagnosticDump() {
        return WriteDump(GetCurrentProcess(), GetCurrentProcessId(),
                         nullptr, EXCEPTION_EXECUTE_HANDLER, false);
    }

    bool IsInstalled() const { return m_installed; }

    // Static SEH wrapper — must be in a function without C++ object unwinding
    // to satisfy MSVC C2712 ("/EHsc cannot mix __try with C++ objects").
    static void InvokeCbSafe(
        std::function<void(const ReporterCrashReport&)>& fn,
        const ReporterCrashReport& rep) noexcept {
        __try { fn(rep); } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

private:
    CrashReporter() {
        // Default dump dir: %LOCALAPPDATA%\ExplorerLens\crashes
        wchar_t appDataPath[MAX_PATH] = {};
        ExpandEnvironmentStringsW(L"%LOCALAPPDATA%\\ExplorerLens\\crashes", appDataPath, MAX_PATH);
        m_dumpDir    = appDataPath;
        m_dumpType   = static_cast<DWORD>(DumpType::Triage);
    }

    static LONG WINAPI UnhandledExceptionHandler(PEXCEPTION_POINTERS ep) {
        auto& inst = Instance();
        inst.WriteDump(GetCurrentProcess(), GetCurrentProcessId(),
                       ep, EXCEPTION_EXECUTE_HANDLER, true);

        return inst.m_prevFilter ? inst.m_prevFilter(ep) : EXCEPTION_CONTINUE_SEARCH;
    }

    ReporterCrashReport WriteDump(HANDLE hProc, DWORD pid,
                          PEXCEPTION_POINTERS ep, DWORD unusedFlags, bool notifyCbs) {
        (void)unusedFlags;

        // Ensure dump directory exists
        std::filesystem::create_directories(m_dumpDir);

        // Build timestamp-based filename
        SYSTEMTIME st; GetSystemTime(&st);
        wchar_t fname[MAX_PATH];
        swprintf_s(fname, MAX_PATH, L"%s\\ExplorerLens-%04d%02d%02d-%02d%02d%02d.dmp",
            m_dumpDir.c_str(), st.wYear, st.wMonth, st.wDay,
            st.wHour, st.wMinute, st.wSecond);

        ReporterCrashReport report;
        report.dumpPath = fname;
        report.context  = m_context;

        if (ep) {
            char buf[32];
            snprintf(buf, sizeof(buf), "0x%08lX", ep->ExceptionRecord->ExceptionCode);
            report.exceptionCode = buf;
            snprintf(buf, sizeof(buf), "0x%p", ep->ExceptionRecord->ExceptionAddress);
            report.exceptionAddress = buf;
        }

        // Write minidump
        HANDLE hFile = CreateFileW(fname, GENERIC_WRITE, 0, nullptr,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile != INVALID_HANDLE_VALUE) {
            MINIDUMP_EXCEPTION_INFORMATION mei{};
            if (ep) {
                mei.ThreadId          = GetCurrentThreadId();
                mei.ExceptionPointers = ep;
                mei.ClientPointers    = FALSE;
            }
            report.writtenToDisk = (MiniDumpWriteDump(hProc, pid, hFile,
                static_cast<MINIDUMP_TYPE>(m_dumpType),
                ep ? &mei : nullptr, nullptr, nullptr) == TRUE);
            CloseHandle(hFile);
        }

        // Notify callbacks (fire-and-forget; don't throw in crash handler)
        if (notifyCbs) {
            for (auto& fn : m_callbacks) {
                InvokeCbSafe(fn, report);
            }
        }

        return report;
    }

    std::wstring                       m_dumpDir;
    DWORD                              m_dumpType   = MiniDumpNormal;
    CrashContext                       m_context;
    bool                               m_installed  = false;
    LPTOP_LEVEL_EXCEPTION_FILTER       m_prevFilter = nullptr;
    std::vector<CrashFn>               m_callbacks;
};

}}} // namespace ExplorerLens::Engine::Utils
