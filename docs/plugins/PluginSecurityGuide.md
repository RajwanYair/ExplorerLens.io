# Plugin Security Guide
## ExplorerLens v5.3.0 - Sprint 14 Integration

---

## Overview

ExplorerLens implements a comprehensive **dual-mode plugin security architecture** that provides flexibility and protection. Plugins can execute in  two modes:

1. **In-Worker Mode**: Direct execution within the thumbnail worker process (trusted plugins)
2. **PluginHost Mode**: Isolated execution in a separate sandboxed process (untrusted plugins)

This guide covers the security infrastructure, how it works, and best practices for plugin developers and system administrators.

---

## Architecture Components

### 1. IsolationModeSelector

**Purpose**: Determines the appropriate execution mode for each plugin based on multiple factors.

**Decision Factors**:
- **Digital Signature**: Authenticode signature verification using Windows Trust API
- **Trust List**: Administrator-maintained list of trusted plugins
- **User Preferences**: Per-plugin user choices (allow/block In-Worker mode)
- **System Policy**: Minimum isolation mode enforced system-wide
- **Crash History**: Automatically demote plugins with crash history

**Configuration**:
```cpp
// Check current mode for a plugin
IsolationMode mode = IsolationModeSelector::Instance().DetermineMode(
    L"MyPlugin", 
    L"C:\\Plugins\\MyPlugin.dll");

// Add to trusted list (requires admin)
IsolationModeSelector::Instance().AddTrustedPlugin(L"MyPlugin");

// Set minimum isolation mode
IsolationModeSelector::Instance().SetMinimumIsolationMode(IsolationMode::PluginHost);
```

---

### 2. PluginDecoder

**Purpose**: Implements the `IThumbnailDecoder` interface as an adapter that wraps plugin instances, providing transparent dual-mode execution.

**Key Features**:
- Automatic mode selection via `PluginDecoderFactory`
- Seamless switching between In-Worker and PluginHost modes
- Statistics tracking (decode counts, error rates, timing)
- Error translation from plugin codes to HRESULT

**Usage**:
```cpp
// Create decoder with automatic mode selection
auto decoder = PluginDecoderFactory::CreateDecoder(
    plugin_handle,    // nullptr if using PluginHost
    L"ImagePlugin",
    L"C:\\Plugins\\ImagePlugin.dll");

// Decode thumbnail (mode is transparent)
HBITMAP hBitmap = nullptr;
HRESULT hr = decoder->Decode(L"image.raw", 256, 256, &hBitmap);

if (SUCCEEDED(hr)) {
    // Use thumbnail...
}

// Check statistics
auto stats = decoder->GetStatistics();
```

---

### 3. PluginHostClient

**Purpose**: Manages communication with the isolated PluginHost.exe process via IPC.

**Features**:
- **Named Pipe Communication**: Request/response protocol for control messages
- **Shared Memory**: High-performance data transfer for large images (>1MB)
- **Timeout Management**: Configurable request timeouts with graceful handling
- **Process Lifecycle**: Automatic start/stop with health monitoring

**IPC Protocol**:
```
Client                      PluginHost
  |                              |
  |------- StartupHandshake ---->|
  |<----- HandshakeResponse -----|
  |                              |
  |------- DecodeRequest ------->|
  |         (via named pipe)     |
  |                              | [Loads plugin]
  |                              | [Decodes image]
  |                              | [Writes to shared memory]
  |<----- DecodeResponse --------|
  |       (metadata + handle)    |
  |                              |
  |------- Shutdown ------------>|
  |                              | [Exit]
```

**Configuration**:
```cpp
PluginHostClient client(L"C:\\Plugins\\UntrustedPlugin.dll");

// Set timeout for long-running operations
client.SetRequestTimeout(30000); // 30 seconds

// Start PluginHost process
HRESULT hr = client.Start();

// Decode via IPC
DecodeResult result = {};
hr = client.DecodeImage(L"test.png", 256, 256, &result);

// Stop when done
client.Stop();
```

---

### 4. CrashHandler

**Purpose**: Detects plugin crashes, maintains crash history, and automatically disables problematic plugins.

**Features**:
- **Crash Detection**: Monitors exit codes (access violations, crashes, hangs)
- **Automatic Disabling**: Disables plugins after repeated crashes (threshold: 3)
- **Re-enable Mechanism**: Time-based or manual re-enable after cooldown
- **Crash Reporting**: Detailed crash logs with stack traces (when available)

**Usage**:
```cpp
CrashHandler& handler = CrashHandler::Instance();

// Check if plugin is disabled
if (handler.IsPluginDisabled(L"ProblematicPlugin")) {
    // Skip or show warning
    return E_FAIL;
}

// Get crash count
uint32_t crashes = handler.GetCrashCount(L"ProblematicPlugin");
if (crashes > 0) {
    LogWarning(L"Plugin has %u previous crashes", crashes);
}

// Manually re-enable (requires admin)
handler.EnablePlugin(L"ProblematicPlugin");
```

---

### 5. JobObjectManager

**Purpose**: Enforces resource limits on PluginHost processes to prevent system resource exhaustion.

**Limits Enforced**:
- **Memory**: Max 512 MB working set per plugin
- **CPU Time**: Max 60 seconds CPU time per decode operation
- **Process Count**: Max 1 child process (prevents fork bombs)
- **Token Restrictions**: Removes admin/debug privileges from process

**Configuration**:
```cpp
JobObjectManager job(L"ExplorerLens_Plugin_MyPlugin");

// Set resource limits
job.SetMemoryLimit(512 * 1024 * 1024);  // 512 MB
job.SetCPUTimeLimit(60);                 // 60 seconds
job.SetProcessCountLimit(1);             // No child processes

// Apply restrictions
job.ApplyTokenRestrictions();

// Assign process to job
job.AssignProcess(hPluginHostProcess);
```

---

### 6. SharedMemoryManager

**Purpose**: Provides high-performance shared memory for transferring large image data (>1MB) between processes.

**Features**:
- **Large Transfer Optimization**: Avoids pipe bottlenecks for multi-MB images
- **Security**: Uses named sections with restricted ACLs
- **Automatic Cleanup**: Releases resources when done

**Usage**:
```cpp
// Create shared memory section
SharedMemorySection section(L"Decode_12345", 4 * 1024 * 1024);

// Write image data
section.Write(pixel_data, data_size, 0);

// Read from another process
uint8_t* buffer = new uint8_t[data_size];
section.Read(buffer, data_size, 0);
```

---

## Security Benefits

### In-Worker Mode
**Pros**:
- ✅ **Fast**: Direct function calls, no IPC overhead (2-5ms latency)
- ✅ **Low Memory**: Plugin loaded once, shared with worker process

**Cons**:
- ⚠️ **No Crash Isolation**: Plugin crash brings down worker process
- ⚠️ **Security Risk**: Malicious plugin has full process access

**Recommended For**: Trusted, signed plugins from known developers.

---

### PluginHost Mode
**Pros**:
- ✅ **Crash Isolation**: Plugin crash doesn't affect worker process
- ✅ **Resource Limits**: Memory, CPU time, and process limits enforced
- ✅ **Sandboxing**: Reduced privileges, no admin/debug rights
- ✅ **Security**: Malicious code contained in separate process

**Cons**:
- ⚠️ **Slower**: IPC overhead adds 8-15ms per decode
- ⚠️ **Higher Memory**: Separate process adds ~10MB overhead

**Recommended For**: Untrusted, unsigned, or third-party plugins.

---

## Performance Impact

| Metric                  | In-Worker  | PluginHost | Difference  |
|-------------------------|------------|------------|-------------|
| **Decode Latency**      | 2-5ms      | 8-15ms     | 3-10ms      |
| **Throughput**          | 200-500/s  | 60-120/s   | ~3x slower  |
| **Memory Overhead**     | ~1MB       | ~10MB      | +9MB        |
| **Process Creation**    | None       | 40-80ms    | First only  |

**Recommendation**: Use PluginHost for untrusted plugins despite performance cost. Security benefit justifies the overhead.

---

## Configuration

### Registry Settings

All configuration is stored under:
```
HKEY_CURRENT_USER\Software\ExplorerLens\PluginSecurity
```

**Keys**:
- `TrustedPlugins` (REG_MULTI_SZ): List of trusted plugin names
- `MinimumIsolationMode` (REG_DWORD): 0=InWorker, 1=PluginHost
- `UserPreferences\<PluginName>` (REG_DWORD): 0=Block In-Worker, 1=Allow In-Worker

### Programmatic Configuration

```cpp
// Get instance
auto& selector = IsolationModeSelector::Instance();

// Load configuration from registry
selector.LoadConfiguration();

// Add trusted plugin
selector.AddTrustedPlugin(L"MyTrustedPlugin");

// Set user preference
selector.SetUserPreference(L"MyPlugin", false); // Block In-Worker

// Set minimum mode
selector.SetMinimumIsolationMode(IsolationMode::PluginHost);

// Save configuration
selector.SaveConfiguration();
```

---

## For Plugin Developers

### Best Practices

1. **Sign Your Plugin**: Use Authenticode to sign your DLL for trust verification
2. **Handle Errors Gracefully**: Return proper error codes, don't crash
3. **Respect Resource Limits**: Stay under 512MB memory, complete in <60 seconds
4. **Test Both Modes**: Verify your plugin works in In-Worker and PluginHost modes
5. **Avoid Dependencies**: Minimize external DLL dependencies

### Error Codes

Return these codes from your plugin's decode function:

| Code | Meaning |
|------|---------|
| `0` | Success |
| `1` | File not found |
| `2` | Unsupported format |
| `3` | Decode error |
| `4` | Out of memory |
| `5` | Timeout |

These are automatically translated to HRESULT by `PluginDecoder`.

### Example Plugin

```cpp
// MyPlugin.cpp
extern "C" __declspec(dllexport)
int DecodeImage(const wchar_t* path, int width, int height, PluginResult* result) {
    // Load image
    if (!FileExists(path)) return 1;
    
    // Decode
    uint8_t* pixels = LoadAndDecode(path, width, height);
    if (!pixels) return 3;
    
    // Fill result
    result->width = width;
    result->height = height;
    result->format = PixelFormat_RGBA;
    result->stride = width * 4;
    result->data = pixels;
    result->data_size = width * height * 4;
    
    return 0; // Success
}
```

---

## For System Administrators

### Enterprise Policies

#### 1. Force PluginHost Mode for All Plugins
```cpp
IsolationModeSelector::Instance().SetMinimumIsolationMode(IsolationMode::PluginHost);
```

Or via registry:
```
HKEY_LOCAL_MACHINE\Software\ExplorerLens\PluginSecurity
MinimumIsolationMode = 1
```

#### 2. Whitelist Trusted Plugins
```
HKEY_LOCAL_MACHINE\Software\ExplorerLens\PluginSecurity
TrustedPlugins = REG_MULTI_SZ: 
    "OfficialImagePlugin"
    "SignedVideoPlugin"
```

#### 3. Block Unsigned Plugins
Set policy to require signatures:
```cpp
IsolationModeSelector::Instance().SetRequireSignature(true);
```

---

## FAQ

**Q: Why is my plugin slower in PluginHost mode?**  
A: IPC overhead adds 8-15ms per decode. This is acceptable for security. Consider optimizing your plugin for batch operations.

**Q: Can I force In-Worker mode for my plugin?**  
A: No, the user/admin must trust your plugin. Sign it with Authenticode to improve trustworthiness.

**Q: What happens if a plugin crashes?**  
A: In PluginHost mode, only the PluginHost process crashes. The worker continues. After 3 crashes, the plugin is automatically disabled.

**Q: How do I re-enable a disabled plugin?**  
A: Use `CrashHandler::Instance().EnablePlugin(L"PluginName")` or wait 24 hours for automatic re-enable.

**Q: Can I use PluginHost mode for local testing?**  
A: Yes, set `EXPLORERLENS_FORCE_PLUGINHOST=1` in environment variables.

---

## Testing

See [SecurityTests.cpp](../Tests/SecurityTests.cpp) for comprehensive security test suite covering:
- Isolation mode selection
- Process isolation
- IPC communication
- Crash detection
- Resource limits
- Signature verification

Run tests:
```
cd Engine/Tests
cmake --build . --config Release
ctest --output-on-failure
```

---

## Performance Benchmarks

See [PerformanceBenchmarks.cpp](../Tests/PerformanceBenchmarks.cpp) for detailed performance analysis:
- IPC overhead measurements
- Shared memory throughput
- In-Worker vs PluginHost comparison
- Memory usage tracking

Run benchmarks:
```
cd Engine/Tests
./PerformanceBenchmarks.exe
```

---

## Additional Resources

- **MASTER_PLAN.md**: Overall project roadmap and sprint planning
- **SDK Documentation**: Plugin development SDK (coming in Sprint 15)
- **API Reference**: Engine API documentation (auto-generated from code)

---

*Last Updated: February 9, 2026 - Sprint 14: Plugin Security Infrastructure*


