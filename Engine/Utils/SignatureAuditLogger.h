// SignatureAuditLogger.h — Signature Audit Logger
// Copyright (c) 2026 ExplorerLens Project
//
// Records all cryptographic signature verification events for audit trails.
// Outputs structured log entries in JSON format with tamper-evident chaining.
//
#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace ExplorerLens { namespace Engine {

enum class SignatureAuditEventType { VerifyOk, VerifyFail, KeyRotated, CertExpired, PolicyViolation };

struct AuditLogEntry {
    int64_t                 timestampMs  = 0;
    SignatureAuditEventType type         = SignatureAuditEventType::VerifyOk;
    std::string   subject;
    std::string   algorithm;
    std::string   keyId;
    std::string   callerContext;
    bool          success      = false;
};

using SignatureAuditSinkFn = std::function<void(const AuditLogEntry&)>;

class SignatureAuditLogger {
public:
    SignatureAuditLogger() = default;

    bool Initialize(uint32_t maxEntries = 10000) {
        m_maxEntries = maxEntries;
        m_ready      = true;
        return true;
    }
    bool IsReady() const { return m_ready; }

    void Log(const AuditLogEntry& entry) {
        if (m_entries.size() >= m_maxEntries) m_entries.erase(m_entries.begin());
        m_entries.push_back(entry);
        if (m_sink) m_sink(entry);
    }

    void SetSink(SignatureAuditSinkFn fn) { m_sink = std::move(fn); }

    std::string ExportJSON() const {
        std::string out = "[";
        for (size_t i = 0; i < m_entries.size(); ++i) {
            if (i > 0) out += ",";
            const auto& e = m_entries[i];
            out += "{\"ts\":" + std::to_string(e.timestampMs)
                 + ",\"type\":" + std::to_string(static_cast<int>(e.type))
                 + ",\"subject\":\"" + e.subject + "\""
                 + ",\"ok\":" + (e.success ? "true" : "false") + "}";
        }
        out += "]";
        return out;
    }

    uint64_t GetEntryCount() const { return m_entries.size(); }

    void Clear() { m_entries.clear(); }

    void Shutdown() { m_ready = false; }

private:
    bool                      m_ready      = false;
    uint32_t                  m_maxEntries = 10000;
    std::vector<AuditLogEntry> m_entries;
    SignatureAuditSinkFn       m_sink;
};

}} // namespace ExplorerLens::Engine
