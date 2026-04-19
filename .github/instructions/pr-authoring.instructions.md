---
applyTo: ".github/**"
---

# PR Authoring Rules — ExplorerLens

## PR Title Format

```
<type>(<scope>): <imperative short description>

Types: feat | fix | refactor | perf | test | docs | chore | ci | build
Scope: engine | shell | manager | cli | tests | ci | ai | gpu | cache | decoder
```

Examples:
- `feat(decoder): add AVIF ProbeHeader + corpus validation`
- `fix(cache): correct LRU eviction when budget exceeded`
- `perf(jpeg): use libjpeg-turbo SIMD path for 2× speedup`
- `docs(readme): update validated format count to 23`

## PR Body Structure

```markdown
## Summary
One paragraph: what changed and why.

## Changes
- `Engine/Decoders/AvifDecoder.cpp`: implements ProbeHeader + DecodeAtSize
- `Engine/CMakeLists.txt`: registers new decoder in ENGINE_SOURCES
- `data/corpus/images/avif/`: adds 3 CC0 test files

## Testing
- [ ] `.\build-scripts\Build-MSVC.ps1 -Test` passes (0 errors, 0 warnings)
- [ ] New tests pass against real corpus files
- [ ] No P95 regression on benchmark suite

## Checklist
- [ ] Zero warnings build with MSVC v145
- [ ] New headers registered in ENGINE_HEADERS
- [ ] New sources registered in ENGINE_SOURCES
- [ ] No corporate artifacts (intel.com, proxy URLs, port 928)
```

## Review Checklist (For Reviewers)

- [ ] Does new code have corresponding tests?
- [ ] Are new decoders tested against real corpus files?
- [ ] Does the PR description accurately describe what changed?
- [ ] Are all new types collision-checked (`grep_search` across `Engine/**/*.h`)?
- [ ] Are all LENSTYPE enum values UPPER_CASE?
- [ ] Are `std::min`/`std::max` parenthesized: `(std::min)(a, b)`?
- [ ] Are Windows macros guarded with `WIN32_LEAN_AND_MEAN` compatibility?

## Merging Policy

1. All CI checks must pass (green)
2. Build: 0 errors, 0 warnings
3. No `TODO`, `FIXME`, or `HACK` comments introduced
4. Reviewer approval required for `Engine/Core/` and `LENSShell/` changes
5. Squash-merge preferred for feature PRs; merge-commit for release PRs

## Security Review Required For

- Any change to `LENSShell/` (COM DLL loaded by explorer.exe)
- New external library integration
- File parsing / decoder logic (potential path traversal, buffer overflow)
- Registry operations in `LENSManager/`
- CI workflow changes (supply chain security)
