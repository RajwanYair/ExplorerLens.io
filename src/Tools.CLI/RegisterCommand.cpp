// RegisterCommand.cpp — COM Shell Extension Registration Implementation
// Copyright (c) 2026 ExplorerLens Project
//
// Wraps regsvr32/regsvr32 /u for registration. Detects admin elevation status
// without requiring admin. Reads HKCR CLSID to determine registration state.
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shlwapi.h>
#include "RegisterCommand.h"
#include <iostream>
#include <filesystem>

#pragma comment(lib, "shlwapi.lib")

namespace fs = std::filesystem;

namespace ExplorerLens {
namespace CLI {

// CLSID for ExplorerLens IThumbnailProvider
static constexpr wchar_t LENS_CLSID[] =
    L"{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}";

//==============================================================================
// Helpers
//==============================================================================

bool RegisterCommand::IsAdminProcess() noexcept
{
    BOOL isAdmin = FALSE;
    PSID adminGroup = nullptr;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    if (::AllocateAndInitializeSid(&ntAuthority, 2,
                                    SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_ADMINS,
                                    0, 0, 0, 0, 0, 0, &adminGroup))
    {
        ::CheckTokenMembership(nullptr, adminGroup, &isAdmin);
        ::FreeSid(adminGroup);
    }
    return isAdmin != FALSE;
}

bool RegisterCommand::IsRegistered() noexcept
{
    wchar_t keyPath[256];
    ::swprintf_s(keyPath, L"CLSID\\%s\\InprocServer32", LENS_CLSID);

    HKEY hKey = nullptr;
    LSTATUS st = ::RegOpenKeyExW(HKEY_CLASSES_ROOT, keyPath, 0, KEY_READ, &hKey);
    if (st == ERROR_SUCCESS) {
        ::RegCloseKey(hKey);
        return true;
    }
    return false;
}

std::wstring RegisterCommand::FindDll(const std::wstring& explicitPath)
{
    if (!explicitPath.empty() && fs::exists(explicitPath))
        return explicitPath;

    // Check next to lens.exe
    wchar_t exePath[MAX_PATH] = {};
    ::GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    fs::path sibling = fs::path(exePath).parent_path() / L"LENSShell.dll";
    if (fs::exists(sibling)) return sibling.wstring();

    return L"";
}

//==============================================================================
// Execute
//==============================================================================

int RegisterCommand::Execute(const ParsedArgs& args)
{
    if (args.HasFlag(L"--help") || args.HasFlag(L"-h")) {
        std::wcout << L"Usage: " << Usage() << L"\n\n"
                   << L"Options:\n"
                   << L"  --status       Show registration status (no admin required)\n"
                   << L"  --dll <path>   Explicit path to LENSShell.dll\n"
                   << L"  --verbose, -v  Verbose output\n\n"
                   << L"Note: Registration and unregistration require administrator privileges.\n";
        return static_cast<int>(ExitCode::Success);
    }

    if (args.HasFlag(L"--status")) return DoStatus();

    const std::wstring dllPath = FindDll(args.GetOption(L"--dll"));
    return DoRegister(dllPath, args.Verbose());
}

int RegisterCommand::DoStatus()
{
    const bool registered = IsRegistered();
    const bool isAdmin    = IsAdminProcess();

    std::wcout << L"\nExplorerLens Registration Status\n"
               << L"  " << std::wstring(40, L'-') << L"\n"
               << L"  CLSID        : " << LENS_CLSID << L"\n"
               << L"  Registered   : " << (registered ? L"YES" : L"NO") << L"\n"
               << L"  Running as   : " << (isAdmin ? L"Administrator" : L"Standard User") << L"\n\n";

    if (!registered) {
        std::wcout << L"  To register, run as Administrator:\n"
                   << L"    lens register\n\n";
    }
    return static_cast<int>(ExitCode::Success);
}

int RegisterCommand::DoRegister(const std::wstring& dllPath, bool verbose)
{
    if (!IsAdminProcess()) {
        std::wcerr << L"lens register: administrator privileges required.\n"
                   << L"  Run 'lens register --status' to check registration state.\n"
                   << L"  Re-run lens.exe from an elevated command prompt.\n";
        return static_cast<int>(ExitCode::AdminRequired);
    }

    if (dllPath.empty()) {
        std::wcerr << L"lens register: LENSShell.dll not found.\n"
                   << L"  Use --dll <path> to specify the DLL location.\n";
        return static_cast<int>(ExitCode::FileNotFound);
    }

    if (verbose)
        std::wcout << L"[register] dll: " << dllPath << L"\n";

    // Invoke regsvr32 /s (silent) to register the DLL
    std::wstring cmd = L"regsvr32 /s \"" + dllPath + L"\"";
    int result = _wsystem(cmd.c_str());

    if (result != 0) {
        std::wcerr << L"lens register: regsvr32 failed (exit=" << result << L")\n";
        return static_cast<int>(ExitCode::GeneralError);
    }

    std::wcout << L"Registered: " << dllPath << L"\n"
               << L"CLSID: " << LENS_CLSID << L"\n";
    return static_cast<int>(ExitCode::Success);
}

int RegisterCommand::DoUnregister(const std::wstring& dllPath, bool verbose)
{
    if (!IsAdminProcess()) {
        std::wcerr << L"lens unregister: administrator privileges required.\n";
        return static_cast<int>(ExitCode::AdminRequired);
    }

    if (dllPath.empty()) {
        std::wcerr << L"lens unregister: LENSShell.dll not found.\n"
                   << L"  Use --dll <path> to specify the DLL location.\n";
        return static_cast<int>(ExitCode::FileNotFound);
    }

    if (verbose)
        std::wcout << L"[unregister] dll: " << dllPath << L"\n";

    std::wstring cmd = L"regsvr32 /u /s \"" + dllPath + L"\"";
    int result = _wsystem(cmd.c_str());

    if (result != 0) {
        std::wcerr << L"lens unregister: regsvr32 /u failed (exit=" << result << L")\n";
        return static_cast<int>(ExitCode::GeneralError);
    }

    std::wcout << L"Unregistered: " << dllPath << L"\n";
    return static_cast<int>(ExitCode::Success);
}

//==============================================================================
// UnregisterCommand
//==============================================================================

int UnregisterCommand::Execute(const ParsedArgs& args)
{
    // Delegate to RegisterCommand::DoUnregister via a temporary instance
    RegisterCommand reg;
    // We need the same FindDll and DoUnregister logic — reuse via composition
    ParsedArgs modArgs = args;
    // Redirect to register command with unregister intent via internal call
    (void)reg;   // suppress unused warning — DoUnregister called directly below

    if (args.HasFlag(L"--help") || args.HasFlag(L"-h")) {
        std::wcout << L"Usage: " << Usage() << L"\n\n"
                   << L"  --dll <path>  Explicit path to LENSShell.dll\n";
        return static_cast<int>(ExitCode::Success);
    }

    // IsAdmin check
    BOOL isAdmin = FALSE;
    PSID adminGroup = nullptr;
    SID_IDENTIFIER_AUTHORITY nt = SECURITY_NT_AUTHORITY;
    if (::AllocateAndInitializeSid(&nt, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_ADMINS,
                                    0, 0, 0, 0, 0, 0, &adminGroup)) {
        ::CheckTokenMembership(nullptr, adminGroup, &isAdmin);
        ::FreeSid(adminGroup);
    }
    if (!isAdmin) {
        std::wcerr << L"lens unregister: administrator privileges required.\n";
        return static_cast<int>(ExitCode::AdminRequired);
    }

    // Locate DLL
    std::wstring dllPath;
    std::wstring explicitDll = args.GetOption(L"--dll");
    if (!explicitDll.empty() && fs::exists(explicitDll)) {
        dllPath = explicitDll;
    } else {
        wchar_t exePath[MAX_PATH] = {};
        ::GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        fs::path sibling = fs::path(exePath).parent_path() / L"LENSShell.dll";
        if (fs::exists(sibling)) dllPath = sibling.wstring();
    }

    if (dllPath.empty()) {
        std::wcerr << L"lens unregister: LENSShell.dll not found.\n";
        return static_cast<int>(ExitCode::FileNotFound);
    }

    if (args.Verbose())
        std::wcout << L"[unregister] dll: " << dllPath << L"\n";

    std::wstring cmd = L"regsvr32 /u /s \"" + dllPath + L"\"";
    int result = _wsystem(cmd.c_str());
    if (result != 0) {
        std::wcerr << L"lens unregister: regsvr32 /u failed (exit=" << result << L")\n";
        return static_cast<int>(ExitCode::GeneralError);
    }
    std::wcout << L"Unregistered: " << dllPath << L"\n";
    return static_cast<int>(ExitCode::Success);
}

} // namespace CLI
} // namespace ExplorerLens
