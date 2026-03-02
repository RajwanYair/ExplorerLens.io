#pragma once
/**
 * @file TelemetryEngine.h
 * @brief Privacy-respecting, local-only telemetry engine for anonymous usage analytics.
 * @version 15.0.0
 * @date 2026-03-02
 *
 * Collects anonymous performance and usage events to local JSONL files under
 * %LOCALAPPDATA%\ExplorerLens\telemetry\. Data NEVER leaves the machine.
 * Supports event recording, metric tracking, daily log rotation, auto-purge
 * of files older than 30 days, and a background flush thread.
 *
 * @note Header-only. Uses Windows API + C++20 standard library only.
 * @note NO NETWORK CODE — purely local analytics.
 *
 * @copyright (c) 2026 ExplorerLens Contributors. All rights reserved.
 */

#include <windows.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// @brief Local-only telemetry engine — data never leaves the machine.
class TelemetryEngine {
public:
    /// @brief Controls what data is collected.
    enum class TelemetryLevel : uint32_t {
        Off = 0,  ///< Nothing collected
        Basic = 1,  ///< Session start/stop + error counts
        Enhanced = 2,  ///< + format usage, cache stats
        Full = 3   ///< + per-file timing, all events
    };

    /// @brief Singleton access.
    static inline TelemetryEngine& Instance() {
        static TelemetryEngine s_instance;
        return s_instance;
    }

    /// @brief Set the collection level.
    inline void SetLevel(TelemetryLevel level) noexcept {
        m_level.store(static_cast<uint32_t>(level), std::memory_order_release);
    }

    /// @brief Get current collection level.
    inline TelemetryLevel GetLevel() const noexcept {
        return static_cast<TelemetryLevel>(m_level.load(std::memory_order_acquire));
    }

    /// @brief Master enable / disable switch.
    inline void SetEnabled(bool enabled) noexcept {
        m_enabled.store(enabled, std::memory_order_release);
    }

    /// @brief Check if telemetry is enabled.
    inline bool IsEnabled() const noexcept {
        return m_enabled.load(std::memory_order_acquire);
    }

    /// @brief Current session ID (random, regenerated on each process start).
    inline uint64_t GetSessionId() const noexcept {
        return m_sessionId;
    }

    // -----------------------------------------------------------------
    //  Event recording
    // -----------------------------------------------------------------

    /// @brief Record a named event with optional string properties.
    inline void RecordEvent(const std::string& eventName,
        const std::unordered_map<std::string, std::string>& properties = {}) {
        if (!ShouldRecord(TelemetryLevel::Basic)) return;

        std::ostringstream os;
        os << "{\"ts\":\"" << GetISO8601() << "\""
            << ",\"sid\":" << m_sessionId
            << ",\"event\":\"" << EscapeJson(eventName) << "\"";

        if (!properties.empty()) {
            os << ",\"props\":{";
            bool first = true;
            for (auto& [k, v] : properties) {
                if (!first) os << ",";
                os << "\"" << EscapeJson(k) << "\":\"" << EscapeJson(v) << "\"";
                first = false;
            }
            os << "}";
        }
        os << "}\n";

        BufferLine(os.str());
        m_totalEvents.fetch_add(1, std::memory_order_relaxed);
    }

    /// @brief Record a numeric metric.
    inline void RecordMetric(const std::string& name, double value) {
        if (!ShouldRecord(TelemetryLevel::Enhanced)) return;

        std::ostringstream os;
        os << "{\"ts\":\"" << GetISO8601() << "\""
            << ",\"sid\":" << m_sessionId
            << ",\"metric\":\"" << EscapeJson(name) << "\""
            << ",\"value\":" << std::fixed << std::setprecision(3) << value
            << "}\n";

        BufferLine(os.str());
    }

    /// @brief Record a duration in microseconds.
    inline void RecordDuration(const std::string& name, uint64_t microseconds) {
        if (!ShouldRecord(TelemetryLevel::Full)) return;

        std::ostringstream os;
        os << "{\"ts\":\"" << GetISO8601() << "\""
            << ",\"sid\":" << m_sessionId
            << ",\"duration\":\"" << EscapeJson(name) << "\""
            << ",\"us\":" << microseconds
            << "}\n";

        BufferLine(os.str());
    }

    // -----------------------------------------------------------------
    //  Reporting & export
    // -----------------------------------------------------------------

    /// @brief Generate a local usage report (plain text).
    inline std::string GenerateUsageReport() const {
        std::ostringstream os;
        os << "=== ExplorerLens Telemetry Summary ===\n"
            << "Session ID: " << m_sessionId << "\n"
            << "Total events: " << m_totalEvents.load(std::memory_order_relaxed) << "\n"
            << "Buffered lines: " << GetBufferSize() << "\n"
            << "Enabled: " << (m_enabled.load() ? "yes" : "no") << "\n"
            << "Level: " << static_cast<uint32_t>(GetLevel()) << "\n"
            << "Telemetry dir: " << WideToUTF8(m_telemetryDir) << "\n";

        // Count daily files
        uint32_t fileCount = 0;
        uint64_t totalBytes = 0;
        WIN32_FIND_DATAW fd{};
        std::wstring pattern = m_telemetryDir + L"\\*.jsonl";
        HANDLE hFind = FindFirstFileW(pattern.c_str(), &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                ++fileCount;
                totalBytes += (static_cast<uint64_t>(fd.nFileSizeHigh) << 32) | fd.nFileSizeLow;
            } while (FindNextFileW(hFind, &fd));
            FindClose(hFind);
        }

        os << "Daily log files: " << fileCount << "\n"
            << "Total log size: " << totalBytes << " bytes\n"
            << "=====================================\n";
        return os.str();
    }

    /// @brief Export all telemetry data to a single file.
    inline bool ExportData(const std::wstring& path) const {
        FlushBuffer();

        HANDLE hOut = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hOut == INVALID_HANDLE_VALUE) return false;

        WIN32_FIND_DATAW fd{};
        std::wstring pattern = m_telemetryDir + L"\\*.jsonl";
        HANDLE hFind = FindFirstFileW(pattern.c_str(), &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                std::wstring filePath = m_telemetryDir + L"\\" + fd.cFileName;
                HANDLE hIn = CreateFileW(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ,
                    nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
                if (hIn != INVALID_HANDLE_VALUE) {
                    char buf[8192];
                    DWORD bytesRead = 0;
                    while (ReadFile(hIn, buf, sizeof(buf), &bytesRead, nullptr) && bytesRead > 0) {
                        DWORD written = 0;
                        WriteFile(hOut, buf, bytesRead, &written, nullptr);
                    }
                    CloseHandle(hIn);
                }
            } while (FindNextFileW(hFind, &fd));
            FindClose(hFind);
        }

        CloseHandle(hOut);
        return true;
    }

    /// @brief Delete all telemetry files.
    inline void PurgeAllData() {
        WIN32_FIND_DATAW fd{};
        std::wstring pattern = m_telemetryDir + L"\\*.jsonl";
        HANDLE hFind = FindFirstFileW(pattern.c_str(), &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                std::wstring filePath = m_telemetryDir + L"\\" + fd.cFileName;
                DeleteFileW(filePath.c_str());
            } while (FindNextFileW(hFind, &fd));
            FindClose(hFind);
        }
        m_totalEvents.store(0, std::memory_order_relaxed);
    }

    /// @brief Flush any buffered events.
    inline void Flush() {
        FlushBuffer();
    }

    ~TelemetryEngine() {
        m_stopFlush.store(true, std::memory_order_release);
        if (m_flushThread.joinable()) {
            m_flushThread.join();
        }
        FlushBuffer();
    }

private:
    TelemetryEngine() {
        InitializeSRWLock(&m_lock);
        m_sessionId = GenerateSessionId();
        m_telemetryDir = ResolveTelemetryDir();
        EnsureDirectoryExists(m_telemetryDir);
        PurgeOldFiles(30);

        // Start background flush thread (every 5 seconds)
        m_stopFlush.store(false);
        m_flushThread = std::thread([this]() {
            while (!m_stopFlush.load(std::memory_order_acquire)) {
                Sleep(5000);
                if (!m_stopFlush.load(std::memory_order_acquire)) {
                    FlushBuffer();
                }
            }
            });
    }

    TelemetryEngine(const TelemetryEngine&) = delete;
    TelemetryEngine& operator=(const TelemetryEngine&) = delete;

    // Members
    SRWLOCK          m_lock{};
    std::atomic<uint32_t> m_level{ static_cast<uint32_t>(TelemetryLevel::Basic) };
    std::atomic<bool>     m_enabled{ true };
    uint64_t         m_sessionId = 0;
    std::wstring     m_telemetryDir;
    std::vector<std::string> m_buffer;
    std::atomic<uint64_t>    m_totalEvents{ 0 };
    std::thread      m_flushThread;
    std::atomic<bool> m_stopFlush{ false };

    // -- Helpers --

    inline bool ShouldRecord(TelemetryLevel minLevel) const noexcept {
        if (!m_enabled.load(std::memory_order_acquire)) return false;
        return m_level.load(std::memory_order_acquire) >= static_cast<uint32_t>(minLevel);
    }

    inline void BufferLine(std::string line) {
        AcquireSRWLockExclusive(&m_lock);
        m_buffer.emplace_back(std::move(line));
        // Auto-flush at 1000 lines
        if (m_buffer.size() >= 1000) {
            auto buf = std::move(m_buffer);
            m_buffer.clear();
            m_buffer.reserve(256);
            ReleaseSRWLockExclusive(&m_lock);
            WriteToDisk(buf);
            return;
        }
        ReleaseSRWLockExclusive(&m_lock);
    }

    inline size_t GetBufferSize() const {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        size_t sz = m_buffer.size();
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        return sz;
    }

    inline void FlushBuffer() const {
        AcquireSRWLockExclusive(const_cast<PSRWLOCK>(&m_lock));
        auto& mutBuf = const_cast<std::vector<std::string>&>(m_buffer);
        auto buf = std::move(mutBuf);
        mutBuf.clear();
        mutBuf.reserve(256);
        ReleaseSRWLockExclusive(const_cast<PSRWLOCK>(&m_lock));

        if (!buf.empty()) {
            WriteToDisk(buf);
        }
    }

    inline void WriteToDisk(const std::vector<std::string>& lines) const {
        if (lines.empty()) return;

        std::wstring fileName = GetDailyFileName();
        std::wstring fullPath = m_telemetryDir + L"\\" + fileName;

        HANDLE hFile = CreateFileW(fullPath.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ,
            nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return;

        for (auto& line : lines) {
            DWORD written = 0;
            WriteFile(hFile, line.data(), static_cast<DWORD>(line.size()), &written, nullptr);
        }
        CloseHandle(hFile);
    }

    static inline std::wstring GetDailyFileName() {
        SYSTEMTIME st{};
        GetLocalTime(&st);
        wchar_t buf[32]{};
        _snwprintf_s(buf, _countof(buf), _TRUNCATE,
            L"%04d-%02d-%02d.jsonl", st.wYear, st.wMonth, st.wDay);
        return std::wstring(buf);
    }

    static inline std::string GetISO8601() {
        SYSTEMTIME st{};
        GetSystemTime(&st);
        char buf[32]{};
        _snprintf_s(buf, _countof(buf), _TRUNCATE,
            "%04d-%02d-%02dT%02d:%02d:%02dZ",
            st.wYear, st.wMonth, st.wDay,
            st.wHour, st.wMinute, st.wSecond);
        return std::string(buf);
    }

    static inline std::wstring ResolveTelemetryDir() {
        wchar_t appData[MAX_PATH]{};
        DWORD len = GetEnvironmentVariableW(L"LOCALAPPDATA", appData, MAX_PATH);
        std::wstring dir;
        if (len > 0 && len < MAX_PATH) {
            dir = std::wstring(appData, len);
        }
        else {
            dir = L"C:\\ProgramData";
        }
        dir += L"\\ExplorerLens\\telemetry";
        return dir;
    }

    static inline void EnsureDirectoryExists(const std::wstring& dir) {
        // Create parent first
        auto slash = dir.rfind(L'\\');
        if (slash != std::wstring::npos && slash > 0) {
            std::wstring parent = dir.substr(0, slash);
            CreateDirectoryW(parent.c_str(), nullptr);
        }
        CreateDirectoryW(dir.c_str(), nullptr);
    }

    inline void PurgeOldFiles(uint32_t maxAgeDays) {
        FILETIME now{};
        GetSystemTimeAsFileTime(&now);

        ULARGE_INTEGER nowLi;
        nowLi.LowPart = now.dwLowDateTime;
        nowLi.HighPart = now.dwHighDateTime;

        // 100-nanosecond intervals per day
        uint64_t maxAgeIntervals = static_cast<uint64_t>(maxAgeDays) * 24ULL * 3600ULL * 10000000ULL;

        WIN32_FIND_DATAW fd{};
        std::wstring pattern = m_telemetryDir + L"\\*.jsonl";
        HANDLE hFind = FindFirstFileW(pattern.c_str(), &fd);
        if (hFind == INVALID_HANDLE_VALUE) return;

        do {
            ULARGE_INTEGER ftWrite;
            ftWrite.LowPart = fd.ftLastWriteTime.dwLowDateTime;
            ftWrite.HighPart = fd.ftLastWriteTime.dwHighDateTime;

            if (nowLi.QuadPart > ftWrite.QuadPart &&
                (nowLi.QuadPart - ftWrite.QuadPart) > maxAgeIntervals) {
                std::wstring filePath = m_telemetryDir + L"\\" + fd.cFileName;
                DeleteFileW(filePath.c_str());
            }
        } while (FindNextFileW(hFind, &fd));
        FindClose(hFind);
    }

    static inline uint64_t GenerateSessionId() {
        LARGE_INTEGER perf{};
        QueryPerformanceCounter(&perf);
        uint64_t seed = static_cast<uint64_t>(perf.QuadPart);
        seed ^= static_cast<uint64_t>(GetCurrentProcessId()) << 32;
        seed ^= static_cast<uint64_t>(GetTickCount64());
        // Simple xorshift64*
        seed ^= seed >> 12;
        seed ^= seed << 25;
        seed ^= seed >> 27;
        return seed * 0x2545F4914F6CDD1DULL;
    }

    static inline std::string EscapeJson(const std::string& s) {
        std::string out;
        out.reserve(s.size() + 8);
        for (char c : s) {
            switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;      break;
            }
        }
        return out;
    }

    static inline std::string WideToUTF8(const std::wstring& wide) {
        if (wide.empty()) return {};
        int len = WideCharToMultiByte(CP_UTF8, 0, wide.data(),
            static_cast<int>(wide.size()),
            nullptr, 0, nullptr, nullptr);
        if (len <= 0) return {};
        std::string out(static_cast<size_t>(len), '\0');
        WideCharToMultiByte(CP_UTF8, 0, wide.data(),
            static_cast<int>(wide.size()),
            out.data(), len, nullptr, nullptr);
        return out;
    }
};

} // namespace Engine
} // namespace ExplorerLens
