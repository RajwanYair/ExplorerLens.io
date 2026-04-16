#Requires -Version 7.0
<#
.SYNOPSIS
    Setup-TestCorpus.ps1 — Download or generate ExplorerLens test corpus files.

.DESCRIPTION
    Creates the data/corpus/ directory tree and populates it with minimal
    synthetic test files for each format in MANIFEST.json.

    Synthetic files are generated using ImageMagick (magick), FFmpeg, or
    PowerShell-native methods, depending on availability.  Real-world files
    can be placed in the corresponding subdirectory and will be used by the
    corpus validation workflow instead.

    Files are written to data/corpus/<category>/<format>/ and are small
    (< 100 KB each) so they can optionally be committed to the repository.
    Files > 500 KB are excluded via .gitignore.

.PARAMETER Formats
    Comma-separated list of format IDs to generate (default: all).
    Example: -Formats "JPEG,PNG,WEBP"

.PARAMETER OutputDir
    Root output directory.  Default: <workspace>/data/corpus

.PARAMETER Force
    Regenerate files even if they already exist.

.PARAMETER DryRun
    Print what would be done without creating files.

.EXAMPLE
    .\scripts\Setup-TestCorpus.ps1
    .\scripts\Setup-TestCorpus.ps1 -Formats "JPEG,PNG,WEBP,PDF" -Force
#>
[CmdletBinding()]
param(
    [string]   $Formats   = "",
    [string]   $OutputDir = "",
    [switch]   $Force,
    [switch]   $DryRun
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# ---------------------------------------------------------------------------
# Resolve paths
# ---------------------------------------------------------------------------

$scriptDir  = $PSScriptRoot
$rootDir    = Split-Path -Parent $scriptDir
if (-not $OutputDir) { $OutputDir = Join-Path $rootDir "data\corpus" }

$manifestPath = Join-Path $rootDir "data\corpus\MANIFEST.json"
if (-not (Test-Path $manifestPath)) {
    Write-Error "MANIFEST.json not found at $manifestPath"
}

$manifest = Get-Content $manifestPath -Raw | ConvertFrom-Json

# ---------------------------------------------------------------------------
# Tool detection
# ---------------------------------------------------------------------------

$magick  = (Get-Command magick  -ErrorAction SilentlyContinue)?.Source
$ffmpeg  = (Get-Command ffmpeg  -ErrorAction SilentlyContinue)?.Source
$python  = (Get-Command python  -ErrorAction SilentlyContinue)?.Source

Write-Host "ImageMagick : $(if ($magick)  { $magick  } else { 'not found — JPEG/PNG/TIFF generate will be skipped' })"
Write-Host "FFmpeg      : $(if ($ffmpeg)  { $ffmpeg  } else { 'not found — MP4/MKV/FLAC generate will be skipped' })"
Write-Host "Python      : $(if ($python)  { $python  } else { 'not found — synthetic formats will be minimal' })"

# ---------------------------------------------------------------------------
# Filter formats if requested
# ---------------------------------------------------------------------------

$filterSet = @{}
if ($Formats) { ($Formats -split ',') | ForEach-Object { $filterSet[$_.Trim().ToUpper()] = $true } }

function ShouldProcess([string]$fmt) {
    return ($filterSet.Count -eq 0) -or $filterSet.ContainsKey($fmt.ToUpper())
}

$created = 0
$skipped = 0
$failed  = 0

# ---------------------------------------------------------------------------
# Helper: ensure directory exists
# ---------------------------------------------------------------------------

function EnsureDir([string]$path) {
    if (-not (Test-Path $path)) {
        if (-not $DryRun) { New-Item -ItemType Directory -Path $path -Force | Out-Null }
        Write-Host "  mkdir $path"
    }
}

# ---------------------------------------------------------------------------
# Helper: write a minimal valid file if it doesn't exist (or -Force)
# ---------------------------------------------------------------------------

function WriteFile([string]$path, [byte[]]$bytes, [string]$desc) {
    if ((Test-Path $path) -and -not $Force) {
        $script:skipped++
        return
    }
    if ($DryRun) {
        Write-Host "  [DryRun] would write $desc → $path ($($bytes.Length) bytes)"
        $script:created++
        return
    }
    try {
        [IO.File]::WriteAllBytes($path, $bytes)
        Write-Host "  ✓ $desc → $(Split-Path $path -Leaf) ($($bytes.Length) B)"
        $script:created++
    } catch {
        Write-Warning "  ✗ failed to write $path : $_"
        $script:failed++
    }
}

# ---------------------------------------------------------------------------
# Minimal valid file byte sequences for formats that need no external tool
# ---------------------------------------------------------------------------

# 1×1 white JPEG (JFIF APP0 marker)
$minimalJpeg = [byte[]]@(
    0xFF,0xD8, 0xFF,0xE0, 0x00,0x10, 0x4A,0x46,0x49,0x46,0x00,0x01,0x01,0x00,0x00,0x01,0x00,0x01,0x00,0x00,
    0xFF,0xDB, 0x00,0x43, 0x00,
    0x08,0x06,0x06,0x07,0x06,0x05,0x08,0x07,0x07,0x07,0x09,0x09,0x08,0x0a,0x0c,0x14,
    0x0d,0x0c,0x0b,0x0b,0x0c,0x19,0x12,0x13,0x0f,0x14,0x1d,0x1a,0x1f,0x1e,0x1d,0x1a,
    0x1c,0x1c,0x20,0x24,0x2e,0x27,0x20,0x22,0x2c,0x23,0x1c,0x1c,0x28,0x37,0x29,0x2c,
    0x30,0x31,0x34,0x34,0x34,0x1f,0x27,0x39,0x3d,0x38,0x32,0x3c,0x2e,0x33,0x34,0x32,
    0xFF,0xC0, 0x00,0x0B, 0x08, 0x00,0x01, 0x00,0x01, 0x01, 0x01,0x11,0x00,
    0xFF,0xC4, 0x00,0x1F, 0x00, 0x00,0x01,0x05,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,
    0xFF,0xDA, 0x00,0x08, 0x01,0x01,0x00,0x00,0x3F,0x00, 0x7F, 0xFF,0xD9
)

# 1×1 white PNG
$minimalPng = [byte[]]@(
    0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A, # signature
    0x00,0x00,0x00,0x0D, 0x49,0x48,0x44,0x52, # IHDR chunk len=13
    0x00,0x00,0x00,0x01, 0x00,0x00,0x00,0x01, # width=1, height=1
    0x08,0x02, 0x00,0x00,0x00, 0x90,0x77,0x53,0xDE, # bitdepth=8, RGB, CRC
    0x00,0x00,0x00,0x0C, 0x49,0x44,0x41,0x54, # IDAT chunk len=12
    0x08,0xD7, 0x63,0xF8,0xFF,0xFF,0x3F, 0x00, 0x05,0xFE, 0x02,0xFE, # compressed
    0xDC,0xCC,0x59,0xE7,
    0x00,0x00,0x00,0x00, 0x49,0x45,0x4E,0x44, 0xAE,0x42,0x60,0x82  # IEND
)

# Minimal BMP (1×1 24-bit)
$minimalBmp = [byte[]]@(
    0x42,0x4D, 0x36,0x00,0x00,0x00, 0x00,0x00, 0x00,0x00, 0x36,0x00,0x00,0x00,
    0x28,0x00,0x00,0x00, 0x01,0x00,0x00,0x00, 0x01,0x00,0x00,0x00,
    0x01,0x00, 0x18,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0xFF,0xFF,0xFF, 0x00 # 1 white pixel + 1 byte padding
)

# Minimal valid PDF (one empty page with 'Hello World')
$minimalPdf = [System.Text.Encoding]::ASCII.GetBytes(
    "%PDF-1.4`n" +
    "1 0 obj<</Type/Catalog/Pages 2 0 R>>endobj`n" +
    "2 0 obj<</Type/Pages/Kids[3 0 R]/Count 1>>endobj`n" +
    "3 0 obj<</Type/Page/MediaBox[0 0 72 72]/Parent 2 0 R>>endobj`n" +
    "xref`n0 4`n0000000000 65535 f `n0000000009 00000 n `n" +
    "0000000062 00000 n `n0000000113 00000 n `n" +
    "trailer<</Size 4/Root 1 0 R>>`nstartxref`n172`n%%EOF`n"
)

# Minimal GIF89a (1×1 white)
$minimalGif = [byte[]]@(
    0x47,0x49,0x46,0x38,0x39,0x61, # GIF89a
    0x01,0x00, 0x01,0x00, 0x80,0x00,0x00, # 1×1, 2-color GCT
    0xFF,0xFF,0xFF, 0x00,0x00,0x00,       # GCT: white, black
    0x2C, 0x00,0x00, 0x00,0x00, 0x01,0x00, 0x01,0x00, 0x00, # Image descriptor
    0x02, 0x02, 0x4C, 0x01, 0x00, # Image data
    0x3B # Trailer
)

# Minimal ZIP (empty archive)
$minimalZip = [byte[]]@(
    0x50,0x4B,0x05,0x06, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00
)

# Minimal RAR4 header (no files — enough to satisfy signature check)
$minimalRar4 = [byte[]]@(
    0x52,0x61,0x72,0x21,0x1A,0x07,0x00,
    0x1A,0x00, # marker block header
    0x01,0x00,0xAD,0x51, 0x07,0x00, 0x00,0x00 # archive header
)

# ---------------------------------------------------------------------------
# Main generation loop
# ---------------------------------------------------------------------------

EnsureDir (Join-Path $OutputDir "images")
EnsureDir (Join-Path $OutputDir "archives")
EnsureDir (Join-Path $OutputDir "documents")
EnsureDir (Join-Path $OutputDir "media")
EnsureDir (Join-Path $OutputDir "models")
EnsureDir (Join-Path $OutputDir "fonts")
EnsureDir (Join-Path $OutputDir "scientific")
EnsureDir (Join-Path $OutputDir "raw")

$imgDir  = Join-Path $OutputDir "images"
$arcDir  = Join-Path $OutputDir "archives"
$docDir  = Join-Path $OutputDir "documents"
$mdDir   = Join-Path $OutputDir "media"

# ---- Images ----
if (ShouldProcess "JPEG") {
    WriteFile (Join-Path $imgDir "sample_minimal.jpg") $minimalJpeg "JPEG 1×1 minimal"
    if ($magick) {
        $out = Join-Path $imgDir "sample_256px.jpg"
        if (-not (Test-Path $out) -or $Force) {
            if (-not $DryRun) { & $magick "xc:white[256x256]" -quality 85 $out 2>$null }
            Write-Host "  ✓ JPEG 256×256 (ImageMagick)"
            $created++
        } else { $skipped++ }
    }
}

if (ShouldProcess "PNG")  { WriteFile (Join-Path $imgDir "sample_minimal.png") $minimalPng  "PNG 1×1 minimal"  }
if (ShouldProcess "BMP")  { WriteFile (Join-Path $imgDir "sample_minimal.bmp") $minimalBmp  "BMP 1×1 24-bit"   }
if (ShouldProcess "GIF")  { WriteFile (Join-Path $imgDir "sample_minimal.gif") $minimalGif  "GIF 1×1 minimal"  }

# ---- Archives ----
if (ShouldProcess "ZIP")  { WriteFile (Join-Path $arcDir "sample_empty.zip")  $minimalZip  "ZIP empty"        }
if (ShouldProcess "RAR")  { WriteFile (Join-Path $arcDir "sample_minimal.rar") $minimalRar4 "RAR4 minimal"     }

# Create a CBZ (ZIP with an image inside)
if (ShouldProcess "CBZ") {
    $cbzPath = Join-Path $arcDir "sample.cbz"
    if (-not (Test-Path $cbzPath) -or $Force) {
        if (-not $DryRun) {
            # Build a ZIP in-memory containing one JPEG
            Add-Type -AssemblyName "System.IO.Compression"
            $ms = [IO.MemoryStream]::new()
            $zip = [IO.Compression.ZipArchive]::new($ms, [IO.Compression.ZipArchiveMode]::Create, $true)
            $entry = $zip.CreateEntry("001.jpg")
            $es = $entry.Open()
            $es.Write($minimalJpeg, 0, $minimalJpeg.Length)
            $es.Dispose(); $zip.Dispose()
            [IO.File]::WriteAllBytes($cbzPath, $ms.ToArray())
            $ms.Dispose()
            Write-Host "  ✓ CBZ (ZIP containing 001.jpg)"
        } else { Write-Host "  [DryRun] CBZ" }
        $created++
    } else { $skipped++ }
}

# ---- Documents ----
if (ShouldProcess "PDF")  { WriteFile (Join-Path $docDir "sample_minimal.pdf") $minimalPdf "PDF empty page"   }

# ---- Media (require ffmpeg) ----
if (ShouldProcess "MP4") {
    $mp4 = Join-Path $mdDir "sample_1s.mp4"
    if ((-not (Test-Path $mp4) -or $Force) -and $ffmpeg) {
        if (-not $DryRun) {
            & $ffmpeg -y -f lavfi -i "color=c=white:size=64x64:rate=1" -t 1 `
              -c:v libx264 -pix_fmt yuv420p $mp4 2>$null
        }
        Write-Host "  ✓ MP4 1s white 64×64"
        $created++
    } elseif (-not $ffmpeg) {
        Write-Warning "  ✗ MP4: ffmpeg not found"
        $skipped++
    } else { $skipped++ }
}

# ---- Summary ----
Write-Host ""
Write-Host "─────────────────────────────────────────────"
Write-Host "Corpus setup complete"
Write-Host "  Created : $created"
Write-Host "  Skipped : $skipped (already exist; use -Force to regenerate)"
Write-Host "  Failed  : $failed"
Write-Host ""
Write-Host "Output  : $OutputDir"
Write-Host "Run     : lens benchmark $OutputDir --json"
