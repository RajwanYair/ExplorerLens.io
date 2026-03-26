// AuditLogger.h — Security Event Audit Log (ETW + Structured JSON)
// Copyright (c) 2026 ExplorerLens Project
//
// Records security-relevant events (plugin loads, sandbox escapes, decode
// failures, policy overrides) to both Windows ETW and a structured JSON
// logfile for SIEM ingestion.
//
#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace ExplorerLens {
namespace Engine {

enum class AuditEventId : uint32_t {
    // Plugin lifecycle
    PluginLoaded          = 1001,
    PluginVerifyFailed    = 1002,
    PluginUnloaded        = 1003,
    PluginSandboxTimeout  = 1004,

    // Decode security
    InputSanitizeFailed   = 2001,
    SandboxEscape         = 2002,  // CRITICAL
    DecodeAnomalyDetected = 2003,
    OversizedInputRejected= 2004,

    // Policy
    PolicyOverride        = 3001,
    FIPSModeActivated     = 3002,
    FIPSViolation         = 3003,

    // Auth
    RegistrationAllowed   = 4001,
    RegistrationBlocked   = 4002,
    IntegrityCheckFailed  = 4003,

    // Updates
    UpdateDownloaded      = 5001,
    UpdateApplied         = 5002,
    UpdateRolledBack      = 5003,
};

enum class AuditSeverity : uint8_t {
    Informational = 0,
    Warning       = 1,
    Error         = 2,
    Critical      = 3,
};

struct AuditEvent {
    AuditEventId    id;
    AuditSeverity   severity;
    std::string     message;
    std::string     detail;    // JSON object string with event-specific fields
    std::wstring    filePath;  // associated file, if any
    std::string     pluginName;
    uint32_t        processId{0};
    uint32_t        threadId{0};
};

class AuditLogger {
public:
    struct Config {
        std::wstring jsonLogPath;     // empty = %LOCALAPPDATA%\ExplorerLens\audit.log
        bool         enableETW{true};
        bool         enableJsonLog{true};
        uint32_t     maxLogSizeMB{50};
        bool         rotateOnSize{true};
        bool         flushOnCritical{true};
    };

    static AuditLogger& Instance() noexcept {
        static AuditLogger s_inst;
        return s_inst;
    }

    void Configure(Config cfg);
    void Open();
    void Close();

    void Log(AuditEvent evt);

    void LogPluginLoad(std::string_view name, bool success, std::string_view thumbprint = {});
    void LogSanitizeFailure(std::wstring_view path, std::string_view reason);
    void LogPolicyOverride(std::string_view policyName, std::string_view oldVal, std::string_view newVal);
    void LogIntegrityFailure(std::wstring_view dllPath, std::string_view detail);

private:
    AuditLogger() = default;

    void WriteJsonEntry(const AuditEvent& evt);
    void EmitEtwEvent(const AuditEvent& evt) noexcept;
    void RotateIfNeeded();

    Config        m_cfg;
    void*         m_logFile{nullptr};  // FILE*
    bool          m_open{false};
};

// Convenience macro
#define AUDIT_LOG(id, sev, msg, detail) \
    ::ExplorerLens::Engine::AuditLogger::Instance().Log({ \
        ::ExplorerLens::Engine::AuditEventId::id, \
        ::ExplorerLens::Engine::AuditSeverity::sev, \
        (msg), (detail), {}, {}, ::GetCurrentProcessId(), ::GetCurrentThreadId() })

} // namespace Engine
} // namespace ExplorerLens
