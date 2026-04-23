// EventLogProvider.cpp — Windows Event Log integration
// Copyright (c) 2026 ExplorerLens Project
//
#include "EventLogProvider.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <string>
#include <codecvt>
#include <locale>

// Undo the ReportEvent → ReportEventW macro from <winbase.h>
// so our method definitions use the un-mangled name.
#ifdef ReportEvent
#undef ReportEvent
#endif
namespace ExplorerLens {
namespace Engine {

EventLogProvider& EventLogProvider::Instance() {
    static EventLogProvider instance;
    return instance;
}

EventLogProvider::~EventLogProvider() {
    Deregister();
}

void EventLogProvider::Register() {
#ifdef _WIN32
    if (m_hEventLog) return;

    // Try to ensure registry key exists (best-effort; may fail without admin rights)
    const std::wstring regPath =
        L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\ExplorerLens";
    HKEY hKey = nullptr;
    DWORD disposition = 0;
    if (RegCreateKeyExW(HKEY_LOCAL_MACHINE, regPath.c_str(),
                        0, nullptr, REG_OPTION_NON_VOLATILE,
                        KEY_SET_VALUE, nullptr, &hKey, &disposition) == ERROR_SUCCESS) {
        // Point EventMessageFile to this module so the event viewer can format messages
        wchar_t modulePath[MAX_PATH] = {};
        GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
        RegSetValueExW(hKey, L"EventMessageFile", 0, REG_EXPAND_SZ,
                       reinterpret_cast<const BYTE*>(modulePath),
                       static_cast<DWORD>((wcslen(modulePath) + 1) * sizeof(wchar_t)));
        DWORD types = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
        RegSetValueExW(hKey, L"TypesSupported", 0, REG_DWORD,
                       reinterpret_cast<const BYTE*>(&types), sizeof(types));
        RegCloseKey(hKey);
    }

    m_hEventLog = static_cast<void*>(RegisterEventSourceW(nullptr, L"ExplorerLens"));
#endif
}

void EventLogProvider::Deregister() {
#ifdef _WIN32
    if (m_hEventLog) {
        ::DeregisterEventSource(static_cast<HANDLE>(m_hEventLog));
        m_hEventLog = nullptr;
    }
#endif
}

void EventLogProvider::ReportEvent(EventId id, EventSeverity severity, const std::wstring& message) {
#ifdef _WIN32
    if (!m_hEventLog) return;

    WORD wType = EVENTLOG_INFORMATION_TYPE;
    switch (severity) {
        case EventSeverity::ERROR_TYPE:  wType = EVENTLOG_ERROR_TYPE;   break;
        case EventSeverity::WARNING:     wType = EVENTLOG_WARNING_TYPE; break;
        case EventSeverity::SUCCESS:     wType = EVENTLOG_SUCCESS;      break;
        default: break;
    }

    const WCHAR* strings[1] = { message.c_str() };
    ::ReportEventW(static_cast<HANDLE>(m_hEventLog),
                   wType,
                   0,                          // category
                   static_cast<DWORD>(id),
                   nullptr,                    // user SID
                   1,                          // string count
                   0,                          // raw data size
                   strings,
                   nullptr);
#else
    (void)id; (void)severity; (void)message;
#endif
}

void EventLogProvider::ReportEvent(EventId id, EventSeverity severity, const std::string& message) {
    ReportEvent(id, severity,
                std::wstring(message.begin(), message.end()));
}

} // namespace Engine
} // namespace ExplorerLens
