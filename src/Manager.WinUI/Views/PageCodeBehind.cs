using ExplorerLens.Manager.ViewModels;
using Microsoft.UI.Xaml.Controls;

namespace ExplorerLens.Manager.Views;

public sealed partial class DashboardPage : Page
{
    public DashboardViewModel ViewModel { get; }

    public DashboardPage()
    {
        ViewModel = App.GetService<DashboardViewModel>();
        this.InitializeComponent();
    }
}

public sealed partial class FormatsPage : Page
{
    public FormatsViewModel ViewModel { get; }

    public FormatsPage()
    {
        ViewModel = App.GetService<FormatsViewModel>();
        this.InitializeComponent();
    }
}

public sealed partial class CachePage : Page
{
    public CacheViewModel ViewModel { get; }

    public CachePage()
    {
        ViewModel = App.GetService<CacheViewModel>();
        this.InitializeComponent();
    }
}

public sealed partial class PerformancePage : Page
{
    public PerformanceViewModel ViewModel { get; }

    public PerformancePage()
    {
        ViewModel = App.GetService<PerformanceViewModel>();
        this.InitializeComponent();
    }
}
