# ExplorerLens — Development Learnings (Archived)

> **This file has been consolidated.** See the canonical references below.

| Topic | Canonical Location |
|-------|--------------------|
| Build rules, vcvars, CRT linkage, zero-warnings | [`.github/copilot-instructions.md`](copilot-instructions.md) |
| Sprint-by-sprint engineering learnings | [`docs/LEARNINGS.md`](../docs/LEARNINGS.md) |
| Build error patterns and known issues | [`.github/standards/build-troubleshooting.md`](standards/build-troubleshooting.md) |
| Common pitfalls quick reference table | [`docs/LEARNINGS.md §11`](../docs/LEARNINGS.md#11-common-pitfalls-quick-reference) |

Historical content from this file (37 sections, 1200+ lines) is preserved in git history
at the commit immediately before the 2026-05 consolidation pass.

---

## Gen-6 Workflow Patterns (v30.0.0–v31.1.0)

### Batch Sprint Pattern

Each version bump covers 10 sprints (8 headers) following this sequence:

1. **Create headers** via subagent (8 files, 90–120 lines each)
2. **Register in `Engine/CMakeLists.txt`** under `# Sprint XXXX-YYYY — Description`
3. **Inject tests** via `_add_tests.py` (72 tests = 9 per header × 8 headers)
4. **Commit** with `feat(gen6): Sprint XXXX-YYYY — Feature Name (vX.Y.Z Codename)`
5. **Bump version** with `Bump-Version.ps1 -TagAndPush`

### Test Injection Anchors

| Anchor | Purpose |
|--------|---------|
| `#include "Last/Header.h"` | Insert new includes after last header |
| `^//== ` (regex, first match) | Insert TEST blocks before first section |
| `    // Integration Test Framework + COM Tests` | Insert RUN_TEST calls before this line |

### Critical Gotchas

- **Always delete `_add_tests.py`** before `git add -A` — it leaks into commits otherwise
- **PowerShell `[IO.File]::ReadAllText` substring count** includes macro definitions / comments — use `^\s+RUN_TEST\(` regex for actual invocation count
- **Subagent `replace_string_in_file`** silently fails on very large replacements (500+ lines) — use Python script for EngineTests.cpp edits
- **OneDrive file sync** can cause stale reads — always verify with `Select-String` after injection
