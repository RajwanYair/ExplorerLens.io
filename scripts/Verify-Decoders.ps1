<#
.SYNOPSIS
    Verifies DarkThumbs decoder source files exist and have expected structure.

.DESCRIPTION
    Checks that all decoder source files are present, contain expected class/function 
    definitions, and validates conditional compilation guards. Reports decoder readiness
    status without requiring compilation.

    Run this after modifying decoders or build configuration to catch missing files,
    broken #ifdef guards, or incomplete implementations early.

.EXAMPLE
    .\Verify-Decoders.ps1
    .\Verify-Decoders.ps1 -Verbose
    .\Verify-Decoders.ps1 -CheckOnly Engine

.NOTES
    Part of DarkThumbs Sprint 15C audit tooling.
    Does NOT require build tools — pure file/text analysis.
#>

[CmdletBinding()]
param(
    [ValidateSet("All", "Engine", "CBXShell")]
    [string]$CheckOnly = "All"
)

$ErrorActionPreference = "Continue"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Split-Path -Parent $scriptDir

# Results tracking
$passed = 0
$failed = 0
$warnings = 0
$results = @()

function Test-DecoderFile {
    param(
        [string]$Path,
        [string]$Name,
        [string[]]$RequiredPatterns,
        [string]$ConditionalFlag = $null
    )

    $fullPath = Join-Path $projectRoot $Path
    $status = "PASS"
    $details = @()

    # Check file exists
    if (-not (Test-Path $fullPath)) {
        $script:failed++
        $script:results += [PSCustomObject]@{
            Decoder = $Name
            File    = $Path
            Status  = "FAIL"
            Details = "File not found"
        }
        return
    }

    $content = Get-Content $fullPath -Raw

    # Check required patterns
    foreach ($pattern in $RequiredPatterns) {
        if ($content -notmatch $pattern) {
            $status = "WARN"
            $details += "Missing: $pattern"
            $script:warnings++
        }
    }

    # Check conditional compilation guard
    if ($ConditionalFlag) {
        if ($content -match "#ifdef\s+$ConditionalFlag") {
            $details += "Has #ifdef $ConditionalFlag guard"
        }
        else {
            $status = "WARN"
            $details += "Missing #ifdef $ConditionalFlag guard"
            $script:warnings++
        }

        # Verify the E_NOTIMPL fallback exists
        if ($content -match "#else" -and $content -match "E_NOTIMPL") {
            $details += "Has E_NOTIMPL fallback"
        }
        elseif ($ConditionalFlag) {
            $details += "No E_NOTIMPL fallback (may not compile without library)"
        }
    }

    # Check for stub indicators
    $isStub = $false
    if ($content -match "return\s+E_NOTIMPL" -and $content -notmatch "#ifdef") {
        $status = "STUB"
        $details += "Returns E_NOTIMPL (no conditional - pure stub)"
    }

    if ($status -eq "PASS") {
        $script:passed++
    }

    $script:results += [PSCustomObject]@{
        Decoder = $Name
        File    = $Path
        Status  = $status
        Details = ($details -join "; ")
    }
}

function Test-CMakeFlags {
    $cmakePath = Join-Path $projectRoot "Engine\CMakeLists.txt"
    if (-not (Test-Path $cmakePath)) {
        Write-Warning "Engine/CMakeLists.txt not found"
        return
    }

    $content = Get-Content $cmakePath -Raw

    $flags = @("HAS_LIBJXL", "HAS_LIBHEIF", "HAS_LIBRAW")
    foreach ($flag in $flags) {
        if ($content -match "option\($flag") {
            Write-Host "  [OK] CMake option $flag defined" -ForegroundColor Green
        }
        else {
            Write-Host "  [!!] CMake option $flag MISSING" -ForegroundColor Red
            $script:failed++
        }
    }
}

function Test-UnitTests {
    $testPath = Join-Path $projectRoot "Engine\Tests\EngineTests.cpp"
    if (-not (Test-Path $testPath)) {
        Write-Warning "Engine/Tests/EngineTests.cpp not found"
        return
    }

    $content = Get-Content $testPath -Raw

    $expectedTests = @(
        "TestImageDecoder",
        "TestWebPDecoder",
        "TestAVIFDecoder",
        "TestArchiveDecoder",
        "TestJXLDecoder",
        "TestHEIFDecoder"
    )

    foreach ($test in $expectedTests) {
        $runMatches = [regex]::Matches($content, "RUN_TEST\($test")
        if ($runMatches.Count -gt 0) {
            Write-Host "  [OK] $test is registered ($($runMatches.Count) test(s))" -ForegroundColor Green
        }
        else {
            Write-Host "  [!!] $test NOT registered in test runner" -ForegroundColor Yellow
            $script:warnings++
        }
    }
}

# ============================================================
# Main
# ============================================================

Write-Host ""
Write-Host "============================================" -ForegroundColor Cyan
Write-Host " DarkThumbs Decoder Verification" -ForegroundColor Cyan
Write-Host " Project: $projectRoot" -ForegroundColor Gray
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

# --- Engine Decoders ---
if ($CheckOnly -in "All", "Engine") {
    Write-Host "--- Engine Decoders ---" -ForegroundColor Yellow

    Test-DecoderFile -Path "Engine\Decoders\ImageDecoder.cpp" -Name "ImageDecoder" `
        -RequiredPatterns @("class.*ImageDecoder", "CanDecode", "Decode\(")

    Test-DecoderFile -Path "Engine\Decoders\WebPDecoder.cpp" -Name "WebPDecoder" `
        -RequiredPatterns @("class.*WebPDecoder|WebPDecoder::", "CanDecode", "WebPDecodeRGBA|WebPDecode")

    Test-DecoderFile -Path "Engine\Decoders\AVIFDecoder.cpp" -Name "AVIFDecoder" `
        -RequiredPatterns @("AVIFDecoder", "CanDecode", "\.avif")

    Test-DecoderFile -Path "Engine\Decoders\HEIFDecoder.cpp" -Name "HEIFDecoder" `
        -RequiredPatterns @("HEIFDecoder", "CanDecode", "\.heif|\.heic") `
        -ConditionalFlag "HAS_LIBHEIF"

    Test-DecoderFile -Path "Engine\Decoders\JXLDecoder.cpp" -Name "JXLDecoder" `
        -RequiredPatterns @("JXLDecoder", "CanDecode", "\.jxl") `
        -ConditionalFlag "HAS_LIBJXL"

    Test-DecoderFile -Path "Engine\Decoders\RAWDecoder.cpp" -Name "RAWDecoder" `
        -RequiredPatterns @("RAWDecoder", "CanDecode", "ScaleToTarget")

    Test-DecoderFile -Path "Engine\Decoders\ArchiveDecoder.cpp" -Name "ArchiveDecoder" `
        -RequiredPatterns @("ArchiveDecoder", "CanDecode", "\.zip|\.cbz")

    Test-DecoderFile -Path "Engine\Decoders\TGADecoder.cpp" -Name "TGADecoder" `
        -RequiredPatterns @("TGADecoder", "CanDecode", "\.tga")

    Test-DecoderFile -Path "Engine\Decoders\QOIDecoder.cpp" -Name "QOIDecoder" `
        -RequiredPatterns @("QOIDecoder", "CanDecode", "\.qoi")

    Write-Host ""
    Write-Host "--- CMake Feature Flags ---" -ForegroundColor Yellow
    Test-CMakeFlags

    Write-Host ""
    Write-Host "--- Unit Test Registration ---" -ForegroundColor Yellow
    Test-UnitTests
}

# --- CBXShell Legacy Decoders ---
if ($CheckOnly -in "All", "CBXShell") {
    Write-Host ""
    Write-Host "--- CBXShell Legacy Decoders ---" -ForegroundColor Yellow

    Test-DecoderFile -Path "CBXShell\avif_decoder.cpp" -Name "AVIF (Legacy)" `
        -RequiredPatterns @("DecodeToHBITMAP|AVIF", "\.avif")

    Test-DecoderFile -Path "CBXShell\webp_decoder.cpp" -Name "WebP (Legacy)" `
        -RequiredPatterns @("DecodeToHBITMAP|WebP", "\.webp")

    Test-DecoderFile -Path "CBXShell\jxl_decoder.cpp" -Name "JXL (Legacy)" `
        -RequiredPatterns @("DecodeToHBITMAP|JXL", "\.jxl")

    Test-DecoderFile -Path "CBXShell\heif_decoder_native.cpp" -Name "HEIF Native (Legacy)" `
        -RequiredPatterns @("DecodeToHBITMAP|HEIF", "\.heif|\.heic")

    Test-DecoderFile -Path "CBXShell\raw_decoder.cpp" -Name "RAW (Legacy)" `
        -RequiredPatterns @("DecodeToHBITMAP|RAW|RawDecoder", "\.cr2|\.nef|\.dng")

    Test-DecoderFile -Path "CBXShell\pdf_decoder.cpp" -Name "PDF (Legacy)" `
        -RequiredPatterns @("PDF|pdf", "\.pdf")

    Test-DecoderFile -Path "CBXShell\svg_decoder.h" -Name "SVG (Legacy)" `
        -RequiredPatterns @("SVG|svg", "\.svg")

    Test-DecoderFile -Path "CBXShell\video_thumbnail.cpp" -Name "Video (Legacy)" `
        -RequiredPatterns @("VideoThumbnail|ExtractFrame", "DirectShow|dshow")

    Test-DecoderFile -Path "CBXShell\audio_thumbnail.cpp" -Name "Audio (Legacy)" `
        -RequiredPatterns @("AudioThumbnail|ExtractAlbumArt", "\.mp3|\.flac")

    Test-DecoderFile -Path "CBXShell\document_thumbnail.cpp" -Name "Document (Legacy)" `
        -RequiredPatterns @("DocumentThumbnail|ExtractDocumentThumbnail", "\.docx|\.doc")

    Test-DecoderFile -Path "CBXShell\font_preview.cpp" -Name "Font (Legacy)" `
        -RequiredPatterns @("FontPreview|GenerateFontPreview", "\.ttf|\.otf")
}

# --- Summary ---
Write-Host ""
Write-Host "============================================" -ForegroundColor Cyan
Write-Host " Results Summary" -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

$results | Format-Table -AutoSize

$total = $passed + $failed + $warnings
Write-Host "  Passed:   $passed" -ForegroundColor Green
Write-Host "  Warnings: $warnings" -ForegroundColor Yellow
Write-Host "  Failed:   $failed" -ForegroundColor Red
Write-Host "  Total:    $total checks" -ForegroundColor Cyan
Write-Host ""

if ($failed -gt 0) {
    Write-Host "  RESULT: FAILURES DETECTED" -ForegroundColor Red
    exit 1
}
elseif ($warnings -gt 0) {
    Write-Host "  RESULT: PASSED WITH WARNINGS" -ForegroundColor Yellow
    exit 0
}
else {
    Write-Host "  RESULT: ALL CHECKS PASSED" -ForegroundColor Green
    exit 0
}
