# DarkThumbs Plugin Developer Guide (v6.0)

**Welcome to the DarkThumbs Ecosystem!** This guide will help you create high-performance thumbnail plugins for the v6.0 platform.

## 1. Do I need a plugin?

Check the **Sprint 15 Format Strategy**.

- **Core Formats (Tier 1):** Built-in. If you want to add a major standard (e.g. JPEG-XR), contribute to Core.
- **Plugins (Tier 2):** Best for proprietary formats, niche industry standards (DICOM, various RAW types), or formats with conflicting licenses.

## 2. The SDK

The SDK is header-only + a small definition file.
**Include:** `src/SDK/include/DarkThumbsPlugin.h`

### 2.1 Essential Exports

Your DLL must export these C-ABI functions:

```cpp
DT_PLUGIN_API const DT_PluginInfo* DT_GetPluginInfo();
DT_PLUGIN_API HRESULT DT_Initialize();
DT_PLUGIN_API HRESULT DT_Shutdown();
DT_PLUGIN_API HRESULT DT_GenerateThumbnail(...);
```

### 2.2 The Manifest (`manifest.json`)

Every plugin ships as a `.dtplugin` package (Zip). The root must contain `manifest.json`:

```json
{
  "id": "com.mycompany.myformat",
  "version": "1.0.0",
  "capabilities": ["read_file", "decode"],
  "supportedExtensions": [".myfmt"]
}
```

## 3. Best Practices

### 3.1 Memory Management

- **Do not leak.** The worker process is long-lived.
- **Respect `sizePx`.** Do not decode a 100MP image at full res if requested size is 256px. Use lower strictness/subsampling capabilities of your decoder.

### 3.2 Security

- **Validate headers first.** Check magic numbers before allocating.
- **Handle fuzzing.** Your plugin will be tested against corrupted files. Return `E_FAIL` gracefully; ensure no crashes.

### 3.3 Cancellation

- The engine may call `TerminateThread` in extreme cases (timeout), but you should check for cancellation signals if your decode takes > 100ms. *[Note: Cancellation API coming in v6.1]*

## 4. Packaging and Signing

1. Build your DLL (`MyPlugin.dll`).
2. Create `manifest.json`.
3. Zip them up: `MyPlugin.zip` -> `MyPlugin.dtplugin`.
4. (Optional) Sign the package using `DarkThumbs.CLI.exe sign`.

## 5. Publishing

See `docs/MARKETPLACE_PROTOCOL.md` for instructions on submitting to the public registry or hosting a private enterprise feed.
