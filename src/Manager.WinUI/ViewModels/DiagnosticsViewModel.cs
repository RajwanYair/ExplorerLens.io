using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using DarkThumbs.Manager.Contracts;
using System;
using System.Collections.ObjectModel;
using System.Threading.Tasks;

namespace DarkThumbs.Manager.ViewModels;

/// <summary>
/// ViewModel for Diagnostics page
/// </summary>
public partial class DiagnosticsViewModel : ObservableObject
{
    private readonly IDiagnosticsService _diagnosticsService;

    [ObservableProperty]
    private ObservableCollection<LogEntry> _logs = new();

    [ObservableProperty]
    private ObservableCollection<ErrorReport> _errors = new();

    [ObservableProperty]
    private DiagnosticsSummary? _summary;

    [ObservableProperty]
    private PerformanceMetrics? _performanceMetrics;

    [ObservableProperty]
    private bool _isLoading;

    [ObservableProperty]
    private string _selectedLogLevel = "All";

    [ObservableProperty]
    private ObservableCollection<string> _logLevels = new() { "All", "Trace", "Debug", "Info", "Warning", "Error", "Critical" };

    [ObservableProperty]
    private bool _autoRefresh = false;

    [ObservableProperty]
    private string _filterText = string.Empty;

    public DiagnosticsViewModel(IDiagnosticsService diagnosticsService)
    {
        _diagnosticsService = diagnosticsService;
        _ = LoadDiagnosticsAsync();
    }

    private async Task LoadDiagnosticsAsync()
    {
        IsLoading = true;
        try
        {
            Summary = await _diagnosticsService.GetSummaryAsync();
            PerformanceMetrics = await _diagnosticsService.GetPerformanceMetricsAsync();

            var logLevel = SelectedLogLevel == "All" ? null : Enum.Parse<LogLevel>(SelectedLogLevel);
            var logs = await _diagnosticsService.GetRecentLogsAsync(1000, logLevel);

            Logs.Clear();
            foreach (var log in logs)
            {
                Logs.Add(log);
            }

            var errors = await _diagnosticsService.GetErrorReportsAsync();
            Errors.Clear();
            foreach (var error in errors)
            {
                Errors.Add(error);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to load diagnostics: {ex.Message}");
        }
        finally
        {
            IsLoading = false;
        }
    }

    [RelayCommand]
    private async Task RefreshAsync()
    {
        await LoadDiagnosticsAsync();
    }

    [RelayCommand]
    private async Task ClearLogsAsync()
    {
        try
        {
            await _diagnosticsService.ClearLogsAsync();
            await LoadDiagnosticsAsync();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to clear logs: {ex.Message}");
        }
    }

    [RelayCommand]
    private async Task ExportDiagnosticsBundleAsync()
    {
        IsLoading = true;
        try
        {
            var outputPath = $"diagnostics-{DateTime.Now:yyyyMMdd-HHmmss}.zip";
            await _diagnosticsService.ExportDiagnosticsBundleAsync(outputPath);
            Console.WriteLine($"Diagnostics bundle exported to: {outputPath}");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to export diagnostics: {ex.Message}");
        }
        finally
        {
            IsLoading = false;
        }
    }

    partial void OnSelectedLogLevelChanged(string value)
    {
        _ = LoadDiagnosticsAsync();
    }

    partial void OnAutoRefreshChanged(bool value)
    {
        if (value)
        {
            // TODO: Start auto-refresh timer
        }
        else
        {
            // TODO: Stop auto-refresh timer
        }
    }
}
