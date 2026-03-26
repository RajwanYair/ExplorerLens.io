# ExplorerLens Deployment Guide

> Version 19.2.0 "Pulsar-S" · Enterprise Edition  
> [ENTERPRISE.md](ENTERPRISE.md) · [TROUBLESHOOTING.md](TROUBLESHOOTING.md)

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Installation Methods](#installation-methods)
3. [Silent / Unattended Install](#silent--unattended-install)
4. [Group Policy Deployment (ADMX)](#group-policy-deployment-admx)
5. [Intune / MDM Deployment](#intune--mdm-deployment)
6. [SCCM / ConfigMgr Deployment](#sccm--configmgr-deployment)
7. [Fleet Tier Configuration](#fleet-tier-configuration)
8. [Plugin Deployment Policy](#plugin-deployment-policy)
9. [SIEM Integration](#siem-integration)
10. [Telemetry Consent for GDPR Environments](#telemetry-consent-for-gdpr-environments)
11. [Crash Reporting Configuration](#crash-reporting-configuration)
12. [Uninstallation](#uninstallation)
13. [Troubleshooting Checklist](#troubleshooting-checklist)

---

## Prerequisites

| Requirement | Minimum | Recommended |
|---|---|---|
| Windows | 10 22H2 (19045) | 11 24H2 |
| Architecture | x64 | x64 + ARM64 |
| DirectX | 11 Feature Level 11.0 | 12 Feature Level 12.0 |
| RAM | 4 GB | 16 GB |
| Disk (app) | 50 MB | 100 MB |
| Disk (cache) | 512 MB | 2 GB |
| .NET Runtime | Not required | — |
| Visual C++ Redist | Included in MSI | — |

> **Note:** ExplorerLens ships the Visual C++ 2026 (v145) runtime within the MSI.
> No separate VC++ Redist deployment is required.

---

## Installation Methods

### Option 1 — MSI (Recommended for Enterprise)

```powershell
# Interactive install
.\ExplorerLens-19.2.0-x64.msi

# Silent install (see §3 for full property list)
msiexec /i ExplorerLens-19.2.0-x64.msi /quiet /log "%TEMP%\lens-install.log"
```

### Option 2 — WinGet (Consumer / Developer)

```powershell
winget install ExplorerLens.ExplorerLens --version 19.2.0
```

### Option 3 — Portable ZIP

Extract `ExplorerLens-19.2.0-x64.zip` and run `Install-ExplorerLens.ps1 -Register`:

```powershell
Expand-Archive ExplorerLens-19.2.0-x64.zip -DestinationPath "C:\Tools\ExplorerLens"
Set-Location C:\Tools\ExplorerLens
.\Install-ExplorerLens.ps1 -Register
```

### Option 4 — MSIX / Store Package

Sideload via `Add-AppxPackage` or distribute via Microsoft Store for Business:

```powershell
Add-AppxPackage .\ExplorerLens-19.2.0-x64.msix
```

---

## Silent / Unattended Install

```powershell
msiexec /i ExplorerLens-19.2.0-x64.msi `
    FLEET_TIER=2 `
    MAX_CACHE_MB=512 `
    ALLOW_TELEMETRY=1 `
    ALLOW_AI=1 `
    ALLOW_CLOUD_SYNC=1 `
    UPDATE_CHANNEL=0 `
    AUDIT_LOG_PATH="C:\Logs\ExplorerLens" `
    REBOOT=ReallySuppress `
    /quiet `
    /log "%TEMP%\lens-install.log"
```

### MSI Property Reference

| Property | Values | Default | Description |
|---|---|---|---|
| `FLEET_TIER` | 1–4 | `2` | Fleet security tier |
| `MAX_CACHE_MB` | 128–8192 | `512` | Thumbnail cache size cap |
| `ALLOW_TELEMETRY` | 0/1 | `1` | Anonymized telemetry |
| `ALLOW_AI` | 0/1 | `1` | AI post-processing pipeline |
| `ALLOW_CLOUD_SYNC` | 0/1 | `1` | OneDrive thumbnail pre-fetch |
| `ALLOW_PLUGINS` | 0/1 | `1` | Plugin marketplace |
| `UPDATE_CHANNEL` | 0=Stable 1=Preview 2=Insider 3=Disabled | `0` | Auto-update channel |
| `DISABLE_GPU` | 0/1 | `0` | Force CPU-only decode |
| `AUDIT_LOG_PATH` | Path string | `""` | SIEM log file directory |
| `ENABLE_NSFW_GUARD` | 0/1 | `0` | NSFWContentGuard (license required) |

---

## Group Policy Deployment (ADMX)

### Step 1 — Copy ADMX Templates

```
packaging\admx\ExplorerLens.admx  →  \\domain\SYSVOL\domain\Policies\PolicyDefinitions\
packaging\admx\en-US\ExplorerLens.adml  →  ...\PolicyDefinitions\en-US\
```

Or for local machine testing:

```
C:\Windows\PolicyDefinitions\ExplorerLens.admx
C:\Windows\PolicyDefinitions\en-US\ExplorerLens.adml
```

### Step 2 — Create GPO

1. Open **Group Policy Management Console (GPMC)**
2. Right-click target OU → **Create a GPO in this domain**
3. Name: `ExplorerLens-Production-v19`

### Step 3 — Configure Settings

Navigate to: **Computer Configuration → Administrative Templates → ExplorerLens**

| Setting | Recommended Value | Notes |
|---|---|---|
| Enable Thumbnails | Enabled | Required for any functionality |
| Fleet Tier | Standard (2) | Adjust per OU security level |
| Max Cache Size (MB) | 512 | Tune per device storage |
| Allow AI Processing | Enabled | Disable on < 4 GB RAM devices |
| Allow Telemetry | Enabled | Set to Disabled for maximum privacy |
| Thumbnail Timeout (ms) | 5000 | Reduce on high-volume servers |

### Step 4 — Link and Enforce

```powershell
# PowerShell GPMC linkage
$gpo = Get-GPO -Name "ExplorerLens-Production-v19"
New-GPLink -Name $gpo.DisplayName -Target "OU=Workstations,DC=contoso,DC=com" -Enforced Yes
Invoke-GPUpdate -Computer "workstation001" -Force
```

---

## Intune / MDM Deployment

### Custom OMA-URI Profile

Create a new **Custom Configuration Profile** (Windows 10/11) with these OMA-URI entries:

```
OMA-URI: ./Device/Vendor/MSFT/Policy/Config/ExplorerLens/EnableThumbnails
Data type: Integer | Value: 1

OMA-URI: ./Device/Vendor/MSFT/Policy/Config/ExplorerLens/FleetTier
Data type: Integer | Value: 2

OMA-URI: ./Device/Vendor/MSFT/Policy/Config/ExplorerLens/MaxCacheSizeMB
Data type: Integer | Value: 512

OMA-URI: ./Device/Vendor/MSFT/Policy/Config/ExplorerLens/AllowTelemetry
Data type: Integer | Value: 1
```

### Win32 App Deployment (MSI)

1. Wrap in IntuneWinAppUtil: `IntuneWinAppUtil.exe -c . -s ExplorerLens-19.2.0-x64.msi -o .`
2. Upload `.intunewin` to Intune → Apps → Windows → Add
3. Install command: `msiexec /i ExplorerLens-19.2.0-x64.msi /quiet FLEET_TIER=2`
4. Detection rule: Registry key `HKLM\SOFTWARE\ExplorerLens` value `Version` = `19.2.0`

---

## SCCM / ConfigMgr Deployment

```powershell
# Create Application
New-CMApplication -Name "ExplorerLens 19.2.0" -Publisher "ExplorerLens Project" -SoftwareVersion "19.2.0"

# Add MSI Deployment Type
Add-CMMsiDeploymentType `
    -ApplicationName "ExplorerLens 19.2.0" `
    -ContentLocation "\\sccm-share\ExplorerLens\19.2.0\" `
    -DeploymentTypeName "ExplorerLens-MSI" `
    -InstallCommand 'msiexec /i ExplorerLens-19.2.0-x64.msi /quiet FLEET_TIER=2' `
    -UninstallCommand 'msiexec /x {LENS-PRODUCT-GUID} /quiet'
```

---

## Fleet Tier Configuration

| Tier | Value | Description | Restrictions |
|---|---|---|---|
| Developer | 1 | Dev machines, all features | None |
| Standard | 2 | Corporate workstations | Default policy |
| Regulated | 3 | HIPAA / FedRAMP / PCI | NSFW required, cloud approval needed |
| Classified | 4 | Air-gapped / classified | Cloud + plugins disabled |

```powershell
# Set tier via registry directly (or use GP/MDM above)
$path = "HKLM:\SOFTWARE\Policies\ExplorerLens"
New-Item $path -Force | Out-Null
Set-ItemProperty $path -Name "FleetTier"    -Value 2 -Type DWORD
Set-ItemProperty $path -Name "MaxCacheSizeMB" -Value 512 -Type DWORD
Set-ItemProperty $path -Name "AllowTelemetry" -Value 1 -Type DWORD
```

---

## Plugin Deployment Policy

### Allow-list via GP

```
HKLM\SOFTWARE\Policies\ExplorerLens\AllowedPlugins
  Type: REG_MULTI_SZ
  Value: (one plugin ID per line, or * for all)
```

### Block-list

```
HKLM\SOFTWARE\Policies\ExplorerLens\BlockedPlugins
  Type: REG_MULTI_SZ
  Value: (plugin IDs to block)
```

---

## SIEM Integration

```powershell
# Configure audit log path
Set-ItemProperty "HKLM:\SOFTWARE\Policies\ExplorerLens" `
    -Name "AuditLogPath" -Value "C:\Logs\ExplorerLens" -Type String

# Configure SIEM format (0=CEF, 1=LEEF, 2=JSON-L)
Set-ItemProperty "HKLM:\SOFTWARE\Policies\ExplorerLens" `
    -Name "AuditLogFormat" -Value 2 -Type DWORD
```

Forward the log file to your SIEM:

| SIEM | Agent | Config |
|---|---|---|
| Splunk | Universal Forwarder | `inputs.conf` → `[monitor://C:\Logs\ExplorerLens\*.jsonl]` |
| Elastic | Filebeat | `filebeat.yml` → `paths: [C:\Logs\ExplorerLens\*.jsonl]` |
| Microsoft Sentinel | Azure Monitor Agent | DCR with JSON-L parser |
| IBM QRadar | WinCollect | LEEF format input |

---

## Telemetry Consent for GDPR Environments

Disable **all** telemetry via GP for GDPR-sensitive deployments:

```
HKLM\SOFTWARE\Policies\ExplorerLens
  AllowTelemetry    = 0  (REG_DWORD)
  TelemetryLevel    = 0  (REG_DWORD)
```

When `AllowTelemetry = 0`, `TelemetryConsentManager.ConsentGate()` blocks all emission
including anonymized counters. Crash reports to WER are separately controlled by
`HKLM\SOFTWARE\Microsoft\Windows\Windows Error Reporting`.

---

## Crash Reporting Configuration

```powershell
# Set custom crash dump directory (default: %LOCALAPPDATA%\ExplorerLens\crashes)
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" `
    -Name "CrashDumpPath" -Value "C:\CrashDumps\ExplorerLens" -Type String

# Dump type: 0=Triage(default), 1=Mini, 2=WithHeap
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" `
    -Name "CrashDumpType" -Value 0 -Type DWORD
```

Triage dumps (`MiniDumpFilterMemory`) are small (~500 KB) and contain no heap content,
making them safe to forward even in classified or regulated environments.

---

## Uninstallation

```powershell
# Silent uninstall via MSI
msiexec /x ExplorerLens-19.2.0-x64.msi /quiet

# Remove registry settings (optional — leaves user cache)
Remove-Item "HKLM:\SOFTWARE\ExplorerLens" -Recurse -Force -ErrorAction SilentlyContinue

# Remove cache (caution: removes all cached thumbnails)
Remove-Item "$env:LOCALAPPDATA\ExplorerLens" -Recurse -Force -ErrorAction SilentlyContinue
```

---

## Troubleshooting Checklist

| Symptom | Check |
|---|---|
| No thumbnails in Explorer | `EnableThumbnails` GP key = 1; re-register DLL: `regsvr32 LENSShell.dll` |
| Thumbnails slow (> 17ms) | Check GPU driver; set `DisableGPUDecode=0` in GP |
| Cache not growing | Verify `MaxCacheSizeMB` > 0; check disk space on cache drive |
| AI features not active | Verify ONNX models in `%ProgramData%\ExplorerLens\models\` |
| NSFW guard not activating | Requires enterprise license key in `HKLM\SOFTWARE\ExplorerLens\NSFWKey` |
| SIEM not receiving events | Check `AuditLogPath` write permissions; verify SIEM forwarder is running |
| SSO login fails | Check tenant ID + client ID in LENSManager → Fleet → SSO; verify App Registration permissions |
| Plugin fails to load | Check `AllowPlugins=1` GP; inspect `%TEMP%\ExplorerLens-plugin.log` for signature error |
| Crash dumps not created | Verify `%LOCALAPPDATA%\ExplorerLens\crashes\` exists and is writable |

For advanced diagnostics, connect to the DiagnosticsConsole named pipe:

```powershell
# PowerShell diagnostic session
$pipe = New-Object System.IO.Pipes.NamedPipeClientStream(".", "ExplorerLensDiag", "InOut")
$pipe.Connect(1000)
$w = New-Object System.IO.StreamWriter($pipe); $r = New-Object System.IO.StreamReader($pipe)
$w.WriteLine("status"); $w.Flush(); $r.ReadLine()
```

---

*ExplorerLens Deployment Guide · v19.2.0 "Pulsar-S" · Copyright (c) 2026 ExplorerLens Project*
