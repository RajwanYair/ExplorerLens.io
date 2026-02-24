//==============================================================================
// StructuredLogger.h - JSON-Lines Structured Logging
// Copyright (c) 2026 - ExplorerLens Project
// Observability & Structured Logging
// 
//Provides structured logging to JSON-lines format as fallback for non-ETW
//==============================================================================

#pragma once

#include <windows.h>
#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <sstream>
#include <iomanip>

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
        m_logFile.flush();  // Ensure immediate write for debugging
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
        } else {
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
        } else {
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
    std::of stream m_logFile;
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

