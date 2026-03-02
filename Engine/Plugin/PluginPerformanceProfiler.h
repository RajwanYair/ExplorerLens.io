//==============================================================================
// ExplorerLens Engine — Plugin Performance Profiler (Sprint 582)
//
// Profiles plugin execution with high-resolution QueryPerformanceCounter
// timing and resource tracking. Features:
//   - Sub-microsecond timing via QPC
//   - Per-session memory delta tracking (GetProcessMemoryInfo via dynamic load)
//   - GDI object leak detection (GetGuiResources)
//   - Rolling window of last 1000 records per plugin
//   - P95 latency calculation, slow operation flagging
//   - CSV report export
//   - Thread-safe with SRWLOCK
//
// Header-only, C++20, MSVC /W4 clean.
//==============================================================================
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <cstdint>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <numeric>

namespace ExplorerLens {
namespace Engine {

// PROCESS_MEMORY_COUNTERS structure (avoids including <psapi.h>)
struct ProcessMemCounters {
    DWORD  cb;
    DWORD  PageFaultCount;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaPeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
};

struct ProfileRecord {
    std::wstring pluginId;
    std::string  operation;
    uint64_t     startUs         = 0;
    uint64_t     endUs           = 0;
    uint64_t     durationUs      = 0;
    int64_t      memoryDeltaBytes = 0;
    int32_t      gdiObjectDelta  = 0;
};

struct ProfileSummary {
    std::wstring pluginId;
    uint64_t     totalRecords    = 0;
    uint64_t     avgDurationUs   = 0;
    uint64_t     minDurationUs   = 0;
    uint64_t     maxDurationUs   = 0;
    uint64_t     p95DurationUs   = 0;
    int64_t      memoryTrendBytes = 0; // positive = growing
    bool         gdiLeakDetected = false;
    std::unordered_map<std::string, uint64_t> avgPerOperation;
};

class PluginPerformanceProfiler {
public:
    static constexpr size_t MAX_RECORDS_PER_PLUGIN = 1000;

    PluginPerformanceProfiler() {
        InitializeSRWLock(&m_lock);

        // Initialize QPC frequency
        LARGE_INTEGER freq{};
        QueryPerformanceFrequency(&freq);
        m_qpcFrequency = freq.QuadPart;

        // Dynamically load GetProcessMemoryInfo from kernel32.dll
        // (On Win7+ it's in kernel32 via K32GetProcessMemoryInfo)
        HMODULE hKernel = GetModuleHandleW(L"kernel32.dll");
        if (hKernel) {
            m_fnGetProcessMemoryInfo = reinterpret_cast<GetProcessMemoryInfoFn>(
                GetProcAddress(hKernel, "K32GetProcessMemoryInfo"));
        }
    }

    ~PluginPerformanceProfiler() = default;

    PluginPerformanceProfiler(const PluginPerformanceProfiler&) = delete;
    PluginPerformanceProfiler& operator=(const PluginPerformanceProfiler&) = delete;

    inline uint64_t BeginProfile(const std::wstring& pluginId, const std::string& operation) {
        ActiveSession session;
        session.pluginId  = pluginId;
        session.operation = operation;

        // Capture start QPC tick
        LARGE_INTEGER tick{};
        QueryPerformanceCounter(&tick);
        session.startTick = tick.QuadPart;
        session.startUs   = TickToMicroseconds(tick.QuadPart);

        // Capture start memory
        session.startMemoryBytes = GetCurrentWorkingSetBytes();

        // Capture start GDI objects
        session.startGdiObjects = static_cast<int32_t>(
            GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS));

        uint64_t sessionId = m_nextSessionId.fetch_add(1, std::memory_order_relaxed);

        AcquireSRWLockExclusive(&m_lock);
        m_activeSessions[sessionId] = std::move(session);
        ReleaseSRWLockExclusive(&m_lock);

        return sessionId;
    }

    inline void EndProfile(uint64_t sessionId) {
        // Capture end QPC tick immediately
        LARGE_INTEGER tick{};
        QueryPerformanceCounter(&tick);
        uint64_t endUs = TickToMicroseconds(tick.QuadPart);

        // Capture end memory
        int64_t endMemory = GetCurrentWorkingSetBytes();

        // Capture end GDI objects
        int32_t endGdi = static_cast<int32_t>(
            GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS));

        AcquireSRWLockExclusive(&m_lock);
        auto it = m_activeSessions.find(sessionId);
        if (it == m_activeSessions.end()) {
            ReleaseSRWLockExclusive(&m_lock);
            return;
        }

        const auto& session = it->second;

        ProfileRecord record;
        record.pluginId         = session.pluginId;
        record.operation        = session.operation;
        record.startUs          = session.startUs;
        record.endUs            = endUs;
        record.durationUs       = (endUs > session.startUs) ? (endUs - session.startUs) : 0;
        record.memoryDeltaBytes = endMemory - session.startMemoryBytes;
        record.gdiObjectDelta   = endGdi - session.startGdiObjects;

        // Store in per-plugin rolling window
        auto& records = m_records[record.pluginId];
        if (records.size() >= MAX_RECORDS_PER_PLUGIN) {
            records.erase(records.begin()); // Evict oldest
        }
        records.push_back(record);

        m_activeSessions.erase(it);
        ReleaseSRWLockExclusive(&m_lock);
    }

    inline std::vector<ProfileRecord> GetRecords(const std::wstring& pluginId) const {
        AcquireSRWLockShared(&m_lock);
        auto it = m_records.find(pluginId);
        std::vector<ProfileRecord> result;
        if (it != m_records.end()) {
            result = it->second;
        }
        ReleaseSRWLockShared(&m_lock);
        return result;
    }

    inline ProfileSummary GetSummary(const std::wstring& pluginId) const {
        ProfileSummary summary;
        summary.pluginId = pluginId;

        AcquireSRWLockShared(&m_lock);
        auto it = m_records.find(pluginId);
        if (it == m_records.end() || it->second.empty()) {
            ReleaseSRWLockShared(&m_lock);
            return summary;
        }

        const auto& records = it->second;
        summary.totalRecords = records.size();

        // Collect durations for percentile calculation
        std::vector<uint64_t> durations;
        durations.reserve(records.size());

        // Per-operation duration accumulators
        std::unordered_map<std::string, std::pair<uint64_t, uint64_t>> opAccum; // sum, count

        int64_t totalMemoryDelta = 0;
        int32_t totalGdiDelta = 0;
        uint64_t minDur = UINT64_MAX;
        uint64_t maxDur = 0;

        for (const auto& r : records) {
            durations.push_back(r.durationUs);
            minDur = (std::min)(minDur, r.durationUs);
            maxDur = (std::max)(maxDur, r.durationUs);
            totalMemoryDelta += r.memoryDeltaBytes;
            totalGdiDelta += r.gdiObjectDelta;
            opAccum[r.operation].first += r.durationUs;
            opAccum[r.operation].second++;
        }

        ReleaseSRWLockShared(&m_lock);

        // Compute statistics
        uint64_t totalDuration = 0;
        for (auto d : durations) totalDuration += d;
        summary.avgDurationUs = totalDuration / durations.size();
        summary.minDurationUs = minDur;
        summary.maxDurationUs = maxDur;

        // P95 calculation
        std::sort(durations.begin(), durations.end());
        size_t p95Index = static_cast<size_t>(
            static_cast<double>(durations.size()) * 0.95);
        p95Index = (std::min)(p95Index, durations.size() - 1);
        summary.p95DurationUs = durations[p95Index];

        // Memory trend: average delta
        summary.memoryTrendBytes = totalMemoryDelta / static_cast<int64_t>(records.size());

        // GDI leak: total delta > 0 across all records
        summary.gdiLeakDetected = (totalGdiDelta > 0);

        // Per-operation averages
        for (const auto& [op, pair] : opAccum) {
            summary.avgPerOperation[op] = pair.first / pair.second;
        }

        return summary;
    }

    inline void SetSlowThreshold(uint64_t microseconds) {
        AcquireSRWLockExclusive(&m_lock);
        m_slowThresholdUs = microseconds;
        ReleaseSRWLockExclusive(&m_lock);
    }

    inline std::vector<ProfileRecord> GetSlowOperations() const {
        std::vector<ProfileRecord> slow;
        AcquireSRWLockShared(&m_lock);
        uint64_t threshold = m_slowThresholdUs;
        for (const auto& [pluginId, records] : m_records) {
            for (const auto& r : records) {
                if (r.durationUs > threshold) {
                    slow.push_back(r);
                }
            }
        }
        ReleaseSRWLockShared(&m_lock);
        return slow;
    }

    inline bool ExportReport(const std::wstring& outputPath) const {
        AcquireSRWLockShared(&m_lock);

        // Collect all records
        std::vector<ProfileRecord> allRecords;
        for (const auto& [pid, records] : m_records) {
            allRecords.insert(allRecords.end(), records.begin(), records.end());
        }
        ReleaseSRWLockShared(&m_lock);

        if (allRecords.empty()) return false;

        // Open output file
        HANDLE hFile = CreateFileW(outputPath.c_str(), GENERIC_WRITE, 0,
            nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return false;

        // Write CSV header
        std::ostringstream csv;
        csv << "PluginId,Operation,StartUs,EndUs,DurationUs,MemoryDeltaBytes,GdiObjectDelta\r\n";

        for (const auto& r : allRecords) {
            // Convert pluginId to narrow for CSV
            int len = WideCharToMultiByte(CP_UTF8, 0, r.pluginId.c_str(),
                static_cast<int>(r.pluginId.size()), nullptr, 0, nullptr, nullptr);
            std::string narrow(static_cast<size_t>(len), '\0');
            WideCharToMultiByte(CP_UTF8, 0, r.pluginId.c_str(),
                static_cast<int>(r.pluginId.size()), narrow.data(), len, nullptr, nullptr);

            csv << narrow << ","
                << r.operation << ","
                << r.startUs << ","
                << r.endUs << ","
                << r.durationUs << ","
                << r.memoryDeltaBytes << ","
                << r.gdiObjectDelta << "\r\n";
        }

        std::string csvStr = csv.str();
        DWORD written = 0;
        WriteFile(hFile, csvStr.c_str(), static_cast<DWORD>(csvStr.size()),
            &written, nullptr);
        CloseHandle(hFile);

        return written == static_cast<DWORD>(csvStr.size());
    }

private:
    struct ActiveSession {
        std::wstring pluginId;
        std::string  operation;
        LONGLONG     startTick        = 0;
        uint64_t     startUs          = 0;
        int64_t      startMemoryBytes = 0;
        int32_t      startGdiObjects  = 0;
    };

    using GetProcessMemoryInfoFn = BOOL(WINAPI*)(HANDLE, ProcessMemCounters*, DWORD);

    inline uint64_t TickToMicroseconds(LONGLONG tick) const {
        // Convert QPC ticks to microseconds: (tick * 1,000,000) / frequency
        // Use double to avoid overflow on high tick values
        return static_cast<uint64_t>(
            static_cast<double>(tick) * 1000000.0 / static_cast<double>(m_qpcFrequency));
    }

    inline int64_t GetCurrentWorkingSetBytes() const {
        if (!m_fnGetProcessMemoryInfo) return 0;

        ProcessMemCounters pmc{};
        pmc.cb = sizeof(pmc);
        if (m_fnGetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
            return static_cast<int64_t>(pmc.WorkingSetSize);
        }
        return 0;
    }

    // Members
    mutable SRWLOCK m_lock{};
    LONGLONG        m_qpcFrequency = 1;
    std::atomic<uint64_t> m_nextSessionId{1};
    uint64_t        m_slowThresholdUs = 10000; // 10ms default

    std::unordered_map<uint64_t, ActiveSession> m_activeSessions;
    std::unordered_map<std::wstring, std::vector<ProfileRecord>> m_records;

    GetProcessMemoryInfoFn m_fnGetProcessMemoryInfo = nullptr;
};

} // namespace Engine
} // namespace ExplorerLens
