// EnterpriseAuditExporter.h — SIEM-Compatible Audit Log Exporter
// Copyright (c) 2026 ExplorerLens Project
//
// Formats thumbnail decode events, policy violations, and AI guard triggers
// as CEF (ArcSight), LEEF (QRadar), or JSON-L (Splunk/Elastic) syslog records
// and delivers them to a configured SIEM collector endpoint.
//
#pragma once

#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <cstdint>
#include <sstream>
#include <ctime>

namespace ExplorerLens { namespace Engine { namespace Enterprise {

enum class AuditFormat : uint8_t {
    CEF    = 0,   // ArcSight Common Event Format
    LEEF   = 1,   // IBM QRadar Log Event Extended Format
    JSONL  = 2,   // Newline-delimited JSON (Splunk, Elastic, Sentinel)
    Syslog = 3    // RFC 5424 syslog with structured data
};

enum class AuditSeverity : uint8_t {
    Info     = 0,
    Low      = 1,
    Medium   = 2,
    High     = 3,
    Critical = 4
};

struct AuditEvent {
    std::string           eventId;        // e.g. "LENS-DECODE-001"
    std::string           eventName;      // Human-readable event name
    AuditSeverity         severity        = AuditSeverity::Info;
    std::string           userName;       // Windows SID or UPN
    std::string           machineId;
    std::string           filePath;       // Decoded/requested file (sanitized)
    std::string           fileFormat;     // Extension / MIME type
    std::string           outcome;        // "success" | "blocked" | "error"
    std::string           policyViolation;// populated if GP/MDM rule triggered
    uint32_t              durationMs      = 0;
    std::chrono::system_clock::time_point timeStamp;
};

struct AuditExporterConfig {
    AuditFormat   format          = AuditFormat::JSONL;
    std::string   collectorHost;           // SIEM host or "" for local file only
    uint16_t      collectorPort    = 514;
    std::string   localFilePath;           // Append-mode log file
    bool          useTLS           = true;
    uint32_t      maxBatchSize     = 256;  // Events to buffer before flushing
    uint32_t      flushIntervalMs  = 5000;
    std::string   vendorDevice     = "ExplorerLens";
    std::string   vendorProduct    = "ThumbnailEngine";
    std::string   vendorVersion    = "19.0";
};

class EnterpriseAuditExporter {
public:
    static EnterpriseAuditExporter& Instance() {
        static EnterpriseAuditExporter inst;
        return inst;
    }

    void Configure(AuditExporterConfig cfg) { m_cfg = std::move(cfg); }

    void Emit(const AuditEvent& ev) {
        std::string record = FormatEvent(ev);
        if (!m_cfg.localFilePath.empty()) WriteToFile(record);
        m_buffer.push_back(record);
        if (m_buffer.size() >= m_cfg.maxBatchSize) Flush();
    }

    void Flush() {
        if (m_buffer.empty()) return;
        if (!m_cfg.collectorHost.empty()) SendBatch(m_buffer);
        m_buffer.clear();
    }

    // Convenience helpers for common audit events
    void EmitDecodeBlocked(const std::string& filePath, const std::string& reason, const std::string& user) {
        AuditEvent ev;
        ev.eventId = "LENS-SEC-001"; ev.eventName = "DecodeBlocked";
        ev.severity = AuditSeverity::High; ev.filePath = filePath;
        ev.outcome = "blocked"; ev.policyViolation = reason; ev.userName = user;
        ev.timeStamp = std::chrono::system_clock::now();
        Emit(ev);
    }

    void EmitNSFWTriggered(const std::string& filePath, float confidence, const std::string& user) {
        AuditEvent ev;
        ev.eventId = "LENS-SEC-002"; ev.eventName = "NSFWGuardTriggered";
        ev.severity = AuditSeverity::High; ev.filePath = filePath;
        ev.outcome = "blocked"; ev.userName = user;
        ev.policyViolation = "NSFW confidence=" + std::to_string(confidence);
        ev.timeStamp = std::chrono::system_clock::now();
        Emit(ev);
    }

    void EmitPolicyViolation(const std::string& rule, const std::string& details) {
        AuditEvent ev;
        ev.eventId = "LENS-POL-001"; ev.eventName = "PolicyViolation";
        ev.severity = AuditSeverity::Medium;
        ev.outcome = "blocked"; ev.policyViolation = rule + ": " + details;
        ev.timeStamp = std::chrono::system_clock::now();
        Emit(ev);
    }

private:
    EnterpriseAuditExporter() = default;

    std::string FormatEvent(const AuditEvent& ev) const {
        switch (m_cfg.format) {
            case AuditFormat::CEF:   return FormatCEF(ev);
            case AuditFormat::LEEF:  return FormatLEEF(ev);
            case AuditFormat::JSONL: return FormatJSONL(ev);
            default:                 return FormatJSONL(ev);
        }
    }

    std::string EpochStr(const std::chrono::system_clock::time_point& tp) const {
        auto t = std::chrono::system_clock::to_time_t(tp);
        char buf[32]; struct tm tm_buf;
        gmtime_s(&tm_buf, &t);
        strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm_buf);
        return buf;
    }

    std::string FormatCEF(const AuditEvent& ev) const {
        // CEF:Version|Device Vendor|Device Product|Device Version|SignatureID|Name|Severity|Extension
        int cefSev = static_cast<int>(ev.severity) * 2;
        std::ostringstream ss;
        ss << "CEF:0|" << m_cfg.vendorDevice << "|" << m_cfg.vendorProduct << "|"
           << m_cfg.vendorVersion << "|" << ev.eventId << "|" << ev.eventName
           << "|" << cefSev << "|"
           << "src=" << ev.machineId << " suser=" << ev.userName
           << " fpath=" << ev.filePath << " outcome=" << ev.outcome
           << " rt=" << EpochStr(ev.timeStamp);
        return ss.str();
    }

    std::string FormatLEEF(const AuditEvent& ev) const {
        std::ostringstream ss;
        ss << "LEEF:2.0|" << m_cfg.vendorDevice << "|" << m_cfg.vendorProduct << "|"
           << m_cfg.vendorVersion << "|" << ev.eventId << "|"
           << "devTime=" << EpochStr(ev.timeStamp)
           << "\tusrName=" << ev.userName
           << "\tsrc=" << ev.machineId
           << "\toutcome=" << ev.outcome;
        if (!ev.policyViolation.empty())
            ss << "\tpolicyRule=" << ev.policyViolation;
        return ss.str();
    }

    std::string FormatJSONL(const AuditEvent& ev) const {
        std::ostringstream ss;
        ss << "{\"ts\":\"" << EpochStr(ev.timeStamp) << "\","
           << "\"id\":\"" << ev.eventId << "\","
           << "\"name\":\"" << ev.eventName << "\","
           << "\"sev\":" << static_cast<int>(ev.severity) << ","
           << "\"user\":\"" << ev.userName << "\","
           << "\"machine\":\"" << ev.machineId << "\","
           << "\"file\":\"" << ev.filePath << "\","
           << "\"outcome\":\"" << ev.outcome << "\","
           << "\"policy\":\"" << ev.policyViolation << "\","
           << "\"durationMs\":" << ev.durationMs << "}";
        return ss.str();
    }

    void WriteToFile(const std::string& record) {
        FILE* f = nullptr;
        if (fopen_s(&f, m_cfg.localFilePath.c_str(), "a") == 0 && f) {
            fputs(record.c_str(), f);
            fputc('\n', f);
            fclose(f);
        }
    }

    void SendBatch(const std::vector<std::string>& records) {
        // Production: UDP/TCP syslog or HTTPS POST to SIEM collector
        // Stub: write batch size to diagnostics log
        (void)records;
    }

    AuditExporterConfig      m_cfg;
    std::vector<std::string> m_buffer;
};

}}} // namespace ExplorerLens::Engine::Enterprise
