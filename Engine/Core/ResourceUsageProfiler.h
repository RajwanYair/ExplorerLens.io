// ResourceUsageProfiler.h — Runtime Resource Usage Profiler
// Copyright (c) 2026 ExplorerLens Project
//
// Profiles resource usage per decode operation: private bytes, handles,
// GDI objects, GPU VRAM, thread time, and I/O bytes. Tracks high-water
// marks and emits per-format resource consumption reports.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <psapi.h>
#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct ProcessResourceSnapshot
{
    uint64_t privateBytes = 0;
    uint64_t workingSet = 0;
    uint32_t handleCount = 0;
    uint32_t gdiObjects = 0;
    uint32_t userObjects = 0;
    uint64_t kernelTimeUs = 0;
    uint64_t userTimeUs = 0;
    uint64_t readBytes = 0;
    uint64_t writeBytes = 0;
    uint64_t tick = 0;
};

struct ResourceDelta
{
    int64_t privateBytesChange = 0;
    int64_t workingSetChange = 0;
    int32_t handleChange = 0;
    int32_t gdiChange = 0;
    int64_t cpuTimeUs = 0;
    uint64_t ioReadBytes = 0;
    uint64_t ioWriteBytes = 0;
    double wallTimeMs = 0.0;
};

struct ProfilerStats
{
    uint32_t snapshotsTaken = 0;
    uint64_t peakPrivateBytes = 0;
    uint64_t peakWorkingSet = 0;
    uint32_t peakHandles = 0;
    uint32_t peakGDI = 0;
    double avgCpuTimeUs = 0.0;
};

class ResourceUsageProfiler
{
  public:
    ResourceUsageProfiler()
    {
        QueryPerformanceFrequency(&m_freq);
    }
    ~ResourceUsageProfiler() = default;

    static const wchar_t* GetName()
    {
        return L"ResourceUsageProfiler";
    }

    /// Take a snapshot of current process resources.
    ProcessResourceSnapshot TakeSnapshot() const
    {
        ProcessResourceSnapshot snap;
        snap.tick = GetTickCount64();

        HANDLE hProcess = GetCurrentProcess();

        PROCESS_MEMORY_COUNTERS_EX pmc = {};
        pmc.cb = sizeof(pmc);
        if (GetProcessMemoryInfo(hProcess, reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
            snap.privateBytes = pmc.PrivateUsage;
            snap.workingSet = pmc.WorkingSetSize;
        }

        snap.handleCount = 0;
        GetProcessHandleCount(hProcess, reinterpret_cast<DWORD*>(&snap.handleCount));

        snap.gdiObjects = GetGuiResources(hProcess, GR_GDIOBJECTS);
        snap.userObjects = GetGuiResources(hProcess, GR_USEROBJECTS);

        FILETIME creation, exit, kernel, user;
        if (GetProcessTimes(hProcess, &creation, &exit, &kernel, &user)) {
            ULARGE_INTEGER k, u;
            k.LowPart = kernel.dwLowDateTime;
            k.HighPart = kernel.dwHighDateTime;
            u.LowPart = user.dwLowDateTime;
            u.HighPart = user.dwHighDateTime;
            snap.kernelTimeUs = k.QuadPart / 10;  // 100ns -> us
            snap.userTimeUs = u.QuadPart / 10;
        }

        IO_COUNTERS io = {};
        if (GetProcessIoCounters(hProcess, &io)) {
            snap.readBytes = io.ReadTransferCount;
            snap.writeBytes = io.WriteTransferCount;
        }

        // Update peaks
        if (snap.privateBytes > m_stats.peakPrivateBytes)
            m_stats.peakPrivateBytes = snap.privateBytes;
        if (snap.workingSet > m_stats.peakWorkingSet)
            m_stats.peakWorkingSet = snap.workingSet;
        if (snap.handleCount > m_stats.peakHandles)
            m_stats.peakHandles = snap.handleCount;
        if (snap.gdiObjects > m_stats.peakGDI)
            m_stats.peakGDI = snap.gdiObjects;

        m_stats.snapshotsTaken++;
        return snap;
    }

    /// Compute delta between two snapshots.
    ResourceDelta ComputeDelta(const ProcessResourceSnapshot& before, const ProcessResourceSnapshot& after) const
    {
        ResourceDelta d;
        d.privateBytesChange = static_cast<int64_t>(after.privateBytes) - static_cast<int64_t>(before.privateBytes);
        d.workingSetChange = static_cast<int64_t>(after.workingSet) - static_cast<int64_t>(before.workingSet);
        d.handleChange = static_cast<int32_t>(after.handleCount) - static_cast<int32_t>(before.handleCount);
        d.gdiChange = static_cast<int32_t>(after.gdiObjects) - static_cast<int32_t>(before.gdiObjects);
        d.cpuTimeUs = static_cast<int64_t>(after.kernelTimeUs + after.userTimeUs)
                      - static_cast<int64_t>(before.kernelTimeUs + before.userTimeUs);
        d.ioReadBytes = after.readBytes - before.readBytes;
        d.ioWriteBytes = after.writeBytes - before.writeBytes;
        d.wallTimeMs = static_cast<double>(after.tick - before.tick);
        return d;
    }

    ProfilerStats GetStats() const
    {
        return m_stats;
    }

  private:
    LARGE_INTEGER m_freq{};
    mutable ProfilerStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
