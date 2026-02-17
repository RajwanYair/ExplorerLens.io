# Build-Helpers.ps1
# DarkThumbs v7.0 - Build Helper Module
# Provides vcpkg integration and additional helper functions
#
# USAGE:
#   . "$PSScriptRoot/../core/Build-Helpers.ps1"
#   Test-VcpkgInstalled
#   Install-VcpkgIfNeeded
#
# Date: February 16, 2026
# Author: DarkThumbs Development Team

$ErrorActionPreference = 'Stop'

# ============================================================================
# vcpkg Integration Functions
# ============================================================================

function Test-VcpkgInstalled {
    <#
    .SYNOPSIS
        Checks if vcpkg is installed and configured
    .OUTPUTS
        Boolean indicating if vcpkg is available
    #>
    [CmdletBinding()]
    param()
    
    # Check for vcpkg in Visual Studio installation (highest priority)
    $vsVcpkgPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\vcpkg\vcpkg.exe"
    if (Test-Path $vsVcpkgPath) {
        Write-Host "✓ vcpkg found in Visual Studio: $vsVcpkgPath" -ForegroundColor Green
        $vcpkgDir = Split-Path $vsVcpkgPath -Parent
        $env:PATH = "$vcpkgDir;$env:PATH"
        $env:VCPKG_ROOT = $vcpkgDir
        return $true
    }
    
    # Check for vcpkg in PATH
    $vcpkgCmd = Get-Command vcpkg -ErrorAction SilentlyContinue
    if ($vcpkgCmd) {
        Write-Host "✓ vcpkg found in PATH: $($vcpkgCmd.Source)" -ForegroundColor Green
        return $true
    }
    
    # Check common installation locations
    $commonPaths = @(
        "$env:VCPKG_ROOT\vcpkg.exe",
        "$env:USERPROFILE\vcpkg\vcpkg.exe",
        "$env:LOCALAPPDATA\vcpkg\vcpkg.exe",
        "C:\vcpkg\vcpkg.exe",
        "C:\tools\vcpkg\vcpkg.exe"
    )
    
    foreach ($path in $commonPaths) {
        if (Test-Path $path) {
            Write-Host "✓ vcpkg found at: $path" -ForegroundColor Green
            # Add to PATH for this session
            $vcpkgDir = Split-Path $path -Parent
            $env:PATH = "$vcpkgDir;$env:PATH"
            return $true
        }
    }
    
    Write-Host "✗ vcpkg not found" -ForegroundColor Yellow
    return $false
}

function Get-VcpkgPath {
    <#
    .SYNOPSIS
        Gets the vcpkg installation directory
    .OUTPUTS
        String path to vcpkg directory or $null if not found
    #>
    [CmdletBinding()]
    param()
    
    # Check Visual Studio installation first
    $vsVcpkgPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\vcpkg"
    if (Test-Path (Join-Path $vsVcpkgPath "vcpkg.exe")) {
        return $vsVcpkgPath
    }
    
    # Check PATH
    $vcpkgCmd = Get-Command vcpkg -ErrorAction SilentlyContinue
    if ($vcpkgCmd) {
        return Split-Path $vcpkgCmd.Source -Parent
    }
    
    # Check common locations
    $commonPaths = @(
        "$env:VCPKG_ROOT",
        "$env:USERPROFILE\vcpkg",
        "$env:LOCALAPPDATA\vcpkg",
        "C:\vcpkg",
        "C:\tools\vcpkg"
    )
    
    foreach ($path in $commonPaths) {
        if (Test-Path (Join-Path $path "vcpkg.exe")) {
            return $path
        }
    }
    
    return $null
}

function Install-VcpkgIfNeeded {
    <#
    .SYNOPSIS
        Downloads and installs vcpkg if not already installed
    .PARAMETER InstallPath
        Directory to install vcpkg (default: $env:USERPROFILE\vcpkg)
    .PARAMETER Force
        Force reinstall even if vcpkg exists
    #>
    [CmdletBinding()]
    param(
        [Parameter()]
        [string]$InstallPath = "$env:USERPROFILE\vcpkg",
        
        [Parameter()]
        [switch]$Force
    )
    
    Write-Host ""
    Write-Host ("=" * 80) -ForegroundColor Cyan
    Write-Host "  vcpkg Package Manager Setup" -ForegroundColor Cyan
    Write-Host ("=" * 80) -ForegroundColor Cyan
    Write-Host ""
    
    # Check if already installed
    if (-not $Force -and (Test-VcpkgInstalled)) {
        Write-Host "✓ vcpkg is already installed and configured" -ForegroundColor Green
        $vcpkgPath = Get-VcpkgPath
        Write-Host "  Location: $vcpkgPath" -ForegroundColor Gray
        return $vcpkgPath
    }
    
    # Check for git (required to clone vcpkg)
    $git = Get-Command git -ErrorAction SilentlyContinue
    if (-not $git) {
        Write-Host "✗ Git is required to install vcpkg" -ForegroundColor Red
        Write-Host "  Please install Git from: https://git-scm.com/downloads" -ForegroundColor Yellow
        throw "Git not found"
    }
    
    Write-Host "Installing vcpkg to: $InstallPath" -ForegroundColor Cyan
    
    # Create parent directory
    $parentDir = Split-Path $InstallPath -Parent
    if (-not (Test-Path $parentDir)) {
        New-Item -ItemType Directory -Path $parentDir -Force | Out-Null
    }
    
    try {
        # Clone vcpkg repository
        if (Test-Path $InstallPath) {
            if ($Force) {
                Write-Host "Removing existing vcpkg installation..." -ForegroundColor Yellow
                Remove-Item -Path $InstallPath -Recurse -Force
            } else {
                throw "vcpkg directory already exists at $InstallPath. Use -Force to reinstall."
            }
        }
        
        Write-Host "Cloning vcpkg repository..." -ForegroundColor Cyan
        & git clone https://github.com/microsoft/vcpkg.git $InstallPath
        
        if ($LASTEXITCODE -ne 0) {
            throw "Failed to clone vcpkg repository"
        }
        
        # Bootstrap vcpkg
        Write-Host "Bootstrapping vcpkg..." -ForegroundColor Cyan
        $bootstrapScript = Join-Path $InstallPath "bootstrap-vcpkg.bat"
        
        if (-not (Test-Path $bootstrapScript)) {
            throw "Bootstrap script not found at: $bootstrapScript"
        }
        
        Push-Location $InstallPath
        try {
            & $bootstrapScript -disableMetrics
            
            if ($LASTEXITCODE -ne 0) {
                throw "vcpkg bootstrap failed"
            }
        } finally {
            Pop-Location
        }
        
        # Integrate vcpkg with Visual Studio
        Write-Host "Integrating vcpkg with Visual Studio..." -ForegroundColor Cyan
        $vcpkgExe = Join-Path $InstallPath "vcpkg.exe"
        & $vcpkgExe integrate install
        
        # Add to PATH for this session
        $env:PATH = "$InstallPath;$env:PATH"
        
        # Suggest adding to user PATH permanently
        Write-Host ""
        Write-Host "✓ vcpkg installed successfully!" -ForegroundColor Green
        Write-Host ""
        Write-Host "To add vcpkg to your PATH permanently, run:" -ForegroundColor Yellow
        Write-Host "  [System.Environment]::SetEnvironmentVariable('Path', `$env:Path + ';$InstallPath', 'User')" -ForegroundColor Gray
        Write-Host ""
        
        return $InstallPath
    } catch {
        Write-Host ""
        Write-Host "✗ vcpkg installation failed: $($_.Exception.Message)" -ForegroundColor Red
        throw
    }
}

function Install-VcpkgPackage {
    <#
    .SYNOPSIS
        Installs a package using vcpkg
    .PARAMETER PackageName
        Name of the package to install (e.g., "zlib:x64-windows-static")
    .PARAMETER Triplet
        vcpkg triplet (default: x64-windows-static)
    #>
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)]
        [string]$PackageName,
        
        [Parameter()]
        [string]$Triplet = "x64-windows-static"
    )
    
    # Ensure vcpkg is installed
    if (-not (Test-VcpkgInstalled)) {
        Write-Host "vcpkg not found. Installing..." -ForegroundColor Yellow
        Install-VcpkgIfNeeded
    }
    
    $vcpkgPath = Get-VcpkgPath
    $vcpkgExe = Join-Path $vcpkgPath "vcpkg.exe"
    
    # Add triplet if not specified in package name
    $fullPackageName = $PackageName
    if (-not $PackageName.Contains(":")) {
        $fullPackageName = "${PackageName}:${Triplet}"
    }
    
    Write-Host "Installing vcpkg package: $fullPackageName" -ForegroundColor Cyan
    
    & $vcpkgExe install $fullPackageName
    
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to install vcpkg package: $fullPackageName"
    }
    
    Write-Host "✓ Package installed successfully: $fullPackageName" -ForegroundColor Green
}

function Get-VcpkgPackageInfo {
    <#
    .SYNOPSIS
        Gets information about installed vcpkg packages
    .PARAMETER PackageName
        Optional package name to filter results
    #>
    [CmdletBinding()]
    param(
        [Parameter()]
        [string]$PackageName
    )
    
    if (-not (Test-VcpkgInstalled)) {
        Write-Host "✗ vcpkg not installed" -ForegroundColor Red
        return $null
    }
    
    $vcpkgPath = Get-VcpkgPath
    $vcpkgExe = Join-Path $vcpkgPath "vcpkg.exe"
    
    if ($PackageName) {
        & $vcpkgExe list $PackageName
    } else {
        & $vcpkgExe list
    }
}

# ============================================================================
# Git Helper Functions
# ============================================================================

function Test-GitInstalled {
    <#
    .SYNOPSIS
        Checks if Git is installed
    #>
    [CmdletBinding()]
    param()
    
    $git = Get-Command git -ErrorAction SilentlyContinue
    if ($git) {
        Write-Host "✓ Git found: $($git.Source)" -ForegroundColor Green
        return $true
    }
    
    Write-Host "✗ Git not found" -ForegroundColor Yellow
    return $false
}

function Initialize-GitSubmodules {
    <#
    .SYNOPSIS
        Initializes git submodules in the current or specified directory
    .PARAMETER Path
        Path to repository (default: current directory)
    #>
    [CmdletBinding()]
    param(
        [Parameter()]
        [string]$Path = "."
    )
    
    if (-not (Test-GitInstalled)) {
        Write-Host "✗ Git not installed, skipping submodule initialization" -ForegroundColor Yellow
        return $false
    }
    
    $gitDir = Join-Path $Path ".git"
    if (-not (Test-Path $gitDir)) {
        Write-Host "✗ Not a git repository: $Path" -ForegroundColor Yellow
        return $false
    }
    
    Write-Host "Initializing git submodules in: $Path" -ForegroundColor Cyan
    
    Push-Location $Path
    try {
        & git submodule update --init --recursive
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✓ Git submodules initialized" -ForegroundColor Green
            return $true
        } else {
            Write-Host "✗ Git submodule initialization failed" -ForegroundColor Yellow
            return $false
        }
    } catch {
        Write-Host "✗ Error: $($_.Exception.Message)" -ForegroundColor Yellow
        return $false
    } finally {
        Pop-Location
    }
}

# ============================================================================
# Environment Helper Functions
# ============================================================================

function Set-VisualStudioEnvironment {
    <#
    .SYNOPSIS
        Sets up Visual Studio environment variables (vcvars64.bat equivalent)
    .PARAMETER VSVersion
        Visual Studio version (2022, 2019, etc.)
    #>
    [CmdletBinding()]
    param(
        [Parameter()]
        [string]$VSVersion = "2022"
    )
    
    # Find vcvars64.bat
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    
    if (-not (Test-Path $vswhere)) {
        Write-Host "✗ Visual Studio not found (vswhere.exe missing)" -ForegroundColor Red
        return $false
    }
    
    $vsPath = & $vswhere -latest -property installationPath -version "[$VSVersion,$(([int]$VSVersion) + 1))"
    
    if (-not $vsPath) {
        Write-Host "✗ Visual Studio $VSVersion not found" -ForegroundColor Red
        return $false
    }
    
    $vcvarsPath = Join-Path $vsPath "VC\Auxiliary\Build\vcvars64.bat"
    
    if (-not (Test-Path $vcvarsPath)) {
        Write-Host "✗ vcvars64.bat not found at: $vcvarsPath" -ForegroundColor Red
        return $false
    }
    
    Write-Host "Setting up Visual Studio $VSVersion environment..." -ForegroundColor Cyan
    
    # Run vcvars64.bat and capture environment variables
    $output = cmd /c "`"$vcvarsPath`" && set" 2>&1
    
    foreach ($line in $output) {
        if ($line -match '^([^=]+)=(.*)$') {
            $name = $matches[1]
            $value = $matches[2]
            
            # Update environment variable
            [System.Environment]::SetEnvironmentVariable($name, $value, 'Process')
        }
    }
    
    Write-Host "✓ Visual Studio environment configured" -ForegroundColor Green
    return $true
}

function Test-CommandExists {
    <#
    .SYNOPSIS
        Tests if a command exists in PATH
    .PARAMETER CommandName
        Name of the command to test
    #>
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)]
        [string]$CommandName
    )
    
    $command = Get-Command $CommandName -ErrorAction SilentlyContinue
    return ($null -ne $command)
}

# ============================================================================
# Package Helper Functions
# ============================================================================

function Get-LibraryVersionFromFile {
    <#
    .SYNOPSIS
        Extracts version from a header file or CMakeLists.txt
    .PARAMETER FilePath
        Path to the file
    .PARAMETER VersionPattern
        Regex pattern to match version
    #>
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)]
        [string]$FilePath,
        
        [Parameter()]
        [string]$VersionPattern = 'VERSION[\s=]+"?(\d+\.\d+\.\d+)'
    )
    
    if (-not (Test-Path $FilePath)) {
        Write-Host "✗ File not found: $FilePath" -ForegroundColor Yellow
        return $null
    }
    
    $content = Get-Content $FilePath -Raw
    
    if ($content -match $VersionPattern) {
        return $matches[1]
    }
    
    return $null
}

function Compare-LibraryVersions {
    <#
    .SYNOPSIS
        Compares two semantic versions
    .OUTPUTS
        -1 if v1 < v2, 0 if equal, 1 if v1 > v2
    #>
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)]
        [string]$Version1,
        
        [Parameter(Mandatory)]
        [string]$Version2
    )
    
    try {
        $v1 = [version]$Version1
        $v2 = [version]$Version2
        
        return $v1.CompareTo($v2)
    } catch {
        Write-Host "✗ Invalid version format: $Version1 or $Version2" -ForegroundColor Yellow
        return 0
    }
}

# ============================================================================
# Functions are automatically available when dot-sourced
# No Export-ModuleMember needed for script files
# ============================================================================

Write-Host "✓ Build-Helpers.ps1 loaded successfully (v7.0)" -ForegroundColor Green
