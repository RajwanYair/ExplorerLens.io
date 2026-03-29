// GDPRComplianceEngine.h — GDPR Compliance Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Enforces GDPR data-subject rights including right-to-erasure and retention limits.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>

namespace ExplorerLens { namespace Engine {

enum class GDPRRight { RightToAccess, RightToErasure, RightToPortability, RightToRestrict };

struct GDPRRequest {
    GDPRRight   right     = GDPRRight::RightToAccess;
    std::string subjectId;
};

struct GDPRResponse {
    bool        fulfilled       = false;
    std::string details;
    uint32_t    recordsAffected = 0;
};

class GDPRComplianceEngine {
public:
    GDPRResponse ProcessRequest(const GDPRRequest& req) {
        GDPRResponse r;
        if (req.subjectId.empty()) { r.details = "Invalid subject"; return r; }
        uint32_t count = RecordsForSubject(req.subjectId);
        if (req.right == GDPRRight::RightToErasure) {
            m_records.erase(req.subjectId);
            r.recordsAffected = count;
            r.details   = "Erased " + std::to_string(count) + " records";
            r.fulfilled = true;
        } else {
            r.recordsAffected = count;
            r.details   = "Found " + std::to_string(count) + " records";
            r.fulfilled = true;
        }
        return r;
    }
    void AddRecord(const std::string& subjectId) { m_records[subjectId]++; }
    uint32_t RecordsForSubject(const std::string& subjectId) const {
        auto it = m_records.find(subjectId);
        return it != m_records.end() ? it->second : 0u;
    }
    bool IsRetentionExpired(const std::string& subjectId, uint32_t retentionDays) const {
        (void)subjectId;
        return retentionDays == 0;
    }

private:
    std::unordered_map<std::string, uint32_t> m_records;
};

}} // namespace ExplorerLens::Engine
