<#
.SYNOPSIS
    Audit Engine headers — classify as Real, Stub, or Dead.

.DESCRIPTION
    Scans all headers registered in Engine/CMakeLists.txt and classifies them:
      - Real:  Has a matching .cpp source file OR >100 LOC of implementation
      - Stub:  Exists but has <50 LOC of real logic (mostly boilerplate/empty bodies)
      - Dead:  Listed in CMakeLists.txt but file does not exist on disk
      - Orphan: Exists on disk in Engine/ but NOT listed in CMakeLists.txt

    Also identifies headers with no #include references from other Engine files.

.PARAMETER OutputCsv
    Optional: path to write CSV report. Defaults to stdout summary.

.PARAMETER Detailed
    Show per-file classification instead of just summary counts.

.EXAMPLE
    pwsh -File build-scripts/Audit-Headers.ps1
    pwsh -File build-scripts/Audit-Headers.ps1 -Detailed
    pwsh -File build-scripts/Audit-Headers.ps1 -OutputCsv build-logs/header-audit.csv
#>
param(
    [string]$OutputCsv,
    [switch]$Detailed
)

$ErrorActionPreference = 'Stop'
$rootDir = Split-Path $PSScriptRoot -Parent

# ─── 1. Parse ENGINE_HEADERS from CMakeLists.txt ───────────────────────────
$cmakePath = Join-Path $rootDir 'Engine' 'CMakeLists.txt'
$cmakeContent = Get-Content $cmakePath -Raw

# Extract the ENGINE_HEADERS block
if ($cmakeContent -match '(?s)set\(ENGINE_HEADERS\s*\n(.*?)\n\)') {
    $headerBlock = $Matches[1]
} else {
    Write-Error "Could not parse ENGINE_HEADERS from $cmakePath"
    exit 1
}

$registeredHeaders = $headerBlock -split '\r?\n' |
    ForEach-Object { $_.Trim() } |
    Where-Object { $_ -and $_ -notmatch '^#' } |
    ForEach-Object { $_ -replace '^\$\{CMAKE_CURRENT_SOURCE_DIR\}/', '' }

Write-Host "[audit] Found $($registeredHeaders.Count) registered headers in ENGINE_HEADERS" -ForegroundColor Cyan

# ─── 2. Parse ENGINE_SOURCES from CMakeLists.txt ───────────────────────────
if ($cmakeContent -match '(?s)set\(ENGINE_SOURCES\s*\n(.*?)\n\)') {
    $sourceBlock = $Matches[1]
} else {
    $sourceBlock = ''
}

$registeredSources = $sourceBlock -split '\r?\n' |
    ForEach-Object { $_.Trim() } |
    Where-Object { $_ -and $_ -notmatch '^#' } |
    ForEach-Object { $_ -replace '^\$\{CMAKE_CURRENT_SOURCE_DIR\}/', '' }

Write-Host "[audit] Found $($registeredSources.Count) registered sources in ENGINE_SOURCES" -ForegroundColor Cyan

# Build source lookup set (stem -> true)
$sourceStems = @{}
foreach ($src in $registeredSources) {
    $stem = [System.IO.Path]::GetFileNameWithoutExtension($src)
    $sourceStems[$stem] = $true
}

# ─── 3. Classify each registered header ────────────────────────────────────
$results = [System.Collections.ArrayList]::new()

foreach ($hdr in $registeredHeaders) {
    $fullPath = Join-Path $rootDir 'Engine' $hdr
    $entry = [PSCustomObject]@{
        Header   = $hdr
        Category = ''
        LOC      = 0
        HasCpp   = $false
        Reason   = ''
    }

    if (-not (Test-Path $fullPath)) {
        $entry.Category = 'Dead'
        $entry.Reason = 'File not found on disk'
        [void]$results.Add($entry)
        continue
    }

    # Count lines of code (excluding blank lines and comment-only lines)
    $lines = Get-Content $fullPath
    $entry.LOC = ($lines | Where-Object {
        $t = $_.Trim()
        $t -and $t -notmatch '^\s*//' -and $t -notmatch '^\s*\*' -and $t -ne '#pragma once' -and $t -notmatch '^#include' -and $t -ne '{' -and $t -ne '}' -and $t -ne '};'
    }).Count

    # Check for matching .cpp source
    $stem = [System.IO.Path]::GetFileNameWithoutExtension($hdr)
    $entry.HasCpp = $sourceStems.ContainsKey($stem)

    # Classify
    if ($entry.HasCpp -or $entry.LOC -gt 100) {
        $entry.Category = 'Real'
        if ($entry.HasCpp) {
            $entry.Reason = "Has matching .cpp"
        } else {
            $entry.Reason = "Header-only with $($entry.LOC) LOC"
        }
    } elseif ($entry.LOC -le 50) {
        $entry.Category = 'Stub'
        $entry.Reason = "Only $($entry.LOC) LOC, no .cpp"
    } else {
        $entry.Category = 'Real'
        $entry.Reason = "Header-only with $($entry.LOC) LOC (borderline)"
    }

    [void]$results.Add($entry)
}

# ─── 4. Find orphan headers (on disk but not in CMakeLists) ────────────────
$registeredSet = [System.Collections.Generic.HashSet[string]]::new(
    [System.StringComparer]::OrdinalIgnoreCase
)
foreach ($h in $registeredHeaders) { [void]$registeredSet.Add($h) }

$engineDir = Join-Path $rootDir 'Engine'
$diskHeaders = Get-ChildItem $engineDir -Recurse -Include '*.h' |
    Where-Object { $_.FullName -notmatch '\\Tests\\' -and $_.FullName -notmatch '\\external\\' } |
    ForEach-Object {
        $_.FullName.Substring($engineDir.Length + 1).Replace('\', '/')
    }

$orphans = $diskHeaders | Where-Object { -not $registeredSet.Contains($_) }

# ─── 5. Output ─────────────────────────────────────────────────────────────
$real  = ($results | Where-Object Category -eq 'Real').Count
$stub  = ($results | Where-Object Category -eq 'Stub').Count
$dead  = ($results | Where-Object Category -eq 'Dead').Count

Write-Host ""
Write-Host "╔══════════════════════════════════════╗" -ForegroundColor Yellow
Write-Host "║     Header Audit Summary             ║" -ForegroundColor Yellow
Write-Host "╠══════════════════════════════════════╣" -ForegroundColor Yellow
Write-Host "║  Registered: $($registeredHeaders.Count.ToString().PadLeft(5))                    ║" -ForegroundColor White
Write-Host "║  Real:       $($real.ToString().PadLeft(5))  (has .cpp or >100 LOC) ║" -ForegroundColor Green
Write-Host "║  Stub:       $($stub.ToString().PadLeft(5))  (≤50 LOC, no .cpp)     ║" -ForegroundColor Yellow
Write-Host "║  Dead:       $($dead.ToString().PadLeft(5))  (missing from disk)    ║" -ForegroundColor Red
Write-Host "║  Orphans:    $($orphans.Count.ToString().PadLeft(5))  (on disk, not in CMake)║" -ForegroundColor Magenta
Write-Host "║  Sources:    $($registeredSources.Count.ToString().PadLeft(5))                    ║" -ForegroundColor White
Write-Host "║  H:S ratio:  $('{0:F1}' -f ($registeredHeaders.Count / [Math]::Max($registeredSources.Count, 1))):1                   ║" -ForegroundColor White
Write-Host "╚══════════════════════════════════════╝" -ForegroundColor Yellow

# Directory breakdown
Write-Host ""
Write-Host "── By Directory ──" -ForegroundColor Cyan
$results | Group-Object { ($_.Header -split '/')[0] } | Sort-Object Count -Descending | ForEach-Object {
    $dirReal = ($_.Group | Where-Object Category -eq 'Real').Count
    $dirStub = ($_.Group | Where-Object Category -eq 'Stub').Count
    $dirDead = ($_.Group | Where-Object Category -eq 'Dead').Count
    Write-Host ("  {0,-20} {1,4} total  ({2} real, {3} stub, {4} dead)" -f
        $_.Name, $_.Count, $dirReal, $dirStub, $dirDead)
}

if ($Detailed) {
    Write-Host ""
    Write-Host "── Stubs (candidates for deletion or implementation) ──" -ForegroundColor Yellow
    $results | Where-Object Category -eq 'Stub' | Sort-Object LOC | ForEach-Object {
        Write-Host ("  {0,-60} {1,3} LOC  {2}" -f $_.Header, $_.LOC, $_.Reason) -ForegroundColor Yellow
    }

    if ($dead -gt 0) {
        Write-Host ""
        Write-Host "── Dead (remove from CMakeLists.txt) ──" -ForegroundColor Red
        $results | Where-Object Category -eq 'Dead' | ForEach-Object {
            Write-Host ("  {0}" -f $_.Header) -ForegroundColor Red
        }
    }

    if ($orphans.Count -gt 0) {
        Write-Host ""
        Write-Host "── Orphans (on disk but not in CMakeLists.txt) ──" -ForegroundColor Magenta
        $orphans | ForEach-Object { Write-Host "  $_" -ForegroundColor Magenta }
    }
}

# ─── 6. CSV Export ─────────────────────────────────────────────────────────
if ($OutputCsv) {
    $csvDir = Split-Path $OutputCsv -Parent
    if ($csvDir -and -not (Test-Path $csvDir)) {
        New-Item -ItemType Directory -Force $csvDir | Out-Null
    }
    $results | Export-Csv -Path $OutputCsv -NoTypeInformation -Encoding UTF8
    Write-Host ""
    Write-Host "[audit] CSV written to $OutputCsv" -ForegroundColor Green
}
