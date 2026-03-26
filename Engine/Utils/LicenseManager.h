// LicenseManager.h — License Key Validation and Trial Management
// Copyright (c) 2026 ExplorerLens Project
//
// Validates product license keys, manages trial period gating, and enforces
// feature entitlements based on license tier (Community/Pro/Enterprise).
//
#pragma once
#include <windows.h>
#include <string>
#include <functional>
#include <array>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class LicenseTier { Community, Pro, Enterprise, Trial, Expired };

struct LicenseInfo {
    LicenseTier    tier         = LicenseTier::Community;
    std::wstring   keyHash;           // SHA-256 of the raw key (never store raw)
    std::wstring   licensee;          // Name/org from activation server
    SYSTEMTIME     expiresAt   = {};
    int            trialDaysLeft = 0;
    bool           valid        = false;
};

// Feature flags controlled by license tier
enum class LicenseFeature : uint32_t {
    BasicThumbnails   = 1 << 0,
    GpuAcceleration   = 1 << 1,
    AdvancedFormats   = 1 << 2, // CAD, glTF, scientific
    BatchProcessing   = 1 << 3,
    EnterpriseGPO     = 1 << 4,
    CloudSync         = 1 << 5,
    AISmartCrop       = 1 << 6,
    PrioritySupport   = 1 << 7,
    All               = 0xFF
};
inline LicenseFeature operator|(LicenseFeature a, LicenseFeature b) {
    return static_cast<LicenseFeature>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

// Entitlement masks per tier
struct TierEntitlements {
    static constexpr uint32_t Community  = static_cast<uint32_t>(LicenseFeature::BasicThumbnails);
    static constexpr uint32_t Pro        = Community
        | static_cast<uint32_t>(LicenseFeature::GpuAcceleration)
        | static_cast<uint32_t>(LicenseFeature::AdvancedFormats)
        | static_cast<uint32_t>(LicenseFeature::BatchProcessing);
    static constexpr uint32_t Enterprise = Pro
        | static_cast<uint32_t>(LicenseFeature::EnterpriseGPO)
        | static_cast<uint32_t>(LicenseFeature::CloudSync)
        | static_cast<uint32_t>(LicenseFeature::AISmartCrop)
        | static_cast<uint32_t>(LicenseFeature::PrioritySupport);
    static constexpr uint32_t Trial      = Pro; // Same as Pro during trial
};

class LicenseManager {
public:
    static LicenseManager& Get() {
        static LicenseManager s_inst;
        return s_inst;
    }

    // Validate a key locally (checksum only) without network call
    bool ValidateKeyFormat(const std::wstring& key) const {
        // Format: XXXXX-XXXXX-XXXXX-XXXXX-XXXXX (25 alphanum chars)
        if (key.size() != 29) return false;
        for (int i = 0; i < 29; ++i) {
            if (i % 6 == 5) {
                if (key[i] != L'-') return false;
            } else {
                wchar_t c = key[i];
                bool alphanum = (c >= L'A' && c <= L'Z') || (c >= L'0' && c <= L'9');
                if (!alphanum) return false;
            }
        }
        return true;
    }

    // Load persisted license from HKCU registry
    bool LoadFromRegistry() {
        HKEY hk = nullptr;
        if (RegOpenKeyExW(HKEY_CURRENT_USER,
                L"Software\\ExplorerLens\\License", 0, KEY_READ, &hk) != ERROR_SUCCESS)
            return false;

        wchar_t buf[512] = {};
        DWORD sz = sizeof(buf);
        DWORD type = 0;

        if (RegQueryValueExW(hk, L"KeyHash", nullptr, &type,
                reinterpret_cast<BYTE*>(buf), &sz) == ERROR_SUCCESS)
            m_info.keyHash = buf;

        sz = sizeof(buf);
        if (RegQueryValueExW(hk, L"Licensee", nullptr, &type,
                reinterpret_cast<BYTE*>(buf), &sz) == ERROR_SUCCESS)
            m_info.licensee = buf;

        DWORD tierVal = 0; sz = sizeof(tierVal);
        if (RegQueryValueExW(hk, L"Tier", nullptr, &type,
                reinterpret_cast<BYTE*>(&tierVal), &sz) == ERROR_SUCCESS)
            m_info.tier = static_cast<LicenseTier>(tierVal);

        RegCloseKey(hk);
        m_info.valid = !m_info.keyHash.empty();
        RefreshEntitlements();
        return m_info.valid;
    }

    // Persist license info to HKCU (call after successful activation)
    bool SaveToRegistry(const LicenseInfo& info) {
        HKEY hk = nullptr;
        RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\ExplorerLens\\License",
                0, nullptr, 0, KEY_WRITE, nullptr, &hk, nullptr);
        if (!hk) return false;

        RegSetValueExW(hk, L"KeyHash", 0, REG_SZ,
            reinterpret_cast<const BYTE*>(info.keyHash.c_str()),
            static_cast<DWORD>((info.keyHash.size() + 1) * sizeof(wchar_t)));
        RegSetValueExW(hk, L"Licensee", 0, REG_SZ,
            reinterpret_cast<const BYTE*>(info.licensee.c_str()),
            static_cast<DWORD>((info.licensee.size() + 1) * sizeof(wchar_t)));
        DWORD tierVal = static_cast<DWORD>(info.tier);
        RegSetValueExW(hk, L"Tier", 0, REG_DWORD,
            reinterpret_cast<const BYTE*>(&tierVal), sizeof(DWORD));
        RegCloseKey(hk);

        m_info = info;
        RefreshEntitlements();
        return true;
    }

    // Check if a specific feature is entitled under current license
    bool IsFeatureEnabled(LicenseFeature feature) const {
        return (m_entitlements & static_cast<uint32_t>(feature)) != 0;
    }

    // Check trial validity
    bool IsTrialActive() const {
        if (m_info.tier != LicenseTier::Trial) return false;
        SYSTEMTIME now = {};
        GetLocalTime(&now);
        FILETIME ftNow = {}, ftExp = {};
        SystemTimeToFileTime(&now, &ftNow);
        SystemTimeToFileTime(&m_info.expiresAt, &ftExp);
        ULARGE_INTEGER uNow, uExp;
        uNow.LowPart = ftNow.dwLowDateTime; uNow.HighPart = ftNow.dwHighDateTime;
        uExp.LowPart = ftExp.dwLowDateTime; uExp.HighPart = ftExp.dwHighDateTime;
        return uNow.QuadPart < uExp.QuadPart;
    }

    // Start a 30-day trial (sets registry marker + returns false if already used)
    bool StartTrial() {
        HKEY hk = nullptr;
        RegCreateKeyExW(HKEY_LOCAL_MACHINE, L"Software\\ExplorerLens\\Trial",
                0, nullptr, 0, KEY_READ | KEY_WRITE, nullptr, &hk, nullptr);
        if (!hk) return false;
        DWORD started = 0; DWORD sz = sizeof(started);
        RegQueryValueExW(hk, L"Started", nullptr, nullptr,
                reinterpret_cast<BYTE*>(&started), &sz);
        if (started) { RegCloseKey(hk); return false; } // Trial already used
        started = 1;
        RegSetValueExW(hk, L"Started", 0, REG_DWORD,
                reinterpret_cast<const BYTE*>(&started), sizeof(DWORD));
        RegCloseKey(hk);

        GetLocalTime(&m_info.expiresAt);
        m_info.expiresAt.wMonth += 1; // +30 days (simplified)
        m_info.tier = LicenseTier::Trial;
        m_info.trialDaysLeft = 30;
        m_info.valid = true;
        RefreshEntitlements();
        return true;
    }

    const LicenseInfo& Info() const { return m_info; }

    void OnChange(std::function<void(const LicenseInfo&)> cb) {
        m_onChange = std::move(cb);
    }

private:
    LicenseManager() { LoadFromRegistry(); }

    void RefreshEntitlements() {
        switch (m_info.tier) {
        case LicenseTier::Pro:        m_entitlements = TierEntitlements::Pro; break;
        case LicenseTier::Enterprise: m_entitlements = TierEntitlements::Enterprise; break;
        case LicenseTier::Trial:      m_entitlements = TierEntitlements::Trial; break;
        default:                      m_entitlements = TierEntitlements::Community; break;
        }
        if (m_onChange) m_onChange(m_info);
    }

    LicenseInfo m_info;
    uint32_t    m_entitlements = TierEntitlements::Community;
    std::function<void(const LicenseInfo&)> m_onChange;
};

}} // namespace ExplorerLens::Engine
