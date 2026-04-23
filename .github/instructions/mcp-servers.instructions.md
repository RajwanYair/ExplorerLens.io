---
applyTo: ".vscode/mcp.json"
---

# MCP Server Configuration Rules — ExplorerLens

## Overview

MCP (Model Context Protocol) servers extend Copilot agent capabilities with external
tool access. Configuration lives in `.vscode/mcp.json` (workspace-scoped).

## Current Server Inventory

| Server | Package | Scope | Purpose |
|--------|---------|-------|---------|
| `github` | `@modelcontextprotocol/server-github` | Full GitHub API | Issues, PRs, Actions, repo metadata |
| `filesystem` | `@modelcontextprotocol/server-filesystem` | `${workspaceFolder}` | Full workspace read/write |
| `project-docs` | `@modelcontextprotocol/server-filesystem` | `.github/`, `docs/` | Docs-only editing scope |

## Adding a New MCP Server

### Checklist

1. **Justify the addition** — document what agent workflow requires it.
2. **Minimize scope** — prefer narrow filesystem paths over full workspace access.
3. **Use `npx -y`** — do not install packages globally; `npx -y` ensures latest version.
4. **Include PATH fallback** — Windows PATH must include npm, scoop, and nodejs paths:
   ```json
   "env": {
     "PATH": "${env:APPDATA}\\npm;${env:USERPROFILE}\\scoop\\shims;${env:USERPROFILE}\\scoop\\apps\\nvm\\current\\nodejs\\nodejs;${env:PROGRAMFILES}\\nodejs;${env:PATH}"
   }
   ```
5. **Never hardcode tokens** — use `${input:...}` for secrets with `"password": true`.
6. **Test locally** — run the MCP server command manually before committing.
7. **Update inventory** — after adding a server, update:
   - This file (server inventory table above)
   - `.github/copilot-instructions.md` (MCP Servers section)
   - `.github/standards/ai-tooling-capabilities.md` (MCP inventory)

### Template

```json
"server-name": {
  "command": "npx",
  "args": [
    "-y",
    "@scope/mcp-server-package",
    "${workspaceFolder}\\target-dir"
  ],
  "env": {
    "PATH": "${env:APPDATA}\\npm;${env:USERPROFILE}\\scoop\\shims;${env:PATH}"
  }
}
```

## Security Rules

1. **No secrets in mcp.json** — use `inputs` array with `"password": true` for tokens.
2. **Filesystem scope** — never scope `server-filesystem` to a parent of `${workspaceFolder}`.
3. **Pin packages** — when stability is critical, pin: `"@modelcontextprotocol/server-github@0.7.0"`.
4. **Audit before merge** — any change to `mcp.json` requires security review.
5. **No proxy URLs** — never embed corporate proxy configuration in committed files.

## Removing a Server

1. Remove the server entry from `"servers"` in `mcp.json`.
2. Remove any associated `inputs` entries if no other server uses them.
3. Update the three inventory files listed in step 7 of the checklist above.

## Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| "Cannot find module" | Node.js not on PATH | Add scoop/nvm nodejs paths to `env.PATH` |
| "EACCES" or "Access denied" | Filesystem scope too narrow | Widen the server's path arguments |
| Token prompt on every session | VS Code doesn't cache MCP inputs | Use `"type": "promptString"` — this is expected |
| Server not appearing in Copilot | mcp.json syntax error | Validate JSON; check VS Code Output > MCP panel |

## When NOT to Add an MCP Server

- The functionality is already available via built-in VS Code tools (file search, terminal).
- The server requires a large runtime dependency (e.g., Docker, Python) not in the dev environment.
- The server handles sensitive data that should not transit through agent context.

## Required GitHub Personal Access Token (PAT) Scopes

The `github` MCP server requires a PAT with the following scopes for full agent functionality:

| Scope | Why Needed | Required? |
|-------|-----------|-----------|
| `repo` | Read/write repo contents, issues, PRs, releases | ✅ Required |
| `workflow` | Trigger and view GitHub Actions workflow runs | ✅ Required |
| `read:packages` | Read packages from GitHub Packages (NuGet, Container) | ✅ Required |
| `write:packages` | Publish packages (for Release agent) | ✅ Required |
| `actions:read` | Read Actions secrets, variables, runner groups | ✅ Required for CI-Ops agent |
| `admin:repo_hook` | Manage webhooks | ⏳ Deferred — not needed yet |
| `delete_repo` | Delete repository | ❌ Never grant |

**Verification script:**
```powershell
# Check current PAT scopes (requires gh CLI authenticated)
gh auth status --show-token
# Or via API:
gh api /user -H "Accept: application/json" | Select-Object login
```

If the PAT is missing `workflow` scope, the CI-Ops agent cannot re-run workflows.
If the PAT is missing `write:packages`, the Release agent cannot publish to GitHub Packages.

## Evaluated and Deferred Servers

| Server | Package | Decision | Rationale |
|--------|---------|----------|-----------|
| Git MCP | `@anthropic/mcp-server-git` | **Deferred** | GitKraken MCP (`mcp_gitkraken_*`) already provides git blame, log, diff, stash, branch, status, and push operations. Adding a second git server would be redundant. Re-evaluate if GitKraken MCP is removed. |
| SQLite MCP | `@anthropic/mcp-server-sqlite` | **Deferred** | No SQLite databases in active development workflow yet. Add when cache DB debugging becomes common (Phase 2+). |
| Fetch MCP | `@anthropic/mcp-server-fetch` | **Deferred** | Built-in `fetch_webpage` tool covers URL content needs. Add only if agents need structured HTTP API access to format specification servers. |
