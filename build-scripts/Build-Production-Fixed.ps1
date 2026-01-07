#Requires -Version 7.0

<#
.SYNOPSIS
    Fixed production build script for DarkThumbs
.DESCRIPTION
    Complete clean build with all identified fixes applied
.NOTES
    FIXES APPLIED:
    1. Updated lzma script to use xz-5.6.3 (has CMake support)
    2. Improved error handling and logging
    3. Added validation steps
#>

param(
    [switch]$Clean,
    [switch]$SkipLibraries,
    [switch]$Verbose,
    [switch]$TestOnly
)

$ErrorActionPreference = "Stop"
$startTime = Get-Date

# Ensure build-logs directory exists
$rootDir = $PSScriptRoot
Set-Location $rootDir

if (-not (Test-Path "build-logs")) {
    New-Item -Path "build-logs" -ItemType Directory -Force | Out-Null
}

$logFile = "build-logs\production-build-$(Get-Date -Format 'yyyy-MM-dd_HHmmss').log"

function Write-Log {
    param([string]$Message, [string]$Color = "White")
    $timestamp = Get-Date -Format "HH:mm:ss"
    $logMessage = "[$timestamp] $Message"
    Write-Host $logMessage -ForegroundColor $Color
    Add-Content -Path $logFile -Value $logMessage -ErrorAction SilentlyContinue
}

function Write-Section {
    param([string]$Title)
    Write-Log ""
    Write-Log "========================================" "Cyan"
    Write-Log "  $Title" "Cyan"
    Write-Log "========================================" "Cyan"
}

Write-Section "DarkThumbs Production Build (Fixed)"
Write-Log "Log file: $logFile" "Gray"
Write-Log "Build started: $startTime" "Gray"
Write-Log ""
Write-Log "APPLIED FIXES:" "Yellow"
Write-Log "  1. Updated lzma build to use xz-5.6.3 directory" "Gray"
Write-Log "  2. Improved libwebp build error detection" "Gray"
Write-Log "  3. Enhanced minizip dependency resolution" "Gray"

# Test environment first
if ($TestOnly) {
    Write-Section "Testing Build Environment"
    & "$PSScriptRoot\build-scripts\Test-Build-Environment.ps1"
    exit $LASTEXITCODE
}

# Clean build if requested
if ($Clean) {
    Write-Section "Cleaning Build Artifacts"
    
    $cleanDirs = @(
        "build",
        "x64", 
        "packages",
        "CBXShell\x64",
        "CBXManager\x64",
        "external\compression\zlib-1.3.1\x64",
        "external\compression\lz4-1.10.0\build\VS2022\x64",
        "external\compression\zstd-1.5.7\build\VS2022\bin",
        "external\compression\xz-5.6.3\build-vs",
        "external\compression\minizip-ng-4.0.10\build-vs",
        "external\image-libs\libwebp-1.5.0\output",
        "external\image-libs\libwebp-1.5.0\build-vs"
    )
    
    foreach ($dir in $cleanDirs) {
        $fullPath = Join-Path $rootDir $dir
        if (Test-Path $fullPath) {
            Write-Log "  Removing: $dir" "Yellow"
            Remove-Item -Path $fullPath -Recurse -Force -ErrorAction SilentlyContinue
        }
    }
    
    Write-Log "✅ Clean complete" "Green"
}

# Build libraries
if (-not $SkipLibraries) {
    Write-Section "Building Compression & Image Libraries"
    
    $buildScripts = @(
        @{ Name = "Build-Zlib.ps1"; Description = "zlib 1.3.1 (DEFLATE compression)" },
        @{ Name = "Build-LZ4.ps1"; Description = "LZ4 1.10.0 (fast compression)" },
        @{ Name = "Build-Zstd.ps1"; Description = "Zstandard 1.5.7" },
        @{ Name = "build-lzma-24.08.ps1"; Description = "liblzma from XZ 5.6.3 (FIXED)" },
        @{ Name = "Build-LibWebP-NMake.ps1"; Description = "libwebp 1.5.0" },
        @{ Name = "Build-MinizipNG.ps1"; Description = "minizip-ng 4.0.10" }
    )
    
    $libraryResults = @()
    
    foreach ($scriptInfo in $buildScripts) {
        $scriptPath = Join-Path "$PSScriptRoot\build-scripts" $scriptInfo.Name
        
        if (-not (Test-Path $scriptPath)) {
            Write-Log "`n⚠️  Script not found: $($scriptInfo.Name)" "Yellow"
            continue
        }
        
        Write-Log "`nBuilding: $($scriptInfo.Description)" "Cyan"
        Write-Log "  Script: $($scriptInfo.Name)" "Gray"
        
        try {
            $buildStart = Get-Date
            
            if ($Verbose) {
                & $scriptPath *>&1 | Tee-Object -Variable buildOutput | ForEach-Object {
                    Write-Log "    $_" "DarkGray"
                }
            } else {
                $buildOutput = & $scriptPath 2>&1
            }
            
            $buildDuration = (Get-Date) - $buildStart
            
            if ($LASTEXITCODE -eq 0 -or $?) {
                Write-Log "  ✅ $($scriptInfo.Name) succeeded ($([Math]::Round($buildDuration.TotalSeconds, 1))s)" "Green"
                $libraryResults += [PSCustomObject]@{
                    Script      = $scriptInfo.Name
                    Description = $scriptInfo.Description
                    Status      = "✅ Success"
                    Duration    = "$([Math]::Round($buildDuration.TotalSeconds, 1))s"
                }
            } else {
                Write-Log "  ❌ $($scriptInfo.Name) failed" "Red"
                if (-not $Verbose -and $buildOutput) {
                    Write-Log "  Error output:" "Yellow"
                    $buildOutput | Select-Object -Last 10 | ForEach-Object {
                        Write-Log "    $_" "Red"
                    }
                }
                $libraryResults += [PSCustomObject]@{
                    Script      = $scriptInfo.Name
                    Description = $scriptInfo.Description
                    Status      = "❌ Failed"
                    Duration    = "$([Math]::Round($buildDuration.TotalSeconds, 1))s"
                }
            }
        } catch {
            Write-Log "  ❌ Error: $_" "Red"
            $libraryResults += [PSCustomObject]@{
                Script      = $scriptInfo.Name
                Description = $scriptInfo.Description
                Status      = "❌ Error: $($_.Exception.Message)"
                Duration    = "N/A"
            }
        }
    }
    
    Write-Section "Library Build Summary"
    $libraryResults | ForEach-Object {
        Write-Log "$($_.Description): $($_.Status) [$($_.Duration)]"
    }
    
    $successCount = ($libraryResults | Where-Object { $_.Status -like "*Success*" }).Count
    $failCount = ($libraryResults | Where-Object { $_.Status -like "*Failed*" -or $_.Status -like "*Error*" }).Count
    
    Write-Log ""
    Write-Log "Success: $successCount / $($libraryResults.Count)" $(if ($failCount -eq 0) { "Green" } else { "Yellow" })
    Write-Log "Failed: $failCount" $(if ($failCount -gt 0) { "Red" } else { "Green" })
    
    if ($failCount -gt 0) {
        Write-Log ""
        Write-Log "⚠️  Some libraries failed to build. Check the log for details." "Yellow"
        Write-Log "You can continue with CBXShell build if the required libraries are available." "Yellow"
    }
}

# Build CBXShell
Write-Section "Building CBXShell Solution"

$msbuildScript = Join-Path "$PSScriptRoot\build-scripts" "Find-MSBuild.ps1"
if (Test-Path $msbuildScript) {
    try {
        $msbuild = & $msbuildScript
        
        if ($msbuild -and (Test-Path $msbuild)) {
            Write-Log "MSBuild: $msbuild" "Gray"
            
            $slnPath = Join-Path $rootDir "CBXShell.sln"
            
            if (Test-Path $slnPath) {
                Write-Log "Building CBXShell.sln..." "Cyan"
                
                & $msbuild $slnPath /p:Configuration=Release /p:Platform=x64 /m /v:minimal
                
                if ($LASTEXITCODE -eq 0) {
                    Write-Log "✅ CBXShell build succeeded" "Green"
                } else {
                    Write-Log "❌ CBXShell build failed" "Red"
                }
            } else {
                Write-Log "❌ CBXShell.sln not found" "Red"
            }
        } else {
            Write-Log "❌ MSBuild not found" "Red"
        }
    } catch {
        Write-Log "❌ Error building CBXShell: $_" "Red"
    }
} else {
    Write-Log "⚠️  Find-MSBuild.ps1 not found, skipping CBXShell build" "Yellow"
}

# Final summary
$endTime = Get-Date
$totalDuration = $endTime - $startTime

Write-Section "Build Complete"
Write-Log "Started:  $startTime" "Gray"
Write-Log "Finished: $endTime" "Gray"
Write-Log "Duration: $([Math]::Round($totalDuration.TotalMinutes, 1)) minutes" "White"
Write-Log ""
Write-Log "Log file: $logFile" "Cyan"

Write-Host ""
