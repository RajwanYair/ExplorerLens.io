/******************************************************************************
 * DarkThumbs Engine - Audit Logger
 * Copyright (c) 2026 - DarkThumbs Project
 * 
 * Enterprise audit logging for file access, plugin usage, and system events.
 * Logs are written to %LOCALAPPDATA%\DarkThumbs\Logs\audit.log with
 * automatic rotation by size.
 * 
 * Thread Safety: All public methods are thread-safe via internal mutex.
 * 
 * Usage:
 *   AuditLogger::Instance().LogFileAccess(L"C:\\Photos\\photo.jpg");
 *   AuditLogger::Instance().LogPluginUsage(L"webp-plugin", L"image.webp");
 *   AuditLogger::Instance().LogEvent(AuditEvent::CacheHit, L"key=abc123");
 * 
 * Log Format:
 *   [2026-01-15 14:30:05.123] [THUMBNAIL] C:\Photos\photo.jpg
 *   [2026-01-15 14:30:05.456] [PLUGIN_LOAD] webp-plugin -> image.webp
 *   [2026-01-15 14:30:06.789] [CACHE_HIT] key=abc123
 * 
 * Configuration:
 *   Logging can be enabled/disabled via registry key or group policy:
 *   HKLM\Software\DarkThumbs\AuditLogging = 1 (DWORD)
 *   Maximum log file size defaults to 10MB before rotation.
 *****************************************************************************/

#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <cstdint>

namespace DarkThumbs {

//============================================================================
// Audit Event Types
//============================================================================

/// Categories of auditable events for enterprise compliance tracking.
enum class AuditEvent : uint32_t
{
    ThumbnailRequested,      ///< A thumbnail was requested for a file
    ThumbnailGenerated,      ///< A thumbnail was successfully generated
    ThumbnailFailed,         ///< Thumbnail generation failed

    PluginLoaded,            ///< A plugin DLL was loaded
    PluginUnloaded,          ///< A plugin DLL was unloaded
    PluginFailed,            ///< A plugin operation failed
    PluginCrashed,           ///< A plugin process crashed (PluginHost mode)

    CacheHit,                ///< Thumbnail served from cache
    CacheMiss,               ///< Thumbnail not found in cache
    CachePurge,              ///< Cache entries were purged

    DecoderError,            ///< A decoder returned an error
    SecurityViolation,       ///< Security policy violation (e.g., blocked plugin)
    ConfigurationChanged,    ///< Settings were changed by user or policy
};

//============================================================================
// Audit Logger
//============================================================================

/// Singleton audit logger for enterprise compliance and diagnostics.
///
/// The logger writes structured log entries to a text file with ISO 8601
/// timestamps and event categories. It supports:
///   - Automatic log directory creation
///   - File rotation when size exceeds threshold
///   - Enable/disable via registry key or group policy
///   - Thread-safe concurrent logging from multiple decoder threads
///
/// The logger initializes lazily on first use. If the log file cannot be
/// opened (e.g., permissions), logging is silently disabled (non-fatal).
class AuditLogger
{
public:
    //------------------------------------------------------------------------
    /// Get the singleton instance (lazy initialization, thread-safe).
    //------------------------------------------------------------------------
    static AuditLogger& Instance();

    // Non-copyable, non-movable
    AuditLogger(const AuditLogger&) = delete;
    AuditLogger& operator=(const AuditLogger&) = delete;

    //------------------------------------------------------------------------
    /// Log a generic audit event with details string.
    /// 
    /// @param event   Event category
    /// @param details Human-readable details (file path, error message, etc.)
    //------------------------------------------------------------------------
    void LogEvent(AuditEvent event, const std::wstring& details);

    //------------------------------------------------------------------------
    /// Log a file access event (shorthand for ThumbnailRequested).
    /// 
    /// @param filePath Full path to the file being accessed
    //------------------------------------------------------------------------
    void LogFileAccess(const std::wstring& filePath);

    //------------------------------------------------------------------------
    /// Log plugin usage event.
    /// 
    /// @param pluginId   Unique identifier of the plugin
    /// @param filePath   File being processed by the plugin
    //------------------------------------------------------------------------
    void LogPluginUsage(const std::wstring& pluginId,
                        const std::wstring& filePath);

    //------------------------------------------------------------------------
    /// Check if audit logging is currently enabled.
    /// 
    /// @return true if logging is active
    //------------------------------------------------------------------------
    bool IsEnabled() const;

    //------------------------------------------------------------------------
    /// Enable or disable audit logging at runtime.
    /// 
    /// @param enabled true to enable, false to disable
    //------------------------------------------------------------------------
    void SetEnabled(bool enabled);

    //------------------------------------------------------------------------
    /// Force flush all buffered log entries to disk.
    //------------------------------------------------------------------------
    void Flush();

    //------------------------------------------------------------------------
    /// Get the current log file path.
    /// 
    /// @return Full path to the active log file, or empty if not initialized
    //------------------------------------------------------------------------
    std::wstring GetLogFilePath() const;

private:
    AuditLogger();
    ~AuditLogger();

    /// Initialize the log file and directory structure.
    /// Called once during construction.
    bool Initialize();

    /// Check registry/group policy for audit logging setting.
    bool ReadEnabledFromRegistry() const;

    /// Rotate log file if it exceeds the size threshold.
    void RotateIfNeeded();

    /// Get string label for an event type.
    static const wchar_t* EventLabel(AuditEvent event);

    /// Format current timestamp as ISO 8601 with milliseconds.
    static std::wstring FormatTimestamp();

    // State
    std::wofstream log_file_;
    std::wstring   log_file_path_;
    bool           enabled_ = false;
    bool           initialized_ = false;

    // Configuration
    static constexpr uint64_t MAX_LOG_SIZE_BYTES = 10 * 1024 * 1024;  // 10 MB
    static constexpr int      MAX_ROTATED_FILES  = 5;

    // Thread safety
    mutable std::mutex mutex_;
};

} // namespace DarkThumbs
