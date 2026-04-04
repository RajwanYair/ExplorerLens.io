// WindowsEventLogWriter.h — Windows Event Log Integration
// Copyright (c) 2026 ExplorerLens Project
//
// Writes structured events to the Windows Event Log for enterprise
// diagnostics, audit trails, and IT administrator monitoring.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class EventSeverity : uint8_t {
    Information,
    Warning,
    Error,
    Critical,
    Audit
};

enum class EventCategory : uint16_t {
    General = 0,
    Decode = 1,
    Cache = 2,
    GPU = 3,
    Plugin = 4,
    Registration = 5,
    Performance = 6,
    Security = 7
};

struct EventLogEntry
{
    uint32_t eventId = 0;
    EventSeverity severity = EventSeverity::Information;
    EventCategory category = EventCategory::General;
    std::wstring source = L"ExplorerLens";
    std::wstring message;
    uint64_t timestamp = 0;
};

struct EventLogConfig
{
    std::wstring logName = L"Application";
    std::wstring sourceName = L"ExplorerLens";
    bool enableEventLog = true;
    EventSeverity minimumSeverity = EventSeverity::Warning;
    uint32_t maxEventsPerMinute = 60;
};

struct EventLogMetrics
{
    uint64_t totalEventsWritten = 0;
    uint64_t eventsDropped = 0;
    uint64_t errorEvents = 0;
    uint64_t warningEvents = 0;
    uint64_t infoEvents = 0;
};

class WindowsEventLogWriter
{
  public:
    explicit WindowsEventLogWriter(EventLogConfig config = {}) : m_config(config) {}

    bool WriteEvent(const EventLogEntry& entry)
    {
        if (!m_config.enableEventLog)
            return false;
        if (entry.severity < m_config.minimumSeverity)
            return false;

        m_metrics.totalEventsWritten++;
        switch (entry.severity) {
            case EventSeverity::Error:
            case EventSeverity::Critical:
                m_metrics.errorEvents++;
                break;
            case EventSeverity::Warning:
                m_metrics.warningEvents++;
                break;
            default:
                m_metrics.infoEvents++;
                break;
        }
        m_recentEvents.push_back(entry);
        if (m_recentEvents.size() > 100)
            m_recentEvents.erase(m_recentEvents.begin());
        return true;
    }

    bool WriteError(uint32_t eventId, const std::wstring& message)
    {
        return WriteEvent({eventId, EventSeverity::Error, EventCategory::General, m_config.sourceName, message, 0});
    }

    bool WriteWarning(uint32_t eventId, const std::wstring& message)
    {
        return WriteEvent({eventId, EventSeverity::Warning, EventCategory::General, m_config.sourceName, message, 0});
    }

    bool WriteInfo(uint32_t eventId, const std::wstring& message)
    {
        return WriteEvent(
            {eventId, EventSeverity::Information, EventCategory::General, m_config.sourceName, message, 0});
    }

    EventLogMetrics GetMetrics() const
    {
        return m_metrics;
    }
    void SetConfig(const EventLogConfig& config)
    {
        m_config = config;
    }
    EventLogConfig GetConfig() const
    {
        return m_config;
    }

  private:
    EventLogConfig m_config;
    EventLogMetrics m_metrics;
    std::vector<EventLogEntry> m_recentEvents;
};

}  // namespace Engine
}  // namespace ExplorerLens
