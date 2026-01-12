#Requires -Version 7.0
<#
.SYNOPSIS
    Comprehensive build verification for DarkThumbs project.

.DESCRIPTION
    Verifies that all external libraries and main project outputs are built correctly.
    Checks file existence, sizes, and provides a complete build status report.

.NOTES
    Author: DarkThumbs Build System
    Created: January 12, 2026
    Version: 1.0.0
#>

[CmdletBinding()]
param()

# ANSI color codes
$script:Green = "`e[32m"
$script:Red = "`e[31m"
$script:Yellow = "`e[33m"
$script:Blue = "`e[34m"
$script:Cyan = "`e[36m"
$script:Reset = "`e[0m"

# Build status tracking
$script:TotalChecks = 0
$script:PassedChecks = 0
$script:FailedChecks = 0
$script:Warnings = 0

function Write-Header {
    param([string]$Text)
    Write-Host "`n$script:Cyan=== $Text ===$script:Reset" -ForegroundColor Cyan
}

function Write-Success {
    param([string]$Text)
    Write-Host "  $script:Green✓$script:Reset $Text" -ForegroundColor Green
}

function Write-Failure {
    param([string]$Text)
    Write-Host "  $script:Red✗$script:Reset $Text" -ForegroundColor Red
}

function Write-Warning {
    param([string]$Text)
    Write-Host "  $script:Yellow⚠$script:Reset $Text" -ForegroundColor Yellow
}

function Test-Library {
    param(
        [string]$Name,
        [string]$Path,
        [int]$MinSizeKB
    )
    
    $script:TotalChecks++
    
    if (Test-Path $Path) {
        $file = Get-Item $Path
        $sizeKB = [math]::Round($file.Length / 1KB, 0)
        
        if ($sizeKB -ge $MinSizeKB) {
            $script:PassedChecks++
            Write-Success "$Name : $sizeKB KB (built: $($file.LastWriteTime.ToString('yyyy-MM-dd HH:mm')))"
            return $true
        } else {
            $script:FailedChecks++
            Write-Failure "$Name : $sizeKB KB (expected >= $MinSizeKB KB)"
            return $false
        }
    } else {
        $script:FailedChecks++
        Write-Failure "$Name : NOT FOUND at $Path"
        return $false
    }
}

function Test-OutputFile {
    param(
        [string]$Name,
        [string]$Path,
        [int]$MinSizeKB,
        [int]$MaxSizeKB = [int]::MaxValue
    )
    
    $script:TotalChecks++
    
    if (Test-Path $Path) {
        $file = Get-Item $Path
        $sizeKB = [math]::Round($file.Length / 1KB, 0)
        
        if ($sizeKB -ge $MinSizeKB -and $sizeKB -le $MaxSizeKB) {
            $script:PassedChecks++
            Write-Success "$Name : $sizeKB KB (built: $($file.LastWriteTime.ToString('yyyy-MM-dd HH:mm')))"
            return $true
        } else {
            $script:FailedChecks++
            Write-Failure "$Name : $sizeKB KB (expected $MinSizeKB - $MaxSizeKB KB)"
            return $false
        }
    } else {
        $script:FailedChecks++
        Write-Failure "$Name : NOT FOUND at $Path"
        return $false
    }
}

# Main verification
Write-Host "`n$script:Blue╔═══════════════════════════════════════════════════════╗$script:Reset"
Write-Host "$script:Blue║   DarkThumbs Complete Build Verification v1.0.0     ║$script:Reset"
Write-Host "$script:Blue╚═══════════════════════════════════════════════════════╝$script:Reset"

# Get project root
$projectRoot = Split-Path -Parent $PSScriptRoot

# === Compression Libraries ===
Write-Header "Compression Libraries"

Test-Library "zlib 1.3.1" `
    "$projectRoot\external\compression\zlib-1.3.1\x64\Release\zlibstatic.lib" `
    100

Test-Library "LZ4 1.10.0" `
    "$projectRoot\external\compression\lz4-1.10.0\build-vs\Release\liblz4_static.lib" `
    500

Test-Library "zstd 1.5.7" `
    "$projectRoot\external\compression\zstd-1.5.7\build-manual\zstd_static.lib" `
    1000

Test-Library "LZMA SDK 24.08" `
    "$projectRoot\external\compression\lzma-24.08\C\build-vs\lib\lzma.lib" `
    1800

Test-Library "liblzma (xz-5.6.3)" `
    "$projectRoot\external\compression\xz-5.6.3\build-vs\lzma.lib" `
    400

Test-Library "Minizip-NG 4.0.10" `
    "$projectRoot\external\compression\minizip-ng-4.0.10\build-manual\minizip.lib" `
    250

# === Image Libraries ===
Write-Header "Image Processing Libraries"

Test-Library "LibWebP 1.5.0 (full)" `
    "$projectRoot\external\image-libs\libwebp-1.5.0\output\x64\Release\release-static\x64\lib\libwebp.lib" `
    1500

Test-Library "LibWebP 1.5.0 (decoder)" `
    "$projectRoot\external\image-libs\libwebp-1.5.0\output\x64\Release\release-static\x64\lib\libwebpdecoder.lib" `
    700

# Check for optional image libraries
$libavifPath = "$projectRoot\external\image-libs\libavif-1.3.0\build\x64\Release\avif.lib"
if (Test-Path $libavifPath) {
    Test-Library "LibAVIF 1.3.0 (optional)" $libavifPath 100
} else {
    $script:Warnings++
    Write-Warning "LibAVIF 1.3.0: Not built (optional, AVIF support disabled)"
}

$libjxlPath = "$projectRoot\external\image-libs\libjxl-0.11.1\build\lib\Release\jxl.lib"
if (Test-Path $libjxlPath) {
    Test-Library "LibJXL 0.11.1 (optional)" $libjxlPath 100
} else {
    $script:Warnings++
    Write-Warning "LibJXL 0.11.1: Not built (optional, JPEG XL support disabled)"
}

# === Archive Libraries ===
Write-Header "Archive Libraries"

$unrarPath = "$projectRoot\x64\Release\UnRAR64.dll"
if (Test-Path $unrarPath) {
    Test-OutputFile "UnRAR 7.2.2 DLL" $unrarPath 300 400
} else {
    $script:Warnings++
    Write-Warning "UnRAR DLL: Not found (RAR support may be disabled)"
}

# === Main Project Outputs ===
Write-Header "Main Project Outputs"

Test-OutputFile "CBXShell.dll" `
    "$projectRoot\x64\Release\CBXShell.dll" `
    1000 1500

Test-OutputFile "CBXManager.exe" `
    "$projectRoot\x64\Release\CBXManager.exe" `
    250 350

# === Visual Studio Solution Files ===
Write-Header "Project Configuration"

$script:TotalChecks++
if (Test-Path "$projectRoot\CBXShell.sln") {
    $script:PassedChecks++
    Write-Success "Solution file: CBXShell.sln"
} else {
    $script:FailedChecks++
    Write-Failure "Solution file: CBXShell.sln NOT FOUND"
}

$script:TotalChecks++
if (Test-Path "$projectRoot\CBXShell\CBXShell.vcxproj") {
    $script:PassedChecks++
    Write-Success "Project file: CBXShell.vcxproj"
} else {
    $script:FailedChecks++
    Write-Failure "Project file: CBXShell.vcxproj NOT FOUND"
}

# === Build Scripts ===
Write-Header "Build Scripts"

$buildScripts = @(
    "build.ps1",
    "Build-Production-SlowMachine.ps1",
    "build-lzma-sdk-24.08.ps1",
    "build-unrar.ps1",
    "Build-LibWebP-NMake.ps1",
    "Build-MinizipNG.ps1",
    "Build-Zstd.ps1",
    "Build-LZ4.ps1",
    "Build-Zlib.ps1"
)

foreach ($script in $buildScripts) {
    $scriptPath = "$projectRoot\build-scripts\$script"
    $script:TotalChecks++
    if (Test-Path $scriptPath) {
        $script:PassedChecks++
        Write-Success "Build script: $script"
    } else {
        $script:FailedChecks++
        Write-Failure "Build script: $script NOT FOUND"
    }
}

# === Final Summary ===
Write-Host "`n$script:Blue╔═══════════════════════════════════════════════════════╗$script:Reset"
Write-Host "$script:Blue║                  Verification Summary                 ║$script:Reset"
Write-Host "$script:Blue╚═══════════════════════════════════════════════════════╝$script:Reset"

Write-Host "`nTotal Checks    : $script:TotalChecks"
Write-Host "Passed          : $script:Green$script:PassedChecks$script:Reset" -NoNewline
Write-Host " ($(([math]::Round(($script:PassedChecks / $script:TotalChecks) * 100, 1)))%)"
Write-Host "Failed          : $script:Red$script:FailedChecks$script:Reset"
Write-Host "Warnings        : $script:Yellow$script:Warnings$script:Reset"

# Overall status
Write-Host ""
if ($script:FailedChecks -eq 0) {
    Write-Host "$script:Green✓ BUILD VERIFICATION PASSED$script:Reset" -ForegroundColor Green
    Write-Host "  All critical components are built and ready."
    
    if ($script:Warnings -gt 0) {
        Write-Host "`n$script:Yellow⚠ Note: $script:Warnings optional component(s) not built$script:Reset" -ForegroundColor Yellow
        Write-Host "  This is normal if you haven't built optional image libraries yet."
    }
    
    exit 0
} else {
    Write-Host "$script:Red✗ BUILD VERIFICATION FAILED$script:Reset" -ForegroundColor Red
    Write-Host "  $script:FailedChecks critical component(s) missing or incorrect."
    Write-Host "  Run the appropriate build scripts to fix the issues."
    exit 1
}
