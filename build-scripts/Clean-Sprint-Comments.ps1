<#
.SYNOPSIS
    Removes sprint/version clutter from source code comments.
.DESCRIPTION
    Strips "Sprint NNN", version strings "v15.0.0 Zenith", and dead
    inline comments from all Engine header/source files. Preserves
    meaningful descriptions and copyright notices.
#>

param(
    [string]$RootDir = (Split-Path -Parent $PSScriptRoot),
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"
$stats = @{ FilesProcessed = 0; LinesRemoved = 0; LinesModified = 0 }

# Collect files with sprint/version references
$targetDirs = @(
    (Join-Path $RootDir "Engine"),
    (Join-Path $RootDir "LENSShell")
)

$files = @()
foreach ($dir in $targetDirs) {
    if (Test-Path $dir) {
        $files += Get-ChildItem -Path $dir -Recurse -Include "*.h","*.cpp","*.hlsl" |
            Where-Object {
                $c = Get-Content $_.FullName -Raw -ErrorAction SilentlyContinue
                $c -and ($c -match 'Sprint \d+' -or $c -match 'v15\.0\.0.*Zenith' -or $c -match 'ExplorerLens Engine v\d+')
            }
    }
}

Write-Host "Found $($files.Count) files to process" -ForegroundColor Cyan

foreach ($file in $files) {
    $lines = Get-Content $file.FullName -Encoding UTF8
    $newLines = [System.Collections.Generic.List[string]]::new()
    $modified = $false
    $baseName = $file.BaseName

    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]
        $original = $line

        # ────────────────────────────────────────────────────────────────
        # RULE 1: Remove entire version+sprint lines (Pattern B)
        # "// ExplorerLens Engine v15.0.0 "Zenith" — Sprint 362"
        # "// ExplorerLens v15.0.0 "Zenith" — Sprint 383"
        # ────────────────────────────────────────────────────────────────
        if ($line -match '^\s*//\s*ExplorerLens\s+(Engine\s+)?v\d+\.\d+\.\d+\s*"?\w*"?\s*[—–-]\s*Sprint\s+\d+') {
            $stats.LinesRemoved++
            $modified = $true
            continue
        }

        # ────────────────────────────────────────────────────────────────
        # RULE 2: Remove standalone version lines without sprint
        # "// ExplorerLens Engine v15.0.0 "Zenith""
        # ────────────────────────────────────────────────────────────────
        if ($line -match '^\s*//\s*ExplorerLens\s+(Engine\s+)?v\d+\.\d+\.\d+\s*"?\w*"?\s*$') {
            $stats.LinesRemoved++
            $modified = $true
            continue
        }

        # ────────────────────────────────────────────────────────────────
        # RULE 3: Clean "// Sprint NNN: Description" → "// Description" (Pattern A)
        # Keep everything after the colon as a standalone comment
        # ────────────────────────────────────────────────────────────────
        if ($line -match '^(\s*)//\s*Sprint\s+\d+(-\d+)?:\s*(.+)$') {
            $indent = $matches[1]
            $desc = $matches[3]
            $line = "${indent}// ${baseName}.h — ${desc}"
            $stats.LinesModified++
            $modified = $true
        }

        # ────────────────────────────────────────────────────────────────
        # RULE 4: Clean banner lines "ExplorerLens Engine — Sprint NNN: Description"
        # "// ExplorerLens Engine — Sprint 354-355: Archive Refactor Engine"
        # → "// ArchiveRefactorEngine.h — Archive Refactor Engine"
        # ────────────────────────────────────────────────────────────────
        if ($line -match '^\s*//\s*ExplorerLens\.?i?o?\s*(Engine\s*)?—\s*Sprint\s+\d+(-\d+)?:\s*(.+)$') {
            $desc = $matches[3].Trim()
            $line = "// ${baseName}.h — ${desc}"
            $stats.LinesModified++
            $modified = $true
        }

        # ────────────────────────────────────────────────────────────────
        # RULE 5: Clean C-style box sprint lines (Pattern D)
        # " * ExplorerLens — Sprint 33: Crash Intelligence & Symbol Pipeline"
        # → " * CrashIntelligence.h — Crash Intelligence & Symbol Pipeline"
        # ────────────────────────────────────────────────────────────────
        if ($line -match '^(\s*\*)\s*ExplorerLens\s*—\s*Sprint\s+\d+(-\d+)?:\s*(.+)$') {
            $prefix = $matches[1]
            $desc = $matches[3].Trim()
            $line = "${prefix} ${baseName}.h — ${desc}"
            $stats.LinesModified++
            $modified = $true
        }

        # ────────────────────────────────────────────────────────────────
        # RULE 6: Remove "Exit criteria:" lines from D-pattern headers
        # " * Exit criteria: any crash symbolized & bucketed in <5 minutes."
        # ────────────────────────────────────────────────────────────────
        if ($line -match '^\s*\*\s*Exit criteria:') {
            $stats.LinesRemoved++
            $modified = $true
            continue
        }

        # ────────────────────────────────────────────────────────────────
        # RULE 7: Clean banner title with version reference (ReleaseGateV33 pattern)
        # "// ExplorerLens.io Engine — Release Gate V33 — v15.0 "Zenith" Ship Gate"
        # → "// ReleaseGateV33.h — Release Gate V33 Ship Gate"
        # ────────────────────────────────────────────────────────────────
        if ($line -match '^\s*//\s*ExplorerLens\.?i?o?\s*(Engine\s*)?—\s*(.+?)\s*—\s*v\d+\.\d+\s*"?\w*"?\s*(.+)$') {
            $part1 = $matches[2].Trim()
            $part2 = $matches[3].Trim()
            $line = "// ${baseName}.h — ${part1} ${part2}"
            $stats.LinesModified++
            $modified = $true
        }

        # ────────────────────────────────────────────────────────────────
        # RULE 8: Clean inline "/// Something — Sprint NNN" doc comments
        # "/// Adaptive Decoder Router — Sprint 395" → "/// Adaptive Decoder Router"
        # ────────────────────────────────────────────────────────────────
        if ($line -match '^(\s*///\s*.+?)\s*—\s*Sprint\s+\d+(-\d+)?\s*$') {
            $line = $matches[1]
            $stats.LinesModified++
            $modified = $true
        }

        # ────────────────────────────────────────────────────────────────
        # RULE 9: Clean inline enum/code comments with sprint refs
        # "V1_5,      // Sprint 150 — sandbox" → "V1_5,      // Sandbox"
        # ────────────────────────────────────────────────────────────────
        if ($line -match '^(\s*\S+.*?)\s*//\s*Sprint\s+\d+\s*—\s*(.+)$') {
            $code = $matches[1]
            $desc = $matches[2].Trim()
            # Capitalize first letter
            if ($desc.Length -gt 0) {
                $desc = $desc.Substring(0,1).ToUpper() + $desc.Substring(1)
            }
            $line = "${code}// ${desc}"
            $stats.LinesModified++
            $modified = $true
        }

        # ────────────────────────────────────────────────────────────────
        # RULE 10: Remove standalone "// Sprint NNN" comments (no description)
        # But NOT inside string literals or data
        # ────────────────────────────────────────────────────────────────
        if ($line -match '^\s*//\s*Sprint\s+\d+(-\d+)?\s*$') {
            $stats.LinesRemoved++
            $modified = $true
            continue
        }

        # ────────────────────────────────────────────────────────────────
        # RULE 11: Clean "// Builds on ReleaseGateVN (Sprint NNN) with:"
        # → "// Builds on ReleaseGateVN with:"
        # ────────────────────────────────────────────────────────────────
        if ($line -match '(.*)\(Sprint\s+\d+\)(.*)') {
            $line = $matches[1].TrimEnd() + $matches[2]
            $stats.LinesModified++
            $modified = $true
        }

        # ────────────────────────────────────────────────────────────────
        # RULE 12: Clean "// ReleaseGateVN — Sprint NNN" lines
        # → "// ReleaseGateVN — Release Quality Gate"
        # ────────────────────────────────────────────────────────────────
        if ($line -match '^(\s*//\s*ReleaseGateV\d+)\s*—\s*Sprint\s+\d+\s*$') {
            $line = "$($matches[1]) — Release Quality Gate"
            $stats.LinesModified++
            $modified = $true
        }

        # ────────────────────────────────────────────────────────────────
        # RULE 13: Clean "Sprint NNN:" prefix in banner description lines
        # "// Sprint 234: Release Gate V12 — ..." → "// Release Gate V12 — ..."
        # ────────────────────────────────────────────────────────────────
        if ($line -match '^(\s*//)\s*Sprint\s+\d+(-\d+)?:\s*(.+)$' -and -not $line.Contains('.h — ')) {
            $prefix = $matches[1]
            $desc = $matches[3]
            $line = "${prefix} ${baseName}.h — ${desc}"
            $stats.LinesModified++
            $modified = $true
        }

        # ────────────────────────────────────────────────────────────────
        # RULE 14: Remove "// ExplorerLens Engine — Sprint NNN: ..." variant
        # that may have been missed by Rule 4 (with .io suffix)
        # ────────────────────────────────────────────────────────────────
        if ($line -match '^(\s*//)\s*ExplorerLens\s+(Engine\s*)?—\s*Sprint\s+\d+:\s*(.+)$') {
            $prefix = $matches[1]
            $desc = $matches[3].Trim()
            $line = "${prefix} ${baseName}.h — ${desc}"
            $stats.LinesModified++
            $modified = $true
        }

        # ────────────────────────────────────────────────────────────────
        # RULE 15: HLSL version line
        # "// ExplorerLens v15.0.0 "Zenith" — Sprint 360"
        # Already handled by Rule 1, but catch any remaining
        # ────────────────────────────────────────────────────────────────

        $newLines.Add($line)
    }

    if ($modified) {
        $stats.FilesProcessed++
        if (-not $DryRun) {
            # Write with UTF-8 BOM to match MSVC expectations
            $utf8 = [System.Text.UTF8Encoding]::new($false)
            [System.IO.File]::WriteAllLines($file.FullName, $newLines.ToArray(), $utf8)
        }
        $relPath = $file.FullName.Replace($RootDir + "\", "")
        Write-Host "  [CLEANED] $relPath" -ForegroundColor Green
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Comment Cleanup Summary" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Files processed: $($stats.FilesProcessed)"
Write-Host "  Lines removed:   $($stats.LinesRemoved)"
Write-Host "  Lines modified:  $($stats.LinesModified)"
if ($DryRun) {
    Write-Host "  [DRY RUN — no files were modified]" -ForegroundColor Yellow
}
Write-Host "========================================" -ForegroundColor Cyan
