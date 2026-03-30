// GenerativeAuditTrail.h — Generative AI Operation Audit and Compliance Log
// Copyright (c) 2026 ExplorerLens Project
//
// Records all generative AI operations for compliance, reproducibility, and model
// governance, with configurable retention policies and JSON export capability.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class AuditEventType : uint8_t
{
    Generated,
    Upscaled,
    StyleTransferred,
    Inpainted,
    Moderated,
    PersonalisationApplied
};

enum class AuditRetentionPolicy : uint8_t
{
    SessionOnly,
    Days7,
    Days30,
    Days90,
    Permanent
};

struct GenerativeAuditEntry
{
    AuditEventType eventType    = AuditEventType::Generated;
    std::string    modelName;
    uint64_t       inputHash    = 0;
    uint64_t       outputHash   = 0;
    uint32_t       durationMs   = 0;
    std::string    operatorId;
    uint64_t       timestampMs  = 0;
};

class GenerativeAuditTrail
{
public:
    GenerativeAuditTrail() = default;
    ~GenerativeAuditTrail() = default;

    GenerativeAuditTrail(GenerativeAuditTrail const&)            = delete;
    GenerativeAuditTrail& operator=(GenerativeAuditTrail const&) = delete;
    GenerativeAuditTrail(GenerativeAuditTrail&&)                 = default;
    GenerativeAuditTrail& operator=(GenerativeAuditTrail&&)      = default;

    void Record(GenerativeAuditEntry const& entry);

    void SetRetentionPolicy(AuditRetentionPolicy policy);

    bool ExportToJson(std::string const& filePath) const;

    uint32_t Purge(uint64_t olderThanMs);

    [[nodiscard]] std::vector<GenerativeAuditEntry>
        Query(AuditEventType type, uint64_t sinceMs) const;

    [[nodiscard]] AuditRetentionPolicy GetRetentionPolicy() const;

private:
    std::vector<GenerativeAuditEntry> m_entries;
    AuditRetentionPolicy              m_policy     = AuditRetentionPolicy::Days30;
    uint32_t                          m_maxEntries = 100000;
};

} // namespace Engine
} // namespace ExplorerLens
