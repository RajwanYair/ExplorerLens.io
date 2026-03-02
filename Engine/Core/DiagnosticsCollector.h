// DiagnosticsCollector.h — Comprehensive System Diagnostics (Sprint 588)
// Copyright (c) 2026 ExplorerLens Project
//
// Collects OS version (RtlGetVersion), CPU info (registry), RAM
// (GlobalMemoryStatusEx), GPU (DXGI enumeration), DirectX feature level,
// installed decoders, and registry-based extension registrations. Generates
// a formatted diagnostic report for support tickets. Thread-safe via SRWLOCK.
// Does NOT include <psapi.h> or <versionhelpers.h>.

#pragma once

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <dxgi.h>
#include <d3d11.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

namespace ExplorerLens {
namespace Engine {

/// System hardware and OS information.
struct SystemInfo {
    std::wstring osVersion;
    std::wstring cpuName;
    uint32_t     cpuCores      = 0;
    uint64_t     totalRAM      = 0;   // bytes
    uint64_t     availableRAM  = 0;   // bytes
    std::wstring gpuName;
    uint64_t     gpuVRAM       = 0;   // bytes
    std::wstring directxVersion;
};

/// Full diagnostic report for a support ticket.
struct DiagReport {
    SystemInfo                  system;
    std::vector<std::string>    loadedDecoders;
    std::vector<std::string>    failedDecoders;
    std::vector<std::string>    warnings;
    uint64_t                    cacheSize          = 0;  // bytes
    uint64_t                    thumbnailsGenerated = 0;
    std::wstring                version;
    std::wstring                installPath;
};

/// Collects comprehensive diagnostics for troubleshooting.
class DiagnosticsCollector {
public:
    DiagnosticsCollector() {
        InitializeSRWLock(&m_lock);
    }

    ~DiagnosticsCollector() = default;
    DiagnosticsCollector(const DiagnosticsCollector&) = delete;
    DiagnosticsCollector& operator=(const DiagnosticsCollector&) = delete;

    // ── System info collection ────────────────────────────────────────────
    /// Collect all system hardware and OS information.
    inline SystemInfo CollectSystemInfo() const {
        SystemInfo info;
        info.osVersion    = CollectOSVersion();
        info.cpuName      = CollectCPUName();
        info.cpuCores     = CollectCPUCoreCount();
        CollectMemoryInfo(info.totalRAM, info.availableRAM);
        CollectGPUInfo(info.gpuName, info.gpuVRAM);
        info.directxVersion = CollectDirectXVersion();
        return info;
    }

    // ── Full report ───────────────────────────────────────────────────────
    /// Aggregate all diagnostics into a single report.
    inline DiagReport CollectFullReport() const {
        DiagReport report;
        report.system      = CollectSystemInfo();
        report.version     = GetExplorerLensVersion();
        report.installPath = GetInstallPath();

        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        report.loadedDecoders      = m_loadedDecoders;
        report.failedDecoders      = m_failedDecoders;
        report.warnings            = m_warnings;
        report.cacheSize           = m_cacheSize;
        report.thumbnailsGenerated = m_thumbnailsGenerated;
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));

        return report;
    }

    /// Format a diagnostic report as human-readable text.
    inline std::wstring FormatReport(const DiagReport& report) const {
        std::wostringstream ss;
        ss << L"====== ExplorerLens Diagnostic Report ======\n\n";

        ss << L"Version:      " << report.version << L"\n";
        ss << L"Install Path: " << report.installPath << L"\n\n";

        ss << L"--- System ---\n";
        ss << L"OS:           " << report.system.osVersion << L"\n";
        ss << L"CPU:          " << report.system.cpuName << L"\n";
        ss << L"CPU Cores:    " << report.system.cpuCores << L"\n";
        ss << L"Total RAM:    " << FormatBytes(report.system.totalRAM) << L"\n";
        ss << L"Available RAM:" << FormatBytes(report.system.availableRAM) << L"\n";
        ss << L"GPU:          " << report.system.gpuName << L"\n";
        ss << L"GPU VRAM:     " << FormatBytes(report.system.gpuVRAM) << L"\n";
        ss << L"DirectX:      " << report.system.directxVersion << L"\n\n";

        ss << L"--- Engine ---\n";
        ss << L"Cache Size:   " << FormatBytes(report.cacheSize) << L"\n";
        ss << L"Thumbnails:   " << report.thumbnailsGenerated << L"\n\n";

        ss << L"--- Decoders (loaded: " << report.loadedDecoders.size() << L") ---\n";
        for (const auto& d : report.loadedDecoders) {
            ss << L"  [OK] ";
            for (char c : d) ss << static_cast<wchar_t>(c);
            ss << L"\n";
        }
        if (!report.failedDecoders.empty()) {
            ss << L"\n--- Failed Decoders (" << report.failedDecoders.size() << L") ---\n";
            for (const auto& d : report.failedDecoders) {
                ss << L"  [FAIL] ";
                for (char c : d) ss << static_cast<wchar_t>(c);
                ss << L"\n";
            }
        }

        if (!report.warnings.empty()) {
            ss << L"\n--- Warnings (" << report.warnings.size() << L") ---\n";
            for (const auto& w : report.warnings) {
                ss << L"  - ";
                for (char c : w) ss << static_cast<wchar_t>(c);
                ss << L"\n";
            }
        }

        ss << L"\n====== End Report ======\n";
        return ss.str();
    }

    /// Write a formatted report to a file.
    inline bool SaveReport(const std::wstring& path, const DiagReport& report) const {
        std::wstring text = FormatReport(report);
        HANDLE hFile = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr,
                                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return false;

        // Write UTF-8 BOM + content
        const uint8_t bom[] = { 0xEF, 0xBB, 0xBF };
        DWORD written = 0;
        WriteFile(hFile, bom, 3, &written, nullptr);

        int len = WideCharToMultiByte(CP_UTF8, 0, text.c_str(),
                                      static_cast<int>(text.size()),
                                      nullptr, 0, nullptr, nullptr);
        if (len > 0) {
            std::string utf8(static_cast<size_t>(len), '\0');
            WideCharToMultiByte(CP_UTF8, 0, text.c_str(),
                                static_cast<int>(text.size()),
                                utf8.data(), len, nullptr, nullptr);
            WriteFile(hFile, utf8.data(), static_cast<DWORD>(utf8.size()),
                      &written, nullptr);
        }
        CloseHandle(hFile);
        return true;
    }

    // ── Version & paths ───────────────────────────────────────────────────
    inline std::wstring GetExplorerLensVersion() const {
        return L"15.0.0";
    }

    /// Read install path from HKLM registry; fall back to module path.
    inline std::wstring GetInstallPath() const {
        std::wstring path = ReadRegistryString(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\ExplorerLens",
            L"InstallPath");

        if (path.empty()) {
            // Fallback: directory of the current module
            wchar_t buf[MAX_PATH]{};
            DWORD len = GetModuleFileNameW(nullptr, buf, MAX_PATH);
            if (len > 0) {
                path = buf;
                size_t pos = path.find_last_of(L"\\/");
                if (pos != std::wstring::npos) {
                    path = path.substr(0, pos);
                }
            }
        }
        return path;
    }

    /// Enumerate HKCR for file extensions registered with ExplorerLens COM handler.
    inline std::vector<std::wstring> GetRegisteredExtensions() const {
        std::vector<std::wstring> extensions;
        const std::wstring clsid = L"{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}";

        HKEY hKey = nullptr;
        if (RegOpenKeyExW(HKEY_CLASSES_ROOT, nullptr, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
            return extensions;
        }

        wchar_t name[256]{};
        DWORD nameLen = 256;
        DWORD index = 0;

        while (RegEnumKeyExW(hKey, index++, name, &nameLen,
                             nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
            if (name[0] == L'.') {
                // Check if this extension has our ShellEx ThumbnailProvider
                std::wstring subkeyPath = std::wstring(name) +
                    L"\\ShellEx\\{E357FCCD-A995-4576-B01F-234630154E96}";
                HKEY hSub = nullptr;
                if (RegOpenKeyExW(HKEY_CLASSES_ROOT, subkeyPath.c_str(),
                                  0, KEY_READ, &hSub) == ERROR_SUCCESS) {
                    wchar_t val[128]{};
                    DWORD valSize = sizeof(val);
                    DWORD type = 0;
                    if (RegQueryValueExW(hSub, nullptr, nullptr, &type,
                                         reinterpret_cast<LPBYTE>(val), &valSize) == ERROR_SUCCESS) {
                        if (type == REG_SZ && std::wstring(val) == clsid) {
                            extensions.push_back(name);
                        }
                    }
                    RegCloseKey(hSub);
                }
            }
            nameLen = 256;
        }
        RegCloseKey(hKey);
        return extensions;
    }

    // ── Decoder tracking (called by engine during initialization) ─────────
    inline void AddLoadedDecoder(const std::string& name) {
        AcquireSRWLockExclusive(&m_lock);
        m_loadedDecoders.push_back(name);
        ReleaseSRWLockExclusive(&m_lock);
    }

    inline void AddFailedDecoder(const std::string& name) {
        AcquireSRWLockExclusive(&m_lock);
        m_failedDecoders.push_back(name);
        ReleaseSRWLockExclusive(&m_lock);
    }

    inline void AddWarning(const std::string& warning) {
        AcquireSRWLockExclusive(&m_lock);
        m_warnings.push_back(warning);
        ReleaseSRWLockExclusive(&m_lock);
    }

    inline void SetCacheSize(uint64_t bytes) {
        AcquireSRWLockExclusive(&m_lock);
        m_cacheSize = bytes;
        ReleaseSRWLockExclusive(&m_lock);
    }

    inline void SetThumbnailsGenerated(uint64_t count) {
        AcquireSRWLockExclusive(&m_lock);
        m_thumbnailsGenerated = count;
        ReleaseSRWLockExclusive(&m_lock);
    }

private:
    // ── OS version via RtlGetVersion ──────────────────────────────────────
    static std::wstring CollectOSVersion() {
        // Dynamically load RtlGetVersion from ntdll.dll to avoid <versionhelpers.h>
        using RtlGetVersionFn = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);

        HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
        if (!ntdll) return L"Unknown";

        auto fn = reinterpret_cast<RtlGetVersionFn>(
            GetProcAddress(ntdll, "RtlGetVersion"));
        if (!fn) return L"Unknown";

        RTL_OSVERSIONINFOW osvi{};
        osvi.dwOSVersionInfoSize = sizeof(osvi);
        if (fn(&osvi) != 0) return L"Unknown";

        std::wostringstream ss;
        ss << L"Windows " << osvi.dwMajorVersion << L"."
           << osvi.dwMinorVersion << L" (Build "
           << osvi.dwBuildNumber << L")";
        return ss.str();
    }

    // ── CPU name from registry ────────────────────────────────────────────
    static std::wstring CollectCPUName() {
        return ReadRegistryString(
            HKEY_LOCAL_MACHINE,
            L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
            L"ProcessorNameString");
    }

    static uint32_t CollectCPUCoreCount() {
        SYSTEM_INFO si{};
        GetSystemInfo(&si);
        return static_cast<uint32_t>(si.dwNumberOfProcessors);
    }

    // ── RAM ───────────────────────────────────────────────────────────────
    static void CollectMemoryInfo(uint64_t& total, uint64_t& available) {
        MEMORYSTATUSEX memStatus{};
        memStatus.dwLength = sizeof(memStatus);
        if (GlobalMemoryStatusEx(&memStatus)) {
            total     = memStatus.ullTotalPhys;
            available = memStatus.ullAvailPhys;
        }
    }

    // ── GPU via DXGI ──────────────────────────────────────────────────────
    static void CollectGPUInfo(std::wstring& name, uint64_t& vram) {
        IDXGIFactory1* factory = nullptr;
        HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1),
                                        reinterpret_cast<void**>(&factory));
        if (FAILED(hr) || !factory) {
            name = L"Unknown";
            return;
        }

        IDXGIAdapter1* adapter = nullptr;
        if (factory->EnumAdapters1(0, &adapter) == S_OK && adapter) {
            DXGI_ADAPTER_DESC1 desc{};
            adapter->GetDesc1(&desc);

            // Skip software adapters
            if (!(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) {
                name = desc.Description;
                vram = desc.DedicatedVideoMemory;
            } else {
                // Try next adapter
                IDXGIAdapter1* adapter2 = nullptr;
                if (factory->EnumAdapters1(1, &adapter2) == S_OK && adapter2) {
                    adapter2->GetDesc1(&desc);
                    name = desc.Description;
                    vram = desc.DedicatedVideoMemory;
                    adapter2->Release();
                } else {
                    name = desc.Description;
                    vram = desc.DedicatedVideoMemory;
                }
            }
            adapter->Release();
        } else {
            name = L"No GPU adapter found";
        }
        factory->Release();
    }

    // ── DirectX feature level ─────────────────────────────────────────────
    static std::wstring CollectDirectXVersion() {
        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };

        D3D_FEATURE_LEVEL obtainedLevel = D3D_FEATURE_LEVEL_10_0;
        ID3D11Device* device = nullptr;

        HRESULT hr = D3D11CreateDevice(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
            0, featureLevels, _countof(featureLevels),
            D3D11_SDK_VERSION, &device, &obtainedLevel, nullptr);

        if (FAILED(hr)) {
            // Try WARP fallback
            hr = D3D11CreateDevice(
                nullptr, D3D_DRIVER_TYPE_WARP, nullptr,
                0, featureLevels, _countof(featureLevels),
                D3D11_SDK_VERSION, &device, &obtainedLevel, nullptr);
        }

        std::wstring result;
        if (SUCCEEDED(hr) && device) {
            switch (obtainedLevel) {
            case D3D_FEATURE_LEVEL_12_1: result = L"DirectX 12.1"; break;
            case D3D_FEATURE_LEVEL_12_0: result = L"DirectX 12.0"; break;
            case D3D_FEATURE_LEVEL_11_1: result = L"DirectX 11.1"; break;
            case D3D_FEATURE_LEVEL_11_0: result = L"DirectX 11.0"; break;
            case D3D_FEATURE_LEVEL_10_1: result = L"DirectX 10.1"; break;
            case D3D_FEATURE_LEVEL_10_0: result = L"DirectX 10.0"; break;
            default:                     result = L"Unknown";       break;
            }
            device->Release();
        } else {
            result = L"Not available";
        }
        return result;
    }

    // ── Registry helper ───────────────────────────────────────────────────
    static std::wstring ReadRegistryString(HKEY root,
                                           const wchar_t* subkey,
                                           const wchar_t* valueName) {
        HKEY hKey = nullptr;
        if (RegOpenKeyExW(root, subkey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
            return {};
        }

        wchar_t buffer[512]{};
        DWORD bufSize = sizeof(buffer);
        DWORD type = 0;
        std::wstring result;

        if (RegQueryValueExW(hKey, valueName, nullptr, &type,
                             reinterpret_cast<LPBYTE>(buffer),
                             &bufSize) == ERROR_SUCCESS && type == REG_SZ) {
            result = buffer;
        }
        RegCloseKey(hKey);
        return result;
    }

    // ── Formatting helper ─────────────────────────────────────────────────
    static std::wstring FormatBytes(uint64_t bytes) {
        std::wostringstream ss;
        if (bytes >= 1073741824ULL) {
            ss << std::fixed << std::setprecision(1)
               << (static_cast<double>(bytes) / 1073741824.0) << L" GB";
        } else if (bytes >= 1048576ULL) {
            ss << std::fixed << std::setprecision(1)
               << (static_cast<double>(bytes) / 1048576.0) << L" MB";
        } else if (bytes >= 1024ULL) {
            ss << (bytes / 1024) << L" KB";
        } else {
            ss << bytes << L" bytes";
        }
        return ss.str();
    }

    // ── State ─────────────────────────────────────────────────────────────
    mutable SRWLOCK m_lock = SRWLOCK_INIT;

    std::vector<std::string> m_loadedDecoders;
    std::vector<std::string> m_failedDecoders;
    std::vector<std::string> m_warnings;
    uint64_t                 m_cacheSize          = 0;
    uint64_t                 m_thumbnailsGenerated = 0;
};

} // namespace Engine
} // namespace ExplorerLens
