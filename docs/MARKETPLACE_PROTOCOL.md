# Marketplace Registry Protocol v1.0 (Sprint 12)

**Date:** January 6, 2026
**Status:** Design

## 1. Overview

The DarkThumbs Marketplace is a simple, static-file-based registry that allows the client to discover, verify, and update plugins without a complex backend. It supports both a central public registry and private enterprise feeds.

## 2. Registry Structure

The registry is hosted as a static HTTP(S) site or a network share.

### 2.1 Index File (`index.v1.json`)

The root entry point.

```json
{
  "protocol": 1,
  "timestamp": "2026-01-06T12:00:00Z",
  "plugins": {
    "com.adobe.psd": {
      "latest": "2.4.1",
      "versions": {
        "2.4.1": "plugins/com.adobe.psd/2.4.1/manifest.json",
        "2.4.0": "plugins/com.adobe.psd/2.4.0/manifest.json"
      }
    },
    "org.gimp.xcf": {
      "latest": "1.0.5",
      "versions": {
        "1.0.5": "plugins/org.gimp.xcf/1.0.5/manifest.json"
      }
    }
  }
}
```

## 3. Package Distribution

Plugins are distributed as `.dtplugin` files (essentially ZIPs with a specific layout).

### URL Pattern

`https://plugins.darkthumbs.io/download/{id}/{version}/{arch}/{filename}`

Example: `https://plugins.darkthumbs.io/download/com.adobe.psd/2.4.1/x64/psd_plugin.dtplugin`

## 4. Verification & Trust

### 4.1 Channels

- **Stable:** Fully tested, signed by Verified Publisher.
- **Beta:** Functionally tested, signed by Publisher.
- **Nightly:** Automated builds, signed by CI Key.

### 4.2 Signature Validation

Every `.dtplugin` container MUST contain a `signature.p7s` (CMS/PKCS#7) file detached signature of the content.

- The Engine validates the signature against the **DarkThumbs Root CA** (embedded in the DLL) or a custom **Enterprise Root CA** (via Group Policy).

## 5. Compatibility "Test Kit"

To appear in the Public Registry, a submitter must maintain a "Green" status on the Compatibility Test Kit (CTK).

1. Submitter uploads `plugin + test_images[]`.
2. CI runs `DarkThumbs.Validator.exe` against the images.
3. Checks: Crashes, memory leaks, timeouts.
4. If Pass -> Publish.

## 6. Enterprise Private Feeds

Enterprises can set a registry override in HKLM:
`HKLM\Software\DarkThumbs\UpdateSource = "https://internal.corp/thumbs/index.json"`

This allows companies to distribute internal format decoders securely.
