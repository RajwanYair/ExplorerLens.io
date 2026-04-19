---
applyTo: "**/*.h,**/*.cpp"
---

# C++ Coding Standards — ExplorerLens

## Language & Toolchain

- **Standard:** C++20 — use `std::span`, `std::format`, concepts, ranges, modules (future)
- **Compiler:** MSVC cl.exe 19.50 (v145 toolset); never target GCC intrinsics
- **Warning level:** `/W4 /WX` — all warnings are errors; **zero warnings policy**
- **No `/wdXXXX` suppressions** — fix the root cause

## Naming Conventions

| Symbol | Convention | Example |
|--------|-----------|---------|
| Classes/structs | `PascalCase` | `JpegDecoder`, `SubMillisecondCacheEngine` |
| Functions/methods | `PascalCase` | `DecodeAtSize()`, `ProbeHeader()` |
| Local variables | `camelBack` | `pixelWidth`, `decoderResult` |
| Private members | `m_` prefix | `m_handle`, `m_bytesDecoded` |
| Constants/enums | `UPPER_CASE` | `MAX_THUMBNAIL_SIZE`, `DecodeResult::SUCCESS` |
| Enum classes | `PascalCase` type, `UPPER_CASE` values | `enum class DecodeResult { SUCCESS, ERROR }` |

## Namespace

All engine code uses `namespace ExplorerLens { namespace Engine { } }`.
Tests use `using namespace ExplorerLens::Engine;`.

## Header Rules

- All headers start with the standard banner (before `#pragma once`):
  ```cpp
  // FileName.h — Short Title
  // Copyright (c) 2026 ExplorerLens Project
  //
  // Description of what this header provides.
  //
  #pragma once
  ```
- **No `=====` decorators, version numbers, or sprint/batch tags** in headers
- Use `#pragma once` — never include guards
- Forward-declare instead of including when possible

## Windows SDK Restrictions

Because `WIN32_LEAN_AND_MEAN` is globally defined:
- **NEVER** include `<versionhelpers.h>` — use `RtlGetVersion()` from ntdll.dll
- **NEVER** include headers that depend on excluded types
- Guard Windows-only code with `#ifdef _WIN32`
- Platform stubs use `#ifdef _WIN32 / __APPLE__ / __linux__` guards

## GCC Intrinsics Prohibition

- **NEVER use `__builtin_*`** — GCC-only; MSVC will not compile
- Use `memcpy()` not `__builtin_memcpy()`
- Use `<intrin.h>` for x86 intrinsics (`_mm_*`, `_mm256_*`)

## Windows Macro Conflicts

- **Always parenthesize `std::min`/`std::max`**: `(std::min)(a, b)`, `(std::max)(a, b)`
- Or `#undef min` / `#undef max` after Windows includes with justification comment

## Memory & Resource Management

- Use RAII; never raw `new`/`delete` for owned resources
- Prefer `std::unique_ptr`, `ComPtr<T>` for COM interfaces
- Pool allocations for hot paths (< 4 KB buffers on stack or pool allocator)
- Check for heap fragmentation in decode loops

## COM Interfaces

- Implement `IUnknown` via `AddRef`/`Release` with `std::atomic<ULONG>` ref count
- Thread-safety: `IThumbnailProvider::GetThumbnail` may be called concurrently
- Never hold UI thread locks in decoder paths

## LENSTYPE Enum

- **Values must not collide** — always `grep_search` before adding
- Values are `UPPER_CASE`
- Check `LENSShell/LENSArchive.h` before assigning any new value

## Pre-Sprint Type Collision Check (MANDATORY)

Before committing any new header batch:
```powershell
# Grep every new struct/enum class/class name across Engine/**/*.h
# Zero matches required (excluding the file being created)
grep_search "struct MyNewType" Engine/**/*.h
```

## Test Registration (for new source files)

1. Header → `Engine/CMakeLists.txt` ENGINE_HEADERS
2. Source → `Engine/CMakeLists.txt` ENGINE_SOURCES
3. Test bodies → `Engine/Tests/EngineTests_Late.cpp`
4. `extern void Runner()` → `Engine/Tests/EngineTestsExterns.h`
5. `RUN_TEST()` calls → `Engine/Tests/EngineTests.cpp`
6. `#include` → `Engine/Tests/EngineTestsIncludes.h`

## Forbidden Patterns

```cpp
// ❌ Raw arrays with size mismatch risk
char buf[256];
strcpy(buf, longInput); // buffer overflow

// ✅ Use span or std::string
std::string buf = input;

// ❌ GCC intrinsic
__builtin_memcpy(dst, src, n);

// ✅ Standard
memcpy(dst, src, n);

// ❌ Windows macro conflict
std::min(a, b);

// ✅ Parenthesized
(std::min)(a, b);
```
