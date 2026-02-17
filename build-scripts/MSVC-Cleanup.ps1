# Path to Visual Studio Installer
$vsInstaller = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vs_installer.exe"

# Check if Visual Studio Installer exists
if (-Not (Test-Path $vsInstaller)) {
    Write-Host "Visual Studio Installer not found at $vsInstaller"
    exit
}

# Prompt for Visual Studio install path
$installPath = Read-Host "Enter the full Visual Studio install path (e.g., C:\Program Files\Microsoft Visual Studio\2022\Community)"

# Confirm path exists
if (-Not (Test-Path $installPath)) {
    Write-Host "Invalid path: $installPath"
    exit
}

# Modify installation: keep only essential build tools
Write-Host "Optimizing Visual Studio installation..."
& $vsInstaller modify --installPath "$installPath" `
    --remove Microsoft.VisualStudio.Workload.ManagedDesktop `
    --remove Microsoft.VisualStudio.Workload.NetWeb `
    --remove Microsoft.VisualStudio.Workload.Azure `
    --remove Microsoft.VisualStudio.Workload.Data `
    --remove Microsoft.VisualStudio.Workload.Office `
    --remove Microsoft.VisualStudio.Workload.Unity `
    --remove Microsoft.VisualStudio.Workload.MobileCrossPlatform `
    --remove Microsoft.VisualStudio.Workload.Node `
    --remove Microsoft.VisualStudio.Workload.Python `
    --remove Microsoft.VisualStudio.Workload.Game `
    --remove Microsoft.VisualStudio.Workload.Desktop `
    --remove Microsoft.VisualStudio.Workload.WebCrossPlatform `
    --add Microsoft.VisualStudio.Workload.MSBuildTools `
    --add Microsoft.VisualStudio.Workload.VCTools `
    --quiet --norestart

Write-Host "Visual Studio has been optimized to keep only essential build tools."