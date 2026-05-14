#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Validates all pre-release gates before publishing a new ExplorerLens version.

.PARAMETER Version
    Expected version string (e.g. "36.0.0"). Compared against VERSION file.

.PARAMETER SkipBuild
    Skip the Engine build step (assume binaries already present).

.PARAMETER SkipTests
    Skip test execution (assume tests already passed).

.EXAMPLE
    .\tools\Check-Release-Readiness.ps1 -Version "36.0.0"
#>
param(
    [string]$Version = "",
    [switch]$SkipBuild,
    [switch]$SkipTests
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root    = Split-Path -Parent $PSScriptRoot
$passed  = 0
$failed  = 0
$results = [System.Collections.Generic.List[PSCustomObject]]::new()

function Check {
    param([string]$Name, [scriptblock]$Block)
    try {
        $msg = & $Block
        $results.Add([PSCustomObject]@{ Status = "PASS"; Gate = $Name; Detail = $msg })
        $script:passed++
    } catch {
        $results.Add([PSCustomObject]@{ Status = "FAIL"; Gate = $Name; Detail = $_.Exception.Message })
        $script:failed++
    }
}

Write-Host "`n=== ExplorerLens Release Readiness Check ===" -ForegroundColor Cyan
Write-Host "Root: $root`n"

# ── 1. VERSION file matches expectation ────────────────────────────────────────
Check "VERSION file" {
    $vFile = Get-Content "$root\VERSION" -Raw -ErrorAction Stop
    $vFile = $vFile.Trim()
    if ($Version -and $vFile -ne $Version) {
        throw "VERSION='$vFile' but expected '$Version'"
    }
    "VERSION=$vFile"
}

# ── 2. CHANGELOG has entry for expected version ─────────────────────────────────
Check "CHANGELOG entry" {
    $cl   = Get-Content "$root\CHANGELOG.md" -Raw -ErrorAction Stop
    $vCheck = if ($Version) { $Version } else { (Get-Content "$root\VERSION" -Raw).Trim() }
    if ($cl -notmatch [regex]::Escape("## [$vCheck]")) {
        throw "CHANGELOG.md has no ## [$vCheck] section"
    }
    "Found ## [$vCheck] in CHANGELOG.md"
}

# ── 3. CMakeLists.txt version matches ──────────────────────────────────────────
Check "CMakeLists.txt version" {
    $cmake = Get-Content "$root\Engine\CMakeLists.txt" -Raw -ErrorAction Stop
    $vCheck = if ($Version) { $Version } else { (Get-Content "$root\VERSION" -Raw).Trim() }
    if ($cmake -notmatch [regex]::Escape($vCheck)) {
        throw "Engine\CMakeLists.txt does not contain version $vCheck"
    }
    "Engine/CMakeLists.txt contains $vCheck"
}

# ── 4. No stale previous version strings ────────────────────────────────────────
Check "No stale version strings" {
    $vFile = (Get-Content "$root\VERSION" -Raw).Trim()
    $parts = $vFile -split '\.'
    if ($parts.Count -eq 3) {
        $patch = [int]$parts[2]
        if ($patch -gt 0) {
            $prev = "$($parts[0]).$($parts[1]).$($patch - 1)"
            $stale = git -C $root grep -l $prev -- '*.md' '*.h' '*.cpp' '*.json' '*.yaml' '*.rc' 2>&1 |
                     Where-Object { $_ -notmatch '^(CHANGELOG|CHANGELOG-archive)' }
            if ($stale) { throw "Stale version $prev found in: $($stale -join ', ')" }
        }
    }
    "No stale version strings found"
}

# ── 5. Engine build artifacts present ──────────────────────────────────────────
Check "Engine lib present" {
    $lib = "$root\build\lib\ExplorerLensEngine.lib"
    if (-not (Test-Path $lib)) {
        throw "Missing $lib — run Build-MSVC.ps1"
    }
    $sz = [math]::Round((Get-Item $lib).Length / 1MB, 1)
    "ExplorerLensEngine.lib exists (${sz} MB)"
}

# ── 6. Shell DLL present ─────────────────────────────────────────────────────
Check "LENSShell.dll present" {
    $dll = "$root\x64\Release\LENSShell.dll"
    if (-not (Test-Path $dll)) {
        throw "Missing $dll — run msbuild LENSShell.sln"
    }
    $sz = [math]::Round((Get-Item $dll).Length / 1KB, 0)
    "LENSShell.dll exists (${sz} KB)"
}

# ── 7. Test executables present ─────────────────────────────────────────────
Check "Test binaries present" {
    $exe = "$root\build\bin\EngineTests.exe"
    if (-not (Test-Path $exe)) {
        throw "Missing $exe — run Build-MSVC.ps1 -Test"
    }
    "EngineTests.exe exists"
}

# ── 8. Corpus validation ─────────────────────────────────────────────────────
Check "Corpus validates" {
    $script = "$root\tools\Validate-Corpus.ps1"
    if (-not (Test-Path $script)) { throw "tools\Validate-Corpus.ps1 not found" }
    $manifest = "$root\data\corpus\MANIFEST.json"
    if (-not (Test-Path $manifest)) { return "SKIP (no corpus manifest)" }
    $out = & $script -ManifestPath $manifest 2>&1 | Out-String
    if ($LASTEXITCODE -ne 0) { throw "Corpus validation failed: $out" }
    "Corpus validated OK"
}

# ── 9. Package manifests have non-placeholder checksums ───────────────────────
Check "Winget manifest checksum" {
    $f = "$root\packaging\winget\ExplorerLens.yaml"
    if (-not (Test-Path $f)) { return "SKIP (no winget manifest)" }
    $c = Get-Content $f -Raw
    if ($c -match 'REPLACE_WITH_SHA256' -or $c -match '0000000000000000') {
        throw "winget manifest still has placeholder checksum"
    }
    "winget manifest checksum appears set"
}

Check "Chocolatey manifest checksum" {
    $f = "$root\packaging\chocolatey\tools\chocolateyInstall.ps1"
    if (-not (Test-Path $f)) { return "SKIP (no Chocolatey script)" }
    $c = Get-Content $f -Raw
    if ($c -match 'REPLACE_WITH_SHA256_HASH_OF_MSI') {
        throw "Chocolatey install script still has placeholder checksum"
    }
    "Chocolatey install script checksum appears set"
}

# ── 10. No Intel/corporate artefacts ─────────────────────────────────────────
Check "No corporate proxy artefacts" {
    $hits = @()
    foreach ($pat in @('intel\.com', 'REPLACE_PROXY', ':928\b')) {
        $found = git -C $root grep -l $pat -- '*.ps1' '*.yml' '*.yaml' '*.md' '*.json' '*.h' '*.cpp' 2>&1 |
                 Where-Object { $_ -and $_ -notmatch '\.git' }
        $hits += $found
    }
    if ($hits) { throw "Corporate artefacts found in: $($hits | Sort-Object -Unique | Join-String -Separator ', ')" }
    "No corporate proxy artefacts"
}

# ── 11. Git workspace is clean ───────────────────────────────────────────────
Check "Git workspace clean" {
    $status = git -C $root status --porcelain 2>&1 | Out-String
    if ($status.Trim()) {
        throw "Uncommitted changes:`n$status"
    }
    "Working tree clean"
}

# ── 12. Tag does not already exist on remote ──────────────────────────────────
Check "Tag is unique" {
    $vCheck = if ($Version) { "v$Version" } else { "v$((Get-Content "$root\VERSION" -Raw).Trim())" }
    $existing = git -C $root tag -l $vCheck 2>&1 | Out-String
    if ($existing.Trim()) {
        throw "Tag $vCheck already exists locally — bump version before releasing"
    }
    "Tag $vCheck does not yet exist"
}

# ─────────────────────── Summary ─────────────────────────────────────────────
Write-Host ""
$results | Format-Table -AutoSize -Property @{L="Status"; E={
    if ($_.Status -eq "PASS") { Write-Host -NoNewline "  PASS" -ForegroundColor Green; "" }
    else { Write-Host -NoNewline "  FAIL" -ForegroundColor Red; "" }
}}, Gate, Detail

# Re-render properly without colour codes in pipeline
foreach ($r in $results) {
    $colour = if ($r.Status -eq "PASS") { "Green" } else { "Red" }
    Write-Host "  [$($r.Status)] $($r.Gate): $($r.Detail)" -ForegroundColor $colour
}

Write-Host ""
if ($failed -eq 0) {
    Write-Host "=== ALL $passed GATES PASSED — ready to release ===" -ForegroundColor Green
} else {
    Write-Host "=== $failed/$($passed+$failed) GATES FAILED — fix before releasing ===" -ForegroundColor Red
    exit 1
}
