# ExplorerLens — Workflows and MCP Skill

## Purpose

Use this skill when editing or reviewing GitHub workflow files, repository AI assets
(instructions, agents, prompts, skills), or MCP server configuration. Read it fully
before touching any `.github/` or `.vscode/mcp.json` file.

---

## When to Use This Skill

- Updating `.github/workflows/*.yml` files or their companion docs
- Auditing workflow inventory, trigger intent, and job ownership
- Configuring or documenting MCP servers in `.vscode/mcp.json`
- Adding/editing instructions, agents, prompts, or skills
- Keeping `.github/standards/ai-tooling-capabilities.md` synchronized

---

## Step-by-Step: Adding a New Workflow

1. Check if a workflow for the same purpose exists: `ls .github/workflows/`
2. Name the file descriptively: `<trigger>-<purpose>.yml` (e.g., `push-build-engine.yml`)
3. Use `windows-latest` runner for C++ builds — **never pin toolset** in `ilammy/msvc-dev-cmd@v1`
4. Use `actions/cache` with `sccache` key for build caching
5. Add the workflow to `.github/instructions/cicd.instructions.md` inventory table
6. Add the workflow to `.github/standards/ai-tooling-capabilities.md`
7. Test with `act` locally or push to a feature branch first

```yaml
# Minimal correct workflow skeleton
name: Build Engine
on:
  push:
    branches: [main]
  pull_request:
    branches: [main]
jobs:
  build:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v4
      - uses: ilammy/msvc-dev-cmd@v1  # NO toolset pin — auto-upgrades with runner
      - run: .\build-scripts\Build-MSVC.ps1 -Clean -Test
        shell: pwsh
```

---

## Step-by-Step: Adding/Editing AI Assets

### Instructions (`*.instructions.md`)
- File: `.github/instructions/<scope>.instructions.md`
- Must have `applyTo:` frontmatter scoping it to relevant file patterns
- Update `.github/standards/ai-tooling-capabilities.md` in the same commit

### Agents (`*.agent.md`)
- File: `.github/agents/<name>.agent.md`
- Must include: description, tools list, behavioral constraints, example invocation
- Agent names must be unique; avoid generic names like "Assistant" or "Helper"

### Prompts (`*.prompt.md`)
- File: `.github/prompts/<name>.prompt.md`
- Must include: purpose, inputs, expected output, constraints
- Prompts must be task-specific — do not create prompts for open-ended exploration

### Skills (`*/SKILL.md`)
- Directory: `.github/skills/<skill-name>/SKILL.md`
- Minimum length: 100 lines with concrete step-by-step procedures
- Must include: purpose, when to use, step-by-step, constraints, validation checklist

---

## Step-by-Step: MCP Server Changes

1. Edit `.vscode/mcp.json` — only add servers that are actually installed and tested
2. **Never add** `NO_PROXY`, `no_proxy`, or corporate proxy URLs to MCP env blocks
3. After adding a server, document it in `.github/copilot-instructions.md` (MCP Servers section)
4. Update `.github/standards/ai-tooling-capabilities.md` MCP inventory
5. Verify the server works: open VS Code → Copilot agent → invoke a tool from the new server

---

## AI Tooling Inventory (Current State)

| Asset Type | Count | Location |
|------------|-------|----------|
| Instructions (scoped) | 13 | `.github/instructions/` |
| Agents | 4 | `.github/agents/` |
| Prompts | 11 | `.github/prompts/` |
| Skills | 6 | `.github/skills/` |
| MCP servers | 3 | `.vscode/mcp.json` |

---

## Required Constraints

1. **Never describe** a workflow that does not exist in `.github/workflows/`.
2. **Never document** MCP servers absent from `.vscode/mcp.json`.
3. **Always update** `ai-tooling-capabilities.md` in the same commit as AI asset changes.
4. **Never pin** `toolset:` in `ilammy/msvc-dev-cmd@v1` — breaks GitHub-hosted runners.
5. **Corporate artifacts** (`intel.com`, `NO_PROXY`, port 928) must never appear in tracked files.
6. Instructions must have correct `applyTo:` frontmatter — test that scoping works as intended.

---

## Canonical File Paths

| Purpose | Path |
|---------|------|
| Workflow rules | `.github/instructions/cicd.instructions.md` |
| AI capability inventory | `.github/standards/ai-tooling-capabilities.md` |
| Main repository rules | `.github/copilot-instructions.md` |
| Agent definitions | `.github/agents/*.agent.md` |
| Prompt templates | `.github/prompts/*.prompt.md` |
| Skills | `.github/skills/*/SKILL.md` |
| MCP config | `.vscode/mcp.json` |

---

## Validation Checklist

- [ ] Workflow names in markdown exactly match workflow filenames in `.github/workflows/`
- [ ] MCP server names and scopes match `.vscode/mcp.json`
- [ ] All instructions have `applyTo:` frontmatter
- [ ] All skills are ≥ 100 lines with step-by-step procedures
- [ ] `ai-tooling-capabilities.md` counts match actual file counts
- [ ] No stale references to retired conventions (`.github/AGENTS.md`, `CODEOWNERS` old casing)
- [ ] No corporate proxy URLs in any `.vscode/` or `.github/` file
