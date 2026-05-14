// GenerativeAuditTrail.cpp — Generative AI Audit Trail
// Copyright (c) 2026 ExplorerLens Project
//
#include "GenerativeAuditTrail.h"
#include <algorithm>

namespace ExplorerLens::Engine {

void GenerativeAuditTrail::Record(const GenerativeAuditEntry& entry)
{
    if (m_entries.size() >= m_maxEntries)
        m_entries.erase(m_entries.begin());
    m_entries.push_back(entry);
}
void GenerativeAuditTrail::SetRetentionPolicy(GenAuditRetention policy) { m_policy = policy; }
bool GenerativeAuditTrail::ExportToJson(const std::string&) const
{
    return false;
}
uint32_t GenerativeAuditTrail::Purge(uint64_t olderThanMs)
{
    auto before = m_entries.size();
    m_entries.erase(
        std::remove_if(m_entries.begin(), m_entries.end(),
            [olderThanMs](const GenerativeAuditEntry& e) { return e.timestampMs < olderThanMs; }),
        m_entries.end());
    return static_cast<uint32_t>(before - m_entries.size());
}
std::vector<GenerativeAuditEntry> GenerativeAuditTrail::Query(GenAuditEventType type, uint64_t sinceMs) const
{
    std::vector<GenerativeAuditEntry> result;
    for (const auto& e : m_entries) {
        if (e.eventType == type && e.timestampMs >= sinceMs)
            result.push_back(e);
    }
    return result;
}
GenAuditRetention GenerativeAuditTrail::GetRetentionPolicy() const
{
    return m_policy;
}

}  // namespace ExplorerLens::Engine
