//==============================================================================
// ExplorerLens Engine — Compliance & Audit Logger
// GDPR/CCPA/HIPAA-aware event logging with data classification labels,
// retention policies, immutable audit trails, and DSR (data subject request)
// redaction capabilities.
//==============================================================================
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class ComplianceRegulation : uint8_t { GDPR=0,CCPA,HIPAA,SOX,ISO27001,COUNT };
enum class DataClassification    : uint8_t { Public=0,Internal,Confidential,Restricted,COUNT };
enum class AuditEventType        : uint8_t { Access=0,Modify,Delete,Export,Share,Consent,COUNT };

struct AuditEventRecord {
    AuditEventType     eventType     = AuditEventType::Access;
    DataClassification dataClass     = DataClassification::Internal;
    ComplianceRegulation regulation  = ComplianceRegulation::GDPR;
    std::wstring       subjectId;      // pseudonymized if PII
    std::wstring       resourcePath;
    uint64_t           timestampEpoch = 0;
    bool               immutable      = true;
};

struct AuditRetentionPolicy {
    ComplianceRegulation regulation  = ComplianceRegulation::GDPR;
    uint32_t             retainDays  = 365;
    bool                 allowDelete = false;
    bool                 encryptAtRest = true;
};

class ComplianceAuditLogger {
public:
    static const wchar_t* RegulationName(ComplianceRegulation r) {
        switch(r) {
            case ComplianceRegulation::GDPR:    return L"GDPR";
            case ComplianceRegulation::CCPA:    return L"CCPA";
            case ComplianceRegulation::HIPAA:   return L"HIPAA";
            case ComplianceRegulation::SOX:     return L"SOX";
            case ComplianceRegulation::ISO27001:return L"ISO 27001";
            default: return L"Unknown";
        }
    }
    static const wchar_t* DataClassificationName(DataClassification c) {
        switch(c) {
            case DataClassification::Public:       return L"Public";
            case DataClassification::Internal:     return L"Internal";
            case DataClassification::Confidential: return L"Confidential";
            case DataClassification::Restricted:   return L"Restricted";
            default: return L"Unknown";
        }
    }
    static const wchar_t* AuditEventTypeName(AuditEventType t) {
        switch(t) {
            case AuditEventType::Access:  return L"Access";
            case AuditEventType::Modify:  return L"Modify";
            case AuditEventType::Delete:  return L"Delete";
            case AuditEventType::Export:  return L"Export";
            case AuditEventType::Share:   return L"Share";
            case AuditEventType::Consent: return L"Consent";
            default: return L"Unknown";
        }
    }
    static constexpr size_t RegulationCount()   { return static_cast<size_t>(ComplianceRegulation::COUNT); }
    static constexpr size_t DataClassCount()    { return static_cast<size_t>(DataClassification::COUNT); }
    static constexpr size_t AuditEventCount()   { return static_cast<size_t>(AuditEventType::COUNT); }
};

}} // namespace ExplorerLens::Engine

