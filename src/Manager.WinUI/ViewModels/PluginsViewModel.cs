using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using DarkThumbs.Manager.Contracts;
using System;
using System.Collections.ObjectModel;
using System.Linq;
using System.Threading.Tasks;

namespace DarkThumbs.Manager.ViewModels;

/// <summary>
/// ViewModel for Plugins page
/// </summary>
public partial class PluginsViewModel : ObservableObject
{
    private readonly IPluginService _pluginService;

    [ObservableProperty]
    private ObservableCollection<PluginInfo> _installedPlugins = new();

    [ObservableProperty]
    private ObservableCollection<PluginInfo> _availablePlugins = new();

    [ObservableProperty]
    private PluginInfo? _selectedPlugin;

    [ObservableProperty]
    private bool _isLoading;

    [ObservableProperty]
    private string _searchQuery = string.Empty;

    [ObservableProperty]
    private int _installedCount;

    [ObservableProperty]
    private int _availableUpdatesCount;

    public PluginsViewModel(IPluginService pluginService)
    {
        _pluginService = pluginService;
        _ = LoadPluginsAsync();
    }

    private async Task LoadPluginsAsync()
    {
        IsLoading = true;
        try
        {
            var installed = await _pluginService.GetInstalledPluginsAsync();
            InstalledPlugins.Clear();
            foreach (var plugin in installed)
            {
                InstalledPlugins.Add(plugin);
            }
            InstalledCount = installed.Count;

            var available = await _pluginService.GetAvailablePluginsAsync();
            AvailablePlugins.Clear();
            foreach (var plugin in available)
            {
                AvailablePlugins.Add(plugin);
            }

            var updates = await _pluginService.CheckForUpdatesAsync();
            AvailableUpdatesCount = updates.Count;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to load plugins: {ex.Message}");
        }
        finally
        {
            IsLoading = false;
        }
    }

    [RelayCommand]
    private async Task RefreshAsync()
    {
        await LoadPluginsAsync();
    }

    [RelayCommand]
    private async Task InstallPluginAsync(PluginInfo plugin)
    {
        IsLoading = true;
        try
        {
            var progress = new Progress<double>(p => Console.WriteLine($"Installing: {p}%"));
            var success = await _pluginService.InstallPluginAsync(plugin.Id, progress);
            if (success)
            {
                await LoadPluginsAsync();
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to install plugin: {ex.Message}");
        }
        finally
        {
            IsLoading = false;
        }
    }

    [RelayCommand]
    private async Task UninstallPluginAsync(PluginInfo plugin)
    {
        IsLoading = true;
        try
        {
            var success = await _pluginService.UninstallPluginAsync(plugin.Id);
            if (success)
            {
                await LoadPluginsAsync();
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to uninstall plugin: {ex.Message}");
        }
        finally
        {
            IsLoading = false;
        }
    }

    [RelayCommand]
    private async Task EnablePluginAsync(PluginInfo plugin)
    {
        try
        {
            await _pluginService.EnablePluginAsync(plugin.Id);
            await LoadPluginsAsync();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to enable plugin: {ex.Message}");
        }
    }

    [RelayCommand]
    private async Task DisablePluginAsync(PluginInfo plugin)
    {
        try
        {
            await _pluginService.DisablePluginAsync(plugin.Id);
            await LoadPluginsAsync();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to disable plugin: {ex.Message}");
        }
    }

    [RelayCommand]
    private async Task UpdatePluginAsync(PluginInfo plugin)
    {
        IsLoading = true;
        try
        {
            var progress = new Progress<double>(p => Console.WriteLine($"Updating: {p}%"));
            var success = await _pluginService.UpdatePluginAsync(plugin.Id, progress);
            if (success)
            {
                await LoadPluginsAsync();
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to update plugin: {ex.Message}");
        }
        finally
        {
            IsLoading = false;
        }
    }

    [RelayCommand]
    private async Task InstallFromFileAsync()
    {
        // TODO: Open file picker for .dtplugin files
        IsLoading = true;
        try
        {
            await Task.Delay(100);
            // Implementation will go here
        }
        finally
        {
            IsLoading = false;
        }
    }

    partial void OnSearchQueryChanged(string value)
    {
        // Filter plugins based on search query
        // For now, just log it
        Console.WriteLine($"Search: {value}");
    }
}
