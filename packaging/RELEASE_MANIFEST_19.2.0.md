# ExplorerLens v19.2.0 "Pulsar-S" — Final Release Manifest
# Sprint 199 · packaging/RELEASE_MANIFEST_19.2.0.md
# Copyright (c) 2026 ExplorerLens Project

## Release Summary

| Field | Value |
|---|---|
| Product | ExplorerLens Shell Extension |
| Version | 19.2.0 |
| Codename | Pulsar-S |
| Series | Pulsar (v19.x) |
| Release Type | Minor — Final Polish & Certification |
| Release Date | 2026-03-26 |

---

## Artifact Manifest

| Artifact | SHA-256 (placeholder) | Size | Condition |
|---|---|---|---|
| `LENSShell.dll` (x64) | *CI-computed* | ~2940 KB | Always |
| `LENSManager.exe` (x64) | *CI-computed* | ~400 KB | Always |
| `lens.exe` (CLI) | *CI-computed* | ~180 KB | Sprint 17+ |
| `ExplorerLens-19.2.0-x64.msi` | *CI-computed* | ~8 MB | Always |
| `ExplorerLens-19.2.0-x64.zip` | *CI-computed* | ~7.5 MB | Always |
| `ExplorerLens-19.2.0-SBOM.json` | *CI-computed* | ~45 KB | Always |
| `SHA256SUMS.txt` | — | — | Always |

> Actual SHA-256 hashes are computed by `release.yml` CI workflow and embedded
> in `SHA256SUMS.txt` at build time using `Get-FileHash -Algorithm SHA256`.

---

## Component Inventory — Pulsar Series (v19.x)

### v19.0.0 "Pulsar" — AI-First Architecture

| Header | Location | Description |
|---|---|---|
| `NeuralThumbnailSynthesizer.h` | `Engine/AI/` | Diffusion-lite ONNX synthesis fallback |
| `ContentCategoryClassifier.h` | `Engine/AI/` | MobileNetV3 13-class content classifier |
| `SemanticColorPalette.h` | `Engine/AI/` | k-means++ dominant color extractor |
| `BlurDetectionFilter.h` | `Engine/AI/` | Laplacian + AI deblur pipeline |
| `NSFWContentGuard.h` | `Engine/AI/` | Enterprise safety classifier |
| `AIThumbnailPipeline.h` | `Engine/AI/` | AI stage orchestration layer |
| `AIModelRegistry.h` | `Engine/AI/` | ONNX model lifecycle manager |
| `AIPerformanceProfiler.h` | `Engine/AI/` | Per-module span profiler |

### v19.1.0 "Pulsar-R" — Enterprise Fleet Management

| Header | Location | Description |
|---|---|---|
| `GroupPolicyBridge.h` | `Engine/Enterprise/` | GP + MDM priority-merge reader |
| `FleetConfigManager.h` | `Engine/Enterprise/` | Fleet thumbnail policy singleton |
| `EnterpriseAuditExporter.h` | `Engine/Enterprise/` | CEF/LEEF/JSON-L SIEM exporter |
| `LDAPUserAttributeResolver.h` | `Engine/Enterprise/` | ADSI AD attribute resolver |
| `TenantIsolationPolicy.h` | `Engine/Enterprise/` | Per-tenant resource isolation |
| `ComplianceReporter.h` | `Engine/Enterprise/` | SOC2/HIPAA/CIS reporter |
| `RemoteConfigPusher.h` | `Engine/Enterprise/` | Azure App Config poller |
| `FleetHealthDashboard.h` | `Engine/Enterprise/` | Prometheus + AzMonitor health |
| `SSOIntegrationBridge.h` | `Engine/Enterprise/` | WAM/OIDC/SAML2 auth bridge |

### v19.2.0 "Pulsar-S" — Final Polish & Certification

| Header | Location | Description |
|---|---|---|
| `EndToEndTestHarness.h` | `Engine/Tests/` | Corpus-driven E2E test runner |
| `CrashReporter.h` | `Engine/Utils/` | WER minidump + CrashContext |
| `TelemetryConsentManager.h` | `Engine/Core/` | GDPR consent gating layer |
| `AutoUpdateManager.h` | `Engine/Utils/` | MSIX delta self-update |
| `DiagnosticsConsole.h` | `Engine/Core/` | Named-pipe diagnostic REPL |
| `CertificationTestSuite.h` | `Engine/Tests/` | 18 WHQL-mapped cert tests |

---

## Certification Status

| Test Suite | Tests | Passing | Blocking Failures |
|---|---|---|---|
| Shell Integration | 4 | 4 | 0 |
| Memory Safety | 3 | 3 | 0 |
| Performance SLOs | 3 | 3 | 0 |
| Reliability | 2 | 2 | 0 |
| Security | 3 | 3 | 0 |
| Compatibility | 2 | 2 | 0 |
| Localization | 2 | 2 | 0 |
| **Total** | **18** | **18** | **0** |

**Certification result: ✅ READY**

---

## Breaking Changes in v19.x

None. All v19.x developments are strictly additive. Full backwards compatibility
with v18.x, v17.x, and v16.x deployments is maintained.

---

## Deprecation Notices

| Symbol | Deprecated Since | Removal Target | Migration |
|---|---|---|---|
| `LegacyThumbnailCodec` | v17.1.0 | v21.0.0 | Use `DecoderRegistry::Dispatch()` |
| `OldCacheKey::Build()` | v18.0.0 | v20.0.0 | Use `HiDPIThumbnailCache::BuildKey()` |

---

## Known Limitations

- ARM64 native build requires manual ONNX Runtime ARM64 model conversion
- `DiagnosticsConsole` named pipe requires Local Admin to connect from remote tools
- `AutoUpdateManager` download stub is not wired to real endpoint in this build

---

## Security Advisories

None for v19.2.0. See [SECURITY_HARDENING.md](../docs/SECURITY_HARDENING.md).

---

*ExplorerLens v19.2.0 "Pulsar-S" · Final Release Manifest · Copyright (c) 2026 ExplorerLens Project*
