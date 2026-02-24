using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace ExplorerLens.Manager.Services;

/// <summary>
/// Core service interfaces for ExplorerLens Manager
/// </summary>

public interface ISettingsService
{
    Task<Dictionary<string, object>> GetAllSettingsAsync();
    Task<T?> GetSettingAsync<T>(string key);
    Task SetSettingAsync<T>(string key, T value);
    Task ResetToDefaultsAsync();
    Task ExportSettingsAsync(string filePath);
    Task ImportSettingsAsync(string filePath);
}

public interface IPluginService
{
    Task<List<PluginInfo>> GetInstalledPluginsAsync();
    Task<List<PluginInfo>> GetAvailablePluginsAsync();
    Task<PluginInfo?> GetPluginDetailsAsync(string pluginId);
    Task<bool> InstallPluginAsync(string packagePath, IProgress<double> progress);
    Task<bool> UninstallPluginAsync(string pluginId);
    Task<bool> EnablePluginAsync(string pluginId);
    Task<bool> DisablePluginAsync(string pluginId);
    Task<List<PluginUpdate>> CheckForUpdatesAsync();
    Task<bool> UpdatePluginAsync(string pluginId, IProgress<double> progress);
}

public interface ICacheService
{
    Task<CacheStatistics> GetStatisticsAsync();
    Task<long> GetCacheSizeAsync();
    Task ClearCacheAsync();
    Task<List<CacheEntry>> GetRecentEntriesAsync(int count);
    Task RemoveCacheEntryAsync(string key);
    Task OptimizeCacheAsync();
}

public interface IDiagnosticsService
{
    Task<DiagnosticsSummary> GetSummaryAsync();
    Task<List<LogEntry>> GetRecentLogsAsync(int count, LogLevel? level = null);
    Task<string> ExportDiagnosticsBundleAsync(string outputPath);
    Task<PerformanceMetrics> GetPerformanceMetricsAsync();
    Task<List<ErrorReport>> GetErrorReportsAsync();
    Task ClearLogsAsync();
}

public interface IFormatService
{
    Task<List<FormatInfo>> GetSupportedFormatsAsync();
    Task<FormatInfo?> GetFormatDetailsAsync(string extension);
    Task SetFormatPriorityAsync(string extension, int priority);
    Task SetPreferredDecoderAsync(string extension, string decoderId);
    Task<Dictionary<string, object>> GetFormatSettingsAsync(string extension);
    Task SetFormatSettingAsync(string extension, string key, object value);
}

public interface IPerformanceService
{
    Task<PerformanceSummary> GetSummaryAsync();
    Task<List<PerformanceEntry>> GetHistoryAsync(TimeSpan duration);
    Task<Dictionary<string, double>> GetBenchmarkResultsAsync();
    Task RunBenchmarksAsync(IProgress<string> progress);
    Task<List<string>> GetPerformanceWarningsAsync();
}

// Data models
public record PluginInfo(
    string Id,
    string Name,
    string Version,
    string Vendor,
    string Description,
    bool IsEnabled,
    bool IsVerified,
    List<string> Formats,
    List<string> Capabilities,
    PluginStatistics? Statistics
);

public record PluginStatistics(
    long TotalRequests,
    long SuccessfulRequests,
    long FailedRequests,
    double AverageTimeMs
);

public record PluginUpdate(
    string PluginId,
    string CurrentVersion,
    string AvailableVersion,
    string Changelog,
    bool Breaking
);

public record CacheStatistics(
    long TotalEntries,
    long SizeBytes,
    double HitRate,
    long TotalHits,
    long TotalMisses
);

public record CacheEntry(
    string Key,
    string FilePath,
    long SizeBytes,
    DateTime CreatedAt,
    DateTime LastAccessedAt
);

public record DiagnosticsSummary(
    int ErrorCount,
    int WarningCount,
    int CrashCount,
    DateTime LastError,
    string EngineStatus
);

public record LogEntry(
    DateTime Timestamp,
    LogLevel Level,
    string Category,
    string Message,
    string? CorrelationId
);

public enum LogLevel
{
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Critical
}

public record PerformanceMetrics(
    double AverageThumbnailTimeMs,
    double P95ThumbnailTimeMs,
    double P99ThumbnailTimeMs,
    long TotalRequests,
    double CacheHitRate
);

public record ErrorReport(
    DateTime Timestamp,
    string ErrorCode,
    string Message,
    string? StackTrace,
    string? FilePath
);

public record FormatInfo(
    string Extension,
    string MimeType,
    string Description,
    string DefaultDecoder,
    List<string> AvailableDecoders,
    int Priority,
    bool IsEnabled
);

public record PerformanceSummary(
    double AverageThroughput,
    double CurrentLoad,
    int ActiveRequests,
    int QueuedRequests,
    GpuStatus GpuStatus
);

public record GpuStatus(
    bool Available,
    string DeviceName,
    double UsagePercent,
    long MemoryUsedBytes,
    long MemoryTotalBytes
);

public record PerformanceEntry(
    DateTime Timestamp,
    string Operation,
    double DurationMs,
    bool Success
);
