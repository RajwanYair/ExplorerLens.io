---
mode: agent
description: "Perform a thorough code review of the selected file or changes"
---

# Code Review

Review the selected code against workspace standards.

## Target

File: `${file}`
Selection: `${selection}`

## Review Checklist

### Security (OWASP Top 10)

- [ ] No hardcoded credentials, tokens, or API keys
- [ ] No `shell=True` with user-controlled input (command injection)
- [ ] No SQL/template injection vectors
- [ ] Input validated at system boundaries
- [ ] Sensitive data not logged in plaintext
- [ ] No SSRF vectors (external URLs not constructed from user input)

### Python Quality

- [ ] Type hints on all function signatures
- [ ] No bare `except:` clauses — use specific exception types
- [ ] No mutable default arguments (`def f(items=[])`)
- [ ] No `print()` in library/core code — use `logging` or `rich.console`
- [ ] `pathlib.Path` used instead of `os.path` string manipulation
- [ ] Context managers (`with`) used for all file/resource operations
- [ ] No hardcoded absolute paths

### Architecture

- [ ] Single responsibility — functions do one thing
- [ ] Dataclasses used for structured data (not raw dicts)
- [ ] Enums used for constants (not magic strings/numbers)
- [ ] Signal handlers implemented (SIGTERM/SIGINT)
- [ ] Configuration loaded from YAML/env, not hardcoded

### Testing

- [ ] New code has corresponding tests
- [ ] Error paths tested (not just happy path)
- [ ] No test bypasses or `# pragma: no cover` without justification

## Output Format

Provide feedback as:
1. **Critical issues** (blockers) — security, data loss, crashes
2. **Standard issues** (should fix) — quality, maintainability
3. **Suggestions** (optional) — style, performance, clarity
