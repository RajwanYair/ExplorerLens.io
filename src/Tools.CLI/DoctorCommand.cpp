// DoctorCommand.cpp — lens doctor Implementation
// Copyright (c) 2026 ExplorerLens Project
//
// Runs all diagnostic checks and prints a colour-annotated (via ANSI codes)
// pass/warn/fail table. Individual checks are independent; failures in one
// do not prevent remaining checks from running.
//
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include "DoctorCommand.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <algorithm>

#pragma comment(lib, "dxgi.lib")

namespace fs  = std::filesystem;

namespace ExplorerLens {
namespace CLI {

static constexpr wchar_t LENS_CLSID[]  = L"{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}";
static constexpr wchar_t LENS_DLL[]    = L"LENSShell.dll";

//==============================================================================
// Execute
//==============================================================================

int DoctorCommand::Execute(const ParsedArgs& args)
{
    if (args.HasFlag(L"--help") || args.HasFlag(L"-h")) {
        std::wcout << L"Usage: " << Usage() << L"\n\n"
                   << L"Checks performed:\n"
                   << L"  Registration    — HKCR\\CLSID\\{...} present\n"
                   << L"  GPU             — DXGI adapter enumeration\n"
                   << L"  Cache dir       — %LOCALAPPDATA%\\ExplorerLens\\ writable\n"
                   << L"  Windows version — 10 1809+ or 11 required\n"
                   << L"  DLL presence    — LENSShell.dll in PATH/adjacent\n"
                   << L"  Thumbnail svc   — Shell thumbnail service not disabled\n";
        return static_cast<int>(ExitCode::Success);
    }

    auto checks = RunChecks();

    if (args.JsonOutput()) {
        PrintJsonReport(checks);
    } else {
        PrintTextReport(checks);
    }

    bool anyFail = std::any_of(checks.begin(), checks.end(),
                               [](const DiagnosticCheck& c) {
                                   return c.status == CheckStatus::Fail;
                               });
    return anyFail ? static_cast<int>(ExitCode::GeneralError)
                   : static_cast<int>(ExitCode::Success);
}

//==============================================================================
// RunAllChecks — public API: runs all checks and returns results.
// Used by unit tests to validate the health check pipeline.
//==============================================================================

std::vector<DiagnosticCheck> DoctorCommand::RunAllChecks()
{
    auto checks = RunChecks();
    // Ensure 'message' is populated from 'detail' for unified API consumers.
    for (auto& c : checks) {
        if (c.message.empty())
            c.message = c.detail.empty() ? c.name : c.detail;
    }
    return checks;
}

//==============================================================================
// RunChecks
//==============================================================================

std::vector<DiagnosticCheck> DoctorCommand::RunChecks()
{
    return {
        CheckWindowsVersion(),
        CheckRegistration(),
        CheckDllPresence(),
        CheckGPUAvailability(),
        CheckCacheDirectory(),
        CheckDiskCacheHealth(),
        CheckCacheWatcherSupport(),
        CheckThumbnailServiceEnabled(),
    };
}

//==============================================================================
// Individual checks
//==============================================================================

DiagnosticCheck DoctorCommand::CheckWindowsVersion()
{
    DiagnosticCheck c;
    c.name = L"Windows Version";

    // Use RtlGetVersion (ntdll) to avoid versionhelpers.h (excluded by WIN32_LEAN_AND_MEAN)
    using RtlGetVersionFn = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);
    HMODULE ntdll = ::GetModuleHandleW(L"ntdll.dll");
    auto fn = ntdll ? reinterpret_cast<RtlGetVersionFn>(
                          ::GetProcAddress(ntdll, "RtlGetVersion")) : nullptr;

    RTL_OSVERSIONINFOW vi{};
    vi.dwOSVersionInfoSize = sizeof(vi);

    if (fn && fn(&vi) == 0) {
        // Windows 10 build 17763 = 1809; Windows 11 starts at 22000
        if (vi.dwMajorVersion > 10 ||
            (vi.dwMajorVersion == 10 && vi.dwBuildNumber >= 17763)) {
            c.status = CheckStatus::Pass;
            c.detail = L"Windows " + std::to_wstring(vi.dwMajorVersion) +
                       L" build " + std::to_wstring(vi.dwBuildNumber);
        } else {
            c.status = CheckStatus::Fail;
            c.detail = L"Windows " + std::to_wstring(vi.dwMajorVersion) +
                       L" build " + std::to_wstring(vi.dwBuildNumber) +
                       L" (too old)";
            c.fix    = L"Upgrade to Windows 10 1809 (build 17763) or later";
        }
    } else {
        c.status = CheckStatus::Warn;
        c.detail = L"Version check unavailable";
    }
    return c;
}

DiagnosticCheck DoctorCommand::CheckRegistration()
{
    DiagnosticCheck c;
    c.name = L"COM Registration";

    wchar_t keyPath[256];
    ::swprintf_s(keyPath, L"CLSID\\%s\\InprocServer32", LENS_CLSID);

    HKEY hKey = nullptr;
    if (::RegOpenKeyExW(HKEY_CLASSES_ROOT, keyPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        wchar_t dllPath[MAX_PATH] = {};
        DWORD sz = sizeof(dllPath);
        ::RegQueryValueExW(hKey, nullptr, nullptr, nullptr,
                           reinterpret_cast<LPBYTE>(dllPath), &sz);
        ::RegCloseKey(hKey);

        c.status = CheckStatus::Pass;
        c.detail = std::wstring(dllPath);
    } else {
        c.status = CheckStatus::Fail;
        c.detail = L"CLSID not found in HKCR";
        c.fix    = L"Run 'lens register' as Administrator (or regsvr32 LENSShell.dll)";
    }
    return c;
}

DiagnosticCheck DoctorCommand::CheckDllPresence()
{
    DiagnosticCheck c;
    c.name = L"LENSShell.dll";

    wchar_t exePath[MAX_PATH] = {};
    ::GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    fs::path sibling = fs::path(exePath).parent_path() / LENS_DLL;

    if (fs::exists(sibling)) {
        c.status = CheckStatus::Pass;
        c.detail = sibling.wstring();
    } else {
        // Try system PATH
        HMODULE h = ::LoadLibraryExW(LENS_DLL, nullptr, DONT_RESOLVE_DLL_REFERENCES);
        if (h) {
            wchar_t loaded[MAX_PATH] = {};
            ::GetModuleFileNameW(h, loaded, MAX_PATH);
            ::FreeLibrary(h);
            c.status = CheckStatus::Pass;
            c.detail = std::wstring(loaded);
        } else {
            c.status = CheckStatus::Fail;
            c.detail = L"LENSShell.dll not found adjacent to lens.exe or in PATH";
            c.fix    = L"Copy LENSShell.dll to the same directory as lens.exe";
        }
    }
    return c;
}

DiagnosticCheck DoctorCommand::CheckGPUAvailability()
{
    DiagnosticCheck c;
    c.name = L"GPU (DXGI)";

    IDXGIFactory* factory = nullptr;
    HRESULT hr = ::CreateDXGIFactory(__uuidof(IDXGIFactory),
                                      reinterpret_cast<void**>(&factory));
    if (FAILED(hr) || !factory) {
        c.status = CheckStatus::Fail;
        c.detail = L"CreateDXGIFactory failed (no GPU/DXGI?)";
        c.fix    = L"Install DirectX 11 runtime; check GPU drivers";
        return c;
    }

    IDXGIAdapter* adapter = nullptr;
    if (factory->EnumAdapters(0, &adapter) == S_OK) {
        DXGI_ADAPTER_DESC desc{};
        adapter->GetDesc(&desc);
        c.status = CheckStatus::Pass;
        c.detail = std::wstring(desc.Description) +
                   L" (VRAM " + std::to_wstring(desc.DedicatedVideoMemory / 1024 / 1024) + L" MB)";
        adapter->Release();
    } else {
        c.status = CheckStatus::Warn;
        c.detail = L"No discrete GPU found; software GDI+ fallback will be used";
    }

    factory->Release();
    return c;
}

DiagnosticCheck DoctorCommand::CheckCacheDirectory()
{
    DiagnosticCheck c;
    c.name = L"Cache Directory";

    wchar_t localAppData[MAX_PATH] = {};
    if (::GetEnvironmentVariableW(L"LOCALAPPDATA", localAppData, MAX_PATH) == 0) {
        c.status = CheckStatus::Fail;
        c.detail = L"%%LOCALAPPDATA%% not set";
        c.fix    = L"Ensure user profile environment variables are intact";
        return c;
    }

    fs::path cacheDir = fs::path(localAppData) / L"ExplorerLens" / L"ThumbnailCache";
    std::error_code ec;
    fs::create_directories(cacheDir, ec);

    if (!ec && fs::exists(cacheDir)) {
        // Test write access
        fs::path probe = cacheDir / L".write_test";
        {
            std::ofstream f(probe);
            f << "lens_doctor_probe";
        }
        if (fs::exists(probe)) {
            fs::remove(probe, ec);
            c.status = CheckStatus::Pass;
            c.detail = cacheDir.wstring();
        } else {
            c.status = CheckStatus::Fail;
            c.detail = L"Cache directory not writable: " + cacheDir.wstring();
            c.fix    = L"Grant write permission to " + cacheDir.wstring();
        }
    } else {
        c.status = CheckStatus::Fail;
        c.detail = L"Cannot create cache directory: " + cacheDir.wstring();
        c.fix    = L"Check disk space and directory permissions";
    }
    return c;
}

DiagnosticCheck DoctorCommand::CheckDiskCacheHealth()
{
    DiagnosticCheck c;
    c.name = L"Disk Cache (L2)";

    wchar_t localAppData[MAX_PATH] = {};
    if (::GetEnvironmentVariableW(L"LOCALAPPDATA", localAppData, MAX_PATH) == 0) {
        c.status = CheckStatus::Fail;
        c.detail = L"%%LOCALAPPDATA%% not set";
        return c;
    }

    fs::path cacheDir  = fs::path(localAppData) / L"ExplorerLens" / L"Cache";
    fs::path storeDir  = cacheDir;
    std::error_code ec;

    if (!fs::exists(storeDir)) {
        c.status = CheckStatus::Warn;
        c.detail = L"L2 cache dir not yet created (first run)";
        c.fix    = L"Generate at least one thumbnail to initialise the disk cache";
        return c;
    }

    // Count .tlc blob files and measure total size
    uintmax_t totalBytes = 0;
    uint32_t blobCount = 0;
    for (const auto& entry : fs::directory_iterator(storeDir, ec)) {
        if (entry.path().extension() == L".tlc") {
            totalBytes += entry.file_size(ec);
            ++blobCount;
        }
    }

    double totalMB = static_cast<double>(totalBytes) / (1024.0 * 1024.0);
    c.status = CheckStatus::Pass;
    c.detail = std::to_wstring(blobCount) + L" blobs, "
             + std::to_wstring(static_cast<int>(totalMB + 0.5)) + L" MB — "
             + storeDir.wstring();
    return c;
}

DiagnosticCheck DoctorCommand::CheckCacheWatcherSupport()
{
    DiagnosticCheck c;
    c.name = L"Cache Watcher (ReadDirChanges)";

    // Probe ReadDirectoryChangesW availability by checking kernel32 export
    HMODULE kernel32 = ::GetModuleHandleW(L"kernel32.dll");
    if (!kernel32) {
        c.status = CheckStatus::Fail;
        c.detail = L"kernel32.dll not loaded";
        return c;
    }

    auto fn = ::GetProcAddress(kernel32, "ReadDirectoryChangesW");
    if (fn) {
        c.status = CheckStatus::Pass;
        c.detail = L"ReadDirectoryChangesW available (kernel32.dll)";
    } else {
        c.status = CheckStatus::Fail;
        c.detail = L"ReadDirectoryChangesW not found — cache invalidation disabled";
        c.fix    = L"Run on Windows Vista+ (all supported OS versions have this API)";
    }
    return c;
}

DiagnosticCheck DoctorCommand::CheckThumbnailServiceEnabled(){
    DiagnosticCheck c;
    c.name = L"Thumbnail Service";

    // Check the Windows Explorer policy that can disable thumbnails globally:
    //   HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced  NoThumbnailCache = 0
    HKEY hKey = nullptr;
    DWORD noCache = 0;
    DWORD sz = sizeof(noCache);
    if (::RegOpenKeyExW(HKEY_CURRENT_USER,
                         L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced",
                         0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        ::RegQueryValueExW(hKey, L"NoThumbnailCache", nullptr, nullptr,
                           reinterpret_cast<LPBYTE>(&noCache), &sz);
        ::RegCloseKey(hKey);
    }

    if (noCache != 0) {
        c.status = CheckStatus::Fail;
        c.detail = L"NoThumbnailCache is set (thumbnails disabled in Explorer)";
        c.fix    = L"Run: reg delete HKCU\\...\\Explorer\\Advanced /v NoThumbnailCache /f";
    } else {
        c.status = CheckStatus::Pass;
        c.detail = L"Thumbnail cache enabled in Explorer";
    }
    return c;
}

//==============================================================================
// PrintTextReport
//==============================================================================

void DoctorCommand::PrintTextReport(const std::vector<DiagnosticCheck>& checks) const
{
    std::wcout << L"\nExplorerLens Doctor\n"
               << std::wstring(60, L'=') << L"\n";

    uint32_t passed = 0, warned = 0, failed = 0;
    for (const auto& c : checks) {
        const wchar_t* icon  = c.status == CheckStatus::Pass ? L"[PASS]"
                             : c.status == CheckStatus::Warn ? L"[WARN]"
                                                             : L"[FAIL]";
        std::wcout << L"  " << std::left << std::setw(7) << icon
                   << std::setw(26) << c.name
                   << L"  " << c.detail << L"\n";
        if (!c.fix.empty() && c.status != CheckStatus::Pass)
            std::wcout << L"           Fix: " << c.fix << L"\n";

        if (c.status == CheckStatus::Pass)       ++passed;
        else if (c.status == CheckStatus::Warn)  ++warned;
        else                                     ++failed;
    }

    std::wcout << std::wstring(60, L'-') << L"\n"
               << L"  " << passed << L" passed, "
               << warned << L" warnings, "
               << failed << L" failed\n\n";

    if (failed == 0 && warned == 0)
        std::wcout << L"  All checks passed.\n\n";
    else if (failed > 0)
        std::wcout << L"  Action required — see Fix hints above.\n\n";
    else
        std::wcout << L"  Minor issues detected — review warnings above.\n\n";
}

//==============================================================================
// PrintJsonReport
//==============================================================================

void DoctorCommand::PrintJsonReport(const std::vector<DiagnosticCheck>& checks) const
{
    auto statusStr = [](CheckStatus s) -> std::wstring_view {
        return s == CheckStatus::Pass ? L"pass"
             : s == CheckStatus::Warn ? L"warn" : L"fail";
    };

    std::wcout << L"{\n  \"checks\": [\n";
    for (size_t i = 0; i < checks.size(); ++i) {
        const auto& c = checks[i];
        std::wcout << L"    {\n"
                   << L"      \"name\": \""   << c.name   << L"\",\n"
                   << L"      \"status\": \"" << statusStr(c.status) << L"\",\n"
                   << L"      \"detail\": \"" << c.detail << L"\",\n"
                   << L"      \"fix\": \""    << c.fix    << L"\"\n"
                   << L"    }" << (i + 1 < checks.size() ? L"," : L"") << L"\n";
    }
    std::wcout << L"  ]\n}\n";
}

} // namespace CLI
} // namespace ExplorerLens
