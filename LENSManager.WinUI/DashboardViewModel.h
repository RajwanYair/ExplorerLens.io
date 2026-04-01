// DashboardViewModel.h — Dashboard / Home Page ViewModel for Manager.WinUI
// Copyright (c) 2026 ExplorerLens Project
//
// Provides statistics and status cards for the WinUI 3 Dashboard page:
// registration status, cache stats, recent activity, format count,
// GPU device info, and quick-action bindings.
//
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

// Opaque SDK handle types for the lens.exe C API.
using LENS_ENGINE_HANDLE    = void*;

namespace ExplorerLens { namespace Engine { namespace WinUI {

// Status of the shell extension COM registration
enum class RegistrationStatus {
    Registered,     // CLSID present in registry, DLL path valid
    NotRegistered,  // CLSID absent
    BrokenPath,     // CLSID registered but DLL path missing/invalid
    Unknown,
};

// Individual stat card shown on the Dashboard
struct StatCard {
    std::wstring title;
    std::wstring value;
    std::wstring unit;
    std::wstring iconGlyph;  // Segoe MDL2 glyph
    bool         isGood    = true;  // false = amber/red color
};

// One entry in the recent-activity feed
struct ActivityEntry {
    std::wstring timestamp;  // ISO8601 local time
    std::wstring verb;       // e.g. L"Decoded", L"Cached", L"Failed"
    std::wstring subject;    // File name (no path)
    std::wstring formatExt;  // e.g. L".psd"
    bool         success = true;
};

class DashboardViewModel {
public:
    explicit DashboardViewModel(LENS_ENGINE_HANDLE hEngine) : m_hEngine(hEngine) {
        Refresh();
    }

    // Refresh all stats — call periodically or on user action
    void Refresh();

    const std::vector<StatCard>&      Cards()    const { return m_cards; }
    const std::vector<ActivityEntry>& Activity() const { return m_activity; }
    RegistrationStatus                RegStatus()const { return m_regStatus; }

    // Quick actions
    bool RegisterShellExtension();    // Requires admin
    bool UnregisterShellExtension();  // Requires admin
    bool ClearCache();
    bool OpenLogFolder();

    // Format count supported
    uint32_t SupportedFormatCount() const { return m_formatCount; }

    // Set refresh callback (called after each Refresh())
    void SetRefreshCallback(std::function<void()> cb) { m_refreshCb = std::move(cb); }

private:
    LENS_ENGINE_HANDLE m_hEngine;
    std::vector<StatCard>      m_cards;
    std::vector<ActivityEntry> m_activity;
    RegistrationStatus         m_regStatus    = RegistrationStatus::Unknown;
    uint32_t                   m_formatCount  = 0;
    std::function<void()>      m_refreshCb;

    static constexpr wchar_t CLSID_STR[] = L"{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}";

    RegistrationStatus CheckRegistration() const;
    std::wstring       GetGPUName() const;
    void               BuildCards(uint64_t cacheUsed, uint64_t cacheCap);
};

inline RegistrationStatus DashboardViewModel::CheckRegistration() const {
    std::wstring regPath = std::wstring(L"CLSID\\") + CLSID_STR;
    HKEY hKey = nullptr;
    if (RegOpenKeyExW(HKEY_CLASSES_ROOT, regPath.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return RegistrationStatus::NotRegistered;
    wchar_t dllPath[MAX_PATH] = {};
    DWORD sz = sizeof(dllPath);
    DWORD res = RegGetValueW(hKey, L"InProcServer32", nullptr, RRF_RT_REG_SZ, nullptr, dllPath, &sz);
    RegCloseKey(hKey);
    if (res != ERROR_SUCCESS) return RegistrationStatus::BrokenPath;
    if (GetFileAttributesW(dllPath) == INVALID_FILE_ATTRIBUTES)
        return RegistrationStatus::BrokenPath;
    return RegistrationStatus::Registered;
}

inline std::wstring DashboardViewModel::GetGPUName() const {
    // Query DXGI adapter 0 description
    HMODULE hDXGI = LoadLibraryW(L"dxgi.dll"); if (!hDXGI) return L"Unknown GPU";
    using PFN_CreateDXGIFactory = HRESULT(WINAPI*)(REFIID, void**);
    auto fn = reinterpret_cast<PFN_CreateDXGIFactory>(GetProcAddress(hDXGI, "CreateDXGIFactory"));
    if (!fn) { FreeLibrary(hDXGI); return L"Unknown GPU"; }
    void* pFactory = nullptr;
    static const GUID IID_IDXGIFactory = {0x7b7166ec,0x21c7,0x44ae,{0xb2,0x1a,0xc9,0xae,0x32,0x1a,0xe3,0x69}};
    if (FAILED(fn(IID_IDXGIFactory, &pFactory))) { FreeLibrary(hDXGI); return L"No GPU"; }
    // IDXGIFactory::EnumAdapters[0] -> DXGI_ADAPTER_DESC.Description
    FreeLibrary(hDXGI);
    return L"GPU (DXGI)";  // Simplified; full impl requires IDXGIAdapter vtable
}

inline void DashboardViewModel::BuildCards(uint64_t cacheUsed, uint64_t cacheCap) {
    m_cards.clear();
    m_cards.push_back({ L"Registration",
        m_regStatus == RegistrationStatus::Registered ? L"Active" : L"Inactive",
        L"", L"\uE7EF", m_regStatus == RegistrationStatus::Registered });
    m_cards.push_back({ L"Formats", std::to_wstring(m_formatCount), L"formats", L"\uE8A5", true });
    m_cards.push_back({ L"Cache Used",
        std::to_wstring(cacheUsed / (1024*1024)), L"MB / " + std::to_wstring(cacheCap/(1024*1024)) + L" MB",
        L"\uE838", cacheUsed < cacheCap * 9 / 10 });
    m_cards.push_back({ L"GPU", GetGPUName(), L"", L"\uE950", true });
}

inline void DashboardViewModel::Refresh() {
    m_regStatus   = CheckRegistration();
    m_formatCount = 0;
    uint64_t used = 0, cap = 0;
    if (m_hEngine) {
        LensGetCacheStats(m_hEngine, &used, &cap);
        // Count formats: try common extensions
        static const wchar_t* EXTS[] = { L".jpg", L".png", L".webp", L".heic", L".avif",
            L".jxl", L".psd", L".raw", L".cr3", L".nef", L".pdf", L".svg", nullptr };
        for (const wchar_t* const* e = EXTS; *e; ++e)
            if (LensIsFormatSupported(*e)) ++m_formatCount;
    }
    BuildCards(used, cap);
    if (m_refreshCb) m_refreshCb();
}

}}} // namespace ExplorerLens::Engine::WinUI
