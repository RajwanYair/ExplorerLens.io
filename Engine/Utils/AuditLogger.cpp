/******************************************************************************
 * ExplorerLens Engine - Audit Logger Implementation
 * Copyright (c) 2026 - ExplorerLens Project
 *
 * See AuditLogger.h for API documentation and usage examples.
 *****************************************************************************/

#include "AuditLogger.h"
#include <Windows.h>
#include <ShlObj.h>
#include <filesystem>
#include <iomanip>
#include <sstream>

#pragma comment(lib, "Shell32.lib")

namespace ExplorerLens {

//============================================================================
// Singleton
//============================================================================

AuditLogger& AuditLogger::Instance()
{
    static AuditLogger instance;
    return instance;
}

//============================================================================
// Constructor / Destructor
//============================================================================

AuditLogger::AuditLogger()
{
    Initialize();
}

AuditLogger::~AuditLogger()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (log_file_.is_open()) {
        log_file_.flush();
        log_file_.close();
    }
}

//============================================================================
// Initialization
//============================================================================

bool AuditLogger::Initialize()
{
    // Check if audit logging is enabled via registry/group policy
    enabled_ = ReadEnabledFromRegistry();
    if (!enabled_) {
        initialized_ = true;  // Successfully initialized (just disabled)
        return true;
    }

    // Build log directory path: %LOCALAPPDATA%\ExplorerLens\Logs
    wchar_t appDataPath[MAX_PATH] = {};
    HRESULT hr = SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, appDataPath);
    if (FAILED(hr)) {
        enabled_ = false;
        initialized_ = true;
        return false;
    }

    std::filesystem::path logDir = std::filesystem::path(appDataPath) / L"ExplorerLens" / L"Logs";

    // Create directories if needed
    std::error_code ec;
    std::filesystem::create_directories(logDir, ec);
    if (ec) {
        enabled_ = false;
        initialized_ = true;
        return false;
    }

    // Open log file in append mode
    log_file_path_ = (logDir / L"audit.log").wstring();
    log_file_.open(log_file_path_, std::ios::app);

    if (!log_file_.is_open()) {
        enabled_ = false;
        initialized_ = true;
        return false;
    }

    initialized_ = true;

    // Log session start
    LogEvent(AuditEvent::ConfigurationChanged, L"Audit logging session started");
    return true;
}

bool AuditLogger::ReadEnabledFromRegistry() const
{
    HKEY hKey = nullptr;
    LONG result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\ExplorerLens", 0, KEY_READ | KEY_WOW64_64KEY, &hKey);

    if (result != ERROR_SUCCESS) {
        // Key doesn't exist - check HKCU as fallback
        result = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\ExplorerLens", 0, KEY_READ, &hKey);

        if (result != ERROR_SUCCESS) {
            return false;  // No registry key = disabled by default
        }
    }

    DWORD value = 0;
    DWORD size = sizeof(value);
    DWORD type = 0;
    result = RegQueryValueExW(hKey, L"AuditLogging", nullptr, &type, reinterpret_cast<BYTE*>(&value), &size);
    RegCloseKey(hKey);

    return (result == ERROR_SUCCESS && type == REG_DWORD && value != 0);
}

//============================================================================
// Public API
//============================================================================

void AuditLogger::LogEvent(AuditEvent event, const std::wstring& details)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!enabled_ || !log_file_.is_open())
        return;

    RotateIfNeeded();

    log_file_ << FormatTimestamp() << L" [" << EventLabel(event) << L"] " << details << L"\n";
    log_file_.flush();
}

void AuditLogger::LogFileAccess(const std::wstring& filePath)
{
    LogEvent(AuditEvent::ThumbnailRequested, filePath);
}

void AuditLogger::LogPluginUsage(const std::wstring& pluginId, const std::wstring& filePath)
{
    std::wstring details = pluginId + L" -> " + filePath;
    LogEvent(AuditEvent::PluginLoaded, details);
}

bool AuditLogger::IsEnabled() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return enabled_;
}

void AuditLogger::SetEnabled(bool enabled)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (enabled && !enabled_) {
        // Re-initialize if enabling for the first time after disable
        if (!log_file_.is_open() && !log_file_path_.empty()) {
            log_file_.open(log_file_path_, std::ios::app);
        }
    }

    enabled_ = enabled;
}

void AuditLogger::Flush()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (log_file_.is_open()) {
        log_file_.flush();
    }
}

std::wstring AuditLogger::GetLogFilePath() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return log_file_path_;
}

//============================================================================
// Internal Helpers
//============================================================================

void AuditLogger::RotateIfNeeded()
{
    // Caller must hold mutex_

    if (log_file_path_.empty())
        return;

    // Check file size (approximate - using tellp is fast)
    auto pos = log_file_.tellp();
    if (pos < 0 || static_cast<uint64_t>(pos) < MAX_LOG_SIZE_BYTES)
        return;

    // Close current file
    log_file_.close();

    // Rotate: audit.log -> audit.1.log -> audit.2.log -> ...
    std::filesystem::path basePath(log_file_path_);
    std::filesystem::path dir = basePath.parent_path();
    std::wstring stem = basePath.stem().wstring();
    std::wstring ext = basePath.extension().wstring();

    std::error_code ec;

    // Delete oldest rotated file
    std::filesystem::path oldest = dir / (stem + L"." + std::to_wstring(MAX_ROTATED_FILES) + ext);
    std::filesystem::remove(oldest, ec);

    // Shift existing rotated files up by one
    for (int i = MAX_ROTATED_FILES - 1; i >= 1; --i) {
        std::filesystem::path src = dir / (stem + L"." + std::to_wstring(i) + ext);
        std::filesystem::path dst = dir / (stem + L"." + std::to_wstring(i + 1) + ext);
        std::filesystem::rename(src, dst, ec);
    }

    // Rename current log to .1
    std::filesystem::path rotated = dir / (stem + L".1" + ext);
    std::filesystem::rename(basePath, rotated, ec);

    // Re-open fresh log file
    log_file_.open(log_file_path_, std::ios::out);
}

const wchar_t* AuditLogger::EventLabel(AuditEvent event)
{
    switch (event) {
        case AuditEvent::ThumbnailRequested:
            return L"THUMBNAIL_REQ";
        case AuditEvent::ThumbnailGenerated:
            return L"THUMBNAIL_OK";
        case AuditEvent::ThumbnailFailed:
            return L"THUMBNAIL_FAIL";
        case AuditEvent::PluginLoaded:
            return L"PLUGIN_LOAD";
        case AuditEvent::PluginUnloaded:
            return L"PLUGIN_UNLOAD";
        case AuditEvent::PluginFailed:
            return L"PLUGIN_ERROR";
        case AuditEvent::PluginCrashed:
            return L"PLUGIN_CRASH";
        case AuditEvent::CacheHit:
            return L"CACHE_HIT";
        case AuditEvent::CacheMiss:
            return L"CACHE_MISS";
        case AuditEvent::CachePurge:
            return L"CACHE_PURGE";
        case AuditEvent::DecoderError:
            return L"DECODER_ERROR";
        case AuditEvent::SecurityViolation:
            return L"SECURITY";
        case AuditEvent::ConfigurationChanged:
            return L"CONFIG";
        default:
            return L"UNKNOWN";
    }
}

std::wstring AuditLogger::FormatTimestamp()
{
    SYSTEMTIME st = {};
    GetLocalTime(&st);

    wchar_t buf[32] = {};
    swprintf_s(buf, L"[%04u-%02u-%02u %02u:%02u:%02u.%03u]", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute,
               st.wSecond, st.wMilliseconds);
    return buf;
}

}  // namespace ExplorerLens
