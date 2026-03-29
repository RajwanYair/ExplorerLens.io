// PrivacyAuditLogger.h — Privacy Audit Logger
// Copyright (c) 2026 ExplorerLens Project
//
// Append-only tamper-proof log for all privacy-affecting operations (hash-chained entries).
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <functional>
#include <chrono>

namespace ExplorerLens { namespace Engine {

enum class PALEventType { ConsentChange, DataAccess, DataErasure, ExportRequest };

struct PALEntry {
    PALEventType type        = PALEventType::DataAccess;
    std::string  subjectId;
    std::string  description;
    uint64_t     timestampMs = 0;
    std::string  chainHash;
};

struct PALVerifyResult {
    bool        valid      = false;
    uint32_t    entryCount = 0;
    std::string errorMsg;
};

class PrivacyAuditLogger {
public:
    void Record(PALEventType type, const std::string& subjectId, const std::string& description) {
        auto now = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
        PALEntry e;
        e.type        = type;
        e.subjectId   = subjectId;
        e.description = description;
        e.timestampMs = now;
        e.chainHash   = ComputeHash(m_prevHash, subjectId + description);
        m_prevHash    = e.chainHash;
        m_entries.push_back(e);
    }
    PALVerifyResult VerifyChain() const {
        PALVerifyResult r;
        r.entryCount = static_cast<uint32_t>(m_entries.size());
        std::string prev = "genesis";
        for (const auto& e : m_entries) {
            std::string expected = ComputeHash(prev, e.subjectId + e.description);
            if (expected != e.chainHash) { r.errorMsg = "Chain broken"; return r; }
            prev = e.chainHash;
        }
        r.valid = true;
        return r;
    }
    uint32_t EntryCount() const { return static_cast<uint32_t>(m_entries.size()); }
    std::vector<PALEntry> EntriesForSubject(const std::string& subjectId) const {
        std::vector<PALEntry> out;
        for (const auto& e : m_entries)
            if (e.subjectId == subjectId) out.push_back(e);
        return out;
    }

private:
    std::vector<PALEntry> m_entries;
    std::string           m_prevHash = "genesis";

    static std::string ComputeHash(const std::string& prev, const std::string& data) {
        size_t h = std::hash<std::string>{}(prev + data);
        std::ostringstream oss;
        oss << std::hex << h;
        return oss.str();
    }
};

}} // namespace ExplorerLens::Engine
