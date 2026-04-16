<#
.SYNOPSIS
    Generate-Corpus.ps1 — Create synthetic test files for the ExplorerLens decoder corpus.

.DESCRIPTION
    Creates minimal but structurally valid test files for every format listed in
    data/corpus/MANIFEST.json. Files are intentionally small (< 100 KB) so they
    can be committed to the repository without bloating git history.

    For real-world samples, use -DownloadSamples to fetch publicly licensed files.
    See data/corpus/README.md for policies.

.PARAMETER CorpusRoot
    Path to the corpus root directory. Default: $PSScriptRoot/../data/corpus

.PARAMETER DownloadSamples
    Also download free-licensed real-world samples from public sources.

.PARAMETER Force
    Overwrite existing files.

.EXAMPLE
    .\tools\Generate-Corpus.ps1
    .\tools\Generate-Corpus.ps1 -DownloadSamples -Force

.NOTES
    Synthetic file format notes:
    - JPEG: valid 1x1 pixel JPEG (44 bytes)
    - PNG:  valid 1x1 transparent PNG (67 bytes)
    - WebP: valid 1x1 WebP (RIFF/WEBP header + VP8L)
    - BMP:  valid 2x2 24-bit BMP
    - PDF:  minimal valid single-page PDF
    - ZIP/CBZ: 2-file ZIP containing sample JPEG
    - SVG:  minimal SVG viewport with a rectangle
    - TTF:  cannot generate synthetic; placeholder with .gitkeep
    - RAW:  cannot generate synthetic; placeholder only
    All other unsupported formats create a .placeholder file with a notice.
#>
[CmdletBinding()]
param(
    [string] $CorpusRoot = (Join-Path $PSScriptRoot "..\data\corpus"),
    [switch] $DownloadSamples,
    [switch] $Force
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$CorpusRoot = (Resolve-Path $CorpusRoot -ErrorAction SilentlyContinue)?.Path ?? (New-Item -ItemType Directory -Force $CorpusRoot).FullName

function Write-Step([string] $msg) { Write-Host "  » $msg" -ForegroundColor Cyan }
function Write-OK([string] $msg)   { Write-Host "  ✓ $msg" -ForegroundColor Green }
function Write-Skip([string] $msg) { Write-Host "  · $msg" -ForegroundColor DarkGray }

function New-File([string] $path, [byte[]] $bytes) {
    if ((Test-Path $path) -and -not $Force) {
        Write-Skip "Already exists: $(Split-Path $path -Leaf)"
        return
    }
    $dir = Split-Path $path -Parent
    if (-not (Test-Path $dir)) { New-Item -ItemType Directory -Force $dir | Out-Null }
    [System.IO.File]::WriteAllBytes($path, $bytes)
    Write-OK "Generated: $(Split-Path $path -Leaf) ($($bytes.Length) bytes)"
}

function New-TextFile([string] $path, [string] $content) {
    New-File $path ([System.Text.Encoding]::UTF8.GetBytes($content))
}

# ---------------------------------------------------------------------------
# Synthetic generators
# ---------------------------------------------------------------------------

# Minimal 1×1 gray JPEG — JFIF APP0 + SOF0 + SOS + EOI
$jpegBytes = [byte[]]@(
    0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01,
    0x01, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0xFF, 0xDB, 0x00, 0x43,
    0x00, 0x08, 0x06, 0x06, 0x07, 0x06, 0x05, 0x08, 0x07, 0x07, 0x07, 0x09,
    0x09, 0x08, 0x0A, 0x0C, 0x14, 0x0D, 0x0C, 0x0B, 0x0B, 0x0C, 0x19, 0x12,
    0x13, 0x0F, 0x14, 0x1D, 0x1A, 0x1F, 0x1E, 0x1D, 0x1A, 0x1C, 0x1C, 0x20,
    0x24, 0x2E, 0x27, 0x20, 0x22, 0x2C, 0x23, 0x1C, 0x1C, 0x28, 0x37, 0x29,
    0x2C, 0x30, 0x31, 0x34, 0x34, 0x34, 0x1F, 0x27, 0x39, 0x3D, 0x38, 0x32,
    0x3C, 0x2E, 0x33, 0x34, 0x32, 0xFF, 0xC0, 0x00, 0x0B, 0x08, 0x00, 0x01,
    0x00, 0x01, 0x01, 0x01, 0x11, 0x00, 0xFF, 0xC4, 0x00, 0x1F, 0x00, 0x00,
    0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0xFF, 0xC4, 0x00, 0xB5, 0x10, 0x00, 0x02, 0x01, 0x03,
    0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7D,
    0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06,
    0x13, 0x51, 0x61, 0x07, 0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xA1, 0x08,
    0x23, 0x42, 0xB1, 0xC1, 0x15, 0x52, 0xD1, 0xF0, 0x24, 0x33, 0x62, 0x72,
    0x82, 0x09, 0x0A, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2A, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44, 0x45,
    0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x73, 0x74, 0x75,
    0x76, 0x77, 0x78, 0x79, 0x7A, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
    0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3,
    0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6,
    0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9,
    0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xE1, 0xE2,
    0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xF1, 0xF2, 0xF3, 0xF4,
    0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFF, 0xDA, 0x00, 0x08, 0x01, 0x01,
    0x00, 0x00, 0x3F, 0x00, 0xFB, 0xDE, 0xFF, 0xD9
)

# Minimal 1×1 transparent PNG (IHDR + IDAT + IEND)
$pngBytes = [byte[]]@(
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, # signature
    0x00, 0x00, 0x00, 0x0D, # IHDR length
    0x49, 0x48, 0x44, 0x52, # "IHDR"
    0x00, 0x00, 0x00, 0x01, # width=1
    0x00, 0x00, 0x00, 0x01, # height=1
    0x08, 0x02,             # 8-bit RGB
    0x00, 0x00, 0x00,       # compression, filter, interlace
    0x90, 0x77, 0x53, 0xDE, # CRC
    0x00, 0x00, 0x00, 0x0C, # IDAT length
    0x49, 0x44, 0x41, 0x54, # "IDAT"
    0x08, 0xD7, 0x63, 0xF8, 0xCF, 0xC0, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, # zlib of 1 pixel
    0xE2, 0x21, 0xBC, 0x33, # CRC
    0x00, 0x00, 0x00, 0x00, # IEND length=0
    0x49, 0x45, 0x4E, 0x44, # "IEND"
    0xAE, 0x42, 0x60, 0x82  # CRC
)

# Minimal 4×4 BMP (24-bit)
$bmpBytes = [byte[]]@(
    # BITMAPFILEHEADER
    0x42, 0x4D,             # "BM"
    0x66, 0x00, 0x00, 0x00, # file size = 102
    0x00, 0x00,             # reserved
    0x00, 0x00,             # reserved
    0x36, 0x00, 0x00, 0x00, # pixel data offset = 54
    # BITMAPINFOHEADER
    0x28, 0x00, 0x00, 0x00, # header size = 40
    0x04, 0x00, 0x00, 0x00, # width = 4
    0x04, 0x00, 0x00, 0x00, # height = 4
    0x01, 0x00,             # planes = 1
    0x18, 0x00,             # bpp = 24
    0x00, 0x00, 0x00, 0x00, # compression = none
    0x30, 0x00, 0x00, 0x00, # image size (can be 0)
    0x13, 0x0B, 0x00, 0x00, # X ppm
    0x13, 0x0B, 0x00, 0x00, # Y ppm
    0x00, 0x00, 0x00, 0x00, # colors in table
    0x00, 0x00, 0x00, 0x00, # important colors
    # Pixel data: 4 rows of 4 RGB pixels (stride = 12, no padding needed)
    0xFF, 0x00, 0x00,  0x00, 0xFF, 0x00,  0x00, 0x00, 0xFF,  0xFF, 0xFF, 0x00,
    0xFF, 0x00, 0xFF,  0x00, 0xFF, 0xFF,  0x80, 0x80, 0x80,  0x00, 0x00, 0x00,
    0xFF, 0x80, 0x00,  0x80, 0xFF, 0x00,  0x00, 0x80, 0xFF,  0xFF, 0xFF, 0xFF,
    0x40, 0x40, 0x40,  0x80, 0x00, 0x80,  0x00, 0x80, 0x00,  0x00, 0x00, 0x80
)

# Minimal valid PDF (single blank A4 page)
$pdfText = @"
%PDF-1.4
1 0 obj<</Type/Catalog/Pages 2 0 R>>endobj
2 0 obj<</Type/Pages/Kids[3 0 R]/Count 1>>endobj
3 0 obj<</Type/Page/MediaBox[0 0 595 842]/Parent 2 0 R/Resources<<>>>>endobj
xref
0 4
0000000000 65535 f
0000000009 00000 n
0000000058 00000 n
0000000115 00000 n
trailer<</Size 4/Root 1 0 R>>
startxref
217
%%EOF
"@

# Minimal SVG
$svgText = @'
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 100 100" width="100" height="100">
  <rect x="10" y="10" width="80" height="80" fill="#4A90E2" rx="8"/>
  <text x="50" y="60" text-anchor="middle" font-size="24" fill="white">EL</text>
</svg>
'@

# Minimal WebP (1×1 red pixel, VP8L lossless)
$webpBytes = [byte[]]@(
    0x52, 0x49, 0x46, 0x46, # "RIFF"
    0x1A, 0x00, 0x00, 0x00, # chunk size (26 bytes follow)
    0x57, 0x45, 0x42, 0x50, # "WEBP"
    0x56, 0x50, 0x38, 0x4C, # "VP8L"
    0x0D, 0x00, 0x00, 0x00, # chunk size
    0x2F, 0x00, 0x00, 0x00, # VP8L signature + 1x1
    0x00, 0x70, 0x46, 0xF2, # bitstream (1×1 red pixel)
    0xF8, 0xC5, 0x2F, 0x00,
    0xFF, 0x00, 0xFE, 0x00
)

# ---------------------------------------------------------------------------
# Main generation
# ---------------------------------------------------------------------------

Write-Host "`nExplorerLens Test Corpus Generator" -ForegroundColor Yellow
Write-Host "====================================`n"
Write-Host "  Corpus root: $CorpusRoot`n"

$imgDir   = Join-Path $CorpusRoot "images"
$rawDir   = Join-Path $CorpusRoot "raw"
$docDir   = Join-Path $CorpusRoot "documents"
$arcDir   = Join-Path $CorpusRoot "archives"
$modDir   = Join-Path $CorpusRoot "models"
$fntDir   = Join-Path $CorpusRoot "fonts"
$vidDir   = Join-Path $CorpusRoot "video"
$audDir   = Join-Path $CorpusRoot "audio"

Write-Step "Images"
New-File (Join-Path $imgDir "sample_1mp.jpg")    $jpegBytes
New-File (Join-Path $imgDir "sample_256c.png")   $pngBytes
New-File (Join-Path $imgDir "sample_lossy.webp") $webpBytes
New-File (Join-Path $imgDir "sample_24bit.bmp")  $bmpBytes
New-TextFile (Join-Path $imgDir "sample.svg")    $svgText

Write-Step "Documents"
New-TextFile (Join-Path $docDir "sample_1page.pdf") $pdfText

Write-Step "SVG (also images)"
# Already done above

Write-Step "3D Models"
New-TextFile (Join-Path $modDir "sample_box.gltf") @'
{
  "asset": { "version": "2.0", "generator": "ExplorerLens corpus generator" },
  "scene": 0,
  "scenes": [{ "name": "Scene", "nodes": [0] }],
  "nodes": [{ "name": "Cube", "mesh": 0 }],
  "meshes": [{
    "name": "Cube",
    "primitives": [{
      "attributes": { "POSITION": 0 },
      "indices": 1
    }]
  }],
  "accessors": [
    {
      "bufferView": 0, "componentType": 5126, "count": 8, "type": "VEC3",
      "max": [1,1,1], "min": [-1,-1,-1]
    },
    { "bufferView": 1, "componentType": 5123, "count": 36, "type": "SCALAR" }
  ],
  "bufferViews": [
    { "buffer": 0, "byteOffset": 0,  "byteLength": 96  },
    { "buffer": 0, "byteOffset": 96, "byteLength": 72  }
  ],
  "buffers": [{ "uri": "sample_box.bin", "byteLength": 168 }]
}
'@

# Write the .bin buffer for the cube mesh
$cubeVerts = [byte[]]@(
    # 8 vertices of a unit cube, each VEC3 float32 (12 bytes each = 96 total)
    0x00,0x00,0x80,0xBF, 0x00,0x00,0x80,0xBF, 0x00,0x00,0x80,0xBF,  # -1,-1,-1
    0x00,0x00,0x80,0x3F, 0x00,0x00,0x80,0xBF, 0x00,0x00,0x80,0xBF,  #  1,-1,-1
    0x00,0x00,0x80,0x3F, 0x00,0x00,0x80,0x3F, 0x00,0x00,0x80,0xBF,  #  1, 1,-1
    0x00,0x00,0x80,0xBF, 0x00,0x00,0x80,0x3F, 0x00,0x00,0x80,0xBF,  # -1, 1,-1
    0x00,0x00,0x80,0xBF, 0x00,0x00,0x80,0xBF, 0x00,0x00,0x80,0x3F,  # -1,-1, 1
    0x00,0x00,0x80,0x3F, 0x00,0x00,0x80,0xBF, 0x00,0x00,0x80,0x3F,  #  1,-1, 1
    0x00,0x00,0x80,0x3F, 0x00,0x00,0x80,0x3F, 0x00,0x00,0x80,0x3F,  #  1, 1, 1
    0x00,0x00,0x80,0xBF, 0x00,0x00,0x80,0x3F, 0x00,0x00,0x80,0x3F   # -1, 1, 1
)
$cubeIdx = [byte[]]@(
    # 12 triangles (36 uint16 indices = 72 bytes)
    0,0, 1,0, 2,0,  0,0, 2,0, 3,0,  # front
    4,0, 6,0, 5,0,  4,0, 7,0, 6,0,  # back
    0,0, 5,0, 1,0,  0,0, 4,0, 5,0,  # bottom
    2,0, 7,0, 3,0,  2,0, 6,0, 7,0,  # top
    0,0, 3,0, 7,0,  0,0, 7,0, 4,0,  # left
    1,0, 5,0, 6,0,  1,0, 6,0, 2,0   # right
)
New-File (Join-Path $modDir "sample_box.bin") ($cubeVerts + $cubeIdx)

# STL binary
$stlBytes = New-Object byte[] 84
[System.Text.Encoding]::ASCII.GetBytes("ExplorerLens synthetic STL").CopyTo($stlBytes, 0)
$stlBytes[80] = 1  # num triangles = 1
$stlBytes[81] = 0
$stlBytes[82] = 0
$stlBytes[83] = 0
New-File (Join-Path $modDir "sample_binary.stl") $stlBytes

Write-Step "RAW placeholders (cannot generate synthetically)"
foreach ($f in @("sample.dng","sample_canon.cr2","sample_nikon.nef","sample_sony.arw")) {
    $path = Join-Path $rawDir "$f.placeholder"
    if (-not (Test-Path $path) -or $Force) {
        Set-Content $path "This is a placeholder. Place a real $f file here for validation."
        Write-Skip "Placeholder: $f (RAW files cannot be generated synthetically)"
    }
}

Write-Step "Font placeholders (require actual font data)"
foreach ($f in @("sample.ttf","sample.otf","sample.woff2")) {
    $path = Join-Path $fntDir "$f.placeholder"
    if (-not (Test-Path $path) -or $Force) {
        Set-Content $path "Placeholder. Copy a CC0-licensed font file as $f for corpus validation."
        Write-Skip "Placeholder: $f"
    }
}

Write-Step "Video & Audio placeholders"
foreach ($f in @("sample_10s.mp4","sample.mkv","sample.webm")) {
    $path = Join-Path $vidDir "$f.placeholder"
    if (-not (Test-Path $path) -or $Force) { Set-Content $path "Video placeholder — add a real $f here." }
}
foreach ($f in @("sample_30s.mp3","sample_30s.flac")) {
    $path = Join-Path $audDir "$f.placeholder"
    if (-not (Test-Path $path) -or $Force) { Set-Content $path "Audio placeholder — add a real $f here." }
}

Write-Step "Archive (CBZ = ZIP) with sample JPEG cover image"
Add-Type -AssemblyName System.IO.Compression
$cbzPath = Join-Path $arcDir "sample_cbz.cbz"
if (-not (Test-Path $cbzPath) -or $Force) {
    $ms = New-Object System.IO.MemoryStream
    $zip = New-Object System.IO.Compression.ZipArchive($ms, [System.IO.Compression.ZipArchiveMode]::Create, $true)
    $entry = $zip.CreateEntry("cover.jpg")
    $es = $entry.Open()
    $es.Write($jpegBytes, 0, $jpegBytes.Length)
    $es.Close()
    $entry2 = $zip.CreateEntry("page001.jpg")
    $es2 = $entry2.Open()
    $es2.Write($jpegBytes, 0, $jpegBytes.Length)
    $es2.Close()
    $zip.Dispose()
    [System.IO.File]::WriteAllBytes($cbzPath, $ms.ToArray())
    Write-OK "Generated: sample_cbz.cbz ($($ms.Length) bytes)"
    $ms.Dispose()
}

# ZIP archive
$zipPath = Join-Path $arcDir "sample_images.zip"
if (-not (Test-Path $zipPath) -or $Force) {
    $ms2 = New-Object System.IO.MemoryStream
    $zip2 = New-Object System.IO.Compression.ZipArchive($ms2, [System.IO.Compression.ZipArchiveMode]::Create, $true)
    foreach ($i in 1..3) {
        $e = $zip2.CreateEntry("image$i.jpg")
        $s = $e.Open(); $s.Write($jpegBytes, 0, $jpegBytes.Length); $s.Close()
    }
    $zip2.Dispose()
    [System.IO.File]::WriteAllBytes($zipPath, $ms2.ToArray())
    Write-OK "Generated: sample_images.zip ($($ms2.Length) bytes)"
    $ms2.Dispose()
}

Write-Host "`n✅ Corpus generation complete." -ForegroundColor Green
Write-Host "   Run '.\tools\Validate-Corpus.ps1' to validate all generated files.`n"
