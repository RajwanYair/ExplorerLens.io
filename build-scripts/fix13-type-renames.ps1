# fix13-type-renames.ps1 — Fix cascading type rename errors in test files
# Run from the workspace root OR any directory (uses $PSScriptRoot)

$root = Split-Path -Parent $PSScriptRoot
Write-Host "Root: $root"

# ─── EngineTests.cpp ─────────────────────────────────────────────────────────

$f = Join-Path $root "Engine\Tests\EngineTests.cpp"
Write-Host "Patching: $f"

$lines = [System.IO.File]::ReadAllLines($f, [System.Text.Encoding]::UTF8)
$n = $lines.Count
$changed = 0

for ($i = 0; $i -lt $n; $i++) {
    $orig = $lines[$i]
    $line = $orig

    # Global safe replacements (all tested single-line, no multi-line)
    $line = $line -replace '\bPackageType::',                'MSIXPackageType::'
    $line = $line -replace '\bGateVerdict::',                'ReleaseGateVerdict::'
    $line = $line -replace '\bQualityPreset::',              'ExportQualityPreset::'
    $line = $line -replace '\bArtifactType::',               'ValidatorArtifactType::'
    $line = $line -replace '\bSyncStatus::',                 'ProviderSyncStatus::'
    $line = $line -replace '\bHealthLevel::',                'DiagHealthLevel::'
    # Locale:: — protect qualified references like ::Locale:: using negative lookbehind on chars
    $line = $line -replace '(?<![:\w])Locale::',             'LocaleInfo::'
    # NetworkProtocol:: — protect ExplorerLens::Engine::Cloud::NetworkProtocol:: references
    $line = $line -replace '(?<![:\w])NetworkProtocol::',    'ProviderNetProtocol::'
    # CloudProvider:: — ONLY change on specific lines (5290–5320, 0-based 5289–5319)
    if ($i -ge 5289 -and $i -le 5319) {
        $line = $line -replace '(?<![:\w])CloudProvider::',  'StorageCloudProvider::'
    }

    if ($line -ne $orig) {
        $changed++
        $lines[$i] = $line
    }
}

[System.IO.File]::WriteAllLines($f, $lines, [System.Text.Encoding]::UTF8)
Write-Host "EngineTests.cpp: $changed lines changed out of $n"

# ─── PluginSandboxPolicyTests.cpp ────────────────────────────────────────────

$f2 = Join-Path $root "Engine\Tests\PluginSandboxPolicyTests.cpp"
Write-Host "Patching: $f2"

$lines2 = [System.IO.File]::ReadAllLines($f2, [System.Text.Encoding]::UTF8)
$changed2 = 0

for ($i = 0; $i -lt $lines2.Count; $i++) {
    $orig = $lines2[$i]
    $line = $orig
    # SandboxPolicy class renamed to SandboxPolicySpec
    $line = $line -replace '(?<![:\w])SandboxPolicy::',     'SandboxPolicySpec::'
    # JobObjectLimits struct renamed to SandboxJobLimits
    $line = $line -replace '\bJobObjectLimits\b',            'SandboxJobLimits'
    if ($line -ne $orig) { $changed2++; $lines2[$i] = $line }
}

[System.IO.File]::WriteAllLines($f2, $lines2, [System.Text.Encoding]::UTF8)
Write-Host "PluginSandboxPolicyTests.cpp: $changed2 lines changed"

# ─── PluginReferencePackTests.cpp ────────────────────────────────────────────

$f3 = Join-Path $root "Engine\Tests\PluginReferencePackTests.cpp"
Write-Host "Patching: $f3"

$lines3 = [System.IO.File]::ReadAllLines($f3, [System.Text.Encoding]::UTF8)
$changed3 = 0

for ($i = 0; $i -lt $lines3.Count; $i++) {
    $orig = $lines3[$i]
    $line = $orig
    # WatermarkConfig renamed to PackWatermarkConfig in PluginReferencePack.h
    $line = $line -replace '\bWatermarkConfig\b',            'PackWatermarkConfig'
    if ($line -ne $orig) { $changed3++; $lines3[$i] = $line }
}

[System.IO.File]::WriteAllLines($f3, $lines3, [System.Text.Encoding]::UTF8)
Write-Host "PluginReferencePackTests.cpp: $changed3 lines changed"

Write-Host ""
Write-Host "All patches applied successfully."
