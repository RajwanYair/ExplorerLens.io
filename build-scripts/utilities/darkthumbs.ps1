#!/usr/bin/env pwsh
<#
.SYNOPSIS
    DarkThumbs - Main Build and Management Script

.DESCRIPTION
    Centralized script for building, testing, installing, and managing DarkThumbs.
    This script provides a unified interface for all common development tasks.

.PARAMETER Action
    The action to perform: build, clean, test, install, uninstall, verify

.PARAMETER Configuration
    Build configuration: Release or Debug (default: Release)

.PARAMETER Platform
    Build platform: x64 or x86 (default: x64)

.EXAMPLE
    .\darkthumbs.ps1 build
    Quick build in Release x64

.EXAMPLE
    .\darkthumbs.ps1 clean
    Clean build from scratch

.EXAMPLE
    .\darkthumbs.ps1 test
    Run tests

.EXAMPLE
    .\darkthumbs.ps1 install
    Install DarkThumbs (requires Administrator)

.EXAMPLE
    .\darkthumbs.ps1 verify
    Verify all required tools are installed
#>

param(
    [Parameter(Position = 0)]
    [ValidateSet('build', 'clean', 'test', 'install', 'uninstall', 'verify', 'setup', 'help')]
    [string]$Action = 'help',
    
    [Parameter()]
    [ValidateSet('Release', 'Debug')]
    [string]$Configuration = 'Release',
    
    [Parameter()]
    [ValidateSet('x64', 'x86')]
    [string]$Platform = 'x64'
)

$ErrorActionPreference = 'Stop'
$ProjectRoot = $PSScriptRoot

Write-Host "+=================================================================+" -ForegroundColor Cyan
Write-Host "|               DarkThumbs Build & Management                   |" -ForegroundColor Cyan
Write-Host "+=================================================================+" -ForegroundColor Cyan
Write-Host ""

function Show-Help {
    Write-Host "Available Commands:" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "  build      " -ForegroundColor Cyan -NoNewline
    Write-Host "Quick incremental build"
    Write-Host "  clean      " -ForegroundColor Cyan -NoNewline
    Write-Host "Clean rebuild from scratch"
    Write-Host "  test       " -ForegroundColor Cyan -NoNewline
    Write-Host "Run all tests"
    Write-Host "  install    " -ForegroundColor Cyan -NoNewline
    Write-Host "Install DarkThumbs (requires Administrator)"
    Write-Host "  uninstall  " -ForegroundColor Cyan -NoNewline
    Write-Host "Uninstall DarkThumbs (requires Administrator)"
    Write-Host "  verify     " -ForegroundColor Cyan -NoNewline
    Write-Host "Verify all required development tools"
    Write-Host "  setup      " -ForegroundColor Cyan -NoNewline
    Write-Host "Setup development environment"
    Write-Host "  help       " -ForegroundColor Cyan -NoNewline
    Write-Host "Show this help message"
    Write-Host ""
    Write-Host "Examples:" -ForegroundColor Yellow
    Write-Host "  .\darkthumbs.ps1 build" -ForegroundColor Gray
    Write-Host "  .\darkthumbs.ps1 clean" -ForegroundColor Gray
    Write-Host "  .\darkthumbs.ps1 test" -ForegroundColor Gray
    Write-Host "  .\darkthumbs.ps1 install" -ForegroundColor Gray
    Write-Host ""
}

function Invoke-Build {
    Write-Host "Building DarkThumbs ($Configuration|$Platform)..." -ForegroundColor Yellow
    & "$ProjectRoot\scripts\build\Quick-Build.ps1" -Configuration $Configuration -Platform $Platform
}

function Invoke-CleanBuild {
    Write-Host "Clean building DarkThumbs ($Configuration|$Platform)..." -ForegroundColor Yellow
    & "$ProjectRoot\scripts\build\Build-Production.ps1"
}

function Invoke-Test {
    Write-Host "Running tests..." -ForegroundColor Yellow
    & "$ProjectRoot\scripts\test\run-tests.ps1"
}

function Invoke-Install {
    Write-Host "Installing DarkThumbs..." -ForegroundColor Yellow
    if (-not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        Write-Warning "Installation requires Administrator privileges. Restarting as Administrator..."
        Start-Process -FilePath "powershell.exe" -ArgumentList "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "$ProjectRoot\scripts\install\install-x64.ps1" -Verb RunAs -WorkingDirectory "$ProjectRoot"
    }
    else {
        Push-Location "$ProjectRoot"
        try {
            & "$ProjectRoot\scripts\install\install-x64.ps1"
        }
        finally {
            Pop-Location
        }
    }
}

function Invoke-Uninstall {
    Write-Host "Uninstalling DarkThumbs..." -ForegroundColor Yellow
    if (-not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        Write-Warning "Uninstallation requires Administrator privileges. Restarting as Administrator..."
        Start-Process -FilePath "powershell.exe" -ArgumentList "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "$ProjectRoot\scripts\install\uninstall-x64.ps1" -Verb RunAs -WorkingDirectory "$ProjectRoot"
    }
    else {
        Push-Location "$ProjectRoot"
        try {
            & "$ProjectRoot\scripts\install\uninstall-x64.ps1"
        }
        finally {
            Pop-Location
        }
    }
}

function Invoke-Verify {
    Write-Host "Verifying development tools..." -ForegroundColor Yellow
    & "$ProjectRoot\scripts\setup\verify-tools.ps1"
}

function Invoke-Setup {
    Write-Host "Setting up development environment..." -ForegroundColor Yellow
    & "$ProjectRoot\scripts\setup\Update-DevTools.ps1" -InstallMissing
}

# Execute action
switch ($Action.ToLower()) {
    'build' { Invoke-Build }
    'clean' { Invoke-CleanBuild }
    'test' { Invoke-Test }
    'install' { Invoke-Install }
    'uninstall' { Invoke-Uninstall }
    'verify' { Invoke-Verify }
    'setup' { Invoke-Setup }
    'help' { Show-Help }
    default { Show-Help }
}

Write-Host ""
Write-Host "[OK] Done!" -ForegroundColor Green
