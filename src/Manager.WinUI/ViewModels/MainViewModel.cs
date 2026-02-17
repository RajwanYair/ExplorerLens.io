using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using System.Threading.Tasks;

namespace DarkThumbs.Manager.ViewModels;

/// <summary>
/// Main ViewModel for the application window
/// </summary>
public partial class MainViewModel : ObservableObject
{
    [ObservableProperty]
    private string _currentPageTitle = "Dashboard";

    [ObservableProperty]
    private string _engineStatus = "DarkThumbs Engine: Active";

    [ObservableProperty]
    private string _version = "v7.0.0";

    public MainViewModel()
    {
        _ = InitializeAsync();
    }

    private async Task InitializeAsync()
    {
        // Check engine status
        await Task.Delay(100); // Simulate async init
        EngineStatus = "DarkThumbs Engine: Active • 0 active requests";
    }
}
