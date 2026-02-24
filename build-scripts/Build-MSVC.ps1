<#
.SYNOPSIS
    Build ExplorerLens using MSVC v145 toolset from VS 18 2026 BuildTools.

.DESCRIPTION
    Sources vcvars64.bat to put cl.exe/link.exe/lib.exe on PATH, then runs
    CMake configure + build using the default-release preset (Ninja + MSVC).

    This is the recommended build entry-point for ExplorerLens.

.PARAMETER Clean
    Delete build/ directory before configuring.

.PARAMETER Preset
    CMake configure preset name (default: "default-release").

.PARAMETER Jobs
    Parallel build jobs (default: 8).

.PARAMETER Configure
    Only configure (skip build step).

.PARAMETER Test
    Run CTest after a successful build.

.EXAMPLE
    .\build-scripts\Build-MSVC.ps1
    .\build-scripts\Build-MSVC.ps1 -Clean
    .\build-scripts\Build-MSVC.ps1 -Preset vcpkg-release -Test
#>
param(
    [switch]$Clean,
    [string]$Preset = "default-release",
    [int]$Jobs = 8,
    [string]$Target = "",
    [switch]$Configure,
    [switch]$Test
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

#==============================================================================
# Configuration — VS 18 2026 BuildTools paths
#==============================================================================

$VS_ROOT = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools"
$VCVARS64 = "$VS_ROOT\VC\Auxiliary\Build\vcvars64.bat"
$MSVC_TOOLSET_VER = "14.50.35717"   # Latest v145 sub-version

# Scoop tools (preferred — newer versions)
$SCOOP_CMAKE = "$env:USERPROFILE\scoop\shims\cmake.exe"
$SCOOP_NINJA = "$env:USERPROFILE\scoop\shims\ninja.exe"

# Bundled tools (fallback)
$BUNDLED_CMAKE = "$VS_ROOT\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$BUNDLED_NINJA = "$VS_ROOT\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"

# Project root (parent of build-scripts/)
$PROJECT_ROOT = Split-Path -Parent $PSScriptRoot

#==============================================================================
# Tool resolution — prefer latest (Scoop) over bundled
#==============================================================================

function Resolve-Tool {
    param([string]$Name, [string]$ScoopPath, [string]$BundledPath)
    if (Test-Path $ScoopPath) {
        $ver = & $ScoopPath --version 2>&1 | Select-Object -First 1
        Write-Host "  $Name : $ScoopPath ($ver)" -ForegroundColor Cyan
        return $ScoopPath
    }
    if (Test-Path $BundledPath) {
        $ver = & $BundledPath --version 2>&1 | Select-Object -First 1
        Write-Host "  $Name : $BundledPath ($ver) [bundled]" -ForegroundColor Yellow
        return $BundledPath
    }
    # Try PATH as last resort
    $found = Get-Command $Name -ErrorAction SilentlyContinue
    if ($found) {
        Write-Host "  $Name : $($found.Source) [PATH]" -ForegroundColor DarkYellow
        return $found.Source
    }
    Write-Error "$Name not found! Install via Scoop or VS BuildTools."
}

#==============================================================================
# Main
#==============================================================================

Write-Host "`n========================================" -ForegroundColor Green
Write-Host " ExplorerLens — MSVC v145 Build" -ForegroundColor Green
Write-Host "========================================`n" -ForegroundColor Green

# 1. Verify vcvars
if (-not (Test-Path $VCVARS64)) {
    Write-Error "vcvars64.bat not found at: $VCVARS64`nInstall VS 18 2026 BuildTools with C++ workload."
}

# 2. Source vcvars64 (import environment into PowerShell)
Write-Host "[1/4] Sourcing vcvars64.bat (MSVC v145 toolset $MSVC_TOOLSET_VER)..." -ForegroundColor Yellow
$envBefore = @{}
Get-ChildItem env: | ForEach-Object { $envBefore[$_.Name] = $_.Value }

# Run vcvars and capture environment
$vcvarsOutput = cmd /c "`"$VCVARS64`" -vcvars_ver=$MSVC_TOOLSET_VER >nul 2>&1 && set" 2>&1
foreach ($line in $vcvarsOutput) {
    if ($line -match '^([^=]+)=(.*)$') {
        $varName = $Matches[1]
        $varValue = $Matches[2]
        [System.Environment]::SetEnvironmentVariable($varName, $varValue, 'Process')
    }
}

# Verify cl.exe is now on PATH
$cl = Get-Command cl.exe -ErrorAction SilentlyContinue
if (-not $cl) {
    Write-Error "cl.exe not found on PATH after sourcing vcvars64.bat!"
}
$clVersion = (& cl.exe 2>&1 | Select-String "Version") -replace '.*Version\s+', ''
Write-Host "  cl.exe  : $($cl.Source)" -ForegroundColor Cyan
Write-Host "  version : $clVersion" -ForegroundColor Cyan

# 3. Resolve cmake and ninja
Write-Host "`n[2/4] Resolving build tools..." -ForegroundColor Yellow
$cmakeExe = Resolve-Tool "cmake" $SCOOP_CMAKE $BUNDLED_CMAKE
$ninjaExe = Resolve-Tool "ninja" $SCOOP_NINJA $BUNDLED_NINJA

# Ensure Ninja is on PATH for CMake to find
$ninjaDir = Split-Path $ninjaExe
if ($env:PATH -notlike "*$ninjaDir*") {
    $env:PATH = "$ninjaDir;$env:PATH"
}

# 4. Clean if requested
if ($Clean) {
    # Determine binary dir from preset
    $presetBinaryDirs = @{
        "default-release" = "build"
        "default-debug"   = "build-debug"
        "vcpkg-release"   = "build-vcpkg"
        "vcpkg-debug"     = "build-vcpkg-debug"
    }
    $binDir = $presetBinaryDirs[$Preset]
    if (-not $binDir) { $binDir = "build" }
    $fullBinDir = Join-Path $PROJECT_ROOT $binDir

    if (Test-Path $fullBinDir) {
        Write-Host "`n[Clean] Removing $fullBinDir..." -ForegroundColor Magenta
        Remove-Item -Path $fullBinDir -Recurse -Force
    }
}

# 5. Configure
Write-Host "`n[3/4] Configuring with preset '$Preset'..." -ForegroundColor Yellow
Push-Location $PROJECT_ROOT
try {
    & $cmakeExe --preset $Preset
    if ($LASTEXITCODE -ne 0) {
        Write-Error "CMake configure failed (exit code $LASTEXITCODE)"
    }
    Write-Host "  Configure: OK" -ForegroundColor Green

    if (-not $Configure) {
        # 6. Build
        Write-Host "`n[4/4] Building ($Jobs parallel jobs)..." -ForegroundColor Yellow

        # Map configure preset to build preset
        $buildPreset = $Preset  # They share names in our presets
        if ([string]::IsNullOrWhiteSpace($Target)) {
            & $cmakeExe --build --preset $buildPreset -j $Jobs
        } else {
            Write-Host "  Target: $Target" -ForegroundColor Cyan
            & $cmakeExe --build --preset $buildPreset --target $Target -j $Jobs
        }
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Build failed (exit code $LASTEXITCODE)"
        }
        Write-Host "`n  Build: OK" -ForegroundColor Green

        # 7. Test (if requested)
        if ($Test) {
            Write-Host "`n[Test] Running CTest..." -ForegroundColor Yellow
            $testPreset = "$Preset-test"
            & $cmakeExe --test --preset $testPreset 2>$null
            if ($LASTEXITCODE -eq 0) {
                Write-Host "  Tests: ALL PASSED" -ForegroundColor Green
            } else {
                # Fallback to ctest directly if test preset doesn't exist
                $binDir = $presetBinaryDirs[$Preset]
                if (-not $binDir) { $binDir = "build" }
                & ctest --test-dir (Join-Path $PROJECT_ROOT $binDir) -C Release --output-on-failure
            }
        }
    }
} finally {
    Pop-Location
}

Write-Host "`n========================================" -ForegroundColor Green
Write-Host " Build Complete!" -ForegroundColor Green
Write-Host "========================================`n" -ForegroundColor Green
