#Requires -Version 7.0
<#
.SYNOPSIS
    Downloads CC0/public-domain corpus files for ExplorerLens decoder testing.

.DESCRIPTION
    Fetches a curated set of test files covering the 25 primary decoder formats.
    Each file is verified against its expected SHA-256 from MANIFEST.json.

    Files are saved to:  data/corpus/<format>/<filename>

    Run this once after cloning, and again when the manifest is updated.
    Safe to re-run: skips files that already exist and pass checksum.

.PARAMETER ManifestPath
    Path to MANIFEST.json.  Default: data/corpus/MANIFEST.json relative to repo root.

.PARAMETER CorpusDir
    Root directory for corpus files.  Default: data/corpus relative to repo root.

.PARAMETER ForceRefresh
    Re-download even files that already pass checksum.

.PARAMETER Formats
    Comma-separated list of format keys to fetch (e.g. 'PNG,AVIF').
    Default: fetch all.

.EXAMPLE
    .\build-scripts\corpus\Fetch-Corpus.ps1
    .\build-scripts\corpus\Fetch-Corpus.ps1 -Formats PNG,AVIF,WEBP
    .\build-scripts\corpus\Fetch-Corpus.ps1 -ForceRefresh

.NOTES
    All files listed in MANIFEST.json must be CC0, public domain, or carry
    a permissive open-source license that permits redistribution.  See each
    file entry's "license" field.
#>
[CmdletBinding(SupportsShouldProcess)]
param(
    [string]$ManifestPath  = '',
    [string]$CorpusDir     = '',
    [switch]$ForceRefresh,
    [string[]]$Formats     = @()
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# ── Locate repo root ─────────────────────────────────────────────────────────
$repoRoot = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent

if (-not $ManifestPath) { $ManifestPath = Join-Path $repoRoot 'data\corpus\MANIFEST.json' }
if (-not $CorpusDir)    { $CorpusDir    = Join-Path $repoRoot 'data\corpus' }

# ── Validate manifest ────────────────────────────────────────────────────────
if (-not (Test-Path $ManifestPath)) {
    Write-Error "Manifest not found: $ManifestPath`nRun from the repo root or pass -ManifestPath."
    exit 1
}

$manifest = Get-Content $ManifestPath -Raw | ConvertFrom-Json
$allFiles = $manifest.files

if ($Formats.Count -gt 0) {
    $fmtSet   = $Formats | ForEach-Object { $_.ToUpper() }
    $allFiles = $allFiles | Where-Object { $fmtSet -contains $_.format.ToUpper() }
    Write-Host "Filtering to formats: $($fmtSet -join ', ')" -ForegroundColor Cyan
}

Write-Host "Corpus root : $CorpusDir" -ForegroundColor Cyan
Write-Host "Total files : $($allFiles.Count)" -ForegroundColor Cyan
Write-Host ""

# ── Download helper ─────────────────────────────────────────────────────────
function Get-CorpusFile {
    param(
        [string]$Url,
        [string]$Dest,
        [string]$ExpectedSha256,
        [bool]  $ForceRefresh
    )

    # Skip if already valid
    if ((Test-Path $Dest) -and (-not $ForceRefresh)) {
        $actual = (Get-FileHash $Dest -Algorithm SHA256).Hash
        if ($actual -eq $ExpectedSha256.ToUpper()) {
            return 'skip'
        }
        Write-Warning "Checksum mismatch for $(Split-Path $Dest -Leaf) — re-downloading"
    }

    $dir = Split-Path $Dest -Parent
    if (-not (Test-Path $dir)) { New-Item $dir -ItemType Directory -Force | Out-Null }

    try {
        Invoke-WebRequest -Uri $Url -OutFile $Dest -UseBasicParsing -TimeoutSec 60
    }
    catch {
        Write-Warning "Failed to download: $Url`n  $_"
        return 'error'
    }

    $actual = (Get-FileHash $Dest -Algorithm SHA256).Hash
    if ($actual -ne $ExpectedSha256.ToUpper()) {
        Write-Warning "Checksum mismatch after download: $Dest`n  Expected: $ExpectedSha256`n  Got:      $actual"
        Remove-Item $Dest -Force -ErrorAction SilentlyContinue
        return 'error'
    }

    return 'ok'
}

# ── Main download loop ───────────────────────────────────────────────────────
$ok = 0; $skipped = 0; $errors = 0

foreach ($entry in $allFiles) {
    $dest = Join-Path $CorpusDir $entry.path

    $label = "$($entry.format.PadRight(6)) $($entry.filename)"
    $result = Get-CorpusFile -Url $entry.url -Dest $dest `
                             -ExpectedSha256 $entry.sha256 -ForceRefresh $ForceRefresh.IsPresent

    switch ($result) {
        'ok'    { Write-Host "  OK      $label" -ForegroundColor Green; $ok++ }
        'skip'  { Write-Host "  SKIP    $label" -ForegroundColor DarkGray; $skipped++ }
        'error' { Write-Host "  ERROR   $label" -ForegroundColor Red; $errors++ }
    }
}

Write-Host ""
Write-Host "Done — $ok downloaded, $skipped skipped (cached), $errors error(s)" `
           -ForegroundColor $(if ($errors -gt 0) { 'Yellow' } else { 'Green' })

if ($errors -gt 0) { exit 1 }
