# ExplorerLens Security Hardening Guide

**Version:** 17.2.0 "Nova-S"  
**Classification:** Public  
**Updated:** 2026-03-26

---

## Threat Model Summary

ExplorerLens operates as a **COM In-Process Server** inside Windows Explorer (`explorer.exe`).
Its primary attack surface is **malicious files** delivered to the thumbnail provider.

| Threat | Likelihood | Impact | Mitigation |
|--------|-----------|--------|-----------|
| Malformed file → heap overflow in decoder | High | Critical | InputSanitizer + SecureDecodeContext sandbox |
| Polyglot file bypass decoder detection | Medium | High | IsLikelyPolyglot() check in InputSanitizer |
| Malicious plugin DLL sideloading | Medium | Critical | IntegrityVerifier + TrustChainV2 |
| MITM on auto-update channel | Low | High | CertificatePinner with SPKI pinset |
| CFG/CET bypass via loaded plugin | Low | Critical | StackGuardPolicy module audit |
| Exfiltration via decoder network access | Low | High | SecureDecodeContext — no network in sandbox |
| EXIF-embedded script injection | Medium | Medium | InputSanitizer.HasSuspiciousEXIF() |

---

## Security Architecture

```
File arrives via IInitializeWithStream::Initialize()
         │
         ▼
InputSanitizer::Validate()        ← magic check, dimension cap, polyglot detect
         │ FAIL → reject, log AuditLogger::LogSanitizeFailure()
         │ OK  ↓
LatencyBudgetManager::GetTier()
         │ Slow/Async tier?
         │─── YES ──→ SecureDecodeContext (sandbox: Isolated/AppContainer)
         │─── NO  ──→ In-process fast path
         ↓
Decoder pipeline
         │
         ↓
AuditLogger  (ETW + JSON — all security events)
```

---

## Mitigation Details

### InputSanitizer

Validates file data before any decoder touches it:
- **Magic bytes**: checks file header against known magic for the claimed extension
- **Dimension cap**: rejects files with width/height > 32768 pixels
- **Stride overflow**: verifies `stride × height` cannot overflow `size_t`
- **Polyglot detection**: scans for embedded ZIP/PDF magic in non-archive files
- **EXIF scanning**: looks for JavaScript/VBScript keywords in JPEG comment segments

### SecureDecodeContext

Routes high-risk formats (PDF, SVG, RAR, EPS, AVI, MKV) through a sandboxed child process:
- Windows Job Object with `JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE`
- No network access (`PROC_THREAD_ATTRIBUTE_SECURITY_CAPABILITIES`)
- 5-second hard timeout — prevents decoder hang attacks
- Max 16 MB output — prevents bitmap inflation attacks

### IntegrityVerifier

Validates every plugin DLL before loading:
1. `WinVerifyTrust` — Authenticode chain must be valid and non-revoked
2. Certificate thumbprint checked against pinned set of trusted publishers
3. SHA-256 content hash checked against marketplace manifest `ExpectedHash`

### CertificatePinner

Pins the update CDN's SPKI SHA-256 hash. If the server certificate changes
to an unknown key (e.g., CA compromise, MITM), the download is aborted and
`AuditEventId::IntegrityCheckFailed` is emitted.

### StackGuardPolicy

At startup, audits all loaded DLLs for:
- `/guard:cf` — Control Flow Guard (CFG)
- `/NXCOMPAT` — Data Execution Prevention (DEP)
- `CETCOMPAT` — CET Shadow Stack (Win11 22H2+ hardware required)
- `DYNAMICBASE` — ASLR

Plugins failing the audit are blocked from loading if `blockOnViolation = true`.

---

## Security Configuration (Registry)

```registry
HKLM\Software\Policies\ExplorerLens
  DisableTelemetry         = 1            ; opt-out telemetry
  ForceRegisterOnLogon     = 0            ; prevent auto-registration
  PreventUserUnregister    = 1            ; lock shell extension registration

HKCU\Software\ExplorerLens\Security
  SandboxLevel             = 2            ; 0=None, 1=Restricted, 2=Isolated, 3=AppContainer
  RequireCFG               = 1            ; block non-CFG plugins
  RequireShadowStack       = 0            ; optional (requires CET hardware)
  PinUpdateCertificate     = 1            ; enforce certificate pinning
```

---

## Incident Response

If `AuditEventId::SandboxEscape` is logged:

1. Immediately terminate Explorer (`taskkill /F /IM explorer.exe`)
2. Collect diagnostic bundle: `LENSManager.exe /collect-diagnostics bundle.zip`
3. File report at https://github.com/RajwanYair/ExplorerLens.io/security/advisories/new
4. Do not re-open affected files until a patched version is available

For non-critical security events, ETW traces are in:  
`Applications and Services Logs\ExplorerLens\Security`

---

## CodeQL SAST

Static analysis runs on every PR via `.github/workflows/codeql.yml`.
Alert categories monitored: `security-extended`, `cpp/overrun-local`, `cpp/uninitialized-local`.
