// EnterpriseAuditExporter.h — SIEM Audit Log Exporter
// Copyright (c) 2026 ExplorerLens Project
//
// Provides EnterpriseAuditExporter for forwarding structured audit events to SIEM
// platforms including Splunk HEC, Microsoft Sentinel, QRadar, and Elastic.
//
#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace ExplorerLens::Engine {

enum class SIEMTarget : uint8_t {
    SplunkHEC = 0,
    MicrosoftSentinel = 1,
    QRadar = 2,
    Elastic = 3,
    Generic = 4
};

enum class AuditLogFormat : uint8_t {
    CEF = 0,
    LEEF = 1,
    NDJSON = 2,
    Syslog = 3
};

enum class AuditSeverity : uint8_t {
    Info = 0,
    Warning = 1,
    Error = 2,
    Critical = 3
};

struct EnterpriseAuditEvent
{
    std::string eventId;
    std::string category;
    std::string severity;
    std::string actor;
    std::string resource;
    std::string outcome;
    std::string details;
    std::chrono::system_clock::time_point timestamp;
};

struct SIEMConfig
{
    SIEMTarget target{SIEMTarget::Generic};
    AuditLogFormat format{AuditLogFormat::NDJSON};
    std::string endpoint;
    std::string authToken;
    uint32_t batchSize{100};
    uint32_t flushIntervalMs{5000};
    bool tlsEnabled{true};
};

struct ExportStats
{
    uint64_t eventsExported{0};
    uint64_t eventsDropped{0};
    uint64_t batchesFlushed{0};
    uint32_t currentQueueDepth{0};
};

class EnterpriseAuditExporter
{
  public:
    EnterpriseAuditExporter() = default;
    explicit EnterpriseAuditExporter(SIEMConfig config);
    ~EnterpriseAuditExporter();

    EnterpriseAuditExporter(const EnterpriseAuditExporter&) = delete;
    EnterpriseAuditExporter& operator=(const EnterpriseAuditExporter&) = delete;

    // Event export
    bool ExportEvent(EnterpriseAuditEvent event);
    bool ExportBatch(std::vector<EnterpriseAuditEvent> events);

    // Target configuration
    bool SetTarget(SIEMConfig config);
    const SIEMConfig& GetConfig() const noexcept;

    // Queue management
    bool FlushBatch();
    uint32_t GetQueueDepth() const noexcept;
    void ClearQueue();

    // Connectivity
    bool TestConnectivity() const;
    bool IsConnected() const noexcept;

    // Statistics
    ExportStats GetStats() const noexcept;
    void ResetStats();

    // Error callback
    using ErrorCallback = std::function<void(const std::string&, const EnterpriseAuditEvent&)>;
    void SetErrorCallback(ErrorCallback cb);

  private:
    SIEMConfig m_config;
    bool m_connected{false};
    ExportStats m_stats;
    std::vector<EnterpriseAuditEvent> m_queue;
    ErrorCallback m_errorCallback;

    std::string FormatEvent(const EnterpriseAuditEvent& event) const;
    bool TransmitBatch(const std::vector<EnterpriseAuditEvent>& batch) const;
    bool AuthenticateTarget() const;
    std::string GenerateEventId() const;
};

}  // namespace ExplorerLens::Engine
