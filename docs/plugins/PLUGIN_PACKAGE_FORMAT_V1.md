# DarkThumbs Plugin Package Format (.dtplugin)

**Version:** 1.0  
**Status:** Draft  
**Created:** January 6, 2026

---

## Overview

A `.dtplugin` file is a ZIP archive containing a thumbnail decoder plugin with its manifest, binaries, assets, and signatures. This format ensures plugins are discoverable, verifiable, and safely installable.

---

## Package Structure

```
MyPlugin.dtplugin (ZIP archive)
├── manifest.json              # Required: Plugin metadata and requirements
├── plugin.dll                 # Required: Main plugin binary (x64)
├── plugin_x86.dll             # Optional: x86 binary for 32-bit support
├── plugin_arm64.dll           # Optional: ARM64 binary
├── assets/                    # Optional: Plugin-specific assets
│   ├── icons/
│   │   ├── icon-16.png
│   │   ├── icon-32.png
│   │   └── icon-256.png
│   ├── shaders/               # For GPU-accelerated plugins
│   │   └── decode.hlsl
│   └── samples/               # Sample files for format detection testing
│       └── sample.psd
├── licenses/                  # Optional: License files
│   ├── LICENSE.txt
│   └── THIRD_PARTY_NOTICES.txt
├── docs/                      # Optional: Documentation
│   ├── README.md
│   └── CHANGELOG.md
└── signatures/                # Optional: Code signatures and hashes
    ├── plugin.dll.sig
    └── hashes.json
```

---

## Manifest Schema (manifest.json)

```json
{
  "$schema": "https://darkthumbs.dev/schemas/plugin-manifest-v1.json",
  "manifestVersion": 1,
  "plugin": {
    "id": "darkthumbs.plugin.psd",
    "name": "Photoshop Document Decoder",
    "version": "1.2.3",
    "vendor": "Adobe Systems Inc.",
    "description": "Native PSD thumbnail decoder with layer support",
    "homepage": "https://github.com/darkthumbs/plugin-psd",
    "icon": "assets/icons/icon-256.png"
  },
  "abiVersion": 1,
  "engineVersion": {
    "min": "5.4.0",
    "max": "6.9.99"
  },
  "capabilities": [
    "read_file",
    "decode",
    "metadata"
  ],
  "formats": [
    {
      "extension": ".psd",
      "mimeType": "image/vnd.adobe.photoshop",
      "description": "Adobe Photoshop Document",
      "priority": 100,
      "magicBytes": "38425053"
    },
    {
      "extension": ".psb",
      "mimeType": "image/vnd.adobe.photoshop",
      "description": "Adobe Photoshop Large Document",
      "priority": 100,
      "magicBytes": "38425053"
    }
  ],
  "binaries": {
    "x64": {
      "path": "plugin.dll",
      "sha256": "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
      "size": 2458624
    },
    "x86": {
      "path": "plugin_x86.dll",
      "sha256": "...",
      "size": 1843200
    }
  },
  "dependencies": {
    "runtime": [
      "vcredist.x64.14.0"
    ],
    "plugins": []
  },
  "configuration": {
    "schemaUri": "https://darkthumbs.dev/schemas/plugin-config/psd-v1.json",
    "defaults": {
      "maxLayers": 10,
      "includeHiddenLayers": false,
      "renderQuality": "high"
    }
  },
  "security": {
    "signatureType": "authenticode",
    "certificate": {
      "thumbprint": "A1B2C3D4E5F6...",
      "subject": "CN=Adobe Systems Inc., O=Adobe Systems Inc., L=San Jose, S=California, C=US",
      "issuer": "CN=DigiCert SHA2 Code Signing CA, O=DigiCert Inc, C=US",
      "validFrom": "2025-01-01T00:00:00Z",
      "validTo": "2028-01-01T00:00:00Z"
    },
    "permissions": {
      "allowNetworkAccess": false,
      "allowFileSystemWrite": false,
      "allowRegistryAccess": false
    }
  },
  "metadata": {
    "author": "John Doe <john@adobe.com>",
    "license": "MIT",
    "tags": ["photoshop", "psd", "layers", "adobe"],
    "category": "Image Decoders",
    "releaseDate": "2026-01-06",
    "changelog": "docs/CHANGELOG.md"
  },
  "testing": {
    "compatibilityTestPassed": true,
    "testKitVersion": "1.0.0",
    "testResults": "https://darkthumbs.dev/plugin-tests/psd-1.2.3.html"
  }
}
```

---

## Manifest Field Definitions

### Required Fields

| Field | Type | Description |
|-------|------|-------------|
| `manifestVersion` | integer | Manifest schema version (currently 1) |
| `plugin.id` | string | Unique stable identifier (reverse domain notation) |
| `plugin.name` | string | Human-readable plugin name |
| `plugin.version` | string | Semantic version (major.minor.patch) |
| `plugin.vendor` | string | Plugin author/organization |
| `abiVersion` | integer | Plugin ABI version (must match engine) |
| `engineVersion.min` | string | Minimum compatible engine version |
| `capabilities` | string[] | Required capabilities (see Capabilities) |
| `formats` | object[] | Supported file formats |
| `binaries.x64.path` | string | Path to x64 DLL within package |
| `binaries.x64.sha256` | string | SHA-256 hash of x64 DLL |

### Optional Fields

| Field | Type | Description |
|-------|------|-------------|
| `plugin.description` | string | Brief plugin description |
| `plugin.homepage` | string | Plugin homepage URL |
| `plugin.icon` | string | Path to icon within package |
| `engineVersion.max` | string | Maximum compatible engine version |
| `binaries.x86` | object | x86 binary info (for 32-bit support) |
| `binaries.arm64` | object | ARM64 binary info |
| `dependencies.runtime` | string[] | Required runtime dependencies |
| `dependencies.plugins` | string[] | Required plugin dependencies |
| `configuration` | object | Plugin-specific configuration schema |
| `security` | object | Security and signing information |
| `metadata` | object | Additional metadata (author, license, tags) |
| `testing` | object | Compatibility test results |

---

## Capabilities

Plugins must declare all capabilities they require. The engine enforces these at runtime.

| Capability | Description |
|------------|-------------|
| `read_file` | Can read local files directly via file path |
| `network` | Can make network requests (HTTP/HTTPS) |
| `gpu` | Can use GPU acceleration (D3D11/D3D12) |
| `decode` | Primary decoder capability (required for decoders) |
| `transform` | Can transform/filter images |
| `metadata` | Can extract metadata from files |
| `archive` | Can extract from archives (ZIP, RAR, etc.) |
| `streaming` | Supports streaming/progressive decode |
| `multithreaded` | Uses multiple threads internally |
| `external_process` | Spawns external processes |

---

## Format Definitions

Each format entry describes a file format the plugin can handle.

```json
{
  "extension": ".psd",
  "mimeType": "image/vnd.adobe.photoshop",
  "description": "Adobe Photoshop Document",
  "priority": 100,
  "magicBytes": "38425053"
}
```

| Field | Type | Description |
|-------|------|-------------|
| `extension` | string | File extension (with dot) |
| `mimeType` | string | MIME type |
| `description` | string | Human-readable format name |
| `priority` | integer | Priority when multiple plugins support format (0-1000, higher = preferred) |
| `magicBytes` | string | Optional: Magic bytes (hex) for format detection |

---

## Binary Hash Format (signatures/hashes.json)

For integrity verification:

```json
{
  "version": 1,
  "timestamp": "2026-01-06T14:30:00Z",
  "files": {
    "plugin.dll": {
      "sha256": "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
      "sha512": "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce...",
      "size": 2458624
    },
    "manifest.json": {
      "sha256": "...",
      "size": 4096
    }
  }
}
```

---

## Signing Requirements

### Development Plugins

- Self-signed certificate acceptable
- Warning shown during installation

### Marketplace Plugins

- Must be signed with EV (Extended Validation) code signing certificate
- Certificate must be from trusted CA (DigiCert, GlobalSign, etc.)
- Plugin binary (DLL) must be Authenticode signed
- Manifest must include certificate thumbprint

### Verified Plugins

Additional requirements:

- Compatibility Test Kit passed with 100% success
- Source code review completed
- Security audit passed
- Vendor verification completed
- Active support commitment (email/forum/GitHub issues)

---

## Installation Process

1. **Package Validation**
   - Verify ZIP integrity
   - Parse manifest.json
   - Validate manifest schema
   - Check ABI version compatibility

2. **Binary Verification**
   - Verify SHA-256 hashes match
   - Check code signature (if present)
   - Verify certificate chain
   - Check revocation status

3. **Capability Check**
   - Review requested capabilities
   - Prompt user for approval (if necessary)
   - Apply enterprise policy restrictions

4. **Compatibility Check**
   - Verify engine version compatibility
   - Check runtime dependencies
   - Verify plugin dependencies

5. **Installation**
   - Extract to plugin directory: `%LocalAppData%\DarkThumbs\Plugins\{plugin-id}`
   - Register plugin in plugin database
   - Update format handler registry

6. **Verification**
   - Load plugin DLL
   - Call DT_GetPluginInfo()
   - Verify info matches manifest
   - Call DT_Initialize()
   - Add to active plugins list

---

## Package Distribution Channels

### 1. Official Marketplace

- <https://marketplace.darkthumbs.dev>
- Curated, verified plugins
- Automatic updates
- User reviews and ratings

### 2. Direct Download

- .dtplugin file from vendor website
- Manual installation via Manager app
- User assumes responsibility for verification

### 3. Enterprise Distribution

- Internal plugin repository
- Group Policy deployment
- Centralized management

---

## Plugin Update Mechanism

### Update Channels

- **Stable**: Production-ready releases
- **Beta**: Pre-release testing
- **Nightly**: Latest development builds

### Update Manifest (update.json)

Hosted by plugin vendor:

```json
{
  "plugin": {
    "id": "darkthumbs.plugin.psd",
    "name": "Photoshop Document Decoder"
  },
  "channels": {
    "stable": {
      "version": "1.2.3",
      "releaseDate": "2026-01-06",
      "downloadUrl": "https://cdn.example.com/psd-1.2.3.dtplugin",
      "sha256": "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
      "size": 2458624,
      "changelog": "Fixed layer rendering bug, improved performance",
      "breaking": false,
      "minEngineVersion": "6.0.0"
    },
    "beta": {
      "version": "1.3.0-beta.1",
      "releaseDate": "2026-01-05",
      "downloadUrl": "https://cdn.example.com/psd-1.3.0-beta.1.dtplugin",
      "sha256": "...",
      "size": 2459136,
      "changelog": "Added Smart Object support",
      "breaking": false
    }
  },
  "deprecated": false,
  "replacedBy": null
}
```

---

## Plugin Revocation

If a plugin is found to be malicious or severely buggy:

### Revocation Manifest

```json
{
  "revocations": [
    {
      "pluginId": "darkthumbs.plugin.psd",
      "version": "1.0.5",
      "reason": "Critical security vulnerability (CVE-2026-1234)",
      "date": "2026-01-06",
      "severity": "critical",
      "action": "block"
    }
  ]
}
```

Hosted at: <https://revocations.darkthumbs.dev/revocations.json>

Actions:

- `warn`: Show warning, allow user to continue
- `block`: Disable plugin, prevent loading
- `uninstall`: Automatically uninstall plugin

---

## Compatibility Test Kit

Plugins seeking "Verified" status must pass the Compatibility Test Kit.

### Test Categories

1. **ABI Compliance**
   - All required exports present
   - Correct function signatures
   - Structure size validation

2. **Format Handling**
   - Correctly identifies supported formats
   - Handles corrupt/truncated files gracefully
   - Respects timeout limits

3. **Performance**
   - Thumbnail generation < 500ms (typical case)
   - Memory usage < 100MB per request
   - No memory leaks

4. **Stability**
   - No crashes during 10,000 thumbnail requests
   - Proper error handling and reporting
   - Clean shutdown (no resource leaks)

5. **Security**
   - No buffer overflows
   - Safe handling of untrusted input
   - Respects capability restrictions

---

## Example: Minimal Plugin Package

```
minimal.dtplugin
├── manifest.json
└── plugin.dll
```

**manifest.json:**

```json
{
  "manifestVersion": 1,
  "plugin": {
    "id": "darkthumbs.plugin.minimal",
    "name": "Minimal Example Plugin",
    "version": "1.0.0",
    "vendor": "Example Corp"
  },
  "abiVersion": 1,
  "engineVersion": {
    "min": "5.4.0"
  },
  "capabilities": ["decode"],
  "formats": [
    {
      "extension": ".xyz",
      "mimeType": "image/x-xyz",
      "description": "XYZ Format",
      "priority": 50
    }
  ],
  "binaries": {
    "x64": {
      "path": "plugin.dll",
      "sha256": "...",
      "size": 65536
    }
  }
}
```

---

## Best Practices

### For Plugin Developers

1. Keep plugins small and focused (single format family)
2. Use semantic versioning rigorously
3. Include comprehensive error handling
4. Respect timeout limits
5. Test with corrupt/malformed files
6. Document configuration options
7. Provide sample files for testing

### For Plugin Users

1. Only install plugins from trusted sources
2. Review requested capabilities before installation
3. Keep plugins updated
4. Report issues to plugin vendor
5. Use "Verified" plugins when available

---

## Future Enhancements

- Multi-plugin packages (plugin suites)
- Plugin dependencies (shared libraries)
- Plugin sandboxing (AppContainer)
- WebAssembly plugin support
- Plugin marketplace API
- Automatic compatibility testing service
