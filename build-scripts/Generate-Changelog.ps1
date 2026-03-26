#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Generate-Changelog.ps1 — Automated Changelog Generation
    Sprint 35 (v15.6.0 Release Engineering)

.DESCRIPTION
    Generates a structured CHANGELOG snippet from git log between two refs.
    Groups commits by conventional-commit prefix (feat/fix/chore/docs/perf/refactor).
    Outputs Markdown compatible with Keep-a-Changelog format.

.PARAMETER FromRef
    Starting git ref (exclusive). Defaults to most recent tag.

.PARAMETER ToRef
    Ending git ref (inclusive). Defaults to HEAD.

.PARAMETER Version
    Version string for the new section header. Auto-read from VERSION file if omitted.

.PARAMETER OutputFile
    Path to write the Markdown snippet. If omitted, writes to stdout.

.PARAMETER Append
    If specified, prepend the new section into an existing CHANGELOG.md.

.EXAMPLE
    .\Generate-Changelog.ps1 -FromRef v15.5.0 -ToRef HEAD -Version 15.6.0
#>
[CmdletBinding()]
param(
    [string] $FromRef    = "",
    [string] $ToRef      = "HEAD",
    [string] $Version    = "",
    [string] $OutputFile = "",
    [switch] $Append
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$rootDir   = Split-Path -Parent $scriptDir

# ── Resolve Version ───────────────────────────────────────────────────────────
if (-not $Version) {
    $versionFile = Join-Path $rootDir "VERSION"
    if (Test-Path $versionFile) {
        $Version = (Get-Content $versionFile -Raw).Trim()
    } else {
        $Version = "0.0.0"
        Write-Warning "VERSION file not found; using $Version"
    }
}

# ── Resolve FromRef ───────────────────────────────────────────────────────────
if (-not $FromRef) {
    try {
        $FromRef = & git -C $rootDir describe --tags --abbrev=0 2>$null
        if ($LASTEXITCODE -ne 0 -or -not $FromRef) {
            $FromRef = ""
            Write-Warning "No previous tag found; including all commits."
        }
    } catch {
        $FromRef = ""
    }
}

# ── Get commits ───────────────────────────────────────────────────────────────
$range = if ($FromRef) { "$FromRef..$ToRef" } else { $ToRef }
$logFormat = "%H|%s|%an|%ai"
$rawLog = & git -C $rootDir log $range "--pretty=format:$logFormat" 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Error "git log failed: $rawLog"
    exit 1
}

$lines = $rawLog -split "`n" | Where-Object { $_ -match '\|' }

# ── Categorise commits ────────────────────────────────────────────────────────
$categories = [ordered]@{
    "Added"      = [System.Collections.Generic.List[string]]::new()
    "Fixed"      = [System.Collections.Generic.List[string]]::new()
    "Changed"    = [System.Collections.Generic.List[string]]::new()
    "Performance"= [System.Collections.Generic.List[string]]::new()
    "Security"   = [System.Collections.Generic.List[string]]::new()
    "Docs"       = [System.Collections.Generic.List[string]]::new()
    "CI/CD"      = [System.Collections.Generic.List[string]]::new()
    "Other"      = [System.Collections.Generic.List[string]]::new()
}

$prefixMap = @{
    "^feat"     = "Added"
    "^fix"      = "Fixed"
    "^refactor" = "Changed"
    "^chore"    = "Changed"
    "^perf"     = "Performance"
    "^security" = "Security"
    "^docs"     = "Docs"
    "^ci"       = "CI/CD"
    "^build"    = "CI/CD"
    "^test"     = "Added"
}

foreach ($line in $lines) {
    if (-not $line) { continue }
    $parts = $line -split '\|', 4
    if ($parts.Count -lt 2) { continue }
    $hash    = $parts[0].Trim().Substring(0, [Math]::Min(8, $parts[0].Trim().Length))
    $subject = $parts[1].Trim()

    # Skip merge commits and version bumps
    if ($subject -match '^Merge|^chore: bump version') { continue }

    $bucket = "Other"
    foreach ($prefix in $prefixMap.Keys) {
        if ($subject -match $prefix) {
            $bucket = $prefixMap[$prefix]
            break
        }
    }

    # Strip conventional commit prefix for cleaner display
    $display = $subject -replace '^(feat|fix|refactor|chore|perf|docs|ci|build|test)(\([^)]+\))?:\s*', ''
    $display = $display.Substring(0,1).ToUpper() + $display.Substring(1)
    $categories[$bucket].Add("- **[$hash]** $display")
}

# ── Build Markdown ────────────────────────────────────────────────────────────
$date  = (Get-Date -Format "yyyy-MM-dd")
$lines_out = [System.Collections.Generic.List[string]]::new()
$lines_out.Add("## [$Version] — $date")
$lines_out.Add("")
$lines_out.Add("### Summary")
$lines_out.Add("*(Auto-generated from git log $range)*")
$lines_out.Add("")

$total = ($categories.Values | ForEach-Object { $_.Count } | Measure-Object -Sum).Sum
if ($total -eq 0) {
    $lines_out.Add("*No changes recorded in this range.*")
} else {
    foreach ($cat in $categories.Keys) {
        $items = $categories[$cat]
        if ($items.Count -eq 0) { continue }
        $lines_out.Add("### $cat")
        $items | ForEach-Object { $lines_out.Add($_) }
        $lines_out.Add("")
    }
}

$lines_out.Add("---")
$lines_out.Add("")

$output = $lines_out -join "`n"

# ── Output ────────────────────────────────────────────────────────────────────
if ($OutputFile) {
    if ($Append) {
        $changelogPath = Join-Path $rootDir "CHANGELOG.md"
        if (Test-Path $changelogPath) {
            $existing = Get-Content $changelogPath -Raw
            # Insert after "## [Unreleased]" section
            $insertMarker = "## [Unreleased]"
            $markerIdx = $existing.IndexOf($insertMarker)
            if ($markerIdx -ge 0) {
                $afterMarker = $existing.IndexOf("`n---", $markerIdx)
                if ($afterMarker -ge 0) {
                    $newContent = $existing.Substring(0, $afterMarker + 5) `
                        + "`n`n" + $output + $existing.Substring($afterMarker + 5)
                    Set-Content -Path $changelogPath -Value $newContent -NoNewline
                    Write-Host "[changelog] Inserted v$Version section into $changelogPath"
                    return
                }
            }
        }
        Set-Content -Path $OutputFile -Value $output -NoNewline
    } else {
        Set-Content -Path $OutputFile -Value $output -NoNewline
    }
    Write-Host "[changelog] Written to: $OutputFile"
} else {
    Write-Output $output
}
