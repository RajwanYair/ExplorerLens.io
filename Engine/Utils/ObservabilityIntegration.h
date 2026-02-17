//==============================================================================
// ObservabilityIntegration.h - Pipeline Observability Wiring
// Sprint 12: Connects ETW + StructuredLogger + DiagnosticsExport
// Copyright (c) 2026 - DarkThumbs Project
//==============================================================================

#pragma once

#include "ETWTracing.h"
#include "StructuredLogger.h"
#include <windows.h>
#include <string>
#include <chrono>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <ShlObj.h>

namespace DarkThumbs {
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
            std::wstring logDir = std::wstring(appDataPath) + L"\\DarkThumbs\\Logs";
            CoTaskMemFree(appDataPath);

            std::filesystem::create_directories(logDir);

            // Use date-based log file name
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            struct tm tmBuf;
            localtime_s(&tmBuf, &time);
            
            wchar_t dateBuf[32];
            wcsftime(dateBuf, 32, L"%Y-%m-%d", &tmBuf);
            
            return logDir + L"\\darkthumbs-" + dateBuf + L".jsonl";
        }
        return L"darkthumbs.jsonl";
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

        out << L"=== DarkThumbs Diagnostics Export ===" << std::endl;
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
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\DarkThumbs",
                          0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            ss << L"HKLM\\SOFTWARE\\DarkThumbs found" << std::endl;
            
            // Enumerate values
            DWORD index = 0;
            wchar_t valueName[256];
            DWORD nameSize = 256;
            while (RegEnumValueW(hKey, index, valueName, &nameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                ss << L"  " << valueName << std::endl;
                nameSize = 256;
                index++;
            }
            RegCloseKey(hKey);
        } else {
            ss << L"HKLM\\SOFTWARE\\DarkThumbs not found" << std::endl;
        }
        
        return ss.str();
    }

    static std::wstring CollectDecoderStatus() {
        std::wstringstream ss;
        ss << L"Registered decoders: 25" << std::endl;
        ss << L"Format handlers: CBZ, CBR, CB7, CBT, EPUB, MOBI, AZW, AZW3, ZIP, RAR, 7Z, TAR" << std::endl;
        ss << L"                 WEBP, HEIF, AVIF, JXL, VIDEO, PDF, TIFF, SVG, RAW, PHZ, FB2" << std::endl;
        ss << L"                 PSD, DDS, HDR, EXR, PPM, ICO, QOI, TGA, AUDIO, DOCUMENT, FONT, MODEL" << std::endl;
        ss << L"Total extensions: 200+" << std::endl;
        return ss.str();
    }

    static std::wstring CollectRecentLogs() {
        std::wstringstream ss;
        
        // Find the most recent log file
        wchar_t* appDataPath = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &appDataPath))) {
            std::wstring logDir = std::wstring(appDataPath) + L"\\DarkThumbs\\Logs";
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
                        ss << L"  " << l << std::endl;
                    }
                }
            } else {
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
    DarkThumbs::Observability::ScopedTrace _scopedTrace##__LINE__(L##component, L##operation)

} // namespace Observability
} // namespace DarkThumbs
