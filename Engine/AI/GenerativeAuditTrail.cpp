// GenerativeAuditTrail.cpp — Generative AI Audit Trail
// Copyright (c) 2026 ExplorerLens Project
//
#include "GenerativeAuditTrail.h"

namespace ExplorerLens::Engine {

void GenerativeAuditTrail::Record(const GenerativeAuditEntry&) {}
void GenerativeAuditTrail::SetRetentionPolicy(GenAuditRetention) {}
bool GenerativeAuditTrail::ExportToJson(const std::string&) const { return false; }
uint32_t GenerativeAuditTrail::Purge(uint64_t) { return 0; }
std::vector<GenerativeAuditEntry> GenerativeAuditTrail::Query(GenAuditEventType, uint64_t) const { return {}; }
GenAuditRetention GenerativeAuditTrail::GetRetentionPolicy() const { return {}; }

} // namespace ExplorerLens::Engine
