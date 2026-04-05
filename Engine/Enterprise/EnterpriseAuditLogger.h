// EnterpriseAuditLogger.h — Enterprise-grade audit event logger
// Copyright (c) 2026 ExplorerLens Project
//
// Structured audit logging for enterprise deployments. Records thumbnail
// generation events, policy changes, plugin loads, and security events to
// Windows Event Log, syslog (Linux), or a configurable remote endpoint.
// Supports immutable append-only log trails for compliance (SOC2/ISO27001).
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class EnterpriseAuditEventType : uint8_t
{
    PolicyChange    = 0,
    ThumbnailAccess = 1,
    PluginLoad      = 2,
    SecurityViolation = 3,
    ConfigChange    = 4,
    UserAction      = 5,
};

struct EnterpriseAuditLogEntry
{
    EnterpriseAuditEventType type        = EnterpriseAuditEventType::UserAction;
    std::string    description;
    std::string    actor;
    uint64_t       timestampMs = 0;
    bool           sensitive   = false;
};

struct AuditLogStats
{
    uint64_t totalEvents   = 0;
    uint64_t securityEvents = 0;
    uint64_t policyEvents  = 0;
    bool     healthy       = true;
};

class EnterpriseAuditLogger
{
public:
    EnterpriseAuditLogger();
    ~EnterpriseAuditLogger();

    EnterpriseAuditLogger(const EnterpriseAuditLogger&)            = delete;
    EnterpriseAuditLogger& operator=(const EnterpriseAuditLogger&) = delete;

    bool             Initialize(const std::string& logPath);
    void             Shutdown();
    bool             Log(const EnterpriseAuditLogEntry& event);
    AuditLogStats    GetStats()     const noexcept { return m_stats; }
    uint64_t         EventCount()   const noexcept { return m_stats.totalEvents; }
    bool             IsOpen()       const noexcept { return m_open; }
    void             Flush();

    static EnterpriseAuditLogger& Instance() noexcept;

private:
    bool              m_open  = false;
    std::string       m_logPath;
    AuditLogStats     m_stats;
    static EnterpriseAuditLogger s_instance;
};

}} // namespace ExplorerLens::Engine
