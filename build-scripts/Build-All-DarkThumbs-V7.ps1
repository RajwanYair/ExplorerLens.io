#Requires -Version 7.0
<#
.SYNOPSIS
    DarkThumbs v7.0 - Master Build Script (High Performance)
    
.DESCRIPTION
    Unified build script using correct paths and best performance practices.
    Replaces all legacy individual build scripts with consolidated approach.
    
.PARAMETER Configuration
    Build configuration (Release or Debug). Default: Release
    
.PARAMETER Clean
    Perform clean build (removes previous outputs)
    
.PARAMETER SkipExternal
    Skip building external libraries (use existing)
    
.PARAMETER UseVcpkg
    Use vcpkg for external dependencies instead of manual builds
    
.PARAMETER Parallel
    Maximum parallel jobs. Default: 8
    
.EXAMPLE
    .\Build-All-DarkThumbs-V7.ps1
    .\Build-All-DarkThumbs-V7.ps1 -Clean
    .\Build-All-DarkThumbs-V7.ps1 -UseVcpkg
    
.NOTES
    Author: DarkThumbs Team
    Date: February 16, 2026
    Version: 7.0
    
    CORRECTED PATHS (as of 2026-02-16):
    - libwebp: external\image-libs\libwebp-1.5.0-build (NOT libwebp-1.5.0)
    - compression: external\compression-libs\ (primary location)
    - legacy: external\compression\ (deprecated, minimal use)
#>

param(
    [ValidateSet("Release", "Debug")]
    [string]$Configuration = "Release",
    
    [switch]$Clean,
    [switch]$SkipExternal,
    [switch]$UseVcpkg,
    
    [ValidateRange(1, 32)]
    [int]$Parallel = 8
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# ═══════════════════════════════════════════════════════════════════════════
# PATH CONFIGURATION - CORRECTED FEBRUARY 2026
# ═══════════════════════════════════════════════════════════════════════════

$script:ProjectRoot = Split-Path -Parent $PSScriptRoot
$script:StartTime = Get-Date

# External library paths (VERIFIED - DO NOT CHANGE WITHOUT VERIFYING DISK)
$script:ExternalPaths = @{
    # Image processing libraries
    LibWebP    = "external\image-libs\libwebp-1.5.0-build"     # CORRECTED: was libwebp-1.5.0
    LibWebPOrg = "external\image-libs\libwebp-1.5.0-original"  # Original source backup
    LibAVIF    = "external\image-libs\libavif-1.3.0"
    LibJXL     = "external\image-libs\libjxl-0.11.1"
    Dav1d      = "external\image-libs\dav1d-1.5.1"
    
    # Compression libraries (PRIMARY LOCATION)
    Zlib       = "external\compression-libs\zlib-1.3.1"
    LZ4        = "external\compression-libs\lz4-1.10.0"
    Zstd       = "external\compression-libs\zstd-1.5.7"
    LZMA       = "external\compression-libs\lzma-26.00"
    XZ         = "external\compression-libs\xz-5.6.3"
    MinizipNG  = "external\compression-libs\minizip-ng-4.0.10"
    Bzip2      = "external\compression-libs\bzip2-1.0.8"
    UnRAR      = "external\compression-libs\unrar-7.2.2"
    LibArchive = "external\compression-libs\libarchive-3.7.6"
    
    # Camera/RAW libraries
    LibRaw     = "external\camera-libs\libraw"
    LibRawInst = "external\camera-libs\libraw-install"
    
    # Document libraries
    MuPDF      = "external\pdf-libs\mupdf-1.24.11-source"
}

# Build output paths
$script:BuildPaths = @{
    CBXShell     = "x64\$Configuration\CBXShell.dll"
    CBXManager   = "x64\$Configuration\CBXManager.exe"
    BuildVcpkg   = "build-vcpkg"
    BuildDefault = "build"
    PackagingOut = "packaging\output"
}

# ═══════════════════════════════════════════════════════════════════════════
# LOGGING AND UTILITIES
# ═══════════════════════════════════════════════════════════════════════════

function Write-BuildLog {
    param(
        [Parameter(Mandatory)]
        [string]$Message,
        
        [ValidateSet("Info", "Success", "Warning", "Error", "Progress")]
        [string]$Level = "Info"
    )
    
    $timestamp = Get-Date -Format "HH:mm:ss"
    $colors = @{
        Info     = "Cyan"
        Success  = "Green"
        Warning  = "Yellow"
        Error    = "Red"
        Progress = "Magenta"
    }
    
    $symbols = @{
        Info     = "ℹ"
        Success  = "✓"
        Warning  = "⚠"
        Error    = "✗"
        Progress = "►"
    }
    
    $color = $colors[$Level]
    $symbol = $symbols[$Level]
    
    Write-Host "[$timestamp] $symbol $Message" -ForegroundColor $color
    
    # Also log to file
    $logFile = Join-Path $ProjectRoot "build-logs\build-$(Get-Date -Format 'yyyyMMdd').log"
    $logDir = Split-Path $logFile -Parent
    if (-not (Test-Path $logDir)) {
        New-Item -ItemType Directory -Path $logDir -Force | Out-Null
    }
    Add-Content -Path $logFile -Value "[$timestamp] [$Level] $Message"
}

function Test-PathExists {
    param(
        [string]$Path,
        [string]$Description
    )
    
    $fullPath = Join-Path $ProjectRoot $Path
    if (Test-Path $fullPath) {
        Write-BuildLog "$Description found at: $Path" -Level Success
        return $true
    } else {
        Write-BuildLog "$Description NOT FOUND at: $Path" -Level Warning
        return $false
    }
}

function Find-MSBuildPath {
    # Try Visual Studio 2022/18
    $vsPaths = @(
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\amd64\MSBuild.exe",
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe",
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64\MSBuild.exe",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe"
    )
    
    foreach ($path in $vsPaths) {
        if (Test-Path $path) {
            Write-BuildLog "Found MSBuild: $path" -Level Success
            return $path
        }
    }
    
    throw "MSBuild not found. Install Visual Studio 2022 or VS Build Tools."
}

function Invoke-ParallelBuild {
    param(
        [string]$Solution,
        [string]$Target = "",
        [string]$Configuration = "Release",
        [string]$Platform = "x64",
        [int]$MaxParallel = 8
    )
    
    $msbuild = Find-MSBuildPath
    $targetArg = if ($Target) { "/t:$Target" } else { "" }
    
    $arguments = @(
        $Solution,
        "/p:Configuration=$Configuration",
        "/p:Platform=$Platform",
        "/m:$MaxParallel",
        "/v:minimal",
        "/nr:false",  # Don't keep MSBuild resident
        $targetArg
    ) | Where-Object { $_ }
    
    Write-BuildLog "Building: $Solution ($Configuration|$Platform)" -Level Progress
    & $msbuild $arguments
    
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed with exit code $LASTEXITCODE"
    }
}

# ═══════════════════════════════════════════════════════════════════════════
# EXTERNAL LIBRARIES BUILD
# ═══════════════════════════════════════════════════════════════════════════

function Build-ExternalLibraries {
    Write-BuildLog "Building external libraries..." -Level Progress
    
    if ($UseVcpkg) {
        Build-WithVcpkg
    } else {
        Build-ManualLibraries
    }
}

function Build-WithVcpkg {
    Write-BuildLog "Using vcpkg for dependencies..." -Level Info
    
    # Check vcpkg installation
    $vcpkgPath = $env:VCPKG_ROOT
    if (-not $vcpkgPath) {
        $vcpkgPath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\18\BuildTools\VC\vcpkg"
    }
    
    if (-not (Test-Path "$vcpkgPath\vcpkg.exe")) {
        Write-BuildLog "vcpkg not found at $vcpkgPath" -Level Warning
        Write-BuildLog "Falling back to manual library builds..." -Level Info
        Build-ManualLibraries
        return
    }
    
    Write-BuildLog "vcpkg found at: $vcpkgPath" -Level Success
    
    # Use CMake with vcpkg preset
    Push-Location $ProjectRoot
    try {
        if ($Clean -and (Test-Path $BuildPaths.BuildVcpkg)) {
            Write-BuildLog "Cleaning vcpkg build directory..." -Level Info
            Remove-Item $BuildPaths.BuildVcpkg -Recurse -Force
        }
        
        Write-BuildLog "Configuring with CMake + vcpkg..." -Level Progress
        cmake --preset vcpkg-release
        
        Write-BuildLog "Building with CMake (Ninja)..." -Level Progress
        cmake --build --preset vcpkg-release --parallel $Parallel
        
        Write-BuildLog "vcpkg build completed successfully" -Level Success
    } catch {
        Write-BuildLog "vcpkg build failed: $_" -Level Error
        Write-BuildLog "Falling back to manual library builds..." -Level Warning
        Build-ManualLibraries
    } finally {
        Pop-Location
    }
}

function Build-ManualLibraries {
    Write-BuildLog "Building libraries manually with MSBuild..." -Level Info
    
    $libraries = @(
        @{
            Name        = "LibWebP"
            Path        = $ExternalPaths.LibWebP
            BuildScript = "build-scripts\external-libs\Build-LibWebP-NMake.ps1"
            OutputLib   = "output\x64\Release\release-static\x64\lib\libwebp.lib"
        },
        @{
            Name        = "Zlib"
            Path        = $ExternalPaths.Zlib
            BuildScript = "build-scripts\external-libs\Build-Zlib.ps1"
            OutputLib   = "x64\Release\zlibstatic.lib"
        },
        @{
            Name        = "LZ4"
            Path        = $ExternalPaths.LZ4
            BuildScript = "build-scripts\external-libs\Build-LZ4.ps1"
            OutputLib   = "build\VS2022\bin\x64_Release\liblz4_static.lib"
        },
        @{
            Name        = "Zstd"
            Path        = $ExternalPaths.Zstd
            BuildScript = "build-scripts\external-libs\Build-Zstd.ps1"
            OutputLib   = "build\VS2022\bin\x64\Release\zstd_static.lib"
        },
        @{
            Name        = "Minizip-NG"
            Path        = $ExternalPaths.MinizipNG
            BuildScript = "build-scripts\external-libs\Build-MinizipNG.ps1"
            OutputLib   = "build-manual\Release\minizip.lib"
        }
    )
    
    foreach ($lib in $libraries) {
        $libPath = Join-Path $ProjectRoot $lib.Path
        $outputPath = Join-Path $libPath $lib.OutputLib
        
        if ((Test-Path $outputPath) -and -not $Clean) {
            Write-BuildLog "$($lib.Name) already built ($(Join-Path $lib.Path $lib.OutputLib))" -Level Success
            continue
        }
        
        Write-BuildLog "Building $($lib.Name)..." -Level Progress
        
        $scriptPath = Join-Path $ProjectRoot $lib.BuildScript
        if (Test-Path $scriptPath) {
            try {
                & $scriptPath -Clean:$Clean
                if ($LASTEXITCODE -eq 0) {
                    Write-BuildLog "$($lib.Name) built successfully" -Level Success
                } else {
                    Write-BuildLog "$($lib.Name) build failed (non-critical)" -Level Warning
                }
            } catch {
                Write-BuildLog "$($lib.Name) build error: $_" -Level Warning
            }
        } else {
            Write-BuildLog "Build script not found: $scriptPath" -Level Warning
        }
    }
}

# ═══════════════════════════════════════════════════════════════════════════
# MAIN PROJECT BUILD
# ═══════════════════════════════════════════════════════════════════════════

function Build-MainProject {
    Write-BuildLog "Building DarkThumbs main project..." -Level Progress
    
    Push-Location $ProjectRoot
    try {
        if ($Clean) {
            Write-BuildLog "Performing clean build..." -Level Info
            Invoke-ParallelBuild -Solution "CBXShell.sln" `
                -Target "Clean" `
                -Configuration $Configuration `
                -Platform "x64" `
                -MaxParallel $Parallel
        }
        
        Write-BuildLog "Building CBXShell solution..." -Level Progress
        Invoke-ParallelBuild -Solution "CBXShell.sln" `
            -Configuration $Configuration `
            -Platform "x64" `
            -MaxParallel $Parallel
        
        # Verify outputs
        $dllPath = Join-Path $ProjectRoot $BuildPaths.CBXShell
        $exePath = Join-Path $ProjectRoot $BuildPaths.CBXManager
        
        if (Test-Path $dllPath) {
            $size = (Get-Item $dllPath).Length / 1MB
            Write-BuildLog "CBXShell.dll: $([math]::Round($size, 2)) MB" -Level Success
        } else {
            throw "CBXShell.dll not found at $dllPath"
        }
        
        if (Test-Path $exePath) {
            $size = (Get-Item $exePath).Length / 1MB
            Write-BuildLog "CBXManager.exe: $([math]::Round($size, 2)) MB" -Level Success
        } else {
            throw "CBXManager.exe not found at $exePath"
        }
        
    } finally {
        Pop-Location
    }
}

# ═══════════════════════════════════════════════════════════════════════════
# MAIN EXECUTION
# ═══════════════════════════════════════════════════════════════════════════

function Main {
    Write-Host "`n" -NoNewline
    Write-Host "╔══════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
    Write-Host "║  DarkThumbs v7.0 - Master Build Script (High Performance)   ║" -ForegroundColor Cyan
    Write-Host "╚══════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
    Write-Host ""
    
    Write-BuildLog "Build started" -Level Info
    Write-BuildLog "Configuration: $Configuration" -Level Info
    Write-BuildLog "Parallel jobs: $Parallel" -Level Info
    Write-BuildLog "Clean build: $Clean" -Level Info
    Write-BuildLog "Use vcpkg: $UseVcpkg" -Level Info
    Write-BuildLog "Skip external: $SkipExternal" -Level Info
    
    try {
        # Verify project structure
        Write-BuildLog "Verifying project structure..." -Level Progress
        Test-PathExists -Path "CBXShell.sln" -Description "Solution file"
        Test-PathExists -Path $ExternalPaths.LibWebP -Description "LibWebP (CORRECTED PATH)"
        Test-PathExists -Path $ExternalPaths.Zlib -Description "Zlib"
        
        # Build external libraries
        if (-not $SkipExternal) {
            Build-ExternalLibraries
        } else {
            Write-BuildLog "Skipping external library builds (--SkipExternal)" -Level Info
        }
        
        # Build main project
        Build-MainProject
        
        # Calculate build time
        $elapsed = (Get-Date) - $StartTime
        Write-Host ""
        Write-Host "╔══════════════════════════════════════════════════════════════╗" -ForegroundColor Green
        Write-Host "║              BUILD COMPLETED SUCCESSFULLY                    ║" -ForegroundColor Green
        Write-Host "╚══════════════════════════════════════════════════════════════╝" -ForegroundColor Green
        Write-BuildLog "Build time: $([math]::Round($elapsed.TotalMinutes, 1)) minutes" -Level Success
        Write-Host ""
        
        # Display outputs
        Write-Host "📦 Build Outputs:" -ForegroundColor Cyan
        Write-Host "  → CBXShell.dll:    $($BuildPaths.CBXShell)" -ForegroundColor White
        Write-Host "  → CBXManager.exe:  $($BuildPaths.CBXManager)" -ForegroundColor White
        Write-Host ""
        
        return 0
        
    } catch {
        $elapsed = (Get-Date) - $StartTime
        Write-Host ""
        Write-Host "╔══════════════════════════════════════════════════════════════╗" -ForegroundColor Red
        Write-Host "║                  BUILD FAILED                                ║" -ForegroundColor Red
        Write-Host "╚══════════════════════════════════════════════════════════════╝" -ForegroundColor Red
        Write-BuildLog "Build failed after $([math]::Round($elapsed.TotalMinutes, 1)) minutes" -Level Error
        Write-BuildLog "Error: $_" -Level Error
        Write-Host ""
        
        return 1
    }
}

# Run main function
exit (Main)
