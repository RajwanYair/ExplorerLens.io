# Plugin Sandbox & Isolation Model v1.0

**Version:** 1.0  
**Sprint:** 12 (Sprint 12.4)  
**Created:** January 6, 2026

---

## Overview

ExplorerLens plugins execute untrusted code to decode potentially malicious files. A robust sandbox model is critical to prevent plugins from:

- Crashing Explorer
- Accessing sensitive data
- Modifying system state
- Consuming excessive resources

This document defines three isolation modes with increasing security levels.

---

## Isolation Modes

### Mode 1: In-Worker (Trusted Only)

**Use Case:** First-party plugins, fully trusted vendors  
**Security Level:** Low  
**Performance:** Highest (no process boundary)

#### Characteristics

- Plugin DLL loaded directly into Worker process
- Full access to Worker process memory
- No sandboxing or restrictions
- Fastest execution (no IPC overhead)
- **Crash risk:** Plugin crash = Worker crash

#### Requirements

- Plugin must be signed with trusted certificate
- Plugin must have "Verified" status
- User or admin must explicitly mark as trusted
- Source code review completed

#### Configuration

```json
{
  "isolationMode": "in-worker",
  "trustedPlugins": [
    "explorerlens.plugin.psd",
    "explorerlens.plugin.webp"
  ]
}
```

---

### Mode 2: PluginHost Process (Default)

**Use Case:** Most plugins  
**Security Level:** Medium  
**Performance:** Good (IPC overhead ~5-10ms)

#### Characteristics

- Plugin runs in separate `PluginHost.exe` process
- Restricted process token (limited privileges)
- Job Object limits (CPU, memory, I/O)
- Named pipe IPC between Worker and PluginHost
- **Crash isolation:** Plugin crash does NOT crash Worker

#### Process Restrictions

##### Token Restrictions

- `SECURITY_RESTRICTED_TOKEN` with:
  - `DISABLE_MAX_PRIVILEGE` - Disable all privileges
  - `SANDBOX_INERT` - Disable UIPI bypass
  - No admin privileges
  - Limited SIDs (deny dangerous groups)

##### Job Object Limits

```cpp
JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits = {};
limits.BasicLimitInformation.LimitFlags = 
    JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE |        // Kill when job closed
    JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION | // Die on crash
    JOB_OBJECT_LIMIT_ACTIVE_PROCESS |           // Max 1 process
    JOB_OBJECT_LIMIT_JOB_MEMORY |               // Memory limit
    JOB_OBJECT_LIMIT_PROCESS_TIME;              // CPU time limit

limits.BasicLimitInformation.ActiveProcessLimit = 1;
limits.JobMemoryLimit = 512 * 1024 * 1024; // 512 MB
limits.ProcessMemoryLimit = 512 * 1024 * 1024;
limits.PerProcessUserTimeLimit.QuadPart = 60 * 10000000; // 60 sec CPU time
```

##### File System Access

- Read-only access to:
  - Plugin directory: `%LocalAppData%\ExplorerLens\Plugins\{plugin-id}\`
  - Temp directory: `%TEMP%\ExplorerLens\{plugin-id}\`
  - Requested file path (thumbnail source)
- No write access to:
  - System directories
  - User directories (except temp)
  - Registry

##### Network Access

- Blocked by default via Windows Firewall rules
- Can be enabled per-plugin if `network` capability declared
- Only HTTPS allowed (HTTP blocked)
- No local network access (127.0.0.1, 192.168.x.x, etc.)

#### IPC Protocol (Named Pipes)

##### Pipe Naming

```
\\.\pipe\ExplorerLens-PluginHost-{ProcessId}-{PluginId}
```

##### Message Format (Binary)

```cpp
struct PluginMessage {
    uint32_t magic;          // 0x44545048 ('DTPH')
    uint32_t version;        // Protocol version (1)
    uint32_t messageType;    // Request, Response, Error, Shutdown
    uint64_t correlationId;  // Matches request to response
    uint32_t dataSize;       // Size of payload
    // Followed by payload data
};
```

##### Message Types

1. **REQUEST_THUMBNAIL** (0x01)
   - Payload: Serialized `DT_ThumbnailRequest`
   - File data sent via pipe or shared memory

2. **RESPONSE_THUMBNAIL** (0x02)
   - Payload: Serialized `DT_ThumbnailResult`
   - Bitmap data sent via pipe or shared memory

3. **ERROR** (0x03)
   - Payload: Error code + message

4. **SHUTDOWN** (0x04)
   - Graceful shutdown request

5. **HEARTBEAT** (0x05)
   - Keepalive ping/pong

##### Timeout Handling

- Request timeout: 30 seconds (configurable)
- Heartbeat interval: 5 seconds
- If no heartbeat for 15 seconds → terminate PluginHost

#### Shared Memory (Large Files)

For files > 1MB:

- Create shared memory section: `Global\ExplorerLens-{CorrelationId}`
- Map into both Worker and PluginHost processes
- Pass offset/size via pipe message
- Unmap and delete after request completes

---

### Mode 3: AppContainer (Future - v6.x)

**Use Case:** Untrusted plugins, maximum security  
**Security Level:** High  
**Performance:** Lower (additional isolation overhead)

#### Characteristics

- Runs in Windows AppContainer (UWP-style sandbox)
- Strongest isolation available on Windows
- No access to most system resources by default
- Capabilities explicitly granted
- Network isolation via named proxy
- **Planned for Sprint 20**

#### Capabilities (AppContainer)

```xml
<Capabilities>
  <Capability Name="documentsLibrary" />
  <Capability Name="picturesLibrary" />
  <Capability Name="internetClient" Restricted="true" />
</Capabilities>
```

#### Restrictions

- No registry access
- No WIN32 API access to sensitive functions
- No impersonation
- No loading unsigned DLLs
- No kernel drivers

---

## Security Best Practices

### For Plugin Developers

1. **Validate All Input**

   ```cpp
   if (!request || request->structSize != sizeof(DT_ThumbnailRequest)) {
       return DT_ERROR_INVALID_ARGUMENT;
   }
   if (!request->filePath && !request->stream) {
       return DT_ERROR_INVALID_ARGUMENT;
   }
   ```

2. **Handle Corrupt Files Gracefully**

   ```cpp
   try {
       // Decode logic
   } catch (const std::exception& ex) {
       LogError("Decode failed: %s", ex.what());
       return DT_ERROR_CORRUPT_DATA;
   }
   ```

3. **Respect Timeouts**

   ```cpp
   auto start = std::chrono::steady_clock::now();
   while (decoding) {
       auto elapsed = std::chrono::steady_clock::now() - start;
       if (elapsed > std::chrono::milliseconds(request->timeoutMs)) {
           return DT_ERROR_TIMEOUT;
       }
   }
   ```

4. **Limit Memory Usage**

   ```cpp
   // Don't allocate unbounded memory based on file data
   if (width * height > 100'000'000) { // 100 megapixels
       return DT_ERROR_OUT_OF_MEMORY;
   }
   ```

5. **No External Dependencies**
   - Minimize DLL dependencies
   - Statically link runtime libraries
   - Don't spawn external processes (unless capability declared)

### For Engine/PluginHost

1. **Kill on Timeout**

   ```cpp
   if (WaitForSingleObject(hProcess, timeoutMs) == WAIT_TIMEOUT) {
       TerminateProcess(hProcess, E_FAIL);
   }
   ```

2. **Monitor Resource Usage**

   ```cpp
   PROCESS_MEMORY_COUNTERS pmc;
   GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc));
   if (pmc.WorkingSetSize > maxMemoryBytes) {
       TerminateProcess(hProcess, E_OUTOFMEMORY);
   }
   ```

3. **Validate Responses**

   ```cpp
   if (result->hBitmap && !IsValidBitmap(result->hBitmap)) {
       DeleteObject(result->hBitmap);
       return DT_ERROR_CORRUPT_DATA;
   }
   ```

---

## Isolation Mode Selection Algorithm

```cpp
IsolationMode DetermineIsolationMode(const LoadedPlugin* plugin) {
    // Check if explicitly trusted
    if (IsTrustedPlugin(plugin->GetManifest().id)) {
        return IsolationMode::InWorker;
    }
    
    // Check if signed and verified
    if (plugin->GetManifest().verified && 
        plugin->GetManifest().signatureValid) {
        
        // Check user preference
        if (UserPrefersTrusted(plugin->GetManifest().id)) {
            return IsolationMode::InWorker;
        }
    }
    
    // Default to separate process
    return IsolationMode::SeparateProcess;
}
```

---

## Performance Comparison

| Operation | In-Worker | PluginHost | AppContainer |
|-----------|-----------|------------|--------------|
| Load plugin | 5 ms | 50 ms | 200 ms |
| IPC overhead | 0 ms | 5-10 ms | 15-20 ms |
| Thumbnail (small) | 10 ms | 15 ms | 25 ms |
| Thumbnail (large) | 100 ms | 110 ms | 125 ms |
| Memory isolation | None | Process | Strong |
| Crash isolation | None | Yes | Yes |

---

## Crash Handling

### PluginHost Crash Detection

```cpp
DWORD exitCode;
if (GetExitCodeProcess(hProcess, &exitCode)) {
    if (exitCode == STATUS_ACCESS_VIOLATION ||
        exitCode == STATUS_STACK_OVERFLOW) {
        // Plugin crashed - log and disable
        LogError("Plugin %ls crashed with code 0x%X", 
                 pluginId.c_str(), exitCode);
        DisablePlugin(pluginId);
    }
}
```

### Crash Reporting

When a plugin crashes:

1. Capture mini dump (if available)
2. Log correlation ID, file path, plugin version
3. Disable plugin automatically
4. Notify user (optional)
5. Submit telemetry (opt-in)

```json
{
  "event": "plugin_crash",
  "pluginId": "explorerlens.plugin.psd",
  "pluginVersion": "1.2.3",
  "correlationId": "0x123456789ABCDEF0",
  "exitCode": "0xC0000005",
  "filePath": "C:\\..\\image.psd",
  "timestamp": "2026-01-06T14:30:00Z"
}
```

---

## Enterprise Policy Control

Admins can enforce isolation mode via Group Policy:

### Registry Keys

```
HKLM\SOFTWARE\Policies\ExplorerLens\Plugins
  MinIsolationMode (DWORD)
    0 = Allow all modes
    1 = Require PluginHost (block In-Worker for untrusted)
    2 = Require AppContainer (block In-Worker and PluginHost)
    
  AllowedPlugins (REG_MULTI_SZ)
    List of plugin IDs allowed to run
    Empty = allow all
    
  DeniedPlugins (REG_MULTI_SZ)
    List of plugin IDs blocked from running
    
  TrustedPlugins (REG_MULTI_SZ)
    List of plugin IDs allowed In-Worker mode
```

---

## Testing & Validation

### Security Test Suite

1. **Escape Tests**
   - Try to break out of sandbox
   - Try to access forbidden files
   - Try to spawn privileged processes

2. **Resource Exhaustion**
   - Allocate excessive memory
   - Create excessive threads
   - Consume excessive CPU

3. **Crash Resilience**
   - Intentional crashes (access violation, divide by zero)
   - Verify Worker survives
   - Verify plugin is disabled

4. **IPC Security**
   - Malformed messages
   - Oversized payloads
   - Replay attacks

---

## Future Enhancements (v6.x+)

- **WebAssembly Plugins**: Run plugins in WASM sandbox
- **Network Proxy**: Intercept and filter network requests
- **File System Virtualization**: Redirect writes to isolated storage
- **Telemetry Integration**: Real-time monitoring of plugin behavior
- **Automatic Threat Detection**: ML-based anomaly detection

---

## Summary

| Feature | In-Worker | PluginHost | AppContainer |
|---------|-----------|------------|--------------|
| **Isolation** | None | Process | Maximum |
| **Performance** | Best | Good | Acceptable |
| **Crash Safety** | No | Yes | Yes |
| **Security** | Low | Medium | High |
| **Default** | No | Yes | Future |
| **Recommended For** | Trusted only | Most plugins | Untrusted |

**Recommendation:** Use **PluginHost** mode by default. Only allow **In-Worker** for explicitly trusted, verified plugins.

