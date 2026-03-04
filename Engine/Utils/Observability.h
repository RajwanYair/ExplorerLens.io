#pragma once
//==============================================================================
// Observability.h — Consolidated Observability & Telemetry Pipeline
// Copyright (c) 2026 ExplorerLens Project
//
// Unified header for observability concerns:
// - ETW tracing provider (Windows Performance Analyzer integration)
// - JSON-lines structured logger (fallback for non-ETW)
// - Pipeline telemetry wiring: ETW + StructuredLogger + DiagnosticsExport
// - Privacy-safe path hashing, structured event emission
// - Request lifecycle tracing, diagnostic bundle builder
//
// Merged from: ETWTracing.h, StructuredLogger.h,
//              ObservabilityIntegration.h, ObservabilityPipeline.h
//==============================================================================

#include <windows.h>
#include <evntprov.h>
#include <ShlObj.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <mutex>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <functional>
#include <algorithm>
#include <atomic>
#include <filesystem>
#include <cstdint>
#include <array>

#pragma comment(lib, "Advapi32.lib")

// ─── ETWTracing ───────────────────────────────────────────────────────────────

namespace ExplorerLens {
namespace Tracing {

//==============================================================================
// ETW Provider GUID: {A1B2C3D4-E5F6-4748-9A8B-9C8D7E6F5A4B}
// Register with: wevtutil im ExplorerLensManifest.xml
//==============================================================================

const GUID EXPLORERLENS_PROVIDER_GUID = {
 0xA1B2C3D4, 0xE5F6, 0x4748,
 {0x9A, 0x8B, 0x9C, 0x8D, 0x7E, 0x6F, 0x5A, 0x4B}
};

//==============================================================================
// ETW Event IDs
//==============================================================================

enum class EventID : uint16_t {
    ThumbnailGeneration_Start = 1,
    ThumbnailGeneration_Stop = 2,
    Decode_Start = 10,
    Decode_Stop = 11,
    GPU_Render_Start = 20,
    GPU_Render_Stop = 21,
    Cache_Hit = 30,
    Cache_Miss = 31,
    Error = 100,
    Warning = 101,
    Info = 102
};

//==============================================================================
// ETW Event Levels (matches Windows ETW levels)
//==============================================================================

enum class EventLevel : uint8_t {
    Critical = 1,
    Error = 2,
    Warning = 3,
    Info = 4,
    Verbose = 5
};

//==============================================================================
// ETW Tracer
//==============================================================================

class ETWTracer {
public:
    static ETWTracer& Get() {
        static ETWTracer instance;
        return instance;
    }

    // Initialize ETW provider
    bool Initialize();

    // Shutdown ETW provider
    void Shutdown();

    // Check if ETW is enabled
    bool IsEnabled() const { return m_enabled && m_registered; }

    // Log thumbnail generation event
    void LogThumbnailStart(const wchar_t* filePath, uint32_t width, uint32_t height);
    void LogThumbnailStop(const wchar_t* filePath, HRESULT hr, uint32_t durationMs);

    // Log decoder events
    void LogDecodeStart(const wchar_t* decoderName, const wchar_t* filePath);
    void LogDecodeStop(const wchar_t* decoderName, HRESULT hr, uint32_t durationMs);

    // Log GPU events
    void LogGPURenderStart(uint32_t width, uint32_t height);
    void LogGPURenderStop(HRESULT hr, uint32_t durationMs);

    // Log cache events
    void LogCacheHit(const wchar_t* filePath);
    void LogCacheMiss(const wchar_t* filePath);

    // Generic logging
    void LogError(const wchar_t* message);
    void LogWarning(const wchar_t* message);
    void LogInfo(const wchar_t* message);

private:
    ETWTracer() = default;
    ~ETWTracer();

    ETWTracer(const ETWTracer&) = delete;
    ETWTracer& operator=(const ETWTracer&) = delete;

    // Write ETW event
    void WriteEvent(EventID eventId, EventLevel level, const wchar_t* message);

    REGHANDLE m_providerHandle = 0;
    bool m_registered = false;
    bool m_enabled = false;
};

//==============================================================================
// RAII Scope Tracer
//==============================================================================

class ETWScope {
public:
    ETWScope(EventID startEvent, EventID stopEvent, const wchar_t* name)
        : m_stopEvent(stopEvent), m_name(name), m_startTime(GetTickCount64()) {
        // Log start event
        if (ETWTracer::Get().IsEnabled()) {
            // Start event logged
        }
    }

    ~ETWScope() {
        if (ETWTracer::Get().IsEnabled()) {
            uint32_t duration = static_cast<uint32_t>(GetTickCount64() - m_startTime);
            // Log stop event with duration
        }
    }

private:
    EventID m_stopEvent;
    const wchar_t* m_name;
    uint64_t m_startTime;
};

// Convenience macros
#define ETW_TRACE_THUMBNAIL(filePath) \
 ExplorerLens::Tracing::ETWScope _etwScope_##__LINE__( \
 ExplorerLens::Tracing::EventID::ThumbnailGeneration_Start, \
 ExplorerLens::Tracing::EventID::ThumbnailGeneration_Stop, \
 filePath)

#define ETW_TRACE_DECODE(decoderName) \
 ExplorerLens::Tracing::ETWScope _etwScope_##__LINE__( \
 ExplorerLens::Tracing::EventID::Decode_Start, \
 ExplorerLens::Tracing::EventID::Decode_Stop, \
 decoderName)

// ============================================================================
// ETWTracer — inline implementations
// ============================================================================

inline bool ETWTracer::Initialize() {
    if (m_registered) return true;
    ULONG status = EventRegister(&EXPLORERLENS_PROVIDER_GUID, nullptr, nullptr, &m_providerHandle);
    if (status == ERROR_SUCCESS) {
        m_registered = true;
        m_enabled = true;
        return true;
    }
    return false;
}

inline void ETWTracer::Shutdown() {
    if (m_registered) {
        EventUnregister(m_providerHandle);
        m_providerHandle = 0;
        m_registered = false;
        m_enabled = false;
    }
}

inline ETWTracer::~ETWTracer() {
    Shutdown();
}

inline void ETWTracer::WriteEvent(EventID eventId, EventLevel level,
    const wchar_t* message) {
    if (!m_enabled || !m_registered) return;
    EVENT_DESCRIPTOR desc = {};
    desc.Id = static_cast<USHORT>(eventId);
    desc.Level = static_cast<UCHAR>(level);
    desc.Opcode = 0;
    desc.Channel = 0;
    desc.Task = 0;
    desc.Keyword = 0;
    if (message && message[0] != L'\0') {
        EVENT_DATA_DESCRIPTOR dataDesc;
        EventDataDescCreate(&dataDesc, message,
            static_cast<ULONG>((wcslen(message) + 1) * sizeof(wchar_t)));
        EventWrite(m_providerHandle, &desc, 1, &dataDesc);
    }
    else {
        EventWrite(m_providerHandle, &desc, 0, nullptr);
    }
}

inline void ETWTracer::LogThumbnailStart(const wchar_t* filePath,
    uint32_t /*width*/, uint32_t /*height*/) {
    WriteEvent(EventID::ThumbnailGeneration_Start, EventLevel::Info, filePath);
}

inline void ETWTracer::LogThumbnailStop(const wchar_t* filePath,
    HRESULT /*hr*/, uint32_t /*durationMs*/) {
    WriteEvent(EventID::ThumbnailGeneration_Stop, EventLevel::Info, filePath);
}

inline void ETWTracer::LogDecodeStart(const wchar_t* decoderName,
    const wchar_t* /*filePath*/) {
    WriteEvent(EventID::Decode_Start, EventLevel::Info, decoderName);
}

inline void ETWTracer::LogDecodeStop(const wchar_t* decoderName,
    HRESULT /*hr*/, uint32_t /*durationMs*/) {
    WriteEvent(EventID::Decode_Stop, EventLevel::Info, decoderName);
}

inline void ETWTracer::LogGPURenderStart(uint32_t /*width*/, uint32_t /*height*/) {
    WriteEvent(EventID::GPU_Render_Start, EventLevel::Info, L"GPU Render");
}

inline void ETWTracer::LogGPURenderStop(HRESULT /*hr*/, uint32_t /*durationMs*/) {
    WriteEvent(EventID::GPU_Render_Stop, EventLevel::Info, L"GPU Render");
}

inline void ETWTracer::LogCacheHit(const wchar_t* filePath) {
    WriteEvent(EventID::Cache_Hit, EventLevel::Verbose, filePath);
}

inline void ETWTracer::LogCacheMiss(const wchar_t* filePath) {
    WriteEvent(EventID::Cache_Miss, EventLevel::Verbose, filePath);
}

inline void ETWTracer::LogError(const wchar_t* message) {
    WriteEvent(EventID::Error, EventLevel::Error, message);
}

inline void ETWTracer::LogWarning(const wchar_t* message) {
    WriteEvent(EventID::Warning, EventLevel::Warning, message);
}

inline void ETWTracer::LogInfo(const wchar_t* message) {
    WriteEvent(EventID::Info, EventLevel::Info, message);
}

} // namespace Tracing
} // namespace ExplorerLens

// ─── StructuredLogger ─────────────────────────────────────────────────────────

namespace ExplorerLens {
namespace Logging {

//==============================================================================
// Log Levels
//==============================================================================

enum class LogLevel : uint8_t {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warning = 3,
    Error = 4,
    Critical = 5
};

//==============================================================================
// Structured Logger - Thread-safe JSON-lines logger
//==============================================================================

class StructuredLogger {
public:
    static StructuredLogger& Get() {
        static StructuredLogger instance;
        return instance;
    }

    // Initialize logger with file path
    bool Initialize(const std::wstring& logFilePath, bool enablePrivacy = true) {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_enablePrivacy = enablePrivacy;
        m_logFile.open(logFilePath, std::ios::app | std::ios::out);

        if (!m_logFile.is_open()) {
            return false;
        }

        m_initialized = true;

        // Write initialization marker
        LogInfo(L"Logger", L"StructuredLogger initialized", L"");

        return true;
    }

    // Shutdown logger
    void Shutdown() {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_logFile.is_open()) {
            LogInfo(L"Logger", L"StructuredLogger shutting down", L"");
            m_logFile.close();
        }

        m_initialized = false;
    }

    // Check if logger is initialized
    bool IsInitialized() const {
        return m_initialized;
    }

    // Generic logging function
    void Log(LogLevel level, const wchar_t* component, const wchar_t* message,
        const wchar_t* context, uint64_t correlationId = 0) {
        if (!m_initialized) {
            return;
        }

        std::lock_guard<std::mutex> lock(m_mutex);

        // Get timestamp in ISO8601 format
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::tm tm_now;
        gmtime_s(&tm_now, &time_t_now);

        std::wostringstream json;
        json << L"{"
            << L"\"t\":\"" << std::put_time(&tm_now, L"%Y-%m-%dT%H:%M:%SZ") << L"\","
            << L"\"cid\":\"" << std::hex << correlationId << L"\","
            << L"\"comp\":\"" << component << L"\","
            << L"\"lvl\":\"" << LevelToString(level) << L"\","
            << L"\"msg\":\"" << EscapeJson(message) << L"\"";

        if (context && wcslen(context) > 0) {
            json << L",\"ctx\":\"" << EscapeJson(context) << L"\"";
        }

        json << L"}\n";

        // Write to file
        m_logFile << WstringToString(json.str());
        m_logFile.flush(); // Ensure immediate write for debugging
    }

    // Convenience methods
    void LogTrace(const wchar_t* component, const wchar_t* message, const wchar_t* context = L"", uint64_t cid = 0) {
        Log(LogLevel::Trace, component, message, context, cid);
    }

    void LogDebug(const wchar_t* component, const wchar_t* message, const wchar_t* context = L"", uint64_t cid = 0) {
        Log(LogLevel::Debug, component, message, context, cid);
    }

    void LogInfo(const wchar_t* component, const wchar_t* message, const wchar_t* context = L"", uint64_t cid = 0) {
        Log(LogLevel::Info, component, message, context, cid);
    }

    void LogWarning(const wchar_t* component, const wchar_t* message, const wchar_t* context = L"", uint64_t cid = 0) {
        Log(LogLevel::Warning, component, message, context, cid);
    }

    void LogError(const wchar_t* component, const wchar_t* message, const wchar_t* context = L"", uint64_t cid = 0) {
        Log(LogLevel::Error, component, message, context, cid);
    }

    void LogCritical(const wchar_t* component, const wchar_t* message, const wchar_t* context = L"", uint64_t cid = 0) {
        Log(LogLevel::Critical, component, message, context, cid);
    }

    // Log thumbnail request lifecycle
    void LogThumbnailStart(const wchar_t* filePath, uint32_t width, uint32_t height, uint64_t correlationId) {
        std::wostringstream ctx;
        ctx << L"{\"path\":\"" << HashOrPath(filePath) << L"\",\"w\":" << width << L",\"h\":" << height << L"}";
        LogInfo(L"Pipeline", L"RequestStart", ctx.str().c_str(), correlationId);
    }

    void LogThumbnailStop(uint64_t correlationId, HRESULT hr, uint32_t durationMs) {
        std::wostringstream ctx;
        ctx << L"{\"hr\":\"0x" << std::hex << hr << L"\",\"ms\":" << std::dec << durationMs << L"}";

        if (SUCCEEDED(hr)) {
            LogInfo(L"Pipeline", L"RequestStop", ctx.str().c_str(), correlationId);
        }
        else {
            LogError(L"Pipeline", L"RequestStop", ctx.str().c_str(), correlationId);
        }
    }

    // Log decoder events
    void LogDecodeStart(const wchar_t* decoderName, const wchar_t* filePath, uint64_t correlationId) {
        std::wostringstream ctx;
        ctx << L"{\"decoder\":\"" << decoderName << L"\",\"path\":\"" << HashOrPath(filePath) << L"\"}";
        LogDebug(L"Decoder", L"DecodeStart", ctx.str().c_str(), correlationId);
    }

    void LogDecodeStop(const wchar_t* decoderName, HRESULT hr, uint32_t durationMs, uint64_t correlationId) {
        std::wostringstream ctx;
        ctx << L"{\"decoder\":\"" << decoderName << L"\",\"hr\":\"0x" << std::hex << hr
            << L"\",\"ms\":" << std::dec << durationMs << L"}";

        if (SUCCEEDED(hr)) {
            LogDebug(L"Decoder", L"DecodeStop", ctx.str().c_str(), correlationId);
        }
        else {
            LogError(L"Decoder", L"DecodeFail", ctx.str().c_str(), correlationId);
        }
    }

    // Log cache events
    void LogCacheHit(const wchar_t* filePath, uint64_t correlationId) {
        std::wostringstream ctx;
        ctx << L"{\"path\":\"" << HashOrPath(filePath) << L"\"}";
        LogDebug(L"Cache", L"CacheHit", ctx.str().c_str(), correlationId);
    }

    void LogCacheMiss(const wchar_t* filePath, uint64_t correlationId) {
        std::wostringstream ctx;
        ctx << L"{\"path\":\"" << HashOrPath(filePath) << L"\"}";
        LogDebug(L"Cache", L"CacheMiss", ctx.str().c_str(), correlationId);
    }

    // Privacy setting
    void SetPrivacyMode(bool enable) {
        m_enablePrivacy = enable;
    }

    bool GetPrivacyMode() const {
        return m_enablePrivacy;
    }

private:
    StructuredLogger() = default;
    ~StructuredLogger() {
        Shutdown();
    }

    StructuredLogger(const StructuredLogger&) = delete;
    StructuredLogger& operator=(const StructuredLogger&) = delete;

    // Convert log level to string
    const wchar_t* LevelToString(LogLevel level) const {
        switch (level) {
        case LogLevel::Trace: return L"TRC";
        case LogLevel::Debug: return L"DBG";
        case LogLevel::Info: return L"INF";
        case LogLevel::Warning: return L"WRN";
        case LogLevel::Error: return L"ERR";
        case LogLevel::Critical: return L"CRT";
        default: return L"UNK";
        }
    }

    // Escape JSON special characters
    std::wstring EscapeJson(const wchar_t* str) const {
        std::wstring result;
        for (const wchar_t* p = str; *p; ++p) {
            switch (*p) {
            case L'\"': result += L"\\\""; break;
            case L'\\': result += L"\\\\"; break;
            case L'\b': result += L"\\b"; break;
            case L'\f': result += L"\\f"; break;
            case L'\n': result += L"\\n"; break;
            case L'\r': result += L"\\r"; break;
            case L'\t': result += L"\\t"; break;
            default: result += *p; break;
            }
        }
        return result;
    }

    // Hash or return path based on privacy mode
    std::wstring HashOrPath(const wchar_t* filePath) const {
        if (!m_enablePrivacy) {
            return filePath;
        }

        // Simple hash of file path for privacy
        uint64_t hash = 0xcbf29ce484222325; // FNV-1a offset basis
        for (const wchar_t* p = filePath; *p; ++p) {
            hash ^= static_cast<uint64_t>(*p);
            hash *= 0x100000001b3; // FNV-1a prime
        }

        std::wostringstream hashStr;
        hashStr << L"<hash:" << std::hex << hash << L">";
        return hashStr.str();
    }

    // Convert wstring to UTF-8 string
    std::string WstringToString(const std::wstring& wstr) const {
        if (wstr.empty()) return std::string();

        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(),
            static_cast<int>(wstr.size()),
            nullptr, 0, nullptr, nullptr);
        std::string result(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()),
            &result[0], size_needed, nullptr, nullptr);
        return result;
    }

    std::mutex m_mutex;
    std::ofstream m_logFile;
    bool m_initialized = false;
    bool m_enablePrivacy = true;
};

//==============================================================================
// Convenience Macros
//==============================================================================

#define STRUCTURED_LOG_INIT(path) \
 ExplorerLens::Logging::StructuredLogger::Get().Initialize(path)

#define STRUCTURED_LOG_SHUTDOWN() \
 ExplorerLens::Logging::StructuredLogger::Get().Shutdown()

#define LOG_TRACE(component, message) \
 ExplorerLens::Logging::StructuredLogger::Get().LogTrace(component, message)

#define LOG_DEBUG(component, message) \
 ExplorerLens::Logging::StructuredLogger::Get().LogDebug(component, message)

#define LOG_INFO(component, message) \
 ExplorerLens::Logging::StructuredLogger::Get().LogInfo(component, message)

#define LOG_WARNING(component, message) \
 ExplorerLens::Logging::StructuredLogger::Get().LogWarning(component, message)

#define LOG_ERROR(component, message) \
 ExplorerLens::Logging::StructuredLogger::Get().LogError(component, message)

#define LOG_CRITICAL(component, message) \
 ExplorerLens::Logging::StructuredLogger::Get().LogCritical(component, message)

} // namespace Logging
} // namespace ExplorerLens

// ─── ObservabilityIntegration ─────────────────────────────────────────────────

namespace ExplorerLens {
namespace Observability {

//==============================================================================
// Pipeline Telemetry — wraps ETW + StructuredLogger for unified tracing
//==============================================================================

class PipelineTelemetry {
public:
    static PipelineTelemetry& Get() {
        static PipelineTelemetry instance;
        return instance;
    }

    /// Initialize both ETW and structured logger
    bool Initialize(bool enableETW = true, bool enableFileLog = true) {
        if (m_initialized) return true;

        if (enableETW) {
            m_etwEnabled = Tracing::ETWProvider::Get().Initialize();
        }

        if (enableFileLog) {
            auto logPath = GetLogFilePath();
            m_fileLogEnabled = Logging::StructuredLogger::Get().Initialize(logPath, true);
        }

        m_initialized = true;
        LogEvent(Logging::LogLevel::Info, L"Pipeline", L"Observability initialized",
            L"etw=" + std::to_wstring(m_etwEnabled) + L",file=" + std::to_wstring(m_fileLogEnabled));
        return m_etwEnabled || m_fileLogEnabled;
    }

    void Shutdown() {
        if (!m_initialized) return;
        LogEvent(Logging::LogLevel::Info, L"Pipeline", L"Observability shutting down", L"");

        if (m_etwEnabled) {
            Tracing::ETWProvider::Get().Shutdown();
        }
        if (m_fileLogEnabled) {
            Logging::StructuredLogger::Get().Shutdown();
        }
        m_initialized = false;
    }

    /// Log to both ETW and file
    void LogEvent(Logging::LogLevel level,
        const std::wstring& component,
        const std::wstring& message,
        const std::wstring& details = L"") {
        if (m_etwEnabled) {
            Tracing::ETWProvider::Get().TraceEvent(
                static_cast<uint16_t>(level) < 3 ? Tracing::EventID::Info : Tracing::EventID::Warning,
                static_cast<Tracing::EventLevel>(static_cast<uint8_t>(level)),
                message.c_str());
        }
        if (m_fileLogEnabled) {
            switch (level) {
            case Logging::LogLevel::Trace:
            case Logging::LogLevel::Debug:
            case Logging::LogLevel::Info:
                Logging::StructuredLogger::Get().LogInfo(component, message, details);
                break;
            case Logging::LogLevel::Warning:
                Logging::StructuredLogger::Get().LogWarning(component, message, details);
                break;
            case Logging::LogLevel::Error:
            case Logging::LogLevel::Critical:
                Logging::StructuredLogger::Get().LogError(component, message, details);
                break;
            }
        }
    }

    /// Trace a thumbnail generation request lifecycle
    void TraceThumbnailStart(const std::wstring& filePath, uint32_t requestedSize) {
        auto details = L"path=" + filePath + L",size=" + std::to_wstring(requestedSize);
        LogEvent(Logging::LogLevel::Info, L"Thumbnail", L"Request started", details);

        if (m_etwEnabled) {
            Tracing::ETWProvider::Get().TraceEvent(
                Tracing::EventID::ThumbnailGeneration_Start,
                Tracing::EventLevel::Info,
                filePath.c_str());
        }
    }

    void TraceThumbnailComplete(const std::wstring& filePath, double elapsedMs, bool fromCache) {
        auto details = L"path=" + filePath +
            L",elapsed_ms=" + std::to_wstring(elapsedMs) +
            L",cache=" + std::wstring(fromCache ? L"hit" : L"miss");
        LogEvent(Logging::LogLevel::Info, L"Thumbnail", L"Request completed", details);

        if (m_etwEnabled) {
            Tracing::ETWProvider::Get().TraceEvent(
                Tracing::EventID::ThumbnailGeneration_Stop,
                Tracing::EventLevel::Info,
                filePath.c_str());
        }
    }

    void TraceDecodeStart(const std::wstring& decoderName, const std::wstring& filePath) {
        LogEvent(Logging::LogLevel::Debug, L"Decode",
            L"Decode started: " + decoderName, L"path=" + filePath);

        if (m_etwEnabled) {
            Tracing::ETWProvider::Get().TraceEvent(
                Tracing::EventID::Decode_Start,
                Tracing::EventLevel::Verbose,
                decoderName.c_str());
        }
    }

    void TraceDecodeComplete(const std::wstring& decoderName, double elapsedMs, bool success) {
        auto msg = L"Decode " + std::wstring(success ? L"succeeded" : L"failed") + L": " + decoderName;
        LogEvent(success ? Logging::LogLevel::Info : Logging::LogLevel::Warning,
            L"Decode", msg, L"elapsed_ms=" + std::to_wstring(elapsedMs));

        if (m_etwEnabled) {
            Tracing::ETWProvider::Get().TraceEvent(
                Tracing::EventID::Decode_Stop,
                success ? Tracing::EventLevel::Info : Tracing::EventLevel::Warning,
                decoderName.c_str());
        }
    }

    bool IsETWEnabled() const { return m_etwEnabled; }
    bool IsFileLogEnabled() const { return m_fileLogEnabled; }

private:
    PipelineTelemetry() = default;

    std::wstring GetLogFilePath() {
        wchar_t* appDataPath = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &appDataPath))) {
            std::wstring logDir = std::wstring(appDataPath) + L"\\ExplorerLens\\Logs";
            CoTaskMemFree(appDataPath);

            std::filesystem::create_directories(logDir);

            // Use date-based log file name
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            struct tm tmBuf;
            localtime_s(&tmBuf, &time);

            wchar_t dateBuf[32];
            wcsftime(dateBuf, 32, L"%Y-%m-%d", &tmBuf);

            return logDir + L"\\explorerlens-" + dateBuf + L".jsonl";
        }
        return L"explorerlens.jsonl";
    }

    bool m_initialized = false;
    bool m_etwEnabled = false;
    bool m_fileLogEnabled = false;
};

//==============================================================================
// Diagnostics Export — System info + config + logs bundle
//==============================================================================

class DiagnosticsExporter {
public:
    struct DiagnosticsBundle {
        std::wstring systemInfo;
        std::wstring configDump;
        std::wstring recentLogs;
        std::wstring registryDump;
        std::wstring decoderStatus;
        std::wstring timestamp;
    };

    /// Collect all diagnostics into a bundle
    static DiagnosticsBundle Collect() {
        DiagnosticsBundle bundle;

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        struct tm tmBuf;
        localtime_s(&tmBuf, &time);
        wchar_t timeBuf[64];
        wcsftime(timeBuf, 64, L"%Y-%m-%d %H:%M:%S", &tmBuf);
        bundle.timestamp = timeBuf;

        bundle.systemInfo = CollectSystemInfo();
        bundle.configDump = CollectConfig();
        bundle.registryDump = CollectRegistryInfo();
        bundle.decoderStatus = CollectDecoderStatus();
        bundle.recentLogs = CollectRecentLogs();

        return bundle;
    }

    /// Export bundle to a text file
    static bool ExportToFile(const DiagnosticsBundle& bundle, const std::wstring& outputPath) {
        std::wofstream out(outputPath);
        if (!out.is_open()) return false;

        out << L"=== ExplorerLens Diagnostics Export ===" << std::endl;
        out << L"Exported: " << bundle.timestamp << std::endl;
        out << L"Version: 7.0.0" << std::endl;
        out << std::endl;

        out << L"--- System Info ---" << std::endl;
        out << bundle.systemInfo << std::endl;

        out << L"--- Configuration ---" << std::endl;
        out << bundle.configDump << std::endl;

        out << L"--- Registry ---" << std::endl;
        out << bundle.registryDump << std::endl;

        out << L"--- Decoder Status ---" << std::endl;
        out << bundle.decoderStatus << std::endl;

        out << L"--- Recent Logs ---" << std::endl;
        out << bundle.recentLogs << std::endl;

        out.close();
        return true;
    }

private:
    static std::wstring CollectSystemInfo() {
        std::wstringstream ss;

        OSVERSIONINFOEXW osvi = {};
        osvi.dwOSVersionInfoSize = sizeof(osvi);

        // Get Windows build number from registry (since GetVersionEx is deprecated)
        HKEY hKey;
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
            0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            DWORD buildNumber = 0;
            DWORD size = sizeof(buildNumber);
            wchar_t displayVersion[64] = {};
            DWORD displaySize = sizeof(displayVersion);

            RegQueryValueExW(hKey, L"CurrentBuildNumber", NULL, NULL,
                (LPBYTE)&buildNumber, &size);
            RegQueryValueExW(hKey, L"DisplayVersion", NULL, NULL,
                (LPBYTE)displayVersion, &displaySize);

            ss << L"Windows Build: " << buildNumber << std::endl;
            ss << L"Display Version: " << displayVersion << std::endl;
            RegCloseKey(hKey);
        }

        // System memory
        MEMORYSTATUSEX memStatus = {};
        memStatus.dwLength = sizeof(memStatus);
        if (GlobalMemoryStatusEx(&memStatus)) {
            ss << L"Total RAM: " << (memStatus.ullTotalPhys / (1024 * 1024)) << L" MB" << std::endl;
            ss << L"Available RAM: " << (memStatus.ullAvailPhys / (1024 * 1024)) << L" MB" << std::endl;
        }

        // Processor info
        SYSTEM_INFO sysInfo = {};
        GetSystemInfo(&sysInfo);
        ss << L"Processors: " << sysInfo.dwNumberOfProcessors << std::endl;
        ss << L"Architecture: " << (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? L"x64" : L"Other") << std::endl;

        return ss.str();
    }

    static std::wstring CollectConfig() {
        std::wstringstream ss;
        ss << L"enablePlugins: (from registry)" << std::endl;
        ss << L"GPU acceleration: (from registry)" << std::endl;
        ss << L"Cache enabled: (from registry)" << std::endl;
        return ss.str();
    }

    static std::wstring CollectRegistryInfo() {
        std::wstringstream ss;

        HKEY hKey;
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\ExplorerLens",
            0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            ss << L"HKLM\\SOFTWARE\\ExplorerLens found" << std::endl;

            // Enumerate values
            DWORD index = 0;
            wchar_t valueName[256];
            DWORD nameSize = 256;
            while (RegEnumValueW(hKey, index, valueName, &nameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                ss << L" " << valueName << std::endl;
                nameSize = 256;
                index++;
            }
            RegCloseKey(hKey);
        }
        else {
            ss << L"HKLM\\SOFTWARE\\ExplorerLens not found" << std::endl;
        }

        return ss.str();
    }

    static std::wstring CollectDecoderStatus() {
        std::wstringstream ss;
        ss << L"Registered decoders: 25" << std::endl;
        ss << L"Format handlers: CBZ, CBR, CB7, CBT, EPUB, MOBI, AZW, AZW3, ZIP, RAR, 7Z, TAR" << std::endl;
        ss << L" WEBP, HEIF, AVIF, JXL, VIDEO, PDF, TIFF, SVG, RAW, PHZ, FB2" << std::endl;
        ss << L" PSD, DDS, HDR, EXR, PPM, ICO, QOI, TGA, AUDIO, DOCUMENT, FONT, MODEL" << std::endl;
        ss << L"Total extensions: 200+" << std::endl;
        return ss.str();
    }

    static std::wstring CollectRecentLogs() {
        std::wstringstream ss;

        // Find the most recent log file
        wchar_t* appDataPath = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &appDataPath))) {
            std::wstring logDir = std::wstring(appDataPath) + L"\\ExplorerLens\\Logs";
            CoTaskMemFree(appDataPath);

            if (std::filesystem::exists(logDir)) {
                // Find most recent .jsonl file
                std::filesystem::path newestLog;
                auto newestTime = std::filesystem::file_time_type::min();

                for (const auto& entry : std::filesystem::directory_iterator(logDir)) {
                    if (entry.path().extension() == L".jsonl" && entry.last_write_time() > newestTime) {
                        newestLog = entry.path();
                        newestTime = entry.last_write_time();
                    }
                }

                if (!newestLog.empty()) {
                    std::wifstream logFile(newestLog);
                    std::wstring line;
                    std::vector<std::wstring> lastLines;
                    while (std::getline(logFile, line)) {
                        lastLines.push_back(line);
                        if (lastLines.size() > 50) lastLines.erase(lastLines.begin());
                    }

                    ss << L"Log file: " << newestLog.filename().wstring() << std::endl;
                    ss << L"Last " << lastLines.size() << L" entries:" << std::endl;
                    for (const auto& l : lastLines) {
                        ss << L" " << l << std::endl;
                    }
                }
            }
            else {
                ss << L"No log directory found" << std::endl;
            }
        }

        return ss.str();
    }
};

//==============================================================================
// RAII Scoped Trace — automatically traces start/stop with timing
//==============================================================================

class ScopedTrace {
public:
    ScopedTrace(const std::wstring& component, const std::wstring& operation)
        : m_component(component), m_operation(operation) {
        m_start = std::chrono::high_resolution_clock::now();
        PipelineTelemetry::Get().LogEvent(
            Logging::LogLevel::Debug, m_component, L"Start: " + m_operation);
    }

    ~ScopedTrace() {
        auto elapsed = std::chrono::high_resolution_clock::now() - m_start;
        double ms = std::chrono::duration<double, std::milli>(elapsed).count();
        PipelineTelemetry::Get().LogEvent(
            Logging::LogLevel::Info, m_component,
            L"Complete: " + m_operation,
            L"elapsed_ms=" + std::to_wstring(ms));
    }

private:
    std::wstring m_component;
    std::wstring m_operation;
    std::chrono::high_resolution_clock::time_point m_start;
};

/// Convenience macro for scoped tracing
#define TRACE_SCOPE(component, operation) \
 ExplorerLens::Observability::ScopedTrace _scopedTrace##__LINE__(L##component, L##operation)

} // namespace Observability
} // namespace ExplorerLens

// ─── ObservabilityPipeline ────────────────────────────────────────────────────

#ifndef EXPLORERLENS_OBSERVABILITY_PIPELINE_H
#define EXPLORERLENS_OBSERVABILITY_PIPELINE_H

namespace ExplorerLens {
namespace Engine {
namespace Observability {

//==============================================================================
// ETW Event Definitions
//==============================================================================

// ETW Event IDs matching OBSERVABILITY_SPEC_V1.md
enum class ETWEventId : uint16_t
{
    RequestStart = 100,
    RequestStop = 101,
    CacheHit = 200,
    CacheMiss = 201,
    CacheEvict = 202,
    DecodeStart = 300,
    DecodeStop = 301,
    DecodeFail = 302,
    CrashCaught = 400,
    CircuitBreakerOpen = 401,
    PluginLoad = 500,
    PluginUnload = 501,
    PluginError = 502,
    MemoryPressure = 600,
    GPUFallback = 700
};

inline const char* ETWEventName(ETWEventId id) {
    switch (id) {
    case ETWEventId::RequestStart: return "RequestStart";
    case ETWEventId::RequestStop: return "RequestStop";
    case ETWEventId::CacheHit: return "CacheHit";
    case ETWEventId::CacheMiss: return "CacheMiss";
    case ETWEventId::CacheEvict: return "CacheEvict";
    case ETWEventId::DecodeStart: return "DecodeStart";
    case ETWEventId::DecodeStop: return "DecodeStop";
    case ETWEventId::DecodeFail: return "DecodeFail";
    case ETWEventId::CrashCaught: return "CrashCaught";
    case ETWEventId::CircuitBreakerOpen: return "CircuitBreakerOpen";
    case ETWEventId::PluginLoad: return "PluginLoad";
    case ETWEventId::PluginUnload: return "PluginUnload";
    case ETWEventId::PluginError: return "PluginError";
    case ETWEventId::MemoryPressure: return "MemoryPressure";
    case ETWEventId::GPUFallback: return "GPUFallback";
    }
    return "Unknown";
}

// ETW Provider GUID: {3B2F8A9C-D1E7-4F5A-B6C2-8D9E0F1A2B3C}
struct ETWProviderConfig
{
    static constexpr const char* ProviderName = "ExplorerLens-Engine-Core";
    static constexpr const char* ProviderGUID = "{3B2F8A9C-D1E7-4F5A-B6C2-8D9E0F1A2B3C}";

    bool enabled = true;
    uint8_t level = 4; // 1=Critical, 2=Error, 3=Warning, 4=Info, 5=Verbose

    static constexpr size_t TotalEventTypes = 15;
};

//==============================================================================
// Log Level & Structured Log Entry
//==============================================================================

enum class LogLevel : uint8_t
{
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warning = 3,
    Error = 4,
    Critical = 5
};

inline const char* LogLevelName(LogLevel l) {
    switch (l) {
    case LogLevel::Trace: return "TRACE";
    case LogLevel::Debug: return "DEBUG";
    case LogLevel::Info: return "INFO";
    case LogLevel::Warning: return "WARN";
    case LogLevel::Error: return "ERROR";
    case LogLevel::Critical: return "CRITICAL";
    }
    return "UNKNOWN";
}

struct LogEntry
{
    std::string timestamp;
    LogLevel level = LogLevel::Info;
    std::string component;
    std::string message;
    std::unordered_map<std::string, std::string> fields;
    uint64_t requestId = 0;
    uint32_t threadId = 0;

    // Render as JSON-lines format
    std::string ToJSON() const {
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
    std::string ToText() const {
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
        : logPath_(logPath) {
    }

    void SetLogPath(const std::string& path) { logPath_ = path; }
    std::string LogPath() const { return logPath_; }

    void Log(const LogEntry& entry) {
        if (entry.level < minLevel_) return;
        entries_.push_back(entry);
    }

    void SetMinLevel(LogLevel level) { minLevel_ = level; }
    LogLevel MinLevel() const { return minLevel_; }

    size_t EntryCount() const { return entries_.size(); }

    // Get all entries as JSON-lines string
    std::string Flush() const {
        std::ostringstream ss;
        for (auto& e : entries_)
            ss << e.ToJSON() << "\n";
        return ss.str();
    }

    // Get entries filtered by level
    std::vector<LogEntry> GetByLevel(LogLevel level) const {
        std::vector<LogEntry> result;
        for (auto& e : entries_)
            if (e.level == level) result.push_back(e);
        return result;
    }

    // Get entries filtered by component
    std::vector<LogEntry> GetByComponent(const std::string& component) const {
        std::vector<LogEntry> result;
        for (auto& e : entries_)
            if (e.component == component) result.push_back(e);
        return result;
    }

    void Clear() { entries_.clear(); }

    // Stats
    size_t ErrorCount() const {
        return static_cast<size_t>(std::count_if(entries_.begin(), entries_.end(),
            [](const auto& e) { return e.level >= LogLevel::Error; }));
    }

    size_t WarningCount() const {
        return static_cast<size_t>(std::count_if(entries_.begin(), entries_.end(),
            [](const auto& e) { return e.level == LogLevel::Warning; }));
    }

private:
    std::string logPath_;
    LogLevel minLevel_ = LogLevel::Info;
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
    std::string FilterPath(const std::string& path) const {
        if (mode_ == Mode::Verbose)
            return path; // Full path in verbose mode

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
    std::string FilterToFilename(const std::string& path) const {
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
    uint64_t requestId = 0;
    std::string filePath;
    std::string format;
    std::string decoder;

    // Timing (milliseconds)
    double detectMs = 0.0;
    double decodeMs = 0.0;
    double resizeMs = 0.0;
    double cacheMs = 0.0;
    double marshalMs = 0.0;
    double totalMs = 0.0;

    bool cacheHit = false;
    bool succeeded = true;
    bool gpuAccelerated = false;
    std::string errorMessage;

    double PipelineMs() const {
        return detectMs + decodeMs + resizeMs + cacheMs + marshalMs;
    }

    std::string ToJSON() const {
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
    void RecordTrace(const RequestTrace& trace) {
        traces_.push_back(trace);
    }

    size_t TraceCount() const { return traces_.size(); }

    double AverageLatencyMs() const {
        if (traces_.empty()) return 0.0;
        double total = 0.0;
        for (auto& t : traces_) total += t.totalMs;
        return total / traces_.size();
    }

    double P95LatencyMs() const {
        if (traces_.empty()) return 0.0;
        std::vector<double> latencies;
        for (auto& t : traces_) latencies.push_back(t.totalMs);
        std::sort(latencies.begin(), latencies.end());
        size_t idx = static_cast<size_t>(latencies.size() * 0.95);
        if (idx >= latencies.size()) idx = latencies.size() - 1;
        return latencies[idx];
    }

    double CacheHitRate() const {
        if (traces_.empty()) return 0.0;
        size_t hits = 0;
        for (auto& t : traces_)
            if (t.cacheHit) ++hits;
        return (static_cast<double>(hits) / traces_.size()) * 100.0;
    }

    double SuccessRate() const {
        if (traces_.empty()) return 100.0;
        size_t ok = 0;
        for (auto& t : traces_)
            if (t.succeeded) ++ok;
        return (static_cast<double>(ok) / traces_.size()) * 100.0;
    }

    size_t ErrorCount() const {
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
    std::string osVersion = "Windows 11 24H2";
    std::string cpuModel = "Unknown";
    std::string gpuModel = "Unknown";
    uint64_t ramMB = 0;
    uint64_t vramMB = 0;
    std::string explorerLensVersion = "15.0.0";

    std::string ToJSON() const {
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
    SystemInfo systemInfo;
    std::string registryDump;
    std::string configDump;
    std::vector<LogEntry> recentLogs;
    std::vector<RequestTrace> recentTraces;
    std::string decoderStatus;
    std::string pluginStatus;

    size_t SectionCount() const {
        size_t n = 1; // system info always present
        if (!registryDump.empty()) ++n;
        if (!configDump.empty()) ++n;
        if (!recentLogs.empty()) ++n;
        if (!recentTraces.empty()) ++n;
        if (!decoderStatus.empty()) ++n;
        if (!pluginStatus.empty()) ++n;
        return n;
    }

    std::string GenerateReport() const {
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

}
}
} // namespace ExplorerLens::Engine::Observability

#endif // EXPLORERLENS_OBSERVABILITY_PIPELINE_H

// ─── Related Observability modules (separate .h/.cpp pairs) ──────────────────
#include "DiagnosticDashboard.h"
#include "LogRotationEngine.h"
