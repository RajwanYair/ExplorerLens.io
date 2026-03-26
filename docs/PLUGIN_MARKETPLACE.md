# Plugin Marketplace

ExplorerLens v18.3.0+ includes a plugin marketplace that allows community and vendor
plugins to extend thumbnail support to any file format via signed `.lenspkg` packages.

---

## Overview

```
User opens LENSManager → Plugins tab
        │
        ▼
MarketplaceClient.Query()  ──→  plugins.explorerlens.io/api/v1
        │
        ▼
PluginSignatureVerifier.Verify()
        │
        ▼
PluginVersionResolver.Resolve()  (dependency graph)
        │
        ▼
PluginInstaller.Install()
        │
        ▼
Plugin DLL loaded into sandbox (PluginSandboxPolicy)
        │
        ▼
PluginSearchIndex.Upsert()  →  LENSShell routes formats to plugin
```

---

## Package Format (.lenspkg)

A `.lenspkg` is a signed ZIP file with the following structure:

```
my-plugin.lenspkg (ZIP)
├── manifest.json          ← PluginPackageManifest (required)
├── MyFormatDecoder.dll    ← Plugin DLL (IThumbnailPlugin C ABI)
├── icon.png               ← 64×64 marketplace icon
└── README.md              ← Optional user docs
```

### manifest.json Fields

| Field              | Required | Description                                |
|--------------------|----------|--------------------------------------------|
| `id`               | Yes      | Reverse-DNS ID e.g. `"com.vendor.myformat"` |
| `displayName`      | Yes      | User-visible plugin name                   |
| `version`          | Yes      | SemVer e.g. `"1.0.0"`                      |
| `author`           | Yes      | Author or organization name               |
| `licenseId`        | Yes      | SPDX identifier e.g. `"MIT"`               |
| `minEngineVersion` | Yes      | Minimum compatible ExplorerLens version    |
| `dllName`          | Yes      | DLL filename inside the package            |
| `extensions`       | Yes      | File extensions e.g. `[".xyz", ".abc"]`   |
| `capabilities`     | Yes      | Bitmask of `PluginCapabilityFlag`          |
| `sha256`           | Yes      | SHA-256 of the ZIP content                 |
| `signatureB64`     | Yes      | Base64 RSA-PSS signature over sha256       |

---

## Signing Your Plugin

All marketplace plugins **must** be signed with the ExplorerLens Plugin CA:

```bash
# 1. Request a signing certificate from plugins.explorerlens.io/dev
# 2. Sign the .lenspkg SHA-256 hash with your private key (RSA-PSS SHA-256)
# 3. Embed the Base64 signature in manifest.json "signatureB64"
```

Self-signed plugins can be sideloaded via LENSManager with `AllowUnsigned` policy
(enterprise environments only — strongly discouraged for end-users).

---

## Installation

### Via LENSManager (GUI)

1. Open **LENSManager** → **Plugins** → **Browse Marketplace**
2. Search by format or keyword
3. Click **Install** — no admin rights required

### Via Command Line

```powershell
lens.exe plugin install com.vendor.myformat
lens.exe plugin install --file ./my-plugin.lenspkg
lens.exe plugin list
lens.exe plugin uninstall com.vendor.myformat
lens.exe plugin update --all
```

---

## Auto-Updates

`PluginUpdateScheduler` checks for updates every 24 hours (default):

| Policy         | Behavior                                    |
|----------------|---------------------------------------------|
| `Disabled`     | No checks, no downloads                     |
| `CheckOnly`    | Notify user in LENSManager; no auto-download |
| `AutoDownload` | Download silently; user confirms apply      |
| `AutoApply`    | Download and apply at idle (default)        |

Failed updates automatically roll back to the previous version after 3 consecutive load failures.

---

## Sandbox Policy

Each plugin runs in a restricted sandbox with:
- **File I/O** limited to the input file path only (no `CreateFile` to other paths)
- **Registry** access limited to declared `allowedRegistryPaths` in the manifest
- **Network** blocked unless `requiresNetwork: true` in manifest
- **Execution time** capped at 10 seconds per thumbnail request

A plugin that exceeds the time limit is terminated and `PluginUsageTracker` records the failure. After 3 crashes in a session, the plugin is auto-disabled.

---

## Developer Guide

See [docs/PLUGIN_DEVELOPMENT.md](PLUGIN_DEVELOPMENT.md) for the full plugin SDK reference, C ABI specification, and sample decoder implementation.

Registry submission: [plugins.explorerlens.io/submit](https://plugins.explorerlens.io/submit)
