# AI Tooling Capabilities & Integration Matrix

**Last Updated:** 22 July 2025  
**Version:** v36.9.0 "Antares"  
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

### Current Instruction Inventory (13 files)

| File | Primary Scope (`applyTo`) | Use When |
|------|--------------------------|----------|
| `workspace.instructions.md` | `**` | Cross-project workspace conventions (Python + C++ projects) |
| `cpp-coding.instructions.md` | `**/*.h, **/*.cpp` | C++20 coding standards, MSVC v145, naming, patterns |
| `build.instructions.md` | `**/CMakeLists.txt, **/build-scripts/**` | Build system, CMake, MSBuild, external libraries |
| `cicd.instructions.md` | `**/*.yml, **/*.yaml, .github/**` | GitHub Actions, CI/CD authoring, workflow patterns |
| `testing.instructions.md` | `**/tests/**, **/*_test.py, **/conftest.py` | Test authoring (pytest, custom TEST macros, Catch2) |
| `security.instructions.md` | `**/*.h, **/*.cpp, **/*.ps1, **/*.yml` | OWASP, secrets, supply-chain security, credential handling |
| `performance.instructions.md` | `**/Engine/**, **/benchmarks/**` | Benchmark scaffolding, profiling, regression gates |
| `decoder-authoring.instructions.md` | `**/Engine/Decoders/**` | Format decoder creation, ProbeHeader/DecodeAtSize pattern |
| `documentation.instructions.md` | `**/*.md, docs/**` | Documentation tiers, MkDocs, CHANGELOG, ADR authoring |
| `release.instructions.md` | `**/Bump-Version.ps1, **/CHANGELOG.md, **/VERSION` | Version bumps, release procedure, artifact validation |
| `pr-authoring.instructions.md` | `.github/**` | PR title format, conventional commits, review assignment |
| `version-bump.instructions.md` | `**` | 20-file version registry, idempotency guard, release checklist |
| `file-size-policy.instructions.md` | `**` | Git performance guardrails (500 KB split, 200 KB monitor) |

### Authoring Rules

1. Repository-wide rules belong in `.github/copilot-instructions.md`.
2. Pattern-specific rules belong in `.github/instructions/*.instructions.md`.
3. Avoid duplicating the same rule in more than one file unless one file is the canonical source and the other is a short pointer.
4. Instructions that are operational and version-sensitive should also be reflected in `.github/standards/`.

---

## Agent Definitions

### Active Repository Agents

| Agent | File | Role |
|------|------|------|
| `ExplorerLens` | `.github/agents/explorerlens.agent.md` | Specialized native-code agent for MSVC/CMake/MSBuild/COM/GPU work |
| `Docs` | `.github/agents/docs.agent.md` | Documentation accuracy agent — checks docs reflect actual code |
| `Release` | `.github/agents/release.agent.md` | Release orchestration — version bumps, artifact validation, post-release checks |
| `TestCorpus` | `.github/agents/test-corpus.agent.md` | Test corpus management — real CC0 files, SSIM scoring, decoder validation |
| `CI-Ops` | `.github/agents/ci-ops.agent.md` | CI/CD operations — workflow authoring, action auditing, failure debugging |
| `Explore` | *(built-in)* | Fast read-only codebase exploration and Q&A subagent |

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

| Prompt | File | Purpose |
|-------|------|---------|
| Architecture review | `.github/prompts/architecture-review.prompt.md` | Architecture and design review |
| Benchmark analysis | `.github/prompts/benchmark-analysis.prompt.md` | Performance benchmark analysis |
| Code review | `.github/prompts/code-review.prompt.md` | Structured security, quality, and architecture review |
| Create project | `.github/prompts/create-project.prompt.md` | Project scaffolding guidance |
| Debug build failure | `.github/prompts/debug-build-failure.prompt.md` | Build failure diagnosis and fix |
| Decoder scaffold | `.github/prompts/decoder-scaffold.prompt.md` | New format decoder scaffolding (7-step playbook) |
| Fix quality | `.github/prompts/fix-quality.prompt.md` | Quality improvement and remediation workflow |
| PR description | `.github/prompts/pr-description.prompt.md` | Pull request description generation |
| Project specification | `.github/prompts/PROJECT_SPEC_PROMPT.md` | Repository and project setup conventions |
| Release prep | `.github/prompts/release-prep.prompt.md` | Release preparation checklist |
| Write tests | `.github/prompts/write-tests.prompt.md` | Test generation workflow |
| CI troubleshooting | `.github/prompts/ci-troubleshooting.prompt.md` | Workflow failure diagnosis with 10 known failure patterns |
| Workspace hygiene | `.github/prompts/workspace-hygiene.prompt.md` | Comprehensive workspace audit (versions, orphans, dead tests, link rot) |
| SVG diagram | `.github/prompts/svg-diagram.prompt.md` | Standardized SVG diagram generation with brand palette |

### Prompt Authoring Rules

1. Prompts should reference the canonical instruction or standards file rather than restating long rule sets.
2. Prompts should describe inputs, expected outputs, and stop conditions.
3. Prompts should avoid stale file-path examples like `.github/AGENTS.md` when the repo uses `.github/agents/*.agent.md`.

---

## Repository-Local Skills

### Current Skill Inventory

| Skill | File | Use For |
|------|------|---------|
| ExplorerLens build and release | `.github/skills/explorerlens-build-and-release/SKILL.md` | Build, clean build, test, version bump, release preparation |
| ExplorerLens workflows and MCP | `.github/skills/explorerlens-workflows-and-mcp/SKILL.md` | Workflow edits, automation review, MCP alignment, repo AI configuration |
| Decoder development | `.github/skills/decoder-development/SKILL.md` | New decoder authoring, format registration, test integration |
| Documentation | `.github/skills/documentation/SKILL.md` | Documentation authoring, link validation, standards compliance |
| Performance | `.github/skills/performance/SKILL.md` | Performance profiling, benchmark analysis, regression investigation |
| Test corpus | `.github/skills/test-corpus/SKILL.md` | Corpus management, CC0 file sourcing, SSIM scoring, MANIFEST.json |
| CI operations | `.github/skills/ci-ops/SKILL.md` | Workflow failure debugging, action version audit, permissions, concurrency |
### Skill Design Rules

1. Skills should be narrow enough to be reusable.
2. Skills should point to canonical repo files, scripts, and workflow names.
3. Skills should not restate generic language syntax or editor help.
4. Skills should call out required validation steps, not just authoring steps.

---

## MCP Server Inventory

The workspace currently defines MCP servers in `.vscode/mcp.json`.

| Server | Backing Package | Scope | Primary Use |
|--------|------------------|-------|-------------|
| `github` | `@modelcontextprotocol/server-github` | GitHub API via PAT input | Issues, PRs, repo metadata, automation triage |
| `filesystem` | `@modelcontextprotocol/server-filesystem` | `${workspaceFolder}` | Full workspace file inspection and read/write operations |
| `project-docs` | `@modelcontextprotocol/server-filesystem` | `${workspaceFolder}\\.github`, `${workspaceFolder}\\docs` | Documentation-only editing and review |

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

### Workflow Authoring Rules

1. Windows jobs must not pin an unavailable MSVC toolset in GitHub-hosted CI.
2. JavaScript-based actions should follow the repo-standard Node 24 opt-in policy already present in workflows.
3. Workflow guidance belongs in `.github/instructions/cicd.instructions.md`; workflow inventory belongs here.
4. When adding a workflow, add both the YAML and a one-line purpose entry in this file.

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
