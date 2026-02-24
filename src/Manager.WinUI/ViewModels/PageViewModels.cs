using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using ExplorerLens.Manager.Contracts;
using System;
using System.Collections.ObjectModel;
using System.Threading.Tasks;

namespace ExplorerLens.Manager.ViewModels;

/// <summary>
/// ViewModel for Formats page
/// </summary>
public partial class FormatsViewModel : ObservableObject
{
    private readonly IFormatService _formatService;

    [ObservableProperty]
    private ObservableCollection<FormatInfo> _formats = new();

    [ObservableProperty]
    private FormatInfo? _selectedFormat;

    [ObservableProperty]
    private bool _isLoading;

    [ObservableProperty]
    private string _searchQuery = string.Empty;

    public FormatsViewModel(IFormatService formatService)
    {
        _formatService = formatService;
        _ = LoadFormatsAsync();
    }

    private async Task LoadFormatsAsync()
    {
        IsLoading = true;
        try
        {
            var formats = await _formatService.GetSupportedFormatsAsync();
            Formats.Clear();
            foreach (var format in formats)
            {
                Formats.Add(format);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to load formats: {ex.Message}");
        }
        finally
        {
            IsLoading = false;
        }
    }

    [RelayCommand]
    private async Task RefreshAsync()
    {
        await LoadFormatsAsync();
    }

    [RelayCommand]
    private async Task SetFormatPriorityAsync(FormatInfo format)
    {
        // TODO: Show slider dialog
        await _formatService.SetFormatPriorityAsync(format.Extension, format.Priority);
    }

    [RelayCommand]
    private async Task ToggleFormatAsync(FormatInfo format)
    {
        // TODO: Enable/disable format
        await Task.Delay(50);
    }
}

/// <summary>
/// ViewModel for Cache page
/// </summary>
public partial class CacheViewModel : ObservableObject
{
    private readonly ICacheService _cacheService;

    [ObservableProperty]
    private CacheStatistics? _statistics;

    [ObservableProperty]
    private ObservableCollection<CacheEntry> _recentEntries = new();

    [ObservableProperty]
    private bool _isLoading;

    [ObservableProperty]
    private string _cacheSizeFormatted = "0 MB";

    public CacheViewModel(ICacheService cacheService)
    {
        _cacheService = cacheService;
        _ = LoadCacheAsync();
    }

    private async Task LoadCacheAsync()
    {
        IsLoading = true;
        try
        {
            Statistics = await _cacheService.GetStatisticsAsync();
            var entries = await _cacheService.GetRecentEntriesAsync(100);
            RecentEntries.Clear();
            foreach (var entry in entries)
            {
                RecentEntries.Add(entry);
            }

            var sizeBytes = await _cacheService.GetCacheSizeAsync();
            CacheSizeFormatted = FormatBytes(sizeBytes);
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to load cache: {ex.Message}");
        }
        finally
        {
            IsLoading = false;
        }
    }

    [RelayCommand]
    private async Task RefreshAsync()
    {
        await LoadCacheAsync();
    }

    [RelayCommand]
    private async Task ClearCacheAsync()
    {
        IsLoading = true;
        try
        {
            await _cacheService.ClearCacheAsync();
            await LoadCacheAsync();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to clear cache: {ex.Message}");
        }
        finally
        {
            IsLoading = false;
        }
    }

    [RelayCommand]
    private async Task OptimizeCacheAsync()
    {
        IsLoading = true;
        try
        {
            await _cacheService.OptimizeCacheAsync();
            await LoadCacheAsync();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to optimize cache: {ex.Message}");
        }
        finally
        {
            IsLoading = false;
        }
    }

    private string FormatBytes(long bytes)
    {
        string[] sizes = { "B", "KB", "MB", "GB", "TB" };
        double len = bytes;
        int order = 0;
        while (len >= 1024 && order < sizes.Length - 1)
        {
            order++;
            len /= 1024;
        }
        return $"{len:0.##} {sizes[order]}";
    }
}

/// <summary>
/// ViewModel for Performance page
/// </summary>
public partial class PerformanceViewModel : ObservableObject
{
    private readonly IPerformanceService _performanceService;

    [ObservableProperty]
    private PerformanceSummary? _summary;

    [ObservableProperty]
    private ObservableCollection<string> _warnings = new();

    [ObservableProperty]
    private bool _isLoading;

    [ObservableProperty]
    private bool _isRunningBenchmarks;

    [ObservableProperty]
    private string _benchmarkProgress = string.Empty;

    public PerformanceViewModel(IPerformanceService performanceService)
    {
        _performanceService = performanceService;
        _ = LoadPerformanceAsync();
    }

    private async Task LoadPerformanceAsync()
    {
        IsLoading = true;
        try
        {
            Summary = await _performanceService.GetSummaryAsync();
            var warnings = await _performanceService.GetPerformanceWarningsAsync();
            Warnings.Clear();
            foreach (var warning in warnings)
            {
                Warnings.Add(warning);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to load performance data: {ex.Message}");
        }
        finally
        {
            IsLoading = false;
        }
    }

    [RelayCommand]
    private async Task RefreshAsync()
    {
        await LoadPerformanceAsync();
    }

    [RelayCommand]
    private async Task RunBenchmarksAsync()
    {
        IsRunningBenchmarks = true;
        try
        {
            var progress = new Progress<string>(msg => BenchmarkProgress = msg);
            await _performanceService.RunBenchmarksAsync(progress);
            await LoadPerformanceAsync();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to run benchmarks: {ex.Message}");
        }
        finally
        {
            IsRunningBenchmarks = false;
            BenchmarkProgress = string.Empty;
        }
    }
}

/// <summary>
/// ViewModel for Dashboard page
/// </summary>
public partial class DashboardViewModel : ObservableObject
{
    private readonly IDiagnosticsService _diagnosticsService;
    private readonly ICacheService _cacheService;
    private readonly IPluginService _pluginService;
    private readonly IPerformanceService _performanceService;

    [ObservableProperty]
    private string _engineStatus = "Initializing...";

    [ObservableProperty]
    private CacheStatistics? _cacheStats;

    [ObservableProperty]
    private PerformanceSummary? _performanceSummary;

    [ObservableProperty]
    private int _installedPluginsCount;

    [ObservableProperty]
    private int _recentErrorCount;

    [ObservableProperty]
    private bool _isLoading;

    public DashboardViewModel(
        IDiagnosticsService diagnosticsService,
        ICacheService cacheService,
        IPluginService pluginService,
        IPerformanceService performanceService)
    {
        _diagnosticsService = diagnosticsService;
        _cacheService = cacheService;
        _pluginService = pluginService;
        _performanceService = performanceService;
        _ = LoadDashboardAsync();
    }

    private async Task LoadDashboardAsync()
    {
        IsLoading = true;
        try
        {
            var diagnostics = await _diagnosticsService.GetSummaryAsync();
            EngineStatus = diagnostics.EngineStatus;
            RecentErrorCount = diagnostics.ErrorCount;

            CacheStats = await _cacheService.GetStatisticsAsync();
            PerformanceSummary = await _performanceService.GetSummaryAsync();

            var plugins = await _pluginService.GetInstalledPluginsAsync();
            InstalledPluginsCount = plugins.Count;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to load dashboard: {ex.Message}");
        }
        finally
        {
            IsLoading = false;
        }
    }

    [RelayCommand]
    private async Task RefreshAsync()
    {
        await LoadDashboardAsync();
    }
}
