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

.PARAMETER DryRun
    List actions that would be taken without hitting the network or touching files.

.PARAMETER ReportOnly
    Report manifest status (which files exist, which are missing, license summary)
    without downloading.  Exit code 0 if all files present; 2 if any missing.

.PARAMETER StrictLicenses
    Fail if the manifest contains any entry with a non-permissive license.
    Accepted: CC0, CC-BY, CC-BY-SA, Public Domain, Public-Domain, SIL-OFL,
    MIT, BSD-2-Clause, BSD-3-Clause, Apache-2.0.

.EXAMPLE
    .\build-scripts\corpus\Fetch-Corpus.ps1
    .\build-scripts\corpus\Fetch-Corpus.ps1 -Formats PNG,AVIF,WEBP
    .\build-scripts\corpus\Fetch-Corpus.ps1 -ForceRefresh
    .\build-scripts\corpus\Fetch-Corpus.ps1 -DryRun
    .\build-scripts\corpus\Fetch-Corpus.ps1 -ReportOnly
    .\build-scripts\corpus\Fetch-Corpus.ps1 -StrictLicenses

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
    [string[]]$Formats     = @(),
    [switch]$DryRun,
    [switch]$ReportOnly,
    [switch]$StrictLicenses
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

# ── License validation (S176 enhancement) ───────────────────────────────────
$permissiveLicenses = @(
    'CC0', 'CC0-1.0', 'CC-BY', 'CC-BY-4.0', 'CC-BY-SA', 'CC-BY-SA-4.0',
    'Public Domain', 'Public-Domain', 'PD',
    'SIL-OFL', 'OFL-1.1',
    'MIT', 'BSD-2-Clause', 'BSD-3-Clause', 'Apache-2.0', 'Zlib'
)

$licenseIssues = @()
foreach ($entry in $allFiles) {
    $lic = if ($entry.PSObject.Properties.Name -contains 'license') { $entry.license } else { '' }
    if ([string]::IsNullOrWhiteSpace($lic)) {
        $licenseIssues += "MISSING license: $($entry.format) $($entry.filename)"
    }
    elseif ($permissiveLicenses -notcontains $lic) {
        $licenseIssues += "NON-PERMISSIVE license '$lic': $($entry.format) $($entry.filename)"
    }
}

if ($licenseIssues.Count -gt 0) {
    Write-Host "License audit issues:" -ForegroundColor Yellow
    $licenseIssues | ForEach-Object { Write-Host "  $_" -ForegroundColor Yellow }
    Write-Host ""
    if ($StrictLicenses) {
        Write-Error "Strict license mode: $($licenseIssues.Count) manifest entries failed license audit."
        exit 3
    }
}
else {
    Write-Host "License audit: all $($allFiles.Count) entries use permissive licenses." -ForegroundColor Green
    Write-Host ""
}

# ── ReportOnly mode (S176 enhancement) ──────────────────────────────────────
if ($ReportOnly) {
    $present = 0; $missing = 0; $mismatch = 0
    foreach ($entry in $allFiles) {
        $dest = Join-Path $CorpusDir $entry.path
        if (-not (Test-Path $dest)) {
            Write-Host "  MISSING  $($entry.format.PadRight(6)) $($entry.filename)" -ForegroundColor Red
            $missing++
            continue
        }
        $actual = (Get-FileHash $dest -Algorithm SHA256).Hash
        if ($actual -ne $entry.sha256.ToUpper()) {
            Write-Host "  MISMATCH $($entry.format.PadRight(6)) $($entry.filename)" -ForegroundColor Yellow
            $mismatch++
        }
        else {
            Write-Host "  OK       $($entry.format.PadRight(6)) $($entry.filename)" -ForegroundColor DarkGray
            $present++
        }
    }
    Write-Host ""
    Write-Host "Report: $present OK, $mismatch mismatched, $missing missing" `
               -ForegroundColor $(if (($missing + $mismatch) -gt 0) { 'Yellow' } else { 'Green' })
    if (($missing + $mismatch) -gt 0) { exit 2 }
    exit 0
}

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
$ok = 0; $skipped = 0; $errors = 0; $dryRunCount = 0

foreach ($entry in $allFiles) {
    $dest = Join-Path $CorpusDir $entry.path

    $label = "$($entry.format.PadRight(6)) $($entry.filename)"

    if ($DryRun) {
        if ((Test-Path $dest) -and (-not $ForceRefresh)) {
            Write-Host "  DRY-SKIP  $label" -ForegroundColor DarkGray
        }
        else {
            Write-Host "  DRY-GET   $label  <- $($entry.url)" -ForegroundColor Cyan
            $dryRunCount++
        }
        continue
    }

    $result = Get-CorpusFile -Url $entry.url -Dest $dest `
                             -ExpectedSha256 $entry.sha256 -ForceRefresh $ForceRefresh.IsPresent

    switch ($result) {
        'ok'    { Write-Host "  OK      $label" -ForegroundColor Green; $ok++ }
        'skip'  { Write-Host "  SKIP    $label" -ForegroundColor DarkGray; $skipped++ }
        'error' { Write-Host "  ERROR   $label" -ForegroundColor Red; $errors++ }
    }
}

Write-Host ""
if ($DryRun) {
    Write-Host "Dry run: $dryRunCount file(s) would be downloaded." -ForegroundColor Cyan
    exit 0
}
Write-Host "Done — $ok downloaded, $skipped skipped (cached), $errors error(s)" `
           -ForegroundColor $(if ($errors -gt 0) { 'Yellow' } else { 'Green' })

if ($errors -gt 0) { exit 1 }
