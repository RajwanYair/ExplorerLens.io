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
