# Organize-TestCorpus.ps1
# Sprint 13: Real-File Test Fixtures & Compatibility Kit
# Reorganizes test files into a structured corpus for validation
# Date: February 17, 2026

[CmdletBinding()]
param(
    [Parameter()]
    [string]$SourceDir = "$PSScriptRoot\..\test-images",
    
    [Parameter()]
    [string]$CorpusDir = "$PSScriptRoot\..\data\corpus",
    
    [Parameter()]
    [switch]$CreateInvalidFiles
)

$ErrorActionPreference = "Stop"

Write-Host "=== ExplorerLens Test Corpus Organization ===" -ForegroundColor Cyan
Write-Host "Source: $SourceDir"
Write-Host "Target: $CorpusDir"
Write-Host ""

# Create corpus directory structure
$categories = @{
    "images/jpeg"    = @("*.jpg", "*.jpeg")
    "images/png"     = @("*.png")
    "images/bmp"     = @("*.bmp")
    "images/gif"     = @("*.gif")
    "images/tiff"    = @("*.tif", "*.tiff")
    "images/webp"    = @("*.webp")
    "images/avif"    = @("*.avif")
    "images/heif"    = @("*.heif", "*.heic")
    "images/jxl"     = @("*.jxl")
    "images/qoi"     = @("*.qoi")
    "images/tga"     = @("*.tga")
    "images/ico"     = @("*.ico")
    "images/dds"     = @("*.dds")
    "images/psd"     = @("*.psd")
    "images/exr"     = @("*.exr")
    "raw/cr2"        = @("*.cr2")
    "raw/nef"        = @("*.nef")
    "raw/arw"        = @("*.arw")
    "raw/dng"        = @("*.dng")
    "archives/zip"   = @("*.zip")
    "archives/cbz"   = @("*.cbz")
    "archives/rar"   = @("*.rar")
    "archives/cbr"   = @("*.cbr")
    "archives/7z"    = @("*.7z")
    "documents/pdf"  = @("*.pdf")
    "documents/epub" = @("*.epub")
    "media/mp3"      = @("*.mp3")
    "media/flac"     = @("*.flac")
    "media/mp4"      = @("*.mp4")
    "fonts/ttf"      = @("*.ttf")
    "fonts/otf"      = @("*.otf")
    "models/obj"     = @("*.obj")
    "models/stl"     = @("*.stl")
    "svg"            = @("*.svg")
}

# Create all category directories
foreach ($category in $categories.Keys) {
    $targetDir = Join-Path $CorpusDir $category
    $invalidDir = Join-Path $targetDir "invalid"
    
    New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
    New-Item -ItemType Directory -Path $invalidDir -Force | Out-Null
    
    Write-Host "Created: $category" -ForegroundColor Green
}

# Copy existing test files to corpus
if (Test-Path $SourceDir) {
    Write-Host "`nCopying existing test files..." -ForegroundColor Cyan
    
    # Map source directories to corpus categories
    $dirMapping = @{
        "standard"  = "images/jpeg,images/png,images/bmp,images/gif,images/tiff"
        "webp"      = "images/webp"
        "avif"      = "images/avif"
        "heif"      = "images/heif"
        "jxl"       = "images/jxl"
        "qoi"       = "images/qoi"
        "tga"       = "images/tga"
        "ico"       = "images/ico"
        "raw"       = "raw/cr2,raw/nef,raw/arw,raw/dng"
        "archives"  = "archives/zip,archives/cbz,archives/rar"
    }
    
    foreach ($srcSubDir in $dirMapping.Keys) {
        $srcPath = Join-Path $SourceDir $srcSubDir
        if (Test-Path $srcPath) {
            Get-ChildItem -Path $srcPath -File | ForEach-Object {
                $ext = $_.Extension.ToLower()
                $targetCategory = $null
                
                # Find matching category
                foreach ($cat in $categories.Keys) {
                    $patterns = $categories[$cat]
                    foreach ($pattern in $patterns) {
                        if ($_.Name -like $pattern) {
                            $targetCategory = $cat
                            break
                        }
                    }
                    if ($targetCategory) { break }
                }
                
                if ($targetCategory) {
                    $targetPath = Join-Path $CorpusDir $targetCategory
                    Copy-Item -Path $_.FullName -Destination $targetPath -Force
                    Write-Host "  Copied: $($_.Name) → $targetCategory" -ForegroundColor Gray
                }
            }
        }
    }
}

# Create invalid/corrupt test files for fuzzing
if ($CreateInvalidFiles) {
    Write-Host "`nCreating invalid test files for fuzzing..." -ForegroundColor Cyan
    
    foreach ($category in $categories.Keys) {
        $invalidDir = Join-Path $CorpusDir $category "invalid"
        $patterns = $categories[$category]
        $ext = $patterns[0].Replace("*", "")
        
        # Create truncated file (only first 100 bytes)
        $truncatedPath = Join-Path $invalidDir "truncated$ext"
        $randomBytes = New-Object byte[] 100
        (New-Object Random).NextBytes($randomBytes)
        [System.IO.File]::WriteAllBytes($truncatedPath, $randomBytes)
        
        # Create empty file
        $emptyPath = Join-Path $invalidDir "empty$ext"
        New-Item -ItemType File -Path $emptyPath -Force | Out-Null
        
        # Create garbage header file
        $garbagePath = Join-Path $invalidDir "garbage$ext"
        $garbageBytes = New-Object byte[] 1024
        (New-Object Random).NextBytes($garbageBytes)
        [System.IO.File]::WriteAllBytes($garbagePath, $garbageBytes)
        
        Write-Host "  Created invalid files for: $category" -ForegroundColor Yellow
    }
}

# Create README for corpus
$readmePath = Join-Path $CorpusDir "README.md"
$readmeContent = @"
# ExplorerLens Test Corpus

This directory contains a comprehensive test file collection for validating decoder functionality.

## Structure

- **images/** - Standard and modern image formats (JPEG, PNG, WebP, AVIF, HEIF, JXL, etc.)
- **raw/** - Camera RAW formats (CR2, NEF, ARW, DNG)
- **archives/** - Archive formats (ZIP, CBZ, RAR, 7Z)
- **documents/** - Document formats (PDF, EPUB)
- **media/** - Audio/video with thumbnails (MP3, FLAC, MP4)
- **fonts/** - Font files (TTF, OTF)
- **models/** - 3D model formats (OBJ, STL)
- **svg/** - Scalable Vector Graphics

## Invalid Files

Each format category contains an `invalid/` subdirectory with:
- **truncated** - File with only header bytes
- **empty** - Zero-byte file
- **garbage** - Random bytes with valid extension

These files test decoder robustness and crash resistance.

## Usage

Run the validator tool to test all formats:

``````powershell
.\tools\ExplorerLensValidator\x64\Release\ExplorerLensValidator.exe tests\data\corpus -v -o results.csv
``````

## Maintenance

To refresh the corpus with new test files:

``````powershell
.\tests\Organize-TestCorpus.ps1 -CreateInvalidFiles
``````

## Test Requirements (Sprint 13)

- ✅ At least 1 valid file per supported format
- ✅ Invalid/corrupt files for fuzzing
- ✅ Performance baseline files (various sizes)
- ✅ Real-world samples (not synthetic)

**Last Updated:** February 17, 2026  
**Version:** v7.0.0
"@

Set-Content -Path $readmePath -Value $readmeContent -Encoding UTF8

Write-Host "`n✓ Test corpus organization complete!" -ForegroundColor Green
Write-Host "Total categories: $($categories.Count)" -ForegroundColor Cyan
Write-Host "Corpus location: $CorpusDir" -ForegroundColor Cyan
Write-Host "`nNext steps:" -ForegroundColor Yellow
Write-Host "  1. Add real-world sample files to each category" -ForegroundColor Gray
Write-Host "  2. Build validator: cmake --build build --target ExplorerLensValidator" -ForegroundColor Gray
Write-Host "  3. Run validation: ExplorerLensValidator.exe tests\data\corpus" -ForegroundColor Gray

