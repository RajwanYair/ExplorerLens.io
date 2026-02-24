# ExplorerLens Plugin Marketplace - Beta Registry

## Overview

The ExplorerLens Plugin Marketplace provides a centralized registry for discovering, installing, and updating plugins that extend format support and functionality.

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│              ExplorerLens Manager UI                       │
│  • Browse marketplace                                    │
│  • Search & filter plugins                               │
│  • Install/update/remove                                 │
└────────────────────┬────────────────────────────────────┘
                     │ HTTPS
┌────────────────────▼────────────────────────────────────┐
│         Marketplace Registry API (v1)                    │
│  • Plugin metadata                                       │
│  • Signature verification                                │
│  • Update channels                                       │
│  • Revocation list                                       │
└────────────────────┬────────────────────────────────────┘
                     │
         ┌───────────┴───────────┐
         │                       │
┌────────▼────────┐    ┌────────▼─────────┐
│  CDN Storage    │    │  Plugin Packages │
│  • .dtplugin    │    │  • Signed DLLs   │
│  • Manifests    │    │  • Assets        │
│  • Signatures   │    │  • Metadata      │
└─────────────────┘    └──────────────────┘
```

## Registry Schema v1.0

### Main Registry (`registry.json`)

```json
{
  "version": "1.0.0",
  "registry": {
    "name": "ExplorerLens Plugin Marketplace Beta",
    "description": "Official plugin registry",
    "baseUrl": "https://marketplace.explorerlens.org/api/v1",
    "updateIntervalHours": 24
  },
  "channels": {
    "stable": { ... },
    "beta": { ... },
    "nightly": { ... }
  },
  "plugins": [ ... ],
  "revocations": [ ... ],
  "metadata": { ... }
}
```

### Plugin Entry

```json
{
  "id": "explorerlens.plugin.psd",
  "name": "Adobe Photoshop (PSD) Support",
  "version": "1.0.0",
  "description": "...",
  "author": "ExplorerLens Project",
  "category": "Creative Formats",
  "channel": "stable",
  "verified": true,
  "signatures": {
    "dll": "sha256:...",
    "manifest": "sha256:..."
  },
  "downloadUrl": "https://...",
  "minEngineVersion": "6.0.0",
  "capabilities": ["READ_FILE", "DECODE"],
  "supportedExtensions": [".psd", ".psb"],
  "size": 524288,
  "publishedAt": "2026-01-06T00:00:00Z"
}
```

## Update Channels

### Stable Channel

- **Target Audience:** Production users
- **Requirements:**
  - Code signed
  - Verified by ExplorerLens team
  - Passed compatibility test kit
  - 30-day beta period completed
- **Auto-Update:** Enabled by default
- **Support:** Full support provided

### Beta Channel

- **Target Audience:** Early adopters, testers
- **Requirements:**
  - Code signed
  - Basic compatibility checks
- **Auto-Update:** Disabled by default
- **Support:** Community support, bug reports accepted

### Nightly Channel

- **Target Audience:** Developers, plugin authors
- **Requirements:**
  - Minimal (manifest validation only)
- **Auto-Update:** Disabled
- **Support:** No support, use at own risk

## Plugin Verification

To achieve "Verified" status, plugins must pass:

### 1. Security Checks

- [ ] Code signing with valid certificate
- [ ] Static analysis (no known malware patterns)
- [ ] Dependency audit (no vulnerable libraries)
- [ ] Capability review (minimal required permissions)

### 2. Compatibility Tests

- [ ] Compatibility test kit passes
- [ ] Works on Windows 10 (2004+) and Windows 11
- [ ] No crashes on sample files (1000+ samples)
- [ ] Performance within budgets (< 100ms per thumbnail)

### 3. Quality Standards

- [ ] Documentation provided
- [ ] Sample files included for testing
- [ ] Error handling (no unhandled exceptions)
- [ ] Logging integrated with ExplorerLens

### 4. Legal Compliance

- [ ] License specified (OSI-approved preferred)
- [ ] Third-party dependencies documented
- [ ] Patent/IP clearance (if applicable)

## Plugin Submission

### For Plugin Authors

1. **Develop Plugin**
   - Follow [SDK Guide](../docs/sdk/plugin-development.md)
   - Implement required interfaces
   - Test with compatibility kit

2. **Create Package**

   ```
   plugin.dtplugin
     manifest.json
     plugin.dll (signed)
     assets/
     licenses/
     README.md
   ```

3. **Submit for Review**
   - Create GitHub repository
   - Tag release (e.g., v1.0.0)
   - Open submission issue: <https://github.com/explorerlens/marketplace/issues/new>
   - Template will request:
     - Plugin ID
     - Repository URL
     - Release tag
     - Test results
     - License

4. **Verification Process**
   - Automated checks run (1-2 hours)
   - Manual review (2-5 business days)
   - Feedback provided via issue
   - Once approved, added to beta channel

5. **Promotion to Stable**
   - 30-day beta period
   - No critical bugs reported
   - Positive user feedback
   - Manual promotion by maintainers

## Security

### Signature Verification

All plugins must be signed. ExplorerLens verifies:

1. **DLL Signature**
   - Code signing certificate valid
   - Signed with SHA-256
   - Timestamped

2. **Manifest Signature**
   - manifest.json hash matches
   - All files listed in manifest

3. **Revocation Check**
   - Plugin not in revocation list
   - Certificate not revoked

### Revocation Process

If a plugin is found to be malicious or vulnerable:

1. **Immediate Revocation**
   - Added to `revocations` list in registry
   - Distributed within 1 hour

2. **User Notification**
   - Manager shows critical warning
   - Plugin disabled automatically
   - Uninstall recommended

3. **Communication**
   - Security advisory published
   - GitHub issue created
   - Email to affected users (if email collected)

### Example Revocation

```json
{
  "pluginId": "explorerlens.plugin.malicious",
  "reason": "Security vulnerability CVE-2026-XXXXX",
  "revokedAt": "2026-01-01T00:00:00Z",
  "severity": "critical",
  "message": "This plugin contains a critical security vulnerability. Please uninstall immediately."
}
```

## API Endpoints (Beta)

### GET /plugins

List all plugins in registry.

**Query Parameters:**

- `channel` - Filter by channel (stable, beta, nightly)
- `category` - Filter by category
- `search` - Text search

**Response:**

```json
{
  "plugins": [ ... ],
  "total": 25,
  "page": 1
}
```

### GET /plugins/{id}

Get detailed information for specific plugin.

**Response:**

```json
{
  "id": "explorerlens.plugin.psd",
  "name": "Adobe Photoshop (PSD) Support",
  ...
}
```

### GET /plugins/{id}/download

Redirect to signed download URL.

**Response:** 302 Redirect to CDN

### GET /revocations

Get list of revoked plugins.

**Response:**

```json
{
  "revocations": [ ... ],
  "lastUpdated": "2026-01-06T09:00:00Z"
}
```

## Integration with ExplorerLens Manager

### Plugin Discovery

```cpp
// Manager fetches registry periodically
auto registry = PluginRegistry::Load("https://marketplace.explorerlens.org/api/v1/plugins");

// Filter by category
auto creativePlugins = registry.GetByCategory("Creative Formats");

// Display in UI
for (const auto& plugin : creativePlugins) {
    ui->AddPlugin(plugin);
}
```

### Installation Flow

1. User clicks "Install" on plugin
2. Manager checks:
   - Engine version compatibility
   - Required capabilities
   - Signature validity
   - Revocation status
3. Download `.dtplugin` package
4. Verify signatures
5. Extract to plugin directory
6. Register with PluginManager
7. Restart required notification

### Update Flow

1. Manager checks for updates (daily)
2. Compare installed version with registry
3. If newer version available:
   - Show update notification
   - If auto-update enabled (stable channel only):
     - Download and install
   - Else:
     - Show "Update available" in UI

## Marketplace Hosting

### Self-Hosted Registry

Organizations can host private registries:

1. **Fork Registry Schema**

   ```bash
   git clone https://github.com/explorerlens/marketplace
   ```

2. **Configure Base URL**

   ```json
   {
     "registry": {
       "baseUrl": "https://internal.company.com/plugins/api/v1"
     }
   }
   ```

3. **Update Manager Configuration**

   ```
   HKCU\Software\ExplorerLens\Marketplace\RegistryUrl = "https://internal.company.com/plugins/registry.json"
   ```

### Enterprise Considerations

- **Whitelist Mode:** Only install from approved registry
- **Signature Requirement:** Enforce internal code signing
- **Offline Mode:** Cache registry locally
- **Audit Logging:** Track plugin installations

## Future Enhancements (Post-v6.0)

- **Web UI:** Browse marketplace at marketplace.explorerlens.org
- **Ratings & Reviews:** User feedback system
- **Plugin Analytics:** Download stats, usage metrics
- **Automated Testing:** CI/CD for plugin submissions
- **Dependency Management:** Plugin can depend on other plugins
- **Multi-Language Support:** Localized plugin descriptions

## Support

- **Plugin Submission Questions:** <marketplace@explorerlens.org>
- **Security Issues:** <security@explorerlens.org>
- **General Support:** <support@explorerlens.org>

---

**Registry Version:** 1.0.0  
**Last Updated:** January 6, 2026  
**Status:** Beta (v6.0.0 Launch)

