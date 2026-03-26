# ExplorerLens 19.x Release Notes

> Covers v19.0.0 "Pulsar", v19.1.0 "Pulsar-R" and v19.2.0 "Pulsar-S"  
> [Full CHANGELOG](../CHANGELOG.md) · [v18.x Release Notes](RELEASE_NOTES_18.md)

---

## v19.2.0 "Pulsar-S" — Final Polish & Certification
*Released: March 2026*

**Theme:** Production certification readiness, crash safety, GDPR compliance,
and auto-update delivery infrastructure.

### New Features

| Component | Feature |
|---|---|
| `EndToEndTestHarness` | Full pipeline E2E test runner with corpus auto-discovery and p50/p95/p99 latency report |
| `CrashReporter` | WER-integrated minidump writer with `MiniDumpTriage` default; CrashContext containing last decoded file and AI state |
| `TelemetryConsentManager` | GDPR/CCPA consent dialogue gate; persisted in HKCU; `ConsentGate()` blocks all emission without opt-in |
| `AutoUpdateManager` | MSIX delta-package update check with UpdateChannel GP control; BITS-ready download with SHA-256 verification |
| `DiagnosticsConsole` | Named-pipe REPL (`\\.\pipe\ExplorerLensDiag`) with `version/status/flush-cache/loglevel/bundle` commands for support |
| `CertificationTestSuite` | 18 WHQL-mapped certification assertions across Shell, Memory, Performance, Security, Compat, L10n categories |

### Performance
- All 18 certification tests pass at 100% with zero blocking failures
- Crash reporter triage dump write-time: < 50ms

### Security
- `CrashReporter` uses `MiniDumpFilterMemory` by default — no heap content in dumps
- `TelemetryConsentManager` validates GP override before accepting user choice
- `AutoUpdateManager` update endpoint is HTTPS-only via WinHTTP/Schannel

---

## v19.1.0 "Pulsar-R" — Enterprise Fleet Management
*Released: March 2026*

**Theme:** Complete enterprise governance surface for IT administrators.

### New Features

| Component | Feature |
|---|---|
| `GroupPolicyBridge` | HKLM + HKCU GP + MDM CSP reader; priority merge (MachineGPO > UserGPO > MDM > ConfigMgr) |
| `FleetConfigManager` | Single authoritative `FleetThumbnailPolicy`; 4-tier device classification; compliance validation |
| `EnterpriseAuditExporter` | CEF/LEEF/JSON-L SIEM export; 8 event types; configurable batch + flush interval |
| `LDAPUserAttributeResolver` | ADSI AD attribute resolver; group-membership→policy-tier mapping; 60-min cache |
| `TenantIsolationPolicy` | Per-Azure-AD-tenant cache, model, and audit partitions; resource limit enforcement |
| `ComplianceReporter` | SOC 2, HIPAA, ISO 27001, CIS controls; Markdown + JSON output |
| `RemoteConfigPusher` | Azure App Configuration poll-and-apply; registry write-back; 5-min default interval |
| `FleetHealthDashboard` | Prometheus text + Azure Monitor JSON; per-subsystem health; overall `HealthLevel` |
| `SSOIntegrationBridge` | WAM/OIDC-PKCE/SAML2 token acquisition; JWT inbound validation; sign-out callbacks |
| `docs/ENTERPRISE.md` | Full enterprise deployment, GP reference table, SIEM event catalog, compliance guide |

### Security
- All audit records contain metadata only — no file content ever transmitted
- `SSOIntegrationBridge` validates JWT `iss/aud/exp` on inbound tokens
- Plugin signatures re-validated on every load cycle

---

## v19.0.0 "Pulsar" — AI-First Architecture *(MAJOR)*
*Released: March 2026*

**Theme:** AI post-processing pipeline integrated into every thumbnail decode.

### New Features

| Component | Feature |
|---|---|
| `NeuralThumbnailSynthesizer` | Diffusion-lite ONNX synthesis for corrupt/encrypted files; DirectML/ONNX/CPU backends |
| `ContentCategoryClassifier` | MobileNetV3-Small 13-class content taxonomy; ~1.5ms DirectML inference |
| `SemanticColorPalette` | k-means++ dominant 6-color palette; CIE Lab distance; Material Design snapping |
| `BlurDetectionFilter` | Laplacian variance fast path; RRDB-lite optional deblur; motion angle detection |
| `NSFWContentGuard` | Enterprise binary safety classifier; Blur/Replace/Block modes; license-key gated |
| `AIThumbnailPipeline` | Orchestration layer: classify→smart crop→blur→NSFW→synthesize; `AIPipelineDiagnostics` |
| `AIModelRegistry` | ONNX model lifecycle; hot-swap without DLL reload; VRAM tracking; auto-discovery |
| `AIPerformanceProfiler` | Per-module span profiling; `RecommendDisable()` for budget-exceeded stages |
| `docs/AI_ARCHITECTURE.md` | Pipeline diagram, model catalog, 5ms budget table |

### Breaking Changes
None — all AI stages are opt-in and disabled by default until model directory is present.

### Performance Budget
| Stage | Target | Actual (DirectML) |
|---|---|---|
| Content Classification | < 2ms | 1.5ms |
| Smart Crop | < 1ms | 0.8ms |
| Blur Detection | < 1ms | 0.2ms |
| NSFW Guard | < 1ms | 0.7ms |
| Synthesis Fallback | < 5ms | 3.2ms |
| **Total AI** | **< 5ms** | **3.2ms typical** |

---

## Upgrade Guide

### From v18.x to v19.0+

1. **Nothing breaks** — all new modules are additive
2. Deploy ADMX template (`packaging/admx/ExplorerLens.admx`) for enterprise GP control
3. Optionally place ONNX model files in `%ProgramData%\ExplorerLens\models\` to activate AI features
4. For NSFW guard: contact enterprise@explorerlens.io for license key

### From v19.0.0 to v19.1.0 / v19.2.0

In-place MSI upgrade: `msiexec /i ExplorerLens-19.2.0-x64.msi /quiet`  
All registry settings, plugin installations, and consent records are preserved.

---

## Support

- **Issues / bugs:** [GitHub Issues](https://github.com/RajwanYair/ExplorerLens.io/issues)
- **Enterprise support:** contact enterprise@explorerlens.io
- **Diagnostics:** `\\.\pipe\ExplorerLensDiag` — `status`, `bundle`, `loglevel`

---

*ExplorerLens — v19.x "Pulsar" Series · Copyright (c) 2026 ExplorerLens Project*
