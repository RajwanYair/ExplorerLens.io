# ExplorerLens Enterprise Fleet Management

> Version 24.1.0 "Altair-R" · [CHANGELOG](../CHANGELOG.md) · [Back to Docs Index](INDEX.md)

## Overview

ExplorerLens Enterprise extends the core thumbnail engine with a complete fleet management
surface: centralized Group Policy deployment, per-tenant isolation, SIEM audit export,
compliance reporting, and SSO integration for the Plugin Marketplace and Manager console.

All enterprise features are **additive and backwards-compatible** — they activate only when
Group Policy keys or MDM profiles are deployed, and have zero overhead on unmanaged devices.

---

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Fleet Policy Sources                      │
│  GPO (HKLM)  │  MDM/Intune  │  Azure App Config  │  Manual  │
└──────┬───────┴──────┬────────┴────────┬────────────┴─────────┘
       │              │                 │
       ▼              ▼                 ▼
┌─────────────────────────────────────────────────────────────┐
│               GroupPolicyBridge / FleetConfigManager         │
│  (priority merge: Machine GPO > User GPO > MDM > ConfigMgr) │
└──────────────────────────┬──────────────────────────────────┘
                           │ FleetThumbnailPolicy
          ┌────────────────┼────────────────┐
          ▼                ▼                ▼
  TenantIsolation   AuditExporter    SSOIntegration
  (cache/model       (CEF/LEEF/       (WAM/OIDC/
   partitions)        JSON-L)          SAML2)
          │                │
          ▼                ▼
  ComplianceReporter  FleetHealthDashboard
  (SOC2/HIPAA/CIS)   (Prometheus/AzMonitor)
```

---

## Group Policy Configuration

### ADMX Template

Install the ExplorerLens ADMX template from `packaging/admx/ExplorerLens.admx` into:

```
C:\Windows\PolicyDefinitions\ExplorerLens.admx
C:\Windows\PolicyDefinitions\en-US\ExplorerLens.adml
```

### Registry Keys (`HKLM\SOFTWARE\Policies\ExplorerLens\`)

| Value Name | Type | Default | Description |
|---|---|---|---|
| `EnableThumbnails` | DWORD | `1` | Master on/off for all thumbnails |
| `AllowAI` | DWORD | `1` | Enable AI post-processing pipeline |
| `AllowCloudSync` | DWORD | `1` | Allow OneDrive/SharePoint thumbnail fetch |
| `AllowPlugins` | DWORD | `1` | Enable plugin marketplace |
| `AllowTelemetry` | DWORD | `1` | Telemetry (anonymized, no file content) |
| `EnableNSFWGuard` | DWORD | `0` | Activate NSFWContentGuard (enterprise key req.) |
| `MaxCacheSizeMB` | DWORD | `512` | Maximum thumbnail cache size in MB |
| `ThumbnailTimeoutMs` | DWORD | `5000` | Per-file decode timeout |
| `DisableGPUDecode` | DWORD | `0` | Force CPU-only decoding |
| `ForceLocalCacheOnly` | DWORD | `0` | Disable network cache writes |
| `FleetTier` | DWORD | `2` | 1=Dev, 2=Standard, 3=Regulated, 4=Classified |
| `AuditLogPath` | SZ | `""` | Local SIEM log file path |

### Fleet Tier Restrictions

| Tier | Cloud Sync | Plugins | AI | NSFW Required |
|---|---|---|---|---|
| Developer (1) | ✅ | ✅ | ✅ | No |
| Standard (2) | ✅ | ✅ | ✅ | No |
| Regulated (3) | ✅ | Admin only | ✅ | **Yes** |
| Classified (4) | ❌ Blocked | ❌ Blocked | ✅ | **Yes** |

---

## MDM / Intune Deployment

Use the ExplorerLens OMA-URI Custom Configuration Profile:

```
OMA-URI: ./Device/Vendor/MSFT/Policy/Config/ExplorerLens/EnableThumbnails
Data type: Integer
Value: 1
```

Or use the Windows Package Manager (winget) integration:

```powershell
winget install ExplorerLens --override "/quiet FLEET_TIER=2"
```

---

## SIEM Audit Export

### Enabling Audit Logging

Set the audit log destination via GP:

```powershell
Set-ItemProperty -Path "HKLM:\SOFTWARE\Policies\ExplorerLens" `
    -Name "AuditLogPath" -Value "C:\Logs\ExplorerLens.jsonl"
```

### Supported Formats

| Format | Target SIEM | Field Separator |
|---|---|---|
| **CEF** (default) | ArcSight ESM | `\|` pipe |
| **LEEF** | IBM QRadar | `\t` tab |
| **JSON-L** | Splunk / Elastic / Sentinel | Newline |
| **Syslog** | Generic RFC 5424 | Structured data |

### Event Catalog

| Event ID | Name | Severity | Trigger |
|---|---|---|---|
| `LENS-DECODE-001` | DecodeComplete | Info | Successful thumbnail decode |
| `LENS-SEC-001` | DecodeBlocked | High | GP policy blocked a file type |
| `LENS-SEC-002` | NSFWGuardTriggered | High | NSFW confidence exceeded threshold |
| `LENS-POL-001` | PolicyViolation | Medium | Fleet policy constraint violated |
| `LENS-AI-001` | AIModelLoaded | Info | ONNX model loaded or hot-swapped |
| `LENS-AUTH-001` | SSOSignIn | Info | Manager console SSO authentication |
| `LENS-AUTH-002` | SSOSignOut | Info | User signed out of Manager console |
| `LENS-CACHE-001` | CacheEviction | Low | Cache size limit reached |

### Sample CEF Record

```
CEF:0|ExplorerLens|ThumbnailEngine|19.0|LENS-SEC-001|DecodeBlocked|8|
src=DESKTOP-ABC123 suser=john@contoso.com fpath=invoice.docx
outcome=blocked rt=2026-03-26T14:22:01Z
```

---

## Compliance Reporting

### Generating a Report

From LENSManager → Fleet → Compliance, or via the CLI:

```powershell
# PowerShell one-liner (lens.exe Sprint 17+)
lens.exe compliance --framework SOC2 --output report.md
```

### Supported Frameworks

| Framework | Controls Checked | Output Format |
|---|---|---|
| **SOC 2 Type II** | CC3.4, CC6.1, CC6.7, CC7.2, CC9.1 | Markdown / JSON |
| **HIPAA** | 164.312(a), 164.312(e) | Markdown / JSON |
| **ISO 27001** | A.9.4, A.12.4, A.18.2 | Markdown / JSON |
| **CIS Controls v8** | CIS 1, 3, 8, 10 | Markdown / JSON |

### Control Reference — SOC 2

| Control | Title | What LENS Checks |
|---|---|---|
| CC3.4 | Content Safety Policy | NSFWContentGuard compiled; policy document present |
| CC6.1 | Access / Group Policy | HKLM\\Policies\\ExplorerLens GP key presence |
| CC6.7 | Encryption at Rest | BitLocker boot status via registry |
| CC7.2 | Audit Logging | Audit log path configured in GP |
| CC9.1 | Telemetry Consent | TelemetryConsentManager active |

---

## Tenant Isolation

For BYOD or kiosk devices shared across Azure AD tenants, ExplorerLens enforces
per-tenant resource partitions:

- **Cache partition** — `%LOCALAPPDATA%\ExplorerLens\tenants\<tenantId>\cache\`
- **AI model namespace** — separate ONNX session per tenant (configurable)
- **Audit stream** — separate CEF log file per tenant
- **Resource limits** — max cache MB, VRAM, concurrent ops per tenant

### Registering Tenants (via FleetConfigManager)

```cpp
TenantDescriptor td;
td.tenantId   = "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx";
td.tenantName = L"Contoso";
td.limits.maxCacheMB = 128;
TenantIsolationPolicy::Instance().RegisterTenant(td);
```

---

## SSO / Authentication

### Supported Protocols

| Protocol | Provider | Notes |
|---|---|---|
| **WAM (recommended)** | Entra ID / Azure AD | Native Windows broker, seamless on W11 |
| **OIDC + PKCE** | Entra ID / Okta / Ping | Browser-based, loopback redirect |
| **SAML 2.0** | ADFS / On-prem | SP-initiated POST binding |
| **Windows Integrated Auth** | On-prem AD | Kerberos/NTLM, no browser required |

### Configuring SSO in LENSManager

1. Open **LENSManager → Fleet → SSO**
2. Enter Tenant ID and Client ID from your App Registration
3. Click **Test Authentication** — WAM browser or broker will open
4. On success, tokens are cached in Windows Credential Manager

### Required App Registration Permissions (Entra ID)

| Permission | Type | Purpose |
|---|---|---|
| `User.Read` | Delegated | User profile for audit log UPN |
| `GroupMember.Read.All` | Delegated | Policy tier from AD group membership |
| `offline_access` | Delegated | Refresh token for seamless re-auth |

---

## Fleet Health Dashboard

ExplorerLens exposes health metrics in two formats:

### Prometheus Scrape Endpoint

When `lens.exe --metrics-port 9090` is running (Sprint 17+):

```
GET http://localhost:9090/metrics

explorerlens_decode_total 124892
explorerlens_decode_errors_total 3
explorerlens_cache_hit_ratio 0.89
explorerlens_cache_used_mb 412
explorerlens_gpu_vram_used_mb 256
explorerlens_memory_pressure 0
explorerlens_health_level 0
```

### Azure Monitor Custom Metrics

Configure the JSON schema export to Log Analytics workspace:

```json
{
  "machine": "DESKTOP-ABC",
  "version": "19.1.0",
  "capturedAt": "2026-03-26T14:22:01Z",
  "overall": 0,
  "decodeTotal": 124892,
  "decodeErrors": 3,
  "cacheHitRate": 0.89,
  "cacheUsedMB": 412,
  "gpuVRAMMB": 256,
  "memPressure": 0,
  "aiLoaded": true,
  "policyCompliant": true
}
```

---

## Remote Configuration via Azure App Configuration

Centrally push settings changes to all fleet machines without GPO:

1. Create an Azure App Configuration resource
2. Add keys under the `ExplorerLens/` prefix
3. Set the endpoint in `HKLM\SOFTWARE\ExplorerLens\RemoteConfigEndpoint`
4. The engine polls every 5 minutes (configurable) and applies deltas atomically

### Example Key Names in Azure App Config

| Key | Value Example | Effect |
|---|---|---|
| `ExplorerLens/MaxCacheSizeMB` | `256` | Immediately reduces cache budget |
| `ExplorerLens/AllowAI` | `0` | Disables AI pipeline fleet-wide |
| `ExplorerLens/ThumbnailTimeoutMs` | `3000` | Tightens decode timeout |

---

## Deployment Guide

### Intune (Modern Management)

```json
{
  "oemConfig": "ExplorerLens",
  "policies": {
    "EnableThumbnails": 1,
    "FleetTier": 2,
    "MaxCacheSizeMB": 256,
    "AllowTelemetry": 0
  }
}
```

### SCCM / ConfigMgr

Deploy via the MSI package with transform:

```powershell
msiexec /i ExplorerLens-24.1.0-x64.msi `
    FLEET_TIER=2 `
    MAX_CACHE_MB=256 `
    ALLOW_TELEMETRY=0 `
    /quiet /log ExplorerLens-install.log
```

### GPO (Traditional)

1. Copy ADMX/ADML to `\\domain\SYSVOL\Policies\PolicyDefinitions\`
2. In GPMC: Computer Configuration → Administrative Templates → ExplorerLens
3. Configure desired policies
4. Link GPO to target OU
5. Run `gpupdate /force` on endpoints or wait for policy refresh (90 min default)

---

## Security Notes

- ExplorerLens **never transmits file content** to any external service
- All SIEM audit records contain metadata only (path, extension, outcome)
- NSFWContentGuard is disabled by default; enterprise license key required
- Plugin signatures are validated via RSA-PSS before loading
- All HTTPS communications use system TLS stack (Schannel) — no bundled OpenSSL

---

*ExplorerLens Enterprise — v19.1.0 "Pulsar-R" · Copyright (c) 2026 ExplorerLens Project*
