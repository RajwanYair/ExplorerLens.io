# Security Policy

## Supported Versions

We provide security patches and updates for the following release lines:

| Version | Supported |
| ------- | --------- |
| 36.x    | :white_check_mark: **Current** |
| 35.x    | :white_check_mark: Security fixes |
| 34.x    | :white_check_mark: Security fixes only |
| < 34.0  | :x: End of Life |

> We strongly recommend running the latest release (v36.0.0 "Altair").
> Patch releases are issued within 7 days for critical/high CVEs.

## Reporting a Vulnerability

**Please do not report security vulnerabilities through public GitHub issues.**

### How to Report

1. **GitHub Private Advisory:** Use [GitHub Security Advisories](https://github.com/RajwanYair/ExplorerLens.io/security/advisories/new) (preferred)
2. **Email:** Contact the maintainer directly via profile contact information
3. **Include:**
   - Description of the vulnerability
   - Steps to reproduce
   - Affected version(s)
   - Potential impact assessment
   - Suggested fix (if available)

### What to Expect

| Severity | Initial Response | Resolution Target |
|----------|-----------------|-------------------|
| Critical (CVSSv3 9.0+) | 24 hours | 7 days |
| High (7.0–8.9) | 48 hours | 30 days |
| Medium (4.0–6.9) | 5 days | 90 days |
| Low (< 4.0) | 14 days | Next minor release |

### Responsible Disclosure

- We will work with you to understand and resolve the issue
- We will credit you in the security advisory (unless you prefer anonymity)
- We request a coordinated disclosure window (max 90 days) before public disclosure
- Critical fixes are released as patch releases outside the normal sprint cycle

## Security Features

For a detailed description of security architecture, see
[docs/SECURITY_HARDENING.md](../docs/SECURITY_HARDENING.md).

### Plugin Security (v24.0+)

- **AppContainer Sandbox:** All plugins run in restricted low-IL containers
- **Code Signing:** Only Authenticode-signed plugins are loaded
- **Trust Chain Verification:** Publisher certificates validated against trust store
- **Process Isolation:** Plugins cannot access host process memory
- **Capability Limits:** No network, restricted filesystem (read-only whitelisted paths)

### Build Security

- **Zero warnings policy:** `/W4 /WX` on MSVC v145 — warnings are treated as errors
- **CodeQL scanning:** Automated via `.github/workflows/codeql.yml` on every push and PR
- **Dependency review:** Dependabot monitors all ecosystems (pip, npm, GitHub Actions, NuGet)
- **Binary reproducibility:** Release artifacts include SHA256SUMS.txt and CycloneDX SBOM
- **Supply chain:** SLSA Level 2 compliance through GitHub-hosted build runners

### Runtime Security

- **COM isolation:** Shell extension runs in Explorer's process with minimal privilege
- **Memory safety:** RAII patterns, smart pointers, no manual `new`/`delete` in hot paths
- **Input validation:** All file format parsers validate headers before buffer allocation
- **GPU resource limits:** DirectX fence timeouts prevent runaway compute shaders
- **ZeroTrustPolicyEngine:** Per-plugin capability negotiation at load time

### Shell Extension Security

- **Memory Safety:** RAII + smart pointers throughout; no raw `new`/`delete` in decoders
- **Input Validation:** All file paths validated; no shell-string construction from user input
- **Compile-Time Hardening:** `/GS /DYNAMICBASE /NXCOMPAT /SAFESEH /guard:cf`
- **Resource Limits:** Per-thumbnail CPU and memory quotas enforced by engine
- **Buffer Overflow Protection:** Stack canaries, ASLR, and Control Flow Guard enabled
- **DEP/NX:** Data Execution Prevention enforced on all binaries


### Archive Handling Security (v7.0+)

- **Zip Bomb Detection:** Compression ratio limits prevent decompression bombs
- **Path Traversal Prevention:** Archive entry paths sanitized (no `../` escape)
- **Size Limits:** Maximum extracted size enforced per archive
- **Format Validation:** Magic bytes verified before decoding
- **Timeout Guards:** Per-file decode timeout prevents CPU exhaustion

### Build Security

- **CodeQL Analysis:** Automated on every push to `main` via GitHub Actions
- **Zero Warnings Policy:** All warnings treated as potential issues
- **Static Analysis:** MSVC CppCoreCheck enabled in CI
- **Dependency Scanning:** vcpkg dependencies pinned to specific versions

## Best Practices for Users

1. **Plugin Sources:** Only install plugins from trusted sources
2. **Digital Signatures:** Verify plugin signatures before installation
3. **Updates:** Keep ExplorerLens and plugins updated
4. **Permissions:** Review plugin permissions before approval
5. **Trust Management:** Regularly audit trusted publishers

## Security Contacts

For security-related questions or to report vulnerabilities privately, please contact the project maintainers through the repository.
