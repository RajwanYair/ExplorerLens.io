// ThumbnailAuditLog.h — Structured Decode/Cache Audit Trail
// Copyright (c) 2026 ExplorerLens Project
//
// Records every decode operation and cache hit/miss as a structured audit event
// for SIEM ingestion (SOC 2 / FedRAMP compliance). Events are append-only and
// can be flushed to a JSON-lines file or forwarded via ETW.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class AuditEventKind : uint8_t {
    DECODE_START   = 0,
    DECODE_SUCCESS = 1,
    DECODE_FAILURE = 2,
    CACHE_HIT      = 3,
    CACHE_MISS     = 4,
    CACHE_EVICT    = 5,
    POLICY_DENY    = 6,
};

struct AuditEvent {
    AuditEventKind kind         = AuditEventKind::DECODE_START;
    std::wstring   filePath;
    uint32_t       userId       = 0;
    uint32_t       tenantId     = 0;
    uint64_t       timestampMs  = 0;
    std::string    detail;       // Extra context (error message, format hint)
};

class ThumbnailAuditLog {
public:
    struct Config {
        uint32_t maxEvents      = 10'000;  // Ring buffer size
        bool     dropOldOnFull  = true;    // True = overwrite oldest
    };

    explicit ThumbnailAuditLog(const Config& cfg = {}) : m_cfg(cfg) {}

    void Record(const AuditEvent& event);
    void Flush();   // Resets the buffer without persisting

    uint32_t EventCount()     const { return static_cast<uint32_t>(m_events.size()); }
    uint32_t TotalRecorded()  const { return m_totalRecorded; }
    uint32_t TotalDropped()   const { return m_totalDropped; }

    // Query events by kind
    std::vector<AuditEvent> Query(AuditEventKind kind) const;

    const Config&            GetConfig() const { return m_cfg; }

private:
    Config               m_cfg;
    std::vector<AuditEvent> m_events;
    uint32_t             m_totalRecorded = 0;
    uint32_t             m_totalDropped  = 0;
};

}} // namespace ExplorerLens::Engine
