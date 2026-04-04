// PrivilegeElevationGuard.h — Minimal Privilege COM Server and UAC Controls
// Copyright (c) 2026 ExplorerLens Project
//
// Manages privilege elevation for shell extension registration and manages
// token privilege dropping. The thumbnail render path runs as a restricted
// token; only the installer/registrar path requests elevation via UAC.
//
#pragma once
#include <windows.h>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// What privilege scope is required for an operation
enum class PrivilegeScope {
    User,   // Standard user: no elevation needed
    Admin,  // Requires local administrator (UAC prompt)
    System  // SYSTEM account required (service installer)
};

struct TokenPrivilege
{
    std::wstring name;  // e.g. L"SeDebugPrivilege"
    bool enabled = false;
};

struct ElevationResult
{
    bool elevated = false;
    bool uacPrompt = false;  // True if UAC consent dialog was shown
    DWORD exitCode = 0;
    std::wstring message;
};

class PrivilegeElevationGuard
{
  public:
    // Query if the current process is running elevated
    static bool IsElevated()
    {
        HANDLE hToken = nullptr;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
            return false;
        TOKEN_ELEVATION elev = {};
        DWORD sz = sizeof(elev);
        bool elevated = false;
        if (QueryTokenInformation(hToken, TokenElevation, &elev, sizeof(elev), &sz))
            elevated = elev.TokenIsElevated != 0;
        CloseHandle(hToken);
        return elevated;
    }

    // Request elevation via ShellExecute runas (will show UAC prompt)
    static ElevationResult RequestElevation(const std::wstring& exePath, const std::wstring& args = L"",
                                            bool wait = true)
    {
        ElevationResult res;
        SHELLEXECUTEINFOW sei = {};
        sei.cbSize = sizeof(sei);
        sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_UNICODE;
        sei.lpVerb = L"runas";
        sei.lpFile = exePath.c_str();
        sei.lpParameters = args.empty() ? nullptr : args.c_str();
        sei.nShow = SW_SHOW;

        if (!ShellExecuteExW(&sei)) {
            DWORD err = GetLastError();
            if (err == ERROR_CANCELLED)
                res.message = L"User cancelled UAC prompt";
            else
                res.message = L"ShellExecuteEx failed: " + std::to_wstring(err);
            return res;
        }
        res.uacPrompt = true;
        if (wait && sei.hProcess) {
            WaitForSingleObject(sei.hProcess, 60000);
            GetExitCodeProcess(sei.hProcess, &res.exitCode);
            CloseHandle(sei.hProcess);
        }
        res.elevated = true;
        return res;
    }

    // Drop a specific privilege from the current token (permanent for this process)
    static bool DropPrivilege(const std::wstring& privName)
    {
        HANDLE hToken = nullptr;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
            return false;
        TOKEN_PRIVILEGES tp = {};
        tp.PrivilegeCount = 1;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_REMOVED;
        if (!LookupPrivilegeValueW(nullptr, privName.c_str(), &tp.Privileges[0].Luid)) {
            CloseHandle(hToken);
            return false;
        }
        bool ok = AdjustTokenPrivileges(hToken, FALSE, &tp, 0, nullptr, nullptr) && GetLastError() == ERROR_SUCCESS;
        CloseHandle(hToken);
        return ok;
    }

    // Drop all unnecessary privileges for the thumbnail render worker
    // Retains only: SeChangeNotifyPrivilege (required for COM/Shell)
    static void DropRenderWorkerPrivileges()
    {
        static const wchar_t* const KEEP[] = {L"SeChangeNotifyPrivilege", nullptr};
        static const wchar_t* const DROP[] = {L"SeDebugPrivilege",       L"SeBackupPrivilege",
                                              L"SeRestorePrivilege",     L"SeCreateSymbolicLinkPrivilege",
                                              L"SeTcbPrivilege",         L"SeAssignPrimaryTokenPrivilege",
                                              L"SeImpersonatePrivilege", nullptr};
        for (const wchar_t* const* p = DROP; *p; ++p)
            DropPrivilege(*p);
        (void)KEEP;
    }

    // Create a restricted token for the COM decode worker
    static HANDLE CreateSandboxToken()
    {
        HANDLE hToken = nullptr, hRestricted = nullptr;
        OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE | TOKEN_QUERY, &hToken);
        if (!hToken)
            return nullptr;

        // Disable no groups (keep all), deny SIDs: Administrators, Power Users
        SID_AND_ATTRIBUTES disabledSids[2] = {};
        BYTE adminSid[SECURITY_MAX_SID_SIZE], puSid[SECURITY_MAX_SID_SIZE];
        DWORD sidSz = SECURITY_MAX_SID_SIZE;
        CreateWellKnownSid(WinBuiltinAdministratorsSid, nullptr, adminSid, &sidSz);
        sidSz = SECURITY_MAX_SID_SIZE;
        CreateWellKnownSid(WinBuiltinPowerUsersSid, nullptr, puSid, &sidSz);
        disabledSids[0].Sid = adminSid;
        disabledSids[0].Attributes = 0;
        disabledSids[1].Sid = puSid;
        disabledSids[1].Attributes = 0;

        CreateRestrictedToken(hToken, DISABLE_MAX_PRIVILEGE | SANDBOX_INERT, 2, disabledSids, 0, nullptr, 0, nullptr,
                              &hRestricted);
        CloseHandle(hToken);
        return hRestricted;
    }

    // Enumerate current token privileges (for diagnostics)
    static std::vector<TokenPrivilege> EnumeratePrivileges()
    {
        std::vector<TokenPrivilege> priv;
        HANDLE hToken = nullptr;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
            return priv;
        DWORD sz = 0;
        GetTokenInformation(hToken, TokenPrivileges, nullptr, 0, &sz);
        if (!sz) {
            CloseHandle(hToken);
            return priv;
        }
        std::vector<BYTE> buf(sz);
        if (!GetTokenInformation(hToken, TokenPrivileges, buf.data(), sz, &sz)) {
            CloseHandle(hToken);
            return priv;
        }
        auto* tp = reinterpret_cast<TOKEN_PRIVILEGES*>(buf.data());
        for (DWORD i = 0; i < tp->PrivilegeCount; ++i) {
            wchar_t name[256] = {};
            DWORD nameSz = 256;
            LookupPrivilegeNameW(nullptr, &tp->Privileges[i].Luid, name, &nameSz);
            priv.push_back({name, (tp->Privileges[i].Attributes & SE_PRIVILEGE_ENABLED) != 0});
        }
        CloseHandle(hToken);
        return priv;
    }

  private:
    static BOOL QueryTokenInformation(HANDLE token, TOKEN_INFORMATION_CLASS cls, void* buf, DWORD sz, DWORD* retSz)
    {
        return ::GetTokenInformation(token, cls, buf, sz, retSz);
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
