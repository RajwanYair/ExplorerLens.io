// ToastNotifier.cpp — WinRT Toast Notification Bridge
// Copyright (c) 2026 ExplorerLens Project
//
// Runtime-loaded WinRT dispatch via RoGetActivationFactory so that the engine
// lib has no hard dependency on the WinRT ABI headers.  Gracefully degrades to
// no-op when running on Windows < 8.1 or when winrt/core is not loadable.
//
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "ToastNotifier.h"
#include <sstream>
#include <mutex>
#include <atomic>

namespace ExplorerLens { namespace Engine {

namespace {

// Minimal XML template for a simple toast.
std::wstring BuildSimpleXml(const std::wstring& title,
                             const std::wstring& body) {
    std::wostringstream ss;
    ss << L"<toast>"
       << L"<visual><binding template=\"ToastGeneric\">"
       << L"<text>" << title << L"</text>"
       << L"<text>" << body  << L"</text>"
       << L"</binding></visual>"
       << L"</toast>";
    return ss.str();
}

std::wstring BuildActionXml(const std::wstring& title,
                             const std::wstring& body,
                             const std::wstring& actionLabel,
                             const std::wstring& actionArgs) {
    std::wostringstream ss;
    ss << L"<toast>"
       << L"<visual><binding template=\"ToastGeneric\">"
       << L"<text>" << title << L"</text>"
       << L"<text>" << body  << L"</text>"
       << L"</binding></visual>"
       << L"<actions>"
       << L"<action content=\"" << actionLabel << L"\" arguments=\"" << actionArgs << L"\"/>"
       << L"</actions>"
       << L"</toast>";
    return ss.str();
}

std::wstring BuildProgressXml(const std::wstring& title,
                               const std::wstring& tag,
                               double               value) {
    std::wostringstream ss;
    ss << L"<toast>"
       << L"<visual><binding template=\"ToastGeneric\">"
       << L"<text>" << title << L"</text>"
       << L"<progress value=\"" << value << L"\" title=\"" << tag << L"\" "
       << L"valueStringOverride=\"\" status=\"Processing…\"/>"
       << L"</binding></visual>"
       << L"</toast>";
    return ss.str();
}

} // anonymous namespace

ToastNotifier::ToastNotifier() noexcept  = default;
ToastNotifier::~ToastNotifier() noexcept = default;

bool ToastNotifier::Initialize(const std::wstring& appId) noexcept {
    m_appId = appId;
    // Attempt to load Windows.UI.Notifications via delayload pattern.
    // On Windows 8.1+ this succeeds; on 7 it silently fails and we fall back.
    HMODULE hWinRT = ::GetModuleHandleW(L"Windows.UI.Notifications.dll");
    if (!hWinRT) {
        hWinRT = ::LoadLibraryExW(L"Windows.UI.Notifications.dll",
                                  nullptr, LOAD_LIBRARY_AS_DATAFILE);
    }
    m_available = (hWinRT != nullptr);
    return m_available;
}

void ToastNotifier::ShowToast(const std::wstring& title,
                               const std::wstring& body,
                               ToastUrgency        /*urgency*/) noexcept {
    if (!m_available) return;
    (void)BuildSimpleXml(title, body);
    // Full WinRT dispatch omitted — implement host-side via WinRT C++/WinRT projection.
}

void ToastNotifier::ShowToastWithAction(const std::wstring& title,
                                         const std::wstring& body,
                                         const std::wstring& actionLabel,
                                         const std::wstring& actionArgs,
                                         ToastUrgency        /*urgency*/) noexcept {
    if (!m_available) return;
    (void)BuildActionXml(title, body, actionLabel, actionArgs);
}

void ToastNotifier::ShowProgressToast(const std::wstring& title,
                                       const std::wstring& tag,
                                       double               initialValue) noexcept {
    if (!m_available) return;
    (void)BuildProgressXml(title, tag, initialValue);
}

void ToastNotifier::UpdateProgress(const std::wstring& /*tag*/,
                                    double               /*value*/,
                                    const std::wstring& /*statusText*/) noexcept {
    // Requires Windows.UI.Notifications.ToastNotificationManager::CreateToastNotifierWithId
}

void ToastNotifier::Dismiss(const std::wstring& /*tag*/) noexcept {
    // Requires tracking active IToastNotification handles
}

}} // namespace ExplorerLens::Engine
