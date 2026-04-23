---
mode: agent
description: "Validate that a PR title, description, and changed files are consistent with the ExplorerLens ROADMAP phase plan. Flags out-of-phase work, missing corpus tests, missing ADRs, and version-bearing file drift."
---

# Roadmap Guardian Review

Validate this PR against the ExplorerLens strategic roadmap (ROADMAP.md) to catch out-of-phase work, missing deliverables, and policy violations before merge.

## Context Files

- `ROADMAP.md` — strategic plan with phase definitions and decision log D1–D48
- `.github/copilot-instructions.md` — engineering invariants
- `.github/instructions/cpp-coding.instructions.md`
- `.github/instructions/testing.instructions.md`
- `CHANGELOG.md` — recent changes
- `VERSION` — current version

## PR Under Review

**PR title:** {{pr_title}}
**PR description:** {{pr_description}}
**Changed files:** {{changed_files}}

## Validation Checklist

Run through each gate in order. Mark ✅ PASS or ❌ FAIL with a brief explanation.

---

### Gate 1: Phase Alignment

1. **Identify the current ROADMAP phase** (check `VERSION` and §18 of ROADMAP.md)
2. **Classify the PR work** into one of: Foundation (Phase 1), Performance (Phase 2), Breadth (Phase 3), Enterprise (Phase 4), AI/macOS (Phase 5), Linux/WASM (Phase 6)
3. **Check:** Does this PR belong to the **current or a prior** phase?
   - ✅ Current or prior phase = allowed
   - ❌ Future phase (e.g., a Phase 4 feature while we're in Phase 1) = **flag as out-of-phase**
   - Exception: stubs/interfaces are allowed one phase early if behind a feature flag

---

### Gate 2: Decision Log Compliance

1. Does this PR introduce any architecture decisions that should be in the Decision Log (§20)?
   - New library → D-number required, ADR in `docs/adr/`
   - New COM interface → D-number required
   - Deprecation of existing functionality → D-number required
2. Does this PR contradict any existing decision (D1–D48)?
   - E.g., adding cross-platform stubs when D44 says "Windows first"
   - E.g., using WinUI 3 in the Shell DLL (D45: never)
   - E.g., using MuPDF in new code when D30 says migrate to PDFium

---

### Gate 3: Corpus & Test Requirements

ROADMAP §10 requires: **No new decoder lands without a corpus test.**

1. If changed files include `Engine/Decoders/` → verify `data/corpus/MANIFEST.json` has ≥3 new entries for that format
2. If changed files include new test code → verify tests use Catch2 (not custom `TEST()` macros for new tests)
3. If SSIM baseline changes → verify `data/baselines/` updated

---

### Gate 4: Version-Bearing File Drift

If this PR changes any functionality that affects version strings:

1. All 20 version-bearing files (version-bump.instructions.md table) must be updated via `Bump-Version.ps1`
2. `CHANGELOG.md` must have an entry for the current sprint
3. If `Engine/CMakeLists.txt` changed → `ENGINE_HEADERS` / `ENGINE_SOURCES` must include all new files

---

### Gate 5: Zero-Warnings Build Compliance

1. No `/wdXXXX` warning suppressions added
2. No `__builtin_*` intrinsics (GCC-only)
3. No `std::min`/`std::max` without parentheses: use `(std::min)(a, b)`
4. No bare `using namespace std;` in headers
5. Enum values are UPPER_CASE

---

### Gate 6: Security & OWASP Compliance

Per `.github/instructions/security.instructions.md`:

1. All new `IStream` consumers call `IStream::Stat()` to validate size before decode
2. Dimension math uses `SafeInt<>` (ROADMAP §15.1)
3. No hardcoded paths, credentials, or proxy URLs
4. No shell-string construction from user input

---

### Gate 7: Documentation Accuracy

1. If a feature is added → `README.md` and `docs/USER_GUIDE.md` reflect it (or a TODO is filed)
2. If a CI workflow is added → `ROADMAP.md §12.1` workflow table updated
3. If a new tool/library is added → `external/LIBRARY_INVENTORY.md` updated

---

## Output Format

Produce a review summary:

```
## Roadmap Guardian Review

**Phase:** [current phase]
**PR phase:** [inferred phase of this PR]

| Gate | Status | Notes |
|------|--------|-------|
| Gate 1: Phase Alignment | ✅/❌ | ... |
| Gate 2: Decision Log | ✅/❌ | ... |
| Gate 3: Corpus & Tests | ✅/❌ | ... |
| Gate 4: Version Files | ✅/❌ | ... |
| Gate 5: Zero-Warnings | ✅/❌ | ... |
| Gate 6: Security | ✅/❌ | ... |
| Gate 7: Documentation | ✅/❌ | ... |

**Overall:** APPROVED / NEEDS CHANGES / BLOCKED (out-of-phase)

### Action items
- [ ] ...
```

If BLOCKED or NEEDS CHANGES, list concrete action items the PR author must address before merge.
