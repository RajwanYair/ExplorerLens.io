# Security Policy

## Supported Versions

We provide security updates for the following versions:

| Version | Supported          |
| ------- | ------------------ |
| 6.x.x   | :white_check_mark: |
| 5.2.x   | :white_check_mark: |
| < 5.2   | :x:                |

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

## Best Practices for Users

1. **Plugin Sources:** Only install plugins from trusted sources
2. **Digital Signatures:** Verify plugin signatures before installation
3. **Updates:** Keep DarkThumbs and plugins updated
4. **Permissions:** Review plugin permissions before approval
5. **Trust Management:** Regularly audit trusted publishers

## Security Contacts

For security-related questions or to report vulnerabilities privately, please contact the project maintainers through the repository.
