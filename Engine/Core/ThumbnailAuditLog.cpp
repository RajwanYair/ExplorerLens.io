// ThumbnailAuditLog.cpp — Structured Decode/Cache Audit Trail
// Copyright (c) 2026 ExplorerLens Project
//
#include "ThumbnailAuditLog.h"
#include <algorithm>

namespace ExplorerLens { namespace Engine {

void ThumbnailAuditLog::Record(const AuditEvent& event)
{
    ++m_totalRecorded;
    if (m_events.size() >= m_cfg.maxEvents) {
        if (m_cfg.dropOldOnFull) {
            m_events.erase(m_events.begin());
            ++m_totalDropped;
        } else {
            ++m_totalDropped;
            return;
        }
    }
    m_events.push_back(event);
}

void ThumbnailAuditLog::Flush()
{
    m_events.clear();
}

std::vector<AuditEvent> ThumbnailAuditLog::Query(AuditEventKind kind) const
{
    std::vector<AuditEvent> result;
    for (const auto& ev : m_events)
        if (ev.kind == kind) result.push_back(ev);
    return result;
}

}} // namespace ExplorerLens::Engine
