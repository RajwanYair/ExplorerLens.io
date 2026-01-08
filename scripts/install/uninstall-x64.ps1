# DarkThumbs v5.0 - Uninstallation Script (PowerShell)
# Unregisters DarkThumbs shell extension from Windows
# Date: November 19, 2025
# REQUIRES: Administrator privileges

#Requires -RunAsAdministrator

param(
    [string]$InstallDir = "$env:ProgramFiles\DarkThumbs"
)

$ErrorActionPreference = "Stop"

Write-Host "`n+====================================================+" -ForegroundColor Cyan
Write-Host "|    DarkThumbs v5.0 Uninstallation Script         |" -ForegroundColor Cyan
Write-Host "+====================================================+`n" -ForegroundColor Cyan

# Check if running as administrator
$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "ERROR: This script must be run as Administrator!" -ForegroundColor Red
    Write-Host "Right-click and select 'Run as Administrator'" -ForegroundColor Yellow
    exit 1
}

# Check if installed in Program Files
$DllPath = Join-Path $InstallDir "CBXShell.dll"
$isInstalled = Test-Path $DllPath

# Check if registered
$regPath = "HKLM:\SOFTWARE\Classes\CLSID\{7D8A33DE-5C89-4508-A228-C05F4DCE0C99}"
$isRegistered = Test-Path $regPath

if (-not $isRegistered -and -not $isInstalled) {
    Write-Host "DarkThumbs is not currently registered or installed." -ForegroundColor Yellow
    Write-Host "Nothing to uninstall.`n" -ForegroundColor Green
    exit 0
}

if ($isInstalled) {
    Write-Host "Found DarkThumbs installation at: $InstallDir" -ForegroundColor Yellow
}

if ($isRegistered) {
    Write-Host "Found registered DarkThumbs COM server" -ForegroundColor Yellow
}

Write-Host "`nUnregistering CBXShell.dll from Windows..." -ForegroundColor Yellow

try {
    # Use regsvr32 to unregister the DLL
    $regsvr32 = Join-Path $env:SystemRoot "System32\regsvr32.exe"
    
    # Try with the DLL file from Program Files if it exists
    if (Test-Path $DllPath) {
        $arguments = @("/u", "/s", "`"$DllPath`"")
        $process = Start-Process -FilePath $regsvr32 -ArgumentList $arguments -Wait -PassThru -NoNewWindow
        
        if ($process.ExitCode -eq 0) {
            Write-Host "  [OK] DLL unregistered successfully from Program Files" -ForegroundColor Green
        } else {
            Write-Host "  [WARNING] regsvr32 returned exit code $($process.ExitCode)" -ForegroundColor Yellow
        }
    }
    
    # Manually remove registry entries if they still exist
    if (Test-Path $regPath) {
        Write-Host "  Removing registry entries..." -ForegroundColor Yellow
        Remove-Item -Path $regPath -Recurse -Force -ErrorAction SilentlyContinue
        Write-Host "  [OK] Registry entries removed" -ForegroundColor Green
    }
    
    # Clear thumbnail cache
    Write-Host "`nClearing Windows thumbnail cache..." -ForegroundColor Yellow
    $thumbcache = Join-Path $env:LOCALAPPDATA "Microsoft\Windows\Explorer\thumbcache_*.db"
    Remove-Item $thumbcache -Force -ErrorAction SilentlyContinue
    Write-Host "  [OK] Thumbnail cache cleared" -ForegroundColor Green
    
    # Remove installation directory
    if (Test-Path $InstallDir) {
        Write-Host "`nRemoving installation directory..." -ForegroundColor Yellow
        try {
            Remove-Item -Path $InstallDir -Recurse -Force -ErrorAction Stop
            Write-Host "  [OK] Installation directory removed: $InstallDir" -ForegroundColor Green
        } catch {
            Write-Host "  [WARNING] Could not remove installation directory: $_" -ForegroundColor Yellow
            Write-Host "  You may need to manually delete: $InstallDir" -ForegroundColor Yellow
        }
    }
    
    # Restart Windows Explorer
    Write-Host "`nRestarting Windows Explorer..." -ForegroundColor Yellow
    
    Stop-Process -Name explorer -Force -ErrorAction SilentlyContinue
    Start-Sleep -Seconds 1
    Start-Process explorer.exe
    
    Write-Host "  [OK] Explorer restarted" -ForegroundColor Green
    
    Write-Host "`n+====================================================+" -ForegroundColor Green
    Write-Host "|      Uninstallation Successful!                   |" -ForegroundColor Green
    Write-Host "+====================================================+" -ForegroundColor Green
    
    Write-Host "`nDarkThumbs has been completely removed from Windows." -ForegroundColor Cyan
    Write-Host "Installation directory: $InstallDir`n" -ForegroundColor Gray
    
    exit 0
} catch {
    Write-Host "`n+====================================================+" -ForegroundColor Red
    Write-Host "|      Uninstallation Failed!                       |" -ForegroundColor Red
    Write-Host "+====================================================+" -ForegroundColor Red
    
    Write-Host "`nError: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host "`nTry manually removing registry entries at:" -ForegroundColor Yellow
    Write-Host "  $regPath`n" -ForegroundColor White
    
    exit 1
}
