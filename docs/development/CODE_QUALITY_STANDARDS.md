# DarkThumbs Code Quality Standards
# Sprint 63: Static Analysis Configuration
# Version: 7.1.0

## Overview

DarkThumbs enforces code quality through multiple layers of static analysis,
both locally and in CI/CD pipelines.

## Tools

### 1. Clang-Tidy (Primary)
Configuration: `.clang-tidy` in project root.

**Run locally:**
```powershell
# Single file
clang-tidy Engine/Core/ThumbnailDecoder.h -- -std=c++20 -I Engine/include

# All engine headers  
Get-ChildItem Engine -Recurse -Include *.h,*.cpp | ForEach-Object {
    clang-tidy $_.FullName -- -std=c++20
}
```

**Key checks enabled:**
- `bugprone-*` — Common bug patterns
- `modernize-*` — C++17/20 modernization  
- `performance-*` — Performance anti-patterns
- `readability-*` — Code readability
- `cppcoreguidelines-*` — C++ Core Guidelines

### 2. MSVC Code Analysis (CI)
Enabled via `/p:EnableCppCoreCheck=true /p:RunCodeAnalysis=true` in CI builds.

### 3. Header Guard Validation (CI)
Automated check that all `.h` files contain `#pragma once` or traditional guards.

## Naming Conventions

| Element | Style | Example |
|---------|-------|---------|
| Classes | PascalCase | `ThumbnailDecoder` |
| Functions | PascalCase | `GetThumbnail()` |
| Variables | camelCase | `imageWidth` |
| Constants | UPPER_CASE | `MAX_THUMBNAIL_SIZE` |
| Private members | m_ prefix | `m_decoder` |
| Enums | PascalCase | `DecodeResult` |

## Suppressing Warnings

Use `NOLINT` comments sparingly and with justification:
```cpp
// NOLINT(cppcoreguidelines-pro-type-reinterpret-cast) — COM interface cast required by Windows API
auto* ptr = reinterpret_cast<IStream*>(stream);
```

## CI Integration

The `code-quality.yml` workflow runs on every push to `main`/`develop` and on PRs:
1. **Lint job** — clang-format check
2. **Analyze job** — MSVC Code Analysis with CppCoreCheck
3. **Header check job** — Validates include guards

All three jobs must pass for a clean quality gate.
