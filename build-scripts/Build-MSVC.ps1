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

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path -LiteralPath $vswhere)) {
    throw 'Visual Studio Installer discovery tool (vswhere.exe) is missing.'
}
$VS_ROOT = & $vswhere -latest -products '*' -version '[18.0,19.0)' `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $VS_ROOT) {
    throw 'Install Visual Studio 2026 Build Tools with the C++ workload and MSVC v145.'
}
$VCVARS64 = "$VS_ROOT\VC\Auxiliary\Build\vcvars64.bat"
$toolset = Get-ChildItem -LiteralPath (Join-Path $VS_ROOT 'VC\Tools\MSVC') -Directory |
    Where-Object Name -Match '^14\.5\d\.' |
    Sort-Object { [version]$_.Name } -Descending | Select-Object -First 1
if (-not $toolset) {
    throw 'No MSVC v145 toolset is installed in Visual Studio 2026.'
}
$MSVC_TOOLSET_VER = $toolset.Name

# Bundled tools (fallback)
$BUNDLED_CMAKE = "$VS_ROOT\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$BUNDLED_NINJA = "$VS_ROOT\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"
$MACHINE_PATH_ENTRIES = @(
    [Environment]::GetEnvironmentVariable('Path', 'Machine') -split ';' |
        Where-Object { -not [string]::IsNullOrWhiteSpace($_) } |
        ForEach-Object { $_.TrimEnd('\') }
)
$MACHINE_CMAKE_CANDIDATES = @(
    'C:\Program Files\CMake\bin\cmake.exe',
    'C:\ProgramData\scoop\shims\cmake.exe'
)
$MACHINE_NINJA_CANDIDATES = @(
    'C:\Program Files\Ninja\ninja.exe',
    'C:\ProgramData\scoop\shims\ninja.exe'
)

# Project root (parent of build-scripts/)
$PROJECT_ROOT = Split-Path -Parent $PSScriptRoot

#==============================================================================
# Tool resolution — prefer the latest machine-installed tool over the bundled VS copy
#==============================================================================

function Resolve-Tool {
    param(
        [string]$Name,
        [string[]]$MachineCandidates,
        [string]$BundledPath,
        [version]$MinimumVersion
    )

    $candidates = @(
        $MACHINE_PATH_ENTRIES | ForEach-Object { Join-Path $_ "$Name.exe" }
        $MachineCandidates
        $BundledPath
    ) | Where-Object { $_ } | Select-Object -Unique

    foreach ($candidate in $candidates) {
        if (-not (Test-Path -LiteralPath $candidate -PathType Leaf)) { continue }

        $versionOutput = & $candidate --version 2>&1 | Select-Object -First 1
        $versionMatch = [regex]::Match([string]$versionOutput, '(\d+\.\d+(?:\.\d+)?)')
        if ($MinimumVersion -and (-not $versionMatch.Success -or [version]$versionMatch.Value -lt $MinimumVersion)) {
            Write-Host "  $Name : $candidate ($versionOutput) [requires $MinimumVersion+]" -ForegroundColor Red
            continue
        }

        $scope = if ($candidate -eq $BundledPath) { 'bundled' } else { 'machine' }
        Write-Host "  $Name : $candidate ($versionOutput) [$scope]" -ForegroundColor Cyan
        return $candidate
    }

    throw "$Name not found in machine PATH, machine-wide tool roots, or the Visual Studio installation."
}

function Resolve-MachineVcpkgRoot {
    $machineVcpkgRoot = [Environment]::GetEnvironmentVariable('VCPKG_ROOT', 'Machine')
    $candidates = @(
        $machineVcpkgRoot,
        'C:\ProgramData\vcpkg',
        'C:\Program Files\vcpkg',
        'C:\vcpkg',
        'C:\tools\vcpkg',
        (Join-Path $VS_ROOT 'VC\vcpkg')
    ) | Where-Object { -not [string]::IsNullOrWhiteSpace($_) } | Select-Object -Unique

    foreach ($candidate in $candidates) {
        $toolchainFile = Join-Path $candidate 'scripts\buildsystems\vcpkg.cmake'
        if (Test-Path -LiteralPath $toolchainFile -PathType Leaf) {
            return $candidate
        }
    }

    throw 'A complete machine-wide vcpkg installation was not found. Configure machine VCPKG_ROOT or install vcpkg under C:\ProgramData\vcpkg.'
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
if ($LASTEXITCODE -ne 0) {
    throw "Failed to initialize MSVC v145 from $VCVARS64."
}
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
$cmakeExe = Resolve-Tool "cmake" $MACHINE_CMAKE_CANDIDATES $BUNDLED_CMAKE ([version]'4.2.0')
$ninjaExe = Resolve-Tool "ninja" $MACHINE_NINJA_CANDIDATES $BUNDLED_NINJA

if ($Preset -like 'vcpkg-*') {
    # Ignore inherited process/user VCPKG_ROOT values. vcpkg presets must use a
    # complete machine-wide installation so the build is reproducible for all users.
    $env:VCPKG_ROOT = Resolve-MachineVcpkgRoot
    Write-Host "  vcpkg   : $env:VCPKG_ROOT [machine]" -ForegroundColor Cyan
}

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
    $configureArgs = @('--preset', $Preset)
    if ($Preset -ne 'vs2026') {
        $configureArgs += "-DCMAKE_MAKE_PROGRAM=$ninjaExe"
    }
    & $cmakeExe @configureArgs
    if ($LASTEXITCODE -ne 0) {
        Write-Error "CMake configure failed (exit code $LASTEXITCODE)"
    }
    Write-Host "  Configure: OK" -ForegroundColor Green
    $phaseTimings["configure"] = $buildTimer.Elapsed.TotalSeconds - ($phaseTimings.Values | Measure-Object -Sum).Sum

    if (-not $Configure) {
        # 6. Build
        Write-Host "`n[4/4] Building ($Jobs parallel jobs)..." -ForegroundColor Yellow

        # Map configure preset to build preset
        $buildPreset = if ($Preset -eq 'vs2026') { 'vs2026-release' } else { $Preset }
        # Capture all build output to a dedicated log for post-build analysis
        $logDir = Join-Path $env:TEMP "ExplorerLens-logs"
        if (-not (Test-Path $logDir)) { New-Item -ItemType Directory -Path $logDir -Force | Out-Null }
        $buildLogPath = Join-Path $logDir "build-latest.log"

        $buildArgs = @('--build', '--preset', $buildPreset, '-j', $Jobs)
        if (-not [string]::IsNullOrWhiteSpace($Target)) {
            Write-Host "  Target: $Target" -ForegroundColor Cyan
            $buildArgs += @('--target', $Target)
        }
        if ($Preset -ne 'vs2026') {
            $buildArgs += @('--', '-k', '0')
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
            $ctestExe = Join-Path (Split-Path $cmakeExe) 'ctest.exe'
            $presetData = Get-Content -LiteralPath (Join-Path $PROJECT_ROOT 'CMakePresets.json') -Raw | ConvertFrom-Json
            if ($testPreset -in $presetData.testPresets.name) {
                & $ctestExe --preset $testPreset
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
                $configuration = if ($Preset -like '*debug') { 'Debug' } else { 'Release' }
                & $ctestExe --test-dir $fullBinDir -C $configuration --output-on-failure --no-tests=error
            }
            if ($LASTEXITCODE -ne 0) {
                throw "CTest failed (exit code $LASTEXITCODE)."
            }
            Write-Host "  Tests: ALL PASSED" -ForegroundColor Green
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
