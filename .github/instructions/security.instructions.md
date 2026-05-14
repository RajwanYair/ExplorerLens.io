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

---

## Supply-Chain Security

### Dependency Integrity

1. **Lock dependency versions** — `vcpkg.json` must specify exact versions, not ranges.
2. **Verify checksums** when downloading external source archives:
   ```powershell
   # ✅ Always verify hash after download
   $hash = (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash
   if ($hash -ne $expectedHash) { throw "Checksum mismatch: $archive" }
   ```
3. **No curl-pipe-sh patterns** — never `iex (iwr ...)` or `curl | bash` in build scripts.
4. **Vendor critical dependencies** in `external/` with documented provenance.

### GitHub Actions Supply-Chain

1. **SHA-pin all third-party actions** (see `cicd.instructions.md` §Action Version Pinning Policy).
2. **First-party actions** (`actions/*`, `github/*`) may use semver tags (`@v4`).
3. **Audit action updates** — when Dependabot proposes an action version bump, verify
   the release notes and diff before merging.
4. **Never use `pull_request_target` with `actions/checkout`** on the PR head ref —
   this grants write permissions to untrusted code.

### Build Reproducibility

1. **Pin compiler version** in CI documentation (MSVC v143 on runners, v145 locally).
2. **Pin CMake version** via `lukka/get-cmake@v4.3.1` — never `@latest`.
3. **Record build environment** in SBOM metadata: compiler, OS, CMake version.

---

## SBOM (Software Bill of Materials)

### Generation

- `Generate-SBOM.ps1` produces `docs/SBOM.json` in CycloneDX 1.5 format.
- `Bump-Version.ps1` updates the SBOM version, timestamp, and serial number.
- SBOM is attached to every GitHub Release as `ExplorerLens-X.Y.Z-SBOM.json`.

### Rules

1. **Every external dependency must appear in the SBOM** with name, version, license, and PURL.
2. **Update SBOM when adding/upgrading a dependency** — `external/LIBRARY_INVENTORY.md` is the
   source of truth for component metadata.
3. **Never remove components from SBOM** without removing the actual dependency first.
4. **SBOM `serialNumber`** must be a unique URN per release — `Bump-Version.ps1` handles this.

---

## Secret Management

### Rules

1. **No secrets in source code** — use `${{ secrets.NAME }}` in workflows, environment
   variables in scripts, and Windows Credential Manager for local development.
2. **Rotate tokens** — GitHub PATs, NuGet API keys, and signing certificates must be
   rotated on the schedule defined in the repository security policy.
3. **`.gitignore` must include**: `.env`, `*.pfx`, `*.snk`, `*.p12`, `secrets.json`.
4. **Audit with `git log -p --all -S 'password'`** before publishing a repository.

### Secret Scanning Patterns

```powershell
# Pre-push scan for accidentally committed secrets
$patterns = @(
    'AKIA[0-9A-Z]{16}',           # AWS Access Key
    'ghp_[a-zA-Z0-9]{36}',        # GitHub PAT (classic)
    'github_pat_[a-zA-Z0-9_]{82}', # GitHub PAT (fine-grained)
    'sk-[a-zA-Z0-9]{48}',         # OpenAI API key
    'npm_[a-zA-Z0-9]{36}',        # npm token
    '-----BEGIN (RSA )?PRIVATE KEY', # PEM private key
    'SG\.[a-zA-Z0-9_-]{22}\.[a-zA-Z0-9_-]{43}' # SendGrid API key
)
$regex = ($patterns -join '|')
git diff --cached --diff-filter=ACMR -z --name-only |
    ForEach-Object { Select-String -LiteralPath $_ -Pattern $regex -AllMatches } |
    ForEach-Object { Write-Warning "POTENTIAL SECRET: $($_.Filename):$($_.LineNumber)" }
```

---

## Memory Safety in C++ Decoders

### Rules

1. **Bounds-check ALL buffer accesses** derived from file data — width, height, offsets,
   palette sizes, chunk lengths are all attacker-controlled.
2. **Use `std::span`** (C++20) for buffer views instead of raw pointer arithmetic.
3. **Integer overflow checks** before allocation:
   ```cpp
   // ✅ Check for overflow before allocating
   if (width > 0 && height > SIZE_MAX / (width * 4))
       return DecodeResult{false, L"Overflow in allocation size"};
   ```
4. **RAII for all resources** — `std::unique_ptr` with custom deleters for C library handles.
5. **No `alloca()` or variable-length arrays** — stack overflow risk with attacker-controlled sizes.
6. **Fuzz testing** — all decoders should be fuzzable targets (see `Engine/Tests/fuzz/`).

---

## Binary Signing (Authenticode)

### Release Signing Requirements

1. **All release DLLs and EXEs must be Authenticode-signed** before distribution.
2. **Use SHA-256 digest** (`/fd SHA256`) — SHA-1 is rejected by modern Windows.
3. **Timestamp with RFC 3161** (`/tr http://timestamp.digicert.com /td SHA256`) — ensures
   signatures remain valid after certificate expiry.
4. **Never commit signing certificates** (`.pfx`, `.p12`) to the repository.
5. **CI signing** uses `${{ secrets.SIGN_CERT_BASE64 }}` decoded to a temp file, used, then deleted.

```powershell
# ✅ Correct signing command (Build-MSVC.ps1 pattern)
signtool sign /f "$certPath" /p "$certPassword" /fd SHA256 `
    /tr http://timestamp.digicert.com /td SHA256 `
    /d "ExplorerLens" /du "https://github.com/RajwanYair/ExplorerLens.io" `
    "$dllPath"
```

### Unsigned Build Policy

- Debug builds and local development builds need not be signed.
- CI builds that skip signing must emit `::warning::Unsigned build — not for distribution`.
- Installer (MSI/MSIX) must be signed independently of the binaries it contains.

---

## Vulnerability Response

### Disclosure

- **Private vulnerability reporting** is enabled on the GitHub repository.
- Security issues are filed via GitHub Security Advisories (GHSA).
- The `SECURITY.md` file documents the reporting process and supported versions.

### Response SLA

| Severity | Response Time | Fix Time |
| ---------- | --------------- | ---------- |
| Critical (RCE, privilege escalation) | 24 hours | 72 hours |
| High (information disclosure, DoS) | 48 hours | 7 days |
| Medium (minor info leak, crash) | 7 days | 30 days |
| Low (cosmetic, minor hardening) | 14 days | Next release |

### CVE for Dependencies

- Subscribe to security advisories for all 18 external libraries.
- When a CVE is published, evaluate impact within 48 hours.
- If affected, update the library within the SLA above and issue a patch release.
- Document the CVE and fix in `CHANGELOG.md` under the "Security" category.
