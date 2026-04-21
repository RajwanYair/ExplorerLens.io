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

### Adding a New MCP Server

1. **Evaluate the need** — verify the agent workflow that requires the server.
   Current servers: `github` (GitHub API), `filesystem` (full workspace), `project-docs` (docs-only).
2. **Check for conflicts** — ensure the new server doesn't overlap with existing ones.
3. **Edit `.vscode/mcp.json`** — add the server entry following this template:
   ```json
   "server-name": {
     "command": "npx",
     "args": ["-y", "@scope/mcp-server-package", "${workspaceFolder}\\scope-dir"],
     "env": {
       "PATH": "${env:APPDATA}\\npm;${env:USERPROFILE}\\scoop\\shims;${env:PATH}"
     }
   }
   ```
4. **Security checks:**
   - Never embed tokens directly — use `"${input:token-name}"` with `"password": true` in inputs.
   - Never scope filesystem servers above `${workspaceFolder}`.
   - Never add corporate proxy URLs (`NO_PROXY`, `no_proxy`, proxy host:port).
5. **Test locally** — open VS Code, invoke the server via Copilot Chat, verify tools appear.
6. **Update 3 inventory files** in the same commit:
   - `.github/instructions/mcp-servers.instructions.md` (server inventory table)
   - `.github/copilot-instructions.md` (MCP Servers section)
   - `.github/standards/ai-tooling-capabilities.md` (MCP inventory)

### Evaluating Git MCP Servers (§8.8.5 Backlog)

If agents need git history access (blame, log, diff), evaluate:
- `@anthropic/mcp-server-git` — Anthropic's official git MCP server
- GitKraken MCP (if installed) — `mcp_gitkraken_*` tools already available
- Before adding, verify: Does the agent need git history, or can it use `run_in_terminal` with `git`?

### Verifying GitHub PAT Scopes

The `github` MCP server requires a PAT with these minimum scopes:
- `repo` — full repository access (read/write)
- `workflow` — GitHub Actions (trigger, read status)
- `read:packages` — package registry access

Verify with: `gh auth status` (check "Token scopes:" line).

---

## Node.js 24 Migration Playbook

GitHub Actions is migrating from Node.js 20 to Node.js 24. Actions still using Node 16 or 20
will emit deprecation warnings and eventually fail.

### How to Opt In

Add this env variable to every workflow's top-level `env:` block:

```yaml
env:
  FORCE_JAVASCRIPT_ACTIONS_TO_NODE24: true
```

### Audit Procedure

1. Search all workflows for actions that may use old Node.js:
   ```powershell
   Get-ChildItem .github/workflows/*.yml | ForEach-Object {
       Select-String -Path $_.FullName -Pattern 'uses:\s+\S+@' | ForEach-Object { $_.Line.Trim() }
   } | Sort-Object -Unique
   ```
2. Check each action's `action.yml` for `runs.using:` — should be `node20` or `node24`.
3. Upgrade any action still at `@v3` to `@v4` (which targets Node 20+).
4. Set `FORCE_JAVASCRIPT_ACTIONS_TO_NODE24: true` in every workflow.

### ExplorerLens Status

All 22 workflows now set `FORCE_JAVASCRIPT_ACTIONS_TO_NODE24: true` (verified S101).
All actions are at `@v4` or later. No Node 16 actions remain.

### Troubleshooting

| Symptom | Fix |
|---------|-----|
| `Node.js 16 actions are deprecated` warning | Upgrade action to `@v4` |
| `This request was rejected` on `upload-artifact` | Upgrade from `@v3` to `@v4` (breaking API change) |
| Custom action fails with Node 24 | Check `engines.node` in action's `package.json`; file issue with action author |

---

## AI Tooling Inventory (Current State)

| Asset Type | Count | Location |
|------------|-------|----------|
| Instructions (scoped) | 15 | `.github/instructions/` |
| Agents | 5 | `.github/agents/` |
| Prompts | 14 | `.github/prompts/` |
| Skills | 7 | `.github/skills/` |
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
