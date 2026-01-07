using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Navigation;
using DarkThumbs.Manager.ViewModels;
using DarkThumbs.Manager.Views.Pages;
using System;
using System.Collections.Generic;

namespace DarkThumbs.Manager.Views;

public sealed partial class MainWindow : Window
{
    public MainViewModel ViewModel { get; }

    private readonly Dictionary<string, Type> _pages = new()
    {
        { "Dashboard", typeof(DashboardPage) },
        { "Settings", typeof(SettingsPage) },
        { "Formats", typeof(FormatsPage) },
        { "Cache", typeof(CachePage) },
        { "Plugins", typeof(PluginsPage) },
        { "Performance", typeof(PerformancePage) },
        { "Diagnostics", typeof(DiagnosticsPage) },
        { "Policies", typeof(PoliciesPage) }
    };

    public MainWindow()
    {
        InitializeComponent();

        ViewModel = App.Services.GetService(typeof(MainViewModel)) as MainViewModel
            ?? throw new InvalidOperationException("MainViewModel not registered");

        Title = "DarkThumbs Manager v5.5.0";
        ExtendsContentIntoTitleBar = true;
        SetTitleBar(NavView);

        // Navigate to Dashboard by default
        ContentFrame.Navigate(typeof(DashboardPage));
        NavView.SelectedItem = NavView.MenuItems[0];
    }

    private void NavigationView_SelectionChanged(NavigationView sender, NavigationViewSelectionChangedEventArgs args)
    {
        if (args.IsSettingsSelected)
        {
            ContentFrame.Navigate(typeof(SettingsPage));
            ViewModel.CurrentPageTitle = "Settings";
        }
        else if (args.SelectedItem is NavigationViewItem item && item.Tag is string tag)
        {
            if (_pages.TryGetValue(tag, out var pageType))
            {
                ContentFrame.Navigate(pageType);
                ViewModel.CurrentPageTitle = item.Content.ToString() ?? tag;
            }
        }
    }
}
