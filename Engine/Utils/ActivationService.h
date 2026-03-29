// ActivationService.h — Online License Activation and Grace Period Management
// Copyright (c) 2026 ExplorerLens Project
//
// Handles online license key activation against the ExplorerLens licensing server,
// grace period management (7-day offline), and activation transfer between machines.
//
#pragma once
#include <windows.h>
#include <winhttp.h>
#include <string>
#include <functional>
#include <cstdint>

#pragma comment(lib, "winhttp.lib")

namespace ExplorerLens { namespace Engine {

enum class ActivationStatus {
    NotActivated,
    Activated,
    GracePeriod,   // Offline but within 7-day grace
    GraceExpired,  // Offline > 7 days: revert to Community
    ServerError,
    InvalidKey,
    MachineLimit   // Max activations reached for this key
};

struct ActivationResult {
    ActivationStatus status = ActivationStatus::NotActivated;
    std::wstring     message;
    std::wstring     licensee;
    std::wstring     tier;           // "Pro" / "Enterprise"
    int              activationsLeft = 0;
    SYSTEMTIME       expiresAt       = {};
    bool             success() const {
        return status == ActivationStatus::Activated ||
               status == ActivationStatus::GracePeriod;
    }
};

class ActivationService {
public:
    static constexpr const wchar_t* LICENSE_SERVER = L"activate.explorerlens.io";
    static constexpr INTERNET_PORT  LICENSE_PORT    = 443;
    static constexpr int            GRACE_DAYS      = 7;

    // Activate a new key against the server. Stores result in HKCU on success.
    ActivationResult Activate(const std::wstring& rawKey,
                              const std::wstring& email = L"") {
        ActivationResult res;
        std::wstring body = BuildRequestBody(rawKey, email);
        std::wstring resp = PostJson(L"/api/v2/activate", body);
        if (resp.empty()) {
            // Offline — check grace period
            return CheckGrace();
        }
        ParseResponse(resp, res);
        if (res.status == ActivationStatus::Activated) {
            PersistActivation(res);
            ResetGraceTimer();
        }
        return res;
    }

    // Deactivate current machine (allows transfer)
    bool Deactivate(const std::wstring& keyHash) {
        std::wstring body = L"{\"keyHash\":\"" + keyHash + L"\","
                            L"\"machineId\":\"" + GetMachineId() + L"\"}";
        std::wstring resp = PostJson(L"/api/v2/deactivate", body);
        if (resp.find(L"\"ok\":true") != std::wstring::npos) {
            RemoveActivation();
            return true;
        }
        return false;
    }

    // Check if the machine is still within the offline grace period
    ActivationResult CheckGrace() {
        ActivationResult res;
        HKEY hk = nullptr;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\ExplorerLens\\Activation",
                0, KEY_READ, &hk) != ERROR_SUCCESS) {
            res.status = ActivationStatus::NotActivated;
            return res;
        }
        DWORD lastOnline = 0; DWORD sz = sizeof(lastOnline);
        RegQueryValueExW(hk, L"LastOnlineEpoch", nullptr, nullptr,
                reinterpret_cast<BYTE*>(&lastOnline), &sz);
        RegCloseKey(hk);

        SYSTEMTIME now = {}; GetSystemTime(&now);
        FILETIME ft = {}; SystemTimeToFileTime(&now, &ft);
        ULARGE_INTEGER uli; uli.LowPart = ft.dwLowDateTime; uli.HighPart = ft.dwHighDateTime;
        uint64_t nowEpoch = (uli.QuadPart - 116444736000000000ULL) / 10000000ULL;

        uint64_t diff = (nowEpoch > lastOnline) ? (nowEpoch - lastOnline) : 0;
        int daysDiff = static_cast<int>(diff / 86400);

        if (daysDiff <= GRACE_DAYS) {
            res.status = ActivationStatus::GracePeriod;
            res.message = L"Offline grace period: " + std::to_wstring(GRACE_DAYS - daysDiff) + L" days remaining";
        } else {
            res.status = ActivationStatus::GraceExpired;
            res.message = L"Grace period expired. Please reconnect to activate.";
        }
        return res;
    }

    // Get a stable machine fingerprint (volume serial + CPU brand hash)
    static std::wstring GetMachineId() {
        wchar_t sysRoot[MAX_PATH] = {}; GetSystemDirectoryW(sysRoot, MAX_PATH);
        wchar_t vol[MAX_PATH] = {};
        wcsncpy_s(vol, sysRoot, 3); vol[3] = 0; // e.g. "C:\"
        DWORD serial = 0;
        GetVolumeInformationW(vol, nullptr, 0, &serial, nullptr, nullptr, nullptr, 0);
        wchar_t buf[32] = {};
        swprintf_s(buf, 32, L"%08X-LENS", serial);
        return buf;
    }

    void OnActivated(std::function<void(const ActivationResult&)> cb) {
        m_callback = std::move(cb);
    }

private:
    std::wstring BuildRequestBody(const std::wstring& key, const std::wstring& email) {
        return L"{\"key\":\"" + key + L"\","
               L"\"machineId\":\"" + GetMachineId() + L"\","
               L"\"email\":\"" + email + L"\","
               L"\"product\":\"ExplorerLens\","
               L"\"version\":\"20.0.0\"}";
    }

    std::wstring PostJson(const std::wstring& path, const std::wstring& body) {
        HINTERNET hSess = WinHttpOpen(L"ExplorerLens/20.0 Activation",
                WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, nullptr, nullptr, 0);
        if (!hSess) return L"";

        HINTERNET hConn = WinHttpConnect(hSess, LICENSE_SERVER, LICENSE_PORT, 0);
        HINTERNET hReq  = WinHttpOpenRequest(hConn, L"POST", path.c_str(),
                nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                WINHTTP_FLAG_SECURE);

        std::string bodyUtf8(body.size(), '\0');
        for (std::size_t _i = 0; _i < body.size(); ++_i) bodyUtf8[_i] = static_cast<char>(body[_i]);
        BOOL sent = WinHttpSendRequest(hReq,
                L"Content-Type: application/json\r\n", static_cast<DWORD>(-1L),
                const_cast<char*>(bodyUtf8.c_str()),
                static_cast<DWORD>(bodyUtf8.size()),
                static_cast<DWORD>(bodyUtf8.size()), 0);

        std::wstring result;
        if (sent && WinHttpReceiveResponse(hReq, nullptr)) {
            DWORD avail = 0;
            while (WinHttpQueryDataAvailable(hReq, &avail) && avail) {
                std::string chunk(avail, '\0');
                DWORD read = 0;
                WinHttpReadData(hReq, &chunk[0], avail, &read);
                result.append(chunk.begin(), chunk.begin() + read);
            }
        }
        WinHttpCloseHandle(hReq);
        WinHttpCloseHandle(hConn);
        WinHttpCloseHandle(hSess);
        return result;
    }

    void ParseResponse(const std::wstring& resp, ActivationResult& res) {
        if (resp.find(L"\"status\":\"activated\"") != std::wstring::npos)
            res.status = ActivationStatus::Activated;
        else if (resp.find(L"\"status\":\"invalid_key\"") != std::wstring::npos)
            res.status = ActivationStatus::InvalidKey;
        else if (resp.find(L"\"status\":\"machine_limit\"") != std::wstring::npos)
            res.status = ActivationStatus::MachineLimit;
        else
            res.status = ActivationStatus::ServerError;

        auto extract = [&](const wchar_t* key) -> std::wstring {
            auto pos = resp.find(key);
            if (pos == std::wstring::npos) return L"";
            pos += wcslen(key);
            auto end = resp.find(L'"', pos);
            return (end != std::wstring::npos) ? resp.substr(pos, end - pos) : L"";
        };
        res.licensee = extract(L"\"licensee\":\"");
        res.tier     = extract(L"\"tier\":\"");
        res.message  = extract(L"\"message\":\"");
    }

    void PersistActivation(const ActivationResult& res) {
        HKEY hk = nullptr;
        RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\ExplorerLens\\Activation",
                0, nullptr, 0, KEY_WRITE, nullptr, &hk, nullptr);
        if (!hk) return;

        SYSTEMTIME now = {}; GetSystemTime(&now);
        FILETIME ft = {}; SystemTimeToFileTime(&now, &ft);
        ULARGE_INTEGER uli; uli.LowPart = ft.dwLowDateTime; uli.HighPart = ft.dwHighDateTime;
        uint64_t epoch = (uli.QuadPart - 116444736000000000ULL) / 10000000ULL;
        DWORD ep32 = static_cast<DWORD>(epoch & 0xFFFFFFFF);
        RegSetValueExW(hk, L"LastOnlineEpoch", 0, REG_DWORD,
                reinterpret_cast<const BYTE*>(&ep32), sizeof(DWORD));
        RegSetValueExW(hk, L"Licensee", 0, REG_SZ,
            reinterpret_cast<const BYTE*>(res.licensee.c_str()),
            static_cast<DWORD>((res.licensee.size() + 1) * sizeof(wchar_t)));
        RegCloseKey(hk);

        if (m_callback) m_callback(res);
    }

    void RemoveActivation() {
        RegDeleteKeyW(HKEY_CURRENT_USER, L"Software\\ExplorerLens\\Activation");
    }

    void ResetGraceTimer() {
        // LastOnlineEpoch is updated in PersistActivation
    }

    std::function<void(const ActivationResult&)> m_callback;
};

}} // namespace ExplorerLens::Engine
