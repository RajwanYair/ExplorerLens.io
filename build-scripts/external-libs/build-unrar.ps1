#Requires -Version 7.0
# Build UnRAR DLL for Windows x64

$ErrorActionPreference = "Stop"

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "Building UnRAR DLL" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$rootDir = Split-Path -Parent $PSScriptRoot
$unrarDir = Join-Path $rootDir "external\compression-libs\unrar"
$projectFile = Join-Path $unrarDir "UnRARDll.vcxproj"
$outputDir = Join-Path $rootDir "x64\Release"

if (-not (Test-Path $projectFile)) {
    Write-Host "❌ ERROR: UnRAR project not found: $projectFile" -ForegroundColor Red
    exit 1
}

Write-Host "Source:  $unrarDir" -ForegroundColor Gray
Write-Host "Project: UnRARDll.vcxproj" -ForegroundColor Gray
Write-Host "Output:  $outputDir`n" -ForegroundColor Gray

# Find MSBuild
$msbuildPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe"
if (-not (Test-Path $msbuildPath)) {
    Write-Host "❌ ERROR: MSBuild not found at: $msbuildPath" -ForegroundColor Red
    exit 1
}

# Build UnRAR DLL
Write-Host "[1/2] Building UnRAR64.dll..." -ForegroundColor Yellow
try {
    $buildArgs = @(
        $projectFile
        "/p:Configuration=Release"
        "/p:Platform=x64"
        "/p:PlatformToolset=v145"
        "/v:minimal"
        "/m"
    )
    
    & $msbuildPath $buildArgs
    
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed with exit code $LASTEXITCODE"
    }
    Write-Host "  ✓ Build complete" -ForegroundColor Green
} catch {
    Write-Host "  ❌ Build error: $_" -ForegroundColor Red
    exit 1
}

# Find the output DLL
Write-Host "`n[2/2] Locating output..." -ForegroundColor Yellow
$possibleLocations = @(
    (Join-Path $unrarDir "x64\Release\UnRAR64.dll"),
    (Join-Path $unrarDir "x64\Release\UnRAR.dll"),
    (Join-Path $unrarDir "Release\UnRAR64.dll")
)

$dllPath = $null
foreach ($location in $possibleLocations) {
    if (Test-Path $location) {
        $dllPath = $location
        break
    }
}

if (-not $dllPath) {
    Write-Host "  ❌ UnRAR64.dll not found in expected locations" -ForegroundColor Red
    Write-Host "  Searched:" -ForegroundColor Yellow
    foreach ($loc in $possibleLocations) {
        Write-Host "    $loc" -ForegroundColor Gray
    }
    exit 1
}

# Copy to project output directory
$size = (Get-Item $dllPath).Length / 1KB
Write-Host "  ✓ Found: $dllPath ($([math]::Round($size, 0)) KB)" -ForegroundColor Green

if (-not (Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
}

$targetPath = Join-Path $outputDir "UnRAR64.dll"
Copy-Item $dllPath $targetPath -Force
Write-Host "  ✓ Copied to: $targetPath" -ForegroundColor Green

Write-Host "`n========================================" -ForegroundColor Green
Write-Host "✓ UnRAR Build Complete" -ForegroundColor Green
Write-Host "========================================`n" -ForegroundColor Green

Write-Host "DLL: $targetPath ($([math]::Round($size, 0)) KB)" -ForegroundColor Cyan
Write-Host "`nUnRAR64.dll provides RAR archive decompression support`n" -ForegroundColor Gray

exit 0
