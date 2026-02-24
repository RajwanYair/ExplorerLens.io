# Sample Plugin - Reference Implementation

This is a reference plugin implementation for the ExplorerLens Plugin SDK. It demonstrates how to create a plugin that decodes a custom image format (.xyz).

## Overview

The Sample Plugin shows:

- ✅ Basic plugin structure and exports
- ✅ Initialization and shutdown lifecycle
- ✅ Thumbnail generation from custom format
- ✅ Format detection via magic bytes
- ✅ Statistics tracking
- ✅ Error handling
- ✅ Manifest.json configuration

## Building

### Prerequisites

- Visual Studio 18 2026 or later
- Windows SDK 10.0.26100.0 or later
- ExplorerLens SDK headers

### Steps

```bash
# Clone or copy SDK
cd SDK/examples/SamplePlugin

# Build with MSBuild
msbuild SamplePlugin.vcxproj /p:Configuration=Release /p:Platform=x64

# Or open in Visual Studio and build
```

## File Structure

```
SamplePlugin/
├── SamplePlugin.cpp        # Main plugin implementation
├── SamplePlugin.vcxproj    # Visual Studio project
├── manifest.json           # Plugin manifest
├── assets/
│   └── icons/
│       ├── icon-16.png
│       ├── icon-32.png
│       └── icon-256.png
├── docs/
│   ├── README.md           # This file
│   └── CHANGELOG.md
└── licenses/
    └── LICENSE.txt
```

## Packaging

```bash
# Create plugin package
zip -r SamplePlugin.dtplugin manifest.json plugin.dll assets/ licenses/ docs/

# Or use PowerShell
Compress-Archive -Path manifest.json,plugin.dll,assets,licenses,docs -DestinationPath SamplePlugin.dtplugin
```

## Installation

1. Open ExplorerLens Manager
2. Go to Plugins → Install Plugin
3. Select `SamplePlugin.dtplugin`
4. Review capabilities and click Install
5. Plugin will appear in installed plugins list

## XYZ Format Specification

The Sample Plugin decodes a fictitious ".xyz" format with this structure:

```
Offset | Size | Description
-------|------|------------
0x00   | 4    | Magic bytes: "XYZ1"
0x04   | 4    | Width (uint32_t, little endian)
0x08   | 4    | Height (uint32_t, little endian)
0x0C   | N    | Pixel data (width * height * 4 bytes, BGRA format)
```

Example:

```
58 59 5A 31           # Magic "XYZ1"
00 02 00 00           # Width: 512
00 02 00 00           # Height: 512
[pixel data...]       # 512 * 512 * 4 = 1,048,576 bytes
```

## Creating Your Own Plugin

### 1. Copy Template

```bash
cp -r SDK/examples/SamplePlugin SDK/examples/MyPlugin
```

### 2. Update Plugin Info

Edit `SamplePlugin.cpp`:

```cpp
static const DT_PluginInfo g_pluginInfo = {
    DT_PLUGIN_ABI_VERSION,
    L"explorerlens.plugin.myplugin",      // Change ID
    L"My Custom Plugin",                 // Change name
    L"1.0.0",                           
    L"Your Name",                        // Change vendor
    L"Decodes XYZ format",
    DT_CAP_READ_FILE | DT_CAP_DECODE,
    DT_ENGINE_VERSION_5_4_0,
    0
};
```

### 3. Update Supported Formats

```cpp
static const DT_FormatInfo g_formats[] = {
    {
        L".myformat",                    // Your extension
        L"image/x-myformat",             // Your MIME type
        L"My Custom Format",
        50
    }
};
```

### 4. Implement Decoder

Replace `DecodeXYZFile()` with your format decoder:

```cpp
static HBITMAP DecodeMyFormat(const wchar_t* filePath, uint32_t sizePx) {
    // 1. Read file
    // 2. Parse format
    // 3. Decode to BGRA pixels
    // 4. Create HBITMAP
    // 5. Resize to thumbnail
    return hBitmap;
}
```

### 5. Update Manifest

Edit `manifest.json`:

```json
{
  "plugin": {
    "id": "explorerlens.plugin.myplugin",
    "name": "My Custom Plugin",
    ...
  },
  "formats": [
    {
      "extension": ".myformat",
      ...
    }
  ]
}
```

### 6. Build and Package

```bash
msbuild MyPlugin.vcxproj /p:Configuration=Release /p:Platform=x64
zip -r MyPlugin.dtplugin manifest.json x64/Release/plugin.dll assets/ licenses/ docs/
```

## Best Practices

### Error Handling

Always validate input:

```cpp
if (!request || request->structSize != sizeof(DT_ThumbnailRequest)) {
    return DT_ERROR_INVALID_ARGUMENT;
}
```

### Timeout Handling

Check timeout periodically:

```cpp
auto start = std::chrono::steady_clock::now();
while (decoding) {
    auto elapsed = std::chrono::steady_clock::now() - start;
    if (elapsed > std::chrono::milliseconds(request->timeoutMs)) {
        return DT_ERROR_TIMEOUT;
    }
}
```

### Memory Management

Limit memory usage:

```cpp
if (width * height > 100'000'000) { // 100 megapixels
    return DT_ERROR_OUT_OF_MEMORY;
}
```

### Statistics

Track performance:

```cpp
g_stats.totalRequests++;
auto start = std::chrono::high_resolution_clock::now();
// ... do work ...
auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
    std::chrono::high_resolution_clock::now() - start
).count();
g_stats.totalElapsedUs += elapsed;
g_stats.successfulRequests++;
```

## Testing

### Unit Tests

```cpp
// Test initialization
DT_Status status = DT_Initialize();
assert(status == DT_SUCCESS);

// Test thumbnail generation
DT_ThumbnailRequest request = {sizeof(DT_ThumbnailRequest)};
request.filePath = L"test.xyz";
request.sizePx = 256;

DT_ThumbnailResult result = {sizeof(DT_ThumbnailResult)};
status = DT_GenerateThumbnail(&request, &result);
assert(status == DT_SUCCESS);
assert(result.hBitmap != nullptr);

// Cleanup
DeleteObject(result.hBitmap);
DT_Shutdown();
```

### Compatibility Test Kit

```bash
# Run official test kit
.\PluginTestKit.exe --plugin MyPlugin.dtplugin --output results.json

# View results
cat results.json
```

## Troubleshooting

### Plugin Won't Load

- Check ABI version matches engine
- Verify all required exports present
- Check DLL dependencies (use `dumpbin /dependents plugin.dll`)
- Verify manifest.json is valid JSON

### Thumbnails Not Generated

- Check file format detection logic
- Verify decoder handles corrupt files
- Add logging to `DT_GenerateThumbnail()`
- Test with sample files

### Performance Issues

- Profile with Visual Studio Performance Profiler
- Check memory allocations
- Use optimized image libraries
- Consider GPU acceleration for large images

## Resources

- [ExplorerLens Plugin SDK Documentation](../../docs/PLUGIN_SDK.md)
- [Plugin Package Format](../../docs/PLUGIN_PACKAGE_FORMAT_V1.md)
- [Marketplace Submission Guide](../../docs/PLUGIN_MARKETPLACE_PROTOCOL_V1.md)
- [Plugin Sandbox Model](../../docs/PLUGIN_SANDBOX_MODEL_V1.md)

## License

MIT License - see LICENSE.txt

## Support

- GitHub Issues: <https://github.com/explorerlens/plugin-samples/issues>
- Documentation: <https://docs.explorerlens.dev>
- Discord: <https://discord.gg/explorerlens>

