# ===========================================================================
# Build-LZ4.ps1
# Build LZ4 1.10.0 - Fast Compression Library
# ===========================================================================

param(
    [string]$Configuration = "Release",
    [string]$Platform = "x64"
)

$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host "Building LZ4 1.10.0 (Fast Compression Library)" -ForegroundColor Cyan
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host ""

# Get project root
$ProjectRoot = Split-Path -Parent $PSScriptRoot
Set-Location $ProjectRoot

# Find MSBuild
$MSBuild = & "$PSScriptRoot\Find-MSBuild.ps1"
if (-not $MSBuild) {
    Write-Host "ERROR: MSBuild not found" -ForegroundColor Red
    exit 1
}

$Solution = Join-Path $ProjectRoot "external\compression\lz4-1.10.0\build\VS2022\lz4.sln"

if (-not (Test-Path $Solution)) {
    Write-Host "ERROR: Solution file not found!" -ForegroundColor Red
    Write-Host "Expected: $Solution" -ForegroundColor Yellow
    exit 1
}

Write-Host "Solution: $Solution" -ForegroundColor White
Write-Host ""

# Build liblz4 project (static library)
$Project = Join-Path $ProjectRoot "external\compression\lz4-1.10.0\build\VS2022\liblz4\liblz4.vcxproj"

if (Test-Path $Project) {
    Write-Host "Building liblz4 static library..." -ForegroundColor White
    & $MSBuild $Project /t:Clean /p:Configuration=$Configuration /p:Platform=$Platform /v:minimal /nologo
    & $MSBuild $Project /t:Build /p:Configuration=$Configuration /p:Platform=$Platform /v:minimal /nologo
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host ""
        Write-Host "[FAILED] LZ4 build failed!" -ForegroundColor Red
        exit 1
    }
}

# Check for output (lib is built to liblz4\bin\ directory)
$OutputLib = Join-Path $ProjectRoot "external\compression\lz4-1.10.0\build\VS2022\liblz4\bin\$Platform`_$Configuration\liblz4_static.lib"

if (Test-Path $OutputLib) {
    $Size = (Get-Item $OutputLib).Length
    $SizeKB = [math]::Round($Size / 1KB, 1)
    Write-Host ""
    Write-Host "[SUCCESS] liblz4_static.lib built: $SizeKB KB ($Size bytes)" -ForegroundColor Green
    Write-Host ""
    Write-Host "Output: $OutputLib" -ForegroundColor White
    Write-Host ""
    Write-Host "Next Steps:" -ForegroundColor Yellow
    Write-Host "1. Copy liblz4_static.lib to external\compression\lz4\x64\Release\lz4.lib" -ForegroundColor Gray
    Write-Host "2. Update CBXShell.vcxproj AdditionalLibraryDirectories if needed" -ForegroundColor Gray
    exit 0
}
else {
    Write-Host ""
    Write-Host "[ERROR] Output file not found: $OutputLib" -ForegroundColor Red
    Write-Host "Searching for .lib files:" -ForegroundColor Yellow
    Get-ChildItem (Join-Path $ProjectRoot "external\compression\lz4-1.10.0\build\VS2022") -Recurse -Filter "*.lib" -ErrorAction SilentlyContinue | 
    Select-Object FullName, @{Name = 'Size(KB)'; Expression = { [math]::Round($_.Length / 1KB, 1) } } | 
    Format-Table -AutoSize
    exit 1
}
