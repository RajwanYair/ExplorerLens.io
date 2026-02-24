using ExplorerLens.Manager.ViewModels;
using Microsoft.UI.Xaml.Controls;

namespace ExplorerLens.Manager.Views;

/// <summary>
/// Plugins page code-behind
/// </summary>
public sealed partial class PluginsPage : Page
{
    public PluginsViewModel ViewModel { get; }

    public PluginsPage()
    {
        ViewModel = App.GetService<PluginsViewModel>();
        this.InitializeComponent();
    }
}
