// ACLManager.h — Cache and Log File Access Control Enforcement
// Copyright (c) 2026 ExplorerLens Project
//
// Applies restrictive ACLs to sensitive files (cache database, audit log, config).
// Removes world-readable permissions and locks ownership to the current user only.
// Uses the Windows DACL APIs: SetFileSecurity / BuildExplicitAccessWithName / SetEntriesInAcl.
//
#pragma once
#include <aclapi.h>
#include <sddl.h>
#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>

#pragma comment(lib, "advapi32.lib")

namespace ExplorerLens {
namespace Engine {

enum class ACLTarget {
    CacheDatabase,  // PersistentDiskCache SQLite file
    AuditLog,       // AuditLogger JSONL file
    Config,         // User configuration YAML/JSON
    PluginDir,      // Plugin installation directory
    Custom          // Caller-supplied path
};

struct ACLVerifyResult
{
    bool ownerIsCurrentUser = false;
    bool noWorldAccess = false;  // True = Everyone/World SID not present with write/exec
    bool daclPresent = false;
    DWORD aceCount = 0;
    std::wstring ownerName;
    std::wstring message;
};

class ACLManager
{
  public:
    // Restrict a file/directory to the current user only (Full Control).
    // Removes all existing ACEs and re-applies a minimal DACL.
    static bool LockToCurrentUser(const std::wstring& path)
    {
        std::wstring user = GetCurrentUserSID();
        if (user.empty())
            return false;

        EXPLICIT_ACCESS_W ea = {};
        ea.grfAccessPermissions = GENERIC_ALL;
        ea.grfAccessMode = SET_ACCESS;
        ea.grfInheritance = NO_INHERITANCE;
        BuildTrusteeWithNameW(&ea.Trustee, const_cast<LPWSTR>(user.c_str()));

        PACL pNewDACL = nullptr;
        DWORD err = SetEntriesInAclW(1, &ea, nullptr, &pNewDACL);
        if (err != ERROR_SUCCESS)
            return false;

        SECURITY_DESCRIPTOR sd = {};
        InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(&sd, TRUE, pNewDACL, FALSE);

        bool ok = SetFileSecurityW(path.c_str(), DACL_SECURITY_INFORMATION, &sd) != 0;
        LocalFree(pNewDACL);
        return ok;
    }

    // Remove explicit write/execute permissions for the Everyone (World) SID
    static bool RemoveWorldAccess(const std::wstring& path)
    {
        // Read existing DACL
        PACL pOldDACL = nullptr;
        PSECURITY_DESCRIPTOR pSD = nullptr;
        if (GetNamedSecurityInfoW(path.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr, nullptr, &pOldDACL,
                                  nullptr, &pSD)
            != ERROR_SUCCESS)
            return false;

        // Build deny-all ACE for Everyone
        PSID pWorldSid = nullptr;
        SID_IDENTIFIER_AUTHORITY worldAuth = SECURITY_WORLD_SID_AUTHORITY;
        AllocateAndInitializeSid(&worldAuth, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pWorldSid);

        EXPLICIT_ACCESS_W deny = {};
        deny.grfAccessPermissions =
            GENERIC_WRITE | GENERIC_EXECUTE | FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | FILE_APPEND_DATA;
        deny.grfAccessMode = DENY_ACCESS;
        deny.grfInheritance = NO_INHERITANCE;
        BuildTrusteeWithSidW(&deny.Trustee, pWorldSid);

        PACL pNewDACL = nullptr;
        DWORD err = SetEntriesInAclW(1, &deny, pOldDACL, &pNewDACL);
        if (err == ERROR_SUCCESS) {
            SECURITY_DESCRIPTOR sd = {};
            InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
            SetSecurityDescriptorDacl(&sd, TRUE, pNewDACL, FALSE);
            SetFileSecurityW(path.c_str(), DACL_SECURITY_INFORMATION, &sd);
            LocalFree(pNewDACL);
        }
        FreeSid(pWorldSid);
        LocalFree(pSD);
        return (err == ERROR_SUCCESS);
    }

    // Apply matching policy for well-known targets
    static bool ApplyDefaultPolicy(ACLTarget target, const std::wstring& path)
    {
        switch (target) {
            case ACLTarget::AuditLog:
                return LockToCurrentUser(path);
            case ACLTarget::CacheDatabase:
                return RemoveWorldAccess(path);
            case ACLTarget::Config:
                return RemoveWorldAccess(path);
            case ACLTarget::PluginDir:
                return RemoveWorldAccess(path);
            default:
                return RemoveWorldAccess(path);
        }
    }

    // Verify current ACL state of a path
    static ACLVerifyResult Verify(const std::wstring& path)
    {
        ACLVerifyResult res;
        PSID pOwner = nullptr;
        PACL pDACL = nullptr;
        PSECURITY_DESCRIPTOR pSD = nullptr;

        if (GetNamedSecurityInfoW(path.c_str(), SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
                                  &pOwner, nullptr, &pDACL, nullptr, &pSD)
            != ERROR_SUCCESS) {
            res.message = L"GetNamedSecurityInfo failed";
            return res;
        }

        // Owner name
        wchar_t name[256] = {}, domain[256] = {};
        DWORD nSz = 256, dSz = 256;
        SID_NAME_USE use;
        if (LookupAccountSidW(nullptr, pOwner, name, &nSz, domain, &dSz, &use))
            res.ownerName = std::wstring(domain) + L"\\" + name;

        // Check owner matches current user
        HANDLE hToken = nullptr;
        PSID pCurrentUser = nullptr;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
            DWORD sz = 0;
            GetTokenInformation(hToken, TokenUser, nullptr, 0, &sz);
            if (sz) {
                std::vector<BYTE> buf(sz);
                if (GetTokenInformation(hToken, TokenUser, buf.data(), sz, &sz)) {
                    pCurrentUser = reinterpret_cast<TOKEN_USER*>(buf.data())->User.Sid;
                    res.ownerIsCurrentUser = EqualSid(pOwner, pCurrentUser) != 0;
                }
            }
            CloseHandle(hToken);
        }

        // Check DACL
        if (pDACL) {
            res.daclPresent = true;
            ACL_SIZE_INFORMATION asi = {};
            if (GetAclInformation(pDACL, &asi, sizeof(asi), AclSizeInformation))
                res.aceCount = asi.AceCount;

            // Check for world SID with write
            PSID pWorldSid = nullptr;
            SID_IDENTIFIER_AUTHORITY worldAuth = SECURITY_WORLD_SID_AUTHORITY;
            AllocateAndInitializeSid(&worldAuth, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pWorldSid);

            res.noWorldAccess = true;
            for (DWORD i = 0; i < res.aceCount; ++i) {
                void* ace = nullptr;
                if (!GetAce(pDACL, i, &ace))
                    continue;
                auto* hdr = static_cast<ACE_HEADER*>(ace);
                if (hdr->AceType == ACCESS_ALLOWED_ACE_TYPE) {
                    auto* allowed = static_cast<ACCESS_ALLOWED_ACE*>(ace);
                    PSID sid = reinterpret_cast<PSID>(&allowed->SidStart);
                    if (EqualSid(sid, pWorldSid) && (allowed->Mask & (GENERIC_WRITE | FILE_WRITE_DATA))) {
                        res.noWorldAccess = false;
                    }
                }
            }
            if (pWorldSid)
                FreeSid(pWorldSid);
        }
        LocalFree(pSD);
        return res;
    }

  private:
    static std::wstring GetCurrentUserSID()
    {
        HANDLE hToken = nullptr;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
            return {};
        DWORD sz = 0;
        GetTokenInformation(hToken, TokenUser, nullptr, 0, &sz);
        if (!sz) {
            CloseHandle(hToken);
            return {};
        }
        std::vector<BYTE> buf(sz);
        if (!GetTokenInformation(hToken, TokenUser, buf.data(), sz, &sz)) {
            CloseHandle(hToken);
            return {};
        }
        PSID sid = reinterpret_cast<TOKEN_USER*>(buf.data())->User.Sid;
        LPWSTR sidStr = nullptr;
        ConvertSidToStringSidW(sid, &sidStr);
        std::wstring result = sidStr ? sidStr : L"";
        if (sidStr)
            LocalFree(sidStr);
        CloseHandle(hToken);
        return result;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
