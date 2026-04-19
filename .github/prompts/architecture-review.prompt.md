---
mode: ask
description: "Architecture review for a new Engine module or subsystem. Checks naming collisions, validates the implement-before-declare rule, and ensures CMakeLists registration."
---

# Architecture Review — ExplorerLens Engine Module

Review the proposed module **{{moduleName}}** in `Engine/{{directory}}/`.

## Collision Check (Run First)

```powershell
# Search for ALL new class/struct/enum names across the codebase
# For each new type in the proposed module:
$names = @("{{typeName1}}", "{{typeName2}}")
foreach ($name in $names) {
    $hits = Select-String -Path Engine\**\*.h -Pattern "\b$name\b" -Recurse
    if ($hits) { Write-Host "COLLISION: $name found in $($hits.Path)" }
    else       { Write-Host "OK: $name is unique" }
}
```

**Zero collisions required before proceeding.**

## Implement-Before-Declare Check

For every proposed header file, verify:

- [ ] A corresponding `.cpp` exists (or will be created) with > 50 LOC of real logic
- [ ] The class/function has a clear non-trivial purpose (not just a stub or wrapper)
- [ ] At least 3 tests are planned that exercise real behavior (not just construction)

## Directory Placement Check

| Proposed location | Correct? | If not, where? |
|-------------------|----------|----------------|
| `Engine/Core/` | If it's pipeline/detection/enterprise | |
| `Engine/Decoders/` | If it decodes a file format | |
| `Engine/GPU/` | If it uses D3D/Vulkan | |
| `Engine/Cache/` | If it manages caching/memory | |
| `Engine/Platform/` | If it abstracts an OS API | |
| `Engine/Utils/` | If it's a utility with no better home | |

Reminder: `AI/`, `Enterprise/`, `Memory/`, `Pipeline/`, `Plugin/` are being merged
into their parent directories — don't create new files there.

## CMakeLists.txt Registration

Confirm the following will be updated:

```cmake
# In Engine/CMakeLists.txt
set(ENGINE_HEADERS
    ...
    {{directory}}/{{moduleName}}.h    # ← NEW
)

set(ENGINE_SOURCES
    ...
    {{directory}}/{{moduleName}}.cpp  # ← NEW
)
```

## Test Registration

Confirm the following will be updated:

- [ ] `#include "{{directory}}/{{moduleName}}.h"` added to `EngineTestsIncludes.h`
- [ ] `extern void {{moduleName}}Tests();` added to `EngineTestsExterns.h`
- [ ] `RUN_TEST({{moduleName}}Tests)` added to `EngineTests.cpp`
- [ ] Test body added to `EngineTests_Late.cpp`

## API Quality Review

- [ ] Error handling uses `DecodeResult` or `std::expected<T, EngineError>` — not exceptions
- [ ] Public API is minimal — expose only what callers need
- [ ] `stop_token` accepted for any long-running operation
- [ ] All types use `namespace ExplorerLens::Engine`
- [ ] No `__builtin_*` intrinsics (GCC-only; use `memcpy()` for MSVC)
- [ ] `std::min`/`std::max` parenthesized: `(std::min)(a, b)`
