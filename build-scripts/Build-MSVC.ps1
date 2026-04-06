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
    CMake configure preset name (default: "temp-release" — uses TEMP dir to avoid OneDrive sync issues).
    Use "default-release" to build into the source tree's build/ directory.

.PARAMETER Jobs
    Parallel build jobs (default: 8).

.PARAMETER Configure
    Only configure (skip build step).

.PARAMETER Test
    Run CTest after a successful build.

    .\.build-scripts\Build-MSVC.ps1
    .\.build-scripts\Build-MSVC.ps1 -Clean
    .\.build-scripts\Build-MSVC.ps1 -Preset temp-release -Test
    .\.build-scripts\Build-MSVC.ps1 -Preset default-release
    .\.build-scripts\Build-MSVC.ps1 -Preset vcpkg-release
#>
param(
    [switch]$Clean,
    [string]$Preset = "temp-release",
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

# Start build timer
$buildTimer = [System.Diagnostics.Stopwatch]::StartNew()
$phaseTimings = @{}

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
$phaseTimings["vcvars"] = $buildTimer.Elapsed.TotalSeconds

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
        "default-release" = "$env:TEMP\ExplorerLens-build"
        "default-debug"   = "$env:TEMP\ExplorerLens-build-debug"
        "vcpkg-release"   = "$env:TEMP\ExplorerLens-build-vcpkg"
        "vcpkg-debug"     = "$env:TEMP\ExplorerLens-build-vcpkg-debug"
        "vs2026"          = "$env:TEMP\ExplorerLens-build-vs"
        "temp-release"    = "$env:TEMP\ExplorerLens-build"
        "temp-debug"      = "$env:TEMP\ExplorerLens-build-debug"
    }
    $binDir = $presetBinaryDirs[$Preset]
    if (-not $binDir) { $binDir = "$env:TEMP\ExplorerLens-build" }

    # Absolute path: TEMP presets are already absolute; relative ones are project-relative
    if ([System.IO.Path]::IsPathRooted($binDir)) {
        $fullBinDir = $binDir
    } else {
        $fullBinDir = Join-Path $PROJECT_ROOT $binDir
    }

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
    $phaseTimings["configure"] = $buildTimer.Elapsed.TotalSeconds - ($phaseTimings.Values | Measure-Object -Sum).Sum

    if (-not $Configure) {
        # 6. Build
        Write-Host "`n[4/4] Building ($Jobs parallel jobs)..." -ForegroundColor Yellow

        # Map configure preset to build preset
        $buildPreset = $Preset  # They share names in our presets
        # Capture all build output to a dedicated log for post-build analysis
        $logDir = Join-Path $env:TEMP "ExplorerLens-logs"
        if (-not (Test-Path $logDir)) { New-Item -ItemType Directory -Path $logDir -Force | Out-Null }
        $buildLogPath = Join-Path $logDir "build-latest.log"

        $buildArgs = @("--build", "--preset", $buildPreset, "-j", $Jobs, "--",
                       "-k", "0")   # -k 0: Ninja continues past errors (no limit on failed jobs)
        if (-not [string]::IsNullOrWhiteSpace($Target)) {
            Write-Host "  Target: $Target" -ForegroundColor Cyan
            $buildArgs = @("--build", "--preset", $buildPreset, "--target", $Target,
                           "-j", $Jobs, "--", "-k", "0")
        }

        # Stream output live to terminal AND capture in $buildOutput for analysis.
        # Using -Variable avoids opening a file during streaming (prevents lock conflicts
        # when callers also redirect output). Log is written to disk after build completes.
        $buildOutput = & $cmakeExe @buildArgs 2>&1 | Tee-Object -Variable cmakeLiveCapture
        $buildExitCode = $LASTEXITCODE

        # Persist log to disk now that build is complete
        $buildOutput | Out-File -FilePath $buildLogPath -Encoding UTF8

        # --- Post-Build Error/Warning Summary ---
        # NOTE: Avoid $errors/$warnings as names — they shadow PowerShell's built-in $Error.
        #       Wrap in @() to force array type even when 0 or 1 result is returned.
        $buildErrors   = @($buildOutput | Where-Object { $_ -match '\berror\s*C\d{4}\b|\s+error:' })
        $buildWarnings = @($buildOutput | Where-Object { $_ -match '\bwarning\s*C\d{4}\b|\s+warning:' })
        $buildNotes    = @($buildOutput | Where-Object { $_ -match '\bnote:' })

        Write-Host "`n========================================" -ForegroundColor Cyan
        Write-Host " Build Analysis" -ForegroundColor Cyan
        Write-Host "========================================" -ForegroundColor Cyan
        Write-Host "  Errors   : $($buildErrors.Count)"   -ForegroundColor $(if ($buildErrors.Count   -gt 0) {'Red'}    else {'Green'})
        Write-Host "  Warnings : $($buildWarnings.Count)" -ForegroundColor $(if ($buildWarnings.Count -gt 0) {'Yellow'} else {'Green'})
        Write-Host "  Notes    : $($buildNotes.Count)"    -ForegroundColor $(if ($buildNotes.Count    -gt 0) {'Cyan'}   else {'Gray'})
        Write-Host "  Log      : $buildLogPath"      -ForegroundColor DarkGray

        if ($buildErrors.Count -gt 0) {
            Write-Host "`n--- Errors ---" -ForegroundColor Red
            $buildErrors | Select-Object -First 50 | ForEach-Object { Write-Host "  $_" -ForegroundColor Red }
            if ($buildErrors.Count -gt 50) { Write-Host "  ... and $($buildErrors.Count - 50) more (see log)" -ForegroundColor DarkRed }
        }
        if ($buildWarnings.Count -gt 0) {
            Write-Host "`n--- Warnings ---" -ForegroundColor Yellow
            $buildWarnings | Select-Object -First 30 | ForEach-Object { Write-Host "  $_" -ForegroundColor Yellow }
            if ($buildWarnings.Count -gt 30) { Write-Host "  ... and $($buildWarnings.Count - 30) more (see log)" -ForegroundColor DarkYellow }
        }

        if ($buildExitCode -ne 0) {
            Write-Error "Build failed (exit code $buildExitCode) — $($buildErrors.Count) errors, $($buildWarnings.Count) warnings. Full log: $buildLogPath"
        } else {
            Write-Host "`n  Build: OK" -ForegroundColor Green
        }
        $phaseTimings["build"] = $buildTimer.Elapsed.TotalSeconds - ($phaseTimings.Values | Measure-Object -Sum).Sum

        # 7. Test (if requested)
        if ($Test) {
            Write-Host "`n[Test] Running CTest..." -ForegroundColor Yellow
            $testPreset = "$Preset-test"
            & $cmakeExe --test --preset $testPreset 2>$null
            if ($LASTEXITCODE -eq 0) {
                Write-Host "  Tests: ALL PASSED" -ForegroundColor Green
            } else {
                # Fallback: resolve binary dir and run ctest directly
                $presetBinaryDirs = @{
                    "default-release" = "$env:TEMP\ExplorerLens-build"
                    "default-debug"   = "$env:TEMP\ExplorerLens-build-debug"
                    "vcpkg-release"   = "$env:TEMP\ExplorerLens-build-vcpkg"
                    "vcpkg-debug"     = "$env:TEMP\ExplorerLens-build-vcpkg-debug"
                    "vs2026"          = "$env:TEMP\ExplorerLens-build-vs"
                    "temp-release"    = "$env:TEMP\ExplorerLens-build"
                    "temp-debug"      = "$env:TEMP\ExplorerLens-build-debug"
                }
                $binDir = $presetBinaryDirs[$Preset]
                if (-not $binDir) { $binDir = "$env:TEMP\ExplorerLens-build" }
                $fullBinDir = if ([System.IO.Path]::IsPathRooted($binDir)) { $binDir } else { Join-Path $PROJECT_ROOT $binDir }
                & ctest --test-dir $fullBinDir -C Release --output-on-failure
            }
            $phaseTimings["test"] = $buildTimer.Elapsed.TotalSeconds - ($phaseTimings.Values | Measure-Object -Sum).Sum
        }
    }
} finally {
    Pop-Location
}

# === Build Timing Summary ===
$buildTimer.Stop()
$totalSeconds = $buildTimer.Elapsed.TotalSeconds

Write-Host "`n========================================" -ForegroundColor Green
Write-Host " Build Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host "`n  Timing Breakdown:" -ForegroundColor Cyan
foreach ($phase in @("vcvars", "configure", "build", "test")) {
    if ($phaseTimings.ContainsKey($phase)) {
        $secs = [math]::Round($phaseTimings[$phase], 2)
        $phaseName = $phase.PadRight(12)
        Write-Host "    $phaseName $secs s" -ForegroundColor White
    }
}
$totalFormatted = [math]::Round($totalSeconds, 2)
Write-Host "    --------------------" -ForegroundColor DarkGray
Write-Host "    Total         $totalFormatted s" -ForegroundColor Yellow
Write-Host ""

# Append to build history log in TEMP (JSONL — keeps repo clean)
$historyPath = Join-Path $env:TEMP "ExplorerLens-logs\build-history.jsonl"
$historyDir = Split-Path $historyPath -Parent
if (-not (Test-Path $historyDir)) { New-Item -ItemType Directory -Path $historyDir -Force | Out-Null }
$entry = [ordered]@{
    timestamp = (Get-Date -Format "o")
    preset    = $Preset
    clean     = [bool]$Clean
    target    = $Target
    phases    = $phaseTimings
    totalSec  = [math]::Round($totalSeconds, 2)
}
$entry | ConvertTo-Json -Compress | Add-Content -Path $historyPath -Encoding UTF8
