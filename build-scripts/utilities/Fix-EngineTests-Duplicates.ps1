#!/usr/bin/env pwsh
# Fix-EngineTests-Duplicates.ps1 — Detect and remove duplicate #include lines from EngineTests.cpp
# Usage: .\Fix-EngineTests-Duplicates.ps1           # report + fix
#        .\Fix-EngineTests-Duplicates.ps1 -DryRun   # report only
param([switch]$DryRun)

$fp = Join-Path $PSScriptRoot "..\..\Engine\Tests\EngineTests.cpp"
$lines = [System.IO.File]::ReadAllLines($fp)
Write-Host "Loaded $($lines.Length) lines"

# Dynamically detect duplicate #include lines; keep first occurrence, drop extras
$seen    = [System.Collections.Generic.HashSet[string]]::new()
$toRemove = [System.Collections.Generic.HashSet[int]]::new()
for ($i = 0; $i -lt $lines.Length; $i++) {
    if ($lines[$i] -match '^#include') {
        if (-not $seen.Add($lines[$i])) { [void]$toRemove.Add($i) }
    }
}

if ($toRemove.Count -eq 0) {
    Write-Host "No duplicate includes found."
    exit 0
}

Write-Host "Found $($toRemove.Count) duplicate include lines:"
$toRemove | Sort-Object | ForEach-Object { Write-Host "  Line $($_+1): $($lines[$_])" }

$cleaned = [System.Collections.Generic.List[string]]::new($lines.Length)
for ($i = 0; $i -lt $lines.Length; $i++) {
    if (-not $toRemove.Contains($i)) { $cleaned.Add($lines[$i]) }
}
$result = $cleaned.ToArray()

$td  = ($result | Where-Object { $_ -match '^TEST\(' }).Count
$rt  = ($result | Where-Object { $_ -match '^\s+RUN_TEST\(' }).Count
$dup = ($result | Where-Object { $_ -match '^#include' } | Group-Object | Where-Object { $_.Count -gt 1 }).Count
Write-Host "Result: $($result.Length) lines | TEST(): $td | RUN_TEST(): $rt | Remaining dup includes: $dup"

if ($DryRun) { Write-Host "[DryRun] Not writing."; exit 0 }

[System.IO.File]::WriteAllLines($fp, $result, [System.Text.UTF8Encoding]::new($false))
Write-Host "Written OK"
