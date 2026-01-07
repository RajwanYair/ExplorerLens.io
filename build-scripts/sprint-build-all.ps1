# DarkThumbs v5.0 - Master Build Script for All Sprints
# PowerShell script to execute complete sprint sequence

param(
    [switch]$DownloadOnly,
    [switch]$BuildOnly,
    [switch]$SkipTests
)

$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "DarkThumbs v5.0 Complete Sprint Builder" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RootDir = Split-Path -Parent $ScriptDir

# Sprint execution tracker
$Sprints = @(
    @{
        Number = 2
        Name = "AVIF/HEIC/HEIF Support"
        Scripts = @("build-dav1d.ps1", "build-libavif.ps1")
        IntegrationFiles = @("avif_decoder.cpp", "avif_decoder.h")
    },
    @{
        Number = 3
        Name = "JPEG XL Support"
        Scripts = @("build-libjxl.ps1")
        IntegrationFiles = @("jxl_decoder.cpp", "jxl_decoder.h")
    },
    @{
        Number = 4
        Name = "PDF Support"
        Scripts = @()
        IntegrationFiles = @("pdf_decoder.cpp", "pdf_decoder.h")
        Notes = "Requires MuPDF download and integration"
    },
    @{
        Number = 5
        Name = "CHM & OpenDoc Formats"
        Scripts = @()
        IntegrationFiles = @()
        Notes = "Uses Windows native CHM APIs, existing minizip-ng"
    },
    @{
        Number = 6
        Name = "Library Updates"
        Scripts = @("build-zstd.ps1")
        Notes = "Update zstd to 1.5.7"
    },
    @{
        Number = 7
        Name = "Build and Test"
        Scripts = @()
        Notes = "Run rebuild-all.cmd and comprehensive tests"
    }
)

# Phase 1: Download all libraries
if (-not $BuildOnly) {
    Write-Host "Phase 1: Downloading Libraries" -ForegroundColor Cyan
    Write-Host "------------------------------" -ForegroundColor Cyan
    
    $DownloadScript = Join-Path $ScriptDir "download-all-libs.ps1"
    if (Test-Path $DownloadScript) {
        Write-Host "Executing: download-all-libs.ps1" -ForegroundColor Yellow
        & powershell -ExecutionPolicy Bypass -File $DownloadScript
        
        if ($LASTEXITCODE -ne 0) {
            Write-Host "[ERROR] Library download failed" -ForegroundColor Red
            exit 1
        }
    }
    else {
        Write-Host "[ERROR] Download script not found: $DownloadScript" -ForegroundColor Red
        exit 1
    }
    
    Write-Host ""
}

if ($DownloadOnly) {
    Write-Host "[COMPLETE] Downloads finished. Use -BuildOnly to continue." -ForegroundColor Green
    exit 0
}

# Phase 2: Build all libraries
Write-Host "Phase 2: Building Libraries" -ForegroundColor Cyan
Write-Host "----------------------------" -ForegroundColor Cyan
Write-Host ""

foreach ($Sprint in $Sprints) {
    if ($Sprint.Scripts.Count -eq 0) { continue }
    
    Write-Host "Sprint $($Sprint.Number): $($Sprint.Name)" -ForegroundColor Magenta
    Write-Host ("=" * 50) -ForegroundColor Magenta
    
    foreach ($Script in $Sprint.Scripts) {
        $ScriptPath = Join-Path $ScriptDir $Script
        
        if (Test-Path $ScriptPath) {
            Write-Host "  Building: $Script" -ForegroundColor Yellow
            & powershell -ExecutionPolicy Bypass -File $ScriptPath
            
            if ($LASTEXITCODE -ne 0) {
                Write-Host "  [ERROR] Build failed: $Script" -ForegroundColor Red
                Write-Host ""
                Write-Host "Sprint $($Sprint.Number) failed. Please fix and retry." -ForegroundColor Red
                exit 1
            }
            
            Write-Host "  [OK] $Script completed" -ForegroundColor Green
        }
        else {
            Write-Host "  [SKIP] Script not found: $Script" -ForegroundColor Yellow
        }
    }
    
    Write-Host ""
}

# Phase 3: Integration status
Write-Host "Phase 3: Integration Requirements" -ForegroundColor Cyan
Write-Host "----------------------------------" -ForegroundColor Cyan
Write-Host ""

$CBXShellDir = Join-Path $RootDir "CBXShell"

foreach ($Sprint in $Sprints) {
    if ($Sprint.IntegrationFiles.Count -eq 0) { continue }
    
    Write-Host "Sprint $($Sprint.Number): $($Sprint.Name)" -ForegroundColor Magenta
    
    foreach ($File in $Sprint.IntegrationFiles) {
        $FilePath = Join-Path $CBXShellDir $File
        
        if (Test-Path $FilePath) {
            Write-Host "  [EXISTS] $File" -ForegroundColor Green
        }
        else {
            Write-Host "  [TODO] Create $File" -ForegroundColor Yellow
        }
    }
    
    if ($Sprint.Notes) {
        Write-Host "  Note: $($Sprint.Notes)" -ForegroundColor Gray
    }
    
    Write-Host ""
}

# Phase 4: Final build and test
Write-Host "Phase 4: Project Build and Test" -ForegroundColor Cyan
Write-Host "--------------------------------" -ForegroundColor Cyan
Write-Host ""

if (-not $SkipTests) {
    Write-Host "Ready for final build. Execute these commands:" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "  1. cd $RootDir" -ForegroundColor White
    Write-Host "  2. rebuild-all.cmd" -ForegroundColor White
    Write-Host "  3. sprint-test.cmd" -ForegroundColor White
    Write-Host ""
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Library builds complete!" -ForegroundColor Green
Write-Host "Next: Integrate decoders into CBXShell" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
