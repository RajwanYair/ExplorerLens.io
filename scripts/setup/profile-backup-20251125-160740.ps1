# DarkThumbs Development Environment - Minimal Profile
# Version: 3.0 - 2025-11-25

# Intel Proxy Configuration
[System.Net.Http.HttpClient]::DefaultProxy = New-Object System.Net.WebProxy('http://proxy-dmz.intel.com:912', $true)

Write-Host "Loading DarkThumbs environment..." -ForegroundColor Cyan

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
$global:DarkThumbsPath = "C:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\DarkThumbs"

function dt {
    Set-Location $global:DarkThumbsPath
}

function Build-DarkThumbs {
    param([switch]$Clean)
    Push-Location $global:DarkThumbsPath
    if ($Clean) {
        & .\clean-rebuild.ps1
    } else {
        & .\Quick-Build.ps1
    }
    Pop-Location
}

function Check-DarkThumbsTools {
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

Write-Host "[OK] Environment ready! Commands: dt, Build-DarkThumbs, Check-DarkThumbsTools" -ForegroundColor Green
