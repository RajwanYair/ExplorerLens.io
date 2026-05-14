---
applyTo: ".github/**"
---

# PR Authoring Rules — ExplorerLens

## PR Title Format

```text
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

---

## Conventional Commits Specification

All commits on `main` follow [Conventional Commits v1.0.0](https://www.conventionalcommits.org/).

### Type Reference

| Type | When to Use | Bumps |
| ------ | ------------ | ------- |
| `feat` | New feature (decoder, API, capability) | Minor |
| `fix` | Bug fix | Patch |
| `perf` | Performance improvement (no functional change) | Patch |
| `refactor` | Code restructure (no functional change) | — |
| `test` | Add/fix tests only | — |
| `docs` | Documentation only | — |
| `ci` | CI/CD workflow changes | — |
| `build` | Build system, dependencies | — |
| `chore` | Housekeeping (version bumps, config) | Patch |
| `security` | Security hardening | Patch |

### Breaking Changes

```text
feat(decoder)!: rename ProbeHeader to Probe

BREAKING CHANGE: All decoder subclasses must rename ProbeHeader() to Probe().
```

- Use `!` after the scope for breaking changes.
- Include a `BREAKING CHANGE:` footer explaining migration steps.

### Complete Scope Reference

| Scope | Directory / Domain | Examples |
| ------- | -------------------- | ---------- |
| `engine` | `Engine/Core/`, `Engine/Engine.h` | Pipeline changes, core architecture |
| `shell` | `LENSShell/` | COM DLL, thumbnail provider, shell integration |
| `manager` | `LENSManager/` | GUI config utility, WTL dialogs |
| `decoder` | `Engine/Decoders/` | New/modified format decoders |
| `cache` | `Engine/Cache/` | Cache manager, eviction, budget |
| `gpu` | `Engine/GPU/` | GPU decode, vendor routing |
| `ai` | `Engine/AI/` | ML modules, scene understanding, IQA |
| `memory` | `Engine/Memory/` | Compactor, hot-mode, pressure controller |
| `pipeline` | `Engine/Pipeline/` | Fallback engine, zero-copy, parallel I/O |
| `plugin` | `Engine/Plugin/`, `Engine/PluginHost/` | Plugin SDK, trust chain, sandbox |
| `platform` | `Engine/Platform/` | PAL — Win32, macOS QL, Linux Nautilus |
| `media` | `Engine/Media/` | Video preview, timeline, scrubber |
| `tests` | `Engine/Tests/` | Test framework, test bodies, benchmarks |
| `cli` | `Engine/CLI/` | Command-line interface |
| `ci` | `.github/workflows/` | CI/CD pipelines, actions |
| `build` | `CMakeLists.txt`, `build-scripts/` | Build system, external libs |
| `packaging` | `packaging/` | MSI, Inno Setup, MSIX, portable ZIP |
| `sdk` | `SDK/` | Plugin SDK, C ABI headers |
| `docs` | `docs/`, `*.md` | Documentation, diagrams |
| `deps` | `external/`, `vcpkg.json` | Dependency updates |
| `security` | Cross-cutting | Security hardening, signing, audit |

Use the most specific scope. If a change spans multiple scopes, use the primary scope
and mention others in the commit body.

---

## PR Size Guidelines

| Size | Lines Changed | Expected Review Time | Policy |
| ------ | -------------- | --------------------- | -------- |
| XS | 1–10 | < 5 min | Self-merge OK for docs/config |
| S | 11–50 | 15 min | Standard review |
| M | 51–200 | 30 min | Standard review |
| L | 201–500 | 1 hour | Break into smaller PRs if possible |
| XL | 500+ | **Not allowed** | Must split into sequential PRs |

### Sprint Delivery PRs

Sprint deliveries (10 headers + 10 test bodies + CMakeLists edits) are exempt from
the XL rule because they are machine-generated batches that must be atomic.

---

## Label Assignment

Apply these labels to PRs based on content:

| Label | When |
| ------- | ------ |
| `build` | CMakeLists, build-scripts, external-libs changes |
| `ci/cd` | `.github/workflows/` changes |
| `decoder` | `Engine/Decoders/` changes |
| `performance` | Benchmark, profiling, or hot-path changes |
| `security` | Security-related changes (see Security Review section) |
| `documentation` | Docs-only changes |
| `shell-extension` | `LENSShell/` changes |
| `installer` | `packaging/` changes |

### Auto-Labeling

The `auto-label.yml` workflow automatically applies labels based on file paths.
Manual labels override automatic ones.

---

## Draft PRs

Use draft PRs (`gh pr create --draft`) when:

1. Work is in progress and you want early CI feedback.
2. You need to share context before requesting review.
3. Sprint work spans multiple sessions.

Convert to "Ready for Review" only when all CI checks pass.

---

## Branch Naming Convention

```text
<type>/<scope>-<short-description>

Examples:
  feat/decoder-avif-support
  fix/cache-eviction-bug
  ci/pin-action-versions
  docs/update-user-guide
  sprint/s51-s60-ai-tooling
```

### Rules

1. **Lowercase with hyphens** — no underscores, no camelCase.
2. **Include the type prefix** — matches conventional commit types.
3. **Keep under 50 characters** — branch names appear in many places.
4. **Delete after merge** — enable "Automatically delete head branches" in repo settings.

---

## Commit Message Best Practices

### Subject Line

- Imperative mood: "add", "fix", "remove" — not "added", "fixes", "removing".
- No period at the end.
- Max 72 characters.

### Body

- Separate from subject with a blank line.
- Wrap at 72 characters.
- Explain **what** and **why**, not **how** (the diff shows how).
- Reference issue numbers: `Fixes #123`, `Closes #456`, `Refs #789`.

### Multi-Sprint Commits

```text
chore: bump version to 36.7.0 (Antares)

Sprints S51-S60:
- S51: expand documentation.instructions.md
- S52: expand pr-authoring.instructions.md
- ...
- S60: version bump
```

---

## Corporate Artifact Scrub (Pre-Push)

Every PR must pass this scan before pushing:

```powershell
git diff --cached --name-only | ForEach-Object {
    Select-String -LiteralPath $_ -Pattern 'intel\.com|proxy\.|:928\b|:911\b' -AllMatches
} | ForEach-Object { Write-Warning "CORPORATE ARTIFACT: $($_.Filename):$($_.LineNumber): $($_.Line.Trim())" }
```

If any match is found, the PR must not be opened until the artifact is removed.

---

## Review Assignment Rules

### Ownership Areas

| Path Pattern | Required Reviewer | Reason |
| ------------- | ------------------- | -------- |
| `LENSShell/**` | Security-aware reviewer | COM DLL loaded by explorer.exe |
| `Engine/Core/**` | Engine lead | Core pipeline changes affect all decoders |
| `Engine/Decoders/**` | Decoder author + one other | Format-specific expertise required |
| `packaging/**` | Release engineer | Installer changes affect distribution |
| `.github/workflows/**` | CI owner | Supply chain security |
| `external/**` | Build engineer | Third-party dependency changes |
| `build-scripts/**` | Build engineer | Build system integrity |

### Review SLA

| PR Size | Initial Review | Final Approval |
| --------- | --------------- | ---------------- |
| XS / S | Same day | Same day |
| M | 1 business day | 2 business days |
| L | 2 business days | 3 business days |

### Self-Merge Exceptions

The following changes may be self-merged without reviewer approval:

1. **Docs-only** — `docs/**`, `*.md` (non-code) with no functional impact.
2. **CI config** — `.github/workflows/` tweaks that only adjust timeouts, labels, or caching.
3. **Version bumps** — automated `Bump-Version.ps1` commits (all 20 files).
4. **Sprint housekeeping** — instruction/prompt/skill file expansion (no Engine code).

All other changes require at least one reviewer approval before merge.
