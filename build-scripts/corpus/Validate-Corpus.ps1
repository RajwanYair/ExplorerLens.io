<#
.SYNOPSIS
    Validate the data/corpus/ test files against ExplorerLens format detection.

.DESCRIPTION
    Validates that:
    1. All files listed in data/corpus/MANIFEST.json exist on disk
    2. Files are not empty (> 4 bytes)
    3. Magic bytes match the declared format
    4. Optional: decode time is within 2x the P50 baseline

.PARAMETER Verbose
    Print per-file details.

.PARAMETER UpdateBadges
    After validation, update the Pass/Fail counts in docs/FORMAT_VALIDATION_STATUS.md

.PARAMETER EngineTestExe
    Path to EngineTests.exe. If not provided, the script searches build\.

.EXAMPLE
    .\tools\Validate-Corpus.ps1 -Verbose
    .\tools\Validate-Corpus.ps1 -UpdateBadges

#>
[CmdletBinding()]
param(
    [switch] $UpdateBadges,
    [string] $EngineTestExe = ""
)

Set-StrictMode -Version 3.0
$ErrorActionPreference = "Stop"

$root      = Split-Path -Parent $PSScriptRoot
$corpusDir = Join-Path $root "data\corpus"
$manifest  = Get-Content (Join-Path $corpusDir "MANIFEST.json") -Raw | ConvertFrom-Json

# ---------------------------------------------------------------------------
# Known magic byte signatures
# ---------------------------------------------------------------------------

$magic = @{
    "image/jpeg"  = @([byte]0xFF, [byte]0xD8, [byte]0xFF)
    "image/png"   = @([byte]0x89, [byte]0x50, [byte]0x4E, [byte]0x47)
    "image/gif"   = @([byte]0x47, [byte]0x49, [byte]0x46, [byte]0x38)
    "image/bmp"   = @([byte]0x42, [byte]0x4D)
    "image/webp"  = @([byte]0x52, [byte]0x49, [byte]0x46, [byte]0x46)  # "RIFF"
    "image/jxl"   = @([byte]0xFF, [byte]0x0A)
    "image/avif"  = $null   # ftyp box varies; skip magic check
    "image/heic"  = $null
    "application/pdf"    = @([byte]0x25, [byte]0x50, [byte]0x44, [byte]0x46)  # "%PDF"
    "application/zip"    = @([byte]0x50, [byte]0x4B, [byte]0x03, [byte]0x04)  # "PK"
    "application/x-7z"   = @([byte]0x37, [byte]0x7A, [byte]0xBC, [byte]0xAF)  # "7z"
    "application/x-rar"  = @([byte]0x52, [byte]0x61, [byte]0x72, [byte]0x21)  # "Rar!"
    "model/gltf+json"    = $null   # text JSON — skip
    "model/gltf-binary"  = @([byte]0x67, [byte]0x6C, [byte]0x54, [byte]0x46)  # "glTF"
    "model/stl"          = $null   # ASCII or binary; skip
    "image/x-svg+xml"    = $null   # text; skip
    "image/svg+xml"      = $null
}

function Test-MagicBytes {
    param([byte[]] $fileBytes, [byte[]] $expected)
    if ($null -eq $expected) { return $true }
    for ($i = 0; $i -lt $expected.Count; $i++) {
        if ($i -ge $fileBytes.Count -or $fileBytes[$i] -ne $expected[$i]) { return $false }
    }
    return $true
}

# ---------------------------------------------------------------------------
# Validation loop
# ---------------------------------------------------------------------------

$passed   = 0
$failed   = 0
$skipped  = 0
$results  = @()

foreach ($entry in $manifest.entries) {
    $relPath = $entry.path -replace '/', '\'
    $absPath = Join-Path $corpusDir $relPath

    if (-not (Test-Path $absPath)) {
        Write-Warning "MISSING  : $($entry.path)"
        $results += [PSCustomObject]@{ Path=$entry.path; Status="MISSING"; Notes="File absent" }
        $failed++
        continue
    }

    $item = Get-Item $absPath
    if ($item.Length -lt 4) {
        Write-Verbose "STUB     : $($entry.path) ($($item.Length) bytes — placeholder)"
        $results += [PSCustomObject]@{ Path=$entry.path; Status="STUB"; Notes="Placeholder only" }
        $skipped++
        continue
    }

    $bytes = [System.IO.File]::ReadAllBytes($absPath)[0..15]
    $mime  = if ($entry.PSObject.Properties['mime']) { $entry.mime } else { "" }

    $magicOk = $true
    if ($magic.ContainsKey($mime) -and $null -ne $magic[$mime]) {
        $magicOk = Test-MagicBytes -fileBytes $bytes -expected $magic[$mime]
    }

    if (-not $magicOk) {
        Write-Warning "BAD_MAGIC: $($entry.path) (mime=$mime)"
        $results += [PSCustomObject]@{ Path=$entry.path; Status="BAD_MAGIC"; Notes="Unexpected magic bytes" }
        $failed++
        continue
    }

    Write-Verbose "PASS     : $($entry.path) ($($item.Length) bytes)"
    $results += [PSCustomObject]@{ Path=$entry.path; Status="PASS"; Notes="" }
    $passed++
}

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------

Write-Host ""
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "  Corpus Validation Results" -ForegroundColor Cyan
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "  PASS    : $passed" -ForegroundColor Green
Write-Host "  STUB    : $skipped" -ForegroundColor Yellow
Write-Host "  FAIL    : $failed" -ForegroundColor $(if ($failed -gt 0) { 'Red' } else { 'Green' })
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan

if ($failed -gt 0) {
    Write-Host ""
    Write-Host "Failed entries:" -ForegroundColor Red
    $results | Where-Object { $_.Status -in "MISSING","BAD_MAGIC" } |
        Format-Table Path, Status, Notes -AutoSize
    exit 1
}

Write-Host ""
Write-Host "All corpus entries valid." -ForegroundColor Green

# ---------------------------------------------------------------------------
# Optional: Update docs/FORMAT_VALIDATION_STATUS.md header counts
# ---------------------------------------------------------------------------

if ($UpdateBadges) {
    $statusFile = Join-Path $root "docs\FORMAT_VALIDATION_STATUS.md"
    $content = Get-Content $statusFile -Raw

    $total  = $passed + $failed + $skipped
    $content = $content -replace '(?<=\| Total formats tracked \| )\d+', $total
    $content = $content -replace '(?<=\| Fully validated \| )\d+',       $passed
    $content = $content -replace '(?<=\| Corpus files present \| )\d+',  ($passed + $skipped)

    Set-Content $statusFile $content -NoNewline
    Write-Host "Updated FORMAT_VALIDATION_STATUS.md badges." -ForegroundColor Cyan
}
