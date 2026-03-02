//==============================================================================
// ExplorerLens Engine — Crash Intelligence Engine
//
// Post-mortem crash analysis with real Windows debugging APIs. Provides
// minidump generation, stack trace capture via dynamically-loaded dbghelp.dll,
// unhandled exception filtering, and crash report aggregation. All dbghelp
// functions are resolved at runtime via GetProcAddress to avoid hard
// dependencies. Falls back to RtlCaptureStackBackTrace when dbghelp is
// unavailable.
//
// Thread-safe via SRWLOCK. Header-only, C++20, MSVC /W4 clean.
//==============================================================================
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <cstdint>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace ExplorerLens {
namespace Engine {

// Forward-declare dbghelp types to avoid header dependency
struct IMAGEHLP_SYMBOL64_EX {
    DWORD   SizeOfStruct;
    DWORD64 Address;
    DWORD   Size;
    DWORD   Flags;
    DWORD   MaxNameLength;
    CHAR    Name[512];
};

struct IMAGEHLP_LINE64_EX {
    DWORD   SizeOfStruct;
    PVOID   Key;
    DWORD   LineNumber;
    PCHAR   FileName;
    DWORD64 Address;
};

// Minidump type flags (mirror from dbghelp.h)
static constexpr DWORD MINIDUMP_NORMAL = 0x00000000;
static constexpr DWORD MINIDUMP_WITH_DATA_SEGS = 0x00000001;
static constexpr DWORD MINIDUMP_WITH_HANDLE_DATA = 0x00000004;

struct CrashStackFrame {
    DWORD64      address = 0;
    std::wstring moduleName;
    std::wstring symbolName;
    uint32_t     lineNumber = 0;
    std::wstring fileName;
};

struct CrashReportEngine {
    DWORD                       exceptionCode = 0;
    std::wstring                faultingModule;
    std::wstring                description;
    std::vector<CrashStackFrame> stackTrace;
    std::wstring                dumpPath;
    uint64_t                    timestampMs = 0;
};

struct CrashStats {
    uint64_t crashesCaught = 0;
    uint64_t dumpsWritten = 0;
    uint64_t symbolsResolved = 0;
    double   avgResolutionMs = 0.0;
};

class CrashIntelligenceEngine {
public:
    static CrashIntelligenceEngine& Instance() {
        static CrashIntelligenceEngine s_instance;
        return s_instance;
    }

    inline bool Initialize() {
        AcquireSRWLockExclusive(&m_lock);
        bool result = InitializeInternal();
        ReleaseSRWLockExclusive(&m_lock);
        return result;
    }

    inline void Shutdown() {
        AcquireSRWLockExclusive(&m_lock);
        if (m_symInitialized && m_fnSymCleanup) {
            using SymCleanupFn = BOOL(WINAPI*)(HANDLE);
            reinterpret_cast<SymCleanupFn>(m_fnSymCleanup)(GetCurrentProcess());
            m_symInitialized = false;
        }
        if (m_dbgHelpModule) {
            FreeLibrary(m_dbgHelpModule);
            m_dbgHelpModule = nullptr;
        }
        m_fnSymInitialize = nullptr;
        m_fnSymCleanup = nullptr;
        m_fnSymFromAddr = nullptr;
        m_fnSymGetLineFromAddr64 = nullptr;
        m_fnMiniDumpWriteDump = nullptr;
        ReleaseSRWLockExclusive(&m_lock);
    }

    inline void SetCrashDumpPath(const std::wstring& dir) {
        AcquireSRWLockExclusive(&m_lock);
        m_dumpDirectory = dir;
        // Ensure trailing backslash
        if (!m_dumpDirectory.empty() && m_dumpDirectory.back() != L'\\') {
            m_dumpDirectory += L'\\';
        }
        // Create directory if it doesn't exist
        CreateDirectoryW(m_dumpDirectory.c_str(), nullptr);
        ReleaseSRWLockExclusive(&m_lock);
    }

    inline bool WriteMiniDump(EXCEPTION_POINTERS* exInfo) {
        AcquireSRWLockShared(&m_lock);
        if (!m_fnMiniDumpWriteDump) {
            ReleaseSRWLockShared(&m_lock);
            return false;
        }

        // Generate timestamped filename
        SYSTEMTIME st{};
        GetLocalTime(&st);
        wchar_t fname[MAX_PATH]{};
        _snwprintf_s(fname, _TRUNCATE,
            L"%sExplorerLens_%04u%02u%02u_%02u%02u%02u_%lu.dmp",
            m_dumpDirectory.c_str(),
            st.wYear, st.wMonth, st.wDay,
            st.wHour, st.wMinute, st.wSecond,
            GetCurrentProcessId());

        HANDLE hFile = CreateFileW(fname, GENERIC_WRITE, 0, nullptr,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            ReleaseSRWLockShared(&m_lock);
            return false;
        }

        // Prepare exception info struct for MiniDumpWriteDump
        struct MiniDumpExceptionInfo {
            DWORD               ThreadId;
            PEXCEPTION_POINTERS ExceptionPointers;
            BOOL                ClientPointers;
        };

        MiniDumpExceptionInfo mei{};
        mei.ThreadId = GetCurrentThreadId();
        mei.ExceptionPointers = exInfo;
        mei.ClientPointers = FALSE;

        // Call MiniDumpWriteDump(hProcess, pid, hFile, type, pExInfo, NULL, NULL)
        using MiniDumpWriteDumpFn = BOOL(WINAPI*)(HANDLE, DWORD, HANDLE, DWORD, PVOID, PVOID, PVOID);
        auto fn = reinterpret_cast<MiniDumpWriteDumpFn>(m_fnMiniDumpWriteDump);
        BOOL ok = fn(GetCurrentProcess(), GetCurrentProcessId(), hFile,
            MINIDUMP_NORMAL | MINIDUMP_WITH_HANDLE_DATA,
            exInfo ? &mei : nullptr, nullptr, nullptr);

        CloseHandle(hFile);
        ReleaseSRWLockShared(&m_lock);

        if (ok) {
            m_stats.dumpsWritten.fetch_add(1, std::memory_order_relaxed);
        }
        return ok != FALSE;
    }

    inline void InstallUnhandledExceptionFilter() {
        SetUnhandledExceptionFilter(&CrashIntelligenceEngine::TopLevelExceptionFilter);
    }

    inline std::vector<CrashStackFrame> CaptureStackTrace(uint32_t framesToSkip = 0) {
        constexpr uint32_t kMaxFrames = 62; // CaptureStackBackTrace limit
        void* stack[kMaxFrames]{};

        USHORT frameCount = RtlCaptureStackBackTrace(
            framesToSkip + 1, // skip this function too
            kMaxFrames,
            stack,
            nullptr);

        std::vector<CrashStackFrame> frames;
        frames.reserve(frameCount);

        auto startTime = std::chrono::steady_clock::now();

        AcquireSRWLockShared(&m_lock);
        for (USHORT i = 0; i < frameCount; ++i) {
            CrashStackFrame frame;
            frame.address = reinterpret_cast<DWORD64>(stack[i]);

            // Resolve module name
            HMODULE hMod = nullptr;
            if (GetModuleHandleExW(
                GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                reinterpret_cast<LPCWSTR>(stack[i]),
                &hMod)) {
                wchar_t modPath[MAX_PATH]{};
                if (GetModuleFileNameW(hMod, modPath, MAX_PATH) > 0) {
                    // Extract just the filename
                    const wchar_t* slash = wcsrchr(modPath, L'\\');
                    frame.moduleName = slash ? (slash + 1) : modPath;
                }
            }

            // Resolve symbol name if dbghelp available
            if (m_fnSymFromAddr && m_symInitialized) {
                IMAGEHLP_SYMBOL64_EX sym{};
                sym.SizeOfStruct = sizeof(sym);
                sym.MaxNameLength = sizeof(sym.Name);
                DWORD64 displacement = 0;

                using SymFromAddrFn = BOOL(WINAPI*)(HANDLE, DWORD64, PDWORD64, PVOID);
                auto fnSym = reinterpret_cast<SymFromAddrFn>(m_fnSymFromAddr);
                if (fnSym(GetCurrentProcess(), frame.address, &displacement, &sym)) {
                    int len = MultiByteToWideChar(CP_ACP, 0, sym.Name, -1, nullptr, 0);
                    if (len > 0) {
                        std::wstring wname(static_cast<size_t>(len - 1), L'\0');
                        MultiByteToWideChar(CP_ACP, 0, sym.Name, -1, wname.data(), len);
                        frame.symbolName = std::move(wname);
                        m_stats.symbolsResolved.fetch_add(1, std::memory_order_relaxed);
                    }
                }
            }

            // Resolve file/line if dbghelp available
            if (m_fnSymGetLineFromAddr64 && m_symInitialized) {
                IMAGEHLP_LINE64_EX line{};
                line.SizeOfStruct = sizeof(line);
                DWORD lineDisp = 0;

                using SymGetLineFn = BOOL(WINAPI*)(HANDLE, DWORD64, PDWORD, PVOID);
                auto fnLine = reinterpret_cast<SymGetLineFn>(m_fnSymGetLineFromAddr64);
                if (fnLine(GetCurrentProcess(), frame.address, &lineDisp, &line)) {
                    frame.lineNumber = line.LineNumber;
                    if (line.FileName) {
                        int flen = MultiByteToWideChar(CP_ACP, 0, line.FileName, -1, nullptr, 0);
                        if (flen > 0) {
                            std::wstring wfn(static_cast<size_t>(flen - 1), L'\0');
                            MultiByteToWideChar(CP_ACP, 0, line.FileName, -1, wfn.data(), flen);
                            frame.fileName = std::move(wfn);
                        }
                    }
                }
            }

            frames.push_back(std::move(frame));
        }
        ReleaseSRWLockShared(&m_lock);

        auto endTime = std::chrono::steady_clock::now();
        double elapsedMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

        // Update average resolution time
        uint64_t count = m_stats.resolutionCount.fetch_add(1, std::memory_order_relaxed) + 1;
        double oldAvg = m_stats.avgResolutionMsAccum.load(std::memory_order_relaxed);
        // Running average
        double newAvg = oldAvg + (elapsedMs - oldAvg) / static_cast<double>(count);
        m_stats.avgResolutionMsAccum.store(newAvg, std::memory_order_relaxed);

        return frames;
    }

    inline std::wstring FormatStackTrace(const std::vector<CrashStackFrame>& frames) {
        std::wostringstream oss;
        oss << L"=== Stack Trace (" << frames.size() << L" frames) ===\n";
        for (size_t i = 0; i < frames.size(); ++i) {
            const auto& f = frames[i];
            oss << L"  [" << i << L"] 0x"
                << std::hex << std::setw(16) << std::setfill(L'0') << f.address
                << std::dec << L"  ";
            if (!f.moduleName.empty()) {
                oss << f.moduleName << L"!";
            }
            if (!f.symbolName.empty()) {
                oss << f.symbolName;
            }
            else {
                oss << L"<unknown>";
            }
            if (!f.fileName.empty()) {
                oss << L"  (" << f.fileName << L":" << f.lineNumber << L")";
            }
            oss << L"\n";
        }
        return oss.str();
    }

    inline CrashReportEngine AnalyzeDump(const std::wstring& dumpPath) {
        CrashReportEngine report;
        report.dumpPath = dumpPath;
        report.timestampMs = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());

        // Open the dump file and read the MINIDUMP_HEADER
        HANDLE hFile = CreateFileW(dumpPath.c_str(), GENERIC_READ, FILE_SHARE_READ,
            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            report.description = L"Failed to open dump file";
            return report;
        }

        // Read first 32 bytes for signature check (MDMP signature = 0x504D444D)
        DWORD bytesRead = 0;
        uint8_t header[32]{};
        if (!ReadFile(hFile, header, sizeof(header), &bytesRead, nullptr) || bytesRead < 32) {
            CloseHandle(hFile);
            report.description = L"Failed to read dump header";
            return report;
        }

        // Verify MDMP signature
        DWORD signature = *reinterpret_cast<DWORD*>(header);
        if (signature != 0x504D444Du) {
            CloseHandle(hFile);
            report.description = L"Invalid minidump signature";
            return report;
        }

        // Extract stream count and directory RVA from header
        DWORD streamCount = *reinterpret_cast<DWORD*>(header + 8);
        // DWORD streamDirRVA = *reinterpret_cast<DWORD*>(header + 12); // reserved for future use

        report.description = L"Valid minidump with " + std::to_wstring(streamCount) + L" streams";

        // Get file size as additional info
        LARGE_INTEGER fileSize{};
        GetFileSizeEx(hFile, &fileSize);
        report.description += L", size=" + std::to_wstring(fileSize.QuadPart) + L" bytes";

        CloseHandle(hFile);

        m_stats.crashesCaught.fetch_add(1, std::memory_order_relaxed);
        return report;
    }

    inline CrashStats GetStats() const {
        CrashStats s;
        s.crashesCaught = m_stats.crashesCaught.load(std::memory_order_relaxed);
        s.dumpsWritten = m_stats.dumpsWritten.load(std::memory_order_relaxed);
        s.symbolsResolved = m_stats.symbolsResolved.load(std::memory_order_relaxed);
        s.avgResolutionMs = m_stats.avgResolutionMsAccum.load(std::memory_order_relaxed);
        return s;
    }

private:
    CrashIntelligenceEngine() {
        InitializeSRWLock(&m_lock);
    }

    ~CrashIntelligenceEngine() {
        Shutdown();
    }

    CrashIntelligenceEngine(const CrashIntelligenceEngine&) = delete;
    CrashIntelligenceEngine& operator=(const CrashIntelligenceEngine&) = delete;

    inline bool InitializeInternal() {
        if (m_symInitialized) return true;

        m_dbgHelpModule = LoadLibraryW(L"dbghelp.dll");
        if (!m_dbgHelpModule) {
            return false; // Fall back to address-only traces
        }

        m_fnSymInitialize = reinterpret_cast<void*>(GetProcAddress(m_dbgHelpModule, "SymInitialize"));
        m_fnSymCleanup = reinterpret_cast<void*>(GetProcAddress(m_dbgHelpModule, "SymCleanup"));
        m_fnSymFromAddr = reinterpret_cast<void*>(GetProcAddress(m_dbgHelpModule, "SymFromAddr"));
        m_fnSymGetLineFromAddr64 = reinterpret_cast<void*>(GetProcAddress(m_dbgHelpModule, "SymGetLineFromAddr64"));
        m_fnMiniDumpWriteDump = reinterpret_cast<void*>(GetProcAddress(m_dbgHelpModule, "MiniDumpWriteDump"));

        if (m_fnSymInitialize) {
            using SymInitFn = BOOL(WINAPI*)(HANDLE, PCSTR, BOOL);
            auto fn = reinterpret_cast<SymInitFn>(m_fnSymInitialize);
            if (fn(GetCurrentProcess(), nullptr, TRUE)) {
                m_symInitialized = true;
            }
        }

        return m_symInitialized || (m_fnMiniDumpWriteDump != nullptr);
    }

    static LONG WINAPI TopLevelExceptionFilter(EXCEPTION_POINTERS* exInfo) {
        auto& engine = Instance();
        engine.m_stats.crashesCaught.fetch_add(1, std::memory_order_relaxed);

        // Write minidump
        engine.WriteMiniDump(exInfo);

        // Capture and log stack trace (best effort)
        auto frames = engine.CaptureStackTrace(0);
        if (!frames.empty()) {
            std::wstring trace = engine.FormatStackTrace(frames);
            // Write trace to a .txt alongside the dump
            SYSTEMTIME st{};
            GetLocalTime(&st);
            wchar_t fname[MAX_PATH]{};
            AcquireSRWLockShared(&engine.m_lock);
            _snwprintf_s(fname, _TRUNCATE,
                L"%sExplorerLens_crash_%04u%02u%02u_%02u%02u%02u.txt",
                engine.m_dumpDirectory.c_str(),
                st.wYear, st.wMonth, st.wDay,
                st.wHour, st.wMinute, st.wSecond);
            ReleaseSRWLockShared(&engine.m_lock);

            HANDLE hLog = CreateFileW(fname, GENERIC_WRITE, 0, nullptr,
                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (hLog != INVALID_HANDLE_VALUE) {
                // Write BOM + trace as UTF-16LE
                DWORD written = 0;
                const BYTE bom[] = { 0xFF, 0xFE };
                WriteFile(hLog, bom, sizeof(bom), &written, nullptr);
                WriteFile(hLog, trace.c_str(),
                    static_cast<DWORD>(trace.size() * sizeof(wchar_t)),
                    &written, nullptr);
                CloseHandle(hLog);
            }
        }

        return EXCEPTION_EXECUTE_HANDLER;
    }

    // Members
    SRWLOCK       m_lock{};
    HMODULE       m_dbgHelpModule = nullptr;
    bool          m_symInitialized = false;
    std::wstring  m_dumpDirectory = L".\\";

    // dbghelp function pointers (void* to avoid header dependency)
    void* m_fnSymInitialize = nullptr;
    void* m_fnSymCleanup = nullptr;
    void* m_fnSymFromAddr = nullptr;
    void* m_fnSymGetLineFromAddr64 = nullptr;
    void* m_fnMiniDumpWriteDump = nullptr;

    // Atomic stats
    struct AtomicStats {
        std::atomic<uint64_t> crashesCaught{ 0 };
        std::atomic<uint64_t> dumpsWritten{ 0 };
        std::atomic<uint64_t> symbolsResolved{ 0 };
        std::atomic<uint64_t> resolutionCount{ 0 };
        std::atomic<double>   avgResolutionMsAccum{ 0.0 };
    } m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
