# ============================================================================
# Install-DarkThumbs.ps1
# Installation script for DarkThumbs thumbnail provider
# Handles fresh install and upgrades from older versions
# ============================================================================

#Requires -RunAsAdministrator

[CmdletBinding()]
param(
    [Parameter(Mandatory=$false)]
    [switch]$Uninstall,
    
    [Parameter(Mandatory=$false)]
    [switch]$Upgrade,
    
    [Parameter(Mandatory=$false)]
    [string]$InstallPath = "$env:ProgramFiles\DarkThumbs",
    
    [Parameter(Mandatory=$false)]
    [switch]$SkipBackup
)

$ErrorActionPreference = "Stop"

# ============================================================================
# Configuration
# ============================================================================

$ScriptRoot = $PSScriptRoot
$Version = "6.2.0"
$BuildConfig = "Release"
$Platform = "x64"

$SourceFiles = @{
    "CBXShell.dll" = "x64\$BuildConfig\CBXShell.dll"
    "CBXManager.exe" = "x64\$BuildConfig\CBXManager.exe"
    "DarkThumbsEngine.dll" = "build\bin\$BuildConfig\DarkThumbsEngine.dll"
}

$RegistryKeys = @(
    "HKCR:\CLSID\{YOUR-CLSID-HERE}",
    "HKLM:\Software\DarkThumbs",
    "HKCU:\Software\DarkThumbs"
)

# ============================================================================
# Functions
# ============================================================================

function Write-Step {
    param([string]$Message)
    Write-Host "`n[$(Get-Date -Format 'HH:mm:ss')] $Message" -ForegroundColor Cyan
}

function Write-Success {
    param([string]$Message)
    Write-Host "  ✓ $Message" -ForegroundColor Green
}

function Write-Warning {
    param([string]$Message)
    Write-Host "  ⚠ $Message" -ForegroundColor Yellow
}

function Write-Error {
    param([string]$Message)
    Write-Host "  ✗ $Message" -ForegroundColor Red
}

function Test-Administrator {
    $currentPrincipal = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
    return $currentPrincipal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Stop-ExplorerProcess {
    Write-Step "Stopping Windows Explorer..."
    $explorerProcesses = Get-Process explorer -ErrorAction SilentlyContinue
    if ($explorerProcesses) {
        Stop-Process -Name explorer -Force
        Start-Sleep -Seconds 2
        Write-Success "Explorer stopped"
    } else {
        Write-Success "Explorer not running"
    }
}

function Start-ExplorerProcess {
    Write-Step "Restarting Windows Explorer..."
    Start-Process explorer.exe
    Start-Sleep -Seconds 2
    Write-Success "Explorer restarted"
}

function Backup-Installation {
    param([string]$BackupPath)
    
    if (Test-Path $InstallPath) {
        Write-Step "Creating backup of existing installation..."
        $BackupDir = "$BackupPath\DarkThumbs_Backup_$(Get-Date -Format 'yyyyMMdd_HHmmss')"
        
        try {
            Copy-Item -Path $InstallPath -Destination $BackupDir -Recurse -Force
            Write-Success "Backup created: $BackupDir"
            return $BackupDir
        } catch {
            Write-Warning "Backup failed: $_"
            return $null
        }
    } else {
        Write-Success "No existing installation found"
        return $null
    }
}

function Unregister-ShellExtension {
    Write-Step "Unregistering shell extension..."
    
    try {
        # Unregister DLL using regsvr32
        $dllPath = Join-Path $InstallPath "CBXShell.dll"
        if (Test-Path $dllPath) {
            $result = Start-Process regsvr32.exe -ArgumentList "/u /s `"$dllPath`"" -Wait -PassThru
            if ($result.ExitCode -eq 0) {
                Write-Success "Shell extension unregistered"
            } else {
                Write-Warning "Unregistration returned code $($result.ExitCode)"
            }
        } else {
            Write-Warning "CBXShell.dll not found at: $dllPath"
        }
        
        # Clean up registry keys
        foreach ($key in $RegistryKeys) {
            if (Test-Path $key) {
                Remove-Item -Path $key -Recurse -Force -ErrorAction SilentlyContinue
                Write-Success "Removed registry key: $key"
            }
        }
        
    } catch {
        Write-Warning "Unregistration error: $_"
    }
}

function Register-ShellExtension {
    Write-Step "Registering shell extension..."
    
    $dllPath = Join-Path $InstallPath "CBXShell.dll"
    
    if (-not (Test-Path $dllPath)) {
        throw "CBXShell.dll not found at: $dllPath"
    }
    
    try {
        $result = Start-Process regsvr32.exe -ArgumentList "/s `"$dllPath`"" -Wait -PassThru
        if ($result.ExitCode -eq 0) {
            Write-Success "Shell extension registered"
        } else {
            throw "Registration failed with code $($result.ExitCode)"
        }
    } catch {
        throw "Registration error: $_"
    }
}

function Copy-InstallationFiles {
    Write-Step "Copying installation files..."
    
    # Create installation directory
    if (-not (Test-Path $InstallPath)) {
        New-Item -ItemType Directory -Path $InstallPath -Force | Out-Null
        Write-Success "Created installation directory: $InstallPath"
    }
    
    # Copy files
    foreach ($file in $SourceFiles.Keys) {
        $sourcePath = Join-Path $ScriptRoot $SourceFiles[$file]
        $destPath = Join-Path $InstallPath $file
        
        if (-not (Test-Path $sourcePath)) {
            throw "Source file not found: $sourcePath"
        }
        
        Copy-Item -Path $sourcePath -Destination $destPath -Force
        Write-Success "Copied: $file"
    }
    
    # Copy SDK if exists
    $sdkSource = Join-Path $ScriptRoot "SDK"
    if (Test-Path $sdkSource) {
        $sdkDest = Join-Path $InstallPath "SDK"
        Copy-Item -Path $sdkSource -Destination $sdkDest -Recurse -Force
        Write-Success "Copied SDK"
    }
    
    # Copy documentation
    $docsSource = Join-Path $ScriptRoot "docs"
    if (Test-Path $docsSource) {
        $docsDest = Join-Path $InstallPath "docs"
        Copy-Item -Path $docsSource -Destination $docsDest -Recurse -Force
        Write-Success "Copied documentation"
    }
}

function Set-RegistryValues {
    Write-Step "Configuring registry settings..."
    
    $rootKey = "HKLM:\Software\DarkThumbs"
    
    if (-not (Test-Path $rootKey)) {
        New-Item -Path $rootKey -Force | Out-Null
    }
    
    Set-ItemProperty -Path $rootKey -Name "InstallPath" -Value $InstallPath
    Set-ItemProperty -Path $rootKey -Name "Version" -Value $Version
    Set-ItemProperty -Path $rootKey -Name "InstallDate" -Value (Get-Date -Format "yyyy-MM-dd HH:mm:ss")
    
    Write-Success "Registry configured"
}

function Clear-ThumbnailCache {
    Write-Step "Clearing thumbnail cache..."
    
    try {
        # Clear Windows thumbnail cache
        $cachePaths = @(
            "$env:LOCALAPPDATA\Microsoft\Windows\Explorer\thumbcache_*.db",
            "$env:LOCALAPPDATA\Microsoft\Windows\Explorer\iconcache_*.db"
        )
        
        foreach ($pattern in $cachePaths) {
            Get-ChildItem -Path (Split-Path $pattern) -Filter (Split-Path $pattern -Leaf) -ErrorAction SilentlyContinue | 
                Remove-Item -Force -ErrorAction SilentlyContinue
        }
        
        Write-Success "Thumbnail cache cleared"
    } catch {
        Write-Warning "Cache clear failed: $_"
    }
}

function Test-Installation {
    Write-Step "Verifying installation..."
    
    $allGood = $true
    
    # Check files
    foreach ($file in $SourceFiles.Keys) {
        $path = Join-Path $InstallPath $file
        if (Test-Path $path) {
            Write-Success "Found: $file"
        } else {
            Write-Error "Missing: $file"
            $allGood = $false
        }
    }
    
    # Check registry
    $rootKey = "HKLM:\Software\DarkThumbs"
    if (Test-Path $rootKey) {
        $version = Get-ItemProperty -Path $rootKey -Name "Version" -ErrorAction SilentlyContinue
        if ($version.Version -eq $Version) {
            Write-Success "Registry version: $Version"
        } else {
            Write-Warning "Registry version mismatch"
            $allGood = $false
        }
    } else {
        Write-Error "Registry key missing"
        $allGood = $false
    }
    
    return $allGood
}

# ============================================================================
# Main Installation Logic
# ============================================================================

Write-Host ""
Write-Host "============================================" -ForegroundColor Cyan
Write-Host "  DarkThumbs Installer v$Version" -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

# Check administrator rights
if (-not (Test-Administrator)) {
    Write-Error "This script must be run as Administrator"
    Write-Host ""
    Write-Host "Please run: " -NoNewline
    Write-Host "Start-Process powershell -Verb RunAs -ArgumentList `"-File '$PSCommandPath'`"" -ForegroundColor Yellow
    exit 1
}

# Verify build files exist
Write-Step "Verifying build files..."
$missingFiles = @()
foreach ($file in $SourceFiles.Keys) {
    $sourcePath = Join-Path $ScriptRoot $SourceFiles[$file]
    if (-not (Test-Path $sourcePath)) {
        $missingFiles += $sourcePath
    }
}

if ($missingFiles.Count -gt 0) {
    Write-Error "Required build files missing:"
    foreach ($file in $missingFiles) {
        Write-Host "  - $file" -ForegroundColor Red
    }
    Write-Host ""
    Write-Host "Please build the project first:" -ForegroundColor Yellow
    Write-Host "  msbuild CBXShell.sln /p:Configuration=Release /p:Platform=x64" -ForegroundColor Gray
    exit 1
}
Write-Success "All build files found"

# Handle uninstall
if ($Uninstall) {
    Write-Host ""
    Write-Host "UNINSTALLING DarkThumbs..." -ForegroundColor Yellow
    Write-Host ""
    
    Stop-ExplorerProcess
    
    try {
        Unregister-ShellExtension
        Clear-ThumbnailCache
        
        if (Test-Path $InstallPath) {
            Remove-Item -Path $InstallPath -Recurse -Force
            Write-Success "Installation directory removed"
        }
        
        Write-Host ""
        Write-Host "Uninstallation completed successfully!" -ForegroundColor Green
    } catch {
        Write-Error "Uninstallation failed: $_"
        exit 1
    } finally {
        Start-ExplorerProcess
    }
    
    exit 0
}

# Handle installation/upgrade
Write-Host ""
if ($Upgrade -or (Test-Path $InstallPath)) {
    Write-Host "UPGRADING DarkThumbs..." -ForegroundColor Yellow
} else {
    Write-Host "INSTALLING DarkThumbs..." -ForegroundColor Green
}
Write-Host ""

# Create backup if needed
$backupPath = $null
if ((Test-Path $InstallPath) -and (-not $SkipBackup)) {
    $backupPath = Backup-Installation -BackupPath (Join-Path $env:TEMP "DarkThumbs_Backups")
}

# Stop Explorer
Stop-ExplorerProcess

try {
    # Unregister old version
    if (Test-Path $InstallPath) {
        Unregister-ShellExtension
    }
    
    # Copy files
    Copy-InstallationFiles
    
    # Register new version
    Register-ShellExtension
    
    # Configure registry
    Set-RegistryValues
    
    # Clear cache
    Clear-ThumbnailCache
    
    # Verify installation
    Write-Host ""
    if (Test-Installation) {
        Write-Host ""
        Write-Host "============================================" -ForegroundColor Green
        Write-Host "  Installation completed successfully!" -ForegroundColor Green
        Write-Host "============================================" -ForegroundColor Green
        Write-Host ""
        Write-Host "DarkThumbs $Version is now installed at:" -ForegroundColor White
        Write-Host "  $InstallPath" -ForegroundColor Gray
        Write-Host ""
        Write-Host "Run CBXManager.exe to configure settings." -ForegroundColor Cyan
        
        if ($backupPath) {
            Write-Host ""
            Write-Host "Backup created at: $backupPath" -ForegroundColor Yellow
        }
    } else {
        Write-Host ""
        Write-Host "Installation completed with warnings." -ForegroundColor Yellow
        Write-Host "Please check the output above for details." -ForegroundColor Yellow
        
        if ($backupPath) {
            Write-Host ""
            Write-Host "To restore backup:" -ForegroundColor Yellow
            Write-Host "  Copy-Item '$backupPath' '$InstallPath' -Recurse -Force" -ForegroundColor Gray
        }
    }
    
} catch {
    Write-Host ""
    Write-Host "============================================" -ForegroundColor Red
    Write-Host "  Installation FAILED" -ForegroundColor Red
    Write-Host "============================================" -ForegroundColor Red
    Write-Host ""
    Write-Error "Error: $_"
    
    if ($backupPath) {
        Write-Host ""
        Write-Host "Backup is available at: $backupPath" -ForegroundColor Yellow
    }
    
    exit 1
} finally {
    # Always restart Explorer
    Start-ExplorerProcess
}

Write-Host ""
