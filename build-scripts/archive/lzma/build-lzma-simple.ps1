#Requires -Version 7.0
# Build liblzma from XZ Utils 5.6.3
# Simplified version with direct file-based execution

$ErrorActionPreference = "Stop"

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "Building liblzma from XZ Utils 5.6.3" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$rootDir = Split-Path -Parent $PSScriptRoot
$xzDir = Join-Path $rootDir "external\compression\xz-5.6.3"
$buildDir = Join-Path $xzDir "build-vs"

if (-not (Test-Path $xzDir)) {
    Write-Host "❌ ERROR: XZ source directory not found: $xzDir" -ForegroundColor Red
    exit 1
}

Write-Host "Source: $xzDir" -ForegroundColor Gray
Write-Host "Build:  $buildDir" -ForegroundColor Gray
Write-Host ""

# Clean and recreate build directory
if (Test-Path $buildDir) {
    Write-Host "[1/4] Cleaning previous build..." -ForegroundColor Yellow
    Remove-Item $buildDir -Recurse -Force
}
New-Item -ItemType Directory -Path $buildDir -Force | Out-Null

# Create a batch file for the build
$batchFile = Join-Path $buildDir "build.bat"
$batchContent = @"
@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" x64
cd /d "$buildDir"

echo Configuring XZ Utils...
cmake "$xzDir" -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DENABLE_NLS=OFF -DCMAKE_C_FLAGS_RELEASE="/MT /O2 /DNDEBUG" -DCMAKE_CXX_FLAGS_RELEASE="/MT /O2 /DNDEBUG"
if errorlevel 1 exit /b 1

echo Building liblzma...
nmake liblzma
if errorlevel 1 exit /b 1

echo Build complete!
exit /b 0
"@

$batchContent | Out-File -FilePath $batchFile -Encoding ASCII

Write-Host "[2/4] Configuring and building..." -ForegroundColor Yellow
Write-Host "      This may take a minute..." -ForegroundColor Gray
Write-Host ""

# Execute the batch file
$process = Start-Process -FilePath "cmd.exe" -ArgumentList "/c `"$batchFile`"" -NoNewWindow -Wait -PassThru

if ($process.ExitCode -ne 0) {
    Write-Host "`n❌ Build failed with exit code: $($process.ExitCode)" -ForegroundColor Red
    Write-Host "Check the build directory for error logs:" -ForegroundColor Yellow
    Write-Host "  $buildDir" -ForegroundColor Gray
    exit 1
}

Write-Host "[3/4] Locating output library..." -ForegroundColor Yellow

# Find the built library
$possiblePaths = @(
    (Join-Path $buildDir "liblzma.lib"),
    (Join-Path $buildDir "liblzma.a"),
    (Join-Path $buildDir "Release\liblzma.lib")
)

$outputLib = $null
foreach ($path in $possiblePaths) {
    if (Test-Path $path) {
        $outputLib = $path
        break
    }
}

# Search recursively if not found
if (-not $outputLib) {
    Write-Host "  Searching build directory..." -ForegroundColor Gray
    $found = Get-ChildItem $buildDir -Recurse -Include "liblzma.lib", "liblzma.a" -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($found) {
        $outputLib = $found.FullName
    }
}

if (-not $outputLib) {
    Write-Host "`n❌ ERROR: liblzma library not found!" -ForegroundColor Red
    Write-Host "Searched in:" -ForegroundColor Yellow
    foreach ($path in $possiblePaths) {
        Write-Host "  $path" -ForegroundColor Gray
    }
    exit 1
}

Write-Host "  ✅ Found: $(Split-Path $outputLib -Leaf)" -ForegroundColor Green
$size = (Get-Item $outputLib).Length
Write-Host "  Size: $([Math]::Round($size/1KB, 1)) KB" -ForegroundColor Gray

Write-Host "`n[4/4] Copying to standard location..." -ForegroundColor Yellow

# Ensure Release subdirectory exists
$releaseDir = Join-Path $buildDir "Release"
if (-not (Test-Path $releaseDir)) {
    New-Item -ItemType Directory -Path $releaseDir -Force | Out-Null
}

# Copy to Release\liblzma.lib
$targetPath = Join-Path $releaseDir "liblzma.lib"
Copy-Item $outputLib -Destination $targetPath -Force
Write-Host "  ✅ Copied to: build-vs\Release\liblzma.lib" -ForegroundColor Green

Write-Host "`n========================================" -ForegroundColor Green
Write-Host "✅ liblzma build completed successfully!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Output: $targetPath" -ForegroundColor Cyan
Write-Host "Size:   $([Math]::Round((Get-Item $targetPath).Length/1KB, 1)) KB" -ForegroundColor Cyan
Write-Host ""

exit 0
