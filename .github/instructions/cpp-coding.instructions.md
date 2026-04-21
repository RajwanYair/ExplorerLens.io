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

## C++23 Migration Patterns

ExplorerLens targets C++20 today but MSVC v145 already supports many C++23 features
behind `/std:c++latest`. Use these patterns when the compiler supports them.

### `std::expected<T, E>` — Error Handling Without Exceptions

Prefer `std::expected` over custom result types for functions that can fail:

```cpp
#include <expected>

// ❌ Old pattern: return code + out-parameter
bool DecodeFile(const wchar_t* path, HBITMAP* out, std::wstring* errMsg);

// ✅ C++23: std::expected
std::expected<HBITMAP, DecodeError> DecodeFile(const wchar_t* path);

// Usage:
auto result = DecodeFile(path);
if (result) {
    UseThumb(*result);
} else {
    LogError(result.error());
}
```

When to use:
- Public API boundaries (decoder entry points, pipeline stages)
- Functions where callers must handle the error

When NOT to use:
- Internal hot-path helpers where a `bool` return is sufficient
- Functions that only fail on programming errors (use `assert`)

### `std::format` — Type-Safe Formatting

Already available in C++20 via `<format>`. Prefer over `sprintf`/`swprintf`:

```cpp
#include <format>

// ❌ Buffer overflow risk
char buf[128];
sprintf(buf, "Decoded %d×%d in %.1fms", w, h, ms);

// ✅ Type-safe, bounds-safe
auto msg = std::format("Decoded {}×{} in {:.1f}ms", w, h, ms);
auto wmsg = std::format(L"File: {} ({} bytes)", path, size);
```

### Structured Bindings — Preferred for Multi-Return

```cpp
// ❌ Verbose
auto pair = cache.Lookup(key);
bool found = pair.first;
CacheEntry entry = pair.second;

// ✅ Structured binding
auto [found, entry] = cache.Lookup(key);
```

### `std::print` (C++23) — Direct Output

When available, prefer `std::print` over `std::cout` in CLI tools:

```cpp
#include <print>

// ❌ Verbose stream insertion
std::cout << "Processed " << count << " files in " << ms << "ms\n";

// ✅ C++23 direct print
std::print("Processed {} files in {}ms\n", count, ms);
```

### Ranges — Prefer Over Raw Loops for Collection Operations

```cpp
#include <ranges>
#include <algorithm>

// ❌ Index-based loop with manual filter
std::vector<Decoder*> active;
for (size_t i = 0; i < decoders.size(); ++i)
    if (decoders[i]->IsEnabled()) active.push_back(decoders[i]);

// ✅ Ranges pipeline
auto active = decoders | std::views::filter(&Decoder::IsEnabled);
```

### Concepts — Constrain Template Parameters

```cpp
#include <concepts>

// ❌ Unconstrained template — error messages are cryptic
template <typename T>
void Render(T& decoder) { decoder.Decode(); }

// ✅ Concept-constrained
template <typename T>
concept Decodable = requires(T d) { { d.Decode() } -> std::same_as<DecodeResult>; };

template <Decodable T>
void Render(T& decoder) { decoder.Decode(); }
```
