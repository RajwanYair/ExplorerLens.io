// StructuredErrorDomain.h — Typed error domain system with severity and chaining
// Copyright (c) 2026 ExplorerLens Project
//
// Provides StructuredErrorDomain enum, ErrorSeverity enum, StructuredError class
// with source location, inner error chain, and human-readable formatting.
//
#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/// Typed error domain — identifies the subsystem that originated the error
enum class StructuredErrorDomain : uint8_t {
    Decode   = 0,
    Pipeline = 1,
    Cache    = 2,
    GPU      = 3,
    Plugin   = 4,
    IO       = 5,
    Memory   = 6,
    Network  = 7,
    Config   = 8,
    Security = 9,
    COUNT    = 10
};

/// Human-readable name for a StructuredErrorDomain value
inline const char* StructuredErrorDomainName(StructuredErrorDomain d) noexcept {
    switch (d) {
    case StructuredErrorDomain::Decode:   return "Decode";
    case StructuredErrorDomain::Pipeline: return "Pipeline";
    case StructuredErrorDomain::Cache:    return "Cache";
    case StructuredErrorDomain::GPU:      return "GPU";
    case StructuredErrorDomain::Plugin:   return "Plugin";
    case StructuredErrorDomain::IO:       return "IO";
    case StructuredErrorDomain::Memory:   return "Memory";
    case StructuredErrorDomain::Network:  return "Network";
    case StructuredErrorDomain::Config:   return "Config";
    case StructuredErrorDomain::Security: return "Security";
    default:                              return "Unknown";
    }
}

/// Severity level for structured errors
enum class ErrorSeverity : uint8_t {
    Trace   = 0,
    Info    = 1,
    Warning = 2,
    Error   = 3,
    Fatal   = 4,
    COUNT   = 5
};

/// Human-readable name for an ErrorSeverity value
inline const char* ErrorSeverityName(ErrorSeverity s) noexcept {
    switch (s) {
    case ErrorSeverity::Trace:   return "Trace";
    case ErrorSeverity::Info:    return "Info";
    case ErrorSeverity::Warning: return "Warning";
    case ErrorSeverity::Error:   return "Error";
    case ErrorSeverity::Fatal:   return "Fatal";
    default:                     return "Unknown";
    }
}

/// Source location where an error was created (file, line, function)
struct ErrorSourceLocation {
    const char* fileName   = nullptr;
    uint32_t    lineNumber = 0;
    const char* functionName = nullptr;

    bool IsValid() const noexcept {
        return fileName != nullptr && lineNumber > 0;
    }
};

/// Macro to capture current source location
#define ELENS_SOURCE_LOCATION() \
    ::ExplorerLens::Engine::ErrorSourceLocation{ __FILE__, static_cast<uint32_t>(__LINE__), __FUNCTION__ }

/// A structured error with domain, severity, HRESULT code, source location,
/// message, and optional inner error chain for contextual wrapping.
class StructuredError {
public:
    StructuredError() = default;

    StructuredError(StructuredErrorDomain domain,
                    ErrorSeverity severity,
                    HRESULT code,
                    const std::string& message,
                    ErrorSourceLocation location = {})
        : m_domain(domain)
        , m_severity(severity)
        , m_code(code)
        , m_message(message)
        , m_location(location)
        , m_timestamp(std::chrono::steady_clock::now())
    {}

    /// Domain that originated this error
    StructuredErrorDomain GetDomain() const noexcept { return m_domain; }

    /// Severity level
    ErrorSeverity GetSeverity() const noexcept { return m_severity; }

    /// HRESULT error code
    HRESULT GetCode() const noexcept { return m_code; }

    /// Human-readable error message
    const std::string& GetMessage() const noexcept { return m_message; }

    /// Source location where the error was created
    const ErrorSourceLocation& GetLocation() const noexcept { return m_location; }

    /// Timestamp when the error was created
    std::chrono::steady_clock::time_point GetTimestamp() const noexcept { return m_timestamp; }

    /// Inner error in the chain (may be null)
    const StructuredError* GetInnerError() const noexcept { return m_innerError.get(); }

    /// Whether this error has an inner error chain
    bool HasInnerError() const noexcept { return m_innerError != nullptr; }

    /// Depth of the inner error chain (0 if no inner error)
    uint32_t GetChainDepth() const noexcept {
        uint32_t depth = 0;
        const StructuredError* current = m_innerError.get();
        while (current) {
            ++depth;
            current = current->m_innerError.get();
        }
        return depth;
    }

    /// Set the inner error (wraps the given error as the cause)
    void SetInnerError(const StructuredError& inner) {
        m_innerError = std::make_shared<StructuredError>(inner);
    }

    /// Format this error (and chain) to a human-readable string
    std::string FormatError() const {
        std::ostringstream oss;
        FormatErrorImpl(oss, 0);
        return oss.str();
    }

    /// True if the severity is Error or Fatal
    bool IsCritical() const noexcept {
        return m_severity == ErrorSeverity::Error ||
               m_severity == ErrorSeverity::Fatal;
    }

private:
    void FormatErrorImpl(std::ostringstream& oss, uint32_t indentLevel) const {
        std::string indent(indentLevel * 2, ' ');
        oss << indent << "[" << ErrorSeverityName(m_severity) << "] "
            << StructuredErrorDomainName(m_domain) << ": "
            << m_message;
        oss << " (HRESULT=0x" << std::hex << std::setw(8) << std::setfill('0')
            << static_cast<uint32_t>(m_code) << std::dec << ")";
        if (m_location.IsValid()) {
            oss << " at " << m_location.fileName
                << ":" << m_location.lineNumber;
            if (m_location.functionName) {
                oss << " in " << m_location.functionName;
            }
        }
        if (m_innerError) {
            oss << "\n" << indent << "  Caused by:\n";
            m_innerError->FormatErrorImpl(oss, indentLevel + 2);
        }
    }

    StructuredErrorDomain m_domain   = StructuredErrorDomain::Decode;
    ErrorSeverity         m_severity = ErrorSeverity::Error;
    HRESULT               m_code     = E_FAIL;
    std::string           m_message;
    ErrorSourceLocation   m_location = {};
    std::chrono::steady_clock::time_point m_timestamp;
    std::shared_ptr<StructuredError> m_innerError;
};

/// Convenience: wrap an existing error with additional context
class ErrorChain {
public:
    /// Wrap an inner error with new context at a higher domain/severity
    static StructuredError Wrap(const StructuredError& inner,
                                StructuredErrorDomain domain,
                                ErrorSeverity severity,
                                HRESULT code,
                                const std::string& contextMessage,
                                ErrorSourceLocation location = {}) {
        StructuredError outer(domain, severity, code, contextMessage, location);
        outer.SetInnerError(inner);
        return outer;
    }

    /// Wrap with same domain/severity, just adding a context message
    static StructuredError AddContext(const StructuredError& inner,
                                     const std::string& contextMessage,
                                     ErrorSourceLocation location = {}) {
        return Wrap(inner, inner.GetDomain(), inner.GetSeverity(),
                    inner.GetCode(), contextMessage, location);
    }
};

/// Format a StructuredError to a string (free function convenience)
inline std::string FormatError(const StructuredError& error) {
    return error.FormatError();
}

} // namespace Engine
} // namespace ExplorerLens
