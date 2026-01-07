# Plugin Sandbox Model v1.0 (Sprint 12)

**Date:** January 6, 2026
**Status:** Draft

## 1. Safety Philosophy

"No single bad thumbnail parser should be able to take down the Explorer shell or steal user data."

To achieve this, we do not inspect code; we constrain *execution context*.

## 2. Isolation Modes

We support three levels of isolation, configurable per-plugin or globally via Group Policy.

### Mode 0: In-Process (Legacy/Trusted)

- **Host:** `DarkThumbsWorker.exe`
- **Security:** None. Runs with the same token as the worker (typically standard user).
- **Use Case:** First-party decoders (LibAVIF) or Enterprise "Trusted" internal plugins.
- **Risk:** High. Crash kills the worker.

### Mode 1: Restricted Worker (Default for Signed 3rd Party)

- **Host:** `DarkThumbsPluginHost.exe` (Child process)
- **Mechanism:**
  - Spawned via IPC (Named Pipe).
  - Windows Job Object applied.
- **Restrictions:**
  - **Memory Limit:** 512MB hard cap per host.
  - **Time Limit:** 5000ms execution timeout per thumbnail.
  - **Process Policy:** `JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE`.
- **Recovery:** If plugin crashes, only the single thumbnail request fails. The Host is restarted.

### Mode 2: AppContainer (Untrusted/Future)

- **Host:** `DarkThumbsSandboxedHost.exe`
- **Security:** Windows AppContainer (Low Integrity Level).
- **Capabilities:**
  - `internetClient` (if Network capability requested).
  - No file access (Target file strictly passed via handle/stream).
- **Use Case:** Community plugins from the public marketplace.

## 3. Policy Enforcement

| Capability | Mode 0 | Mode 1 | Mode 2 |
|---|---|---|---|
| Read Arbitrary File | ✅ | ⚠️ (Audited) | ❌ (Strict Handle) |
| Read Target File | ✅ | ✅ | ✅ |
| Network Access | ✅ | ⚠️ (Opt-in) | ⚠️ (Capability gated) |
| GPU Access | ✅ | ✅ | ❌ (Software fallback) |

## 4. Crash Containment Strategy

1. **Watchdog Timer:** The main worker starts a timer before calling `GetThumbnail`.
2. **Circuit Breaker:** If a specific `PluginID` crashes more than 3 times in 5 minutes, it is auto-disabled.
3. **Report:** Crash dumps are captured by `DarkThumbs.CrashHandler.exe` (MiniDumpWriteDump) and logged.
