# fix15-type-renames.ps1 — Fix next wave of cascade type rename errors in EngineTests.cpp
# Run from the workspace root OR any directory (uses $PSScriptRoot)

$root = Split-Path -Parent $PSScriptRoot
Write-Host "Root: $root"

$f = Join-Path $root "Engine\Tests\EngineTests.cpp"
Write-Host "Patching: $f"

$lines = [System.IO.File]::ReadAllLines($f, [System.Text.Encoding]::UTF8)
$n = $lines.Count
$changed = 0

for ($i = 0; $i -lt $n; $i++) {
    $orig = $lines[$i]
    $line = $orig

    # Fix 1 ─ ExplorerLens::Cloud::CloudFileInfo → ExplorerLens::Cloud::ThumbCloudFileInfo
    $line = $line -replace 'ExplorerLens::Cloud::CloudFileInfo\b', 'ExplorerLens::Cloud::ThumbCloudFileInfo'

    # Fix 2 ─ ExplorerLens::Cloud::CloudProvider → ExplorerLens::Cloud::ThumbCloudProvider
    $line = $line -replace 'ExplorerLens::Cloud::CloudProvider\b', 'ExplorerLens::Cloud::ThumbCloudProvider'

    # Fix 3 ─ CircuitState:: → BreakerCircuitState:: (in ExplorerLens namespace)
    # Guard: don't touch already-qualified 'BreakerCircuitState'
    $line = $line -replace '(?<![:\w])CircuitState::', 'BreakerCircuitState::'

    # Fix 4 ─ PluginTestScenario:: → ValidationTestScenario:: (in ExplorerLens::Plugin)
    # ValidationTestScenario has NormalDecode/CrashInjection/TimeoutInjection factories
    $line = $line -replace '(?<![:\w])PluginTestScenario::', 'ValidationTestScenario::'

    # Fix 5 ─ JobObjectLimits → SandboxJobLimits (in ExplorerLens::Plugin)
    # SandboxJobLimits has maxMemoryBytes, maxCPUPercent, maxHandles, killOnJobClose
    $line = $line -replace '\bJobObjectLimits\b', 'SandboxJobLimits'

    # Fix 6 ─ ExplorerLens::Engine::Utils::HashAlgorithm:: → ExplorerLens::Engine::Utils::PerceptualHashAlgo::
    # PerceptualHashAlgo has pHash/dHash/aHash/wHash values
    $line = $line -replace 'ExplorerLens::Engine::Utils::HashAlgorithm::', 'ExplorerLens::Engine::Utils::PerceptualHashAlgo::'

    # Fix 7 ─ unqualified HashAlgorithm:: (in ExplorerLens::Engine::Utils context) → PerceptualHashAlgo::
    # NOTE: FileHashEngine.h also has HashAlgorithm in ExplorerLens::Engine, but its values
    # are CRC32/MD5/SHA1/SHA256/SHA512/COUNT — not pHash/aHash. The perceptual tests use
    # pHash/aHash which ONLY exist in PerceptualHashAlgo. Safe to change iff preceded by Utils:: context.
    # We do a per-line replacement only when the line also references PerceptualHash OR the
    # ExplorerLens::Engine::Utils prefix is present:
    if (($line -match 'ExplorerLens::Engine::Utils') -or ($line -match 'PerceptualHash')) {
        $line = $line -replace '(?<![:\w])HashAlgorithm::', 'PerceptualHashAlgo::'
    }

    # Fix 8 ─ PluginManifest (unqualified) → MarketplacePluginManifest
    # MarketplacePluginManifest is in ExplorerLens::Engine (PluginMarketplaceUnified.h)
    # with fields: architecture (PluginArch::x64) and type (PluginPackageType::Decoder)
    # The unqualified PluginManifest at line 17562+ is in ExplorerLens::Engine context.
    # CAUTION: PluginManifest also exists in ExplorerLens::Engine (PluginManager.h) with
    # DIFFERENT fields (name, version, api_version, etc.). Since the test checks .architecture
    # and .type, it must be MarketplacePluginManifest.
    # Pattern: match lines that use the architecture/type fields OR in ExplorerLens::Engine context.
    # Safest: only change standalone "PluginManifest " (declaration), not "PluginManifest::" or
    # fully qualified "ExplorerLens::Engine::PluginManifest"
    $line = $line -replace '(?<![:\w])PluginManifest\b(?!V2)', 'MarketplacePluginManifest'

    # Fix 9 ─ MetricType:: (unqualified) → ExplorerLens::Engine::Core::MetricType::
    # MetricType is in ExplorerLens::Engine::Core (Telemetry.h)
    # Protect: already-qualified references like ExplorerLens::Engine::Core::MetricType::
    $line = $line -replace '(?<![:\w])MetricType::', 'ExplorerLens::Engine::Core::MetricType::'

    # Fix 10 ─ FLIFDecoder::CanDecode(wstring) → FLIFDecoder::IsSupported
    # FLIFDecoder.h only has IsSupported(const uint8_t*, size_t) — the wchar_t variant
    # was renamed/removed. But since the signature changed (extension string → byte header),
    # we cannot fix this as a pure rename. We comment out and note:
    # SKIP: FLIFDecoder::CanDecode is a fundamental API change — keep as-is.

    if ($line -ne $orig) {
        $changed++
        $lines[$i] = $line
    }
}

[System.IO.File]::WriteAllLines($f, $lines, [System.Text.Encoding]::UTF8)
Write-Host "EngineTests.cpp: $changed lines changed out of $n"
Write-Host "Done."
