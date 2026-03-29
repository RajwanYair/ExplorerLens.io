// UXTelemetryPrivacyDashboard.h — UX Telemetry Privacy Dashboard
// Copyright (c) 2026 ExplorerLens Project
//
// User-controlled telemetry collection with configurable retention, data
// export (JSON/CSV), and GDPR-compliant purge. No data leaves device without
// explicit opt-in consent.
//
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

struct TelemetryEvent {
    std::string category;
    std::string action;
    int64_t     timestampMs = 0;
    std::string sessionId;
};

struct TelemetryExport {
    std::string format;
    std::string payload;
    uint64_t    eventCount = 0;
};

class UXTelemetryPrivacyDashboard {
public:
    UXTelemetryPrivacyDashboard() = default;

    bool Initialize() { m_ready = true; return true; }
    bool IsReady() const { return m_ready; }

    void SetConsentGranted(bool granted) { m_consentGranted = granted; }
    bool IsConsentGranted() const { return m_consentGranted; }

    void SetRetentionDays(uint32_t days) { m_retentionDays = days; }
    uint32_t GetRetentionDays() const { return m_retentionDays; }

    bool RecordEvent(const TelemetryEvent& evt) {
        if (!m_consentGranted) return false;
        m_events.push_back(evt);
        return true;
    }

    uint64_t GetEventCount() const { return m_events.size(); }

    TelemetryExport ExportData(const std::string& format = "json") const {
        TelemetryExport exp;
        exp.format     = format;
        exp.eventCount = m_events.size();
        if (format == "json") {
            exp.payload = "{\"events\":[";
            for (size_t i = 0; i < m_events.size(); ++i) {
                if (i > 0) exp.payload += ",";
                exp.payload += "{\"cat\":\"" + m_events[i].category
                             + "\",\"act\":\"" + m_events[i].action + "\"}";
            }
            exp.payload += "]}";
        } else if (format == "csv") {
            exp.payload = "category,action,timestampMs\n";
            for (const auto& e : m_events) {
                exp.payload += e.category + "," + e.action + ","
                             + std::to_string(e.timestampMs) + "\n";
            }
        }
        return exp;
    }

    void PurgeData() {
        m_events.clear();
    }

    void Shutdown() { m_ready = false; }

private:
    bool                       m_ready          = false;
    bool                       m_consentGranted = false;
    uint32_t                   m_retentionDays  = 30;
    std::vector<TelemetryEvent> m_events;
};

}} // namespace ExplorerLens::Engine
