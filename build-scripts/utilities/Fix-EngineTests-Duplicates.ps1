#!/usr/bin/env pwsh
# Fix-EngineTests-Duplicates.ps1 — Remove duplicate includes from EngineTests.cpp
param([switch]$DryRun)

$fp = Join-Path $PSScriptRoot "..\..\Engine\Tests\EngineTests.cpp"
$lines = [System.IO.File]::ReadAllLines($fp)
Write-Host "Loaded $($lines.Length) lines"

# Second occurrences of pre-existing duplicate includes (0-indexed line numbers to remove)
$toRemove = [System.Collections.Generic.HashSet[int]]::new()
@(
  260,  # PipelineStateCacheV2.h line 261
  261,  # CacheWarmingService.h line 262
  479,  # BitmapPool.h line 480
  503,  # DeadCodeAudit.h line 504
  504,  # DeadCodeAuditor.h line 505
  679,  # AdaptiveQualityScaler.h line 680
  760,  # PerceptualHashEngine.h line 761
  786,  # PredictivePrefetchEngine.h line 787
  805,  # PluginDependencyResolver.h line 806
  1253, # FLIFDecoder.h line 1254
  1264, # ShellContextMenuV2.h line 1265
  1321  # CacheEncryptionLayer.h line 1322
) | ForEach-Object { [void]$toRemove.Add($_) }

# Duplicate NPU includes block: blank line + 9 includes (0-indexed 27528..27537)
27528..27537 | ForEach-Object { [void]$toRemove.Add($_) }

Write-Host "Removing $($toRemove.Count) duplicate lines"

$cleaned = [System.Collections.Generic.List[string]]::new($lines.Length)
for ($i = 0; $i -lt $lines.Length; $i++) {
    if (-not $toRemove.Contains($i)) { $cleaned.Add($lines[$i]) }
}
$result = $cleaned.ToArray()

# Verify
$td  = ($result | Where-Object { $_ -match '^TEST\(' }).Count
$rt  = ($result | Where-Object { $_ -match '^\s+RUN_TEST\(' }).Count
$dup = ($result | Where-Object { $_ -match '^#include' } | Group-Object | Where-Object { $_.Count -gt 1 }).Count
Write-Host "Result: $($result.Length) lines | TEST(): $td | RUN_TEST(): $rt | Dup includes: $dup"

if ($DryRun) { Write-Host "[DryRun] Not writing."; exit 0 }

[System.IO.File]::WriteAllLines($fp, $result, [System.Text.UTF8Encoding]::new($false))
Write-Host "Written OK"
