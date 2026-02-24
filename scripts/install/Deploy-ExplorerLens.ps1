# ExplorerLens Deployment Script
# PowerShell script for automated installation and configuration

#Requires -RunAsAdministrator

param(
    [switch]$Uninstall,
    [switch]$Update,
    [switch]$ClearCache,
    [switch]$Verify,
    [string]$SourcePath = "$PSScriptRoot\..\x64\Release"
)

$ErrorActionPreference = "Stop"

# Colors
function Write-Success { param($msg) Write-Host "[OK] $msg" -ForegroundColor Green }
function Write-Info { param($msg) Write-Host "[INFO] $msg" -ForegroundColor Cyan }
function Write-Warning { param($msg) Write-Host "[WARN] $msg" -ForegroundColor Yellow }
function Write-ErrorMsg { param($msg) Write-Host "[FAIL] $msg" -ForegroundColor Red }

# Banner
function Show-Banner {
    Write-Host ""
    Write-Host "+========================================+" -ForegroundColor Cyan
    Write-Host "|   ExplorerLens v5.2.0+ Deployment Tool   |" -ForegroundColor Cyan
    Write-Host "+========================================+" -ForegroundColor Cyan
    Write-Host ""
}

# Check prerequisites
function Test-Prerequisites {
    Write-Info "Checking prerequisites..."
    
    # Check Windows version
    $os = Get-CimInstance Win32_OperatingSystem
    $build = [int]$os.BuildNumber
    
    if ($build -lt 17134) {  # Windows 10 1803
        Write-ErrorMsg "Windows 10 version 1803 or later required (current: $build)"
        return $false
    }
    
    Write-Success "Windows version OK (Build $build)"
    
    # Check architecture
    if ([Environment]::Is64BitOperatingSystem -eq $false) {
        Write-ErrorMsg "64-bit Windows required"
        return $false
    }
    
    Write-Success "64-bit OS confirmed"
    
    # Check admin rights
    $isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
    if (-not $isAdmin) {
        Write-ErrorMsg "Administrator privileges required"
        return $false
    }
    
    Write-Success "Administrator rights confirmed"
    
    return $true
}

# Stop Windows Explorer
function Stop-Explorer {
    Write-Info "Stopping Windows Explorer..."
    
    $explorer = Get-Process explorer -ErrorAction SilentlyContinue
    if ($explorer) {
        Stop-Process -Name explorer -Force
        Start-Sleep -Seconds 2
        Write-Success "Explorer stopped"
        return $true
    }
    
    return $false
}

# Start Windows Explorer
function Start-Explorer {
    Write-Info "Starting Windows Explorer..."
    Start-Process explorer.exe
    Start-Sleep -Seconds 1
    Write-Success "Explorer started"
}

# Clear thumbnail cache
function Clear-ThumbnailCache {
    Write-Info "Clearing thumbnail cache..."
    
    $cachePath = "$env:LOCALAPPDATA\Microsoft\Windows\Explorer"
    $cacheFiles = Get-ChildItem -Path $cachePath -Filter "thumbcache_*.db" -ErrorAction SilentlyContinue
    
    $count = 0
    foreach ($file in $cacheFiles) {
        try {
            Remove-Item $file.FullName -Force
            $count++
        } catch {
            Write-Warning "Could not delete $($file.Name): $_"
        }
    }
    
    Write-Success "Cleared $count thumbnail cache file(s)"
}

# Uninstall ExplorerLens
function Uninstall-ExplorerLens {
    Write-Info "Uninstalling ExplorerLens..."
    
    $systemDll = "$env:SystemRoot\System32\LENSShell.dll"
    $managerExe = "$env:ProgramFiles\ExplorerLens\LENSManager.exe"
    $managerDir = "$env:ProgramFiles\ExplorerLens"
    
    # Unregister DLL
    if (Test-Path $systemDll) {
        try {
            Write-Info "Unregistering COM component..."
            $result = Start-Process regsvr32.exe -ArgumentList "/u", "/s", $systemDll -Wait -PassThru -NoNewWindow
            if ($result.ExitCode -eq 0) {
                Write-Success "COM component unregistered"
            }
        } catch {
            Write-Warning "Error unregistering: $_"
        }
        
        # Delete DLL
        try {
            Remove-Item $systemDll -Force
            Write-Success "Removed LENSShell.dll"
        } catch {
            Write-ErrorMsg "Could not remove LENSShell.dll: $_"
        }
    } else {
        Write-Info "LENSShell.dll not found (already uninstalled)"
    }
    
    # Remove manager
    if (Test-Path $managerDir) {
        try {
            Remove-Item $managerDir -Recurse -Force
            Write-Success "Removed ExplorerLens directory"
        } catch {
            Write-Warning "Could not remove ExplorerLens directory: $_"
        }
    }
}

# Install ExplorerLens
function Install-ExplorerLens {
    param([string]$source)
    
    Write-Info "Installing ExplorerLens..."
    
    $dllSource = Join-Path $source "LENSShell.dll"
    $exeSource = Join-Path $source "LENSManager.exe"
    
    # Verify source files exist
    if (-not (Test-Path $dllSource)) {
        Write-ErrorMsg "LENSShell.dll not found in $source"
        return $false
    }
    
    if (-not (Test-Path $exeSource)) {
        Write-ErrorMsg "LENSManager.exe not found in $source"
        return $false
    }
    
    Write-Success "Source files verified"
    
    # Copy DLL to System32
    $systemDll = "$env:SystemRoot\System32\LENSShell.dll"
    try {
        Copy-Item $dllSource $systemDll -Force
        Write-Success "Copied LENSShell.dll to System32"
        
        # Verify file
        $dllInfo = Get-Item $systemDll
        Write-Info "  Size: $([math]::Round($dllInfo.Length/1MB, 2)) MB"
        Write-Info "  Version: $($dllInfo.VersionInfo.FileVersion)"
    } catch {
        Write-ErrorMsg "Failed to copy DLL: $_"
        return $false
    }
    
    # Register COM component
    try {
        Write-Info "Registering COM component..."
        $result = Start-Process regsvr32.exe -ArgumentList "/s", $systemDll -Wait -PassThru -NoNewWindow
        if ($result.ExitCode -eq 0) {
            Write-Success "COM component registered successfully"
        } else {
            Write-ErrorMsg "COM registration failed (Exit code: $($result.ExitCode))"
            return $false
        }
    } catch {
        Write-ErrorMsg "Error registering COM: $_"
        return $false
    }
    
    # Copy manager exe
    $managerDir = "$env:ProgramFiles\ExplorerLens"
    if (-not (Test-Path $managerDir)) {
        New-Item -ItemType Directory -Path $managerDir -Force | Out-Null
    }
    
    try {
        Copy-Item $exeSource "$managerDir\LENSManager.exe" -Force
        Write-Success "Installed LENSManager.exe"
    } catch {
        Write-Warning "Could not install LENSManager.exe: $_"
    }
    
    return $true
}

# Verify installation
function Test-Installation {
    Write-Info "Verifying installation..."
    
    $systemDll = "$env:SystemRoot\System32\LENSShell.dll"
    
    # Check DLL exists
    if (-not (Test-Path $systemDll)) {
        Write-ErrorMsg "LENSShell.dll not found"
        return $false
    }
    
    Write-Success "LENSShell.dll present"
    
    # Check COM registration
    $clsid = "{3CA50AA2-92F6-4FD6-82C2-37E0AF7F967C}"
    $regPath = "Registry::HKEY_CLASSES_ROOT\CLSID\$clsid"
    
    if (Test-Path $regPath) {
        Write-Success "COM component registered"
        
        $inprocServer = Get-ItemProperty -Path "$regPath\InprocServer32" -ErrorAction SilentlyContinue
        if ($inprocServer) {
            Write-Info "  Location: $($inprocServer.'(default)')"
        }
    } else {
        Write-ErrorMsg "COM component not registered"
        return $false
    }
    
    # Check file properties
    $dllInfo = Get-Item $systemDll
    Write-Info "File Information:"
    Write-Info "  Size: $([math]::Round($dllInfo.Length/1MB, 2)) MB"
    Write-Info "  Modified: $($dllInfo.LastWriteTime)"
    
    if ($dllInfo.VersionInfo.FileVersion) {
        Write-Info "  Version: $($dllInfo.VersionInfo.FileVersion)"
    }
    
    return $true
}

# Show status
function Show-Status {
    Write-Info "Current Status:"
    
    $systemDll = "$env:SystemRoot\System32\LENSShell.dll"
    
    if (Test-Path $systemDll) {
        Write-Success "ExplorerLens is installed"
        
        $dllInfo = Get-Item $systemDll
        Write-Info "  DLL Size: $([math]::Round($dllInfo.Length/1MB, 2)) MB"
        Write-Info "  Modified: $($dllInfo.LastWriteTime)"
        
        $clsid = "{3CA50AA2-92F6-4FD6-82C2-37E0AF7F967C}"
        if (Test-Path "Registry::HKEY_CLASSES_ROOT\CLSID\$clsid") {
            Write-Success "  COM registration: Active"
        } else {
            Write-Warning "  COM registration: Missing"
        }
        
        $managerPath = "$env:ProgramFiles\ExplorerLens\LENSManager.exe"
        if (Test-Path $managerPath) {
            Write-Success "  Configuration tool: Installed"
        } else {
            Write-Info "  Configuration tool: Not installed"
        }
    } else {
        Write-Info "ExplorerLens is not installed"
    }
}

# Main execution
try {
    Show-Banner
    
    if (-not (Test-Prerequisites)) {
        Write-ErrorMsg "Prerequisites check failed"
        exit 1
    }
    
    Write-Info ""
    
    # Handle different modes
    if ($Verify) {
        # Verification mode
        if (Test-Installation) {
            Write-Success "Installation verification passed"
            exit 0
        } else {
            Write-ErrorMsg "Installation verification failed"
            exit 1
        }
    }
    elseif ($ClearCache) {
        # Clear cache mode
        $explorerStopped = Stop-Explorer
        Clear-ThumbnailCache
        if ($explorerStopped) {
            Start-Explorer
        }
        Write-Success "Cache cleared successfully"
        exit 0
    }
    elseif ($Uninstall) {
        # Uninstall mode
        $explorerStopped = Stop-Explorer
        Uninstall-ExplorerLens
        Clear-ThumbnailCache
        if ($explorerStopped) {
            Start-Explorer
        }
        Write-Success "Uninstallation completed"
        Write-Info ""
        Show-Status
        exit 0
    }
    else {
        # Install/Update mode
        $explorerStopped = Stop-Explorer
        
        if ($Update) {
            Write-Info "Performing update..."
            Uninstall-ExplorerLens
        }
        
        if (Install-ExplorerLens -source $SourcePath) {
            Clear-ThumbnailCache
            
            if ($explorerStopped) {
                Start-Explorer
            }
            
            Write-Info ""
            if (Test-Installation) {
                Write-Success "Installation completed successfully"
                Write-Info ""
                Write-Info "Next Steps:"
                Write-Info "  1. Navigate to a folder with WebP/HEIF/AVIF/PDF/Video/RAR files"
                Write-Info "  2. Switch to Large Icons or Extra Large Icons view"
                Write-Info "  3. Thumbnails should appear automatically"
                Write-Info ""
                exit 0
            } else {
                Write-ErrorMsg "Installation verification failed"
                exit 1
            }
        } else {
            Write-ErrorMsg "Installation failed"
            if ($explorerStopped) {
                Start-Explorer
            }
            exit 1
        }
    }
    
} catch {
    Write-ErrorMsg "Deployment error: $_"
    Write-ErrorMsg $_.ScriptStackTrace
    
    # Ensure Explorer is running
    if (-not (Get-Process explorer -ErrorAction SilentlyContinue)) {
        Start-Explorer
    }
    
    exit 1
}

