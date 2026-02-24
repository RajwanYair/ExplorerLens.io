# ============================================================================
# ExplorerLens Development Environment Setup
# Version: 1.0 - February 9, 2026
# Auto-loaded in PowerShell profile for instant build tool availability
# ============================================================================

<#
.SYNOPSIS
    Configures complete build environment for ExplorerLens project

.DESCRIPTION
    Sets up all required tools and environment variables for building ExplorerLens:
    - Visual Studio 2026 Build Tools (MSVC 14.44.35207)
    - CMake 4.2.1
    - MSBuild 18.3.0
    - Git, Ninja, NMake
    - Windows SDK 10.0.26100.0

.EXAMPLE
    # In PowerShell profile, add:
    . "C:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\ExplorerLens\scripts\Setup-DevEnvironment.ps1"
    
.EXAMPLE
    # Manual load:
    Setup-ExplorerLensEnv -Verbose
#>

# ============================================================================
# Configuration
# ============================================================================

$Global:ExplorerLensConfig = @{
    # Visual Studio 2026 Build Tools
    VSPath      = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools"
    MSVCVersion = "14.44.35207"
    WindowsSDK  = "10.0.26100.0"
    
    # Build Tools (Updated Feb 9, 2026)
    MSBuild     = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe"
    CMake       = "C:\Users\ryair\scoop\shims\cmake.exe"  # v4.2.3
    Git         = "C:\Users\ryair\scoop\shims\git.exe"      # v2.53.0
    Ninja       = "C:\Users\ryair\scoop\shims\ninja.exe"  # v1.13.2
    
    # VC Tools
    VCVarsAll   = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
    VCVars64    = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    
    # Project Paths
    ProjectRoot = Split-Path -Parent $PSScriptRoot
}

# ============================================================================
# Main Setup Function
# ============================================================================

function Setup-ExplorerLensEnv {
    [CmdletBinding()]
    param(
        [switch]$Force,
        [switch]$Quiet
    )
    
    # Check if already loaded
    if ($Global:ExplorerLensEnvLoaded -and -not $Force) {
        if (-not $Quiet) {
            Write-Host "✓ ExplorerLens environment already loaded" -ForegroundColor Green
        }
        return
    }
    
    if (-not $Quiet) {
        Write-Host "`n" -NoNewline
        Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan
        Write-Host " ExplorerLens Development Environment Setup" -ForegroundColor Cyan
        Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan
    }
    
    # Load MSVC environment
    Load-MSVCEnvironment -Quiet:$Quiet
    
    # Verify all tools
    Test-BuildTools -Quiet:$Quiet
    
    # Set convenient aliases
    Set-ExplorerLensAliases
    
    # Mark as loaded
    $Global:ExplorerLensEnvLoaded = $true
    
    if (-not $Quiet) {
        Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan
        Write-Host "✓ Environment ready for ExplorerLens builds" -ForegroundColor Green
        Write-Host "  Type 'dtbuild' for quick build commands" -ForegroundColor Gray
        Write-Host "═══════════════════════════════════════════════════════`n" -ForegroundColor Cyan
    }
}

# ============================================================================
# MSVC Environment Loader
# ============================================================================

function Load-MSVCEnvironment {
    [CmdletBinding()]
    param([switch]$Quiet)
    
    # Check if already loaded
    if (Get-Command cl.exe -ErrorAction SilentlyContinue) {
        if (-not $Quiet) {
            Write-Host "✓ MSVC environment already active" -ForegroundColor Green
        }
        return
    }
    
    if (-not $Quiet) {
        Write-Host "Loading MSVC 14.44 (VS 2026 Build Tools)..." -ForegroundColor Yellow
    }
    
    # Load vcvars64 and capture environment
    $tempFile = [System.IO.Path]::GetTempFileName()
    
    try {
        # Run vcvarsall for x64 and capture environment vars
        $vcvarsCmd = "`"$($Global:ExplorerLensConfig.VCVarsAll)`" x64"
        cmd /c "$vcvarsCmd >nul 2>&1 && set" | Out-File -FilePath $tempFile -Encoding ASCII
        
        # Parse and set environment variables
        Get-Content $tempFile | ForEach-Object {
            if ($_ -match "^([^=]+)=(.*)$") {
                $varName = $matches[1]
                $varValue = $matches[2]
                
                # Set in current environment
                [System.Environment]::SetEnvironmentVariable($varName, $varValue, 'Process')
                
                # Also update PowerShell env: drive
                Set-Item -Path "env:$varName" -Value $varValue -Force -ErrorAction SilentlyContinue
            }
        }
        
        if (-not $Quiet) {
            Write-Host "✓ MSVC environment loaded successfully" -ForegroundColor Green
            Write-Host "  - Compiler: MSVC $($Global:ExplorerLensConfig.MSVCVersion)" -ForegroundColor Gray
            Write-Host "  - Windows SDK: $($Global:ExplorerLensConfig.WindowsSDK)" -ForegroundColor Gray
            Write-Host "  - Tools: CL, Link, NMake, RC" -ForegroundColor Gray
        }
    } catch {
        Write-Error "Failed to load MSVC environment: $_"
    } finally {
        Remove-Item $tempFile -Force -ErrorAction SilentlyContinue
    }
}

# ============================================================================
# Build Tool Verification
# ============================================================================

function Test-BuildTools {
    [CmdletBinding()]
    param([switch]$Quiet)
    
    if (-not $Quiet) {
        Write-Host "`nVerifying build tools..." -ForegroundColor Yellow
    }
    
    $tools = @(
        @{ Name = "MSBuild"; Command = "msbuild"; Expected = "18.3" }
        @{ Name = "CMake"; Command = "cmake"; Expected = "4.2" }
        @{ Name = "Git"; Command = "git"; Expected = "2." }
        @{ Name = "MSVC (CL)"; Command = "cl"; Expected = "19." }
        @{ Name = "NMake"; Command = "nmake"; Expected = "" }
        @{ Name = "Link"; Command = "link"; Expected = "" }
        @{ Name = "RC"; Command = "rc"; Expected = "" }
        @{ Name = "Ninja"; Command = "ninja"; Expected = "1." }
    )
    
    $allGood = $true
    
    foreach ($tool in $tools) {
        $cmd = Get-Command $tool.Command -ErrorAction SilentlyContinue
        
        if ($cmd) {
            if (-not $Quiet) {
                Write-Host "  ✅ $($tool.Name)" -ForegroundColor Green -NoNewline
                
                # Try to get version
                $version = ""
                try {
                    if ($tool.Command -eq "msbuild") {
                        $version = (& $tool.Command -version 2>&1 | Select-String -Pattern "\d+\.\d+\.\d+" | Select-Object -First 1).Matches.Value
                    } elseif ($tool.Command -eq "cmake") {
                        $version = (& $tool.Command --version 2>&1 | Select-String -Pattern "\d+\.\d+\.\d+" | Select-Object -First 1).Matches.Value
                    } elseif ($tool.Command -eq "git") {
                        $version = (& $tool.Command --version 2>&1 | Select-String -Pattern "\d+\.\d+\.\d+" | Select-Object -First 1).Matches.Value
                    }
                    
                    if ($version) {
                        Write-Host " ($version)" -ForegroundColor Gray
                    } else {
                        Write-Host ""
                    }
                } catch {
                    Write-Host ""
                }
            }
        } else {
            $allGood = $false
            if (-not $Quiet) {
                Write-Host "  ❌ $($tool.Name) - NOT FOUND" -ForegroundColor Red
            }
        }
    }
    
    return $allGood
}

# ============================================================================
# Convenient Aliases and Functions
# ============================================================================

function Set-ExplorerLensAliases {
    # Build shortcuts
    Set-Alias -Name dtbuild -Value Invoke-ExplorerLensBuild -Scope Global -ErrorAction SilentlyContinue
    Set-Alias -Name dtclean -Value Invoke-ExplorerLensClean -Scope Global -ErrorAction SilentlyContinue
    Set-Alias -Name dttest -Value Invoke-ExplorerLensTest -Scope Global -ErrorAction SilentlyContinue
}

function Invoke-ExplorerLensBuild {
    <#
    .SYNOPSIS
        Quick build commands for ExplorerLens
    #>
    param(
        [Parameter(Position = 0)]
        [ValidateSet("Release", "Debug", "Engine", "Shell", "Clean", "Rebuild", "Help")]
        [string]$Target = "Help"
    )
    
    $projectRoot = $Global:ExplorerLensConfig.ProjectRoot
    
    switch ($Target) {
        "Release" {
            Write-Host "Building ExplorerLens (Release)..." -ForegroundColor Cyan
            Push-Location $projectRoot
            msbuild LENSShell.sln /p:Configuration=Release /p:Platform=x64 /m /v:minimal
            Pop-Location
        }
        "Debug" {
            Write-Host "Building ExplorerLens (Debug)..." -ForegroundColor Cyan
            Push-Location $projectRoot
            msbuild LENSShell.sln /p:Configuration=Debug /p:Platform=x64 /m /v:minimal
            Pop-Location
        }
        "Engine" {
            Write-Host "Building Engine..." -ForegroundColor Cyan
            Push-Location "$projectRoot\Engine"
            if (-not (Test-Path "build")) { mkdir build }
            Set-Location build
            cmake .. -G "Visual Studio 18 2026" -A x64
            cmake --build . --config Release -j 8
            Pop-Location
        }
        "Shell" {
            Write-Host "Building LENSShell..." -ForegroundColor Cyan
            Push-Location $projectRoot
            msbuild LENSShell\LENSShell.vcxproj /p:Configuration=Release /p:Platform=x64 /m
            Pop-Location
        }
        "Clean" {
            Write-Host "Cleaning build artifacts..." -ForegroundColor Yellow
            Push-Location $projectRoot
            Remove-Item -Path build, x64, packages, LENSShell\x64, LENSManager\x64 -Recurse -Force -ErrorAction SilentlyContinue
            Write-Host "✓ Clean complete" -ForegroundColor Green
            Pop-Location
        }
        "Rebuild" {
            Invoke-ExplorerLensBuild -Target Clean
            Invoke-ExplorerLensBuild -Target Release
        }
        default {
            Write-Host "`nExplorerLens Quick Build Commands:" -ForegroundColor Cyan
            Write-Host "  dtbuild Release  - Build full solution (Release)"
            Write-Host "  dtbuild Debug    - Build full solution (Debug)"
            Write-Host "  dtbuild Engine   - Build Engine only (CMake)"
            Write-Host "  dtbuild Shell    - Build LENSShell only"
            Write-Host "  dtbuild Clean    - Clean all build outputs"
            Write-Host "  dtbuild Rebuild  - Clean + Release build"
            Write-Host ""
        }
    }
}

function Invoke-ExplorerLensClean {
    Invoke-ExplorerLensBuild -Target Clean
}

function Invoke-ExplorerLensTest {
    Write-Host "Running ExplorerLens tests..." -ForegroundColor Cyan
    $projectRoot = $Global:ExplorerLensConfig.ProjectRoot
    Push-Location "$projectRoot\Engine\build"
    ctest --output-on-failure
    Pop-Location
}

# ============================================================================
# Show-ExplorerLensInfo - Display environment information
# ============================================================================

function Show-ExplorerLensInfo {
    Write-Host "`n" -NoNewline
    Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan
    Write-Host " ExplorerLens Development Environment" -ForegroundColor Cyan
    Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Visual Studio:" -ForegroundColor Yellow
    Write-Host "  Path:    $($Global:ExplorerLensConfig.VSPath)" -ForegroundColor Gray
    Write-Host "  MSVC:    $($Global:ExplorerLensConfig.MSVCVersion)" -ForegroundColor Gray
    Write-Host "  SDK:     $($Global:ExplorerLensConfig.WindowsSDK)" -ForegroundColor Gray
    Write-Host ""
    Write-Host "Build Tools:" -ForegroundColor Yellow
    Write-Host "  MSBuild: $(if (Get-Command msbuild -EA SilentlyContinue) { (msbuild -version | Select-String '\d+\.\d+' | Select-Object -First 1).Matches.Value } else { 'Not found' })" -ForegroundColor Gray
    Write-Host "  CMake:   $(if (Get-Command cmake -EA SilentlyContinue) { (cmake --version | Select-String '\d+\.\d+\.\d+' | Select-Object -First 1).Matches.Value } else { 'Not found' })" -ForegroundColor Gray
    Write-Host "  Git:     $(if (Get-Command git -EA SilentlyContinue) { (git --version | Select-String '\d+\.\d+\.\d+' | Select-Object -First 1).Matches.Value } else { 'Not found' })" -ForegroundColor Gray
    Write-Host ""
    Write-Host "Project:" -ForegroundColor Yellow
    Write-Host "  Root:    $($Global:ExplorerLensConfig.ProjectRoot)" -ForegroundColor Gray
    Write-Host ""
    Write-Host "Quick Commands:" -ForegroundColor Yellow
    Write-Host "  dtbuild          - Show build commands" -ForegroundColor Gray
    Write-Host "  dtbuild Release  - Build release version" -ForegroundColor Gray
    Write-Host "  dtbuild Clean    - Clean build outputs" -ForegroundColor Gray
    Write-Host "  dttest           - Run tests" -ForegroundColor Gray
    Write-Host "═══════════════════════════════════════════════════════`n" -ForegroundColor Cyan
}

# ============================================================================
# Auto-Load on Import
# ============================================================================

# Automatically setup environment when script is dot-sourced
# To disable auto-load, set $env:EXPLORERLENS_NO_AUTO_LOAD = "1"
if (-not $env:EXPLORERLENS_NO_AUTO_LOAD) {
    Setup-ExplorerLensEnv -Quiet
}


