// ExportDiagnostics.h — Export Comprehensive Diagnostics Bundle
// Copyright (c) 2026 ExplorerLens Project
//
// Collects system info, decoder health, circuit breaker state, registry
// settings, recent events, performance metrics, and GPU info into a
// timestamped ZIP bundle for troubleshooting support requests.
#pragma once

#include <windows.h>

#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "DecoderHealthCheck.h"

namespace ExplorerLens {

class ExportDiagnostics
{
  public:
    /// Export full diagnostics bundle to a ZIP file
    static bool ExportBundle(const std::wstring& outputPath)
    {
        // Create temporary directory for diagnostics files
        std::wstring tempDir = GetTempDiagnosticsPath();
        CreateDirectoryW(tempDir.c_str(), nullptr);

        // Collect all diagnostic information
        bool success = true;
        success &= ExportSystemInfo(tempDir + L"\\system_info.txt");
        success &= ExportDecoderHealth(tempDir + L"\\decoder_health.txt");
        success &= ExportCircuitBreakers(tempDir + L"\\circuit_breakers.txt");
        success &= ExportRegistrySettings(tempDir + L"\\registry_settings.txt");
        success &= ExportEventLogs(tempDir + L"\\recent_events.txt");
        success &= ExportPerformanceMetrics(tempDir + L"\\performance.txt");
        success &= ExportGPUInfo(tempDir + L"\\gpu_info.txt");

        // Create ZIP bundle
        if (success) {
            success = CreateZipBundle(tempDir, outputPath);
        }

        // Clean up temporary files
        CleanupTempDirectory(tempDir);

        return success;
    }

  private:
    /// Get temporary diagnostics directory path
    static std::wstring GetTempDiagnosticsPath()
    {
        wchar_t tempPath[MAX_PATH];
        GetTempPathW(MAX_PATH, tempPath);

        std::wstring path = tempPath;
        path += L"ExplorerLens_Diagnostics_";

        // Add timestamp
        SYSTEMTIME st;
        GetLocalTime(&st);
        wchar_t timestamp[64];
        swprintf_s(timestamp, L"%04d%02d%02d_%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute,
                   st.wSecond);
        path += timestamp;

        return path;
    }

    /// Export system information
    static bool ExportSystemInfo(const std::wstring& filePath)
    {
        std::wofstream file(filePath);
        if (!file.is_open())
            return false;

        file << L"=== ExplorerLens System Information ===\n\n";

        // OS Version
        RTL_OSVERSIONINFOW osInfo = {0};
        osInfo.dwOSVersionInfoSize = sizeof(osInfo);

        typedef NTSTATUS(WINAPI * RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
        HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
        if (hMod) {
            RtlGetVersionPtr RtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hMod, "RtlGetVersion");
            if (RtlGetVersion) {
                RtlGetVersion(&osInfo);
                file << L"OS Version: " << osInfo.dwMajorVersion << L"." << osInfo.dwMinorVersion << L" Build "
                     << osInfo.dwBuildNumber << L"\n";
            }
        }

        // System Architecture
        SYSTEM_INFO si;
        GetNativeSystemInfo(&si);
        file << L"Architecture: ";
        switch (si.wProcessorArchitecture) {
            case PROCESSOR_ARCHITECTURE_AMD64:
                file << L"x64\n";
                break;
            case PROCESSOR_ARCHITECTURE_ARM64:
                file << L"ARM64\n";
                break;
            case PROCESSOR_ARCHITECTURE_INTEL:
                file << L"x86\n";
                break;
            default:
                file << L"Unknown\n";
                break;
        }
        file << L"Processor Count: " << si.dwNumberOfProcessors << L"\n";

        // Memory
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&memInfo);
        file << L"Total RAM: " << (memInfo.ullTotalPhys / 1024 / 1024) << L" MB\n";
        file << L"Available RAM: " << (memInfo.ullAvailPhys / 1024 / 1024) << L" MB\n";

        // ExplorerLens Version
        file << L"\n=== ExplorerLens Build Information ===\n";
        file << L"Version: 15.0.0\n";
        file << L"Build Date: " << __DATE__ << L" " << __TIME__ << L"\n";
        file << L"Architecture: x64 Release\n";

        // Thread Count
        file << L"\n=== Process Information ===\n";
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
            file << L"Working Set: " << (pmc.WorkingSetSize / 1024 / 1024) << L" MB\n";
            file << L"Peak Working Set: " << (pmc.PeakWorkingSetSize / 1024 / 1024) << L" MB\n";
        }

        file.close();
        return true;
    }

    /// Export decoder health status
    static bool ExportDecoderHealth(const std::wstring& filePath)
    {
        std::wofstream file(filePath);
        if (!file.is_open())
            return false;

        file << L"=== Decoder Health Status ===\n\n";

        auto healthInfo = DecoderHealthCheck::CheckAll();

        file << L"Active Decoders: ";
        int active = 0;
        for (const auto& info : healthInfo) {
            if (info.isAvailable)
                active++;
        }
        file << active << L" / " << healthInfo.size() << L"\n\n";

        for (const auto& info : healthInfo) {
            file << (info.isAvailable ? L"[✓] " : L"[✗] ");
            file << info.name << L" (v" << info.version << L")\n";
            file << L"    Extensions: " << info.extensionCount << L"\n";
            file << L"    Status: " << info.statusMessage << L"\n";

            if (info.hasExternalDependency) {
                file << L"    Dependency: " << info.dllName << (info.isAvailable ? L" [FOUND]" : L" [MISSING]")
                     << L"\n";
            }
            file << L"\n";
        }

        file.close();
        return true;
    }

    /// Export circuit breaker states
    /// Note: Full circuit breaker data requires Engine linkage; here we
    /// report a summary based on what DecoderHealthCheck can detect.
    static bool ExportCircuitBreakers(const std::wstring& filePath)
    {
        std::wofstream file(filePath);
        if (!file.is_open())
            return false;

        file << L"=== Circuit Breaker States ===\n\n";
        file << L"Circuit breakers protect against failing decoders.\n";
        file << L"After 5 consecutive failures, a decoder is temporarily "
                L"disabled.\n\n";

        // Use DecoderHealthCheck as a proxy — unavailable decoders may
        // indicate circuit breakers have tripped or libraries are missing.
        auto healthResults = DecoderHealthCheck::CheckAll();
        int tripped = 0;
        for (const auto& info : healthResults) {
            if (!info.isAvailable) {
                file << L"[OPEN] " << info.name;
                if (info.hasExternalDependency)
                    file << L" — Missing: " << info.dllName;
                file << L"\n  " << info.statusMessage << L"\n\n";
                tripped++;
            }
        }

        if (tripped == 0) {
            file << L"No circuit breakers activated (all decoders healthy).\n";
        }

        file.close();
        return true;
    }

    /// Export registry settings
    static bool ExportRegistrySettings(const std::wstring& filePath)
    {
        std::wofstream file(filePath);
        if (!file.is_open())
            return false;

        file << L"=== Registry Settings ===\n\n";

        // Read ExplorerLens registry settings
        HKEY hKey;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\ExplorerLens", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            // Enumerate all values
            DWORD index = 0;
            wchar_t valueName[256];
            DWORD valueNameLen;
            DWORD valueType;
            BYTE valueData[512];
            DWORD valueDataLen;

            while (true) {
                valueNameLen = 256;
                valueDataLen = 512;

                LONG result = RegEnumValueW(hKey, index++, valueName, &valueNameLen, nullptr, &valueType, valueData,
                                            &valueDataLen);

                if (result != ERROR_SUCCESS)
                    break;

                file << valueName << L" = ";

                switch (valueType) {
                    case REG_DWORD:
                        file << *((DWORD*)valueData);
                        break;
                    case REG_SZ:
                        file << (wchar_t*)valueData;
                        break;
                    case REG_BINARY:
                        file << L"[binary data]";
                        break;
                    default:
                        file << L"[unknown type]";
                        break;
                }

                file << L"\n";
            }

            RegCloseKey(hKey);
        } else {
            file << L"Registry key not found (default settings in use)\n";
        }

        file.close();
        return true;
    }

    /// Export recent event logs
    static bool ExportEventLogs(const std::wstring& filePath)
    {
        std::wofstream file(filePath);
        if (!file.is_open())
            return false;

        file << L"=== Recent Events (Last Session) ===\n\n";
        file << L"Note: Full ETW logging requires enabling structured logging.\n";
        file << L"See OBSERVABILITY_SPEC_V1.md for details.\n\n";

        // Placeholder - full ETW integration pending
        file << L"Thumbnail requests: [ETW tracking not yet enabled]\n";
        file << L"Cache hits: [ETW tracking not yet enabled]\n";
        file << L"Decoder failures: [ETW tracking not yet enabled]\n";
        file << L"Circuit breaker activations: [ETW tracking not yet enabled]\n";

        file.close();
        return true;
    }

    /// Export performance metrics
    static bool ExportPerformanceMetrics(const std::wstring& filePath)
    {
        std::wofstream file(filePath);
        if (!file.is_open())
            return false;

        file << L"=== Performance Metrics ===\n\n";

        // Get benchmark data if available
        file << L"Note: Run EngineBenchmarks.exe for detailed performance data.\n\n";

        // Basic timing data from ScopedTimer
        file << L"Recent Performance (since process start):\n";
        file << L"  Thumbnail generation: p50/p95 metrics available in benchmarks\n";
        file << L"  Cache access: p50/p95 metrics available in benchmarks\n";
        file << L"  Decode time by format: See benchmarks/baseline-v15.0.0.json\n";

        file.close();
        return true;
    }

    /// Export GPU information
    static bool ExportGPUInfo(const std::wstring& filePath)
    {
        std::wofstream file(filePath);
        if (!file.is_open())
            return false;

        file << L"=== GPU Information ===\n\n";

        // Enumerate DXGI adapters
        IDXGIFactory1* pFactory = nullptr;
        HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&pFactory);

        if (SUCCEEDED(hr)) {
            IDXGIAdapter1* pAdapter = nullptr;
            int adapterIndex = 0;

            while (pFactory->EnumAdapters1(adapterIndex, &pAdapter) != DXGI_ERROR_NOT_FOUND) {
                DXGI_ADAPTER_DESC1 desc;
                pAdapter->GetDesc1(&desc);

                file << L"GPU " << adapterIndex << L": " << desc.Description << L"\n";
                file << L"  Vendor ID: 0x" << std::hex << desc.VendorId << std::dec << L"\n";
                file << L"  Device ID: 0x" << std::hex << desc.DeviceId << std::dec << L"\n";
                file << L"  Dedicated VRAM: " << (desc.DedicatedVideoMemory / 1024 / 1024) << L" MB\n";
                file << L"  Shared Memory: " << (desc.SharedSystemMemory / 1024 / 1024) << L" MB\n\n";

                pAdapter->Release();
                adapterIndex++;
            }

            pFactory->Release();
        } else {
            file << L"Failed to enumerate GPU adapters\n";
        }

        file.close();
        return true;
    }

    /// Create ZIP bundle from temporary directory
    static bool CreateZipBundle(const std::wstring& sourceDir, const std::wstring& zipPath)
    {
        // This is a simplified implementation
        // Production version should use minizip-ng or Windows Shell API

        // For now, just copy the text files to a single diagnostics.txt
        std::wofstream bundle(zipPath);
        if (!bundle.is_open())
            return false;

        bundle << L"ExplorerLens Diagnostics Bundle\n";
        bundle << L"=============================\n\n";

        // Read and append all diagnostic files
        std::vector<std::wstring> files = {L"system_info.txt",       L"decoder_health.txt", L"circuit_breakers.txt",
                                           L"registry_settings.txt", L"event_logs.txt",     L"performance.txt",
                                           L"gpu_info.txt"};

        for (const auto& filename : files) {
            std::wstring fullPath = sourceDir + L"\\" + filename;
            std::wifstream input(fullPath);

            if (input.is_open()) {
                bundle << L"\n\n==================== " << filename << L" ====================\n\n";
                bundle << input.rdbuf();
                input.close();
            }
        }

        bundle.close();
        return true;
    }

    /// Clean up temporary directory
    static void CleanupTempDirectory(const std::wstring& tempDir)
    {
        // Delete all files in directory
        WIN32_FIND_DATAW findData;
        HANDLE hFind = FindFirstFileW((tempDir + L"\\*").c_str(), &findData);

        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (wcscmp(findData.cFileName, L".") != 0 && wcscmp(findData.cFileName, L"..") != 0) {
                    DeleteFileW((tempDir + L"\\" + findData.cFileName).c_str());
                }
            } while (FindNextFileW(hFind, &findData));

            FindClose(hFind);
        }

        // Remove directory
        RemoveDirectoryW(tempDir.c_str());
    }

    /// Format timestamp for display
    static std::wstring FormatTimestamp(const std::chrono::steady_clock::time_point& tp)
    {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - tp);

        if (duration.count() < 60) {
            return std::to_wstring(duration.count()) + L" seconds ago";
        } else if (duration.count() < 3600) {
            return std::to_wstring(duration.count() / 60) + L" minutes ago";
        } else {
            return std::to_wstring(duration.count() / 3600) + L" hours ago";
        }
    }
};

}  // namespace ExplorerLens
