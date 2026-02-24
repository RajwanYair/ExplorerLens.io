# Enable-ExplorerLensDiagnostics.ps1
# Enables Phase 10 diagnostic features via registry configuration

param(
    [switch]$EnableAll = $false,
    [switch]$Logging = $false,
    [switch]$Profiling = $false,
    [switch]$MemoryTracking = $false,
    [int]$MemoryCacheMB = 100,
    [int]$DiskCacheMB = 1024
)

$ErrorActionPreference = "Stop"

Write-Host "`n=====================================" -ForegroundColor Cyan
Write-Host "ExplorerLens Diagnostics Configuration" -ForegroundColor Cyan
Write-Host "=====================================`n" -ForegroundColor Cyan

# Registry paths
$settingsPath = "HKCU:\Software\ExplorerLens\Settings"
$cachePath = "HKCU:\Software\ExplorerLens\Cache"

function Set-RegistryValue {
    param(
        [string]$Path,
        [string]$Name,
        [int]$Value,
        [string]$Description
    )
    
    # Ensure path exists
    if (-not (Test-Path $Path)) {
        New-Item -Path $Path -Force | Out-Null
        Write-Host "  Created registry key: $Path" -ForegroundColor Gray
    }
    
    # Set value
    Set-ItemProperty -Path $Path -Name $Name -Value $Value -Type DWord -Force
    
    $status = if ($Value -eq 1) { "ENABLED" } else { "DISABLED" }
    $color = if ($Value -eq 1) { "Green" } else { "Yellow" }
    Write-Host "  [$status] " -ForegroundColor $color -NoNewline
    Write-Host "$Description"
}

# Determine what to enable
if ($EnableAll) {
    $Logging = $true
    $Profiling = $true
    $MemoryTracking = $true
}

Write-Host "Configuring diagnostic features...`n" -ForegroundColor Yellow

# Settings configuration
Write-Host "System Settings:" -ForegroundColor Cyan
Set-RegistryValue -Path $settingsPath -Name "DebugLogging" -Value $(if ($Logging) { 1 } else { 0 }) `
                  -Description "Error Logging System"

Set-RegistryValue -Path $settingsPath -Name "EnableProfiling" -Value $(if ($Profiling) { 1 } else { 0 }) `
                  -Description "Performance Profiling"

Set-RegistryValue -Path $settingsPath -Name "EnableMemoryTracking" -Value $(if ($MemoryTracking) { 1 } else { 0 }) `
                  -Description "Memory Leak Tracking"

Write-Host "`nCache Configuration:" -ForegroundColor Cyan
Set-RegistryValue -Path $cachePath -Name "MemoryCacheSizeMB" -Value $MemoryCacheMB `
                  -Description "Memory Cache Size: $MemoryCacheMB MB"

Set-RegistryValue -Path $cachePath -Name "DiskCacheSizeMB" -Value $DiskCacheMB `
                  -Description "Disk Cache Size: $DiskCacheMB MB"

Set-RegistryValue -Path $cachePath -Name "EnableMemoryCache" -Value 1 `
                  -Description "Memory Cache (LRU)"

Set-RegistryValue -Path $cachePath -Name "EnableDiskCache" -Value 1 `
                  -Description "Disk Cache (Persistent)"

Set-RegistryValue -Path $cachePath -Name "EnablePreloading" -Value 0 `
                  -Description "Folder Preloading (Future)"

# Log file location
if ($Logging) {
    $logPath = "$env:LOCALAPPDATA\ExplorerLens\logs"
    Write-Host "`nLog Files Location:" -ForegroundColor Cyan
    Write-Host "  $logPath" -ForegroundColor Gray
    
    if (-not (Test-Path $logPath)) {
        New-Item -Path $logPath -ItemType Directory -Force | Out-Null
        Write-Host "  Created log directory" -ForegroundColor Green
    }
}

# Cache location
$diskCachePath = "$env:LOCALAPPDATA\ExplorerLens\cache"
Write-Host "`nDisk Cache Location:" -ForegroundColor Cyan
Write-Host "  $diskCachePath" -ForegroundColor Gray

if (-not (Test-Path $diskCachePath)) {
    New-Item -Path $diskCachePath -ItemType Directory -Force | Out-Null
    Write-Host "  Created cache directory" -ForegroundColor Green
}

# Summary
Write-Host "`n=====================================" -ForegroundColor Cyan
Write-Host "Configuration Complete!" -ForegroundColor Green
Write-Host "=====================================`n" -ForegroundColor Cyan

if ($Logging -or $Profiling -or $MemoryTracking) {
    Write-Host "Restart Explorer to apply changes:" -ForegroundColor Yellow
    Write-Host "  Stop-Process -Name explorer -Force" -ForegroundColor Gray
    Write-Host "`nOr run:" -ForegroundColor Yellow
    Write-Host "  .\Deploy-ExplorerLens.ps1 -Update" -ForegroundColor Gray
} else {
    Write-Host "No diagnostic features enabled." -ForegroundColor Yellow
    Write-Host "Use -EnableAll to enable all features" -ForegroundColor Gray
}

Write-Host ""

