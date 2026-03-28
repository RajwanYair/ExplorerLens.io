# ExplorerLens - Download Remaining Library Updates
# Downloads zstd 1.5.7 and unrar 7.2.2
# Date: November 19, 2025

param(
    [string]$ProxyUrl = ""
)

$ErrorActionPreference = "Stop"

Write-Host "`n=== Downloading Remaining Library Updates ===" -ForegroundColor Green

# Setup proxy
$env:HTTP_PROXY = $ProxyUrl
$env:HTTPS_PROXY = $ProxyUrl
[System.Net.WebRequest]::DefaultWebProxy = New-Object System.Net.WebProxy($ProxyUrl)

$rootDir = Split-Path -Parent $PSScriptRoot
$compressionDir = Join-Path $rootDir "external\compression-libs"

# Download zstd 1.5.7
Write-Host "`nDownloading zstd 1.5.7..." -ForegroundColor Cyan
$zstdUrl = "https://github.com/facebook/zstd/releases/download/v1.5.7/zstd-1.5.7.tar.gz"
$zstdFile = Join-Path $env:TEMP "zstd-1.5.7.tar.gz"
$zstdDir = Join-Path $compressionDir "zstd-1.5.7"

if (Test-Path $zstdDir) {
    Write-Host "  zstd 1.5.7 already exists" -ForegroundColor Yellow
} else {
    try {
        Invoke-WebRequest -Uri $zstdUrl -OutFile $zstdFile -UseBasicParsing
        Write-Host "  Downloaded: $zstdFile" -ForegroundColor Green
        
        Write-Host "  Extracting..." -ForegroundColor Cyan
        tar -xzf $zstdFile -C $compressionDir
        Write-Host "  Extracted to: $zstdDir" -ForegroundColor Green
        
        Remove-Item $zstdFile -Force
    } catch {
        Write-Host "  ERROR: $($_.Exception.Message)" -ForegroundColor Red
    }
}

# Download unrar 7.2.2
Write-Host "`nDownloading unrar 7.2.2..." -ForegroundColor Cyan
$unrarUrl = "https://www.rarlab.com/rar/unrarsrc-7.2.2.tar.gz"
$unrarFile = Join-Path $env:TEMP "unrarsrc-7.2.2.tar.gz"
$unrarDir = Join-Path $compressionDir "unrar-7.2.2"

if (Test-Path $unrarDir) {
    Write-Host "  unrar 7.2.2 already exists" -ForegroundColor Yellow
} else {
    try {
        Invoke-WebRequest -Uri $unrarUrl -OutFile $unrarFile -UseBasicParsing
        Write-Host "  Downloaded: $unrarFile" -ForegroundColor Green
        
        Write-Host "  Extracting..." -ForegroundColor Cyan
        tar -xzf $unrarFile -C $compressionDir
        
        # Rename extracted directory
        $extractedDir = Join-Path $compressionDir "unrar"
        if (Test-Path $extractedDir) {
            Move-Item $extractedDir $unrarDir -Force
        }
        
        Write-Host "  Extracted to: $unrarDir" -ForegroundColor Green
        Remove-Item $unrarFile -Force
    } catch {
        Write-Host "  ERROR: $($_.Exception.Message)" -ForegroundColor Red
    }
}

Write-Host "`n=== Download Complete ===" -ForegroundColor Green
Write-Host "Next: Build the updated libraries" -ForegroundColor Cyan

