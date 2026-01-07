# DarkThumbs Library Update Script
# Updates all external open-source libraries to latest stable versions
# Date: November 19, 2025

param(
    [switch]$UseProxy,
    [string]$ProxyUrl = "http://proxy-chain.intel.com:911"
)

$ErrorActionPreference = "Stop"

# Configure proxy if requested
if ($UseProxy) {
    Write-Host "Configuring Intel proxy: $ProxyUrl" -ForegroundColor Cyan
    $env:HTTP_PROXY = $ProxyUrl
    $env:HTTPS_PROXY = $ProxyUrl
    [System.Net.WebRequest]::DefaultWebProxy = New-Object System.Net.WebProxy($ProxyUrl)
}

# Base directory
$rootDir = Split-Path -Parent $PSScriptRoot
$externalDir = Join-Path $rootDir "external"
$compressionDir = Join-Path $externalDir "compression"

# Create directories if they don't exist
New-Item -ItemType Directory -Force -Path $compressionDir | Out-Null

Write-Host "`n=== DarkThumbs Library Update Utility ===" -ForegroundColor Green
Write-Host "Checking for latest versions of all libraries...`n" -ForegroundColor Cyan

# Library version tracking
$libraries = @{
    # Compression libraries
    "zlib" = @{
        "Current" = "1.3.1"
        "Latest" = "1.3.1"
        "Url" = "https://github.com/madler/zlib/releases/download/v1.3.1/zlib-1.3.1.tar.gz"
        "Dir" = Join-Path $compressionDir "zlib-1.3.1"
        "Category" = "Compression"
    }
    "bzip2" = @{
        "Current" = "1.0.8"
        "Latest" = "1.0.8"
        "Url" = "https://sourceware.org/pub/bzip2/bzip2-1.0.8.tar.gz"
        "Dir" = Join-Path $compressionDir "bzip2-1.0.8"
        "Category" = "Compression"
    }
    "zstd" = @{
        "Current" = "1.5.6"
        "Latest" = "1.5.7"
        "Url" = "https://github.com/facebook/zstd/releases/download/v1.5.7/zstd-1.5.7.tar.gz"
        "Dir" = Join-Path $compressionDir "zstd-1.5.7"
        "Category" = "Compression"
    }
    "lz4" = @{
        "Current" = "1.10.0"
        "Latest" = "1.10.0"
        "Url" = "https://github.com/lz4/lz4/releases/download/v1.10.0/lz4-1.10.0.tar.gz"
        "Dir" = Join-Path $compressionDir "lz4-1.10.0"
        "Category" = "Compression"
    }
    "lzma" = @{
        "Current" = "24.07"
        "Latest" = "24.08"
        "Url" = "https://www.7-zip.org/a/lzma2408.7z"
        "Dir" = Join-Path $compressionDir "lzma-24.08"
        "Category" = "Compression"
    }
    "minizip-ng" = @{
        "Current" = "4.0.7"
        "Latest" = "4.0.7"
        "Url" = "https://github.com/zlib-ng/minizip-ng/archive/refs/tags/4.0.7.tar.gz"
        "Dir" = Join-Path $compressionDir "minizip-ng-4.0.7"
        "Category" = "Compression"
    }
    "unrar" = @{
        "Current" = "7.2.1"
        "Latest" = "7.2.2"
        "Url" = "https://www.rarlab.com/rar/unrarsrc-7.2.2.tar.gz"
        "Dir" = Join-Path $compressionDir "unrar-7.2.2"
        "Category" = "Compression"
    }
    # Image libraries
    "libwebp" = @{
        "Current" = "1.4.0"
        "Latest" = "1.5.0"
        "Url" = "https://github.com/webmproject/libwebp/archive/refs/tags/v1.5.0.tar.gz"
        "Dir" = Join-Path $externalDir "libwebp-1.5.0"
        "Category" = "Image"
    }
}

# Display current vs latest
Write-Host "Library Update Summary:" -ForegroundColor Yellow
Write-Host ("=" * 80) -ForegroundColor Gray
Write-Host ("{0,-20} {1,-12} {2,-12} {3,-10} {4}" -f "Library", "Current", "Latest", "Status", "Category") -ForegroundColor Cyan
Write-Host ("-" * 80) -ForegroundColor Gray

$updateCount = 0
foreach ($lib in $libraries.Keys | Sort-Object) {
    $info = $libraries[$lib]
    $status = if ($info.Current -eq $info.Latest) { "OK" } else { "UPDATE!" }
    $color = if ($status -eq "OK") { "Green" } else { "Yellow" }
    
    Write-Host ("{0,-20} {1,-12} {2,-12} {3,-10} {4}" -f `
        $lib, $info.Current, $info.Latest, $status, $info.Category) -ForegroundColor $color
    
    if ($status -eq "UPDATE!") {
        $updateCount++
    }
}

Write-Host ("-" * 80) -ForegroundColor Gray
Write-Host "Total libraries needing update: $updateCount" -ForegroundColor $(if ($updateCount -gt 0) { "Yellow" } else { "Green" })

# Ask user if they want to proceed with updates
if ($updateCount -eq 0) {
    Write-Host "`nAll libraries are up to date!" -ForegroundColor Green
    exit 0
}

Write-Host "`nDo you want to download and extract updated libraries? (Y/N): " -ForegroundColor Yellow -NoNewline
$response = Read-Host

if ($response -ne 'Y' -and $response -ne 'y') {
    Write-Host "Update cancelled." -ForegroundColor Red
    exit 0
}

# Download and extract updates
Write-Host "`nDownloading updated libraries..." -ForegroundColor Green

function Download-And-Extract {
    param(
        [string]$Name,
        [string]$Url,
        [string]$TargetDir
    )
    
    $fileName = Split-Path $Url -Leaf
    $downloadPath = Join-Path $env:TEMP $fileName
    
    try {
        Write-Host "  Downloading $Name..." -ForegroundColor Cyan
        
        if ($UseProxy) {
            Invoke-WebRequest -Uri $Url -OutFile $downloadPath -Proxy $ProxyUrl -UseBasicParsing
        } else {
            Invoke-WebRequest -Uri $Url -OutFile $downloadPath -UseBasicParsing
        }
        
        Write-Host "  Extracting to $TargetDir..." -ForegroundColor Cyan
        
        # Determine extraction method
        if ($fileName -match '\.tar\.gz$') {
            # Use tar (available in Windows 10+)
            $parentDir = Split-Path $TargetDir -Parent
            New-Item -ItemType Directory -Force -Path $parentDir | Out-Null
            tar -xzf $downloadPath -C $parentDir
        }
        elseif ($fileName -match '\.7z$') {
            # Requires 7-Zip
            if (Test-Path "C:\Program Files\7-Zip\7z.exe") {
                & "C:\Program Files\7-Zip\7z.exe" x $downloadPath -o"$TargetDir" -y
            } else {
                Write-Host "  WARNING: 7-Zip not found. Please extract manually: $downloadPath" -ForegroundColor Red
                return $false
            }
        }
        else {
            Expand-Archive -Path $downloadPath -DestinationPath (Split-Path $TargetDir -Parent) -Force
        }
        
        Write-Host "  OK: $Name" -ForegroundColor Green
        Remove-Item $downloadPath -Force -ErrorAction SilentlyContinue
        return $true
    }
    catch {
        Write-Host "  ERROR: Failed to download/extract $Name - $($_.Exception.Message)" -ForegroundColor Red
        return $false
    }
}

# Process updates
foreach ($lib in $libraries.Keys | Sort-Object) {
    $info = $libraries[$lib]
    
    if ($info.Current -ne $info.Latest) {
        Write-Host "`nUpdating $lib ($($info.Current) -> $($info.Latest))..." -ForegroundColor Yellow
        
        if (Download-And-Extract -Name $lib -Url $info.Url -TargetDir $info.Dir) {
            Write-Host "  Successfully updated $lib to $($info.Latest)" -ForegroundColor Green
        }
    }
}

Write-Host "`n=== Library Update Complete ===" -ForegroundColor Green
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "  1. Run rebuild-compression-libs.cmd to rebuild updated libraries" -ForegroundColor White
Write-Host "  2. Run rebuild-all.cmd to rebuild DarkThumbs with updated libraries" -ForegroundColor White
Write-Host "  3. Test thoroughly before committing changes" -ForegroundColor White
