---
applyTo: ".github/agents/**"
---

# AI Agent Authoring Rules — ExplorerLens

## Overview

Agent files (`.github/agents/*.agent.md`) define specialized Copilot agents that
operate within the repository context. Each agent has a focused domain and access
to workspace tools, MCP servers, and instruction files.

## Current Agent Inventory

| Agent | File | Domain |
|-------|------|--------|
| ExplorerLens | `explorerlens.agent.md` | Engine development, sprints, build, version bumps |
| Docs | `docs.agent.md` | Documentation accuracy, link validation, content tiers |
| Release | `release.agent.md` | Version bumps, artifact validation, post-release checks |
| TestCorpus | `test-corpus.agent.md` | Corpus management, decoder validation, SSIM scoring |
| CI-Ops | `ci-ops.agent.md` | GitHub Actions, workflow debugging, action audits |

## Agent File Structure

Every agent file must follow this structure:

```markdown
---
description: One-line description shown in agent picker.
---

# Agent Name

## Identity
Who the agent is and its expertise domain.

## Context
Which instruction files, skills, and MCP servers the agent should load.

## Rules
Specific behavioral constraints for the agent.

## Workflow
Step-by-step procedure for the agent's primary task.
```

### Required Frontmatter

```yaml
---
description: Short description for VS Code agent picker (max 120 chars).
---
```

## Creating a New Agent

### Checklist

1. **Justify the need** — verify no existing agent covers the domain.
2. **Name it clearly** — lowercase kebab-case: `domain-name.agent.md`.
3. **Define scope** — list which files/directories the agent should operate on.
4. **Reference instructions** — tell the agent to read relevant `.instructions.md` files.
5. **Reference skills** — point to relevant `.github/skills/*/SKILL.md` files.
6. **Add context: section** — list the instruction files, skills, and MCP servers.
7. **Test locally** — invoke the agent via `@AgentName` in Copilot Chat.
8. **Update inventory** — after creating an agent, update:
   - This file (agent inventory table above)
   - `.github/copilot-instructions.md` (agents table in AI Tooling Surface)
   - `.github/standards/ai-tooling-capabilities.md` (agent inventory)

## Agent Design Rules

1. **One domain per agent** — agents should not overlap in scope.
   If two agents need the same data, share via skills or instruction files.
2. **Agents must not modify files outside their domain** — e.g., Docs agent
   should not edit `Engine/` source files.
3. **Agents must read instructions first** — every agent should include
   "Read `.github/copilot-instructions.md` before proceeding" in its rules.
4. **No hardcoded paths** — use `${workspaceFolder}` or relative paths.
5. **No secrets** — agents must never store or log credentials.
6. **Idempotent operations** — agent workflows should be safe to run repeatedly.

## Naming Conventions

| Element | Convention | Example |
|---------|-----------|---------|
| File name | `kebab-case.agent.md` | `ci-ops.agent.md` |
| Agent display name | PascalCase | `CI-Ops`, `TestCorpus` |
| Description | Imperative, < 120 chars | `Manages CI/CD workflows and debugs action failures` |

## Skill Integration

Agents should reference skills for complex multi-step workflows:

```markdown
## Context
- **Skills:** `.github/skills/ci-ops/SKILL.md`, `.github/skills/explorerlens-build-and-release/SKILL.md`
- **Instructions:** `.github/instructions/cicd.instructions.md`
- **MCP Servers:** `github` (for Actions API)
```

## Anti-Patterns

- Do not create agents that duplicate VS Code built-in functionality.
- Do not create "general purpose" agents — keep them focused.
- Do not reference external URLs in agent files — all context must be local.
- Do not create agents that require interactive user input mid-workflow.
