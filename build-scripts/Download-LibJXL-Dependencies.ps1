# Download and extract libjxl third-party dependencies
# Required: Highway, Brotli, and skcms

param(
    [string]$LibjxlDir = (Join-Path (Split-Path -Parent $PSScriptRoot) "external\image-libs\libjxl-0.11.1")
)

$ErrorActionPreference = "Stop"

Write-Host "Downloading libjxl dependencies..." -ForegroundColor Cyan
Write-Host ""

$ThirdPartyDir = Join-Path $LibjxlDir "third_party"

if (-not (Test-Path $ThirdPartyDir)) {
    Write-Host "[ERROR] third_party directory not found at: $ThirdPartyDir" -ForegroundColor Red
    exit 1
}

# Dependencies to download
$Dependencies = @(
    @{
        Name          = "highway"
        Url           = "https://github.com/google/highway/archive/refs/tags/1.0.7.zip"
        ExtractFolder = "highway-1.0.7"
        TargetFolder  = "highway"
    },
    @{
        Name          = "brotli"
        Url           = "https://github.com/google/brotli/archive/refs/tags/v1.1.0.zip"
        ExtractFolder = "brotli-1.1.0"
        TargetFolder  = "brotli"
    },
    @{
        Name          = "skcms"
        Url           = "https://skia.googlesource.com/skcms/+archive/42030a771244ba67f86b1c1c76a6493f873c5f91.tar.gz"
        ExtractFolder = ""  # tar.gz extracts directly
        TargetFolder  = "skcms"
        IsTarGz       = $true
    }
)

# Create temp directory
$TempDir = Join-Path $env:TEMP "libjxl-deps"
if (Test-Path $TempDir) {
    Remove-Item -Recurse -Force $TempDir
}
New-Item -ItemType Directory -Path $TempDir | Out-Null

foreach ($dep in $Dependencies) {
    $TargetPath = Join-Path $ThirdPartyDir $dep.TargetFolder
    
    # Check if already exists
    if (Test-Path $TargetPath) {
        $fileCount = (Get-ChildItem -Path $TargetPath -Recurse -File | Measure-Object).Count
        if ($fileCount -gt 0) {
            Write-Host "[SKIP] $($dep.Name) already exists with $fileCount files" -ForegroundColor Yellow
            continue
        } else {
            Write-Host "[INFO] Removing empty $($dep.Name) directory" -ForegroundColor Yellow
            Remove-Item -Recurse -Force $TargetPath
        }
    }
    
    Write-Host "[Downloading] $($dep.Name)..." -ForegroundColor Cyan
    
    $isTarGz = $dep.IsTarGz -eq $true
    $archivePath = Join-Path $TempDir "$($dep.Name)$(if ($isTarGz) {'.tar.gz'} else {'.zip'})"
    
    try {
        # Download with progress
        $ProgressPreference = 'SilentlyContinue'
        Invoke-WebRequest -Uri $dep.Url -OutFile $archivePath -UseBasicParsing
        $ProgressPreference = 'Continue'
        
        Write-Host "  Downloaded: $('{0:N2}' -f ((Get-Item $archivePath).Length / 1MB)) MB" -ForegroundColor Green
        
        # Extract
        Write-Host "  Extracting..." -ForegroundColor Cyan
        
        if ($isTarGz) {
            # Handle tar.gz - extract directly to target
            New-Item -ItemType Directory -Path $TargetPath -Force | Out-Null
            tar -xzf $archivePath -C $TargetPath
            Write-Host "[OK] $($dep.Name) installed" -ForegroundColor Green
        } else {
            # Handle zip
            $ExtractPath = Join-Path $TempDir $dep.Name
            Expand-Archive -Path $archivePath -DestinationPath $ExtractPath -Force
            
            # Move to target location
            $SourcePath = Join-Path $ExtractPath $dep.ExtractFolder
            if (Test-Path $SourcePath) {
                Write-Host "  Moving to: $TargetPath" -ForegroundColor Cyan
                Move-Item -Path $SourcePath -Destination $TargetPath -Force
                Write-Host "[OK] $($dep.Name) installed" -ForegroundColor Green
            } else {
                Write-Host "[ERROR] Extract folder not found: $SourcePath" -ForegroundColor Red
            }
        }
        
    } catch {
        Write-Host "[ERROR] Failed to download/extract $($dep.Name): $($_.Exception.Message)" -ForegroundColor Red
    }
    
    Write-Host ""
}

# Cleanup
if (Test-Path $TempDir) {
    Remove-Item -Recurse -Force $TempDir
}

# Verify installation
Write-Host "Verifying installation..." -ForegroundColor Cyan
$AllOk = $true

foreach ($dep in $Dependencies) {
    $TargetPath = Join-Path $ThirdPartyDir $dep.TargetFolder
    if (Test-Path $TargetPath) {
        $fileCount = (Get-ChildItem -Path $TargetPath -Recurse -File | Measure-Object).Count
        if ($fileCount -gt 0) {
            Write-Host "[OK] $($dep.Name): $fileCount files" -ForegroundColor Green
        } else {
            Write-Host "[FAIL] $($dep.Name): empty directory" -ForegroundColor Red
            $AllOk = $false
        }
    } else {
        Write-Host "[FAIL] $($dep.Name): not found" -ForegroundColor Red
        $AllOk = $false
    }
}

Write-Host ""
if ($AllOk) {
    Write-Host "[SUCCESS] All dependencies installed!" -ForegroundColor Green
    Write-Host "You can now build libjxl with: .\build-scripts\build-libjxl.ps1" -ForegroundColor Cyan
} else {
    Write-Host "[WARNING] Some dependencies are missing" -ForegroundColor Yellow
    exit 1
}
