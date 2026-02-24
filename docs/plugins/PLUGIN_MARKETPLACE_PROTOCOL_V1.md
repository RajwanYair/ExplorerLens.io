# Plugin Marketplace & Registry Protocol v1.0

**Version:** 1.0  
**Sprint:** 12 (Sprint 12.5)  
**Created:** January 6, 2026

---

## Overview

The ExplorerLens Plugin Marketplace provides a centralized, trusted repository for discovering, installing, and updating plugins. This document defines:

- Registry API schema
- Update channels
- Signature and revocation mechanisms
- Compatibility testing requirements
- Verification process

---

## Registry API Endpoints

Base URL: `https://api.marketplace.explorerlens.dev/v1`

### 1. List Plugins

**Endpoint:** `GET /plugins`

**Query Parameters:**

- `category` (optional): Filter by category
- `format` (optional): Filter by supported format (e.g., `.psd`)
- `verified` (optional): Show only verified plugins (`true`/`false`)
- `page` (optional): Page number (default: 1)
- `limit` (optional): Results per page (default: 20, max: 100)

**Response:**

```json
{
  "version": "1.0",
  "timestamp": "2026-01-06T14:30:00Z",
  "total": 127,
  "page": 1,
  "limit": 20,
  "plugins": [
    {
      "id": "explorerlens.plugin.psd",
      "name": "Photoshop Document Decoder",
      "vendor": "Adobe Systems Inc.",
      "description": "Native PSD thumbnail decoder with layer support",
      "version": "1.2.3",
      "category": "Image Decoders",
      "verified": true,
      "rating": 4.8,
      "downloads": 125000,
      "lastUpdated": "2026-01-05T10:00:00Z",
      "icon": "https://cdn.marketplace.explorerlens.dev/plugins/psd/icon-256.png",
      "formats": [".psd", ".psb"],
      "abiVersion": 1,
      "minEngineVersion": "6.0.0",
      "maxEngineVersion": "6.9.99"
    }
  ]
}
```

---

### 2. Get Plugin Details

**Endpoint:** `GET /plugins/{pluginId}`

**Response:**

```json
{
  "id": "explorerlens.plugin.psd",
  "name": "Photoshop Document Decoder",
  "vendor": "Adobe Systems Inc.",
  "description": "Native PSD thumbnail decoder with layer support",
  "longDescription": "Full markdown description...",
  "version": "1.2.3",
  "category": "Image Decoders",
  "verified": true,
  "rating": 4.8,
  "ratingsCount": 3425,
  "downloads": 125000,
  "lastUpdated": "2026-01-05T10:00:00Z",
  "created": "2025-03-15T08:00:00Z",
  "homepage": "https://github.com/adobe/explorerlens-psd",
  "repository": "https://github.com/adobe/explorerlens-psd",
  "license": "MIT",
  "author": {
    "name": "John Doe",
    "email": "john@adobe.com",
    "url": "https://adobe.com"
  },
  "icon": "https://cdn.marketplace.explorerlens.dev/plugins/psd/icon-256.png",
  "screenshots": [
    "https://cdn.marketplace.explorerlens.dev/plugins/psd/screenshot-1.png",
    "https://cdn.marketplace.explorerlens.dev/plugins/psd/screenshot-2.png"
  ],
  "formats": [
    {
      "extension": ".psd",
      "mimeType": "image/vnd.adobe.photoshop",
      "description": "Adobe Photoshop Document"
    },
    {
      "extension": ".psb",
      "mimeType": "image/vnd.adobe.photoshop",
      "description": "Adobe Photoshop Large Document"
    }
  ],
  "capabilities": ["read_file", "decode", "metadata"],
  "abiVersion": 1,
  "minEngineVersion": "5.4.0",
  "maxEngineVersion": "6.9.99",
  "dependencies": {
    "runtime": ["vcredist.x64.14.0"],
    "plugins": []
  },
  "channels": {
    "stable": {
      "version": "1.2.3",
      "releaseDate": "2026-01-05",
      "downloadUrl": "https://cdn.marketplace.explorerlens.dev/plugins/psd/psd-1.2.3.dtplugin",
      "sha256": "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
      "size": 2458624,
      "changelog": "Fixed layer rendering bug, improved performance"
    },
    "beta": {
      "version": "1.3.0-beta.1",
      "releaseDate": "2026-01-01",
      "downloadUrl": "https://cdn.marketplace.explorerlens.dev/plugins/psd/psd-1.3.0-beta.1.dtplugin",
      "sha256": "...",
      "size": 2459136,
      "changelog": "Added Smart Object support"
    }
  },
  "verification": {
    "verified": true,
    "verifiedDate": "2025-12-15",
    "certificate": {
      "thumbprint": "A1B2C3D4E5F6...",
      "subject": "CN=Adobe Systems Inc.",
      "issuer": "CN=DigiCert SHA2 Code Signing CA",
      "validFrom": "2025-01-01T00:00:00Z",
      "validTo": "2028-01-01T00:00:00Z"
    },
    "compatibilityTestPassed": true,
    "testKitVersion": "1.0.0",
    "sourceReview": true,
    "securityAudit": true
  },
  "reviews": {
    "average": 4.8,
    "count": 3425,
    "breakdown": {
      "5": 2850,
      "4": 425,
      "3": 100,
      "2": 35,
      "1": 15
    }
  }
}
```

---

### 3. Search Plugins

**Endpoint:** `GET /plugins/search?q={query}`

**Query Parameters:**

- `q`: Search query
- `category`, `verified`, `page`, `limit`: Same as List Plugins

**Response:** Same format as List Plugins

---

### 4. Download Plugin

**Endpoint:** `GET /plugins/{pluginId}/download`

**Query Parameters:**

- `channel` (optional): `stable` (default), `beta`, `nightly`
- `version` (optional): Specific version (e.g., `1.2.3`)

**Response:** Redirects to CDN download URL

---

### 5. Check for Updates

**Endpoint:** `POST /plugins/check-updates`

**Request Body:**

```json
{
  "plugins": [
    {
      "id": "explorerlens.plugin.psd",
      "version": "1.2.0",
      "channel": "stable"
    },
    {
      "id": "explorerlens.plugin.webp",
      "version": "2.1.5",
      "channel": "stable"
    }
  ]
}
```

**Response:**

```json
{
  "updates": [
    {
      "id": "explorerlens.plugin.psd",
      "currentVersion": "1.2.0",
      "availableVersion": "1.2.3",
      "updateAvailable": true,
      "breaking": false,
      "downloadUrl": "https://cdn.marketplace.explorerlens.dev/plugins/psd/psd-1.2.3.dtplugin",
      "sha256": "...",
      "size": 2458624,
      "changelog": "Fixed layer rendering bug, improved performance"
    },
    {
      "id": "explorerlens.plugin.webp",
      "currentVersion": "2.1.5",
      "availableVersion": "2.1.5",
      "updateAvailable": false
    }
  ]
}
```

---

### 6. Get Revocation List

**Endpoint:** `GET /revocations`

**Response:**

```json
{
  "version": 1,
  "timestamp": "2026-01-06T14:30:00Z",
  "revocations": [
    {
      "pluginId": "explorerlens.plugin.malicious",
      "version": "1.0.5",
      "reason": "Critical security vulnerability (CVE-2026-1234)",
      "date": "2026-01-06",
      "severity": "critical",
      "action": "block",
      "details": "https://security.explorerlens.dev/CVE-2026-1234"
    }
  ]
}
```

**Actions:**

- `warn`: Show warning, allow user to continue
- `block`: Disable plugin, prevent loading
- `uninstall`: Automatically uninstall plugin

---

## Update Channels

### Stable

- Production-ready releases
- Recommended for all users
- Thoroughly tested
- Update frequency: ~1-2 months

### Beta

- Pre-release testing
- New features, not fully tested
- For advanced users
- Update frequency: ~1-2 weeks

### Nightly

- Latest development builds
- Bleeding edge, may be unstable
- For developers and testers
- Update frequency: Daily

---

## Plugin Verification Process

To achieve "Verified" status, plugins must complete:

### 1. Compatibility Test Kit

Run automated test suite covering:

- ABI compliance
- Format handling (100+ test files per format)
- Performance (< 500ms avg thumbnail time)
- Stability (10,000 requests without crash)
- Memory usage (< 100MB per request)
- Error handling (corrupt/truncated files)

**Test Kit:** <https://github.com/explorerlens/plugin-test-kit>

**Submission:**

```bash
# Run test kit
.\PluginTestKit.exe --plugin MyPlugin.dtplugin --output results.json

# Submit results
curl -X POST https://api.marketplace.explorerlens.dev/v1/verification/submit \
  -H "Authorization: Bearer {api-key}" \
  -F "pluginId=explorerlens.plugin.myplugin" \
  -F "results=@results.json"
```

### 2. Source Code Review

- Public GitHub repository (preferred)
- Or: Submit source for private review
- Reviewers check for:
  - Security vulnerabilities
  - Malicious code
  - License compliance
  - Code quality

### 3. Security Audit

- Static analysis (CodeQL, Coverity)
- Dynamic analysis (fuzzing, sanitizers)
- Manual security review
- No known CVEs

### 4. Vendor Verification

- Verify vendor identity
- Contact information (email, support site)
- Active support commitment
- Response time SLA (< 48 hours)

### 5. Code Signing

- EV (Extended Validation) code signing certificate
- Certificate from trusted CA (DigiCert, GlobalSign, etc.)
- Plugin DLL must be Authenticode signed
- Certificate not revoked

---

## Signature Verification

### At Download Time

```cpp
bool VerifyPluginSignature(const std::wstring& pluginPath) {
    // 1. Verify Authenticode signature
    if (!VerifyAuthenticodeSignature(pluginPath)) {
        return false;
    }
    
    // 2. Check certificate chain
    if (!VerifyCertificateChain(pluginPath)) {
        return false;
    }
    
    // 3. Check revocation status (OCSP/CRL)
    if (IsCertificateRevoked(pluginPath)) {
        return false;
    }
    
    // 4. Verify hash matches manifest
    std::string actualHash = ComputeSha256(pluginPath);
    if (actualHash != manifestHash) {
        return false;
    }
    
    return true;
}
```

### At Load Time

```cpp
bool VerifyPluginIntegrity(LoadedPlugin* plugin) {
    // 1. Re-verify hash (detect tampering)
    std::string actualHash = ComputeSha256(plugin->GetBinaryPath());
    if (actualHash != plugin->GetManifest().binarySha256) {
        return false;
    }
    
    // 2. Check revocation list
    if (IsPluginRevoked(plugin->GetManifest().id, 
                        plugin->GetManifest().version)) {
        return false;
    }
    
    // 3. Verify ABI compatibility
    const DT_PluginInfo* info = plugin->GetInfo();
    if (info->abiVersion != DT_PLUGIN_ABI_VERSION) {
        return false;
    }
    
    return true;
}
```

---

## Marketplace Submission Process

### Step 1: Create Account

1. Visit <https://marketplace.explorerlens.dev>
2. Sign in with GitHub, Microsoft, or email
3. Create vendor profile
4. Verify email and identity

### Step 2: Prepare Plugin

1. Implement plugin using SDK
2. Create manifest.json
3. Package as .dtplugin (ZIP)
4. Sign DLL with code signing certificate
5. Run Compatibility Test Kit

### Step 3: Submit Plugin

1. Upload .dtplugin to marketplace
2. Fill out plugin details form
3. Upload screenshots, documentation
4. Submit test results
5. Pay submission fee (if applicable)

### Step 4: Review Process

1. Automated checks (ABI, hashes, signatures)
2. Compatibility test verification
3. Security scan (static analysis)
4. Manual code review (if open source)
5. Approval/rejection notification

### Step 5: Publication

1. Plugin appears in marketplace
2. Users can discover and install
3. Automatic update notifications
4. Analytics dashboard available

---

## Plugin Metadata Schema (marketplace.json)

Hosted at plugin repository root:

```json
{
  "plugin": {
    "id": "explorerlens.plugin.psd",
    "name": "Photoshop Document Decoder",
    "vendor": "Adobe Systems Inc."
  },
  "marketplace": {
    "category": "Image Decoders",
    "tags": ["photoshop", "psd", "layers", "adobe"],
    "icon": "https://raw.githubusercontent.com/adobe/explorerlens-psd/main/assets/icon-256.png",
    "screenshots": [
      "https://raw.githubusercontent.com/adobe/explorerlens-psd/main/assets/screenshot-1.png"
    ]
  },
  "support": {
    "homepage": "https://github.com/adobe/explorerlens-psd",
    "issues": "https://github.com/adobe/explorerlens-psd/issues",
    "email": "support@adobe.com",
    "documentation": "https://github.com/adobe/explorerlens-psd/wiki"
  },
  "updates": {
    "updateUrl": "https://raw.githubusercontent.com/adobe/explorerlens-psd/main/update.json"
  }
}
```

---

## Analytics & Telemetry

### Plugin Performance Metrics (Opt-in)

```json
{
  "pluginId": "explorerlens.plugin.psd",
  "version": "1.2.3",
  "metrics": {
    "totalRequests": 1523,
    "successfulRequests": 1489,
    "failedRequests": 34,
    "avgElapsedUs": 45000,
    "p50ElapsedUs": 38000,
    "p95ElapsedUs": 89000,
    "p99ElapsedUs": 125000,
    "crashCount": 0,
    "timeoutCount": 2
  },
  "system": {
    "os": "Windows 11 23H2",
    "cpu": "Intel Core i7-12700K",
    "gpu": "NVIDIA RTX 4070",
    "engineVersion": "5.4.0"
  },
  "timestamp": "2026-01-06T14:30:00Z"
}
```

### Submitted To

`POST https://api.marketplace.explorerlens.dev/v1/telemetry`

### Dashboard

Plugin developers can view:

- Install count
- Active users
- Performance metrics (avg time, failure rate)
- Crash reports
- Reviews and ratings
- Revenue (if paid plugin)

---

## Monetization (Future)

### Free Plugins

- No cost to users
- Vendor can accept donations
- Marketplace shows "Donate" button

### Paid Plugins

- One-time purchase ($1 - $99)
- Revenue split: 70% vendor, 30% marketplace
- Free trial available (14-30 days)
- License key validation

### Subscription Plugins

- Monthly/annual subscription
- Auto-renewal
- Pro features unlocked
- Enterprise licensing available

---

## Rate Limiting

API rate limits (per IP):

- Anonymous: 60 requests/hour
- Authenticated: 5000 requests/hour
- Enterprise: Unlimited

Headers:

```
X-RateLimit-Limit: 5000
X-RateLimit-Remaining: 4999
X-RateLimit-Reset: 1704556800
```

---

## CDN Distribution

Plugin packages hosted on global CDN:

- CloudFlare or Azure CDN
- HTTPS only
- Geo-distributed (< 100ms latency)
- Automatic caching
- DDoS protection

---

## Future Enhancements

- GraphQL API
- WebSocket for real-time updates
- Plugin collections/bundles
- Plugin recommendations (ML-based)
- Plugin dependencies graph viewer
- Automated security scanning (continuous)
- Plugin usage heatmaps
- A/B testing framework for plugins

---

## Summary

The Plugin Marketplace provides:
✅ Centralized plugin discovery  
✅ Trusted, verified plugins  
✅ Automatic updates  
✅ Revocation mechanism  
✅ Performance telemetry  
✅ Secure distribution  
✅ Developer analytics

**Status:** Specification complete, ready for implementation in Sprint 12.

