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

### C++20 / MSVC Quality (ExplorerLens Engine)

- [ ] `#pragma once` on all headers
- [ ] Header banner: `// File.h — Title` + copyright + description (before `#pragma once`)
- [ ] Namespace: `namespace ExplorerLens { namespace Engine { ... } }`
- [ ] Classes `PascalCase`, functions `PascalCase`, variables `camelBack`, members `m_` prefix
- [ ] Enum values `UPPER_CASE` (enforced by `.clang-tidy`)
- [ ] No bare `new`/`delete` — use RAII, `std::unique_ptr`, `std::vector`
- [ ] `(std::min)(a, b)` and `(std::max)(a, b)` parenthesized (Windows macro conflict)
- [ ] No `__builtin_*` intrinsics (GCC-only; use `memcpy()`, `memset()` for MSVC)
- [ ] No `memmem()` (POSIX-only; use manual `memcmp` loop)
- [ ] No `<versionhelpers.h>` include (incompatible with `WIN32_LEAN_AND_MEAN`)
- [ ] No `/wdXXXX` warning suppressions — fix root cause
- [ ] No LENSTYPE enum value collisions (check `LENSArchive.h`)
- [ ] New headers registered in `Engine/CMakeLists.txt` `ENGINE_HEADERS`
- [ ] New sources registered in `ENGINE_SOURCES`

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
- [ ] Dataclasses / structs used for structured data (not raw dicts / ad-hoc fields)
- [ ] Enums used for constants (not magic strings/numbers)
- [ ] Signal handlers implemented (SIGTERM/SIGINT) where applicable
- [ ] Configuration loaded from YAML/env, not hardcoded

### Testing

- [ ] New code has corresponding tests
- [ ] Error paths tested (not just happy path)
- [ ] No test bypasses or `# pragma: no cover` without justification
- [ ] C++ tests use custom `TEST()`/`ASSERT()`/`RUN_TEST()` macros (not GTest)
- [ ] Test bodies in `EngineTests_Late.cpp`, extern decls in `EngineTestsExterns.h`

### Build & CI

- [ ] Compiles with 0 errors, 0 warnings under `/W4 /WX` on MSVC v145
- [ ] No toolset pin in CI workflows (let `ilammy/msvc-dev-cmd` auto-detect)
- [ ] No new dependencies without `Engine/CMakeLists.txt` link registration

## Output Format

Provide feedback as:
1. **Critical issues** (blockers) — security, data loss, crashes, build breaks
2. **Standard issues** (should fix) — quality, maintainability, naming violations
3. **Suggestions** (optional) — style, performance, clarity
