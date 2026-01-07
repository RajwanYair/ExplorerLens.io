using DarkThumbs.Manager.ViewModels;
using Microsoft.UI.Xaml.Controls;

namespace DarkThumbs.Manager.Views;

/// <summary>
/// Settings page code-behind
/// </summary>
public sealed partial class SettingsPage : Page
{
    public SettingsViewModel ViewModel { get; }

    public SettingsPage()
    {
        ViewModel = App.GetService<SettingsViewModel>();
        this.InitializeComponent();
    }
}
