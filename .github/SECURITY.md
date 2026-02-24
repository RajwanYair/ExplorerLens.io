# Security Policy

## Supported Versions

We provide security updates for the following versions:

| Version | Supported          |
| ------- | ------------------ |
| 14.0.x  | :white_check_mark: Current |
| 13.x.x  | :white_check_mark: |
| 12.x.x  | :white_check_mark: Security fixes only |
| < 12.0  | :x:                |

## Reporting a Vulnerability

**Please do not report security vulnerabilities through public GitHub issues.**

### How to Report

1. **Email:** Send details to the project maintainers (check repository for contact info)
2. **Include:**
   - Description of the vulnerability
   - Steps to reproduce
   - Potential impact
   - Suggested fix (if available)

### What to Expect

- **Initial Response:** Within 48 hours
- **Status Updates:** Every 7 days until resolved
- **Resolution Timeline:**
  - Critical vulnerabilities: 7 days
  - High severity: 30 days
  - Medium/Low severity: 90 days

### Responsible Disclosure

- We will work with you to understand and resolve the issue
- We will credit you in the security advisory (unless you prefer anonymity)
- We request 90 days before public disclosure to allow time for fixes

## Security Features

### Plugin Security (v6.0+)

- **AppContainer Sandbox:** All plugins run in restricted containers
- **Code Signing:** Only signed plugins can be loaded
- **Trust Verification:** Publisher certificates validated
- **Process Isolation:** Plugins cannot access host process memory
- **Capability Limits:** Restricted filesystem and no network access

### Shell Extension Security

- **Memory Safety:** RAII patterns prevent leaks
- **Exception Handling:** Comprehensive error handling prevents crashes
- **Input Validation:** All file paths and data validated
- **Resource Limits:** CPU and memory quotas enforced
- **Buffer Overflow Protection:** Stack canaries and ASLR enabled via `/GS /DYNAMICBASE`
- **DEP/NX:** Data Execution Prevention enabled via `/NXCOMPAT`
- **Control Flow Guard:** `/guard:cf` enabled in Release builds

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

