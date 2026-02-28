#pragma once
// AuditTrailLogger.h — Audit Trail Logger
// Structured, tamper-evident audit logging for enterprise compliance —
// logs all thumbnail operations with ISO 27001/SOC 2 compatible fields.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Audit event severity
enum class AuditSeverity : uint8_t {
 Info = 0,
 Warning,
 Error,
 Critical,
 SecurityEvent,
 COUNT
};

/// Audit event category
enum class AuditEventCategory : uint8_t {
 FileAccess = 0,
 ThumbnailGenerate,
 CacheOperation,
 PolicyEvaluation,
 PluginLoad,
 ConfigChange,
 SecurityViolation,
 SystemHealth,
 COUNT
};

struct AuditEvent {
 uint64_t eventId = 0;
 AuditSeverity severity = AuditSeverity::Info;
 AuditEventCategory category = AuditEventCategory::FileAccess;
 const wchar_t *description = nullptr;
 const wchar_t *filePath = nullptr;
 const wchar_t *userName = nullptr;
 uint64_t timestampTicks = 0;
 uint32_t processId = 0;
 uint32_t threadId = 0;
 bool success = true;
};

struct AuditConfig {
 bool enabled = true;
 bool logToFile = true;
 bool logToETW = true;
 bool includeFilePaths = true;
 bool hashSensitiveFields = false;
 uint32_t maxLogSizeMB = 100;
 uint32_t retentionDays = 90;
};

class AuditTrailLogger {
public:
 static constexpr size_t SeverityCount() {
 return static_cast<size_t>(AuditSeverity::COUNT);
 }
 static constexpr size_t CategoryCount() {
 return static_cast<size_t>(AuditEventCategory::COUNT);
 }

 static const wchar_t *SeverityName(AuditSeverity s) {
 switch (s) {
 case AuditSeverity::Info:
 return L"Info";
 case AuditSeverity::Warning:
 return L"Warning";
 case AuditSeverity::Error:
 return L"Error";
 case AuditSeverity::Critical:
 return L"Critical";
 case AuditSeverity::SecurityEvent:
 return L"Security Event";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *CategoryName(AuditEventCategory c) {
 switch (c) {
 case AuditEventCategory::FileAccess:
 return L"File Access";
 case AuditEventCategory::ThumbnailGenerate:
 return L"Thumbnail Generate";
 case AuditEventCategory::CacheOperation:
 return L"Cache Operation";
 case AuditEventCategory::PolicyEvaluation:
 return L"Policy Evaluation";
 case AuditEventCategory::PluginLoad:
 return L"Plugin Load";
 case AuditEventCategory::ConfigChange:
 return L"Config Change";
 case AuditEventCategory::SecurityViolation:
 return L"Security Violation";
 case AuditEventCategory::SystemHealth:
 return L"System Health";
 default:
 return L"Unknown";
 }
 }
};

} // namespace Engine
} // namespace ExplorerLens
