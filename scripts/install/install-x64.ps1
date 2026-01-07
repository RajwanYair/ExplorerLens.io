# DarkThumbs v5.0 - Installation Script (PowerShell)
# Registers DarkThumbs shell extension with Windows
# Date: November 19, 2025
# REQUIRES: Administrator privileges

#Requires -RunAsAdministrator

param(
    [switch]$Force
)

$ErrorActionPreference = "Stop"
$rootDir = $PSScriptRoot
$dllPath = Join-Path $rootDir "CBXShell\x64\Release\CBXShell.dll"

Write-Host "`n+====================================================+" -ForegroundColor Cyan
Write-Host "|     DarkThumbs v5.0 Installation Script          |" -ForegroundColor Cyan
Write-Host "+====================================================+`n" -ForegroundColor Cyan

# Check if running as administrator
$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "ERROR: This script must be run as Administrator!" -ForegroundColor Red
    Write-Host "Right-click and select 'Run as Administrator'" -ForegroundColor Yellow
    exit 1
}

# Verify DLL exists
if (-not (Test-Path $dllPath)) {
    Write-Host "ERROR: CBXShell.dll not found!" -ForegroundColor Red
    Write-Host "Expected location: $dllPath" -ForegroundColor Yellow
    Write-Host "`nPlease build the project first:" -ForegroundColor Yellow
    Write-Host "  .\rebuild-all.ps1" -ForegroundColor White
    exit 1
}

# Display DLL info
$dllInfo = Get-Item $dllPath
$dllSize = [math]::Round($dllInfo.Length / 1024 / 1024, 2)
Write-Host "Found: CBXShell.dll ($dllSize MB)" -ForegroundColor Green
Write-Host "Path: $dllPath`n" -ForegroundColor Gray

# Check if already registered
$regPath = "HKLM:\SOFTWARE\Classes\CLSID\{7D8A33DE-5C89-4508-A228-C05F4DCE0C99}"
$alreadyRegistered = Test-Path $regPath

if ($alreadyRegistered -and -not $Force) {
    Write-Host "WARNING: DarkThumbs appears to be already registered." -ForegroundColor Yellow
    Write-Host "Do you want to re-register? (Y/N): " -NoNewline
    $response = Read-Host
    
    if ($response -ne 'Y' -and $response -ne 'y') {
        Write-Host "Installation cancelled." -ForegroundColor Red
        exit 0
    }
}

# Register DLL
Write-Host "Registering CBXShell.dll with Windows..." -ForegroundColor Yellow

try {
    # Use regsvr32 to register the DLL
    $regsvr32 = Join-Path $env:SystemRoot "System32\regsvr32.exe"
    $arguments = @("/s", "/i", "`"$dllPath`"")
    
    $process = Start-Process -FilePath $regsvr32 -ArgumentList $arguments -Wait -PassThru -NoNewWindow
    
    if ($process.ExitCode -eq 0) {
        Write-Host "  [OK] DLL registered successfully" -ForegroundColor Green
    } else {
        throw "regsvr32 returned exit code $($process.ExitCode)"
    }
    
    # Verify registration
    Start-Sleep -Milliseconds 500
    if (Test-Path $regPath) {
        Write-Host "  [OK] Registry entries verified" -ForegroundColor Green
    } else {
        Write-Host "  [WARNING] Registry entries not found" -ForegroundColor Yellow
    }
    
    # Restart Windows Explorer
    Write-Host "`nRestarting Windows Explorer..." -ForegroundColor Yellow
    
    Stop-Process -Name explorer -Force -ErrorAction SilentlyContinue
    Start-Sleep -Seconds 1
    Start-Process explorer.exe
    
    Write-Host "  [OK] Explorer restarted" -ForegroundColor Green
    
    Write-Host "`n+====================================================+" -ForegroundColor Green
    Write-Host "|       Installation Successful!                    |" -ForegroundColor Green
    Write-Host "+====================================================+" -ForegroundColor Green
    
    Write-Host "`nDarkThumbs is now installed and active!" -ForegroundColor Cyan
    Write-Host "`nSupported formats:" -ForegroundColor Yellow
    Write-Host "  Archives: .cbz, .cbr, .cb7, .cbt, .zip, .rar, .7z, .tar" -ForegroundColor White
    Write-Host "  E-books: .epub, .mobi, .azw, .azw3, .fb2" -ForegroundColor White
    Write-Host "  Images (in archives): .jpg, .png, .bmp, .gif, .tif, .webp, .heic, .heif" -ForegroundColor White
    
    Write-Host "`nTo test:" -ForegroundColor Yellow
    Write-Host "  1. Open Windows Explorer" -ForegroundColor White
    Write-Host "  2. Navigate to a folder with comic books or archives" -ForegroundColor White
    Write-Host "  3. Change view to 'Large Icons' or 'Extra Large Icons'" -ForegroundColor White
    Write-Host "  4. Thumbnails should appear automatically!`n" -ForegroundColor White
    
    exit 0
}
catch {
    Write-Host "`n+====================================================+" -ForegroundColor Red
    Write-Host "|       Installation Failed!                        |" -ForegroundColor Red
    Write-Host "+====================================================+" -ForegroundColor Red
    
    Write-Host "`nError: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host "`nTroubleshooting:" -ForegroundColor Yellow
    Write-Host "  1. Make sure you're running as Administrator" -ForegroundColor White
    Write-Host "  2. Disable antivirus temporarily" -ForegroundColor White
    Write-Host "  3. Check if CBXShell.dll is not corrupted" -ForegroundColor White
    Write-Host "  4. Try uninstalling first: .\uninstall-x64.ps1`n" -ForegroundColor White
    
    exit 1
}
