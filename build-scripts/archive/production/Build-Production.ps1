#Requires -Version 7.0

<#
.SYNOPSIS
    Production build script for DarkThumbs project
.DESCRIPTION
    Complete clean build: all libraries, CBXShell, with logging and verification
.PARAMETER Clean
    Remove all build artifacts before building
.PARAMETER SkipLibraries
    Skip building compression/image libraries (use existing)
.PARAMETER Verbose
    Show detailed build output
#>

param(
    [switch]$Clean,
    [switch]$SkipLibraries,
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"
$startTime = Get-Date

# Ensure build-logs directory exists
if (-not (Test-Path "build-logs")) {
    New-Item -Path "build-logs" -ItemType Directory -Force | Out-Null
}

$logFile = "build-logs\production-build-$(Get-Date -Format 'yyyy-MM-dd_HHmmss').log"

function Write-Log {
    param([string]$Message, [string]$Color = "White")
    $timestamp = Get-Date -Format "HH:mm:ss"
    $logMessage = "[$timestamp] $Message"
    Write-Host $logMessage -ForegroundColor $Color
    Add-Content -Path $logFile -Value $logMessage
}

function Write-Section {
    param([string]$Title)
    Write-Log "`n========================================" "Cyan"
    Write-Log "  $Title" "Cyan"
    Write-Log "========================================" "Cyan"
}

Write-Section "DarkThumbs Production Build"
Write-Log "Log file: $logFile" "Gray"
Write-Log "Build started: $startTime" "Gray"

# Clean build if requested
if ($Clean) {
    Write-Section "Cleaning Build Artifacts"
    
    $cleanDirs = @("build", "x64", "packages", "CBXShell\x64", "CBXManager\x64")
    foreach ($dir in $cleanDirs) {
        if (Test-Path $dir) {
            Write-Log "  Removing: $dir" "Yellow"
            Remove-Item -Path $dir -Recurse -Force -ErrorAction SilentlyContinue
        }
    }
    
    # Clean library build artifacts
    $libDirs = Get-ChildItem "external" -Directory -ErrorAction SilentlyContinue
    foreach ($libDir in $libDirs) {
        $buildPath = Join-Path $libDir.FullName "build"
        if (Test-Path $buildPath) {
            Write-Log "  Cleaning: $($libDir.Name)\build" "Yellow"
            Remove-Item -Path $buildPath -Recurse -Force -ErrorAction SilentlyContinue
        }
    }
    
    Write-Log "✅ Clean complete" "Green"
}

# Build libraries
if (-not $SkipLibraries) {
    Write-Section "Building Compression & Image Libraries"
    
    $buildScripts = @(
        "build-scripts\Build-Zlib.ps1",
        "build-scripts\Build-LZ4.ps1",
        "build-scripts\Build-Zstd.ps1",
        "build-scripts\Build-LibWebP-NMake.ps1",
        "build-scripts\build-lzma-24.08.ps1",
        "build-scripts\Build-MinizipNG.ps1"
    )
    
    $libraryResults = @()
    
    foreach ($script in $buildScripts) {
        if (Test-Path $script) {
            $scriptName = Split-Path $script -Leaf
            Write-Log "`nBuilding: $scriptName" "Yellow"
            
            try {
                $output = & $script 2>&1
                if ($Verbose) {
                    $output | ForEach-Object { Write-Log "  $_" "Gray" }
                }
                
                if ($LASTEXITCODE -eq 0 -or $?) {
                    Write-Log "  ✅ $scriptName succeeded" "Green"
                    $libraryResults += [PSCustomObject]@{ Script = $scriptName; Status = "✅ Success" }
                } else {
                    Write-Log "  ❌ $scriptName failed" "Red"
                    $libraryResults += [PSCustomObject]@{ Script = $scriptName; Status = "❌ Failed" }
                }
            } catch {
                Write-Log "  ❌ Error: $_" "Red"
                $libraryResults += [PSCustomObject]@{ Script = $scriptName; Status = "❌ Error" }
            }
        } else {
            Write-Log "  ⚠️  Script not found: $script" "Yellow"
        }
    }
    
    Write-Log "`nLibrary Build Summary:" "Cyan"
    $libraryResults | ForEach-Object { Write-Log "  $($_.Script): $($_.Status)" }
} else {
    Write-Log "Skipping library builds (using existing)" "Yellow"
}

# Verify required libraries
Write-Section "Verifying Required Libraries"

$requiredLibs = @(
    "external\compression\zlib-1.3.1\x64\Release\zlibstatic.lib",
    "external\compression\lz4-1.10.0\build\VS2022\liblz4\bin\x64_Release\liblz4_static.lib",
    "external\compression\zstd-1.5.7\build\x64\Release\zstd_static.lib",
    "external\image-libs\libwebp-1.5.0\build-vs\Release\webp.lib",
    "external\compression\lzma-24.08\build-vs\Release\lzma.lib"
)

$missingLibs = @()
foreach ($lib in $requiredLibs) {
    if (Test-Path $lib) {
        $size = (Get-Item $lib).Length / 1KB
        Write-Log "  ✅ $lib ($([Math]::Round($size, 1)) KB)" "Green"
    } else {
        Write-Log "  ❌ MISSING: $lib" "Red"
        $missingLibs += $lib
    }
}

if ($missingLibs.Count -gt 0) {
    Write-Log "`n❌ Missing $($missingLibs.Count) required libraries!" "Red"
    Write-Log "Cannot proceed with CBXShell build." "Red"
    exit 1
}

# Build CBXShell
Write-Section "Building CBXShell"

# Find MSBuild
$msbuildPath = & "build-scripts\Find-MSBuild.ps1"
if (-not $msbuildPath) {
    Write-Log "❌ MSBuild not found!" "Red"
    exit 1
}
Write-Log "Using MSBuild: $msbuildPath" "Gray"

# Build configurations
$configurations = @("Debug", "Release")
$platforms = @("x64")

foreach ($config in $configurations) {
    foreach ($platform in $platforms) {
        Write-Log "`nBuilding CBXShell: $platform | $config" "Yellow"
        
        $msbuildArgs = @(
            "CBXShell.sln",
            "/p:Configuration=$config",
            "/p:Platform=$platform",
            "/m",
            "/v:minimal"
        )
        
        if (-not $Verbose) {
            $msbuildArgs += "/nologo"
        }
        
        try {
            $buildOutput = & $msbuildPath $msbuildArgs 2>&1
            
            if ($Verbose) {
                $buildOutput | ForEach-Object { Write-Log "  $_" "Gray" }
            }
            
            if ($LASTEXITCODE -eq 0) {
                Write-Log "  ✅ Build succeeded: $platform | $config" "Green"
            } else {
                Write-Log "  ❌ Build failed: $platform | $config" "Red"
                # Save error output
                $buildOutput | Add-Content -Path $logFile
            }
        } catch {
            Write-Log "  ❌ Build error: $_" "Red"
        }
    }
}

# Verify build outputs
Write-Section "Verifying Build Outputs"

$outputs = @(
    @{ Path = "x64\Release\CBXShell.dll"; Name = "CBXShell.dll (Release)" },
    @{ Path = "x64\Debug\CBXShell.dll"; Name = "CBXShell.dll (Debug)" }
)

$buildSuccess = $true
foreach ($output in $outputs) {
    if (Test-Path $output.Path) {
        $size = (Get-Item $output.Path).Length / 1MB
        Write-Log "  ✅ $($output.Name): $([Math]::Round($size, 2)) MB" "Green"
    } else {
        Write-Log "  ❌ MISSING: $($output.Name)" "Red"
        $buildSuccess = $false
    }
}

# Summary
Write-Section "Build Summary"

$endTime = Get-Date
$duration = $endTime - $startTime

Write-Log "Build started:  $($startTime.ToString('HH:mm:ss'))" "Gray"
Write-Log "Build finished: $($endTime.ToString('HH:mm:ss'))" "Gray"
Write-Log "Duration:       $($duration.ToString('mm\:ss'))" "Gray"
Write-Log "Log file:       $logFile" "Gray"

if ($buildSuccess) {
    Write-Log "`n✅ PRODUCTION BUILD SUCCESSFUL!" "Green"
    Write-Log "   All components built and verified." "Green"
    exit 0
} else {
    Write-Log "`n❌ PRODUCTION BUILD FAILED!" "Red"
    Write-Log "   Check log file for details: $logFile" "Red"
    exit 1
}
