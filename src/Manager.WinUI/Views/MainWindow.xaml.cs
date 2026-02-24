using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Navigation;
using ExplorerLens.Manager.ViewModels;
using ExplorerLens.Manager.Views.Pages;
using System;
using System.Collections.Generic;

namespace ExplorerLens.Manager.Views;

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

        Title = "ExplorerLens.io Manager v14.0.0";
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
