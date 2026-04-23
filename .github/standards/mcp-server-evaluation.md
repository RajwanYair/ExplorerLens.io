# MCP Server Evaluation — SQLite + Fetch (§13.1 P2)

> **Status:** Evaluated · Decision: **Adopt both (conditional)**
> **Updated:** 2026-04-23 · Follows `.vscode/mcp.json` conventions (see
> `.github/instructions/mcp-servers.instructions.md`)

---

## Overview

This document records the evaluation of two MCP servers identified as **Priority 2** in
ROADMAP §13.1 for the ExplorerLens AI tooling surface.  Results inform whether each server
should be added to `.vscode/mcp.json`.

---

## 1. SQLite MCP Server (`@modelcontextprotocol/server-sqlite`)

### Purpose

Expose a SQLite database over MCP so agents can:
- Query the ExplorerLens thumbnail cache schema (`cache.db`, `pso-cache.db`)
- Debug multi-tenant cache state without needing raw SQL tooling in the environment
- Run analytical queries during performance investigations (§14.1, §14.2)

### Candidate databases

| Database | Location | Schema owner |
|----------|----------|--------------|
| `cache.db` | `%LOCALAPPDATA%\ExplorerLens\cache\cache.db` | `Engine/Cache/` |
| `pso-cache.db` | `%LOCALAPPDATA%\ExplorerLens\cache\pso-cache.db` | `Engine/Cache/PSOCache.h` |
| `baseline.db` | `data/baselines/` (planned, v39+) | `Engine/Tests/benchmarks/` |

### Evaluation findings

| Criterion | Result |
|-----------|--------|
| Installation | `npx @modelcontextprotocol/server-sqlite <path>` |
| Read-only mode | Supported via `--readonly` flag — **required for cache DBs** |
| Schema exposure | `list_tables`, `describe_table`, `run_query` tools |
| Write risk | Mutations blocked by `--readonly` flag; safe for production DBs |
| OWASP risk | Low — parameterised queries; no raw SQL injection surface via MCP |
| Latency | Negligible (<1 ms) for cache schema queries |
| Agent use case | Performance agent reading hit-rate tables; CI-Ops querying PSO cache health |

### Recommendation

**Adopt** — add to `.vscode/mcp.json` with `--readonly` and a path variable pointing to
the dev-local cache database.  Keep out of CI runners (database doesn't exist in CI).

```jsonc
// .vscode/mcp.json — proposed addition
{
  "servers": {
    "sqlite-cache": {
      "type": "stdio",
      "command": "npx",
      "args": [
        "-y",
        "@modelcontextprotocol/server-sqlite",
        "--readonly",
        "${env:LOCALAPPDATA}/ExplorerLens/cache/cache.db"
      ]
    }
  }
}
```

> **Prerequisite:** The cache database is only present when ExplorerLens has been run at
> least once.  The server starts cleanly even when the file does not exist (returns empty
> table list).

---

## 2. Fetch MCP Server (`@modelcontextprotocol/server-fetch`)

### Purpose

Enable agents (ExplorerLens, Docs, Corpus) to retrieve live format specification documents
from the web without leaving the agent context:
- Fetch JPEG/PNG/WebP/AVIF/JXL/HEIF specification pages
- Download codec reference documents for decoder authoring (§7.1, decoder-authoring skill)
- Retrieve winget package manifests for packaging consistency checks (§12.2)
- Pull IANA media-type registry entries on demand

### Evaluation findings

| Criterion | Result |
|-----------|--------|
| Installation | `npx @modelcontextprotocol/server-fetch` |
| Allowed domains | Configurable via `--allowed-domains` — **always restrict** |
| DNS rebinding | Mitigated by domain allowlist |
| SSRF risk | Mitigated by `--allowed-domains`; never allow `localhost` or `169.254.*` |
| Caching | None (stateless per invocation); use sparingly to avoid rate limits |
| Agent use case | `spec-fetch.prompt.md` prompt; Docs agent research; Corpus manifest retrieval |

### Recommended domain allowlist

```
github.com
raw.githubusercontent.com
www.w3.org
jpeg.org
www.ietf.org
aomediacodec.github.io
nokiatech.github.io
learn.microsoft.com
docs.microsoft.com
winget.run
scoop.sh
```

### Recommendation

**Adopt** — add to `.vscode/mcp.json` with explicit domain allowlist.  CI runners should
**not** use this server (use static spec snapshots instead to avoid flakiness).

```jsonc
// .vscode/mcp.json — proposed addition
{
  "servers": {
    "fetch-specs": {
      "type": "stdio",
      "command": "npx",
      "args": [
        "-y",
        "@modelcontextprotocol/server-fetch",
        "--allowed-domains",
        "github.com,raw.githubusercontent.com,www.w3.org,jpeg.org,www.ietf.org,aomediacodec.github.io,nokiatech.github.io,learn.microsoft.com,winget.run,scoop.sh"
      ]
    }
  }
}
```

---

## 3. Action Items

| Item | Owner | Target |
|------|-------|--------|
| Add `sqlite-cache` server to `.vscode/mcp.json` | Engineering | v39 sprint |
| Add `fetch-specs` server to `.vscode/mcp.json` | Engineering | v39 sprint |
| Update `ai-tooling-capabilities.md` MCP server inventory | AI tooling | same sprint |
| Document both servers in `.github/instructions/mcp-servers.instructions.md` | AI tooling | same sprint |
| Create `data/specs/` directory for offline spec snapshots (CI use) | Corpus agent | v39 sprint |

---

## 4. References

- ROADMAP §13.1 — MCP server integration strategy
- `.github/instructions/mcp-servers.instructions.md` — workspace MCP configuration rules
- `.vscode/mcp.json` — current workspace MCP configuration
- `Engine/Cache/` — cache subsystem that exposes SQLite schemas
- `.github/prompts/spec-fetch.prompt.md` — uses fetch MCP for spec retrieval
- [MCP server-sqlite](https://github.com/modelcontextprotocol/servers/tree/main/src/sqlite)
- [MCP server-fetch](https://github.com/modelcontextprotocol/servers/tree/main/src/fetch)
