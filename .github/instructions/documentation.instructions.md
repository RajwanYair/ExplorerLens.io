---
applyTo: "**/*.md,docs/**"
---

# Documentation Rules — ExplorerLens

## Core Principle: Docs Follow Code

**Never document a feature before its `.cpp` implementation exists.**
Documentation must lag code — document what works, not what is planned.

## Tier Placement (Required)

| Tier | Files | Audience | Rule |
|------|-------|----------|------|
| 1 — User | `README.md`, `docs/USER_GUIDE.md`, `CHANGELOG.md` | End users | Validated features ONLY |
| 2 — Developer | `docs/development/`, `.github/standards/` | Contributors | Accurate build/test instructions |
| 3 — Architecture | `ROADMAP.md`, `docs/architecture/` | Deep contributors | Label aspirational sections clearly |
| 4 — Historical | `CHANGELOG-archive.md`, `docs/archive/` | Reference | Rarely changed |

## File Naming

- All docs: `UPPER_SNAKE_CASE.md`
- Exceptions (keep): `README.md`, `CHANGELOG.md`, `ROADMAP.md`, `LICENSE`
- Tool configs (keep lowercase): `mkdocs.yml`, `.markdownlint.json`
- GitHub community files MUST be uppercase: `CONTRIBUTING.md`, `SECURITY.md`, `CODEOWNERS`

## README.md Rules

1. Features table: only list formats **validated** against real test corpus files
2. Badge test count: must match actual test count in `EngineTests.cpp` `RUN_TEST()` calls
3. Version badge: must match `VERSION` file
4. No "coming soon", "planned", or "TODO" in Tier 1 docs
5. Installation section must be tested on a clean Windows 10/11 VM

## CHANGELOG.md Rules

1. New entries go at the **top** in `## [Unreleased]` or a new `## [X.Y.Z]` header
2. Use Keep-a-Changelog sections: `### Added`, `### Changed`, `### Fixed`, `### Removed`
3. When file exceeds ~100 KB → move oldest version block to `CHANGELOG-archive.md`
4. Never edit archived entries in `CHANGELOG-archive.md`

## ROADMAP.md Rules

1. Single ROADMAP.md — this is the ONLY strategic planning document
2. Old roadmaps (ROADMAP_V30, V34, V35) live in `docs/archive/` — never edit them
3. Check off items with `[x]` when completed; phases move from "planned" to "completed"
4. Decision log (§17) is append-only — never modify existing decisions

## mkdocs.yml Rules

1. Every nav entry must point to an **existing** file — no phantom links
2. Run `mkdocs build --strict` (or note it should be run) before adding nav entries
3. `docs_dir: docs` — all docs live under `docs/`

## Anti-Patterns (Never Do)

- ❌ Create docs for features with only header stubs (no `.cpp`)
- ❌ Claim "200+ validated formats" — only validated-against-corpus formats count
- ❌ Create multiple roadmap files for the same project
- ❌ Use inline code for file names — use markdown links: [ROADMAP.md](ROADMAP.md)
- ❌ Leave aspirational content in Tier 1 (user-facing) docs

---

## Architecture Decision Records (ADR)

ADRs live in `docs/adr/` using a numbered naming scheme: `NNNN-short-title.md`.

### ADR Template

```markdown
# NNNN — Title

**Status:** Proposed | Accepted | Deprecated | Superseded by [XXXX](XXXX-*.md)
**Date:** YYYY-MM-DD
**Deciders:** [names or roles]

## Context
What is the issue motivating this decision?

## Decision
What is the change that we're proposing?

## Consequences
What becomes easier or harder because of this change?
```

### ADR Rules

1. **Immutable once accepted** — never edit the Context/Decision of an accepted ADR; create a new one that supersedes it.
2. **Number sequentially** — use `docs/adr/` directory listing to find the next number.
3. **Link from ROADMAP.md** — every major architectural decision should be traceable from the roadmap.
4. **Keep short** — target 100–300 words. ADRs are decisions, not design docs.

---

## MkDocs Configuration

### Site Structure

```yaml
# mkdocs.yml — required nav structure
nav:
  - Home: index.md
  - Quick Start: QUICK_START.md
  - User Guide: USER_GUIDE.md
  - Formats: formats/
  - Architecture: architecture/
  - Development: development/
  - Testing: testing/
  - ADRs: adr/
  - Release Process: RELEASE_PROCESS.md
  - Troubleshooting: TROUBLESHOOTING.md
```

### Rules

1. **`--strict` mode** — `mkdocs build --strict` must pass before merging any docs change.
2. **No orphan pages** — every `.md` file under `docs/` must appear in `nav:` or be explicitly excluded.
3. **Relative links only** — never use absolute URLs for internal docs links.
4. **Material theme** — use `material` theme with ExplorerLens brand palette.
5. **Code blocks require language hints** — use ` ```cpp `, ` ```yaml `, etc., never bare ` ``` `.

---

## Cross-Reference Conventions

### Internal Links

```markdown
<!-- ✅ Relative paths with anchors -->
See [Build Instructions](development/BUILD.md#prerequisites) for details.
See [ADR-0003](adr/0003-cache-eviction.md) for the design rationale.

<!-- ❌ Never use absolute GitHub URLs for repo-internal links -->
See [Build](https://github.com/RajwanYair/ExplorerLens.io/blob/main/docs/BUILD.md)
```

### Version References

```markdown
<!-- ✅ Dynamic version badge (auto-updated by Bump-Version.ps1) -->
**Version:** 36.6.0 "Antares"

<!-- ❌ Never hardcode a version without being in the Bump-Version registry -->
Requires ExplorerLens v35.0.0 or later.
```

### Section Cross-References in ROADMAP

```markdown
<!-- ✅ Use §N.N notation for ROADMAP internal refs -->
This implements the cache policy described in §6.3.

<!-- ❌ Don't use page numbers or line numbers -->
See line 1234 of ROADMAP.md
```

---

## Markdown Style Guide

### Headings

- Use ATX-style (`#`) headings, not Setext (`===`/`---`).
- One `# H1` per file (the title). Everything else is `##` or deeper.
- Leave one blank line before and after every heading.
- No trailing punctuation in headings.

### Lists

- Use `-` for unordered lists (not `*` or `+`).
- Indent nested lists by 2 spaces (for compatibility with markdownlint).
- Use `1.` for all ordered list items (auto-numbering).

### Tables

- Align columns with pipes for readability.
- Keep tables under 120 characters wide — split wide tables or use description lists.
- Header row is required.

### Code Blocks

- Always specify a language identifier.
- Use `powershell` (not `ps1` or `shell`) for PowerShell blocks.
- Use `cpp` (not `c++`) for C++ blocks.
- Fence with triple backticks, never indentation-based code blocks.

### Line Length

- Target 120 characters max (enforced by `.markdownlint.json`).
- Exception: URLs, table rows, and code blocks may exceed the limit.

---

## SVG Diagram Standards

### File Location

All architecture/workflow diagrams live in `docs/assets/` as SVG files.

### Rules

1. **SVG, not PNG** — vector diagrams render crisply at all zoom levels.
2. **Text in SVGs** — use `font-family: 'Segoe UI', system-ui, sans-serif` for consistency.
3. **Version chips** — `Bump-Version.ps1` patches `social-preview.svg` and `architecture-build.svg` automatically.
4. **Color palette** — use the ExplorerLens brand colors:
   - Primary: `#2563EB` (blue)
   - Accent: `#F59E0B` (amber)
   - Success: `#10B981` (green)
   - Error: `#EF4444` (red)
   - Neutral: `#6B7280` (gray)
5. **Accessibility** — diagrams must not rely solely on color to convey information. Add labels or patterns.
6. **Size** — keep SVGs under 50 KB; optimize with SVGO if needed.

### SVG Creation Workflow

When creating a new architecture or workflow SVG:

1. **Use the ExplorerLens SVG template** — start from an existing `docs/assets/*.svg` file for consistent styling.
2. **Viewport** — set `viewBox="0 0 1200 800"` (landscape) or `viewBox="0 0 800 1200"` (portrait). Never use fixed `width`/`height` in pixels.
3. **Font sizing** — titles: 24px bold, section headers: 18px bold, body text: 14px regular, badges/chips: 12px.
4. **Rounded rectangles** — use `rx="8"` for container boxes, `rx="12"` for badges/chips.
5. **Arrow style** — use `stroke-width="2"` with `marker-end` arrowheads, color `#6B7280` for flow lines.
6. **Background** — transparent or `#FFFFFF`; never use dark backgrounds (dark mode is handled by GitHub/MkDocs CSS).
7. **Validation** — run `docs-validation.yml` SVG job; it checks for `<script>` tags, non-UTF8 encoding, and size limits.
8. **Naming** — lowercase-kebab-case: `gpu-pipeline.svg`, `plugin-lifecycle.svg`.

### SVG Version Patching

Two SVGs are patched by `Bump-Version.ps1`:

| SVG | Patched Elements |
|-----|-----------------|
| `social-preview.svg` | Version chip, codename label, build/test stats |
| `architecture-build.svg` | MSI filename chip (`ExplorerLens-X.Y.Z-x64.msi`), version label |

Other SVGs are **not** auto-patched — if they contain version strings, add them to the
`Bump-Version.ps1` registry (see `version-bump.instructions.md`).

---

## MkDocs Validation & CI

### Local Validation

```powershell
# Install MkDocs + Material theme
pip install mkdocs-material

# Build with strict mode — fails on broken links, missing nav entries
mkdocs build --strict

# Preview locally
mkdocs serve
```

### CI Integration

The `docs-validation.yml` workflow runs on every PR that touches `docs/**` or `**/*.md`:

1. **Link validation** — `mkdocs build --strict` catches broken internal links.
2. **SVG validation** — checks for `<script>` injection, file size limits, UTF-8 encoding.
3. **Markdownlint** — enforces `.markdownlint.json` rules (heading style, list indent, line length).

### Nav Synchronization Rules

1. **Every new `docs/*.md` file** must be added to `mkdocs.yml` `nav:` within the same PR.
2. **Deleted files** must be removed from `nav:` — orphan nav entries break `--strict` mode.
3. **Subdirectory index files** — use `index.md` inside each `docs/` subdirectory for section landing pages.
4. **nav order** — follow the tier hierarchy: user-facing first, developer second, architecture third.

---

## Documentation Review Checklist

Before merging any documentation PR:

- [ ] All internal links resolve (no 404s in `mkdocs build --strict`)
- [ ] Version references match current `VERSION` file
- [ ] No aspirational content in Tier 1 docs
- [ ] No corporate artifacts (Intel, proxy URLs, port 928)
- [ ] Code examples compile/run as shown
- [ ] Tables are properly formatted and under 120 chars wide
- [ ] New files are added to `mkdocs.yml` nav
