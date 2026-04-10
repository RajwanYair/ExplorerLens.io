---
applyTo: "**"
---

# File Size Policy — Git Performance

## Thresholds

| Threshold | Action |
|-----------|--------|
| > 500 KB | **Must split** — impacts git diff, blame, and merge performance |
| 200–500 KB | **Monitor** — consider splitting on next major change |
| < 200 KB | OK |

## Tracked Large Files Allowlist

These are known large files that cannot be easily split:

| File | Size | Reason |
|------|------|--------|
| `LENSShell/Resources/zip.ico` | 264 KB | Binary icon resource |
| `LENSManager/RegManager.h` | 103 KB | Registry format table — monolithic by design |
| `LENSShell/LENSArchive.h` | 103 KB | Format dispatch table — monolithic by design |
| `Engine/Utils/ReleaseGate.h` | 94 KB | Gate evaluators V2–V33 — append-only |
| `Engine/CMakeLists.txt` | 83 KB | Single build definition — cannot split |
| `CHANGELOG-archive.md` | 113 KB | Historical archive — rarely changed |
| `Engine/Tests/EngineTestsExterns.h` | ~233 KB | Extern Runner declarations — append-only; split when >400 KB |

## Test File Split Convention

Test files are split to keep each under ~400 KB:

```
Engine/Tests/
├── EngineTestsIncludes.h      — Shared #include block (all Engine headers)
├── EngineTestsMacros.h        — TEST/ASSERT/RUN_TEST macros + MockDecoder
├── EngineTests.cpp            — Harness: globals, extern decls, main() + RUN_TEST()
├── EngineTests_Core.cpp       — TEST() bodies: decoder, registry, cache, GPU, gate
├── EngineTests_Features.cpp   — TEST() bodies: feature modules, SIMD, enterprise
├── EngineTests_Mid.cpp        — TEST() bodies: settings, memory, plugin, format
└── EngineTests_Late.cpp       — TEST() bodies: CLI, workflow, AI, platform PAL
```

**Rules:**
- New TEST() bodies go into `EngineTests_Late.cpp` (or the most relevant split file)
- New `extern void` declarations + `RUN_TEST()` calls go into `EngineTests.cpp`
- New `#include` directives go into `EngineTestsIncludes.h`
- When any split file exceeds 500 KB, split it again at a `//==` section boundary

## CHANGELOG Split Convention

- `CHANGELOG.md` — Recent releases (v28+), actively edited
- `CHANGELOG-archive.md` — Historical releases (v5.3.0–v27.7.0), rarely changed

When CHANGELOG.md exceeds ~100 KB, move the oldest major-version block to the archive.

## Checking File Sizes

```powershell
# Find all tracked files > 100 KB (excluding external/)
git ls-files | Where-Object { $_ -notmatch '^external/' } | ForEach-Object {
    $len = (Get-Item -LiteralPath $_ -ErrorAction SilentlyContinue).Length
    if ($len -gt 100KB) { "{0,8:N1} KB  {1}" -f ($len/1024), $_ }
} | Sort-Object -Descending
```
