#!/usr/bin/env pwsh
# BumpHelper-v32.0.0.ps1 — Bump to v32.0.0 "Fomalhaut"
# Post-Quantum Security & Zero-Trust (Sprints 1071-1080)
$entry = @"
## [32.0.0] — $(Get-Date -Format 'yyyy-MM-dd')

### v32.0.0 "Fomalhaut" — Post-Quantum Security & Zero-Trust

#### New: Post-Quantum & Zero-Trust Security Layer (8 components, +72 tests)
- **PostQuantumCryptoProvider** — Kyber768 key encapsulation, Dilithium3 signing, SPHINCS+ hash-based signatures
- **ZeroTrustAccessBroker** — JWT-style capability tokens with issue/validate/revoke lifecycle (singleton)
- **QuantumResistantHashEngine** — SHA3-256, BLAKE3, KangarooTwelve with constant-time compare
- **PluginZeroTrustSandbox** — Per-plugin capability enforcement: Allow/Deny/Quarantine decisions (singleton)
- **BinaryTrustVerifier** — DLL/dylib/so trust chain validation with tamper-evident detection
- **SecureConfigurationManager** — DPAPI (Win32) / SecureEnclave (macOS) / Fallback (Linux) key storage (singleton)
- **ThreatModelingEngine** — STRIDE-based runtime threat analysis with pipeline safety gate
- **SecurityPostureAnalyzer** — TPM attestation + code integrity + patch-level scoring with JSON serialization (singleton)

#### Test Coverage
- Unit tests: 4290 → 4362 (+72)
"@

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
& "$scriptDir\Bump-Version.ps1" `
    -Version "32.0.0" `
    -Codename "Fomalhaut" `
    -TestCount 4362 `
    -ChangelogEntry $entry `
    -TagAndPush
