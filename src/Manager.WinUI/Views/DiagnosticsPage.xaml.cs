using ExplorerLens.Manager.ViewModels;
using Microsoft.UI.Xaml.Controls;

namespace ExplorerLens.Manager.Views;

/// <summary>
/// Diagnostics page code-behind
/// </summary>
public sealed partial class DiagnosticsPage : Page
{
    public DiagnosticsViewModel ViewModel { get; }

    public DiagnosticsPage()
    {
        ViewModel = App.GetService<DiagnosticsViewModel>();
        this.InitializeComponent();
    }
}
