#Requires -Version 7.0

<#
.SYNOPSIS
    Install DarkThumbs shell extension to Program Files

.DESCRIPTION
    Installs DarkThumbs to %ProgramFiles%\DarkThumbs\
    Registers COM DLL (CBXShell.dll) for Windows Explorer integration
    
    IMPORTANT: Must run as Administrator

.PARAMETER Configuration
    Build configuration to install (Debug or Release)

.PARAMETER Unregister
    Unregister and uninstall instead of install

.EXAMPLE
    .\install.ps1
    # Install Release build

.EXAMPLE
    .\install.ps1 -Configuration Debug
    # Install Debug build

.EXAMPLE
    .\install.ps1 -Unregister
    # Uninstall and unregister
#>

[CmdletBinding()]
param(
    [Parameter()]
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release",

    [Parameter()]
    [switch]$Unregister,

    [Parameter()]
    [switch]$DryRun
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# Constants
$WorkspaceRoot = $PSScriptRoot | Split-Path -Parent
$ProductName = "DarkThumbs"
$InstallDir = Join-Path $env:ProgramFiles $ProductName
$SourceDir = Join-Path $WorkspaceRoot "x64\$Configuration"

# Helper function for logging
function Write-Status {
    param([string]$Message, [string]$Type = "INFO")
    switch ($Type) {
        "SUCCESS" { Write-Host "✓ $Message" -ForegroundColor Green }
        "ERROR" { Write-Host "✗ $Message" -ForegroundColor Red }
        "INFO" { Write-Host "• $Message" -ForegroundColor Cyan }
        default { Write-Host $Message }
    }
}

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "DarkThumbs Installation Script" -ForegroundColor Cyan
if ($DryRun) {
    Write-Host "[DRY RUN MODE - No changes will be made]" -ForegroundColor Yellow
}
Write-Host "========================================`n" -ForegroundColor Cyan

# Check admin rights (skip for dry-run)
if (-not $DryRun) {
    $isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
    if (-not $isAdmin) {
        Write-Status "ERROR: This script must run as Administrator" "ERROR"
        Write-Status "Right-click PowerShell and select 'Run as Administrator'" "INFO"
        exit 1
    }
}

# Uninstall mode
if ($Unregister) {
    Write-Status "Uninstalling $ProductName..." "INFO"
    
    # Unregister DLL
    $dllPath = Join-Path $InstallDir "CBXShell.dll"
    if (Test-Path $dllPath) {
        Write-Status "Unregistering COM DLL..."
        try {
            $regsvr32 = Join-Path $env:SystemRoot "System32\regsvr32.exe"
            $proc = Start-Process -FilePath $regsvr32 -ArgumentList "/u", "/s", "`"$dllPath`"" -Wait -PassThru -NoNewWindow
            
            if ($proc.ExitCode -eq 0) {
                Write-Status "COM DLL unregistered" "SUCCESS"
            } else {
                Write-Status "Warning: regsvr32 returned exit code $($proc.ExitCode)" "ERROR"
            }
        } catch {
            Write-Status "Error unregistering DLL: $($_.Exception.Message)" "ERROR"
        }
    }
    
    # Remove installation directory
    if (Test-Path $InstallDir) {
        Write-Status "Removing installation directory..."
        try {
            Remove-Item -Recurse -Force -Path $InstallDir
            Write-Status "Installation directory removed" "SUCCESS"
        } catch {
            Write-Status "Error removing directory: $($_.Exception.Message)" "ERROR"
            Write-Status "You may need to restart Explorer and try again" "INFO"
            exit 1
        }
    } else {
        Write-Status "Installation directory not found (already uninstalled?)" "INFO"
    }
    
    Write-Host "`n✓ Uninstall complete`n" -ForegroundColor Green
    Write-Status "You may need to restart Windows Explorer for changes to take effect" "INFO"
    Write-Status "Run: taskkill /f /im explorer.exe && start explorer.exe" "INFO"
    exit 0
}

# Install mode
Write-Status "Installing $ProductName $Configuration build..." "INFO"
Write-Status "Source: $SourceDir"
Write-Status "Target: $InstallDir"

# Verify source files exist
$requiredFiles = @(
    "CBXShell.dll",
    "CBXManager.exe"
)

$optionalFiles = @(
    "UnRAR64.dll"
)

Write-Status "`nVerifying source files..." "INFO"
$missingFiles = @()
$filesToCopy = @()

foreach ($file in $requiredFiles) {
    $sourcePath = Join-Path $SourceDir $file
    if (Test-Path $sourcePath) {
        $fileInfo = Get-Item $sourcePath
        $sizeKB = [math]::Round($fileInfo.Length / 1KB, 0)
        Write-Status "  $file ($sizeKB KB)" "SUCCESS"
        $filesToCopy += $file
    } else {
        Write-Status "  $file NOT FOUND" "ERROR"
        $missingFiles += $file
    }
}

foreach ($file in $optionalFiles) {
    $sourcePath = Join-Path $SourceDir $file
    if (Test-Path $sourcePath) {
        $fileInfo = Get-Item $sourcePath
        $sizeKB = [math]::Round($fileInfo.Length / 1KB, 0)
        Write-Status "  $file ($sizeKB KB) [OPTIONAL]" "SUCCESS"
        $filesToCopy += $file
    } else {
        Write-Status "  $file not found (optional - RAR support disabled)" "INFO"
    }
}

if ($missingFiles.Count -gt 0) {
    Write-Status "`nERROR: Missing required files. Build the project first:" "ERROR"
    Write-Status "  .\scripts\build.ps1 -Configuration $Configuration" "INFO"
    exit 1
}

# Create installation directory
Write-Status "`nCreating installation directory..." "INFO"
if ($DryRun) {
    Write-Status "[DRY RUN] Would create: $InstallDir" "INFO"
} else {
    try {
        New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null
        Write-Status "Directory created" "SUCCESS"
    } catch {
        Write-Status "Error creating directory: $($_.Exception.Message)" "ERROR"
        exit 1
    }
}

# Copy files
Write-Status "`nCopying files..." "INFO"
foreach ($file in $filesToCopy) {
    $sourcePath = Join-Path $SourceDir $file
    $destPath = Join-Path $InstallDir $file
    
    if ($DryRun) {
        Write-Status "  [DRY RUN] Would copy: $file" "INFO"
    } else {
        try {
            Copy-Item -Path $sourcePath -Destination $destPath -Force
            Write-Status "  Copied $file" "SUCCESS"
        } catch {
            Write-Status "  Error copying ${file}: $($_.Exception.Message)" "ERROR"
            exit 1
        }
    }
}

# Register COM DLL
Write-Status "`nRegistering COM DLL..." "INFO"
$dllPath = Join-Path $InstallDir "CBXShell.dll"

if ($DryRun) {
    Write-Status "[DRY RUN] Would register: $dllPath" "INFO"
    Write-Status "[DRY RUN] Command: regsvr32.exe /s `"$dllPath`"" "INFO"
} else {
    try {
        $regsvr32 = Join-Path $env:SystemRoot "System32\regsvr32.exe"
        Write-Status "Running: $regsvr32 /s `"$dllPath`"" "INFO"
        Write-Status "(This may take 10-30 seconds...)" "INFO"
        
        # Start process with timeout
        $proc = Start-Process -FilePath $regsvr32 -ArgumentList "/s", "`"$dllPath`"" -PassThru -NoNewWindow
        
        # Wait with timeout (30 seconds)
        $timeout = 30
        $waitResult = $proc.WaitForExit($timeout * 1000)
        
        if (-not $waitResult) {
            Write-Status "Registration timed out after $timeout seconds" "ERROR"
            Write-Status "Attempting to kill hung process..." "INFO"
            $proc.Kill()
            Write-Status "Try manual registration: regsvr32.exe `"$dllPath`"" "INFO"
            exit 1
        }
        
        if ($proc.ExitCode -eq 0) {
            Write-Status "COM DLL registered successfully" "SUCCESS"
        } else {
            Write-Status "Warning: regsvr32 returned exit code $($proc.ExitCode)" "ERROR"
            Write-Status "The DLL may already be registered or registration failed" "INFO"
            
            # Try verbose registration to see error
            Write-Status "Attempting verbose registration for diagnostic..." "INFO"
            $verboseProc = Start-Process -FilePath $regsvr32 -ArgumentList "`"$dllPath`"" -Wait -PassThru
        }
    } catch {
        Write-Status "Error registering DLL: $($_.Exception.Message)" "ERROR"
        Write-Status "Installation may be incomplete" "ERROR"
        exit 1
    }
}

# Installation summary
Write-Host "`n========================================" -ForegroundColor Green
Write-Host "INSTALLATION COMPLETE" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host "Installation directory:" -ForegroundColor Cyan
Write-Host "  $InstallDir`n" -ForegroundColor White

Write-Status "Installed files:" "INFO"
Get-ChildItem $InstallDir | ForEach-Object {
    $sizeKB = [math]::Round($_.Length / 1KB, 0)
    Write-Host "  $($_.Name) ($sizeKB KB)" -ForegroundColor White
}

Write-Host "`nNext steps:" -ForegroundColor Yellow
Write-Host "  1. Restart Windows Explorer for shell extension to activate" -ForegroundColor White
Write-Host "     Run: taskkill /f /im explorer.exe && start explorer.exe" -ForegroundColor Gray
Write-Host "  2. Test thumbnail generation on .cbz/.cbr files" -ForegroundColor White
Write-Host "  3. Run CBXManager.exe to configure settings" -ForegroundColor White

Write-Host "`nTo uninstall:" -ForegroundColor Yellow
Write-Host "  .\scripts\install.ps1 -Unregister" -ForegroundColor Gray
Write-Host ""

exit 0
