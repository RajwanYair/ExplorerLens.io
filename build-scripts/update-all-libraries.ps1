# DarkThumbs Library Update Script
# Updates all external open-source libraries to latest stable versions
# Dynamically fetches latest versions from GitHub and other sources
# Date: January 8, 2026

param(
    [switch]$UseProxy,
    [string]$ProxyUrl = "http://proxy-chain.intel.com:928"
)

$ErrorActionPreference = "Stop"

# Configure proxy if requested
if ($UseProxy) {
    Write-Host "Configuring Intel proxy: $ProxyUrl" -ForegroundColor Cyan
    $env:HTTP_PROXY = $ProxyUrl
    $env:HTTPS_PROXY = $ProxyUrl
    [System.Net.WebRequest]::DefaultWebProxy = New-Object System.Net.WebProxy($ProxyUrl)
}

# Helper function to get latest GitHub release version
function Get-LatestGitHubRelease {
    param([string]$Repo)
    
    try {
        $apiUrl = "https://api.github.com/repos/$Repo/releases/latest"
        $headers = @{ "User-Agent" = "DarkThumbs-Updater" }
        
        if ($UseProxy) {
            $response = Invoke-RestMethod -Uri $apiUrl -Headers $headers -Proxy $ProxyUrl -UseBasicParsing
        } else {
            $response = Invoke-RestMethod -Uri $apiUrl -Headers $headers -UseBasicParsing
        }
        
        return $response.tag_name -replace '^v', ''
    } catch {
        Write-Host "  Warning: Could not fetch latest version for $Repo" -ForegroundColor Yellow
        return $null
    }
}

# Helper function to get latest 7-Zip/LZMA version
function Get-LatestLZMAVersion {
    try {
        $url = "https://www.7-zip.org/download.html"
        if ($UseProxy) {
            $content = Invoke-WebRequest -Uri $url -Proxy $ProxyUrl -UseBasicParsing
        } else {
            $content = Invoke-WebRequest -Uri $url -UseBasicParsing
        }
        
        if ($content.Content -match 'lzma(\d{4})\.7z') {
            $version = $matches[1]
            return "$($version.Substring(0,2)).$($version.Substring(2,2))"
        }
    } catch {
        Write-Host "  Warning: Could not fetch latest LZMA version" -ForegroundColor Yellow
    }
    return $null
}

# Base directory
$rootDir = Split-Path -Parent $PSScriptRoot
$externalDir = Join-Path $rootDir "external"
$compressionDir = Join-Path $externalDir "compression"
$imageLibsDir = Join-Path $externalDir "image-libs"

# Create directories if they don't exist
New-Item -ItemType Directory -Force -Path $compressionDir | Out-Null
New-Item -ItemType Directory -Force -Path $imageLibsDir | Out-Null

Write-Host "`n=== DarkThumbs Library Update Utility ===" -ForegroundColor Green
Write-Host "Fetching latest versions from sources...`n" -ForegroundColor Cyan

# Fetch latest versions dynamically
$latestZlib = Get-LatestGitHubRelease "madler/zlib"
$latestZstd = Get-LatestGitHubRelease "facebook/zstd"
$latestLz4 = Get-LatestGitHubRelease "lz4/lz4"
$latestMinizipNG = Get-LatestGitHubRelease "zlib-ng/minizip-ng"
$latestLibWebP = Get-LatestGitHubRelease "webmproject/libwebp"
$latestLibAvif = Get-LatestGitHubRelease "AOMediaCodec/libavif"
$latestLibJxl = Get-LatestGitHubRelease "libjxl/libjxl"
$latestDav1d = Get-LatestGitHubRelease "videolan/dav1d"
$latestLZMA = Get-LatestLZMAVersion

# Library version tracking - dynamically populated
$libraries = @{
    # Compression libraries
    "zlib"       = @{
        "Current"  = "1.3.1"
        "Latest"   = if ($latestZlib) { $latestZlib } else { "1.3.1" }
        "Repo"     = "madler/zlib"
        "Category" = "Compression"
    }
    "zstd"       = @{
        "Current"  = "1.5.7"
        "Latest"   = if ($latestZstd) { $latestZstd } else { "1.5.7" }
        "Repo"     = "facebook/zstd"
        "Category" = "Compression"
    }
    "lz4"        = @{
        "Current"  = "1.10.0"
        "Latest"   = if ($latestLz4) { $latestLz4 } else { "1.10.0" }
        "Repo"     = "lz4/lz4"
        "Category" = "Compression"
    }
    "minizip-ng" = @{
        "Current"  = "4.0.10"
        "Latest"   = if ($latestMinizipNG) { $latestMinizipNG } else { "4.0.10" }
        "Repo"     = "zlib-ng/minizip-ng"
        "Category" = "Compression"
    }
    "lzma"       = @{
        "Current"  = "24.08"
        "Latest"   = if ($latestLZMA) { $latestLZMA } else { "26.00" }
        "Url"      = if ($latestLZMA) { "https://www.7-zip.org/a/lzma$($latestLZMA.Replace('.', '')).7z" } else { "https://www.7-zip.org/a/lzma2408.7z" }
        "Category" = "Compression"
    }
    # Image libraries
    "libwebp"    = @{
        "Current"  = "1.5.0"
        "Latest"   = if ($latestLibWebP) { $latestLibWebP } else { "1.5.0" }
        "Repo"     = "webmproject/libwebp"
        "Category" = "Image"
    }
    "libavif"    = @{
        "Current"  = "1.1.1"
        "Latest"   = if ($latestLibAvif) { $latestLibAvif } else { "1.1.1" }
        "Repo"     = "AOMediaCodec/libavif"
        "Category" = "Image"
    }
    "libjxl"     = @{
        "Current"  = "0.11.0"
        "Latest"   = if ($latestLibJxl) { $latestLibJxl } else { "0.11.0" }
        "Repo"     = "libjxl/libjxl"
        "Category" = "Image"
    }
    "dav1d"      = @{
        "Current"  = "1.5.0"
        "Latest"   = if ($latestDav1d) { $latestDav1d } else { "1.5.0" }
        "Repo"     = "videolan/dav1d"
        "Category" = "Image/Video"
    }
}

# Build download URLs and target directories dynamically
foreach ($lib in $libraries.Keys) {
    $info = $libraries[$lib]
    $version = $info.Latest
    
    if (-not $info.ContainsKey("Url")) {
        $repo = $info.Repo
        
        # Build URL based on library patterns
        switch ($lib) {
            "zlib" { 
                $info["Url"] = "https://github.com/$repo/releases/download/v$version/zlib-$version.tar.gz"
                $info["Dir"] = Join-Path $compressionDir "zlib-$version"
            }
            "zstd" { 
                $info["Url"] = "https://github.com/$repo/releases/download/v$version/zstd-$version.tar.gz"
                $info["Dir"] = Join-Path $compressionDir "zstd-$version"
            }
            "lz4" { 
                $info["Url"] = "https://github.com/$repo/releases/download/v$version/lz4-$version.tar.gz"
                $info["Dir"] = Join-Path $compressionDir "lz4-$version"
            }
            "minizip-ng" { 
                $info["Url"] = "https://github.com/$repo/archive/refs/tags/$version.tar.gz"
                $info["Dir"] = Join-Path $compressionDir "minizip-ng-$version"
            }
            "libwebp" { 
                $info["Url"] = "https://github.com/$repo/archive/refs/tags/v$version.tar.gz"
                $info["Dir"] = Join-Path $imageLibsDir "libwebp-$version"
            }
            "libavif" { 
                $info["Url"] = "https://github.com/$repo/archive/refs/tags/v$version.tar.gz"
                $info["Dir"] = Join-Path $imageLibsDir "libavif-$version"
            }
            "libjxl" { 
                $info["Url"] = "https://github.com/$repo/archive/refs/tags/v$version.tar.gz"
                $info["Dir"] = Join-Path $imageLibsDir "libjxl-$version"
            }
            "dav1d" { 
                $info["Url"] = "https://github.com/$repo/archive/refs/tags/$version.tar.gz"
                $info["Dir"] = Join-Path $imageLibsDir "dav1d-$version"
            }
        }
    } else {
        # LZMA has custom URL already set above
        $info["Dir"] = Join-Path $compressionDir "lzma-$version"
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
        } elseif ($fileName -match '\.7z$') {
            # Requires 7-Zip
            if (Test-Path "C:\Program Files\7-Zip\7z.exe") {
                & "C:\Program Files\7-Zip\7z.exe" x $downloadPath -o"$TargetDir" -y
            } else {
                Write-Host "  WARNING: 7-Zip not found. Please extract manually: $downloadPath" -ForegroundColor Red
                return $false
            }
        } else {
            Expand-Archive -Path $downloadPath -DestinationPath (Split-Path $TargetDir -Parent) -Force
        }
        
        Write-Host "  OK: $Name" -ForegroundColor Green
        Remove-Item $downloadPath -Force -ErrorAction SilentlyContinue
        return $true
    } catch {
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
Write-Host "  1. Review the extracted library versions in external/ directory" -ForegroundColor White
Write-Host "  2. Update build scripts if paths changed (e.g., zstd-1.5.7 vs zstd-1.5.8)" -ForegroundColor White
Write-Host "  3. Run .\scripts\build.ps1 to rebuild DarkThumbs with updated libraries" -ForegroundColor White
Write-Host "  4. Test thoroughly before committing changes" -ForegroundColor White
Write-Host "`nNote: Current version tracking in this script should be updated after successful build" -ForegroundColor Yellow

