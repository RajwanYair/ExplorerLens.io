#Requires -Version 7.0
# ExplorerLens v7.0 - Complete Build & Package Script
# Builds all dependencies, projects, and creates MSI installer
#
# USAGE:
#   .\Build-All-And-Package.ps1                    # Build everything
#   .\Build-All-And-Package.ps1 -SkipDependencies  # Skip external libs
#   .\Build-All-And-Package.ps1 -Configuration Debug
#
# Date: February 16, 2026

param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release',
    
    [switch]$SkipDependencies,
    [switch]$SkipTests,
    [switch]$Clean,
    [switch]$SkipPackaging,
    [string]$Version = "7.0.0"
)

$ErrorActionPreference = 'Stop'

# Import build modules
$coreModule = Join-Path $PSScriptRoot "core\Build-Library-Core.ps1"
$helperModule = Join-Path $PSScriptRoot "core\Build-Helpers.ps1"

if (-not (Test-Path $coreModule)) {
    Write-Host "✗ Build-Library-Core.ps1 not found" -ForegroundColor Red
    exit 1
}

. $coreModule
if (Test-Path $helperModule) {
    . $helperModule
}

# ============================================================================
# Main Build Process
# ============================================================================

$stopwatch = [System.Diagnostics.Stopwatch]::StartNew()

Write-Host ""
Write-Host ("=" * 80) -ForegroundColor Cyan
Write-Host "  ExplorerLens v$Version - Complete Build Pipeline" -ForegroundColor Cyan
Write-Host ("=" * 80) -ForegroundColor Cyan
Write-Host ""
Write-Host "Configuration: $Configuration" -ForegroundColor Yellow
Write-Host "Clean Build:   $Clean" -ForegroundColor Yellow
Write-Host "Date:          $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Yellow
Write-Host ""

$rootDir = Split-Path -Parent $PSScriptRoot
$buildScriptsDir = Join-Path $rootDir "build-scripts"
$externalLibsDir = Join-Path $buildScriptsDir "external-libs"

# ============================================================================
# Phase 1: Build External Dependencies
# ============================================================================

if (-not $SkipDependencies) {
    Write-Host ""
    Write-Host ("=" * 80) -ForegroundColor Cyan
    Write-Host "  Phase 1: Building External Dependencies" -ForegroundColor Cyan
    Write-Host ("=" * 80) -ForegroundColor Cyan
    Write-Host ""
    
    $libScripts = @(
        # Compression libraries (build order matters - zlib first)
        "Build-Zlib.ps1",
        "Build-LZ4.ps1",
        "Build-Zstd.ps1",
        "Build-LZMA-SDK-26.00.ps1",
        "Build-UnRAR.ps1",
        "Build-MinizipNG.ps1",
        # Image libraries
        "Build-LibWebP-NMake.ps1",
        "Build-Dav1d.ps1",
        "Build-LibAVIF.ps1",
        "Build-LibJXL.ps1",
        "Build-LibHEIF.ps1",
        # Camera RAW
        "Build-LibRaw.ps1"
    )
    
    foreach ($script in $libScripts) {
        $scriptPath = Join-Path $externalLibsDir $script
        
        if (Test-Path $scriptPath) {
            Write-Host ""
            Write-Host "Building: $script" -ForegroundColor Cyan
            Write-Host ("-" * 80) -ForegroundColor Gray
            
            try {
                & $scriptPath -Configuration $Configuration -Clean:$Clean
                
                if ($LASTEXITCODE -ne 0) {
                    throw "Build failed with exit code $LASTEXITCODE"
                }
                
                Write-Host "✓ $script completed" -ForegroundColor Green
            } catch {
                Write-Host "✗ $script failed: $($_.Exception.Message)" -ForegroundColor Red
                exit 1
            }
        } else {
            Write-Host "⚠ Script not found: $script (skipping)" -ForegroundColor Yellow
        }
    }
    
    Write-Host ""
    Write-Host "✓ Phase 1 Complete: All external dependencies built" -ForegroundColor Green
} else {
    Write-Host ""
    Write-Host "⊘ Skipping external dependencies build" -ForegroundColor Yellow
}

# ============================================================================
# Phase 2: Build ExplorerLens Engine (CMake)
# ============================================================================

Write-Host ""
Write-Host ("=" * 80) -ForegroundColor Cyan
Write-Host "  Phase 2: Building ExplorerLens Engine" -ForegroundColor Cyan
Write-Host ("=" * 80) -ForegroundColor Cyan
Write-Host ""

$buildMsvcScript = Join-Path $buildScriptsDir "Build-MSVC.ps1"

if (Test-Path $buildMsvcScript) {
    try {
        $preset = if ($Configuration -eq 'Debug') { 'default-debug' } else { 'default-release' }

        Write-Host "Using Build-MSVC pipeline (preset: $preset, target: ExplorerLensEngine)" -ForegroundColor Cyan

        $buildArgs = @{
            Preset = $preset
            Jobs = 8
            Target = "ExplorerLensEngine"
            Clean = $Clean
        }

        & $buildMsvcScript @buildArgs

        if ($LASTEXITCODE -ne 0) {
            throw "Build-MSVC failed with exit code $LASTEXITCODE"
        }
        
        Write-Host "✓ Phase 2 Complete: ExplorerLens Engine built" -ForegroundColor Green
    } catch {
        Write-Host "✗ Engine build failed: $($_.Exception.Message)" -ForegroundColor Red
        exit 1
    }
} else {
    Write-Host "✗ Build script not found: $buildMsvcScript" -ForegroundColor Red
    exit 1
}

# ============================================================================
# Phase 3: Build LENSShell & LENSManager (MSBuild)
# ============================================================================

Write-Host ""
Write-Host ("=" * 80) -ForegroundColor Cyan
Write-Host "  Phase 3: Building LENSShell Solution" -ForegroundColor Cyan
Write-Host ("=" * 80) -ForegroundColor Cyan
Write-Host ""

$solutionFile = Join-Path $rootDir "LENSShell.sln"

if (Test-Path $solutionFile) {
    try {
        $msbuildPath = Find-MSBuildPath
        
        if (-not $msbuildPath) {
            throw "MSBuild not found"
        }
        
        Write-Host "Using MSBuild: $msbuildPath" -ForegroundColor Cyan
        Write-Host "Building solution: $solutionFile" -ForegroundColor Cyan
        Write-Host ""
        
        $target = if ($Clean) { 'Rebuild' } else { 'Build' }
        
        & $msbuildPath `
            $solutionFile `
            /p:Configuration=$Configuration `
            /p:Platform=x64 `
            /t:$target `
            /m `
            /v:minimal `
            /nologo
        
        if ($LASTEXITCODE -ne 0) {
            throw "MSBuild failed with exit code $LASTEXITCODE"
        }
        
        Write-Host ""
        Write-Host "✓ Phase 3 Complete: LENSShell solution built" -ForegroundColor Green
    } catch {
        Write-Host "✗ Solution build failed: $($_.Exception.Message)" -ForegroundColor Red
        exit 1
    }
} else {
    Write-Host "✗ Solution file not found: $solutionFile" -ForegroundColor Red
    exit 1
}

# ============================================================================
# Phase 4: Create MSI Installer Package
# ============================================================================

if (-not $SkipPackaging) {
    Write-Host ""
    Write-Host ("=" * 80) -ForegroundColor Cyan
    Write-Host "  Phase 4: Creating MSI Installer Package" -ForegroundColor Cyan
    Write-Host ("=" * 80) -ForegroundColor Cyan
    Write-Host ""
    
    $packagingScript = Join-Path $rootDir "packaging\Build-Installer.ps1"
    
    if (Test-Path $packagingScript) {
        try {
            & $packagingScript -Configuration $Configuration -Version $Version
            
            if ($LASTEXITCODE -ne 0) {
                throw "Packaging failed with exit code $LASTEXITCODE"
            }
            
            Write-Host ""
            Write-Host "✓ Phase 4 Complete: MSI installer created" -ForegroundColor Green
        } catch {
            Write-Host "✗ Packaging failed: $($_.Exception.Message)" -ForegroundColor Red
            exit 1
        }
    } else {
        Write-Host "⚠ Packaging script not found: $packagingScript" -ForegroundColor Yellow
        Write-Host "   Binaries are built, but MSI installer not created" -ForegroundColor Yellow
    }
} else {
    Write-Host ""
    Write-Host "⊘ Skipping MSI packaging" -ForegroundColor Yellow
}

# ============================================================================
# Build Summary
# ============================================================================

$stopwatch.Stop()
$elapsed = $stopwatch.Elapsed

Write-Host ""
Write-Host ("=" * 80) -ForegroundColor Cyan
Write-Host "  Build Pipeline Complete!" -ForegroundColor Green
Write-Host ("=" * 80) -ForegroundColor Cyan
Write-Host ""
Write-Host "Total Time: $($elapsed.ToString('hh\:mm\:ss'))" -ForegroundColor Cyan
Write-Host "Configuration: $Configuration" -ForegroundColor Cyan
Write-Host ""
Write-Host "Build Artifacts:" -ForegroundColor Yellow
Write-Host "  • ExplorerLens Engine: build\lib\$Configuration\" -ForegroundColor Gray
Write-Host "  • LENSShell DLL:      LENSShell\x64\$Configuration\LENSShell.dll" -ForegroundColor Gray
Write-Host "  • LENSManager EXE:    LENSManager\x64\$Configuration\LENSManager.exe" -ForegroundColor Gray

if (-not $SkipPackaging) {
    $msiFile = Join-Path $rootDir "packaging\output\ExplorerLens-Setup-$Version.msi"
    if (Test-Path $msiFile) {
        Write-Host "  • MSI Installer:     packaging\output\ExplorerLens-Setup-$Version.msi" -ForegroundColor Gray
    }
}

Write-Host ""
Write-Host "Next Steps:" -ForegroundColor Yellow
Write-Host "  • Run tests:      .\scripts\run-tests.ps1" -ForegroundColor Gray
Write-Host "  • Install MSI:    msiexec /i packaging\output\ExplorerLens-Setup-$Version.msi" -ForegroundColor Gray
Write-Host "  • Register COM:   regsvr32 LENSShell\x64\$Configuration\LENSShell.dll" -ForegroundColor Gray
Write-Host ""

exit 0

