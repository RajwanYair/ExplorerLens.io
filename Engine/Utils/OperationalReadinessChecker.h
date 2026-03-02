// OperationalReadinessChecker.h — Runtime Subsystem Health Probe
// Copyright (c) 2026 ExplorerLens Project
//
// Probes five subsystems for operational readiness: GPU availability
// (D3D11), disk I/O (free space), physical memory headroom, WIC codec
// availability, and basic memory allocation capability for cache. Each
// subsystem is graded Ready, Degraded, or Unavailable. The system is
// considered operational only when no subsystem is Unavailable.
//
// Thread-safe singleton.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class SubsystemReadiness : uint32_t {
    Ready = 0,
    Degraded = 1,
    Unavailable = 2,
    NotChecked = 3
};

struct SubsystemProbeResult {
    std::wstring         name;
    SubsystemReadiness   readiness = SubsystemReadiness::NotChecked;
    std::wstring         detail;
    uint32_t             latencyMs = 0;
};

struct OperationalReadinessReport {
    uint64_t                             timestamp = 0;
    uint32_t                             totalProbes = 0;
    uint32_t                             ready = 0;
    uint32_t                             degraded = 0;
    uint32_t                             unavailable = 0;
    bool                                 operational = false;
    std::vector<SubsystemProbeResult>    probes;
};

// ========================================================================
// OperationalReadinessChecker — Probes all subsystems for readiness
// ========================================================================
class OperationalReadinessChecker {
public:
    static OperationalReadinessChecker& Instance() {
        static OperationalReadinessChecker instance;
        return instance;
    }

    void Initialize() {
        m_totalChecks = 0;
        m_initialized = true;
    }

    bool IsInitialized() const { return m_initialized; }

    // Run all probes
    OperationalReadinessReport CheckAll() {
        OperationalReadinessReport report;
        report.timestamp = GetTickCount64();

        report.probes.push_back(ProbeGPU());
        report.probes.push_back(ProbeDiskIO());
        report.probes.push_back(ProbeMemory());
        report.probes.push_back(ProbeCodecs());
        report.probes.push_back(ProbeCache());

        // Tally
        report.totalProbes = static_cast<uint32_t>(report.probes.size());
        for (auto& p : report.probes) {
            switch (p.readiness) {
            case SubsystemReadiness::Ready:       report.ready++;       break;
            case SubsystemReadiness::Degraded:    report.degraded++;    break;
            case SubsystemReadiness::Unavailable: report.unavailable++; break;
            default: break;
            }
        }

        // Operational if no subsystems are completely unavailable
        report.operational = (report.unavailable == 0);
        m_totalChecks++;
        m_lastReport = report;
        return report;
    }

    // Check single subsystem
    SubsystemProbeResult ProbeGPU() {
        SubsystemProbeResult result;
        result.name = L"GPU";
        DWORD start = GetTickCount();

        HMODULE d3d11 = GetModuleHandleW(L"d3d11.dll");
        if (!d3d11) d3d11 = LoadLibraryExW(L"d3d11.dll", nullptr, LOAD_LIBRARY_AS_DATAFILE);

        if (d3d11) {
            result.readiness = SubsystemReadiness::Ready;
            result.detail = L"Direct3D 11 available";
        }
        else {
            result.readiness = SubsystemReadiness::Degraded;
            result.detail = L"D3D11 not available; CPU fallback active";
        }

        result.latencyMs = GetTickCount() - start;
        return result;
    }

    SubsystemProbeResult ProbeDiskIO() {
        SubsystemProbeResult result;
        result.name = L"Disk I/O";
        DWORD start = GetTickCount();

        wchar_t tempPath[MAX_PATH];
        if (GetTempPathW(MAX_PATH, tempPath)) {
            ULARGE_INTEGER freeBytes;
            if (GetDiskFreeSpaceExW(tempPath, &freeBytes, nullptr, nullptr)) {
                uint64_t freeMB = freeBytes.QuadPart / (1024 * 1024);
                if (freeMB > 50) {
                    result.readiness = SubsystemReadiness::Ready;
                    result.detail = L"Sufficient temp disk space";
                }
                else {
                    result.readiness = SubsystemReadiness::Degraded;
                    result.detail = L"Low temp disk space";
                }
            }
            else {
                result.readiness = SubsystemReadiness::Unavailable;
                result.detail = L"Cannot query disk space";
            }
        }
        else {
            result.readiness = SubsystemReadiness::Unavailable;
            result.detail = L"Cannot determine temp path";
        }

        result.latencyMs = GetTickCount() - start;
        return result;
    }

    SubsystemProbeResult ProbeMemory() {
        SubsystemProbeResult result;
        result.name = L"Memory";
        DWORD start = GetTickCount();

        MEMORYSTATUSEX memStatus = {};
        memStatus.dwLength = sizeof(memStatus);
        if (GlobalMemoryStatusEx(&memStatus)) {
            uint64_t availMB = memStatus.ullAvailPhys / (1024 * 1024);
            if (availMB > 512) {
                result.readiness = SubsystemReadiness::Ready;
                result.detail = L"Sufficient physical memory";
            }
            else if (availMB > 128) {
                result.readiness = SubsystemReadiness::Degraded;
                result.detail = L"Low physical memory - may impact performance";
            }
            else {
                result.readiness = SubsystemReadiness::Unavailable;
                result.detail = L"Critically low memory";
            }
        }
        else {
            result.readiness = SubsystemReadiness::Degraded;
            result.detail = L"Cannot query memory status";
        }

        result.latencyMs = GetTickCount() - start;
        return result;
    }

    SubsystemProbeResult ProbeCodecs() {
        SubsystemProbeResult result;
        result.name = L"Codecs";
        DWORD start = GetTickCount();

        // Check WIC availability (Windows Imaging Component)
        HMODULE wic = GetModuleHandleW(L"windowscodecs.dll");
        if (!wic) wic = LoadLibraryExW(L"windowscodecs.dll", nullptr, LOAD_LIBRARY_AS_DATAFILE);

        if (wic) {
            result.readiness = SubsystemReadiness::Ready;
            result.detail = L"WIC codecs available";
        }
        else {
            result.readiness = SubsystemReadiness::Degraded;
            result.detail = L"WIC unavailable; using built-in decoders only";
        }

        result.latencyMs = GetTickCount() - start;
        return result;
    }

    SubsystemProbeResult ProbeCache() {
        SubsystemProbeResult result;
        result.name = L"Cache";
        DWORD start = GetTickCount();

        // Cache readiness = can allocate small temp buffer
        void* testAlloc = VirtualAlloc(nullptr, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (testAlloc) {
            VirtualFree(testAlloc, 0, MEM_RELEASE);
            result.readiness = SubsystemReadiness::Ready;
            result.detail = L"Memory allocation functional";
        }
        else {
            result.readiness = SubsystemReadiness::Unavailable;
            result.detail = L"Cannot allocate memory for cache";
        }

        result.latencyMs = GetTickCount() - start;
        return result;
    }

    // Quick operational check
    bool IsOperational() {
        auto report = CheckAll();
        return report.operational;
    }

    // Stats
    uint64_t GetTotalChecks() const { return m_totalChecks; }
    OperationalReadinessReport GetLastReport() const { return m_lastReport; }

private:
    OperationalReadinessChecker() = default;

    OperationalReadinessReport m_lastReport;
    uint64_t m_totalChecks = 0;
    bool m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
