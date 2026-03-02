// ============================================================================
// AdaptiveGPUScheduler.h — Dynamic GPU/CPU Work Scheduling
// ExplorerLens Engine v15.0.0  (Sprint 567)
// Copyright (c) 2026 ExplorerLens Project
//
// PURPOSE
//   Dynamically schedules thumbnail decode and resize work between GPU
//   and CPU based on real-time GPU memory pressure and system power state.
//   Queries GPU utilisation via IDXGIAdapter3::QueryVideoMemoryInfo
//   (dynamically loaded), checks battery status with GetSystemPowerStatus,
//   and maintains a priority-sorted work queue that is dispatched according
//   to the current decision policy.
//
// CLASSES
//   AdaptiveGPUScheduler — main scheduler
//
// KEY API
//   Initialize()           — enumerate DXGI adapters, select primary
//   QueryGPULoad()         — returns GPULoadInfo (dedicated/shared mem)
//   ShouldUseGPU(...)      — tri-state decision: GPU / CPU / Defer
//   SetGPUMemoryReserve()  — always keep this much VRAM free
//   SetPreferCPUOnBattery()— reduce GPU usage on battery power
//   SubmitGPUWork(fn, pri) — enqueue work; dispatched by ProcessQueue()
//   ProcessQueue()         — drain the queue honouring current policy
//   GetStats()             — SchedulerStats
//
// THREAD SAFETY
//   All public methods guarded by SRWLOCK.
//
// DEPENDENCIES
//   Windows API only.  dxgi.dll loaded dynamically — no .lib needed.
// ============================================================================
#pragma once

#include <windows.h>
#include <dxgi1_4.h>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <atomic>
#include <cstdint>
#include <chrono>
#include <string>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

// -----------------------------------------------------------------------
// GPULoadInfo
// -----------------------------------------------------------------------
struct GPULoadInfo {
    uint64_t dedicatedUsed     = 0;
    uint64_t dedicatedBudget   = 0;
    uint64_t sharedUsed        = 0;
    uint64_t sharedBudget      = 0;

    float DedicatedUsagePercent() const {
        return (dedicatedBudget > 0)
            ? (static_cast<float>(dedicatedUsed) / static_cast<float>(dedicatedBudget) * 100.0f)
            : 0.0f;
    }
    uint64_t DedicatedAvailable() const {
        return (dedicatedBudget > dedicatedUsed) ? (dedicatedBudget - dedicatedUsed) : 0;
    }
};

// -----------------------------------------------------------------------
// ScheduleDecision
// -----------------------------------------------------------------------
enum class ScheduleDecision : uint8_t {
    GPU   = 0,
    CPU   = 1,
    Defer = 2
};

inline const char* ScheduleDecisionName(ScheduleDecision d) {
    switch (d) {
        case ScheduleDecision::GPU:   return "GPU";
        case ScheduleDecision::CPU:   return "CPU";
        case ScheduleDecision::Defer: return "Defer";
    }
    return "Unknown";
}

// -----------------------------------------------------------------------
// SchedulerStats
// -----------------------------------------------------------------------
struct SchedulerStats {
    uint64_t gpuDecisions    = 0;
    uint64_t cpuDecisions    = 0;
    uint64_t deferredItems   = 0;
    uint64_t workItemsQueued = 0;
    uint64_t workItemsDone   = 0;
    GPULoadInfo lastGPULoad;
    bool     onBattery       = false;
};

// -----------------------------------------------------------------------
// AdaptiveGPUScheduler
// -----------------------------------------------------------------------
class AdaptiveGPUScheduler {
public:
    AdaptiveGPUScheduler()  = default;
    ~AdaptiveGPUScheduler() { Shutdown(); }

    AdaptiveGPUScheduler(const AdaptiveGPUScheduler&)            = delete;
    AdaptiveGPUScheduler& operator=(const AdaptiveGPUScheduler&) = delete;

    // ================================================================
    // Initialize — enumerate DXGI adapters
    // ================================================================
    inline bool Initialize() {
        AcquireExclusive();
        if (m_ready) { ReleaseExclusive(); return true; }

        m_hDXGI = ::LoadLibraryW(L"dxgi.dll");
        if (m_hDXGI) {
            using PFN_CreateDXGIFactory1 = HRESULT(WINAPI*)(REFIID, void**);
            auto pfn = reinterpret_cast<PFN_CreateDXGIFactory1>(
                ::GetProcAddress(m_hDXGI, "CreateDXGIFactory1"));
            if (pfn) {
                IDXGIFactory1* factory = nullptr;
                if (SUCCEEDED(pfn(__uuidof(IDXGIFactory1),
                                  reinterpret_cast<void**>(&factory)))) {
                    IDXGIAdapter1* adapter1 = nullptr;
                    if (SUCCEEDED(factory->EnumAdapters1(0, &adapter1))) {
                        HRESULT hr = adapter1->QueryInterface(
                            __uuidof(IDXGIAdapter3),
                            reinterpret_cast<void**>(&m_adapter));
                        if (FAILED(hr)) m_adapter = nullptr;
                        adapter1->Release();
                    }
                    factory->Release();
                }
            }
        }
        m_ready = true;
        ReleaseExclusive();
        return true;
    }

    // ================================================================
    // QueryGPULoad
    // ================================================================
    inline GPULoadInfo QueryGPULoad() {
        AcquireShared();
        GPULoadInfo info{};
        if (m_adapter) {
            DXGI_QUERY_VIDEO_MEMORY_INFO dedicated{};
            DXGI_QUERY_VIDEO_MEMORY_INFO shared{};
            if (SUCCEEDED(m_adapter->QueryVideoMemoryInfo(
                    0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &dedicated))) {
                info.dedicatedUsed   = dedicated.CurrentUsage;
                info.dedicatedBudget = dedicated.Budget;
            }
            if (SUCCEEDED(m_adapter->QueryVideoMemoryInfo(
                    0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &shared))) {
                info.sharedUsed   = shared.CurrentUsage;
                info.sharedBudget = shared.Budget;
            }
        }
        m_stats.lastGPULoad = info;
        ReleaseShared();
        return info;
    }

    // ================================================================
    // ShouldUseGPU — tri-state decision
    // ================================================================
    inline ScheduleDecision ShouldUseGPU(size_t estimatedVRAM,
                                         uint32_t estimatedDispatchCount) {
        (void)estimatedDispatchCount;
        AcquireShared();

        // Check battery
        SYSTEM_POWER_STATUS ps{};
        ::GetSystemPowerStatus(&ps);
        bool battery = (ps.ACLineStatus == 0);
        m_stats.onBattery = battery;

        if (battery && m_preferCPUOnBattery) {
            m_stats.cpuDecisions++;
            ReleaseShared();
            return ScheduleDecision::CPU;
        }

        GPULoadInfo load = QueryGPULoadInternal();

        // Dedicated memory > 80% of budget → CPU
        if (load.DedicatedUsagePercent() > 80.0f) {
            m_stats.cpuDecisions++;
            ReleaseShared();
            return ScheduleDecision::CPU;
        }

        // Not enough available VRAM (including reserve)
        uint64_t available = load.DedicatedAvailable();
        uint64_t required  = static_cast<uint64_t>(estimatedVRAM) + m_gpuMemReserve;
        if (required > available) {
            m_stats.cpuDecisions++;
            ReleaseShared();
            return ScheduleDecision::CPU;
        }

        m_stats.gpuDecisions++;
        ReleaseShared();
        return ScheduleDecision::GPU;
    }

    // ================================================================
    // Configuration
    // ================================================================
    inline void SetGPUMemoryReserve(size_t bytes) {
        AcquireExclusive();
        m_gpuMemReserve = bytes;
        ReleaseExclusive();
    }

    inline void SetPreferCPUOnBattery(bool prefer) {
        AcquireExclusive();
        m_preferCPUOnBattery = prefer;
        ReleaseExclusive();
    }

    // ================================================================
    // Work queue
    // ================================================================
    inline void SubmitGPUWork(std::function<void()> work, uint32_t priority) {
        AcquireExclusive();
        m_workQueue.push({ std::move(work), priority });
        m_stats.workItemsQueued++;
        ReleaseExclusive();
    }

    inline void ProcessQueue() {
        AcquireExclusive();

        // Move queue items to a local vector sorted by priority
        std::vector<WorkItem> items;
        while (!m_workQueue.empty()) {
            items.push_back(std::move(m_workQueue.top()));
            m_workQueue.pop();
        }
        ReleaseExclusive();

        // Execute outside lock
        for (auto& item : items) {
            if (item.work) {
                item.work();
            }
            AcquireExclusive();
            m_stats.workItemsDone++;
            ReleaseExclusive();
        }
    }

    // ================================================================
    // GetStats
    // ================================================================
    inline SchedulerStats GetStats() {
        AcquireShared();
        SchedulerStats copy = m_stats;
        ReleaseShared();
        return copy;
    }

private:
    // ---- work item with priority ----
    struct WorkItem {
        std::function<void()> work;
        uint32_t              priority = 0;

        bool operator<(const WorkItem& rhs) const {
            return priority < rhs.priority;  // max-heap: higher priority first
        }
    };

    // ---- SRWLOCK ----
    SRWLOCK m_srw = SRWLOCK_INIT;
    inline void AcquireExclusive() { ::AcquireSRWLockExclusive(&m_srw); }
    inline void ReleaseExclusive() { ::ReleaseSRWLockExclusive(&m_srw); }
    inline void AcquireShared()    { ::AcquireSRWLockShared(&m_srw);    }
    inline void ReleaseShared()    { ::ReleaseSRWLockShared(&m_srw);    }

    // ---- state ----
    bool            m_ready            = false;
    bool            m_preferCPUOnBattery = true;
    size_t          m_gpuMemReserve    = 64 * 1024 * 1024;  // 64 MB default reserve
    HMODULE         m_hDXGI            = nullptr;
    IDXGIAdapter3*  m_adapter          = nullptr;
    SchedulerStats  m_stats{};
    std::priority_queue<WorkItem> m_workQueue;

    // ---- internal GPU load query (no lock) ----
    inline GPULoadInfo QueryGPULoadInternal() const {
        GPULoadInfo info{};
        if (m_adapter) {
            DXGI_QUERY_VIDEO_MEMORY_INFO dedicated{};
            DXGI_QUERY_VIDEO_MEMORY_INFO shared{};
            if (SUCCEEDED(m_adapter->QueryVideoMemoryInfo(
                    0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &dedicated))) {
                info.dedicatedUsed   = dedicated.CurrentUsage;
                info.dedicatedBudget = dedicated.Budget;
            }
            if (SUCCEEDED(m_adapter->QueryVideoMemoryInfo(
                    0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &shared))) {
                info.sharedUsed   = shared.CurrentUsage;
                info.sharedBudget = shared.Budget;
            }
        }
        return info;
    }

    // ---- Shutdown ----
    inline void Shutdown() {
        if (m_adapter) { m_adapter->Release(); m_adapter = nullptr; }
        if (m_hDXGI)   { ::FreeLibrary(m_hDXGI); m_hDXGI = nullptr; }
        m_ready = false;
    }
};

} // namespace Engine
} // namespace ExplorerLens
