# Fix PowerShell Profile
# Run this with: pwsh -NoProfile -File fix-profile.ps1

$profilePath = "$env:USERPROFILE\OneDrive - Intel Corporation\Documents\PowerShell\Microsoft.PowerShell_profile.ps1"

$minimalProfile = @'
# ExplorerLens Development Environment - Minimal Profile
# Version: 3.0 - 2025-11-25

# Intel Proxy Configuration
[System.Net.Http.HttpClient]::DefaultProxy = New-Object System.Net.WebProxy('http://proxy-dmz.intel.com:912', $true)

Write-Host "Loading ExplorerLens environment..." -ForegroundColor Cyan

# Load MSVC Environment
$VSPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
if (Test-Path $VSPath) {
    $tempFile = [System.IO.Path]::GetTempFileName()
    cmd /c "`"$VSPath`" x64 >nul 2>&1 && set" > $tempFile
    
    Get-Content $tempFile | ForEach-Object {
        if ($_ -match '^([^=]+)=(.*)$') {
            Set-Item -Path "env:$($matches[1])" -Value $matches[2] -Force
        }
    }
    
    Remove-Item $tempFile -Force
    Write-Host "[OK] MSVC environment loaded" -ForegroundColor Green
} else {
    Write-Warning "Visual Studio Build Tools not found"
}

# Project navigation
$global:ExplorerLensPath = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)

function dt {
    Set-Location $global:ExplorerLensPath
}

function Build-ExplorerLens {
    param([switch]$Clean)
    Push-Location $global:ExplorerLensPath
    if ($Clean) {
        & .\clean-rebuild.ps1
    } else {
        & .\Quick-Build.ps1
    }
    Pop-Location
}

function Check-ExplorerLensTools {
    Write-Host "Checking development tools..." -ForegroundColor Cyan
    
    $tools = @('git', 'cmake', 'ninja', 'cl', 'msbuild', 'python', 'gcc')
    foreach ($tool in $tools) {
        if (Get-Command $tool -ErrorAction SilentlyContinue) {
            Write-Host "  [OK] $tool" -ForegroundColor Green
        } else {
            Write-Host "  [FAIL] $tool" -ForegroundColor Red
        }
    }
}

Write-Host "[OK] Environment ready! Commands: dt, Build-ExplorerLens, Check-ExplorerLensTools" -ForegroundColor Green
'@

# Backup old profile if it exists
if (Test-Path $profilePath) {
    $backupPath = "$env:USERPROFILE\OneDrive - Intel Corporation\Documents\MyScripts\ExplorerLens\profile-backup-$(Get-Date -Format 'yyyyMMdd-HHmmss').ps1"
    Copy-Item $profilePath $backupPath -Force
    Write-Host "Backed up old profile to: $backupPath" -ForegroundColor Yellow
}

# Write new profile
Set-Content -Path $profilePath -Value $minimalProfile -Force -Encoding UTF8

Write-Host "[OK] Profile fixed and saved to: $profilePath" -ForegroundColor Green
Write-Host "Please restart PowerShell to test the new profile" -ForegroundColor Cyan

