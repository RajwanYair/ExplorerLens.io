using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using ExplorerLens.Manager.Contracts;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace ExplorerLens.Manager.ViewModels;

/// <summary>
/// ViewModel for Settings page
/// </summary>
public partial class SettingsViewModel : ObservableObject
{
    private readonly ISettingsService _settingsService;

    // General
    [ObservableProperty]
    private bool _darkMode = true;

    [ObservableProperty]
    private bool _startWithWindows = false;

    [ObservableProperty]
    private bool _minimizeToTray = true;

    // Cache
    [ObservableProperty]
    private long _cacheMaxSizeMB = 2048;

    [ObservableProperty]
    private string _cacheLocation = @"C:\ProgramData\ExplorerLens.io\Cache";

    [ObservableProperty]
    private int _cacheRetentionDays = 30;

    [ObservableProperty]
    private bool _autoClearCache = true;

    // GPU
    [ObservableProperty]
    private bool _gpuAccelerationEnabled = true;

    [ObservableProperty]
    private string _selectedGpuDevice = "Auto-detect";

    [ObservableProperty]
    private List<string> _availableGpuDevices = new() { "Auto-detect", "NVIDIA RTX 4070", "Intel UHD Graphics" };

    // Formats
    [ObservableProperty]
    private bool _preferNativeDecoders = true;

    [ObservableProperty]
    private bool _enableExperimentalFormats = false;

    [ObservableProperty]
    private int _maxThumbnailSize = 1024;

    // Plugins
    [ObservableProperty]
    private bool _enablePlugins = true;

    [ObservableProperty]
    private bool _onlyTrustedPlugins = true;

    [ObservableProperty]
    private string _pluginIsolationMode = "PluginHost Process";

    [ObservableProperty]
    private List<string> _isolationModes = new() { "In-Worker", "PluginHost Process", "AppContainer" };

    // Advanced
    [ObservableProperty]
    private bool _enableLogging = true;

    [ObservableProperty]
    private string _logLevel = "Info";

    [ObservableProperty]
    private List<string> _logLevels = new() { "Trace", "Debug", "Info", "Warning", "Error", "Critical" };

    [ObservableProperty]
    private bool _enablePerformanceMonitoring = true;

    [ObservableProperty]
    private bool _enableExperimentalFeatures = false;

    [ObservableProperty]
    private bool _hasUnsavedChanges = false;

    public SettingsViewModel(ISettingsService settingsService)
    {
        _settingsService = settingsService;
        _ = LoadSettingsAsync();
    }

    private async Task LoadSettingsAsync()
    {
        try
        {
            // General
            DarkMode = await _settingsService.GetSettingAsync<bool>("ui.dark_mode") ?? true;
            StartWithWindows = await _settingsService.GetSettingAsync<bool>("general.start_with_windows") ?? false;
            MinimizeToTray = await _settingsService.GetSettingAsync<bool>("general.minimize_to_tray") ?? true;

            // Cache
            CacheMaxSizeMB = await _settingsService.GetSettingAsync<long>("cache.max_size_mb") ?? 2048;
            CacheLocation = await _settingsService.GetSettingAsync<string>("cache.location") ?? @"C:\ProgramData\ExplorerLens.io\Cache";
            CacheRetentionDays = await _settingsService.GetSettingAsync<int>("cache.retention_days") ?? 30;
            AutoClearCache = await _settingsService.GetSettingAsync<bool>("cache.auto_clear") ?? true;

            // GPU
            GpuAccelerationEnabled = await _settingsService.GetSettingAsync<bool>("gpu.enabled") ?? true;
            SelectedGpuDevice = await _settingsService.GetSettingAsync<string>("gpu.device") ?? "Auto-detect";

            // Formats
            PreferNativeDecoders = await _settingsService.GetSettingAsync<bool>("formats.prefer_native") ?? true;
            EnableExperimentalFormats = await _settingsService.GetSettingAsync<bool>("formats.experimental") ?? false;
            MaxThumbnailSize = await _settingsService.GetSettingAsync<int>("formats.max_thumbnail_size") ?? 1024;

            // Plugins
            EnablePlugins = await _settingsService.GetSettingAsync<bool>("plugins.enabled") ?? true;
            OnlyTrustedPlugins = await _settingsService.GetSettingAsync<bool>("plugins.only_trusted") ?? true;
            PluginIsolationMode = await _settingsService.GetSettingAsync<string>("plugins.isolation_mode") ?? "PluginHost Process";

            // Advanced
            EnableLogging = await _settingsService.GetSettingAsync<bool>("advanced.logging") ?? true;
            LogLevel = await _settingsService.GetSettingAsync<string>("advanced.log_level") ?? "Info";
            EnablePerformanceMonitoring = await _settingsService.GetSettingAsync<bool>("advanced.performance_monitoring") ?? true;
            EnableExperimentalFeatures = await _settingsService.GetSettingAsync<bool>("advanced.experimental") ?? false;

            HasUnsavedChanges = false;
        }
        catch (Exception ex)
        {
            // Log error
            Console.WriteLine($"Failed to load settings: {ex.Message}");
        }
    }

    [RelayCommand]
    private async Task ApplySettingsAsync()
    {
        try
        {
            // General
            await _settingsService.SetSettingAsync("ui.dark_mode", DarkMode);
            await _settingsService.SetSettingAsync("general.start_with_windows", StartWithWindows);
            await _settingsService.SetSettingAsync("general.minimize_to_tray", MinimizeToTray);

            // Cache
            await _settingsService.SetSettingAsync("cache.max_size_mb", CacheMaxSizeMB);
            await _settingsService.SetSettingAsync("cache.location", CacheLocation);
            await _settingsService.SetSettingAsync("cache.retention_days", CacheRetentionDays);
            await _settingsService.SetSettingAsync("cache.auto_clear", AutoClearCache);

            // GPU
            await _settingsService.SetSettingAsync("gpu.enabled", GpuAccelerationEnabled);
            await _settingsService.SetSettingAsync("gpu.device", SelectedGpuDevice);

            // Formats
            await _settingsService.SetSettingAsync("formats.prefer_native", PreferNativeDecoders);
            await _settingsService.SetSettingAsync("formats.experimental", EnableExperimentalFormats);
            await _settingsService.SetSettingAsync("formats.max_thumbnail_size", MaxThumbnailSize);

            // Plugins
            await _settingsService.SetSettingAsync("plugins.enabled", EnablePlugins);
            await _settingsService.SetSettingAsync("plugins.only_trusted", OnlyTrustedPlugins);
            await _settingsService.SetSettingAsync("plugins.isolation_mode", PluginIsolationMode);

            // Advanced
            await _settingsService.SetSettingAsync("advanced.logging", EnableLogging);
            await _settingsService.SetSettingAsync("advanced.log_level", LogLevel);
            await _settingsService.SetSettingAsync("advanced.performance_monitoring", EnablePerformanceMonitoring);
            await _settingsService.SetSettingAsync("advanced.experimental", EnableExperimentalFeatures);

            HasUnsavedChanges = false;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to apply settings: {ex.Message}");
        }
    }

    [RelayCommand]
    private async Task ResetToDefaultsAsync()
    {
        try
        {
            await _settingsService.ResetToDefaultsAsync();
            await LoadSettingsAsync();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to reset settings: {ex.Message}");
        }
    }

    [RelayCommand]
    private void BrowseCacheLocation()
    {
        // TODO: Open folder picker dialog
        // For now, just demonstrate the command binding
    }

    partial void OnDarkModeChanged(bool value) => HasUnsavedChanges = true;
    partial void OnStartWithWindowsChanged(bool value) => HasUnsavedChanges = true;
    partial void OnMinimizeToTrayChanged(bool value) => HasUnsavedChanges = true;
    partial void OnCacheMaxSizeMBChanged(long value) => HasUnsavedChanges = true;
    partial void OnCacheLocationChanged(string value) => HasUnsavedChanges = true;
    partial void OnCacheRetentionDaysChanged(int value) => HasUnsavedChanges = true;
    partial void OnAutoClearCacheChanged(bool value) => HasUnsavedChanges = true;
    partial void OnGpuAccelerationEnabledChanged(bool value) => HasUnsavedChanges = true;
    partial void OnSelectedGpuDeviceChanged(string value) => HasUnsavedChanges = true;
    partial void OnPreferNativeDecodersChanged(bool value) => HasUnsavedChanges = true;
    partial void OnEnableExperimentalFormatsChanged(bool value) => HasUnsavedChanges = true;
    partial void OnMaxThumbnailSizeChanged(int value) => HasUnsavedChanges = true;
    partial void OnEnablePluginsChanged(bool value) => HasUnsavedChanges = true;
    partial void OnOnlyTrustedPluginsChanged(bool value) => HasUnsavedChanges = true;
    partial void OnPluginIsolationModeChanged(string value) => HasUnsavedChanges = true;
    partial void OnEnableLoggingChanged(bool value) => HasUnsavedChanges = true;
    partial void OnLogLevelChanged(string value) => HasUnsavedChanges = true;
    partial void OnEnablePerformanceMonitoringChanged(bool value) => HasUnsavedChanges = true;
    partial void OnEnableExperimentalFeaturesChanged(bool value) => HasUnsavedChanges = true;
}
