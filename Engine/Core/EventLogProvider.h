// EventLogProvider.h — Windows Event Log integration
// Copyright (c) 2026 ExplorerLens Project
//
// Reports administrator-visible errors to the Windows Application Event Log.
// Events appear in Event Viewer → Windows Logs → Application under source
// "ExplorerLens".  Designed for high-severity conditions only (registration
// failures, cache corruption, decoder crashes) to avoid log spam.
//
#pragma once

#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

// Event IDs — keep stable across versions (never reuse a retired ID)
enum class EventId : uint16_t {
    DLL_REGISTRATION_FAILED  = 1001,
    DLL_UNREGISTRATION_FAILED = 1002,
    CACHE_CORRUPTION_DETECTED = 1101,
    CACHE_STORE_UNAVAILABLE   = 1102,
    DECODER_CRASH             = 1201,
    DECODER_INIT_FAILED       = 1202,
    GPU_INIT_FAILED           = 1301,
    GPU_SHADER_COMPILE_FAILED = 1302,
    CONFIG_LOAD_FAILED        = 1401,
    PLUGIN_LOAD_FAILED        = 1501,
    PLUGIN_TRUST_VIOLATION    = 1502,
};

// Severity maps to EVENTLOG_*_TYPE constants
enum class EventSeverity : uint16_t {
    SUCCESS     = 0x0000,
    ERROR_TYPE  = 0x0001,
    WARNING     = 0x0002,
    INFORMATION = 0x0004,
};

class EventLogProvider {
public:
    // Source name registered under HKLM\SYSTEM\CurrentControlSet\Services\EventLog\Application
    static constexpr const char* SOURCE_NAME = "ExplorerLens";

    static EventLogProvider& Instance();

    // Register the event source (call once at DLL attach; requires HKLM write — silently skips if insufficient privilege)
    void Register();

    // Deregister (call at DLL detach)
    void Deregister();

    // Report an event with a message string
    void ReportEvent(EventId id, EventSeverity severity, const std::wstring& message);
    void ReportEvent(EventId id, EventSeverity severity, const std::string& message);

    // Convenience wrappers
    void Error(EventId id, const std::wstring& msg)   { ReportEvent(id, EventSeverity::ERROR_TYPE, msg); }
    void Warning(EventId id, const std::wstring& msg) { ReportEvent(id, EventSeverity::WARNING, msg); }
    void Info(EventId id, const std::wstring& msg)    { ReportEvent(id, EventSeverity::INFORMATION, msg); }

    bool IsRegistered() const { return m_hEventLog != nullptr; }

    EventLogProvider(const EventLogProvider&) = delete;
    EventLogProvider& operator=(const EventLogProvider&) = delete;

private:
    EventLogProvider() = default;
    ~EventLogProvider();

    void* m_hEventLog = nullptr;   // HANDLE — opaque to avoid Windows.h in header
};

} // namespace Engine
} // namespace ExplorerLens
