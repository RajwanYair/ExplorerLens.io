# fix16-type-renames.ps1 — Fix wave-16 cascade type rename errors in EngineTests.cpp
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

    # Fix 1 — BreakerCircuitState::Closed/HalfOpen (wrong name, wrong case) in
    # ExplorerLens::Core context (DecoderHealthDashboard.h has CircuitState).
    # These were incorrectly renamed by fix15 at lines 15323-15324 (0-based).
    # DecoderHealthDashboard.h (ExplorerLens::Core) uses CircuitState with Closed/HalfOpen.
    # DecoderCircuitBreaker.h  (ExplorerLens)        uses BreakerCircuitState with CLOSED/HALF_OPEN.
    # Only change CamelCase variants; UPPER_CASE variants at lines 16326+ are correct.
    $line = $line -replace '\bBreakerCircuitState::Closed\b',   'CircuitState::Closed'
    $line = $line -replace '\bBreakerCircuitState::HalfOpen\b', 'CircuitState::HalfOpen'

    # Fix 2 — AuditLogger ambiguity (lines 16210-16230, 0-based 16209-16229).
    # File-level 'using namespace ExplorerLens::Engine' (line 1327) makes
    # ExplorerLens::Engine::AuditLogger visible.  Local 'using namespace ExplorerLens'
    # inside those tests also makes ExplorerLens::AuditLogger visible → ambiguous.
    # Fix: fully qualify the Utils-level class only in the ambiguous lines.
    # Line 26582 (0-based) uses ExplorerLens::Engine::AuditLogger — leave it alone.
    if ($i -ge 16209 -and $i -le 16229) {
        $line = $line -replace '(?<![:\w])AuditLogger::Instance\(\)', 'ExplorerLens::AuditLogger::Instance()'
    }

    # Fix 3 — PluginTrustChainValidator (default-constructible variety in EngineTests)
    # → TrustChainValidatorV2 (ExplorerLens::Engine, PluginTrustChainValidator.h).
    # The PluginTrustChain.h version requires a PublisherPolicy argument; the tests
    # at 17857+ use PluginTrustChainValidator without arguments, which matches
    # TrustChainValidatorV2's default ctor and its GetPolicy/SetPolicy/GetStats API.
    # Note: PluginTrustChainTests.cpp uses PluginTrustChainValidator v(policy)
    # (ExplorerLens::Plugin context) — that file is NOT being patched here.
    $line = $line -replace '(?<![:\w])PluginTrustChainValidator validator;', 'TrustChainValidatorV2 validator;'

    if ($line -ne $orig) {
        $changed++
        $lines[$i] = $line
    }
}

[System.IO.File]::WriteAllLines($f, $lines, [System.Text.Encoding]::UTF8)
Write-Host "EngineTests.cpp: $changed lines changed out of $n"

Write-Host ""
Write-Host "All fix16 patches applied."
