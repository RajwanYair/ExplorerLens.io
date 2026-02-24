# Observability Spec v1.0 (Sprint 11)

**Date:** January 6, 2026
**Status:** Draft

## 1. Philosophy

"If a user can't tell why a thumbnail failed, it is a bug."

We are moving away from ad-hoc `ODS` (OutputDebugString) logging to strictly structured logging and high-performance tracing (ETW).

## 2. ETW Provider

- **Provider Name:** `ExplorerLens-Engine-Core`
- **GUID:** `{D4257123-ABCD-4321-8765-000000000001}` (To be generated)
- **Symbol:** `ExplorerLens_Provider`

### 2.1 Key Events

| Event ID | Level | Name | Payload | description |
|---|---|---|---|---|
| 100 | Info | RequestStart | correlationId, pathHash, size | Start of thumbnail request |
| 101 | Info | RequestStop  | correlationId, hr, elapsedUs | End of request |
| 200 | Verbose | CacheHit | correlationId, keyHash | Found in cache |
| 201 | Verbose | CacheMiss | correlationId | Cache miss |
| 300 | Error | DecodeFail | correlationId, decoderName, hr | Decoder returned error |
| 400 | Critical | CrashCaught | exceptionCode, address | Structured exception handler caught crash |

## 3. Structured Logging (File/Debug)

For users who cannot capture ETW traces, we will support writing a structured text log (JSON-lines or TSV).

### 3.1 Schema

Every log line MUST contain:

- `timestamp_utc`: ISO8601
- `correlation_id`: Unique request ID
- `component`: (e.g., "Decoder", "Cache", "ShellHost")
- `level`: (INFO, WARN, ERROR, DEBUG)
- `message`: Human readable
- `context`: JSON object with variable data

**Example:**

```json
{"t": "2026-01-06T10:00:01Z", "cid": "a1b2", "comp": "WebP", "lvl": "ERR", "msg": "Invalid header", "ctx": {"offset": 0, "magic": "0x00"}}
```

## 4. Diagnostics Bundle

The Manager App will have a "Export Diagnostics" button. It produces a ZIP containing:

1. `system_info.json` (OS ver, Monitor config, GPU driver ver)
2. `config_dump.json` (Current effective settings)
3. `recent_logs.txt` (Last N MB of logs)
4. `registry_snapshot.reg` (HKCU/HKLM paths only)
5. `crash_dumps/` (Minidump files if any)

## 5. Privacy

- **PII Stripping:** File paths should be hashed in ETW by default. Full paths only in Verbose/Diagnostic mode.
- **Data Scrubbing:** No image content is ever logged.

