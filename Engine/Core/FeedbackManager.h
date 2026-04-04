// FeedbackManager.h — In-App Feedback Collection and Submission
// Copyright (c) 2026 ExplorerLens Project
//
// Manages user feedback submission from LENSManager: rating, category tagging,
// optional screenshot annotation, and delivery to the feedback endpoint.
//
#pragma once
#include <windows.h>
#include <winhttp.h>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#pragma comment(lib, "winhttp.lib")

namespace ExplorerLens {
namespace Engine {

enum class FeedbackCategory {
    GeneralComment,
    BugReport,
    FeatureRequest,
    PerformanceIssue,
    UIComplaint,
    Praise
};

enum class FeedbackStatus {
    Draft,
    Submitted,
    Failed,
    Throttled
};

struct FeedbackItem
{
    FeedbackCategory category = FeedbackCategory::GeneralComment;
    int rating = 0;  // 1-5 stars, 0 = no rating
    std::wstring title;
    std::wstring body;
    std::wstring userEmail;  // Optional; may be empty
    bool includeSystemInfo = true;
    bool includeLogs = false;
    std::wstring attachedScreenshot;  // Path to PNG, may be empty
    // Auto-filled on submit:
    std::wstring version;
    std::wstring osVersion;
    uint64_t feedbackId = 0;
    FeedbackStatus status = FeedbackStatus::Draft;
};

class FeedbackManager
{
  public:
    static constexpr const wchar_t* FEEDBACK_ENDPOINT = L"feedback.explorerlens.io";
    static constexpr INTERNET_PORT FEEDBACK_PORT = 443;

    // Validate before submit
    bool Validate(const FeedbackItem& item, std::wstring& outError) const
    {
        if (item.title.empty()) {
            outError = L"Title is required.";
            return false;
        }
        if (item.title.size() > 200) {
            outError = L"Title must be ≤ 200 characters.";
            return false;
        }
        if (item.body.size() > 4000) {
            outError = L"Body must be ≤ 4000 characters.";
            return false;
        }
        if (item.rating < 0 || item.rating > 5) {
            outError = L"Rating must be 0-5.";
            return false;
        }
        return true;
    }

    // Submit feedback asynchronously (callback on completion)
    void Submit(FeedbackItem item, std::function<void(const FeedbackItem&)> onDone)
    {
        // Check throttle: max 5 submissions per hour
        if (IsThrottled()) {
            item.status = FeedbackStatus::Throttled;
            if (onDone)
                onDone(item);
            return;
        }

        // Enrich with system info
        EnrichItem(item);

        std::wstring json = BuildJson(item);
        std::string body(json.size(), '\0');
        for (std::size_t _i = 0; _i < json.size(); ++_i)
            body[_i] = static_cast<char>(json[_i]);

        HINTERNET hSess =
            WinHttpOpen(L"ExplorerLens/20.0 Feedback", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, nullptr, nullptr, 0);
        HINTERNET hConn = WinHttpConnect(hSess, FEEDBACK_ENDPOINT, FEEDBACK_PORT, 0);
        HINTERNET hReq = WinHttpOpenRequest(hConn, L"POST", L"/api/v1/feedback", nullptr, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

        BOOL ok = WinHttpSendRequest(hReq, L"Content-Type: application/json\r\n", static_cast<DWORD>(-1L),
                                     const_cast<char*>(body.c_str()), static_cast<DWORD>(body.size()),
                                     static_cast<DWORD>(body.size()), 0);

        if (ok && WinHttpReceiveResponse(hReq, nullptr)) {
            item.status = FeedbackStatus::Submitted;
            RecordSubmission();
        } else {
            item.status = FeedbackStatus::Failed;
        }

        WinHttpCloseHandle(hReq);
        WinHttpCloseHandle(hConn);
        WinHttpCloseHandle(hSess);
        if (onDone)
            onDone(item);
    }

    // Save draft to registry so it survives restarts
    void SaveDraft(const FeedbackItem& item)
    {
        HKEY hk = nullptr;
        RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\ExplorerLens\\FeedbackDraft", 0, nullptr, 0, KEY_WRITE, nullptr,
                        &hk, nullptr);
        if (!hk)
            return;
        RegSetValueExW(hk, L"Title", 0, REG_SZ, reinterpret_cast<const BYTE*>(item.title.c_str()),
                       static_cast<DWORD>((item.title.size() + 1) * sizeof(wchar_t)));
        RegSetValueExW(hk, L"Body", 0, REG_SZ, reinterpret_cast<const BYTE*>(item.body.c_str()),
                       static_cast<DWORD>((item.body.size() + 1) * sizeof(wchar_t)));
        DWORD cat = static_cast<DWORD>(item.category);
        RegSetValueExW(hk, L"Category", 0, REG_DWORD, reinterpret_cast<const BYTE*>(&cat), sizeof(DWORD));
        RegCloseKey(hk);
    }

    // Load saved draft
    FeedbackItem LoadDraft() const
    {
        FeedbackItem item{};
        HKEY hk = nullptr;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\ExplorerLens\\FeedbackDraft", 0, KEY_READ, &hk)
            != ERROR_SUCCESS)
            return item;
        wchar_t buf[4096] = {};
        DWORD sz = sizeof(buf);
        if (RegQueryValueExW(hk, L"Title", nullptr, nullptr, reinterpret_cast<BYTE*>(buf), &sz) == ERROR_SUCCESS)
            item.title = buf;
        sz = sizeof(buf);
        if (RegQueryValueExW(hk, L"Body", nullptr, nullptr, reinterpret_cast<BYTE*>(buf), &sz) == ERROR_SUCCESS)
            item.body = buf;
        RegCloseKey(hk);
        return item;
    }

  private:
    static constexpr int MAX_SUBMISSIONS_PER_HOUR = 5;
    int m_recentSubmissions = 0;
    DWORD m_hourWindowStart = 0;

    bool IsThrottled()
    {
        DWORD now = GetTickCount();
        if (now - m_hourWindowStart > 3600000) {
            m_hourWindowStart = now;
            m_recentSubmissions = 0;
        }
        return m_recentSubmissions >= MAX_SUBMISSIONS_PER_HOUR;
    }

    void RecordSubmission()
    {
        m_recentSubmissions++;
    }

    void EnrichItem(FeedbackItem& item)
    {
        item.version = L"20.0.0";
        auto pfn = reinterpret_cast<LONG(WINAPI*)(OSVERSIONINFOW*)>(
            GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion"));
        OSVERSIONINFOW osv = {};
        osv.dwOSVersionInfoSize = sizeof(osv);
        if (pfn)
            pfn(&osv);
        item.osVersion = std::to_wstring(osv.dwMajorVersion) + L"." + std::to_wstring(osv.dwMinorVersion) + L"."
                         + std::to_wstring(osv.dwBuildNumber);
    }

    static std::wstring Escape(const std::wstring& s)
    {
        std::wstring r;
        r.reserve(s.size());
        for (wchar_t c : s) {
            if (c == L'"')
                r += L"\\\"";
            else if (c == L'\\')
                r += L"\\\\";
            else if (c == L'\n')
                r += L"\\n";
            else
                r += c;
        }
        return r;
    }

    static std::wstring BuildJson(const FeedbackItem& item)
    {
        return L"{\"category\":" + std::to_wstring(static_cast<int>(item.category)) + L",\"rating\":"
               + std::to_wstring(item.rating) + L",\"title\":\"" + Escape(item.title) + L"\"" + L",\"body\":\""
               + Escape(item.body) + L"\"" + L",\"version\":\"" + item.version + L"\"" + L",\"os\":\"" + item.osVersion
               + L"\"}";
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
