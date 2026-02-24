#pragma once
//==============================================================================
// ExplorerLens — Observability & Structured Logging Pipeline
// ETW provider, JSON-lines logger, diagnostics export, privacy-safe
// path hashing, request lifecycle tracing, diagnostic bundle builder.
//==============================================================================

#ifndef EXPLORERLENS_OBSERVABILITY_PIPELINE_H
#define EXPLORERLENS_OBSERVABILITY_PIPELINE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <sstream>
#include <functional>
#include <algorithm>
#include <cstdint>
#include <array>
#include <iomanip>

namespace ExplorerLens { namespace Engine { namespace Observability {

//==============================================================================
// ETW Event Definitions
//==============================================================================

// ETW Event IDs matching OBSERVABILITY_SPEC_V1.md
enum class ETWEventId : uint16_t
{
    RequestStart       = 100,
    RequestStop        = 101,
    CacheHit           = 200,
    CacheMiss          = 201,
    CacheEvict         = 202,
    DecodeStart        = 300,
    DecodeStop         = 301,
    DecodeFail         = 302,
    CrashCaught        = 400,
    CircuitBreakerOpen = 401,
    PluginLoad         = 500,
    PluginUnload       = 501,
    PluginError        = 502,
    MemoryPressure     = 600,
    GPUFallback        = 700
};

inline const char* ETWEventName(ETWEventId id)
{
    switch (id) {
        case ETWEventId::RequestStart:       return "RequestStart";
        case ETWEventId::RequestStop:        return "RequestStop";
        case ETWEventId::CacheHit:           return "CacheHit";
        case ETWEventId::CacheMiss:          return "CacheMiss";
        case ETWEventId::CacheEvict:         return "CacheEvict";
        case ETWEventId::DecodeStart:        return "DecodeStart";
        case ETWEventId::DecodeStop:         return "DecodeStop";
        case ETWEventId::DecodeFail:         return "DecodeFail";
        case ETWEventId::CrashCaught:        return "CrashCaught";
        case ETWEventId::CircuitBreakerOpen: return "CircuitBreakerOpen";
        case ETWEventId::PluginLoad:         return "PluginLoad";
        case ETWEventId::PluginUnload:       return "PluginUnload";
        case ETWEventId::PluginError:        return "PluginError";
        case ETWEventId::MemoryPressure:     return "MemoryPressure";
        case ETWEventId::GPUFallback:        return "GPUFallback";
    }
    return "Unknown";
}

// ETW Provider GUID: {3B2F8A9C-D1E7-4F5A-B6C2-8D9E0F1A2B3C}
struct ETWProviderConfig
{
    static constexpr const char* ProviderName = "ExplorerLens-Engine-Core";
    static constexpr const char* ProviderGUID = "{3B2F8A9C-D1E7-4F5A-B6C2-8D9E0F1A2B3C}";

    bool enabled = true;
    uint8_t level = 4;  // 1=Critical, 2=Error, 3=Warning, 4=Info, 5=Verbose

    static constexpr size_t TotalEventTypes = 15;
};

//==============================================================================
// Log Level & Structured Log Entry
//==============================================================================

enum class LogLevel : uint8_t
{
    Trace    = 0,
    Debug    = 1,
    Info     = 2,
    Warning  = 3,
    Error    = 4,
    Critical = 5
};

inline const char* LogLevelName(LogLevel l)
{
    switch (l) {
        case LogLevel::Trace:    return "TRACE";
        case LogLevel::Debug:    return "DEBUG";
        case LogLevel::Info:     return "INFO";
        case LogLevel::Warning:  return "WARN";
        case LogLevel::Error:    return "ERROR";
        case LogLevel::Critical: return "CRITICAL";
    }
    return "UNKNOWN";
}

struct LogEntry
{
    std::string timestamp;
    LogLevel    level = LogLevel::Info;
    std::string component;
    std::string message;
    std::unordered_map<std::string, std::string> fields;
    uint64_t    requestId = 0;
    uint32_t    threadId = 0;

    // Render as JSON-lines format
    std::string ToJSON() const
    {
        std::ostringstream ss;
        ss << "{\"ts\":\"" << timestamp << "\""
           << ",\"level\":\"" << LogLevelName(level) << "\""
           << ",\"component\":\"" << component << "\""
           << ",\"msg\":\"" << message << "\"";
        if (requestId > 0)
            ss << ",\"requestId\":" << requestId;
        if (threadId > 0)
            ss << ",\"threadId\":" << threadId;
        for (auto& [k, v] : fields)
            ss << ",\"" << k << "\":\"" << v << "\"";
        ss << "}";
        return ss.str();
    }

    // Render as human-readable text
    std::string ToText() const
    {
        std::ostringstream ss;
        ss << "[" << timestamp << "] "
           << LogLevelName(level) << " "
           << "[" << component << "] "
           << message;
        return ss.str();
    }
};

//==============================================================================
// JSON-Lines Logger — file-based fallback for non-ETW environments
//==============================================================================

class JSONLinesLogger
{
public:
    explicit JSONLinesLogger(const std::string& logPath = "")
        : logPath_(logPath) {}

    void SetLogPath(const std::string& path) { logPath_ = path; }
    std::string LogPath() const { return logPath_; }

    void Log(const LogEntry& entry)
    {
        if (entry.level < minLevel_) return;
        entries_.push_back(entry);
    }

    void SetMinLevel(LogLevel level) { minLevel_ = level; }
    LogLevel MinLevel() const { return minLevel_; }

    size_t EntryCount() const { return entries_.size(); }

    // Get all entries as JSON-lines string
    std::string Flush() const
    {
        std::ostringstream ss;
        for (auto& e : entries_)
            ss << e.ToJSON() << "\n";
        return ss.str();
    }

    // Get entries filtered by level
    std::vector<LogEntry> GetByLevel(LogLevel level) const
    {
        std::vector<LogEntry> result;
        for (auto& e : entries_)
            if (e.level == level) result.push_back(e);
        return result;
    }

    // Get entries filtered by component
    std::vector<LogEntry> GetByComponent(const std::string& component) const
    {
        std::vector<LogEntry> result;
        for (auto& e : entries_)
            if (e.component == component) result.push_back(e);
        return result;
    }

    void Clear() { entries_.clear(); }

    // Stats
    size_t ErrorCount() const
    {
        return static_cast<size_t>(std::count_if(entries_.begin(), entries_.end(),
            [](const auto& e) { return e.level >= LogLevel::Error; }));
    }

    size_t WarningCount() const
    {
        return static_cast<size_t>(std::count_if(entries_.begin(), entries_.end(),
            [](const auto& e) { return e.level == LogLevel::Warning; }));
    }

private:
    std::string logPath_;
    LogLevel    minLevel_ = LogLevel::Info;
    std::vector<LogEntry> entries_;
};

//==============================================================================
// Privacy — path hashing for ETW in non-Verbose mode
//==============================================================================

class PrivacyFilter
{
public:
    enum class Mode { Hash, Verbose };

    explicit PrivacyFilter(Mode mode = Mode::Hash) : mode_(mode) {}

    void SetMode(Mode mode) { mode_ = mode; }
    Mode GetMode() const { return mode_; }

    // Hash a file path to a privacy-safe representation
    std::string FilterPath(const std::string& path) const
    {
        if (mode_ == Mode::Verbose)
            return path;  // Full path in verbose mode

        // Simple hash: FNV-1a 32-bit
        uint32_t hash = 2166136261u;
        for (char c : path) {
            hash ^= static_cast<uint32_t>(c);
            hash *= 16777619u;
        }

        std::ostringstream ss;
        ss << "path:" << std::hex << std::setw(8) << std::setfill('0') << hash;
        return ss.str();
    }

    // Extract just the filename (less identifying than full path)
    std::string FilterToFilename(const std::string& path) const
    {
        auto pos = path.find_last_of("\\/");
        if (pos == std::string::npos) return path;
        return path.substr(pos + 1);
    }

    bool IsVerbose() const { return mode_ == Mode::Verbose; }
};

//==============================================================================
// Request Lifecycle Tracer — tracks a single thumbnail request
//==============================================================================

struct RequestTrace
{
    uint64_t    requestId = 0;
    std::string filePath;
    std::string format;
    std::string decoder;

    // Timing (milliseconds)
    double detectMs  = 0.0;
    double decodeMs  = 0.0;
    double resizeMs  = 0.0;
    double cacheMs   = 0.0;
    double marshalMs = 0.0;
    double totalMs   = 0.0;

    bool   cacheHit      = false;
    bool   succeeded     = true;
    bool   gpuAccelerated = false;
    std::string errorMessage;

    double PipelineMs() const
    {
        return detectMs + decodeMs + resizeMs + cacheMs + marshalMs;
    }

    std::string ToJSON() const
    {
        std::ostringstream ss;
        ss << "{\"requestId\":" << requestId
           << ",\"format\":\"" << format << "\""
           << ",\"decoder\":\"" << decoder << "\""
           << ",\"totalMs\":" << totalMs
           << ",\"cacheHit\":" << (cacheHit ? "true" : "false")
           << ",\"succeeded\":" << (succeeded ? "true" : "false")
           << ",\"gpu\":" << (gpuAccelerated ? "true" : "false")
           << "}";
        return ss.str();
    }
};

class RequestTracer
{
public:
    void RecordTrace(const RequestTrace& trace)
    {
        traces_.push_back(trace);
    }

    size_t TraceCount() const { return traces_.size(); }

    double AverageLatencyMs() const
    {
        if (traces_.empty()) return 0.0;
        double total = 0.0;
        for (auto& t : traces_) total += t.totalMs;
        return total / traces_.size();
    }

    double P95LatencyMs() const
    {
        if (traces_.empty()) return 0.0;
        std::vector<double> latencies;
        for (auto& t : traces_) latencies.push_back(t.totalMs);
        std::sort(latencies.begin(), latencies.end());
        size_t idx = static_cast<size_t>(latencies.size() * 0.95);
        if (idx >= latencies.size()) idx = latencies.size() - 1;
        return latencies[idx];
    }

    double CacheHitRate() const
    {
        if (traces_.empty()) return 0.0;
        size_t hits = 0;
        for (auto& t : traces_)
            if (t.cacheHit) ++hits;
        return (static_cast<double>(hits) / traces_.size()) * 100.0;
    }

    double SuccessRate() const
    {
        if (traces_.empty()) return 100.0;
        size_t ok = 0;
        for (auto& t : traces_)
            if (t.succeeded) ++ok;
        return (static_cast<double>(ok) / traces_.size()) * 100.0;
    }

    size_t ErrorCount() const
    {
        return static_cast<size_t>(std::count_if(traces_.begin(), traces_.end(),
            [](const auto& t) { return !t.succeeded; }));
    }

    void Clear() { traces_.clear(); }

private:
    std::vector<RequestTrace> traces_;
};

//==============================================================================
// Diagnostic Bundle Builder — "Export Diagnostics" feature
//==============================================================================

struct SystemInfo
{
    std::string osVersion      = "Windows 11 24H2";
    std::string cpuModel       = "Unknown";
    std::string gpuModel       = "Unknown";
    uint64_t    ramMB          = 0;
    uint64_t    vramMB         = 0;
    std::string explorerLensVersion = "7.0.0";

    std::string ToJSON() const
    {
        std::ostringstream ss;
        ss << "{\"os\":\"" << osVersion << "\""
           << ",\"cpu\":\"" << cpuModel << "\""
           << ",\"gpu\":\"" << gpuModel << "\""
           << ",\"ramMB\":" << ramMB
           << ",\"vramMB\":" << vramMB
           << ",\"version\":\"" << explorerLensVersion << "\""
           << "}";
        return ss.str();
    }
};

struct DiagnosticBundle
{
    SystemInfo         systemInfo;
    std::string        registryDump;
    std::string        configDump;
    std::vector<LogEntry> recentLogs;
    std::vector<RequestTrace> recentTraces;
    std::string        decoderStatus;
    std::string        pluginStatus;

    size_t SectionCount() const
    {
        size_t n = 1;  // system info always present
        if (!registryDump.empty()) ++n;
        if (!configDump.empty()) ++n;
        if (!recentLogs.empty()) ++n;
        if (!recentTraces.empty()) ++n;
        if (!decoderStatus.empty()) ++n;
        if (!pluginStatus.empty()) ++n;
        return n;
    }

    std::string GenerateReport() const
    {
        std::ostringstream ss;
        ss << "# ExplorerLens Diagnostic Report\n\n";
        ss << "## System Info\n" << systemInfo.ToJSON() << "\n\n";

        if (!decoderStatus.empty())
            ss << "## Decoder Status\n" << decoderStatus << "\n\n";
        if (!pluginStatus.empty())
            ss << "## Plugin Status\n" << pluginStatus << "\n\n";

        ss << "## Recent Logs (" << recentLogs.size() << " entries)\n";
        for (auto& log : recentLogs)
            ss << log.ToText() << "\n";

        ss << "\n## Recent Traces (" << recentTraces.size() << " requests)\n";
        for (auto& t : recentTraces)
            ss << t.ToJSON() << "\n";

        return ss.str();
    }
};

class DiagnosticBundleBuilder
{
public:
    void SetSystemInfo(const SystemInfo& info) { bundle_.systemInfo = info; }
    void SetRegistryDump(const std::string& dump) { bundle_.registryDump = dump; }
    void SetConfigDump(const std::string& config) { bundle_.configDump = config; }
    void AddLog(const LogEntry& entry) { bundle_.recentLogs.push_back(entry); }
    void AddTrace(const RequestTrace& trace) { bundle_.recentTraces.push_back(trace); }
    void SetDecoderStatus(const std::string& status) { bundle_.decoderStatus = status; }
    void SetPluginStatus(const std::string& status) { bundle_.pluginStatus = status; }

    const DiagnosticBundle& Build() const { return bundle_; }

    size_t LogCount() const { return bundle_.recentLogs.size(); }
    size_t TraceCount() const { return bundle_.recentTraces.size(); }
    size_t SectionCount() const { return bundle_.SectionCount(); }

private:
    DiagnosticBundle bundle_;
};

}}} // namespace ExplorerLens::Engine::Observability

#endif // EXPLORERLENS_OBSERVABILITY_PIPELINE_H


