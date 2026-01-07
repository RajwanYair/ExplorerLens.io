using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using DarkThumbs.Manager.Services;
using DarkThumbs.Manager.Views;
using System;

namespace DarkThumbs.Manager;

/// <summary>
/// DarkThumbs Manager Application
/// Modern WinUI 3 application for managing DarkThumbs configuration, plugins, and diagnostics.
/// </summary>
public partial class App : Application
{
    private Window? _mainWindow;
    public static IServiceProvider Services { get; private set; } = null!;

    public App()
    {
        InitializeComponent();

        // Initialize services
        Services = ConfigureServices();
    }

    protected override void OnLaunched(LaunchActivatedEventArgs args)
    {
        _mainWindow = new MainWindow();
        _mainWindow.Activate();
    }

    private static IServiceProvider ConfigureServices()
    {
        var services = new Microsoft.Extensions.DependencyInjection.ServiceCollection();

        // Core services
        services.AddSingleton<ISettingsService, SettingsService>();
        services.AddSingleton<IPluginService, PluginService>();
        services.AddSingleton<ICacheService, CacheService>();
        services.AddSingleton<IDiagnosticsService, DiagnosticsService>();
        services.AddSingleton<IFormatService, FormatService>();
        services.AddSingleton<IPerformanceService, PerformanceService>();

        // ViewModels
        services.AddTransient<MainViewModel>();
        services.AddTransient<DashboardViewModel>();
        services.AddTransient<SettingsViewModel>();
        services.AddTransient<PluginsViewModel>();
        services.AddTransient<DiagnosticsViewModel>();
        services.AddTransient<FormatsViewModel>();
        services.AddTransient<CacheViewModel>();
        services.AddTransient<PerformanceViewModel>();

        return services.BuildServiceProvider();
    }
}
