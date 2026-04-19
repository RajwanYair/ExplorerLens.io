---
applyTo: "**/*.h,**/*.cpp,**/*.ps1,**/*.yml,**/*.yaml"
---

# Security Rules — ExplorerLens (OWASP-aligned)

## Critical Context: COM DLL Loaded by Explorer

`LENSShell.dll` is loaded into **every Explorer process** on the system.
A security vulnerability here affects ALL users on the machine.
Apply the highest security bar to LENSShell and Engine decoder code.

---

## A1 — Broken Access Control

- Registry keys: use `KEY_READ` only for enumeration; `KEY_SET_VALUE` only when explicitly needed
- File system: never follow symlinks without validation (`DeviceIoControl(FSCTL_GET_REPARSE_POINT)`)
- COM registration: verify CLSID at initialization; reject unexpected CLSIDs
- Plugin loading: verify digital signature before loading any third-party decoder DLL

## A2 — Cryptographic Failures

- Plugin trust chain: use SHA-256 for plugin manifest hashes
- Cache keys: use `SHA-256(canonical_path + mtime + size + dimensions)` — never use path alone
- No custom crypto: use Windows CNG (`BCryptHash`, `BCryptVerifySignature`)

## A3 — Injection

```cpp
// ❌ NEVER build shell commands from user-supplied paths
system(("thumbnailer " + userPath).c_str());

// ✅ Use parameterized APIs only
ShellExecuteExW(&sei);  // with lpFile/lpParameters separate
CreateProcessW(appPath, commandLine, ...);  // appPath is trusted binary

// ❌ NEVER pass user paths to format strings
sprintf(buf, userSuppliedFormat, ...);

// ✅ Treat file paths as opaque bytes; validate before use
if (!IsPathSafe(path)) return E_INVALIDARG;
```

## A4 — Insecure Design

- Decoder input is **untrusted** — any file the user opens could be malicious
- All decode operations must be bounded: max file size, max decoded dimensions, timeout
- Two-phase decode: ProbeHeader MUST check magic bytes before any allocation

```cpp
// Required size guards in every decoder
constexpr size_t MAX_DECODE_SIZE = 4096 * 4096 * 4;  // 64 MB max bitmap
if (static_cast<size_t>(width) * height * 4 > MAX_DECODE_SIZE)
    return DecodeResult{false, L"Image too large for thumbnail"};
```

## A5 — Security Misconfiguration

- Build flags required in `CMakeLists.txt` (already set; never remove):
  ```cmake
  /DYNAMICBASE  # ASLR
  /NXCOMPAT     # DEP
  /GUARD:CF     # Control Flow Guard
  /CETCOMPAT    # CET Shadow Stack
  ```
- Never use `#pragma comment(linker, "/SAFESEH:NO")` — it disables SafeSEH
- All COM objects must implement `IUnknown` correctly (no reference count bugs)

## A6 — Vulnerable Components

- All 18 external libraries must be kept current — check with Dependabot
- When a CVE is published for a dependency, update within 7 days
- Check: `vcpkg x-check-support <package>` for known vulnerabilities

## A8 — Software Integrity Failures

- CI workflows: pin all third-party GitHub Actions to a specific SHA, not a tag
  ```yaml
  # ❌ NEVER
  uses: actions/checkout@v4
  # ✅ ALWAYS
  uses: actions/checkout@11bd71901bbe5b1630ceea73d27597364c9af683  # v4.2.2
  ```
- Sign release artifacts with Authenticode before publishing
- Plugin SDK: plugins must be signed; unsigned plugins are rejected at load time

## A9 — Logging & Monitoring Failures

- All decode errors must be logged to Windows Event Log (not just ETW)
- Failed COM registration attempts: log to Event Log with source `ExplorerLens`
- Never log file contents or user data — log only error codes and paths (truncated)

## PowerShell Script Security

```powershell
# ✅ ALWAYS use -LiteralPath for user-supplied paths
Get-Item -LiteralPath $userPath

# ❌ NEVER use -Path with user input (supports wildcards = injection)
Get-Item -Path $userPath

# ✅ Validate before use
if ($userPath -notmatch '^[a-zA-Z]:\\[\w\\ ._-]+$') {
    Write-Error "Invalid path"; exit 1
}

# ✅ No hardcoded credentials or proxy URLs
# ❌ $proxy = "http://proxy.intel.com:911"  ← never commit this
```

## Pre-Commit Security Scan

```powershell
# Scrub corporate/sensitive artifacts before every commit
git grep -rn "intel.com|password|secret|token|api.key|928\b" `
    -- "*.ps1" "*.yml" "*.h" "*.cpp" "*.json" "*.md"
```
