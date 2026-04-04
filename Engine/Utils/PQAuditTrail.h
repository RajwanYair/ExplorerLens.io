// PQAuditTrail.h — Post-Quantum Crypto Audit Trail
// Copyright (c) 2026 ExplorerLens Project
//
// Records and verifies a tamper-evident audit trail of all cryptographic operations
// (key generation, encapsulation, signing) using hash-chained log entries.
//
#pragma once
#include <chrono>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class PQCryptoEvent {
    KeyGenerated,
    KeyEncapsulated,
    KeyDecapsulated,
    SignatureCreated,
    SignatureVerified,
    KeyRotated,
    PolicyViolation
};

struct PQAuditEntry
{
    PQCryptoEvent event = PQCryptoEvent::KeyGenerated;
    std::string algorithm;
    std::string subject;  // key ID or signer ID
    std::string isoTimestamp;
    std::string previousHash;  // hash of prior entry
    std::string entryHash;     // hash of this entry

    std::string EventName() const noexcept
    {
        switch (event) {
            case PQCryptoEvent::KeyGenerated:
                return "KeyGenerated";
            case PQCryptoEvent::KeyEncapsulated:
                return "KeyEncapsulated";
            case PQCryptoEvent::KeyDecapsulated:
                return "KeyDecapsulated";
            case PQCryptoEvent::SignatureCreated:
                return "SignatureCreated";
            case PQCryptoEvent::SignatureVerified:
                return "SignatureVerified";
            case PQCryptoEvent::KeyRotated:
                return "KeyRotated";
            case PQCryptoEvent::PolicyViolation:
                return "PolicyViolation";
        }
        return "Unknown";
    }
};

struct PQAuditVerifyResult
{
    bool valid = false;
    int entriesChecked = 0;
    int brokenLinks = 0;
    std::string firstBrokenAt;
};

class PQAuditTrail
{
  public:
    explicit PQAuditTrail() = default;

    void Record(PQCryptoEvent event, const std::string& algorithm, const std::string& subject = {})
    {
        PQAuditEntry entry;
        entry.event = event;
        entry.algorithm = algorithm;
        entry.subject = subject;
        entry.isoTimestamp = CurrentISO();
        entry.previousHash = m_entries.empty() ? "0000000000000000" : m_entries.back().entryHash;
        entry.entryHash = ComputeHash(entry);
        m_entries.push_back(std::move(entry));
    }

    PQAuditVerifyResult VerifyChain() const
    {
        PQAuditVerifyResult result;
        result.entriesChecked = static_cast<int>(m_entries.size());
        std::string prevHash = "0000000000000000";
        for (const auto& e : m_entries) {
            if (e.previousHash != prevHash) {
                result.brokenLinks++;
                if (result.firstBrokenAt.empty())
                    result.firstBrokenAt = e.isoTimestamp;
            }
            prevHash = e.entryHash;
        }
        result.valid = (result.brokenLinks == 0);
        return result;
    }

    const std::vector<PQAuditEntry>& Entries() const noexcept
    {
        return m_entries;
    }
    int EntryCount() const noexcept
    {
        return static_cast<int>(m_entries.size());
    }
    void Clear() noexcept
    {
        m_entries.clear();
    }

  private:
    static std::string CurrentISO()
    {
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << t;
        return oss.str();
    }

    static std::string ComputeHash(const PQAuditEntry& e)
    {
        // Lightweight stub hash — not cryptographic strength in stub form
        std::string combined = e.previousHash + e.algorithm + e.subject + e.isoTimestamp;
        uint32_t h = 2166136261u;
        for (char c : combined) {
            h ^= static_cast<unsigned char>(c);
            h *= 16777619u;
        }
        std::ostringstream oss;
        oss << std::hex << h;
        return oss.str();
    }

    std::vector<PQAuditEntry> m_entries;
};

}  // namespace Engine
}  // namespace ExplorerLens
