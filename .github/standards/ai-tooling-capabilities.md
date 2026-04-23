# AI Tooling Capabilities & Integration Matrix

**Last Updated:** 23 April 2026  
**Version:** v38.8.0 "Betelgeuse" (post S191–S198)  
**Scope:** Repository-local AI instructions, prompts, agents, skills, MCP server usage, and workflow automation guidance

---

## Purpose

This file is the canonical reference for how ExplorerLens uses modern GitHub Copilot and VS Code agent features.

It documents:

- Repository instruction files and when they apply
- Custom agent definitions and delegation boundaries
- Prompt templates and reusable task entry points
- Repository-local skills (`SKILL.md`) and their intended use
- MCP server inventory from the workspace configuration
- Workflow coverage across build, release, validation, and regression automation

When updating any AI-facing repository asset under `.github/`, keep this file in sync.

---

## Repository AI Asset Layout

| Asset Type | Location | Purpose |
|-----------|----------|---------|
| Copilot repository instructions | `.github/copilot-instructions.md` | Primary project rules, build constraints, release policy, architectural guidance |
| Scoped instruction files | `.github/instructions/*.instructions.md` | Narrow, file-pattern-aware rules for CI, tests, versions, file size, and workspace conventions |
| Custom agents | `.github/agents/*.agent.md` | Domain-specific delegated agents with project-specialized operating rules |
| Prompt templates | `.github/prompts/*.prompt.md` | Reusable prompts for review, project creation, quality fixes, and test generation |
| Repository-local skills | `.github/skills/*/SKILL.md` | Domain knowledge packs for common multi-step tasks |
| Workflow docs and standards | `.github/standards/*.md` | Long-lived operational references for tool versions, lessons learned, build rules, and AI capabilities |
| MCP server configuration | `.vscode/mcp.json` | Workspace-local Model Context Protocol servers and allowed filesystem scopes |

---

## Instruction Files

### Current Instruction Inventory (15 files)

| File | Primary Scope (`applyTo`) | Use When | Last Revised |
|------|--------------------------|----------|-------------|
| `workspace.instructions.md` | `**` | Cross-project workspace conventions (Python + C++ projects) | v38.0 |
| `cpp-coding.instructions.md` | `**/*.h, **/*.cpp` | C++20 coding standards, MSVC v145, naming, patterns | v38.0 |
| `build.instructions.md` | `**/CMakeLists.txt, **/build-scripts/**` | Build system, CMake, MSBuild, external libraries | v38.0 |
| `cicd.instructions.md` | `**/*.yml, **/*.yaml, .github/**` | GitHub Actions, CI/CD authoring, workflow patterns | v38.0 |
| `testing.instructions.md` | `**/tests/**, **/*_test.py, **/conftest.py` | Test authoring (pytest, custom TEST macros, Catch2) | v38.0 |
| `security.instructions.md` | `**/*.h, **/*.cpp, **/*.ps1, **/*.yml` | OWASP, secrets, supply-chain security, credential handling | v38.0 |
| `performance.instructions.md` | `**/Engine/**, **/benchmarks/**` | Benchmark scaffolding, profiling, regression gates | v38.0 |
| `decoder-authoring.instructions.md` | `**/Engine/Decoders/**` | Format decoder creation, ProbeHeader/DecodeAtSize pattern | v38.0 |
| `documentation.instructions.md` | `**/*.md, docs/**` | Documentation tiers, MkDocs, CHANGELOG, ADR authoring, SVG rules | v38.0 |
| `release.instructions.md` | `**/Bump-Version.ps1, **/CHANGELOG.md, **/VERSION` | Version bumps, release procedure, artifact validation | v38.0 |
| `pr-authoring.instructions.md` | `.github/**` | PR title format, conventional commits, review assignment | v38.0 |
| `version-bump.instructions.md` | `**` | 20-file version registry, idempotency guard, release checklist | v38.0 |
| `file-size-policy.instructions.md` | `**` | Git performance guardrails (500 KB split, 200 KB monitor) | v38.0 |
| `mcp-servers.instructions.md` | `.vscode/mcp.json` | MCP server configuration rules, addition checklist, security | v38.0 |
| `ai-agents.instructions.md` | `.github/agents/**` | Agent file structure, creation checklist, design rules | v38.0 |

### Authoring Rules

1. Repository-wide rules belong in `.github/copilot-instructions.md`.
2. Pattern-specific rules belong in `.github/instructions/*.instructions.md`.
3. Avoid duplicating the same rule in more than one file unless one file is the canonical source and the other is a short pointer.
4. Instructions that are operational and version-sensitive should also be reflected in `.github/standards/`.

---

## Agent Definitions

### Active Repository Agents

| Agent | File | Role | Last Revised |
|------|------|------|--------------|
| `ExplorerLens` | `.github/agents/explorerlens.agent.md` | Specialized native-code agent for MSVC/CMake/MSBuild/COM/GPU work | v38.0 |
| `Docs` | `.github/agents/docs.agent.md` | Documentation accuracy agent — checks docs reflect actual code | v38.0 |
| `Release` | `.github/agents/release.agent.md` | Release orchestration — version bumps, artifact validation, post-release checks | v38.0 |
| `TestCorpus` | `.github/agents/test-corpus.agent.md` | Test corpus management — real CC0 files, SSIM scoring, decoder validation | v38.0 |
| `CI-Ops` | `.github/agents/ci-ops.agent.md` | CI/CD operations — workflow authoring, action auditing, failure debugging | v38.0 |
| `Performance` | `.github/agents/performance.agent.md` | Benchmark analysis, regression gates, ETW traces, decode latency targets | v38.3 |
| `Corpus` | `.github/agents/corpus.agent.md` | Corpus ingest, CC0 file sourcing, SSIM drift detection, MANIFEST.json validation | v38.4 |
| `Explore` | *(built-in)* | Fast read-only codebase exploration and Q&A subagent | — |

### Agent Expectations

The ExplorerLens agent should:

- Treat `.github/copilot-instructions.md` as the top-level project contract
- Apply scoped `.instructions.md` files before changing matching files
- Prefer repository tasks and scripted build entry points over ad hoc shell commands
- Use the shared AI tooling docs in this file when deciding between instructions, prompts, skills, MCP, and workflows
- Keep zero-warning MSVC builds, version synchronization, and release discipline as non-negotiable constraints

### When to Add Another Agent

Add a new `.agent.md` file only when all of the following are true:

1. The task area has a stable boundary, such as packaging, docs publishing, or test-corpus generation.
2. The operating rules are materially different from the main ExplorerLens agent.
3. The additional agent reduces ambiguity rather than creating overlap.

---

## Prompt Templates

### Current Prompt Inventory

| Prompt | File | Purpose | Last Revised |
|-------|------|---------|-------------|
| Architecture review | `.github/prompts/architecture-review.prompt.md` | Architecture and design review | v38.0 |
| Benchmark analysis | `.github/prompts/benchmark-analysis.prompt.md` | Performance benchmark analysis | v38.0 |
| Code review | `.github/prompts/code-review.prompt.md` | Structured security, quality, and architecture review | v38.0 |
| Create project | `.github/prompts/create-project.prompt.md` | Project scaffolding guidance | v38.0 |
| Debug build failure | `.github/prompts/debug-build-failure.prompt.md` | Build failure diagnosis and fix | v38.0 |
| Decoder scaffold | `.github/prompts/decoder-scaffold.prompt.md` | New format decoder scaffolding (7-step playbook) | v38.0 |
| Fix quality | `.github/prompts/fix-quality.prompt.md` | Quality improvement and remediation workflow | v38.0 |
| PR description | `.github/prompts/pr-description.prompt.md` | Pull request description generation | v38.0 |
| Project specification | `.github/prompts/PROJECT_SPEC_PROMPT.md` | Repository and project setup conventions | v38.0 |
| Release prep | `.github/prompts/release-prep.prompt.md` | Release preparation checklist | v38.0 |
| Write tests | `.github/prompts/write-tests.prompt.md` | Test generation workflow | v38.0 |
| CI troubleshooting | `.github/prompts/ci-troubleshooting.prompt.md` | Workflow failure diagnosis with 10 known failure patterns | v38.0 |
| Workspace hygiene | `.github/prompts/workspace-hygiene.prompt.md` | Comprehensive workspace audit (versions, orphans, dead tests, link rot) | v38.0 |
| SVG diagram | `.github/prompts/svg-diagram.prompt.md` | Standardized SVG diagram generation with brand palette | v38.0 |
| Spec fetch | `.github/prompts/spec-fetch.prompt.md` | 5-step authoritative format specification fetch for decoder authoring | v38.4 |
| Roadmap guardian | `.github/prompts/roadmap-guardian.prompt.md` | 7-gate PR validation against roadmap phase plan, decision log, and corpus requirements | v38.4 |

### Prompt Authoring Rules

1. Prompts should reference the canonical instruction or standards file rather than restating long rule sets.
2. Prompts should describe inputs, expected outputs, and stop conditions.
3. Prompts should avoid stale file-path examples like `.github/AGENTS.md` when the repo uses `.github/agents/*.agent.md`.

---

## Repository-Local Skills

### Current Skill Inventory

| Skill | File | Use For | Last Revised |
|------|------|---------|-------------|
| ExplorerLens build and release | `.github/skills/explorerlens-build-and-release/SKILL.md` | Build, clean build, test, version bump, release preparation | v38.0 |
| ExplorerLens workflows and MCP | `.github/skills/explorerlens-workflows-and-mcp/SKILL.md` | Workflow edits, automation review, MCP alignment, repo AI configuration | v38.0 |
| Decoder development | `.github/skills/decoder-development/SKILL.md` | New decoder authoring, format registration, test integration | v38.0 |
| Documentation | `.github/skills/documentation/SKILL.md` | Documentation authoring, link validation, standards compliance | v38.0 |
| Performance | `.github/skills/performance/SKILL.md` | Performance profiling, benchmark analysis, regression investigation | v38.0 |
| Test corpus | `.github/skills/test-corpus/SKILL.md` | Corpus management, CC0 file sourcing, SSIM scoring, MANIFEST.json | v38.0 |
| CI operations | `.github/skills/ci-ops/SKILL.md` | Workflow failure debugging, action version audit, permissions, concurrency | v38.0 |
### Skill Design Rules

1. Skills should be narrow enough to be reusable.
2. Skills should point to canonical repo files, scripts, and workflow names.
3. Skills should not restate generic language syntax or editor help.
4. Skills should call out required validation steps, not just authoring steps.

---

## MCP Server Inventory

The workspace currently defines MCP servers in `.vscode/mcp.json`.

| Server | Backing Package | Scope | Primary Use | Last Revised |
|--------|------------------|-------|-------------|-------------|
| `github` | `@modelcontextprotocol/server-github` | GitHub API via PAT input | Issues, PRs, repo metadata, automation triage | v38.4 |
| `filesystem` | `@modelcontextprotocol/server-filesystem` | `${workspaceFolder}` | Full workspace file inspection and read/write operations | v38.4 |
| `project-docs` | `@modelcontextprotocol/server-filesystem` | `${workspaceFolder}\.github`, `${workspaceFolder}\docs` | Documentation-only editing and review | v38.4 |

**Required PAT scopes:** `repo`, `workflow`, `read:packages`, `write:packages`, `actions:read`.
See `.github/instructions/mcp-servers.instructions.md` for the full PAT scope table and verification script.

### MCP Guidance

1. Keep MCP scope minimal and explicit.
2. Use a docs-scoped filesystem server when you want to hard-bound documentation edits.
3. Document new servers here whenever `.vscode/mcp.json` changes.
4. If a server depends on secrets, document the input contract and secret name, but never commit tokens.

---

## Workflow Capability Map

### Core Build and Validation Workflows

| Workflow | File | Purpose |
|---------|------|---------|
| Canonical CI | `.github/workflows/ci-matrix.yml` | Main push/PR build matrix for Engine and shell validation |
| Scheduled build | `.github/workflows/build.yml` | Manual/scheduled weekly verification build (not push/PR) |
| Reusable build | `.github/workflows/reusable-build.yml` | Reusable `workflow_call` Engine build+test pattern (DRY) |
| PR validation | `.github/workflows/pr-checks.yml` | PR title, size, changelog, and version consistency checks |
| Toolchain verification | `.github/workflows/toolchain-verify.yml` | Verifies MSVC, CMake, and Ninja availability |
| Code quality | `.github/workflows/code-quality.yml` | Static analysis and quality checks |
| CodeQL | `.github/workflows/codeql.yml` | Security scanning |
| Coverage | `.github/workflows/coverage.yml` | Coverage collection and reporting |
| Docs validation | `.github/workflows/docs-validation.yml` | mkdocs strict build + SVG well-formedness validation |

### Release and Distribution Workflows

| Workflow | File | Purpose |
|---------|------|---------|
| Release packaging | `.github/workflows/release.yml` | Build artifacts, package release assets, create draft release |
| Package publishing | `.github/workflows/publish-packages.yml` | Publish NuGet, npm, and container packages (Maven/RubyGems removed — R5) |
| Release drafting | `.github/workflows/release-drafter.yml` | Maintains release draft notes |

### Phase 4 Stub Workflows (gated — not yet active on push/PR)

| Workflow | File | Purpose |
|---------|------|---------|
| Sanitizer CI | `.github/workflows/sanitizer-ci.yml` | ASAN + UBSAN build + tests (Phase 4 gate; `workflow_dispatch` only) |
| Fuzz CI | `.github/workflows/fuzz-ci.yml` | libFuzzer targets for 13 P0/P1 decoders (Phase 4 gate; `workflow_dispatch` only) |

### Regression and Specialized Validation

| Workflow | File | Purpose |
|---------|------|---------|
| Binary size gate | `.github/workflows/binary-size.yml` | Size regression protection |
| Catch2 tests | `.github/workflows/catch2-tests.yml` | Catch2-specific validation surface |
| Corpus validation | `.github/workflows/corpus-validation.yml` | Test corpus/data validation |
| Performance gate | `.github/workflows/performance-regression-gate.yml` | Performance regression detection |
| Screenshot regression | `.github/workflows/screenshot-regression.yml` | Visual regression validation |

### Repository Operations

| Workflow | File | Purpose |
|---------|------|---------|
| Auto label | `.github/workflows/auto-label.yml` | Issue/PR labeling automation |
| Sync labels | `.github/workflows/sync-labels.yml` | Label catalog synchronization |
| Notify failure | `.github/workflows/notify-failure.yml` | Failure notification routing |
| Pages | `.github/workflows/pages.yml` | Documentation/site publishing |
| Stale | `.github/workflows/stale.yml` | Issue/PR stale lifecycle management |
| Pin actions | `.github/workflows/pin-actions.yml` | SHA-pin all workflow action references (supply chain, D40); creates PR via `peter-evans/create-pull-request` |
| Devcontainer test | `.github/workflows/devcontainer-test.yml` | Clone-to-build gate for `.devcontainer/**`; runs `post-create-validate.ps1` (§11.2, S178) |

### Workflow Authoring Rules

1. Windows jobs must not pin an unavailable MSVC toolset in GitHub-hosted CI.
2. JavaScript-based actions should follow the repo-standard Node 24 opt-in policy already present in workflows.
3. Workflow guidance belongs in `.github/instructions/cicd.instructions.md`; workflow inventory belongs here.
4. When adding a workflow, add both the YAML and a one-line purpose entry in this file.

### Workflow Compliance Status (Audited v38.7.0 — post S188)

| Check | Status |
|-------|--------|
| All actions at v4+ (or latest stable) | 26/26 ✅ |
| `permissions:` block present | 26/26 ✅ |
| `FORCE_JAVASCRIPT_ACTIONS_TO_NODE24: true` | 26/26 ✅ |
| `concurrency:` block present | 21/26 (5 exempt — see below) |
| `workflow_dispatch` trigger | 23/26 (3 exempt — see below) |

**Concurrency exemptions:** `notify-failure.yml` (workflow_run), `release.yml` (tag-push), `reusable-build.yml` (callable), `stale.yml` (scheduled), `sync-labels.yml` (utility).

**workflow_dispatch exemptions:** `notify-failure.yml` (auto-trigger only), `reusable-build.yml` (callable only), `stale.yml` (scheduled only).

---

## Standards Reference Documents

| Document | File | Purpose | Last Revised |
|----------|------|---------|-------------|
| Lessons learned | `.github/standards/lessons-learned.md` | Engineering retrospective, sprint incident log | v38.4 |
| Tool versions | `.github/standards/tool-versions.md` | Canonical toolchain version table | v38.4 |
| Build method | `.github/standards/build-method.md` | Build entry-point reference | v38.4 |
| MCP server evaluation | `.github/standards/mcp-server-evaluation.md` | SQLite + fetch MCP adoption evaluation, OWASP risk, config snippets (S164) | v38.5 |

---

## Architecture Decision Records (ADRs)

ADRs live in `docs/adr/` and are indexed in `docs/adr/README.md`.

### Active ADRs (v38.6.0)

| ADR | File | Status | Topic | Version |
|-----|------|--------|-------|---------|
| ADR-001 | `ADR-001-wasm-sandbox.md` | Accepted | WASM sandbox for plugin isolation | — |
| ADR-002 | `ADR-002-npu-acceleration.md` | Accepted | NPU acceleration strategy | — |
| ADR-003 | `ADR-003-clip-embeddings.md` | Accepted | CLIP embeddings for visual search | — |
| ADR-008 | `ADR-008-cpp23-adoption.md` | Accepted | C++23 adoption timeline | — |
| ADR-009 | `ADR-009-mupdf-agpl-license-strategy.md` | Accepted | MuPDF AGPL-3.0 license strategy | v36.2 |
| ADR-010 | `ADR-010-catch2-test-migration.md` | Accepted | Catch2 as primary test framework | v36.2+ |
| ADR-011 | `ADR-011-streaming-decoder-interface.md` | Accepted | IStreamingDecoder probe-then-decode interface (D38, §7.4) | v38.4 |
| ADR-012 | `ADR-012-engine-directory-consolidation.md` | Accepted | Engine directory consolidation 16→7 (D32, §7.2) | v38.4 |
| ADR-013 | `ADR-013-sqlite-l2-cache.md` | Accepted | SQLite WAL for L2 thumbnail cache index (D42, §7.5, §14) | v38.4 |
| ADR-014 | `ADR-014-safe-integer-overflow.md` | Accepted | Safe integer arithmetic for decode dimensions (§15.1 P0, CWE-190, OWASP A4/A8) | v38.6 |

---

## Maintenance Checklist

Update this file when any of the following change:

- A new `.instructions.md` file is added
- A new `.agent.md` file is added
- A prompt template is added, removed, or renamed
- A new skill directory with `SKILL.md` is added
- `.vscode/mcp.json` changes server inventory or scope
- A new workflow is added or an existing one materially changes role

Keep this file factual, inventory-driven, and path-accurate.
