#Requires -Version 7.0

<#
.SYNOPSIS
    Format support testing for ExplorerLens thumbnail generation
.DESCRIPTION
    Tests thumbnail generation capabilities across all supported image and archive formats.
    Creates test images for each format and validates thumbnail extraction.
.PARAMETER OutputDir
    Directory to store test images (default: tests\test-images)
.PARAMETER TestExisting
    Only test with existing images in OutputDir, don't generate new ones
#>

param(
    [string]$OutputDir = "tests\test-images",
    [switch]$TestExisting
)

$ErrorActionPreference = "Continue"
$testResults = @{
    Passed   = @()
    Failed   = @()
    Skipped  = @()
    Warnings = @()
}

function Write-TestResult {
    param([string]$TestName, [string]$Status, [string]$Message = "")
    
    $color = switch ($Status) {
        "Passed" { "Green"; $script:testResults.Passed += $TestName }
        "Failed" { "Red"; $script:testResults.Failed += $TestName }
        "Skipped" { "Yellow"; $script:testResults.Skipped += $TestName }
        default { "Gray" }
    }
    
    $icon = switch ($Status) {
        "Passed" { "✅" }
        "Failed" { "❌" }
        "Skipped" { "⏭️" }
        default { "ℹ️" }
    }
    
    Write-Host "  $icon $TestName" -ForegroundColor $color
    if ($Message) { Write-Host "     $Message" -ForegroundColor Gray }
}

function Write-TestWarning {
    param([string]$Message)
    Write-Host "  ⚠️  $Message" -ForegroundColor Yellow
    $script:testResults.Warnings += $Message
}

function New-TestImage {
    param(
        [string]$Format,
        [string]$OutputPath,
        [int]$Width = 256,
        [int]$Height = 256
    )
    
    try {
        Add-Type -AssemblyName System.Drawing
        
        # Create a bitmap
        $bitmap = New-Object System.Drawing.Bitmap $Width, $Height
        $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
        
        # Fill with gradient background
        $brush = New-Object System.Drawing.Drawing2D.LinearGradientBrush(
            (New-Object System.Drawing.Point 0, 0),
            (New-Object System.Drawing.Point $Width, $Height),
            [System.Drawing.Color]::Blue,
            [System.Drawing.Color]::Cyan
        )
        $graphics.FillRectangle($brush, 0, 0, $Width, $Height)
        
        # Add text label
        $font = New-Object System.Drawing.Font("Arial", 24, [System.Drawing.FontStyle]::Bold)
        $textBrush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::White)
        $graphics.DrawString($Format.ToUpper(), $font, $textBrush, 10, 100)
        
        # Save in appropriate format
        switch ($Format.ToLower()) {
            "jpg" { $bitmap.Save($OutputPath, [System.Drawing.Imaging.ImageFormat]::Jpeg) }
            "jpeg" { $bitmap.Save($OutputPath, [System.Drawing.Imaging.ImageFormat]::Jpeg) }
            "png" { $bitmap.Save($OutputPath, [System.Drawing.Imaging.ImageFormat]::Png) }
            "bmp" { $bitmap.Save($OutputPath, [System.Drawing.Imaging.ImageFormat]::Bmp) }
            "gif" { $bitmap.Save($OutputPath, [System.Drawing.Imaging.ImageFormat]::Gif) }
            "tif" { $bitmap.Save($OutputPath, [System.Drawing.Imaging.ImageFormat]::Tiff) }
            "tiff" { $bitmap.Save($OutputPath, [System.Drawing.Imaging.ImageFormat]::Tiff) }
            default { return $false }
        }
        
        # Cleanup
        $graphics.Dispose()
        $bitmap.Dispose()
        $brush.Dispose()
        $font.Dispose()
        $textBrush.Dispose()
        
        return $true
    }
    catch {
        Write-TestWarning "Failed to create $Format image: $_"
        return $false
    }
}

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "FORMAT SUPPORT TESTING" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

# Ensure output directory exists
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
    Write-Host "Created test image directory: $OutputDir`n" -ForegroundColor Gray
}

# Phase 1: Generate test images
if (-not $TestExisting) {
    Write-Host "Phase 1: Generating Test Images" -ForegroundColor Yellow
    Write-Host "--------------------------------" -ForegroundColor Gray
    
    $imageFormats = @(
        @{ Format = "jpg"; Extension = ".jpg"; Name = "JPEG" },
        @{ Format = "png"; Extension = ".png"; Name = "PNG" },
        @{ Format = "bmp"; Extension = ".bmp"; Name = "BMP" },
        @{ Format = "gif"; Extension = ".gif"; Name = "GIF" },
        @{ Format = "tif"; Extension = ".tif"; Name = "TIFF" }
    )
    
    foreach ($img in $imageFormats) {
        $outputPath = Join-Path $OutputDir "test-$($img.Format)$($img.Extension)"
        
        if (New-TestImage -Format $img.Format -OutputPath $outputPath) {
            $file = Get-Item $outputPath
            Write-TestResult "$($img.Name) generation" "Passed" "Created: $($file.Name) ($([Math]::Round($file.Length/1KB,1)) KB)"
        }
        else {
            Write-TestResult "$($img.Name) generation" "Failed" "Could not create test image"
        }
    }
}

# Phase 2: Verify test image files exist
Write-Host "`nPhase 2: Test Image Verification" -ForegroundColor Yellow
Write-Host "---------------------------------" -ForegroundColor Gray

$expectedFormats = @(
    @{ Name = "JPEG"; Pattern = "*.jpg" },
    @{ Name = "PNG"; Pattern = "*.png" },
    @{ Name = "BMP"; Pattern = "*.bmp" },
    @{ Name = "GIF"; Pattern = "*.gif" },
    @{ Name = "TIFF"; Pattern = "*.tif" }
)

$testFiles = @()
foreach ($fmt in $expectedFormats) {
    $files = Get-ChildItem -Path $OutputDir -Filter $fmt.Pattern -ErrorAction SilentlyContinue
    
    if ($files) {
        $testFiles += $files
        Write-TestResult "$($fmt.Name) test files" "Passed" "Found $($files.Count) file(s)"
    }
    else {
        Write-TestResult "$($fmt.Name) test files" "Skipped" "No test files found"
    }
}

# Phase 3: Format capability check (what LENSShell should support)
Write-Host "`nPhase 3: LENSShell Format Capabilities" -ForegroundColor Yellow
Write-Host "--------------------------------------" -ForegroundColor Gray

$supportedFormats = @(
    @{ Format = "JPEG"; Extensions = ".jpg, .jpeg"; Status = "Core" },
    @{ Format = "PNG"; Extensions = ".png"; Status = "Core" },
    @{ Format = "BMP"; Extensions = ".bmp, .dib"; Status = "Core" },
    @{ Format = "GIF"; Extensions = ".gif"; Status = "Core" },
    @{ Format = "TIFF"; Extensions = ".tif, .tiff"; Status = "Core" },
    @{ Format = "WebP"; Extensions = ".webp"; Status = "Extended - libwebp" },
    @{ Format = "AVIF"; Extensions = ".avif"; Status = "Extended - libavif" },
    @{ Format = "HEIF"; Extensions = ".heif, .heic"; Status = "Extended - libheif" },
    @{ Format = "JXL"; Extensions = ".jxl"; Status = "Extended - libjxl" },
    @{ Format = "ZIP"; Extensions = ".zip, .cbz"; Status = "Archive" },
    @{ Format = "RAR"; Extensions = ".rar, .cbr"; Status = "Archive - UnRAR" },
    @{ Format = "7-Zip"; Extensions = ".7z, .cb7"; Status = "Archive - LZMA" },
    @{ Format = "PDF"; Extensions = ".pdf"; Status = "Document" }
)

Write-Host "`nSupported Formats Matrix:" -ForegroundColor Cyan
Write-Host "-" * 70 -ForegroundColor Gray
Write-Host $("{0,-15} {1,-25} {2}" -f "Format", "Extensions", "Implementation") -ForegroundColor White
Write-Host "-" * 70 -ForegroundColor Gray

foreach ($fmt in $supportedFormats) {
    $color = switch ($fmt.Status) {
        { $_ -match "Core" } { "Green" }
        { $_ -match "Extended" } { "Cyan" }
        { $_ -match "Archive" } { "Yellow" }
        default { "Gray" }
    }
    Write-Host $("{0,-15} {1,-25} {2}" -f $fmt.Format, $fmt.Extensions, $fmt.Status) -ForegroundColor $color
}

# Phase 4: Archive format test preparation
Write-Host "`n`nPhase 4: Archive Format Test Files" -ForegroundColor Yellow
Write-Host "-----------------------------------" -ForegroundColor Gray

# Check if we can create ZIP archives
try {
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    
    # Create a test ZIP archive with an image
    $zipPath = Join-Path $OutputDir "test-archive.zip"
    $testImage = Get-ChildItem -Path $OutputDir -Filter "*.png" | Select-Object -First 1
    
    if ($testImage) {
        if (Test-Path $zipPath) { Remove-Item $zipPath -Force }
        
        $zip = [System.IO.Compression.ZipFile]::Open($zipPath, [System.IO.Compression.ZipArchiveMode]::Create)
        [System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($zip, $testImage.FullName, $testImage.Name) | Out-Null
        $zip.Dispose()
        
        if (Test-Path $zipPath) {
            $zipFile = Get-Item $zipPath
            Write-TestResult "ZIP archive creation" "Passed" "Created: $($zipFile.Name) ($([Math]::Round($zipFile.Length/1KB,1)) KB)"
        }
    }
    else {
        Write-TestResult "ZIP archive creation" "Skipped" "No PNG test image available"
    }
}
catch {
    Write-TestResult "ZIP archive creation" "Failed" "Error: $_"
}

# Phase 5: Document extended format requirements
Write-Host "`nPhase 5: Extended Format Requirements" -ForegroundColor Yellow
Write-Host "--------------------------------------" -ForegroundColor Gray

$extendedLibs = @(
    @{ Library = "libwebp"; Formats = "WebP"; File = "external\libwebp\build\Release\libwebp.lib" },
    @{ Library = "libavif"; Formats = "AVIF"; File = "external\libavif\build\Release\avif.lib" },
    @{ Library = "libjxl"; Formats = "JPEG XL"; File = "external\libjxl\build\lib\Release\jxl.lib" }
)

foreach ($lib in $extendedLibs) {
    if (Test-Path $lib.File) {
        $file = Get-Item $lib.File
        Write-TestResult "$($lib.Library) available" "Passed" "$($lib.Formats) support ready ($([Math]::Round($file.Length/1KB,0)) KB)"
    }
    else {
        Write-TestResult "$($lib.Library) available" "Skipped" "$($lib.Formats) - Library not built"
    }
}

# Summary
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "TEST SUMMARY" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

$total = $testResults.Passed.Count + $testResults.Failed.Count + $testResults.Skipped.Count
$passRate = if ($testResults.Passed.Count + $testResults.Failed.Count -gt 0) {
    [Math]::Round(($testResults.Passed.Count / ($testResults.Passed.Count + $testResults.Failed.Count)) * 100, 1)
}
else { 0 }

Write-Host "Passed:   $($testResults.Passed.Count)" -ForegroundColor Green
Write-Host "Failed:   $($testResults.Failed.Count)" -ForegroundColor Red
Write-Host "Skipped:  $($testResults.Skipped.Count)" -ForegroundColor Yellow
Write-Host "Warnings: $($testResults.Warnings.Count)" -ForegroundColor Yellow
if ($passRate -gt 0) {
    Write-Host "Pass Rate: $passRate%" -ForegroundColor $(if ($passRate -ge 90) { "Green" } elseif ($passRate -ge 70) { "Yellow" } else { "Red" })
}

Write-Host "`nTest Images Location: $OutputDir" -ForegroundColor Cyan
Write-Host "Total Test Files: $(($testFiles | Measure-Object).Count)" -ForegroundColor Gray

if ($testResults.Failed.Count -eq 0) {
    Write-Host "`n✅ FORMAT SUPPORT TEST PREPARATION COMPLETE!" -ForegroundColor Green
    Write-Host "`nNext Steps:" -ForegroundColor Cyan
    Write-Host "1. Install LENSShell (requires admin)" -ForegroundColor White
    Write-Host "2. Test thumbnail generation in Windows Explorer" -ForegroundColor White
    Write-Host "3. Verify each format renders correctly" -ForegroundColor White
}
else {
    Write-Host "`n❌ SOME FORMAT TESTS FAILED!" -ForegroundColor Red
}

Write-Host ""

# Return test results for automation
return @{
    Success   = ($testResults.Failed.Count -eq 0)
    Passed    = $testResults.Passed.Count
    Failed    = $testResults.Failed.Count
    Skipped   = $testResults.Skipped.Count
    TestFiles = $testFiles
}

