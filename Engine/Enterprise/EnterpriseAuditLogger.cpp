// EnterpriseAuditLogger.cpp — Enterprise-grade audit event logger
// Copyright (c) 2026 ExplorerLens Project
//
#include "EnterpriseAuditLogger.h"

namespace ExplorerLens { namespace Engine {

EnterpriseAuditLogger EnterpriseAuditLogger::s_instance;

EnterpriseAuditLogger::EnterpriseAuditLogger()  = default;
EnterpriseAuditLogger::~EnterpriseAuditLogger() { Shutdown(); }

EnterpriseAuditLogger& EnterpriseAuditLogger::Instance() noexcept { return s_instance; }

bool EnterpriseAuditLogger::Initialize(const std::string& logPath)
{
    if (logPath.empty())
        return false;
    m_logPath = logPath;
    m_stats   = {};
    m_open    = true;
    return true;
}

void EnterpriseAuditLogger::Shutdown()
{
    Flush();
    m_open = false;
}

bool EnterpriseAuditLogger::Log(const EnterpriseAuditLogEntry& event)
{
    if (!m_open)
        return false;
    ++m_stats.totalEvents;
    if (event.type == EnterpriseAuditEventType::SecurityViolation)
        ++m_stats.securityEvents;
    if (event.type == EnterpriseAuditEventType::PolicyChange)
        ++m_stats.policyEvents;
    return true;
}

void EnterpriseAuditLogger::Flush()
{
    // Would flush to file or Windows Event Log on real implementation.
}

}} // namespace ExplorerLens::Engine
