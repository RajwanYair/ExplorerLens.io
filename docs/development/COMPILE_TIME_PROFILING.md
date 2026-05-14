# Compile-Time Profiling Guide

> MSVC `/d1reportTime` for identifying slow-to-compile headers and template instantiations.

---

## Why Profile Compile Times?

ExplorerLens has 500+ headers across 25 decoders. Compile time directly impacts developer
productivity. A single bloated header included transitively across 50 TUs can add minutes
to a clean build.

| Build Type | Target | Alert |
|-----------|--------|-------|
| Full rebuild (Release) | < 120 s | > 180 s → investigate |
| Incremental (1 file) | < 15 s | > 30 s → investigate |
| PCH generation | < 10 s | > 15 s → consider splitting |

---

## Quick Start: Profile a Single File

```powershell
# 1. Source MSVC environment
& "$env:VSINSTALLDIR\VC\Auxiliary\Build\vcvars64.bat"

# 2. Compile one TU with timing report
cl.exe /c /EHsc /std:c++20 /O2 /d1reportTime Engine/Core/DecodePipeline.cpp 2>&1 |
    Tee-Object -FilePath build-logs/d1-DecodePipeline.txt

# 3. Sort by time (slowest first)
Select-String "time\(" build-logs/d1-DecodePipeline.txt |
    Sort-Object { [double]($_ -replace '.*time\(([0-9.]+).*','$1') } -Descending |
    Select-Object -First 20
```

### Reading the Output

```
time(source): 0.00523s    Engine/Core/DecodePipeline.h
time(source): 0.00312s    Engine/Decoders/JpegDecoder.h
time(source): 0.12500s    <windows.h>           ← slow system header
time(code-gen): 0.00245s  Engine::DecodePipeline::Process
```

- `time(source)` — time parsing/preprocessing the header
- `time(code-gen)` — time generating code for a function/template
- Headers over 100 ms are candidates for PCH or forward declarations

---

## Profile the Entire Engine Build

```powershell
# Inject /d1reportTime via CMAKE_CXX_FLAGS
cmake --preset default-release -DCMAKE_CXX_FLAGS="/d1reportTime"
cmake --build build --config Release 2>&1 |
    Tee-Object -FilePath build-logs/compile-time-report.txt

# Extract the 30 slowest includes across all TUs
Select-String "time\(source\)" build-logs/compile-time-report.txt |
    ForEach-Object {
        if ($_ -match 'time\(source\):\s+([\d.]+)s\s+(.+)') {
            [PSCustomObject]@{ Seconds = [double]$Matches[1]; Header = $Matches[2].Trim() }
        }
    } |
    Sort-Object Seconds -Descending |
    Select-Object -First 30 |
    Format-Table -AutoSize
```

---

## Common Patterns and Fixes

### 1. Heavy System Headers in User Headers

**Problem:** `<windows.h>` or `<d3d11.h>` included in a frequently-used header.

**Fix:** Move to PCH (`Engine/pch.h`) or use forward declarations:

```cpp
// ❌ Bad: includes windows.h in every TU that uses this header
#include <windows.h>
class ThumbnailRenderer { HBITMAP Render(); };

// ✅ Good: forward-declare, include only in .cpp
struct HBITMAP__;
using HBITMAP = HBITMAP__*;
class ThumbnailRenderer { HBITMAP Render(); };
```

### 2. Template-Heavy Headers

**Problem:** Template instantiation > 200 ms per TU.

**Fix:** Use explicit instantiation in one `.cpp` file:

```cpp
// In MyTemplate.h — declaration only
template <typename T> class Decoder { T Process(); };

// In MyTemplate.cpp — explicit instantiation
template class Decoder<JpegConfig>;
template class Decoder<PngConfig>;
```

### 3. Unnecessary Transitive Includes

**Problem:** Header A includes Header B which includes Header C, but A only needs a forward declaration of C.

**Fix:** Use include-what-you-use analysis:

```powershell
# Find headers that could use forward declarations
Get-ChildItem Engine/**/*.h -Recurse | ForEach-Object {
    $includes = Select-String '#include\s+"' $_.FullName
    if ($includes.Count -gt 10) {
        Write-Host "$($includes.Count) includes: $($_.Name)"
    }
} | Sort-Object -Descending
```

### 4. Missing `#pragma once`

```powershell
# Verify all headers have #pragma once
Get-ChildItem Engine/**/*.h -Recurse | Where-Object {
    -not (Get-Content $_.FullName -First 1 | Select-String '#pragma once')
} | ForEach-Object { Write-Warning "Missing #pragma once: $_" }
```

---

## Integration with Build Scripts

`Build-MSVC.ps1` supports compile-time profiling via environment variable:

```powershell
# Enable profiling for a single build
$env:LENS_COMPILE_PROFILE = "1"
.\build-scripts\Build-MSVC.ps1

# Results written to build-logs/compile-time-report.txt
```

---

## Tracking Baselines Over Time

Store compile-time snapshots in `data/baselines/compile-times.json`:

```json
{
  "_comment": "Compile-time profiling baseline — ExplorerLens v39.9.0",
  "_updated": "2025-01-01T00:00:00Z",
  "full_rebuild_seconds": 95,
  "incremental_seconds": 12,
  "pch_generation_seconds": 4.2,
  "slowest_headers": [
    { "header": "Engine/pch.h", "time_ms": 3200, "note": "Expected — precompiled header" },
    { "header": "Engine/Decoders/AllDecoders.h", "time_ms": 850, "note": "Monitor" },
    { "header": "windows.h", "time_ms": 650, "note": "System — in PCH" }
  ],
  "tu_count": 48,
  "average_tu_seconds": 1.98
}
```

Update this file after each major version bump to track trends.

---

## Thresholds and Actions

| Metric | Threshold | Action |
|--------|-----------|--------|
| Single header | > 500 ms | Move to PCH or add forward declarations |
| Template instantiation | > 200 ms | Use explicit instantiation in `.cpp` |
| PCH generation | > 15 s | Split PCH into stable/volatile sections |
| Full rebuild | > 180 s | Profile and fix top-5 slowest headers |
| Include count per header | > 10 | Audit for unnecessary transitive includes |
| Incremental build (1 file) | > 30 s | Check PCH invalidation, transitive deps |

---

## See Also

- [Performance Skill](../../.github/skills/performance/SKILL.md) — runtime performance profiling
- [Build Quick Reference](BUILD_QUICK_REFERENCE.md) — build commands and presets
- [build.instructions.md](../../.github/instructions/build.instructions.md) — build system rules
