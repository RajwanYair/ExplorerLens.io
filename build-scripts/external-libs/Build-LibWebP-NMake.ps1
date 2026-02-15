#Requires -Version 7.0
# DarkThumbs - Build libwebp 1.5.0 using native Makefile.vc (BEST METHOD)
# Uses the official Windows nmake build system
# Date: January 6, 2026

param(
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "Building libwebp 1.5.0 (nmake method)" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$rootDir = Split-Path -Parent $PSScriptRoot
$webpDir = Join-Path $rootDir "external\image-libs\libwebp-1.5.0"

if (-not (Test-Path $webpDir)) {
    Write-Host "❌ ERROR: libwebp-1.5.0 not found at $webpDir" -ForegroundColor Red
    exit 1
}

Write-Host "Source directory: $webpDir" -ForegroundColor Gray
Write-Host ""

Push-Location $webpDir

try {
    # Clean if requested
    if ($Clean -and (Test-Path "output")) {
        Write-Host "Cleaning previous build..." -ForegroundColor Yellow
        Remove-Item "output" -Recurse -Force -ErrorAction SilentlyContinue
    }
    
    # Setup MSVC environment
    Write-Host "[1/3] Setting up Visual Studio environment..." -ForegroundColor Cyan
    
    $vcvarsPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    if (-not (Test-Path $vcvarsPath)) {
        throw "vcvars64.bat not found at: $vcvarsPath"
    }
    
    # Create a temporary batch file to run the build
    $buildScript = @"
@echo off
call "$vcvarsPath" x64
cd /d "$webpDir"
nmake /f Makefile.vc CFG=release-static RTLIBCFG=static OBJDIR=output\x64\Release
"@
    
    $tempBat = [System.IO.Path]::GetTempFileName() + ".bat"
    $buildScript | Out-File -FilePath $tempBat -Encoding ASCII
    
    Write-Host "[2/3] Building with nmake..." -ForegroundColor Cyan
    Write-Host "      This may take a few minutes..." -ForegroundColor Gray
    Write-Host ""
    
    # Run the build
    $process = Start-Process -FilePath "cmd.exe" -ArgumentList "/c `"$tempBat`"" -NoNewWindow -Wait -PassThru -RedirectStandardOutput "$env:TEMP\webp-build-stdout.log" -RedirectStandardError "$env:TEMP\webp-build-stderr.log"
    
    Remove-Item $tempBat -Force -ErrorAction SilentlyContinue
    
    if ($process.ExitCode -ne 0) {
        Write-Host "`n❌ Build failed with exit code: $($process.ExitCode)" -ForegroundColor Red
        Write-Host "`nBuild output:" -ForegroundColor Yellow
        if (Test-Path "$env:TEMP\webp-build-stdout.log") {
            Get-Content "$env:TEMP\webp-build-stdout.log" | Select-Object -Last 30
        }
        if (Test-Path "$env:TEMP\webp-build-stderr.log") {
            Get-Content "$env:TEMP\webp-build-stderr.log"
        }
        throw "nmake failed"
    }
    
    Write-Host "[3/3] Verifying build outputs..." -ForegroundColor Cyan
    Write-Host ""
    
    # Check for built libraries in multiple possible locations
    $possibleOutputDirs = @(
        "output\release-static\x64\lib",
        "output\x64\Release\release-static\x64\lib",
        "output\x64\Release"
    )
    
    $outputDir = $null
    foreach ($dir in $possibleOutputDirs) {
        if (Test-Path $dir) {
            $outputDir = $dir
            Write-Host "  Found output directory: $outputDir" -ForegroundColor Gray
            break
        }
    }
    
    if (-not $outputDir) {
        Write-Host "  ⚠️  Standard output directories not found. Searching..." -ForegroundColor Yellow
        $foundLibs = Get-ChildItem "output" -Recurse -Filter "libwebp.lib" -ErrorAction SilentlyContinue
        if ($foundLibs) {
            $outputDir = Split-Path $foundLibs[0].FullName -Parent
            Write-Host "  Found libraries in: $outputDir" -ForegroundColor Gray
        }
    }
    
    $builtLibs = @()
    $libFiles = @(
        "$outputDir\libwebp.lib",
        "$outputDir\libwebpdecoder.lib", 
        "$outputDir\libwebpdemux.lib",
        "$outputDir\libwebpmux.lib",
        "$outputDir\libsharpyuv.lib"
    )
    
    foreach ($libFile in $libFiles) {
        if (Test-Path $libFile) {
            $size = (Get-Item $libFile).Length
            $name = Split-Path $libFile -Leaf
            Write-Host "  ✅ $name - $([Math]::Round($size/1KB, 1)) KB" -ForegroundColor Green
            $builtLibs += $libFile
        }
    }
    
    if ($builtLibs.Count -eq 0) {
        Write-Host "`n⚠️  No libraries found in $outputDir" -ForegroundColor Yellow
        Write-Host "Searching for libraries..." -ForegroundColor Gray
        Get-ChildItem "output" -Recurse -Filter "*.lib" -ErrorAction SilentlyContinue | ForEach-Object {
            Write-Host "  Found: $($_.FullName)" -ForegroundColor Gray
        }
        throw "No libraries were built!"
    }
    
    # Copy to expected location for Build-Production.ps1
    $targetDir = "build-vs\Release"
    if (-not (Test-Path $targetDir)) {
        New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
    }
    
    Write-Host ""
    Write-Host "Copying libraries to build-vs\Release..." -ForegroundColor Cyan
    
    # Copy main webp library
    $mainLib = Get-ChildItem "output" -Recurse -Filter "libwebp.lib" -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($mainLib) {
        Copy-Item $mainLib.FullName "$targetDir\webp.lib" -Force
        Write-Host "  ✅ Copied webp.lib" -ForegroundColor Green
    }
    
    # Copy sharpyuv if available
    $sharpyuvLib = Get-ChildItem "output" -Recurse -Filter "libsharpyuv.lib" -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($sharpyuvLib) {
        Copy-Item $sharpyuvLib.FullName "$targetDir\sharpyuv.lib" -Force
        Write-Host "  ✅ Copied sharpyuv.lib" -ForegroundColor Green
    }
    
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "✅ libwebp 1.5.0 Build Complete!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Libraries ready in: $targetDir" -ForegroundColor Cyan
    
} catch {
    Write-Host "`n❌ Build Failed: $($_.Exception.Message)" -ForegroundColor Red
    Pop-Location
    exit 1
} finally {
    # Cleanup temp files
    Remove-Item "$env:TEMP\webp-build-stdout.log" -Force -ErrorAction SilentlyContinue
    Remove-Item "$env:TEMP\webp-build-stderr.log" -Force -ErrorAction SilentlyContinue
}

Pop-Location

Write-Host "Ready to rebuild DarkThumbs with updated libwebp!" -ForegroundColor Cyan
Write-Host ""
