# ===========================================================================
# Install-DarkThumbs.ps1
# DarkThumbs - 64-bit Installation Script
# Run as Administrator
# ===========================================================================

#Requires -RunAsAdministrator

param(
    [string]$SourcePath = ".",
    [string]$InstallDir = "$env:ProgramFiles\DarkThumbs"
)

$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host "DarkThumbs 64-bit Installation" -ForegroundColor Cyan
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host ""

# Navigate to source path
Set-Location $SourcePath

# Check if build exists
if (-not (Test-Path "x64\Release\CBXShell.dll")) {
    Write-Host "[ERROR] Build not found!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please rebuild the project first." -ForegroundColor Yellow
    Write-Host ""
    Read-Host "Press Enter to exit"
    exit 1
}

# Check and copy UnRAR64.dll if missing from Release
if (-not (Test-Path "x64\Release\UnRAR64.dll")) {
    Write-Host "[INFO] UnRAR64.dll not in Release directory, copying..." -ForegroundColor Yellow
    if (Test-Path "CBXShell\UnRAR64.dll") {
        Copy-Item "CBXShell\UnRAR64.dll" "x64\Release\" -Force
        Write-Host "  UnRAR64.dll copied to Release directory" -ForegroundColor Green
    }
    else {
        Write-Host "  [WARNING] UnRAR64.dll not found" -ForegroundColor Yellow
        Write-Host "  RAR archive support may not work" -ForegroundColor Yellow
    }
    Write-Host ""
}

Write-Host "[1/8] Stopping Windows Explorer..." -ForegroundColor Green
Stop-Process -Name explorer -Force -ErrorAction SilentlyContinue
Start-Sleep -Seconds 2
Write-Host "  Explorer stopped" -ForegroundColor White

Write-Host ""
Write-Host "[2/8] Unregistering previous version (if any)..." -ForegroundColor Green
Start-Process -FilePath "regsvr32" -ArgumentList "/s", "/u", "`"$InstallDir\CBXShell.dll`"" -Wait -NoNewWindow -ErrorAction SilentlyContinue
Start-Process -FilePath "regsvr32" -ArgumentList "/s", "/u", "x64\Release\CBXShell.dll" -Wait -NoNewWindow -ErrorAction SilentlyContinue
if (Test-Path "$InstallDir\CBXShell.dll") {
    Write-Host "  Previous version unregistered" -ForegroundColor White
}
else {
    Write-Host "  No previous version found" -ForegroundColor White
}

Write-Host ""
Write-Host "[3/8] Creating installation directory..." -ForegroundColor Green
if (-not (Test-Path $InstallDir)) {
    New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
    Write-Host "  Created: $InstallDir" -ForegroundColor White
}
else {
    Write-Host "  Directory exists: $InstallDir" -ForegroundColor White
}

Write-Host ""
Write-Host "[4/8] Copying files to installation directory..." -ForegroundColor Green

# Copy main DLL
Copy-Item "x64\Release\CBXShell.dll" "$InstallDir\" -Force
if ($?) {
    Write-Host "  CBXShell.dll installed" -ForegroundColor White
}
else {
    Write-Host "  [ERROR] Failed to copy CBXShell.dll" -ForegroundColor Red
    Start-Process explorer.exe
    Read-Host "Press Enter to exit"
    exit 1
}

# Copy UnRAR dependency
if (Test-Path "x64\Release\UnRAR64.dll") {
    Copy-Item "x64\Release\UnRAR64.dll" "$InstallDir\" -Force
    if ($?) {
        Write-Host "  UnRAR64.dll installed" -ForegroundColor White
    }
}

# Copy configuration utility
if (Test-Path "x64\Release\CBXManager.exe") {
    Copy-Item "x64\Release\CBXManager.exe" "$InstallDir\" -Force
    if ($?) {
        Write-Host "  CBXManager.exe installed" -ForegroundColor White
    }
}

Write-Host ""
Write-Host "[5/8] Registering CBXShell.dll (64-bit COM Server)..." -ForegroundColor Green
Push-Location $InstallDir

# Verify dependencies
if (-not (Test-Path "CBXShell.dll")) {
    Write-Host "  [ERROR] CBXShell.dll not found in install directory!" -ForegroundColor Red
    Pop-Location
    Start-Process explorer.exe
    Read-Host "Press Enter to exit"
    exit 1
}

if (-not (Test-Path "UnRAR64.dll")) {
    Write-Host "  [WARNING] UnRAR64.dll not found - RAR support will not work" -ForegroundColor Yellow
}

# Register the DLL
$RegProcess = Start-Process -FilePath "regsvr32" -ArgumentList "/s", "CBXShell.dll" -Wait -PassThru -NoNewWindow
if ($RegProcess.ExitCode -eq 0) {
    Write-Host "  CBXShell.dll registered successfully" -ForegroundColor White
}
else {
    Write-Host "  [ERROR] DLL registration failed!" -ForegroundColor Red
    Write-Host ""
    Write-Host "  Trying with full path..." -ForegroundColor Yellow
    Start-Process -FilePath "regsvr32" -ArgumentList "`"$InstallDir\CBXShell.dll`"" -Wait -NoNewWindow
    Write-Host ""
    Pop-Location
    Start-Process explorer.exe
    Read-Host "Press Enter to exit"
    exit 1
}
Pop-Location

Write-Host ""
Write-Host "[6/8] Clearing thumbnail cache..." -ForegroundColor Green
$ThumbCachePath = "$env:LOCALAPPDATA\Microsoft\Windows\Explorer\thumbcache_*.db"
Remove-Item $ThumbCachePath -Force -ErrorAction SilentlyContinue
Write-Host "  Thumbnail cache cleared" -ForegroundColor White

Write-Host ""
Write-Host "[7/8] Configuring file associations..." -ForegroundColor Green

# Helper function to set registry
function Set-FileAssociation {
    param(
        [string]$Extension,
        [string]$ProgId,
        [string]$Description
    )
    
    $CLSID = "{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}"
    
    New-Item -Path "Registry::HKEY_CLASSES_ROOT\$Extension" -Force | Out-Null
    Set-ItemProperty -Path "Registry::HKEY_CLASSES_ROOT\$Extension" -Name "(Default)" -Value $ProgId
    
    New-Item -Path "Registry::HKEY_CLASSES_ROOT\$ProgId" -Force | Out-Null
    Set-ItemProperty -Path "Registry::HKEY_CLASSES_ROOT\$ProgId" -Name "(Default)" -Value $Description
    
    New-Item -Path "Registry::HKEY_CLASSES_ROOT\$ProgId\shellex\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}" -Force | Out-Null
    Set-ItemProperty -Path "Registry::HKEY_CLASSES_ROOT\$ProgId\shellex\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}" -Name "(Default)" -Value $CLSID
}

# Register formats
Set-FileAssociation ".webp" "DarkThumbs.WebP" "WebP Image"
Set-FileAssociation ".heif" "DarkThumbs.HEIF" "HEIF Image"
Set-FileAssociation ".heic" "DarkThumbs.HEIC" "HEIC Image"
Set-FileAssociation ".avif" "DarkThumbs.AVIF" "AVIF Image"
Set-FileAssociation ".cbz" "DarkThumbs.CBZ" "Comic Book Archive (ZIP)"
Set-FileAssociation ".cbr" "DarkThumbs.CBR" "Comic Book Archive (RAR)"
Set-FileAssociation ".cb7" "DarkThumbs.CB7" "Comic Book Archive (7-Zip)"

Write-Host "  File associations configured" -ForegroundColor White

Write-Host ""
Write-Host "[8/8] Restarting Windows Explorer..." -ForegroundColor Green
Start-Process explorer.exe
Write-Host "  Explorer restarted" -ForegroundColor White

Write-Host ""
Write-Host "==========================================================================" -ForegroundColor Green
Write-Host "Installation Complete!" -ForegroundColor Green
Write-Host "==========================================================================" -ForegroundColor Green
Write-Host ""
Write-Host "Installation Directory: $InstallDir" -ForegroundColor White
Write-Host ""
Write-Host "Supported Formats:" -ForegroundColor Cyan
Write-Host "  Sprint 1 - WebP Images: .webp" -ForegroundColor White
Write-Host "  Sprint 2 - HEIF Images: .heif, .heic" -ForegroundColor White
Write-Host "  Archives: .zip, .rar, .7z, .cbz, .cbr, .cb7" -ForegroundColor White
Write-Host ""
Write-Host "Configuration:" -ForegroundColor Cyan
Write-Host "  Run: $InstallDir\CBXManager.exe" -ForegroundColor White
Write-Host ""
Write-Host "Testing:" -ForegroundColor Cyan
Write-Host "  1. Download sample images (.webp, .heif, .heic)" -ForegroundColor White
Write-Host "  2. View in Windows Explorer (Large/Extra Large Icons)" -ForegroundColor White
Write-Host "  3. Thumbnails should appear automatically" -ForegroundColor White
Write-Host ""
Write-Host "Troubleshooting:" -ForegroundColor Cyan
Write-Host "  - If thumbnails don't appear, restart your PC" -ForegroundColor White
Write-Host "  - Clear cache: Remove-Item `$env:LOCALAPPDATA\Microsoft\Windows\Explorer\thumbcache_*.db" -ForegroundColor White
Write-Host "  - Rebuild icon cache: ie4uinit.exe -show" -ForegroundColor White
Write-Host ""
Write-Host "==========================================================================" -ForegroundColor Green
Write-Host ""

Read-Host "Press Enter to exit"
