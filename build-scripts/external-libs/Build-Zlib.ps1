# ===========================================================================
# Build-Zlib.ps1
# Build zlib 1.3.1 - DEFLATE Compression Library
# ===========================================================================

param(
    [string]$Configuration = "Release",
    [string]$Platform = "x64"
)

$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host "Building zlib 1.3.1 (DEFLATE Compression)" -ForegroundColor Cyan
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host ""

# Get project root
$ProjectRoot = Split-Path -Parent $PSScriptRoot
Set-Location $ProjectRoot

# Check if already built
$ExistingLib = Join-Path $ProjectRoot "external\compression\zlib-1.3.1\x64\Release\zlibstatic.lib"
if (Test-Path $ExistingLib) {
    $Size = (Get-Item $ExistingLib).Length
    Write-Host "zlib already built: $ExistingLib ($Size bytes)" -ForegroundColor Green
    exit 0
}

# Find MSBuild
$MSBuild = & "$PSScriptRoot\Find-MSBuild.ps1"
if (-not $MSBuild) {
    Write-Host "ERROR: MSBuild not found" -ForegroundColor Red
    exit 1
}

# Try to find existing vcxproj, or use CMake/Ninja
$Project = Join-Path $ProjectRoot "external\compression\zlib-1.3.1\build-vs\zlib.vcxproj"
$ZlibDir = Join-Path $ProjectRoot "external\compression\zlib-1.3.1"

if (-not (Test-Path $Project)) {
    # Use CMake/Ninja build
    Write-Host "Using CMake/Ninja build..." -ForegroundColor Yellow
    $buildDir = Join-Path $ZlibDir "x64\Release"
    
    if (-not (Test-Path $buildDir)) {
        New-Item -Path $buildDir -ItemType Directory -Force | Out-Null
    }
    
    Push-Location $buildDir
    try {
        cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CONFIGURATION_TYPES=Release -DBUILD_SHARED_LIBS=OFF "../.."
        ninja zlibstatic
        if ($LASTEXITCODE -ne 0) {
            Write-Host "ERROR: ninja build failed" -ForegroundColor Red
            exit 1
        }
    } finally {
        Pop-Location
    }
    
    exit 0
}

Write-Host "Project: $Project" -ForegroundColor White
Write-Host ""

# Clean and build
& $MSBuild $Project /t:Clean /p:Configuration=$Configuration /p:Platform=$Platform /v:minimal /nologo
& $MSBuild $Project /t:Build /p:Configuration=$Configuration /p:Platform=$Platform /v:minimal /nologo

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "[FAILED] zlib build failed!" -ForegroundColor Red
    exit 1
}

$Output = Join-Path $ProjectRoot "external\compression\zlib-1.3.1\build-vs\$Configuration\zlibstatic.lib"
if (Test-Path $Output) {
    $Size = (Get-Item $Output).Length
    Write-Host ""
    Write-Host "[SUCCESS] zlibstatic.lib built: $Size bytes" -ForegroundColor Green
    exit 0
} else {
    Write-Host ""
    Write-Host "[ERROR] Output file not found: $Output" -ForegroundColor Red
    exit 1
}
