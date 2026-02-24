# Build ExplorerLens Installer Package
# Sprint 23: Packaging & Deployment
# Requires: WiX Toolset v4.0+ or v5.0+

param(
    [string]$Configuration = "Release",
    [string]$Version = "7.0.0"
)

$ErrorActionPreference = "Stop"

Write-Host "============================================" -ForegroundColor Cyan
Write-Host "ExplorerLens Installer Build Script" -ForegroundColor Cyan
Write-Host "Sprint 23: MSI Package Creation" -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RootDir = Split-Path -Parent $ScriptDir
$PackagingDir = Join-Path $RootDir "packaging"
$OutputDir = Join-Path $PackagingDir "output"

# Verify WiX installation
Write-Host "[1/5] Checking for WiX Toolset..." -ForegroundColor Yellow
$WixBuild = Get-Command wix -ErrorAction SilentlyContinue
if (-not $WixBuild) {
    Write-Host "[ERROR] WiX Toolset not found in PATH" -ForegroundColor Red
    Write-Host "Install from: https://wixtoolset.org/releases/" -ForegroundColor Yellow
    exit 1
}

$WixVersion = & wix --version
Write-Host "[OK] Found WiX version: $WixVersion" -ForegroundColor Green
Write-Host ""

# Verify all binaries are built
Write-Host "[2/5] Verifying build artifacts..." -ForegroundColor Yellow

$BinariesToCheck = @(
    "x64\$Configuration\LENSShell.dll",
    "x64\$Configuration\LENSManager.exe"
)

$AllExist = $true
foreach ($binary in $BinariesToCheck) {
    $FullPath = Join-Path $RootDir $binary
    if (Test-Path $FullPath) {
        $FileInfo = Get-Item $FullPath
        Write-Host "  [OK] $binary ($([math]::Round($FileInfo.Length / 1MB, 2)) MB)" -ForegroundColor Green
    } else {
        Write-Host "  [MISSING] $binary" -ForegroundColor Red
        $AllExist = $false
    }
}

if (-not $AllExist) {
    Write-Host ""
    Write-Host "[ERROR] One or more binaries missing. Build the solution first:" -ForegroundColor Red
    Write-Host "  msbuild LENSShell.sln /p:Configuration=$Configuration /p:Platform=x64 /m" -ForegroundColor Yellow
    exit 1
}
Write-Host ""

# Create output directory
Write-Host "[3/5] Preparing output directory..." -ForegroundColor Yellow
if (Test-Path $OutputDir) {
    Remove-Item -Recurse -Force $OutputDir
}
New-Item -ItemType Directory -Path $OutputDir | Out-Null
Write-Host "[OK] Output directory: $OutputDir" -ForegroundColor Green
Write-Host ""

# Build MSI package
Write-Host "[4/5] Building MSI installer..." -ForegroundColor Yellow
$WxsFile = Join-Path $PackagingDir "ExplorerLens.wxs"
$MsiFile = Join-Path $OutputDir "ExplorerLens-Setup-$Version.msi"

Write-Host "  Source: $WxsFile" -ForegroundColor Gray
Write-Host "  Target: $MsiFile" -ForegroundColor Gray

try {
    & wix build `
        "$WxsFile" `
        -out "$MsiFile" `
        -define "BuildDir=$RootDir" `
        -define "Version=$Version" `
        -arch x64
    
    if ($LASTEXITCODE -ne 0) {
        throw "WiX build failed with exit code $LASTEXITCODE"
    }
    
    Write-Host "[OK] MSI package created successfully" -ForegroundColor Green
} catch {
    Write-Host "[ERROR] MSI build failed: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}
Write-Host ""

# Verify output
Write-Host "[5/5] Verifying installer package..." -ForegroundColor Yellow
if (Test-Path $MsiFile) {
    $MsiInfo = Get-Item $MsiFile
    Write-Host "[OK] Installer: $MsiFile" -ForegroundColor Green
    Write-Host "     Size: $([math]::Round($MsiInfo.Length / 1MB, 2)) MB" -ForegroundColor Green
    Write-Host "     Created: $($MsiInfo.CreationTime)" -ForegroundColor Green
} else {
    Write-Host "[ERROR] MSI file not found after build" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "============================================" -ForegroundColor Cyan
Write-Host "Build Complete!" -ForegroundColor Green
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "  1. Test installer: msiexec /i `"$MsiFile`" /l*v install.log" -ForegroundColor Gray
Write-Host "  2. Uninstall test: msiexec /x `"$MsiFile`" /l*v uninstall.log" -ForegroundColor Gray
Write-Host "  3. Code signing:   signtool sign /sha1 <thumbprint> /t http://timestamp.digicert.com `"$MsiFile`"" -ForegroundColor Gray
Write-Host ""

