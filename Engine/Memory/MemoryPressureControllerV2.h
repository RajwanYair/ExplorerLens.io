#pragma once
/******************************************************************************
 * MemoryPressureControllerV2.h
 * Copyright (c) 2026 ExplorerLens Project
 *
 * PURPOSE:
 *   5-tier memory pressure controller with real Windows memory APIs and
 *   progressive action escalation. Monitors both system-wide and per-process
 *   memory pressure using GlobalMemoryStatusEx and dynamically-loaded
 *   GetProcessMemoryInfo (avoids psapi.h / WIN32_LEAN_AND_MEAN conflicts).
 *
 * CLASSES:
 *   MemoryPressureControllerV2 — Background polling (configurable interval),
 *     hysteresis (2 consecutive checks before escalation, immediate on
 *     de-escalation), registered pressure callbacks, emergency release,
 *     and detailed statistics.
 *
 * INPUTS:
 *   Custom pressure ladder (optional), polling interval, callback registrations.
 *
 * OUTPUTS:
 *   PressureTier, PressureAction bitmask, MemoryPressureStats with timing
 *   and escalation counts.
 *
 * THREAD SAFETY:
 *   All public methods are thread-safe via SRWLOCK. Background polling thread
 *   uses atomic stop flag.
 *
 * NOTE:
 *   GetProcessMemoryInfo is loaded dynamically from kernel32.dll (on Win7+
 *   it's forwarded there) or psapi.dll, using GetProcAddress. This avoids
 *   including <psapi.h> which conflicts with WIN32_LEAN_AND_MEAN.
 *****************************************************************************/

#include <windows.h>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace ExplorerLens {
namespace Memory {

// ─── Pressure level (5-tier) ─────────────────────────────────────────────────

enum class PressureLevel : uint32_t {
    Normal = 0,    // > 50% free
    Low = 1,       // 25–50% free — start background compaction
    Medium = 2,    // 10–25% free — shed D3D11 cache
    High = 3,      // 5–10% free — shed all caches
    Critical = 4,  // < 5% free — emergency eviction, no new decodes
};

inline std::string ToString(PressureLevel p)
{
    switch (p) {
        case PressureLevel::Normal:
            return "Normal";
        case PressureLevel::Low:
            return "Low";
        case PressureLevel::Medium:
            return "Medium";
        case PressureLevel::High:
            return "High";
        case PressureLevel::Critical:
            return "Critical";
        default:
            return "Unknown";
    }
}

// ─── Response action bitmask ─────────────────────────────────────────────────

enum class PressureAction : uint32_t {
    None = 0x00,
    TrimDecodeCaches = 0x01,
    ReleasePooled = 0x02,
    FlushWriteBehind = 0x04,
    ReduceCacheBudget = 0x08,
    EvictUnpinned = 0x10,
    CompactHeaps = 0x20,
    EmergencyRelease = 0x40,
    PauseDecodeQueue = 0x80,
    BackgroundCompact = 0x01,   // alias for TrimDecodeCaches
    EvictD3D11Cache = 0x10,     // alias for EvictUnpinned
    EvictCPUPixelCache = 0x10,  // alias
    EvictMetadataCache = 0x10,  // alias
    BlockNewDecodes = 0x80,     // alias for PauseDecodeQueue
    EmitETWEvent = 0x100,
};

inline PressureAction operator|(PressureAction a, PressureAction b)
{
    return static_cast<PressureAction>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline PressureAction operator&(PressureAction a, PressureAction b)
{
    return static_cast<PressureAction>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline bool HasAction(PressureAction combined, PressureAction flag)
{
    return (static_cast<uint32_t>(combined) & static_cast<uint32_t>(flag)) != 0;
}

// ─── Response ladder rung ────────────────────────────────────────────────────

struct PressureLadderRung
{
    PressureLevel level;
    PressureAction actions;
    uint32_t maxEvictionMs{200};
    size_t targetFreeBytes{0};
};

inline std::vector<PressureLadderRung> DefaultPressureLadder()
{
    return {
        {PressureLevel::Normal, PressureAction::None, 0, 0},
        {PressureLevel::Low,
         PressureAction::TrimDecodeCaches | PressureAction::ReleasePooled | PressureAction::EmitETWEvent, 200,
         64ULL * 1024 * 1024},
        {PressureLevel::Medium,
         PressureAction::TrimDecodeCaches | PressureAction::ReleasePooled | PressureAction::FlushWriteBehind
             | PressureAction::ReduceCacheBudget | PressureAction::EmitETWEvent,
         300, 128ULL * 1024 * 1024},
        {PressureLevel::High,
         PressureAction::EvictUnpinned | PressureAction::CompactHeaps | PressureAction::FlushWriteBehind
             | PressureAction::ReduceCacheBudget | PressureAction::EmitETWEvent,
         400, 256ULL * 1024 * 1024},
        {PressureLevel::Critical,
         PressureAction::EmergencyRelease | PressureAction::PauseDecodeQueue | PressureAction::EvictUnpinned
             | PressureAction::CompactHeaps | PressureAction::EmitETWEvent,
         500, 512ULL * 1024 * 1024},
    };
}

// ─── Pressure transition event ───────────────────────────────────────────────

struct PressureTransition
{
    PressureLevel from;
    PressureLevel to;
    uint64_t timestampMs{0};
    size_t freeBytesBefore{0};
    size_t freeBytesAfter{0};
    PressureAction actionsExecuted{PressureAction::None};

    bool IsEscalation() const
    {
        return static_cast<uint32_t>(to) > static_cast<uint32_t>(from);
    }
};

// ─── Memory pressure statistics ──────────────────────────────────────────────

struct MemoryPressureStats
{
    PressureLevel currentTier{PressureLevel::Normal};
    uint64_t processWorkingSet{0};
    uint64_t processCommitCharge{0};
    uint32_t systemMemoryLoad{0};
    uint32_t escalationCount{0};
    uint64_t timeInNormalMs{0};
    uint64_t timeInLowMs{0};
    uint64_t timeInMediumMs{0};
    uint64_t timeInHighMs{0};
    uint64_t timeInCriticalMs{0};
};

// ─── Process memory info (dynamically loaded) ───────────────────────────────

struct ProcessMemInfo
{
    uint64_t workingSetBytes{0};
    uint64_t peakWorkingSetBytes{0};
    uint64_t privateBytes{0};
};

// ─── Callback registration ──────────────────────────────────────────────────

struct PressureCallbackEntry
{
    PressureLevel minTier;
    std::function<void(PressureLevel, PressureAction)> callback;
};

// ─── Pressure transition callback (backward-compatible) ─────────────────────

using PressureTransitionCallback = std::function<void(const PressureTransition&)>;

// ─── MemoryPressureControllerV2 ─────────────────────────────────────────────

class MemoryPressureControllerV2
{
  public:
    explicit MemoryPressureControllerV2(std::vector<PressureLadderRung> ladder = DefaultPressureLadder())
        : m_ladder(std::move(ladder))
        , m_current(PressureLevel::Normal)
        , m_pending(PressureLevel::Normal)
        , m_hysteresisCount(0)
        , m_escalationCount(0)
        , m_pollingRunning(false)
    {
        ::InitializeSRWLock(&m_srwLock);
        m_tierEntryTime = std::chrono::steady_clock::now();
        for (auto& t : m_tierDurations)
            t = std::chrono::milliseconds(0);
        LoadProcessMemoryInfoProc();
    }

    ~MemoryPressureControllerV2()
    {
        StopPolling();
        if (m_psapiModule)
            ::FreeLibrary(m_psapiModule);
    }

    MemoryPressureControllerV2(const MemoryPressureControllerV2&) = delete;
    MemoryPressureControllerV2& operator=(const MemoryPressureControllerV2&) = delete;

    // Allow move for Create() factory
    MemoryPressureControllerV2(MemoryPressureControllerV2&& other) noexcept
        : m_ladder(std::move(other.m_ladder))
        , m_current(other.m_current)
        , m_pending(other.m_pending)
        , m_hysteresisCount(other.m_hysteresisCount)
        , m_escalationCount(other.m_escalationCount)
        , m_pollingRunning(other.m_pollingRunning.load())
        , m_pfnGetProcessMemoryInfo(other.m_pfnGetProcessMemoryInfo)
        , m_psapiModule(other.m_psapiModule)
    {
        ::InitializeSRWLock(&m_srwLock);
        m_tierEntryTime = other.m_tierEntryTime;
        for (int i = 0; i < 5; ++i)
            m_tierDurations[i] = other.m_tierDurations[i];
        other.m_psapiModule = nullptr;
        other.m_pfnGetProcessMemoryInfo = nullptr;
    }

    // ── Factory ─────────────────────────────────────────────────────────────

    static MemoryPressureControllerV2 Create()
    {
        return MemoryPressureControllerV2(DefaultPressureLadder());
    }

    // ── Core evaluation ─────────────────────────────────────────────────────

    PressureLevel CurrentLevel() const
    {
        return m_current;
    }
    struct PressureTier
    {
        PressureLevel level{PressureLevel::Normal};
        PressureAction actions{PressureAction::None};
    };
    PressureTier EvaluatePressure()
    {
        ::AcquireSRWLockExclusive(&m_srwLock);
        MEMORYSTATUSEX msx{};
        msx.dwLength = sizeof(msx);
        ::GlobalMemoryStatusEx(&msx);

        ProcessMemInfo procMem = QueryProcessMemory();
        PressureLevel newLevel = ComputeLevel(msx, procMem);

        PressureTier result;
        result.level = newLevel;
        result.actions = GetRecommendedActionsForLevel(newLevel);
        ApplyHysteresisAndTransition(newLevel, msx.ullAvailPhys);

        ::ReleaseSRWLockExclusive(&m_srwLock);
        return result;
    }

    // ── Backward-compatible Evaluate ────────────────────────────────────────

    PressureTransition Evaluate(uint64_t totalBytes, uint64_t freeBytes)
    {
        PressureTransition t;
        t.from = m_current;
        t.freeBytesBefore = static_cast<size_t>(freeBytes);

        double ratio = totalBytes > 0 ? static_cast<double>(freeBytes) / static_cast<double>(totalBytes) : 1.0;

        PressureLevel newLevel;
        if (ratio > 0.50)
            newLevel = PressureLevel::Normal;
        else if (ratio > 0.25)
            newLevel = PressureLevel::Low;
        else if (ratio > 0.10)
            newLevel = PressureLevel::Medium;
        else if (ratio > 0.05)
            newLevel = PressureLevel::High;
        else
            newLevel = PressureLevel::Critical;

        t.to = newLevel;
        m_current = newLevel;

        for (const auto& rung : m_ladder) {
            if (rung.level == newLevel) {
                t.actionsExecuted = rung.actions;
                break;
            }
        }

        if (m_transitionCallback)
            m_transitionCallback(t);
        return t;
    }

    // ── Recommended actions ─────────────────────────────────────────────────

    static PressureAction GetRecommendedActions(PressureLevel tier)
    {
        return GetRecommendedActionsForLevel(tier);
    }

    // ── Callback registration ───────────────────────────────────────────────

    void RegisterPressureCallback(PressureLevel minTier, std::function<void(PressureLevel, PressureAction)> fn)
    {
        ::AcquireSRWLockExclusive(&m_srwLock);
        m_callbacks.push_back({minTier, std::move(fn)});
        ::ReleaseSRWLockExclusive(&m_srwLock);
    }

    void OnTransition(PressureTransitionCallback cb)
    {
        m_transitionCallback = std::move(cb);
    }

    // ── Background polling ──────────────────────────────────────────────────

    void StartPolling(uint32_t intervalMs = 2000)
    {
        if (m_pollingRunning.exchange(true))
            return;
        m_pollingThread = std::thread([this, intervalMs]() {
            while (m_pollingRunning.load()) {
                PollOnce();
                for (uint32_t w = 0; w < intervalMs && m_pollingRunning.load(); w += 100) {
                    ::Sleep(100);
                }
            }
        });
    }

    void StopPolling()
    {
        if (!m_pollingRunning.exchange(false))
            return;
        if (m_pollingThread.joinable())
            m_pollingThread.join();
    }

    // ── Emergency release ───────────────────────────────────────────────────

    void EmergencyRelease()
    {
        ::AcquireSRWLockExclusive(&m_srwLock);
        PressureLevel old = m_current;
        m_current = PressureLevel::Critical;
        m_escalationCount++;

        auto now = std::chrono::steady_clock::now();
        m_tierDurations[static_cast<size_t>(old)] +=
            std::chrono::duration_cast<std::chrono::milliseconds>(now - m_tierEntryTime);
        m_tierEntryTime = now;

        PressureAction actions = GetRecommendedActionsForLevel(PressureLevel::Critical);

        // Compact all heaps
        HANDLE heaps[64];
        DWORD heapCount = ::GetProcessHeaps(64, heaps);
        for (DWORD i = 0; i < heapCount; ++i) {
            ::HeapCompact(heaps[i], 0);
        }

        // Fire callbacks
        auto callbacks = m_callbacks;
        ::ReleaseSRWLockExclusive(&m_srwLock);

        for (const auto& entry : callbacks) {
            if (static_cast<uint32_t>(PressureLevel::Critical) >= static_cast<uint32_t>(entry.minTier)) {
                if (entry.callback)
                    entry.callback(PressureLevel::Critical, actions);
            }
        }
    }

    // ── Statistics ──────────────────────────────────────────────────────────

    MemoryPressureStats GetStats() const
    {
        ::AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_srwLock));
        MemoryPressureStats stats;
        stats.currentTier = m_current;

        ProcessMemInfo procMem = const_cast<MemoryPressureControllerV2*>(this)->QueryProcessMemory();
        stats.processWorkingSet = procMem.workingSetBytes;
        stats.processCommitCharge = procMem.privateBytes;

        MEMORYSTATUSEX msx{};
        msx.dwLength = sizeof(msx);
        ::GlobalMemoryStatusEx(&msx);
        stats.systemMemoryLoad = msx.dwMemoryLoad;

        stats.escalationCount = m_escalationCount;
        stats.timeInNormalMs = static_cast<uint64_t>(m_tierDurations[0].count());
        stats.timeInLowMs = static_cast<uint64_t>(m_tierDurations[1].count());
        stats.timeInMediumMs = static_cast<uint64_t>(m_tierDurations[2].count());
        stats.timeInHighMs = static_cast<uint64_t>(m_tierDurations[3].count());
        stats.timeInCriticalMs = static_cast<uint64_t>(m_tierDurations[4].count());

        ::ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_srwLock));
        return stats;
    }

  private:
    // ── Dynamic psapi loading ───────────────────────────────────────────────

    // PROCESS_MEMORY_COUNTERS equivalent to avoid #include <psapi.h>
    struct ProcessMemoryCounters
    {
        DWORD cb;
        DWORD PageFaultCount;
        SIZE_T PeakWorkingSetSize;
        SIZE_T WorkingSetSize;
        SIZE_T QuotaPeakPagedPoolUsage;
        SIZE_T QuotaPagedPoolUsage;
        SIZE_T QuotaPeakNonPagedPoolUsage;
        SIZE_T QuotaNonPagedPoolUsage;
        SIZE_T PagefileUsage;
        SIZE_T PeakPagefileUsage;
    };

    using FnGetProcessMemoryInfo = BOOL(WINAPI*)(HANDLE, ProcessMemoryCounters*, DWORD);

    void LoadProcessMemoryInfoProc()
    {
        // Try kernel32 first (Win7+), then psapi.dll
        m_psapiModule = ::GetModuleHandleW(L"kernel32.dll");
        if (m_psapiModule) {
            m_pfnGetProcessMemoryInfo =
                reinterpret_cast<FnGetProcessMemoryInfo>(::GetProcAddress(m_psapiModule, "K32GetProcessMemoryInfo"));
            if (m_pfnGetProcessMemoryInfo) {
                m_psapiModule = nullptr;  // Don't FreeLibrary kernel32
                return;
            }
        }
        m_psapiModule = ::LoadLibraryW(L"psapi.dll");
        if (m_psapiModule) {
            m_pfnGetProcessMemoryInfo =
                reinterpret_cast<FnGetProcessMemoryInfo>(::GetProcAddress(m_psapiModule, "GetProcessMemoryInfo"));
        }
    }

    ProcessMemInfo QueryProcessMemory()
    {
        ProcessMemInfo info;
        if (!m_pfnGetProcessMemoryInfo)
            return info;

        ProcessMemoryCounters pmc{};
        pmc.cb = sizeof(pmc);
        if (m_pfnGetProcessMemoryInfo(::GetCurrentProcess(), &pmc, sizeof(pmc))) {
            info.workingSetBytes = pmc.WorkingSetSize;
            info.peakWorkingSetBytes = pmc.PeakWorkingSetSize;
            info.privateBytes = pmc.PagefileUsage;
        }
        return info;
    }

    // ── Level computation ───────────────────────────────────────────────────

    PressureLevel ComputeLevel(const MEMORYSTATUSEX& msx, const ProcessMemInfo& /*proc*/)
    {
        uint32_t load = msx.dwMemoryLoad;
        if (load < 50)
            return PressureLevel::Normal;
        if (load < 75)
            return PressureLevel::Low;
        if (load < 90)
            return PressureLevel::Medium;
        if (load < 95)
            return PressureLevel::High;
        return PressureLevel::Critical;
    }

    static PressureAction GetRecommendedActionsForLevel(PressureLevel tier)
    {
        switch (tier) {
            case PressureLevel::Normal:
                return PressureAction::None;
            case PressureLevel::Low:
                return PressureAction::TrimDecodeCaches | PressureAction::ReleasePooled;
            case PressureLevel::Medium:
                return PressureAction::TrimDecodeCaches | PressureAction::ReleasePooled
                       | PressureAction::FlushWriteBehind | PressureAction::ReduceCacheBudget;
            case PressureLevel::High:
                return PressureAction::EvictUnpinned | PressureAction::CompactHeaps | PressureAction::FlushWriteBehind
                       | PressureAction::ReduceCacheBudget;
            case PressureLevel::Critical:
                return PressureAction::EmergencyRelease | PressureAction::PauseDecodeQueue
                       | PressureAction::EvictUnpinned | PressureAction::CompactHeaps;
            default:
                return PressureAction::None;
        }
    }

    // ── Hysteresis and transition ───────────────────────────────────────────

    void ApplyHysteresisAndTransition(PressureLevel newLevel, uint64_t freeBytes)
    {
        auto now = std::chrono::steady_clock::now();

        // Immediate de-escalation
        if (static_cast<uint32_t>(newLevel) < static_cast<uint32_t>(m_current)) {
            AccumulateTierTime(now);
            m_current = newLevel;
            m_hysteresisCount = 0;
            m_pending = newLevel;
            FireCallbacks(newLevel, GetRecommendedActionsForLevel(newLevel));
            return;
        }

        // Escalation requires 2 consecutive checks
        if (newLevel != m_current) {
            if (newLevel == m_pending) {
                m_hysteresisCount++;
            } else {
                m_pending = newLevel;
                m_hysteresisCount = 1;
            }
            if (m_hysteresisCount >= 2) {
                AccumulateTierTime(now);
                m_current = newLevel;
                m_escalationCount++;
                m_hysteresisCount = 0;
                FireCallbacks(newLevel, GetRecommendedActionsForLevel(newLevel));
            }
        } else {
            m_hysteresisCount = 0;
            m_pending = m_current;
        }
        (void)freeBytes;
    }

    void AccumulateTierTime(std::chrono::steady_clock::time_point now)
    {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_tierEntryTime);
        m_tierDurations[static_cast<size_t>(m_current)] += elapsed;
        m_tierEntryTime = now;
    }

    void FireCallbacks(PressureLevel level, PressureAction actions)
    {
        for (const auto& entry : m_callbacks) {
            if (static_cast<uint32_t>(level) >= static_cast<uint32_t>(entry.minTier)) {
                if (entry.callback)
                    entry.callback(level, actions);
            }
        }
    }

    // ── Poll once ───────────────────────────────────────────────────────────

    void PollOnce()
    {
        MEMORYSTATUSEX msx{};
        msx.dwLength = sizeof(msx);
        ::GlobalMemoryStatusEx(&msx);
        ProcessMemInfo procMem = QueryProcessMemory();

        ::AcquireSRWLockExclusive(&m_srwLock);
        PressureLevel newLevel = ComputeLevel(msx, procMem);
        ApplyHysteresisAndTransition(newLevel, msx.ullAvailPhys);
        ::ReleaseSRWLockExclusive(&m_srwLock);
    }

    // ── Members ─────────────────────────────────────────────────────────────

    SRWLOCK m_srwLock{};
    std::vector<PressureLadderRung> m_ladder;
    PressureLevel m_current;
    PressureLevel m_pending;
    uint32_t m_hysteresisCount;
    uint32_t m_escalationCount;

    std::chrono::steady_clock::time_point m_tierEntryTime;
    std::chrono::milliseconds m_tierDurations[5]{};

    std::vector<PressureCallbackEntry> m_callbacks;
    PressureTransitionCallback m_transitionCallback;

    std::atomic<bool> m_pollingRunning;
    std::thread m_pollingThread;

    FnGetProcessMemoryInfo m_pfnGetProcessMemoryInfo = nullptr;
    HMODULE m_psapiModule = nullptr;
};

}  // namespace Memory
}  // namespace ExplorerLens
